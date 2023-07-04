#include <iostream>
#include <memory>
#include <functional>

// gcc -I<你的路径>/wheels/C++/include -std=c++11 ./adaptor.cpp -o a1.exe -lstdc++
/* 执行结果：
Engine started.
Body shaped.
Wheels rolled.
Car build successfully!
*/
#include "designM/builder.hpp"
using namespace wheels;
using namespace dm;
// 定义组件类
class Engine {
public:
    void start() {
        std::cout << "Engine started." << std::endl;
    }
};

class Body {
public:
    void shape() {
        std::cout << "Body shaped." << std::endl;
    }
};

class Wheels {
public:
    void roll() {
        std::cout << "Wheels rolled." << std::endl;
    }
};

int main() {
    // 定义产品类型
    using Car = product<Engine, Body, Wheels>;

    // 定义构建指导者
    using CarDirector = director<Engine, Body, Wheels>;

    // 构建函数，用于指导构建过程
    std::function<void(std::shared_ptr<Car>)> buildProcess = [](std::shared_ptr<Car> car) {
        // 在这里可以自定义构建过程
        car->Engine::start();
        car->Body::shape();
        car->Wheels::roll();
    };

    // 构建汽车产品
    std::shared_ptr<Car> myCar = CarDirector::build(buildProcess);

    if (myCar) {
        std::cout << "Car build successfully!" << std::endl;
    } else {
        std::cout << "Failed to build car." << std::endl;
    }

    return 0;
}