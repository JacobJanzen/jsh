#include <config.h>
#include <stdio.h>

int main(void)
{
    printf("%s %d.%d\n", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR);
    printf("Hello, world!\n");

    return 0;
}
