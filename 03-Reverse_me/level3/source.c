#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

void ___syscall_malloc(void)
{
    puts("Nope.");
    exit(1);
}

void ____syscall_malloc(void)
{
    puts("Good job.");
    return;
}

int main(void)
{
    // rbp-0x4 = 0000
    int scanf_ret;    // rbp-0x8
    int index1;       // rbp-0xc
    int var1;         // rbp-0x10
    size_t index2;    // rbp-0x18
    char buffer1[9];  // rbp-0x21 (0x21 - 0x18 = 0x9 = 9)
    char buffer2[31]; // rbp-0x40 (0x40 - 0x21 = 0x1F = 31)
    char buffer3[4];  // rbp-0x44 (0x44 - 0x40 = 0x4 = 4)
    bool var_bool;    // rbx-0x45
                      // 3 bytes gap
    size_t var2;      // rbx-0x50 (0x45 + 0x3 + 0x8 = 0x50)

    printf("Please enter key:");
    scanf_ret = scanf("%23s", buffer2);
    if (scanf_ret != 1)
        ___syscall_malloc();
    if (buffer2[1] != '2')
        ___syscall_malloc();
    if (buffer2[0] != '4')
        ___syscall_malloc();

    fflush(0);
    memset(buffer1, 0, 9);
    buffer1[0] = '*';
    buffer3[3] = '\0'; // rbp-0x41
    index2 = 2;
    index1 = 1;
    while (true)
    {
        var_bool = false;
        if (strlen(buffer1) < 8)
        {
            var2 = index2;
            var_bool = var2 < strlen(buffer2);
        }
        if (!var_bool)
            break;
        buffer3[0] = buffer2[index2];
        buffer3[1] = buffer2[index2 + 1];
        buffer3[2] = buffer2[index2 + 2];
        buffer1[index1] = atoi(buffer3);
        index2 += 3;
        index1++;
    }
    buffer1[index1] = '\0';

    var1 = strcmp(buffer1, "********");
    if (var1 == -2)
        ___syscall_malloc();
    else if (var1 == -1)
        ___syscall_malloc();
    else if (var1 == 0)
        ____syscall_malloc();
    else if (var1 == 1)
        ___syscall_malloc();
    else if (var1 == 2)
        ___syscall_malloc();
    else if (var1 == 3)
        ___syscall_malloc();
    else if (var1 == 4)
        ___syscall_malloc();
    else if (var1 == 5)
        ___syscall_malloc();
    else if (var1 == 115) // 0x73 = 115
        ___syscall_malloc();
    else
        ___syscall_malloc();
    return 0;
}