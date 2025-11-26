#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

#include "common.hpp"
#include "ipc.hpp"

static constexpr ipc::Fd FN_FD = 3;

// fn2(x): sqrt(|x|), if x == 0 we consider as undefined
static FnResult compute(double x) {
    FnResult r;
    if (x == 0.0) {
        r.status = FnStatus::Undefined;
        r.value  = "no_meaning_for_zero";
        return r;
    }
    r.status = FnStatus::Ok;
    r.value  = std::to_string(std::sqrt(std::fabs(x)));
    return r;
}

int main(int /*argc*/, char* /*argv*/[]) {
    std::string line;

    while (ipc::recv_line(FN_FD, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd != "REQ") {
            continue;
        }

        double x;
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
            ipc::send_message(FN_FD, "UNDEF " + r.value);
        }
    }

    return 0;
}