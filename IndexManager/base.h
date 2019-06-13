//
//  base.h
//  MiniSQL
//
//  Created by runganghan on 16/5/28.
//  Copyright © 2016年 hrg. All rights reserved.
//

#ifndef base_h
#define base_h
#include <iostream>
#include <vector>

class Data{
public:
    short flag;
    //-1-int 0-float 1~255-char.
    virtual ~Data(){};
};

class Datai : public Data{
public:
    Datai(int i):x(i){
        flag = -1;
    };
    virtual ~Datai(){}
    int x;
};

class Dataf : public Data{
public:
    Dataf(float f):x(f){
        flag = 0;
    };
    virtual ~Dataf(){}
    float x;
};

class Datac : public Data{
public:
    Datac(std::string c):x(c){
        flag = c.length();
        if(flag==0)
            flag = 1;
    };
    virtual ~Datac(){}
    std::string x;
};

struct Attribute{
    short flag[32];//data type
    std::string name[32];//attribute name
    bool unique[32];//unique
    int num;
};

struct Index{
    int num;
    short location[10];
    std::string indexname[10];
};

class tuper{
public:
    std::vector<Data*> data;
public:
    tuper(){};
    tuper(const tuper& t);
    ~tuper();
    
    int length() const{
        return (int)data.size();
    }//return the length of the data.
    
    void addData(Data* d){
        data.push_back(d);
    }//add a new data to the tuper.
    
    Data* operator[](unsigned short i);
    //return the pointer to a specified data item.
    
    void disptuper();
    //display the data in the tuper.
    
};

class Table{
    friend class CataManager;
public:
    int blockNum;//total number of blocks occupied in data file;
    int dataSize(){ //size of a single tuper;
        int res = 0;
        for (int i = 0; i < attr.num;i++){
            switch (attr.flag[i]){
                case -1:res += sizeof(int); break;
                case 0:res += sizeof(float); break;
                default:res += attr.flag[i]+1; break; //多一位储存'\0'
            }
        }
        return res;
    }
public:
    std::string Tname;
    Attribute attr;//number of attributes
public:
    Table(std::string s,Attribute aa, int bn):Tname(s),attr(aa),blockNum(bn){
        primary = -1;
        for(int i = 0;i<32;i++){ aa.unique[i] = false; }
        index.num=0;
    }
    //Construct with Tname and column.
    Table(const Table& t);
    ~Table();
    std::vector<tuper*> T;//pointers to each tuper
    short primary;//the location of primary key. -1 means no primary key.
    Index index;
    
    Attribute getattribute(){
        return attr;
    }
    
    void setindex(short i, std::string iname);
    
    void dropindex(std::string iname);
    
    void Copyindex(Index ind){
        index = ind;
    }
    
    Index Getindex(){
        return index;
    }
    
    void setprimary(int p){
        primary = p;
    }//set the primary key
    
    void disp();
    
    std::string getname(){
        return Tname;
    };
    
    int getCsize() const{
        return attr.num;
    };
    
    int getRsize() const{
        return (int)T.size();
    };
    
    void addData(tuper* t);
    
    
};
typedef enum{
    eq,leq,l,geq,g,neq} WHERE;

struct where{
    Data* d;
    WHERE flag;
};




class TableException: public std::exception{
public:
    TableException(std::string s):text(s){}
    std::string what(){
        return text;
    };
private:
    std::string text;
};


#endif /* base_h */
