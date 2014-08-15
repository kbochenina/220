// GenerateHydro.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

int _tmain(int argc, _TCHAR* argv[])
{
    // N - a number of parallel tasks
    int N = 1 ;
    int filesCount = 20;
    if (argc == 1){
        N = 1;
    }
    else {
        N = _wtoi(argv[1]);
    }
    string filename = "n.8.0.dax";
    ifstream file(filename, ifstream::in);
    if (file.fail()) {
        cout << "Error while opening file " << endl;
        system("pause");
        exit(1);
    }
   create_directory("OutputDAX");
   current_path("OutputDAX");
   string s;
   getline(file,s);
   string newName = "n." + to_string((3 + 5*N)) + ".0.dax"; 
   ofstream newFile(newName);
   while (s.find("adag") == string::npos){
       newFile << s << endl;
       getline(file,s);
   }
   string adagBegin = "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"test\" jobCount=\"";
   string adagEnd = "\" fileCount=\"0\" childCount=\"48\">";
   string currentString = adagBegin + to_string(3+N*5) + adagEnd;
   newFile << currentString << endl;
   currentString = "<!-- part 1: list of all referenced files (may be empty) -->";
   newFile << currentString << endl;
   currentString = "<!-- part 2: definition of all jobs (at least one) -->";
   newFile << currentString << endl;
   currentString = "<job id=\"ID00000\" namespace=\"CyberShake\" name=\"ZipPSA\" version=\"1.0\" runtime=\"8\" resTypes=\"1\">";
   newFile << currentString << endl;


   string usesStart = "<uses file=\"";
   string usesOutputEnd = "\" link=\"output\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
   string usesInputEnd = "\" link=\"input\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
   int currentJob = 1;
   for (int i = 0; i < N * 2; i++){
       string currString = usesStart + to_string(currentJob) + "-" + to_string(2 + i +1) + usesOutputEnd;
       newFile << currString << endl;
   }
   string endJob = "</job>";
   newFile << endJob << endl;
   string jobStart = "<job id=\"ID00001\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"46\"  resTypes=\"2\">";
   newFile << jobStart << endl;
   currentJob = 2;
   for (int i = 0; i < N * 3; i++){
       string currString = usesStart + to_string(currentJob) + "-" + to_string(2 + N*2 + i + 1) + usesOutputEnd;
       newFile << currString << endl;
   }
   newFile << endJob << endl;
   jobStart = "<job id=\"ID0000";
   string jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"15\"  resTypes=\"1\">";
   for (int i = 0; i < N * 2; i++){
       if (i==N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"21\"  resTypes=\"1\">";
       string currString = jobStart + to_string(2+i) + jobEnd;
       newFile << currString << endl;
       string input, output;
       if (i==0)
           input = usesStart + to_string(1) + "-" + to_string(2 + i+1) + usesInputEnd;
       if (i!=0) 
           input = usesStart + to_string(2+i) + "-" + to_string(2 + i+1) + usesInputEnd;
       newFile << input << endl;
       if (i == 2*N-1)
           output = usesStart + to_string(2 + i+1) + "-" + to_string(2 + N*5 + 1) + usesOutputEnd;
       else 
           output = usesStart + to_string(2 + i+1) + "-" + to_string(2 + i + 2) + usesOutputEnd;
       newFile << output << endl;
       newFile << endJob << endl;
   }
   jobEnd = "\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"2\"  resTypes=\"2,3\">";
   for (int i = 0; i < N * 3; i++){
       if (i==N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"12\"  resTypes=\"2,3\">";
       if (i==2 * N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"32\"  resTypes=\"2,3\">";
       string currString = jobStart + to_string(2 + N* 2 + i) + jobEnd;
       newFile << currString << endl;

       string input, output;
       if (i==0)
           input = usesStart + to_string(2) + "-" + to_string(2 + N*2 + i + 1) + usesInputEnd;
       if (i!=0) 
           input = usesStart + to_string(2 + N*2 + i) + "-" + to_string(2 + N*2 + i + 1) + usesInputEnd;
       newFile << input << endl;
       if (i == 2*N-1)
           output = usesStart + to_string(2 + N*2 + i + 1) + "-" + to_string(2 + N*5 + 1) + usesOutputEnd;
       else 
           output = usesStart + to_string(2 + N*2 + i + 1) + "-" + to_string(2 + N*2 + i + 2) + usesOutputEnd;

       newFile << output << endl;
       newFile << endJob << endl;
   }
   jobEnd = "\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"3\"  resTypes=\"1\">";
    string currString = jobStart + to_string(2 + N* 5) + jobEnd;
    newFile << currString << endl;
    for (int i = 0; i < N * 5; i++){
    string input = usesStart + to_string(i+3) + "-" + to_string(2 + N* 5 +1) + usesInputEnd;
    newFile << input << endl; 
    }
    newFile << endJob << endl;
    newFile << "<!-- part 3: list of control-flow dependencies (may be empty) -->"<< endl;
    string cRefBegin = "<child ref=\"ID0000";
    string cRefEnd = "\">";
    string cEnd = " </child>";
    string pRefBegin = "<parent ref=\"ID0000";
    string pRefEnd = "\"/>";
    currentString = cRefBegin + to_string(0) + cRefEnd;
    newFile << currentString << endl;
    for (int i = 0; i < N * 2; i++){
        currentString = pRefBegin + to_string(2+i) + pRefEnd;
        newFile << currentString << endl;
    }
    newFile << cEnd << endl;
    currentString = cRefBegin + to_string(1) + cRefEnd;
    newFile << currentString << endl;
    for (int i = 0; i < N * 3; i++){
        currentString = pRefBegin + to_string(2+ N * 2 + i) + pRefEnd;
        newFile << currentString << endl;
    }
    newFile << cEnd << endl;
    for (int i = 0; i < N*5; i++){
        currentString = cRefBegin + to_string(2+i) + cRefEnd;
        newFile << currentString << endl;
        currentString = pRefBegin + to_string(2+ N * 5) + pRefEnd;
        newFile << currentString << endl;
    }
    currentString = "</adag>" ;
    newFile << currentString << endl;

    // getting many equal files
    newFile.close();
    for (int i = 1; i < filesCount; i++){
        ifstream basic(newName);
        string copyName = "n." + to_string((3 + 5*N)) + "." + to_string(i) + ".dax"; 
        ofstream copyFile(copyName);
        string s;
        while (!basic.eof()){
            getline(basic,s);
            copyFile << s << endl;
        }
        basic.close();
        copyFile.close();
    }
	return 0;
}

