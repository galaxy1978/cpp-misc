#include <iostream>
#include "absFactory.hpp"
using namespace wheels;
using namespace dm;

	class a{
	public:
	a(){
		std::cout << "a" << std::endl;
	}
	};
	class b{
	public:
	b(){
		std::cout << "b" << std::endl;
	}
	};
	
	class a1 : public a{
	public:
		a1(){
			std::cout << "a1" << std::endl;
		}
	};
	class b1 : public b{
		public:
		b1( int p ){
			std::cout << "b1" << std::endl;
		}
	};
	
	class a2 : public a{
		public:
		a2(){
			std::cout << "a2" << std::endl;
		}
	};
	class b2 : public b{
		
		public:
		b2( int p ){
			std::cout << "b2" << std::endl;
		}
	};
	
	// 声明抽象工厂
	DECLARE_ABSTRACT_FACTORY( myAbsFactory )
		ABST_PRODUCT_NAME_0( a )
		ABST_PRODUCT_NAME_1( b , int )
	END_DECLARE_ABSTRACT_FACTORY();


	// 实现抽象工厂1
	START_IMPL_ABST_FACTORY( implF1 , myAbsFactory )
		PRODUCT_NAME_0( a1 , a )
		PRODUCT_NAME_1( b1 , b , int )
	END_IMPL_ABST_FACTORY()

	// 实现抽象工厂2
	START_IMPL_ABST_FACTORY( implF2 , myAbsFactory )
		PRODUCT_NAME_0( a2 , a )
		PRODUCT_NAME_1( b2 , b , int )
	END_IMPL_ABST_FACTORY()

		
int main( void )
{
	myAbsFactory  * factory = nullptr;

	enum factoryType{ f1 , f2 };

	factoryType f;

	f = f2;
		
	switch( f ){
	case f1:
		factory = new implF1;
	break;
	case f2:
		factory = new implF2;
	}
	a * pA = factory->create_a();
	b * pB = factory->create_b( 12 );
	
	delete factory;
	delete pA;
	delete pB;

    return 0;
}