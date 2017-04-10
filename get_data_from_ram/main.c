#include <stdio.h>
#include <stdlib.h>
#include <visa.h>
#include <string.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))

static const unsigned int MEMORY_DEPTH = 120000;
static const unsigned int SINGLE_READ_MAX_LEN = 250000;
static const unsigned int RIGOL_HEAD_MAX_LEN = 11;
static const unsigned char data_mask = 0x0F;

int FindSubstring(char *source_file_name);
void test();
void test_GetValidData();
void compareDiff();


int IsFallingEdge(unsigned char *buffer)
{
    return ((*buffer & 0x20) && !(*(buffer + 1) & 0x20));
}

void DataFormatConverse()
{
    __int64 data_wrap = 0;
	size_t data_size;
	unsigned char *read_buffer = (unsigned char *)malloc(2 * SINGLE_READ_MAX_LEN * sizeof(char));

	int i;
	for(i = 0; ; i++)
	{
		char origin_file_name[64];
		char valid_file_name[64];
		sprintf(origin_file_name, "C:\\rigol_data\\rigol_data_%d.bin", i);
		sprintf(valid_file_name, "C:\\rigol_data\\data_oscillo\\oscillo_valid_data_%d.bin", i);
		FILE *fp_source = fopen(origin_file_name, "rb");
		if(fp_source == NULL)
        {
            break;
        }
		FILE *fp_destin = fopen(valid_file_name, "wb");

		if((data_size = fread(read_buffer, 1, 2 * SINGLE_READ_MAX_LEN, fp_source)) != 0)
		{
			int count = 0;
			int wrap_4bit_count = 0;
			__int64 *write_slip = (__int64 *)read_buffer;

			while(((*(read_buffer + count) & 0x10) == 0) && (count < data_size))
			{
				count++;
			}

			while((count < data_size) && (*(read_buffer+count) & 0x10) != 0)
			{
				if(IsFallingEdge(read_buffer + count) == 1)
				{
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
		fclose(fp_source);
		fclose(fp_destin);
	}

	free(read_buffer);
    printf("data handle finished!\n");
}

int DiscardDataHead(unsigned char *buffer)
{
	if(buffer[0] != '#')
	{
		return -1;
	}
	int des_data_len = buffer[1] - '0';
	int buffer_offset = 1 + des_data_len + 1;

	return buffer_offset;
}

void delay(unsigned int time)
{
    while(time--)
    {
        ;
    }
}

int main(int argc, char* argv[])
{
	ViSession defaultRM, vi;

	char strStarPos[128];
	char strStopPos[128];
	char* strStop = ":STOP\n";
	char* strRun = ":RUN\n";
	char* strSource = "WAV:SOURce D0\n";
	char* strMode = ":WAV:MODE RAW\n";
	char* strGetData = ":WAV:DATA?\n";

	unsigned long strStop_WriteLen;
	unsigned long strSource_WriteLen;
	unsigned long strMode_WriteLen;
	unsigned long strStarPos_WriteLen;
	unsigned long strStopPos_WriteLen;
	unsigned long strGetData_WriteLen;

	unsigned char *buffer = (unsigned char *)malloc((SINGLE_READ_MAX_LEN + RIGOL_HEAD_MAX_LEN) * sizeof(char));
	unsigned long readLen;

	ViRsrc matches = (ViRsrc)malloc(256);
	ViUInt32 nmatches;
	ViFindList list;
	viOpenDefaultRM (&defaultRM);
	viFindRsrc(defaultRM, "USB?*", &list,&nmatches, matches);
	viOpen (defaultRM,matches,VI_NULL,VI_NULL,&vi);

	//set source
	viWrite(vi, (unsigned char *)strSource, strlen(strSource), &strSource_WriteLen);
	//set
	viWrite(vi, (unsigned char *)strMode, strlen(strMode), &strMode_WriteLen);

	int file_num = 0;
    char des_file_name[64];
	while(file_num < 100)
	{
		//stop
		viWrite(vi, (unsigned char *)strStop, strlen(strStop), &strStop_WriteLen);

        sprintf(des_file_name, "C:\\rigol_data\\rigol_data_%d.bin", file_num++);
		FILE *fp = fopen(des_file_name, "wb");
		int i;
		for(i = 0; i < 2; i++) {
			sprintf(strStarPos, ":WAV:STAR %u\n", i * SINGLE_READ_MAX_LEN + 400001);
			sprintf(strStopPos, ":WAV:STOP %u\n", (i + 1) * SINGLE_READ_MAX_LEN + 400001);
			//set star position
			viWrite(vi, (unsigned char *)strStarPos, strlen(strStarPos), &strStarPos_WriteLen);
			//set end position
			viWrite(vi, (unsigned char *)strStopPos, strlen(strStopPos), &strStopPos_WriteLen);
			//send get data commend
			viWrite(vi, (unsigned char *)strGetData, strlen(strGetData), &strGetData_WriteLen);
			//read data from ram of oscilloscope
			viRead(vi, buffer, SINGLE_READ_MAX_LEN + RIGOL_HEAD_MAX_LEN, &readLen);

			fwrite(buffer + RIGOL_HEAD_MAX_LEN, 1, readLen - RIGOL_HEAD_MAX_LEN, fp);
		}
		viWrite(vi, (unsigned char *)strRun, strlen(strRun), &strStop_WriteLen);

		fclose(fp);
        delay(700000000);
	}

	viClose (vi);
	viClose (defaultRM);
	free(buffer);
	free(matches);

    DataFormatConverse();
	printf("converse finished\n");

	int match_file_num = 0;
	while(1)
	{
		char sou_file_name[64];
		sprintf(sou_file_name, "C:\\rigol_data\\data_oscillo\\oscillo_valid_data_%d.bin", match_file_num++);
	 	if(FindSubstring(sou_file_name) == -1)
        {
            break;
        }
	}
	printf("match finished\n");

	return 0;
}

