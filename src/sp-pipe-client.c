/**
 * A @code sp-pipe-client is the client side of a minimal implementation of the 
 * mechanics of the game n puzzle. The client is responsible for user interface 
 * and interacting with the game server.
 * 
 * @author Adam Khoukhi
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include "sp-pipe-client.h"

extern int client_to_server[2]; // Sends to server & reads from client
extern int server_to_client[2]; // Sends to client & reads from server


/**
 * Traverses through the matrix and displays the entries in a user-friendly manner
 * @param matrix the matrix consisting of the gameboards entries
 * @param size the size of the square matrix (gameboard)
 */
void display(int *matrix,int size){
    for(int i = 0; i<(size*size);i++){
            int current_tile = matrix[i];
            if(!current_tile == 0) // Must display an empty slot for the user in the place of 0
                fprintf(stdout,((((i+1)%size == 0) && (i != 0)) ? "%3d\n" : "%3d"), current_tile); // to display a user friendly matrix
            else
                fprintf(stdout,((((i+1)%size == 0) && (i != 0)) ? "   \n" : "   "));
        }
        fprintf(stdout,"\n");
    }

/**
 * The game loop from the client side. Handles the user interaction 
 * and the sending and retrieving of data from the server.
*/
void init_client(){
    enum command {cmd_new, cmd_move, cmd_load, cmd_save, cmd_won, cmd_retrieve}; // enum values for the different requests
    int loop_status = 1; // status set to true for the game loop to proceed
    while(loop_status){
        enum command win_cmd = cmd_won;
        write(client_to_server[1], &win_cmd, sizeof(win_cmd)); // sends a request to the server to check if the user has won
        bool won;
        read(server_to_client[0], &won, sizeof(won)); // reads back wether the user has won
        if(won)
            fprintf(stdout,"\nWinner winner Chicken Dinner.\n");     
          
        fprintf(stdout,"Menu: [h]elp [n]ew, [p]rint, [m]ove, [s]ave, [l]oad, [q]uit? ");
        char input;
        scanf(" %c", &input);
        switch(input){
            case 'n':
            {
                fprintf(stdout,"Please input an integer (2-10) for the size of your gameboard!\n");
                int temp_size;
                if (1 == scanf("%d", &temp_size)) { // if the input was an int
                    enum command cmd = cmd_new;
                    write(client_to_server[1], &cmd, sizeof(cmd));
                    write(client_to_server[1], &temp_size, sizeof(temp_size));
                    bool result;
                    read(server_to_client[0], &result, sizeof(result));
                    if(result){
                        fprintf(stdout,"New board Successfully created.\n");                        
                    }else{
                        fprintf(stderr,"An Error Occurred. Please try again later.\n");
                    }
                }
                else {
                    getchar(); // skip over the char if the user inputs a char
                    fprintf(stderr,"Invalid gameboard size. Size must be between 2 & 10.\n");
                }    
                break;
            }
            case 'p': 
            {
                fprintf(stdout,"\n Current Game Board.... \n");
                enum command cmd = cmd_retrieve;
                write(client_to_server[1], &cmd, sizeof(cmd));
                int curr_size;
                read(server_to_client[0], &curr_size, sizeof(curr_size)); // first read in the size of the current board inorder to know how large the vector will be
                int matrix_board[(curr_size*curr_size)]; // size squared is the number of entries in the board
                read(server_to_client[0], &matrix_board, sizeof(matrix_board)); 
                display(matrix_board,curr_size); // method for displaying the board
                break;
            }
            case 'h':
            {
                printf("\n--------------------- Game Manual ---------------------\n\
                [h]elp:  Displays the manual for the menu options \n\
                [n]ew:   Prompts for a size (1 integer) and restarts the game with a new gameboard of the size inputted \n\
                [p]rint: Displays the current game state\n\
                [m]ove:  Prompts for a tile to move and moves it if permissible \n\
                [s]ave:  Saves the current state of the game\n\
                [l]oad:  Loads a previously saved game state\n\
                [q]uit:  Quit the game\n\
                -------------------------------------------------------\n");
                break;
            }
            case 'm':
            {
                fprintf(stdout,"Which tile would you like to move? ");
                enum command cmd = cmd_move;
                write(client_to_server[1], &cmd, sizeof(cmd));
                int tile;
                scanf("%d", &tile);
                write(client_to_server[1], &tile, sizeof(tile)); // write to the server the tile num to move
                bool result;
                read(server_to_client[0], &result, sizeof(result));  // read the result status of the move
                if(result){
                    fprintf(stdout,"Tile Successfully Moved\n");                        
                }else{
                    fprintf(stderr,"Invalid Tile move\n");
                }
                break;
            }
            case 's':
            {
                fprintf(stdout,"Input filename (99 characters max)\n");
                char input_filename[100]; // the name of the file to be saved
                enum command cmd = cmd_save;
                write(client_to_server[1], &cmd, sizeof(cmd));
                scanf("%s", input_filename);
                write(client_to_server[1], &input_filename, sizeof(input_filename)); // let the server know what to name the new the file to be saved
                bool result;
                read(server_to_client[0], &result, sizeof(result));
                if(result){
                    fprintf(stdout,"Progress Successfully Saved\n");                        
                }else{
                    fprintf(stderr,"An Error Occurred with saving the file\n");
                }
                break;
            }
            case 'l':
            {
                fprintf(stdout,"Input filename (99 characters max)\n");
                char input_filename[100]; // the name of the file to be loaded
                enum command cmd = cmd_load;
                write(client_to_server[1], &cmd, sizeof(cmd));
                scanf("%s", input_filename);
                write(client_to_server[1], &input_filename, sizeof(input_filename));// let the server know what the name of the file to be loaded is
                bool result;
                read(server_to_client[0], &result, sizeof(result));
                if(result){
                    fprintf(stdout,"Progress Successfully Loaded\n");                        
                }else{
                    fprintf(stderr,"An Error Occurred with loading the file\n");
                }
                break;
            }
            case 'q':
            {
                loop_status = 0; // status set to false to end the loop
                printf("Ending the game\n");
                break;
            }
            default:
            {
                fprintf(stdout,"Invalid input! Please enter one of the characters in the menu option...\n");
                break;
            }
        }
    }
    exit(0);
}


/**
 * Called in the main unit to initalize the client side of the game
 */
void client(){
    close(client_to_server[0]); // client won't use the read side of the server
    close(server_to_client[1]); // client won't use the write side of server
    init_client();
}
