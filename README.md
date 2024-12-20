### Duo: João Pedro PENELU and Théo FARIAS
### Professor: Christophe Barès

---

## TP 1: ENSEA in the Shell

### Objective
This project implements a simplified shell for executing system commands with additional functionalities such as measuring execution time, displaying return codes, and handling redirections (`<` and `>`). The shell provides a user-friendly prompt to interact with these features.

### Features
1. Display a custom prompt with the status of the last executed command.
2. Execute system commands with or without arguments.
3. Measure and display execution time for commands.
4. Handle input redirection (`<`) and output redirection (`>`).
5. Provide detailed error handling for system calls and invalid commands.
6. Exit the shell gracefully using the `exit` command or `<Ctrl+D>`.

---

## TP 2: TFTP Client

### Objective
This project implements a simplified TFTP client capable of sending and receiving files using the Trivial File Transfer Protocol.
---

### Features
1. **gettftp**:
   - Requests files from a TFTP server using the RRQ (Read Request) operation.
   - Handles file reception in blocks of configurable sizes.
   - Sends ACKs (Acknowledgments) for each block received.
   - Reports the completion of file transfer or any errors encountered.

2. **puttftp**:
   - Sends files to a TFTP server using the WRQ (Write Request) operation.
   - Handles file transmission in blocks of configurable sizes.
   - Waits for ACKs (Acknowledgments) for each block sent.
   - Reports the completion of file transfer or any errors encountered.
---

### Instructions

#### Compilation
To compile the TFTP client programs:
```bash
gcc -o gettftp gettftp.c
gcc -o puttftp puttftp.c
```

#### Running the Programs
- **gettftp**:
  ```bash
  ./gettftp <server> <file>
  ```
  Example:
  ```bash
  ./gettftp localhost testfile.txt
  ```

- **puttftp**:
  ```bash
  ./puttftp <server> <file>
  ```
  Example:
  ```bash
  ./puttftp localhost testfile.txt
  ```

---

### Example Usage

#### gettftp:
```bash
$ ./gettftp localhost testfile.txt
Sending RRQ...
RRQ packet built
RRQ sent to server
Received DATA block 1, size 512 bytes
Received DATA block 2, size 512 bytes
File transfer complete
```

#### puttftp:
```bash
$ ./puttftp localhost testfile.txt
Sending WRQ...
WRQ packet built
WRQ sent to server
ACK received for block 0
Sent DATA block 1, size 512 bytes
Sent DATA block 2, size 512 bytes
File transfer complete
```

---

### Notes
- Ensure a compatible TFTP server is running and accessible on the specified port (default: 1069).
- The programs are designed for educational purposes and adhere to TFTP protocol simplicity.
- Error messages are logged on the terminal for easier debugging and testing.

---

### Files
- `gettftp.c`: Source code for the TFTP file reception client.
- `puttftp.c`: Source code for the TFTP file transmission client.

---

### License
This project is for educational purposes under the guidance of Professor Christophe Barès at ENSEA. Redistribution is subject to academic regulations.