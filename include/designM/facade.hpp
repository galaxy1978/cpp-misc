/**
 * @brief 外观模式
 * @version 1.0
 * @author 宋炜
 * @date 2023-7-6
*/

#pragma once

namespace wheels{ namespace dm {
template <typename retType, typename... subTypes>
struct facadeInh{};

template <typename retType, typename subType1, typename... subTypes>
struct facadeInh<retType, subType1, subTypes...> : public subType1, public facadeInh<retType, subTypes...>
{
};

template <typename retType>
struct facadeInh<retType>
{
	template <typename... Args>
    retType run(std::function<retType(facadeInh<retType> * )> func)
    {
        return func(this);
    }
};

// 下面是方式2, 以接口的方式实现
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#define FACADE_START_DECLARE_SUB_ITFC( name ) \
struct name{ \
	virtual ~name(){}

#define FACADE_END_DECLARE_SUB_ITFC  };

#define FACADE_ADD_ITFC( RET , NAME , ... ) \
virtual RET NAME( __VA_ARGS__ ) = 0;
	

template< typename itfcType ,  typename... subTypes >
class facadeItfc
{
public:
	template< int N , typename tupleParams >
	struct INIT_HELPER__
	{
		static void init__( std::vector< itfcType * >& vec , tupleParams& t ){
			vec[ N - 1 ] = std::get< N - 1 >( t );
			
			INIT_HELPER__< N - 1 , tupleParams >::init__( vec , t );
		}
	};
	
	template< typename tupleParams >
	struct INIT_HELPER__<0 , tupleParams >{
		static void init__( std::vector< itfcType * >& vec , tupleParams& t ){}
	};
protected:
    std::vector< itfcType * >  m_subs__;
public:
    facadeItfc(subTypes*... subs):m_subs__( sizeof...(subTypes) ) {
		std::tuple< subTypes*... > t = std::make_tuple( subs... );
		INIT_HELPER__< sizeof...( subTypes ) ,decltype(t) >::init__( m_subs__ , t );
	}

    void run(std::function< void ( itfcType * )> fun) {
		for( int i = 0; i < sizeof...( subTypes ); i ++ ){
			if( m_subs__[ i ] != nullptr ){
				fun( m_subs__[ i ] );
			}
		}
    }
};
}}