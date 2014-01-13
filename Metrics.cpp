#include "stdafx.h"
#include "Metrics.h"

void Metrics::GetMetrics(Schedule & s){
	sched = s;
	AvgUnfinischedTime();
	AvgReservedTime();
	out.close();
}

void Metrics::AvgUnfinischedTime(){
	// check
	vector <double> unfinishedTimes(data.GetWFCount(),0);
	vector <int> unfinishedTasks(data.GetWFCount(),0);
	vector <int> unscheduledTasks(data.GetWFCount(),0);
	vector <vector<int>> scheduledTasks;
	scheduledTasks.resize(data.GetWFCount());
	double summUnfinishedTime = 0;
	int summUnfinishedTasks = 0;
	// for each p
	for (int i = 0; i < sched.size(); i++){
		int pNum = sched[i].get<0>();
		int tBegin = sched[i].get<1>();
		double execTime = sched[i].get<3>();
		int wfNum, localNum;
		data.GetLocalNumbers(pNum, wfNum, localNum);
		scheduledTasks[wfNum].push_back(localNum);
		// DEADLINES FEATURE
		if ( tBegin + execTime > data.GetT() ){
			unfinishedTimes[wfNum] += tBegin + execTime - data.GetT();
			reservedTime[wfNum] = 0;
			unfinishedTasks[wfNum]++;
			summUnfinishedTasks++;
		}
	}
	//cout << "Average unfinished times and tasks" << endl << "***************************************************"  << endl;
	//cout << "Unfinished times: " << endl;
	for (int i = 0; i < unfinishedTimes.size(); i++){
		// if we have some unscheduled tasks
		if (scheduledTasks[i].size() != data.Workflows(i).GetPackageCount()){
			for (int j = 0; j < data.Workflows(i).GetPackageCount(); j++ ){
				// if package was not scheduled
				if (find(scheduledTasks[i].begin(), scheduledTasks[i].end(), j) == scheduledTasks[i].end()){
					unfinishedTimes[i] += data.Workflows(i).GetAvgExecTime(j);
					unscheduledTasks[i]++;
					reservedTime[i] = 0.0;
				}
			}
		}

		/*cout << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
		<< " unscheduled tasks: " << unscheduledTasks[i] 
		<< " scheduled tasks: " << scheduledTasks[i].size() << endl;*/
		summUnfinishedTime += unfinishedTimes[i];
	}
	/*cout << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
	cout << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;*/

	out << "Average unfinished times and tasks" << endl << "***************************************************" << endl;
	out << "Unfinished times: " << endl;
	for (int i = 0; i < unfinishedTimes.size(); i++){
		out << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
		<< " unscheduled tasks: " << unscheduledTasks[i] 
		<< " scheduled tasks: " << scheduledTasks[i].size() << endl;
	}
	out << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
	out << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;
	int sumUnsched = 0;
	for (int i = 0; i < unscheduledTasks.size(); i++)
		sumUnsched += unscheduledTasks[i];
	out << "Unscheduled tasks count: " << sumUnsched << endl;
	out << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;
}

void Metrics::AvgReservedTime(){
	double summReserved = 0.0;
	for (int i = 0; i < sched.size(); i++){
		int pNum = sched[i].get<0>();
		int tBegin = sched[i].get<1>();
		double execTime = sched[i].get<3>();
		int wfNum, localNum;
		data.GetLocalNumbers(pNum, wfNum, localNum);
		if (reservedTime[wfNum]!=0){
			// if this is the last package of WF
			if (data.Workflows(wfNum).IsPackageLast(localNum)){
				// get its reserved time
				double packageReservedTime = data.GetT() - (tBegin + execTime);
				// if this time is positive
				if ( packageReservedTime > 0 ){
					// if it is first positive reserve, save it
					if (reservedTime[wfNum] == numeric_limits<double>::infinity())
						reservedTime[wfNum] = packageReservedTime;
					// otherwise if saved reserve > than new reserve
					// we should save new reserve
					else if (packageReservedTime < reservedTime[wfNum])
						reservedTime[wfNum] = packageReservedTime;
				}
				else reservedTime[wfNum] = 0;
			}
		}
	}
	//cout << "Reserved times" << endl << "***************************************************"  << endl;
	for (int i = 0; i < reservedTime.size(); i++){
		//cout << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
		summReserved += reservedTime[i];
	}
	//cout << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;

	out << "Reserved times" << endl << "***************************************************"  << endl;
	for (int i = 0; i < reservedTime.size(); i++){
		out << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
	}
	out << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
}

Metrics::~Metrics(void)
{
}
