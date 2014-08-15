// DAXCreator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <map>
#include <boost/filesystem.hpp>
#include <sstream>

using namespace std;
using namespace boost::filesystem;

// terrible variable
string previous;

double findAvgTime(ifstream &out, string name){
    string toFind = "timethis";
    double res = 0.0;
    int pCount = 0;
    string s =  previous;
    while (s.find(toFind)==string::npos){
        getline(out,s);
    }
    
    bool isName = false;
    while (!isName){
        if (s.find(name) != string::npos)
            isName = true;
        else {
            getline(out,s);
            if (out.eof()){
                cout << "Profiler file does not contain info for package " << name << endl;
                system("pause");
                exit(1);
            }
        }
    }
    
    while (isName){
        while (s.find("Elapsed Time") ==  string::npos) {
            getline(out,s);
            if (out.eof()){
                cout << "Profiler file does not contain info about elapsed time for package " << name << endl;
                system("pause");
                exit(1);
            }
        }
        size_t pos = s.find_last_of(":");
        string time = s.substr(pos+1, s.size() - pos - 1);
        res += stod(time);
        pCount++;
        do {
            getline(out,s);
            if (out.eof())
                break;
        }
        while (s.find("timethis") == string::npos);

        if (s.find(name) == string::npos){
            isName = false;
            previous = s;
        }

    } 
    return res/pCount;
}

int GetJobNumber(string s){
    size_t posBegin = s.find_first_of("\""),
        posEnd = s.find_last_of("\"");
    string id = s.substr(posBegin + 1, posEnd - 1);
    // erase "ID"
    id.erase(0,2);
    istringstream iss(id);
    int res;
    iss >> res;
    if (iss.fail()){
         cout << "Error while converting job id " << endl;
         exit(1);
    }
    return res;
}

void GetMComm(int jobCount){
    ifstream dag("dag_1.dax");
    vector <vector<int>> matrix;
    matrix.resize(jobCount);
    for (int i = 0; i < jobCount; i++)
        matrix[i].resize(jobCount);
    string s;
    while (1){
        bool fileEnd = false;
        do{
            getline(dag,s);
            if (s.find("</adag>") != string::npos){
                fileEnd = true;
                break;
            }
        } while (s.find("<child") == string::npos);
        if (fileEnd)
            break;
        int jobChild = GetJobNumber(s);
        while (1){
            getline(dag,s);
            if (s.find("</child") != string::npos)
                break;
            int jobParent = GetJobNumber(s);
            matrix[jobParent-1][jobChild-1] = 1;
        }
    }
    dag.close();
    ofstream mcomm("data.mcomm");
    for (auto &row : matrix){
        for (auto &col: row){
            mcomm << col << " ";
        }
        mcomm << endl;
    }
    mcomm.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
    // number of workflows in the set (command line parameter)
    int N = 1 ;
    if (argc == 1){
        N = 1;
    }
    else {
        N = _wtoi(argv[1]);
    }
    ifstream dag("dag.xml"); 
    if (dag.fail()) {
        cout << "Error while opening file " << endl;
      //  system("pause");
        exit(1);
    }
    ofstream dagnew("dag_new.xml"); 
    if (dagnew.fail()) {
        cout << "Error while creating file " << endl;
     //   system("pause");
        exit(1);
    }
    // finding actual job count
    string s;
    int jobCount = 0;
    while (getline(dag,s)){
        if (s.find("<job ") != string::npos)
            jobCount++;
    }
    //cout << "Actual job count " << jobCount << endl;
    dag.close();
    dag.open("dag.xml");
    bool adagFound = false;

    while (!adagFound){
        getline(dag,s);
        if (s.find("count=") == string::npos){
            dagnew << s << endl;
        }
        else {
            string toFind = "name=\"montage\"";
            size_t pos = s.find(toFind);
            if (pos != string::npos){
                string add = " jobCount=\"" + to_string(jobCount)+"\"";
                s.insert(pos + toFind.size(), add);
                dagnew << s << endl;
                adagFound = true;
            }
            else {
                cout << "Error in adag description " << endl;
             //   system("pause");
                exit(1);
            }
        }
    }

    bool jobFound = false;
    while (!jobFound){
        getline(dag,s);
        if (s.find("<job") == string::npos)
            dagnew << s << endl;
        else jobFound = true;
    }

    map <string, int> filesSize;
    path p("workdir");
    for (directory_iterator itr(p); itr!=directory_iterator(); ++itr){
        string name = itr->path().filename().string();
        int size = file_size(itr->path());
        filesSize[name] = size;
    }

    ifstream out("fileOut.txt");
    if (out.fail()) {
        cout << "Error while opening file " << endl;
       // system("pause");
        exit(1);
    }

    string jobName;
    double res  = 0.0;

    int processedJobs = 0;
    while (processedJobs < jobCount){
        // find job name
        string name;
        string toFind = "name=";
        size_t posNameBegin = s.find(toFind);
        if (posNameBegin == string::npos){
            cout << "Error in job description " << endl;
           // system("pause");
            exit(1);
        }
        posNameBegin += toFind.size()+1;
        size_t posNameEnd = s.find("\"", posNameBegin);
        name = s.substr(posNameBegin, posNameEnd - posNameBegin);
        // if previous job didn't have the same type
        if (jobName != name){
             // find average execution time from profiler file
             res = findAvgTime(out, name);
             jobName = name;
        }
       
        // find position for insertion of information
       size_t lastQuote = s.find_last_of("\"");
       string add = " runtime=\"" + to_string(res) + "\" resTypes=\"\"";
       s.insert(lastQuote + 1, add);
      
       while (s.find("<uses") == string::npos) {
           dagnew << s << endl;
           getline(dag,s);
       }
       while (s.find("<uses") != string::npos){
           string toFind = "file=\"";
           size_t posBegin = s.find(toFind);
           posBegin += toFind.size();
           size_t posEnd = s.find("\"", posBegin);
           string name = s.substr(posBegin, posEnd - posBegin);
           if (name.find("big_region") != string::npos)
                name = "big_region.hdr";
            else if (name.find("statfile") != string::npos)
                name = "statfile.tbl";
            else if (name.find("pimages") != string::npos)
                name = "pimages.tbl";
            else if (name.find("cimages") != string::npos)
                name = "cimages.tbl";
            else if (name.find("region") != string::npos)
                name = "region.hdr";
            else if (name.find("mosaic") != string::npos){
                if (name.find("area") == string::npos)
                    name = "mosaic.fits";
                else 
                    name = "mosaic_area.fits";
            }
            else if (name.find("shrunken") != string::npos){
                if (name.find("jpg") == string::npos)
                    name = "shrunken.fits";
                else 
                    name = "shrunken.jpg";
            }
           double size = filesSize[name];
           size_t lastQuote = s.find_last_of("\"");
           string add = " size=\"" + to_string(static_cast<int>(size)) + "\"";
           s.insert(lastQuote + 1, add);
           dagnew << s << endl;
           getline(dag,s);
       }
       getline(dag,s);
       while (s.find("<job id") == string::npos){
           dagnew << s << endl;
           getline(dag,s);
           if (dag.eof())
               break;
       }
       processedJobs ++;
       //cout << "processedJobs = " << processedJobs << " name = " << name << endl;
    }
    dagnew.close();
    dag.close();
    create_directory("wfset");
    current_path("wfset");
    for (int i = 0; i < N; i++){
        string fname = "dag_" + to_string(i+1) + ".dax";
        ofstream file(fname);
        ifstream dagread("..\\dag_new.xml");
        string s;
        while (getline(dagread,s)){
            file << s << endl;
        }
        dagread.close();
        file.close();
    }

    GetMComm(jobCount);

    //system("pause");
	 return 0;
}

