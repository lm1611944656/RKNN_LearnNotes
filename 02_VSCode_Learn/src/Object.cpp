#include "Object.h"
#include <iostream>

// 构造函数的实现
Object::Object(const std::string& name) : name_(name) {
    // 初始化工作可以在这里完成
    std::cout << "The Object object is constructed!" << std::endl;
}

// 成员函数的实现
void Object::printName() const {
    std::cout << "Object Name: " << name_ << std::endl;
}