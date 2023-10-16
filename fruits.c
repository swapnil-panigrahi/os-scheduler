#include <stdio.h>
#include <stdlib.h>
#include "dummy_main.h"

int fib(int n){
    if (n==1 || n==0) return n;
    else return fib(n-2)+fib(n-1);
}

int main(int argc, char** argv){
    printf("%d\n",fib(42));
    return 0;
}