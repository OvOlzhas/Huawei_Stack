#include <Stack.h>

int main() {
    Stack stk = {};
    ErrorCheck(StackCtor(&stk));
    ErrorCheck(StackPush(&stk, 5));
    ErrorCheck(StackPush(&stk, 4));
    ErrorCheck(StackPush(&stk, 3));
    ErrorCheck(StackPush(&stk, 2));
    ErrorCheck(StackPush(&stk, 1));
    data_t top = 0;
    ErrorCheck(StackPop(&stk, &top));
    printf("%d", top);
    ErrorCheck(StackPop(&stk, &top));
    printf("%d", top);
    ErrorCheck(StackPop(&stk, &top));
    printf("%d", top);
    ErrorCheck(StackPop(&stk, &top));
    printf("%d", top);
    ErrorCheck(StackPop(&stk, &top));
    printf("%d", top);
    ErrorCheck(StackDtor(&stk));
    return 0;
}
