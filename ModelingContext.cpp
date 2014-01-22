#include "StdAfx.h"
#include "ModelingContext.h"


ModelingContext::ModelingContext(void)
{
	T = 0; 
   CCR = 0.0;
}



ModelingContext::~ModelingContext(void)
{
}

void ModelingContext::SetContext( int T,  double CCR )
{
	this->T = T;
	this->CCR = CCR;
}
