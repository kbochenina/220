#include <vector>
using namespace std;

#pragma once

class PriorityQueue
{
	// vector of (node number, priority)
	vector <pair<int,double>> pq;
public:
	//changes the priority (node value) of queue element
	void chgPrioirity(int node, double priority);
	// removes the top element of the queue
	void minPrioirty();
	// does the queue contain queue_element
	bool contains(int node);
	// insert queue_element into queue
	void insert(int node, double priority);
	// get the priority of selected node
	double getPriority(int node);
	// returns the top element of the queue
	pair <int, double> top(); 
	// return the number of queue_elements
	int size() {return pq.size();}
	void clear() { pq.clear(); }
};

// create a function object to return a node value from pq
// required as a predicate for find_if function
class NodeObject{
	int node;
public:
	NodeObject(int n) : node(n) {};
	bool operator()(pair<int,double> p){
		return node == p.first;
	}
};