#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include "Catalog.h"
#include "api.h"


using namespace std;

class Interpreter
{
private:
    string query; //query string
public:
    int EXEC();
    void EXEC_SELECT();
    void EXEC_DROP();
    void EXEC_CREATE();
    void EXEC_CREATE_TABLE();
    void EXEC_CREATE_INDEX();
    void EXEC_INSERT();
    void EXEC_DELETE();
    void EXEC_SHOW();
    void EXEC_EXIT();
    void EXEC_FILE();

    inline int next_space(int pos);


    void GetQs();
    //Get the inputing string

    void Normalize();
    //Normalize the inputting string

};

class QueryException :std::exception {
public:
	QueryException(std::string s) :text(s) {}
	std::string what() {
		return text;
	};
private:
	std::string text;
};


