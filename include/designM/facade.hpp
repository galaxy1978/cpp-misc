/**
 * @brief 外观模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-6
*/

#pragma once

#include <type_traits>
#include <functional>

#if !defined( FACADE_USE_INHERIT )
#	define FACADE_USE_INHERIT  (0)
#endif

#if FACADE_USE_INHERIT == 1
namespace wheels{namespace dm{
	template <typename retType, typename... subTypes>
	struct facade{};

	template <typename retType, typename subType1, typename... subTypes>
	struct facade<retType, subType1, subTypes...> : public subType1, public facadeInh<retType, subTypes...>
	{
	};

	template <typename retType>
	struct facade<retType>
	{
		template <typename Func_t , typename... Args>
		retType run( Func_t func , Args&&... args )
		{
			return func(this , std::forward<Args>(args)...);
		}
	};
}}
#else 
namespace facade_private__ {
    template < typename itfcType , typename... implType >
    struct TYPE_CHK_HELPER__{};

    template< typename itfcType , typename implType1 , typename... implType >
    struct TYPE_CHK_HELPER__< itfcType , implType1 , implType...>
    {
        static_assert( std::is_base_of< itfcType , implType1 >::value , "" );
    };

    template< typename itfcType >
    struct TYPE_CHK_HELPER__<itfcType> {};
}

namespace wheels{ namespace dm {

#define FACADE_START_DECLARE_SUB_ITFC( name ) \
struct name{ \
	virtual ~name(){}

#define FACADE_END_DECLARE_SUB_ITFC  };

#define FACADE_ADD_ITFC( RET , NAME , ... ) \
virtual RET NAME( __VA_ARGS__ ) = 0;


template< typename itfcType ,  typename... subTypes >
class facade
{
public:
    using itfc_t = typename std::remove_pointer< typename std::decay< itfcType >::type >::type;
    using iterator = typename std::vector< itfc_t * >::iterator;
public:

	template< int N , typename tupleParams >
	struct INIT_HELPER__
	{
        static void init__( std::vector< itfc_t * >& vec , tupleParams& t ){
			vec[ N - 1 ] = std::get< N - 1 >( t );
			
			INIT_HELPER__< N - 1 , tupleParams >::init__( vec , t );
		}
	};
	
	template< typename tupleParams >
	struct INIT_HELPER__<0 , tupleParams >{
        static void init__( std::vector< itfc_t * >&  , tupleParams& ){}
	};
protected:
    std::vector< itfc_t * >  m_subs__;

    facade_private__::TYPE_CHK_HELPER__<itfc_t , subTypes... >   m_chker__[0];
public:
    facade(){}

    facade( std::vector< itfc_t * > subs ):m_subs__( subs ) {}

    facade(subTypes*... subs):m_subs__( sizeof...(subTypes) ) {
		std::tuple< subTypes*... > t = std::make_tuple( subs... );
		INIT_HELPER__< sizeof...( subTypes ) ,decltype(t) >::init__( m_subs__ , t );
	}

    virtual ~facade(){}

    template< typename subType >
    void add( subType * obj ){
        facade_private__::TYPE_CHK_HELPER__<itfc_t , subType >   m_chker__[0];
        m_subs__.push_back( obj );
    }

    void erase( iterator it ){
        m_subs__.erase( it );
    }

    void erase( iterator b , iterator e ){
        m_subs__.erase( b , e );
    }

    template< typename Func_t , typename ...Params >
    void run( Func_t fun , Params&&... args) {
		for( int i = 0; i < sizeof...( subTypes ); i ++ ){
			if( m_subs__[ i ] != nullptr ){
                fun( m_subs__[ i ] , std::forward<Params>(args )...);
			}
		}
    }
};
}}
#endif