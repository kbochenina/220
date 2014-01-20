#include <vector>

using namespace std;
#pragma once
class ModelingContext
{
	int T;
	int delta;
	double CCR;
	int stages;
	vector<int> stageBorders;
public:
	ModelingContext();
	void SetContext(int T, int delta, double CCR);
	int GetT() const {return T;}
	inline int GetDelta() {return delta;}
	inline int GetStages() {return stages;}
	inline double GetCCR() {return CCR;}
	void SetT(double newT) { T = static_cast<int>(newT); }
	const vector<int>& GetBorders() {return stageBorders;}
	~ModelingContext(void);
};

