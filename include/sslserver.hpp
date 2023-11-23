/**
 * @brief ssl服务器
*/

#pragma once

#include <type_traits>
#include <memory>

#include "details/sslConnection.hpp"

namespace private__
template< typename backendType >
class sslServer__
{
public:
	static_assert( std::is_base_of< sslSvrItfc , backendType >::value , "" );
	
	using connection_t = backendType::connection_t;
	using event_t = backendType::event;
	using evtData_t = backendType::evtData;
private:
	std::shared_ptr< backendType >   pt_backend__;
public:
	sslServer__( const std::string& ca , const std::string& cert , const std::string& key ){
		pt_backend__ = std::make_shared< backendType >( ca , cert , key );
	}
	virtual ~sslServer__(){}
	
	std::shared_ptr< backendType >
	operator->(){
		std::weak_ptr< backendType >  ptr( pt_backend__ );
		if( ptr.expired()){
			return {}
		}

		return ptr.lock();
	}
};

}

#if defined( __LINUX__ )
#include "details/sslSvrEpoll.hpp"

namespace wheels
{
	using sslServer = private__::sslServer__<sslSvrEpoll>;
}
#elif defined( WINNT ) || defined( WIN32 )

#endif
