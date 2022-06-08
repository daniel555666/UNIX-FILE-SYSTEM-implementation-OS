#include "file_system.h"

int main(int argc, char const *argv[])
{
    mymkfs(10000);
    mymount("fs_data", NULL, NULL, 0, NULL);
    // print_fs();
    // int dvirdiscriptor = allocate_file(100, "Dvir");
    int dvirdiscriptor = myopen("root/Dvir", 1);
    // int ariel_discriptor = allocte_file("Ariel");
    printf("%d\n", dvirdiscriptor);
    mywrite(dvirdiscriptor, "dvir is the king of os and he need to get 100!", 50);
    mylseek(dvirdiscriptor,0, SEEK_SET);
    char buf[50];
    myread(dvirdiscriptor,buf, 50);
    printf("%s\n", buf);
    myclose(dvirdiscriptor);
    print_fs();
    mymount(NULL, "fs_data" , NULL, 0, NULL);
    printf("done :)\n");
    return 0;
}
