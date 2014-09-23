// GenerateHydro.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

// SWAN
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
    string filename = "n.10.0.dax";
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
   string newName = "n." + to_string((8 + 2*N)) + ".0.dax"; 
   ofstream newFile(newName);
   while (s.find("adag") == string::npos){
       newFile << s << endl;
       getline(file,s);
   }
   string adagBegin = "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"test\" jobCount=\"";
   string adagEnd = "\" fileCount=\"0\" childCount=\"48\">";
   string currentString = adagBegin + to_string(8 + N*2) + adagEnd;
   newFile << currentString << endl;
   currentString = "<!-- part 1: list of all referenced files (may be empty) -->";
   newFile << currentString << endl;
   currentString = "<!-- part 2: definition of all jobs (at least one) -->";
   newFile << currentString << endl;
   currentString = "<job id=\"ID00000\" namespace=\"SWAN\" name=\"HIRLAM\" version=\"1.0\" runtime=\"42\" resTypes=\"2,3\">";
   newFile << currentString << endl;


   string usesStart = "<uses file=\"";
   string usesOutputEnd = "\" link=\"output\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
   string usesInputEnd = "\" link=\"input\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
   string endJob = "</job>";

   string jobName = "1-2";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl << endJob;

   currentString = "<job id=\"ID00001\" namespace=\"SWAN\" name=\"SWAN\" version=\"1.0\" runtime=\"28\" resTypes=\"1,2,3\">";
   newFile << currentString << endl;
   jobName = "1-2";
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   jobName = "2-3";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl;
   jobName = "2-4";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl;
   jobName = "2-5";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl << endJob;


   currentString = "<job id=\"ID00002\" namespace=\"SWAN\" name=\"S1\" version=\"1.0\" runtime=\"3\" resTypes=\"1\">";
   newFile << currentString << endl;
   jobName = "2-3";
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   jobName = "3-6";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl << endJob;

   currentString = "<job id=\"ID00003\" namespace=\"SWAN\" name=\"BSM-2010\" version=\"1.0\" runtime=\"22\" resTypes=\"1\">";
   newFile << currentString << endl;
   jobName = "2-4";
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;

   int currentJob = 4;
   for (int i = 0; i < N; i++){
       currentString = usesStart + to_string(currentJob) + "-" + to_string(6 + i + 1) + usesOutputEnd;
       newFile << currentString << endl;
   }
   
   newFile << endJob << endl;
   
   string jobStart = "<job id=\"ID00004\" namespace=\"SWAN\" name=\"SWAN\" version=\"1.0\" runtime=\"14\"  resTypes=\"1,2,3\">";
   newFile << jobStart << endl;
   jobName = "2-5";
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   jobName = "5-8";
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl << endJob;

   currentString = "<job id=\"ID00005\" namespace=\"SWAN\" name=\"S2\" version=\"1.0\" runtime=\"1\" resTypes=\"1\">";
   newFile << currentString << endl;
   jobName = "3-6";
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;

   currentJob = 6;
   for (int i = 0; i < N; i++){
       currentString = usesStart + to_string(currentJob) + "-" + to_string(6 + i + 1) + usesOutputEnd;
       newFile << currentString << endl;
   }
   
   jobName = "6-" + to_string(2 * N + 8 - 1);
   currentString = usesStart + jobName + usesOutputEnd;
   newFile << currentString << endl << endJob;

   jobStart = "<job id=\"ID0000";
   string jobEnd ="\" namespace=\"SWAN\" name=\"MVP\" version=\"1.0\" runtime=\"25\"  resTypes=\"1\">";
   for (int i = 0; i < N ; i++){
       string currString = jobStart + to_string(6 + i + 1) + jobEnd;
       newFile << currString << endl;
       string input, output;
       input = usesStart + to_string(4) + "-" + to_string(6 + i + 1) + usesInputEnd;
       newFile << input << endl;
       input = usesStart + to_string(6) + "-" + to_string(6 + i + 1) + usesInputEnd;
       newFile << input << endl;
       output = usesStart + to_string(6 + i + 1) + "-" + to_string(2 * N + 8 - 1) + usesOutputEnd;
       newFile << output << endl;
       newFile << endJob << endl;
   }




   jobEnd = "\" namespace=\"SWAN\" name=\"S3\" version=\"1.0\" runtime=\"1\"  resTypes=\"1\">";
   currentString = jobStart + to_string(N + 6 + 1) + jobEnd;
   newFile << currentString << endl;
   jobName = "5-" + to_string(N + 6 + 2);
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   jobName = to_string(N + 6 + 2) + "-" + to_string(2 * N + 8 - 1);
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   currentJob = N + 6 + 2;
   for (int i = 0; i < N; i++){
       currentString = usesStart + to_string(currentJob) + "-" + to_string(N + 6 + 2 + 1 + i) + usesOutputEnd;
       newFile << currentString << endl;
   }
   
   jobEnd = "\" namespace=\"SWAN\" name=\"DataProc\" version=\"1.0\" runtime=\"1\"  resTypes=\"1\">";
   currentString = jobStart + to_string(2 * N + 8 - 2) + jobEnd;
   newFile << currentString << endl;
   jobName = "6-" + to_string(2 * N + 8 - 1);
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
   currentJob = 2 * N + 8 - 1;
   for (int i = 0; i < N; i++){
       currentString = usesStart + "-" + to_string(6 + i + 1) + to_string(currentJob) + usesInputEnd;
       newFile << currentString << endl;
   }
   jobName = to_string(6 + N + 1) + to_string(currentJob);
   currentString = usesStart + jobName + usesInputEnd;
   newFile << currentString << endl;
    for (int i = 0; i < N; i++){
        currentString = usesStart + "-" + to_string(6 + N + 3 + i) + to_string(currentJob) + usesInputEnd;
       newFile << currentString << endl;
   }
   newFile << endJob << endl;

   jobEnd ="\" namespace=\"SWAN\" name=\"MVP\" version=\"1.0\" runtime=\"25\"  resTypes=\"1\">";
   for (int i = 0; i < N ; i++){
       string currString = jobStart + to_string(N + 6 + 3 + i) + jobEnd;
       newFile << currString << endl;
       string input, output;
       input = usesStart + to_string(N + 6 + 2) + "-" + to_string(N + 6 + 3 + i) + usesInputEnd;
       newFile << input << endl;
       output = usesStart + to_string(N + 6 + 3 + i) + "-" + to_string(2 * N + 8 - 1) + usesOutputEnd;
       newFile << output << endl;
       newFile << endJob << endl;
   }

   
   
    newFile << "<!-- part 3: list of control-flow dependencies (may be empty) -->"<< endl;
    newFile << "Old values, had not time to generate" << endl;
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
        cout << N << endl;
        string copyName = "n." + to_string((8 + 2*N)) + "." + to_string(i) + ".dax"; 
        ofstream copyFile(copyName);
        string s;
        while (!basic.eof()){
            getline(basic,s);
            copyFile << s << endl;
        }
        basic.close();
        copyFile.close();
    }
    system("pause");
	return 0;
}

//int _tmain(int argc, _TCHAR* argv[])
//{
//    // N - a number of parallel tasks
//    int N = 1 ;
//    int filesCount = 20;
//    if (argc == 1){
//        N = 1;
//    }
//    else {
//        N = _wtoi(argv[1]);
//    }
//    string filename = "n.8.0.dax";
//    ifstream file(filename, ifstream::in);
//    if (file.fail()) {
//        cout << "Error while opening file " << endl;
//        system("pause");
//        exit(1);
//    }
//   create_directory("OutputDAX");
//   current_path("OutputDAX");
//   string s;
//   getline(file,s);
//   string newName = "n." + to_string((3 + 5*N)) + ".0.dax"; 
//   ofstream newFile(newName);
//   while (s.find("adag") == string::npos){
//       newFile << s << endl;
//       getline(file,s);
//   }
//   string adagBegin = "<adag xmlns=\"http://pegasus.isi.edu/schema/DAX\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pegasus.isi.edu/schema/DAX http://pegasus.isi.edu/schema/dax-2.1.xsd\" version=\"2.1\" count=\"1\" index=\"0\" name=\"test\" jobCount=\"";
//   string adagEnd = "\" fileCount=\"0\" childCount=\"48\">";
//   string currentString = adagBegin + to_string(3+N*5) + adagEnd;
//   newFile << currentString << endl;
//   currentString = "<!-- part 1: list of all referenced files (may be empty) -->";
//   newFile << currentString << endl;
//   currentString = "<!-- part 2: definition of all jobs (at least one) -->";
//   newFile << currentString << endl;
//   currentString = "<job id=\"ID00000\" namespace=\"CyberShake\" name=\"ZipPSA\" version=\"1.0\" runtime=\"8\" resTypes=\"1\">";
//   newFile << currentString << endl;
//
//
//   string usesStart = "<uses file=\"";
//   string usesOutputEnd = "\" link=\"output\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
//   string usesInputEnd = "\" link=\"input\" register=\"true\" transfer=\"true\" optional=\"false\" type=\"data\" size=\"0\"/>";
//   int currentJob = 1;
//   for (int i = 0; i < N * 2; i++){
//       string currString = usesStart + to_string(currentJob) + "-" + to_string(2 + i +1) + usesOutputEnd;
//       newFile << currString << endl;
//   }
//   string endJob = "</job>";
//   newFile << endJob << endl;
//   string jobStart = "<job id=\"ID00001\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"46\"  resTypes=\"2\">";
//   newFile << jobStart << endl;
//   currentJob = 2;
//   for (int i = 0; i < N * 3; i++){
//       string currString = usesStart + to_string(currentJob) + "-" + to_string(2 + N*2 + i + 1) + usesOutputEnd;
//       newFile << currString << endl;
//   }
//   newFile << endJob << endl;
//   jobStart = "<job id=\"ID0000";
//   string jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"15\"  resTypes=\"1\">";
//   for (int i = 0; i < N * 2; i++){
//       if (i==N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"21\"  resTypes=\"1\">";
//       string currString = jobStart + to_string(2+i) + jobEnd;
//       newFile << currString << endl;
//       string input, output;
//       if (i==0)
//           input = usesStart + to_string(1) + "-" + to_string(2 + i+1) + usesInputEnd;
//       if (i!=0) 
//           input = usesStart + to_string(2+i) + "-" + to_string(2 + i+1) + usesInputEnd;
//       newFile << input << endl;
//       if (i == 2*N-1)
//           output = usesStart + to_string(2 + i+1) + "-" + to_string(2 + N*5 + 1) + usesOutputEnd;
//       else 
//           output = usesStart + to_string(2 + i+1) + "-" + to_string(2 + i + 2) + usesOutputEnd;
//       newFile << output << endl;
//       newFile << endJob << endl;
//   }
//   jobEnd = "\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"2\"  resTypes=\"2,3\">";
//   for (int i = 0; i < N * 3; i++){
//       if (i==N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"12\"  resTypes=\"2,3\">";
//       if (i==2 * N) jobEnd ="\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"32\"  resTypes=\"2,3\">";
//       string currString = jobStart + to_string(2 + N* 2 + i) + jobEnd;
//       newFile << currString << endl;
//
//       string input, output;
//       if (i==0)
//           input = usesStart + to_string(2) + "-" + to_string(2 + N*2 + i + 1) + usesInputEnd;
//       if (i!=0) 
//           input = usesStart + to_string(2 + N*2 + i) + "-" + to_string(2 + N*2 + i + 1) + usesInputEnd;
//       newFile << input << endl;
//       if (i == 2*N-1)
//           output = usesStart + to_string(2 + N*2 + i + 1) + "-" + to_string(2 + N*5 + 1) + usesOutputEnd;
//       else 
//           output = usesStart + to_string(2 + N*2 + i + 1) + "-" + to_string(2 + N*2 + i + 2) + usesOutputEnd;
//
//       newFile << output << endl;
//       newFile << endJob << endl;
//   }
//   jobEnd = "\" namespace=\"CyberShake\" name=\"ZipSeis\" version=\"1.0\" runtime=\"3\"  resTypes=\"1\">";
//    string currString = jobStart + to_string(2 + N* 5) + jobEnd;
//    newFile << currString << endl;
//    for (int i = 0; i < N * 5; i++){
//    string input = usesStart + to_string(i+3) + "-" + to_string(2 + N* 5 +1) + usesInputEnd;
//    newFile << input << endl; 
//    }
//    newFile << endJob << endl;
//    newFile << "<!-- part 3: list of control-flow dependencies (may be empty) -->"<< endl;
//    string cRefBegin = "<child ref=\"ID0000";
//    string cRefEnd = "\">";
//    string cEnd = " </child>";
//    string pRefBegin = "<parent ref=\"ID0000";
//    string pRefEnd = "\"/>";
//    currentString = cRefBegin + to_string(0) + cRefEnd;
//    newFile << currentString << endl;
//    for (int i = 0; i < N * 2; i++){
//        currentString = pRefBegin + to_string(2+i) + pRefEnd;
//        newFile << currentString << endl;
//    }
//    newFile << cEnd << endl;
//    currentString = cRefBegin + to_string(1) + cRefEnd;
//    newFile << currentString << endl;
//    for (int i = 0; i < N * 3; i++){
//        currentString = pRefBegin + to_string(2+ N * 2 + i) + pRefEnd;
//        newFile << currentString << endl;
//    }
//    newFile << cEnd << endl;
//    for (int i = 0; i < N*5; i++){
//        currentString = cRefBegin + to_string(2+i) + cRefEnd;
//        newFile << currentString << endl;
//        currentString = pRefBegin + to_string(2+ N * 5) + pRefEnd;
//        newFile << currentString << endl;
//    }
//    currentString = "</adag>" ;
//    newFile << currentString << endl;
//
//    // getting many equal files
//    newFile.close();
//    for (int i = 1; i < filesCount; i++){
//        ifstream basic(newName);
//        string copyName = "n." + to_string((3 + 5*N)) + "." + to_string(i) + ".dax"; 
//        ofstream copyFile(copyName);
//        string s;
//        while (!basic.eof()){
//            getline(basic,s);
//            copyFile << s << endl;
//        }
//        basic.close();
//        copyFile.close();
//    }
//	return 0;
//}

