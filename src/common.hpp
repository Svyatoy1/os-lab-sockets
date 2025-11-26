#pragma once

#include <string>
#include <optional>
#include <sstream>

enum class FnStatus {
    Ok,
    Fail,
    Undefined
};

struct FnResult {
    FnStatus   status {FnStatus::Undefined};
    std::string value;   // value or error message
};

// trying to get double from value
inline std::optional<double> as_double(const FnResult& r) {
    if (r.status != FnStatus::Ok) return std::nullopt;
    std::istringstream iss(r.value);
    double v;
    if (!(iss >> v)) return std::nullopt;
    return v;
}

inline const char* status_to_string(FnStatus s) {
    switch (s) {
        case FnStatus::Ok:        return "OK";
        case FnStatus::Fail:      return "FAIL";
        case FnStatus::Undefined: return "UNDEF";
    }
    return "???";
}