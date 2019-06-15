#include <iostream>
#include <fstream>
#include <string>
#include "API.h"
#include "Catalog.h"
/*
  create_table put the TableName s and Attribute atr and the primary key and index 
  into buffer. The sequence is atr.num, index.num, blockNumber, locationOfPrimaryKey, 
  NameOfAttribute, dataTypeOfAttribute, ifUnique, indexName, indexLocation.
*/
void CataManager::create_table(std::string s, Attribute atr, short primary, Index index){
    
    if(hasTable(s))
        throw TableException("ERROR in create_table: redefinition of table: " + s);
    if(primary>=0)
        atr.unique[primary] = true;
    
    string tableName = "T_" + s;
    int i;
    
    string filename = tableName;
    fstream fout(filename.c_str(), ios::out);
    fout.close();
    
    int No = bf.getBlockAddr(tableName, 0);

    char* begin = bf.blocks[No].data;
    int pos = 0;
    
	//put the number of attribute and index into buffer
    memcpy(&begin[pos], &atr.num, sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&begin[pos], &index.num, sizeof(int));
    pos = pos + sizeof(int);
    
    //the number of block and if it's primary
    int bn = 0;
    memcpy(&begin[pos], &bn, sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&begin[pos], &primary, sizeof(short));
    pos = pos + sizeof(short);
    
    //put name of attribute into buffer
    for(i=0;i<atr.num;i++){
        memcpy(&begin[pos], atr.name[i].data(), atr.name[i].length()*sizeof(char));
        pos = pos + (int)atr.name[i].length()*sizeof(char);
        memcpy(&begin[pos], "\0", sizeof(char));
        pos += sizeof(char);
    }
    //attribute's dataType
    for(i=0;i<atr.num;i++){
        memcpy(&begin[pos], &atr.flag[i], sizeof(short));
        pos = pos + sizeof(short);
    }
    //if the attribute is unique
    for(i=0;i<atr.num;i++){
        memcpy(&begin[pos], &atr.unique[i], sizeof(bool));
        pos = pos + sizeof(bool);
    }
    // the name of index
    for(i=0;i<index.num;i++){
        memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length()*sizeof(char));
        pos = pos + (int)index.indexname[i].length()*sizeof(char);
        memcpy(&begin[pos], "\0", sizeof(char));
        pos += sizeof(char);
    }
    //the location of index
    for(i=0;i<index.num;i++){
        memcpy(&begin[pos], &index.location[i], sizeof(short));
        pos = pos + sizeof(short);
    }
    
    //after the write operation, record
    bf.modifyBlock(No);
    
    Table* t = getTable(s);
    API api;
    api.CreateTable(*t);
    delete t;
    
}

/*
  if fail to read, return false;
  else return true, do have that table
*/
bool CataManager::hasTable(std::string s){
    string tableName = "T_" + s;
    std::ifstream in(tableName);
    if(!in){
        return false;
    }
    else {
        in.close();
        return true;
    }
}

Table* CataManager::getTable(std::string s){
    std::string tableName = "T_" + s;
    //if no table in buffer ,throw exception
    if(!hasTable(s))
        throw TableException("ERROR in getTable: No table named " + s);
    
    Attribute atr;
    Index ind;
    int pri;
    int bn;
    //use tableName to locate the buffer number
    int No = bf.getBlockAddr(tableName, 0);
    char* begin = bf.blocks[No].data;
    int pos = 0;
    int length;
    
    //read the data from buffer
    memcpy(&atr.num, &begin[pos], sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&ind.num, &begin[pos], sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&bn, &begin[pos], sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&pri, &begin[pos], sizeof(short));
    pos = pos + sizeof(short);
    
    //posEnd represent the end position of a name of arribute
    int posEnd = pos;
    char temp[20];
    int i;
    for(i=0;i<atr.num;i++){
    	//every name of attribute is divided by '\0'
        while(begin[posEnd]!='\0'){
        	posEnd += sizeof(char);
        }
        //length of the name
        length = posEnd-pos+1;
        memcpy(temp, &begin[pos], length*sizeof(char));
        atr.name[i] = temp;
        pos = posEnd + sizeof(char);
        posEnd = pos;
    }
    
    //read dataType
    for(i=0;i<atr.num;i++){
        memcpy(&atr.flag[i], &begin[pos], sizeof(short));
        pos = pos + sizeof(short);
    }
    
    //read if unique
    for(i=0;i<atr.num;i++){
        memcpy(&atr.unique[i], &begin[pos], sizeof(bool));
        pos = pos + sizeof(bool);
    }
    
    //read indexName
    posEnd = pos;
    for(i=0;i<ind.num;i++){
        while(begin[posEnd]!='\0')
            posEnd += 1;
        length = posEnd-pos+1;
        memcpy(temp, &begin[pos], length*sizeof(char));
        ind.indexname[i] = temp;
        pos = posEnd + sizeof(char);
        posEnd = pos;
    }
    
    //location of index
    for(i=0;i<ind.num;i++){
        memcpy(&ind.location[i], &begin[pos], sizeof(short));
        pos = pos + sizeof(short);
    }
    
	//record the read operation after reading
    //bf.useBlock(No);
    Table* t = new Table(s,atr,bn);
    
    
    t->Copyindex(ind);
    t->setprimary(pri);
    return t;
}


void CataManager::create_index(std::string tname, std::string aname, std::string iname){
    Table* temp = getTable(tname);
    try{
        int i;
        //find the attribute which name is aname
        for(i=0;i<temp->getCsize();i++){
        	if(temp->attr.name[i]==aname){
        		break;
        	}
        } 
        //if not found, throw error"not found"
        if(i==temp->getCsize())
            throw TableException("No attribute named " + aname);
        //if the attribute is not unique, throw error "not unique"
        if(temp->attr.unique[i]==0)
            throw TableException("This attribute is not unique!");
        temp->setindex(i, iname);
        
        string tableName = "T_"+tname;
        int No = bf.getBlockAddr(tableName, 0);
        
        //get the pointer of beginning of value in the table
        char* begin = bf.blocks[No].data;
        int pos = 0;
        //read the data from original table, then add the new index, put them in
        Attribute atr = temp->attr;
        Index index = temp->index;
        short primary = temp->primary;
        
        //rewrite to the buffer
        memcpy(&begin[pos], &atr.num, sizeof(int));
        pos = pos + sizeof(int);
        memcpy(&begin[pos], &index.num, sizeof(int));
        pos = pos + sizeof(int);
        
        int bn = 0;
        memcpy(&begin[pos], &bn, sizeof(int));
        pos = pos + sizeof(int);
        memcpy(&begin[pos], &primary, sizeof(short));
        pos = pos + sizeof(short);
        
        
        for(i=0;i<atr.num;i++){
            memcpy(&begin[pos], atr.name[i].data(), atr.name[i].length()*sizeof(char));
            pos = pos + (int)atr.name[i].length()*sizeof(char);
            memcpy(&begin[pos], "\0", sizeof(char));
            pos += sizeof(char);
        }
        
        for(i=0;i<atr.num;i++){
            memcpy(&begin[pos], &atr.flag[i], sizeof(short));
            pos = pos + sizeof(short);
        }
        
        for(i=0;i<atr.num;i++){
            memcpy(&begin[pos], &atr.unique[i], sizeof(bool));
            pos = pos + sizeof(bool);
        }
        
        for(i=0;i<index.num;i++){
            memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length()*sizeof(char));
            pos = pos + (int)index.indexname[i].length()*sizeof(char);
            memcpy(&begin[pos], "\0", sizeof(char));
            pos += sizeof(char);
        }
        
        for(i=0;i<index.num;i++){
            memcpy(&begin[pos], &index.location[i], sizeof(short));
            pos = pos + sizeof(short);
        }
        
        bf.modifyBlock(No);
        
        
        delete temp;
    }
    catch(TableException e1){
        delete temp;
        throw e1;
    }
}

void CataManager::drop_table(std::string t){
    if(!hasTable(t))
        throw TableException("ERROR in drop_table: No table named " + t);
    Table* table = getTable(t);
    API api;
    api.DropTable(*table);
    string tableName = "T_"+t;
    remove(tableName.c_str());
}

void CataManager::drop_index(std::string tname, std::string iname){
    Table* temp = getTable(tname);
    try{
        temp->dropindex(iname);
        drop_table(tname);
        create_table(tname, temp->attr, temp->primary, temp->index);
        delete temp;
    }
    catch(TableException e1){
        delete temp;
        throw e1;
    }
}

void CataManager::show_table(std::string tname){
    Table* t = getTable(tname);
    std::cout <<"Table" << tname << ":" << std::endl;
    Index ind;
    ind = t->Getindex();
    int i;
    int j;
    int length;
    //print every attribute,"attributeName dataType ifUnique ifPrimary"
    std::cout << "|  attributeName   | dataType | ifUnique | ifPrimary" << std::endl;
    for(i=0;i<t->getCsize();i++){
    	j = 0;
        std::cout << t->attr.name[i] << endl;
        while( (t->attr.name[i].length()+j) < 20 ){
        	std::cout << " ";
        	j++;
        }
        if(t->attr.flag[i]==-1)
            std::cout << "int" << "        ";
        else if(t->attr.flag[i]==0)
            std::cout << "float" << "      ";
        else{
        	length = 0;
        	if(t->attr.flag[i]<10){
        		length = 1;
        	}else if(t->attr.flag[i]<100){
        		length = 2;
        	}else{
        		length = 3;
        	}
            std::cout << "char(" << t->attr.flag[i] << ")";
            if(length == 1 ){
            	std::cout << "    ";
            }else if(length == 2){
            	std::cout << "   ";
            }else{
            	std::cout << "  ";
            }
        }
           
        if(t->attr.unique[i]==1)
            std::cout << "unique   ";
        else 
            std::cout << "         ";
        if(i==t->primary)
            std::cout << "primary key";
        std::cout << std::endl;
    }
    if(ind.num>0){
        std::cout << "index: ";
        for(i=0;i<ind.num;i++){
            std::cout << ind.indexname[i] << "(" << t->attr.name[ind.location[i]] << ") ";
            if(i%4==0){
            	std::cout << std::endl;
            	std::cout << "       ";
            }
        }
        std::cout << std::endl;
    }
    delete t;
}


void CataManager::changeblock(std::string tname, int bn){
    string tableName = "T_" + tname;
    int No = bf.getBlockAddr(tableName, 0);
    char* begin = bf.blocks[No].data;
    int pos = 0;
    pos = pos + sizeof(int) + sizeof(int);
    memcpy(&begin[pos], &bn, sizeof(int));
    //after changing block, record it
    bf.modifyBlock(No);

}


