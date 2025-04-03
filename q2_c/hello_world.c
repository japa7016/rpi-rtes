#define VERSION3

// breaking Rule 8 by using variable argument macro
#define PRINT_ERR(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
// breaking Rule 8 by using recursive macro
#define RECURSE(x) (x + RECURSE(x))
// breaking Rule 8 by using conditional compilation derivatives
#if defined(VERSION1)
    #define VERSION "1.0"
#elif defined(VERSION2)
    #define VERSION "2.0"
#elif defined(VERSION3)
    #define VERSION "3.0"
#elif defined(VERSION4)
    #define VERSION "4.0"
#elif defined(VERSION5)
    #define VERSION "5.0"
#elif defined(VERSION6)
    #define VERSION "6.0"
#elif defined(VERSION7)
    #define VERSION "7.0"
#elif defined(VERSION8)
    #define VERSION "8.0"
#elif defined(VERSION9)
    #define VERSION "9.0"
#elif defined(VERSION10)
    #define VERSION "10.0"
#else
    #define VERSION "unknown"
#endif

// breaking rule 9 by hiding pointer in a macro
#define GET_VALUE(p) (*(p))
// breaking rule 9 by using function pointer
typedef int (*FuncPtr)(int);

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

// Global variable and array for breaking rule 6
int counter = 0;
static int reusedValue = 5;
char globalData[50];

void rule_1(void);
void rule_2(void);
void rule_3(void);
void rule_4(void);
void rule_5(void);
void rule_6(void);
void rule_7(void);
void rule_8(void);
void rule_9(void);
void rule_10(void);

int main(void)
{
    rule_1();
    rule_2();
    rule_3();
    rule_4();
    rule_5();
    rule_6();
    rule_7();
    rule_8();
    rule_9();
    rule_10();

    return 0;
}

// breaking rule_1 using goto
void rule_1(void)
{
    int a = 1;
    if (a==1)
    {
        goto here;
        printf("This would not print");
    }
    here:
        printf("Rule 1 broken!\n\n");
}

// breaking rule 2 without any fixed upper bound
void rule_2(void)
{
    srand(time(NULL));
    int bound = rand() % 100;
    for(int i = 0; i < bound; i++)
    {
        printf("%d,",i);
    }
    printf("\nRule 2 broken!\n\n");

}

// Breaking rule 3 by allocating memory after initialization and
// not freeing the memory
void rule_3(void)
{
    char *msg = (char *)malloc(32);
    strcpy(msg, "Rule 3 broken!\n");
    printf("%s\n", msg);

    // not freeing the allocated memory(Rule 3 broken, memory leak introduced)
    // free(msg);
}

// Breaking rule 4 by making function more than 60 lines
void rule_4(void)
{
    int sum = 0;
    int i;
    
    for (i = 0; i < 10; i++) 
    {
        sum += i;
    }

    for (i = 10; i < 20; i++) 
    {
        sum += i;
    }

    for (i = 20; i < 30; i++) 
    {
        sum += i;
    }

    for (i = 30; i < 40; i++) 
    {
        sum += i;
    }

    for (i = 40; i < 50; i++) 
    {
        sum += i;
    }

    for (i = 50; i < 60; i++) 
    {
        sum += i;
    }

    for (i = 60; i < 70; i++) 
    {
        sum += i;
    }

    for (i = 70; i < 80; i++) 
    {
        sum += i;
    }

    for (i = 80; i < 90; i++) 
    {
        sum += i;
    }

    for (i = 90; i < 100; i++) 
    {
        sum += i;
    }

    printf("Final sum: %d\n", sum);
    printf("Rule 4 broken!\n\n");
}

void rule_5(void)
{
    int x = 0;
    
    //trivial assertion
    assert(1 == 1);

    //This causes side effect
    assert((x = 42));

    printf("x is now: %d\n", x);

    // No handling for failing assertion

    printf("Rule 5 broken!\n\n");
}

void rule_6(void)
{
    int i, total;
    total = 0;

    for (i = 0; i < 10; i++)
    {
        // This should have been declared locally(only used here)
        globalData[i] = 'a' + i; 
        total += i;
    }
    printf("Total = %d\n", total);

    // Using the global variable to reassign it something unrelated
    reusedValue += total;

    // Again this should be local(only used here)
    counter += reusedValue;
    printf("Global Counter: %d, Reused Value: %d\n", counter, reusedValue);
    printf("Rule 6 broken!\n\n");
}

// Returning something for rule 7
int random_func(int value)
{
    return (value *2);
}

void rule_7(void)
{
    //Not checking the return value
    random_func(4);

    (void)printf("This call to printf is explicitly ignored.\n");
    
    printf("Rule 7 broken!\n\n");
}

// breaking rule 8 by defining macros(see the top of the file)
void rule_8(void)
{
    printf("Version: %s\n", VERSION);
    printf("Rule 8 broken!\n\n");
}

void rule_9(void)
{
    int x = 10;
    int *ptr = &x;
    // More than one level of dereferencing
    int **pptr = &ptr;

    // Using the macro that hides pointer dereference.
    int val2 = GET_VALUE(ptr);

    // using function pointer
    FuncPtr fptr = random_func;
    printf("The returned value is %d\n",fptr(2));
    printf("Rule 9 broken!\n\n");
}

// breaking rule 10 by ignoring b to give warning
int add(int a, int b) 
{
    // 'b' is ignored.
    return a; 
}
void rule_10(void)
{
    int result = add(5, 3);
    printf("Result: %d\n", result);

    int x;
    // using uninitialized variable
    printf("Value of x: %d\n", x);
}

