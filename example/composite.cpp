#include <iostream>
#include <functional>
#include "composite.hpp"

// 定义一个接口类
class Interface {
public:
    virtual void performAction() = 0;
};

// 实现具体的接口类
class ConcreteClass : public Interface {
public:
    void performAction() override {
        std::cout << "Performing action" << std::endl;
    }
};

int main() {
    // 创建根对象
    auto root = std::make_shared<composite<std::shared_ptr<Interface>>>(nullptr);

    // 创建子对象
    auto child1 = std::make_shared<composite<std::shared_ptr<Interface>>>(std::make_shared<ConcreteClass>());
    auto child2 = std::make_shared<composite<std::shared_ptr<Interface>>>(std::make_shared<ConcreteClass>());
    auto child3 = std::make_shared<composite<std::shared_ptr<Interface>>>(std::make_shared<ConcreteClass>());

    // 将子对象添加到根对象中
    root->add(child1);
    root->add(child2);
    root->add(child3);

    // 调用子对象的接口方法
    root->for_each([](composite<Interface>* item) {
        item->performAction();
    });

    return 0;
}