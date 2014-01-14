#include "StdAfx.h"
#include "Scheduler.h"
#include "UserException.h"
#include "SchedulingFactory.h"
#include "CriteriaFactory.h"
#include "ScheduleToXML.h"
// in StagedScheme()
#include <time.h>
// StagedScheme() : to_string()
#include <string>
#include <iostream>
// StagedScheme() : ofstream
#include <fstream>
#include "direct.h"

using namespace std;


Scheduler::Scheduler( ModelData& md ): data(md.GetData())
{
	methodsSet.resize(data.GetWFCount());
	maxEff = 0.0;
	xmlWriter = unique_ptr<ScheduleToXML>(new ScheduleToXML(data));
	eff = unique_ptr<Efficiency>(new Efficiency(2.00 / data.GetFullCoresCount(), data.GetT()));
	maxPossible = 0.0;
	for (int i = 0; i < data.GetResourceCount(); i++){
		Intervals initIntervals;
		data.Resources(i).GetCurrentWindows(initIntervals);
		vector <BusyIntervals> current;
		current = initIntervals.GetCurrentIntervals();
		for (int j = 0; j < current.size(); j++){
			BusyIntervals & processorIntervals = current[j];
			for (auto it = processorIntervals.begin(); it != processorIntervals.end(); it++){
				for (int k = 0; k < it->second.size(); k++){
					maxPossible += eff->EfficiencyByPeriod(1, it->second[k].first, it->second[k].second);
				}
			}
		}
	}
	maxPossible = 1.00 - maxPossible;
}


Scheduler::~Scheduler(void)
{
}

void Scheduler::SetSchedulingStrategy(int strategyNumber)
{
	switch (strategyNumber)
	{
		// only Bellman
	case 1: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 1;
		break;
		// only Greedy
	case 2: for (unsigned int i = 0; i < methodsSet.size(); i++)
				methodsSet[i] = 2;
		break;
		// mixed
	}
}

// implements staging scheme for finding the schedule for WFs set
// <PRE> 0 <= firstWfNum < data.workflows.size()
double Scheduler::StagedScheme(int firstWfNum){
	cout << "StagedScheme(int) was called\n";
	try{
		int wfCount = data.GetWFCount();
		if (firstWfNum < 0 || firstWfNum > wfCount) 
			throw UserException("Scheduler::StagedScheme(int) error. Wrong init workflow number");
		// creating XML with init time windows
		//xmlWriter->SetXMLBaseName("Init_");
		Schedule oneWFsched;
		//xmlWriter->CreateXML(oneWFsched, -1);
		// ??!! think about it !
		xmlWriter->SetXMLBaseName("Staged_");
		//double stagedT = clock();

		//string resFileName = "staged_scheme_" + to_string(firstWfNum) + ".txt";
		//ofstream res(resFileName);
		//if (res.fail()) 
		//	throw UserException("Scheduler::StagedScheme(int) error. Unable to create res file");
		//res << "Stage 1, workflow # " << firstWfNum << endl;
		//cout << "Stage 1, workflow # " << firstWfNum << endl;
		
		vector <double> eff;
		// applying settings of scheduling method for initial WF
		unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[firstWfNum], firstWfNum);
		method->printInfo();
		// getting schedule for first WF
		double oneStepStart = clock();
		eff.push_back(method->GetWFSchedule(oneWFsched));

		/*	ReadData(firstWfNum);
		int currentWfNum = firstWfNum;
		directBellman = false;
		BackBellmanProcedure();
		directBellman = true;
		eff.push_back(DirectBellman(firstWfNum));
		allStagesCores = stagesCores;
		BellmanToXML(true);
		//std::system("pause");
		stagesCores.clear();
		FixNewBusyIntervals();
		BellmanToXML(true);
		//std::system("pause");
		states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();*/

		// set local to global packages
		int initNum = data.GetInitPackageNumber(firstWfNum);
		for (int i = 0; i < oneWFsched.size(); i++)
			oneWFsched[i].get<0>() += initNum;


		fullSchedule = oneWFsched;
		
		//cout << "Elapsed time: " << (clock()-oneStepStart)/1000.0 << " sec" << endl;
		scheduledWFs.push_back(firstWfNum);	
		//xmlWriter->CreateXML(oneWFsched, firstWfNum);
		// write result to XML
		data.FixBusyIntervals();

		
		// write result to res file
		//PrintOneWFSched(res, oneWFsched, firstWfNum);
		
		
		// we need to store current busy intervals
		// of schedule that give the best efficiency
		// current best schedule is stored in oneWFsched
		vector<vector <BusyIntervals>> storedIntervals;
		Schedule storedSched;

		while (scheduledWFs.size() != wfCount ){
			//cout << "Stage " << scheduledWFs.size() + 1 << endl;
			double stageMaxEff = -1.0;
			int bestWfNum = -1;
			for (int i = 0; i < wfCount; i++){
				// if this WF wasn't scheduled yet
				if (find(scheduledWFs.begin(), scheduledWFs.end(), i) == scheduledWFs.end()){
					//cout << "CurrentWfNum = " << i << " ";
					oneStepStart = clock();
					method = SchedulingFactory::GetMethod(data, methodsSet[i], i);
					oneWFsched.clear();
					double currentEff = method->GetWFSchedule(oneWFsched);
					method->printInfo();
					//cout << "Elapsed time: " << (clock()-oneStepStart)/1000.0 << " sec" << endl;
					/*ReadData(i);
					directBellman = false;
					BackBellmanProcedure();
					directBellman = true;
					double currentEff = DirectBellman(i);*/
					if (stageMaxEff < currentEff){
						stageMaxEff = currentEff;
						bestWfNum = i;
						storedSched = oneWFsched;
						storedIntervals.clear();
						data.GetCurrentIntervals(storedIntervals);
						//GetBestBusyIntervals(bestBusyIntervals);
					}
					data.ResetBusyIntervals(); // newfag in my program
					//states.clear(); controls.clear(); nextStateNumbers.clear(); stagesCores.clear();
				}
			}
			// set local to global packages
			int initNum = data.GetInitPackageNumber(bestWfNum);
			for (int i = 0; i < storedSched.size(); i++)
				storedSched[i].get<0>() += initNum;

			copy(storedSched.begin(), storedSched.end(), back_inserter(fullSchedule));
			//copy(bestStagesCores.begin(), bestStagesCores.end(),back_inserter(allStagesCores));
			scheduledWFs.push_back(bestWfNum);
			//usedNums = scheduledWFs; ???
			//stagesCores = bestStagesCores;
			//currentWfNum = bestWfNum;
			eff.push_back(stageMaxEff);
			// set current intervals as stored intervals
			data.SetCurrentIntervals(storedIntervals);
			// write result to XML
			// xmlWriter->CreateXML(storedSched, bestWfNum);
			// write result to res file
		//	PrintOneWFSched(res, storedSched, bestWfNum);
			  
			data.FixBusyIntervals();
			
			/*SetBestBusyIntervals(bestBusyIntervals);
			FixNewBusyIntervals();
			BellmanToXML(true);*/
			//std::system("pause");
		}
		/*usedNums = scheduledWFs; ???
		SetFirstBusyIntervals();
		stagesCores = allStagesCores;
		BellmanToXML(false);*/
		//PrintFooter(res, eff);
		double sumEff = 0.0;
		for (int i = 0; i < eff.size(); i++)
			sumEff += eff[i];
		
		data.SetInitBusyIntervals();
		//xmlWriter->CreateXML(fullSchedule, -1);
		//res.close();
		cout << "Max eff: " << sumEff/maxPossible << endl;
		//cout << "Elapsed time: " << (clock()-stagedT)/1000.0 << " sec" << endl;
		return sumEff/maxPossible ;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

void Scheduler::StagedScheme(vector<int>& order){
	cout << "StagedScheme(vector<int>&) was called\n";
}

void Scheduler::GetSchedule(int scheduleVariant){
	Schedule storedSched;
	data.SetInitBusyIntervals();
	fullSchedule.clear();
	int bestStage = 0;
	ofstream resTime("time.txt", ios::app);
	double t = 0, end = 0, stagedTime = 0;
	switch (scheduleVariant)
	{
		case 1:
			resTime << "Staged scheme: " << endl;
			data.SetWfPriorities();
			maxEff = 0;
			for (int i = 0; i < data.GetWFCount(); i++) {
				scheduledWFs.clear();
				t = clock();
				double eff = StagedScheme(i);
				stagedTime = (clock() - t)/1000.0;
				end += stagedTime;
				resTime << "Time of executing stage " << i+1 << " " << stagedTime << endl;
				if (eff > maxEff){
					maxEff = eff;
					storedSched = fullSchedule;
					bestStage= i;
				}
			}
			cout << "Time of executing staged scheme " << end << endl;
			resTime << "Time of executing staged scheme " << end << endl;
			cout << "Average time of stage " << end/data.GetWFCount() << endl;
			resTime << "Average time of stage " << end/data.GetWFCount() << endl;
			fullSchedule = storedSched;
			xmlWriter->CreateXML(fullSchedule, -1);
			cout << "Best stage: " << bestStage << endl;
			break;
		case 2:
		{
			vector <int> order;
			for (int i = 0; i < data.GetWFCount(); i++)
				order.push_back(i);
			StagedScheme(order);
			break;
		}
		case 3: 
			// simple sched
			t = clock();
			SimpleSched();
			end = (clock() - t)/1000.0;
			cout << "Time of executing simple scheduling algorithm " << end << endl;
			resTime << "Time of executing simple scheduling algorithm " << end << endl;
			break;
		case 4:
			// reserved_ordered sched
			data.SetWfPriorities();
			t = clock();
			OrderedScheme(1);
			end = (clock() - t)/1000.0;
			cout << "Time of executing ordered scheme " << end << endl;
			resTime << "Time of executing ordered scheme " << end << endl;
			break;
		default:
			break;
		}
	resTime.close();
}

// scheduling ordered due to prioretization criteria
void Scheduler::OrderedScheme(int criteriaNumber){
	maxEff = 0.0;
	// get pointer to criteria 
	unique_ptr<CriteriaMethod> criteria = CriteriaFactory::GetMethod(data,criteriaNumber);
	bool tendsToMin = criteria->TendsToMin();
	// unscheduled WF numbers
	vector <int> unscheduled;
	for (int i = 0; i < data.GetWFCount(); i++)
		unscheduled.push_back(i);
	// while we have unscheduled WFs
	while (unscheduled.size() != 0){
		// best schedule (from the prioretization criteria point of view)
		Schedule best;
		// and "best" wf number
		int bestWFNum = 0;
		// max eff (on this iteration)
		double currentBestEff = 0.0;
		// current best criteria value
		double bestCriteria = tendsToMin ? numeric_limits<double>::max() : 
			-1 * numeric_limits<double>::max();
		// busy intervals for best schedule

		vector<vector <BusyIntervals>> storedIntervals;
		// for each unscheduled WF
		for (auto &wfNum : unscheduled){
			Schedule current;
			unique_ptr <SchedulingMethod> method = 
				SchedulingFactory::GetMethod(data, methodsSet[wfNum], wfNum);
			// get current schedule in current variable
			double currentEff = method->GetWFSchedule(current);
			// get current criteria
			double currentCriteria = criteria->GetCriteria(current);
			if (criteria->IsBetter(currentCriteria, bestCriteria)){
				best = current;
				bestWFNum = wfNum;
				currentBestEff = currentEff;
				bestCriteria = currentCriteria;
				storedIntervals.clear();
				data.GetCurrentIntervals(storedIntervals);
			}
			data.ResetBusyIntervals();
		}

		// set local to global packages
		int initNum = data.GetInitPackageNumber(bestWFNum);
		for (int i = 0; i < best.size(); i++)
			best[i].get<0>() += initNum;
		// add best schedule to full schedule
		copy(best.begin(), best.end(), back_inserter(fullSchedule));

		data.SetCurrentIntervals(storedIntervals);
		data.FixBusyIntervals();

		maxEff += currentBestEff;

		cout << "Best wf num: " << bestWFNum << " bestCriteria: " << bestCriteria << endl;

		auto idx = find(unscheduled.begin(), unscheduled.end(), bestWFNum);
		unscheduled.erase(idx);
	}
	data.SetInitBusyIntervals();
	maxEff /= maxPossible;
	cout << "Ordered scheme eff: " << maxEff << endl;
	xmlWriter->SetXMLBaseName("Ordered_");
	// write result to XML
	xmlWriter->CreateXML(fullSchedule, -1);
	string resFileName = "ordered.txt";
	ofstream res(resFileName);
	if (res.fail()) 
		throw UserException("Scheduler::OrderedScheme error. Unable to create res file");
	PrintOneWFSched(res, fullSchedule, -1);
	res.close();
}

void Scheduler::SimpleSched(){
	// third parameter = -1 means that we will find the schedule for whole big WF
	unique_ptr <SchedulingMethod> method = SchedulingFactory::GetMethod(data, methodsSet[0], -1);
	// get full schedule
	maxEff = method->GetWFSchedule(fullSchedule);
	maxEff = maxEff/maxPossible;
	cout << "Efficiency: " << maxEff << endl;
	//data.FixBusyIntervals();
	xmlWriter->SetXMLBaseName("Simple_");
	// write result to XML
	xmlWriter->CreateXML(fullSchedule, -1);
	string resFileName = "simple.txt";
	ofstream res(resFileName);
	if (res.fail()) 
		throw UserException("Scheduler::SimpleSched error. Unable to create res file");
	PrintOneWFSched(res, fullSchedule, -1);
	data.SetInitBusyIntervals();
	res.close();
}

// add to file info about schedule
void Scheduler::PrintOneWFSched(ofstream & res, Schedule & sched, int wfNum){
	res << "WF " << wfNum << endl;
	for (Schedule::iterator it = sched.begin(); it!= sched.end(); it++){
		res << "(" << it->get<0>() << " " << it->get<1>() << " " << it->get<3>() << " ";
		for (vector<int>::iterator it2 = it->get<2>().begin(); it2 != it->get<2>().end(); it2++)
			res << *it2 ;
		res << "))";
	}
	res << endl;
}

// add to res file additional schedule information
void Scheduler::PrintFooter(ofstream & res, vector<double>&eff){
	res << "Workflow order: " ;
		for (vector<int>::size_type i = 0; i < scheduledWFs.size(); i++){
			res << scheduledWFs[i] << " ";
		}
	res << endl << "Efficiencies: " ;
	for (vector<int>::size_type i = 0; i < eff.size(); i++){
		res << eff[i] << " ";
	}
	res << endl << "Max eff: " << maxEff << endl << endl;
}

void Scheduler::GetMetrics(string filename){
	Metrics m(data, filename);
	m.GetMetrics(fullSchedule);
	ofstream out(filename, ios::app);
	out << "Efficiency: " << maxEff << endl;
}

void Scheduler::TestSchedule(){
	Test t(data, fullSchedule);
	if (t.TestIntervals())
		cout << "Test intervals passed" << endl;
	if (t.TestWFLinks())
		cout << "Test wf links passed" << endl;
}