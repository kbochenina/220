#include "StdAfx.h"
#include "ShortestPath.h"
#include "Graph.h"
// for find_if and reverse
#include <algorithm>

#include <iostream>
using namespace std;

// find the shortest path between first node and last node
// result will be stored in vertices variable
/*void ShortestPath::path (int firstNode, int lastNode){
	// clear the vertices variable
	vertices.clear();
	// closed set
	vector <pair<int,double>> closedSet;
	// (node, previousNode) for all visited nodes
	map <int,int> previousNodes;
	pair <int, double> currentPair = make_pair(firstNode,0.0);
	int currentNode = currentPair.first;
	// currentPath variable is a length of path to the currentNode
	double currentPath = currentPair.second;
	// push the first node to the closed set
	closedSet.push_back(currentPair);
	// neigbors of currentNode
	vector <int> neighbors;
	// get neighbors of first node
	g.neighbors(currentNode, neighbors);
	// do while lastNode not in closed set
	// or open set has values
	do{
		vector <int>::const_iterator nIt = neighbors.begin();
		// for all neighbors of current node
		while (nIt != neighbors.end()){
			int neighborNode = *nIt++;
			// if neigborNode is not already in an closed set
			if (find_if(closedSet.begin(),closedSet.end(),NodeObject(neighborNode))==closedSet.end()){
				// neighborPath is a length of path to the neighborNode
				double neighborPath = currentPath + g.get_edge_value(currentNode, neighborNode);
				// if neighborNode is not in an open set
				if (!pq.contains(neighborNode)) {
					// insert neighborNode to an open set
					pq.insert(neighborNode, neighborPath);
					// and store the previous node
					previousNodes[neighborNode] = currentNode;
				}
				// if open set contains neigborNode
				else{
					// find the LONGEST PATH
					// and path to the neigborNode will be improved
					if (pq.getPriority(neighborNode) > neighborPath){
						// change the priority of neighborNode
						pq.chgPrioirity(neighborNode, neighborPath);
						// and update the previous node
						previousNodes[neighborNode] = currentNode;
					}
				}
			}
		}
		// get the top from priority queue
		currentPair = pq.top();
		int prevCurrentNode = currentNode;
		currentNode = currentPair.first;
		currentPath = currentPair.second;
		pq.minPrioirty();
		closedSet.push_back(currentPair);
		// get neighbors of currentNode
		g.neighbors(currentNode, neighbors);
		// if currentNode haven't unvisited neighbors
		// and open set is empty
		// there is no path between firstNode and lastNode
		if (pq.size() == 0 && neighbors.size()==0)
			break;
		
	} while (currentNode!=lastNode  );

	// if path exists
	if (currentNode == lastNode){
		// place to vertices nodes of shortest path from lastNode to firstNode
		vertices.push_back(lastNode);
		int previousNode = previousNodes[lastNode];
		while (previousNode != firstNode){
			vertices.push_back(previousNode);
			previousNode = previousNodes[previousNode];
		}
		vertices.push_back(firstNode);
		// and reverse the vector of shortest path nodes
		reverse(vertices.begin(), vertices.end());
		pathSize = currentPath;
	}
	else {
		pathSize = -1;
	}
	// clear the priority queue
	pq.clear();
}
*/

void ShortestPath::path(int first, int last){
	vector <bool> mark;
	vector<double> dist;
	vector <int> prev;
	int size = this->g.V() + 1;
	prev.resize(size);
	for (int i = 0; i < size; i++) {
		mark.push_back(false);
		dist.push_back(std::numeric_limits<double>::infinity());
	}
	int y = first;
	mark[first] = true;
	dist[first] = 0.0;
	bool unvisited = true;

	while (unvisited){
		for (int i = 0; i <size; i++){
			double edge = g.get_edge_value(y,i); 
			if (edge != -1) {
				if (dist[i] > dist[y] + edge){
					dist[i] = dist[y] + edge;
					prev[i] = y;
				}
			}
		}
		mark[y] = true;
		double min = std::numeric_limits<double>::infinity();
		
		unvisited = false;

		for (int i = 0; i < size; i++){
			if (!mark[i]) {
				unvisited = true;
				if (dist[i] < min){
					min = dist[i];
					y = i;
				}
			}
		}
	}


	int previousNode = prev[last];
	pathSize = dist[last];
	while (previousNode != first){
		vertices.push_back(previousNode);
		previousNode = prev[previousNode];
	}
	//vertices.push_back(first);
	// and reverse the vector of shortest path nodes
	reverse(vertices.begin(), vertices.end());
}

double ShortestPath::path_size (int firstNode, int lastNode){
	// find the shortest path
	// this function modifies field pathSize (cost of shortest path)
	// if path is not exists, pathSize = -1
	path(firstNode, lastNode);
	return pathSize;
}

void ShortestPath::print_path(){
	for (const auto & i : vertices)
		cout << i << " ";
	cout << endl;
}
