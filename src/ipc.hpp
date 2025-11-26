#pragma once

#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ipc {

using Fd = int;

// created pair of connected local sockets (AF_UNIX).
inline void make_socket_pair(Fd sv[2]) {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        throw std::runtime_error(
            std::string("socketpair failed: ") + std::strerror(errno));
    }
}

// Відправити рядок (з \n в кінці) у дескриптор.
inline bool send_message(Fd fd, const std::string& msg) {
    std::string data = msg;
    data.push_back('\n');

    ssize_t total = 0;
    const ssize_t len = static_cast<ssize_t>(data.size());

    while (total < len) {
        ssize_t n = ::write(fd, data.data() + total, len - total);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return false;
        }
        total += n;
    }
    return true;
}

// Прочитати один рядок до \n (спрощений протокол).
inline bool recv_line(Fd fd, std::string& out) {
    out.clear();
    char ch;
    while (true) {
        ssize_t n = ::read(fd, &ch, 1);
        if (n == 0) {
            // EOF
            return !out.empty();
        }
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (ch == '\n') break;
        out.push_back(ch);
    }
    return true;
}

}