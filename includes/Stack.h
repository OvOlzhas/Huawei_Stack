#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define data_t int
#define canary_l_const 0xFFAAFFBBFAFBFCFD
#define canary_r_const 0xAAFFBBAAFFCCABCD
#define canary_t unsigned long long
#define hash_base_const 5381
#define hash_data_const 3571
#define poison 0xF0

#define WithCanary
#define WithHash

#define ErrorCheck_(error) {                                        \
    if (error == NULL_STACK) assert(!"Null stack");                 \
    if (error == CREATED_STACK) assert(!"Repeat call StackCtor");   \
    if (error == HASH_BASE_DAMAGED) assert(!"HashBase damaged");    \
    if (error == HASH_DATA_DAMAGED) assert(!"HashData damaged");    \
    if (error == NO_MEMORY) assert(!"No memory");                   \
    if (error == EMPTY_STACK) assert(!"Empty stack");               \
}

#define ErrorCheck(FUNC) {      \
    StackERROR error = FUNC;    \
    ErrorCheck_(error);         \
}

enum StackERROR {
    OK_STACK = 0,
    NULL_STACK,
    CREATED_STACK,
    HASH_BASE_DAMAGED,
    HASH_DATA_DAMAGED,
    NO_MEMORY,
    EMPTY_STACK,
};

struct Stack {
#ifdef WithCanary
    canary_t canary_l;
#endif
    data_t* data;
    size_t capacity;
    size_t size;
#ifdef WithCanary
    canary_t canary_r;
#endif
#ifdef WithHash
    unsigned long long hash_base;
    unsigned long long hash_data;
#endif
};

void CanaryCheck (Stack* stk);

void HashCheck (Stack* stk);

unsigned long long HashBaseCounter (Stack* stk, StackERROR* error = nullptr);

unsigned long long HashDataCounter (Stack* stk, StackERROR* error = nullptr);

void UpdateHash (Stack* stk);

StackERROR StackRealloc(Stack* stk);

StackERROR StackCtor (Stack* stk);

StackERROR StackPush (Stack* stk, data_t value);

StackERROR StackPop (Stack* stk, data_t* pop_elem = nullptr);

StackERROR StackDtor (Stack* stk);
