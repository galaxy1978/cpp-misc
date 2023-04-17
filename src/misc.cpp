
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>

#include <iostream>
#include <string>
#include <regex>
#include <sstream>

#if defined( __LINUX__ )
#    include <sys/time.h>
#    include <sys/socket.h>
#    include <sys/types.h>
#    include <sys/ioctl.h>
#    include <netdb.h>
#    include <net/if.h>
#    include <arpa/inet.h>
#    include <netinet/in.h>

#elif defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
#    include <winsock2.h>
#    include <ws2tcpip.h>
#    include <iphlpapi.h> /* From the Microsoft Platform SDK */
#    include <iprtrmib.h>
#    include <windows.h>
#endif
#if USE_ICONV == 1
#    include <iconv.h>
#endif

#include "misc.hpp"
// 全局LOG数据格式化缓冲区

#if defined(__cplusplus)
extern "C"{
#endif
char G_LOG_BUF[ 1024 ];
#if defined(__cplusplus)
}
#endif

hash_t hash_(char const* str)
{
	hash_t ret{__basis};

	while(*str){
		ret ^= *str;
		ret *= __prime;
		str++;
	}

	return ret;
}

hash_t hash_( char const *str , size_t len )
{
	hash_t ret{__basis};
	size_t i = 0;
	while( i < len ){
		ret ^= str[ i ];
		ret *= __prime;
		i ++;
	}

	return ret;
}

void log_base( bool enable , std::ostream& os , const std::string& str )
{
	 if( enable ){
		os << str;
	}
}

void logln_base( bool enable , std::ostream& os , const std::string& str )
{
	if( enable ){
		os << str << std::endl;
	}
}

#if defined( __WIN32 ) || defined( __WIN64 )
BYTE *getARPTable(DWORD DestIP)
{
	static BYTE buffMAC[6];
	BOOL fOrder = TRUE;
	DWORD status;

	MIB_IPNETTABLE *pIpNetTable = NULL;
	DWORD Size = 0;

	memset(buffMAC, 0, sizeof(buffMAC));
	// 应该连接Iphlpapi
	status = GetIpNetTable(NULL, &Size, fOrder);
	if (status == ERROR_INSUFFICIENT_BUFFER){
		pIpNetTable = ( MIB_IPNETTABLE * )malloc(Size);
		status = GetIpNetTable(pIpNetTable, &Size, fOrder);
	}

	if (status == NO_ERROR){
		DWORD i, ci = pIpNetTable->table[0].dwIndex;  /* set current interface */

		for (i = 0; i < pIpNetTable->dwNumEntries; ++i){
			if (pIpNetTable->table[i].dwIndex != ci){
				ci = pIpNetTable->table[i].dwIndex;
			}

			if(pIpNetTable->table[i].dwAddr == DestIP){ /* found IP in arp cache */
				memcpy(buffMAC, pIpNetTable->table[i].bPhysAddr, sizeof(buffMAC));
				if( pIpNetTable ){
					free( pIpNetTable );
				}
				return buffMAC;
			}
		}
	}
	if( pIpNetTable ){
		free( pIpNetTable );
	}
	return(NULL);
}

void ps( const std::string& pattern , std::function< void ( std::vector<std::string>&& ) > fun )
{
	std::stringstream ss;
	if( !pattern.empty() ){
		ss << "tasklist " << " | findstr /C:" << pattern;
	}else{
		ss << "tasklist";
	}
	std::string str = ExeCmd( ss.str() );

	std::vector< std::string > rst;

	::split( str , '\n' , rst );

	if( fun ){
		fun( std::move( rst ));
	}
}

std::string getLocalIP( bool ip4 )
{
	std::string ret;
	//2.获取主机名  
	char hostname[256];
	int e = gethostname( hostname , sizeof( hostname ) );
	if( e == SOCKET_ERROR ) return ret;

	//3.获取主机ip
	HOSTENT* host=gethostbyname(hostname);
	if (host==NULL){
		return ret;
	}
	memset( hostname , 0 , 256 );
	//4.转化为char*并拷贝返回
	if( ip4 == true ){
		inet_ntop( AF_INET , (void*)*host->h_addr_list , hostname , 256 );
	}else{
		inet_ntop( AF_INET6 , (void*)*host->h_addr_list , hostname , 256 );
	}

	ret = hostname;
    return ret;
}
#elif defined( __LINUX__ )
std::string ExeCmd(std::string pszCmd)
{
	std::string ret;
	MSG_1( "执行指令：%s" , TNORMAL , pszCmd.c_str() );
	FILE * fp = popen( pszCmd.c_str() , "r");
	if( fp ){
		char * buff = ( char *)malloc( 2048 );
		if( buff == nullptr ){
			pclose( fp );
			return ret;
		}
		char * rst = nullptr;
		do{
			rst = fgets( buff , 2047 , fp );
			if( rst != nullptr ){
				ret += buff;
			}
		}while( rst != nullptr );
		rst = nullptr;
		free( buff );
		pclose( fp );
	}
	return ret;
}


bool execCmd(std::string pszCmd , std::function< void ( const std::string& )> fun)
{
	bool ret = true;

	std::thread thd( [=]{
		std::string ret;
		FILE * fp = popen( pszCmd.c_str() , "r");
		if( fp == nullptr ) return;
		char * buff = ( char *)malloc( 2048 );
		if( buff == nullptr ){
			pclose( fp );
			return;
		}
		do{
			char * rst = fgets( buff , 2047 , fp );
			if( rst != nullptr ){
				ret += std::string( rst );
				fun( ret );
				ret = "";
			}

			std::this_thread::sleep_for( std::chrono::milliseconds(200));
		}while( !ferror( fp ) );

		free( buff );
		pclose( fp );
	});

	thd.detach();

	return ret;
}

std::string cpuID()
{
	std::string ret;

	ret = ExeCmd("cat /proc/cpuinfo");

	if( !ret.empty() ){

		ArrayString a_str;
		::split( ret , '\n' , a_str );


		std::regex reg( R"((?:\s+)[0-9a-fA-F]+)" );
		
		for( auto i : a_str ){
			std::smatch sm;
			if( std::regex_search( ret , sm , reg ) == true ){
				ret = sm[ 0 ];
			}
		}
	}

	for( size_t pos = ret.find(" "); pos != std::string::npos; pos =  ret.find(" ") ){
		ret.erase( pos , 1 );
	}
	return ret;
}

void ps( const std::string& pattern , std::function< void ( std::vector<std::string>&& ) > fun )
{
	std::stringstream ss;
	if( !pattern.empty() ){
		ss << "ps -ef " << " | grep " << pattern << " | grep -v grep";
	}else{
		ss << "ps -ef";
	}
	std::string str = ExeCmd( ss.str() );

	std::vector< std::string > rst;

	::split( str , '\n' , rst );

	if( fun ){
		fun( std::move( rst ));
	}
}


std::string localIP( const std::string& eth )
{
	std::string  ret;
	register int fd;
	struct ifreq ifr;

	if( ( fd=socket(AF_INET, SOCK_DGRAM, 0)) > 0){
		strcpy(ifr.ifr_name, eth.c_str() );
		if (!(ioctl(fd, SIOCGIFADDR, &ifr))){
			ret = std::string(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		}
	}

	if (fd > 0){
		close(fd);
	}
	return ret;
}
#endif

#if defined __DEBUG__
void dump( const char* data , size_t s )
{
	for( size_t i = 0; i < s; i ++ ){
		printf(" 0X%02X" , (uint8_t)data[ i ]);
		if( i % 16 == 0 && i != 0) puts("");
	}
	printf( "\n" );
}

void dump_2( const char* data , size_t s )
{
	std::string str( data );
	printf("[RECV DATA] %s\n" , str.c_str() );
}
#endif
#if USE_ICONV == 1
int gbk2utf8( const char * from , size_t len , char ** rst , size_t & rlen )
{
	if( from == nullptr ) return -3;
	if( len == 0 ) return -4;
	if( *rst == nullptr ){
		*rst = ( char *)malloc( len * 2 );
		__show_line_2();
		memset( *rst , 0 , len * 2 );
	}

	if( *rst == nullptr ) return -1;
	char * out = *rst , * pin = ( char *)from;
	int ret = 0;
	size_t inlen = len;
	rlen = len * 2 ;

	iconv_t ctx;
	ctx = iconv_open( "UTF-8" , "GBK" );
	if( ctx == 0 ) return -2;

	ret = iconv( ctx , &pin , &inlen , &out , &rlen );

	if( ret > 0 ){
		__show_line_2();
		rlen = strlen( *rst );
		__show_line_2();
		ret = 0;
	}

	iconv_close( ctx );
	return ret;
}

/**
 * @brief 将UTF8字符转为GBK
*/
size_t utf2gbk( const std::string& msg , char ** data )
{
#if !defined( _MSC_VER )
	iconv_t ctx = iconv_open( "GBK" , "UTF-8" );

	if( ctx ){
		size_t il = msg.length() , ol = il;

		char * df= (char *)(msg.c_str());
		if( *data == nullptr ){
			*data = ( char *)malloc( msg.length() );
			if( data == nullptr ){
				return 0;
			}

			char * dt = *data;

			memset( *data , 0 , msg.length() );
            iconv( ctx ,&df , &il , &dt , &ol );
		}
		iconv_close( ctx );
		size_t ret = strlen( *data );
		return ret;
	}
	return 0;
#else
	size_t ret = 0;

	return ret;
#endif
}
#endif

std::string nowStr()
{
	char data[ 100 ];
#if defined( __WIN32 ) || defined( __WIN64 )
	std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::time_t timestamp = tmp.count();

	std::time_t milli = timestamp+ (std::time_t)8*60*60*1000;//此处转化为东八区北京时间，如果是其它时区需要按需求修改
	auto mTime = std::chrono::milliseconds(milli);
	auto tp1=std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds>(mTime);
	auto tt = std::chrono::system_clock::to_time_t(tp1);
	std::tm* now = std::gmtime(&tt);
	sprintf( data , "%4d-%02d-%02d %02d:%02d:%02d.%03lld",now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec, milli%1000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tv.tv_sec = tv.tv_sec + 8 * 3600;
	std::tm * now = std::gmtime( &tv.tv_sec );
	sprintf( data , "%4d-%02d-%02d %02d:%02d:%02d.%03ld",now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec, tv.tv_usec / 1000);
#endif
	return std::string( data );
}

bool reverseByte(void* data, size_t len)
{
	if (len & 0x1) return false;
	if (data == nullptr) return false;
	if (len == 0) return false;

	uint8_t* p_d = (uint8_t*)data;


	for (size_t i = 0; i < len / 2; i++) {
		uint8_t tmp = 0;
		tmp = p_d[ i ];
		p_d[i] = p_d[len - 1 - i];
		p_d[len - 1 - i] = tmp;
	}

	return true;
}

bool isIPv4(const std::string& ipstr)
{
	if (ipstr.empty()) return false;
	std::regex reg(R"(^((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}$)");
	if (std::regex_match(ipstr, reg) == true) {
		return true;
	}

	return false;
}

bool isIPv6(const std::string& ipstr)
{
	if (ipstr.empty()) return false;
	std::regex reg(R"(^([\da-fA-F]{1,4}:){6}((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)|::([\da−fA−F]1,4:)0,4((25[0−5]|2[0−4]\d|[01]?\d\d?)\.)3(25[0−5]|2[0−4]\d|[01]?\d\d?)|^([\da-fA-F]{1,4}:):([\da-fA-F]{1,4}:){0,3}((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)|([\da−fA−F]1,4:)2:([\da−fA−F]1,4:)0,2((25[0−5]|2[0−4]\d|[01]?\d\d?)\.)3(25[0−5]|2[0−4]\d|[01]?\d\d?)|^([\da-fA-F]{1,4}:){3}:([\da-fA-F]{1,4}:){0,1}((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)|([\da−fA−F]1,4:)4:((25[0−5]|2[0−4]\d|[01]?\d\d?)\.)3(25[0−5]|2[0−4]\d|[01]?\d\d?)|^([\da-fA-F]{1,4}:){7}[\da-fA-F]{1,4}|:((:[\da−fA−F]1,4)1,6|:)|^[\da-fA-F]{1,4}:((:[\da-fA-F]{1,4}){1,5}|:)|([\da−fA−F]1,4:)2((:[\da−fA−F]1,4)1,4|:)|^([\da-fA-F]{1,4}:){3}((:[\da-fA-F]{1,4}){1,3}|:)|([\da−fA−F]1,4:)4((:[\da−fA−F]1,4)1,2|:)|^([\da-fA-F]{1,4}:){5}:([\da-fA-F]{1,4})?|([\da−fA−F]1,4:)6:)");
	if (std::regex_match(ipstr, reg) == true) {
		return true;
	}

	return false;
}

time_t parseGMT( const std::string& gmt , int loc )
{
	time_t ret = 0;
	if( gmt.empty() ){
		ERROR_MSG("GMT string should not be empty.");
		throw -1;
	}

	int y = 0, m = 0 , d = 0 , h = 0 , min = 0 , s = 0;
	std::regex reg("Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec");
	std::smatch sm;
	// 月
	if( std::regex_search( gmt , sm , reg ) == true ){
		std::string mon = sm[ 0 ];
		switch( hash_( mon.c_str() )){
		case "Jan"_hash:m = 1;	break;
		case "Feb"_hash:m = 2;	break;
		case "Mar"_hash:m = 3;	break;
		case "Apr"_hash:m = 4;	break;
		case "May"_hash:m = 5;	break;
		case "Jun"_hash:m = 6;	break;
		case "Jul"_hash:m = 7;	break;
		case "Aug"_hash:m = 8;	break;
		case "Sep"_hash:m = 9;	break;
		case "Oct"_hash:m = 10;	break;
		case "Nov"_hash:m = 11;	break;
		case "Dec"_hash:m = 12;	break;
		}
	}
	// 时间
	std::regex reg_time( R"(\d+:\d+:\d+(?=\s+))" );
	if( std::regex_search( gmt , sm , reg_time ) == true ){
		std::string str = sm[ 0 ];
		sscanf( str.c_str() , "%d:%d:%d" , &h , &min , &s );
	}


	// 年
	std::regex reg_year( R"(\d{4}(?=\s+))" );
	if( std::regex_search( gmt , sm , reg_year ) == true ){
		std::string str = sm[ 0 ];
		y = atoi( str.c_str() );
	}
	// 日
	std::regex reg_date(R"((Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\s+\d+(?=\s+))");
	if( std::regex_search( gmt , sm , reg_date ) == true ){
		std::string str = sm[ 0 ];
		std::regex reg_date_( "\\d+" );
		if( std::regex_search( str , sm , reg_date_ ) == true ){
			str = sm[ 0 ];
			d = atoi( str.c_str());
		}
	}

	struct tm _tm;
	memset( &_tm , 0 , sizeof( tm ));
	_tm.tm_year = y - 1900;
	_tm.tm_mon = m - 1;
	_tm.tm_mday = d;
	_tm.tm_hour = h;
	_tm.tm_min = min;
	_tm.tm_sec = s;

	ret = mktime( &_tm );
	ret += 3600 * loc;              // 调整时区
	return ret;
}


bool fileNameFromPath( const std::string& path , std::string& name )
{
	bool ret = false;
	if( access( path.c_str() , 0 ) == 0 ){
		if( isDir( path ) == false ){
#if defined( WIN32 ) || defined( __WIN64 ) || defined( WINNT )
			name = path.substr( path.find_last_of( '\\' ) );
			ret = true;
#elif defined( __LINUX__ )
			name = path.substr( path.find_last_of( '/' ) );
			ret = true;
#endif
		}else{
			throw -1;
		}
	}else{
#if defined( WIN32 ) || defined( __WIN32 ) || defined( __MINGW32__ ) || defined( __MINGW64__ ) || defined( __WIN64 ) || defined( __WIN__ ) || defined( __WXMSW__ )
		if( path.back() != '\\' ){
			name = path.substr( path.find_last_of( '\\' ) );
			ret = true;
		}
#elif defined( __LINUX__ )
		if( path.back() != '/' ){
			name = path.substr( path.find_last_of( '/' ) );
			ret = true;
		}
#endif
	}

	return ret;
}

size_t getFileSize( const std::string& path )
{
	size_t   ret = 0;
	struct stat   statbuf;

	if( stat( path.c_str() , &statbuf ) == 0 ){
                ret = statbuf.st_size;
	}
	return ret;
}


#if defined( __WIN32 ) || defined( __WIN64 )
std::wstring Str2Wstr(std::string str)
{
	unsigned len = str.size() * 2;// 预留字节数
	setlocale(LC_CTYPE, "");     //必须调用此函数
	wchar_t *p = new wchar_t[len];// 申请一段内存存放转换后的字符串
	mbstowcs(p, str.c_str(), len);// 转换
	std::wstring str1(p);
	delete[] p;// 释放申请的内存
	return str1;
}

std::string ExeCmd(std::string pszCmd)
{
#if defined( _UNICODE ) || defined( UNICODE )	
	wchar_t * pszCmd_w = (wchar_t *)Str2Wstr(pszCmd).c_str();
#else
	char * pszCmd_w = ( char*)pszCmd.c_str();
#endif

	// 创建匿名管道,write->read;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)){return (" ");	}

	// 设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite
	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; //隐藏窗口；
	si.hStdError = hWrite;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite; //管道的输入端口连接命令行的输出；
	// 启动命令行
	PROCESS_INFORMATION pi;// Pointer to PROCESS_INFORMATION structure;
	if (!CreateProcess(NULL, pszCmd_w , NULL,NULL,TRUE, 0 ,NULL,NULL,&si,&pi)){
		return ("Cannot create process");
	}
	CloseHandle(hWrite);//关闭管道的输入端口；
	// 读取命令行返回值
	std::string strRetTmp;
	char buff[1024] = { 0 };
	DWORD dwRead = 0;
	strRetTmp = buff;
	while (ReadFile(hRead, buff, 1024, &dwRead, NULL)){//从管道的输出端获取命令行写入的数据；
		strRetTmp += buff;
	}
	CloseHandle(hRead);//关闭管道的输出端口；

	return strRetTmp;
}

bool execCmd(std::string pszCmd , std::function< void ( const std::string& )> fun)
{
#if defined( _UNICODE ) || defined( UNICODE )	
	wchar_t * pszCmd_w = (wchar_t *)Str2Wstr(pszCmd).c_str();
#else
	char * pszCmd_w = ( char*)pszCmd.c_str();
#endif

	// 创建匿名管道,write->read;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)){
		return false;
	}

	// 设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite
	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;		//隐藏窗口；
	si.hStdError = hWrite;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;			//管道的输入端口连接命令行的输出；
	// 启动命令行
	PROCESS_INFORMATION pi;// Pointer to PROCESS_INFORMATION structure;
	if(!CreateProcess(NULL, pszCmd_w,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)){
		return false;
	}
	CloseHandle(hWrite);//关闭管道的输入端口；
	// 读取命令行返回值
	std::thread thd([=]{
		std::string strRetTmp;
		char * buff = ( char *)malloc( 2048 );
		DWORD dwRead = 0;

		while (ReadFile(hRead, buff, 2047, &dwRead, NULL)){//从管道的输出端获取命令行写入的数据；
			if (dwRead > 0) {
				std::string strRetTmp(buff, dwRead);
				fun(strRetTmp);
			}
		}
		CloseHandle(hRead);//关闭管道的输出端口；
	} );

	thd.detach();
	return true;
}

std::string cpuID()
{
	std::string ret;

	ret = ExeCmd("wmic CPU get ProcessorID");

	if( !ret.empty() ){

		for( size_t pos = ret.find("\n"); pos != std::string::npos; pos =  ret.find("\n") ){
			ret.erase( pos , 1 );
		}

		for( size_t pos = ret.find("\r"); pos != std::string::npos; pos =  ret.find("\r") ){
			ret.erase( pos , 1 );
		}


		std::regex reg( R"((?:\s+)[0-9a-fA-F]+)" );
		std::smatch sm;
		if( std::regex_search( ret , sm , reg ) == true ){
			ret = sm[ 0 ];
		}
	}

	for( size_t pos = ret.find(" "); pos != std::string::npos; pos =  ret.find(" ") ){
		ret.erase( pos , 1 );
	}
	return ret;
}

#endif

int split( const std::string& str , char c , std::vector< std::string >& rst )
{
	int ret = 0;
	size_t pos = 0;
	std::string str1 , str2 = str;
	while( ( pos = str2.find( c )) != std::string::npos ){
		str1 = str2.substr( 0 , pos );
		str2 = str2.substr( pos + 1 );
		if( !str1.empty() ){
			rst.push_back( str1 );
		}
	}

	if( !str2.empty() ){
		rst.push_back( str2 );
	}

	ret = rst.size();
	return ret;
}
