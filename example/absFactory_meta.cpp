#include <type_traits>
#include <memory>
#include <iostream>
#include "designM/absFactory.hpp"
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
	
using abf1 = absFactory< a1 , b1 >;
using abf2 = absFactory< a2 , b2 >;		

int main( void )
{

	abf1 f1; 
	abf2 f2;

	a * pA = f1.create<0>();
	b * pB = f1.create<1>( 12 );
	
	delete pA;
	delete pB;
	
	pA = f2.create<0>();
	pB = f2.create<1>( 12 );
	
	delete pA;
	delete pB;
	
}