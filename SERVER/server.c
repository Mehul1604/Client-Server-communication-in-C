#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#define PORT 8000
#define CHUNK 1000000


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);
    
    char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is up and running and is now listening..\n");

    int client_closed = 0;

    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0) // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv              
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Recieved a request\n\n");

        client_closed = 0;

        char file_name[1024];
        //char finish_ack[5];
        char data[CHUNK+1];
        char chunk_ack[5];
        //char pause[5];
        char len_info[15];
        char len_ack[5];

        while(1)
        {
           
            
            printf("Server waiting to read..\n");
            bzero(file_name ,1024);

            valread = recv(new_socket , file_name, 1024 ,0);  //recieve the file name from the client
            if(valread < 0)
            {
                printf("Read error\n");
                exit(EXIT_FAILURE);
            }

            printf("Filename read by server\n");

            if(valread == 0)
            {
                printf("Client closed connection!\n");
                client_closed = 1;
                break;
            }

            printf("%s\n",file_name);
            

            int fd = open(file_name , O_RDONLY);

            printf("Opening file %s\n" , file_name);

            bzero(len_info , 15);

            if(fd == -1)
            {
                printf("Not Found!\n");
                sprintf(len_info , "-1");

                send(new_socket , len_info , strlen(len_info) , 0);  //sending -1 as length to indicate file not found

                bzero(len_ack , 5);

                recv(new_socket , len_ack , 5 , 0);

                send(new_socket , "DONE" , 4 , 0);

                continue;

            }


            int tot,offset;
            tot = lseek(fd , 0 , SEEK_END);
            
            offset = lseek(fd , 0 , SEEK_SET);

            int cnt = 0;

            bzero(len_info , 10);
            sprintf(len_info , "%d" , tot);
            send(new_socket , len_info , strlen(len_info) , 0);  //seding the length of file in bytes

            bzero(len_ack , 5);

            recv(new_socket , len_ack , 5 , 0); //recieving the acknowledgement of length

            printf("Sending data of file %s to client\n" , file_name);

            while(offset < tot) //while the whole file is read
            {
               
                bzero(data , CHUNK+1);

                int read_bytes = read(fd , data , CHUNK); //read from the file
                offset = lseek(fd , 0 , SEEK_CUR);

                int sent_bytes = send(new_socket , data , strlen(data) , 0); //send a chunk to client
                cnt++;

                bzero(chunk_ack , 5);

                recv(new_socket , chunk_ack , 1 , 0); //recieve the acknowledgement for the chunk
            
            }

            printf("Data completely sent\n");

            close(fd);

            send(new_socket , "DONE" , 4 , 0);
            

        }

        if(client_closed)
            continue;
    }

    close(server_fd); //close the socket 
    
    return 0;
}
