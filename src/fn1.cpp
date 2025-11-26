#include <iostream>
#include "common.hpp"
#include "ipc.hpp"

int main(int argc, char* argv[]) {
    std::cout << "fn1 process stub (later: read x from socket, send fn1(x))\n";
    // TODO: get socket's FD from argv or in other way
    return 0;
}