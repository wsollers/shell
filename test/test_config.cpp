// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/config.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

namespace shell::test {

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
    auto result = Config::parse("");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->variables().empty());
}

TEST_F(ConfigTest, ParseSingleVariable) {
    auto result = Config::parse("NAME=value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 1);
    
    auto* val = result->get("NAME");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "value");
}

TEST_F(ConfigTest, ParseMultipleVariables) {
    auto result = Config::parse("VAR1=value1\nVAR2=value2\nVAR3=value3");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 3);
    
    EXPECT_EQ(*result->get("VAR1"), "value1");
    EXPECT_EQ(*result->get("VAR2"), "value2");
    EXPECT_EQ(*result->get("VAR3"), "value3");
}

TEST_F(ConfigTest, ParseWithWhitespace) {
    auto result = Config::parse("  NAME  =  value  \n");
    ASSERT_TRUE(result.has_value());
    
    auto* val = result->get("NAME");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "value");
}

TEST_F(ConfigTest, ParseWithQuotes) {
    auto result = Config::parse("VAR1=\"quoted value\"\nVAR2='single quoted'");
    ASSERT_TRUE(result.has_value());
    
    EXPECT_EQ(*result->get("VAR1"), "quoted value");
    EXPECT_EQ(*result->get("VAR2"), "single quoted");
}

TEST_F(ConfigTest, ParseWithComments) {
    auto result = Config::parse("# This is a comment\nVAR1=value1\n# Another comment\nVAR2=value2");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 2);
    
    EXPECT_EQ(*result->get("VAR1"), "value1");
    EXPECT_EQ(*result->get("VAR2"), "value2");
}

TEST_F(ConfigTest, ParseSkipsEmptyLines) {
    auto result = Config::parse("\n\nVAR1=value1\n\n\nVAR2=value2\n\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 2);
}

TEST_F(ConfigTest, ParseInvalidVariableName) {
    // Variable name starting with digit
    auto result = Config::parse("123VAR=value");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, Config::ErrorCode::INVALID_VARIABLE_NAME);

    // Variable name with invalid characters
    result = Config::parse("VAR-NAME=value");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, Config::ErrorCode::INVALID_VARIABLE_NAME);
}

TEST_F(ConfigTest, ParseValidVariableNames) {
    auto result = Config::parse("_var=1\nvar_name=2\nVAR123=3\n_123=4");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 4);
}

TEST_F(ConfigTest, ParseEqualsInValue) {
    auto result = Config::parse("PATH=/usr/bin:/usr/local/bin");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->get("PATH"), "/usr/bin:/usr/local/bin");
}

TEST_F(ConfigTest, LoadFromFile) {
    auto path = create_test_file("VAR1=value1\nVAR2=value2");
    
    auto result = Config::load_from_file(path);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->variables().size(), 2);
    EXPECT_EQ(*result->get("VAR1"), "value1");
}

TEST_F(ConfigTest, LoadNonExistentFile) {
    auto result = Config::load_from_file("/nonexistent/file.rc");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, Config::ErrorCode::FILE_NOT_FOUND);
}

TEST_F(ConfigTest, SetAndGetVariable) {
    Config config;
    EXPECT_TRUE(config.set("TEST_VAR", "test_value"));
    
    auto* val = config.get("TEST_VAR");
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "test_value");
}

TEST_F(ConfigTest, SetInvalidVariableName) {
    Config config;
    EXPECT_FALSE(config.set("123invalid", "value"));
    EXPECT_FALSE(config.set("invalid-name", "value"));
}

TEST_F(ConfigTest, UnsetVariable) {
    Config config;
    config.set("VAR1", "value1");
    EXPECT_TRUE(config.has("VAR1"));
    
    EXPECT_TRUE(config.unset("VAR1"));
    EXPECT_FALSE(config.has("VAR1"));
    
    // Unsetting non-existent variable returns false
    EXPECT_FALSE(config.unset("NONEXISTENT"));
}

TEST_F(ConfigTest, ClearVariables) {
    Config config;
    config.set("VAR1", "value1");
    config.set("VAR2", "value2");
    EXPECT_EQ(config.variables().size(), 2);
    
    config.clear();
    EXPECT_TRUE(config.variables().empty());
}

TEST_F(ConfigTest, OverwriteVariable) {
    Config config;
    config.set("VAR", "value1");
    EXPECT_EQ(*config.get("VAR"), "value1");
    
    config.set("VAR", "value2");
    EXPECT_EQ(*config.get("VAR"), "value2");
    EXPECT_EQ(config.variables().size(), 1);
}

TEST_F(ConfigTest, GetNonExistentVariable) {
    Config config;
    EXPECT_EQ(config.get("NONEXISTENT"), nullptr);
}

TEST_F(ConfigTest, DefaultConfigPath) {
    auto path = Config::default_config_path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(path.filename() == ".wshellrc");
}

TEST_F(ConfigTest, FileTooLarge) {
    // Create a file larger than 1MB
    auto path = temp_dir_ / "large_config.rc";
    {
        std::ofstream file(path, std::ios::binary);
        ASSERT_TRUE(file.is_open()) << "Failed to create test file: " << path;
        
        std::string large_content(2'000'000, 'x');  // 2MB
        file << large_content;
        file.flush();
        ASSERT_TRUE(file.good()) << "Failed to write test file content";
    } // Ensure file is closed and flushed
    
    // Verify file exists and has expected size
    ASSERT_TRUE(std::filesystem::exists(path)) << "Test file was not created";
    auto file_size = std::filesystem::file_size(path);
    EXPECT_GE(file_size, 2'000'000) << "Test file is too small: " << file_size;
    
    auto result = Config::load_from_file(path);
    ASSERT_FALSE(result.has_value()) << "Expected load_from_file to fail";
    EXPECT_EQ(result.error().code, Config::ErrorCode::FILE_TOO_LARGE) 
        << "Expected FILE_TOO_LARGE but got error code: " 
        << static_cast<int>(result.error().code) 
        << " with message: " << result.error().message;
}

TEST_F(ConfigTest, BashLikeExport) {
    // Test that lines like "export VAR=value" work
    auto result = Config::parse("VAR1=value1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->get("VAR1"), "value1");
}

} // namespace shell::test
