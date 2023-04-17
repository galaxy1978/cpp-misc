/**
 * @brief 串口接口模块，主要用来实现和串口的数据通讯操作。
 * @version 1.1
 * @date 2018-2-25~2020-10-1
 * @author 宋炜
 */

/// 2020-10-1 修改串口接口实现方式，将原来部分接口抽象出来，方便在不同的硬件或者软件
///           平台实现通用的代码
#ifndef __SERIAL_HPP__
#define __SERIAL_HPP__

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <type_traits>

#include "details/iSerial.hpp"

namespace wheels
{

template< typename T ,
		typename midT0 = typename std::decay< T >::type ,
		typename midT1 = typename std::conditional< std::is_pointer< midT0 >::value , typename std::remove_pointer< midT0 >::type , midT0 >::type,
		typename realT = typename std::conditional< std::is_base_of<iSerial , midT1>::value , midT1 , void >::type
		>
class tSerial: public iSerial
{
private:
	std::unique_ptr< realT >   __pt_imp;
public:
	/**
	 * @brief 创建对象，设置窗口名称和接收到数据的时候回调响应
	 * @param name
	 * @exception 如果缓冲区内存分配失败，会抛出ERR_ALLOC_MEM
	 */
	tSerial( const std::string& name ){
		static_assert( !std::is_void< realT >::value , "串口后端类型是不被支持的类型，后端实现类型必须继承与iSerial" );
		try{
			__pt_imp.reset( new realT( name ) );
		}catch( std::bad_alloc& e ){
			throw ERR_ALLOC_MEM;
		};
	}

	/**
	 * @brief 创建并打开串口
	 * @param baud 波特率
	 * @param char_len 字长
	 * @param s 停止位
	 * @param p 校验位
	 * @param f 流控参数
	 * @exception 如果缓冲区内存分配失败，会抛出ERR_ALLOC_MEM
	 */
	tSerial( const std::string& name ,int baud ,int char_len ,stop::value s ,parity::value p ,flow::value f){
		static_assert( !std::is_void< realT >::value , "串口后端类型是不被支持的类型，后端实现类型必须继承与iSerial" );
		try{
			__pt_imp.reset( new realT( name , baud , char_len , s , p , f) );
		}catch( std::bad_alloc& e ){
			throw ERR_ALLOC_MEM;
		}
	}

	virtual ~tSerial(){}
	/**
	 * @brief 打开串口。
	 * @param baud 波特率
	 * @param char_len 字符长度
	 * @param s 停止位
	 * @param p 校验位
	 * @param f 流控
	 * @retval 成功返回OK, 失败返回错误代码
	 */
	virtual err_code open( int baud , int char_len , stop::value s , parity::value p , flow::value f ) final{
		return __pt_imp->open( baud , char_len , s , p , f );
	}

	virtual err_code close() final{
		return __pt_imp->close();
	}
	/**
	 * @brief 发送数据。这个函数调用后立即返回，操作结果从回调中获取，因此这个函数执行返回后
	 *        并不一位置发送完成
	 * @param len ， 要发送的数据长度
	 * @param data , 要发送的数据
	 */
	virtual err_code send( size_t len , const char* data ) final{
		return __pt_imp->send( len , data );
	}
	/**
	 * @brief 操作事件设置操作事件
	 */
	virtual iSerial& evtOpen(  std::function< void (err_code)> fun ) final{
		return __pt_imp->evtOpen( fun );
	}
	virtual iSerial& evtClose( std::function< void (err_code)> fun )final{
		return __pt_imp->evtClose( fun );
	}
	virtual iSerial& evtSend(  std::function< void (size_t , err_code)> fun )final{
		return __pt_imp->evtSend( fun );
	}
	virtual iSerial& evtRecv(  std::function< void (size_t , const char* , err_code)> fun )final{
		return __pt_imp->evtRecv( fun );
	}
	/**
	 * @brief 复位串口。关闭串口重新打开
	 */
	virtual err_code reset() final{
		return __pt_imp->reset();
	}

	err_code reset( int baud , int char_len , stop::value s , parity::value p , flow::value f )final{
		return __pt_imp->reset( baud , char_len , s , p , f );
	}
	/**
	 * @brief 串口是否打开
	 * @return 打开返回true , 否则返回false
	 */
	virtual bool is_open()final{
		return __pt_imp->is_open();
	}
	/**
	 * @brief 启动串口循环
	 * @param run[ I ], true，启动；false停止
	 */
	virtual void run( bool run )final{
		__pt_imp->run( run );
	}
	/**
	 * @brief 结束串口消息循环
	 */
	virtual void standby()final{
		__pt_imp->standby();
	}
	virtual void SetBufferSize( size_t l )final{
		__pt_imp->SetBufferSize( l );
	}
};
}

/// 预定义两个平台的类型。
#if defined( __LINUX__ )
#	include "details/linuxSerial.hpp"
using serial = wheels::tSerial< wheels::linuxSerial >;
#elif defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
#	include "details/winSerial.hpp"
using serial = wheels::tSerial< wheels::winSerial >;
#endif

#endif // __SERIAL_HPP__
