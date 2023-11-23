#include "details/sslConnEpoll.hpp"

buff_t sslConnEpoll :: read()
{
	buff_t ret( SSL_DATA_BUFF_SIZE );
	int len = SSL_read(p_ssl__, buffer, SSL_DATA_BUFF_SIZE );

	if( len > 0 ){
		ret.dataLen( len );
	}
	return ret;
}

size_t sslConnEpoll :: send( buff_t buff )
{
	size_t ret = SSL_write( p_ssl__ , buff.get() , buff.dataLen() );
	return ret;
}
