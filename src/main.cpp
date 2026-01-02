#include <iostream>
#include <string>
using namespace std;

int main() {
	// Flush after every std::cout / std:cerr
	cout << unitbuf;
	cerr << unitbuf;
	
	//REPL implementation
	while(true){
		//prompt
		cout << "$ ";
		
		//command input
		string command;
		getline(cin,command);
		
		//invalid command handling
		if(command!=""){
			cout<<command<<": command not found";
		}
		cout<<"\n";
	}	
}
