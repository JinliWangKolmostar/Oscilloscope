#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DES_FILE_SIZE 65 * 1024
#define COMPARE_SIZE 256

static unsigned char file_contet[DES_FILE_SIZE];
static const char source_file_name[64] = "C:\\rigol_data\\valid_data.bin";

//two args should be introduced from funtion 'DataFormatConverse'
//arg1:current source number which need to match
//arg2:content of file need to compare
//return:file name which depend on spi file 
void *find_substring(void *args)
{
    int i;
    int oscillo_data_num = 0;    
    char sou_file_name[64];
    char des_file_name[64];
    unsigned char buffer[COMPARE_SIZE];

    for(i = 0; ; i++)
    {
        FILE *fp_sou;
        while(1) {
            sprintf(des_file_name, "file_name_%d", oscillo_data_num);
            fp_sou = fopen(des_file_name, "rb");
            if(fp_sou == NULL)
            {
                continue;
            }else {
                oscillo_data_num++;
                break;
            }
        }

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

        int j;
        fseek(fp_sou, 0, SEEK_SET);
        for(j = 0; j < 4; j++)
        {
            fseek(fp_sou, 2048 * j, SEEK_CUR);
            fread(buffer, 1, COMPARE_SIZE, fp_sou);

            char *substring_pos = strstr((char *)file_contet, (char *)buffer);
            if(substring_pos != NULL)
            {
                printf("file:%s\n", des_file_name);
                fclose(fp_sou);
                return 1;
            }
        }
        fclose(fp_sou);
    }

    return 0;
}
