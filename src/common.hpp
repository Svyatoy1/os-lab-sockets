// src/common.hpp
#pragma once

#include <string>

enum class FnStatus {
    Ok,
    Fail,
    Undefined
};

struct FnResult {
    FnStatus   status {FnStatus::Undefined};
    std::string value;   // значення або повідомлення про помилку
};