#include "../../include/url_decode.h"

static int hex_value(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';

    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;

    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;

    return -1;
}

void url_decode(char *dst, const char *src)
{
    while (*src) {

        if (*src == '%' &&
            isxdigit((unsigned char)src[1]) &&
            isxdigit((unsigned char)src[2])) {

            *dst =
                (hex_value(src[1]) << 4) |
                 hex_value(src[2]);

            src += 3;
        }

        else if (*src == '+') {
            *dst = ' ';
            src++;
        }

        else {
            *dst = *src;
            src++;
        }

        dst++;
    }

    *dst = '\0';
}