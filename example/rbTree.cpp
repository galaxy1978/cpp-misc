/**
 * @brief AVL测试程序
 */
#include <string>
#include "container/rbtree.hpp"

using avlInt = wheels::rbTree< int , std::string >;

avlInt gAvl;

void testInsert_2()
{
	gAvl.insert( 1, "str1" );
	gAvl.insert( 2, "str2" );
	gAvl.insert( 41, "str41" );
	gAvl.insert( 3, "str3" );
	gAvl.insert( 4, "str4" );
	gAvl.insert( 10, "str10" );
	gAvl.insert( 5, "str5" );
	gAvl.insert( 6, "str6" );
	gAvl.insert( -4, "str-4" );
	gAvl.insert( 7, "str7" );
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
        auto it = gAvl.find( -4 );
	std::cout << "data is: " <<  *it << std::endl;
	return ret;
}
