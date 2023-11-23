#include "designM/decrator.hpp"
using namespace wheels;
using namespace dm;

class ExampleClass
{
public:
    std::string name;
    
    ExampleClass() {}
    ExampleClass(const std::string& n) : name(n) {}

    void print()
    {
        std::cout << "Name: " << name << std::endl;
    }
};

int main()
{
    decorator<ExampleClass> dec;

    // 创建一个ExampleClass对象并装饰
    ExampleClass* obj1 = new ExampleClass("Obj1");
    dec.decrat(obj1);

    // 创建一个ExampleClass对象并装饰
    ExampleClass* obj2 = new ExampleClass("Obj2");
    dec.decrat(obj2);

    // 使用lambda表达式打印所有装饰的ExampleClass对象
    dec.decratMe([](ExampleClass* obj) {
        obj->print();
    });

    // 删除第一个装饰的对象
    dec.remove(0);

    std::cout << "After remove:" << std::endl;

    // 再次使用lambda表达式打印所有装饰的ExampleClass对象
    dec.decratMe([](ExampleClass* obj) {
        obj->print();
    });

    delete obj1;
    delete obj2;

    return 0;
}