#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include "readfile.h"

void myprintf(char *str, unsigned int size)
{
    unsigned int i;
    for(i = 0; i < size; i++)
    {
        printf("%c",*(str+i));
    }
    printf("\n");
}



int main()
{
    blk *fblk;
	int n;
    int fd;
    char filename[]="data.txt";
    char dstfile[] = "data.txt.lzo";

    fblk = init_read(filename);
    
    fd = open(dstfile, O_CREAT| O_WRONLY);
    //while( read_blk(fblk) != NULL)
    while( read_and_compress(fblk) != NULL)
    {
        //printf("----------------------len: %d\n",fblk->len);
        n = write(fd, fblk->data, fblk->len);
		printf("write size:%d\n",n); 
        //myprintf(fblk->data,fblk->len);
    }
    close(fd);
}
