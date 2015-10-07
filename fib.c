/*
 * C Program to Compute First N Fibonacci Numbers using Command Line Arguments
 */
#include <stdio.h>
 #include <time.h>
 
/* Global Variable Declaration */
int first = 0;
int second = 1;
int third;
/* Function Prototype */
void rec_fibonacci(int);
 
void main(int argc, char *argv[])/* Command line Arguments*/
{
    int number = atoi(argv[1]);
    //printf("%d\t%d", first, second); /* To print first and second number of fibonacci series */
    //rec_fibonacci(number);
    sleep(number);
    //printf("\n");
    printf("done, sleep = %d\n", number);
}
 
/* Code to print fibonacci series using recursive function */
void rec_fibonacci(int num)
{
    if (num == 2)    /* To exit the function as the first two numbers are already printed */
    {
        return;
    }
    third = first + second;
    //printf("\t%d", third);
    first = second;
    second = third;
    num--;
    rec_fibonacci(num);
}