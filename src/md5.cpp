#include <stdio.h>
#include <string.h>
#include <string>
#include <openssl/md5.h>
#include "misc.hpp"
#include "md5.hpp"
///////////////////////////////////////////////////////////////////////////////////////////////
CMd5::CMd5():p_data_l((unsigned long long*)(&m_data[ 0 ])),
	p_data_h((unsigned long long*)(&m_data[ 8 ] )),
      __p_ctx( nullptr ){}
///////////////////////////////////////////////////////////////////////////////////////////////
CMd5 :: CMd5( const std::string& data )
    : p_data_l((unsigned long long*)(&m_data[ 0 ])),
      p_data_h( (unsigned long long*)(&m_data[ 8 ] )),
      __p_ctx( nullptr )
{
    char *d = (char*)malloc( data.length() + 1 );

    if( d == NULL ){//内存不足
        m_error = FATAL_NO_MEM;
        throw m_error;
    }
    else{
        memcpy( d , data.c_str() , strlen( data.c_str()) );
        do_digest( d, data.length() );
    }
	if( d != NULL )
		free( d );
	d = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CMd5 :: CMd5( const char* data , int len )
:p_data_l((unsigned long long*)(&m_data[ 0 ]))
,p_data_h( (unsigned long long*)(&m_data[ 8 ] ))
,__p_ctx( nullptr )
{
    if( data == NULL || len < 0 ) return;
    do_digest( data , len );
}
///////////////////////////////////////////////////////////////////////////////////////////////
CMd5 :: CMd5( const CMd5& b):
	p_data_l((unsigned long long*)(&m_data[ 0 ])),
	p_data_h( (unsigned long long*)(&m_data[ 8 ] )),
	__p_ctx( nullptr )
{
    memcpy( m_data , b.m_data , 16 );
}
///////////////////////////////////////////////////////////////////////////////////////////////
/*
CMd5& CMd5 :: operator=( CMd5& md5 )
{
    memcpy( m_data , md5.m_data , 16 );
    return *this;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////
CMd5 :: ~CMd5()
{
      p_data_l = NULL;
      p_data_h = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CMd5 :: Caculate( const char* data , int len )
{
    if( data == NULL || len < 0 ) return;

    do_digest( data , len );

}
///////////////////////////////////////////////////////////////////////////////////////////////
void CMd5 :: Caculate( const string& data )
{
	char *d = (char*)malloc( data.length() + 1 );

	if( d == NULL ){//内存不足
		m_error = FATAL_NO_MEM;

		return;
	}else{
		memcpy( d , data.c_str() , strlen( data.c_str()));
		do_digest( d, data.length() );
	}

	free( d );
}
///////////////////////////////////////////////////////////////////////////////////////////////

bool CMd5 :: operator==(const CMd5& b)const
{
	bool ret = false;
	char b1[ 16 ], b2[ 16 ];
	memcpy( b1, m_data , 16 );
	memcpy( b2 , b.m_data , 16 );
	if( memcmp( b1 , b2 , 16 ) == 0 )
		ret = true;
	else
		ret = false;

	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool CMd5 :: operator <( const CMd5& b )const
{
	char b1[ 16 ], b2[ 16 ];
	memcpy( b1, m_data , 16 );
	memcpy( b2 , b.m_data , 16 );
	int ret = memcmp( b1 , b2 , 16 );
	if( ret < 0 ) return true;
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool CMd5 :: operator>( const CMd5& b )const
{
    char b1[ 16 ], b2[ 16 ];
    memcpy( b1, m_data , 16 );
    memcpy( b2 , b.m_data , 16 );
    int ret = memcmp( b1 , b2 , 16 );
    if( ret > 0 ) return true;
    return false;

}
///////////////////////////////////////////////////////////////////////////////////////////////
int CMd5 :: do_digest( const char *data , int len )
{
	if( data == NULL ){
		m_error = NULL_POINTER;
		return m_error;
	}
	int ret = 0;
	MD5_CTX mdctx;

	MD5_Init( &mdctx );
	MD5_Update( &mdctx , data , len );
	MD5_Final( m_data , &mdctx );

	return ret;
}

int CMd5 :: update( void * data , size_t len )
{
	int ret = 0;
	if( data == nullptr ) return -1;
	if( nullptr == __p_ctx ){
		__p_ctx = ( MD5_CTX *)malloc( sizeof( MD5_CTX ));

		if( __p_ctx == nullptr ){
			__show_line( "Allocate memory fail." );
			return -1;
		}

		MD5_Init( __p_ctx );
	}

	MD5_Update( __p_ctx , data , len );

	return ret;
}

std::string CMd5 :: finish()
{
	std::string ret;
	if( nullptr != __p_ctx ){
		MD5_Final( m_data , __p_ctx );
		ToString( ret );
		free( __p_ctx );
		__p_ctx = nullptr;
	}else{
		__show_line( "openssl context is not initializ." );
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CMd5& CMd5 :: operator=(const CMd5& b )
{
    memcpy( m_data , b.m_data , 16 );
    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CMd5 :: ToString( std::string& str )const
{
  /*
    这个函数不能区分MD5数据是否存在，所以再使用这个函数的时候需要自己了解
    数据是存在的。
   */
    char *buff = (char*)malloc( 33 );
    if( buff == NULL ){
        return;
    }
    memset( buff , 0 , 33 );

    for( int i = 0; i < 16; i ++ )
        sprintf( buff + 2 * i , "%02X" , m_data[ i ] );
    str = string( buff );

    if( buff != NULL )
		free( buff );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
int CMd5 :: FromString( const string& md5 )
{
    int ret = 0;
    char buff[ 50 ] ;
    memset( buff , 0 , 50 );

    unsigned char *p = m_data;
    unsigned int   tmp_data;
    memset( buff , 0 , 50 );
    if( md5.empty() == true ) return MD5_STRING_NULL;

    memcpy( buff , md5.c_str() , md5.length() );

    for( size_t i = 0; i < md5.length(); i += 2 , p ++){
        sscanf( buff + i , "%2X" , &tmp_data );
        *p = (unsigned char)tmp_data;
    }
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
string CMd5 :: Str()const
{
    string ret;
    ToString( ret );
    return ret;
}
