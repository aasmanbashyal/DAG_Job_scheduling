/**
 * @file Utility.hpp
 * @brief Utility functions
 */

#pragma once

#include <string>
#include <iostream>

/**
 * @brief Prints a formatted step header for better logging readability
 * @param step_name Name of the current processing step
 * @param details Optional additional details about the step
 */
inline void print_step(const std::string& step_name, const std::string& details = "") {
    std::cout << "\n==================================================\n";
    std::cout << "STEP: " << step_name << "\n";
    if (!details.empty()) {
        std::cout << "Details: " << details << "\n";
    }
    std::cout << "==================================================\n";
}