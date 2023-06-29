/**
 * @brief 构造一个可以自动释放的内存，并分配内存的指针
 * @version 1.0
 * @author 宋炜
 * @date 2021-12-21
 */

#pragma once

#include <stdlib.h> 

#include <memory>
#include <type_traits>

namespace wheels {
template< typename T , typename realT = typename std::conditional< std::is_fundamental< T >::value , T , void >::type >
class mallocSharedPtr : public std::shared_ptr< realT >
{
private:
    size_t      m_data_len__;
    size_t      m_buf_len__;
public:
	mallocSharedPtr():
        std::shared_ptr< realT >( nullptr , []( realT * data ){ if( data != nullptr ){ free( data );}}),
        m_data_len__( 0 ),
        m_buf_len__(0)
	{}
	mallocSharedPtr( size_t count ):
        std::shared_ptr< realT >( nullptr , []( realT * data ){ if( data != nullptr ){ free( data );}}),
        m_data_len__( 0 ),
        m_buf_len__( count )
	{
		realT * d = (realT*)malloc( count * sizeof( realT ));
		if( d == nullptr ){
			std::bad_alloc e;
			throw e;
		}

		this->reset( d );
	}
    mallocSharedPtr( realT * data ):std::shared_ptr< realT >( data , []( realT * data ){ if( data != nullptr ){ free( data );}}),
        m_data_len__( 0 ){}

    mallocSharedPtr( const mallocSharedPtr<realT>&b ): std::shared_ptr< realT >( b ),m_data_len__( b.m_data_len__ ),m_buf_len__(b.m_buf_len__){}

    mallocSharedPtr( mallocSharedPtr<realT>&&b ): std::shared_ptr< realT >( b ),m_data_len__( b.m_data_len__ ),m_buf_len__(b.m_buf_len__){}
	
	realT& operator[]( size_t pos ){
		if( pos > m_buf_len__ ){
			throw std::runtime_error( "position range overflow" );
		}
		realT * p = get();
		if( p ){
			return *( p + pos );
		}else{
			throw std::runtime_error( "null data pointer" );
		}
	}
    /**
     * @brief 指定缓冲区大小
     * @param len
     */
    void size( size_t len ){ m_buf_len__ = len; }
    size_t size(){ return m_buf_len__; }
    /**
     * @brief 指定当前的数据长度
     * @param len
     */
    void dataLen( size_t len ){ m_data_len__ = len; }
    /**
     * @brief 读取当前的数据长度
     * @return 返回当前的数据长度
     */
    size_t dataLen( ){ return m_data_len__; }
	/**
	 * @brief 自动分配内存，并且拷贝传入的指针内容
	 * @param data[ I ]， 传入的数据
	 * @param len[ I ]， 数据个数
	 */
	mallocSharedPtr( const realT* data , size_t len ):
        std::shared_ptr< realT >( nullptr , []( realT * data ){ if( data != nullptr ){ free( data );}}),m_data_len__( len )
	{
		realT * d = ( realT*)malloc( len * sizeof( realT ));
		if( d == nullptr ){
			std::bad_alloc e;
			throw e;
		}
		memcpy( d , data , len * sizeof( realT ));
		this->reset( d );
	}
	/** 
	 * @brief 
	*/
	void resize( size_t s )
	{
		realT * d = (realT*)malloc( s * sizeof( realT ));

		if( d == nullptr ){
			std::bad_alloc e;
			throw e;
		}

		this->reset( d );
	}				  	

};
}

