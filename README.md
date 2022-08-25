# MTL458 Assignment 1 - Building a shell

### Running the shell
```
make all
```
Alternatively,
```
clang/gcc shell.c -o shell
./shell
```

### Shell commands
- **Changing directory:** 
  - Working directory of the shell can be changed using `cd [path_to_directory]`, where `cd ~` can be used to change to home directory.
- **Exiting the shell:** The shell can be exited by pressing Ctrl+C or using the command `exit`.
- **Background processes:** A process can be run in the background by prepending `&` to the command, i.e. `&cmd1 arg1 arg2 ... argn`.
- **Piped commands:** Two commands can be piped together in the format `cmd1 arg1 arg2 ... argn | cmd2 arg1 arg2 ... argn`.
- **Environment variables:**
  - Environment variables can be declared using the format `var=value`.
  - Can be used in subsequent shell commands in the format `cmd $var`.
- **Command history:** The last 5 commands executed in the shell can be fetched by `cmd_history`.
- **Process history:** A list of all processes created by the shell along with their status can be retrieved using `ps_history`.
