/**
 * @brief 单例模式
 * @version 1.2
 * @author 宋炜
 * @date 2023-03-08~2023-6-21

 */

#pragma once

#include <memory>
#include <type_traits>
#include "misc.hpp"

namespace wheels
{
	namespace dm
	{
		template<
			typename typeName ,
			typename T = typename std::enable_if< std::is_class< typeName >::value , typeName >::type
			>
		class singleton
		{
		protected:
			static std::unique_ptr< T > pt_obj__;
		protected:
			/// 避免直接调用构造函数
			singleton(){}
		public:
			singleton( const singleton& ) = delete;
			singleton( singleton&& ) = delete;
			/**
			 * @brief 工厂函数，用来创建单例对象
			 * @return c
			 */
			template< typename ...Args >
			static T * create( Args... args ){
				if( pt_obj__ ) return pt_obj__.get();

				try {
					pt_obj__.reset( new T((0,args)...) );
				} catch ( std::bad_alloc& e ) {
					ERROR_MSG( e.what() );
				}
				T * ret = pt_obj__.get();
				return ret;
			}
			/**
			 * @brief 获取当前的模块对象指针
			 * @return 返回指针，如果模块没有初始化返回时nullptr
			 */
			static T * get(){
				return pt_obj__.get();
			}
		};

#define IMP_SINGLETON( type )   template<> std::unique_ptr< type > singleton< type > :: pt_obj__ = {}
	}
}

