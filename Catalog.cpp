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
void CataManager::create_table(std::string tname, Attribute atrribute, short primary, Index index) {
	if (hasTable(tname))
		throw TableException("ERROR in create_table: redefinition of table: " + tname);
	if (primary >= 0)
		atrribute.unique[primary] = true;

	string tableName = "T_" + tname;
	int i;

	string filename = tableName;
	fstream fout(filename.c_str(), ios::out);
	fout.close();

	int bufferNum = bf.getbufferNum(tableName, 0);

	char* readWrite = bf.bufferBlock[bufferNum].values;
	int pos = 0;

	//put the number of attribute and index into buffer
	memcpy(&readWrite[pos], &atrribute.num, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&readWrite[pos], &index.num, sizeof(int));
	pos = pos + sizeof(int);

	//the number of block and if it's primary
	int blockNumber = 0;
	memcpy(&readWrite[pos], &blockNumber, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&readWrite[pos], &primary, sizeof(short));
	pos = pos + sizeof(short);

	//put name of attribute into buffer
	for (i = 0; i<atrribute.num; i++) {
		memcpy(&readWrite[pos], atrribute.name[i].data(), atrribute.name[i].length() * sizeof(char));
		pos = pos + (int)atrribute.name[i].length() * sizeof(char);
		memcpy(&readWrite[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}
	//attribute's dataType
	for (i = 0; i<atrribute.num; i++) {
		memcpy(&readWrite[pos], &atrribute.flag[i], sizeof(short));
		pos = pos + sizeof(short);
	}
	//if the attribute is unique
	for (i = 0; i<atrribute.num; i++) {
		memcpy(&readWrite[pos], &atrribute.unique[i], sizeof(bool));
		pos = pos + sizeof(bool);
	}
	// the name of index
	for (i = 0; i<index.num; i++) {
		memcpy(&readWrite[pos], index.indexname[i].data(), index.indexname[i].length() * sizeof(char));
		pos = pos + (int)index.indexname[i].length() * sizeof(char);
		memcpy(&readWrite[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}
	//the location of index
	for (i = 0; i<index.num; i++) {
		memcpy(&readWrite[pos], &index.location[i], sizeof(short));
		pos = pos + sizeof(short);
	}

	//after the write operation, record
	bf.writeBlock(bufferNum);

	Table* table = getTable(tname);
	API api;
	api.CreateTable(*table);
	delete table;

}

/*
if fail to read, return false;
else return true, do have that table
*/
bool CataManager::hasTable(std::string tname) {
	string tableName = "T_" + tname;
	std::ifstream in(tableName);
	if (!in) {
		return false;
	}
	else {
		in.close();
		return true;
	}
}

Table* CataManager::getTable(std::string tname) {
	std::string tableName = "T_" + tname;
	//if no table in buffer ,throw exception
	if (!hasTable(tname))
		throw TableException("ERROR in getTable: No table named " + tname);

	Attribute atrribute;
	Index index;
	int pri;
	int blockNumber;
	//use tableName to locate the buffer number
	int bufferNum = bf.getbufferNum(tableName, 0);
	char* readWrite = bf.bufferBlock[bufferNum].values;
	int pos = 0;
	int length;

	//read the data from buffer
	memcpy(&atrribute.num, &readWrite[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&index.num, &readWrite[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&blockNumber, &readWrite[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&pri, &readWrite[pos], sizeof(short));
	pos = pos + sizeof(short);

	//posEnd represent the end position of a name of arribute
	int posEnd = pos;
	char temp[20];
	int i;
	for (i = 0; i<atrribute.num; i++) {
		//every name of attribute is divided by '\0'
		while (readWrite[posEnd] != '\0') {
			posEnd += sizeof(char);
		}
		//length of the name
		length = posEnd - pos + 1;
		memcpy(temp, &readWrite[pos], length * sizeof(char));
		atrribute.name[i] = temp;
		pos = posEnd + sizeof(char);
		posEnd = pos;
	}

	//read dataType
	for (i = 0; i<atrribute.num; i++) {
		memcpy(&atrribute.flag[i], &readWrite[pos], sizeof(short));
		pos = pos + sizeof(short);
	}

	//read if unique
	for (i = 0; i<atrribute.num; i++) {
		memcpy(&atrribute.unique[i], &readWrite[pos], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	//read indexName
	posEnd = pos;
	for (i = 0; i<index.num; i++) {
		while (readWrite[posEnd] != '\0')
			posEnd += 1;
		length = posEnd - pos + 1;
		memcpy(temp, &readWrite[pos], length * sizeof(char));
		index.indexname[i] = temp;
		pos = posEnd + sizeof(char);
		posEnd = pos;
	}

	//location of index
	for (i = 0; i<index.num; i++) {
		memcpy(&index.location[i], &readWrite[pos], sizeof(short));
		pos = pos + sizeof(short);
	}

	//record the read operation after reading
	bf.useBlock(bufferNum);
	Table* table = new Table(tname, atrribute, blockNumber);
	table->Copyindex(index);
	table->setprimary(pri);
	return table;
}


void CataManager::create_index(std::string tname, std::string attributeName, std::string indexName) {
	Table* tempTable = getTable(tname);
	try {
		int i;
		//find the attribute which name is aname
		for (i = 0; i<tempTable->getCsize(); i++) {
			if (tempTable->attr.name[i] == attributeName) {
				break;
			}
		}
		//if not found, throw error"not found"
		if (i == tempTable->getCsize())
			throw TableException("No attribute named " + attributeName);
		//if the attribute is not unique, throw error "not unique"
		if (tempTable->attr.unique[i] == 0)
			throw TableException("This attribute is not unique!");
		tempTable->setindex(i, indexName);

		string tableName = "T_" + tname;
		int bufferNum = bf.getbufferNum(tableName, 0);

		//get the pointer of beginning of value in the table
		char* readWrite = bf.bufferBlock[bufferNum].values;
		int pos = 0;
		//read the data from original table, then add the new index, put them in
		Attribute atrribute = tempTable->attr;
		Index index = tempTable->index;
		short primary = tempTable->primary;

		//rewrite to the buffer
		memcpy(&readWrite[pos], &atrribute.num, sizeof(int));
		pos = pos + sizeof(int);
		memcpy(&readWrite[pos], &index.num, sizeof(int));
		pos = pos + sizeof(int);

		int blockNumber = 0;
		memcpy(&readWrite[pos], &blockNumber, sizeof(int));
		pos = pos + sizeof(int);
		memcpy(&readWrite[pos], &primary, sizeof(short));
		pos = pos + sizeof(short);


		for (i = 0; i<atrribute.num; i++) {
			memcpy(&readWrite[pos], atrribute.name[i].data(), atrribute.name[i].length() * sizeof(char));
			pos = pos + (int)atrribute.name[i].length() * sizeof(char);
			memcpy(&readWrite[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i<atrribute.num; i++) {
			memcpy(&readWrite[pos], &atrribute.flag[i], sizeof(short));
			pos = pos + sizeof(short);
		}

		for (i = 0; i<atrribute.num; i++) {
			memcpy(&readWrite[pos], &atrribute.unique[i], sizeof(bool));
			pos = pos + sizeof(bool);
		}

		for (i = 0; i<index.num; i++) {
			memcpy(&readWrite[pos], index.indexname[i].data(), index.indexname[i].length() * sizeof(char));
			pos = pos + (int)index.indexname[i].length() * sizeof(char);
			memcpy(&readWrite[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i<index.num; i++) {
			memcpy(&readWrite[pos], &index.location[i], sizeof(short));
			pos = pos + sizeof(short);
		}

		bf.writeBlock(bufferNum);


		delete tempTable;
	}
	catch (TableException e1) {
		delete tempTable;
		throw e1;
	}
}

void CataManager::drop_table(std::string tname) {
	if (!hasTable(tname))
		throw TableException("ERROR in drop_table: No table named " + tname);
	Table* table = getTable(tname);
	API api;
	api.DropTable(*table);
	string tableName = "T_" + tname;
	remove(tableName.c_str());
}

void CataManager::drop_index(std::string tname, std::string indexName) {
	Table* tempTable = getTable(tname);
	try {
		tempTable->dropindex(indexName);
		drop_table(tname);
		create_table(tname, tempTable->attr, tempTable->primary, tempTable->index);
		delete tempTable;
	}
	catch (TableException e1) {
		delete tempTable;
		throw e1;
	}
}

void CataManager::show_table(std::string tname) {
	Table* table = getTable(tname);
	std::cout << "Table" << tname << ":" << std::endl;
	Index index;
	index = table->Getindex();
	Attribute attribute;
	attribute = table->getattribute();
	int i;
	int j;
	int length;
	//print every attribute,"attributeName dataType ifUnique ifPrimary"
	std::cout << "|  attributeName   | dataType | ifUnique | ifPrimary" << std::endl;
	for (i = 0; i<table->getCsize(); i++) {
		j = 0;
		std::cout << table->attr.name[i];
		while ((attribute.name[i].length() + j) < 20) {
			std::cout << " ";
			j++;
		}
		if (table->attr.flag[i] == -1)
			std::cout << "int" << "        ";
		else if (table->attr.flag[i] == 0)
			std::cout << "float" << "      ";
		else {
			length = 0;
			if (attribute.flag[i]<10) {
				length = 1;
			}
			else if (attribute.flag[i]<100) {
				length = 2;
			}
			else {
				length = 3;
			}
			std::cout << "char(" << table->attr.flag[i] << ")";
			if (length == 1) {
				std::cout << "    ";
			}
			else if (length == 2) {
				std::cout << "   ";
			}
			else {
				std::cout << "  ";
			}
		}

		if (table->attr.unique[i] == 1)
			std::cout << "unique   ";
		else
			std::cout << "         ";
		if (i == table->primary)
			std::cout << "primary key";
		std::cout << std::endl;
	}
	if (index.num>0) {
		std::cout << "index: ";
		for (i = 0; i<index.num; i++) {
			std::cout << index.indexname[i] << "(" << table->attr.name[index.location[i]] << ") ";
			if (i % 4 == 0) {
				std::cout << std::endl;
				std::cout << "       ";
			}
		}
		std::cout << std::endl;
	}
	delete table;
}


void CataManager::changeblock(std::string tname, int blockNumber) {
	string tableName = "T_" + tname;
	int bufferNum = bf.getbufferNum(tableName, 0);
	char* readWrite = bf.bufferBlock[bufferNum].values;
	int pos = 0;
	pos = pos + sizeof(int) + sizeof(int);
	memcpy(&readWrite[pos], &blockNumber, sizeof(int));
	//after changing block, record it
	bf.writeBlock(bufferNum);

}


