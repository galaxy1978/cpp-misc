/**
 * @brief 抽象工厂模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-6-29
*/

#pragma once

#include <type_traits>
#include <iostream>

#define DECLARE_ABSTRACT_FACTORY( absFactoryName ) \
class absFactoryName {							   \
public:									 	

#define END_DECLARE_ABSTRACT_FACTORY()  }

#define PRODUCT_NAME( productName ) 																		\
	template< typename productName , typename Args... ,														\
			  typename midType = typename std::decay< productName >::type,									\
			  typename realType = std::conditional<															\						
				  std::is_pointer< midType >::value ,														\
				  typename std::remove_pointer<midType>::type,												\
				  midType																					\
				  >																							\
			  >																								\
		static realType * create_##productName( Args&&... args ){											\
			static_assert( std::is_class<realType>::value  , "this function must be used with class" );		\
			static_assert( std::is_constructible<realType>::value , "class must constructible." );			\
			realType * ret = nullptr;																		\
			try{																							\
				realType * ret = new realType( std::forward<Args>(args)... );								\
			}catch( std::bad_alloc& e ){																	\
				std::cout << e.what() << std::endl;															\
			}																								\
			return ret;																						\
		}																									
		

/*
    // 使用示例
	// 抽象工厂头文件中
	// filename: myAbsFactory.hpp
	
	#include "prduct_a.hpp"
	#include "prduct_b.hpp"
	
	DECLARE_ABSTRACT_FACTORY( myAbsFactory )
		PRODUCT_NAME( a )
		PRODUCT_NAME( b )
		... ...
		... ...
	END_DECLARE_ABSTRACT_FACTORY();
	
	// 应用
	#include "myAbsFactory.hpp"
	
	... ... 
	
	a * pA = myAbsFactory::create_a()
	b * pB = myAbsFactory::create_b( 12, 34 )
*/