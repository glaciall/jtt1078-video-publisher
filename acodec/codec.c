#include "adpcm/codec.c"
#include "g711/codec.c"
#include "g726/codec.c"

typedef int AUDO_CODEC(char *, int, char *, int);

/**
 * copy raw data to outBuff, returning non-zero value for errors
 */
int raw_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    int i = 0;
    for (; i < inBuffLen; i++) outBuff[i] = inBuff[i];
    *outBuffLen = inBuffLen;
    return 0;
}

int adpcm_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    return 0;
}

int g711a_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    return 0;
}

int g711u_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    return 0;
}

int g726_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    return 0;
}

