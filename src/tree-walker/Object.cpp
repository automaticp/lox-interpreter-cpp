#include "Object.hpp"
#include "ObjectImpl.hpp"
#include <memory>


Object::Object() : pimpl_{ std::make_unique<ObjectImpl>() } {}
Object& Object::operator=(Object &&) = default;
Object::Object(Object &&) = default;
Object::~Object() = default;
