#include <stdio.h>

char *int2base(int num, int base)
{
    static char retstr[33] = {0};
    unsigned int temp = num;
    char *p = NULL;

    if ((base >= 2) && (base <= 36))
    {
        p = &retstr[sizeof(retstr) - 1];
        *p = '\0';
        do
        {
            *--p = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[temp % base];
            temp /= base;
        } while (temp != 0);
    }

    return p;
}

int main(void)
{
    char *p_2str = NULL;

    p_2str = int2base(-10, 16);
    printf("½øÖÆ×ª»»:%s\n", p_2str);

    return 0;
}
