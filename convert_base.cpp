#include <string>
#include <algorithm>
#include <iostream>

std::string convertBase(const std::string &s, int src_base, int dst_base)
{
    const char *base = "0123456789ABCDEF";
    std::string dividend(s);
    std::string ret;

    while (!dividend.empty())
    {
        int remains = 0;
        std::string quotient;
        for (int i = 0; i < dividend.length(); i++)
        {
            remains = remains * src_base +
                      ((dividend[i] <= 'f') && (dividend[i] >= 'a')) * (dividend[i] - 'a' + 10) +
                      ((dividend[i] <= 'F') && (dividend[i] >= 'A')) * (dividend[i] - 'A' + 10) +
                      ((dividend[i] <= '9') && (dividend[i] >= '0')) * (dividend[i] - '0');
            if (remains >= dst_base)
            {
                quotient.push_back(base[remains / dst_base]);
                remains %= dst_base;
            }
            else
            {
                if (!quotient.empty())
                {
                    quotient.push_back('0');
                }
            }
        }
        dividend = quotient;
        ret.push_back(base[remains]);
    }
    reverse(ret.begin(), ret.end());

    return (ret);
}

int main(void)
{
    std::cout << std::dec << 123456789 << std::endl;
    std::cout << convertBase("123456789", 10, 2) << std::endl;
    std::cout << std::hex << 123456789 << std::endl;

    return 0;
}
