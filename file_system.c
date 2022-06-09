/**
 * we get help from:
 * https://www.youtube.com/watch?v=n2AAhiujAqs&ab_channel=drdelhart
 */
#include "file_system.h"



//***find empty block and inode
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
// resize the file size
void shorten_file(int bn)
{
    int nn = dbs[bn].next_block_num;
    if (dbs[bn].next_block_num >= 0)
    {
        shorten_file(nn);
    }
    dbs[bn].next_block_num = -1;
}
void set_filesize(int filenum, int size)
{
    int temp = size + BLOCKSIZE - 1;
    int num = temp / BLOCKSIZE;
    int bn = inodes[filenum].first_block;
    // grow the file if necessary
    for (num--; num > 0; num--)
    {
        // check next block number
        int next_num = dbs[bn].next_block_num;
        if (next_num == -2)
        {
            int empty = find_empty_block();
            dbs[bn].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        bn = dbs[bn].next_block_num;
    }
    // short the file if necessary
    shorten_file(bn);
    dbs[bn].next_block_num = -2;
}
int allocate_file(const char *name, int size)
{
    if (strlen(name) >= NAME_SIZE)
    {
        perror("strlen(name) >= NAME_SIZE");
        exit(1);
    }
    int inode = find_empty_inode();
    if (inode == -1)
    {
        perror("inode == -1");
        exit(1);
    }
    int first_block = find_empty_block();
    if (first_block == -1)
    {
        perror("first_block == -1");
        exit(1);
    }
    inodes[inode].size = size;
    inodes[inode].first_block = first_block;
    dbs[first_block].next_block_num = -2;
    strcpy(inodes[inode].name, name);
    set_filesize(inode, size);
    return inode;
}
//*** write and read a char
void write_a_char(int fd, int pos, char data)
{
    int index = pos;
    int block_index = inodes[fd].first_block;
    for (index = pos - BLOCKSIZE; index >= BLOCKSIZE; index -= BLOCKSIZE)
    {
        index -= BLOCKSIZE;
        if (dbs[block_index].next_block_num == -1)
        {
            perror("dbs[rb].next_block_num == -1");
            exit(1);
        }
        else if (dbs[block_index].next_block_num == -2)
        {
            dbs[block_index].next_block_num = find_empty_block();
            block_index = dbs[block_index].next_block_num;
            dbs[block_index].next_block_num = -2;
            break;
        }
        block_index = dbs[block_index].next_block_num;
    }
    if (pos > inodes[fd].size)
    {
        inodes[fd].size = pos + 1;
    }
    dbs[block_index].data[index] = data;
}
char read_a_char(int fd, int pos)
{
    int block_index = inodes[fd].first_block;
    for (pos -= BLOCKSIZE; pos >= BLOCKSIZE; pos -= BLOCKSIZE)
    {
        block_index = dbs[block_index].next_block_num;
        if (block_index == -1)
        {
            perror("rb == -1");
            exit(1);
        }
        if (block_index == -2)
        {
            perror("rb == -2");
            exit(1);
        }
    }
    return dbs[block_index].data[pos];
}
int get_block_num(int file, int offeset)
{
    int bn = inodes[file].first_block;
    for (int togo = offeset; togo > 0; togo--)
    {
        bn = dbs[bn].next_block_num;
    }
    return bn;
}
void write_data(int filenum, int pos, char *data)
{
    // calculate witch block
    int relative_block = pos / BLOCKSIZE;
    // find the block number
    int bn = get_block_num(filenum, relative_block);
    // calculate the offset in the block
    int offset = pos % BLOCKSIZE;
    // wrote the data
    strcpy(&dbs[bn].data[offset], data);
}
int open_new_dir(const char *path, const char *name)
{
    int fd = myopendir(path)->n;
    if (fd == -1)
    {
        perror("fd==-1");
        exit(1);
    }
    if (inodes[fd].dir == 0)
    {
        perror("inodes[fd].dir == 0");
        exit(1);
    }
    int dirblock = inodes[fd].first_block;
    struct mydirent *currdir = (struct mydirent *)dbs[dirblock].data;
    if (currdir->size >= MAX_DIR_SIZE)
    {
        perror("currdir->size>=MAX_DIR_SIZE");
        exit(1);
    }
    int newdirfd = allocate_file(name, sizeof(struct mydirent));
    currdir->fds[currdir->size++] = newdirfd;
    inodes[newdirfd].dir = 1;
    struct mydirent *newdir = malloc(sizeof(struct mydirent));
    newdir->size = 0;
    for (int i = 0; i < MAX_DIR_SIZE; i++)
    {
        newdir->fds[i] = -1;
    }

    char *newdiraschar = (char *)newdir;

    for (int i = 0; i < sizeof(struct mydirent); i++)
    {
        write_a_char(newdirfd, i, newdiraschar[i]);
    }
    strcpy(newdir->d_name, name);
    return newdirfd;
}

myDIR *myopendir(const char *name)
{
    char str[80];
    strcpy(str, name);
    char *choset;
    const char mak[2] = "/";
    choset = strtok(str, mak);
    char this_p[NAME_SIZE] = "";
    char last_p[NAME_SIZE] = "";

    while (choset != NULL)
    {
        strcpy(last_p, this_p);
        strcpy(this_p, choset);
        choset = strtok(NULL, mak);
    }

    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, this_p))
        {
            if (inodes[i].dir != 1)
            {
                printf("%s\n", inodes[i].name);
                perror("inodes[i].dir!=1");
                exit(1);
            }
            myDIR *res = (myDIR *)malloc(sizeof(myDIR));
            res-> n =i;
            return res;
        }
    }

    int fd = myopendir(last_p)->n;
    if (fd == -1)
    {
        perror("fd == -1");
        exit(1);
    }

    if (inodes[fd].dir == 0)
    {
        perror("inodes[fd].dir == 0");
        exit(1);
    }

    int db = inodes[fd].first_block;
    struct mydirent *live_d = (struct mydirent *)dbs[db].data;
    if (live_d->size >= 10)
    {
        perror("live_d->size >= 10");
        exit(1);
    }

    int dir = allocate_file(this_p, sizeof(struct mydirent));
    live_d->fds[live_d->size++] = dir;
    inodes[dir].dir = 1;
    struct mydirent *newdir = malloc(sizeof(struct mydirent));
    newdir->size = 0;
    for (int i = 0; i < 10; i++)
    {
        newdir->fds[i] = -1;
    }

    char *newdiraschar = (char *)newdir;
    for (size_t i = 0; i < sizeof(struct mydirent); i++)
    {
        write_data(dir, i, &newdiraschar[i]);
    }
    strcpy(newdir->d_name, this_p);
    myDIR *res = (myDIR *)malloc(sizeof(myDIR));
    res->n = dir;
    return res;
}

void mount_fs(const char *source)
{
    FILE *file = fopen(source, "r");
    // super block
    fread(&sb, sizeof(struct super_block), 1, file);

    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);

    // inodes
    fread(inodes, sizeof(struct inode), sb.num_inodes, file);
    // dbs
    fread(dbs, sizeof(struct disk_block), sb.num_blocks, file);
    fclose(file);
}
// write the file system
void sync_fs(const char *target)
{
    FILE *file = fopen(target, "w+");
    // super block
    fwrite(&sb, sizeof(struct super_block), 1, file);
    // inodes
    fwrite(inodes, sizeof(struct inode), sb.num_inodes, file);
    // dbs
    fwrite(dbs, sizeof(struct disk_block), sb.num_blocks, file);
    fclose(file);
}

// retrun file discriptor
int createfile(const char *path, const char *name)
{
    int newfd = allocate_file(name, sizeof(struct mydirent));
    myDIR *dirfd = myopendir(path);
    struct mydirent *currdir = myreaddir(dirfd);
    currdir->fds[currdir->size++] = newfd;
    return newfd;
}
void createroot()
{
    int rootd = allocate_file("root", sizeof(struct mydirent));
    if (rootd < 0)
    {
        perror("rootd < 0");
        exit(1);
    }
    inodes[rootd].dir = 1;
    struct mydirent *rootdir = malloc(sizeof(struct mydirent));
    for (int i = 0; i < MAX_DIR_SIZE; i++)
    {
        rootdir->fds[i] = -1;
    }
    strcpy(rootdir->d_name, "root");
    rootdir->size = 0;
    char *rootdiraschar = (char *)rootdir;
    for (int i = 0; i < sizeof(struct mydirent); i++)
    {
        write_a_char(rootd, i, rootdiraschar[i]);
    }
    free(rootdir);
}

// NOW
void mymkfs(int size)
{
    int real_size = size - sizeof(struct super_block);

    sb.num_inodes = (real_size / 10) / (sizeof(struct inode));
    sb.num_blocks = (real_size - sb.num_inodes) / (sizeof(struct disk_block));
    sb.size_blocks = BLOCKSIZE;
    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    if (inodes == NULL)
    {
        perror("inodes == NULL");
        exit(1);
    }

    for (int i = 0; i < sb.num_inodes; i++)
    {
        inodes[i].size = -1;
        inodes[i].real_size = 0;
        inodes[i].first_block = -1;
        strcpy(inodes[i].name, "");
    }
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);
    if (dbs == NULL)
    {
        perror("dbs == NULL");
        exit(1);
    }
    for (int i = 0; i < sb.num_blocks; i++)
    {
        dbs[i].next_block_num = -1;
    }
    createroot();
}

int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data)
{
    if (source == NULL && target == NULL)
    {
        perror("source == NULL && target == NULL");
        exit(1);
    }
    if (target != NULL)
    {
        sync_fs(target);
    }
    if (source != NULL)
    {
        mount_fs(source);
    }
    
}

int myopen(const char *pathname, int flags)
{
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, pathname))
        {
            if (inodes[i].dir == 1)
            {
                perror("inodes[i].dir == 1");
                exit(1);
            }
            myopenfile[i].fd = i;
            myopenfile[i].pos = 0;
            return i;
        }
    }
    int i = createfile("root", pathname);
    myopenfile[i].fd = i;
    myopenfile[i].pos = 0;
    return i;
}
int myclose(int myfd)
{
    myopenfile[myfd].fd = -1;
    myopenfile[myfd].pos = -1;
    return 0; // 0 is good
}
size_t myread(int myfd, void *buf, size_t count)
{
    if (inodes[myfd].dir == 1)
    {
        perror("inodes[i].dir == 1");
        exit(1);
    }
    if (myopenfile[myfd].fd != myfd)
    {
        perror("myopenfile[myfd].fd != myfd");
        exit(1);
    }
    char *buffer = malloc(count + 1);
    for (int i = 0; i < count; i++)
    {
        buffer[i] = read_a_char(myfd, myopenfile[myfd].pos);
        myopenfile[myfd].pos++;
    }
    buffer[count] = '\0';
    strncpy(buf, buffer, count);
    free(buffer);

    return myopenfile[myfd].pos;
}
size_t mywrite(int myfd, const void *buf, size_t count)
{
    if (inodes[myfd].dir == 1)
    {
        perror("inodes[myfd].dir == 1");
        exit(1);
    }
    if (myopenfile[myfd].fd != myfd)
    {
        perror("myopenfile[myfd].fd != myfd");
        exit(1);
    }
    char *buffer = (char *)buf;
    for (int i = 0; i < count; i++)
    {   
        inodes[myfd].real_size++;
        write_a_char(myfd, myopenfile[myfd].pos, buffer[i]);
        myopenfile[myfd].pos++;
    }
    return myopenfile[myfd].pos;
}
off_t mylseek(int myfd, off_t offset, int whence)
{
    if (myopenfile[myfd].fd != myfd)
    {
        perror("myopenfile[myfd].fd != myfd");
        exit(1);
    }
    if (whence == SEEK_CUR)
    {
        myopenfile[myfd].pos += offset;
    }
    else if (whence == SEEK_END)
    {
        myopenfile[myfd].pos += inodes[myfd].real_size;
        
        
    }
    else if (whence == SEEK_SET)
    {
        myopenfile[myfd].pos = offset;
    }
    if (myopenfile[myfd].pos < 0)
    {
        myopenfile[myfd].pos = 0;
    }
    return myopenfile[myfd].pos;
}
struct mydirent *myreaddir(myDIR *dirp)
{
    if (inodes[dirp->n].dir == 0)
    {
        perror("inodes[fd].dir == 0");
        exit(1);
    }
    return (struct mydirent *)dbs[inodes[dirp->n].first_block].data;
}
int myclosedir(myDIR *dirp)
{
    printf("We did not, because we did not have to, according to Yuval\n");
    return 0;
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
        printf("\tfirst_block %d\n\n", inodes[i].first_block);
    }
    // dbs
    printf("block:\n");
    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("\tblock num: %d next block %d\n\n", i, dbs[i].next_block_num);
    }
}