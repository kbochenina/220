#pragma once
#include "criteriamethod.h"
#include "DataInfo.h"
class ReservedTimeCriteria :
	public CriteriaMethod
{
public:
	ReservedTimeCriteria(DataInfo &d);
	double GetCriteria(const Schedule &in);
	bool TendsToMin() {return false;}
	// return true if first value is better than second
	bool IsBetter(double first, double second){ return (first > second ? true : false); }
	~ReservedTimeCriteria(void);
};

