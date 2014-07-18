#include "stdafx.h"
#include "Clustered.h"


Clustered::Clustered(DataInfo &d, int uid): SchedulingMethod(d,uid)
{
	maxAvgDiffWeight = 0;
	maxSumL = 0;
	currentCluster = 1;
	currentF = 0.0;
	maxWeight = 0.0;
	minAvgTaskCount = 1.0;
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
		Cluster newCluster;
		newCluster.Add(globalNum,weight,finishingTime,startTime);
		newCluster.SetLength();
		clusters.push_back(newCluster);
	}
	
	ofstream file;
	file.open("clustersInfoBegin.txt");
	file.close();
	for (size_t i = 0; i < clusters.size(); i++){
		file.open("clustersInfoBegin.txt", ios::app);
		clusters[i].PrintInfo(file);
		file << endl;
		file.close();
	}

	SetMaxAvgDiffWeight();
	SetMaxWeight();
	//cout << "Max weight : " << maxWeight << endl;
	SetMaxSumL();
	
	double initWeight = GetAvgDiffWeight();
	/*cout << "AvgDiffWeight: " << initWeight << endl;
	cout << "CurrentF: " << currentF << endl;*/
	cout << "MaximumWeight: " << GetMaximumWeight() << endl;
	double initSumL = GetSumL();
	//cout << "SumL: " << GetSumL() << endl;
	currentF = GetF();
}

void Clustered::ClusterizeConsequence(){
	ofstream file;
	bool wasMerged = true;
	// while there was at least one merge during pass through the loop
	while (wasMerged){
		cout << "clusters: " << clusters.size() << endl;
		wasMerged = false;
		currentCluster = 1;
		for (size_t i = currentCluster; i < clusters.size()-1; i++){
			double f1 = GetF(currentCluster - 1);
			double f2 = GetF(currentCluster + 1);
			if (currentF < f1 && currentF < f2){
				currentCluster++;
			}
			// if cluster should be merged with previous cluster
			else if (f1 <= f2){
				Merge(currentCluster - 1, true);
				currentF = f1;
				//cout << "CurrentF: " << f1 << " " << GetAvgDiffWeight()/maxAvgDiffWeight  
				//	<< " " << GetSumL()/maxSumL  << endl;
				//cout << "CurrentF: " << f1 << " " << GetMaximumWeight()/maxWeight << " " << GetSumL()/maxSumL << endl;
				wasMerged = true;
				
				file.open("clustersDebugInfo.txt");
				file.close();
				for (size_t i = 0; i < clusters.size(); i++){
					file.open("clustersDebugInfo.txt", ios::app);
					clusters[i].PrintInfo(file);
					file << endl;
					file.close();
				}
				currentCluster++;
			}
			else {
				//cout << "Before:" << GetAvgDiffWeight(currentCluster + 1) << endl;
				Merge(currentCluster + 1, false);
				currentF = f2;
				//cout << "CurrentF: " << f2 << " " << GetAvgDiffWeight()/maxAvgDiffWeight  
				//	<< " " << GetSumL()/maxSumL  << endl;
				//cout << "CurrentF: " << f2 << " " << GetMaximumWeight()/maxWeight << " " << GetSumL()/maxSumL << endl;
				wasMerged = true;
				
				file.open("clustersDebugInfo.txt");
				file.close();
				for (size_t i = 0; i < clusters.size(); i++){
					file.open("clustersDebugInfo.txt", ios::app);
					clusters[i].PrintInfo(file);
					file << endl;
					file.close();
				}
				currentCluster++;
			}
		}
	}

	file.open("clustersInfo.txt");
	file.close();
	for (size_t i = 0; i < clusters.size(); i++){
		file.open("clustersInfo.txt", ios::app);
		clusters[i].PrintInfo(file);
		file << endl;
		file.close();
	}
	//cout << "Resulting f: " << currentF << endl;
	//cout << "Number of clusters: " << clusters.size() << endl;
	//cout << "MaximumWeight: " << GetMaximumWeight() << endl;
	//cout << "AvgDiffWeight: " << GetAvgDiffWeight() << endl;
	//cout << "SumL: " << GetSumL() << endl;

	//for (currentCluster = 0; currentCluster < clusters.size(); currentCluster++){
		//cout << "Scheduling cluster # " << currentCluster << endl;
		// sort cluster upward weigth/length ratio with non-violating precedence constraints
		//bool flag = true;
		//while (flag){
		//	flag = false;
		//	for (size_t current = 0; current < clusters[currentCluster].GetSize() - 1; current++){
		//		double firstWeigth = clusters[currentCluster].GetWeight(current), 
		//			firstDeadline = clusters[currentCluster].GetDeadline(current),
		//			secondWeigth = clusters[currentCluster].GetWeight(current + 1),
		//			secondDeadline = clusters[currentCluster].GetDeadline(current + 1);
		//		double first = firstWeigth / firstDeadline ,
		//			second = secondWeigth / secondDeadline;
		//		bool isPred = false;
		//		int firstWf, secondWf, firstNum, secondNum;
		//		data.GetLocalNumbers(current, firstWf, firstNum);
		//		data.GetLocalNumbers(current + 1, secondWf, secondNum);
		//		if (firstWf == secondWf)
		//			if (data.Workflows(firstWf).IsDepends(secondNum, firstNum)){
		//				isPred = true;
		//				//cout << firstNum << " was found before " << secondNum << endl;
		//			}
		//		if (second < first && !isPred) {
		//			clusters[currentCluster].Change(current, current + 1);
		//			//cout << clusters[currentCluster].GetPNum(current) << " was changed with " 
		//			//	<< clusters[currentCluster].GetPNum(current+1) << endl;
		//			flag = true;
		//		}
		//	}
		//}
		/*cout << "Before: " << endl;
		for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++)
			cout << clusters[currentCluster].GetPNum(i) << " ";
		cout << endl;*/
}

void Clustered::CheckPrecedence(){
	// check precedence constraints
		bool wasInsertion = true;

		while (wasInsertion)
		{
			wasInsertion = false;
			for (size_t first = 0; first < clusters[currentCluster].GetSize() - 1; first++){
				int firstWf, secondWf, firstNum, secondNum;
				for (size_t second = first + 1; second < clusters[currentCluster].GetSize() ; second++){
					int firstGlobal = clusters[currentCluster].GetPNum(first),
						secondGlobal = clusters[currentCluster].GetPNum(second);
					data.GetLocalNumbers(firstGlobal, firstWf, firstNum);
					data.GetLocalNumbers(secondGlobal, secondWf, secondNum);
					if (firstWf == secondWf)
						if (data.Workflows(firstWf).IsDepends(secondNum, firstNum)) {
							//if (firstNum == 28 || firstNum == 30 || firstNum == 32 ) 
							//if (firstWf == 15){
							//	//if (data.Workflows(firstWf).IsDepends(secondNum, firstNum)) 
							//	//	cout << firstNum << "depends on " << secondNum << endl;
							//	cout << firstWf << " " << firstNum << " " << secondWf << " " << secondNum << endl;
							//	cout << "first: " << clusters[currentCluster].GetPNum(first) << 
							//		" second :" << clusters[currentCluster].GetPNum(second) << endl;
							//}
							clusters[currentCluster].InsertAfter(first, second);
							second = first + 1;
							/*if (firstNum == 28 || firstNum == 30 || firstNum == 32 ) 
							if (firstWf == 15){
								for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++)
									cout << clusters[currentCluster].GetPNum(i) << " ";
								cout << endl << firstNum << "of " << firstWf << " was inserted after " << secondNum << "of " << firstWf << endl;
								char c; cin >>c;
								}*/
							wasInsertion = true;
						}
				}
			}
		}
		
		//cout << "After: " << endl;
		//cout << "Current cluster: " << currentCluster << endl;
		//for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++){
		//	//if (currentCluster == 5) 
		//	cout << clusters[currentCluster].GetPNum(i) << " ";
		//	if (clusters[currentCluster].GetPNum(i) == 251 || clusters[currentCluster].GetPNum(i)== 254) cout << "!!!" << endl;
		//}
		//cout << endl;
}

void Clustered::ClusterizeMerged(){
	int index = 0;
	while (clusterized.size() != data.GetPackagesCount()){
		for (vector<Cluster>::iterator it = clusters.begin(); it != clusters.end(); it++){
			currentCluster = it-clusters.begin();
			if (it->GetSize() == 1){
				int globalNum = it->GetPNum(0);
				int wfNum, localNum;
				data.GetLocalNumbers(globalNum, wfNum, localNum);
				vector <int> currCluster;
				currCluster.push_back(globalNum);
				//cout << "Search for:" << globalNum << "(" << wfNum << ", " << localNum << ")"<< endl ;
				currentF = GetFMerged(currentCluster);
				bool wasMerged = true;
				vector <int> candidates;
				vector <int> candIndexes;
				while (wasMerged == true){ // currCluster.size()!=data.GetWFCount() || 
					wasMerged = false;
					// building the vector with candidates for merging
					FindCandidates(currCluster, candidates, candIndexes);
					/*if (currCluster.size()== 1){
						cout << "Candidates: ";
						for (size_t i = 0; i < candidates.size(); i++){
							cout << candidates[i] << " ";
						}
						cout << endl;
					}*/
					double minVal = currentF;
					//cout << "currentF = " << currentF << endl;
					int bestCandidate = 0;
					for (size_t i = 0; i < candidates.size(); i++){
						//cout << "Candidate: " << candidates[i] << " ";
						double newF = GetFMerged(currentCluster,candIndexes[i]);
						//cout << "Value after merging: " << newF << endl;
						if (newF < minVal){
							minVal = newF;
							bestCandidate = i;
						}

						//currentF = newF;
					}
					if (minVal < currentF){
						Merge(candIndexes[bestCandidate],false);
						//cout << "Best candidate: " << candidates[bestCandidate] << " ";
						//cout << "Value after merging: " << minVal << endl;
						int indexVal = candIndexes[bestCandidate];
						currCluster.push_back(candidates[bestCandidate]);
						candidates.erase(candidates.begin()+bestCandidate);
						candIndexes.erase(candIndexes.begin()+bestCandidate);
						for (size_t clust = 0; clust < candIndexes.size(); clust++){
							if (candIndexes[clust]>indexVal) candIndexes[clust]--;
						}
						currentF = minVal;
						wasMerged = true;
						//system("pause");
						//cout << endl;
					}
					else {
						//cout << "Increase!" << endl;
					}
				}
				index++;
				//cout << "Cluster # " << index << endl;
				for (size_t i = 0; i < currCluster.size(); i++){
					clusterized.push_back(currCluster[i]);
					int wfNum, localNum;
					data.GetLocalNumbers(currCluster[i], wfNum, localNum);
					//cout << "(" << wfNum << ", " << localNum << ")";
				}
				//system("pause");
			}
			//cout << endl;
		}
	}
	CheckPrecedence();
	ofstream file;
	file.open("clustersInfo.txt");
	file.close();
	for (size_t i = 0; i < clusters.size(); i++){
		file.open("clustersInfo.txt", ios::app);
		clusters[i].PrintInfo(file);
		file << endl;
		file.close();
	}
}

double Clustered::GetFMerged(int currentCluster){
	double res;
	SetMaxSumL(currentCluster);
	double sumL = GetSumLMerged(currentCluster);
	double avgTaskCount = GetAvgTaskCount(currentCluster);
	//cout << "MaxSumL: " << maxSumL << " SumL: " << sumL << " SumL/maxSumL: " << sumL/maxSumL;
	//cout << endl << "1 - avgTaskCount: " << 1 - avgTaskCount << endl;
	res = 0.5 * sumL/maxSumL + 0.5 * (1 - avgTaskCount);
	//res = maxW/maxWeight + sumL/maxSumL;
	//res = avgDiffWeight/maxAvgDiffWeight  + sumL/maxSumL;
	//res = avgDiffWeight/maxAvgDiffWeight ;
	//res = sumL/maxSumL;
	return res;
}

double Clustered::GetFMerged(int currentCluster, int candIndex){
	double res;
	SetMaxSumL(currentCluster);
	maxSumL += clusters[candIndex].GetPLength(0);
	double sumL = GetSumLMerged(currentCluster, candIndex);
	double avgTaskCount = GetAvgTaskCount(currentCluster, candIndex);
	//cout << "MaxSumL: " << maxSumL << " SumL: " << sumL << " SumL/maxSumL: " << sumL/maxSumL;
	//cout << endl << "1 - avgTaskCount: " << 1 - avgTaskCount << endl;
	res = 0.5 * sumL/maxSumL + 0.5 * (1 - avgTaskCount);
	return res;
}

void Clustered::SetMaxSumL(int index){
	maxSumL = 0;
	for (size_t i = 0; i < clusters[index].GetSize(); i++){
		maxSumL += clusters[index].GetPLength(i);
	}
}

double Clustered::GetSumLMerged(int index){
	double res = 0;
	double start = data.GetT(), deadline = 0;
	for (size_t i = 0; i < clusters[index].GetSize(); i++){
		double currStart = clusters[index].GetPStart(i),
			currDeadline = clusters[index].GetDeadline(i);
		if (currStart < start) start = currStart;
		if (currDeadline > deadline) deadline = currDeadline;
	}
	return deadline-start;
}
double Clustered::GetSumLMerged(int currentCluster, int clusterIndex){
	double res = GetSumLMerged(currentCluster);
	double start = clusters[clusterIndex].GetStart(),
		deadline = clusters[clusterIndex].GetDeadline();
	if (start < clusters[currentCluster].GetStart())
		res += clusters[currentCluster].GetStart() - start;
	if (deadline > clusters[currentCluster].GetDeadline())
		res+= deadline-clusters[currentCluster].GetDeadline();
	return res;
}

double Clustered::GetAvgTaskCount(int index){
	vector <int> wfNums;
	vector <int> counts;
	for (size_t i = 0; i < clusters[index].GetSize(); i++){
		int globalNum = clusters[index].GetPNum(i);
		int wfNum, localNum;
		data.GetLocalNumbers(globalNum, wfNum, localNum);
		vector<int>::iterator it = find(wfNums.begin(), wfNums.end(), wfNum);
		if (it == wfNums.end()){
			wfNums.push_back(wfNum);
			counts.push_back(1);
		}
		else {
			counts[it-wfNums.begin()]++;
		}
	}
	double sum = 0;
	for (size_t i = 0; i < counts.size(); i++){
		sum += static_cast<double>(counts[i])/static_cast<double>(data.Workflows(wfNums[i]).GetPackageCount());
	}
	double res = sum/counts.size();
	return res;
}

double Clustered::GetAvgTaskCount(int index, int clusterIndex){
	vector <int> wfNums;
	vector <int> counts;
	for (size_t i = 0; i < clusters[index].GetSize(); i++){
		int globalNum = clusters[index].GetPNum(i);
		int wfNum, localNum;
		data.GetLocalNumbers(globalNum, wfNum, localNum);
		vector<int>::iterator it = find(wfNums.begin(), wfNums.end(), wfNum);
		if (it == wfNums.end()){
			wfNums.push_back(wfNum);
			counts.push_back(1);
		}
		else {
			counts[it-wfNums.begin()]++;
		}
	}
	int newPNum = clusters[clusterIndex].GetPNum(0);
	int wfNum, localNum;
	data.GetLocalNumbers(newPNum, wfNum, localNum);
	vector<int>::iterator it = find(wfNums.begin(), wfNums.end(), wfNum);
	if (it == wfNums.end()){
		wfNums.push_back(wfNum);
		counts.push_back(1);
	}
	else {
		counts[it-wfNums.begin()]++;
	}
	double sum = 0;
	for (size_t i = 0; i < counts.size(); i++){
		sum += static_cast<double>(counts[i])/static_cast<double>(data.Workflows(wfNums[i]).GetPackageCount());
	}
	double res = sum/counts.size();
	return res;
}

void Clustered::FindCandidates(vector<int>&input, vector<int>&candidates, vector<int>&candIndexes){
	// for all tasks in current cluster
	for (size_t in = 0; in < input.size(); in++){
		int globalNum = input[in];
		int wfNum, localNum;
		data.GetLocalNumbers(globalNum, wfNum, localNum);
		int index = 0;
		for (vector<Cluster>::iterator cand = clusters.begin(); cand != clusters.end(); cand++){
			if (cand->GetPNum(0) == globalNum) continue;
			if (cand->GetSize() == 1){
				int candNum = cand->GetPNum(0);
				if (find(candidates.begin(), candidates.end(),candNum) != candidates.end()) continue;
				int wfCandNum, localCandNum;
				data.GetLocalNumbers(candNum, wfCandNum, localCandNum);
				// if wf is the same
				if (wfCandNum == wfNum){
					// if task is the direct parent of another task
					if (data.Workflows(wfNum).IsParent(localNum, localCandNum)){
						candidates.push_back(candNum);
						candIndexes.push_back(cand-clusters.begin());
						continue;
					}
					// or task is independent and all its parents are already clustered
					if (!data.Workflows(wfNum).IsDepends(localNum, localCandNum)){
						vector <int> parentsCandNum;
						data.Workflows(wfNum).GetInput(localCandNum, parentsCandNum);
						bool allParentsClusterized = true;
						for (vector<int>::iterator parentsIt = parentsCandNum.begin();
							parentsIt != parentsCandNum.end(); parentsIt++){
								*parentsIt += data.GetInitPackageNumber(wfCandNum);
								if (find(clusterized.begin(), clusterized.end(), *parentsIt) == clusterized.end()){
									allParentsClusterized = false;
									break;
								}
						}
						if (allParentsClusterized) {
							candidates.push_back(candNum);
							candIndexes.push_back(cand-clusters.begin());
						}
					}
				} // if (wfCandNum == wfNum)
				else {
					vector <int> parentsCandNum;
					data.Workflows(wfCandNum).GetInput(localCandNum,parentsCandNum);
					// if task has no parents and it is not clusterized yet
					if (parentsCandNum.size() == 0){
						candidates.push_back(candNum);
						candIndexes.push_back(cand-clusters.begin());
					}
					else {
						bool allParentsClusterized = true;
						for (vector<int>::iterator parentsIt = parentsCandNum.begin();
							parentsIt != parentsCandNum.end(); parentsIt++){
								*parentsIt += data.GetInitPackageNumber(wfCandNum);
								if (find(clusterized.begin(), clusterized.end(), *parentsIt) == clusterized.end()){
									allParentsClusterized = false;
									break;
								}
						}
						if (allParentsClusterized) {
							candidates.push_back(candNum);
							candIndexes.push_back(cand-clusters.begin());
						}
					}
				}
			}
			index++;
		} // cycle on clusters
	}
}

struct CompareFirst
{
  CompareFirst(int val) : val_(val) {}
  bool operator()(const std::pair<int,vector<int>>& elem) const {
    return val_ == elem.first;
  }
  private:
    int val_;
};

void Clustered::SetWfPackages(vector<pair<int,vector<int>>>&wfPackages, int clusterIndex){
	for (size_t i = 0; i < clusters[clusterIndex].GetSize(); i++){
		int pNum = clusters[clusterIndex].GetPNum(i);
		int wfNum, localNum;
		data.GetLocalNumbers(pNum, wfNum, localNum);
		auto it = find_if(wfPackages.begin(),wfPackages.end(), CompareFirst(wfNum)) ;
		if (it == wfPackages.end()){
			vector<int> newVec;
			newVec.push_back(pNum);
			wfPackages.push_back(make_pair(wfNum, newVec));
		}
		else {
			it->second.push_back(pNum);
		}
	}
}

double Clustered::GetWFSchedule(Schedule &out){
	double res = 0.0;
	Greedy alg(data,-1,-1);
	SetInitClusters();
	//ClusterizeMerged();
	ClusterizeConsequence();
	for (size_t i = 0; i < clusters.size(); i++){
		vector<pair<int,vector<int>>>wfPackages;
		//SetWfPackages(wfPackages, i);
		for (size_t j = 0; j < clusters[i].GetSize(); j++){
		//for (size_t j = 0; j < wfPackages.size(); j++){
		//	for (size_t k = 0; k < wfPackages[j].second.size(); k++){
				double eff = 0.0;
				int sizeBefore = out.size();
		//		int pNum = wfPackages[j].second[k];
				/*int wfNum, localNum;
				data.GetLocalNumbers(pNum, wfNum, localNum);
				cout << "(" << wfNum << ", " << localNum << ")" << endl;*/
				int pNum = clusters[i].GetPNum(j);
				alg.FindSchedule(out, eff, pNum, false);
				res += eff;
				if (out.size() == sizeBefore){
					int wfNum, localNum;
					data.GetLocalNumbers(pNum, wfNum, localNum);
					vector <int> succ;
					data.Workflows(wfNum).GetOutput(localNum, succ);
					for (size_t s = 0; s < succ.size(); s++){
						// set global numbers
						succ[s] += data.GetInitPackageNumber(wfNum);
						for (size_t i1 = 0; i1 < clusters.size(); i1++){
							for (size_t j1 = 0; j1 < clusters[i1].GetSize(); j1++){
								if (clusters[i1].GetPNum(j1) == succ[s]){
									clusters[i1].Delete(j1);
									j1--;
								}
							}
						}
					}
				}
			//}// for inner inner
		}// for inner
	}
	
	return res;
}


double Clustered::GetClusterSchedule(Schedule &out){
	double res = 0.0;
	Greedy alg(data,-1,-1);
	// for each packege in cluster
	//for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++){
	int clusterSize = clusters[currentCluster].GetSize();
	int attempted = 0;
	while (clusterSize != attempted){
		int index = 0;
		int package = clusters[currentCluster].GetPNum(index);
		int wfNum, localNum;
		data.GetLocalNumbers(package, wfNum, localNum);
		vector <int> wfPackages;
		vector <int> indexesToDelete;
		wfPackages.push_back(package);
		clusters[currentCluster].Delete(0);

		for (size_t i = 0; i < clusters[currentCluster].GetSize(); i++){
			int secondWfNum, secondLocalNum;
			int secondPackage = clusters[currentCluster].GetPNum(i);
			data.GetLocalNumbers(secondPackage, secondWfNum, secondLocalNum);
			if (wfNum == secondWfNum){
				wfPackages.push_back(secondPackage);
				indexesToDelete.push_back(i);
			}
		}

		//cout << "wfpackages size " << wfPackages.size() << endl;

		for (size_t i = 0; i < indexesToDelete.size(); i++){
			clusters[currentCluster].Delete(indexesToDelete[i]);
			for (size_t j = i+1; j < indexesToDelete.size(); j++){
				if (indexesToDelete[i] < indexesToDelete[j]) indexesToDelete[j]--;
			}
		}

		for (size_t i = 0; i < wfPackages.size(); i++){
			double eff = 0.0;
			//int sizeBefore = out.size();
			//cout << clusters[currentCluster].GetPNum(i) << " ";
			alg.FindSchedule(out, eff, wfPackages[i],false);
			attempted++;
			/*if (currentCluster == 5 && sizeBefore == out.size())
				cout << "Cannot find placement for package " << clusters[currentCluster].GetPNum(i) << endl;*/
			res += eff;
		}
	}
	return res;
}

// should be called before merging any clusters
void Clustered::SetMaxAvgDiffWeight(){
	maxAvgDiffWeight = 0.0;
	for (size_t i = 0; i < clusters.size(); i++)
		maxAvgDiffWeight += clusters[i].GetWeight();
	maxAvgDiffWeight /= 2;
	//cout << "MaxAvgDiffWeight " << maxAvgDiffWeight << endl;
}

void Clustered::SetMaxWeight(){
	for (size_t i = 0; i < clusters.size(); i++)
		maxWeight += clusters[i].GetWeight();
}

void Clustered::SetMaxSumL(){
	maxSumL = 0.0;
	for (size_t i = 0; i < clusters.size(); i++){
		maxSumL += clusters[i].GetLength();
		
		//cout << clusters[i].GetStart() << " " << clusters[i].GetDeadline() << " " << clusters[i].GetLength() << endl;
	}
	
}

double Clustered::GetF(){
	double res;
	double avgDiffWeight = GetAvgDiffWeight();
	double maxW = GetMaximumWeight();
	double sumL = GetSumL();
	//res = maxW/maxWeight + sumL/maxSumL;
	res = avgDiffWeight/maxAvgDiffWeight  + sumL/maxSumL;
	//res = avgDiffWeight/maxAvgDiffWeight ;
	//res = sumL/maxSumL;
	return res;
}

double Clustered::GetF(int second){
	double res;
	double avgDiffWeight = GetAvgDiffWeight(second);
	//cout << "GetF() weight " << avgDiffWeight << " ";
	double maxW = GetMaximumWeight(second);
	double sumL = GetSumL(second);
	//res = maxW/maxWeight + sumL/maxSumL;
	//cout << "GetF() sum " << sumL << endl;
	res = avgDiffWeight/maxAvgDiffWeight  + sumL/maxSumL ;
	//res = avgDiffWeight/maxAvgDiffWeight;
	//res = sumL/maxSumL;
	return res;
}

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

double Clustered::GetAvgDiffWeight(){
	ofstream debug;
//	debug.open("debug2.txt");
	int currNum = 0;
	int iterations = 0;
	double sum = 0.0;
//	cout << "Check: " << clusters.size() << endl;
	while (currNum <= clusters.size() - 2){
//		debug << "currNum: " << currNum << endl;
		double currWeight = clusters[currNum].GetWeight();
//		debug << "currWeight: " << currWeight << endl;
		for (size_t i = currNum + 1; i < clusters.size(); i++){
			double anotherWeight = clusters[i].GetWeight();
//			debug << "anotherWeight: " << anotherWeight << endl;
			double diff = abs(currWeight - anotherWeight);
			sum += diff;
			++iterations;
		}
		currNum++;
	}
//	debug.close();
	return sum/iterations;
}

double Clustered::GetAvgDiffWeight(int second){
//	ofstream debug;
//	debug.open("debug.txt");
	unsigned int currNum = 0;
	int iterations = 0;
	double sum = 0.0;
	bool wasMergedClusterViewed = false;
	//cout << "Size: "<< clusters.size() << endl;
	while (currNum <= clusters.size() - 2){
		if (currNum == second || currNum == currentCluster){
			if (!wasMergedClusterViewed){
//				debug << "currNum: " << currNum << endl;
				double currWeight = clusters[second].GetWeight() + clusters[currentCluster].GetWeight();
//				debug << "currWeight: " << currWeight << endl;
					for (size_t i = currNum + 2; i < clusters.size(); i++){
					double anotherWeight = clusters[i].GetWeight();
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
			double currWeight = clusters[currNum].GetWeight();
//			debug << "currWeight: " << currWeight << endl;
			for (size_t i = currNum + 1; i < clusters.size(); i++){
				double anotherWeight;
				if (i == second || i == currentCluster){
					anotherWeight = clusters[second].GetWeight() + clusters[currentCluster].GetWeight();
					// to skip next cluster
					i++;
				}
				else anotherWeight = clusters[i].GetWeight();
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
	double res = GetSumL() - clusters[second].GetLength() - clusters[currentCluster].GetLength();
	double startCurr = clusters[currentCluster].GetStart(),
		deadlineCurr = clusters[currentCluster].GetDeadline(),
		startSecond = clusters[second].GetStart(),
		deadlineSecond = clusters[second].GetDeadline();
	double start = startCurr < startSecond ? startCurr : startSecond,
		d = deadlineCurr > deadlineSecond ? deadlineCurr : deadlineSecond;
	res += d - start;
	return res;
}

double Clustered::GetSumL(){
	double sumL = 0.0;
	for (size_t i = 0; i < clusters.size(); i++)
		sumL += clusters[i].GetLength();
	return sumL;
}

void Clustered::Merge(int second, bool isPrev){
	clusterInfo secondInfo;
	if (!isPrev){
		clusters[second].GetInfo(secondInfo);
		// add packages to current cluster
		for (size_t i = 0; i < secondInfo.get_head().size(); i++ ) {
			int pNum = secondInfo.get<0>()[i];
			double pStart = secondInfo.get<1>()[i];
			double pDeadline = secondInfo.get<2>()[i];
			double pWeight = secondInfo.get<3>()[i];
			clusters[currentCluster].Add(pNum, pWeight, pDeadline, pStart);
		}
		// delete another cluster
		clusters.erase(clusters.begin()+second);
	}
	else {
		clusters[currentCluster].GetInfo(secondInfo);
		// add packages to current cluster
		for (size_t i = 0; i < secondInfo.get_head().size(); i++ ) {
			int pNum = secondInfo.get<0>()[i];
			double pStart = secondInfo.get<1>()[i];
			double pDeadline = secondInfo.get<2>()[i];
			double pWeight = secondInfo.get<3>()[i];
			clusters[second].Add(pNum, pWeight, pDeadline, pStart);
		}
		// delete another cluster
		clusters.erase(clusters.begin()+currentCluster);
	}
}

Clustered::~Clustered(void)
{
}
