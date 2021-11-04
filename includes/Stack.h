#include <assert.h>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
//! @file

typedef int data_t;
typedef unsigned long long canary_t;

static const canary_t CANARY_L_CONST = 0xFFAAFFBBFAFBFCFD;
static const canary_t CANARY_R_CONST = 0xAAFFBBAAFFCCABCD;
static const int HASH_BASE_CONST = 5381;
static const int HASH_DATA_CONST = 3571;

/// Unused memory is covered in POISON
static const char POISON = (char)0xF0;

/// Adding canary protection.
#define WITH_CANARY

/// Adding hash protection. (Caution! The stack will slow down.)
#define WITH_HASH

/// On error, terminates the program with a description of the error
static const char* ERROR_MESSAGE[] = {"OK",
                                      "Null stack",
                                      "Repeat call StackCtor",
                                      "Left canary of stack damaged",
                                      "Right canary of stack damaged",
                                      "Left canary of data damaged",
                                      "Right canary of data damaged",
                                      "HashBase or Base damaged",
                                      "HashData or Data damaged",
                                      "No memory",
                                      "Trying to pop from empty stack",
                                      "Size more Capacity"};

/// Checking for errors after executing the function
#define StackVerifier(stk, FUNC) {                          \
    int error = FUNC;                                       \
    if (error) StackDump(stk, error);                       \
}

/// Stops the program when an error occurs after executing the function
#define StackVerifierKiller(stk, FUNC) {                    \
    int error = FUNC;                                       \
    if (error) {                                            \
        StackDump(stk, error);                              \
        assert(0);                                          \
    }                                                       \
}

enum StackERROR {
    OK_STACK = 0,
    NULL_STACK, // manually numerate
    CREATED_STACK,
    LEFT_CANARY_STACK_DAMAGED,
    RIGHT_CANARY_STACK_DAMAGED,
    LEFT_CANARY_DATA_DAMAGED,
    RIGHT_CANARY_DATA_DAMAGED,
    HASH_BASE_DAMAGED,
    HASH_DATA_DAMAGED,
    NO_MEMORY,
    EMPTY_STACK,
    SIZE_MORE_CAPACITY,
    SIZE_OF_ENUM
};

struct Stack {
#ifdef WITH_CANARY
    canary_t canary_l;
#endif
    data_t* data;
    size_t capacity;
    size_t size;
    void (*print_func)(data_t);
    data_t (*poison_func)();
#ifdef WITH_CANARY
    canary_t canary_r;
#endif
#ifdef WITH_HASH
    unsigned long long hash_base;
    unsigned long long hash_data;
#endif
};

struct StackInfo {
    const char* name;
    const char* func;
    const char* file;
    size_t line;
    const char* type;
    int error;
};

/// Variable Type
#define VariableType(var) abi::__cxa_demangle(typeid(var).name(), 0, 0, nullptr)

/// Displaying all necessary stack data, taking into account the "error"
#define StackDump(stk, error) {                                                                     \
    StackInfo info = {#stk, __PRETTY_FUNCTION__, __FILE__, __LINE__, VariableType(data_t), error};  \
    StackDump_ (&stk, info);                                                                        \
}

/*!
 * Displaying all necessary stack data, taking into account the "info"
 * @param [in] stk Pointer to stack
 * @param [in] info Information about where and what broke
 */
void StackDump_ (Stack* stk, StackInfo info);

/*!
 * Checking the stack for damage
 * @param stk
 * @return Error code(0 if there are no errors)
 */
int StackCheck_ (Stack* stk);

/*!
 * Canary damage check
 * @param [in] stk Pointer to stack
 * @return Error code(0 if there are no errors)
 */
int StackCanaryCheck (Stack* stk);

/*!
 * Hash damage check
 * @param [in] stk Pointer to stack
 * @return Error code(0 if there are no errors)
 */
int StackHashCheck (Stack* stk);

/*!
 * Counting the hash of a stack without an array
 * @param [in] stk Pointer to stack
 * @param [out] error Error code(0 if there are no errors)
 * @return Calculated Hash
 */
unsigned long long StackHashBaseCounter (Stack* stk, int* error = nullptr);

/*!
 * Counting the hash of a stack array
 * @param [in] stk Pointer to stack
 * @param [out] error Error code(0 if there are no errors)
 * @return Calculated Hash
 */
unsigned long long StackHashDataCounter (Stack* stk, int* error = nullptr);

/*!
 * Update hash after stack change
 * @param [in] stk Pointer to stack
 */
void StackUpdateHash (Stack* stk);

/*!
 * Update array after capacity change
 * @param [in] stk Pointer to stack
 * @return Error code(0 if there are no errors)
 */
int StackRealloc(Stack* stk);

/*!
 * Stack constructor
 * @param [in] stk Pointer to stack
 * @return Error code(0 if there are no errors)
 */
int StackCtor (Stack* stk, void (*print_func)(data_t) = nullptr, data_t (*poison_func)() = nullptr);

/*!
 * Push value to the stack
 * @param [in] stk Pointer to stack
 * @param [in] value Value pushing to stack
 * @return Error code(0 if there are no errors)
 */
int StackPush (Stack* stk, data_t value);

/*!
 * Pop a value from the stack
 * @param [in] stk Pointer to stack
 * @param [out] pop_elem Value popped from the stack
 * @return Error code(0 if there are no errors)
 */
int StackPop (Stack* stk, data_t* pop_elem = nullptr);

/*!
 * Return value at the top of the stack
 * @param [in] stk Pointer to stack
 * @param [out] value Value at the top of the stack
 * @return Error code(0 if there are no errors)
 */
int StackTop (Stack* stk, data_t* value);

/*!
 * Stack destructor
 * @param [in] stk Pointer to stack
 * @return Error code(0 if there are no errors)
 */
int StackDtor (Stack* stk);

/*!
 * Stack Size
 * @param [in] stk Pointer to stack
 * @return Size of stack or -1 if stack is damaged
 */
size_t StackSize (Stack* stk);
