#include <stdio.h>
#include <faac.h>

char * pcm_to_aac(unsigned char *inputBuffer, int inputBufferLength,int sample_rate, int channels, int sample_bit, char *output)
{
	unsigned long inputSamples = 0;
	unsigned long maxOutputBytes = 0;
	faacEncHandle faacEncHandle1 = faacEncOpen(sample_rate, channels, &inputSamples, &maxOutputBytes);

	int frameSize = (int)(inputSamples * channels * sample_bit / 8);
	faacEncConfigurationPtr faacEncConfiguration1 = faacEncGetCurrentConfiguration(faacEncHandle1);
	faacEncConfiguration1->inputFormat = FAAC_INPUT_16BIT;
	faacEncConfiguration1->outputFormat = 1;
	faacEncConfiguration1->useTns = 1;
	faacEncConfiguration1->useLfe = 0;
	faacEncConfiguration1->aacObjectType = LOW;
	faacEncConfiguration1->shortctl = SHORTCTL_NORMAL;
	faacEncConfiguration1->quantqual = 100;
	faacEncConfiguration1->bandWidth = 0;
	faacEncConfiguration1->bitRate = 0;
	// printf("pcm_to_aac frameSize:%d \n", frameSize);
	int nRet = faacEncSetConfiguration(faacEncHandle1, faacEncConfiguration1);
	if (nRet < 0)
	{
		// printf("set faac configuration failed!");
		return 0;
	}

	unsigned char *buff = (unsigned char *)malloc(inputSamples);
	unsigned char *outBuff = (unsigned char *)malloc(maxOutputBytes);
	// unsigned char *output = (unsigned char *)malloc(4096 * 1000);

	int i, k, s, f = 0, x = 0;
	int totalBytes = 0;
	int writtenBytes = 0;
	while (1)
	{
		if (f >= inputBufferLength) { /*printf("read finished...\n");*/ break; }
		for (i = 0, k = 0; i < frameSize && f < inputBufferLength; i++, k++)
		{
			buff[i] = inputBuffer[f++];
		}
		writtenBytes += k;
		int nInputSamples = k / (sample_bit / 8);
		s = faacEncEncode(faacEncHandle1, buff, nInputSamples, outBuff, maxOutputBytes);
		for (i = 0; i < s; i++) output[x++] = outBuff[i];
		totalBytes += s;
		// printf("encode: %d\n", s);
	}
	printf("total written: %d\n", writtenBytes);

	while (1)
	{
		s = faacEncEncode(faacEncHandle1, NULL, 0, outBuff, maxOutputBytes);
		if (s > 0)
		{
			for (i = 0; i < s; i++) output[x++] = outBuff[i];
			totalBytes += s;
		}
		else break;
	}

	faacEncClose(faacEncHandle1);

	return totalBytes;

	/*
	while (1)
	{
		int outLength = faacEncEncode(faacEncHandle1, NULL, 0, ucOutBuff, maxOutputBytes);
		if (outLength > 0)
		{
			// fwrite(ucOutBuff, 1, outLength, pOutFile);
			for (k = 0; k < outLength; k++) output[s++] = *ucOutBuff++;
			outputLength += outLength;
			memset(ucOutBuff, 0, sizeof(ucOutBuff));
			printf("read...\n");
		}
		else
		{
			printf("faacEncEncode no data \n");
			break;
		}
	}
	return outputLength;
	*/
}

int main()
{
	FILE *fp = fopen("fuck.pcm", "r");
	unsigned char input[4096 * 1024];
	memset(input, 0, sizeof(input));

	char block[1024];
	int offset = 0, i, totalRead = 0;
	while (!feof(fp))
	{
		int len = fread(block, 1, 1024, fp);
		if (len < 1) continue;
		for (i = 0; i < len; i++) input[i + offset] = block[i];
		offset += len;
		totalRead += len;
	}
	fclose(fp);

	printf("total read: %d\n", totalRead);

	char output[4096 * 1000];
	int len = pcm_to_aac(input, offset, 8000, 1, 16, output);
	fp = fopen("fuck.aac", "w");
	fwrite(output, 1, len, fp);
	fflush(fp);
	fclose(fp);
	printf("done...\n");
}

