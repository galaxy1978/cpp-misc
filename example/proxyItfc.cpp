#include <iostream>
#include <sstream>

#include "designM/proxy.hpp"

using namespace wheels;
using namespace dm;

// itfc定义了一个代理函数的代理接口类
using itfc = proxyItfc< int , const std::string& >;

// 这个是代理目标类
class abc{
public:
	int dosth( const std::string& a , int b ){
		std::cout << "abc::dosth " << a << " " << b << std::endl; 
		return 3;
	}
};
// 定义具体代理类，从模板函数进行继承并实现接口函数。
class myProxy : public proxy< itfc , abc >{
	// 实现接口函数，这个函数在proxyItfc声明了
public:
	virtual int agent( const std::string& str ) override{
		std::cout << "myProxy::agent " << str << std::endl; 
		int param1 = 12;
		std::stringstream ss;
		ss << str << ":" << param1;
		auto ptr = get__();
		if( ptr ){
			param1 = 12 / ptr->dosth( ss.str() , 12 );
		}
		return param1;
	}
};

int main()
{
	myProxy   my_proxy;   // 实实例化代理类
	my_proxy.create();    // 实例化代理目标
	
	// 执行代理操作
	int rst = my_proxy.agent( "abc" );
	std::cout << rst << std::endl;
	return 0;
} 