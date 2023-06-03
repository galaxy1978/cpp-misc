/**
 * @brief AVL测试程序
 */
#include <string>
#include "container/avl.hpp"

using avlInt = wheels::avl< int , std::string >;

avlInt gAvl;

void testInsert_2()
{
	gAvl.insert( 1, "1" );
	gAvl.insert( 2, "2" );
	gAvl.insert( 41, "41" );
	gAvl.insert( 3, "3" );
	gAvl.insert( 4, "4" );
	gAvl.insert( 10, "10" );
	gAvl.insert( 5, "5" );
	gAvl.insert( 6, "6" );
	gAvl.insert( -4, "-4" );
	gAvl.insert( 7, "7" );
}

void testDel_1()
{
//	gAvl.erase( 7 );
//	gAvl.erase( 10 );
//	gAvl.erase( 1 );
	gAvl.erase( 4 );
	
}

void testInsert_1()
{
	gAvl.insert( 1, "abc" );
	gAvl.insert( 1, "abc" );
}

int main( int argc , char * const argv[] )
{
	int ret = 0;

	testInsert_2();
	//testDel_1();
	std::list<std::string> rst;
	gAvl.order( rst );

	for( auto i : rst ){
		std::cout << i << " ";
	}
	return ret;
}
