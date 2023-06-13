/**
 * @brief 工厂模式实现
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-25
 */

#pragma once
#include <type_traits>
#include <functional>
#include <iostream>

using namespace wheels
{
	using namespace dm
	{       /**
		 * @example:
		 *      class a : public Factory< a >{
		 *           public:
		 *                a();
		 *                a( int c );
		 *      };
		 *
		 *      a * pa = a::factory( 23 );
		 *      a * pa2 = a::factory( []( Factory::emErrCode err){
		 *                      // 错误处理
		 *                } , 23 );
		 */
		template< typename type ,
			  typename midType = typename std::decay< type >::type,
			  typename realType = std::conditional<
				  std::is_pointer< midType >::value ,
				  typename std::remove_pointer<midType>::type,
				  midType
				  >
			>
		struct Factory
		{
		private:
			/// 禁止采用构造函数进行构造
			Factory(){};
		public:
			static_assert( std::is_class<realType>::value  , "this function must be used with class" );
			
			enum emErrCode{
				ERR_ALLOC_MEM = -1000,
				OK = 0
			};

			template< typename ...Args >
			static realType * factory( std::functional< void (Factory::emErrCode) > errfun , Args... args ){
				realType * ret = nullptr;
				try{
					realType * ret = new reakType( (0,args)... );
				}catch( std::bad_alloc& e ){
					std::cout << e.what() << std::endl;
					if( errfun ){
						errfun( Factory::ERR_ALLOC_MEM );
					}
				}
			
				return ret;
			}

			template< typename ...Args >
			static realType * factory( Args... args ){
				realType * ret = nullptr;
				try{
					realType * ret = new reakType( (0,args)... );
				}catch( std::bad_alloc& e ){
					std::cout << e.what() << std::endl;
					if( errfun ){
						errfun( Factory::ERR_ALLOC_MEM );
					}
				}
			
				return ret;
			}
		};

		/**
		 * @brief 模板工厂函数
		 * @tparam type 类名称		 
		 * @tparam Args... ， 调用构造函数的时候构造函数的参数类型表
		 * @param args [ I ], 构造函数的参数表
		 * @param errfun[ I ],发生错误的回调函数
		 * @note 这个函数接口，只能针对共有构造函数的进行工厂创建。如果存在保护的或者私有的构造函数的类则需要继承Factory
		 *    类完成。
		 * 
		 * @example:
		 *      class a{
		 *           public:
		 *                a();
		 *                a( int c );
		 *      };
		 *
		 *      a * pa = factory< a >( 23 );
		 *      a * pa2 = Factory< a >( []( Factory::emErrCode err){
		 *                      // 错误处理
		 *                } , 23 );
		 */
		template< typename type , typename ...Args ,
			  typename midType = typename std::decay< type >::type,
			  typename realType = std::conditional<
				  std::is_pointer< midType >::value ,
				  typename std::remove_pointer<midType>::type,
				  midType
				  >
			  >
		realType * factory( std::functional< void (Factory::emErrCode) > errfun , Args... args ){
			static_assert( std::is_class<realType>::value  , "this function must be used with class" );
			static_assert( std::is_constructible<realType>::value , "class must constructible." );
			
			realType * ret = nullptr;
			try{
				realType * ret = new reakType( (0,args)... );
			}catch( std::bad_alloc& e ){
				std::cout << e.what() << std::endl;
				if( errfun ){
					errfun( Factory::ERR_ALLOC_MEM );
				}
			}
			
			return ret;
		}
		
		/**
		 * @brief 模板工厂函数
		 * @tparam type 类名称		 
		 * @tparam Args... ， 调用构造函数的时候构造函数的参数类型表
		 * @param args [ I ], 构造函数的参数表
		 * @param errfun[ I ],发生错误的回调函数
		 */
		template< typename type , typename ...Args ,
			  typename midType = typename std::decay< type >::type,
			  typename realType = std::conditional<
				  std::is_pointer< midType >::value ,
				  typename std::remove_pointer<midType>::type,
				  midType
				  >
			  >
		realType * factory( Args... args ){
			static_assert( std::is_class<realType>::value  , "this function must be used with class" );
			static_assert( std::is_constructible<realType>::value , "class must constructible." );
			
			realType * ret = nullptr;
			try{
				realType * ret = new realType( (0,args)... );
			}catch( std::bad_alloc& e ){
				std::cout << e.what() << std::endl;
			}
			
			return ret;
		}
	}	       
}
