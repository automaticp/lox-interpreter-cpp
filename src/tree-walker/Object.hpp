#pragma once
#include <memory>
#include <utility>

class ObjectImpl;

class Object {
private:
    std::unique_ptr<ObjectImpl> pimpl_;
public:
    Object();
    // Defined is cpp file, as otherwise unique_ptr
    // tries to generate ObjectImpl's destructor here.
    // Which doesn't work due to class being incomplete.
    Object &operator=(Object &&);
    Object(Object &&);
    ~Object();

private:
    ObjectImpl& impl() const noexcept {
        return *pimpl_;
    }
};


