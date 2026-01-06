#include <iostream>
#include <string>
#include <algorithm>
#include <set>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;
namespace fs = filesystem;
vector<string> path_without_delimiter;
set<string> builtin_commands = {"exit","echo","type","pwd","cd","history"};
vector<const char*> executables_list={"cd","echo","exit","history","pwd","type"};

bool fs_exists_and_exec(fs::path s){
	// function that checks if file is readable and executable
	error_code ec;
	fs::perms prms = fs::status(s,ec).permissions();
	if(ec) return false;
	constexpr fs::perms owner_execution = fs::perms::owner_exec;
	return ((prms & owner_execution) != fs::perms::none);
}
void print_history(int i, int end){
	HIST_ENTRY **current_history=history_list();
	current_history+=i;
	while(i!=end){
		cout<<right<<setw(4)<<i+1<<"  "<<((*current_history)->line)<<"\n";
		i++;
		current_history++;
	}
	
}
char * command_generator(const char* text, int state){
	static int list_index, len;
	const char *name;
	if(!state){
		list_index = 0;
		len = strlen(text);
	}
	while((name = executables_list[list_index])!=nullptr){
		list_index++;
		if(strncmp(name,text,len)==0){
			return strdup(name);
		}
	}
	return nullptr;
}


char **executable_completion(const char* text, int start, int end){
	char **matches;
	matches = nullptr;
	if(start==0) matches = rl_completion_matches(text,command_generator);
	return matches;
}

void initialize_completion_list(){
	for(fs::path x : path_without_delimiter){
		for(const auto &entry : fs::directory_iterator(x)){
			fs::path curr = entry.path();
			if(fs_exists_and_exec(curr)){
				if(builtin_commands.count(curr.filename().string())==0)executables_list.push_back(strdup(curr.filename().string().c_str()));
			}
		}
	}
	sort(executables_list.begin(),executables_list.end());
	executables_list.push_back(nullptr);
	rl_attempted_completion_function = executable_completion;
}


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
				s.push_back(*it);
				continue;
			}
			else break;
	
		}
		if(*it == '\\' && (ct1&1)){
			it++;
			if((it)!=line.end()){
				if(*it == '"' || *it == '\\' || *it == '$' || *it == '`' || *it == '\n'){
					s.push_back(*it);
				}
				else{
					it--;
					s.push_back(*it);
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
			auto it_prev=it;
			it_prev--;
			if((*it)=='>'&&(*it_new)=='>'){
				if(s!="") v.push_back(s);
				v.push_back(">>");
				s="";
				it++;
			}
			else if((*it)=='>'){
				if(s!="") v.push_back(s);
				v.push_back(">");
				s="";
			}
			else if(((*it)=='1'||(*it)=='2')&&(*it_new)=='>'&&(*it_prev)==' '){
				if(s!="") v.push_back(s);
				string x;
				x.push_back((*it));
				x.push_back((*it_new));
				it_new++;
				if((*it_new)=='>'){
					x.push_back((*it_new));
					it++;
				}
				v.push_back(x);
				s="";
				it++;
			}
			else{
				s.push_back(*it);
			}
		}
	}
	return v;
}




int main() {
	// Flush after every std::cout / std:cerr
	cout << unitbuf;
	cerr << unitbuf;
	
	
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
	path_without_delimiter = split_string(path,os_pathstep);
	const char* home_variable = getenv("HOME");
	const char* histfile_variable = getenv("HISTFILE");
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stderr = dup(STDERR_FILENO);
	char *line_read;
	initialize_completion_list();
	int prev_append=0;
	using_history();
	read_history(histfile_variable);
	//REPL implementation using readline
	while((line_read = readline("$ ")) != nullptr){
		bool exec_done = false;
		if(line_read && *line_read){
			add_history(line_read);
			prev_append++;
		}
		
		//command input
		
		string line=line_read;
		free(line_read);
		if(line == "") continue;
		line.push_back(' '); //added so that split_string works uniformly
		vector<string> command_unedited = split_string(line,' ');
		vector<string> command;
		bool stdout_redirected = false;
		bool stderr_redirected = false;
		int file, file_err;
		if(command_unedited.size()>1){
			if(command_unedited[command_unedited.size()-2] == ">" || command_unedited[command_unedited.size()-2] == "1>"||command_unedited[command_unedited.size()-2] == "1>>"||command_unedited[command_unedited.size()-2] == ">>"){
				//redirect stdout
				
				stdout_redirected = true;
				const char * location_of_file = command_unedited[command_unedited.size()-1].c_str();
				if(command_unedited[command_unedited.size()-2] == ">" || command_unedited[command_unedited.size()-2] == "1>") file = creat(location_of_file,0644);
				if(command_unedited[command_unedited.size()-2] == ">>" || command_unedited[command_unedited.size()-2] == "1>>") file = open(location_of_file,O_CREAT|O_APPEND|O_WRONLY,0644);
				dup2(file, STDOUT_FILENO);
				for(long long i=0;i<(long long)command_unedited.size()-2;i++){
					command.push_back(command_unedited[i]);
				}
				
			}
			else if(command_unedited[command_unedited.size()-2] == "2>" || command_unedited[command_unedited.size()-2] == "2>>" ){
					//redirect stderr
					
					stderr_redirected = true;
					const char * location_of_file_err = command_unedited[command_unedited.size()-1].c_str();
					if(command_unedited[command_unedited.size()-2] == "2>") file_err = creat(location_of_file_err,0644);
					if(command_unedited[command_unedited.size()-2] == "2>>") file_err = open(location_of_file_err,O_APPEND|O_CREAT|O_WRONLY,0644);
					dup2(file_err, STDERR_FILENO);
					close(file_err);
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
			write_history(histfile_variable);
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
				cerr<<argument<<": not found";
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
					cerr<<"cd: "<<(string)newp<<": No such file or directory\n";
				}
				exec_done = true;
		
		}
		else if(command[0]=="history"){
			long long i=0;
			if(command.size()>1){
				if(command[1][0]=='-'){
					if(command[1][1]=='r'){
						read_history(command[2].c_str());
						continue;
					}
					if(command[1][1]=='w'){
						write_history(command[2].c_str());
						continue;
					}
					if(command[1][1]=='a'){
						append_history(prev_append,command[2].c_str());
						prev_append=0;
						continue;
					}
				}
				else{
					i=stoi(command[1]);
					print_history(history_length-i,history_length);
				}
			}
			else{
				print_history(i,history_length);
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
					pid_t prog_pid = fork();
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
	clear_history();
}
