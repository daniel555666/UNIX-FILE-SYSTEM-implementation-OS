/**
 * we get help from:
 * https://www.youtube.com/watch?v=n2AAhiujAqs&ab_channel=drdelhart
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 128
#define BLOCKSIZE 512
#define MAX_FILES 10000
#define NAME_SIZE 8
#define MAX_DIR_SIZE 10

typedef struct dir 
{
    int n;
    char *d_name;
} myDIR;
struct super_block
{
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode
{
    int size;
    int first_block;
    int dir; // 0 for file 1 for dir
    char name[NAME_SIZE + 1];
    int real_size;
};


struct disk_block
{
    int next_block_num;
    char data[BLOCKSIZE];
};

struct open_file 
{
    int fd;
    int pos; 
};
struct mydirent { 
    int size;
    int fds[MAX_DIR_SIZE];
    char d_name[NAME_SIZE+1];
};
struct open_file myopenfile[MAX_FILES];
struct super_block sb;
struct inode *inodes;
struct disk_block *dbs;

void mymkfs(int size);
int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
size_t myread(int myfd, void *buf, size_t count);
size_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
struct mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);


void print_fs(); // print out info about file system
