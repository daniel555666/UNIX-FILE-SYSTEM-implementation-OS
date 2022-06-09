#include "file_system.h"
#include "mylibc.h"

int main(int argc, char const *argv[])
{
    mymkfs(10000);
    mymount("DISC", "DISC", NULL, 0, NULL);
    printf("TEST PASS: Was able to open the disk\n");
    myFILE *myfd = myfopen("file", "r+");
    if (myfprintf(myfd, "%c%d",'p',5) == 2)
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
    if (myfprintf(myfd, "%f", (float)7.5) == 1)
    {
        printf("TEST PASS: Was able to write a float\n");
    }
    myfclose(myfd);
    myfd = myfopen("file", "r");
    c = '0';
    n = 0;
    float f = 0;
    if (myfscanf(myfd, "%c%d%f", &c, &n, &f) == 3)
    {
        printf("TEST PASS: Was able to read:\n\t char- %c and int- %d and float- %f \n", c, n, f);
    }
    myfclose(myfd);
    printf("TEST PASS: Was able to close the file\n");

    return 0;
}
