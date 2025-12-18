// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/config.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

namespace shell::test {
/*
class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        temp_dir_ = std::filesystem::temp_directory_path() / "wshell_test";
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override {
        // Clean up temporary directory
        std::error_code ec;
        std::filesystem::remove_all(temp_dir_, ec);
    }

    std::filesystem::path create_test_file(std::string const& content) {
        static int counter = 0;
        auto path = temp_dir_ / ("test_config_" + std::to_string(counter++) + ".rc");
        std::ofstream file(path);
        file << content;
        file.close();
        return path;
    }

    std::filesystem::path temp_dir_;
};

TEST_F(ConfigTest, ParseEmptyConfig) {
    auto result = DefaultConfig::parse("");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->variables().empty());
}

*/

} // namespace shell::test
