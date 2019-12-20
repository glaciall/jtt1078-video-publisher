#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "util/common.h"
#include "util/linkedlist.c"
#include "acodec/codec.c"

unsigned char buffer[4096];
unsigned int buffer_offset = 0;
unsigned int buffer_size = 0;
unsigned char *ENCODING[] = {
	"reserved",             // 0
	"G.721",                // 1
	"G.722",                // 2
	"G.723",                // 3
	"G.728",                // 4
	"G.729",                // 5
	"G.711A",               // 6
	"G.711U",               // 7
	"G.726",                // 8
	"G.729A",               // 9
	"DVI4_3",               // 10
	"DVI4_4",               // 11
	"DVI4_8K",              // 12
	"DVI4_16K",             // 13
	"LPC",                  // 14
	"S16BE_STEREO",         // 15
	"S16BE_MONO",           // 16
	"MPEGAUDIO",            // 17
	"LPCM",                 // 18
	"AAC",                  // 19
	"WMA9STD",              // 20
	"HEAAC",                // 21
	"PCM_VOICE",            // 22
	"PCM_AUDIO",            // 23
	"AACLC",                // 24
	"MP3",                  // 25
	"ADPCMA",               // 26
	"MP4AUDIO",             // 27
	"AMR"                   // 28
};
int audioCodecId;

struct publisher
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct node_t llist;
    unsigned long long currentIndex;
} videoPublisher, audioPublisher;

unsigned long long sequence = 0;

FILE *input;
FILE *videoFifoFile;
FILE *audioFifoFile;
FILE *logFile;

pid_t pid, cpid;

unsigned char fifoPath[128];
unsigned char filePath[140];
unsigned char videoEncoding[] = "h264";
unsigned char rtmpUrl[128];
unsigned char audioRate[16] = "8000";
unsigned char audioChannel[4] = "1";

pthread_t video_publish_worker, audio_publish_worker;

int exit_no_arg(char *msg)
{
    logger(msg);
    fprintf("usage: jtt1078 <--fifo-path=> <--video-encoding=h264> <--audio-encoding=> <--rtmp-url=rtmp://server/app/stream>\n", stderr);
    return 0;
}

void *video_publish_func(void *arg)
{
    logger("video publish worker started...");
    int ret;
    struct timespec ts;
    struct node_t *node;

    // printf("open %s.video\n", fifoPath);
    char filePath[128];
    sprintf(filePath, "%s.video", fifoPath);
    videoFifoFile = fopen(filePath, "a+");
    if (videoFifoFile == NULL)
    {
        return logger("cannot open fifo for video stream"), 1;
    }

    while (1)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        // lock and wait
        pthread_mutex_lock(&videoPublisher.mutex);
        while (videoPublisher.llist.count == 0)
        {
            // TODO: check child process running state...
            ret = pthread_cond_timedwait(&videoPublisher.cond, &videoPublisher.mutex, &ts);
            if (ret == 0)
            {
                break;
            }
            ts.tv_sec += 1;
            logger("video publisher timedout");
        }
        node_remove_first(&videoPublisher.llist, &node);
        pthread_mutex_unlock(&videoPublisher.mutex);

        // logger("ready to write...");
        fwrite(node->data, 1, node->dataLength, videoFifoFile);
        fflush(videoFifoFile);
    }
    return NULL;
}

void *audio_publish_func(void *arg)
{
    logger("audio publish worker started...");
    int ret;
    struct timespec ts;
    struct node_t *node;
    char outBuff[1024];
    int outBuffLen;
    AUDIO_CODEC *codec;

    // sprintf(filePath, "%s.audio", fifoPath);
    // printf("open %s.audio\n", fifoPath);
    char filePath[128];
    sprintf(filePath, "%s.audio", fifoPath);
    audioFifoFile = fopen(filePath, "a+");
    if (audioFifoFile == NULL)
    {
        return logger("cannot open fifo for audio stream"), 1;
    }

    while (1)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        // lock and wait
        pthread_mutex_lock(&audioPublisher.mutex);
        while (audioPublisher.llist.count == 0)
        {
            // TODO: check child process running state...
            ret = pthread_cond_timedwait(&audioPublisher.cond, &audioPublisher.mutex, &ts);
            if (ret == 0)
            {
                break;
            }
            ts.tv_sec += 1;
            logger("audio publisher timedout");
        }
        node_remove_first(&audioPublisher.llist, &node);
        pthread_mutex_unlock(&audioPublisher.mutex);

        // RAW -> PCM
        switch (audioCodecId)
        {
            case  6 : codec = g711a_to_pcm; break;
            case  7 : codec = g711u_to_pcm; break;
            case  8 : codec = g726_to_pcm; break;
            case 26 : codec = adpcm_to_pcm; break;
            default : codec = raw_to_pcm;
        }

        ret = codec(node->data, node->dataLength, outBuff, &outBuffLen);
        // TODO: if ret != 0 ....

        fwrite(outBuff, 1, outBuffLen, audioFifoFile);
        fflush(audioFifoFile);
    }

    return NULL;
}

// distribute audio/video segment to separate threads
void distribute(char *data, int start, int len, unsigned long long vSequence, struct publisher *publisher)
{
    char *temp = (char *)malloc(len);
    array_copy(data, start, temp, 0, len);
    pthread_mutex_lock(&publisher->mutex);
    node_add_last(&publisher->llist, temp, len, vSequence);
    pthread_mutex_unlock(&publisher->mutex);
    pthread_cond_signal(&publisher->cond);
    free(temp);
}

void terminate(int signal)
{
    logger("Ctrl + Break terminate signal...");
    kill(cpid, SIGINT);
    exit(0);
}

int main(int argc, char **argv)
{
    int status;
	int audioEncodingPrinted = 0;
	int videoEncodingPrinted = 0;
	unsigned int i, len, lOffset, dType, pLen, bodyLength, pt;
	unsigned char block[1024];
	unsigned char msg[128];
	unsigned char command[512];
	unsigned char videoFilePath[128];
	unsigned char audioFilePath[128];

    memset(fifoPath, 0, 128);
    memset(rtmpUrl, 0, 128);

    if (get_opt(argc, argv, "--fifo-path=", fifoPath, 127) == 0) return exit_no_arg("no argument for fifo path"), 1;
    if (get_opt(argc, argv, "--rtmp-url=", rtmpUrl, 127) == 0) return exit_no_arg("no argument for rtmp url"), 1;
    get_opt(argc, argv, "--audio-rate=", audioRate, 15);
    get_opt(argc, argv, "--audio-channel=", audioChannel, 3);

    sprintf(videoFilePath, "%s.video", fifoPath);
    sprintf(audioFilePath, "%s.audio", fifoPath);

    // (cpid = fork()) == 0
    if ((cpid = fork()) == 0)
    {
        execl("/usr/local/bin/ffmpeg", "ffmpeg", "-re", "-r", "25", "-f", "h264", "-i", videoFilePath, "-f", "s16le", "-ar", "8000", "-ac", "1", "-i", audioFilePath, "-vcodec", "copy", "-acodec", "aac", "-probesize", "512", "-f", "flv", rtmpUrl, NULL);
    }
    else
    {
        remove(videoFilePath);
        remove(audioFilePath);
        if (mkfifo(videoFilePath, 0666) != 0) return logger("create fifo for video stream failed"), 1;
        if (mkfifo(audioFilePath, 0666) != 0) return logger("create fifo for audio stream failed"), 1;

        sleep(1);
        pid = getpid();
        // signal(SIGINT, terminate);

        // sprintf(msg, "log-%ld.log", pid);
        // logFile = fopen(msg, "a+");

        pthread_mutex_init(&videoPublisher.mutex, NULL);
        pthread_cond_init(&videoPublisher.cond, NULL);

        pthread_mutex_init(&audioPublisher.mutex, NULL);
        pthread_cond_init(&audioPublisher.cond, NULL);

        pthread_create(&video_publish_worker, NULL, video_publish_func, NULL);
        pthread_create(&audio_publish_worker, NULL, audio_publish_func, NULL);

        logger("threads created...");

        while (!feof(stdin))
        {
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
                    distribute(buffer, lOffset + 2, bodyLength, sequence++, &videoPublisher);
                }
                else if (dType == 0x03)
                {
                    if (audioEncodingPrinted == 0)
                    {
                        if (pt > 28) strcpy(msg, "audio encoding: unknown");
                        else sprintf(msg, "audio encoding: %s", *(ENCODING + pt));
                        logger(msg);
                        audioEncodingPrinted = 1;
                        audioCodecId = pt;
                    }
                    distribute(buffer, lOffset + 2, bodyLength, sequence, &audioPublisher);
                }

                for (i = 0, len = buffer_size - pLen; i < len; i++)
                {
                    buffer[i] = buffer[pLen + i];
                }

                buffer_size -= pLen;
                buffer_offset -= pLen;
            }
        }

        wait(&status);
        // fclose(logFile);
    }

	return 0;
}
