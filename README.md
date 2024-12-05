### Duo: João Pedro PENELU and Théo FARIAS
### Professor: Christophe Barès

## TP 1: ENSEA in the Shell

### Objective
This project implements a simplified shell for executing system commands with additional functionalities such as measuring execution time, displaying return codes, and handling redirections (`<` and `>`). The shell provides a user-friendly prompt to interact with these features.

---

### Features
1. Display a custom prompt with the status of the last executed command.
2. Execute system commands with or without arguments.
3. Measure and display execution time for commands.
4. Handle input redirection (`<`) and output redirection (`>`).
5. Provide detailed error handling for system calls and invalid commands.
6. Exit the shell gracefully using the `exit` command or `<Ctrl+D>`.

---

### Instructions

#### 1. Compilation
To compile the shell program:
```bash
gcc -o enseash enseash.c
```

#### 2. Running the Shell
Execute the compiled program:
```bash
./enseash
```

#### 3. Interacting with the Shell
- The prompt will display as:
  ```
  enseash %
  ```
- You can type commands as you would in a regular shell, for example:
  ```bash
  ls
  pwd
  fortune
  ```

#### 4. Special Features
- **Input Redirection (`<`)**:
  ```bash
  wc -l < input.txt
  ```
- **Output Redirection (`>`)**:
  ```bash
  ls > output.txt
  ```
- **Combined Redirection**:
  ```bash
  grep "search_term" < input.txt > output.txt
  ```

#### 5. Exiting the Shell
- Type `exit`:
  ```bash
  enseash % exit
  ```
- Or press `<Ctrl+D>`.

---

### Notes
- Ensure necessary programs (`ls`, `wc`, `fortune`, etc.) are installed on your system.
- The shell adheres to DRY principles with modular functions for clarity and maintainability.
- Every system call is error-checked to prevent crashes and provide meaningful feedback.

---

### Example Usage
```bash
$ ./enseash
Welcome to ENSEA Tiny Shell.
Type 'exit' to quit.
enseash % ls
file1.c  file2.txt
enseash [exit:0|2ms] % grep "hello" < file2.txt > output.txt
enseash [exit:0|3ms] % wc -l < output.txt
1
enseash [exit:0|1ms] % exit
Bye bye...
$
```

---

### Files
- `enseash.c`: Source code for the shell implementation.

---

### License
This project is for educational purposes under the guidance of Professor Christophe Barès at ENSEA. Redistribution is subject to academic regulations.
