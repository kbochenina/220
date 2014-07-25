#include "stdafx.h"
#include "Clustered.h"


Clustered::Clustered(DataInfo &d, int uid): SchedulingMethod(d,uid)
{
	int wfCount = data.GetWFCount();
	maxAvgDiffWeight.resize(wfCount);
	maxSumL.resize(wfCount);
	currentCluster = 1;
	//currentF = 0.0;
	//maxWeight = 0.0;
	//minAvgTaskCount = 1.0;
	wfNum = 0;
	clusters.resize(wfCount);
	dep.resize(wfCount);
	// to create new file
	ofstream file;
	file.open("clustersInfo.txt");
	file.close();
}

void Clustered::SetInitClusters(){
	data.SetPriorities();
	// set initial clusters
	for (size_t i = 0; i < data.GetPackagesCount(); i++){
		int globalNum = data.GetNextPackage();
		int wfNum, localNum;
		data.GetLocalNumbers(globalNum, wfNum, localNum);
		double weight = data.Workflows(wfNum).GetAvgExecTime(localNum);
		double startTime = data.Workflows(wfNum).GetStartTime(localNum);
		double finishingTime = data.Workflows(wfNum).GetFinishingTime(localNum);
		if (startTime == finishingTime){
			cout << "Clustered::SetInitClusters(). Start time = finishing time = " << startTime << endl;
			exit(1);
		}
		Cluster newCluster;
		newCluster.Add(globalNum,weight,finishingTime,startTime);
		newCluster.SetLength();
		clusters[wfNum].push_back(newCluster);
	}
	
	ofstream file;
	file.open("clustersInfoBegin.txt");
	file.close();
	for (size_t i = 0; i < clusters.size(); i++){
		file.open("clustersInfoBegin.txt", ios::app);
		for (size_t j = 0; j < clusters[i].size(); j++){
			file << "Workflow " << i+1 << endl;
			clusters[i][j].PrintInfo(file);
		}
		file << endl;
		file.close();
	}

	SetMaxAvgDiffWeight();
	SetMaxSumL();
   // initial clusters dependencies are equal to wf dependencies
   for (int i = 0; i < data.GetWFCount(); i++)
       data.Workflows(i).GetDep(dep[i]);
	//SetMaxWeight();
	//cout << "Max weight : " << maxWeight << endl;
	//double initWeight = GetAvgDiffWeight();
	/*cout << "AvgDiffWeight: " << initWeight << endl;
	cout << "CurrentF: " << currentF << endl;*/
	//cout << "MaximumWeight: " << GetMaximumWeight() << endl;
	//double initSumL = GetSumL();
	//cout << "SumL: " << GetSumL() << endl;
	//currentF = GetF();
}

void Clustered::ClusterizeConsequence(){
	ofstream file;
   ofstream f("currentFinfo.txt");
	bool wasMerged = true;
   currentADW = 0.0;
//    double t = clock();
	double currentF = GetF(currentADW);
  // cout << "Time of get first F: " << (clock()-t)/1000.0 << endl;
	// while there was at least one merge during pass through the loop
	while (wasMerged){
		//cout << "Workflow # " << wfNum <<"\nClusters: " << clusters[wfNum].size() << endl;
     // f << currentF << endl;
		wasMerged = false;
		currentCluster = 1;
		for (size_t i = currentCluster; i < clusters[wfNum].size()-1; i++){
         double adw1, adw2;
        // double t = clock();
			double f1 = GetF(currentCluster - 1, adw1);
        // cout << "Time of getF: " << (clock()-t)/1000.0 << endl;
			double f2 = GetF(currentCluster + 1, adw2);
			if (currentF < f1 && currentF < f2){
				currentCluster++;
			}
			// if cluster should be merged with previous cluster
			else if (f1 <= f2){
				Merge(currentCluster - 1, true);
				//cout << 1 << endl;
				currentF = f1;
            currentADW = adw1;
				//cout << "CurrentF: " << f1 << " " << GetAvgDiffWeight()/maxAvgDiffWeight  
				//	<< " " << GetSumL()/maxSumL  << endl;
				//cout << "CurrentF: " << f1 << " " << GetMaximumWeight()/maxWeight << " " << GetSumL()/maxSumL << endl;
				wasMerged = true;
				
				/*file.open("clustersDebugInfo.txt");
				file.close();
				for (size_t i = 0; i < clusters[wfNum].size(); i++){
					file.open("clustersDebugInfo.txt", ios::app);
					clusters[wfNum][i].PrintInfo(file);
					file << endl;
					file.close();
				}*/
				currentCluster++;
			}
			else {
				//cout << "Before:" << GetAvgDiffWeight(currentCluster + 1) << endl;
				Merge(currentCluster + 1, false);
				//cout << 2 << endl;
				currentF = f2;
            currentADW = adw2;
				//cout << "CurrentF: " << f2 << " " << GetAvgDiffWeight()/maxAvgDiffWeight  
				//	<< " " << GetSumL()/maxSumL  << endl;
				//cout << "CurrentF: " << f2 << " " << GetMaximumWeight()/maxWeight << " " << GetSumL()/maxSumL << endl;
				wasMerged = true;
				
				/*file.open("clustersDebugInfo.txt");
				file.close();
				for (size_t i = 0; i < clusters[wfNum].size(); i++){
					file.open("clustersDebugInfo.txt", ios::app);
					clusters[wfNum][i].PrintInfo(file);
					file << endl;
					file.close();
				}*/
				currentCluster++;
			}
		}
	}

	
	for (size_t i = 0; i < clusters[wfNum].size(); i++){
		file.open("clustersInfo.txt", ios::app);
		file << "Workflow " << wfNum+1 << endl;
		clusters[wfNum][i].PrintInfo(file);
		file << endl;
		file.close();
	}
   f.close();

}

void Clustered::SetClusterDep(){
	vector <vector<int>> &matrix = dep[wfNum];
	matrix.resize(clusters[wfNum].size());
	for (auto &ind : matrix)
		ind.resize(clusters[wfNum].size());
	for (size_t i = 0; i < clusters[wfNum].size(); i++){
		for (size_t j = 0; j < clusters[wfNum].size(); j++){
			if (i == j) {
				matrix[i][j] = 0;
				continue;
			}
			bool isDepends = false;
			for (size_t firstPIndex = 0; firstPIndex < clusters[wfNum][i].GetSize(); firstPIndex++){
				for (size_t secondPIndex = 0; secondPIndex < clusters[wfNum][j].GetSize(); secondPIndex++){
					int firstPNum = clusters[wfNum][i].GetPNum(firstPIndex),
						secondPNum = clusters[wfNum][j].GetPNum(secondPIndex);
					int initNum = data.GetInitPackageNumber(wfNum);
					firstPNum -= initNum;
					secondPNum -= initNum;
					if (data.Workflows(wfNum).IsDepends(firstPNum, secondPNum)){
						isDepends = true;
						break;
					}
				}
				if (isDepends)
					break;
			}
			if (isDepends) {
				matrix[i][j] = 1;
			}
		}
	}

	/*for (auto& row : matrix){
		for (auto& col : row)
			cout << col << " ";
		cout << endl;
	}
	cout << endl;*/
	//dep[wfNum] = matrix;
}


// should be called before merging any clusters
void Clustered::SetMaxAvgDiffWeight(){
	for (size_t i = 0; i < clusters.size(); i++){
		double current = 0.0;
		for (size_t j = 0; j < clusters[i].size(); j++)
			current += clusters[i][j].GetWeight();
		current /= 2;
		maxAvgDiffWeight[i] = current;
	}
	//cout << "MaxAvgDiffWeight " << maxAvgDiffWeight << endl;
}



double Clustered::GetWFSchedule(Schedule &out){
	double res = 0.0;
	Greedy alg(data,-1,-1);
	SetInitClusters();
	for (size_t i = 0; i < clusters.size(); i++){
		wfNum = i;
      double t = clock();
		ClusterizeConsequence();
     // cout << "t = " << t << " clock()=" << clock() << endl;
     // cout << "Time of clusterization: " << (clock()-t)/1000.0 << endl;
		SetClusterDep();
     // cout << "Time of setting clusters dependency: " << (clock()-t)/1000.0 << endl;
	}

  

	int clustersCount = 0;
	for (auto &wfDep : dep)
		clustersCount += wfDep.size();
	int schedClustersCount = 0;
	int wfCount = data.GetWFCount();

	// find the cluster to schedule
	vector<vector<int>> unschedClusters;
	unschedClusters.resize(wfCount);
	for (int i = 0; i < wfCount; i++)
		for (size_t j = 0; j < dep[i].size(); j++)
			unschedClusters[i].push_back(j);
		
   for (int i = 0; i < dep[0].size(); i++){
       for (int j = 0; j < dep[0].size(); j++)
           cout << dep[0][i][j] << " " ;
       cout << endl;
   }

	vector<vector<int>> variants;
	variants.resize(wfCount);

	vector <double> wfMetric;
	for (size_t wf = 0; wf < clusters.size(); wf++){
		double weight = 0.0;
		double lastEnd = 0.0;
		double scheduled = 0;
		for (size_t cluster = 0; cluster < clusters[wf].size(); cluster++){
			auto it = find(unschedClusters[wf].begin(), unschedClusters[wf].end(), cluster);
			// if current cluster is already scheduled
			if (it == unschedClusters[wf].end()){
				double currDeadline = clusters[wf][cluster].GetDeadline();
				if (currDeadline > lastEnd)
					lastEnd = currDeadline;
				scheduled += clusters[wf][cluster].GetSize();
			}
			else {
				weight += clusters[wf][cluster].GetWeight();

			}
		}
		double percentScheduled = scheduled / data.Workflows(wf).GetPackageCount();
		//wfMetric.push_back(data.Workflows(wf).GetDeadline() - (lastEnd + weight));
		wfMetric.push_back(data.Workflows(wf).GetDeadline() - lastEnd);//weight);// + percentScheduled );
	}

	vector<int> wfToAddClusters;
	for (int i = 0; i < wfCount; i++)
		wfToAddClusters.push_back(i);
	
	double minDeadline = numeric_limits<double>::max();
	double maxWeightLength = 0;
	
   //double t = clock();
  // cout << "t = " << t << endl;

	while (schedClustersCount != clustersCount){
		int clustersSetSize = 0;
		
		// fill the variants vector
		for (auto& wf: wfToAddClusters){
			vector<vector<int>>& wfDep = dep[wf];

			for (size_t i = 0; i < wfDep.size(); i++){
				bool isVariant = true;
				for (size_t j = 0; j < wfDep.size(); j++){
					if (wfDep[j][i] == 1){
						isVariant = false;
						break;
					}
				}
				if (isVariant && 
					find(unschedClusters[wf].begin(), unschedClusters[wf].end(), i) != unschedClusters[wf].end() &&
					find(variants[wf].begin(), variants[wf].end(), i) == variants[wf].end()){
					variants[wf].push_back(i);
					//cout << "Cluster " << i << " was added to " << wf << "WF." << endl;
					//if (minDeadline > clusters[wf][i].GetDeadline())
					//	minDeadline = clusters[wf][i].GetDeadline();
					//double weightLength = clusters[wf][i].GetWeight() / clusters[wf][i].GetLength();
					////cout << "weightLength = " << weightLength << endl;
					//if (weightLength > maxWeightLength)
					//	maxWeightLength = weightLength;
					clustersSetSize++;
				}
			}
		}


	
		// scheduling variants set
		int schedSetCount = 0;
		// while all clusters from the set aren't scheduled
		//while (schedSetCount != clustersSetSize){
		//double prevMinDeadline = numeric_limits<double>::max();
		//double prevMaxWeightLength = 0;
		double bestMetric = numeric_limits<double>::max();
		int bestWf = 0, bestCluster = 0, bestClusterIndex = 0;
		for (size_t wf = 0; wf < wfMetric.size(); wf++){
			if (wfMetric[wf] < bestMetric){
				bestMetric = wfMetric[wf];
				bestWf = wf;
			}
		}
		//cout << "Best metric: " << bestMetric << " wf " << bestWf << endl;
		//system("pause");
		maxWeightLength = 0;
		minDeadline = numeric_limits<double>::max();
		double bestClusterMetric = 0.0;
		//for (int i = 0; i < wfCount; i++){
			for (size_t j = 0; j < variants[bestWf].size(); j++){
				int clusterNum = variants[bestWf][j];
				double currWeightLength = clusters[bestWf][clusterNum].GetWeight() / clusters[bestWf][clusterNum].GetLength();
				double currDeadline = clusters[bestWf][clusterNum].GetDeadline();
				if (currWeightLength > maxWeightLength)
					maxWeightLength = currWeightLength;
				if (currDeadline < minDeadline)
					minDeadline = currDeadline;
			}
			for (size_t j = 0; j < variants[bestWf].size(); j++){
				int clusterNum = variants[bestWf][j];
				double currWeightLength = clusters[bestWf][clusterNum].GetWeight() / clusters[bestWf][clusterNum].GetLength();
				double metric =  //data.GetDeadline() / clusters[i][clusterNum].GetDeadline(); //+
					0.5 * minDeadline / clusters[bestWf][clusterNum].GetDeadline()  + 
					0.5 * currWeightLength / maxWeightLength;
				//cout << metric << endl;
				/*cout << clusters[i][clusterNum].GetDeadline() << " " <<
					minDeadline / clusters[i][clusterNum].GetDeadline() << " " <<
						" " << clusters[i][clusterNum].GetWeight() <<  " " << clusters[i][clusterNum].GetLength() << " " 
						<< clusters[i][clusterNum].GetWeight()  / clusters[i][clusterNum].GetLength() / maxWeightLength<< " ";*/
				//cout << metric << endl;
				if (metric > bestClusterMetric){
					bestWf = bestWf;
					bestClusterIndex = j;
					bestCluster = clusterNum;
					bestClusterMetric = metric;
				}
			
			}
		//system("pause");
		//cout << "Best metric " << bestMetric << endl;
		currentCluster = bestCluster;
		wfNum = bestWf;
		int realBegin, realEnd;
		res += GetClusterSchedule(out, realBegin, realEnd);


	   cout << "Cluster " << bestCluster << " of " << bestWf << " was scheduled." << endl;  
		/*cout << "Tstart: " << clusters[bestWf][bestCluster].GetStart() ;
		cout << " Deadline: " << clusters[bestWf][bestCluster].GetDeadline() ;
		cout << " Weight: " << clusters[bestWf][bestCluster].GetWeight() << endl;
		cout << "Real begin: " << realBegin << " realEnd " << realEnd << endl;
		system("pause");*/
		/*for (int i = 0; i < unschedClusters.size(); i++){
			cout << "Unsched clusters for WF" << i << endl;
			for (int j = 0; j < unschedClusters[i].size(); j++)
				cout << unschedClusters[i][j] << " ";
			cout << endl;
		}*/
		schedClustersCount++;
		schedSetCount++;
		//cout << "Cluster " << variants[bestWf][bestClusterIndex] << " was deleted from " << bestWf <<"WF"<<endl;
		variants[bestWf].erase(variants[bestWf].begin() + bestClusterIndex);
		
		auto it = find(unschedClusters[bestWf].begin(), unschedClusters[bestWf].end(), bestCluster);
		if (it == unschedClusters[bestWf].end())
			cout << "error" << endl;
		unschedClusters[wfNum].erase(it);
		wfToAddClusters.clear();
		wfToAddClusters.push_back(bestWf);

		double bestWfWeight = 0.0,
			bestWfLastEnd = 0;
		double scheduled = 0;
		for (int i = 0; i < clusters[bestWf].size(); i++){
			auto it = find(unschedClusters[bestWf].begin(), unschedClusters[bestWf].end(), i);
			if (it != unschedClusters[bestWf].end()){
				bestWfWeight += clusters[bestWf][i].GetWeight();
				
			}
			else {
				if (clusters[bestWf][i].GetDeadline() > bestWfLastEnd)
					bestWfLastEnd = clusters[bestWf][i].GetDeadline();
				scheduled += clusters[bestWf][i].GetSize();
			}
		}
		double percentScheduled = scheduled / data.Workflows(bestWf).GetPackageCount();
		//cout << "Best wf weight: " << bestWfWeight << endl;
		//cout << "Best wf lastEnd: " << bestWfLastEnd << endl;
		//cout << "Best wf lastEnd: " << percentScheduled << endl;
	    if (unschedClusters[bestWf].size() == 0)
			wfMetric[bestWf] = numeric_limits<double>::max();
		else 
			wfMetric[bestWf] = (data.Workflows(bestWf).GetDeadline() - bestWfLastEnd);// / bestWfWeight;// + percentScheduled;
		
		//minDeadline = prevMinDeadline;
		//maxWeightLength = prevMaxWeightLength;

		for (size_t i = 0; i < dep[bestWf].size(); i++)
			dep[bestWf][bestCluster][i] = 0;
			
		
	}



   //cout << "t = " << t << " clock()=" << clock() << endl;
   //cout << "Time of scheduling: " << (clock()-t)/1000.0 << endl;
	return res;
}


double Clustered::GetClusterSchedule(Schedule &out, int&realBegin, int&realEnd){
	double res = 0.0;
	Greedy alg(data,-1,-1);
	realBegin = data.GetT(); 
	realEnd = 0;
	// for each packege in cluster
	//for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++){
	int clusterSize = clusters[wfNum][currentCluster].GetSize();
	int attempted = 0;
	while (clusterSize != attempted){
		int package = clusters[wfNum][currentCluster].GetPNum(attempted);
		double eff = 0.0;
		int sizeBefore = out.size();
		//cout << "wfNum " << wfNum << " package " << package << endl;
		alg.FindSchedule(out, eff, package, false);
		if (out.size() > sizeBefore){
			PackageSchedule &cP = out[out.size()-1];
			int currBegin = cP.get<1>();
			int currEnd = cP.get<3>() + currBegin;
			if (currBegin < realBegin) realBegin = currBegin;
			if (currEnd > realEnd) realEnd = currEnd;
		}
		attempted++;
		/*if (currentCluster == 5 && sizeBefore == out.size())
			cout << "Cannot find placement for package " << clusters[currentCluster].GetPNum(i) << endl;*/
		res += eff;
	}
	double add = realEnd - clusters[wfNum][currentCluster].GetDeadline();
	for (int i = 0; i < clusters[wfNum].size(); i++){
			if (dep[wfNum][currentCluster][i] == 1){
				double currStart = clusters[wfNum][i].GetStart(),
					currDeadline = clusters[wfNum][i].GetDeadline();
				clusters[wfNum][i].SetStart(currStart + add);
				clusters[wfNum][i].SetDeadline(currDeadline + add);
				//cout << "Cluster " << i << " of " << wfNum << " has changed: start = " <<
				//	currStart + add << " , deadline = " << currDeadline + add << endl;
			}
		
	}
	return res;
}



//void Clustered::SetMaxWeight(){
//	for (size_t i = 0; i < clusters.size(); i++)
//		maxWeight += clusters[i].GetWeight();
//}

void Clustered::SetMaxSumL(){
	for (size_t i = 0; i < clusters.size(); i++){
		double current = 0.0;
		for (size_t j = 0; j < clusters[i].size(); j++)
			current += clusters[i][j].GetLength();
		maxSumL[i] = current;
		//cout << clusters[i].GetStart() << " " << clusters[i].GetDeadline() << " " << clusters[i].GetLength() << endl;
	}
	
}

double Clustered::GetF(double&avgDiffWeight){
	double res = 0.0;
	avgDiffWeight = GetAvgDiffWeight();
	double sumL = GetSumL();
	//res = maxW/maxWeight + sumL/maxSumL;
	res = avgDiffWeight/maxAvgDiffWeight[wfNum]  + sumL/maxSumL[wfNum];
	//res = avgDiffWeight/maxAvgDiffWeight ;
	//res = sumL/maxSumL;
	return res;
}

double Clustered::GetF(int second, double& avgDiffWeight){
	double res = 0.0;
   avgDiffWeight = GetAvgDiffWeight(second, avgDiffWeight); // GetAvgDiffWeight(second)
  	//cout << "GetF() weight " << avgDiffWeight << " ";
	//double maxW = GetMaximumWeight(second);
	double sumL = GetSumL(second);
   
	//res = maxW/maxWeight + sumL/maxSumL;
	//cout << "GetF() sum " << sumL << endl;
	res = avgDiffWeight/maxAvgDiffWeight[wfNum]  + sumL/maxSumL[wfNum] ;
	//res = avgDiffWeight/maxAvgDiffWeight;
	//res = sumL/maxSumL;
	return res;
}

/*
double Clustered::GetMaximumWeight(){
	double res = 0.0;
	for (size_t i = 0; i < clusters.size(); i++){
		if (clusters[i].GetWeight() > res)
			res = clusters[i].GetWeight();
	}
	return res;
}

double Clustered::GetMaximumWeight(int second){
	double res = 0.0;
	for (size_t i = 0; i < clusters.size(); i++){
		if (i == currentCluster || i == second ){
			double weight = clusters[currentCluster].GetWeight() + clusters[second].GetWeight();
			if (weight > res) res = weight;
			++i; // to skip next cluster
		}
		else if (clusters[i].GetWeight() > res)
			 res = clusters[i].GetWeight();
	}
	return res;
}
*/
double Clustered::GetAvgDiffWeight(){
	ofstream debug;
//	debug.open("debug2.txt");
	int currNum = 0;
	int iterations = 0;
	double sum = 0.0;
//	cout << "Check: " << clusters.size() << endl;
	while (currNum <= clusters[wfNum].size() - 2){
//		debug << "currNum: " << currNum << endl;
		double currWeight = clusters[wfNum][currNum].GetWeight();
//		debug << "currWeight: " << currWeight << endl;
		for (size_t i = currNum + 1; i < clusters[wfNum].size(); i++){
			double anotherWeight = clusters[wfNum][i].GetWeight();
//			debug << "anotherWeight: " << anotherWeight << endl;
			double diff = abs(currWeight - anotherWeight);
			sum += diff;
			++iterations;
		}
		currNum++;
	}
//	debug.close();
   //cout << "Iterations for simple " << iterations << endl;
	return sum/iterations;
}

double Clustered::GetSumL(){
	double sumL = 0.0;
	for (size_t i = 0; i < clusters[wfNum].size(); i++)
		sumL += clusters[wfNum][i].GetLength();
	return sumL;
}


double Clustered::GetAvgDiffWeight(int second, double adw){
    int size = clusters[wfNum].size();
    double sum = currentADW * size * (size-1)/2;
    double currWeight = clusters[wfNum][currentCluster].GetWeight(), secondWeight = clusters[wfNum][second].GetWeight();
    double mergeWeight = currWeight + secondWeight;
    int iterations = 0;
    for (int i = 0; i < size; i++){
        if ( i!= currentCluster) {
            sum -= abs(currWeight - clusters[wfNum][i].GetWeight());
         }
        if ( i!= second) {
            sum -= abs(secondWeight - clusters[wfNum][i].GetWeight());
        }
        if ( i!= currentCluster && i != second )
            sum += abs(mergeWeight - clusters[wfNum][i].GetWeight());
        iterations++;
    }
    sum += abs(currWeight - secondWeight);
    sum /= (size - 1) * (size - 2) / 2;
    //cout << "Iterations for hard " << iterations << endl;
    return sum;
}

double Clustered::GetAvgDiffWeight(int second){
//	ofstream debug;
//	debug.open("debug.txt");
	unsigned int currNum = 0;
	int iterations = 0;
	double sum = 0.0;
	bool wasMergedClusterViewed = false;
	//cout << "Size: "<< clusters.size() << endl;
	while (currNum <= clusters[wfNum].size() - 2){
		if (currNum == second || currNum == currentCluster){
			if (!wasMergedClusterViewed){
//				debug << "currNum: " << currNum << endl;
				double currWeight = clusters[wfNum][second].GetWeight() + clusters[wfNum][currentCluster].GetWeight();
//				debug << "currWeight: " << currWeight << endl;
					for (size_t i = currNum + 2; i < clusters[wfNum].size(); i++){
					double anotherWeight = clusters[wfNum][i].GetWeight();
//					debug << "anotherWeight: " << anotherWeight << endl;
					double diff = abs(currWeight - anotherWeight);
					sum += diff;
					++iterations;
					wasMergedClusterViewed = true;
				}
				// to skip next cluster
				currNum++;
			}
		}
		else {
//			debug << "currNum: " << currNum << endl;
			double currWeight = clusters[wfNum][currNum].GetWeight();
//			debug << "currWeight: " << currWeight << endl;
			for (size_t i = currNum + 1; i < clusters[wfNum].size(); i++){
				double anotherWeight;
				if (i == second || i == currentCluster){
					anotherWeight = clusters[wfNum][second].GetWeight() + clusters[wfNum][currentCluster].GetWeight();
					// to skip next cluster
					i++;
				}
				else anotherWeight = clusters[wfNum][i].GetWeight();
//				debug << "anotherWeight: " << anotherWeight << endl;
				double diff = abs(currWeight - anotherWeight);
				sum += diff;
				++iterations;
			}
		}
		currNum++;
		//cout << "currNum " << currNum << endl;
	}
//	debug.close();
	return sum/iterations;
}

double Clustered::GetSumL(int second){
	double res = GetSumL() - clusters[wfNum][second].GetLength() - clusters[wfNum][currentCluster].GetLength();
	double startCurr = clusters[wfNum][currentCluster].GetStart(),
		deadlineCurr = clusters[wfNum][currentCluster].GetDeadline(),
		startSecond = clusters[wfNum][second].GetStart(),
		deadlineSecond = clusters[wfNum][second].GetDeadline();
	double start = startCurr < startSecond ? startCurr : startSecond,
		d = deadlineCurr > deadlineSecond ? deadlineCurr : deadlineSecond;
	res += d - start;
	return res;
}



void Clustered::Merge(int second, bool isPrev){
	clusterInfo secondInfo;
   int toDelete = 0;
	if (!isPrev){
		clusters[wfNum][second].GetInfo(secondInfo);
		// add packages to current cluster
		for (size_t i = 0; i < secondInfo.get_head().size(); i++ ) {
			int pNum = secondInfo.get<0>()[i];
			double pStart = secondInfo.get<1>()[i];
			double pDeadline = secondInfo.get<2>()[i];
			double pWeight = secondInfo.get<3>()[i];
			clusters[wfNum][currentCluster].Add(pNum, pWeight, pDeadline, pStart);
		}
		// delete another cluster
		clusters[wfNum].erase(clusters[wfNum].begin()+second);
      toDelete = second;
	}
	else {
		clusters[wfNum][currentCluster].GetInfo(secondInfo);
		// add packages to current cluster
		for (size_t i = 0; i < secondInfo.get_head().size(); i++ ) {
			int pNum = secondInfo.get<0>()[i];
			double pStart = secondInfo.get<1>()[i];
			double pDeadline = secondInfo.get<2>()[i];
			double pWeight = secondInfo.get<3>()[i];
			clusters[wfNum][second].Add(pNum, pWeight, pDeadline, pStart);
		}
		// delete another cluster
		clusters[wfNum].erase(clusters[wfNum].begin()+currentCluster);
      toDelete = currentCluster;
	}
   // rearrange dependecies
   // delete row
   dep[wfNum].erase(dep[wfNum].begin()+toDelete);
   for (auto& row: dep[wfNum])
       row.erase(row.begin()+toDelete);

	if (clusters[wfNum][currentCluster].GetStart() == clusters[wfNum][currentCluster].GetDeadline()){
		cout << "GetStart()==Deadline()" << endl; system("pause");
	}
}

Clustered::~Clustered(void)
{
}
