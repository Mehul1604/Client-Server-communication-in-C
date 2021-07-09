# OSN Assignment 6

# **Socket Programming**

#### Mehul Mathur 2019101046

## Introduction
Two folders CLIENT and SERVER are present both containing client.c and server.c for client and server code respectively. This assignment makes use of the tcp protocol and its helper libraries in c to show a simple client-server CLI program which allows a client to download files from the server directory. 

### Commands
- ####  get < filename(s) >
User can request to download files using the get command. For eg. get < filename1 > < filename2 > will send the request for both these filename to the server (sequentially).
If the file is not available on the server , an acknowledgement of -1 is sent , which tells the client to give a valid name. Otherwise an acknowledgement is sent containing the length of the file in bytes. 
Then the download process is begun.

---
- #### exit
Type this command after the prompt to close the connection from the client side.
Note the server will keep on running and now go on to look for other connections.

--- 

### Running the code

 - Open two terminals.
 - In one terminal open the CLIENT folder and the other open the SERVER folder
 - First run the server.c in the SERVER folder by :- 
    - gcc server.c
    - ./a.out
 - After you see the "server is running.." message , in the CLIENT folder , run the client code and the start the CLI by :-
     - gcc client.c
     - ./a.out
 - Proceed to use the "get" command as above or close the connection and exit the CLI by the "exit" command. Ctrl C will also do the same thing.
 - To stop running the server code , press Ctrl C on the server terminal

---


### Assumptions / Points

- The program only supports single client and single server communication.
- In the code , each file is downloaded in chunks of 10^6 or less. 
- send() and recv() are used for socket communication
- The progress in percentage is displayed during the file download (this will be too fast too see for file sizes less than 500 MB).
- The program only works for non-binary files (only txt , .c and other similar text files). It does not support jpeg , pdf etc.

---









