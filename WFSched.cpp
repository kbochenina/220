// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataInfo.h"
#include "Scheduler.h"
#include <cstdlib>
#include <stdlib.h>
#include "direct.h"
#include "windows.h"


using namespace std;

enum SchedulingTypes { ONLY_BELLMAN = 1, ONLY_GREEDY = 2, CLUST = 3 };
enum SchedulingSchemes { STAGED = 1, EFF_ORDERED = 2, SIMPLE = 3, RESERVED_ORDERED = 4, CLUSTERED = 5};

int _tmain(int argc, wchar_t**argv)
{
    ofstream log("WFSchedLog.txt");

    // fileSettings is a file with program settings
    // it is a first command line argument
    // if program is started without arguments, filename is "settings.txt"
    double deadline = 350;
    int periodsCount = 1, experCount = 1;
    wstring fileSettings;
    if (argc != 1 ) {
	     deadline = _wtof(argv[1]);
        //periodsCount = _wtoi(argv[2]);
        //experCount = _wtoi(argv[3]);
	     //cout << minLInit << endl;
    }

    // creating/check output directory
    DWORD dAttr = GetFileAttributes(L"Output");
    if ((dAttr & FILE_ATTRIBUTE_DIRECTORY) && dAttr != 0xffffffff){
        if (chdir("Output")){
            log << "Output directory cannot be used" << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(1);
        }
    }
    else {
        if (_mkdir("Output")){
            log << "Error while creating output directory" << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(1);
        }
        _chdir("Output");
    }

    // creation of files with time and schedule quality metrics in output directory
    string timeFileName = "time.txt";
    ofstream timeFile(timeFileName);
    if (timeFile.fail()){
        log << "Error while creating time metrics file" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }
    timeFile.close();

    string metricsFileName = "fullmetrics.txt";
    ofstream metricsFile(metricsFileName, ios::trunc);
    if (metricsFile.fail()){
        log << "Error while creating schedule metrics file" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }
    metricsFile.close();

    if (_chdir("..")){
        log << "Cannot change directory to working directory" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }
    
   
    fileSettings=L"settings.txt";
    string settings(fileSettings.begin(),fileSettings.end());
	
    ifstream settFile(fileSettings);
    if (settFile.fail()){
        log << "File " << settings << " was not open" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }

    string s;
    // reading name of scheduling method
    while(1) {
        getline(settFile,s);
        if (s.find("SchedMethod") != string::npos)
            break;
        if (settFile.eof()) {
            log << "Description of scheduling method was not found" << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(1);
        }
    } 

    size_t pos = s.find("=");
    if (pos == string::npos){
        log << "Description of scheduling method has wrong format" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }
    string parName = "SchedMethod=";
    string schedName = s.substr(pos+1, s.size()-parName.size());

    // initializing data and scheduler
    DataInfo data(settings,deadline);
	 Scheduler sched(data);
    sched.SetSchedulingStrategy(ONLY_GREEDY);	

    if (schedName == "SIMPLE"){
        sched.GetSchedule(SIMPLE);
        sched.GetMetrics("simple_metrics.txt", "SimpleSched", metricsFileName);
        sched.TestSchedule();
    }
    else if (schedName == "STAGED"){
        sched.GetSchedule(RESERVED_ORDERED);
        sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime",metricsFileName);
        sched.TestSchedule();
    }
    else if (schedName == "CLUSTERED"){
        sched.SetSchedulingStrategy(CLUST);
        sched.GetSchedule(CLUSTERED);
        sched.GetMetrics("clustered.txt", "Clustered", metricsFileName);
        sched.TestSchedule();
    }
    else if (schedName == "ALL"){
        sched.GetSchedule(SIMPLE);
        sched.GetMetrics("simple_metrics.txt", "SimpleSched", metricsFileName);
        sched.TestSchedule();
        sched.GetSchedule(RESERVED_ORDERED);
        sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime",metricsFileName);
        sched.TestSchedule();
        sched.SetSchedulingStrategy(CLUST);
        sched.GetSchedule(CLUSTERED);
        sched.GetMetrics("clustered.txt", "Clustered", metricsFileName);
        sched.TestSchedule();
    }
    else {
        log << "Description of scheduling method has wrong format" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(1);
    }

    #ifdef _DEBUG 
        system("pause");
    #endif

    log.close();

    return 0;
}

