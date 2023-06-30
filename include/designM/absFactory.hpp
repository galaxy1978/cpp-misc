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
#include "designM/factory.hpp"

namespace wheels
{
namespace dm
{
/// @brief 声明抽象工厂
#define DECLARE_ABSTRACT_FACTORY( absFactoryName )	\
	class absFactoryName {				\
public:							\
virtual ~absFactoryName(){}

/// @brief 结束声明抽象工厂
#define END_DECLARE_ABSTRACT_FACTORY()  };

/// @brief 声明纯虚的工厂函数，宏名称后的参数是工程函数的参数数量
#define ABST_PRODUCT_NAME_0( productName )			\
	virtual productName * create_##productName() = 0;

#define ABST_PRODUCT_NAME_1( productName , paramType1 )			\
	virtual productName * create_##productName(  paramType1 param ) = 0;

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

	
	
/// 具体工厂声明
/// @mparam implFactoryName , 具体工厂名称 ， 实现工厂名称必须是抽象工厂的子类
/// @mparam abstFactoryName , 抽象工厂名称
#define START_IMPL_ABST_FACTORY( implFactoryName , abstFactoryName )	\
	class implFactoryName: public abstFactoryName{			\
public:									\

/// 关闭具体工厂声明
#define END_IMPL_ABST_FACTORY()  };

/// 具体产品声明，实际上实现了具体产品的工厂函数 create_<抽象产品>(...) 的工厂函数
/// @mparam productType , 具体产品名称 ， 具体产品名称必须是抽象产品的子类
/// @mparam baseType , 抽象产品名称
#define PRODUCT_NAME_0( productType , baseType )	\
	virtual baseType * create_##baseType() override	\
	{						\
		return factory< productType >();	\
	}
	
#define PRODUCT_NAME_1( productType , baseType , paramType1 )		\
	virtual baseType * create_##baseType( paramType1 param ) override \
	{								\
		return factory< productType >( param );			\
	}

#define PRODUCT_NAME_2( productType , baseType , , paramType1 ,paramType2 ) \
	virtual baseType * create_##baseType( paramType1 param , paramType2 param ) override \
	{								\
		return factory< productType >( param , param2 );	\
	}

#define PRODUCT_NAME_3( productType , baseType , paramType1 , paramType2 , paramType3 ) \
	virtual baseType * create_##baseType( paramType1 p , paraType2 p2 , paramType3 p3 ) override \
	{								\
		return factory< productType >( p , p2, p3 );		\
	}

#define PRODUCT_NAME_4( productType , baseType , paramType1 , paramType2 , paramType3, paramType4 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2, paramType3 p3, paramType4 p4) override \
	{								\
		return factory< productType >( p1 , p2, p3, p4 );	\
	}

#define PRODUCT_NAME_5( productType , baseType , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2, paramType3 p3, paramType4 p4, paramType5 p5) override \
	{								\
		return factory< productType >( p1 , p2, p3,p4,p5 );	\
	}

#define PRODUCT_NAME_6( productType , baseType , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 , paramType6 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2, paramType3 p3, paramType4 p4, paramType5 p5 , paramType6 p6 ) override \
	{								\
		return factory< productType >( p1,p2,p3,p4,p5,p6 );	\
	}

#define PRODUCT_NAME_7( productType , baseType , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 , paramType6 , paramType7 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2 , paramType3 p3 , paramType4 p4, paramType5 p5 , paramType6 p6 , paramType7 p7 , paramType8 p8, paramType9 p9  ) override \
	{								\
		return factory< productType >( p1,p2,p3,p4,p5,p6,p7 );	\
	}

#define PRODUCT_NAME_8( productType , baseType , paramType1 , paramType2 , paramType3 , paramType4 , paramType5 , paramType6 , paramType7 , paramType8 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2 , paramType3 p3 , paramType4 p4, paramType5 p5 , paramType6 p6 , paramType7 p7 , paramType8 p8, paramType9 p9 ) override \
	{								\
		return factory< productType >( p1,p2,p3,p4,p5,p6,p7,p8 ); \
	}

#define PRODUCT_NAME_9( productType , baseType ,  paramType1 , paramType2 , paramType3 , paramType4 , paramType5 , paramType6, paramType7 , paramType8 , paramType9 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2 , paramType3 p3 , paramType4 p4, paramType5 p5 , paramType6 p6 , paramType7 p7 , paramType8 p8, paramType9 p9 ) override \
	{								\
		return factory< productType >( p1,p2,p3,p4,p5,p6,p7,p8,p9 ); \
	}

#define PRODUCT_NAME_10( productType , baseType ,  paramType1 , paramType2 , paramType3 , paramType4 , paramType5 , paramType6 , paramType7 , paramType8 , paramType9 , paramType10 ) \
	virtual baseType * create_##baseType( paramType1 p1, paramType2 p2 , paramType3 p3 , paramType4 p4, paramType5 p5 , paramType6 p6 , paramType7 p7 , paramType8 p8, paramType9 p9, paramType10 p10 ) override \
	{								\
		return factory< productType >( p1,p2,p3,p4,p5,p6,p7,p8,p9,p10 ); \
	}
	
	/**
	 * @brief 使用模板实现的抽象工厂模式
	 */
	template< typename... productTypes >
	struct abstractFactory
	{
		using prdtTypes = std::tuple< productTypes... >;
	
		template< size_t N , typename...Args >
		auto create( Args&&... args ) -> typename std::tuple_element< N , prdtTypes >::type *{
			return factory< typename std::tuple_element< N , prdtTypes >::type >( std::forward<Args>(args)...);
		}
	};
	
}}
