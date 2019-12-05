#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util/common.h"

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

struct publisher
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    node_t llist;
};

FILE *input, *videoFifoFile, *audioFifoFile;

void exit_no_arg(char *msg)
{
    logger(msg);
    printf("usage: jtt1078 <--fifo-path=> <--video-encoding=h264> <--audio-encoding=> <--rtmp-url=rtmp://server/app/stream>");
}

void *video_publish_func(void *arg)
{
    while (1)
    {
        // lock and wait

        // write to fifo
    }
    return NULL;
}

void *audio_publish_func(void *arg)
{
    while (1)
    {
        // lock and wait

        // audio transcode...

        // write to fifo
    }

    return NULL;
}

// distribute audio/video segment to separate threads
void distribute(char *data, int len, struct publisher publisher)
{
    char *temp = (char *)malloc(len);
    array_copy(data, 0, temp, 0, len);
    pthread_mutex_lock(&publisher.mutex);
    node_add_last(publisher.llist, data, len);
    pthread_mutex_unlock(&publisher.mutex);
    pthread_cond_signal(&publisher.cond);
}

int main(int argc, char **argv)
{
	int audioEncodingPrinted = 0;
	int videoEncodingPrinted = 0;
	unsigned int i, len, lOffset, dType, pLen, bodyLength, pt;
	unsigned char block[1024];
	unsigned char msg[128];
	unsigned char fifoPath[128];
	unsigned char filePath[140];
	unsigned char videoEncoding[] = "h264";
	unsigned char rtmpUrl[128];
	unsigned char command[512];

    memset(fifoPath, 0, 128);
    memset(rtmpUrl, 0, 128);

    if (get_opt(argc, argv, "--fifo-path=", fifoPath, 127) == 0) return exit_no_arg("no argument for fifo path"), 1;
    if (get_opt(argc, argv, "--rtmp-url=", rtmpUrl, 127) == 0) return exit_not_arg("no argument for rtmp url"), 1;

    sprintf(command, "ffmpeg -f %s -i %s.video -f alaw -i %s.audio -c copy -f flv %s", videoEncoding, fifoPath, fifoPath, rtmpUrl);
	input = popen(command, "r");
	if (input == NULL)
	{
		sprintf(msg, "exec ffmpeg failed: %s", strerror(errno));
		logger(msg);
		return 1;
	}

    // create fifo file
    sprintf(filePath, "%s.video", fifoPath);
    remove(filePath);
    if (!mkfifo(filePath, 0666)) return logger("create fifo for video stream failed"), 1;
    videoFifoFile = open(filePath, O_WRONLY | O_NONBLOCK);
    if (videoFifoFile != 0) return logger("cannot open fifo for video stream"), 1;

    sprintf(filePath, "%s.audio", fifoPath);
    remove(filePath);
    if (!mkfifo(filePath, 0666)) return logger("create fifo for audio stream failed"), 1;
    audioFifoFile = open(filePath, O_WRONLY | O_NONBLOCK);
    if (audioFifoFile != 0) return logger("cannot open fifo for audio stream"), 1;

    // create threads


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
				// bytes_dump(buffer, 64);
				// logger("invalid header bytes");
				return 1;
			}

			lOffset = 28;
			dType = (buffer[15] >> 4) & 0x0f;
			if (dType == 0x04) lOffset = 28 - 8 - 2 - 2;
			else if (dType == 0x03) lOffset = 28 - 4;
			bodyLength = ((buffer[lOffset] << 8) | (buffer[lOffset + 1])) & 0xffff;
			pLen = bodyLength + lOffset + 2;
		
			// sprintf(msg, "Type: %02x, Length: %04x, pLen: %d, Buffer: %d", dType, bodyLength, pLen, buffer_size);
			// logger(msg);

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
					logger(msg);
					videoEncodingPrinted = 1;
				}
				/*
				for (i = 0; i < bodyLength; i++)
				{
					fputc(buffer[i + lOffset + 2], output);
				}
				fflush(output);
				*/
			}
			else if (dType == 0x03)
			{
				if (audioEncodingPrinted == 0)
				{
					if (pt > 28) strcpy(msg, "audio encoding: unknown");
					else sprintf(msg, "audio encoding: %s", *(ENCODING + pt));
					logger(msg);
					// bytes_dump(buffer, 64);
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
			}
		
			buffer_size -= pLen;
			buffer_offset -= pLen;
		}
	}

	pclose(input);
	fflush(stdout);
	fclose(stdout);

	return 0;
}
