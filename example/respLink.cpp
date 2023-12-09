#include <iostream>
#include "designM/rspsLink.hpp"
using namespace wheels;
using namespace dm;

DECLARE_RESPLINK_ITFC( respItfc )
	RESPLINK_ITFC_MTD( int , int )
END_DECLARE_RESPLINK_ITFC()

struct Implementation1 : respItfc {
    bool operation(int data , int b ) override {
        std::cout << "Implementation1: " << data << " data b: " << b << std::endl;
        return true;
    }
};

struct Implementation2 : respItfc {
    bool operation(int data , int b ) override {
        std::cout << "Implementation2: " << data << " data b: " << b << std::endl;
        return false;
    }
};

int main() {
    rspsLink<respItfc> link;

    Implementation1 impl1;
    Implementation2 impl2;

    link.push_back(&impl1);
    link.push_back(&impl2);

    link.request(123 , 789);

    return 0;
}

// 运行结果
/*
Implementation1: 123 data b: 789
Implementation2: 123 data b: 789
*/