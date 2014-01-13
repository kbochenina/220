#include "StdAfx.h"
#include "Workflow.h"
#include "UserException.h"
#include <string>
#include <iostream>

double Workflow::GetExecTime ( int pNum, int type, int cores) const {
	try{
		if (pNum < 0 || pNum > packages.size()-1) 
			throw UserException("Workflow::GetExecTime() error. Wrong packageNum" + to_string(pNum));
		return packages[pNum].GetExecTime(type, cores);
	}
	catch (UserException& e){
		std::cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

// return true if package pNum is init
bool Workflow::IsPackageInit(int pNum) const {
	for (const auto &i : matrix)
		if (i[pNum]!=0) return false;
	return true;
}

// return true if first depends on second
bool Workflow::IsDepends(unsigned one, unsigned two) const {
	try {
		string errorMsg = "Workflow::IsDepends() error. Workflow " + to_string(uid) + ", incorrect package num - ";
		if (one > packages.size()-1) throw errorMsg +  to_string(one);
		if (two > packages.size()-1) throw errorMsg +  to_string(two);
		if (matrix[one][two]==1) return true;
		// if dependency is indirect
		else {
			for (unsigned int i = 0; i < matrix[one].size(); i++){
				if (matrix[one][i]!=0) 
					if (IsDepends(i,two)) return true;
			}
			return false;
		}
	}
	catch(const string msg){
		cout << msg << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
}

Workflow::~Workflow(void)
{
}

// return vector with packages that depend from pNum
void Workflow::GetOutput(int pNum, vector<int>& out) const{
	for (int i = 0; i < matrix.size(); i++){
		if (matrix[pNum][i] == 1)
			out.push_back(i);
	}
}
// return vector with packages from which pNum depends 
void Workflow::GetInput(int pNum, vector<int>& in) const{
	for (int i = 0; i < matrix.size(); i++){
		if (matrix[i][pNum]==1)
			in.push_back(i);
	}
}

// return true if the package pNum is last
bool Workflow::IsPackageLast(int pNum) const {
	bool isLast = true;
	for (int i = 0; i < matrix.size(); i++){
		if (matrix[pNum][i]==1){
			isLast = false;
			return isLast;
		}
	}
	return isLast;
}

// return average exectime of package pNume
double Workflow::GetAvgExecTime(int pNum) const{
	return packages[pNum].GetAvgExecTime();
}
