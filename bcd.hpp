/**
 * @brief BCD编解码模块。这个模块的BCD编码是8421码
 * @version 1.0
 * @author 宋炜
 * @date 2022-2-21
 */

#pragma once

#include <type_traits>
#include <typeinfo>
#include <vector>
#include <memory>
#include <sstream>

#include "misc.hpp"

#include "typetraits.hpp"

class BCD
{
public:
	enum emErrCode{
		ERR_ALLOC_MEM = -1000,
		OK = 0
	};
private:
	std::vector< uint8_t >     __m_data;
	bool                       __m_packed;
public:
	BCD():__m_packed( true ){}
	/**
	 * @brief 直接读取BCD码
	 * @param data[ I ],
	 * @param packed[ I ], 是否是压缩编码。true压缩编码，false非压缩编码
	 */
	BCD( const uint8_t * data , size_t len , bool packed = true ): __m_packed( packed )
	{
		for( size_t i = 0; i < len; i ++ ){
			__m_data.push_back( data[ i ] );
		}
	}
	/**
	 * @brief 从整数转换成压缩的8421 BCD码
	 * @tparam realT
	 * @param value[ I ]
	 */
	template< typename T, typename realT = typename std::conditional<  std::is_integral<T>::value && !is_bool< T >::value , T , void >::type >
	BCD( const realT& value ):__m_packed( true ){
		fromUInt(value);
	}
	/**
	 * @brief 指定BCD码数据
	 * @param data[ I ] ， 
	 * @param len[ I ]
	 * @param packed[ I ]
	*/
	void set( uint8_t * data , size_t len , bool packed = true)
	{
		__m_data.resize( len );
		memcpy( __m_data.data() , data , len );
	}
	/**
	 * @brief 从字符串转换到BCD码，这个字符串可以是常见的进制的内容，主要包括了8, 10, 16
	 * @param str[ I ], 要转换的字符串
	 * @param base[ I ], 进制类型
	 */
	void fromString( const std::string& str , int base = 10 )
	{
		uint64_t data;
		std::stringstream ss;
		
		if( str.empty() ){ return; }
		switch( base ){
		case 8:
			ss << "o" << str;
			ss >> std::oct >> data;
			break;
		case 10:
			ss << str;
			ss >> std::dec >> data;
			break;
		case 16:
			ss << "0x" << str;
			ss >> std::hex >> data;
			break;
		default:
			ERROR_MSG( "暂时不支持该进制模式" );
		}

		fromUInt<decltype(data)>( data );
	}
	
	/**
	 * @brief 从整数转换成BCD内容
	 */
	template< typename T , typename realT =
		  typename std::conditional< std::is_integral<T>::value && !is_bool<T>::value, T , void >::type >
	void fromUInt( const realT& data )
	{
		static_assert( !std::is_void<realT>::value , "不支持bool类型数据的BCD编码" );
		
		size_t bytes = sizeof( realT );
		uint8_t * p = (uint8_t *)&data ;
		
		uint8_t inc = 0;
		std::vector<uint8_t> tmp;

		for( size_t i = 1; i <= bytes; i ++ ){
			uint8_t d = p[ bytes - i ] && 0x0f;
			d += inc;
			if( d > 10 ){
				d -= 10;
				inc = 1;
			}

			uint8_t d2 = ( p[ bytes - i ] & 0x0f ) >> 4;
			d2 += inc;
			
			if( d2 > 10 ){
				d2 -= 10;
				inc = 1;
			}

			d = ( d2 << 4 ) + d;
			tmp.push_back( d );
		}

		for( auto it = tmp.rbegin(); it != tmp.rend(); it ++ ){
			__m_data.push_back( *it );
		}

		__m_packed = true;
	}
	/**
	 * @brief 将BCD码转换成字符串。
	 * @return 返回10进制数字描述的字符串
	 */
	std::string toString(){
		std::string ret;

		return ret;
	}
	
	
	/**
	 * @brief 从BCD码转换成无符号整数
	 * @tparam T 
	 * @param rst[ O ]
	 * @return 操作成功返回true，否则返回false
	 */
	template< typename T , typename realT = typename std::conditional< std::is_integral<T>::value && std::is_unsigned<T>::value, T , void >::type >
	realT toUInt( realT * data = nullptr )
	{
		static_assert( std::is_same<realT , void >::value != true , "error data type" );
		
		realT ret = (realT)0;
		
		size_t len = sizeof( realT );
		
		for( size_t i = 0; i < len && i < __m_data.size(); i ++ ){
			uint8_t d = 0 , dh;
			ret *= 10;
			d = __m_data[ i ];
			dh = d & 0xf0;
			ret += (dh >> 4 );
			ret *= 10;
			
			d = d & 0xf;
			ret += d;
		}
		return ret;
	}
	/**
	 * @brief 将bcd码转换成有符号的整数
	*/
	template< typename T , 
		typename realT = typename std::conditional< std::is_integral<T>::value ,typename std::make_signed<T>::type, void >::type
	>
	realT toInt( realT * data = nullptr )
	{
		static_assert( std::is_same<realT , void >::value != true , "error data type" );
		realT ret = (realT)0;
		int r =0 ;
		size_t len = sizeof( realT );
		if( ( __m_data[ 0 ] >> 4 ) >= 10 ){
			r = 1;
		}
		for( size_t i = r; i < len && i < __m_data.size(); i ++ ){
			uint8_t d = 0 , dh;
			ret *= 10;
			d = __m_data[ i ];
			dh = d & 0xf0;
			ret += (dh >> 4 );
			ret *= 10;
			
			d = d & 0xf;
			ret += d;
		}
		
		if( r == 1 ){
			ret = -ret;
		}
		
		return ret;
	}
	/**
	 * @brief 获取BCD数据内容。从内容中进行深拷贝并分配新的内存，使用std::shared_ptr管理，使用后可以自动释放内存
	 */
	std::shared_ptr<uint8_t> get()
	{
		try{
			std::shared_ptr<uint8_t> ret( new uint8_t[ __m_data.size() ] , [](const uint8_t * d ){
					delete []d;
				});

			memcpy( ret.get() , __m_data.data() , __m_data.size() );

			return ret;
		}catch( std::bad_alloc& e ){
			ERROR_MSG( e.what() );
		}
		
		return {};
	}
     
private:
	/**
	 * @brief 判断是否是BCD码
	 * @param data[ I ] 要检查的数据内容
	 * @param len[ I ] 数据的长度，按字节
	 * @param packed[ I ] 是否是压缩方式的BCD
	 */
	bool __is_bcd( const uint8_t * data , size_t len , bool packed = true ){
		bool ret = true;
		if( data == nullptr || len == 0 ) return false;
		
		if( packed == false ){
			for( size_t i = 0; i < len; i ++ ){
				ret = ret && ( data[ i ] < 10 );
			}
		}else{
			for( size_t i = 0; i < len; i ++ ){
				uint8_t d1 = data[ i ] & 0x0f,
					d2 = data[ i ] >> 4 & 0xf;

				ret = ret && ( d1 < 10 ) && ( d2 < 10 );
				if( !ret ) break;
			}
		}
		return ret;
	}
};
