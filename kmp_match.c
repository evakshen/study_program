#include <stdio.h>

static void GetNext(const char *pattern, int p_len, int *next)
{
    int i = 0;
    int j = -1;

    next[0] = -1;

    while (i < p_len)
    {
        if ((-1 == j) || (pattern[i] == pattern[j]))
        {
            i++;
            j++;

            if (pattern[i] != pattern[j])
            {
                next[i] = j;
            }
            else
            {
                next[i] = next[j];
            }
        }
        else
        {
            j = next[j];
        }
    }
}

int IndexKMP(const char *text, const char *pattern)
{
    int i = 0, j = 0;
    int s_len = 0, p_len = 0;
    int next[2048] = {0};
    int ret = -1;

    if ((NULL != text) && (NULL != pattern))
    {
        while ('\0' != text[s_len])
        {
            s_len++;
        }

        while ('\0' != pattern[p_len])
        {
            p_len++;
        }

        if (p_len <= sizeof(next))
        {
            GetNext(pattern, p_len, next);

            while ((i < s_len) && (j < p_len))
            {
                if ((-1 == j) || (text[i] == pattern[j]))
                {
                    i++;
                    j++;
                }
                else
                {
                    j = next[j];
                }
            }

            if (j == p_len)
            {
                ret = i - j;
            }
        }
        else
        {
            ret = -2;
        }
    }
    else
    {
        ret = -3;
    }

    return (ret);
}

int main(void)
{
    char *text = "bbc abcdab abcdabcdabde";
    char *pattern = "abcdabd";
    int idx = 0;

    idx = IndexKMP(text, pattern);

    printf("×Ö·û´®Î»ÖÃ:%d\n", idx);
    if (idx >= 0)
    {
        printf("Ê£Óà×Ö·û´®:%s\n", &text[idx]);
    }

    return 0;
}
