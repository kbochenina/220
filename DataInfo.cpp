#include "StdAfx.h"
#include "DataInfo.h"
#include <fstream>
#include <sstream> // istringstream
#include <string>
#include <iostream>
#include <boost/filesystem.hpp> // directory_iterator, path
#include <iterator>
#include "direct.h"
#include "UserException.h"

using namespace boost::filesystem; // directory_iterator, path

DataInfo::~DataInfo(void)
{
}

DataInfo::DataInfo( string fSettings )
{
   Init(fSettings);
    _mkdir("Output");
   _chdir("Output");
  
}

const double DataInfo::GetDeadline(int wfNum){
    return workflows[wfNum].GetDeadline();
}

void DataInfo::Init(string settingsFile){
   try{
      char second[21]; 
      ifstream file(settingsFile, ifstream::in);
      string errOpen = "File " + settingsFile + " was not open";
      string errWrongFormat = "Wrong format in file " + settingsFile + " at line ";
      string errWrongValue = "Wrong value of parameter ";
      string errWrongFormatFull = errWrongFormat;
      string errEarlyEnd = "Unexpected end of file " + settingsFile;
      if (file.fail()) throw UserException(errOpen);
      unsigned int line = 0;
      string s, trim;
      int T = 0, delta = 0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "canExecuteOnDiffResources=";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int flag = atoi(s.c_str());
      bool canExecuteOnDiffResources;
      if (flag==1) canExecuteOnDiffResources = true;
      else if (flag == 0) canExecuteOnDiffResources = false;
      else throw UserException(errWrongValue + "canExecuteOnDiffResources");
      // InputFolderPath="FolderName"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "InputFolderPath=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      path dir = s;
      // DebugInfoFile="debugInfoFileName"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "DebugInfoFile=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      string openErr = " DebugInfoFile cannot be open";
      ofstream ex;
      ex.open(s, ios::app);
      if (ex.fail()) throw UserException(openErr);
      // ResultFile="result.txt"
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "ResultFile=\"";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      trim = "\"";
      found = s.find(trim);
      if (found != s.size()-1) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(found,1);
      string resFileName = s;
      // T=Tvalue
      getline(file,s);
      ++line;
      trim = "T=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      T = stoi(s);
      
      double CCR = 0.0;
      
      getline(file,s);
      ++line;
      trim = "CCR=";
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      CCR = stof(s);

      context.SetContext(T, CCR);	
      
      // read all filenames from path
      string resourcesFileName;
      vector <string> WFFileNames;
      bool isResourcesFileWasFound = false;
      for (directory_iterator it(dir), end; it != end; ++it) 
      {
         std::cout << "File processed - ";
         std::cout << *it << std::endl;
         string filename = it->path().string();
         if (filename.find("res")==string::npos && filename.find("n")==string::npos) continue;
         if (filename.find("res")!=string::npos ){
            resourcesFileName = filename;
            if (isResourcesFileWasFound == true)
                throw UserException("InputFiles folder contains more than 1 file with resource description");
            isResourcesFileWasFound = true;
         }
         else 
            WFFileNames.push_back(filename);
      }
      InitResources(resourcesFileName, canExecuteOnDiffResources);
      for (vector<string>::iterator it = WFFileNames.begin(); it!= WFFileNames.end(); it++)
         InitWorkflows(*it);
      int initNum = 0;
      initPackageNumbers.resize(workflows.size());
      for (int i = 0; i < workflows.size(); i++){
         initPackageNumbers[i] = initNum;
         initNum += workflows[i].GetPackageCount();
      }
      ofstream resTime("Output/time.txt");
      cout << "Start to init finishing times..." << endl;
      double t = clock();
      InitFinishingTimes();
      double end = (clock()-t)/1000.0 ;
      cout << "Time of init finishing times " << end << endl;
      resTime << "Time of init finishing times " << end << endl;
      resTime.close();
      ex.close();
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
   catch (std::exception& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// NOTE: types will be recorded from 1!
void DataInfo::InitWorkflows(string f){
   try{
      char second[21]; // enough to hold all numbers up to 64-bits
      ifstream file(f, ifstream::in);
      string errOpen = "File " + f + " was not open";
      string errEarlyEnd = "Unexpected end of file " + f;
      string errWrongFormat = "Wrong format in file " + f + " at line ";
      string errPackagesCount = "Packages count cannot be less than 1";
      string errCoresCount = "Cores count cannot be less than 1";
      string errConnMatrix = "Wrong value in connectivity matrix";
      string errWrongFormatFull = errWrongFormat;
      if (file.fail()) throw UserException(errOpen);

      string s, trim; int line = 0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "Workflows count = ";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int workflowsCount = stoi(s);
      vector <int> types; map <pair <int,int>, double> execTime; vector <Package> pacs; 
      vector <int> cCount; 
      vector <vector <int>> connectMatrix;
      int fullPackagesCount = 0;
      for (int i = 0; i < workflowsCount; i++){
         int packagesCount = 0;
         getline(file,s);
         ++line;
         if (file.eof()) throw UserException(errEarlyEnd);

         if ((found = s.find("(")) == std::string::npos){
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,found+1);
         istringstream iss(s);
         iss >> packagesCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         if (packagesCount < 1) {
            sprintf_s(second, "%d", i+1);
            string beginStr = "Workflow ";
            beginStr += second;
            beginStr += " - ";
            beginStr += errPackagesCount;
            throw UserException(beginStr);
         }
         for (int j = 0; j < packagesCount; j++){
            double alpha = 0.0; // part of consequentually executed code
            ++fullPackagesCount;
            // Package [packageNumber]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            // Alpha: [alpha value]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Alpha: ";
            size_t found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            iss >> alpha;
            if (iss.fail()) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            // Resource types: [resource types values]. -1 means all possible resources
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Resources types: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            int typeNumber = 0;
            string comma;
            do{
               comma = "";
               iss >> typeNumber;
               // if package can execute on all possible resources
               if (typeNumber == -1){
                  for (unsigned int i = 0; i < resources.size(); i++)
                     types.push_back(i+1);
                  break;
               }
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               types.push_back(typeNumber);
               iss >> comma;
            } while (comma==",");



            // Cores count: [cores count values]
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Cores count: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            int coresCount = 0;
            iss.str(s);
            iss.clear();
            int coreCount = 0;
            do{
               comma = "";
               iss >> coreCount;
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               cCount.push_back(coreCount);
               iss >> comma;
            } while (comma==",");

            if (cCount.size() < 1) {
               sprintf_s(second, "%d", i+1);
               string beginStr = "Workflow ";
               beginStr += second;
               beginStr += " - ";
               beginStr += errCoresCount;
               throw UserException(beginStr);
            }

            // Computational amount: [amount value]
            long int amount = 0;
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            trim = "Computation amount: ";
            found = s.find(trim);
            if (found != 0) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }
            s.erase(0,trim.size());
            iss.str(s);
            iss.clear();
            iss >> amount;
            if (iss.fail()) {
               sprintf_s(second, "%d", line);
               errWrongFormatFull += second;
               throw UserException(errWrongFormatFull);
            }

            for (unsigned int k = 0; k < types.size(); k++){
               for (unsigned int l = 0; l < cCount.size(); l++){
                  // assume that the core numbers are in ascending order (else continue)
                  if (resources[k].GetCoresCount() < cCount[l]) break; 
                  // Amdal's law
                  double acc = (double) 1.00 / (alpha + (1-alpha)/(l+1));
                  // execTime = amount / (perf * acc)
                  double exTime = amount / (resources[k].GetPerf() * acc);
                  execTime.insert(make_pair(make_pair(types[k], cCount[l]), exTime));
               }
            }
            Package p(fullPackagesCount,types,cCount, execTime, amount);
            //pacs.push_back(std::move(p));
            pacs.push_back(p);
            types.clear();
            execTime.clear();
            cCount.clear();
         }
         getline(file,s);
         if (file.eof()) throw UserException(errEarlyEnd);
         ++line;
         for (int j = 0; j < packagesCount; j++){
            vector <int> row;
            getline(file,s);
            if (file.eof()) throw UserException(errEarlyEnd);
            ++line;
            iss.str(s);
            iss.clear();
            for (int k = 0; k < packagesCount; k++){
               int val = 0;
               iss >> val;
               if (iss.fail()) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               if (val!=0  && val!=1){
                  sprintf_s(second, "%d", i+1);
                  string beginStr = "Workflow ";
                  beginStr += second;
                  beginStr += " - ";
                  beginStr += errConnMatrix;
                  throw UserException(beginStr);
               }
               row.push_back(val);
            }
            connectMatrix.push_back(row);
         }
         Workflow w(workflows.size() + i+1, pacs,connectMatrix, GetT());
         workflows.push_back(w);
         pacs.clear();
         connectMatrix.clear();
      }

   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void DataInfo::InitResources(string f, bool canExecuteOnDiffResources){
   try{
      processorsCount = 0;
      map <int, vector<pair <int,int>>> busyIntervals;
      char second[21]; // enough to hold all numbers up to 64-bits
      ifstream file(f.c_str(), ifstream::in);
      string errOpen = "File " + f + " was not open";
      string errEarlyEnd = "Unexpected end of file " + f;
      string errWrongFormat = "Wrong format in file " + f + " at line ";
      string errWrongFormatFull = errWrongFormat;
      //string errStageBorders = "InitResources(): stageBorders vector is empty!";
      //if (stageBorders.size()==0) throw UserException(errStageBorders);
      if (file.fail()) 
         throw UserException(errOpen);
      string s, trim; int line = 0;
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      trim = "Resources count = ";
      size_t found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int allResourcesCount = stoi(s);

      trim = "Resources types count = ";
      getline(file,s);
      ++line;
      if (file.eof()) throw UserException(errEarlyEnd);
      found = s.find(trim);
      if (found != 0) {
         sprintf_s(second, "%d", line);
         errWrongFormatFull += second;
         throw UserException(errWrongFormatFull);
      }
      s.erase(0,trim.size());
      int typesCount = stoi(s);
      int resourcesCount = 0, coresCount = 0;

      for (int i = 0; i < typesCount; i++)
      {
         vector <BusyIntervals> typeBI;
         istringstream iss(s);
         getline(file,s);
         ++line;
         if (file.eof()) throw UserException(errEarlyEnd);
         sprintf_s(second, "%d", i+1);
         string first = "Type ";
         trim = first + second;
         found = s.find(trim);
         if (found != 0) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,trim.size()+2);
         iss.str(s);
         iss.clear();
         iss >> resourcesCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         found = s.find(",");
         s.erase(0,found+2);
         iss.str(s);
         iss.clear();
         iss >> coresCount;
         if (iss.fail()) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         double perf = 0.0;
         getline(file,s);
         trim = "Performance (GFlops): ";
         found = s.find(trim);
         if (found != 0) {
            sprintf_s(second, "%d", line);
            errWrongFormatFull += second;
            throw UserException(errWrongFormatFull);
         }
         s.erase(0,trim.size());
         perf = atof(s.c_str());

         for (int j = 0; j < resourcesCount; j++){
            getline(file,s);
            ++line;
            if (file.eof()) throw UserException(errEarlyEnd);
            busyIntervals.clear();
            for (int k = 0; k < coresCount; k++){
               getline(file,s);
               ++line;
               if (file.eof()) throw UserException(errEarlyEnd);
               sprintf_s(second, "%d", k+1);
               first = "Core ";
               trim = first + second;
               found = s.find(trim);
               if (found != 0) {
                  sprintf_s(second, "%d", line);
                  errWrongFormatFull += second;
                  throw UserException(errWrongFormatFull);
               }
               s.erase(0,trim.size()+1);
               int diapCount = stoi(s);
               vector<pair<int,int>> oneResDiaps;
               for (int l = 0; l < diapCount; l++){
                  if (file.eof()) throw UserException(errEarlyEnd);
                  getline(file,s);
                  ++line;
                  iss.str(s);
                  iss.clear();
                  int one,two;
                  iss >> one;
                  if (iss.fail()) {
                     sprintf_s(second, "%d", line);
                     errWrongFormatFull += second;
                     throw UserException(errWrongFormatFull);
                  }
                  iss >> two;
                  if (iss.fail()) {
                     sprintf_s(second, "%d", line);
                     errWrongFormatFull += second;
                     throw UserException(errWrongFormatFull);
                  }
                  oneResDiaps.push_back(make_pair(one,two));
               }
               busyIntervals.insert(make_pair(k+1, oneResDiaps));
            }
            // add busyIntervals for current resource to a vector <BusyIntervals>
            typeBI.push_back(busyIntervals);
         }
         ResourceType r(i+1,resourcesCount, coresCount, perf, typeBI, canExecuteOnDiffResources, context);
         resources.push_back(r);
         processorsCount += coresCount * resourcesCount;
      }

        file.close();
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

void DataInfo::FixBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->FixBusyIntervals();
   }
}

void DataInfo::ResetBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->ResetBusyIntervals();
   }
}

void DataInfo::SetInitBusyIntervals(){
   for (auto i = resources.begin(); i!= resources.end(); i++){
      i->SetInitBusyIntervals();
   }
}

void DataInfo::GetCurrentIntervals(vector<vector<BusyIntervals>> &out){
   out.resize(resources.size());
   for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
      resources[res].GetCurrentIntervals(out[res]);
   }
}

void DataInfo::SetCurrentIntervals(vector<vector<BusyIntervals>> &out){
   out.resize(resources.size());
   for (vector<ResourceType>::size_type res = 0; res < resources.size(); res++){
      resources[res].SetCurrentIntervals(out[res]);
   }
}

int DataInfo::GetResourceType(int number){
   try{
      if (number < 0 || number > processorsCount-1) 
         throw UserException("DataInfo::GetResourceType() error. Wrong coreNumber");
      int current = 0;
      for (vector <ResourceType>::iterator it = resources.begin(); it!= resources.end(); it++){
         int currentCoreCount = it->GetCoresCount();
         if (number >= current && number < current + currentCoreCount) 
            return distance(resources.begin(), it);
         current+=currentCoreCount;
      }
      return -1;
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// get initial core index of resource type
int DataInfo::GetInitResourceTypeIndex(int type){
   int index = 0;
   for (int i = 0; i < type; i++)
      index += resources[i].GetCoresCount();
   return index;
}


// PRE: wfNum >=0 && wfNum < workflows.size()
const Workflow& DataInfo::Workflows(int wfNum) const {
   try{
      if ( wfNum < 0 || static_cast<unsigned>(wfNum) >= workflows.size() )
         throw UserException("DataInfo::Workflows() error. Index out of range");
      return workflows[wfNum]; 
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// PRE: resNum >=0 && resNum < resources.size()
ResourceType& DataInfo::Resources(int resNum)  {
   try {
      if ( resNum < 0 || static_cast<unsigned>(resNum) >= resources.size() )
         throw UserException("DataInfo::Resources() error. Index out of range");
      return resources[resNum]; 
   }
   catch (UserException& e){
      cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}


void DataInfo::InitFinishingTimes(){
   int wfIndex = 0;
   double avgCalcPower = 0.0;
   for (int i = 0; i < resources.size(); i++)
      avgCalcPower += resources[i].GetPerf();
   avgCalcPower /= resources.size();
   // for each workflow
   for (Workflow& wf: workflows){
      wf.SetFinishingTimes(avgCalcPower);
   }
}

// get all packages count
int DataInfo::GetPackagesCount(){
   int count = 0;
   for (int i = 0; i < workflows.size(); i++)
      count += workflows[i].GetPackageCount();
   return count;
}

// set priorities to whole packages (packages numbered from zero according to wf order)
void DataInfo::SetPriorities(){
   // constructing list of pairs (package number, finishing time)
   list <pair<int, double>> priorityList;
   int pNumber = 0;
   for (int i = 0; i < workflows.size(); i++){
      for (int j = 0; j < workflows[i].GetFinishingTimesSize(); j++)
         priorityList.insert(priorityList.end(),make_pair(pNumber++, workflows[i].GetFinishingTime(j)));
   }
   double minTime = std::numeric_limits<double>::infinity();
   int minIndex = 0;

   while (priorities.size() < GetPackagesCount()){
      list<pair<int, double>>::iterator position, dependPosition = priorityList.begin();
      for (list<pair<int, double>>::iterator it = priorityList.begin();
         it!= priorityList.end(); it++){
         if (it->second < minTime){
                  minTime = it->second;
                  minIndex = it->first;
                  position = it;
         }
      }
      int wfIndex, localPackage;
      GetLocalNumbers(minIndex, wfIndex, localPackage);
      vector<int> dependsOn;
      workflows[wfIndex].GetInput(localPackage, dependsOn);
      bool allPrioretized = true;
      for (int i = 0; i < dependsOn.size(); i++){
         dependsOn[i] += (minIndex - localPackage);
                  
         if (find(priorities.begin(), priorities.end(), dependsOn[i]) == priorities.end()){
            allPrioretized = false;
            for (int j = i+1; j < dependsOn.size(); j++)
               dependsOn[j] += (minIndex - localPackage);
            // find last depend position
            for (int i = 0; i < dependsOn.size(); i++){
               auto foundPosition = 
                  find(priorityList.begin(), priorityList.end(),
                  make_pair(dependsOn[i], workflows[wfIndex].GetFinishingTime(localPackage)));

               if (distance(foundPosition, dependPosition))
                  dependPosition = foundPosition;
            }
            priorityList.erase(position);
            priorityList.insert(dependPosition, make_pair(minIndex,minTime));
            break;
         }
      }

      if (allPrioretized) {
         priorities.push_back(minIndex);
         priorityList.erase(position);
      }
      
      
      minTime = std::numeric_limits<double>::infinity();
   }
   // smallest elements will be in the end of vector
   reverse(priorities.begin(), priorities.end());
}

// set different wf priorities
void DataInfo::SetWfPriorities(){
   // constructing list of pairs (package number, finishing time)
   for (int i = 0; i < workflows.size(); i++){
      workflows[i].SetPriorities();
   }
   //SetT(maxDeadline);
   cout << "Max deadline = " << GetT() << endl;

}

// get resource type by global index
int DataInfo::GetResourceTypeIndex(int globalIndex){
   int index = 0; 
   int border = resources[index].GetCoresCount();
   while (1){
      if (globalIndex < border){
         return index;
      }
      index++;
      border += resources[index].GetCoresCount();
   }
}

// getting next package with smallest finishing time
int DataInfo::GetNextPackage(){
   try{
      if (priorities.size() == 0)
         throw UserException("DataInfo::GetNextPackage() error. Priority is empty");
      int next = priorities.back();
      priorities.pop_back();
      return next;
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// get wfNum and local package number for global package number
void DataInfo::GetLocalNumbers (const int & current, int &wfNum, int &localNum){
   int aggregated = 0;
   for (int i = 0; i < workflows.size(); i++){
      if (current >= aggregated && current < aggregated + workflows[i].GetPackageCount()){
         wfNum = i;
         localNum = current - aggregated;
         return;
      }
      aggregated += workflows[i].GetPackageCount();
   }
}

// get global processor index 
int DataInfo::GetGlobalProcessorIndex(int resource, int local){
   int global = 0;
   for (int i = 0; i < resource; i++)
      global += resources[i].GetCoresCount();
   global += local;
   return global;
}

double DataInfo::GetDeadline(){
     double maxDeadline = 0.0; 
     for (int i = 0; i < workflows.size(); i++) {
         double d = workflows[i].GetDeadline() ;
         if (d > maxDeadline) 
             maxDeadline = d; 
     }
     return maxDeadline;
}

// remove some numbers from priorities
void DataInfo::RemoveFromPriorities(const vector<int>& toRemove){
   for (const auto& val : toRemove){
      auto & it = find(priorities.begin(), priorities.end(), val);
      if (it != priorities.end()) priorities.erase(it);
   }
}
