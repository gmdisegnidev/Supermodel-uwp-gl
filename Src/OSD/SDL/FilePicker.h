#pragma once

#include <string>
#include <vector>

namespace UWP {
    // Returns command-line-style arguments supplied by Windows activation.
    // File activation returns the selected ROM path. Protocol activation accepts
    // supermodel:?rom=<url-encoded-path> and supermodel:?cmd=<url-encoded-args>.
    std::vector<std::string> activation_arguments();

    // Interactive fallback used when the app was launched without a ROM.
    std::string pick_a_file();
}
