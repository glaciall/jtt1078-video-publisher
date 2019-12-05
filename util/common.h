
/**
 * print logs to stdout
 */
void logger(char *msg)
{
	time_t now;
	struct tm *timeinfo;
	char str[64];

	time(&now);
	timeinfo = localtime(&now);
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", timeinfo);

	printf("[jtt1078] %s %s\n", str, msg);
}

/**
 * print byte array
 */
void bytes_dump(unsigned char *data, int len)
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

/**
 * get command parameter
 */
int get_opt(int argc, char **args, char *prefix, char *dest, int max_len)
{
	int i, k, len, pLen;
	char chr;
	char *text;
	for (i = 0; i < argc; i++)
	{
		text = *args++;
		if (str_starts_with(text, prefix))
		{
			pLen = strlen(prefix);
			for (k = 0, len = strlen(text); (chr = text[k + pLen]) && k < max_len; k++)
			{
			    *dest++ = chr;
			}
			*dest++ = '\0';
			return k;
		}
	}
	return 0;
}

// test if str a starts with b, non-zero value for found
int str_starts_with(char *a, char *b)
{
    int i, k, j;
    char cha, chb;
    for (i = 0; ; i++)
    {
        cha = a[i];
        chb = b[i];
        if (cha == 0 || chb == 0) return 1;
        if (cha != chb) return 0;
    }
    return 1;
}