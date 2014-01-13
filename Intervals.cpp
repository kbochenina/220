#include "StdAfx.h"
#include "Intervals.h"
#include "UserException.h"

void Intervals::SetData(vector<BusyIntervals> i, ModelingContext& c){ 
	init = i; 
	context = c;
	if (init.size() > 0) 
		// numCoresPerOneRes * numResources
		numCores = init[0].size() * init.size(); 
	else numCores = 0; 
	int stageCount = context.GetStages();
	// allCoresFreeTimes[stagecount * numcores]
	allCoresFreeTimes.resize(stageCount);
	for (int i = 0; i < stageCount; i++)
		allCoresFreeTimes[i].resize(numCores);
	Correct();
	SetFreeTime();
}

void Intervals::Correct(){
	const vector<int> &stageBorders = context.GetBorders();
	// for each resource intervals
	for (vector<BusyIntervals>::iterator resIt = init.begin(); resIt!= init.end(); resIt++){
		// for each core
		for (BusyIntervals::iterator it = resIt->begin(); it!= resIt->end(); it++){
			vector <int> toEraseNums;
			int newVal = 0; 
			for (vector<pair<int,int>>::iterator it2 = it->second.begin(); it2!=it->second.end(); it2++){
				// if second border of changed interval > than left border of next interval
				// we need to join it
				if (newVal > it2->first){
					// left border of joined interval is equal to left border of changed interval
					it2->first = (it2-1)->first;
					// after joining changed interval will be erased
					toEraseNums.push_back(distance(it->second.begin(), it2-1));
				}
				int bEnd = it2->second;
				for (unsigned int i = 0; i < stageBorders.size()-1; i++) {
					// if end of interval is among stageBorders[i] and stageBorders[i+1]
					// it will be equal to stageBorders[i+1]
					if (stageBorders[i] < bEnd && stageBorders[i+1] > bEnd) {
						newVal = it2->second = stageBorders[i+1];
					}
				}
			}
			// erasing unused intervals
			for (unsigned int i = 0; i < toEraseNums.size(); i++){
				it->second.erase(it->second.begin()+toEraseNums[i]);
			}
		}
	}
	SetInit();
}

// ((core1 freeTime, core2 FreeTime), (core 1 freeTime, core2 FreeTime)) - stage 
void Intervals::SetFreeTime(){
	int delta = context.GetDelta();
	int coreIndex = 0; //?
	// for each resource
	for (vector<BusyIntervals>::iterator resIt = current.begin(); resIt!= current.end(); resIt++){
		// for each core
		for (BusyIntervals::iterator it = resIt->begin(); it!= resIt->end(); it++){
			// if core has no busy windows, all its free times = delta
			if (it->second.size()==0) {
				for (int i = 0; i < context.GetStages(); i++)
					allCoresFreeTimes[i][coreIndex] = delta;
				coreIndex++;
				continue;
			}
			vector <pair<int, int>> intervals = it->second;
			int indexBusyInterval = 0;
			for (int i = 0; i < context.GetT(); i+=delta){
				int bBegin = intervals[indexBusyInterval].first;
				int bEnd = intervals[indexBusyInterval].second;
				// if right border are not equal to stage border
				// we should ceil it to highest stage border
				if (bEnd % delta!=0) {
					bEnd = (bEnd/delta + 1) * delta;
				}
				// debug part
				if (coreIndex > numCores - 1) 
					throw UserException("Resource::GetFreeTime(): wrong coreIndex");
				// if current stage is not busy by [bBegin; bEnd],
				// allCoresFreeTimes = delta
				if (i < bBegin && i+delta <= bBegin) 
					allCoresFreeTimes[i/delta][coreIndex] = delta;
				// if bBegin is in [i; i+delta]
				else {
					// set particle free time for current stage
					allCoresFreeTimes[i/delta][coreIndex] = bBegin - i;
					// move to next stage
					i+=delta;
					// bEnd is divisible by i
					while(i != bEnd) {
						// debug part
						if (i > context.GetT()) {
							/*vector <pair<int, int>>::iterator it = intervals.begin();
							for (; it!= intervals.end(); it++){
								cout << it->first << " " << it->second << endl;
							}
							cout << "bBegin = " << bBegin << endl;
							cout << "bEnd = " << bEnd << endl;*/
							throw UserException("Intervals::SetFreeTime(): out of range exception");
						}
						allCoresFreeTimes[i/delta][coreIndex] = 0;
						i+=delta;
					}

					indexBusyInterval++;
					// if there are no more intervals  
					if (indexBusyInterval == intervals.size())
						while (i < context.GetT()) {
							allCoresFreeTimes[i/delta][coreIndex] = delta;
							i+=delta;
						}
					// to cancel last increment of i (in first or second while)
					i-=delta;
				}
			}
			coreIndex++;
		}
	}
}

// find placement !for 1 processor for execTime
// tbegin is in/out parameter (in - earliest start time of package)
// processor is out parameters
// we can use non-full execution times in that case
// function return false and (-1, -1) as out parameters 
// if we have not time at all processors
bool Intervals::FindPlacement(const double &execTime, int &tbegin, int& processor) const{
	vector <double> tb(numCores, context.GetT());
	int currentProcessor = 0;
	// cycle for resources
	for (int i = 0; i < current.size(); i++){
		const BusyIntervals & bi = current[i];
		// cycle for processors
		for (BusyIntervals::const_iterator j = bi.begin(); j != bi.end(); j++){
			double begin = tbegin;
			// if we have no busy intervals for this processor
			// we can start execution from tbegin
			if (j->second.size()==0){
				tb[currentProcessor++] = tbegin;
				continue;
			}
			// cycle for intervals
			for (int k = 0; k < j->second.size(); k++){
				if (j->second[k].second <= begin)
					continue;
				if (begin + execTime <= j->second[k].first){
					tb[currentProcessor] = begin;
					break;
				}
				else
					begin = j->second[k].second;
				if ( k == j->second.size()-1 )
					tb[currentProcessor] = begin;

			}
			currentProcessor++;
		}
	}
	int trealbegin = context.GetT();
	// find resource with earliest finishing time
	for (int i = 0; i < tb.size(); i++){
		// if starting time is less than previous
		// and more than earliest finishing time
		if (tb[i] < trealbegin && tb[i]>=tbegin){
			trealbegin = tb[i];
			processor = i;
		}
	}
	tbegin = trealbegin;
	if (tbegin == context.GetT()){
		processor = -1;
		tbegin = -1;
		return false;
	}
	return true;
}

// add busy intervals [tBegin; (tBegin + execTime) round to highest stage border] to cores in coreNumbers
void Intervals::AddDiaps(vector <int> coreNumbers, int tBegin, double execTime)  {
	for (int i = 0; i < coreNumbers.size(); i++){
		int processor = coreNumbers[i];
		int number = 0;
		for (int i = 0; i < current.size(); i++){
			for (BusyIntervals::iterator j = current[i].begin(); j != current[i].end(); j++){
				if (number != processor){
					number++;
					continue;
				}
				else {
					if (j->second.size() == 0 || tBegin < j->second[0].first){
						pair<int,int> newPair = make_pair(tBegin, tBegin + execTime + 1);
						j->second.insert(j->second.begin(), newPair);
						return;
					}

					// cycle for intervals
					for (int k = 0; k < j->second.size(); k++){
						// if tbegin > last tend
						if (tBegin >= j->second[k].second )
							// and this is last interval
							// or we can insert this interval among two intervals
							if (k == j->second.size() - 1 || tBegin + execTime <= j->second[k+1].second){
							pair<int,int> newPair = make_pair(tBegin, tBegin + execTime + 1);
							j->second.insert(j->second.begin() + k + 1, newPair);
							return;
						}
					}
				}
			}
		}
	}
}

Intervals::~Intervals(void)
{
}
