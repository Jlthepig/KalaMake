#include <stdio.h>

void print(msg) char* msg;
{
    printf("[LOG] %s\n", msg);
}

int main()
{
    print("hello from c89!");
    return 0;
}