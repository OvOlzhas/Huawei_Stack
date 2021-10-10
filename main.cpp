#include <test.h>

#define Tests

int main() {
#ifdef Tests
    MainTest();
    TestWithTorture();
    SniperTest();
    TestWithoutErrors();
#endif
    return 0;
}
