#include <stdio.h>
#include "queue.h"
//#include "minilzo.h"


int read_file_handle;

blk * init_read(char *filename);
void * read_and_compress(blk *fblk);
void * read_blk(blk *fblk);
void  free_read(blk *fblk);
