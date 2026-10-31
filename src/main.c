#include<stdio.h>
#include<stdlib.h>

// argc is the count of command-line arguments, and argv is an array of strings representing those arguments.
// Input: argc includes the program name: 1 means no IP, 2 means an IP was given.
// argv[0] is the program name, argv[1] is the IP address if provided.
int main(int argc, char *argv[])
{
    if (argc > 2) {
        printf("Usage: %s [receiver-ip]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if(argc == 2)
    {
        printf("IP Address: %s\n", argv[1]);
    }
    else
    {
        printf("Please provide an IP address.\n");
    }
    return 0;
}