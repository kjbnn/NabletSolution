#ifndef _CALLBACK_
#define _CALLBACK_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable:4996)

void status_information(int status, void * app, const char * fmt,  ...)
{
    char lst[256];
    va_list marker;

    va_start(marker,fmt);
    vsprintf(lst,fmt,marker);
    va_end(marker);

    printf("%s\n", lst);
}

// this function provides the offsets to finish the file
void data_deliever_ex(void * app, unsigned char * data, unsigned int size, int seek_offset)
{ 
    static int fp;

#ifdef _WIN32
    if ((fp = _open("dump.mp4", _O_RDWR | _O_BINARY)) == -1)
        return;
 
    _lseeki64(fp, seek_offset, SEEK_SET);
    _write(fp, data, size);
#else
    if ((fp = open("dump.mp4", O_RDWR)) == -1)
        return;
#if defined(__APPLE__)
    lseek(fp, seek_offset, SEEK_SET);
#else
    lseek64(fp, seek_offset, SEEK_SET);
#endif // Apple
    write(fp, data, size);
#endif
}

//static FILE * file;
void data_deliever(void * app, unsigned char * data, unsigned int size, void * sample)
{
    static FILE * file;

    if (file == NULL)
        file = fopen("dump.mp4", "wb");

    fwrite(data, size, 1, file);
}

void * EXPORT_API get_callback(char * function_name)
{
    if(!strcmp(function_name,"status_information"))
        return (void*) status_information;
    else if(!strcmp(function_name,"data_deliever"))
        return (void*) data_deliever;
    else if (!strcmp(function_name, "data_deliever_ex"))
        return (void*)data_deliever_ex;

    printf("Not initialized: %s\n",function_name);

    return NULL;
}

#ifdef __cplusplus
}
#endif


#endif //_CALLBACK_
