#include "DataInfo.h"

#pragma once
class ScheduleToXML
{
	// model data
	DataInfo& data;
	string resFileName;
	string xmlBaseName;
	// current xml file count
	static int xmlCount;
	void MetaXMLInfo(ofstream &f);
	void BusyToXML(ofstream &f);
	void OneWFScheduleToXML(ofstream&f, Schedule &currentSchedule, int currentWfNum);
public:
	ScheduleToXML(DataInfo &d): data(d) {}
	// create an XML file based on current data state (especially fixed intervals)
	// add packages from currentSchedule
	void CreateXML(Schedule currentSchedule, int currentWf);
	void SetXMLBaseName(string name) { xmlBaseName = name; }
	~ScheduleToXML(void);
};

