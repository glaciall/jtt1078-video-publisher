#include <stdio.h>
#include <time.h>
#include "../util/common.h"
#include "codec.c"

unsigned char buffer[4096];
unsigned int buffer_offset = 0;
unsigned int buffer_size = 0;

int main(int argc, char **argv)
{
	unsigned int i, len, lOffset, dType, pLen, bodyLength, pt;
	unsigned char block[1024];

	unsigned int inBuffLen, outBuffLen;
	unsigned char inBuff[1024];
	unsigned char outBuff[1024];

	while (!feof(stdin))
	{
		// store to buffer
		len = fread(block, 1, sizeof(block), stdin);
		for (i = 0; i < len; i++)
		{
			buffer[i + buffer_offset] = block[i];
		}
		buffer_size += len;
		buffer_offset += len;

		while (1)
		{
			if (buffer_size < 30) break;
			if (buffer[0] == 0x30 && buffer[1] == 0x31 && buffer[2] == 0x63 && buffer[3] == 0x64) ;
			else
			{
				return 1;
			}

			lOffset = 28;
			dType = (buffer[15] >> 4) & 0x0f;
			if (dType == 0x04) lOffset = 28 - 8 - 2 - 2;
			else if (dType == 0x03) lOffset = 28 - 4;
			bodyLength = ((buffer[lOffset] << 8) | (buffer[lOffset + 1])) & 0xffff;
			pLen = bodyLength + lOffset + 2;

			if (buffer_size < pLen) break;
			pt = buffer[5] & 0x7f;

			if (dType == 0x03)
			{
                // distribute(buffer, lOffset + 2, bodyLength, &audioPublisher);
                for (i = 0; i < bodyLength; i++)
                {
                    // fputc(buffer[i + lOffset + 2], stdout);
                    inBuff[i - 0] = buffer[i + lOffset + 2];
                }
                // fflush(stdout);
                adpcm_to_pcm(inBuff, bodyLength, outBuff, &outBuffLen);
                // printf("int: %d, out len: %d\n", bodyLength, outBuffLen);
                fwrite(outBuff, 1, outBuffLen, stdout);
                fflush(stdout);
			}

			for (i = 0, len = buffer_size - pLen; i < len; i++)
			{
				buffer[i] = buffer[pLen + i];
			}

			buffer_size -= pLen;
			buffer_offset -= pLen;
		}
	}

	fflush(stdout);
	fclose(stdout);

	return 0;
}