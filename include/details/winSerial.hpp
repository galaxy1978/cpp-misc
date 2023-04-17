/**
 * @brief windows下的串口通讯接口
 * @version 1.0
 * @date 2017-10-31
 * @author 宋炜
 *    2018-5-10   添加异步回调处理
 */

#pragma once

#include <functional>
#include <windows.h>
#include <thread>
#include <string>
#include <mutex>
#include <vector>
#include <atomic>

#include "details/iSerial.hpp"

namespace wheels
{
class winSerial : public iSerial
{
private:
	HANDLE                   __m_hPort;            // 端口句柄

	bool                     __m_is_init;
#if defined( _UNICODE ) || defined( UNICODE )	
	wchar_t                  __m_port[ 64 ];       // 端口名称
#else
	char                     __m_port[ 64 ];
#endif
	size_t                   __m_buff_size;        // 当前缓冲区
	size_t                   __m_last_read;        // 最后读到的数据长度
	std::atomic<long>        __m_dead_line_time;   //
	std::atomic<err_code>    __m_error;            // 最后一次错误的错误代码
	std::atomic<bool>        __m_is_open;          // 串口是否打开
	std::atomic<bool>        __m_is_run;           // 异步读取控制线程是否在运行
	std::atomic<bool>        __m_is_stoped;        // 判断是否停止运行
	OVERLAPPED               m_ovlped;
	// 使用双缓冲的方式交替使用。每一次收到数据后交换缓冲
	char                   * __p_buff;
	char                   * __p_buff_1;           // 缓冲区1
	char                   * __p_buff_2;           // 缓冲区2
	/**
	 */
	//std::function< void ( const char* , size_t ) >   m_fun;
	std::function< void (size_t , const char* , err_code)> on_recv;   // 接收到数据后回调
	/*
	 * 发送完成回调函数定义：
	 */
	std::function< void (size_t , err_code ) > on_send;   // 发送操作回调函数
	/*
	 * 串口打开回调函数定义：
	 *
	 */
	std::function< void (err_code) > on_open;   // 响应打开串口操作
	/*
	 * 串口关闭回调函数定义：
	 */
	std::function< void (err_code) > on_close;  // 响应关闭操作

private:

	friend void CALLBACK on_serial_read( DWORD err , DWORD len , LPOVERLAPPED ovlp );
	friend void CALLBACK on_serial_write( DWORD err , DWORD len , LPOVERLAPPED ovlp );

	void __do_set_stopbits( HANDLE h , stop::value s , LPDCB dcb );
	void __do_set_parity  ( HANDLE h , parity::value p , LPDCB dcb );
	void __do_set_flow    ( HANDLE h , flow::value f , LPDCB dcb );
	size_t __do_real_read( char * buff , size_t len , long ovt );
	/**
	 * @brief 发送数据
	 */
	size_t __write( const char *buff , size_t len );
	/**
	 * @brief 执行打开串口操作
	 */
	bool __do_open_port( int baud , int d_len , parity::value p , stop::value s , flow::value f );
	/**
	 * @brief 取消异步操作
	 */
	void __do_reset_io();

	err_code __do_real_readEx(char * buff, size_t len);
	/**
	 * @brief 读取系统错误字符串，并输出到标准流上
	 */
	void __do_process_error( DWORD err );

public:
	/**
	 * @brief 处理异步收到数据的操作
	 * @param povlped[ I ], 异步对象指针
	 */
	void __on_recv( LPOVERLAPPED povlped );
	/**
	 * @brief 处理异步发送完成通知
	 * @param err[ I ]
	 * @param len[ I ]
	 */
	void __on_write( DWORD err , DWORD len );
	/**
	*/
	virtual err_code open( int baud , int char_len , stop::value s , parity::value p , flow::value f )final;
	virtual err_code close()final;
	/**
	 * @brief 发送数据。这个函数调用后立即返回，操作结果从回调中获取，因此这个函数执行返回后
	 *        并不一位置发送完成
	 * @param len ， 要发送的数据长度
	 * @param data , 要发送的数据
	 */
	virtual err_code send( size_t len , const char* data )final;
	/**
	 * @brief 操作事件设置操作事件
	 */
	virtual iSerial& evtOpen(  std::function< void (err_code)> fun )final;
	virtual iSerial& evtClose( std::function< void (err_code)> fun )final;
	virtual iSerial& evtSend(  std::function< void (size_t , err_code)> fun )final;
	virtual iSerial& evtRecv(  std::function< void (size_t , const char* , err_code)> fun )final;

	virtual void SetBufferSize( size_t l )final{
		__m_buff_size = l;

	}

	/**
	 * @brief 复位串口。关闭串口重新打开
	 */
	virtual err_code reset()final;
	virtual err_code reset( int baud , int char_len , stop::value s , parity::value p , flow::value f )final;
	/**
	 * @brief 串口是否打开
	 * @return 打开返回true , 否则返回false
	 */
	virtual bool is_open()final;
	/**
	 * @brief 启动串口循环
	 * @param run[ I ], true，启动；false停止
	 */
	virtual void run( bool run )final;
	/**
	 * @brief 结束串口消息循环
	 */
	virtual void standby()final;
	/**
	 * @brief 构造串口通讯程序。使用默认的构造函数与进行初始化后还需要调用create来打开串口。
	 * @brief const string& port , 要打开的串口, 在windows下为"COM(X)", 在linux下为ttyS（X）
	 * @param int baud , 波特率
	 * @param serial_port::parity p , 奇偶校验
	 *        serial_port::stop_bits stop , 停止位
	 *        serial_port::flow_control flow , 硬件流控制
	 *        char d_len ,数据长度
	*/
	winSerial( const std::string& dev );
	winSerial( const std::string& dev , int baud , int d_len ,  stop::value s ,  parity::value p , flow::value f );
	virtual ~winSerial();

	void Stop();
};
	/* 串口描述结构体。*/
	struct stDevDesc{
		std::string   name;        // 设备名称
		std::string   desc;        // 设备描述
		std::string   friend_name; // 友好名称
		std::string   vendor;      // 厂商

		stDevDesc(){}
		virtual ~stDevDesc(){}

		stDevDesc( const stDevDesc& b )	{
			name = b.name;
			desc = b.desc;
			friend_name = b.friend_name;
			vendor = b.vendor;
		}

		stDevDesc& operator=(  const stDevDesc& b ){
			name = b.name;
			desc = b.desc;
			friend_name = b.friend_name;
			vendor = b.vendor;

			return *this;
		}

		void clear(){
			name.clear();
			desc.clear();
			friend_name.clear();
			vendor.clear();
		}
	};
	/**
	 * @brief 采用读取注册表的方式枚举串口信息
	 */
	extern void EnumSerial( std::vector< std::string >& a_rst );
	/**
	 * @brief 枚举串口设备
	 * @param a_rst[ O ], 枚举结果
	 */
	extern void EnumSerial( std::vector< stDevDesc >& a_rst );
}


