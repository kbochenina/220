// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ModelData.h"
#include <cstdlib>
#include "direct.h"
#include "Scheduler.h"
#include <string>
#include <iostream>

using namespace std;

enum SchedulingTypes { ONLY_BELLMAN = 1, ONLY_GREEDY = 2, MIXED = 3 };
enum SchedulingSchemes { STAGED = 1, DIRECT_ORDER = 2, SIMPLE = 3, RESERVED_ORDERED = 4 };

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
	// set data
	DataInfo data(s);
	
	// init model data
	ModelData md(data);
	Scheduler sched(md);
	sched.SetSchedulingStrategy(ONLY_GREEDY);	
	sched.GetSchedule(SIMPLE);
	sched.GetMetrics("simple_metrics.txt");
	sched.TestSchedule();
	/*sched.GetSchedule(STAGED);
	sched.GetMetrics("staged_metrics.txt");
	sched.TestSchedule();
	sched.GetSchedule(RESERVED_ORDERED);
	sched.GetMetrics("reserved_metrics.txt");
	sched.TestSchedule();*/
//	system("pause");
	return 0;
}

