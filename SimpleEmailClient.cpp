#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

using namespace std;

int main(int argc, char *args[]){

    // Check the correct number of arguements
    if(argc != 4){
        cerr<<"Incorrect number of arguments passed!\n";
        exit(1);
    }

    // Get IP Address and Port Number of destination address 
    char *address;
    address = args[1];
    char* token = strtok(address, ":");
    const char* ip_address;
    int portNum;
    const char* _tokens[2];
    for(int i=0;token!=NULL;i++){
        _tokens[i] = token;
        token = strtok(NULL,":");
    }
    ip_address = _tokens[0];
    portNum = atoi(_tokens[1]);

    // Get username and password
    const char* userName = args[2];
    const char* passwd = args[3];

    // Define the client socket
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM,0);

    // specify an address for the socket
    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(portNum);
    inet_aton(ip_address, &remote_address.sin_addr);

    // Connect to the remote server
    int connection_status = connect(client_socket, (struct sockaddr *) &remote_address, sizeof(remote_address));
    if(connection_status == -1){
        printf("There was an error making a connection to the remote server\n\n");
        exit(2);
    }
    cout<<"Connect Done: "<<address<<"\n";

    // Send login details to the server
    string request = (string("User: ")+ string(userName) + string(" Pass: ")+ string(passwd)+ string("\0")) ;
    char response[4096] = "\0";
    send(client_socket, request.c_str(), strlen(request.c_str()), 0);

    // Handle message received from the server appropriately
    if(recv(client_socket, response, sizeof(response), 0) >= 0){
        printf("%s\n",response);
        if(strcmp(response, (string("Welcome ")+string(userName)+ string("\n")).c_str()) != 0){
            close(client_socket);
            exit(1);
        }
    }
    else{
        close(client_socket);
        exit(1);
    }
    
   
    string list = string("LIST\0");
    send(client_socket,list.c_str(), strlen(list.c_str()),0);
    char response3[4096] = "\0";
    recv(client_socket, response3, sizeof(response3),0);
    printf("%s\n",response3);

     //After a brief pause, send a 'quit' request
    sleep(1);
    string quit = string("quit\0");
    send(client_socket,quit.c_str(), strlen(quit.c_str()),0);
    char response2[4096] = "\0";
    recv(client_socket, response2, sizeof(response2),0);
    close(client_socket);

    return 0;
}