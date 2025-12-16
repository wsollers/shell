// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/output_destination.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <filesystem>

namespace shell::test {

class OutputDestinationTest : public ::testing::Test {
protected:
    void SetUp() override {
        temp_dir_ = std::filesystem::temp_directory_path() / "wshell_output_test";
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(temp_dir_, ec);
    }

    std::filesystem::path temp_dir_;
};

//==============================================================================
// StreamOutputDestination Tests
//==============================================================================

TEST_F(OutputDestinationTest, StreamWrite) {
    std::ostringstream stream;
    StreamOutputDestination dest(stream, "test_stream");
    
    EXPECT_EQ(dest.destination_name(), "test_stream");
    
    auto result = dest.write("Hello, ");
    EXPECT_TRUE(result.has_value());
    
    result = dest.write("World!");
    EXPECT_TRUE(result.has_value());
    
    EXPECT_EQ(stream.str(), "Hello, World!");
}

TEST_F(OutputDestinationTest, StreamFlush) {
    std::ostringstream stream;
    StreamOutputDestination dest(stream, "test_stream");
    
    dest.write("Test");
    auto result = dest.flush();
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(stream.str(), "Test");
}

TEST_F(OutputDestinationTest, StreamWriteMultipleLines) {
    std::ostringstream stream;
    StreamOutputDestination dest(stream, "multiline");
    
    dest.write("Line 1\n");
    dest.write("Line 2\n");
    dest.write("Line 3\n");
    
    EXPECT_EQ(stream.str(), "Line 1\nLine 2\nLine 3\n");
}

//==============================================================================
// StringOutputDestination Tests
//==============================================================================

TEST_F(OutputDestinationTest, StringCapture) {
    StringOutputDestination dest("capture");
    
    EXPECT_EQ(dest.destination_name(), "capture");
    EXPECT_TRUE(dest.captured_output().empty());
    
    auto result = dest.write("First ");
    EXPECT_TRUE(result.has_value());
    
    result = dest.write("Second ");
    EXPECT_TRUE(result.has_value());
    
    result = dest.write("Third");
    EXPECT_TRUE(result.has_value());
    
    EXPECT_EQ(dest.captured_output(), "First Second Third");
}

TEST_F(OutputDestinationTest, StringClear) {
    StringOutputDestination dest;
    
    dest.write("Some content");
    EXPECT_FALSE(dest.captured_output().empty());
    
    dest.clear();
    EXPECT_TRUE(dest.captured_output().empty());
    
    dest.write("New content");
    EXPECT_EQ(dest.captured_output(), "New content");
}

TEST_F(OutputDestinationTest, StringFlushNoOp) {
    StringOutputDestination dest;
    
    dest.write("Test");
    auto result = dest.flush();
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(dest.captured_output(), "Test");
}

//==============================================================================
// FileOutputDestination Tests
//==============================================================================

TEST_F(OutputDestinationTest, FileWriteTruncate) {
    auto path = temp_dir_ / "output.txt";
    
    {
        FileOutputDestination dest(path, FileOutputDestination::Mode::Truncate);
        EXPECT_EQ(dest.destination_name(), path.string());
        
        auto result = dest.write("First write\n");
        EXPECT_TRUE(result.has_value());
        
        result = dest.write("Second write\n");
        EXPECT_TRUE(result.has_value());
    }
    
    // Verify file contents
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "First write\nSecond write\n");
}

TEST_F(OutputDestinationTest, FileWriteAppend) {
    auto path = temp_dir_ / "append.txt";
    
    // First write
    {
        FileOutputDestination dest(path, FileOutputDestination::Mode::Truncate);
        dest.write("Initial content\n");
    }
    
    // Append
    {
        FileOutputDestination dest(path, FileOutputDestination::Mode::Append);
        dest.write("Appended content\n");
    }
    
    // Verify
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "Initial content\nAppended content\n");
}

TEST_F(OutputDestinationTest, FileFlush) {
    auto path = temp_dir_ / "flush.txt";
    
    FileOutputDestination dest(path);
    dest.write("Buffered content");
    
    auto result = dest.flush();
    EXPECT_TRUE(result.has_value());
    
    // File should be flushed even without closing
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "Buffered content");
}

//==============================================================================
// Polymorphic Usage Tests
//==============================================================================

TEST_F(OutputDestinationTest, PolymorphicWrite) {
    std::ostringstream stream;
    StreamOutputDestination stream_dest(stream, "stream");
    StringOutputDestination string_dest("string");
    
    auto write_to_dest = [](IOutputDestination& dest, std::string_view msg) {
        return dest.write(msg);
    };
    
    EXPECT_TRUE(write_to_dest(stream_dest, "Test").has_value());
    EXPECT_TRUE(write_to_dest(string_dest, "Test").has_value());
    
    EXPECT_EQ(stream.str(), "Test");
    EXPECT_EQ(string_dest.captured_output(), "Test");
}

} // namespace shell::test
