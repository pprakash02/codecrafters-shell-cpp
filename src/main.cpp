#include <iostream>
#include <string>
#include <set>
using namespace std;

int main() {
	// Flush after every std::cout / std:cerr
	cout << unitbuf;
	cerr << unitbuf;
	set<string> builtin_commands={"exit","echo","type"};
	
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
		else if(command.substr(0,4) == "type"){
			if(builtin_commands.count(command.substr(5))){
				cout<<command.substr(5)<<" is a shell builtin";
			}
			else{
				cout<<command.substr(5)<<": not found";
			}
		}
		else{
			cout<<command<<": command not found";
		}
		cout<<"\n";
	}	
}
