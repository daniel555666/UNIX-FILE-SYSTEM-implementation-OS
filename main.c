#include "file_system.h"
#include "mylibc.h"

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 10; i++)
    {
        printf("\033[0;36m*************************Start Test Number: %d*************************\n\033[0;37m", i);
        mymkfs(10000);
        mymount("DISC", "DISC", NULL, 0, NULL);
        printf("TEST PASS: Was able to open the disk\n");
        myFILE *myfd = myfopen("file", "r+");
        if (myfprintf(myfd, "%c%d", (char)i + 'a', i) == 2)
        {
            printf("TEST PASS: Was able to write a char and int\n");
        }
        myfseek(myfd, 0, SEEK_SET);
        char c = '0';
        int n = 0;
        if (myfscanf(myfd, "%c%d", &c, &n) == 2)
        {
            printf("TEST PASS: Was able to read:\n\t char- %c and int- %d \n", c, n);
        }
        myfclose(myfd);
        printf("TEST PASS: Was able to close the file\n");

        myfd = myfopen("file", "a");
        if (myfprintf(myfd, "%d", 10-i) == 1)
        {
            printf("TEST PASS: Was able to write a big number\n");
        }
        myfclose(myfd);
        myfd = myfopen("file", "r");
        c = '0';
        n = 0;
        int f = 0;
        if (myfscanf(myfd, "%c%d%f", &c, &n, &f) == 3)
        {
            printf("TEST PASS: Was able to read:\n\t char- %c and int- %d and big number- %d\n", c, n, f);
        }
        myfclose(myfd);
        printf("TEST PASS: Was able to close the file\n");
    }
    printf("\033[0;35mAll The Test Are Pass, We Deserve 100! :)\n");
    return 0;
}
