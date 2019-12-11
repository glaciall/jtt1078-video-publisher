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

adpcm_state adpcm_state_instance = { valprev : 0, index : 0 };
int adpcm_to_pcm(char *inBuff, int inBuffLen, char *outBuff, int *outBuffLen)
{
    int i = 8, k, l = (inBuffLen - 8);
    char indata[l];
    for (k = 0; i < inBuffLen; i++, k++) indata[k] = inBuff[i];
    // 00 01 02 03 04 05 06 07
    // -----------|----------|
    //    海思头   |prev|idx--|
    // 00 01 52 00 11 22 33 44
    adpcm_state_instance.valprev = (inBuff[5] << 8) | (inBuff[4] & 0xff);
    adpcm_state_instance.index = inBuff[6];
    adpcm_decoder(indata, (short *)outBuff, l * 2, &adpcm_state_instance);
    *outBuffLen = l * 4;
    return 0;
}

// TODO: 以下三类都需要测试验证
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
