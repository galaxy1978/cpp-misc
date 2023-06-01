/**
 * @brief 
 * @version 1.0
 * @author 宋炜
 * @date 
 */



#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>


#include "fsm2.hpp"
#include "mallocSharedPtr.hpp"

	const static bool SLAVE = false;
	const static bool MASTER = true;
		
	class fsmImpl : public wheels::fsm2::fsm2< std::string , uint8_t >
	{
	public:

	private:

	private:

		bool __on_site_ent( const std::string& s , const fsm2::stDataItem& data );
		std::string __on_ready_leave( const std::string& s , const fsm2::stDataItem& data );
		bool __on_overtime_ent( const std::string& , const fsm2::stDataItem& data );
		void __on_statSite_overtime_raise( const std::string& f , const std::string& t , const fsm2::stDataItem& data );
		/**
		 * @brief 初始化FSM
		 * @param slave_master[ I ], 配置协议为sla
		 * @return 成功操作返回true，否则返回false
		 */
		bool __init_fsm( bool slave_master );

	public:
		explicit fsmImpl( bool sm );
		virtual ~fsmImpl();
		
		/**
		 * @brief 调用这个程序实际处理数据
		*/
		fsm::emErrCode eat( const char * data , size_t len )

	};


// 下面的代码在实现部分

fsmImpl :: fsmImpl( bool sm ): fsm2( "READY" , [&]( const std::string& stat , const fsm2::stDataItem& raw)->bool{ return __on_ready_ent( stat , raw );	} )
{
	__init_fsm( sm );
}

fsmImpl :: ~fsmImpl()
{
}

bool fsmImpl :: __on_ready_ent( const std::string& s , const fsm2::stDataItem& data )
{
	bool ret = TRIGER_RUN;
	MSG( "解析器就绪，等候MODBUS-RTU协议数据" , OK_MSG );
	__m_aux_data.clear();
	memset( &__m_rst_data.__m_resp , 0 , sizeof( stFrmReq ) );
	clearRaw();
	return ret;
}

void fsmImpl :: __data_overtime()
{
	__m_timer.Stop();
	if( __m_cb_err ){
		__m_cb_err( ERR_DATA_OVERTIME );
	}
}


fsm::emErrCode
fsmImpl :: eat( const char * data , size_t len )
{
	fsm::emErrCode ret = OK;

	assert( data );
	// 拷贝内存到临时变量，避免在nodejs情况下因为异步操作导致
	// 内存失效
	uint8_t d[ 256 ] = {0};
	memcpy( d , data , len );
	size_t i = 0;
	for( i = 0; i < len; i ++ ){
		ret = fsm2::eat( data[ i ] );
		if( ret != OK ){
			return ret;
		}
	}

	return ret;
}

bool fsmImpl :: __init_fsm( bool s_m )
{
	bool ret = true;
	if( s_m == SLAVE ){
		ret = __init_fsm_slave();
	}
	
	return ret;
}

bool fsmImpl :: __on_site_ent( const std::string& s , const fsm2::stDataItem& data )
{
	bool ret = TRIGER_RUN;
	uint8_t d = 0;
	data.get( d );

	__m_rst_data.__m_resp.m_site = d;

	return ret;
}
std::string
fsmImpl :: __on_ready_leave( const std::string& s , const fsm2::stDataItem& data )
{
	__m_aux_data.clear();
	
	__m_timer.Stop();
	return {"statSite"};
}

bool fsmImpl :: __on_overtime_ent( const std::string& , const fsm2::stDataItem& data )
{
	// 当发生通讯超时，需要从当前状态切换到出事状态，以避免后续解析死锁
	bool ret = AUTO_RUN;
	
	return ret;
}

void fsmImpl :: __on_statSite_overtime_raise( const std::string& f , const std::string& t , const fsm2::stDataItem& data )
{
	ERROR_MSG( "收到站点号后等候后续数据时间过长" );
}


bool fsmImpl :: __init_fsm_master()
{
	bool ret = false;

	add( "READY" , "statSite" , onFsm2ENT( __on_site_ent ) , onFsm2LEAVE( __on_ready_leave ) );
	add( "statSite" , "OVERTIME" , onFsm2ENT( __on_overtime_ent ));
	add( "statSite" , "OVERTIME" , onFsm2RAISE( __on_statSite_overtime_raise));
	
	return ret;
}

