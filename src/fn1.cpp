#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

#include "common.hpp"
#include "ipc.hpp"

// via fd process connected with Mn
static constexpr ipc::Fd FN_FD = 3;

// fn1(x): x^2, if |x| > 1000 we consider as fail
static FnResult compute(double x) {
    FnResult r;
    if (std::fabs(x) > 1000.0) {
        r.status = FnStatus::Fail;
        r.value  = "x_out_of_range";
        return r;
    }
    r.status = FnStatus::Ok;
    r.value  = std::to_string(x * x);
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
            ipc::send_message(FN_FD, "UNDEF");
        }
    }

    return 0;
}