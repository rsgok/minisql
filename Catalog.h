
#ifndef Catalog_h
#define Catalog_h
#include "base.h"
#include "BufferManager.h"
extern BufferManager bf;

class CataManager {
public:
	CataManager() {};
	/*
	CataManager(BufferManager* buf, string tname):bf(buf){
	No = buf->getbufferNum(tname, 0);
	}
	*/
	void create_table(string tname, Attribute attribute, short primary, Index index);
	bool hasTable(std::string tname);
	Table* getTable(std::string tname);
	void create_index(std::string tname, std::string attributeName, std::string indexName);
	void drop_table(std::string tname);
	void drop_index(std::string tname, std::string indexName);
	void show_table(std::string tname);
	void changeblock(std::string tname, int blockNumber);
};



#endif /* Catalog_h */
