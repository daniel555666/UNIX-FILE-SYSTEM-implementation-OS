#include "mylibc.h"
#include "file_system.h"
#include <string.h>
#include <stdarg.h>

myFILE *myfopen(const char *pathname, const char *mode)
{ // int myopen(const char *pathname, int flags);
    myFILE *f = (myFILE *)malloc(sizeof(myFILE));
    if (f == NULL)
    {
        printf("error in malloc");
        exit(0);
    }
    f->fd = myopen(pathname, 0);
    if (strlen(mode) > 2)
    {
        printf("wrong mode");
        exit(0);
    }
    strcpy(f->mod, mode);

    if (f->mod[0] == 'a')
    {
        mylseek(f->fd, 0, SEEK_END);
    }
    return f;
}

int myfclose(myFILE *stream)
{ // int myclose(int myfd);
    if (myclose(stream->fd) == 0)
    {
        free(stream);
        return 0; // return 0 if close work
    }
    free(stream);
    return 1; // return 1 if didnt close
}

size_t myfread(void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream)
{ // size_t myread(int myfd, void *buf, size_t count)

    if (stream->mod[0] == 'r' || (stream->mod[0] == 'r' && stream->mod[1] == '+'))
    {
        size_t start_pos = myopenfile[stream->fd].pos;
        size_t end_pos = myread(stream->fd, ptr, nmemb * size);

        return start_pos - end_pos; // if not read successfully will handle at myread
    }
    return 0; // cant read no mode for reading
}

size_t myfwrite(const void *restrict ptr, size_t size, size_t nmemb, myFILE *stream)
{ // size_t mywrite(int myfd, const void *buf, size_t count)

    if (stream->mod[0] == 'w' || (stream->mod[0] == 'r' && stream->mod[1] == '+') || stream->mod[0] == 'a')
    {
        size_t start_pos = myopenfile[stream->fd].pos;
        size_t end_pos = mywrite(stream->fd, ptr, size * nmemb);
        return end_pos - start_pos;
    }
    // need to add for w and r+
    perror("cant write no mode");
    exit(0);
}

int myfseek(myFILE *stream, long offset, int whence)
{

    mylseek(stream->fd, offset, whence);
    return 0; // return 0 if successful o,
    // in our implemntion if not successful will error in read
}

// insert from the stream file to the objects
int myfscanf(myFILE *restrict stream, const char *restrict format, ...)
{

    va_list arguments;
    va_start(arguments, format);
    int format_len = strlen(format);
    int j = 0;
    for (size_t i = 0; i < format_len; i++)
    {
        if (format[i] == '%')
        {
            if (format[i + 1] == 'd')
            {
                myfread(va_arg(arguments, void *), sizeof(int), 1, stream);
                j++;
            }
            else if (format[i + 1] == 'f')
            {
                myfread(va_arg(arguments, void *), sizeof(float), 1, stream);
                j++;
            }
            else if (format[i + 1] == 'c')
            {
                myfread(va_arg(arguments, void *), sizeof(char), 1, stream);
                j++;
            }
            i++;
        }
    }
    return j;
}

// write to the file the chars
int myfprintf(myFILE *stream, const char *format, ...)
{
    int j=0;
    va_list arguments;
    va_start(arguments, format);
    int format_len = strlen(format);

    for (size_t i = 0; i < format_len; i++)
    {
        if (format[i] == '%')
        {
            if (format[i + 1] == 'd')
            {
                int temp1 = va_arg(arguments, int);
                myfwrite(&temp1, sizeof(int), 1, stream);
                j++;
            }
            else if (format[i + 1] == 'f')
            {
                float temp2 = (float)va_arg(arguments, double);
                myfwrite(&temp2, sizeof(float), 1, stream);
                j++;
            }
            else if (format[i + 1] == 'c')
            {
                char temp3 = (va_arg(arguments, int));
                myfwrite(&temp3, sizeof(char), 1, stream);
                j++;
            }
            i++;
        }
        else
        {
            while (format[i] != '%' && i < format_len)
            {
                char temp4 = format[i];
                myfwrite(&temp4, sizeof(char), 1, stream);
                i++;
            }
            i--; // return to the '%' char
        }
    }
    return j;
}
