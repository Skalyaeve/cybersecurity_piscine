#include <stdio.h>
#include <string.h>

int main(void)
{
    const char key[] = "__stack_check";
    char buffer[100]; // 0x6c - 0x8 = 100

    printf("Please enter key: ");
    scanf("%s", buffer);
    if (strcmp(buffer, key) == 0)
        printf("Good job.\n");
    else
        printf("Nope.\n");
    return 0;
}