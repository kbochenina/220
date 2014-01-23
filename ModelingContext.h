#pragma once
class ModelingContext
{
	int T;
	double CCR;
public:
	ModelingContext();
	void SetContext(int T, double CCR);
	int GetT() const {return T;}
	inline double GetCCR() {return CCR;}
	void SetT(double newT) { T = static_cast<int>(newT); }
	~ModelingContext(void);
};

