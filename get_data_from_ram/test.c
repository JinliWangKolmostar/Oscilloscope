#include <stdio.h>
#include <stdlib.h>
//#include <visa.h>
#include <string.h>
//#include <malloc.h>

int IsFallingEdge(unsigned char *);

#define MIN(a, b) ((a) > (b) ? (b) : (a))

//static const unsigned int MEMORY_DEPTH = 120000;
static const unsigned int SINGLE_READ_MAX_LEN = 250000;
static const char origin_file_name[64] = "/Users/wangjingli/Downloads/rigol_data/data.bin";
static const char source_file_name[64] = "/Users/wangjingli/Downloads/rigol_data/valid_data_test.bin";
static const unsigned char data_mask = 0x0F;

void test_DataFormatConverse()
{
    __int64 test_count = 0;
    __int64 data_wrap = 0;
    size_t data_size;
    unsigned char *read_buffer = (unsigned char *)malloc(SINGLE_READ_MAX_LEN * sizeof(char));

    FILE *fp_source = fopen(origin_file_name, "rb");
    FILE *fp_destin = fopen(source_file_name, "wb");

    while((data_size = fread(read_buffer, 1, SINGLE_READ_MAX_LEN, fp_source)) != 0)
    {
        int count = 0;
        int wrap_4bit_count = 0;
        __int64 *write_slip = (__int64 *)read_buffer;

        while(((*(read_buffer+count) & 0x10) == 0) && (count < data_size))
        {
            count++;
        }

        while((count < data_size) && (*(read_buffer+count) & 0x10) != 0)
        {
            if(IsFallingEdge(read_buffer + count) == 1)
            {
                test_count++;
                if(test_count == 30000)
                {
                    break;
                }
                data_wrap |= (__int64)(*(read_buffer + count) & data_mask) << (4 * wrap_4bit_count);

                wrap_4bit_count++;
                if(wrap_4bit_count % 16 == 0)
                {
                    *write_slip++ = data_wrap;
                    data_wrap = 0;
                }
            }
            count++;
        }
        if(data_wrap != 0)
        {
            *write_slip = data_wrap;
        }
        fwrite(read_buffer, 1, (wrap_4bit_count + 1) / 2, fp_destin);
    }
    free(read_buffer);
    fclose(fp_source);
    fclose(fp_destin);
    printf("data handle finished!\n");
}


int compareDiff(char *sou_buffer, char *des_buffer)
{
	int i;
	for(i = 0; i < 66*1024; i++)
	{
		if(*sou_buffer++ != *des_buffer++)
		{
            break;
		}
	}
    return i;
}

