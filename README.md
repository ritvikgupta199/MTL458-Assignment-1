# MTL458 Assignment1

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
- Changing directory: Working directory of the shell can be changed using `cd [path_to_directory]`.
- `cmd_history`: Get the last 5 commands executed on the shell
- `ps_history`: Get a list of all processes created by the shell along with their status
- Exiting the shell: The shell can be exited by pressing Ctrl+C or using the command `exit`
- Background processes: A process can be run in the background by prepending `&` to the command, i.e. `&cmd1 arg1 arg2 ... argn` 
- Piped commands: Two commands can be piped together in the format `cmd1 arg1 arg2 ... argn | cmd2 arg1 arg2 ... argn`
- Environment variables: 
  - Environment variables can be declared using the format `var=value` 
  - Can be used in subsequent shell commands in the format `cmd $var`.