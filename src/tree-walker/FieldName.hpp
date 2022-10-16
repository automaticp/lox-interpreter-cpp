#pragma once
#include <string>
#include <functional>
#include <utility>


class FieldName {
private:
    std::string name_;
public:
    FieldName(std::string name) : name_{ std::move(name) } {
        // FIXME: validate name
    }

    bool operator==(const FieldName& other) const noexcept {
        return name_ == other.name_;
    }

    friend class std::hash<FieldName>;
};

template<>
struct std::hash<FieldName> {
    auto operator()(const FieldName& fname) const {
        return std::hash<std::string>{}(fname.name_);
    }
};

