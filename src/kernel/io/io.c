#include <io/io.h>

void io_wait()
{
    outbyte(0x80, 0);
}
