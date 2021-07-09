// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#define PORT 8000
#define READLIMIT 4096
#define CHUNK 1000000

int parseBySpace(char* typed_cmnd , char** stor_list)
{

    char* token;
    token = strtok(typed_cmnd , " \t");

    int ptr = 0;
    while(token != NULL)
    {

        stor_list[ptr] = token;
        //printf("token number %d is %s\n", ptr+1,token);
        ptr++;
        token = strtok(NULL," \t");
    }


    return ptr;
}

int min(int a , int b)
{
    if(a < b)
        return a;

    return b;
}


int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server!\n");

    char typed_line[READLIMIT]; 
    //char ack_buffer[5];
    char typed_copy[READLIMIT]; 
    char* args_list[256];
    int fd;
    char file_path[1024];
    char data_rec[CHUNK+1];
    char data_chunk[CHUNK+1];
    //char finish[5];
    char chunk_read[5];
    //char pause[5];
    double perc;
    int done = 0;
    char len_info[15];
    char len_ack[5];

    while(1)
    {
        printf("\033[1;34mCLIENT>> \033[0m");

        if(!fgets(typed_line , READLIMIT+1 , stdin)) //takinf user input
            break;

        int len = strlen(typed_line);
        if(typed_line[len-1] == '\n')
            typed_line[len-1] = '\0';

        bzero(typed_copy , READLIMIT);

        strcpy(typed_copy , typed_line);

        int num_args = parseBySpace(typed_copy , args_list);  //parsing

        if(num_args == 0)
        {
            continue;
        }

        if(!strcmp("get" , args_list[0]))
        {
            if(num_args == 1) //no filename entered
            {
                printf("Please enter atleast 1 file name\n");
                continue;
            }

            for(int i=1;i<num_args;i++)
            {

                done = 0;
                fd = open(args_list[i] , O_WRONLY);
                if(fd != -1)
                {
                    printf("File already exists\n");
                    close(fd);
                    continue;
                }

                send(sock , args_list[i] , strlen(args_list[i]) , 0 );  // send the filename.
                printf("File request for %s sent\n" , args_list[i]);

                bzero(len_info , 15);

                valread = recv( sock , len_info, 15 , 0);  // recieve the length info
                if(!valread)
                {
                    printf("Connection from server closed abruptly\n");
                    return -1;
                }

                bzero(len_ack , 5);
                sprintf(len_ack , "1");

                send(sock , len_ack , strlen(len_ack) , 0); //sedning the length acknowledgement

                if(!strcmp("-1" , len_info))
                {
                    char done[6];
                    bzero(done , 6);

                    printf("File %s could not be found\n" , args_list[i]);

                    recv(sock , done , 6 , 0);
                }

                else
                {
                    bzero(file_path , 1024);

                    printf("File %s found and is being downloaded\n" , args_list[i]);

                    sprintf(file_path , "./%s" , args_list[i]);



                    fd = open(file_path , O_WRONLY | O_TRUNC | O_CREAT , 0644); //creating the file
                    if(fd == -1)
                    {
                        perror("open");
                        continue;
                    }

                    int cnt = 0;

                    int tot_len = atoi(len_info);

                    int bytes_to_be_read;
                    while(done < tot_len) //while the whole file is read
                    {
                        bzero(data_rec , CHUNK+1);
                        bytes_to_be_read = min(CHUNK , tot_len - done);
                        strcpy(data_rec , "");

                        int bytes_read = 0;
                        int loc_cnt = 0;
                        int loc_read = 0;
                        while(bytes_read < bytes_to_be_read) //reading the chunk of bytes from the server
                        {
                            bzero(data_chunk , CHUNK+1);
                            loc_read = recv(sock , data_chunk , CHUNK , 0);
                            if(loc_read == 0)
                            {
                                printf("Connection closed abruptly   \n");
                                return -1;
                            }
                            loc_cnt++;
                            bytes_read += loc_read;
                            strcat(data_rec , data_chunk);
                            
                        }


                        cnt++;

                        bzero(chunk_read , 5);
                        sprintf(chunk_read , "1");
                        send(sock , chunk_read , strlen(chunk_read) , 0);  //send the acknowledgement of the chunk

                        int bytes_written = write(fd , data_rec , strlen(data_rec)); //writing into the file

                        done = done + bytes_read;

                        perc = (double)done / tot_len;

                        perc = perc*100;
                        printf("%.2lf percent file downloaded\r" , perc);

                        fflush(stdout);

                    }

                    close(fd);
                    printf("Download of %s complete!\n\n" , args_list[i]);

                    char done[6];
                    bzero(done , 6);
                    recv(sock , done , 6 , 0);

                }

            }
              
        }

        else if(!strcmp("exit" , args_list[0]))
        {
            close(sock); //closing the connection
            return 0;
        }
        else
        {
            printf("Invalid command!\n");
        }

    }
   
    return 0;
}
