//
// Created by wsollers on 12/24/25.
//

#ifndef WSHELL_SHELL_PROCESS_CONTEXT_H
#define WSHELL_SHELL_PROCESS_CONTEXT_H

struct ShellProcessContext {
    int argc;
    char** argv;



};

ShellProcessContext & shell_process_context();


#endif  // WSHELL_SHELL_PROCESS_CONTEXT_H
