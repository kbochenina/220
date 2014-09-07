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

void ReadData(map<int,pair<int,int>>&taskId, map<int,int>& taskWF,  map <int, boost::tuple<double,double,double,double>>& times){
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
        //cout << dirName << " ";
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
        //cout << "Task: " << task << endl;
        taskWF[task] = taskId[clavireId].first;
        times[clavireId] = boost::make_tuple(scriptBegin, taskBegin, taskEnd, scriptEnd);
       /* double inCommTime = taskBegin - scriptBegin,
            outCommTime = scriptEnd - taskEnd;
        inOut[task] = make_pair(inCommTime, outCommTime);*/

        current_path(basePath);
    }
    cout << times.size() << endl;
}

// get estimations from directory
void GetEstimations(map<int,pair<double, double>>&estimations, map<int,pair<int,int>>&taskId, vector<vector<int>>& matrix){
    path basePath = current_path();
    ofstream add("addInfo.dat");
    map <int, boost::tuple<double,double,double,double>> times;
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
        add << taskId[get<0>(*task)].second << " "  << get<0>(task->second) - startTime  << " " << get<1>(task->second) - startTime  << " " 
            << get<2>(task->second) - startTime  << " " << get<3>(task->second) - startTime  << endl;
    }

    for (auto task = times.begin(); task != times.end(); task++){
        double estComm = 0.0, estTime = get<2>(task->second) - get<1>(task->second);
        int currentTaskId = taskId[task->first].first - 1;
        double maxParentsFin = 0.0;
        bool isInitTask = true;
        // find finishing times for all parents of current task
        for (int i = 0; i < matrix.size(); i++){
            if (currentTaskId >= matrix.size() - 1){
                cout << "GetEstimations() error. Current task id " << currentTaskId << " > size of dependency matrix " << matrix.size() << endl;
                exit(1);
            }

            // if task has parents
            if (matrix[i][currentTaskId] != 0){
                // time between end of parent task and end of script
                double commParents = get<3>(times[i+1]) - get<2>(times[i+1]);
                if (commParents > maxParentsFin)
                    maxParentsFin = commParents;
                isInitTask = false;
            }
        }
        // time before start of initial task is time between start of the script and start of task execution
        if (isInitTask)
            estComm = get<0>(task->second) - startTime;
        // otherwise it is sum of task begin communication time and maximum end communication time among its parents
        else { 
            estComm = get<1>(task->second) - get<0>(task->second) + maxParentsFin;
        }
        estimations[currentTaskId] = make_pair(estTime, estComm);
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
    } while (s.find("=Workflows information=")==string::npos);
    // dirty trick (current log file contains two sections with wf information)
    do {
        getline(log,s);
        if (log.eof()){
            cout << "Log file does not contain workflow information";
            exit (1);
        }
    } while (s.find("=Workflows information=")==string::npos);

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
        // to pass the line in new version of log file
        getline(log, s);
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

void GetEstimations(path& inputPath, path& outputPath){
    // ORLY??
    vector<vector<int>> matrix;
    ReadMatrix(matrix);
    map <int, pair<int,int>> taskId;
    ReadLog(taskId);
    map <int, boost::tuple<double,double,double,double>> times;
    // (taskID, wfID)
    map<int,int> taskWF;
    
    if (exists(inputPath))
        current_path(inputPath);
    else {
        cout << "No input folder " << inputPath << " was found" << endl;
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
        res << taskId[get<0>(*task)].first << " " << taskId[get<0>(*task)].second << " "  << get<0>(task->second) << " " << get<1>(task->second) << " " 
            << get<2>(task->second) << " " << get<3>(task->second) << endl;
    }
     res.close();
}
void ReadPath(path& outputPath, bool isNewOutputPath, string whichPath){
    //cout << current_path() << endl;
    ifstream cfg("config.txt");
    if (cfg.fail()){
        cout << "Cannot open configuration file\n";
        exit(1);
    }
    string s, toFind;
    if (whichPath == "output")
        toFind = "Output path = ";
    else if (whichPath == "statagg"){
        toFind = "Path to stat agg file = ";
        isNewOutputPath = false;
    }
    else if (whichPath == "input"){
        toFind = "Path to input files = ";
        isNewOutputPath = false;
    }
    else if (whichPath == "working"){
        toFind = "Path to working directory = ";
        isNewOutputPath = false;
    }
    else {
        cout << "ReadPath() wrong parameter. Third parameter can be \"output\" or \"statagg\" or \"input\" or \"working\"";
        exit(1);
    }

    bool wasFound = false;

    while (getline(cfg, s)){
        if (s.find(toFind) != string::npos){
           wasFound = true; 
           break;
        }
    }

    if (!wasFound){
        cout << "Cannot find " << whichPath << " path in configuration file\n";
        exit(1);
    }
    size_t pos = s.find_last_of(" ");
    if (pos == string::npos){
        cout << "Wrong format in " << whichPath << " path line in configuration file\n";
        exit(1);
    }
    s.erase(0, pos + 1);
    outputPath.assign(s.begin(), s.end());
    if (!exists(outputPath)){
        try{
            if (isNewOutputPath){
                if (!create_directory(outputPath)){
                    cout << "Cannot create output directory\n";
                    exit(1);
                }
            }
            else {
                cout << whichPath << " path does not exist\n";
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
    path inputPath, outputPath;
    ReadPath(inputPath, false, "input");
    ReadPath(outputPath, true, "output");
    cout << inputPath << endl;

    GetEstimations(inputPath, outputPath);
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

typedef boost::tuple<int, int, double, double, double, double> esttuple;

bool compare(const esttuple& first, const esttuple& second ){
    return get<2>(first) <  get<2>(second);
}

void PrintTupleVec(const vector<esttuple>& estim){
    for (auto tupleIt = estim.begin(); tupleIt != estim.end(); tupleIt++ ){
        cout << tupleIt->get<0>() << " " << tupleIt->get<1>() << " " << tupleIt->get<2>() << " " <<
                tupleIt->get<3>() << " " << tupleIt->get<4>() << " " << tupleIt->get<5>() << endl;
    }
}

void StatAgg(){
    map<string, int> resInfo;
    ReadResourceInfo(resInfo);
    /*for (auto it = resInfo.begin(); it != resInfo.end(); it++)
        cout << it->first << " " << it-> second << endl;       */
    path outputPath;
    ReadPath(outputPath, false, "output");
    path statAggPath;
    ReadPath(statAggPath, false, "statagg");
    ofstream statagg(statAggPath.string() + "\\stat.dat");
    if (statagg.fail()){
        cout << "Cannot create stat.dat in " << statAggPath.string() << endl;
        exit(1);
    }

    // resIndex, wfId, taskId, script_start, task_start, task_finished, script_finished
    map <int, vector<boost::tuple<int, int, double, double, double, double>>> estimations;

    double min = numeric_limits<double>::infinity();

    for (directory_iterator end, begin(outputPath); begin != end; begin++){
        if (!is_directory(*begin)){
            string fileName = begin->path().string();
            size_t pos = fileName.find_last_of("\\");
            fileName.erase(0, pos + 1);
            if (resInfo.find(fileName) == resInfo.end()){
                cout << "Resource description file does not contain information about resource " << fileName << endl;
                exit(1);
            }
            int resIndex = resInfo[fileName];
            ifstream file(begin->path().string());
            if (file.fail()){
                cout << "Cannot open file " << begin->path().string() << endl;
                exit(1);
            }
            string s;
            vector<boost::tuple<int, int, double, double, double, double>> resEstimations;
            while (getline(file, s)){
                istringstream iss(s);
                int wfID, taskID;
                double scriptStart, taskStart, taskEnd, scriptEnd;
                bool error = false;
                iss >> wfID;
                if (iss.fail()) error = true;
                iss >> taskID;
                if (iss.fail()) error = true;
                 iss >> scriptStart;
                if (iss.fail()) error = true;
                iss >> taskStart;
                if (iss.fail()) error = true;
                 iss >> taskEnd;
                if (iss.fail()) error = true;
                iss >> scriptEnd;
                if (iss.fail()) error = true;
                if (error){
                    cout << "Error while reading data from stat file " << endl;
                    exit(1);
                }
                if (scriptStart < min) min = scriptStart;
                resEstimations.push_back(make_tuple(wfID, taskID, scriptStart, taskStart, taskEnd, scriptEnd));
            }
            estimations[resIndex] = resEstimations;
            file.close();
        }
    }

    // subtracting min from all times
    for (auto it = estimations.begin(); it != estimations.end(); it++ ){
        for (auto tupleIt = it->second.begin(); tupleIt != it->second.end(); tupleIt++ ){
            tupleIt->get<2>() -= min;
            tupleIt->get<3>() -= min;
            tupleIt->get<4>() -= min;
            tupleIt->get<5>() -= min;
        }
    }



    // writing information to a file
    for (auto it = estimations.begin(); it != estimations.end(); it++ ){
        statagg << "Node " << it->first << endl;
        vector<esttuple> &estim = it->second;
        /*cout << "Before sorting: " << endl;
        PrintTupleVec(estim);*/
        sort(estim.begin(), estim.end(), compare);
        /*cout << "After sorting: " << endl;
        PrintTupleVec(estim);*/
        for (auto tupleIt = estim.begin(); tupleIt != estim.end(); tupleIt++ ){
            statagg << tupleIt->get<0>() << " " << tupleIt->get<1>() << " " << tupleIt->get<2>() << " " <<
                tupleIt->get<3>() << " " << tupleIt->get<4>() << " " << tupleIt->get<5>() << endl;
        }
    }
    statagg.close();
}



int _tmain(int argc, _TCHAR* argv[])
{
    //current_path("D:\\ITMO\\Degree\\Programs\\WFSched\\CommCreator");
    path cPath;
    ReadPath(cPath, false, "working");
    current_path(cPath);
    
  // current_path("D:\\ITMO\\Degree\\Programs\\WFSched\\CommCreator");       
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
    //system("pause");
    return 0;
}

