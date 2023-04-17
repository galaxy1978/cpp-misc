/**
 * @brief 串口接口模块，主要用来实现和串口的数据通讯操作。
 * @version 1.1
 * @date 2018-2-25~2020-10-1
 * @author 宋炜
 */

/// 2020-10-1 修改串口接口实现方式，将原来部分接口抽象出来，方便在不同的硬件或者软件
///           平台实现通用的代码
#ifndef __LINUX_SERIAL_HPP__
#define __LINUX_SERIAL_HPP__

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include "timer.hpp"
#include "iserial.hpp"
namespace wheels{
class linuxSerial : public iSerial
{
private:
    
	/**
	 * libuv 没有提供串口操作的功能，但是在linux下所有的设备都是可以看做文件来处理的
	 * 使用libuv提供的文件操作功能打开串口文件，然后获取标准的文件描述符调用linux
	 *      #include<termios.h>
	 *      int tcsetattr(int fd , int actions , const struct termios *termios_h);
	 * 设置串口参数。此后所有的异步操作均采用libuv操作。
	 */
	int             m_fd;
	std::string     m_name;         // 串口名称

	int             m_baud;         // 波特率
	int             m_char_len;     // 字符长度
	stop::value     m_stop;         // 停止位
	parity::value   m_parity;       // 校验位
	flow::value     m_flow;         // 流控

	std::atomic<err_code>        m_error;        // 错误编号
	std::atomic<bool>            m_is_open;      // 当前状态
	std::atomic< int >           m_is_run;       // 0，就绪；1 运行；2等待停止；3 停止；4 关闭
	std::atomic<size_t >         m_buff_size;    // 缓冲区大小
	std::mutex                   m_mutex;        //
	/*
	 * 接收数据的回调函数定义：
	 *
	 *  void fun( size_t len , const char* data , err_code err );
	 *  参数：
	 *    len , 接收到的数据长度
	 *    data ， 接收到的数据缓冲指针
	 *    err , 操作错误代码，成功操作为OK ，否则为错误代码
	 */
	std::function< void (size_t , const char* , err_code)> on_recv;   // 接收到数据后回调
	/*
	 * 发送完成回调函数定义：
	 */
	std::function< void (size_t , err_code )             > on_send;   // 发送操作回调函数
	/*
	 * 串口打开回调函数定义：
	 *
	 */
	std::function< void (err_code)                       > on_open;   // 响应打开串口操作
	/*
	 * 串口关闭回调函数定义：
	 */
	std::function< void (err_code)                       > on_close;  // 响应关闭操作

public:
	/**
	 * @brief 创建对象，设置窗口名称和接收到数据的时候回调响应
	 * @param name
	 * @exception 如果缓冲区内存分配失败，会抛出ERR_ALLOC_MEM
	 */
	linuxSerial( const std::string& name );
	/**
	 * @brief 创建并打开串口
	 * @param baud 波特率
	 * @param char_len 字长
	 * @param s 停止位
	 * @param p 校验位
	 * @param f 流控参数
	 * @exception 如果缓冲区内存分配失败，会抛出ERR_ALLOC_MEM
	 */
	linuxSerial( const std::string& name ,
		int baud ,
		int char_len ,
		stop::value s ,
		parity::value p ,
		flow::value f
		);

	virtual ~linuxSerial();
	/**
	 * @brief 打开串口。
	 * @param baud 波特率
	 * @param char_len 字符长度
	 * @param s 停止位
	 * @param p 校验位
	 * @param f 流控
	 * @retval 成功返回OK, 失败返回错误代码
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
	virtual iSerial& evtOpen(  std::function< void (err_code)> fun )final
	{
		on_open = fun;
		return *this;
	}
	virtual iSerial& evtClose( std::function< void (err_code)> fun )final{
		on_close = fun;
		return *this;
	}
	virtual iSerial& evtSend(  std::function< void (size_t , err_code)> fun )final{
		on_send = fun;
		return *this;
	}
	virtual iSerial& evtRecv(  std::function< void (size_t , const char* , err_code)> fun )final{on_recv = fun; return *this; }
	/**
	 * @brief 复位串口。关闭串口重新打开
	 */
	virtual err_code reset()final;
	virtual err_code reset( int baud , int char_len , stop::value s , parity::value p , flow::value f )final;
	/**
	 * @brief 串口是否打开
	 * @return 打开返回true , 否则返回false
	 */
	virtual bool is_open()final{ return m_is_open.load();}
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
	 * @brief 响应libuv 打开文件操作
	 * @param fd , uv_file句柄
	 */
	void OnOpen( int fd );
	/**
	 * @brief 响应读取数据完成
	 * @param data[ I ]
	 * @param len
	 */

    	void OnRecv( const char * dat , size_t len);
	/**
	 * @brief 响应写数据完成
	 * @param len , 已经发送的数据长度
	 * @param e , 操作过程出现的错误代码
	 */
	void OnSend( size_t len , err_code e );
	/**
	 * @brief 响应串口关闭
	 */
	void OnClose( err_code e );
	/**
	 * @brief 更新串口参数，这个函数主要的目的是方便RFC2217.
	 */
	static bool updt_param( int fd , int baud , int char_len , stop::value s , parity::value p , flow::value f );
	/**
	 * @brief 设置缓冲大小
	 * @param l
	 * @exception 如果内存分配失败抛出ERR_ALLOC_MEM
	 */
	void SetBufferSize( size_t l );
	static std::string errMsg( emErrCode& e );
protected:
	/**
	 * @brief 初始化设备参数，试着波特率这些内容
	 * @param fd , 注意这个fd是linux下的文件描述符
	 * @param baud
	 * @param char_len
	 * @param s
	 * @param p
	 * @param f
	 */
	void init_device( int fd , int baud , int char_len , stop::value s , parity::value p , flow::value f );
	/**
	 * @brief 调用SELECT函数检查是否数据准备好
	 */
	bool __is_ready();
};
}
#endif // __SERIAL_HPP__
