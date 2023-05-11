# singleton 单例模式
#### 实现原理
- 1 利用 std::map , std::function 存储策略方法
- 2 统一调用接口, 使用call根据条件变量调用对应的方法
- 3 支持任意类型的返回值
- 4 纯模板实现，直接包含使用
#### 主要功能
- 支持任意条件变量
- 支持任意个参数传递

#### 示例
```
#include <vector>
#include <iostream>

#include "designM/strategy.hpp"


using namespace wheels;
using namespace dm;

int fun1( int param , int param2)
{
	std::cout << "fun1 called" << std::endl;
	return param + param2;
}

int fun2( int param , int param2)
{
	std::cout << "fun2 called" << std::endl;
	return param * param2;
}

int main( void )
{
        // 构造策略对象，模板参数的含义：条件变量类型，返回值类型，方法参数表
	strategy< int , int , int , int >  strat;
	// 调用add添加方法
	strat.add( 0 , []( int param , int param2)->int{
		return fun1( param , param2);
	});
	
	strat.add( 1 , []( int param , int param2)->int{
		return fun2( param , param2);
	});
	// 使用策略对象的call方法调用实际的具体方法
	std::cout << strat.call( 0 , 12 , 12) << std::endl;
	std::cout << strat.call( 1 , 78 , 12) << std::endl;

	return 0;
}
```