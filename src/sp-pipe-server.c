/**
 * A @code sp-pipe-server is the server side of a minimal implementation 
 * of the mechanics of the game n puzzle. The server will build and maintain the game state.  
 * 
 * @author Adam Khoukhi
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sp-pipe-server.h"

extern int client_to_server[2]; // Sends to server & reads from client
extern int server_to_client[2]; // Sends to client & reads from server


int **gameboard; // Original gameboard to be modified and used

/**
 * Initializes the gameboard and fills the tile slots with values
 * @param old_size the size of the old board to deallocate
 * @param size the size of the square matrix (gameboard)
 * @return whether the gameboard was initialized successfully
 */
bool initialization(int old_size, int size){
    if(size > 10 || size < 2){
        return false;     
    }else{
    if(!(old_size == 37826)) // if it isnt the dummy number, then it's not the inital initialization
        deallocate(old_size); // deallocate old board
    int tile_number = (size*size)-1; // Largest tile value in a nxn gameboard
    printf("Setting up the game\n");
    gameboard = malloc(sizeof(int*) * size); 
    if(gameboard == NULL) return false;
    for(int i = 0; i<size; i++){
        gameboard[i] = malloc(sizeof(int) * size);
        if(gameboard[i] == NULL) return false;
    }
    for(int i = 0; i<size;i++){
        for(int j = 0;j<size;j++){
            gameboard[i][j] = tile_number--; //Populates the board to allow for shuffling tiles
        }
    }
    shuffle_tiles(size); // to randomize the board
    return true;
    }
}

/**
 * Shuffles the positions of the tiles in the gameboard
 * @param size the size of the square matrix (gameboard)
 */
void shuffle_tiles(int size){
    srand(time(NULL));
    for(int i = 0; i<(size*size);i++){ // Arbitrary shuffling size, O(n)
        moveTile((rand()%(size*size)),size); // swaps a random tiles with the empty slot (second parameter is not used in the shuffling)
    }
}

/**
 * Frees the memory used by the gameboard
 * @param size the size of the square matrix (gameboard)
 */
void deallocate(int size){
    for(int i = 0; i<size; i++){
        free(gameboard[i]); // frees the memory pointed to by *gameboard
    }
    free(gameboard); // frees the memory pointed to by gameboard
}

/**
 * Returns an encrypted version of the tile's entry location
 * @param tile the value of the tile entry
 * @param size the size of the square matrix (gameboard)
 * @return an encoded version of the tile's location
 */ 
int getTileLocation(int tile, int size){
    int i, j; // i hat and j hat of the matrix entries
    for(i = 0; i<size;i++){
        for(j = 0;j<size;j++){
            if(tile == gameboard[i][j]) // Finds the i and j coordinates of the entry
                return ((i*10)+j); // merge the two components into a number that is decryptable
        }
    }
    return -1; // avoid a warning 
}

/**
 * Checks whether the inputted tile move is permissible
 * @param tile the value of the tile entry
 * @param size the size of the square matrix (gameboard)
 * @return true if the move valid and false otherwise
 */
bool isMoveValid(int tile, int size){
    if(tile < 1 || tile > (size*size)-1) // exceeds the lower and upper bounds of the gameboard
        return false;
    else{
        int tile_slot = getTileLocation(tile, size);
        int empty_slot = getTileLocation(0, size);
        int row_diff = abs((tile_slot/10) - (empty_slot/10)); // the row distance between the tile and the empty slot
        int column_diff = abs((tile_slot%10) - (empty_slot%10)); // the column distance between the tile and the empty slot
        if(((row_diff == 1) ^ (column_diff == 1)) && row_diff < 2 && column_diff < 2) // move cannot be diagonal, either vertical or horizental. Move must also only be 1 row or 1 column away
            return true;
        else
            return false;
    }
}

/**
 * swaps the inputted tile and empty slot entries in the gameboard
 * @param tile the value to swap with 0
 * @param size the size of the square matrix (gameboard)
 */
void moveTile(int tile, int size){
    int tile_slot = getTileLocation(tile, size);
    int empty_slot = getTileLocation(0, size);
    gameboard[tile_slot/10][tile_slot%10] = 0; // decrypts and assigns 0 as the value of the inpuuted tile entry's location
    gameboard[empty_slot/10][empty_slot%10] = tile; // decrypts and assigns the tile's entry as the value of the 0 entry's location
}

/**
 * Prompts the user that the game has ended
 * @param size the size of the square matrix (gameboard)
 */
void teardown(int size){
    deallocate(size); // frees the memory allocated for gameboard since the game is over
}

/**
 * Checks whether the current matrix is ordered correctly
 * @param size the size of the square matrix (gameboard)
 * @return true if gameboard is in win mode, false otherwise
 */
bool checkForWin(int size){
    int expected_value = 1; // every board starts with 1
    for(int i = 0; i<size;i++){
        for(int j = 0;j<size;j++){
            if(gameboard[i][j] != 0){ // empty slot can be anywhere so skip over it
                if(gameboard[i][j] == expected_value) expected_value++; // if the value matches then increment to the next expected value
                else return false;
            }
        }
    }
    return true; // passed the entire loop
}

/**
 * Saves the current game state in a file so the user's progress isn't lost
 * @param filename a pointer to the name of the file
 * @param size the size of the square matrix (gameboard)
 * @return true if save was a success, false otherwise
 */
bool save(char *filename, int size){
    FILE *fp = fopen(filename,"w");
    if(fp == NULL) return false;
    fprintf(fp,"%d\n", size); // so we can retrive the size of the gameboard when loading
    for(int i = 0; i<size;i++){
        for(int j = 0;j<size;j++){
            int checkForError = fprintf(fp,"%d\n", gameboard[i][j]);
            if(checkForError < 0) return false;
        }
    }
    fclose(fp);
    return true;
}

/**
 * Loads a saved game state as the current game state
 * @param filename a pointer to the name of the file
 * @param size pointer to the size of the gameboard
 * @return true if load was a success, false otherwise
 */
bool load(char *filename, int *size){
    FILE *fp = fopen(filename, "r");
    if(fp  == NULL) return false;
    int new_size;
    fscanf(fp, "%d\n", &new_size); // first line of file is the size of the gameboard
    initialization(*size,new_size);
    int current_tile_number;
    for(int i = 0; i<new_size;i++){
        for(int j = 0;j<new_size;j++){
            fscanf(fp, "%d\n", &current_tile_number); // file formatted so that every line has a tile
            gameboard[i][j] = current_tile_number;
        }
    }
    *size = new_size; // update the size of the gameboard
    fclose(fp);
    return true;
}

/**
 * Retrieves data from the client and responds accordingly
 */
void init_server(){
    int size;
    size = 4;  // inital size of the gameboard as requested
    initialization(37826,size); // since there is no memory to deallocate for the first initialization, we used a dummy int to let the function know there is no need to deallocate memoroy
    int command;
    int bytes_read;
    while(1){ // while the client hasnt quit
        bytes_read = read(client_to_server[0], &command, sizeof(command)); // reads in the type of request
        if(bytes_read == 0){ // clue that the user has ended
            teardown(size);
            exit(0);
        }
        switch(command){
            case 0: // client requested for a new board
            {
                int temp_size;
                read(client_to_server[0], &temp_size, sizeof(int)); // size of the new board
                bool result;
                if(initialization(size, temp_size)){
                    size = temp_size;
                    result = true;
                }else
                    result = false;
                write(server_to_client[1], &result, sizeof(result)); // the result status of the initialization
                break;
            }
            case 1: // client requested to move a tile 
            {
                int tile;
                read(client_to_server[0], &tile, sizeof(int));
                bool result;
                if(isMoveValid(tile, size)){ // checks whether the move is valid before swaping the entries
                    moveTile(tile, size);
                    result = true;
                }
                else
                    result = false;
                write(server_to_client[1], &result, sizeof(result));
                break;
            }
            case 2: // client requested to load progress
            {
                char filename[100];
                read(client_to_server[0], filename, sizeof(filename));
                bool result;
                if(load(filename, &size)) // pass in pointer to size so that it can update it
                    result = true;
                else 
                    result = false;
                write(server_to_client[1], &result, sizeof(result));
                break;
            }
            case 3: // client requested to save progress
            {
                char filename[100];
                read(client_to_server[0], filename, sizeof(filename));
                bool result;
                if(save(filename, size)) 
                    result = true;
                else 
                    result = false;
                write(server_to_client[1], &result, sizeof(result));
                break;
            }
            case 4: // checks whether the user has won
            {
                bool result;
                if(checkForWin(size)){
                    initialization(size, size);
                    result = true;
                }
                else 
                    result = false;
                write(server_to_client[1], &result, sizeof(result));
                break;
            }
            case 5: // client requested to view his current gameboard
            {
                int vector[(size*size)]; // to send the entries of the gameboard in a 1d array
                int vector_index = 0;
                write(server_to_client[1], &size, sizeof(size)); // let the client side know the size of the gameboard beforehand to prepare for the correct size
                for(int i = 0; i<size;i++){ // populate the 1d array with the entries
                    for(int j = 0;j<size;j++){
                        vector[vector_index++] = gameboard[i][j];
                    }
                } 
                write(server_to_client[1], vector, sizeof(vector));
                break;
            }
            default:
                break;
        }
    }
}

/**
 * Called in the main unit to initalize the server side of the game
 */
void server(){
    close(client_to_server[1]); // server won't use the write side of client
    close(server_to_client[0]); // server won't use the read side of the client
    init_server();
}
