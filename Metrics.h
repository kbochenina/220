#include "ModelData.h"
#include "ScheduleToXML.h"
#include <iostream>
#include <fstream>
#pragma once
class Metrics
{
	// reference on dataInfo
	DataInfo &data;
	Schedule sched;
	ofstream out;
	ofstream full;
	string filename;
	vector<double> reservedTime;
	void AvgUnfinischedTime();
	void AvgReservedTime();
public:
	Metrics(DataInfo& md, string filename) : data(md){  out.open(filename); reservedTime.resize(data.GetWFCount());
														for (int i = 0; i < reservedTime.size(); i++) reservedTime[i] = numeric_limits<double>::infinity(); }
	void GetMetrics(Schedule & sched, string schemeName);
	~Metrics(void);
};

