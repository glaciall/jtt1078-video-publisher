#include <stdio.h>

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
			printf("found: ");
			for (k = 0, len = strlen(text); chr = text[k + pLen]; k++)
			{
				fputc(chr, stdout);
			}
			printf("\n");
			fflush(stdout);
		}
	}
}

int str_starts_with(char *a, char *b)
{
	printf("is %s starts with %s\n", a, b);
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

int main(int argc, char **argv)
{
	get_opt(argc, argv, "--video-encoding=", NULL, NULL);

	return 0;
}
