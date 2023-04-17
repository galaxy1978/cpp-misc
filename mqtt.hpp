/**
 * @brief MQTT 客户端C++实现
 * @version 1.0
 * @author 宋炜
 * @date 2021-7-5
 */

#pragma once
#include <functional>
#include <string>
#include <map>
#include <atomic>
#include <mutex>
#include "MQTTClient.h"

class mqttClient
{
public:
	enum emErrCode{
		ERR_ALLOC_MEM = -1000,   // 内存分配事变
		ERR_USR_PSWD,            // 用户名或者密码错误
		ERR_CONN,	         	 // 连接失败
		ERR_CREATE_CTX,          // 创建MQTT句柄失败
		ERR_PUB,
		ERR_SUB,
		ERR_INSERT_CB,
		ERR_UNSUB,
		ERR_QOS_RANGE,
		ERR_DATA_NULL,
		ERR_URL_EMPTY,
		ERR_CID_EMPTY,
		ERR_ACCOUNT_EMPTY,
	    OK = 0
	};

	/// 从path.mqtt继承消息结构，提供消息构造接口
	struct stMsg : public MQTTClient_message{
		/**
		 * @brief 默认构造qos = 1, retain = true; 构造后需要调用msg来指定消息数据内容。
		 * 也可以调用包含数据参数的构造
		 */
		stMsg();
		stMsg( const stMsg& b );

		explicit stMsg( const std::string& data );
		stMsg( const void * data , size_t len );

		stMsg( int qos , const void * data , size_t len , bool retain = false );
		stMsg( const std::string& data , int qos , bool retaiin = false );

		~stMsg();

		stMsg& operator=( const stMsg& b );
		/**
		 * @brief 指定消息内容
		 * @param data[ I ]， 消息内容。对于std::string类型的参数，直接使用std::string::length获取
		 *	载荷长度
		 * @param len[ I ]， 载荷长度
		 */
		void msg( const void * data , size_t len );
		void msg( const std::string& data );
		/**
		 * @brief 从paho.mqtt构造对象
		 * @param msg
		 */
		static stMsg fromPaho( const MQTTClient_message& msg );
	};
	/// 定义消息回调存储结构。
	/// @tparam std::string  消息主题
	/// @tparam std::function< void (const stMsg& ) > 消息回调函数
	typedef std::map< std::string , std::function< void (const std::string& , const stMsg& ) > >  msgCallback;
private:
	std::string                __m_cid;       // 客户端ID
	std::string                __m_account;   // 用户账号
	std::string                __m_pswd;      // 用户密码
	std::atomic< bool >        __m_is_connected; // 是否建立连接
	std::atomic< int >         __m_wait_timeout; // 等候超时时间，单位为ms

	std::function< void () >   __m_on_disconnected;
	msgCallback                __m_msg_cb;
	MQTTClient                 __m_client;    // 客户端
	MQTTClient_deliveryToken   __m_dt;        //
	int                        __m_mqtt_version;
	std::mutex                 __m_mutex;
public:
	/**
	 * @brief 构造MQTT客户端。在默认情况下如果不指定MQTT协议的版本，以3.1.1版本尝试连接；如果练级失败会尝试以3.1连接，再次失败会以5.0版本尝试
	 * @param svr[ I ],
	 * @param cid[ I ], 客户端ID
	 */
	mqttClient( const std::string& svr , const std::string& cid );
	/**
	 * 在MQTT 3.1.1 会支持使用用户名和密码登录
	 * @param svr [ I ], 服务器信息，比如tcp://xxx.xxx.xxx:1883
	 * @param usr [ I ], 用户名
	 * @param pswd [ I ], 登录密码
	 */
	mqttClient( const std::string& svr ,
		    const std::string& cid ,
		    const std::string& usr ,
		    const std::string& pswd );

	~mqttClient();

	inline void version( int v ){ __m_mqtt_version = v; }
	
	emErrCode connect();
	emErrCode connect( const std::string& cid , const std::string& user , const std::string& pswd );
	
	void setDisconnect( std::function< void ( ) > fun );
	/**
	 * @brief onDisconnect
	 * @param cause
	 */
	void onDisconnect( const std::string& cause );
	/**
	 * @brief 采用默认的消息体发布消息，在默认情况下Qos = 1, retain = true， dup=0
	 * @param topic[ I ]， 要发布的消息主题
	 * @param data[ I ]， 消息内容
	 * @param len[ I ]，消息数据长度
	 * @return 成功操作返回OK，否则返回错误代码
	 */
	emErrCode pub( const std::string& topic , const uint8_t * data , size_t len );
	/**
	 * @brief 按照自定义消息的方法发布消息，可以通过构造msg内容来实现
	 * @param topic[ I ], 主题
	 * @param msg[ I ], 消息
	 * @return
	 */
	emErrCode pub( const std::string& topic , stMsg& msg );
	/**
	 * @brief 订阅主题，并登记相关主题的回调函数
	 * @param topic[ I ]，要订阅的主题
	 * @param cb[ I ]，回调函数
	 * @return 操作成功返回OK，否则返回错误代码
	 */
	emErrCode sub( const std::string& topic , int qos,
		       std::function< void ( const std::string& tp , const stMsg& ) > cb );
	emErrCode unSub( const std::string& topic );
	/**
	 * @brief 处理接收到消息后的操作
	 * @param tp[ I ]，主题名
	 * @param msg[ I ]，消息内容
	 * @return 消息处理正常返回1，否则返回0
	 */
	int onRecv( const std::string& tp , const stMsg& msg );
	/**
	 * @brief 是否已经成功建立连接
	 * @return 成功连接返回true, 否则返回false
	 */
	bool isConnected();

	/**
	 * @brief 重新连接
	 */
	void reconnect( const std::string& cid , const std::string& pswd );

	static std::string errMsg( emErrCode e );
};
