#include "file_system.h"

int main(int argc, char const *argv[])
{
    // create_fs();
    // sync_fs();
    mount_fs();
    int dvirdiscriptor = allocte_file("DVIR");
    int ariel_discriptor = allocte_file("Ariel");
    printf("%d\n", dvirdiscriptor);
    set_filesize(0, 5000);
    write_data(dvirdiscriptor, 0, "dvir is the king of os and he need to get 100!");
    printf("%s\n", read_data(dvirdiscriptor, 0));
    print_fs();
    sync_fs();
    printf("done :)\n");
    return 0;
}
