// CommCreator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <map>
#include <vector>
#include <sstream>

using namespace std;
using namespace boost::filesystem;

// inputs: file log.txt
// folders Temp_1, Temp_2, ... with output information

// get second count from hh:mm:ss format
double ParseData(string s){
    int hours, minutes;
    double seconds;
    istringstream iss(s);
    iss >> hours;
    if (iss.fail()){
        cout << "Error while reading matrix " << endl;
        exit(1);
    }
    char c;
    iss >> c;
    iss >> minutes;
    if (iss.fail()){
        cout << "Error while reading matrix " << endl;
        exit(1);
    }
    iss >> c;
    iss >> seconds;
    if (iss.fail()){
        cout << "Error while reading matrix " << endl;
        exit(1);
    }
    double res = hours * 3600 + minutes * 60 + seconds;
    return res;
}

// get estimations from directory
void GetEstimations(map<int,double>&estimations, map<int,int>&taskId, vector<vector<int>>& matrix){
    path basePath = current_path();
    ofstream add("addInfo.dat");
    current_path(basePath);
    // taskID, (inCommTime, outCommTime)
    //map <int, pair<double, double>> inOut;
    // taskID, (scriptBegin, taskBegin, scriptEnd, taskEnd)
    map <int, tuple<double,double,double,double>> times;
    directory_iterator dirIt(current_path()), dirEnd;
    while (dirIt != dirEnd){
        path current = *dirIt++;
        if (!is_directory(current))
            continue;
        current_path(current);
        string dirName = current.string();
        size_t pos = dirName.find_last_of("\\");
        dirName = dirName.substr(pos + 1, dirName.size() - pos - 1);
        int clavireId = atoi(dirName.c_str());
        if (clavireId == 0){
            cout << "Error while getting clavireID value " << endl;
            exit(1);
        }
        double scriptBegin = 0.0, scriptEnd = 0.0, taskBegin = 0.0, taskEnd = 0.0;
        string s;
        ifstream file("clavire_script_started");
        if (file.fail()){
             cout << "Error while opening clavire_script_started " << endl;
             exit(1);
        }
        getline(file,s);
        scriptBegin = ParseData(s);
        file.close();
        file.open("clavire_script_finished");
        if (file.fail()){
             cout << "Error while opening clavire_script_finished " << endl;
             exit(1);
        }
        getline(file,s);
        scriptEnd = ParseData(s);
        file.close();
        file.open("clavire_task_started");
        if (file.fail()){
             cout << "Error while opening clavire_task_started " << endl;
             exit(1);
        }
        getline(file,s);
        taskBegin = ParseData(s);
        file.close();
        file.open("clavire_task_finished");
        if (file.fail()){
             cout << "Error while opening clavire_task_finished " << endl;
             exit(1);
        }
        getline(file,s);
        taskEnd = ParseData(s);
        file.close();
        
        int task = taskId[clavireId];
        times[task] = make_tuple(scriptBegin, taskBegin, taskEnd, scriptEnd);
       /* double inCommTime = taskBegin - scriptBegin,
            outCommTime = scriptEnd - taskEnd;
        inOut[task] = make_pair(inCommTime, outCommTime);*/

        current_path(basePath);
    }

    // find minimum starting time
    double startTime = std::numeric_limits<double>::infinity();
    for (auto &task : times){
        if (get<0>(task.second) < startTime)
            startTime = get<0>(task.second);
    }

    for (auto &task : times){
        add << get<0>(task) << " "  << get<0>(task.second) - startTime << " " << get<2>(task.second) - startTime << " " 
            << get<2>(task.second) - startTime << " " << get<3>(task.second) - startTime << endl;
    }

    for (auto &task : times){
        double est = 0.0;
        int taskId = task.first;
        double maxParentsFin = 0.0;
        bool isInitTask = true;
        // find finishing times for all parents of current task
        for (int i = 0; i < matrix.size(); i++){
            // if task has parents
            if (matrix[i][taskId - 1] != 0){
                double fin = get<2>(times[i+1]);
                if (fin > maxParentsFin)
                    maxParentsFin = fin;
                isInitTask = false;
            }
        }
        // time before start of initial task is time between start of the script and start of task execution
        if (isInitTask)
            est = get<0>(task.second) - startTime;
        // otherwise it is the length of period between finish of all parent tasks and start of current task
        else 
            est = get<1>(task.second) - maxParentsFin;
        estimations[taskId] = est;
    }
    current_path(basePath);
    add << endl;
    add.close();
}

void ReadMatrix(vector<vector<int>>& matrix){
    ifstream data("data.mcomm");
    if (data.fail()){
        cout << "Error while initial opening data.mcomm " << endl;
        exit(1);
    }
    int rowCount = 0;
    string s;
    while (!data.eof()){
        getline(data,s);
        rowCount++;
    }
    rowCount--;
    data.close();
    data.open("data.mcomm");
    if (data.fail()){
        cout << "Error while opening data.mcomm " << endl;
        exit(1);
    }
    matrix.resize(rowCount);
    for (int i = 0; i < rowCount; i++){
        matrix[i].resize(rowCount);
        getline(data,s);
        istringstream iss(s);
        for (int j = 0; j < rowCount; j++){
            iss >> matrix[i][j];
            if (iss.fail()){
                cout << "Error while reading matrix " << endl;
                exit(1);
            }
        }
    }
    data.close();
}

void InitComm(){
     // reading the communication matrix
    vector<vector<int>> matrix;
    ReadMatrix(matrix);
    
    ifstream log("log.txt");
    if (log.fail()){
         cout << "Error while opening log file " << endl;
         exit(1);
    }
    // (CLAVIRE task id, workflow task id)
    map<int,int> taskId;
    string s;
    do {
        getline(log,s);
    } while (s.find("Workflows information")==string::npos);
    while (1){
        string toFind = "Task ID: ";
        int pos = 0;
        int clavireId = 0, workflowId = 0;
        bool isEnd = false;
        do {
            getline(log,s);
            pos = s.find(toFind);
            if (s.find("End of workflows information") != string::npos){
                isEnd = true;
                break;
            }
        } while (pos==string::npos);
       
        if (isEnd)
            break;
        s.erase(0, pos + toFind.length());
        istringstream iss(s);
        iss >> clavireId;
        if (iss.fail()){
             cout << "Error while reading CLAVIRE id " << endl;
             exit(1);
        }
        getline(log,s);
        toFind = "WF task id: ";
        s.erase(0, pos + toFind.length());
        iss.clear();
        iss.str(s);
        iss >> workflowId;
        if (iss.fail()){
             cout << "Error while reading workflow id " << endl;
             exit(1);
        }
        taskId[clavireId] = workflowId;
    }
    log.close();

    cout << "Log has been successfully read " << endl;

    vector <map<int,double>> estimations;

    directory_iterator dirIt(current_path()), dirEnd;
    path basePath = current_path();
    while (dirIt != dirEnd){
        path current = *dirIt++;
        if (is_directory(current)){
            string pathStr = current.string();
            if (pathStr.find("Temp_") != string::npos){
                current_path(current);
                map <int,double> est;
                GetEstimations(est, taskId, matrix);
                estimations.push_back(est);
            }
        }
        current_path(basePath);
    }

    cout << "Estimations have been successfully taken " << endl;

    map<int,double> resultEstimations;
    ofstream resFile("commTime.dat");
    if (estimations.size() == 0){
        cout << "Estimations vector cannot have zero size " << endl;
        exit(1);
    }
    for (auto &estVector: estimations){
        for (auto& est: estVector){
            //resFile << est.first << " " << est.second << endl;
            resultEstimations[est.first] += est.second;
        }
        //resFile << endl;
    }
       
    int jobCount = estimations.size();

    for (auto& resEst: resultEstimations){
        resEst.second /= jobCount;
        resFile << resEst.first << " " << resEst.second << endl;

    }
     resFile.close();
}

void GetEstimations(map <int,double>& est){
    ifstream comm("commTime.dat");
    if (comm.fail()){
        cout << "Failed to open commtime.dat" << endl;
        exit(1);
    }
    string s;
    while (getline(comm,s)){
        int task;
        double e;
        istringstream iss(s);
        iss >> task;
        if (iss.fail()){
            cout << "Failed to get number of tasks" << endl;
            exit(1);
        }
        iss >> e;
        if (iss.fail()){
            cout << "Failed to get extimation" << endl;
            exit(1);
        }
        est[task] += e;
    }
}

void AggComm(){
    int estCount = 0;
    map <int,double> est;

    directory_iterator dirIt(current_path()), dirEnd;
    path basePath = current_path();
    while (dirIt != dirEnd){
        path current = *dirIt++;
        if (is_directory(current)){
            string pathStr = current.string();
            if (pathStr.find("Case") != string::npos){
                current_path(current);
                GetEstimations(est);
                estCount++;
            }
        }
        current_path(basePath);
    }
    ofstream out("aggComm.dat");
    for (auto& estim:est){
        estim.second /= estCount;
        out << estim.first << " " << estim.second << endl;
    }
    out.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc == 1){
        InitComm();
    }
    else {
        AggComm();
    }
   
    return 0;
}

