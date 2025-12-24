//
// Created by wsollers on 12/24/25.
//

#include "shell/shell_process_context.h"
static ShellProcessContext ctx;

ShellProcessContext & shell_process_context() {
    return ctx;
}

