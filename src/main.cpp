#include <iostream>
#include <string>
#include <set>
#include <filesystem>
#include <vector>
#include <cstdlib>
using namespace std;
namespace fs = filesystem;


vector<string> split_path(string path, const char delimiter){
	vector<string> v;
	string s="";
	for(auto it = path.begin(); it!=path.end(); it++){
		if(*it == delimiter){
			v.push_back(s);
			s="";
		}
		else{
			s+=(*it);
		}
	}
	return v;
}

bool fs_exists_and_exec(fs::path s){
	error_code ec;
	fs::perms prms = fs::status(s,ec).permissions();
	if(ec) return false;
	constexpr fs::perms owner_execution = fs::perms::owner_exec;
	return ((prms & owner_execution) != fs::perms::none);
}



int main() {
	// Flush after every std::cout / std:cerr
	cout << unitbuf;
	cerr << unitbuf;
	set<string> builtin_commands = {"exit","echo","type"};
	
	//environment preprocessor for os path split
	#ifdef _WIN32
		const char os_pathstep = ';';
	#else
		const char os_pathstep = ':';
	#endif
	
	// getting path from env
	const char* path_variable = getenv("PATH");
	string path = "";
	if(path_variable != nullptr){
		path = path_variable;
	}
	vector<string> path_without_delimiter = split_path(path,os_pathstep);
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
			string argument = command.substr(5);
			bool invalid = true;
			if(builtin_commands.count(argument) == 0){
				for(auto x:path_without_delimiter){
					if(fs_exists_and_exec(x+"/"+argument)){
						cout<<argument<<" is "<<x+"/"+argument;
						invalid = false;
						break;
					}
				}
			}
			else{
				cout<<argument<<" is a shell builtin";
				invalid = false;
			}
			if(invalid){
				cout<<argument<<": not found";
			}
		}
		else{
			cout<<command<<": command not found";
		}
		cout<<"\n";
	}	
}
