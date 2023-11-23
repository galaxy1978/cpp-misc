#include "designM/factory.hpp"
using namespace wheels;
using namespace dm;

class ExampleClass
{
public:
    int value;

    ExampleClass() : value(0) {}
    ExampleClass(int v) : value(v) {}

    void print()
    {
        std::cout << "Value: " << value << std::endl;
    }
};

int main()
{
    auto obj1 = Factory<ExampleClass>::factory();
    obj1->print();

    auto obj2 = Factory<ExampleClass>::factory(10);
    obj2->print();

    auto obj3 = Factory<ExampleClass>::factory([](Factory<ExampleClass>::emErrCode err) {
        std::cout << "Error allocating memory" << std::endl;
    });
    if (obj3 == nullptr) {
        std::cout << "Failed to allocate memory" << std::endl;
    }

    delete obj1;
    delete obj2;
    delete obj3;

    return 0;
}