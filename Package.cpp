#include "StdAfx.h"
#include "Package.h"
#include "UserException.h"
#include "string"
#include <iostream>


double Package::GetExecTime(int type, int cores) const {
	try{
		std::pair<int,int> typeCore = make_pair(type,cores);
		auto it = execTimes.find(typeCore);
		if (it==execTimes.end()) 
			throw UserException("Package::GetExecTime() : combination of type " + to_string(type) + 
			"  and cores " + to_string(cores) + " not found");
		return it->second;
	}
	catch (UserException& e){
		cout<<"error : " << e.what() <<endl;
		std::system("pause");
		exit(EXIT_FAILURE);
	}
}

double Package::GetAvgExecTime() const{
	double sum = 0.0;
	int counter = 0;
	for (auto it = execTimes.begin(); it!= execTimes.end(); it++){
		// for 1 core
		if (it->first.second == 1){
			sum += it->second;
			counter ++;
		}
	}
	return sum/counter;
}

// copy constructor
Package::Package (const Package & p){
	uid = p.uid;
	resTypes.reserve(p.resTypes.size());
	coreCounts.reserve(p.resTypes.size());
	copy(p.resTypes.begin(), p.resTypes.end(), back_inserter(resTypes));
	copy(p.coreCounts.begin(), p.coreCounts.end(), back_inserter(coreCounts));
	execTimes.insert(p.execTimes.begin(), p.execTimes.end());
	amount = p.amount;

}

Package::~Package(void)
{
}
