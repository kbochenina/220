#pragma once
#include "schedulingmethod.h"
#include "Cluster.h"
#include "Greedy.h"
class Clustered :
	public SchedulingMethod
{
	vector <Cluster> clusters;
	vector <int> clusterized;
	double GetF();
	double GetF(int second);
	double GetFMerged(int currentCluster);
	double GetFMerged(int currentCluster, int taskToAdd);
	double maxAvgDiffWeight;
	double maxSumL;
	double maxWeight;
	double minAvgTaskCount;
	void SetMaxAvgDiffWeight();
	void SetMaxSumL();
	int currentCluster;
	double currentF;
	double GetAvgDiffWeight();
	double GetAvgDiffWeight(int second);
	double GetMaximumWeight();
	double GetMaximumWeight(int second);
	void SetMaxWeight();
	double GetSumL();
	double GetSumL(int second);
	double GetSumLMerged(int currentCluster);
	double GetSumLMerged(int currentCluster, int taskToAdd);
	void Merge(int second, bool isPrev);
	double GetClusterSchedule(Schedule &out);
	void SetInitClusters();
	void ClusterizeConsequence();
	void ClusterizeMerged();
	void CheckPrecedence();
	void SetMaxSumL(int currentCluster);
	void FindCandidates(vector<int>&input, vector<int>&output, vector<int>&outputIndexes);
	double GetAvgTaskCount(int currentCluster);
	double GetAvgTaskCount(int currentCluster, int taskToAdd);
	void SetWfPackages(vector<pair<int,vector<int>>>&wfPackages, int clusterIndex);
public:
	Clustered() {}
	Clustered(DataInfo &d, int uid);
	double GetWFSchedule(Schedule &out);
	~Clustered(void);
};

