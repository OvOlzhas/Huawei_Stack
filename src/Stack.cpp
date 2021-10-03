#include <Stack.h>

#define StackCheck(stk) {                                                           \
    if (stk == nullptr) assert(!"Null stack");                                      \
    if (stk->size > stk->capacity) assert(!"size > capacity");                      \
    CanaryCheck(stk);                                                               \
    HashCheck(stk);                                                                 \
}

void CanaryCheck (Stack* stk) {
#ifdef WithCanary
    if (stk->canary_l != canary_l_const) assert(!"Left canary of stack damaged");
    if (stk->canary_r != canary_r_const) assert(!"Right canary of stack damaged");
    if (((canary_t *)stk->data)[-1] != canary_l_const)
        assert(!"Left canary of data damaged");
    if (*((canary_t *)(stk->data + stk->capacity)) != canary_r_const)
        assert(!"Right canary of data damaged");
#endif
}

void HashCheck (Stack* stk) {
#ifdef WithHash
    StackERROR error = OK_STACK;
    HashBaseCounter(stk, &error);
    if (error) assert(!"HashBase damaged");
    HashDataCounter(stk, &error);
    if (error) assert(!"HashData damaged");
#endif
}

#ifdef WithHash
unsigned long long HashBaseCounter (Stack* stk, StackERROR* error) {
    if (stk == nullptr) {
        if (error) *error = NULL_STACK;
        return 0;
    }
    unsigned long long hash = hash_base_const;
    hash = hash * 33 + ((unsigned long long) stk->data);
    hash = hash * 33 + stk->capacity;
    hash = hash * 33 + stk->size;
    if (hash != stk->hash_base) if (error) *error = HASH_BASE_DAMAGED;
    return hash;
}

unsigned long long HashDataCounter (Stack* stk, StackERROR* error) {
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

void UpdateHash (Stack* stk) {
    stk->hash_base = HashBaseCounter(stk);
    stk->hash_data = HashDataCounter(stk);
}
#endif


StackERROR StackRealloc(Stack* stk) {
#ifdef WithCanary
    canary_t* ptr = (canary_t *)stk->data - 1;
    ptr = (canary_t *) realloc(ptr, sizeof(canary_t) * 2 +
                                    sizeof(data_t) * stk->capacity);
    if (ptr == nullptr) return NO_MEMORY;
    memset((char *)ptr + sizeof(canary_t) + sizeof(data_t) * stk->size,
           poison, sizeof(data_t) * (stk->capacity - stk->size));
    *(canary_t *)((char *)ptr + sizeof(canary_t) +
                  sizeof(data_t) * stk->capacity) = canary_r_const;
    stk->data = (data_t *)(ptr + 1);
#else
    stk->data = (data_t *) realloc(stk->data, sizeof(data_t) * stk->capacity);
    if (stk->data == nullptr) return NO_MEMORY;
    memset((char *)stk->data + sizeof(data_t) * stk->size,
           poison, sizeof(data_t) * (stk->capacity - stk->size));
#endif
    return OK_STACK;
}

StackERROR StackCtor (Stack* stk) {
    if (stk->data != nullptr || stk->size != 0 || stk->capacity != 0)
        return CREATED_STACK;
    stk->capacity = 4;
    stk->size = 0;
#ifdef WithCanary
    stk->canary_l = canary_l_const;
    stk->canary_r = canary_r_const;
    auto* ptr = (canary_t *)calloc(sizeof(canary_t) * 2 +
                                   sizeof(data_t) * stk->capacity, sizeof(char));
    if (ptr == nullptr) return NO_MEMORY;
    ptr[0] = canary_l_const;
    *((canary_t *)((char *)ptr + sizeof(canary_t) +
                   sizeof(data_t) * stk->capacity)) = canary_r_const;
    stk->data = (data_t *)(ptr + 1);
#else
    auto* ptr = (canary_t *)calloc(sizeof(data_t) * stk->capacity, sizeof(char));
    stk->data = (data_t *)ptr;
#endif

    memset(stk->data, poison, sizeof(data_t) * stk->capacity);

#ifdef WithHash
    UpdateHash(stk);
#endif
    return OK_STACK;
}

StackERROR StackPush (Stack* stk, data_t value) {
    StackCheck(stk);
    if (stk->size == stk->capacity) {
        stk->capacity *= 2;
        StackERROR error = StackRealloc(stk);
        if (error) return error;
    }
    stk->data[stk->size++] = value;
#ifdef WithHash
    UpdateHash(stk);
#endif
    StackCheck(stk);
    return OK_STACK;
}

StackERROR StackPop (Stack* stk, data_t* pop_elem) {
    StackCheck(stk);
    if (stk->size == 0) return EMPTY_STACK;
    if (stk->size == stk->capacity / 4 && stk->capacity != 4) {
        stk->capacity /= 2;
        StackERROR error = StackRealloc(stk);
        if (error) return error;
    }
    stk->size--;
    if (pop_elem) *pop_elem = stk->data[stk->size];
    memset((char *)stk->data + sizeof(data_t) * stk->size, poison, sizeof(data_t));
#ifdef WithHash
    UpdateHash(stk);
#endif
    StackCheck(stk);
    return OK_STACK;
}

StackERROR StackDtor (Stack* stk) {
    StackCheck(stk);
#ifdef WithCanary
    canary_t* ptr = (canary_t *)stk->data - 1;
    memset(ptr, poison, sizeof(canary_t) * 2 + sizeof(data_t) * stk->capacity);
    free(ptr);
#else
    memset(stk->data, poison, sizeof(data_t) * stk->capacity);
    free(stk->data);
#endif
    stk->data = nullptr;
    stk->size = 0;
    stk->capacity = 0;
    return OK_STACK;
}
