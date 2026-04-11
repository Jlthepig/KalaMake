#include <stdio.h>

void print(const char* msg)
{
    printf("[LOG] %s\n", msg);
}

int main()
{
    print("hello from c23!");
    return 0;
}