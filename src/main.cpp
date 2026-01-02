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
		if(command == "exit"){
			break;
		}
		else if(command.substr(0,4) == "echo"){
			cout<<command.substr(5);
		}
		else{
			cout<<command<<": command not found";
		}
		cout<<"\n";
	}	
}
