#ifndef OBJECT_H
#define OBJECT_H

#include <string>

class Object {
public:
    // 构造函数
    Object(const std::string& name);

    // 成员函数：打印对象的名字
    void printName() const;

private:
    // 私有成员变量
    std::string name_;
};

#endif // OBJECT_H
