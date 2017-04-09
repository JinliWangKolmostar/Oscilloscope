#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DES_FILE_NUM 100
#define DES_FILE_SIZE 65 * 1024
#define COMPARE_SIZE 256

static unsigned char file_contet[DES_FILE_SIZE];
static const char source_file_name[64] = "C:\\rigol_data\\valid_data.bin";

int find_substring()
{
    int j, i;
    unsigned char buffer[COMPARE_SIZE];
    FILE *fp = fopen(source_file_name, "rb");

    for(i = 0; i < DES_FILE_NUM; i++)
    {
        char des_file_name[64];
        sprintf(des_file_name, "C:\\rigol_data\\data_spi\\data_capture_interval_%d.bin", i);
        FILE *fp_des = fopen(des_file_name, "rb");
        if(fp_des == NULL)
        {
            printf("nothing find\n");
            break;
        }
        int get_size = fread(file_contet, 1, DES_FILE_SIZE, fp_des);
        if(get_size == 0)
        {
            printf("get_size error\n");
            break;
        }
        fclose(fp_des);
    //    if(remove(des_file_name) != 0)
   //     {
   //         printf("remove %s error\n", des_file_name);
 //           break;
 //       }

        fseek(fp, 0, SEEK_SET);
        for(j = 0; j < 4; j++)
        {
            fseek(fp, 512 * j, SEEK_CUR);
            fread(buffer, 1, COMPARE_SIZE, fp);

            char *substring_pos = strstr((char *)file_contet, (char *)buffer);
            if(substring_pos != NULL)
            {
                printf("file:%s\n", des_file_name);
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}
