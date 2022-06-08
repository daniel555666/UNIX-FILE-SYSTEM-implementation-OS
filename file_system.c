#include "file_system.h"

struct super_block sb;
struct inode *inodes;
struct disk_block *dbs;
struct open_file myopenfile[MAX_FILES];

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
void shorten_file(int bn)
{
    int nn = dbs[bn].next_block_num;
    if (dbs[bn].next_block_num >= 0)
    {
        shorten_file(nn);
    }
    dbs[bn].next_block_num = -1;
}
int allocate_file(int size, const char *name)
{
    if (strlen(name) > NAME_SIZE - 1)
    {
        perror("strlen(name) > NAME_SIZE - 1");
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
int allocate_file2(char *name)
{
    // Find if the file is already found
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (strcmp(inodes[i].name, name) == 0)
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
void writebyte(int fd, int opos, char data)
{
    /**
     * @brief Write a SINGLE byte into a disk block.
     * The function calculates the correct relevant block (rb) that is needed to be accessed.
     * if the position that is needed to be wrriten is out of the bounds of the file,
     * allocate a new disk block for it.
     */
    int pos = opos;
    int rb = inodes[fd].first_block;
    while (pos >= BLOCKSIZE)
    {
        pos -= BLOCKSIZE;
        if (dbs[rb].next_block_num == -1)
        {
            perror("dbs[rb].next_block_num == -1");
            exit(1);
        }
        else if (dbs[rb].next_block_num == -2)
        { // the current block is the last block, so we allocate a new block
            dbs[rb].next_block_num = find_empty_block();
            rb = dbs[rb].next_block_num;
            dbs[rb].next_block_num = -2;
        }
        else
        {
            rb = dbs[rb].next_block_num;
        }
    }
    if (opos > inodes[fd].size)
    {
        inodes[fd].size = opos + 1;
    }
    dbs[rb].data[pos] = data;
}
char readbyte(int fd, int pos)
{
    /**
     * @brief Read a SINGLE byte from a disk block.
     * The function calculates the correct relevant block (rb) that is needed to be accessed.
     * The single byte is @return 'ed.
     */
    int rb = inodes[fd].first_block;
    while (pos >= BLOCKSIZE)
    {
        pos -= BLOCKSIZE;
        rb = dbs[rb].next_block_num;
        if (rb == -1)
        {
            perror("rb == -1");
            exit(1);
        }
        if (rb == -2)
        {
            perror("rb == -2");
            exit(1);
        }
    }
    return dbs[rb].data[pos];
}

int mymkdir(const char *path, const char *name)
{
    /**
     * @brief This function goes through the path and finds the FD of the last directory in the path.
     * Then, it creates a new directory inside the FD that was found.
     */
    int fd = myopendir(path);
    if (fd == -1)
    {
        perror("fd==-1");
        exit(1);
    }
    if (inodes[fd].dir != 1)
    {
        perror("inodes[fd].dir!=1");
        exit(1);
    }
    int dirblock = inodes[fd].first_block;
    struct dirent *currdir = (struct dirent *)dbs[dirblock].data;
    if (currdir->size >= MAX_DIR_SIZE)
    {
        perror("currdir->size>=MAX_DIR_SIZE");
        exit(1);
    }
    int newdirfd = allocate_file((int)sizeof(struct dirent), name);
    currdir->fds[currdir->size++] = newdirfd;
    inodes[newdirfd].dir = 1;
    struct dirent *newdir = malloc(sizeof(struct dirent));
    newdir->size = 0;
    for (int i = 0; i < MAX_DIR_SIZE; i++)
    {
        newdir->fds[i] = -1;
    }

    char *newdiraschar = (char *)newdir;

    for (int i = 0; i < sizeof(struct dirent); i++)
    {
        writebyte(newdirfd, i, newdiraschar[i]);
    }
    strcpy(newdir->d_name, name);
    return newdirfd;
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

myDIR myopendir(const char *name)
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
        strcpy(last_p, this_p); // if?
        strcpy(this_p, choset);
        choset = strtok(NULL, mak);
    }

    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, this_p)) // 1 == used dir
        {
            if (inodes[i].dir != 1)
            {
                printf("%s\n", inodes[i].name);
                perror("inodes[i].dir!=1");
                return -1;
            }
            return i;
        }
    }

    int fd = myopendir(last_p);
    if (fd == -1)
    {
        perror("fd==-1");
        return -1;
    }

    if (inodes[fd].dir != 1)
    {
        perror("inodes[fd].dir!=1");
        return -1;
    }

    int db = inodes[fd].first_block;
    struct dirent *live_d = (struct dirent *)dbs[db].data;
    if (live_d->size >= 10)
    {
        perror("live_d->size>=MAX_DIR");
        return -1;
    }

    int dir = allocate_file(sizeof(struct dirent), this_p);
    live_d->fds[live_d->size++] = dir;
    inodes[dir].dir = 1;
    struct dirent *newdir = malloc(sizeof(struct dirent));
    newdir->size = 0;
    for (int i = 0; i < 10; i++)
    {
        newdir->fds[i] = -1;
    }

    char *newdiraschar = (char *)newdir;
    for (size_t i = 0; i < sizeof(struct dirent); i++)
    {
        write_data(dir, i, &newdiraschar[i]);
    }
    strcpy(newdir->d_name, this_p);
    return dir;
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
    int newfd = allocate_file(1, name);
    int dirfd = myopendir(path);
    printf("hello\n");
    struct dirent *currdir = myreaddir(dirfd);
    currdir->fds[currdir->size++] = newfd;
    return newfd;
}
void createroot()
{
    /**
     * @brief Creates a root directory at inode 0.
     * If inode 0 is not empty an exit failure will occur.
     *
     */
    int zerofd = allocate_file(sizeof(struct dirent), "root");
    if (zerofd != 0)
    {
        perror("zerofd != 0");
        exit(1);
    }
    inodes[zerofd].dir = 1;
    struct dirent *rootdir = malloc(sizeof(struct dirent));
    for (int i = 0; i < MAX_DIR_SIZE; i++)
    {
        rootdir->fds[i] = -1;
    }
    strcpy(rootdir->d_name, "root");
    rootdir->size = 0;
    char *rootdiraschar = (char *)rootdir;
    for (int i = 0; i < sizeof(struct dirent); i++)
    {
        writebyte(zerofd, i, rootdiraschar[i]);
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
    if (source != NULL)
    {
        mount_fs(source);
    }
    if (target != NULL)
    {
        sync_fs(target);
    }
}

int myopen(const char *pathname, int flags)
{
    /**
     * @brief Open a file at the given path.
     * The opened file will be added into 'openfiles' struct array and this instance will be used to get the pointer of the file.
     */
    char str[80];
    strcpy(str, pathname);
    char *token;
    const char s[2] = "/";
    token = strtok(str, s);
    char currpath[NAME_SIZE] = "";
    char lastpath[NAME_SIZE] = "";
    while (token != NULL)
    {
        //כנראה סגמנטיישן פולט
        strcpy(lastpath, currpath);
        strcpy(currpath, token);
        token = strtok(NULL, s);
    }
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, currpath))
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
    int i = createfile(lastpath, currpath);
    myopenfile[i].fd = i;
    myopenfile[i].pos = 0;
    return i;
}
int myclose(int myfd)
{
    myopenfile[myfd].fd = -1;
    myopenfile[myfd].pos = -1;
}
size_t myread(int myfd, void *buf, size_t count)
{
    /**
     * @brief Read a chunk of bytes from the given file into @param buf.
     * @return the current position of the file pointer.
     *
     */
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
        buffer[i] = readbyte(myfd, myopenfile[myfd].pos);
        myopenfile[myfd].pos++;
    }
    buffer[count] = '\0';
    strncpy(buf, buffer, count);
    free(buffer);

    return myopenfile[myfd].pos;
}
size_t mywrite(int myfd, const void *buf, size_t count)
{
    /**
     * @brief Write a chunk of bytes from the given @param buf into fd myfd.
     * @return the current position of the file pointer.
     *
     */
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
        writebyte(myfd, myopenfile[myfd].pos, buffer[i]);
        myopenfile[myfd].pos++;
    }
    return myopenfile[myfd].pos;
}
off_t mylseek(int myfd, off_t offset, int whence)
{
    /**
     * @brief This function is used to move the file pointer, @return the new location of the pointer.
     *
     */
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
        myopenfile[myfd].pos = inodes[myfd].size + offset;
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
struct dirent *myreaddir(myDIR dirp)
{
    /**
     * @brief Uses @param fd to find the asked directory and @return it as a @struct mydirent. 
     * 
     */
    if (inodes[dirp].dir == 0) {
        perror("inodes[fd].dir == 0");
        exit(1);
    }
    return (struct dirent*)&dbs[inodes[dirp].first_block].data;
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
        printf("\tfirst_block %d\n", inodes[i].first_block);
    }
    // dbs
    printf("block:\n");
    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("block num: %d next block %d\n", i, dbs[i].next_block_num);
    }
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