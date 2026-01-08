

# C++ Shell : PraSH

A robust, feature-rich command-line shell interface built from scratch in C++. This project emulates core functionalities of standard Unix shells like Bash, leveraging system calls to manage processes, I/O redirection, pipelines, and environment navigation.

## Features

### Core Functionality

* **Command Parsing**: robust tokenization of user input handling arguments, flags, and paths.
* **Process Management**: Uses `fork()`, `execv()`, and `waitpid()` to launch and manage external programs.
* **Built-in Commands**: Native implementations for shell-specific operations:
* `cd`: Change directory (supports `~` for home).
* `pwd`: Print working directory.
* `echo`: Display text to standard output.
* `type`: Identify if a command is a builtin or executable path.
* `history`: View and manage command history.
* `exit`: Terminate the shell session.



### Advanced Features

* **I/O Redirection**:
* `>` : Overwrite standard output.
* `>>`: Append to standard output.
* `2>`: Overwrite standard error.
* `2>>`: Append to standard error.


* **Pipelines**: Support for chaining multiple commands using `|`, connecting the stdout of one process to the stdin of the next.
* **Autocompletion**: Tab-completion for executables found in the system `PATH`.
* **Quote Handling**: Proper parsing of single (`'`) and double (`"`) quotes, allowing for arguments with spaces.
* **Shell History**:
* Persistent history across sessions (reads/writes to `HISTFILE`).
* Arrow-key navigation through previous commands.
* Commands to read/write/append history manually.



##  System Requirements

* **OS**: Linux / Unix-like environment (macOS, WSL).
* **Compiler**: `g++` (supporting C++17 or later recommended).
* **Libraries**: `libreadline` (used for input handling, history, and autocomplete).

##  Installation & Compilation

1. **Install Dependencies** (Debian/Ubuntu):
```bash
sudo apt-get update
sudo apt-get install libreadline-dev

```


2. **Clone/Download the source code** into a file named `main.cpp`.
3. **Compile**:
Link against the readline library during compilation.
```bash
g++ main.cpp -o PraSH -lreadline

```


4. **Run**:
```bash
./PraSH

```



##  Usage Examples

**Navigation & Execution:**

```bash
$ pwd
/home/user
$ cd Documents
$ ls -la

```

**Redirection:**

```bash
$ echo "Error Log" 2> error.txt
$ ls -l > filelist.txt
$ cat main.cpp >> backup.cpp

```

**Pipelining:**

```bash
$ cat file.txt | grep "search_term" | wc -l

```

**History Management:**

```bash
$ history        # List all history
$ history 5      # List last 5 commands
$ history -w     # Write current history to file

```

## Implementation Details

This shell is built using low-level C/C++ system programming concepts:

* **Process Creation**: `fork()` is used to create child processes for command execution. The parent process waits for the child using `waitpid()`.
* **Execution**: `execv()` replaces the child process image with the requested program.
* **File Descriptors**: `dup2()` is utilized to clone file descriptors for redirecting `STDOUT_FILENO` and `STDERR_FILENO` to files or pipes.
* **Inter-Process Communication**: `pipe()` creates unidirectional data channels between chained commands in a pipeline.
* **Signal & Memory Safety**: Includes proper cleanup of allocated memory for readline history and undo lists to prevent leaks.

