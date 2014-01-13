#include "stdafx.h"
#include "ReservedTimeCriteria.h"


ReservedTimeCriteria::ReservedTimeCriteria(DataInfo &d) : CriteriaMethod(d)
{
}

double ReservedTimeCriteria::GetCriteria(const Schedule &in){
	// replace in case of different deadlines
	double deadline = data.GetT();
	// max ending time
	double maxEndTime = 0;
	for (auto &sched: in){
		double currentEndTime = sched.get<1>() + sched.get<3>();
		if (maxEndTime < currentEndTime)
			maxEndTime = currentEndTime;
	}
	return (deadline - maxEndTime);
}


ReservedTimeCriteria::~ReservedTimeCriteria(void)
{
}
