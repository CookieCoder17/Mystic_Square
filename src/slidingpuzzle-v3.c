/**
 * A @code slidingpuzzle is a minimal implementation of the mechanics
 * of the game n puzzle using a simple text interface as the gui
 * 
 * @author Adam Khoukhi
 * @version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sp-pipe-client.h"
#include "sp-pipe-server.h"

int client_to_server[2]; // Take care of the transactions from client to server
int server_to_client[2]; // Take care of the transactions from server to client


int main(){
    if(pipe(client_to_server) == -1 || pipe(server_to_client) == -1){ // Incase either of the pipes fail
        fprintf(stderr,"Oops.. An error occurred. Please try again later.\n");
        exit(1);
    }
    pid_t child = fork();
    if(child < 0){ // Incase forking fails 
        fprintf(stderr, "Oops.. An error occurred. Please try again later.\n");
        exit(1);
    }
    if(child == 0){ // Child process aka the client
        client();
    }else{
        server();
    }
}
