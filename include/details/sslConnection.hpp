/**
 * @brief SSL连接
 * @version 1.0
 * @author 宋炜
 */

#pragma once
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "mallocSharedPtr.hpp"
#include "sslSvrItfc.hpp"

class sslSvrConnection
{
public:
	using buff_t = wheels::mallocSharedPtr<uint8_t >;
private:
	SSL        * p_ssl__;
#if defined( WIN32 ) || defined( WINNT )
#    include <windows.h>
	HANDLE  m_sock__;
#elif defined( __LINUX__ )
	int     m_sock__;
#endif
private:

public:
	sslConnEpoll( native_socket sock , SSL * ssl ):m_sock__( sock ) , p_ssl__( ssl ){}
	virtual ~sslConnEpoll(){
		close( sock );
		SSL_free(ssl);

		auto dispt = sslSvrItfc::disppatcher_t::get();
		if( dispt ){
			dispt->emit( sslSvrItfc::event::EVT_CLOSE , std::string() );
		}
	}

	buff_t read();
	size_t send( buff_t buff );
};
