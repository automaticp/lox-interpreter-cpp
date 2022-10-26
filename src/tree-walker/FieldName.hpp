#pragma once
#include <string>
#include <functional>
#include <utility>
#include <boost/functional/hash.hpp>

class FieldName {
private:
    std::string name_;
public:
    explicit(false) FieldName(std::string name) : name_{ std::move(name) } {
        // FIXME: validate name
    }

    bool operator==(const FieldName& other) const noexcept {
        return name_ == other.name_;
    }

    friend class std::hash<FieldName>;
    friend class boost::hash<FieldName>;
};

template<>
struct std::hash<FieldName> {
    auto operator()(const FieldName& fname) const {
        return std::hash<std::string>{}(fname.name_);
    }
};

template<>
struct boost::hash<FieldName> {
    auto operator()(const FieldName& fname) const {
        return std::hash<std::string>{}(fname.name_);
    }
};
