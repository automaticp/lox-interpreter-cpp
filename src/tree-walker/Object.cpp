#include "Object.hpp"
#include "ObjectImpl.hpp"
#include <memory>


Object::Object() : pimpl_{ std::make_unique<ObjectImpl>() } {}
Object& Object::operator=(Object &&) = default;
Object::Object(Object &&) = default;
Object::~Object() = default;


Object::Object(const Object& other) :
    pimpl_{ std::make_unique<ObjectImpl>(other.impl()) } {}

Object& Object::operator=(const Object& other) {
    if (this != &other) {
        pimpl_ = std::make_unique<ObjectImpl>(other.impl());
    }
    return *this;
}
