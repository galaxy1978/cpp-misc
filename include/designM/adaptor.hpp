/**
 * @brief 适配器模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-12
 *  2023-10-5  增加了可变参数的支持
 */

#pragma once

#include <tuple>
#include <functional>

namespace wheels
{namespace dm{

template< int N , typename adapterType , typename tplType >
struct CALLER_HELPER__{

    template< typename ...Params >
    static void call( adapterType * adpter , tplType& t , Params&& ...args ){
        if( adpter && adpter->m_callback__ ){
            adpter->m_callback__( &std::get< N - 1 >( t ) , std::forward<Params>(args)...);
            CALLER_HELPER__< N - 1 , adapterType , tplType>::call( adpter , t , std::forward<Params>(args)... );
        }
    }
};

template< typename adapterType , typename tplType  >
struct CALLER_HELPER__< 0 ,  adapterType , tplType>{

    template< typename ...Params >
    static void call( adapterType * adpter , tplType& t, Params&& ... ){
        (void)adpter;
        (void)t;
    }
};

template < typename itfcType , typename... implType >
struct TYPE_CHK_HELPER__{};

template< typename itfcType , typename implType1 , typename... implType >
struct TYPE_CHK_HELPER__< itfcType , implType1 , implType...> //: public TYPE_CHK_HELPER__< itfcType , implType... >
{
    static_assert( std::is_base_of< itfcType , implType1 >::value , "" );
};

template< typename itfcType >
struct TYPE_CHK_HELPER__<itfcType> {};


// 适配器类
template <typename functor , typename itfcType , typename ...T>
class adapter {
public:
    adapter(const T&... adap) : m_adaptees__(adap...) {}
    virtual ~adapter(){}
    /**
     * @brief 设置回调函数。在回调函数中进行适配操作
     * @param fun[ I ]，回调函数对象
     */
    void set( functor fun ){ m_callback__ = fun; }
    /**
     * @brief 执行请求操作
     * @tparam Params 参数类型表
     * @param params[ IO ]， 参数表
     */
    template< typename ...Params >
    void request( Params&& ...params ) {
        CALLER_HELPER__< sizeof...(T) , adapter<functor , itfcType , T...> , std::tuple<T...> >::
            call( this , m_adaptees__ ,  std::forward<Params>(params)... );
    }
public:
    functor  m_callback__;
protected:
    std::tuple<T...>    m_adaptees__;      // 被适配的对象
    // 检查每一个被适配对象和接口是否相符合
    TYPE_CHK_HELPER__<itfcType , T...>   m_chk_helper__[0];
};
}}
