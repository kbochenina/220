#include "StdAfx.h"
#include "ScheduleToXML.h"
#include <memory>

int ScheduleToXML::xmlCount = 0;

void ScheduleToXML::CreateXML(Schedule currentSchedule, int currentWf){
	//std::cout << "ScheduleToXML::CreateXML() was called\n";
	resFileName = xmlBaseName + to_string(++xmlCount) + ".jed";
	ofstream f(resFileName);
	MetaXMLInfo(f);
	f << "\t<node_infos>\n";
	BusyToXML(f);
	// if we want to get xml with init busy intervals, currentWf will be equal to -1
	//if (currentWf != -1) 
		OneWFScheduleToXML(f, currentSchedule, currentWf);
	f << "\t</node_infos>\n";
	f << "</grid_schedule>\n";
	f.close();
}

void ScheduleToXML::MetaXMLInfo(ofstream &f){
	f << "<grid_schedule>\n";
	f << "\t<meta_info>\n";
	f << "\t\t<meta name=\"alloc\" value=\"mcpa\"/>\n";
	f << "\t\t<meta name=\"pack\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"bf\" value=\"0\"/>\n";
	f << "\t\t<meta name=\"ialloc\" value=\"0\"/>\n";
	f << "\t</meta_info>\n";
	f << "\t<grid_info>\n";
	f << "\t\t<info name=\"nb_clusters\" value=\"1\"/>\n";
	f << "\t\t<clusters>\n";
	f << "\t\t  <cluster id=\"0\" hosts=\"" << data.GetprocessorsCount() << "\" first_host=\"0\"/>\n";
	f << "\t\t</clusters>\n";
	f << "\t</grid_info>\n";
}

// busy intervals are produced from Intervals.fixed
void ScheduleToXML::BusyToXML(ofstream &f){
	int inc = 0;
	// for each resource type
	for (unsigned i = 0; i < data.resources.size(); i++){
		Intervals windows;
		// get intervals of this resource type
		data.resources[i].GetCurrentWindows(windows);
		// get fixed intervals 
		vector <BusyIntervals> fixed = windows.GetFixedIntervals();
		// for each resource
		for (unsigned j = 0; j < fixed.size(); j++){
			// get pointer to resource's intervals
			BusyIntervals::iterator bIt = fixed[j].begin();
			// loop on different cores
			for (;bIt != fixed[j].end(); bIt++){
				int coreNum = bIt->first-1;
				coreNum += inc;
				// loop on different intervals
				for (vector<pair<int,int>>::size_type k = 0; k < bIt->second.size(); k++){
					int tBegin = bIt->second[k].first;
					int tEnd = bIt->second[k].second;
					f << "\t\t<node_statistics>" << endl;
					f << "\t\t	<node_property name=\"id\" value=\""<< coreNum <<"\"/>" << endl;
					f << "\t\t	<node_property name=\"type\" value=\"busy\"/>" << endl;
					f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
					f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
					f << "\t\t	<configuration>" << endl;
					f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
					f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
					f << "\t\t	  <host_lists>" << endl;
					f << "\t\t	    <hosts start=\"" << coreNum << "\" nb=\"1\"/>" << endl;
					f << "\t\t	  </host_lists>" << endl;
					f << "\t\t	</configuration>" << endl;
					f << "\t\t</node_statistics>" << endl;
				}
			}
			inc += fixed[j].size();
			
		}
	}
}

void ScheduleToXML::OneWFScheduleToXML(ofstream&f, Schedule &currentSchedule, int currentWfNum){
	int currentWfPackage = 0;
	for (Schedule::size_type i = 0; i < currentSchedule.size(); i++){
		int packageNum = currentSchedule[i].get<0>();
		int tBegin = currentSchedule[i].get<1>();
		int coresCount = currentSchedule[i].get<2>().size();
		vector <int> cores = currentSchedule[i].get<2>();
		int type = data.GetResourceType(cores[0]);
		int currBeginIndex = 0;
		// correct
		int localNum = 0;
		data.GetLocalNumbers(packageNum, currentWfNum, localNum);
		//cout << "type=" << type << " cores = " << coresCount << " package = " << packageNum << " ";
		double execTime = currentSchedule[i].get<3>();
		//cout << execTime << endl;
		int tEnd = tBegin + static_cast<int>(execTime);
		for (unsigned j = 0; j < cores.size(); j++){
			f << "\t\t<node_statistics>" << endl;
			f << "\t\t	<node_property name=\"id\" value=\"" << packageNum+1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"type\" value=\"wf"<< currentWfNum % 10 + 1 <<"\"/>" << endl;
			f << "\t\t	<node_property name=\"start_time\" value=\"" << tBegin << "\"/>" << endl;
			f << "\t\t	<node_property name=\"end_time\" value=\"" << tEnd << "\"/>" << endl;
			f << "\t\t	<configuration>" << endl;
			f << "\t\t	  <conf_property name=\"cluster_id\" value=\"0\"/>" << endl;
			f << "\t\t	  <conf_property name=\"host_nb\" value=\"1\"/>" << endl;
			f << "\t\t	  <host_lists>" << endl;
			f << "\t\t	    <hosts start=\"" << cores[j] << "\" nb=\"1\"/>" << endl;
			f << "\t\t	  </host_lists>" << endl;
			f << "\t\t	</configuration>" << endl;
			f << "\t\t</node_statistics>" << endl;
		}
		currentWfPackage++;
	}
}

ScheduleToXML::~ScheduleToXML(void)
{
}
