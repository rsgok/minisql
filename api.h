// #pragma once
#ifndef _API_H
#define _API_H
#include "base.h"
#include "RM.h"

extern BufferManager bf;

class API
{
public:
	API() :rm(&bf){}
	~API();
	Table Select(Table& tableIn, vector<int> attrSelect, vector<int>mask, vector<where> w);
	int Delete(Table& tableIn, vector<int>mask, vector<where> w);
	void Insert(Table& tableIn, tuper& singleTuper);
	bool DropTable(Table& tableIn);
	void DropIndex(Table& tableIn, int attr);
	bool CreateTable(Table& tableIn);
	bool CreateIndex(Table& tableIn, int attr);
private:
	RecordManager rm;
};


#endif
