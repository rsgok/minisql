#include "Interpreter.h"

int Interpreter::next_space(int pos)
{
    int pos1 = pos;
    while (pos1 < query.length() && query[pos1] != ' ')
        pos1++;
    if (pos1 == query.length())
        throw QueryException("ERROR: invalid query format!");
    return pos1;
}



// ljk
//  get query
//  normalize
//  show
//  read file and execute 

void Interpreter::GetQs()
{
    string tmp;
    query = "";
    do
    {
        getline(cin, tmp);
        query = query + " "; // each line with a space ahead
        query = query + tmp;
    } while (query[query.length() - 1] != ';');

    //cout << query;
}

void Interpreter::Normalize()
{
    int pos;

    for (pos = 0; pos < query.length(); pos++)
    {
        if (query[pos] == '<' || query[pos] == '>' || query[pos] == '=' || query[pos] == '(' || query[pos] == ')' ||
            query[pos] == ';' || query[pos] == ',' || query[pos] == '*')
        {
            if (query[pos - 1] != ' ' && !(query.substr(pos - 1, 2) == ">=" || query.substr(pos - 1, 2) == "<=" || query.substr(pos - 1, 2) == "<>"))
            {
                query.insert(pos, " ");
                pos++;
            }

            if (query[pos + 1] != ' ' && !(query.substr(pos, 2) == ">=" || query.substr(pos, 2) == "<=" || query.substr(pos, 2) == "<>"))
            {
                query.insert(pos + 1, " ");
                pos++;
            }
        }
    }
    //cout << query;
    int flag = 0;
    for (auto it = query.begin(); it < query.end(); it++)
    {
        if (flag == 0 && *it == ' ' )
        {
            flag = 1;
            continue;
        }
        if (flag == 1 && *it == ' ' )
        {
            query.erase(it);
            if (it != query.begin())
                it--;
            continue;
        }
        if (*it != ' ')
        {
            flag = 0;
            continue;
        }
    }

    if (query[0] == ' ')
        query.erase(query.begin());
    if (query[query.length() - 1] == ' ')
        query.erase(--query.end());

    //cout << query;
}

void Interpreter::EXEC_SHOW()
{
    CataManager cm;
    string tmp;
    int pos = 11;

    // go to fetch file name
    int pos1 = next_space(pos);
    string table_name = query.substr(pos, pos1 - pos);
    if (query[pos1 + 1] != ';')
        throw QueryException("ERROR: invalid query format!");

    cm.show_table(table_name);
}

void Interpreter::EXEC_FILE()
{
    string file_name = query.substr(9, query.length() - 11);
    ifstream input(file_name);
    if (!input)
        throw QueryException("No file named " + file_name);
    query = "";
    string tmp;
    while (input.peek() != EOF)
    {

        try
        {
            input >> tmp;
            query = query + tmp + " ";
            if (tmp[tmp.length() - 1] == ';')
            {
                //   cout << query << endl;
                EXEC();
                query = "";
            }
        }
        catch (TableException te)
        {
            cout << query << endl;
            cout << te.what() << endl;
            query = "";
        }
        catch (QueryException qe)
        {
            cout << query << endl;
            cout << qe.what() << endl;
            query = "";
        }
    }
    input.close();
}

void Interpreter::EXEC_EXIT()
{
    cout << "Exiting...";
    exit(0);
}

int Interpreter::EXEC() {
	Normalize();
	//cout << query << endl;
	if (query.substr(0, 6) == "select") {
		EXEC_SELECT();
		cout << "Interpreter: successful select!" << endl;
		return 1;
	}

	else if (query.substr(0, 4) == "drop") {
		EXEC_DROP();
		cout << "Interpreter: successful drop!" << endl;
		return 1;
	}

	else if (query.substr(0, 6) == "insert") {
		EXEC_INSERT();
		// cout << "Interpreter: successful insert!" << endl;
		return 1;
	}

	else if (query.substr(0, 6) == "create") {
		EXEC_CREATE();
		cout << "Interpreter: successful create!" << endl;
		return 1;
	}

	else if (query.substr(0, 6) == "delete") {
		EXEC_DELETE();
		cout << "Interpreter: successful delete!" << endl;
		return 1;
	}

	else if (query.substr(0, 10) == "show table") {
		EXEC_SHOW();
		return 1;
	}

	else if (query.substr(0, 4) == "exit") {
		EXEC_EXIT();
		return 0;
	}

	else if (query.substr(0, 9) == "execfile:") {
		EXEC_FILE();
		return 1;
	}

	else
		throw QueryException("ERROR: invalid query format!");
}




// jbw
//  create table, index
//  drop     

void Interpreter::EXEC_CREATE_TABLE()
{
    int i, k;
    int nowPos = 13;
    int tNamePosEnd = next_space(nowPos);
    //read tableName
    string tableName = query.substr(nowPos, tNamePosEnd - nowPos);
    nowPos = tNamePosEnd + 1;
    int symbolPosEnd;
    symbolPosEnd = nowPos + 2;
    //read symbol
    if (query.substr(nowPos, symbolPosEnd - nowPos) != "( ")
    {
        throw QueryException("ERROR: incorrect query format!");
    }
    Attribute attribute;
    attribute.num = 0;
    for (i = 0; i < 32; i++)
    {
        attribute.unique[i] = false;
    }
    //beginning of the attribute
    nowPos = nowPos + 2;
    string word;
    int ifprimary = 0;
    int attrNameOrPriPos;
    int typePos;
    int uniquePos;
    while (true)
    {
        attrNameOrPriPos = next_space(nowPos);
        word = query.substr(nowPos, attrNameOrPriPos - nowPos);
        if (word == "primary")
        {
            ifprimary = 1;
            break;
        }
        attribute.name[attribute.num] = word;
        nowPos = attrNameOrPriPos + 1;
        typePos = next_space(nowPos);
        word = query.substr(nowPos, typePos - nowPos);
        if (word == "int")
        {
            attribute.flag[attribute.num] = -1;
            nowPos = typePos + 1;
            typePos = next_space(nowPos);
        }
        else if (word == "float")
        {
            attribute.flag[attribute.num] = 0;
            nowPos = typePos + 1;
            typePos = next_space(nowPos);
        }
        else if (word == "char")
        {
            nowPos = typePos + 1;
            typePos = next_space(nowPos);
            if (query[nowPos] != '(')
                throw QueryException("ERROR: incorrect query format!");
            nowPos = typePos + 1;
            typePos = next_space(nowPos);
            k = 0;
            for (i = typePos - 1; i >= nowPos; i--)
            {
                if (query[i] <= '9' && query[i] >= '0')
                    k += pow(10, typePos - 1 - i) * (query[i] - '0');
                else
                    throw QueryException("ERROR: incorrect query format!");
            }
            attribute.flag[attribute.num] = k;
            nowPos = typePos + 1;
            if (query.substr(nowPos, 2) != ") ")
                throw QueryException("ERROR: incorrect query format!");
            nowPos = nowPos + 2;
            typePos = next_space(nowPos);
        }
        else
        {
            throw QueryException("ERROR: incorrect query format!");
        }

        if (query[nowPos] == ')')
        {
            attribute.num++;
            break;
        }
        else if (query.substr(nowPos, 6) == "unique")
        {
            uniquePos = typePos;
            attribute.unique[attribute.num] = 1;
            nowPos = uniquePos + 1;
            uniquePos = next_space(nowPos);
            if (query[nowPos] == ')')
            {
                break;
            }
            else if (query[nowPos] == ',')
            {
				attribute.num++;
                nowPos = uniquePos + 1;
                continue;
            }
            else
                throw QueryException("ERROR: incorrect query format!");
        }
        else if (query[nowPos] == ',')
        {
            attribute.num++;
            nowPos = typePos + 1;
            continue;
        }
        else
        {
            throw QueryException("ERROR: incorrect query format!");
        }
    }

    //Table T(tname,attr,0);
    string priKeyName;
    int posEnd;
    if (ifprimary)
    {
        nowPos = posEnd + 1;
        posEnd = next_space(nowPos);
        if (query.substr(nowPos, posEnd - nowPos) != "key")
            throw QueryException("ERROR: incorrect query format!");
        nowPos = posEnd + 1;
        if (query.substr(nowPos, 2) != "( ")
            throw QueryException("ERROR: incorrect query format!");
        nowPos = nowPos + 2;
        posEnd = next_space(nowPos);
        priKeyName = query.substr(nowPos, posEnd - nowPos);
        nowPos = posEnd + 1;
        if (query.substr(nowPos, 2) != ") ")
            throw QueryException("ERROR: incorrect query format!");
        nowPos = nowPos + 2;
        if (query[nowPos] != ')')
            throw QueryException("ERROR: incorrect query format!");
        posEnd = nowPos + 1;
    }

    if (query[posEnd + 1] != ';')
        throw QueryException("ERROR: incorrect query format!");

    short priKeyId;
    if (ifprimary)
    {
        for (priKeyId = 0; priKeyId < attribute.num; priKeyId++)
        {
            if (attribute.name[priKeyId] == priKeyName)
                break;
        }
    }
    else
        priKeyId = -1;

    CataManager catalogManager;
    Index index;
    if (priKeyId == attribute.num || !ifprimary)
        priKeyId = -1;
    else
        attribute.unique[priKeyId] = 1;
    index.num = 0;
    catalogManager.create_table(tableName, attribute, priKeyId, index);
    if (priKeyId != -1)
    {
        catalogManager.create_index(tableName, attribute.name[priKeyId], attribute.name[priKeyId]);
    }
}

void Interpreter::EXEC_CREATE_INDEX()
{
    CataManager catalogManager;
    int posEnd;
    int nowPos = 13;
    posEnd = next_space(nowPos);
    string indexName;
    //read index name
    indexName = query.substr(nowPos, posEnd - nowPos);
    nowPos = posEnd + 1;
    posEnd = next_space(nowPos);
    //if not "on", throw exception
    if (query.substr(nowPos, posEnd - nowPos) != "on")
        throw QueryException("ERROR: incorrect query format!");
    nowPos = posEnd + 1;
    posEnd = next_space(nowPos);
    string tableName;
    //read table name
    tableName = query.substr(nowPos, posEnd - nowPos);
    nowPos = posEnd + 1;
    posEnd = next_space(nowPos);
    //if format is not correct, throw exception
    if (query.substr(nowPos, posEnd - nowPos) != "(")
        throw QueryException("ERROR: incorrect query format!");
    nowPos = posEnd + 1;
    posEnd = next_space(nowPos);
    //read attribute name;
    string attributeName = query.substr(nowPos, posEnd - nowPos);
    nowPos = posEnd + 1;
    posEnd = next_space(nowPos);
    //check format
    if (query.substr(nowPos, posEnd - nowPos) != ")" || query[posEnd + 1] != ';')
        throw QueryException("ERROR: incorrect query format!");
    catalogManager.create_index(tableName, attributeName, indexName);
}

void Interpreter::EXEC_DROP()
{
    int nowPos, posEnd;
    CataManager catalogManager;
    string tableName;
    string indexName;
    if (query.substr(0, 10) == "drop table")
    {
        nowPos = 11;
        posEnd = next_space(nowPos);
        tableName = query.substr(nowPos, posEnd - nowPos);
        Table *table = catalogManager.getTable(tableName);

        if (query[posEnd + 1] != ';')
        {
            delete table;
            throw QueryException("ERROR: incorrect query format!");
        }

        if (table->index.num > 0)
        {
            for (int j = 0; j < table->index.num; j++)
            {
                catalogManager.drop_index(tableName, table->index.indexname[j]);
            }
        }

        delete table;
        catalogManager.drop_table(tableName);
        //code:drop the data in datafile!
    }

    else if (query.substr(0, 10) == "drop index")
    {
        nowPos = 11;
        posEnd = next_space(nowPos);
        indexName = query.substr(nowPos, posEnd - nowPos);
        nowPos = posEnd + 1;
        posEnd = next_space(nowPos);
        if (query.substr(nowPos, posEnd - nowPos) != "on")
        {
            throw QueryException("ERROR: incorrect query format!");
        }

        nowPos = posEnd + 1;
        posEnd = next_space(nowPos);
        tableName = query.substr(nowPos, posEnd - nowPos);

        if (query[posEnd + 1] != ';')
            throw QueryException("ERROR: incorrect query format!");

        catalogManager.drop_index(tableName, indexName);
        //code:delete the index
    }

    else
        throw QueryException("ERROR: incorrect query format!");
}

void Interpreter::EXEC_CREATE()
{
    if (query.substr(0, 13) == "create table ")
        EXEC_CREATE_TABLE();
    else if (query.substr(0, 13) == "create index ")
        EXEC_CREATE_INDEX();
    else
        throw QueryException("ERROR: invalid query format!");
}


//wjw
//select insert delete show

#include<string>
#include<iostream>
#include"Interpreter.h"

void Interpreter::EXEC_SHOW() {
	CataManager cm;
	string tname;
	tname.append(query.substr(11, query.size() - 13));
	cm.show_table(tname);
}

void Interpreter::EXEC_INSERT() {
	tuper* tp = new tuper();
	int table_pos1 = 12;
	int tabel_pos2;
	for (int i = 12; i < query.size(); i++) {
		if (query[i + 1] == ' ') {
			tabel_pos2 = i;
			break;
		}
	}
	string tname = query.substr(table_pos1, tabel_pos2);
	CataManager cm;
	Table* tb = cm.getTable(tname);

	Attribute attr = tb->getattribute();
	Data* dtemp;
	int pos1, pos2;
	pos1 = 28;

	int x;
	float y;
	string str;

	string str;

	while (1) {
		int i;
		for (i = 0; i < query.size(); i++) {
			if (query[i + 1] == ' ') {
				pos2 = i;
				break;
			}
		}
		str = query.substr(pos1, pos2 - pos1);
		if (attr.flag[tp->length()] == -1) {
			x = stoi(str);
			dtemp = new Datai(x);
			tp->addData(dtemp);
		}
		else if (attr.flag[tp->length()] == 0) {
			y = stof(str);
			dtemp = new Dataf(y);
			tp->addData(dtemp);
		}
		else {
			dtemp = new Datac(str);
			tp->addData(dtemp);
		}
		pos1 = pos2;
		if (query[pos1] == ')')
			break;
	}

	API api;
	api.Insert(*tb, *tp);
	delete tb;
}

void Interpreter::EXEC_DELETE() {
	int pos1 = 7;
	int pos2 = next_space(pos1);
	if (query.substr(pos1, pos2 - pos1) != "from") {
		cout << "Exec_delete Error";
		return;
	}
	int flag = 0;
	pos1 = pos2 + 1;
	pos2 = next_space(pos1);
	string tname = query.substr(pos1, pos2 - pos1);
	CataManager cm;
	Table * tb = cm.getTable(tname);
	vector<int> mask;
	vector<where> w;
	Attribute A = tb->getattribute();
	string temp;
	where w_temp;
	int flag;
	int x = 0;
	float y = 0;
	string z;
	Data * m;
	while (query[pos2 + 2] != ';') {
		int i;
		pos1 = pos2 + 1;
		pos2 = next_space(pos1);
		temp = query.substr(pos1, pos2 - pos1);
		if (temp == "where")
			continue;
		else if (temp == "and")
			continue;
		else {
			for (i = 0; i < tb->getattribute.num; i++) {
				if (temp == tb->getattribute.name[i]) {
					flag = 1;
					mask.push_back(i);
					break;
				}
			}
			if (flag == 0)
				throw QueryException("ERROR: no that attribute!");
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			temp = query.substr(pos1, pos2 - pos1);
			if (temp == "=") {
				w_temp.flag = eq;
			}
			else if (temp == "<>") {
				w_temp.flag = neq;
			}
			else if (temp == "<") {
				w_temp.flag = l;
			}
			else if (temp == ">") {
				w_temp.flag = g;
			}
			else if (temp == "<=") {
				w_temp.flag = leq;
			}
			else if (temp == ">=") {
				w_temp.flag = geq;
			}
			else
				throw QueryException("ERROR: no that op");
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			temp = query.substr(pos1, pos2 - pos1);
			flag = tb->attr.flag[i];
			if (flag == -1) {
				x = stoi(temp);
				m = new Datai(x);
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else if (flag == 0) {
				y = stof(temp);
				m = new Dataf(y);
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else if (flag > 0) {
				m = new Datac(temp.substr(0, temp.length()));
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else
				throw QueryException("ERROR: no that value");

		}
	}
	API api;
	api.Delete(*tb, mask, w);
	delete tb;
}

void Interpreter::EXEC_SELECT() {
	int pos1 = 7;
	int pos2 = next_space(pos1);
	CataManager cm;
	vector<int> attrselect;//the index of selected attributes
	vector<int> mask;//the index of where attributes
	vector<where> w;
	int attr_select = 0;
	string attr_name[32];
	if (query.substr(pos1, pos2 - pos1) == "*") {
		attr_select = -1;//全部选择
		pos1 = pos2 + 1;
		pos1 = next_space(pos1);
		if (query.substr(pos1, pos2 - pos1) != "from") {
			throw QueryException("ERROR: invalid query format!");
		}
	}
	else {
		while (query.substr(pos1, pos2 - pos1) != "from") {
			attr_name[attr_select] = query.substr(pos1, pos2 - pos1);
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			if (query.substr(pos1, pos2 - pos1) != ",") {
				attr_select++;
				if (query.substr(pos1, pos2 - pos1) == "from")
					break;
				throw QueryException("ERROR: invalid query format!");
			}
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			attr_select++;
		}
	}

	//from 字段
	pos1 = pos2 + 1;
	pos2 = next_space(pos1);
	string tname = query.substr(pos1, pos2 - pos1);
	Table * tb = cm.getTable(tname);

	//where 字段

	Attribute A = tb->getattribute();
	string temp;
	where w_temp;
	int flag;
	int x = 0;
	float y = 0;
	string z;
	Data * m;
	while (query[pos2 + 2] != ';') {
		pos1 = pos2 + 1;
		pos2 = next_space(pos1);
		temp = query.substr(pos1, pos2 - pos1);
		if (temp == "where")
			continue;
		else if (temp == "and")
			continue;
		else {
			int i;
			for (i = 0; i < tb->getattribute.num; i++) {
				if (temp == tb->getattribute.name[i]) {
					flag = 1;
					mask.push_back(i);
					break;
				}
			}
			if (flag == 0)
				throw QueryException("ERROR: no that attribute!");
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			temp = query.substr(pos1, pos2 - pos1);
			if (temp == "=") {
				w_temp.flag = eq;
			}
			else if (temp == "<>") {
				w_temp.flag = neq;
			}
			else if (temp == "<") {
				w_temp.flag = l;
			}
			else if (temp == ">") {
				w_temp.flag = g;
			}
			else if (temp == "<=") {
				w_temp.flag = leq;
			}
			else if (temp == ">=") {
				w_temp.flag = geq;
			}
			else
				throw QueryException("ERROR: no that op");
			pos1 = pos2 + 1;
			pos2 = next_space(pos1);
			temp = query.substr(pos1, pos2 - pos1);
			flag = tb->attr.flag[i];
			if (flag == -1) {
				x = stoi(temp);
				m = new Datai(x);
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else if (flag == 0) {
				y = stof(temp);
				m = new Dataf(y);
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else if (flag > 0) {
				m = new Datac(temp.substr(0, temp.length()));
				w_temp.d = m;
				w.push_back(w_temp);
			}
			else
				throw QueryException("ERROR: no that value");

		}
	}

}
