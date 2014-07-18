// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataInfo.h"
#include "Scheduler.h"
#include <cstdlib>
#include "direct.h"


using namespace std;

enum SchedulingTypes { ONLY_BELLMAN = 1, ONLY_GREEDY = 2, CLUST = 3 };
enum SchedulingSchemes { STAGED = 1, EFF_ORDERED = 2, SIMPLE = 3, RESERVED_ORDERED = 4, CLUSTERED = 5};

int _tmain(int argc, wchar_t** argv)
{
	// fileSettings is a file with program settings
	// it is a first command line argument
	// if program is started without arguments, filename is "settings.txt"
	wstring fileSettings;
	if (argc == 1) {
		fileSettings=L"settings.txt";
	}
	else {
		fileSettings = argv[1];
	}
	string s(fileSettings.begin(),fileSettings.end());
	cout << "File settings name: " << s << endl;
	srand(time(NULL));
	// set data
	DataInfo data(s);
	Scheduler sched(data);
	ofstream f("fullmetrics.txt", ios::trunc);
	f.close();
	sched.SetSchedulingStrategy(ONLY_GREEDY);	
	sched.GetSchedule(SIMPLE);
	sched.GetMetrics("simple_metrics.txt", "SimpleSched");
	sched.TestSchedule();
	cout << "***************************************************" << endl;
	sched.GetSchedule(RESERVED_ORDERED);
	sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime");
	sched.TestSchedule();
	cout << "***************************************************" << endl;
	sched.GetSchedule(EFF_ORDERED);
	sched.GetMetrics("eff_metrics.txt", "StagedEfficiency");
	sched.TestSchedule();
	cout << "***************************************************" << endl;
	sched.SetSchedulingStrategy(CLUST);
	sched.GetSchedule(CLUSTERED);
	sched.GetMetrics("clustered.txt", "Clustered");
	sched.TestSchedule();
	cout << "***************************************************" << endl;
	/*sched.GetSchedule(STAGED);
	sched.GetMetrics("staged_metrics.txt");
	sched.TestSchedule();*/
	//system("pause");
	return 0;
}

