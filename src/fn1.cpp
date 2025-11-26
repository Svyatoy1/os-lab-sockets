#include <iostream>
#include <sstream>
#include <string>

#include "common.hpp"
#include "ipc.hpp"

// fd, через який цей процес спілкується з Mn
static constexpr ipc::Fd FN_FD = 3;

// приклад fn1(x): просто x + 1
static FnResult compute(int x) {
    FnResult r;
    r.status = FnStatus::Ok;
    r.value  = std::to_string(x + 1);
    return r;
}

int main(int /*argc*/, char* /*argv*/[]) {
    std::string line;

    while (ipc::recv_line(FN_FD, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd != "REQ") {
            // ігноруємо невідомі команди
            continue;
        }

        int x;
        if (!(iss >> x)) {
            ipc::send_message(FN_FD, "FAIL bad_input");
            continue;
        }

        FnResult r = compute(x);

        if (r.status == FnStatus::Ok) {
            ipc::send_message(FN_FD, "OK " + r.value);
        } else if (r.status == FnStatus::Fail) {
            ipc::send_message(FN_FD, "FAIL " + r.value);
        } else {
            ipc::send_message(FN_FD, "UNDEF");
        }
    }

    // EOF з боку Mn — виходимо
    return 0;
}