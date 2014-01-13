#include "StdAfx.h"
#include "Efficiency.h"

// return value of efficiency by period
double Efficiency::EfficiencyByPeriod(int busyCores, int t1, int t2){
	return ( busyCores * (double)(t2-t1)/(double)T * (EfficiencyFunction((double)t1/(double)T) + EfficiencyFunction((double)t2/(double)T)) / 2  );
}

double Efficiency::EfficiencyFunction(double x) { return (koeff*(1-x)); }
	

Efficiency::~Efficiency(void)
{
}
