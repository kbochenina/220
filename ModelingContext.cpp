#include "StdAfx.h"
#include "ModelingContext.h"


ModelingContext::ModelingContext(void)
{
	T = 0; 
	delta = 0;
	stages = 0;
}



ModelingContext::~ModelingContext(void)
{
}

void ModelingContext::SetContext( int T, int delta, double CCR )
{
	this->T = T;
	this->delta = delta;
	stages = T/delta;
	for (int i = 0; i <= T; i+=delta) 
		stageBorders.push_back(i);
	this->CCR = CCR;
}
