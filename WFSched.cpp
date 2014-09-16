// WFSched.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataInfo.h"
#include "Scheduler.h"
#include <cstdlib>
#include <stdlib.h>
#include "direct.h"
#include "windows.h"
#include "errno.h"


using namespace std;

enum SchedulingTypes { ONLY_BELLMAN = 1, ONLY_GREEDY = 2, CLUST = 3 };
enum SchedulingSchemes { STAGED = 1, EFF_ORDERED = 2, SIMPLE = 3, RESERVED_ORDERED = 4, CLUSTERED = 5};

int _tmain(int argc, wchar_t**argv)
{
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

    wstring wpath(argv[0]);
    string path(wpath.begin(), wpath.end());
    size_t pathpos = path.find_last_of("\\");
    path.erase(pathpos + 1, path.size() - pathpos);
    path += "Output";

    wstring outputPath(path.begin(), path.end());

    // creating/check output directory
    DWORD dAttr = GetFileAttributes(outputPath.c_str());
    if ((dAttr & FILE_ATTRIBUTE_DIRECTORY) && dAttr != 0xffffffff){
        if (chdir(path.c_str())){
            cout << "Output directory cannot be used" << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(2);
        }
    }
    else {
        if (_mkdir(path.c_str())){
            cout << "Error while creating output directory, ";
            cout << std::strerror(errno) << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(3);
        }
        _chdir(path.c_str());
    }

   
    // creation of files with time and schedule quality metrics in output directory
    string timeFileName = "time.txt";
    ofstream timeFile(timeFileName);
    if (timeFile.fail()){
        cout << "Error while creating time metrics file" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(4);
    }
    timeFile.close();

    string metricsFileName = "fullmetrics.txt";
    ofstream metricsFile(metricsFileName, ios::trunc);
    if (metricsFile.fail()){
        cout << "Error while creating schedule metrics file" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(5);
    }
    metricsFile.close();

    if (_chdir("..")){
        cout << "Cannot change directory to working directory" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(6);
    }
    
   
    fileSettings=L"settings.txt";
    string settings(fileSettings.begin(),fileSettings.end());
	
    ifstream settFile(fileSettings);
    if (settFile.fail()){
        cout << "File " << settings << " was not open" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(7);
    }

    string s;
    // reading name of scheduling method
    while(1) {
        getline(settFile,s);
       
        if (s.find("SchedMethod") != string::npos)
            break;
        if (settFile.eof()) {
            cout << "Description of scheduling method was not found" << endl;
            #ifdef _DEBUG 
                system("pause");
            #endif
            exit(8);
        }
    } 

    size_t pos = s.find("=");
    if (pos == string::npos){
        cout << "Description of scheduling method has wrong format" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(9);
    }
    string parName = "SchedMethod=";
    string schedName = s.substr(pos+1, s.size()-parName.size());
    deadline = 350;
    cout << "Deadline = " << deadline << endl;

    // initializing data and scheduler
    DataInfo data(settings,deadline);
	 Scheduler sched(data);
    sched.SetSchedulingStrategy(ONLY_GREEDY);	

    if (schedName == "SIMPLE"){
        sched.GetSchedule(SIMPLE);
        sched.GetMetrics("simple_metrics.txt", "SimpleSched", metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
    }
    else if (schedName == "STAGED"){
        sched.GetSchedule(RESERVED_ORDERED);
        sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime", metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
    }
    else if (schedName == "CLUSTERED"){
        sched.SetSchedulingStrategy(CLUST);
        sched.GetSchedule(CLUSTERED);
        sched.GetMetrics("clustered.txt", "Clustered", metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
    }
    else if (schedName == "ALL"){
        sched.GetSchedule(SIMPLE);
        sched.GetMetrics("simple_metrics.txt", "SimpleSched", metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
        sched.GetSchedule(RESERVED_ORDERED);
        sched.GetMetrics("reserved_metrics.txt", "StagedReservedTime",metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
        sched.SetSchedulingStrategy(CLUST);
        sched.GetSchedule(CLUSTERED);
        sched.GetMetrics("clustered.txt", "Clustered", metricsFileName);
        data.SetInitBusyIntervals();
        sched.TestSchedule();
    }
    else {
        cout << "Description of scheduling method has wrong format" << endl;
        #ifdef _DEBUG 
            system("pause");
        #endif
        exit(10);
    }

    #ifdef _DEBUG 
        system("pause");
    #endif

   
    return 0;
}

