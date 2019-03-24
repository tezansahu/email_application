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

#include <map>
#include <iterator>
#include <regex>

using namespace std;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *args[]){
    // Check the correct number of arguements
    if(argc != 4){
        cerr<<"Incorrect number of arguments passed!\n";
        return 1;
    }

    const char* portNum = args[1];
    const char* passwdfile = args[2];
    const char* file_path = args[3];

    string check_file_path = string("test -d ")+string(file_path);
    if(system(check_file_path.c_str()) !=0 ){
        cerr<<"Unable to access the user-database\n";
        exit(4);
    }
    

    // Open the password file and try to read the data and store it in a mapping
    ifstream file;
    map<string, string> data;
    map<string, string>::iterator itr;
    try{
        file.open(passwdfile);
        if(file.good() == 0){
            cerr<<"Unable to open password file\n";
            return 3;
        }
        while(file){
            char line[100];
            file.getline(line, 100);
            char* token = strtok(line, " ");
            const char* _tokens[2];
            int i = 0;
            for(int i=0;token!=NULL;i++){
                _tokens[i] = token;
                token = strtok(NULL," ");
            }
            data.insert(pair<string, string>(string(_tokens[0]), string(_tokens[1])));
        }
        
    }
    catch(exception& e){
        cerr<<e.what()<<"\n";
        return 3;
    }

    

    // Define the server socket
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM,0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(portNum));
    
    // Bind the server socket to the port entered
    if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address))<0){
        cerr<<"Bind Unsuccessful\n";
        exit(2);
    }
    cout<<"Bind Done: "<<portNum<<"\n";

    
    // Start listening on the port for connections
    if(listen(server_socket,10)<0){
        cerr<<"Listen Unsuccessful\n";
        exit(2);
    }
    cout<<"Listen Done: "<<portNum<<"\n";

    int client_socket;

    while(1){
        socklen_t sin_size = sizeof their_addr;

        // Accept connection from a client
        client_socket = accept(server_socket,(struct sockaddr *)&their_addr, &sin_size);

        if(client_socket < 0){
            cerr<<"Accept error";
            continue;
        }

        // Get the details of the client connected
        char response[4096] = "\0";
        inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        
        // Receive the login details from the client
        recv(client_socket, response, sizeof(response),0);
        printf("Client: %s:%s\n", s,portNum);

        // Compare the format of message received 
        regex r("^User: .* Pass: .*$");
        if(! regex_match(response, r)){
            cout<<"Unknown Command\n";
            close(client_socket);
            continue;
        }

        // Get the username and password from the message received
        char* token = strtok(response, " ");
        const char* _tokens[4];
        for(int i=0;token!=NULL;i++){
            _tokens[i] = token;
            token = strtok(NULL," ");
        }
        const char* input_userName = _tokens[1];
        const char* input_passwd = _tokens[3];

        // Check if the username is valid
        itr = data.find(input_userName);
        if(itr == data.end()){
            string error  = "Invalid User\n";
            cout<<error;
            send(client_socket, error.c_str(), strlen(error.c_str()), 0);
            close(client_socket);
            continue;
        }

        // Check if the password is correct
        if(itr->second != input_passwd){
            string error  = "Wrong Password\n";
            cout<<error;
            send(client_socket, error.c_str(), strlen(error.c_str()), 0);
            close(client_socket);
            continue;
        }

        // Welcome the client
        string welcome_msg = string("Welcome ")+string(input_userName)+string("\n");
        cout<<welcome_msg;
        send(client_socket, welcome_msg.c_str(), strlen(welcome_msg.c_str()), 0);


        int quit_connection = 0;
        while(quit_connection == 0){    
            // Get the next message from the client
            char response2[4096] = "\0";
            recv(client_socket, response2, sizeof(response2),0);

            if(strcmp(response2, "LIST") == 0){

                // Use system calls to get the number of messages for the user
                string user_folder = string(file_path) + string("/") + string(input_userName);
                string s = string("test -d ")+string(user_folder);
                if(system(check_file_path.c_str()) !=0 ){
                    string error = string(input_userName) + string(": Folder Read Fail\n");
                    cout<<error;
                    send(client_socket, error.c_str(), strlen(error.c_str()), 0);
                    close(client_socket);
                    continue;
                }
                s = string("touch temp.txt && find ") + string(user_folder) + string(" -type f | wc -l > temp.txt");
                system(s.c_str());
                ifstream file;
                file.open("temp.txt");
                char num_messages[10];
                file.getline(num_messages, 10); 
                file.close();
                s = "rm -rf temp.txt";
                system(s.c_str());

                string response_num_msg = string(input_userName) + string(": No. of messages ") + string(num_messages) + string("\n");
                cout<<response_num_msg;
                send(client_socket, response_num_msg.c_str(), strlen(response_num_msg.c_str()), 0);
            }
            // If the client sends 'quit' message, logout and close connections
            else if(strcmp(response2, "quit") == 0){
                quit_connection = 1;
                string logout_msg = string("Bye ")+string(input_userName)+string("\n");
                cout<<logout_msg;
                send(client_socket, logout_msg.c_str(), strlen(logout_msg.c_str()), 0);
                close(client_socket);
                continue;
            }
            else{
                string logout_msg = string("Unknown Command\n");
                cout<<logout_msg;
                send(client_socket, logout_msg.c_str(), strlen(logout_msg.c_str()), 0);
                close(client_socket);
                continue;
            }
        }

        cout<<"\n";
        
    }
    close(server_socket);

    file.close();
    return 0;
}