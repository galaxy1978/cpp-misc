/**
 * @brief 缓冲区模块。这个模块的定义可以用来存储原始二进制数据，可以用来存储在这个容器内的数据数据只能是C/C++
 *    的原生数据。
 * @version 1.0
 * @date 2018-10-8
 * @author 宋炜
 *
 * @note 因为在内存分配的时候使用了malloc，realloc 等C内存管理函数，并没有使用标准的alloc。特别注意不要在使用
 *    这个容器保存结构化的数据。
 */

#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <string>
#include <memory>
#include <atomic>
#include <string.h>
#include "typetraits.hpp" 
template< typename Type >
class Buffer:public type_traits< Type >
{
public:
	/*typedef typename ltrpTypeTraits::value_type	value_type;
	typedef typename ltrpTypeTraits::pointer	pointer;
	typedef typename ltrpTypeTraits::refrence	refrence;
	*/
	enum err_code{
		ERR_BASE = -1000,
		ERR_ALLOC_MEM,
		ERR_DATA_EMPTY,
		ERR_DEST_NULL,
		ERR_SRC_NULL,
		OK = 0
	};
private:
	std::atomic< size_t >        m_len;             // 存储长度
	std::atomic< size_t >        m_size;            // 数据长度, 是数据元素的长度
	std::shared_ptr< Type >      mp_data;           // 数据指针
private:
	void deleter( Type * p )
	{
		if( p ) free( p );
	}
public:
	/**
	 * @brief 默认的初始化函数
	 */
	Buffer(): m_len( 0 ) , mp_data( nullptr , [this]( Type * pt ){ if( pt ) free( pt ); }) { }
	/**
	 * @brief 按照给定的长度初始化内存。
	 * @param len , 最大存储长度
	 * @exception 如果内存分配失败则抛出ERR_ALLOC_MEM
	 */
	Buffer( size_t len ): m_len( len ), mp_data( nullptr , [this]( Type * pt ){ if( pt ) free( pt ); })
	{
		Type * data = ( Type *)malloc( sizeof( Type ) * len );
		if( data ){
			mp_data.reset( data );
		}else{
			throw ERR_ALLOC_MEM;
		}
	}
	/**
	 * @brief 按照给定的长度和数据初始化对象。按照长度分配内存，并将数据拷贝到分配好的内存中
	 * @param data
	 * @param len
	 */
	Buffer( Type * src , size_t len ):
	m_len( len ) ,
	mp_data( nullptr , [this]( Type * pt ){ if( pt ) free( pt ); } )
	{
		Type * data = ( Type *)malloc( sizeof( Type  ) * len );
		if( data ){
		if( src != nullptr )
			memcpy( data , src , len );
		else{
			mp_data.reset( src );
			throw ERR_SRC_NULL;
		}
		mp_data.reset( data );

		}else{
		throw ERR_ALLOC_MEM;
		}
	}
	/**
	 * @brief
	 * @param b
	 */
	Buffer( const Buffer<Type>& b ): mp_data( nullptr , [this]( Type * pt ){ if( pt ) free( pt ); } )
	{
		m_size = b.m_size.load();
		m_len = b.m_len.load();
		mp_data = b.mp_data;
	}
	/**
	 * @brief
	 */
	Buffer( Buffer<Type> && b ) : mp_data( nullptr , [this]( Type * pt ){ if( pt ) free( pt ); } )
	{
		m_size = b.m_size.load();
		m_len = b.m_len.load();
		mp_data = std::move( b.mp_data );
	}
	/**
	 * @brief 赋值重载
	 */
	Buffer<Type> & operator=( const Buffer<Type>& b )
	{
		m_size = b.m_size;
		m_len = b.m_len;
		mp_data = b.mp_data;

		return *this;
	}
	/**
	 * @brief
	 */
	Buffer<Type> & operator=( Buffer<Type>&& b )
	{
		m_size = b.m_size;
		m_len = b.m_len;
		mp_data = std::move( b.mp_data );

		return *this;
	}
	/**
	 * @brief 在结尾添加一条数据
	 * @param data ， 要添加的数据
	 * @return 返回当前对象
	 * @exception 内存分配失败后抛出ERR_EMPTY_DATA
	 */
	Buffer<Type> & push_back( Type data )
	{
		Type * pt = mp_data.get();
		if( m_len > m_size ){
		pt[ m_size.load() ] = data;
		m_size ++;
		}else{
		pt = ( Type * )malloc( m_len * 2 );
		if( pt ){
			m_len = m_len.load() * 2;
			pt[ m_size ] = data;
			m_size ++;
		}else{
			throw ERR_ALLOC_MEM;
		}
		}
		return *this;
	}
	/**
	 * @brief 删除首元素，并将改元素传回的给定的参数。
	 * @param data , 接受首元素的值
	 */
	Buffer<Type> & pop_front( Type &data )
	{
		if( !mp_data ) throw ERR_DATA_EMPTY;
		if( m_size == 0 ) throw ERR_DATA_EMPTY;

		Type *pt = mp_data.get() , *pt2 = nullptr;
		if( pt == nullptr ){
		data.clear();
		throw ERR_DATA_EMPTY;
		}
		data = pt[ 0 ];
		pt ++;
		m_size --;
		pt2 = ( Type *)malloc( m_len );
		if( pt2 ){
		memcpy( pt2 , pt , m_size.load() * sizeof( Type ) );
		}else{
		data.clear();
		throw ERR_ALLOC_MEM;
		}

		mp_data.reset( pt2 );
		return *this;
	}
	/**
	 * @brief 删除所有的元素。
	 * @note 这个函数只是将所有长度标记清零，并不修改实际数
	 */
	void clear()
	{
		m_size = 0;
	}
	/**
	 * @brief 使用给定的数据覆盖所有的数据，并将数据长度标记为0
	 */
	void erase( Type d = 0)
	{
		Type *pt = mp_data.get();
		memset( pt , d , m_size * sizeof( d ) );
		m_size = 0;
	}
	/**
	 * @brief 读取内容长度
	 */
	size_t size()
	{
		return m_size.load();
	}
	/**
	 * @brief 读取内容长度
	 */
	size_t length()
	{
		return m_len.load();
	}

	/**
	 * @brief 读取数据指针
	 */
	Type * data()
	{
		return mp_data.get();
	}
	/**
	 * @brief 指定数据.如果原来的内存不足，则会自动分配内存，分配成功后将数据拷贝到内存中。
	 * @param data
	 * @param len
	 * @return
	 * @exception ERR_ALLOC_MEM , ERR_SRC_NULL
	 */
	Buffer< Type >& data( const Type * data , size_t len )
	{
		if( data == nullptr || len == 0 ){
		throw ERR_SRC_NULL;
		}

		Type * pt = mp_data.get();
		if( m_len < len ){

			if( pt )
				pt = ( Type* )realloc( pt , sizeof( Type ) * len );
			else
				pt = ( Type *)malloc( sizeof( Type ) * len );

			if( pt ){
				m_len = len;
			}else{
				throw ERR_ALLOC_MEM;
			}
		}

		if( pt ){

			memcpy( pt , data , len );
			m_size = len;
			mp_data.reset( pt );
		}
		else{
			throw ERR_DEST_NULL;
		}

		return *this;
	}
	/**
	 * @brief 在数据后面添加新的数据
	 * @param data
	 * @param len
	 * @return
	 * @exception ERR_ALLOC_MEM ERR_SRC_NULL
	 */
	Buffer< Type >& append( Type * data , size_t len )
	{
		if( data == nullptr ) throw ERR_SRC_NULL;
		if( len == 0 ) throw ERR_SRC_NULL;

		Type * pt = mp_data.get();
		if( m_len < m_size + len ){
			if( mp_data ){
				pt = ( Type *)realloc( pt , sizeof( Type ) * m_size + len );
			}else{
				pt = (Type *)malloc( sizeof( Type ) * (m_size + len) );
			}

			if( pt == nullptr ){ throw ERR_ALLOC_MEM; }

			m_len = m_size + len;

		}

		if( pt ){
			memcpy( pt + m_size , data , len );
			m_size += len;
			mp_data.reset( pt );
		}

		return *this;
	}
	/**
	 * @brief 在原来的数据中间添加新的数据
	 * @param pos 要插入数据的位置
	 * @param data 要插入的数据
	 * @param len 数据长度
	 * @return
	 * @exception 如果源数据指针空或者长度为0，抛出ERR_SRC_NULL , 如果内存分配失败，抛出ERR_ALLOC_MEM
	 */
	Buffer<Type> &insert( size_t pos , Type * data , size_t len )
	{
		if( data == nullptr ) throw ERR_SRC_NULL;
		if( len == 0 ) throw ERR_SRC_NULL;

		Type * pt = mp_data.get();

		if( m_len < m_size + len ){                     // 需要重新分配内存
			if( mp_data ){
				pt = ( Type *)realloc( pt , sizeof( Type ) * m_size + len );
			}else{
				pt = (Type *)malloc( sizeof( Type ) * (m_size + len) );
			}

			if( pt == nullptr ){ throw ERR_ALLOC_MEM; }

			m_len = m_size + len;
		}

		if( pt ){
			memcpy( pt + pos , pt + pos + len , len );   // 将原来的数据向后移动
			memcpy( pt + pos , data , len );
			m_size += len;
			mp_data.reset( pt );
		}
		return *this;
	}
	/**
	 * @brief 构造拷贝一份新的数据并返回数据指针
	 * @return 成功操作返回数据指针，否则返回空指针
	 * @exception 内存分配失败则抛出ERR_ALLOC_MEM；如果源数据不存在则抛出ERR_SRC_NULL
	 * @note 拷贝的数据必须在外部进行释放。释放应该使用free进行操作。
	 */
	Type * clone( )
	{
		Type  * ret = nullptr;

		ret = ( Type *)malloc( sizeof( Type ) * m_size.load() );
		if( ret ){
		if( mp_data )
			memcpy( ret , mp_data.get() , sizeof( Type ) * m_size.load() );
		else
			throw ERR_SRC_NULL;
		}else{
			throw ERR_ALLOC_MEM;
		}
		return ret;
	}
	/**
	 * @brief 将数据切片，返回切片的内容。
	 * @param from
	 * @param len
	 */
	Type * slice( size_t from , size_t len )
	{
		Type   * ret = nullptr;

		return ret;
	}
};

typedef Buffer< uint8_t >  BufferByte;
/*
typedef Buffer< uint16_t > BufferWord;
typedef Buffer< uint32_t > BufferDWord;
*/
#endif
