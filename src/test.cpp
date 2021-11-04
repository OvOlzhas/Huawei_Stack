#include <test.h>

#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

static void IntPrint(int val) { printf("%d", val); }

static int IntPoison() { return 0xF0F0F0F0; }

void TestWithTorture () {
    printf(CYAN "/////////////////////////////////////TestWithTorture//////////////////////////////////////////" RESET);
    TortureChamber tortureChamber = {};
    StackVerifier(tortureChamber.victim,
                  StackCtor(&tortureChamber.victim, IntPrint, IntPoison));
    StackVerifier(tortureChamber.victim,
                  StackPush(&tortureChamber.victim, 1));
    StackVerifier(tortureChamber.victim,
                  StackPush(&tortureChamber.victim, 2));
    for (int i = -24; i <= -1; i++) tortureChamber.executionerRight[i] = 0xDD;
    StackDump(tortureChamber.victim, OK_STACK);
    for (int i = 2; i <= 9; i++) tortureChamber.executionerLeft[i] = 0xDD;
    StackDump(tortureChamber.victim, OK_STACK);
}

void MainTest () {
    printf(CYAN "/////////////////////////////////////////MainTest/////////////////////////////////////////////" RESET);
    Stack stk = {};
    StackVerifier(stk, StackCtor(&stk, IntPrint, IntPoison));
    StackVerifier(stk, StackCtor(&stk, IntPrint, IntPoison));
    StackVerifier(stk, StackPush(&stk, 1));
    StackVerifier(stk, StackPush(&stk, 2));
    StackVerifier(stk, StackPush(&stk, 3));
    StackVerifier(stk, StackPush(&stk, 4));
    StackVerifier(stk, StackPush(&stk, 5));
    StackDump(stk, OK_STACK);
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackPop(&stk));
    StackVerifier(stk, StackDtor(&stk));
    StackVerifier(stk, StackDtor(&stk));
}

void SniperTest () {
    printf(CYAN "////////////////////////////////////////SniperTest////////////////////////////////////////////" RESET);
    Stack stk = {};
    StackVerifier(stk, StackCtor(&stk, IntPrint, IntPoison));
    StackVerifier(stk, StackPush(&stk, 1));
    StackVerifier(stk, StackPush(&stk, 2));
    StackVerifier(stk, StackPush(&stk, 3));
    char* aim = (char *) &stk.data;
    *aim = 0xF0;
    StackDump(stk, OK_STACK);
    StackVerifier(stk, StackDtor(&stk));
}

void TestWithoutErrors () {
    printf(CYAN "////////////////////////////////////TestWithoutErrors///////////////////////////////////////\n" RESET);
    Stack stk = {};
    StackVerifier(stk, StackCtor(&stk, IntPrint, IntPoison));
    StackVerifier(stk, StackPush(&stk, 1));
    StackVerifier(stk, StackPush(&stk, 2));
    StackVerifier(stk, StackPush(&stk, 3));
    data_t value = 0;
    StackVerifier(stk, StackPop(&stk, &value));
    printf("%d", value);
    StackVerifier(stk, StackPop(&stk, &value));
    printf("%d", value);
    StackVerifier(stk, StackPop(&stk, &value));
    printf("%d", value);
    StackVerifier(stk, StackPush(&stk, 4));
    StackVerifier(stk, StackTop(&stk, &value));
    printf("%d", value);
    StackVerifier(stk, StackDtor(&stk));
}
