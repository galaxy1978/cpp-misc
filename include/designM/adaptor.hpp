/**
 * @brief 适配器模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-12
 *  2023-10-5  增加了可变参数的支持
 */

#pragma once

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
	static void call( adapterType * adpter , tplType& t, Params&& ...args ){
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
class Adapter {
public:
    Adapter(const T&... adap) : m_adaptees__(adap...) {}
	
	template< typename ...Params >
	void set( functor fun ){ m_callback__ = fun; }
	
	template< typename ...Params >
    void request( Params&& ...params ) {
        CALLER_HELPER__< sizeof...(T) , Adapter<functor , itfcType , T...> , std::tuple<T...> >::
		   call( this , m_adaptees__ ,  params... );
    }
	
	functor  m_callback__;
protected:
    std::tuple<T...>    m_adaptees__;
	
	TYPE_CHK_HELPER__<itfcType , T...>   m_chk_helper__;
};
}}
