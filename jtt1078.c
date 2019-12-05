#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

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
unsigned char *ENCODING[] = {
	"reserved", "G.721", "G.722", "G.723", "G.728", "G.729",
	"G.711A", "G.711U", "G.726", "G.729A", "DVI4_3",
	"DVI4_4", "DVI4_8K", "DVI4_16K", "LPC", "S16BE_STEREO",
	"S16BE_MONO", "MPEGAUDIO", "LPCM", "AAC", "WMA9STD", "HEAAC",
	"PCM_VOICE", "PCM_AUDIO", "AACLC", "MP3", "ADPCMA", "MP4AUDIO", "AMR"
};
FILE *output;

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
	int audioEncodingPrinted = 0;
	int videoEncodingPrinted = 0;
	unsigned int i, len, lOffset, dType, pLen, bodyLength, pt;
	unsigned char block[1024];
	unsigned char msg[128];

	output = popen("ffmpeg -re -i - -c copy -f flv rtmp://localhost/live/fuck", "w");
	if (output == NULL)
	{
		sprintf(msg, "exec ffmpeg failed: %s", strerror(errno));
		log(msg);
		return 1;
	}

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
			pt = buffer[5] & 0x7f;

			if (dType == 0x00 || dType == 0x01 || dType == 0x02)
			{
				if (videoEncodingPrinted == 0)
				{
					switch (pt)
					{
						case 98 : strcpy(msg, "vidio encoding: H.264"); break;
						case 99 : strcpy(msg, "video encoding: H.265"); break;
						case 100 : strcpy(msg, "video encoding: AVS"); break;
						case 101 : strcpy(msg, "video encoding: SVAC"); break;
						default : strcpy(msg, "video encoding: unknown");
					}
					log(msg);
					videoEncodingPrinted = 1;
				}
				for (i = 0; i < bodyLength; i++)
				{
					fputc(buffer[i + lOffset + 2], output);
				}
				fflush(output);
			}
			else if (dType == 0x03)
			{
				if (audioEncodingPrinted == 0)
				{
					if (pt > 28) strcpy(msg, "audio encoding: unknown");
					else sprintf(msg, "audio encoding: %s", *(ENCODING + pt));
					log(msg);
					// dump(buffer, 64);
					audioEncodingPrinted = 1;
				}
				/*
				for (i = 0; i < bodyLength; i++)
				{
					fputc(buffer[i + lOffset + 2], output);
				}
				fflush(output);
				*/
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

	fclose(output);
	fflush(stdout);
	fclose(stdout);

	return 0;
}
