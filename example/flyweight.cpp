#include <iostream>
#include <string>

#include "designM/flyweight.hpp"

using namespace wheels;
using namespace dm;
// 定义一个飞机类
class Airplane {
public:
	Airplane():capacity_(0){}
	Airplane( int c ):capacity_(c){}
	
	int getC(){ return capacity_; }
private:
    int capacity_;
};

int main() {
    // 创建一个flyweight对象来管理飞机对象
    using AirplaneFlyweight = flyweight<std::string >;

    // 创建飞机工厂对象，使用flyweight类管理飞机对象
    AirplaneFlyweight airplaneFactory;

    // 添加飞机对象到工厂
    airplaneFactory.set<Airplane>("Boeing747" , 500 );
    airplaneFactory.set<Airplane>("AirbusA380", 600);

    // 获取飞机对象并打印其名称
    auto boeing747 = airplaneFactory.get("Boeing747");
    auto airbusA380 = airplaneFactory.get("AirbusA380");

    std::cout << "Boeing747 name: " << boeing747.get<Airplane>().getC() << std::endl;
    std::cout << "AirbusA380 name: " << airbusA380.get<Airplane>().getC() << std::endl;

    return 0;
}
