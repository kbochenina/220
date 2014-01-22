#include "Workflow.h"
#include "ResourceType.h"
#include "ModelingContext.h"
#include "memory.h"

using namespace std;

typedef vector <TimeCore> AllTimeCore;
// for each package - last finishing time
typedef vector<double> FinishingTime; 

#pragma once
class DataInfo
{
	// friend classes
	friend class ScheduleToXML;
	// ATTRIBUTES
	// modeling time characteristics
	ModelingContext context;
	// vector of init global package number for wf
	vector<int> initPackageNumbers;
	// vector of model workflows
	vector <Workflow> workflows;
	// deadlines vector
	vector <double> deadlines;
	// vector of model resource types
	vector <ResourceType> resources;
	// vector of WFs packages finishing times
	vector<FinishingTime> finishingTimes;
	// priorities according to finishing time (vector of package numbers)
	vector <int> priorities;
	// vector of different wf priorities (local package numbers)
	vector<vector<int>> wfPriorities;
	// sum of resource types processors count
	int processorsCount;
	//OPERATIONS
	// init data placement settings
	void Init(string fName);
	// init resources
	void InitResources(string fName, bool canExecuteOnDiffResources);
	// init workflows
	void InitWorkflows(string fName);
	// init finishing times
	void InitFinishingTimes();
	
	
public:
	DataInfo(){}
	DataInfo(string fSettings);
	// get WF count
	inline int GetWFCount() {return workflows.size();}
	inline int GetResourceCount() {return resources.size(); }
	// get full cores count
	inline int GetprocessorsCount() {return processorsCount;}
	int GetResourceType (int coreNumber);
	void FixBusyIntervals();
	void ResetBusyIntervals();
	void SetInitBusyIntervals();
	void GetCurrentIntervals(vector<vector<BusyIntervals>> &storedIntervals);
	void SetCurrentIntervals(vector<vector<BusyIntervals>> &storedIntervals);
	// getter 
	const vector<Workflow> & Workflows() const { return workflows; }
	const vector<ResourceType> & Resources() const { return resources; }
	const Workflow& Workflows(int wfNum) const ;
	ResourceType& Resources(int resNum)  ;
	double GetCCR() {return context.GetCCR();}
	// get initial processor index of resource type
	int GetInitResourceTypeIndex(int type);
	// get all packages count
	int GetPackagesCount();
	// set priorities to whole packages (packages numbered from zero according to wf order)
	void SetPriorities();
	// getting next package with smallest finishing time
	int GetNextPackage();
	// getting next package with smallest finishing time
	int GetNextPackage(int wfNum, int pNum);
	// get wfNum and local package number for global package number
	void GetLocalNumbers (const int &current, int &wfNum, int &localNum);
	int GetT() {return context.GetT();}
	void SetT(double newT) {context.SetT(newT);}
	// get global processor index 
	int GetGlobalProcessorIndex(int resource, int local);
	// get resource type by global index
	int GetResourceTypeIndex(int globalIndex);
	// set different wf priorities
	void SetWfPriorities();
	// get init package number by wfNum
	int GetInitPackageNumber(int wfNum) {return initPackageNumbers[wfNum];}
	// remove some numbers from priorities
	void RemoveFromPriorities(const vector<int>& toRemove);
	bool IsWFPrioretized() {if (wfPriorities.size() == 0) return false;}
	double GetDeadline(int wfNum){return deadlines[wfNum];}
	double GetDeadline() {double maxDeadline = 0.0; for (auto& d: deadlines) if (d > maxDeadline) maxDeadline = d; return maxDeadline;}
	int GetPrioritiesSize() {return priorities.size();}
	~DataInfo(void);
};

