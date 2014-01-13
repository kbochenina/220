#include "DataInfo.h"
#pragma once
class ModelData
{
	DataInfo data;
public:
	ModelData(DataInfo &d) {data = d;}
	DataInfo& GetData() {return data;}
	~ModelData(void);
};

