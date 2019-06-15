#include<string>
#include<iostream>
using namespace std;
int main(void){
    string query = "show table hello ;";
	string tname;
	tname.append(query.substr(11, query.size()-13));
    cout<<tname;
}