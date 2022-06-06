#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 128
#define BLOCKSIZE 512

struct superblock
{
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode
{
    int size;
    int first_block;
    char name[9];
};

struct disk_block
{
    int next_block_num;
    char data[BLOCKSIZE];
};

void create_fs(); // initialize new filesysyem
void mount_fs();  // load adile system
void sync_fs();   // write the file system

void print_fs(); // print out info about file system
void set_filesize(int filenum, int size);
void write_data(int filenum, int pos, char *data);
char* read_data(int filenum, int pos);
int allocte_file(char name[8]); // retrun file discriptor