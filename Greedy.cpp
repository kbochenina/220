#include "stdafx.h"
#include "Greedy.h"
#include <iostream>

using namespace std;


Greedy::Greedy(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){ 

}

double Greedy::GetWFSchedule(Schedule &out){
    double eff = 0.0;
    switch (wfNum){
    case -1: {
        eff = GetFullSchedule(out); break;
    }
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
    int expectedBegin, processor = 0;
    double time = 0;
    double deadline = data.GetDeadline();
    while (finished.size() != data.GetPackagesCount()){
        if (data.GetPrioritiesSize() == 0)
            cout << "finished.size() = " << finished.size() << endl;
        expectedBegin = 0;
        // get unscheduled package with minimum finishing count
        int current = data.GetNextPackage();
        // get wfNum and package local num
        int wfNum = 0, localNum = 0;
        data.GetLocalNumbers(current, wfNum, localNum);
        // should get latest finishing time of previous packages
        vector<int> dependsOn;
        data.Workflows(wfNum).GetInput(localNum, dependsOn);
        // commTimeToType[i] = max(commTime) from other types
        vector <double> commTimeToType(data.GetResourceCount(),0.0);
        for (int i = 0; i < dependsOn.size(); i++){
            // set global numbers
            for (auto& j : out){
                if (boost::get<0>(j) == dependsOn[i] + current - localNum)
                {
                    int resType = data.GetResourceTypeIndex(boost::get<2>(j)[0]);
                    double execTime = data.Workflows(wfNum).GetExecTime(dependsOn[i], resType + 1, 1);
                    double tEnd = boost::get<1>(j) + execTime;
                    if (tEnd > expectedBegin)
                        expectedBegin = tEnd + 1;
                    for (int i = 0; i < commTimeToType.size(); i++){
                        if ( i!= resType )
                            commTimeToType[i] += execTime;
                    }
                }
            }
        }
        for (int i = 0; i < commTimeToType.size(); i++)
            commTimeToType[i] *= data.GetCCR();
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
        for (auto &res : resTypes){
                // resources indexed from 1
                res -=1;
                int tbegin = expectedBegin + commTimeToType[res];
                if (data.Resources(res).FindPlacement(execTime[res], tbegin, processor, deadline)){
                    if (tbegin + execTime[res] < bestTimeEnd){
                        savedPlan.get<0>() = processor; 
                        savedPlan.get<1>() = tbegin;
                        savedPlan.get<2>() = tbegin + execTime[res]; 
                        savedPlan.get<3>() = res;
                        bestTimeEnd = tbegin + execTime[res];
                    }
                    planWasFound = true;
                }
        }
       
        if (planWasFound){
            //cout << wfNum << endl;
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
            int tend = (tbegin + time > data.GetDeadline()) ? data.GetDeadline() : tbegin + time;
            efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
            }
        else {
            //cout << "Can not find placement for package " << current << endl;
            finished.push_back(current);
            int initPackageNum = data.GetInitPackageNumber(wfNum);
            // get all successors of this package
            vector <int> successors;
            data.Workflows(wfNum).GetSuccessors(current - initPackageNum, successors);
            
            for (auto & i : successors){
                // transform local to global
                i += initPackageNum;
                if (find(finished.begin(), finished.end(),i) == finished.end())
                    finished.push_back(i);
                
            }
            // remove successor from priority queue
            data.RemoveFromPriorities(successors);
        }
        
        // fix resource

        // if we cannot find the resource, we should mark this package
    }
     return efficiency;
}

double Greedy::GetOneWFSchedule(Schedule& out){
    
    double efficiency = 0.0;
    vector <int> finished;
    int index = data.Workflows(wfNum).GetPackageCount() - 1;
    double minFinishingTime = 0.0;
    int expectedBegin, processor = 0;
    double time = 0;
    vector<int> unscheduled;
    /// while all packages are not scheduled
    while (finished.size() != data.Workflows(wfNum).GetPackageCount()){
        expectedBegin = 0;
        // get unscheduled package with minimum finishing count
        int current = data.Workflows(wfNum).GetNextPackage(index--);
        if (find(unscheduled.begin(), unscheduled.end(), current) != unscheduled.end())
            continue;
        // should get latest finishing time of previous packages
        vector<int> dependsOn;
        data.Workflows(wfNum).GetInput(current, dependsOn);
        // commTimeToType[i] = max(commTime) from other types
        vector <double> commTimeToType(data.GetResourceCount(),0.0);
        for (int i = 0; i < dependsOn.size(); i++){
            // set global numbers
            for (auto& j : out){
                // if current depends on this package
                if (boost::get<0>(j) == dependsOn[i])
                {
                    int resType = data.GetResourceTypeIndex(boost::get<2>(j)[0]);
                    double tEnd = boost::get<1>(j) + boost::get<3>(j);
                    // and tend of this package > current package beginning time,
                    // set new beginning time
                    if (tEnd > expectedBegin)
                        expectedBegin = tEnd + 1;
                    for (int i = 0; i < commTimeToType.size(); i++){
                    if ( i!= resType )
                        commTimeToType[i] += boost::get<3>(j);
                    }
                }
            }
        }
        for (int i = 0; i < commTimeToType.size(); i++)
            commTimeToType[i] *= data.GetCCR();
        
        // getting possible types of resources
        vector <int> resTypes = data.Workflows(wfNum)[current].GetResTypes();
        vector <double> execTime;
        for (int i = 0; i < resTypes.size(); i++){
            execTime.push_back(data.Workflows(wfNum)[current].GetExecTime(resTypes[i],1));
        }
        vector<int> viewedResources;
        double deadline = data.GetDeadline(wfNum);
        // processor, tbegin, tend, resIndex
        boost::tuple<int, double, double, int> savedPlan;
        double bestTimeEnd = std::numeric_limits<double>::infinity();
        bool planWasFound = false;
        
        for (auto &res : resTypes){
            // resources indexed from 1
            res -=1;
            int tbegin = expectedBegin + commTimeToType[res];
        
            if (data.Resources(res).FindPlacement(execTime[res], tbegin, processor, deadline)){
               
                
                if (tbegin + execTime[res] < bestTimeEnd){
                    savedPlan.get<0>() = processor; 
                    savedPlan.get<1>() = tbegin;
                    savedPlan.get<2>() = tbegin + execTime[res]; 
                    savedPlan.get<3>() = res;
                    bestTimeEnd = tbegin + execTime[res];
                }
                planWasFound = true;
            }
            
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
            int tend = (tbegin + time > data.GetDeadline(wfNum)) ? data.GetDeadline(wfNum) : tbegin + time;
            efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
        }
        else {
            //cout << "Can not find placement for package " << current << endl;
            finished.push_back(current);
            //int initPackageNum = data.GetInitPackageNumber(wfNum);
            // get all successors of this package
            vector <int> successors;
            //data.Workflows(wfNum).GetSuccessors(current - initPackageNum, successors);
            data.Workflows(wfNum).GetSuccessors(current, successors);
            
            for (auto & i : successors){
                // transform local to global
                // i += initPackageNum;
                // add successors to finished
                if (find(finished.begin(), finished.end(),i) == finished.end())
                    finished.push_back(i);
                if (find(unscheduled.begin(),unscheduled.end(), i) == unscheduled.end()) 
                    unscheduled.push_back(i);
            }
        }
    }
    //cout << "Efficiency value: " << efficiency << endl;
    return efficiency;
}



Greedy::~Greedy(void)
{
}
