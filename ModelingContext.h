#include <vector>

using namespace std;
#pragma once
class ModelingContext
{
	int T;
	int delta;
	int stages;
	vector<int> stageBorders;
public:
	ModelingContext();
	void SetContext(int T, int delta);
	int GetT() const {return T;}
	inline int GetDelta() {return delta;}
	inline int GetStages() {return stages;}
	const vector<int>& GetBorders() {return stageBorders;}
	~ModelingContext(void);
};

