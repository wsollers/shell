#!/bin/bash
set -euo pipefail

echo "🧪 Testing Development Container Environment"
echo "=========================================="

# Track test results
TESTS_PASSED=0
TESTS_FAILED=0

test_command() {
    local name="$1"
    local command="$2"
    local expected_pattern="$3"
    
    echo -n "Testing $name... "
    if output=$(eval "$command" 2>&1) && echo "$output" | grep -q "$expected_pattern"; then
        echo "✅ PASS"
        ((TESTS_PASSED++))
        echo "  → $output"
    else
        echo "❌ FAIL"
        echo "  → Expected pattern: $expected_pattern"
        echo "  → Got: $output"
        ((TESTS_FAILED++))
    fi
    echo
}

test_file() {
    local name="$1"
    local file="$2"
    
    echo -n "Testing $name... "
    if [[ -f "$file" ]]; then
        echo "✅ PASS"
        ((TESTS_PASSED++))
    else
        echo "❌ FAIL - File not found: $file"
        ((TESTS_FAILED++))
    fi
    echo
}

# Test system information
echo "📋 System Information:"
echo "  OS: $(lsb_release -d | cut -f2)"
echo "  Kernel: $(uname -r)"
echo "  User: $(whoami)"
echo "  Working Directory: $(pwd)"
echo

# Test core tools
test_command "Clang C++ compiler" "clang++ --version" "clang version 18"
test_command "CMake build system" "cmake --version" "cmake version 3"
test_command "Python interpreter" "python3 --version" "Python 3.12"
test_command "Git version control" "git --version" "git version"
test_command "GDB debugger" "gdb --version" "GNU gdb"

# Test C++ standard library
echo -n "Testing C++23 support... "
if cat > /tmp/test_cpp23.cpp << 'EOF'
#include <expected>
#include <iostream>

std::expected<int, std::string> test_function() {
    return 42;
}

int main() {
    auto result = test_function();
    if (result) {
        std::cout << "C++23 std::expected works: " << *result << std::endl;
        return 0;
    }
    return 1;
}
EOF
then
    if clang++ -std=c++23 -stdlib=libc++ /tmp/test_cpp23.cpp -o /tmp/test_cpp23 2>/dev/null && /tmp/test_cpp23; then
        echo "✅ PASS"
        ((TESTS_PASSED++))
    else
        echo "❌ FAIL - C++23 compilation or execution failed"
        ((TESTS_FAILED++))
    fi
    rm -f /tmp/test_cpp23.cpp /tmp/test_cpp23
else
    echo "❌ FAIL - Could not create test file"
    ((TESTS_FAILED++))
fi
echo

# Test Python SBOM dependencies
echo "🐍 Testing Python Dependencies:"
python3 -c "
import sys
packages = [
    ('reuse', 'REUSE license compliance'),
    ('spdx_tools', 'SPDX SBOM tools'),
    ('ntia_conformance_checker', 'NTIA conformance checker')
]

passed = 0
total = len(packages)

for pkg, desc in packages:
    try:
        __import__(pkg)
        print(f'  ✅ {pkg} ({desc})')
        passed += 1
    except ImportError:
        print(f'  ❌ {pkg} ({desc}) - NOT FOUND')

print(f'\\nPython packages: {passed}/{total} available')
"

# Test CMake presets
echo -n "Testing CMake presets... "
if cmake --list-presets 2>/dev/null | grep -q "linux-debug"; then
    echo "✅ PASS - CMake presets available"
    ((TESTS_PASSED++))
else
    echo "❌ FAIL - CMake presets not found"
    ((TESTS_FAILED++))
fi
echo

# Test project build (if we're in the workspace)
if [[ -f "CMakeLists.txt" ]]; then
    echo -n "Testing project configuration... "
    if cmake --preset linux-debug 2>/dev/null; then
        echo "✅ PASS - Project configures successfully"
        ((TESTS_PASSED++))
        
        echo -n "Testing project build... "
        if cmake --build build/linux-debug --target wshell_lib 2>/dev/null; then
            echo "✅ PASS - Project builds successfully"
            ((TESTS_PASSED++))
        else
            echo "❌ FAIL - Project build failed"
            ((TESTS_FAILED++))
        fi
    else
        echo "❌ FAIL - Project configuration failed"
        ((TESTS_FAILED++))
    fi
    echo
else
    echo "ℹ️  Skipping project build tests (not in workspace)"
    echo
fi

# Summary
echo "=========================================="
echo "🏁 Test Summary:"
echo "  ✅ Passed: $TESTS_PASSED"
echo "  ❌ Failed: $TESTS_FAILED"
echo "  📊 Total:  $((TESTS_PASSED + TESTS_FAILED))"
echo

if [[ $TESTS_FAILED -eq 0 ]]; then
    echo "🎉 All tests passed! Development environment is ready."
    exit 0
else
    echo "💥 Some tests failed. Please check the configuration."
    exit 1
fi