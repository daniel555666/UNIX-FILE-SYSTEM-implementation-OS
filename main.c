#include "file_system.h"
#include "mylibc.h"

int main(int argc, char const *argv[])
{
    mymkfs(10000);
    mymount("fs_data", NULL, NULL, 0, NULL);
    // print_fs();
    // int dvirdiscriptor = allocate_file(100, "Dvir");
    // int dvirdiscriptor = myopen("root/Dvir", 1);
    // int ariel_discriptor = allocte_file("Ariel");

    // mywrite(dvirdiscriptor, "dvir is the king of os and he need to get 100!", 50);
    // mylseek(dvirdiscriptor,0, SEEK_SET);
    // char buf[50];
    // myread(dvirdiscriptor,buf, 50);
    // printf("%s\n", buf);
    // myclose(dvirdiscriptor);
    // myDIR *dd = myopendir("root");
    // printf("%d\n", *dd);
    // struct mydirent *fd = myreaddir(dd);
    // printf("name: %s, size: %d\n", fd->d_name, fd->size);
    // // print_fs();
    // mymount(NULL, "fs_data" , NULL, 0, NULL);
    // printf("done :)\n");

    char c = 'a';
    char c2 = 'b';
    myFILE *fd = myfopen("root/d7", "a");
    myfprintf(fd, "%c%c", 'y', ';');
    myfclose(fd);
    myFILE *fd2 = myfopen("root/d7", "r");
    int n = 0;
    int n2 = 0;
    myfscanf(fd2, "%c%c", &c, &c2);

    printf("%c%c-last print", c, c2);

    return 0;
}