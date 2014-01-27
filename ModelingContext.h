#pragma once
class ModelingContext
{
	int T;
	double CCR;
   // koefficient of inhomogeneity of data distribution between packages
   double h;
public:
	ModelingContext();
	void SetContext(int T, double CCR, double h);
	int GetT() const {return T;}
	inline double GetCCR() {return CCR;}
   inline double GetH() {return h;}
	void SetT(double newT) { T = static_cast<int>(newT); }
 	~ModelingContext(void);
};

