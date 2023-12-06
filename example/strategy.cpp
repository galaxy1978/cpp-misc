#include <iostream>

#include "designM/strategy.hpp"

using namespace wheels;
using namespace dm;

enum class emKey
{
	EM_A,
	EM_B,
	EM_C
};

void fun1( int data )
{
	std::cout << "fun1 data = " << data << std::endl;
}

struct myClassA
{
	void fun1( int data ){
		std::cout << "myClassA::fun1 data = " << data << std::endl;
	}
};

int main()
{
	using strategy_t = strategy<emKey , std::function< void (int) > >;
	
	strategy_t strgy;
	myClassA  class_a;
	
	strgy.add( emKey::EM_A , fun1 );
	strgy.add( emKey::EM_B , std::bind( &myClassA::fun1 , &class_a , std::placeholders::_1 ) );
	strgy.add( emKey::EM_C , [](int data ){
		std::cout << "lambda fun data = " << data << std::endl;
	});
	
	strgy.call( emKey::EM_A , 1 );
	strgy.call( emKey::EM_B , 10 );
	strgy.call( emKey::EM_C , 99 );
	
	return 0;
}