// Test harness for simple IDE
// Malcolm Harrow, November 2024
//
// MIT license
//
// Derived from the Mackerel IDE example:
//   https://github.com/crmaykish/mackerel-68k/blob/master/firmware/hello.c

#include "ide.h"
#include <stdio.h>
#include <string.h>

void hexdump(uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i += 16)
    {
        printf("%08x  ", i);
        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < len)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        }
        printf(" ");
        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                uint8_t c = buf[i + j];
                if (c >= 32 && c <= 126)
                    printf("%c", c);
                else
                    printf(".");
            }
        }
        printf("\n");
    }
}

int main()
{
    uint16_t buf[256] = {0};

    printf("IDE Demo %08X\n", IDE_COMMAND);

    printf("Reset IDE device\n");
    for (;;) {
        MEM(IDE_COMMAND) = IDE_CMD_RESET;
        printf("*");
    }
    printf("Reset2 IDE device\n");
    IDE_wait_for_device_ready();

    printf("Get drive info\n");
    IDE_device_info(buf);
    hexdump((uint8_t *)buf, 512);

    printf("Read sector 1\n");
    IDE_read_sector(buf, 1);
    hexdump((uint8_t *)buf, 512);

    printf("Read sector 2\n");
    IDE_read_sector(buf, 2);
    hexdump((uint8_t *)buf, 512);

    printf("Done\n");

    return 0;
}
