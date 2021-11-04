#include <Stack.h>

#define YELLOW  "\x1b[33m" // colors.h
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

#define StackCheck(stk) {           \
    int error = StackCheck_(stk);   \
    if (error) return error;        \
}

static int NumberOfBits (int error) {
    int num = 0;
    for (int i = 0; i < SIZE_OF_ENUM; i++)
        num += ((error >> i) & 1);
    return num;
}

void StackDump_ (Stack* stk, StackInfo info) {
    printf("\n");

    if (stk == nullptr) {
        printf("DUMP:" RED "Pointer of stack is nullptr\n" RESET);
        return;
    }

    info.error |= StackCheck_(stk);
    printf("Stack<" YELLOW "%s" RESET ">[" BLUE "%p" RESET "]",
           info.type, stk);
    if (info.error == OK_STACK)
        printf(GREEN " OK " RESET);
    else
        printf(RED " ERROR(%d) " RESET, NumberOfBits(info.error));
    printf(YELLOW "'%s'" RESET " at " YELLOW "%s" RESET " at " YELLOW "%s" RESET
           "(" BLUE "%zu" RESET ")\n", info.name, info.func, info.file, info.line);

    printf(RED);
    if (info.error) {

        for (int i = 0; i < SIZE_OF_ENUM; i++) {
            if ((info.error >> i) & 1) {
                printf("%s\n", ERROR_MESSAGE[i]);
            }
        }

    }
    printf(RESET);

    printf("{\n");
    printf(YELLOW "\tsize" RESET " = %zu",
           stk->size);
    printf(YELLOW "\tcapacity" RESET " = %zu\n",
           stk->capacity);

    printf(YELLOW "\tdata" RESET);
    if (stk->data == nullptr) { // code style
        if (stk->capacity != 0)
            printf("[" RED "NULL" RESET "]\n");
        else printf("[" BLUE "NULL" RESET "]\n");
    }else {
        printf("[" BLUE "%p" RESET "]\n", stk->data);
        printf("\t{\n");
        for (size_t i = 0; i < stk->capacity; i++) {
            if (i < stk->size) {
                printf("\t\t* [" GREEN "%zu" RESET "] = ", i);
                if (stk->poison_func && stk->data[i] == stk->poison_func()) {
                    printf(YELLOW "POISON" RESET " ");
                }
                if (stk->print_func)
                    stk->print_func(stk->data[i]);
                else printf(RED "---" RESET);
                printf("\n");
            }else {
                printf("\t\t  [" RED "%zu" RESET "] = ", i);
                if (stk->poison_func && stk->data[i] == stk->poison_func()) {
                    printf(YELLOW "POISON" RESET " (");
                }else  if (stk->poison_func) {
                    printf(RED "POISON DAMAGED" RESET " (");
                }else printf(RED "---" RESET " (");
                if (stk->print_func)
                    stk->print_func(stk->data[i]);
                else printf(RED "---" RESET);
                printf(")\n");
            }
        }
        printf("\t}\n");
    }

    printf("}\n");
}

int StackCanaryCheck (Stack* stk) {
#ifdef WITH_CANARY
    int all_errors = OK_STACK;
    if (stk->canary_l != CANARY_L_CONST) all_errors |= 1 << LEFT_CANARY_STACK_DAMAGED;
    if (stk->canary_r != CANARY_R_CONST) all_errors |= 1 << RIGHT_CANARY_STACK_DAMAGED;
    if (((canary_t *)stk->data)[-1] != CANARY_L_CONST)
        all_errors |= 1 << LEFT_CANARY_DATA_DAMAGED;
    if (*((canary_t *)(stk->data + stk->capacity)) != CANARY_R_CONST)
        all_errors |= 1 << RIGHT_CANARY_DATA_DAMAGED;
    return all_errors;
#endif
}

int StackHashCheck (Stack* stk) {
#ifdef WITH_HASH
    int all_errors = OK_STACK;
    StackHashBaseCounter(stk, &all_errors);
    StackHashDataCounter(stk, &all_errors);
    return all_errors;
#endif
}

int StackCheck_(Stack* stk) {
    if (stk == nullptr || (stk->data == nullptr && stk->size == 0 && stk->capacity == 0))
        return 1 << NULL_STACK;
    int all_errors = OK_STACK;
    if (stk->size > stk->capacity) all_errors |= 1 << SIZE_MORE_CAPACITY;
    all_errors |= StackCanaryCheck(stk);
    all_errors |= StackHashCheck(stk);
    return all_errors;
}

#ifdef WITH_HASH
unsigned long long StackHashBaseCounter (Stack* stk, int* error) {
    if (stk == nullptr) {
        if (error) *error |= 1 << NULL_STACK;
        return 0;
    }
    auto* ptr = (char *)stk;
    unsigned long long hash = HASH_BASE_CONST;
    for (size_t i = 0; i < sizeof(Stack) - 16; i++) {
        hash = hash * 17 + ptr[i];
    }
    if (hash != stk->hash_base) if (error) *error |= 1 << HASH_BASE_DAMAGED;
    return hash;
}

unsigned long long StackHashDataCounter (Stack* stk, int* error) {
    if (stk == nullptr) {
        if (error) *error |= 1 << NULL_STACK;
        return 0;
    }
    unsigned long long hash = HASH_DATA_CONST;
    if (stk->data == nullptr) {
        if (stk->hash_data != HASH_DATA_CONST)
            if (error) *error |= 1 << HASH_DATA_DAMAGED;
        return hash;
    }
    for (size_t i = 0; i < stk->size; i++)
        hash = hash * 37 + stk->data[i];
    if (hash != stk->hash_data) if (error) *error |= 1 << HASH_DATA_DAMAGED;
    return hash;
}

void StackUpdateHash (Stack* stk) {
    stk->hash_base = StackHashBaseCounter(stk);
    stk->hash_data = StackHashDataCounter(stk);
}
#endif

int StackRealloc(Stack* stk) {
#ifdef WITH_CANARY
    canary_t* ptr = (canary_t *)stk->data - 1;
    ptr = (canary_t *) realloc(ptr, sizeof(canary_t) * 2 +
                                    sizeof(data_t) * stk->capacity);
    if (ptr == nullptr) return 1 << NO_MEMORY;
    memset((char *)ptr + sizeof(canary_t) + sizeof(data_t) * stk->size,
           POISON, sizeof(data_t) * (stk->capacity - stk->size));
    *(canary_t *)((char *)ptr + sizeof(canary_t) + sizeof(data_t) * stk->capacity) = CANARY_R_CONST;
    stk->data = (data_t *)(ptr + 1);
#else
    stk->data = (data_t *) realloc(stk->data, sizeof(data_t) * stk->capacity);
    if (stk->data == nullptr) return 1 << NO_MEMORY;
    memset((char *)stk->data + sizeof(data_t) * stk->size,
           POISON, sizeof(data_t) * (stk->capacity - stk->size));
#endif
    return 0;
}

int StackCtor (Stack* stk, void (*print_func)(data_t), data_t (*poison_func)()) {
    if (stk->data != nullptr || stk->size != 0 || stk->capacity != 0)
        return 1 << CREATED_STACK;
    stk->capacity = 4;
    stk->size = 0;
#ifdef WITH_CANARY
    stk->canary_l = CANARY_L_CONST;
    stk->canary_r = CANARY_R_CONST;
    stk->print_func = print_func;
    stk->poison_func = poison_func;
    auto* ptr = (canary_t *)calloc(sizeof(canary_t) * 2 +
                                   sizeof(data_t) * stk->capacity, sizeof(char));
    if (ptr == nullptr) return 1 << NO_MEMORY;
    ptr[0] = CANARY_L_CONST;
    *(canary_t *)((char *)ptr + sizeof(canary_t) + sizeof(data_t) * stk->capacity) = CANARY_R_CONST;
    stk->data = (data_t *)(ptr + 1);
#else
    auto* ptr = (canary_t *)calloc(sizeof(data_t) * stk->capacity, sizeof(char));
    if (ptr == nullptr) return 1 << NO_MEMORY;
    stk->data = (data_t *)ptr;
#endif

    memset(stk->data, POISON, sizeof(data_t) * stk->capacity);

#ifdef WITH_HASH
    StackUpdateHash(stk);
#endif
    return OK_STACK;
}

int StackPush (Stack* stk, data_t value) {
    StackCheck(stk);
    if (stk->size == stk->capacity) {
        stk->capacity *= 2;
        int all_errors = StackRealloc(stk);
        if (all_errors) return all_errors;
    }
    stk->data[stk->size++] = value;
#ifdef WITH_HASH
    StackUpdateHash(stk);
#endif
    StackCheck(stk);
    return OK_STACK;
}

int StackPop (Stack* stk, data_t* pop_elem) {
    StackCheck(stk);
    if (stk->size == 0) return 1 << EMPTY_STACK;
    if (stk->size == stk->capacity / 4 && stk->capacity != 4) {
        stk->capacity /= 2;
        int all_errors = StackRealloc(stk);
        if (all_errors) return all_errors;
    }
    stk->size--;
    if (pop_elem) *pop_elem = stk->data[stk->size];
    memset((char *)stk->data + sizeof(data_t) * stk->size, POISON, sizeof(data_t));
#ifdef WITH_HASH
    StackUpdateHash(stk);
#endif
    StackCheck(stk);
    return OK_STACK;
}

int StackTop(Stack* stk, data_t* value) {
    StackCheck(stk);
    if (stk->size == 0) return 1 << EMPTY_STACK;
    *value = stk->data[stk->size - 1];
    return OK_STACK;
}

int StackDtor (Stack* stk) {
    StackCheck(stk);
#ifdef WITH_CANARY
    canary_t* ptr = (canary_t *)stk->data - 1;
    memset(ptr, POISON, sizeof(canary_t) * 2 + sizeof(data_t) * stk->capacity);
    free(ptr);
#else
    memset(stk->data, POISON, sizeof(data_t) * stk->capacity);
    free(stk->data);
#endif
    stk->data = nullptr;
    stk->size = 0;
    stk->capacity = 0;
    return OK_STACK;
}

size_t StackSize (Stack* stk) {
    if (StackCheck_(stk)) return -1ULL;
    return stk->size;
}
