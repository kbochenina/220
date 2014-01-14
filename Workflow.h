#include "Package.h"

#pragma once
class Workflow
{
	// workflow number - from 1
	int uid;
	// packages of WF
	vector <Package> packages;
	// connectivity matrix
	vector<vector<int>> matrix;
public:
	const Package& operator[] (int pNum) const { return packages[pNum]; } 
	// return package count
	inline int GetPackageCount() const { return packages.size(); } 
	// return execTime of package pNum on resource with type resType and coreCount
	double GetExecTime (int pNum, int resType, int coreCount) const ;
	// return vector with possible cores count for package pNum
	void GetCoresCount(int pNum, vector<int>&out) ;
	// return vector with possible resource types for package pNum
	void GetResTypes(int pNum, vector<int>&out) ;
	// return true if package pNum is init
	bool IsPackageInit(int pNum) const;
	// return true if first depends on second
	bool IsDepends(unsigned one, unsigned two) const;
	// return matrix[i][j]
	int GetMatrixValue(int i, int j) const { return matrix[i][j]; }
	// return vector with packages that depend from pNum
	void GetOutput(int pNum, vector<int>& out) const;
	// return vector with packages from which pNum depends 
	void GetInput(int pNum, vector<int>& in) const;
	// return true if the package pNum is last
	bool IsPackageLast(int pNum) const;
	// return average exectime of package pNume
	double GetAvgExecTime(int pNum) const;
	// return computational amount of package pNum
	double GetAmount(const int & pNum) { return packages[pNum].GetAmount(); }
	// return all successors of package pNum
	void GetSuccessors(const int &pNum, vector<int>&out) const;
	Workflow(int u, vector <Package> p, vector<vector<int>> m) {uid = u; packages = p; matrix = m;}
	Workflow(){ uid = 0; }
	~Workflow(void);
};

