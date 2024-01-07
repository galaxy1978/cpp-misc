#include <iostream>
#include "designM/decorator.hpp"

using namespace wheels;
using namespace dm;

DECLARE_DECORTEE_DEFAULT(IDecoratee , const std::string& )

// 实现被装饰者接口的具体类
class ConcreteDecoratee1 : public IDecoratee {
public:
    void operation(const std::string& str) override {
        std::cout << "ConcreteDecoratee 1: " << str << std::endl;
    }
};

class ConcreteDecoratee2 : public IDecoratee {
public:
    void operation(const std::string& str) override {
        std::cout << "ConcreteDecoratee 2: " << str << std::endl;
    }
};

int main() {
    decorator<IDecoratee> dcrtr;
    ConcreteDecoratee1 * obj1 = new ConcreteDecoratee1();
    ConcreteDecoratee2 * obj2 = new ConcreteDecoratee2();

    // 将被装饰者对象添加到装饰者
    size_t idx1 = dcrtr.decrat(obj1);
    size_t idx2 = dcrtr.decrat(obj2);

    // 对所有被装饰者对象进行操作
    dcrtr.decratMe("Hello");

    return 0;
}