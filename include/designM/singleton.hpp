/**
 * @brief 单例模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-03-08
 */

#pragma once

#include <memory>
#include <type_traits>
#include "misc.hpp"

namespace wheels
{
namespace dm
{
    template< typename T , bool is_ok=std::is_class<T>::value >
    class singleton
    {
    protected:
        static std::unique_ptr< T > __pt_obj;
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
        static T * create( )
        {
            static_assert( is_ok , "必须使用class或者struct定义，并且包含默认的构造函数" );
            if( __pt_obj ) return __pt_obj.get();

            try {
                __pt_obj.reset( new T );
            } catch ( std::bad_alloc& e ) {
                ERROR_MSG( e.what() );
            }
            T * ret = __pt_obj.get();
            return ret;
        }
        /**
         * @brief 获取当前的模块对象指针
         * @return 返回指针，如果模块没有初始化返回时nullptr
         */
        static T * get()
        {
            return __pt_obj.get();
        }
    };

#define IMP_SINGLETON( type )   template<> std::unique_ptr< type > singleton< type > :: __pt_obj = {}
}
}