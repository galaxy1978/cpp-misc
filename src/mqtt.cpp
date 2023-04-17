#include <assert.h>
#include <unordered_map>
#include <cstring>

#include "mqtt.hpp"
#include "misc.hpp"

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	// 返回值为1表示已经成功收到消息，返回0表示没有收到消息
	mqttClient * obj = (mqttClient*)context;
	
	if( obj == nullptr ){
		return 1;
	}
	if( message == nullptr ){
		return 0;
	}

	std::string tp( topicName );
	MSG_1( "收到%s的消息" , OK_MSG , topicName );
	mqttClient::stMsg msg = mqttClient::stMsg::fromPaho( *message );
	obj->onRecv( tp , msg );
	
	MQTTClient_freeMessage( &message );

	return 1;

}
void connlost(void *context, char *cause)
{
	mqttClient * obj = (mqttClient*)context;
	if( obj == nullptr ){
		return;
	}
	std::string c;
	if( cause ) c = cause;
	obj->onDisconnect( c );
}

void delivered(void* context, MQTTClient_deliveryToken dt)
{
	MSG( "mqtt消息发送成功" , TNORMAL);
}
// ----------------------------------------------------------------------------------------- //
mqttClient ::
stMsg :: stMsg()
{
	memcpy( struct_id , "MQTM" , 4 );
	struct_version =  0;
	payload = nullptr ;
	payloadlen = 0;
	qos = 1;
	retained = 0;
}

mqttClient ::
stMsg :: stMsg( const stMsg& b )
{
	memcpy( struct_id , "MQTM" , 4 );

	struct_version = 0;
	payload = b.payload;
	payloadlen = b.payloadlen;
	qos = b.qos;
	retained = b.retained;
}

mqttClient ::
stMsg :: stMsg( const std::string& data )
{
	memcpy( struct_id , "MQTM" , 4 );
	struct_version = 0;
	payload = ( char *)data.c_str();
	payloadlen = data.length();
	qos = 1;
	retained = 0;
}

mqttClient ::
stMsg :: stMsg( const void * data , size_t len )
{
	memcpy( struct_id , "MQTM" , 4 );
	struct_version =  0;
	payload = ( void *)data;
	payloadlen = len;
	qos = 1;
	retained = 0;
}

mqttClient ::
stMsg :: stMsg( int _qos , const void * data , size_t len , bool retain )
{
	if( ( _qos < 0 ) || ( _qos > 2 ) )
		throw mqttClient::ERR_QOS_RANGE;
	if( data == nullptr )
		throw mqttClient::ERR_DATA_NULL;
	memcpy( struct_id , "MQTM" , 4 );
	struct_version = 0;
	payload = (void *)data;
	payloadlen = len;
	qos = _qos;
	retained = ( retain ? 1 : 0 );
}

mqttClient ::
stMsg :: stMsg( const std::string& data , int _qos , bool retain )
{
	if( ( _qos < 0 ) || ( _qos > 2 ) )
		throw mqttClient::ERR_QOS_RANGE;
	if( data.empty() )
		throw mqttClient::ERR_DATA_NULL;

	memcpy( struct_id , "MQTM" , 4 );
	struct_version = 0;
	payload = (void *)data.c_str();
	payloadlen = data.length();
	qos = _qos;
	retained = ( retain ? 1 : 0 );
}

mqttClient ::
stMsg :: ~stMsg()
{

}

mqttClient :: stMsg&
mqttClient ::
stMsg :: operator=( const stMsg& b )
{
	payload = (void *)b.payload;
	payloadlen = b.payloadlen;
	qos = b.qos;
	retained = b.retained;

	return *this;
}

void mqttClient ::
stMsg :: msg( const void * data , size_t len )
{
	assert( data );
	payload = ( void *)data;
	payloadlen = len;
}

void mqttClient :: 
stMsg :: msg( const std::string& data )
{
	assert( data.length() );
	payload = (void *)data.c_str();
	payloadlen = data.length();
}

mqttClient :: stMsg
mqttClient :: stMsg :: fromPaho( const MQTTClient_message& msg )
{
	stMsg ret;
	ret.dup = msg.dup;
	ret.msgid = msg.msgid;
	ret.payload = msg.payload;
	ret.payloadlen = msg.payloadlen;
	ret.properties = msg.properties;
	ret.qos = msg.qos;
	ret.retained = msg.retained;
	memcpy( ret.struct_id , "MQTM" , 4 );
	ret.struct_version = 0;
	return ret;
}
//                                                                                          //
// ---------------------------------------------------------------------------------------- //


mqttClient :: mqttClient( const std::string& svr , const std::string& cid ):
	__m_is_connected( false ),
	__m_wait_timeout( 10000 )
{
	if( svr.empty() ) throw ERR_URL_EMPTY;
	if( cid.empty() ) throw ERR_CID_EMPTY;
	
	
	int rc = MQTTClient_create(&__m_client, svr.c_str(), cid.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if( rc != MQTTCLIENT_SUCCESS ){
		ERROR_MSG( "创建MQTT连接句柄失败" );
		throw ERR_CREATE_CTX;
	}

	MQTTClient_setCallbacks(__m_client, this, connlost, msgarrvd, delivered);
}

mqttClient :: mqttClient( const std::string& svr , const std::string& cid , const std::string& usr , const std::string& pswd ):
	__m_is_connected( false ),
	__m_wait_timeout( 1000 )
{
	if( svr.empty() ){
		ERROR_MSG( "服务器地址没有配置" );
		throw ERR_URL_EMPTY;
	}
	if( cid.empty() ){
		ERROR_MSG( "CID参数没有设置" );
		throw ERR_CID_EMPTY;
	}
	if( usr.empty() ){
		ERROR_MSG( "用户名没有配置" );
		throw ERR_ACCOUNT_EMPTY;
	}

	int rc = MQTTClient_create(&__m_client, svr.c_str(), cid.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);

	if( rc != MQTTCLIENT_SUCCESS ){
		ERROR_MSG( "创建MQTT连接句柄失败" );
		throw ERR_CREATE_CTX;
	}

	MQTTClient_setCallbacks(__m_client, this, connlost, msgarrvd, delivered);
}

mqttClient :: emErrCode
mqttClient :: connect()
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	//conn_opts.MQTTVersion = __m_mqtt_version;
	conn_opts.username = __m_account.c_str();
	conn_opts.password = __m_pswd.c_str();
	

	int rc = MQTTClient_connect(__m_client, &conn_opts);
	if( rc != MQTTCLIENT_SUCCESS ){
		MSG_1( "连接MQTT服务器失败:%s" , TRED , MQTTClient_strerror( rc ) );
		return ERR_CONN;
	}else{
		__m_is_connected = true;
	}

	return OK;
}

mqttClient :: emErrCode
mqttClient :: connect( const std::string& cid , const std::string& user , const std::string& pswd )
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if( cid.empty() == false ){
		conn_opts.username = user.c_str();
	}
	if( pswd.empty() == false ){
		conn_opts.password = pswd.c_str();
	}

	int rc = MQTTClient_connect(__m_client, &conn_opts);
	
	if( rc != MQTTCLIENT_SUCCESS ){
		ERROR_MSG( "连接MQTT服务器失败" );
		return ERR_CONN;
	}else{
		MSG( "连接MQTT服务器成功",OK_MSG );
		__m_is_connected = true;
	}

	return OK;
}

void mqttClient :: reconnect( const std::string& cid , const std::string& pswd )
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if( cid.empty() == false ){
		conn_opts.username = cid.c_str();
	}
	if( pswd.empty() == false ){
		conn_opts.password = pswd.c_str();
	}

	int rc = MQTTClient_connect(__m_client, &conn_opts);
	
	if( rc != MQTTCLIENT_SUCCESS ){
		ERROR_MSG( "连接MQTT服务器失败" );
		throw ERR_CONN;
	}else{
		MSG( "重新连接MQTT服务器成功",OK_MSG );
		__m_is_connected = true;
	}
}

std::string
mqttClient :: errMsg( emErrCode e )
{
	std::string ret;
	switch( e ) {
	case ERR_ALLOC_MEM:
		ret = "内存分配失败";
		break;
	case ERR_USR_PSWD:
		ret = "用户名或者密码错误";
		break;
	case ERR_CONN:
		ret = "连接失败";
		break;
	case ERR_CREATE_CTX:
		ret = "创建MQTT句柄失败";
		break;
	case ERR_PUB:
		ret = "发布信息失败";
		break;
	case ERR_SUB:
		ret = "订阅主题操作失败";
		break;
	case ERR_INSERT_CB:
		ret = "添加订阅回调记录失败, 可能是因为主题重复。";
		break;
	case ERR_UNSUB:
		ret = "取消主题订阅操作失败";
		break;
	case ERR_QOS_RANGE:
		ret = "QoS取值错误";
		break;
	case ERR_DATA_NULL:
		ret = "数据指针为空指针";
		break;
	case ERR_URL_EMPTY:
		ret = "URL内容为空";
		break;
	case ERR_CID_EMPTY:
		ret = "CID内容为空";
		break;
	case ERR_ACCOUNT_EMPTY:
		ret = "账号信息为空";
		break;
	default:break;
	}
	return ret;
}

mqttClient :: ~mqttClient()
{
	MQTTClient_destroy(&__m_client);
}

void mqttClient :: setDisconnect( std::function< void ( ) > fun )
{
	__m_on_disconnected = fun;
}

void mqttClient :: onDisconnect( const std::string& cause )
{
	std::string info( "MQTT连接断开:" );
	
	MSG( info + cause , TRED );
	if( __m_on_disconnected ){
		__m_on_disconnected();
	}
}

mqttClient :: emErrCode
mqttClient :: pub( const std::string& topic , const uint8_t * data , size_t len )
{
	emErrCode ret = OK;
	MQTTClient_deliveryToken dt;
	stMsg msg( data , len );
	//std::unique_lock< std::mutex > l( __m_mutex );
	int rc = MQTTClient_publishMessage( __m_client , topic.c_str() , &msg , &dt );
	if( rc == MQTTCLIENT_SUCCESS ){
	        MQTTClient_waitForCompletion( __m_client , dt , __m_wait_timeout.load() );
	}else{
		ERROR_MSG( MQTTClient_strerror( rc ) );
		ret = ERR_PUB;
	}
	return ret;
}

mqttClient :: emErrCode
mqttClient :: pub( const std::string& topic, stMsg& msg )
{
	emErrCode ret = OK;
	MQTTClient_deliveryToken dt;

	int rc = MQTTClient_publishMessage( __m_client , topic.c_str() , &msg , &dt );
	if( rc == MQTTCLIENT_SUCCESS ){
		rc = MQTTClient_waitForCompletion( __m_client , dt , __m_wait_timeout.load() );

		if( rc != MQTTCLIENT_SUCCESS ){
			ERROR_MSG( MQTTClient_strerror( rc ) );
			ret = ERR_PUB;
		}
	}else{
		ERROR_MSG( MQTTClient_strerror( rc ) );
		ret = ERR_PUB;
	}
	return ret;
}

mqttClient :: emErrCode
mqttClient :: sub( const std::string& topic , int qos , std::function< void ( const std::string& tp , const stMsg& ) > cb )
{
	emErrCode ret = OK;
	try{
		auto rst = __m_msg_cb.insert( std::make_pair( topic , cb ) );
		if( rst.second == false ){
			ret = ERR_INSERT_CB;
			ERROR_MSG( std::string("添加订阅回调记录失败, 可能是因为主题重复。") );
		}else{
			MSG_1( "添加订阅 %s 通知回调函数成功" , OK_MSG , topic.c_str());
		}

	}catch( std::bad_alloc& e ){
		ERROR_MSG( std::string("添加订阅回调记录失败：") + e.what() );
		ret = ERR_ALLOC_MEM;
	}
	
	int rc = MQTTClient_subscribe( __m_client , topic.c_str() , qos );
	if( rc != MQTTCLIENT_SUCCESS ){
		
		__m_msg_cb.erase( topic );
		
		return ERR_SUB;
	}

	return ret;
}

mqttClient :: emErrCode
mqttClient :: unSub( const std::string& topic )
{
	int rc = MQTTClient_unsubscribe( __m_client , topic.c_str() );
	if( rc != MQTTCLIENT_SUCCESS ){
		return ERR_UNSUB;
	}

	auto it = __m_msg_cb.find( topic );
	if( it != __m_msg_cb.end() ){
		__m_msg_cb.erase( it );
	}

	return OK;
}

bool mqttClient :: isConnected()
{
	bool ret = false;
	ret = __m_is_connected.load();
	return ret;
}

int mqttClient ::onRecv( const std::string& tp , const stMsg& msg )
{
	int ret = 0;

	auto it = __m_msg_cb.find( tp );
	if( it != __m_msg_cb.end() ){
		it->second( tp , msg );
		ret = 1;
	}else{
		MSG_1( "找不到订阅 %s 的回调通知函数" , TRED , tp.c_str() );
	}
	return ret;
}
