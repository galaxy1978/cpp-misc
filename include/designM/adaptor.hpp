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
     * @brief set
     * @param fun
     */
    template< typename ...Params >
    void set( std::function<  void (Params&&...) > fun ){
        static_assert( std::is_same< std::function<  void (Params&&...) > , functor >::value , "" );
        m_callback__ = fun;
    }

    template< typename ...Params >
    void request( Params&& ...params ) {
        CALLER_HELPER__< sizeof...(T) , adapter<functor , itfcType , T...> , std::tuple<T...> >::
            call( this , m_adaptees__ ,  std::forward<Params>(params)... );
    }
public:
    functor  m_callback__;
protected:
    std::tuple<T...>    m_adaptees__;
    // 检查每一个被适配对象和接口是否相符合
    TYPE_CHK_HELPER__<itfcType , T...>   m_chk_helper__[0];
};
}}