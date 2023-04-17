#include <windows.h>
#include <devguid.h>
#include <setupapi.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <regex>
#include <list>
#include <map>
#include <string>

#if defined( _UNICODE ) || defined( UNICODE )
#    include <locale>
#    include <codecvt>
using SERIAL_CHAR_T = wchar_t;
#else
using SERIAL_CHAR_T = char;
#endif

#include "details/iSerial.hpp"
#include "details/winSerial.hpp"
#include "misc.hpp"


using namespace wheels;
/*
  2018-5-10   添加异步回调处理
*/
// 事件索引，使用自动增加的方式生成事件ID，并记录事件在一个
uint32_t oid = 0;

struct currentEvt{
	uint32_t       id;
	winSerial    * ptobj;
	LPOVERLAPPED   ptovlp;

	currentEvt( winSerial * ser , LPOVERLAPPED ovlp ){
		id     = oid;
		ptobj  = ser;
		ptovlp = ovlp;

		oid    ++;
	}

	~currentEvt(){}

	currentEvt( const currentEvt& evt ){
		id = evt.id;
		ptobj = evt.ptobj;
		ptovlp = evt.ptovlp;
	}

	currentEvt& operator=( const currentEvt& evt ){
		id = evt.id;
		ptobj = evt.ptobj;
		ptovlp = evt.ptovlp;
		return *this;
	}
};

typedef std::map< HANDLE , currentEvt >  evtQue;

#define DEFAULT_BUFF_SIZE (1024)
#define DEFAULT_OVERFLOW_TIME  (5000)
evtQue   globeEvtQue;
/**
 * @brief windows API 的异步回调操作函数
 * @param err
 * @param len
 * @param ovlp
 */
void CALLBACK on_serial_read( DWORD err , DWORD len , LPOVERLAPPED ovlp )
{
	winSerial * obj = nullptr;
	MSG( "****************" , OK_MSG );
	evtQue::iterator it = globeEvtQue.find( ovlp->hEvent );

	if( it != globeEvtQue.end() ){
		obj = it->second.ptobj;
		globeEvtQue.erase( it );

		if( obj != nullptr ){
			obj->__on_recv( ovlp );
		}
   	}else{
		ERROR_MSG( "事件对象没有登记" );
	}
	
	CloseHandle( ovlp->hEvent );
}

void CALLBACK on_serial_write( DWORD err , DWORD len , LPOVERLAPPED ovlp )
{
	winSerial * obj = nullptr;

	evtQue::iterator it = globeEvtQue.find( ovlp->hEvent );

	if( it != globeEvtQue.end() ){
		obj = it->second.ptobj;
		globeEvtQue.erase( it );

		if( obj != nullptr ){
			obj->__on_write( err , len );
		}
   	}else{
		ERROR_MSG( "事件对象没有登记" );
	}
	
	CloseHandle( ovlp->hEvent );
	free( ovlp );
}

winSerial :: winSerial( const std::string& port ):
	__p_buff( nullptr ), __p_buff_1( nullptr ) , __p_buff_2( nullptr )
{
#if defined( _UNICODE )	|| defined( UNICODE )
	std::wstring_convert< std::codecvt_utf8< wchar_t> > cvt;
	std::wstring  __str = cvt.from_bytes( port );
#else
	std::string   __str = port;
#endif
	__m_dead_line_time    = DEFAULT_OVERFLOW_TIME;
	__m_is_stoped         = true;
	__m_is_open           = false;
	__p_buff              = NULL;
	__m_buff_size         = DEFAULT_BUFF_SIZE;
	__m_is_run            = false;
	__m_last_read         = 0;

	memset( __m_port , 0 , 64 * sizeof(SERIAL_CHAR_T) );

	__p_buff_1 = ( char *)malloc( DEFAULT_BUFF_SIZE );
	__p_buff_2 = ( char *)malloc( DEFAULT_BUFF_SIZE );
	__p_buff = __p_buff_1;

	
#if defined( _UNICODE ) || defined( UNICODE )
	if( port.length() > 4 )
		__str = std::wstring(L"\\\\.\\") + __str;
#else
	if( port.length() > 4 )
		__str = std::string("\\\\.\\") + __str;
#endif
	memcpy( __m_port , __str.c_str() , __str.length() * sizeof(SERIAL_CHAR_T ));
}

winSerial :: winSerial( const std::string& port , int baud , int d_len ,  stop::value s , parity::value pa , flow::value f ):
	__p_buff( nullptr ), __p_buff_1( nullptr ) , __p_buff_2( nullptr )
{
	memset( __m_port , 0 , 64 * sizeof(SERIAL_CHAR_T));
#if defined( _UNICODE ) || defined( UNICODE )
	std::wstring_convert< std::codecvt_utf8< wchar_t> > cvt;
	std::wstring  __str = cvt.from_bytes( port );
#else
	std::string   __str = port;
#endif

	__m_dead_line_time    = DEFAULT_OVERFLOW_TIME;
	__m_is_stoped         = true;
	__m_is_open           = false;
	__p_buff              = NULL;
	__m_buff_size         = DEFAULT_BUFF_SIZE;
	__m_is_run            = false;
	__m_last_read         = 0;

	__p_buff_1 = ( char *)malloc( DEFAULT_BUFF_SIZE );
	__p_buff_2 = ( char *)malloc( DEFAULT_BUFF_SIZE );
	__p_buff = __p_buff_1;

#if defined( _UNICODE ) || defined( UNICODE )
	if( port.length() > 4 )
		__str = std::wstring(L"\\\\.\\") + __str;
#else
	if( port.length() > 4 )
		__str = std::string("\\\\.\\") + __str;
#endif
	memcpy( __m_port , __str.c_str() , __str.length() * sizeof(SERIAL_CHAR_T ));
	__m_is_open = __do_open_port( baud , d_len , pa , s , f );

	if( __m_is_open == true ){
		__m_error = OK;
	}
}

winSerial :: ~winSerial()
{
	bool   __flag     = false;
	bool   __is_open  = false;

	__flag    = __m_is_run;
	__is_open = __m_is_open;
	PurgeComm( __m_hPort , PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR );

	if( __flag ){
		__do_reset_io();
	}

	__p_buff = nullptr;
	free( __p_buff_1 );
	free( __p_buff_2 );

	__m_is_stoped = false;
	while( __is_open == true && __m_is_stoped.load() == false ){
		std::this_thread::yield();
		std::this_thread::sleep_for( std::chrono::milliseconds(1));
	}
	if( __is_open ){
		CloseHandle( __m_hPort );
	}
}


void winSerial :: __on_recv( LPOVERLAPPED povlped )
{
	DWORD len;

	if( GetOverlappedResult( __m_hPort , povlped , &len , false ) == TRUE ){
		char * tmp_buff = __p_buff;
		// 交换缓冲区
		if( __p_buff == __p_buff_1 ){
			__p_buff = __p_buff_2;
		}else{
			__p_buff = __p_buff_1;
		}

		if( on_recv ){
			on_recv( len , tmp_buff , __m_error.load() );
		}
	}

	__do_real_readEx( __p_buff , __m_buff_size );
}

void winSerial :: __do_set_stopbits( HANDLE h ,stop::value s , LPDCB __dcb )
{
	switch( s ){
	case stop::ONE:
	default:
		__dcb->StopBits = ONESTOPBIT;
		break;
	case stop::ONE_P_FIVE:
		__dcb->StopBits = ONE5STOPBITS;
		break;
	case stop::TWO:
		__dcb->StopBits = TWOSTOPBITS;
		break;
	}
}

void winSerial :: __do_set_parity( HANDLE h , parity::value p , LPDCB __dcb )
{
	switch( p ){
	case parity::NONE:
	default:
		__dcb->Parity = NOPARITY;
		break;
	case parity::ODD:
		__dcb->Parity = ODDPARITY;
		break;
	case parity::EVEN:
		__dcb->Parity = EVENPARITY;
	}
}

void winSerial :: __do_set_flow( HANDLE h , flow::value f , LPDCB dcb )
{
	switch( f ){
	case flow::NONE:
	default:
		break;
	case flow::HARD:
		break;
	case flow::SOFT:
		break;
	}
}

bool winSerial :: __do_open_port( int baud , int d_len , parity::value p , stop::value s , flow::value f )
{
	bool    __ret = false;

	__m_hPort = CreateFile( __m_port ,                      //串口名称 ,
				GENERIC_READ|GENERIC_WRITE,       //操作方式
				0,                                //独占方式
				(LPSECURITY_ATTRIBUTES)NULL,      //安全属性
				OPEN_EXISTING,                    //串口必须存在
				FILE_FLAG_OVERLAPPED,             //使用异步方式打开串口
				NULL                              //
		);                                  	        //打开串口设备，设置串口为可读可写
	if( __m_hPort !=  INVALID_HANDLE_VALUE ){
		//设置超时
		struct _COMMTIMEOUTS   __timeouts;
		__timeouts.ReadIntervalTimeout            = 20;       //出现这种情况都是数据包发送完成或者是发生错误
		__timeouts.ReadTotalTimeoutMultiplier     = 2;        //
		__timeouts.ReadTotalTimeoutConstant       = 0;
		__timeouts.WriteTotalTimeoutMultiplier    = 0;
		__timeouts.WriteTotalTimeoutConstant      = 0;

		SetCommTimeouts( __m_hPort , &__timeouts );
		//设置缓冲区
		SetupComm( __m_hPort , 1024 , 1024 );
		//设置波特率，等信息
		DCB __dcb;
		if( GetCommState( __m_hPort , &__dcb ) ){
			__dcb.BaudRate = baud;
			__dcb.ByteSize = d_len;
			__do_set_stopbits( __m_hPort , s , &__dcb );
			__do_set_parity( __m_hPort , p , &__dcb );
			SetCommMask( __m_hPort , EV_RXFLAG );

			if( SetCommState( __m_hPort , &__dcb ) ){//清空缓冲区
				PurgeComm( __m_hPort , PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);
				__m_is_open = true;
				__m_error = OK;
			}else{
				DWORD err = GetLastError();
				__do_process_error( err );
				__m_is_open = false;
				__m_error = SET_COM_FAIL;
			}
		}else{
			__m_is_open = false;
			__m_error   = GET_COM_FAIL;

		}
	}else{//串口打开失败
		__m_is_open = false;
		ERROR_MSG( "打开串口失败." );
        	__m_error = ERR_OPEN_SERIAL;
	}
    	__ret = __m_is_open;

    	if( on_open ){
		on_open( __m_error );
        }
    	return  __ret;
}
size_t winSerial :: __write( const char *buff , size_t len )
{
	size_t ret = len;
	if( buff == NULL ){
		ret = 0;
		return ret;
	}
	LPOVERLAPPED p_ovlped = nullptr;
	p_ovlped = ( LPOVERLAPPED )malloc( sizeof( OVERLAPPED ) );
	if( p_ovlped == nullptr ){
		ERROR_MSG( "内存分配失败" );
		return 0;
	}
	memset( p_ovlped , 0 , sizeof(OVERLAPPED) );
	p_ovlped->hEvent = CreateEvent( NULL , TRUE , FALSE , NULL );

	currentEvt  evt( this , p_ovlped );
	globeEvtQue.insert( std::make_pair(p_ovlped->hEvent , evt ) );
	
	if(WriteFileEx( __m_hPort, buff, len, p_ovlped, (LPOVERLAPPED_COMPLETION_ROUTINE)on_serial_write )== 0){
		DWORD err = GetLastError();
		__do_process_error( err );
		ret = 0;
		if( on_send ){
			on_send( 0 , ERR_SEND_FAIL );
		}
	}

    	return ret;
}

winSerial::err_code
winSerial :: __do_real_readEx( char * buff , size_t len )
{
	err_code ret = OK;
	
	LPOVERLAPPED  p_ovlped = nullptr;
	
	p_ovlped = ( LPOVERLAPPED )malloc( sizeof( OVERLAPPED ) );
	if( p_ovlped == nullptr ){
		ERROR_MSG( "内存分配失败" );
		return ERR_ALLOC_MEM;
	}
	memset( p_ovlped , 0 , sizeof( OVERLAPPED )) ;
	
	p_ovlped->hEvent = CreateEvent( NULL , TRUE , false , NULL );

	currentEvt  evt( this , p_ovlped );
	// 利用 p_ovlped->hEvent 找到 this 指针然后可以回调到对象内部
	globeEvtQue.insert( std::make_pair( p_ovlped->hEvent , evt ) );

	if( ReadFileEx( __m_hPort , buff , len , p_ovlped, (LPOVERLAPPED_COMPLETION_ROUTINE)on_serial_read ) == 0 ){
		DWORD err = GetLastError();
		__do_process_error( err );
		ret = ERR_READ_DATA;
	}else{
		MSG( "发起读请求成功" , OK_MSG );
	}

	return ret;
}

void winSerial :: __do_process_error( DWORD err )
{
	LPTSTR lpMsgBuf;
	char
#if defined( UNICODE ) || defined( _UNICODE )
		msg[ 512 ] ,
#endif
		* p_msg;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL , err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

#if defined( UNICODE ) || defined( _UNICODE )
	WideCharToMultiByte( CP_UTF8 , 0 ,lpMsgBuf , -1 ,  msg , 511 , NULL , NULL );
	p_msg = msg;
#else
	p_msg = ( char *)lpMsgBuf;
#endif
	ERROR_MSG( p_msg );

	LocalFree(lpMsgBuf);
}

void winSerial :: __do_reset_io()
{
	__m_is_run = false;
	CancelIoEx( __m_hPort , NULL );
}



void winSerial :: __on_write( DWORD err , DWORD len )
{
	if( on_send ){
		if( err == 0 ){
			on_send( (size_t)len , OK );
		}else{
			on_send( 0 , ERR_SEND_FAIL );
		}
	}
}

void winSerial :: Stop()
{
	__do_reset_io();
}

void EnumSerial( std::vector< std::string >& a_rst )
{
#if defined( _UNICODE ) || defined( UNICODE )
	LPCTSTR	        data_Set= L"HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
	std::wstring	str;
	wchar_t         Name[ 100 ];
	wchar_t         szPortName[ 100 ];

#else
	LPCTSTR	        data_Set= "HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
	std::string	str;
	char 	        Name[ 100 ];
	char            szPortName[ 100 ];

#endif
	HKEY		hKey;

	LONG		Status;
	DWORD		dwIndex = 0, dwName, dwSizeofPortName;

	long ret0 = (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, data_Set, 0, KEY_READ, &hKey));
	if(ret0 != ERROR_SUCCESS) return;
	dwSizeofPortName = sizeof(szPortName);
	dwName = sizeof(Name);

	do{
		Status = ::RegEnumValue(hKey, dwIndex, Name, &dwName, NULL, NULL , (PUCHAR)szPortName, &dwSizeofPortName);
		if((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA)){

#if defined( _UNICODE ) || defined( UNICODE )
			str = std::wstring( szPortName );
			std::wstring_convert< std::codecvt_utf8<wchar_t>> cvt;
			std::string str1 = cvt.to_bytes( str );
			a_rst.push_back( str1 );
#else
			str = std::string(szPortName);
			a_rst.push_back( str );
#endif
		}

		dwIndex++;
		dwSizeofPortName = sizeof(szPortName);
		dwName = sizeof(Name);
	} while((Status == ERROR_SUCCESS)||(Status == ERROR_MORE_DATA));

	RegCloseKey(hKey);
}

void EnumSerial( std::vector< stDevDesc >& a_rst )
{
	GUID portGUID = {0x4d36e978,0xe325,0x11ce,0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18};//GUID_DEVCLASS_PORTS;

	HDEVINFO infoset = SetupDiGetClassDevs( &portGUID , NULL , NULL , DIGCF_PRESENT );
	if( infoset != INVALID_HANDLE_VALUE ){
		SP_DEVINFO_DATA    data;
		DWORD              size;
		BYTE             * buff = ( BYTE*)malloc( 500 );
		bool               flag = true;
		stDevDesc          item;

		for( DWORD i = 0; flag ; i ++ ){
			if( SetupDiEnumDeviceInfo( infoset , i , &data ) ){

				if( SetupDiGetDeviceRegistryProperty( infoset , &data , SPDRP_DEVICEDESC , NULL , buff , 500 , &size ) ){  // 获取描述
					buff[ size ] = 0;
					item.desc = ( char *)buff;
				}

				if( SetupDiGetDeviceRegistryProperty( infoset , &data , SPDRP_FRIENDLYNAME , NULL , buff , 500 , &size ) ){  // 获取友好名称
					buff[ size ] = 0;
					item.friend_name = (char*)buff;
				}

				if( SetupDiGetDeviceRegistryProperty( infoset , &data , SPDRP_MFG , NULL , buff , 500 , &size ) ){  // 获取厂商
					buff[ size ] = 0;
					item.vendor = (char*)buff;

					std::regex reg( "COM\\d+");
					std::smatch sm;
					if( std::regex_search( item.vendor , sm , reg ) == true ){
						item.name = sm[ 0 ];
						flag = true;
					}
				}

				a_rst.push_back( item );
				item.clear();
            		}else{
				DWORD e = GetLastError();
				if( e  == ERROR_NO_MORE_ITEMS ){
					flag = false;
				}
            		}
        	}// end for
        	if( buff != nullptr ) free( buff );
    	}
}

iSerial :: err_code
winSerial :: open( int baud , int char_len , stop::value s , parity::value p , flow::value f )
{
	if( __do_open_port( baud , char_len , p , s , f ) == false ){
		return ERR_OPEN_SERIAL;
	}
	return OK;
}

iSerial :: err_code
winSerial :: close()
{
	__do_reset_io();

	CloseHandle( __m_hPort );

	return __m_error.load();
}

iSerial :: err_code
winSerial :: send( size_t len , const char* data )
{
	if( data == nullptr ){
		return ERR_DATA_NULL;
	}
	auto rst = __write( data , len );
	if( rst > 0 ){
		return OK;
	}
	return __m_error.load();
}

iSerial& winSerial :: evtOpen(  std::function< void ( err_code )> fun )
{
	on_open = fun;
	return *this;
}
iSerial& winSerial :: evtClose( std::function< void ( err_code )> fun )
{
	on_close = fun;
	return *this;
}
iSerial& winSerial :: evtSend(  std::function< void (size_t , err_code )> fun )
{
	on_send = fun;
	return *this;
}
iSerial& winSerial :: evtRecv(  std::function< void (size_t , const char* , err_code )> fun )
{
	on_recv = fun;
	return *this;
}

winSerial :: err_code
winSerial :: reset()
{
	__do_reset_io();
	return __m_error.load();
}

winSerial :: err_code
winSerial :: reset( int baud , int char_len , stop::value s , parity::value p , flow::value f )
{
	__do_reset_io();

	CloseHandle( __m_hPort );
	if( __do_open_port( baud , char_len , p , s , f ) == false ){
		return ERR_OPEN_SERIAL;
	}

	return OK;
}

bool winSerial :: is_open()
{
	return __m_is_open.load();
}

size_t winSerial :: __do_real_read( char * buff , size_t len , long ovt )
{
	DWORD    __ret = 0;
	memset( &m_ovlped , 0 ,  sizeof( OVERLAPPED ) );

	m_ovlped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);             //创建一个匿名的
	if( !ReadFile( __m_hPort , buff , len , &__ret , &m_ovlped ) ){
		//如果成功读取数据则返回，负责等候操作指导接收到数据
		DWORD _w_r = WaitForSingleObject( m_ovlped.hEvent , ovt );
		if( _w_r == WAIT_TIMEOUT ){//等候超时
			__ret = 0;
		}

		if( _w_r == WAIT_FAILED ){//等候失败
			DWORD err = GetLastError();
			__do_process_error( err );
		}

        	if( WAIT_OBJECT_0 == _w_r ){//正常响应，说明有数据到达开始读取数据
			if( !GetOverlappedResult( __m_hPort , &m_ovlped, &__ret , true ) ){
				DWORD err = GetLastError();
				__do_process_error( err );
			}
        	}
    	}else{
		::dump( buff , __ret );
	}
	buff[ __ret ] = 0;
    	return (DWORD)__ret;
}

void winSerial :: run( bool run )
{
	if( run == true ){
		/*CancelIo( __m_hPort );
		__do_real_readEx( __p_buff ,DEFAULT_BUFF_SIZE );
		*/
		std::thread thd([&]{
			while( true ){
				size_t len = __do_real_read( __p_buff ,1024 , 5000 );
				if( len > 0 && on_recv ){
					on_recv( len , __p_buff , OK );
				}
			}
		});
		thd.detach();
		
	}else{
		__do_reset_io();
	}
}

void winSerial :: standby()
{
	__do_reset_io();
}
