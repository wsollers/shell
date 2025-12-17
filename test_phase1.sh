#!/bin/bash
# Copyright (c) 2024 William Sollers
# SPDX-License-Identifier: BSD-2-Clause

# Test Phase 1 shell features

echo "=== Phase 1 Shell Test ==="
echo ""

echo "Test 1: Comment handling"
echo "# This is a comment" | ./build/linux-release/src/main/wshell
echo ""

echo "Test 2: Variable assignment"
echo -e "let x = hello\nexit" | ./build/linux-release/src/main/wshell
echo ""

echo "Test 3: Command execution"
echo -e "echo Phase 1 works!\nexit" | ./build/linux-release/src/main/wshell
echo ""

echo "Test 4: Multiple commands"
echo -e "# Comment first\nlet name = wshell\necho Testing\nls /tmp > /dev/null\nexit" | ./build/linux-release/src/main/wshell
echo ""

echo "Test 5: Invalid command"
echo -e "nonexistent_command_12345\nexit" | ./build/linux-release/src/main/wshell 2>&1 | grep -i "error"
echo ""

echo "=== All Phase 1 tests complete ==="
