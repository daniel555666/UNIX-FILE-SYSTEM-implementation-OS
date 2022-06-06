#include "file_system.h"
struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;

void create_fs()
{
    sb.num_inodes = 10;
    sb.num_blocks = 100;
    sb.size_blocks = sizeof(struct disk_block);

    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    for (int i = 0; i < sb.num_inodes; i++)
    {
        inodes[i].size = -1;
        inodes[i].first_block = -1;
        strcpy(inodes[i].name, "");
    }
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);
    for (int i = 0; i < sb.num_blocks; i++)
    {
        dbs[i].next_block_num = -1;
    }
}

void mount_fs()
{
    FILE *file = fopen("fs_data", "r");
    // super block
    fread(&sb, sizeof(struct superblock), 1, file);

    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);

    // inodes
    fread(inodes, sizeof(struct inode), sb.num_inodes, file);
    // dbs
    fread(dbs, sizeof(struct disk_block), sb.num_blocks, file);
    fclose(file);
}
void sync_fs()
{
    FILE *file = fopen("fs_data", "w+");
    // super block
    fwrite(&sb, sizeof(struct superblock), 1, file);
    // inodes
    fwrite(inodes, sizeof(struct inode), sb.num_inodes, file);
    // dbs
    fwrite(dbs, sizeof(struct disk_block), sb.num_blocks, file);
    fclose(file);
}

void print_fs()
{
    printf("superblock info\n");
    printf("\tnum_inodes %d\n", sb.num_inodes);
    printf("\tnum_blocks %d\n", sb.num_blocks);
    printf("\tsize_blocks %d\n", sb.size_blocks);

    printf("inodes:\n");
    for (int i = 0; i < sb.num_inodes; i++)
    {
        printf("\tname %s\n", inodes[i].name);
        printf("\tsize %d\n", inodes[i].size);
        printf("\tfirst_block %d\n", inodes[i].first_block);
    }
    // dbs
    printf("block:\n");
    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("block num: %d next block %d\n", i, dbs[i].next_block_num);
    }
}
int find_empty_inode()
{
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (inodes[i].first_block == -1)
        {
            return i;
        }
    }
    return -1;
}
int find_empty_block()
{
    for (int i = 0; i < sb.num_blocks; i++)
    {
        if (dbs[i].next_block_num == -1)
        {
            return i;
        }
    }
    return -1;
}
int allocte_file(char name[9])
{
    //Find if the file is already found
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (strcmp(inodes[i].name, name)==0)
        {
            return inodes[i].first_block;
        }
        
    }
    
    int in = find_empty_inode();
    int block = find_empty_block();

    inodes[in].first_block = block;
    strcpy(inodes[in].name, name);
    dbs[block].next_block_num = -2;
    return block;
}

void shorten_file(int bn)
{
    int nn = dbs[bn].next_block_num;
    if(dbs[bn].next_block_num >= 0)
    {
        shorten_file(nn);
    }
    dbs[bn].next_block_num = -1;
}
void set_filesize(int filenum, int size)
{
    int temp = size + BLOCKSIZE -1;
    int num = temp / BLOCKSIZE;
    int bn = inodes[filenum].first_block;
    //grow the file if necessary
    for (num-- ; num > 0; num--)
    {
        //check next block number
        int next_num = dbs[bn].next_block_num;
        if (next_num == -2)
        {
            int empty = find_empty_block();
            dbs[bn].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        bn = dbs[bn].next_block_num;
    }
    //short the file if necessary 
    shorten_file(bn);
    dbs[bn].next_block_num = -2;
}
int get_block_num(int file, int offeset)
{
    int bn = inodes[file].first_block;
    for(int togo = offeset; togo > 0; togo--)
    {
        bn = dbs[bn].next_block_num;
    }
    return bn;
}
void write_data(int filenum, int pos, char *data)
{
    //calculate witch block
    int relative_block = pos/BLOCKSIZE;
    //find the block number
    int bn = get_block_num(filenum, relative_block);
    //calculate the offset in the block
    int offset = pos % BLOCKSIZE;
    //wrote the data
    strcpy(&dbs[bn].data[offset], data);
}
char* read_data(int filenum, int pos)
{
    //calculate witch block
    int relative_block = pos/BLOCKSIZE;
    //find the block number
    int bn = get_block_num(filenum, relative_block);
    //calculate the offset in the block
    int offset = pos % BLOCKSIZE;
    //read the data
    char* res = malloc(sizeof(char)*1024);
    strcpy(res, &(dbs[bn].data[offset]));
}