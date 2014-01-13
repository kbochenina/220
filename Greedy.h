#pragma once
#include "SchedulingMethod.h"
class Greedy :
	public SchedulingMethod
{
public:
	Greedy(DataInfo &d,int u, int w);
	void printInfo();
	double GetWFSchedule(Schedule &out);
	double GetFullSchedule(Schedule& out);
	double GetOneWFSchedule(Schedule& out);
	~Greedy(void);
};

