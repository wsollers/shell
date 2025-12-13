// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../lib/shell_core.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "wshell version " << wshell::ShellCore::version() << "\n";
    
    if (argc > 1) {
        std::string command;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) command += " ";
            command += argv[i];
        }
        
        wshell::ShellCore shell;
        auto result = shell.execute(command);
        
        if (result.has_value()) {
            return result.value();
        } else {
            std::cerr << "Error executing command\n";
            return 1;
        }
    } else {
        std::cout << "Interactive mode not yet implemented\n";
        std::cout << "Usage: wshell <command>\n";
        return 0;
    }
}
