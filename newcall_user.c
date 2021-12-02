#include <stdio.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
int main(int argc, char *argv[])
{
    int count, pid, i, x, next, x2;
    long result;
    char *buff;
    if (argc <= 1)
    {
        printf("pid not provided\n");
        return -1;
    }
    pid = atoi(argv[1]);
    count = syscall(334, pid);
    if (count < 0)
    {
        printf("Error with mm_struct\n");
        return -1;
    }
    if (count == 0)
    {
        printf("Has no environ\n");
        return 0;
    }
    buff = (char *)malloc(count * sizeof(char));
    result = syscall(333, pid, buff, count);
    printf("System call new_call returned %ld\n", result);
    if (result > 0)
    {
        x = 1;
        printf("\n\nSTART OF ENVIRON\n");
        for (i = 0; i < result; i++)
        {
            printf("%d-", x++);
            x2 = 0;
            for (next = i; next < result && x2 < 2; next++)
            {
                if (buff[next] == '=' && isupper(buff[next - 1]))
                {
                    x2++;
                }
            }
            next--;
            if (next != result - 1)
            {
                for (next--; isupper(buff[next]) || buff[next] == '_' || isdigit(buff[next]); next--)
                {
                }
            }
            for (x2 = i; x2 <= next; x2++)
            {
                printf("%c", buff[x2]);
            }
            i = next + 1;
            printf("\n");
        }
        printf("\nEND OF ENVIRON\n\n\n");
    }
    else
    {
        printf("Some error occurred!\n");
    }
    return 0;
}