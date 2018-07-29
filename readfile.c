#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include "queue.h"
#include "readfile.h"
#include "minilzo.h"


#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

static void myprintf(char *str, unsigned int size)
{
    unsigned int i;
    for(i = 0; i < size; i++)
    {
        printf("%c",*(str+i));
    }
    printf("\n");
}

blk * init_read(char *filename)
{
        int fd;
        blk *fblk;
        int n; 

        char buf[BLK_SIZE];    

        //printf("%s\n",filename);
        /*打开文件*/
        if ((read_file_handle = open(filename, O_RDONLY)) == -1 )
        {
            fatal_error("Cannot open file,exit!");
        }

        /*初始化压缩算法*/
       if (lzo_init() != LZO_E_OK)
       {
            fatal_error("can not init lzo!\n");
       }

        /*创建数据结构*/
        if ((fblk = (blk *)malloc(sizeof(blk))) == NULL )
        {
           fatal_error("Cannot malloc memory,eixt!");
        }

        fblk->offset = 0;
        fblk->len = 0;

        return fblk;
}

void * read_blk(blk *fblk)
{
        int offset = fblk->offset;
        char *buf = fblk->data;
        int n;

        if ((n = read(read_file_handle, buf, BLK_SIZE)) > 0)
        { 
            fblk->len = n | BLK_IMAGE;
            return (void *)fblk;
        }
        else
        {
            return NULL;   
        }

}

void * read_and_compress(blk *fblk)
{
        int offset = fblk->offset;
        char *src = fblk->buf;
        char *dst = fblk->data;
        unsigned int src_len;
        unsigned int dst_len;

        /*把数据读到buf中去*/
        if ((src_len = read(read_file_handle, src, BLK_SIZE)) > 0)
        { 
             lzo1x_1_compress((lzo_bytep)src, (lzo_uint)src_len,
                         (lzo_bytep)dst, (lzo_uintp)&dst_len, wrkmem);
            
            fblk->len = dst_len;

			if(fblk->len > BLK_SIZE)
			{
				printf("Debug:length too long,len: %d",dst_len);
			}
            return (void *)fblk;
        }
        else
        {
            return NULL;   
        }

}


void  free_read(blk *fblk)
{
        close(read_file_handle);
        free(fblk);
}
