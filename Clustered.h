#pragma once
#include "schedulingmethod.h"
#include "Cluster.h"
#include "Greedy.h"
class Clustered :
	public SchedulingMethod
{
	// clusters for each WF
	vector <vector<Cluster>> clusters;
	vector <int> clusterized;
	int currentCluster;
   double currentADW;
	vector<double> maxAvgDiffWeight;
	vector<double> maxSumL;
	// cluster dependencies
	vector<vector<vector<int>>> dep;
	//double maxWeight;
	//double minAvgTaskCount;
	//double currentF;
	// clusterize current workflow
	

	
	void SetInitClusters();
	void SetMaxAvgDiffWeight();
	void SetMaxSumL();
	void ClusterizeConsequence();

	double GetF(double&adw);
	double GetF(int second, double&adw);
	double GetAvgDiffWeight();
	double GetAvgDiffWeight(int second);
   double GetAvgDiffWeight(int second, double adw);
	double GetSumL();
	double GetSumL(int second);

	void Merge(int second, bool isPrev);
	void SetClusterDep();

	double GetClusterSchedule(Schedule &out, int&realBegin, int&realEnd);

	//double GetMaximumWeight();
	//double GetMaximumWeight(int second);
	//void SetMaxWeight();
	double GetFMerged(int currentCluster);
	double GetFMerged(int currentCluster, int taskToAdd);
	double GetSumLMerged(int currentCluster);
	double GetSumLMerged(int currentCluster, int taskToAdd);
	void CheckPrecedence();
	void FindCandidates(vector<int>&input, vector<int>&output, vector<int>&outputIndexes);
	double GetAvgTaskCount(int currentCluster);
	double GetAvgTaskCount(int currentCluster, int taskToAdd);
	void SetWfPackages(vector<pair<int,vector<int>>>&wfPackages, int clusterIndex);
   void ModifyStartDeadline(int wfNum, int cluster, double realEnd);
public:
	Clustered() {}
	Clustered(DataInfo &d, int uid);
	double GetWFSchedule(Schedule &out);
	~Clustered(void);
};

