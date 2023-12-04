// 此示例程序由@Leezheng1提供

#include <iostream>
#include <functional>

#include "designM/adaptor.hpp"

// 接口类
class Interface {
public:
    virtual void method(int param) = 0;
};

// 实现接口的类
class Implementation1 : public Interface {
public:
    void method(int param) override {
        std::cout << "Implementation1: " << param << std::endl;
    }
};

class Implementation2 : public Interface {
public:
    void method(int param) override {
        std::cout << "Implementation2: " << param << std::endl;
    }
};

int main() {

    // 创建被适配的对象
    Implementation1 impl1;
    Implementation2 impl2;

    // 创建适配器
    wheels::dm::adapter<std::function< void(Interface*, int) >, Interface, Implementation1, Implementation2> adapter(impl1, impl2);

    // 设置回调函数
    adapter.set([](Interface* obj, int param) {
        obj->method(param);
    });

    // 执行请求
    adapter.request(10);

    return 0;
}