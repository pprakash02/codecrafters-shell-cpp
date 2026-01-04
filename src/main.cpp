#include <iostream>
#include <string>
#include <set>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
namespace fs = filesystem;


vector<string> split_string(string line, const char delimiter){
	// function that splits a string into vector of strings based on delimiter
	vector<string> v;
	string s="";
	for(auto it = line.begin(); it!=line.end(); it++){
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
	// function that checks if file is readable and executable
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
	vector<string> path_without_delimiter = split_string(path,os_pathstep);
	
	//REPL implementation
	while(true){
		//prompt
		cout << "$ ";
		bool exec_done = false;
		//command input
		string line;
		getline(cin,line);
		line+=' '; //added so that split_string works uniformly
		vector<string> command = split_string(line,' ');
		
		//invalid command handling
		if(command[0] == "exit"){
			break;
		}
		else if(command[0] == "echo"){
			auto it = command.begin();
			it++;
			if(it != command.end()){
				cout<<*it;
				it++;
			}
			while(it != command.end()){
				cout<<" "<<*it;
				it++;
			}
		}
		else if(command[0] == "type"){
			string argument = "";
			if(command.size() > 1){
				argument = command[1];
			}
			if(argument == ""){
				continue;
			}
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
			bool notfound = true;
			auto it = command.begin();
			vector<char*> argv;
			for(; it != command.end(); it++){
				argv.push_back(const_cast<char*>((*it).c_str()));
			}
			argv.push_back(nullptr);
			for(auto x:path_without_delimiter){
				string path_string = x + "/" + command[0];
				if(fs_exists_and_exec(path_string)){
					notfound = false;
					pid_t prog_pid = fork ();
					int status;
					if(!prog_pid){
						execv(path_string.c_str(), argv.data());
						exit(0);
					}
					wait(&status);
					exec_done = true;
					break;
				}
			}
			if(notfound)cout<<command[0]<<": command not found";
		}
		if(not exec_done) cout<<"\n";
	}	
}
