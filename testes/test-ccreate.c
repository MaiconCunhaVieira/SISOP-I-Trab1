#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void first_func()
{
    printf("\nSou a primeira funcao a ser chamada!");
}

void second_func()
{
    printf("\nSou a segunda funcao a ser chamada!");
}

int third_func()
{
    printf("\nSou a terceira e ultima funcao a ser chamada!");
}

int main()
{
    printf("\nSou a funcao main executando!");

    ccreate((void *) first_func, NULL, 0);
    ccreate((void *) second_func, NULL, 0);
    ccreate((void *) third_func, NULL, 0);

    printf("\nSou a funcao main liberando processamento!");
    cyield();

    printf("\nSou a funcao main retornando a execucao!\n");
    return 0;
}
