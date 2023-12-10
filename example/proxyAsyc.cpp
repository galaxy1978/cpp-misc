#include <iostream>
#include <sstream>

#include "designM/proxy.hpp"

using namespace wheels;
using namespace dm;

DECLARE_PROXY_ITFC(MyInterface) 
END_PROXY_ITFC();

class MyImplementation { 
public: 
	std::string methodName( int data ) { 
		std::stringstream ss;
		ss << " data to str: " << data;
		return ss.str(); 
	} 
}; 

int main() { 
	// 创建代理类 
	using MyProxy = proxy<MyInterface, MyImplementation >;
	MyProxy  myProxy; // 创建实现类对象 
	myProxy.create();
	auto result = myProxy.agentCall([]( std::shared_ptr<MyImplementation > ptr , int a ){
		return ptr->methodName( a );
	} , 12 ); 
	std::cout << result.get() << std::endl; 
	return 0; 
}