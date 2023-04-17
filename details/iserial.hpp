/**
 * @brief 通用的串口接口定义。
 * @verion 1.0
 * @author 宋炜
 *
 */

#pragma once

#include <functional>
#include <string>
#include <atomic>

namespace wheels{
class iSerial
{
public:
	struct stop{ enum value { NONE = 0, ONE , ONE_P_FIVE , TWO }; };
	struct parity{ enum value {NONE = 0 , EVEN = 1, ODD = 2};};
	struct flow{enum value{NONE = 0, HARD , SOFT, KEEPBACK };};

	enum err_code{
		ERR_BASE  = -1000,
		ERR_ALLOC_MEM,             // 分配内存失败
		ERR_DEV_OCCUPIED ,         // 串口被占用
		ERR_DEV_NOT_EXIST,         // 串口不存在
		ERR_SEND_DATA_EMPTY,       // 发送数据空
		ERR_SET_PARAMS,            // 设置串口参数失败
		ERR_BAD_DEV,               // 串口号错误
		ERR_BAD_FD,                // 系统错误文件描述符
		ERR_OPEN_SERIAL,           // 打开串口错误
		ERR_CLOSE_UART,            // 关闭串口出错
		ERR_SEND_FAIL,             //
		ERR_READ_DATA,			   // 读取数据操作错误
		ERR_ALREADY_OPENED,
		SET_COM_FAIL,              // WIN特殊错误，配置接收事件错误
		ERR_DATA_NULL,             // 数据指针为空
		GET_COM_FAIL,
		ERR_STOP_READ_THD,
		ERR_OVT,
		ERR_ASYNC_WAIT_FAIL,
		ERR_ASYNC_GET_RST_FAIL,
		OK = 0
	};

	using emErrCode = err_code;
public:
	/**
	 * @brief 打开串口。
	 * @param baud 波特率
	 * @param char_len 字符长度
	 * @param s 停止位
	 * @param p 校验位
	 * @param f 流控
	 * @retval 成功返回OK, 失败返回错误代码
	 */
	virtual err_code open( int baud , int char_len , stop::value s , parity::value p , flow::value f ) = 0;
	virtual err_code close() = 0;
	/**
	 * @brief 发送数据。这个函数调用后立即返回，操作结果从回调中获取，因此这个函数执行返回后
	 *        并不一位置发送完成
	 * @param len ， 要发送的数据长度
	 * @param data , 要发送的数据
	 */
	virtual err_code send( size_t len , const char* data ) = 0;
	/**
	 * @brief 操作事件设置操作事件
	 */
	virtual iSerial& evtOpen(  std::function< void (err_code)> fun ) = 0;
	iSerial& evtClose( std::function< void (err_code)> fun                        );
	iSerial& evtSend(  std::function< void (size_t , err_code)> fun               );
	iSerial& evtRecv(  std::function< void (size_t , const char* , err_code)> fun );

	/**
	 * @brief 复位串口。关闭串口重新打开
	 */
	virtual err_code reset() = 0;
	virtual err_code reset( int baud , int char_len , stop::value s , parity::value p , flow::value f ) = 0;
	/**
	 * @brief 串口是否打开
	 * @return 打开返回true , 否则返回false
	 */
	virtual bool is_open() = 0;
	/**
	 * @brief 启动串口循环
	 * @param run[ I ], true，启动；false停止
	 */
	virtual void run( bool run ) = 0;
	/**
	 * @brief 结束串口消息循环
	 */
	virtual void standby() = 0;

	static std::string errMsg( emErrCode& e );
};
}
