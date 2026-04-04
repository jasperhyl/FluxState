#pragma once
#include <string>

namespace flux {

inline std::string BuildError(const std::string& msg, int line, int column) {
    return msg + " at " + std::to_string(line) + ":" + std::to_string(column);
}

} // namespace flux