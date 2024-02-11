/**
 * @brief 抽象工厂模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-6-29
 */

#pragma once
#include <memory>
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
#define ABST_PRODUCT_NAME( productName , ... )			\
	virtual productName * create_##productName(__VA_ARGS__ ) = 0;


	
	
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
#define PRODUCT_NAME_0( productType , baseType , ... )	\
	virtual baseType * create_##baseType(__VA_ARGS__ ) override	\
	{						\
		return factory< productType >();	\
	}
	
	/**
	 * @brief 使用模板实现的抽象工厂模式
	 */
	template< typename... productTypes >
	struct abstractFactory
	{
		using prdtTypes = std::tuple< productTypes... >;
        /**
         * @brief create
         * @param args
         */
		template< size_t N , typename...Args >
		auto create( Args&&... args ) -> typename std::tuple_element< N , prdtTypes >::type *{
			return factory< typename std::tuple_element< N , prdtTypes >::type >( std::forward<Args>(args)...);
		}
        /**
         * @brief makeShared
         * @param args
         * @return
         */
        template< size_t N , typename...Args >
        auto makeShared( Args&&... args ) -> std::shared_ptr< typename std::tuple_element< N , prdtTypes >::type >{
            return std::make_shared< typename std::tuple_element< N , prdtTypes >::type >( std::forward<Args>(args)...);
        }
	};
	
}}





