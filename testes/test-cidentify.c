#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "../include/support.h"
//#include "../include/cdata.h"
#include "../include/cthread.h"

#define SIZE_OF_STRING 100

int main()
{
    char name[SIZE_OF_STRING] = "";

    int testCIdentify = cidentify(name, SIZE_OF_STRING);
    printf("Grupo: \n%s", name);

    return 0;
}
