#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_

#include<string>
#include<iostream>
#include<fstream>
#include"bplusTree.h"
#include"base.h"

class IndexManager{
public:
	void Establish(string file);
	void Insert(string file, int key, int addr);
	void Delete(string file, int key);
	int Find(string file, int key);
	void Drop(string file);
};

#endif