#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define data_t int
#define canary_l_const 0xFFAAFFBBFAFBFCFD
#define canary_r_const 0xAAFFBBAAFFCCABCD
#define canary_t unsigned long long
#define hash_stack_const 5381
#define hash_data_const 3571
#define poison 0xF0

#define StackCheck(stk) {                                                           \
    if (stk == nullptr) assert(!"Null stack");                                      \
    if (stk->size > stk->capacity) assert(!"size > capacity");                      \
    if (stk->canary_l != canary_l_const) assert(!"Left canary of stack damaged");   \
    if (stk->canary_r != canary_r_const) assert(!"Right canary of stack damaged");  \
    if (((canary_t *)stk->data)[-1] == canary_l_const)                              \
        assert(!"Left canary of data damaged");                                     \
    if (*((canary_t *)(stk->data[stk->capacity])) == canary_r_const)                \
        assert(!"Right canary of data damaged");                                    \
    StackERROR error = OK_STACK;                                                    \
    HashBaseCounter(stk, &error);                                                   \
    if (error) assert(!"HashBase damaged");                                         \
    HashDataCounter(stk, &error);                                                   \
    if (error) assert(!"HashData damaged");                                         \
}

enum StackERROR {
    OK_STACK = 0,
    NULL_STACK,
    CREATED_STACK,
    HASH_STACK_DAMAGED,
    HASH_DATA_DAMAGED,
    NO_MEMORY,
    EMPTY_STACK,
};

struct Stack {
    canary_t canary_l;
    data_t* data;
    size_t capacity;
    size_t size;
    canary_t canary_r;
    unsigned long long hash_stack;
    unsigned long long hash_data;
};

StackERROR StackOK (Stack* stk) {
    if (stk == nullptr)
        return NULL_STACK;
}

unsigned long long HashBaseCounter (Stack* stk, StackERROR* error = nullptr) {
    if (stk == nullptr) {
        if (error) *error = NULL_STACK;
        return 0;
    }
    unsigned long long hash = hash_stack_const;
    hash = hash * 33 + ((unsigned long long) stk->data);
    hash = hash * 33 + stk->capacity;
    hash = hash * 33 + stk->size;
    if (hash != stk->hash_stack) if (error) *error = HASH_STACK_DAMAGED;
    return hash;
}

unsigned long long HashDataCounter (Stack* stk, StackERROR* error = nullptr) {
    if (stk == nullptr) {
        if (error) *error = NULL_STACK;
        return 0;
    }
    unsigned long long hash = hash_data_const;
    if (stk->data == nullptr) {
        if (stk->hash_data != hash_data_const)
            if (error) *error = HASH_DATA_DAMAGED;
        return hash;
    }
    for (int i = 0; i < stk->size; i++)
        hash = hash * 37 + stk->data[i];
    if (hash != stk->hash_data) if (error) *error = HASH_DATA_DAMAGED;
    return hash;
}

StackERROR StackCtor (Stack* stk) {
    if (stk != nullptr) return CREATED_STACK;
    stk = (Stack *) malloc(sizeof(Stack));
    stk->capacity = 1;
    stk->size = 0;
    stk->canary_l = canary_l_const;
    stk->canary_r = canary_r_const;
    stk->hash_stack = hash_stack_const;
    stk->hash_data = hash_data_const;
    auto* alpha = (canary_t *)malloc(sizeof(canary_t) * 2 + sizeof(data_t));
    if (alpha == nullptr) return NO_MEMORY;
    alpha[0] = canary_l_const;
    *(canary_t *)((char *)alpha + sizeof(canary_t) +
                   sizeof(data_t) * stk->capacity) = canary_r_const;
    stk->data = (data_t *)(alpha + 1);
    memset(stk->data, poison, sizeof(data_t) * stk->capacity);
    return OK_STACK;
}

StackERROR StackRealloc(Stack* stk) {
    auto* alpha = (canary_t *)stk->data - 1;
    alpha = (canary_t *) realloc(alpha, sizeof(canary_t) * 2 +
                                        sizeof(data_t) * stk->capacity);
    if (alpha == nullptr) return NO_MEMORY;
    memset((char *)alpha + sizeof(canary_t) + sizeof(data_t) * stk->size,
           poison, sizeof(data_t) * (stk->capacity - stk->size));
    *(canary_t *)((char *)alpha + sizeof(canary_t) +
                  sizeof(data_t) * stk->capacity) = canary_r_const;
    stk->data = (data_t *)(alpha[1]);
}

StackERROR StackPush (Stack* stk, data_t value) {
    StackCheck(stk);
    if (stk->size == stk->capacity) {
        stk->capacity *= 2;
        StackERROR error = StackRealloc(stk);
        if (error) return error;
    }
    stk->data[stk->size++] = value;
    StackCheck(stk);
    return OK_STACK;
}

StackERROR StackPop (Stack* stk) {
    StackCheck(stk);
    if (stk->size == 0) return EMPTY_STACK;
    if (stk->size == stk->capacity / 4) {
        stk->capacity /= 2;
        StackERROR error = StackRealloc(stk);
        if (error) return error;
    }
    stk->size--;
    memset((char *)stk->data + sizeof(data_t) * stk->size, poison, sizeof(data_t));
    StackCheck(stk);
    return OK_STACK;
}

int main() {
    return 0;
}
