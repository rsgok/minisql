//
//  Catalog.h
//  MiniSQL
//
//  Created by runganghan on 16/5/30.
//  Copyright © 2016年 hrg. All rights reserved.
//

#ifndef Catalog_h
#define Catalog_h
#include "base.h"
#include "BufferManager.h"
extern BufferManager bf;

class CataManager{
public:
    CataManager(){};
    /*
    CataManager(BufferManager* buf, string tname):bf(buf){
        No = buf->getbufferNum(tname, 0);
    }
    */
    void create_table(string s, Attribute atb, short primary, Index index);
    bool hasTable(std::string s);
    Table* getTable(std::string s);
    void create_index(std::string tname, std::string aname, std::string iname);
    void drop_table(std::string t);
    void drop_index(std::string tname, std::string iname);
    void show_table(std::string tname);
    void changeblock(std::string tname, int bn);
};



#endif /* Catalog_h */
