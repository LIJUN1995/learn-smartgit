#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int array[10] = {0};
    int rec[10] = {0};

    memset(array,0,sizeof(array));

    for (size_t i = 0; i < 10; i++)
    {
        array[i] = 32;
        printf("array[%lu] = %d\n",i,array[i]);
    }

    FILE *fp = fopen("./myfile.txt","wa+");
    fwrite(array,sizeof(array),1,fp);
    
    // fseek(fp, 0L, SEEK_END);
    // length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    fread(rec,sizeof(rec),1,fp);
    fclose(fp);

    for (size_t i = 0; i < 10; i++)
    {
        printf("rec[%lu] = %d\n",i,rec[i]);
    }
    

    return 0;
}
