/**
 * @brief 抽象工厂模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-6-29
 */

#pragma once

#include <type_traits>
#include <memory>
#include <iostream>

/// @brief 声明抽象工厂
#define DECLARE_ABSTRACT_FACTORY( absFactoryName )	\
	class absFactoryName {				\
public:									 	

/// @brief 结束声明抽象工厂
#define END_DECLARE_ABSTRACT_FACTORY()  };

/// @brief 声明纯虚的工厂函数，宏名称后的参数是工程函数的参数数量
#define ABST_PRODUCT_NAME_0( productName )			\
	virtual productName * create_##productName() = 0;

#define ABST_PRODUCT_NAME_1( productName , paramType1 )			\
	virtual productName * create_##productName(  paramType1 ) = 0;

#define ABST_PRODUCT_NAME_2( productName , paramType1 , paramType2 )	\
	virtual productName * create_##productName( paramType1 , paramType2 ) = 0;

#define ABST_PRODUCT_NAME_3( productName , paramType1 , paramType2 , paramType3 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 ) = 0;

#define ABST_PRODUCT_NAME_4( productName , paramType1 , paramType2 , paramType3 , paramType4 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 ) = 0;

#define ABST_PRODUCT_NAME_5( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ) = 0;

#define ABST_PRODUCT_NAME_6( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ,paramType6 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 ) = 0;

#define ABST_PRODUCT_NAME_7( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 ) = 0;

#define ABST_PRODUCT_NAME_8( productName , aparamType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7, paramType8 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8) = 0;

#define ABST_PRODUCT_NAME_9( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 ) = 0;

#define ABST_PRODUCT_NAME_10( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 , paramType10 ) \
	virtual productName * create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 , paramType10 ) = 0;

// 以上是裸指针的方式，下面的声明使用了std::shared_ptr
#define ABST_PRODUCT_NAME_SHARED_0( productName )			\
	virtual std::shared_ptr< productName > create_##productName() = 0;

#define ABST_PRODUCT_NAME_SHARED_1( productName , paramType1 )			\
	virtual std::shared_ptr< productName > create_##productName(  paramType1 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_2( productName , paramType1 , paramType2 )	\
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_3( productName , paramType1 , paramType2 , paramType3 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_4( productName , paramType1 , paramType2 , paramType3 , paramType4 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_5( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_6( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ,paramType6 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_7( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_8( productName , aparamType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7, paramType8 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8) = 0;

#define ABST_PRODUCT_NAME_SHARED_9( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 ) = 0;

#define ABST_PRODUCT_NAME_SHARED_10( productName , paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 , paramType10 ) \
	virtual std::shared_ptr< productName > create_##productName( paramType1 , paramType2 , paramType3 , paramType4 , paramType5, paramType6 , paramType7 , paramType8 , paramType9 , paramType10 ) = 0;
	
	
/// 具体工厂声明
/// @mparam implFactoryName , 具体工厂名称 ， 实现工厂名称必须是抽象工厂的子类
/// @mparam abstFactoryName , 抽象工厂名称
#define START_IMPL_ABST_FACTORY( implFactoryName , abstFactoryName )	\
	class implFactoryName : public abstFactoryName{                     \
    public:                                                             \
        implFactoryName(){}

/// 关闭具体工厂声明
#define END_IMPL_ABST_FACTORY()  };

/// 具体产品声明，实际上实现了具体产品的工厂函数 create_<抽象产品>(...) 的工厂函数
/// @mparam productType , 具体产品名称 ， 具体产品名称必须是抽象产品的子类
/// @mparam baseType , 抽象产品名称
#define PRODUCT_NAME( productType , baseType )                                                                                              \
	template< typename productType , typename ...Args ,	                                                                                    \
		typename midType = typename std::decay< type >::type,                                                                               \
		typename midType2 = std::conditional<	std::is_pointer< midType >::value , typename std::remove_pointer<midType>::type, midType >, \
		typename realType = typename std::enable_if< std::is_base_of< realType , baseType >::value ,midType2 >::type >                      \
	virtual baseType * create_##baseType( Args&&... args) override                                                                           \
	{                                                                                                                                       \
		realType * ret = nullptr;                                                                                                           \
		try{                                                                                                                                \
			ret = new realType( std::forward<Args>(args)... );                                                                              \
		}catch( std::bad_alloc& e ){                                                                                                        \
			std::cout << e.what() << std::endl;	                                                                                            \
		}                                                                                                                                   \
		return ret;						                                                                                                    \
	}

/// 这个实现使用了shared_ptr。这也要求抽象工厂中产品声明的时候使用XXX_SHARED_Y进行声明
#define PRODUCT_NAME_SHARED( productType , baseType )                                                                                       \
	template< typename productType , typename ...Args ,                                                                                     \
		typename midType = typename std::decay< type >::type,                                                                               \
		typename midType2 = std::conditional<	std::is_pointer< midType >::value , typename std::remove_pointer<midType>::type, midType >, \
		typename realType = typename std::enable_if< std::is_base_of< realType , baseType >::value , midType2 >::type >                     \
	virtual std::shared_ptr< baseType > create##baseType( Args&&... args) override                                                          \
	{                                                                                                                                       \
		std::shared_ptr< baseType > ret;			                                                                                        \
		try{                                                                                                                                \
			ret = std::make_shared< realType >( std::forward<Args>(args)... );                                                              \
		}catch( std::bad_alloc& e ){                                                                                                        \
			std::cout << e.what() << std::endl;                                                                                             \
		}                                                                                                                                   \
		return ret;                                                                                                                         \
	}

/*
	// 使用示例
	// 抽象工厂头文件中
	// filename: myAbsFactory.hpp
	// 这两个头文件是抽象的产品头文件
	#include "prduct_a.hpp"
	#include "prduct_b.hpp"
	
	// 杰西莱的四个头文件是实际产品的头文件
	#include "prduct_a1.hpp"
	#include "prduct_b1.hpp"

	#include "prduct_a2.hpp"
	#include "prduct_b2.hpp"

	// 声明抽象工厂
	DECLARE_ABSTRACT_FACTORY( myAbsFactory )
		// 相当于定义一个抽象函数
		// virtual a * create_a() = 0;
		ABST_PRODUCT_NAME_0( a )
		
		// 相当于定义一个抽象函数
		// virtual b * create_b( int ) = 0;
		ABST_PRODUCT_NAME_1( b , int )
		... ...
		... ...
	END_DECLARE_ABSTRACT_FACTORY();


	// 实现抽象工厂1
	START_IMPL_ABST_FACTORY( implF1 , myAbsFactory )
		PRODUCT_NAME( a1 , a )
		PRODUCT_NAME( b1 , b )
	END_IMPL_ABST_FACTORY()

	// 实现抽象工厂2
	START_IMPL_ABST_FACTORY( implF2 , myAbsFactory )
		PRODUCT_NAME( a2 , a )
		PRODUCT_NAME( b2 , b )
	END_IMPL_ABST_FACTORY()

		
	// 应用
	#include "myAbsFactory.hpp"

	myAbsFactory  * factory = nullpr;
	... ... 

	enum factoryType{ f1 , f2 };

	factoryType f;

	... ...
		
	switch( f ){
	case f1:
		factory = new implF1;
	break;
	case f2:
		factory = new implF2;
	}
	a * pA = factory->create_a();
	b * pB = factory->create_b( 12, 34 );
*/
