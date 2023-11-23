

#pragma once
#include <stdint.h>

#include <type_traits>
#include <string>
#include <atomic>
#include <memory>

#include "details/sslSvrItfc.hpp"
#include "designM/command.hpp"

class sslSvrEpoll : public sslSvrItfc
{
private:
	std::string   m_addr__;
	std::atomic< uint16_t >      m_port__;
	std::atomic< int >           m_sock__;
public:
	sslSvrEpoll( const std::string& ip , uint16_t port , const std::string& ca, const std::string& cert , const std::string& key );
	virtual ~sslSvrEpoll(){}
	/**
	 */
	bool start( int maxConn = 100 );

	bool connect( const event& cmd , std::function< wheels::dm::disppatcher<event> :: evt_func_type> func ){
		wheels::dm::disppatcher<event> * dispt = disppatcher<event>::get();
		return dispt->connect( cmd , func );
	}
	/**
	 * @brief 连接命令和函数指针指向的处理函数
	 * @param func[ I ]， 函数指针
	 */
	bool connect( const event& cmd , wheels::dm::disppatcher<event> :: func ){
		wheels::dm::disppatcher<event> * dispt = wheels::dm::disppatcher<event>::get();
		return dispt->connect( cmd , func );
	}

	template<typename classType , typename funcType >
	bool connect( const event& cmd , funcType classType::* func , classType * obj ){
		disppatcher<event> * dispt = disppatcher<event>::get();
		return dispt->connect( cmd , func , obj );
	}
private:
	void process_accept__();
	void process_read__( int socket );
};
