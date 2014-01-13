#pragma once
#include <map>
#include <vector>
#include <list>
using namespace std;

// notation: (node,[(neighbor1, distance1), (neighbor2, distance2),...])
// for example: ( 1, [(2,3), (4,5)]) means vertex 1 has pathes
// to vertices 2 and 4, edge (1->3) has length 3 and edge (1->4) has length 5
typedef map <int, map <int,double>> edgeList;

class Graph
{
	// values for graph's nodes
	// in this version of program values are simply the same as node indexes
	map <int,int> nodesValues;
	// for each vertice - list of graph edges with associated distances
	edgeList g;
public:
	// return the number of vertices in the graph
	int V();
	// return the number of edges in the graph
	int E();
	// tests whether there is an edge from x to y
	bool adjacent(int x, int y);
	// lists all nodes y such that there is an edge from x to y
	void neighbors(int x, vector <int>& neighborsList);
	// adds to G the edge from x to y, if it is not there
	void addEdge(int x, int y);
	// removes the edge from x to y, if it is there
	void deleteEdge(int x, int y);
	// returns the value associated with the node x
	int get_node_value(int x);
	// sets the value associated with the node x to a
	void set_node_value(int x, int a);
	// returns the value associated to the edge (x,y)
	double get_edge_value(int x, int y);
	// sets the value associated to the edge (x,y) to v
	void set_edge_value(int x, int y, double v);
	// print graph information to file
	void print(string filename);
	Graph(){};
	// initialization from file (homework 3)
	void InitFromFile(string fname);
};

