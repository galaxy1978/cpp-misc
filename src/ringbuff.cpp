#if !defined( __GNUC__ )
#    error "[ ERROR ]    This module must complied by GCC"
#endif

#include <assert.h>
#include <string.h>
#include <memory.h>
#include <mutex>

#include "ringbuff.hpp"
using namespace wheels;
rb :: rb( size_t size , bool autoZero )
	: __p_data( NULL ),
	  __m_maxSize( size ),
	  __m_curr_len( 0 ),
	  __m_curr_add_point( 0 ),
	  __m_curr_get_point( 0 )
{
	__p_data = malloc( size );
	if( NULL == __p_data ){
		throw ERR_ALLOC_MEM;
	}

	if( autoZero ){
		memset( __p_data , 0 , size );
	}
}

rb :: rb( const rb& b )
	: __p_data( NULL ),
	  __m_maxSize( b.__m_maxSize ),
	  __m_curr_len( 0 ),
	  __m_curr_add_point( 0 ),
	  __m_curr_get_point( 0 )
{
	rb * p_b = ( rb*)&b;

	__p_data = malloc( p_b->__m_maxSize );
	if( NULL == __p_data ){
		throw ERR_ALLOC_MEM;
	}

	__m_maxSize        = p_b->__m_maxSize;
	memcpy( __p_data , p_b->__p_data , __m_maxSize );

	__m_curr_len       = p_b->__m_curr_len;
	__m_curr_add_point = p_b->__m_curr_add_point;
	__m_curr_get_point = p_b->__m_curr_get_point;

}

rb :: ~rb()
{
	std::lock_guard<std::mutex> l( __m_mutex );
	if( __p_data ) free( __p_data );
}

rb& rb :: operator=( const rb& b )
{
	rb * p_b = ( rb*)&b;
	std::lock_guard<std::mutex> l( __m_mutex );

	__p_data = malloc( p_b->__m_maxSize );
	if( NULL == __p_data ){
		throw ERR_ALLOC_MEM;
	}

	__m_maxSize = p_b->__m_maxSize;
	memcpy( __p_data , p_b->__p_data , __m_maxSize );

	__m_curr_len       = p_b->__m_curr_len;
	__m_curr_add_point = p_b->__m_curr_add_point;
	__m_curr_get_point = p_b->__m_curr_get_point;


	return *this;
}

rb :: emErrCode
rb :: append( const void * __data , size_t size )
{
	emErrCode ret = OK;

	if( NULL == __data ){return ERR_SRC_NULL;}
	if( NULL == __p_data ) return ERR_DATA_NULL;

	char * data = ( char *)__data;
	char * p_data = ( char *)__p_data;

	std::lock_guard<std::mutex> l( __m_mutex );
	/// @note 这里的操作可能会覆盖没有取出的数据
	if( __m_curr_add_point + size <= __m_maxSize ){
		memcpy( p_data + __m_curr_add_point , data , size );

		__m_curr_len += size;
		__m_curr_add_point += size;
	}else{
		// 计算后段位置
		size_t bk_pos = __m_maxSize - __m_curr_add_point;
		// 拷贝前段
		memcpy( p_data + __m_curr_add_point , data , bk_pos );
		// 拷贝后段落.
		memcpy( p_data , data + bk_pos , size - bk_pos );

		__m_curr_add_point = size - bk_pos;
		__m_curr_len += size;

	}

	if( __m_curr_len > __m_maxSize ) __m_curr_len = __m_maxSize;

	return ret;
}

rb :: emErrCode
rb :: get( void * __data , size_t size )
{
	emErrCode ret = OK;
	if( __data == NULL ) return ERR_DST_NULL;
	char * data = ( char *)__data;
	char * p_data = ( char *)__p_data;
	std::lock_guard<std::mutex> l( __m_mutex );
	if( size <= __m_curr_len ){
		if( __m_curr_get_point + size < __m_maxSize ){
			memcpy( data , p_data + __m_curr_get_point , size );

			__m_curr_get_point += size;
			__m_curr_len -= size;
		}else{
			size_t sec_ph = __m_maxSize - __m_curr_get_point;

			memcpy( data , p_data + __m_curr_get_point , sec_ph );
			memcpy( data + sec_ph , __p_data , size - sec_ph );

			__m_curr_get_point = size - sec_ph;
			__m_curr_len -= size;
		}
	}else{
		return ERR_LESS_DATA;
	}

	return ret;
}

rb::emErrCode
rb :: get( void * __data , size_t pos , size_t size )
{
	emErrCode ret = OK;
	if( __data == NULL ) return ERR_DST_NULL;
	char * data = ( char *)__data;
	char * p_data = ( char *)__p_data;
	std::lock_guard<std::mutex> l( __m_mutex );
	size_t data_end = __m_curr_get_point + __m_curr_len;
	if( data_end >= __m_maxSize ){
		data_end = __m_maxSize;
	}

	if( size + pos < __m_curr_len ){ // 数据没有跨过存储位置的尾部
		if( __m_curr_get_point + size + pos < __m_maxSize ){
			memcpy( data , p_data + __m_curr_get_point + pos, size );

            __m_curr_get_point += size;
		}else{ // 数据跨过了尾部，则需要分成两种情况来看，第一是取数据位置没有跨过，但是加上数据长度后跨过了
			if( __m_curr_get_point + pos < __m_maxSize ){
				size_t sec_ph = __m_maxSize - __m_curr_get_point - pos;

				memcpy( data , p_data + __m_curr_get_point + pos , sec_ph );
				memcpy( data + sec_ph , __p_data , size - sec_ph );

				// 调整下次取数据的位置
				__m_curr_get_point = size-sec_ph;
			}else{// 第二种是取数据的位置跨过了
				size_t real_pos = __m_curr_get_point + pos - __m_maxSize;

				memcpy( data , p_data + real_pos , size );

				__m_curr_get_point = real_pos + size;
			}
		}

		__m_curr_len -= size;
	}else{
		return ERR_LESS_DATA;
	}

	return ret;
}

size_t rb :: length()
{
	size_t ret = 0;
	std::lock_guard<std::mutex> l( __m_mutex );
	ret = __m_curr_len;

	return ret;
}

void rb :: drop( size_t size )
{
	std::lock_guard<std::mutex> l( __m_mutex );
	if( size > __m_curr_len ) clear();

	if( __m_curr_get_point + size >= __m_maxSize ){
		__m_curr_get_point = __m_curr_get_point + size - __m_maxSize;
	}else{
		__m_curr_get_point += size;
	}

	__m_curr_len -= size;
}
void rb :: clear()
{
	std::lock_guard<std::mutex> l( __m_mutex );

	memset( __p_data , 0 , __m_maxSize );

	__m_curr_get_point = 0;
	__m_curr_add_point = 0;
	__m_curr_len = 0;
}

rb :: emErrCode
rb :: resize( size_t size , bool autoZero  )
{
	std::lock_guard<std::mutex> l( __m_mutex );

	__p_data = realloc( __p_data , size );

	if( __p_data == NULL ) return ERR_ALLOC_MEM;

	__m_maxSize = size;

	if( autoZero ){
		memset( __p_data , 0 , __m_maxSize );

		__m_curr_len = 0;
		__m_curr_add_point = 0;
		__m_curr_get_point = 0;
	}

	return OK;
}

size_t rb :: index( const void * __data , size_t len )
{
	size_t ret = (size_t)-1;
	const char * data = ( char *)__data;
	char * p_data = ( char *)__p_data;
	std::lock_guard<std::mutex> l( __m_mutex );

	size_t i = __m_curr_get_point , pos = 0;
	for( ; i < __m_maxSize; ){
		bool is_find = false;

		__asm__ __volatile__(
			"1:cmpb %3,(%4)\n"          // do{ 比较数据
			"je 2f\n"                   //    如果数据相同执行相关的记录工作
			"inc %4\n"                  //    增加地址，取下一个数据进行比对
			"dec %2\n"                  //    否则递减循环计数器
			"jnz 1b\n"                  //    如果循环计数器不等于0 则继续比对
			"jmp 3f\n"                  // 循环结束还没有找到，则设置is_find = false, pos = -1;
			                            // }while( count > 0 );
			"2:movb $1, %0\n"           // 如果比较的字符和模式相同，则is_find=true
			"mov %4, %1\n"              // 同时记录下来发现的数据位置
			"jmp 4f\n"
			"3:movb $0, %0\n"
			"movl $-1, %1\n"
			"4:\n"
			: "=m" ( is_find ),
			  "=m" ( pos )
			: "c"  ( __m_curr_len ) ,
			  "a"  ( data[ 0 ] ),
			  "S"  ( p_data + i )
			:"memory"
			);

		if( is_find == false ){         // 找不到头数据
			throw ERR_IDX_CAN_NOT_FIND;
		}
		pos = pos - (size_t)p_data;
		i = pos + 1;
		if( len > 1 ){ // 如果检索长度超过1个字节在进行内存比对
			if( pos + len < __m_maxSize ){
				if( memcmp( p_data + pos , data , len ) == 0 ){ // 找到目标
					ret = pos - __m_curr_get_point;
					break;
				}
			}else if( memcmp( p_data + pos , data , __m_maxSize - pos ) == 0 ){ // 比较前半段
				if( memcmp( p_data + __m_curr_get_point ,
					    data + len - __m_maxSize + pos ,
					    len - __m_maxSize + pos ) == 0 ){ // 比较后半段
					if( pos >= __m_curr_get_point )
						ret = pos - __m_curr_get_point;
					else{
						ret = pos + __m_curr_get_point;
					}
					break;
				}
			}// if( pos + len < __m_maxSize )
		}else{ // if( len > 1 )
			if( pos >= __m_curr_get_point )
				ret = pos - __m_curr_get_point;
			else{
				ret = pos + __m_curr_get_point;
			}
			break;
		}
	}
	if( i >= __m_maxSize ){ // 没有找到数据
		throw ERR_IDX_CAN_NOT_FIND;
	}

	return ret;
}

std::string rb :: errMsg( emErrCode e )
{
	std::string ret;
	switch( e ){
	case ERR_ALLOC_MEM:
		ret = "内存分配失败";
	break;
	case ERR_NO_DATA:
		ret = "当前缓冲区没有数据";
		break;
	case ERR_LESS_DATA:
		ret = "当前数据长度少于请求长度";
		break;
	case ERR_PTHREAD_MUTEX_INIT:
		ret = "初始化pthread_mutex错误";
		break;
	case ERR_SRC_NULL:
		ret = "源数据指针空";
		break;
	case ERR_DST_NULL:
		ret = "目标数据指针空";
		break;
	case ERR_DATA_NULL:
		ret = "缓冲区指针空";
		break;
	case ERR_IDX_CAN_NOT_FIND:
		ret = "检索数据的时候没有找到内容";
		break;
	default:break;
	}

	return ret;
}

