#include "Graph.h"
#include "PriorityQueue.h"

#pragma once

class ShortestPath
{
	Graph g;
	PriorityQueue pq;
	// vertices of the shortest path
	vector <int> vertices;
	// path cost associated with the shortest path
	double pathSize;
public:
	ShortestPath(Graph g) { this->g = g; pathSize = -1;}
	// find the shortest path between first node and last node
	// result will be stored in vertices variable
	void path (int firstNode, int lastNode);
	// return the path cost associated with the shortest path
	double path_size (int firstNode, int lastNode);
	// print vertices of the shortest path
	void print_path();
};

