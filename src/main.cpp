#include <iostream>
#include <string>
using namespace std;
int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
 
  std::cout << "$ ";
   string command;
  getline(cin,command);
  if(command!=""){
	  cout<<command<<": command not found";
  }	
}
