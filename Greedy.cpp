#include "stdafx.h"
#include "Greedy.h"
#include <iostream>

using namespace std;


Greedy::Greedy(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){ 

}

double Greedy::GetWFSchedule(Schedule &out){
	double eff = 0.0;
	switch (wfNum){
	case -1: 
		eff = GetFullSchedule(out); break;
	default:
		eff = GetOneWFSchedule(out); break;
	}
	return eff;
}



double Greedy::GetFullSchedule(Schedule& out){
	double efficiency = 0.0;
	vector <int> finished;
	data.SetPriorities();
	// while all packages are not scheduled
	double minFinishingTime = 0.0;
	int tbegin, processor = 0;
	double time = 0;
	while (finished.size() != data.GetPackagesCount()){
		tbegin = 0;
		// get unscheduled package with minimum finishing count
		int current = data.GetNextPackage();
		// get wfNum and package local num
		int wfNum = 0, localNum = 0;
		data.GetLocalNumbers(current, wfNum, localNum);
		// should get latest finishing time of previous packages
		vector<int> dependsOn;
		data.Workflows(wfNum).GetInput(localNum, dependsOn);
		for (int i = 0; i < dependsOn.size(); i++){
			// set global numbers
			for (auto& j : out){
				if (boost::get<0>(j) == dependsOn[i] + current - localNum)
				{
					int resType = data.GetResourceTypeIndex(boost::get<2>(j)[0]);
					double tEnd = boost::get<1>(j) + data.Workflows(wfNum).GetExecTime(dependsOn[i], resType + 1, 1);
					if (tEnd > tbegin)
						tbegin = tEnd + 1;
				}
			}
		}
		// getting possible types of resources
		vector <int> resTypes = data.Workflows(wfNum)[localNum].GetResTypes();
		vector <double> execTime;
		for (int i = 0; i < resTypes.size(); i++){
			execTime.push_back(data.Workflows(wfNum)[localNum].GetExecTime(resTypes[i],1));
		}
		vector<int> viewedResources;
		// processor, tbegin, tend, resIndex
		boost::tuple<int, double, double, int> savedPlan;
		double bestTimeEnd = std::numeric_limits<double>::infinity();
		bool planWasFound = false;
		
		while (viewedResources.size() != resTypes.size()){
			double min = std::numeric_limits<double>::infinity();
			int minResIndex = 0;
			for (int i = 0; i < resTypes.size(); i++){
				if (execTime[i] < min 
					&& find(viewedResources.begin(), viewedResources.end(), i) == viewedResources.end()){
						min = execTime[i];
						minResIndex = i;
				}
			}
			// trying to find avaliable resource with minimum execution time
			if (data.Resources(minResIndex).FindPlacement(min, tbegin, processor)){
				// if we can finish package execution before DEADLINE
				if (tbegin + min <= data.GetT()){
					savedPlan.get<0>() = processor; 
					savedPlan.get<1>() = tbegin; 
					savedPlan.get<2>() = tbegin + min; 
					savedPlan.get<3>() = minResIndex;
					planWasFound = true;
					break;
				}
				else {
					if (tbegin + min < bestTimeEnd){
						savedPlan.get<0>() = processor; 
						savedPlan.get<1>() = tbegin; 
						savedPlan.get<2>() = tbegin + min; 
						savedPlan.get<3>() = minResIndex;
						bestTimeEnd = tbegin + min;
					}
					planWasFound = true;
				}
			}
			viewedResources.push_back(minResIndex);
		}
		if (planWasFound){
			double tbegin = static_cast<double>(savedPlan.get<1>());
			double execTime = savedPlan.get<2>()-tbegin;
			data.Resources(savedPlan.get<3>()).AddInterval(execTime, 
				tbegin, savedPlan.get<0>());
			time = execTime;
			finished.push_back(current);
			int procGlobalIndex = data.GetGlobalProcessorIndex(savedPlan.get<3>(), savedPlan.get<0>());
			vector <int> processors;
			processors.push_back(procGlobalIndex);
			out.push_back(make_tuple(current, tbegin, processors, execTime));
			// add efficiency value
			int tend = (tbegin + time > data.GetT()) ? data.GetT() : tbegin + time;
			efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
			}
		else {
			//cout << "Can not find placement for package " << current << endl;
			finished.push_back(current);
		}
		
		// fix resource

		// if we cannot find the resource, we should mark this package
	}
	//cout << "Efficiency value: " << efficiency << endl;
	return efficiency;
}

double Greedy::GetOneWFSchedule(Schedule& out){
	
	double efficiency = 0.0;
	vector <int> finished;
	int index = data.Workflows(wfNum).GetPackageCount() - 1;
	double minFinishingTime = 0.0;
	int tbegin, processor = 0;
	double time = 0;
	/// while all packages are not scheduled
	while (finished.size() != data.Workflows(wfNum).GetPackageCount()){
		tbegin = 0;
		// get unscheduled package with minimum finishing count
		int current = data.GetNextPackage(wfNum,index--);
		
		// should get latest finishing time of previous packages
		vector<int> dependsOn;
		data.Workflows(wfNum).GetInput(current, dependsOn);
		for (int i = 0; i < dependsOn.size(); i++){
			// set global numbers
			for (auto& j : out){
				// if current depends on this package
				if (boost::get<0>(j) == dependsOn[i])
				{
					double tEnd = boost::get<1>(j) + boost::get<3>(j);
					// and tend of this package > current package beginning time,
					// set new beginning time
					if (tEnd > tbegin)
						tbegin = tEnd + 1;
				}
			}
		}
		// getting possible types of resources
		vector <int> resTypes = data.Workflows(wfNum)[current].GetResTypes();
		vector <double> execTime;
		for (int i = 0; i < resTypes.size(); i++){
			execTime.push_back(data.Workflows(wfNum)[current].GetExecTime(resTypes[i],1));
		}
		vector<int> viewedResources;
		// processor, tbegin, tend, resIndex
		boost::tuple<int, double, double, int> savedPlan;
		double bestTimeEnd = std::numeric_limits<double>::infinity();
		bool planWasFound = false;
		// for each resource type
		while (viewedResources.size() != resTypes.size()){
			double min = std::numeric_limits<double>::infinity();
			int minResIndex = 0;
			// find next minimum exec time
			for (int i = 0; i < resTypes.size(); i++){
				if (execTime[i] < min 
					&& find(viewedResources.begin(), viewedResources.end(), i) == viewedResources.end()){
						min = execTime[i];
						minResIndex = i;
				}
			}
			// trying to find avaliable resource with minimum execution time
			if (data.Resources(minResIndex).FindPlacement(min, tbegin, processor)){
				// if we can finish package execution before DEADLINE
				if (tbegin + min <= data.GetT()){
					savedPlan.get<0>() = processor; 
					savedPlan.get<1>() = tbegin; 
					savedPlan.get<2>() = tbegin + min; 
					savedPlan.get<3>() = minResIndex;
					planWasFound = true;
					break;
				}
				else {
					// if this plan is violating package deadline
					// we compare it with previous saved plan
					// and prefer better plan
					if (tbegin + min < bestTimeEnd){
						savedPlan.get<0>() = processor; 
						savedPlan.get<1>() = tbegin; 
						savedPlan.get<2>() = tbegin + min; 
						savedPlan.get<3>() = minResIndex;
						bestTimeEnd = tbegin + min;
					}
					planWasFound = true;
				}
			}
			viewedResources.push_back(minResIndex);
		}
		// if we found the plan
		if (planWasFound){
			double tbegin = static_cast<double>(savedPlan.get<1>());
			double execTime = savedPlan.get<2>()-tbegin;
			data.Resources(savedPlan.get<3>()).AddInterval(execTime, 
				tbegin, savedPlan.get<0>());
			time = execTime;
			finished.push_back(current);
			int procGlobalIndex = data.GetGlobalProcessorIndex(savedPlan.get<3>(), savedPlan.get<0>());
			vector <int> processors;
			processors.push_back(procGlobalIndex);
			out.push_back(make_tuple(current, tbegin, processors, execTime));
			// add efficiency value
			int tend = (tbegin + time > data.GetT()) ? data.GetT() : tbegin + time;
			efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
		}
		else {
			//cout << "Can not find placement for package " << current << endl;
			finished.push_back(current);
		}
	}
	//cout << "Efficiency value: " << efficiency << endl;
	return efficiency;
}

void Greedy::printInfo(){
	//std::cout << "BellmanScheme is instantiated\n";
	//std::cout << "koeff = " << eff->GetKoeff() << "\n";
}

Greedy::~Greedy(void)
{
}
