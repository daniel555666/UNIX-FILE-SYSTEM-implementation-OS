#include "file_system.h"

int main(int argc, char const *argv[])
{
    mymkfs(10000);
    mymount("fs_data", "fs_data", NULL, 0, NULL);
    // print_fs();
    // int dvirdiscriptor = allocate_file(100, "Dvir");
    int dvirdiscriptor = myopen("root/DANIEL", 1);
    // int ariel_discriptor = allocte_file("Ariel");
    // mylseek(dvirdiscriptor,0, SEEK_END);
    mywrite(dvirdiscriptor, "daniel", 6);
    mylseek(dvirdiscriptor,0, SEEK_SET);
    mylseek(dvirdiscriptor,0, SEEK_END);
    mywrite(dvirdiscriptor, "dvir", 4);
    mylseek(dvirdiscriptor,0, SEEK_SET);

    char buf[200];
    bzero(buf, 200);
    myread(dvirdiscriptor,buf, 10);
    printf("%s\n", buf);

    myclose(dvirdiscriptor);
    myDIR *dd = myopendir("root/home");
    printf("%d\n", *dd);
    struct mydirent *fd = myreaddir(dd);
    printf("name: %s, size: %d\n", fd->d_name, fd->size);
    // print_fs();
    mymount(NULL, "fs_data" , NULL, 0, NULL);
    printf("done :)\n");
    return 0;
}
