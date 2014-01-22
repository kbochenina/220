#include "StdAfx.h"
#include "Workflow.h"
#include "UserException.h"
#include <string>
#include <iostream>

double Workflow::GetExecTime ( int pNum, int type, int cores) const {
   try{
      if (pNum < 0 || pNum > packages.size()-1) 
         throw UserException("Workflow::GetExecTime() error. Wrong packageNum" + to_string(pNum));
      return packages[pNum].GetExecTime(type, cores);
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// return true if package pNum is init
bool Workflow::IsPackageInit(int pNum) const {
   for (const auto &i : matrix)
      if (i[pNum]!=0) return false;
   return true;
}

// return true if first depends on second
bool Workflow::IsDepends(unsigned one, unsigned two) const {
   try {
      string errorMsg = "Workflow::IsDepends() error. Workflow " + to_string(uid) + ", incorrect package num - ";
      if (one > packages.size()-1) throw errorMsg +  to_string(one);
      if (two > packages.size()-1) throw errorMsg +  to_string(two);
      if (matrix[one][two]==1) return true;
      // if dependency is indirect
      else {
         for (unsigned int i = 0; i < matrix[one].size(); i++){
            if (matrix[one][i]!=0) 
               if (IsDepends(i,two)) return true;
         }
         return false;
      }
   }
   catch(const string msg){
      cout << msg << endl;
      system("pause");
      exit(EXIT_FAILURE);
   }
}

Workflow::~Workflow(void)
{
}

// return vector with packages that depend from pNum
void Workflow::GetOutput(int pNum, vector<int>& out) const{
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetOutput() error. Wrong package number");
      for (int i = 0; i < matrix.size(); i++){
         if (matrix[pNum][i] == 1)
            out.push_back(i);
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}
// return vector with packages from which pNum depends 
void Workflow::GetInput(int pNum, vector<int>& in) const{
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetInput() error. Wrong package number");
      for (int i = 0; i < matrix.size(); i++){
         if (matrix[i][pNum]==1)
            in.push_back(i);
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}

// return true if the package pNum is last
bool Workflow::IsPackageLast(int pNum) const {
   bool isLast = true;
   for (int i = 0; i < matrix.size(); i++){
      if (matrix[pNum][i]==1){
         isLast = false;
         return isLast;
      }
   }
   return isLast;
}

// return average exectime of package pNume
double Workflow::GetAvgExecTime(int pNum) const{
   return packages[pNum].GetAvgExecTime();
}

// return all successors of package pNum
void Workflow::GetSuccessors(const int &pNum, vector<int>&out) const {
   try{
      if (pNum < 0 || pNum > matrix.size()-1)
         throw UserException("Workflow::GetInput() error. Wrong package number");
      out.clear();
      vector <int> successors;
      GetOutput(pNum, successors);
      out = successors;
      while (successors.size() != 0){
         vector<int>tmp = successors;
         for (auto &current: tmp){
            vector<int> currentSuccessors;
            GetOutput(current, currentSuccessors);
            for (const auto& i : currentSuccessors){
               if (find(out.begin(), out.end(), i) == out.end()){
                  out.push_back(i);
                  successors.push_back(i);
               }
            }
            successors.erase(find(successors.begin(), successors.end(), current));
         }
      }
   }
   catch (UserException& e){
      std::cout<<"error : " << e.what() <<endl;
      std::system("pause");
      exit(EXIT_FAILURE);
   }
}
