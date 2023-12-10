#include <iostream>

#include "designM/proxy.hpp"

using namespace wheels;
using namespace dm;

// 定义一个接口 
DECLARE_PROXY_ITFC(MyInterface) 
	PROXY_ITFC_MTD(int, methodItfc) 
END_PROXY_ITFC(); 

// 定义一个实现类 
class MyImplementation { 
public: 
	int methodName() { 
		std::cout << "Mission is done int subject object" << std::endl;
		return 42; 
	} 
}; 

class myProxy : public proxy<MyInterface , MyImplementation  > {
public:
	int methodItfc() override{
		int ret = 12 * get__()->methodName();
		return ret;
	}
};

int main() 
{ 
	// 创建代理类 
	myProxy  my_proxy; // 创建实现类对象
	// 调用代理类的方法
	my_proxy.create();  
	auto result = my_proxy.methodItfc(); 
	std::cout << "after proxy modified result: " << result << std::endl; 
	return 0; 
}