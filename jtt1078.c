#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define SUCCESS 0

void log(char *msg)
{
	time_t now;
	struct tm *timeinfo;
	char str[64];

	time(&now);
	timeinfo = localtime(&now);
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", timeinfo);

	printf("[jtt1078] %s %s\n", str, msg);
}

unsigned char buffer[4096];
unsigned int buffer_offset = 0;
unsigned int buffer_size = 0;

void dump(unsigned char *data, int len)
{
	int i;
	printf("-------------------------------\n");
	for (i = 0; i < len; i++)
	{
		printf("%02x ", *(data++));
		if (i % 4 == 3) printf("  ");
		if (i % 16 == 15) printf("\n");
	}
	printf("-------------------------------\n");
}

int main(int argc, char **argv)
{
	unsigned int i, len, lOffset, dType, pLen, bodyLength;
	unsigned char block[1024];
	unsigned char msg[128];

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

		// slice from buffer
		// first 4 bytes
		// determine length offset by type

		while (1)
		{
			if (buffer_size < 30) break;
			if (buffer[0] == 0x30 && buffer[1] == 0x31 && buffer[2] == 0x63 && buffer[3] == 0x64) ;
			else
			{
				// dump(buffer, 64);
				// log("invalid header bytes");
				return 1;
			}

			lOffset = 28;
			dType = (buffer[15] >> 4) & 0x0f;
			if (dType == 0x04) lOffset = 28 - 8 - 2 - 2;
			else if (dType == 0x03) lOffset = 28 - 4;
			bodyLength = ((buffer[lOffset] << 8) | (buffer[lOffset + 1])) & 0xffff;
			pLen = bodyLength + lOffset + 2;
		
			// sprintf(msg, "Type: %02x, Length: %04x, pLen: %d, Buffer: %d", dType, bodyLength, pLen, buffer_size);
			// log(msg);

			if (buffer_size < pLen) break;

			if (dType == 0x00 || dType == 0x01 || dType == 0x02)
			{
				for (i = 0; i < bodyLength; i++) fputc(buffer[i + lOffset + 2], stdout);
				fflush(stdout);
			}

			for (i = 0, len = buffer_size - pLen; i < len; i++)
			{
				buffer[i] = buffer[pLen + i];
				// printf("%02x ", buffer[i]);
				// if (i % 16 == 15) printf("\n");
			}
		
			buffer_size -= pLen;
			buffer_offset -= pLen;

			// sprintf(msg, "buffer size: %d", buffer_size);
			// log(msg);

			// dump(buffer, 64);
		}
	}

	return 0;
}
