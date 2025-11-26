#include <iostream>
#include <string>
#include "common.hpp"
#include "task.hpp"

// TODO: later we change on real call-up via local socket to fn1/fn2.
FnResult fn1_sync(int x) {
    FnResult r;
    r.status = FnStatus::Ok;
    r.value  = "fn1(" + std::to_string(x) + ")";
    return r;
}

FnResult fn2_sync(int x) {
    FnResult r;
    r.status = FnStatus::Ok;
    r.value  = "fn2(" + std::to_string(x) + ")";
    return r;
}

int main() {
    std::cout << "Mn: skeleton (later: local sockets + processes)\n";

    while (true) {
        std::cout << "Enter x (or q to quit): ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line == "q" || line == "quit") break;

        int x = 0;
        try {
            x = std::stoi(line);
        } catch (...) {
            std::cerr << "Invalid integer, try again.\n";
            continue;
        }

        // parallel launching two tasks
        auto t1 = Task<FnResult>::start([x] { return fn1_sync(x); });
        auto t2 = Task<FnResult>::start([x] { return fn2_sync(x); });

        FnResult r1 = t1.get();
        FnResult r2 = t2.get();

        // now ⊗ = concatenation
        std::string combined = r1.value + " ⊗ " + r2.value;

        std::cout << "Mn: result = " << combined << "\n";
    }

    std::cout << "Mn: bye.\n";
    return 0;
}