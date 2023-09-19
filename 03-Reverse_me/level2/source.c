#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

void no(void)
{
    puts("Nope.");
    exit(1);
}

void ok(void)
{
    puts("Good job.");
    return;
}

int main(void)
{
    // ebp-0x8 = 0000
    int scanf_ret;    // ebp-0xc
    int index1;       // ebp-0x10
    int index2;       // ebp-0x14
    char buffer1[9];  // ebp-0x1d (0x1d - 0x14 = 9)
    char buffer2[24]; // ebp-0x35 (0x35 - 0x1d = 24)
    char buffer3[4];  // ebp-0x39 (0x39 - 0x35 = 4)
                      // 3 bytes gap
                      // ebp-0x40 = .data section ptr (4 bytes)
    bool bool1;       // ebp-0x41
    size_t var1;      // ebp-0x48

    printf("Please enter key:");
    scanf_ret = scanf("%23s", buffer2);
    if (scanf_ret != 1)
        no();
    if (buffer2[1] != '0')
        no();
    if (buffer2[0] != '0')
        no();
    fflush(0);

    memset(buffer1, 0, 9);
    buffer1[0] = 'd';
    buffer3[3] = '\0'; // ebp-0x36
    index2 = 2;
    index1 = 1;
    while (true)
    {
        bool1 = false;
        if (strlen(buffer1) < 8)
        {
            var1 = index2;
            bool1 = var1 < strlen(buffer2);
        }
        if (!bool1)
            break;
        buffer3[0] = buffer2[index2];
        buffer3[1] = buffer2[index2 + 1];
        buffer3[2] = buffer2[index2 + 2];
        buffer1[index1] = atoi(buffer3);
        index2 += 3;
        index1++;
    }
    buffer1[index1] = '\0';

    if (!strcmp(buffer1, "delabere"))
        ok();
    else
        no();
    return 0;
}