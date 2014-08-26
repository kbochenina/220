// CommCreator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
#include <vector>
#include <sstream>
#include "Winsock2.h"

using namespace std;
using namespace boost::filesystem;
using namespace boost::tuples;
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

void ReadData(map<int,pair<int,int>>&taskId, map<int,int>& taskWF,  map <int, tuple<double,double,double,double>>& times){
    path basePath = current_path();
    current_path(basePath);
    // taskID, (inCommTime, outCommTime)
    //map <int, pair<double, double>> inOut;
    // taskID, (scriptBegin, taskBegin, scriptEnd, taskEnd)
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
        
        int task = taskId[clavireId].second;
        taskWF[task] = taskId[clavireId].first;
        times[task] = make_tuple(scriptBegin, taskBegin, taskEnd, scriptEnd);
       /* double inCommTime = taskBegin - scriptBegin,
            outCommTime = scriptEnd - taskEnd;
        inOut[task] = make_pair(inCommTime, outCommTime);*/

        current_path(basePath);
    }

}

// get estimations from directory
void GetEstimations(map<int,pair<double, double>>&estimations, map<int,pair<int,int>>&taskId, vector<vector<int>>& matrix){
    path basePath = current_path();
    ofstream add("addInfo.dat");
    map <int, tuple<double,double,double,double>> times;
    // (taskID, wfID)
    map<int,int> taskWF;

    ReadData(taskId, taskWF, times);

    //// find minimum starting time
    double startTime = std::numeric_limits<double>::infinity();
    for (auto task = times.begin(); task != times.end(); task++){
        if (get<0>(task->second) < startTime)
            startTime = get<0>(task->second);
    }

    for (auto task = times.begin(); task != times.end(); task++){
        add << get<0>(*task) << " "  << get<0>(task->second) - startTime  << " " << get<1>(task->second) - startTime  << " " 
            << get<2>(task->second) - startTime  << " " << get<3>(task->second) - startTime  << endl;
    }

    for (auto task = times.begin(); task != times.end(); task++){
        double estComm = 0.0, estTime = get<2>(task->second) - get<1>(task->second);
        int taskId = task->first;
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
            estComm = get<0>(task->second) - startTime;
        // otherwise it is the length of period between finish of all parent tasks and start of current task
        else { 
            estComm = get<1>(task->second) - maxParentsFin;
        }
        estimations[taskId] = make_pair(estTime, estComm);
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

// (globalID, (wfId, localId))
void ReadLog(map<int,pair<int, int>>& taskId){
    ifstream log("log.txt");
    if (log.fail()){
         cout << "Error while opening log file " << endl;
         exit(1);
    }
   
    string s;
    do {
        getline(log,s);
        if (log.eof()){
            cout << "Log file does not contain workflow information";
            exit (1);
        }
    } while (s.find("Workflows information")==string::npos);
    int workflowId = 0;
    while (1){
        string toFind = "Task ID: ";
        int pos = 0;
        int clavireId = 0, localId = 0;
        bool isEnd = false;
        do {
            getline(log,s);
            pos = s.find(toFind);
            if (s.find("WF ID:") != string::npos)
                workflowId++;
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
        iss >> localId;
        if (iss.fail()){
             cout << "Error while reading workflow id " << endl;
             exit(1);
        }
        taskId[clavireId] = make_pair(workflowId, localId);
    }
    log.close();

    cout << "Log has been successfully read " << endl;
}

void InitComm(){
     // reading the communication matrix
    vector<vector<int>> matrix;
    ReadMatrix(matrix);
    map <int, pair<int,int>> taskId;
    ReadLog(taskId);
    
    // vector of [task id, (execTime, delayTime)]
    vector <map<int,pair<double, double>>> estimations;

    directory_iterator dirIt(current_path()), dirEnd;
    path basePath = current_path();
    while (dirIt != dirEnd){
        path current = *dirIt++;
        if (is_directory(current)){
            string pathStr = current.string();
            if (pathStr.find("Temp") != string::npos){
                current_path(current);
                map <int,pair<double, double>> est;
                GetEstimations(est, taskId, matrix);
                estimations.push_back(est);
            }
        }
        current_path(basePath);
    }

    cout << "Estimations have been successfully taken " << endl;
    // task id, (execTime, delayTime)
    map<int,pair<double, double>> resultEstimations;
    ofstream resFile("commTime.dat");
    if (estimations.size() == 0){
        cout << "Estimations vector cannot have zero size " << endl;
        exit(1);
    }
    for (auto estVector = estimations.begin(); estVector != estimations.end(); estVector++){
        for (auto est = estVector->begin(); est != estVector->end(); est++){
            //resFile << est.first << " " << est.second << endl;
            resultEstimations[est->first].first += est->second.first;
            resultEstimations[est->first].second += est->second.second;
        }
        //resFile << endl;
    }
       
    int estCount = estimations.size();

    for (auto resEst = resultEstimations.begin(); resEst != resultEstimations.end(); resEst++){
        resEst->second.first /= estCount;
        resEst->second.second /= estCount;
        resFile << resEst->first << " " << resEst->second.first << " " << resEst->second.second << endl;

    }
     resFile.close();
}

void GetEstimations(map <int,pair<double, double>>& est){
    ifstream comm("commTime.dat");
    if (comm.fail()){
        cout << "Failed to open commtime.dat" << endl;
        exit(1);
    }
    string s;
    while (getline(comm,s)){
        int task;
        double execTime, commTime;
        istringstream iss(s);
        iss >> task;
        if (iss.fail()){
            cout << "Failed to get number of tasks" << endl;
            exit(1);
        }
        iss >> execTime;
        if (iss.fail()){
            cout << "Failed to get execution time" << endl;
            exit(1);
        }
        iss >> commTime;
        if (iss.fail()){
            cout << "Failed to get communication time" << endl;
            exit(1);
        }
        est[task].first += execTime;
        est[task].second += commTime;

    }
}

void AggComm(){
    int estCount = 0;
    map <int,pair<double, double>> est;

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
    for (auto estim = est.begin(); estim != est.end(); estim++ ){
        estim->second.first /= estCount;
        estim->second.second /= estCount;
        out << estim->first << " " << estim->second.first << " " << estim->second.second << endl;
    }
    out.close();
}

string GetIPAddress(){
    string res;
    char szPath[128] = "";
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    gethostname(szPath, sizeof(szPath));
    printf("%s\n", szPath);

    struct hostent *phe = gethostbyname(szPath);
    if (phe == 0) {
        cerr << "Yow! Bad host lookup." << endl;
    }

    
    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
        if (i == 0) {
            string s(inet_ntoa(addr));
            res = s;
        }
    }

    WSACleanup(); 
    return res;
}

void GetEstimations(path& outputPath){
    // ORLY??
    vector<vector<int>> matrix;
    ReadMatrix(matrix);
    map <int, pair<int,int>> taskId;
    ReadLog(taskId);
    map <int, tuple<double,double,double,double>> times;
    // (taskID, wfID)
    map<int,int> taskWF;
    if (exists("Temp"))
        current_path("Temp");
    else {
        cout << "No temp folder was found" << endl;
        exit(1);
    }
    ReadData(taskId, taskWF, times);
    string ipAddress = GetIPAddress();
    string filename = outputPath.string() + "\\" + ipAddress;
    ofstream res(filename);
    if (res.fail()){
        cout << "Output file cannot be created" << endl;
        exit(1);
    }
     for (auto task = times.begin(); task != times.end(); task++){
        res << taskWF[get<0>(*task)] << " " << get<0>(*task) << " "  << get<0>(task->second) << " " << get<1>(task->second) << " " 
            << get<2>(task->second) << " " << get<3>(task->second) << endl;
    }
     res.close();
}
void ReadPath(path& outputPath){
    ifstream cfg("config.txt");
    if (cfg.fail()){
        cout << "Cannot open configuration file\n";
        exit(1);
    }
    string s;
    getline(cfg, s);
    if (s.find("Output path = ") == string::npos){
        cout << "Cannot find ouput path in configuration file\n";
        exit(1);
    }
    size_t pos = s.find_last_of(" ");
    if (pos == string::npos){
        cout << "Wrong format in output path line in configuration file\n";
        exit(1);
    }
    s.erase(0, pos + 1);
    outputPath.assign(s.begin(), s.end());
    if (exists(outputPath)){
        try {
            for (directory_iterator endDir, dir(outputPath); dir != endDir; dir++)
                remove_all(dir->path());
        }
        catch (filesystem_error const & e){
            cout << "Error while removing files from output directory\n";
            cout << e.what() << endl;
        }
    }
    else {
        try{
            if (!create_directory(outputPath)){
                cout << "Cannot create output directory\n";
                exit(1);
            }
        }
         catch (filesystem_error const & e){
            cout << "Error while removing files from output directory\n";
            cout << e.what() << endl;
        }
    }
    cfg.close();
}
void StatRes(){
    path outputPath;
    ReadPath(outputPath);
    GetEstimations(outputPath);
}

void ReadResourceInfo(map<string, int>& resInfo){
    ifstream cfg("config.txt");
    if (cfg.fail()){
        cout << "Cannot open configuration file\n";
        exit(1);
    }
    string s, toFind = "Path to resource description file = ";
    bool wasFound = false;
    size_t pos = string::npos;
    while (!wasFound) {
        getline(cfg, s);
        if ((pos = s.find(toFind)) != string::npos){
            wasFound = true;
        }
        if (cfg.eof() && !wasFound){
            cout << "Cannot find resource description file path in configuration file\n";
            exit(1);
        }
       
    }
    s.erase(0, toFind.size());
    path resFile(s);
    if (!exists(s)){
        cout << "Cannot find resource description file, path " << resFile.string() << endl;
        exit(1);
    }
    ifstream res(resFile.string());
    if (res.fail()){
        cout << "Cannot open resource description file, path " << resFile.string() << endl;
        exit(1);
    }
    do {
        getline(res, s);
    } while (s.find("NodeName") == string::npos);

    int resIndex = 1;

    while (!res.eof()){
        toFind = "\"NodeAddress\": ";
        pos = s.find(toFind);
        if (pos != string::npos){
            string address = s.substr(pos + 1, s.size() - pos);
            address.erase(0, toFind.size()-1);
            size_t firstQuote = address.find_first_of("\""),
                secondQuote = address.find_last_of("\"");
            address = address.substr(firstQuote + 1, secondQuote - firstQuote - 1);
            resInfo[address] = resIndex++;
        }
        getline(res, s);
    }

    cfg.close();
}

void StatAgg(){
    map<string, int> resInfo;
    ReadResourceInfo(resInfo);
    for (auto it = resInfo.begin(); it != resInfo.end(); it++)
        cout << it->first << " " << it-> second << endl;       
}



int _tmain(int argc, _TCHAR* argv[])
{
        
    if (argc == 1){
        InitComm();
    }
    else{
        string arg(argv[1]);
        if (arg == "-agg")
            AggComm();
        else if (arg == "-statres")
            StatRes();
        else if (arg == "-statagg")
            StatAgg();
        else {
            cout << "CommCreator [-agg] [-statres] [-statagg]" << endl;
            exit(1);
        }
            
    }
    system("pause");
    return 0;
}

