

#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include "Catalog.h"
#include "Interpreter.h"
#include "API.h"


using namespace std;
int InterManager::EXEC() {
	Normolize();
	//cout << qs << endl;
	if (qs.substr(0, 6) == "select") {
		EXEC_SELECT();
		cout << "Interpreter: successful select!" << endl;
		return 1;
	}

	else if (qs.substr(0, 4) == "drop") {
		EXEC_DROP();
		cout << "Interpreter: successful drop!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "insert") {
		EXEC_INSERT();
		// cout << "Interpreter: successful insert!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "create") {
		EXEC_CREATE();
		cout << "Interpreter: successful create!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "delete") {
		EXEC_DELETE();
		cout << "Interpreter: successful delete!" << endl;
		return 1;
	}

	else if (qs.substr(0, 10) == "show table") {
		EXEC_SHOW();
		return 1;
	}

	else if (qs.substr(0, 4) == "exit") {
		EXEC_EXIT();
		return 0;
	}

	else if (qs.substr(0, 9) == "execfile:") {
		EXEC_FILE();
		return 1;
	}

	else
		throw QueryException("ERROR: invalid query format!");
}


void InterManager::GetQs() {
	string temp;
	qs = "";
	do {
		getline(cin, temp);
		qs = qs + " ";
		qs = qs + temp;
	} while (qs[qs.length() - 1] != ';');
	//cout << qs;
}

void InterManager::Normolize() {
	int pos;

	for (pos = 0; pos<qs.length(); pos++) {
		if (qs[pos] == '<' || qs[pos] == '>' || qs[pos] == '=' || qs[pos] == '(' || qs[pos] == ')' ||
			qs[pos] == ';' || qs[pos] == ',' || qs[pos] == '*')
		{
			if (qs[pos - 1] != ' ')
			{
				qs.insert(pos, " ");
				pos++;
			}

			if (qs[pos + 1] != ' ')
			{
				qs.insert(pos + 1, " ");
				pos++;
			}

		}
	}
	//cout << qs;
	int flag = 0;
	string::iterator it;
	for (it = qs.begin(); it<qs.end(); it++) {
		if (flag == 0 && (*it == ' ' || *it == '\t'))
		{
			flag = 1;
			continue;
		}
		if (flag == 1 && (*it == ' ' || *it == '\t')) {
			qs.erase(it);
			if (it != qs.begin())
				it--;
			continue;
		}
		if (*it != ' ' && *it != '\t') {
			flag = 0;
			continue;
		}
	}

	if (qs[0] == ' ')
		qs.erase(qs.begin());
	if (qs[qs.length() - 1] == ' ')
		qs.erase(--qs.end());

	//cout << qs;
}


void InterManager::EXEC_DROP() {
	int nowPos, posEnd;
	CataManager catalogManager;
	string tableName;
	string indexName;
	if (qs.substr(0, 10) == "drop table") {
		nowPos = 11;
		posEnd = GOGOGO(nowPos);
		tableName = qs.substr(nowPos, posEnd - nowPos);
		Table* table = catalogManager.getTable(tableName);

		if (qs[posEnd + 1] != ';')
		{
			delete table;
			throw QueryException("ERROR: incorrect query format!");
		}

		if (table->index.num>0) {
			for (int j = 0; j<table->index.num; j++) {
				catalogManager.drop_index(tableName, table->index.indexname[j]);
			}
		}

		delete  table;
		catalogManager.drop_table(tableName);
		//code:drop the data in datafile!
	}

	else if (qs.substr(0, 10) == "drop index") {
		nowPos = 11;
		posEnd = GOGOGO(nowPos);
		indexName = qs.substr(nowPos, posEnd - nowPos);
		nowPos = posEnd + 1;
		posEnd = GOGOGO(nowPos);
		if (qs.substr(nowPos, posEnd - nowPos) != "on")
		{
			throw QueryException("ERROR: incorrect query format!");
		}

		nowPos = posEnd + 1;
		posEnd = GOGOGO(nowPos);
		tableName = qs.substr(nowPos, posEnd - nowPos);

		if (qs[posEnd + 1] != ';')
			throw QueryException("ERROR: incorrect query format!");

		catalogManager.drop_index(tableName, indexName);
		//code:delete the index


	}

	else throw QueryException("ERROR: incorrect query format!");
}


void InterManager::EXEC_EXIT() {

}

void InterManager::EXEC_CREATE() {
	if (qs.substr(0, 13) == "create table ")
		EXEC_CREATE_TABLE();
	else if (qs.substr(0, 13) == "create index ")
		EXEC_CREATE_INDEX();
	else
		throw QueryException("ERROR: invalid query format!");
}

void InterManager::EXEC_INSERT() {
	int pos, pos1;
	int i;
	if (qs.substr(0, 12) != "insert into ")
		throw QueryException("ERROR: invalid query format!");
	pos = 12;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "values")
		throw QueryException("ERROR: invalid query format!");

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "(")
		throw QueryException("ERROR: invalid query format!");
	pos = pos1 + 1;
	tuper* tp = new tuper();
	string temp;
	CataManager cm;
	float p;
	Table* tb = cm.getTable(tname);
	Attribute attr = tb->getattribute();
	Data* dtemp;
	try {
		while (1) {
			pos1 = GOGOGO(pos);
			//int
			temp = qs.substr(pos, pos1 - pos);
			if (attr.flag[tp->length()] == -1) {
				if (!To_int(temp, i))
					throw QueryException("ERROR: " + temp + " is not a (int)!");
				dtemp = new Datai(i);
				tp->addData(dtemp);
			}

			//float
			else if (attr.flag[tp->length()] == 0) {
				if (!To_float(temp, p))
					throw QueryException("ERROR: " + temp + "is not a (float)!");
				dtemp = new Dataf(p);
				tp->addData(dtemp);
			}
			//char
			else {




				if (temp[0] != '\'')
					throw QueryException("ERROR: " + temp + " is not a (string)");
				while (qs[pos1 + 1] != ',' && qs[pos1 - 1] != '\'')
				{
					pos = pos1 + 1;
					pos1 = GOGOGO(pos);
					temp += " ";
					temp += qs.substr(pos, pos1 - pos);
				}

				if (temp[temp.length() - 1] != '\'')
					throw QueryException("ERROR: " + temp + " is not a (string)");


				temp.erase(temp.begin());
				temp.erase(--temp.end());
				dtemp = new Datac(temp);
				//dtemp->flag = 1;
				tp->addData(dtemp);
			}

			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs.substr(pos, pos1 - pos) != ",") {
				if (qs.substr(pos, pos1 - pos) != ")")
					throw QueryException("ERROR: invalid query format!");
				else break;
			}

			pos = pos1 + 1;
		}
		if (qs[pos1 + 1] != ';')
			throw QueryException("ERROR: invalid query format!");


	}
	catch (QueryException qe) {
		delete tb;
		delete tp;
		throw qe;

	}
	//tb->blockNum = 1;
	API api;
	api.Insert(*tb, *tp);

	//
	//  tb->addData(tp);

	//debug
	//    tb->disp();
	//debug

	delete tb;
	//delete tp;
}

void InterManager::EXEC_SELECT() {
	int pos = 7;
	int pos1 = GOGOGO(pos);
	CataManager cm;
	vector<int> attrselect;//the index of selected attributes
	vector<int> attrwhere;//the index of where attributes
	vector<where> w;
	string temp[32];//attribute name of selected attribute
	string temp0;
	int countselect = 0;
	if (qs.substr(pos, pos1 - pos) == "*") {
		countselect = -1;//flag : select all
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "from")
			throw QueryException("ERROR: invalid query format!");
	}
	else {
		while (qs.substr(pos, pos1 - pos) != "from") {
			temp[countselect] = qs.substr(pos, pos1 - pos);
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs.substr(pos, pos1 - pos) != ",") {
				countselect++;
				if (qs.substr(pos, pos1 - pos) == "from")
					break;
				throw QueryException("ERROR: invalid query format!");
			}

			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			countselect++;
		}
	}

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	Table* t = cm.getTable(tname);
	int i, j;
	Attribute A = t->getattribute();
	if (countselect>0) {
		for (i = 0; i<countselect; i++) {
			for (j = 0; j<t->getCsize(); j++)
				if (A.name[j] == temp[i]) {
					attrselect.push_back(j);
					break;
				}

			if (j == t->getCsize())
				throw QueryException("No attribute named " + temp[i]);
		}
	}
	else {
		for (i = 0; i<A.num; i++)
			attrselect.push_back(i);
	}
	if (qs[pos1 + 1] != ';') {
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "where")
			throw  QueryException("ERROR: invalid query format!");
		interwhere(pos1, attrwhere, w, A, t);
	}

	/*
	cout << "location of selected attr:\n";
	for(int i = 0;i<attrselect.size();i++)
	cout << attrselect[i] << " ";
	cout << "\n";

	cout << "location of where attr:\n";
	for(int i = 0;i<attrwhere.size();i++)
	cout << attrselect[i] << " ";
	cout << "\n";

	for(int i = 0;i<w.size();i++)
	cout << w[i].flag << " ";
	cout << "\n";

	for(int i = 0;i<w.size();i++){
	if(w[i].d->flag==-1)
	cout << ((Datai*)(w[i].d))->x << " ";
	if(w[i].d->flag==0)
	cout << ((Dataf*)(w[i].d))->x << " ";
	if(w[i].d->flag>0)
	cout << ((Datac*)(w[i].d))->x << " ";
	}
	cout << endl;
	*/

	API api;
	//t->blockNum = 1;
	Table output = api.Select(*t, attrselect, attrwhere, w);
	output.disp();
	//   throw std::bad_alloc();
	//delete t; //#Todo 不应该删


}

void InterManager::EXEC_DELETE() {
	int pos = 7;
	int pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "from")
		throw QueryException("ERROR: invalid query format!");

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	CataManager cm;
	Table* t = cm.getTable(tname);
	vector<int> attrwhere;
	vector<where> w;
	Attribute A = t->getattribute();
	if (qs[pos1 + 1] != ';') {
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "where")
			throw  QueryException("ERROR: invalid query format!");
		interwhere(pos1, attrwhere, w, A, t);
	}
	//
	//
	/*
	int i;
	cout << attrwhere.size() << " " << up.length() << endl;
	for(i=0;i<attrwhere.size();i++)
	cout << attrwhere[i] << endl;
	up.disptuper();
	down.disptuper();
	*/
	API api;
	//t->blockNum = 1;
	api.Delete(*t, attrwhere, w);
	delete t;

}



int InterManager::GOGOGO(int pos) {
	int pos1 = pos;
	while (pos1<qs.length() && qs[pos1] != ' ')
		pos1++;
	if (pos1 == qs.length())
		throw QueryException("ERROR: invalid query format!");
	return pos1;
}


void InterManager::EXEC_CREATE_TABLE() {
	int i, k;
	int nowPos = 13;
	int tNamePosEnd = GOGOGO(nowPos);
	//read tableName
	string tableName = qs.substr(nowPos, tNamePosEnd - nowPos);
	nowPos = tNamePosEnd + 1;
	int symbolPosEnd;
	symbolPosEnd = nowPos + 2;
	//read symbol
	if (qs.substr(nowPos, symbolPosEnd - nowPos) != "( ") {
		throw QueryException("ERROR: incorrect query format!");
	}
	Attribute attribute;
	attribute.num = 0;
	for (i = 0; i<32; i++) {
		attribute.unique[i] = false;
	}
	//beginning of the attribute
	nowPos = nowPos + 2;
	string word;
	int ifprimary = 0;
	int attrNameOrPriPos;
	int typePos;
	int uniquePos;
	int posEnd;
	while (true) {
		attrNameOrPriPos = GOGOGO(nowPos);
		posEnd = attrNameOrPriPos;
		word = qs.substr(nowPos, attrNameOrPriPos - nowPos);
		if (word == "primary") {
			ifprimary = 1;
			break;
		}
		attribute.name[attribute.num] = word;
		nowPos = attrNameOrPriPos + 1;
		typePos = GOGOGO(nowPos);
		posEnd = typePos;
		word = qs.substr(nowPos, typePos - nowPos);
		if (word == "int") {
			attribute.flag[attribute.num] = -1;
			nowPos = typePos + 1;
			typePos = GOGOGO(nowPos);
			posEnd = typePos;
		}
		else if (word == "float") {
			attribute.flag[attribute.num] = 0;
			nowPos = typePos + 1;
			typePos = GOGOGO(nowPos);
			posEnd = typePos;
		}
		else if (word == "char") {
			nowPos = typePos + 1;
			typePos = GOGOGO(nowPos);
			posEnd = typePos;
			if (qs[nowPos] != '(')
				throw QueryException("ERROR: incorrect query format!");
			nowPos = typePos + 1;
			typePos = GOGOGO(nowPos);
			posEnd = typePos;
			k = 0;
			for (i = typePos - 1; i >= nowPos; i--) {
				if (qs[i] <= '9' && qs[i] >= '0')
					k += pow(10, typePos - 1 - i)*(qs[i] - '0');
				else
					throw QueryException("ERROR: incorrect query format!");
			}
			attribute.flag[attribute.num] = k;
			nowPos = typePos + 1;
			if (qs.substr(nowPos, 2) != ") ")
				throw QueryException("ERROR: incorrect query format!");
			nowPos = nowPos + 2;
			typePos = GOGOGO(nowPos);
			posEnd = typePos;

		}
		else {
			throw QueryException("ERROR: incorrect query format!");
		}

		if (qs[nowPos] == ')') {
			attribute.num++;
			break;
		}
		else if (qs.substr(nowPos, 6) == "unique") {
			uniquePos = typePos;
			attribute.unique[attribute.num] = 1;
			nowPos = uniquePos + 1;
			uniquePos = GOGOGO(nowPos);
			posEnd = uniquePos;
			if (qs[nowPos] == ')') {
				break;
			}
			else if (qs[nowPos] == ',') {
				attribute.num++;
				nowPos = uniquePos + 1;
				continue;
			}
			else throw QueryException("ERROR: incorrect query format!");
		}
		else if (qs[nowPos] == ',') {
			attribute.num++;
			nowPos = typePos + 1;
			continue;
		}
		else {
			throw QueryException("ERROR: incorrect query format!");
		}

	}

	//Table T(tname,attr,0);
	string priKeyName;
	if (ifprimary) {
		nowPos = posEnd + 1;
		posEnd = GOGOGO(nowPos);
		if (qs.substr(nowPos, posEnd - nowPos) != "key")
			throw QueryException("ERROR: incorrect query format!");
		nowPos = posEnd + 1;
		if (qs.substr(nowPos, 2) != "( ")
			throw QueryException("ERROR: incorrect query format!");
		nowPos = nowPos + 2;
		posEnd = GOGOGO(nowPos);
		priKeyName = qs.substr(nowPos, posEnd - nowPos);
		nowPos = posEnd + 1;
		if (qs.substr(nowPos, 2) != ") ")
			throw QueryException("ERROR: incorrect query format!");
		nowPos = nowPos + 2;
		if (qs[nowPos] != ')')
			throw QueryException("ERROR: incorrect query format!");
		posEnd = nowPos + 1;
	}

	if (qs[posEnd + 1] != ';')
		throw QueryException("ERROR: incorrect query format!");

	short priKeyId;
	if (ifprimary) {
		for (priKeyId = 0; priKeyId<attribute.num; priKeyId++) {
			if (attribute.name[priKeyId] == priKeyName)
				break;
		}
	}
	else priKeyId = -1;

	CataManager catalogManager;
	Index index;
	if (priKeyId == attribute.num || !ifprimary)
		priKeyId = -1;
	else
		attribute.unique[priKeyId] = 1;
	index.num = 0;
	catalogManager.create_table(tableName, attribute, priKeyId, index);
	if (priKeyId != -1) {
		catalogManager.create_index(tableName, attribute.name[priKeyId], attribute.name[priKeyId]);
	}

}

void InterManager::EXEC_CREATE_INDEX() {
	CataManager catalogManager;
	int posEnd;
	int nowPos = 13;
	posEnd = GOGOGO(nowPos);
	string indexName;
	//read index name
	indexName = qs.substr(nowPos, posEnd - nowPos);
	nowPos = posEnd + 1;
	posEnd = GOGOGO(nowPos);
	//if not "on", throw exception
	if (qs.substr(nowPos, posEnd - nowPos) != "on")
		throw QueryException("ERROR: incorrect query format!");
	nowPos = posEnd + 1;
	posEnd = GOGOGO(nowPos);
	string tableName;
	//read table name
	tableName = qs.substr(nowPos, posEnd - nowPos);
	nowPos = posEnd + 1;
	posEnd = GOGOGO(nowPos);
	//if format is not correct, throw exception
	if (qs.substr(nowPos, posEnd - nowPos) != "(")
		throw QueryException("ERROR: incorrect query format!");
	nowPos = posEnd + 1;
	posEnd = GOGOGO(nowPos);
	//read attribute name;
	string attributeName = qs.substr(nowPos, posEnd - nowPos);
	nowPos = posEnd + 1;
	posEnd = GOGOGO(nowPos);
	//check format
	if (qs.substr(nowPos, posEnd - nowPos) != ")" || qs[posEnd + 1] != ';')
		throw QueryException("ERROR: incorrect query format!");
	catalogManager.create_index(tableName, attributeName, indexName);


}

void InterManager::EXEC_SHOW() {
	CataManager cm;
	string temp;
	int pos = 11;
	int pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	if (qs[pos1 + 1] != ';')
		throw QueryException("ERROR: invalid query format!");

	cm.show_table(tname);
}


bool To_int(string s, int& a) {
	int i;
	a = 0;
	for (i = 0; i<s.length(); i++) {
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(10, s.length() - i - 1)*(s[i] - '0');
		else return false;
	}
	return true;
}

bool To_float(string s, float& a) {
	int i;
	a = 0;
	int dot;
	for (i = 0; i<s.length(); i++)
		if (s[i] == '.')
			break;
	int j = 0;
	if (i == s.length()) {
		if (To_int(s, j))
		{
			a = j;
			return true;
		}
		else return false;
	}

	dot = i;
	for (i = 0; i<dot; i++) {
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(10, dot - i - 1)*(s[i] - '0');
		else return false;
	}
	for (i = dot + 1; i<s.length(); i++) {
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(0.1, i - dot)*(s[i] - '0');
		else return false;
	}
	return true;
}



//when pos1 is point to the space after "where".
void InterManager::interwhere(int& pos1, vector<int> &attrwhere, vector<where> &w, Attribute A, Table* t)
{
	int flag = 0;
	int x = 0;
	float y = 0;
	string z;
	Data* m;

	int pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string temp0;
	int j;
	where temp;
	while (1) {

		temp0 = qs.substr(pos, pos1 - pos);
		for (j = 0; j<t->getCsize(); j++)
			if (A.name[j] == temp0) {
				flag = A.flag[j];
				attrwhere.push_back(j);
				break;
			}
		if (j == t->getCsize())
			throw QueryException("No attribute named " + temp0);
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);

		if (qs.substr(pos, pos1 - pos) == "=") {
			temp.flag = eq;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (flag == -1 && To_int(temp0, x)) {
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)) {
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag>0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");

		}
		else if (qs.substr(pos, pos1 - pos) == ">") {
			temp.flag = g;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (temp0 == "=") {
				temp.flag = geq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}

			if (flag == -1 && To_int(temp0, x)) {
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)) {
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag>0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");

		}

		else if (qs.substr(pos, pos1 - pos) == "<") {
			temp.flag = l;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (temp0 == "=") {
				temp.flag = leq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}
			else if (temp0 == ">") {
				temp.flag = neq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}




			if (flag == -1 && To_int(temp0, x)) {
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)) {
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);

			}

			else if (flag>0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");
		}
		else throw QueryException("ERROR: invalid query format!");


		if (qs[pos1 + 1] == ';')
			break;

		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "and")
			throw QueryException("ERROR: invalid query format!");
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
	}
}

void InterManager::EXEC_FILE() {
	string fname = qs.substr(9, qs.length() - 11);
	ifstream in(fname);
	if (!in)
		throw QueryException("No file named " + fname);
	qs = "";
	string temp;
	while (in.peek() != EOF) {

		try {
			in >> temp;
			qs = qs + temp + " ";
			if (temp[temp.length() - 1] == ';') {
				//   cout << qs << endl;
				EXEC();
				qs = "";
			}
		}
		catch (TableException te) {
			cout << qs << endl;
			cout << te.what() << endl;
			qs = "";
		}
		catch (QueryException qe) {
			cout << qs << endl;
			cout << qe.what() << endl;
			qs = "";
		}



	}
	in.close();
}