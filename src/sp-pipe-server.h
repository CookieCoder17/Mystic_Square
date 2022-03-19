#ifndef SP_PIPE_SERVER
#define SP_PIPE_SERVER

#include <stdbool.h>
bool initialization(int old_size, int size);
int getTileLocation(int tile, int size);
bool isMoveValid(int tile, int size);
void moveTile(int tile, int size);
void teardown(int size);
bool checkForWin(int size);
void deallocate(int size);
void shuffle_tiles(int size);
bool save(char *filename, int size);
bool load(char *filename, int *size);
void init_server();
void server();

#endif
