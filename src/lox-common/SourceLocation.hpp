#pragma once
#include <memory>
#include <filesystem>
#include <cstdint>
#include <utility>


// 16 + 4 bytes with 8-byte alignment = 24 bytes.
// I wanted to pack this into 20 bytes as part of Token
// but it seems that life is not so forgiving.
struct SourceLocation {
    // UD constructors in order to disable aggregate initialization.
    SourceLocation(uint16_t line, uint16_t column, std::shared_ptr<std::filesystem::path> file) :
        line{ line }, column{ column }, filepath{ std::move(file) }
    {}

    SourceLocation(uint16_t line, uint16_t column) :
        SourceLocation{ line, column, nullptr }
    {}

    // Filenames can be long, and will not fit into the 16 or 23 byte sso buffer.
    // We share the filenames in order to not spam the heap with strings.
    std::shared_ptr<std::filesystem::path> filepath{ nullptr };
    // Hope that your files aren't more than ~64k lines long...
    uint16_t line;
    uint16_t column;

    bool has_file() const noexcept { return filepath.get(); }

    const std::filesystem::path& file() const noexcept {
        return *filepath;
    }

    bool operator==(const SourceLocation&) const noexcept = default;
};


