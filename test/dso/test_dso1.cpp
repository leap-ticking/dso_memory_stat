#include <iostream>

void test_malloc1() {
    int *p1 = new int[2000];
    delete []p1;
    p1 = new int[1000];
//    std::cout << "new int 2000" << std::endl;
}