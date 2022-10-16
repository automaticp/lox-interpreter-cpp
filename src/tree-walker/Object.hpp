#pragma once
#include <memory>
#include <utility>


class ObjectImpl;

class Object {
private:
    std::unique_ptr<ObjectImpl> pimpl_;
public:
    template<typename ...Args>
    Object(Args&&... args) :
        pimpl_{ std::make_unique<ObjectImpl>(std::forward<Args>(args)...) }
    {}


private:
    ObjectImpl& impl() const noexcept {
        return *pimpl_;
    }
};

