#include "stdafx.h"
#include "Metrics.h"

void Metrics::GetMetrics(Schedule & s, string name){
   sched = s;
   full.open("fullmetrics.txt", ios::app);
   full << name.c_str() << endl;
   AvgUnfinischedTime();
   AvgReservedTime();
   AvgCCR();
   full << "***************************************************"  << endl;
   full.close();
   out.close();
}

void Metrics::AvgCCR(){

}

void Metrics::AvgUnfinischedTime(){
   // check
   int wfCount = data.GetWFCount();
   vector <double> unfinishedTimes(wfCount,0);
   vector <int> unfinishedTasks(wfCount,0);
   vector <int> unscheduledTasks(wfCount,0);
   vector <vector<int>> scheduledTasks;
   vector <double> execTimePerWf(wfCount,0);
   vector <double> commTimePerWf(wfCount,0);
   vector <vector<int>> resTypesPerWfPackages;
   resTypesPerWfPackages.resize(wfCount);
   for (int i = 0; i < wfCount; i++)
       resTypesPerWfPackages[i].resize(data.Workflows(i).GetPackageCount());
   scheduledTasks.resize(wfCount);
   double summUnfinishedTime = 0;
   int summUnfinishedTasks = 0;
   // for each p
   for (size_t i = 0; i < sched.size(); i++){
      int pNum = sched[i].get<0>();
      int tBegin = sched[i].get<1>();
      double execTime = sched[i].get<3>();
      int processor = sched[i].get<2>()[0];
      int type = data.GetResourceTypeIndex(processor) ; // !
      int wfNum, localNum;
      data.GetLocalNumbers(pNum, wfNum, localNum);
      resTypesPerWfPackages[wfNum][localNum] = type;
      scheduledTasks[wfNum].push_back(localNum);
      execTimePerWf[wfNum] += execTime;
      vector <int> in;
      data.Workflows(wfNum).GetInput(localNum, in);
      for (size_t j = 0; j < in.size(); j++){
          double amount = data.Workflows(wfNum).GetTransfer(in[j], localNum);
          double commRate = data.GetBandwidth(resTypesPerWfPackages[wfNum][in[j]], type);
          if (commRate) 
              commTimePerWf[wfNum] += amount/commRate;
      }
      // DEADLINES FEATURE
      if ( tBegin + execTime > data.GetDeadline(wfNum) ){
         unfinishedTimes[wfNum] += tBegin + execTime - data.GetDeadline(wfNum);
         reservedTime[wfNum] = 0;
         unfinishedTasks[wfNum]++;
         summUnfinishedTasks++;
      }
   }
   //cout << "Average unfinished times and tasks" << endl << "***************************************************"  << endl;
   //cout << "Unfinished times: " << endl;
   vector<int> violatedDeadlines;

   for (size_t i = 0; i < unfinishedTimes.size(); i++){
      // if we have some unscheduled tasks
      if (scheduledTasks[i].size() != data.Workflows(i).GetPackageCount()){
         violatedDeadlines.push_back(i);
         for (int j = 0; j < data.Workflows(i).GetPackageCount(); j++ ){
            // if package was not scheduled
            if (find(scheduledTasks[i].begin(), scheduledTasks[i].end(), j) == scheduledTasks[i].end()){
               unfinishedTimes[i] += data.Workflows(i).GetAvgExecTime(j);
               unscheduledTasks[i]++;
               reservedTime[i] = 0.0;
            }
         }
      }

      /*cout << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
      << " unscheduled tasks: " << unscheduledTasks[i] 
      << " scheduled tasks: " << scheduledTasks[i].size() << endl;*/
      summUnfinishedTime += unfinishedTimes[i];
   }
   /*cout << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   cout << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;*/
   out << "Deadlines violated: " << violatedDeadlines.size() << endl;
   out << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   cout << "Deadlines violated: " << violatedDeadlines.size() << endl;
   cout << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   full << "Deadlines violated: " << violatedDeadlines.size() << endl;
   full << "Average deadlines violated: " << static_cast<double>(violatedDeadlines.size())/data.GetWFCount() << endl;
   out << "Average unfinished times and tasks" << endl << "***************************************************" << endl;
   out << "Unfinished times: " << endl;
   for (size_t i = 0; i < unfinishedTimes.size(); i++){
      out << "Workflow # " << i+1 << " " << unfinishedTimes[i] << " " << "unfinished tasks: " << unfinishedTasks[i] 
      << " unscheduled tasks: " << unscheduledTasks[i] 
      << " scheduled tasks: " << scheduledTasks[i].size() << endl;
   }
   out << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   out << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;

   cout << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   cout << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;
   full << "Avg unfinished time: " << summUnfinishedTime/data.GetWFCount() << endl;
   full << "Avg unfinished tasks: " << static_cast<double>(summUnfinishedTasks)/static_cast<double>(data.GetWFCount()) << endl;
   int sumUnsched = 0;
   for (size_t i = 0; i < unscheduledTasks.size(); i++)
      sumUnsched += unscheduledTasks[i];

   out << "Unscheduled tasks count: " << sumUnsched << endl;
   out << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;
   cout << "Unscheduled tasks count: " << sumUnsched << endl;
   cout << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;
   full << "Unscheduled tasks count: " << sumUnsched << endl;
   full << "Percent of unscheduled tasks: " << static_cast<double>(sumUnsched)/data.GetPackagesCount() << endl;

   double avgCCR = 0.0;

   for (size_t i = 0; i < wfCount; i++){
       double currCCR = commTimePerWf[i]/execTimePerWf[i];
       avgCCR += currCCR;
       
       /*out << "Workflow " << i + 1 << endl;
       out << "Execution time:" << execTimePerWf[i] << endl;
       out << "Communication time:" << commTimePerWf[i] << endl;
       out << "CCR: " << currCCR << endl;
      
       full << "Workflow " << i + 1 << endl;
       full << "Execution time:" << execTimePerWf[i] << endl;
       full << "Communication time:" << commTimePerWf[i] << endl;
       full << "CCR: " << currCCR << endl;*/
   }

   avgCCR /= wfCount;
   out << "Average CCR: " << avgCCR << endl;
   full << "Average CCR: " << avgCCR << endl;
}

void Metrics::AvgReservedTime(){
   double summReserved = 0.0;
   for (size_t i = 0; i < sched.size(); i++){
      int pNum = sched[i].get<0>();
      int tBegin = sched[i].get<1>();
      double execTime = sched[i].get<3>();
      int wfNum, localNum;
      data.GetLocalNumbers(pNum, wfNum, localNum);
      if (reservedTime[wfNum]!=0){
         // if this is the last package of WF
         if (data.Workflows(wfNum).IsPackageLast(localNum)){
            // get its reserved time
            double packageReservedTime = data.GetDeadline(wfNum) - (tBegin + execTime);
            // if this time is positive
            if ( packageReservedTime > 0 ){
               // if it is first positive reserve, save it
               if (reservedTime[wfNum] == numeric_limits<double>::infinity())
                  reservedTime[wfNum] = packageReservedTime;
               // otherwise if saved reserve > than new reserve
               // we should save new reserve
               else if (packageReservedTime < reservedTime[wfNum])
                  reservedTime[wfNum] = packageReservedTime;
            }
            else reservedTime[wfNum] = 0;
         }
      }
   }
   //cout << "Reserved times" << endl << "***************************************************"  << endl;
   for (size_t i = 0; i < reservedTime.size(); i++){
      //cout << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
      summReserved += reservedTime[i];
   }
   //cout << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;

   out << "Reserved times" << endl << "***************************************************"  << endl;
   for (size_t i = 0; i < reservedTime.size(); i++){
      out << "Workflow # " << i+1 << " " << reservedTime[i] << endl;
   }
   out << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   cout << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   full << "Avg reserved time: " << static_cast<double>(summReserved)/data.GetWFCount() << endl;
   out << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
   cout << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
   full << "Avg reserved time (part of T): " << static_cast<double>(summReserved)/data.GetWFCount()/data.GetT() << endl;
}

Metrics::~Metrics(void)
{
}
