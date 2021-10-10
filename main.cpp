#include <test.h>

#define Tests

int main() {
#ifdef Tests
    MainTest();
    TestWithTorture();
    SniperTest();
#endif
    return 0;
}
