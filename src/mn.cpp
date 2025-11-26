#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.hpp"
#include "ipc.hpp"
#include "task.hpp"

struct Worker {
    pid_t   pid   { -1 };
    ipc::Fd fd    { -1 };
};

// creating socket pair, forking the process and launching ./prog
Worker start_worker(const char* prog_path) {
    int sv[2];
    ipc::make_socket_pair(sv);   // sv[0] — in Mn, sv[1] — in child

    pid_t pid = ::fork();
    if (pid < 0) {
        throw std::runtime_error("fork failed");
    }

    if (pid == 0) {
        // child
        ::close(sv[0]); // we don't need Mn side

        // flip sv[1] on fd=3 in purpose fn1/fn2 knows, where read
        if (::dup2(sv[1], 3) == -1) {
            ::perror("dup2");
            _exit(1);
        }
        ::close(sv[1]);

        execl(prog_path, prog_path, (char*)nullptr);
        ::perror("execl");
        _exit(1);
    }

    // parent (Mn)
    ::close(sv[1]); // child process side doesn't need
    Worker w;
    w.pid = pid;
    w.fd  = sv[0];
    return w;
}

// synchronical call fn(x) via socket, return FnResult
FnResult call_over_socket(ipc::Fd fd, int x) {
    std::string req = "REQ " + std::to_string(x);
    if (!ipc::send_message(fd, req)) {
        FnResult r;
        r.status = FnStatus::Fail;
        r.value  = "send_error";
        return r;
    }

    std::string reply;
    if (!ipc::recv_line(fd, reply)) {
        FnResult r;
        r.status = FnStatus::Fail;
        r.value  = "recv_error";
        return r;
    }

    std::istringstream iss(reply);
    std::string tag;
    iss >> tag;

    FnResult r;
    if (tag == "OK") {
        r.status = FnStatus::Ok;
        std::string val;
        std::getline(iss, val);
        if (!val.empty() && val[0] == ' ') val.erase(0, 1);
        r.value = val;
    } else if (tag == "FAIL") {
        r.status = FnStatus::Fail;
        std::string msg;
        std::getline(iss, msg);
        if (!msg.empty() && msg[0] == ' ') msg.erase(0, 1);
        r.value = msg;
    } else {
        r.status = FnStatus::Undefined;
        r.value  = reply;
    }
    return r;
}

int main() {
    std::cout << "Mn: starting workers fn1/fn2 via local sockets...\n";

    Worker w1 = start_worker("./fn1");
    Worker w2 = start_worker("./fn2");

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

        // assynchronucally launching two tasks, each blocks on own socket
        auto t1 = Task<FnResult>::start([&]() { return call_over_socket(w1.fd, x); });
        auto t2 = Task<FnResult>::start([&]() { return call_over_socket(w2.fd, x); });

        FnResult r1 = t1.get();
        FnResult r2 = t2.get();

        // ⊗ = concatenction
        std::string combined = r1.value + " ⊗ " + r2.value;

        std::cout << "Mn: result = " << combined << "\n";
    }

    // closing sockets and waiting childs
    ::close(w1.fd);
    ::close(w2.fd);

    int status = 0;
    ::waitpid(w1.pid, &status, 0);
    ::waitpid(w2.pid, &status, 0);

    std::cout << "Mn: bye.\n";
    return 0;
}