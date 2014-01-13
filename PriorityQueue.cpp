#include "StdAfx.h"
#include "PriorityQueue.h"
// for find_if
#include <algorithm>

using namespace std;

//changes the priority (node value) of queue element
void PriorityQueue::chgPrioirity(int node, double priority){
	// trying to find a node with required node value 
	vector <pair<int,double>>::iterator iter = find_if(pq.begin(),pq.end(),NodeObject(node));
	// if node was found, we will change its priority
	if (iter != pq.end())
		iter->second = priority;
}
// removes the top element of the queue
void PriorityQueue::minPrioirty(){
	// if queue is not empty, erase the first element
	if (!pq.empty())
		pq.erase(pq.begin());
}
// does the queue contain queue_element?
bool PriorityQueue::contains(int node){
	// trying to find a node with required node value 
	vector <pair<int,double>>::const_iterator iter = find_if(pq.begin(),pq.end(),NodeObject(node));
	if (iter != pq.end())
		return true;
	else return false;
}
// insert queue_element into queue
void PriorityQueue::insert(int node, double priority){
	// constructing queue element
	pair<int,double> elem = make_pair(node, priority);
	// if queue is empty, insert the element
	if (pq.empty()){
		pq.push_back(elem);
	}
	// else we need to find first element with priority value more than current elem's priority
	else{
		vector<pair<int,double>>::const_iterator iter = pq.begin();
		// if element was inserted, wasElemInserted = true
		bool wasElemInserted = false;
		for (; iter!= pq.end(); iter++){
			// if we find an element with lower priority
			if (iter->second > priority){
				// insert new element before it
				pq.insert(iter,1,elem);
				wasElemInserted = true;
				// exit from the loop
				break;
			}
		}
		// if new element has the highest priority, it will be placed to the end
		if (!wasElemInserted)
			pq.push_back(elem);
	}
}

// get the priority of selected node
double PriorityQueue::getPriority(int node){
	// trying to find a node with required node value 
	vector <pair<int,double>>::const_iterator iter = find_if(pq.begin(),pq.end(),NodeObject(node));
	// if node was found, return priority value
	if (iter != pq.end())
		return iter->second;
	else return -1;
}
// returns the top element of the queue
pair <int, double> PriorityQueue::top(){
	// if queue is not empty, return the first element
	if (!pq.empty())
		return pq.front();
	else return make_pair(-1,-1);
}
