#include "base.h"
#include <iostream>
#include <string>
Table::Table(const Table& t){
    this->attr = t.attr;
    this->Tname = t.Tname;
    this->primary = t.primary;
    this->index = t.index;
    this->blockNum = t.blockNum;
    int i;
    for(i=0;i<t.getRsize();i++){
        tuper* tp = new tuper(*(t.T[i]));
        this->addData(tp);
    }
}

tuper::tuper(const tuper& t){
    int i;
    Data* d;
    for(i=0;i<t.length();i++){
        if(t.data[i]->flag==-1){
            d = new Datai( ((Datai*)(t.data[i]))->x );
            this->addData(d);
        }
        if(t.data[i]->flag==0){
            d = new Dataf( ((Dataf*)(t.data[i]))->x );
            this->addData(d);
        }
        if(t.data[i]->flag>0){
            d = new Datac( ((Datac*)(t.data[i]))->x );
            this->addData(d);
        }
    }
}

tuper::~tuper(){
    int i;
    for(i=0;i<data.size();i++)
    {
        if(data[i]==NULL)
            continue;
        if(data[i]->flag==-1)
            delete (Datai*)data[i];
        else if(data[i]->flag==0)
            delete (Dataf*)data[i];
        else
            delete (Datac*)data[i];
    }
}

void tuper::disptuper(){
    int i;
    for(i=0;i<data.size();i++)
    {
        if(data[i]==NULL)
            std::cout << "NULL" << "\t";
        else if(data[i]->flag==-1)
            std::cout << ((Datai*)data[i])->x << "\t";
        else if(data[i]->flag==0)
            std::cout << ((Dataf*)data[i])->x << "\t";
        else std::cout << ((Datac*)data[i])->x << "\t";
    }
    std::cout << std::endl;
}

Data* tuper::operator[](unsigned short i){
    if(i>=data.size())
        throw std::out_of_range("out of range in t[i]");
    return data[i];
}


Table::~Table(){
    int i;
    for(i=0;i<getRsize();i++)
        delete T[i];
}

void Table::addData(tuper* t){
    if(t->length()!=attr.num)
        throw TableException("Illegal Tuper Inserted: unequal column size!");
    int i,j;
    //影响效率的话可以删掉这段
    for(i=0;i<getCsize();i++){
        j = (*t)[i]->flag;
        if(j>attr.flag[i])
            throw TableException("Illegal Tuper Inserted: unequal attributes!");
        else if(j<attr.flag[i] && j<=0)
            throw TableException("Illegal Tuper Inserted: unequal attributes!");
        }
    T.push_back(t);
}


void Table::disp(){
    int i;
    for(i=0;i<attr.num;i++)
        std::cout << attr.name[i] << "\t";
    std::cout << std::endl;
    for(i=0;i<T.size();i++)
        T[i]->disptuper();
};//display the whole table


void Table::setindex(short i, std::string iname){
    short j;
    for(j=0;j<index.num;j++)
        if(i==index.location[j])
            break;
    if(j<index.num)
        throw TableException("Index already exists on this attribute!");
    
    for(j=0;j<index.num;j++)
        if(iname==index.indexname[j])
            break;
    if(j<index.num)
        throw TableException("Index name has been used!");
    
    index.location[index.num] = i;
    index.indexname[index.num] = iname;
    index.num++;
}

void Table::dropindex(std::string iname){
    short j;
    for(j=0;j<index.num;j++)
        if(iname==index.indexname[j])
            break;
    if(j==index.num)
        throw TableException("No index named " + iname + " on " + Tname + "!");
    
    std::string temps;
    short tempi;
    temps = index.indexname[j];
    index.indexname[j] = index.indexname[index.num-1];
    index.indexname[index.num-1] = temps;
    
    tempi = index.location[j];
    index.location[j] = index.location[index.num-1];
    index.location[index.num-1] = tempi;
    
    index.num--;
    
}