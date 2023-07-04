#include <iostream>
#include <memory>
#include <functional>
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



class carItfc{
public:
	void drive(){
		std::cout << "drive to hill " << std::endl;
	}
};
 
using CarBase = product<carItfc , Engine, Body, Wheels>;	

int main() {
    // 定义产品类型
   

    // 定义构建指导者
    using CarDirector = director<carItfc , Engine, Body, Wheels>;

    // 构建函数，用于指导构建过程
    std::function<void(std::shared_ptr<CarBase>)> buildProcess = [](std::shared_ptr<CarBase> car) {
        // 在这里可以自定义构建过程
        car->Engine::start();
        car->Body::shape();
        car->Wheels::roll();
    };

    // 构建汽车产品
    std::shared_ptr<CarBase> myCar = CarDirector::build(buildProcess);

    if (myCar) {
        std::cout << "Car build successfully!" << std::endl;
		myCar->drive();
    } else {
        std::cout << "Failed to build car." << std::endl;
    }

    return 0;
}