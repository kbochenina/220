#include "stdafx.h"
#include "Greedy.h"

using namespace std;


Greedy::Greedy(DataInfo &d,int u, int w) : SchedulingMethod(d,u,w){ 
    finished.clear();
    unscheduled.clear();
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
    data.SetPriorities();
    while (finished.size() != data.GetPackagesCount()){
        // get unscheduled package with minimum finishing count
        int current = data.GetNextPackage();
        FindSchedule(out, efficiency, current, false);
 
    }
    return efficiency;
}

double Greedy::GetOneWFSchedule(Schedule& out){
    
    double efficiency = 0.0;
    int index = data.Workflows(wfNum).GetPackageCount() - 1;
    // while all packages are not scheduled
    while (finished.size() != data.Workflows(wfNum).GetPackageCount()){
        // get unscheduled package with minimum finishing count
        int current = data.Workflows(wfNum).GetNextPackage(index--);
        if (find(unscheduled.begin(), unscheduled.end(), current) != unscheduled.end())
            continue;
        FindSchedule(out, efficiency, current, true);
    }
    return efficiency;
}

void Greedy::FindSchedule(Schedule& out, double &efficiency, int pNum, bool forOneWf){
    vector<int> dependsOn;    
    int localNum = 0, expectedBegin = 0, processor = 0;
    double deadline = 0.0;
    if (!forOneWf){
        deadline = data.GetDeadline();
         // get wfNum and package local num
        data.GetLocalNumbers(pNum, wfNum, localNum);
         // should get latest finishing time of previous packages
        data.Workflows(wfNum).GetInput(localNum, dependsOn);
        //deadline = data.Workflows(wfNum).GetDeadline();
    }
    else {
        data.Workflows(wfNum).GetInput(pNum, dependsOn);
        deadline = data.Workflows(wfNum).GetDeadline();
        localNum = pNum;
    }
    for (size_t i = 0; i < dependsOn.size(); i++){
        // set global numbers
        for (auto& j : out){
            int add = 0;
            if (!forOneWf) add += pNum - localNum;
            if (boost::get<0>(j) == dependsOn[i] + add)
            {
                int resType = data.GetResourceTypeIndex(boost::get<2>(j)[0]);
                double tEnd = boost::get<1>(j) + boost::get<3>(j);
                if (tEnd > expectedBegin)
                    expectedBegin = static_cast<int>(tEnd) + 1;

            }
        }
    }
   
    // getting possible types of resources
    vector <int> resTypes = data.Workflows(wfNum)[localNum].GetResTypes();
    vector <double> execTime;
    for (size_t i = 0; i < resTypes.size(); i++){
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
            int tbegin = expectedBegin;
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
        int tbegin = static_cast<int>(savedPlan.get<1>());
        double execTime = savedPlan.get<2>()-tbegin;
        data.Resources(savedPlan.get<3>()).AddInterval(execTime, 
            tbegin, savedPlan.get<0>());
        finished.push_back(pNum);
        int procGlobalIndex = data.GetGlobalProcessorIndex(savedPlan.get<3>(), savedPlan.get<0>());
        vector <int> processors;
        processors.push_back(procGlobalIndex);
        out.push_back(make_tuple(pNum, tbegin, processors, execTime));
        // add efficiency value
        int tend = static_cast<int>((tbegin + execTime > deadline) ? deadline : tbegin + execTime);
        efficiency += eff->EfficiencyByPeriod(1, tbegin, tend);
        }
    else {
        //cout << "Can not find placement for package " << current << endl;
        finished.push_back(pNum);
        int initPackageNum = data.GetInitPackageNumber(wfNum);
        // get all successors of this package
        vector <int> successors;
        if (!forOneWf) 
            data.Workflows(wfNum).GetSuccessors(pNum - initPackageNum, successors);
        else data.Workflows(wfNum).GetSuccessors(pNum, successors);
            
        for (auto & i : successors){
            // transform local to global
            if (!forOneWf) i += initPackageNum;
            if (find(finished.begin(), finished.end(),i) == finished.end())
                finished.push_back(i);
            if (forOneWf){
                 if (find(unscheduled.begin(),unscheduled.end(), i) == unscheduled.end()) 
                    unscheduled.push_back(i);

            }
                               
        }
        // remove successor from priority queue
        if (!forOneWf) data.RemoveFromPriorities(successors);
    }

}

Greedy::~Greedy(void)
{
}
