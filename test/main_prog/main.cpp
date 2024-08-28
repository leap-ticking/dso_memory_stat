#include <iostream>

extern void test_malloc1();
extern void test_malloc2();

int main() {
    test_malloc1();
    test_malloc2();

    while (1) {
        ;
    }

    return 0;
}
