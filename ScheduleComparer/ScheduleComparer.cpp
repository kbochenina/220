// ScheduleComparer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
#include <vector>
#include <sstream>

using namespace std;
using namespace boost::filesystem;
using namespace boost::tuples;
using namespace boost;

typedef map<pair<int,int>, vector<pair<int,int>>> windowType;

void ReadWindows(string filename, windowType& windows){
    ifstream file(filename);
    if (file.fail()){
        cout << "ReadWindows() error. Error while opening resource file" << endl;
        std::system("pause");
        exit(2);
    }
    string s;
    int typeCounter = 0;
    while (getline(file, s)){
        if (typeCounter == 0){
            string toFind = "Type ";
            while (s.find(toFind) != 0){
                getline(file, s);
                if (file.eof()){
                    cout << "ReadWindows() error. Can not find information about resource types" << endl;
                    std::system("pause");
                    exit(3);
                }
            }
        }
        ++typeCounter;
        int processorCounter = 0;
        do {
            if (typeCounter == 1) 
                getline(file,s);
            if (s.find("Processor") != string::npos)
                ++processorCounter;
            else if ( s.find("Type") == string::npos){
                istringstream iss(s);
                int tbegin, tend;
                iss >> tbegin;
                iss >> tend;
                
                if (iss.fail() && !iss.eof()){
                    cout << "ReadWindows() error. Error while reading borders of time windows" << endl;
                    std::system("pause");
                    exit(4);
                }
                windows[make_pair(typeCounter, processorCounter)].push_back(make_pair(tbegin, tend));
            }
            else 
                break;
        } while (1);
    }
}

void PrintWindows(windowType windows){
    for (auto procIt = windows.begin(); procIt != windows.end(); procIt++){
        cout << "(" << procIt->first.first << " " << procIt->first.second << ")" << endl;
        for (auto windowIt = procIt->second.begin(); windowIt != procIt->second.end(); windowIt++){
            cout << windowIt->first << " " << windowIt->second << endl;
        }
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    windowType windows;
    directory_iterator dirIt(current_path()), dirEnd;
    cout << current_path() << endl;
    while (dirIt != dirEnd){
        path current = *dirIt++;
        if (is_directory(current))
            continue;
        string filename = current.string();
        size_t pos = filename.find_last_of("\\");
        filename.erase(0, pos + 1);
        cout << filename << endl;
        if (filename.find("res") == 0)
            ReadWindows(filename, windows);
        PrintWindows(windows);
    }
    std::system("pause");
    return 0;
}

