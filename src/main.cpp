#include <iostream>
#include <string>
#include <set>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;
namespace fs = filesystem;


vector<string> split_string(string line, const char delimiter){
	// function that splits a string into vector of strings based on delimiter
	vector<string> v;
	string s="";
	long long ct1=0, ct2=0;
	for(auto it = line.begin(); it!=line.end(); it++){
		auto it_new = it;
		if(*it == '\\' && !(ct1&1) && !(ct2&1)){	
			it++;
			if(it!=line.end()){
				s+=(*it);
				continue;
			}
			else break;
	
		}
		if(*it == '\\' && (ct1&1)){
			it++;
			if((it)!=line.end()){
				if(*it == '"' || *it == '\\' || *it == '$' || *it == '`' || *it == '\n'){
					s+=(*it);
				}
				else{
					it--;
					s+=(*it);
				}
				continue;
			}
			else break;
		}
		if(*it == '"' && !(ct2&1)){
			ct1++;
			continue;
		}
		if(*it == '\'' && !(ct1&1)){
			ct2++;
			continue;
		}
		if(*it == delimiter && !(ct1&1) && !(ct2&1)){
			if(s != "") v.push_back(s);
			s="";
		}
		else{
			it_new++;
			if((*it)=='>'){
				if(s!="") v.push_back(s);
				v.push_back(">");
				s="";
			}
			else if(((*it)=='1'||(*it)=='2')&&(*it_new)=='>'){
				if(s!="") v.push_back(s);
				string x;
				x.push_back((*it));
				x.push_back((*it_new));
				v.push_back(x);
				s="";
				it=it_new;
			}
			else{
				s+=(*it);
			}
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
	set<string> builtin_commands = {"exit","echo","type","pwd","cd"};
	
	//environment preprocessor for os path split
	#ifdef _WIN32
		const char os_pathstep = ';';
	#else
		const char os_pathstep = ':';
	#endif
	
	// getting home and path from env
	const char* path_variable = getenv("PATH");
	string path = "";
	if(path_variable != nullptr){
		path = path_variable;
	}
	vector<string> path_without_delimiter = split_string(path,os_pathstep);
	const char* home_variable = getenv("HOME");
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stderr = dup(STDERR_FILENO);
	//REPL implementation
	while(true){
		//prompt
		cout << "$ ";
		bool exec_done = false;
		//command input
		string line;
		getline(cin,line);
		if(line == "") continue;
		line+=' '; //added so that split_string works uniformly
		vector<string> command_unedited = split_string(line,' ');
		vector<string> command;
		bool stdout_redirected = false;
		bool stderr_redirected = false;
		int file, file_err;
		if(command_unedited.size()>1){
			if(command_unedited[command_unedited.size()-2] == ">" || command_unedited[command_unedited.size()-2] == "1>"){
				stdout_redirected = true;
				const char * location_of_file = command_unedited[command_unedited.size()-1].c_str();
				file = creat(location_of_file,0644);
				dup2(file, STDOUT_FILENO);
				for(long long i=0;i<(long long)command_unedited.size()-2;i++){
					command.push_back(command_unedited[i]);
				}
				
			}
			else if(command_unedited[command_unedited.size()-2] == "2>"){
					stderr_redirected = true;
					const char * location_of_file_err = command_unedited[command_unedited.size()-1].c_str();
					file_err = creat(location_of_file_err,0666);
					dup2(file_err, STDERR_FILENO);
					for(long long i=0;i<(long long)command_unedited.size()-2;i++){
						command.push_back(command_unedited[i]);
					}
				}
			else{
				command = command_unedited;
			}
		
		}
		else{
			command = command_unedited;
		}
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
		else if(command[0]=="pwd"){
			string cwd = fs::current_path();
			cout<<cwd;
		}
		else if(command[0]=="cd"){
				error_code ec;
				fs::path newp = home_variable;
				if((command.size()>1) && (command[1]!="~")) newp = command[1];
				fs::current_path(newp, ec);
				if(ec){
					cout<<"cd: "<<(string)newp<<": No such file or directory\n";
				}
				exec_done = true;
		
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
		fflush(stdout);
		if(not exec_done) cout<<"\n";
		
		
		if(stdout_redirected){
			dup2(saved_stdout, STDOUT_FILENO);
			close(file);
		}
		if(stderr_redirected){
			dup2(saved_stderr, STDERR_FILENO);
			close(file_err);
		}
	}	
}
