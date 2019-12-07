#include "adpcm/codec.c"
#include "g711/codec.c"
#include "g726/codec.c"

typedef int AUDIO_CODEC(char *, int, char *, int *);

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

adpcm_state adpcm_state_instance;
int adpcm_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    int i = 4;
    char indata[inBuffLen - 4];
    for (; i < inBuffLen; i++) indata[i] = inBuff[i];
    adpcm_coder((short *)indata, outBuff, (inBuffLen - 4) / 2, &adpcm_state_instance);
    return 0;
}

int g711a_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    g711a_decode((short *)outBuff, inBuff, inBuffLen);
    *outBuffLen = inBuffLen * 2;
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
