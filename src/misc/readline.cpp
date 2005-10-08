#include "readline.h"
#include <errno.h>

int
readline(FILE * fd, void *vptr, int maxlen)
{
	int n, rc;
    char    c, *ptr;

    ptr = (char *)vptr;
    for (n = 1; n < maxlen; n++) {
      again:
        if ( (rc = fread(&c, 1, 1, fd)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;          /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);     /* EOF, n - 1 bytes were read */
        } else {
            if (errno == EINTR)
                goto again;
            return (-1);        /* error, errno set by read() */
        }
    }

    *ptr = 0;                   /* null terminate like fgets() */
    return (n);
}