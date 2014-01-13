#include "StdAfx.h"
#include "Graph.h"
#include <fstream>
#include <iostream>

using namespace std;

// return the number of vertices in the graph
int Graph::V(){
	return g.size();
}

// return the number of edges in the graph
int Graph::E(){
	// the number of edges can be counted as E = loopEdgesCount + nonLoopEdgesCount/2,
	// where E1 is the number of loop edges, for example (1,1)
	// and E2 is the number of non-loop edged, for example (1,3)
	// As the graph is undirected, all non-loop edges will be meeted twice,
	// therefore we divide nonLoopEdgesCount per 2
	int loopEdgesCount = 0, nonLoopEdgesCount = 0;
	edgeList::const_iterator vertice = g.begin();
	// loop for all vertices
	for (; vertice!= g.end(); vertice++){
		map<int,double>::const_iterator edge = vertice->second.begin();
		// loop for all current vertice edges
		for (; edge!= vertice->second.end(); edge++){
			// if edge is loop
			if (vertice->first == edge->first)
				loopEdgesCount++;
			else nonLoopEdgesCount++;
		}
	}
	return loopEdgesCount + nonLoopEdgesCount/2;
}

// tests whether there is an edge from x to y
bool Graph::adjacent(int x, int y){
	// find x in the graph
	edgeList::const_iterator firstIt = g.find(x);
	// if x is not in the graph, return false
	if (firstIt == g.end())
		return false;
	else {
		// trying to find y in the list of connected vertices of x
		map<int,double>::const_iterator secondIt = firstIt->second.find(y);
		// if y was found, first and second vertice are adjacent
		if (secondIt != firstIt->second.end())
			return true;
		else return false;
	}
}

// lists all nodes y such that there is an edge from x to y
void Graph::neighbors(int x, vector <int>& neighborsList){
	// if vector of neighbors is not empty,
	// clear it
	if (neighborsList.size()!=0) 
		neighborsList.clear();

	// find x node in the graph
	edgeList::const_iterator firstIt = g.find(x);
	// if x is not in the graph, neighborsList will be empty
	if (firstIt == g.end())
		return;

	// get the list of x neighbors and push them in the result vector
	map<int,double> neighbors = g[x];
	map<int,double>::const_iterator neighborsIt = neighbors.begin();
	for (; neighborsIt != neighbors.end(); neighborsIt++)
		neighborsList.push_back(neighborsIt->first);
}

// adds to G the edge from x to y, if it is not there
void Graph::addEdge(int x, int y){
	// g is a map<int, map<int,double>>
	// in a map, if key is not exists, it will be created
	// else its value will be changed
	// -1 means default value for path length
	// because real path length must set in set_edge_value function
	g[x][y] = -1;
}

// removes the edge from x to y, if it is there
void Graph::deleteEdge(int x, int y){
	edgeList::iterator xIt = g.find(x);
	// if x is not in the graph, we should not delete the edge
	if (xIt == g.end())
		return;
	// find y and delete if y is a neighbor of x
	map<int,double>::iterator yIt = xIt->second.find(y);
	xIt->second.erase(yIt);
}

// returns the value associated with the node x
int Graph::get_node_value(int x){
	// if x node exists
	if (nodesValues.find(x) != nodesValues.end()){
		return nodesValues[x];
	}
	// else return error value
	else return -1;
}

// sets the value associated with the node x to a
void Graph::set_node_value(int x, int a){
	nodesValues[x]=a;
}

// returns the value associated to the edge (x,y)
double Graph::get_edge_value(int x, int y){
	// if there is no x node in the graph, return error value -1
	if (g.find(x) == g.end())
		return -1;
	map<int,double>::const_iterator edge = g[x].find(y);
	// if there is no such edge, return -1
	if (edge == g[x].end())
		return -1;
	// else return distance
	return edge->second;
}

// sets the value associated to the edge (x,y) to v
void Graph::set_edge_value(int x, int y, double v){
	g[x][y] = v;
}


void Graph::print(string filename){
	ofstream f(filename);
	edgeList::const_iterator vertice = g.begin();
	// print edges as triples (begin, end, distance)
	for (; vertice != g.end(); vertice++){
		int begin = vertice->first;
		map<int,double>::const_iterator edge = vertice->second.begin();
		for (; edge!= vertice->second.end(); edge++)
			f << "(" << begin << ", " << edge->first << ", " << edge->second << ")";
		f << endl;
	}
	f.close();
}

// initialization from file (homework 3)
void Graph::InitFromFile( string fname )
{
	ifstream f(fname);
	if (f.fail()){
		cout << "Error while opening file with graph information" << endl;
		return;
	}
	// erase old values
	nodesValues.clear();
	g.clear();
	int nodesCount = 0;
	f >> nodesCount;
	// initializing nodesValues
	for (int i = 0; i < nodesCount; i++)
		nodesValues[i] = i;
	// reading edgelist values
	int x, y, cost;
	while(!f.eof()){
		f >> x >> y >> cost;
		set_edge_value(x, y, cost);
	}
}
