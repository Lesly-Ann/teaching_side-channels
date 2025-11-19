// 0_victim_memcmp.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "0_victim_memcmp.h"

char *secret_pwd;
int secret_len;

char *read_from_user(void)
{
    char *buffer = NULL;
    int len;
    size_t size;

    printf("Enter super secret password ('q' to exit): ");
    if ((len = getline(&buffer, &size, stdin)) != -1)
    {
        /* get rid of the terminating newline character */
        buffer[len-1] = '\0';
        printf("--> You entered: '%s'\n", buffer);
        return buffer;
    }
    else
    {
        printf("--> failure to read line\n");
        return NULL;
    }
}

void delay(void)
{
    volatile int i;
    for (i = 0; i < 100; i++);
}

void victim_init(void)
{
    // Deobfuscate password once
    secret_len = strlen(SECRET_PWD);
    secret_pwd = malloc((secret_len + 1) * sizeof(char));
    for (int i = 0; i < secret_len; ++i) {
        secret_pwd[i] = SECRET_PWD[i] ^ OBFUSCATE[i];
    }
    secret_pwd[secret_len] = '\0';
}

int check_pwd(char *user, int user_len)
{
    int i;

    /* reject if incorrect length */
    if (user_len != secret_len)
        return 0;

#if DELAY
    delay();
#endif

    /* reject on first byte mismatch */
    for (i = 0; i < user_len; i++)
    {
        if (user[i] != secret_pwd[i])
            return 0;

#if DELAY
        delay();
#endif
    }

    /* user password passed all the tests */
    return 1;
}

void print_success_art(){
            printf("ACCESS ALLOWED\n");
            printf("________$$$$\n");
            printf("_______$$__$\n");
            printf("_______$___$$\n");
            printf("_______$___$$\n");
            printf("_______$$___$$\n");
            printf("________$____$$\n");
            printf("________$$____$$$\n");
            printf("_________$$_____$$\n");
            printf("_________$$______$$\n");
            printf("__________$_______$$\n");
            printf("____$$$$$$$________$$\n");
            printf("__$$$_______________$$$$$$\n");
            printf("_$$____$$$$____________$$$\n");
            printf("_$___$$$__$$$____________$$\n");
            printf("_$$________$$$____________$\n");
            printf("__$$____$$$$$$____________$\n");
            printf("__$$$$$$$____$$___________$\n");
            printf("__$$_______$$$$___________$\n");
            printf("___$$$$$$$$$__$$_________$$\n");
            printf("____$________$$$$_____$$$$\n");
            printf("____$$____$$$$$$____$$$$$$\n");
            printf("_____$$$$$$____$$__$$\n");
            printf("_______$_____$$$_$$$\n");
            printf("________$$$$$$$$$$\n");
}