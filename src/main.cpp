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
		cin>>command;
		
		//invalid command handling
		if(command=="exit"){
			break;
		}
		else if(command=="echo"){
			char c;
			scanf("%c",&c);
			string argument;
			getline(cin,argument);
			cout<<argument;
		}
		else{
			cout<<command<<": command not found";
		}
		cout<<"\n";
	}	
}
