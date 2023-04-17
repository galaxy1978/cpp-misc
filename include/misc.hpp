/**
 * @brief 常用的一些工具算法或者定义
 * @note 现在这些函数不支持获取多个查询的URL内容
 * @version 1.2.4
 * @date 2017-2-8 ~ 2022-10-28
 * @author 宋炜
 */

/// 2021-02-01 1.2.0 ADDED 添加parseGMP函数
/// 2021-07-12 1.2.1 ADDED 添加字符串的split函数
/// 2021-08-02 1.2.2 ADDED 添加VT100颜色定义
/// 2021-9-30  1.2.3 ADDED 添加判断目录是否为空函数
///                  ADDED 添加删除文件接口
///                  ADDED 添加清理目录接口
/// 2022-10-28 1.2.4 ADDED 添加从字符串中抽取文件名

#ifndef __MISC_H__
#define __MISC_H__

#include <string>
#include <iostream>
#include <thread>
#include <functional>
#include <vector>

#include "fs.hpp"

/*
 * 终端输出颜色定义。方便输出错误或者录数据的分辨。 
 */
#if !defined(USE_GUI)
#	define TRED          "\033[31m"
#	define ERR_MSG       TRED
#	define TGREEN        "\033[32m"
#	define OK_MSG        TGREEN
#	define TBLUE         "\033[34m"
#	define TYELLOW       "\033[33m"
#	define WARNING_MSG   TYELLOW
#	define TNORMAL       "\033[0m"
#else
#	define TRED          ""
#	define ERR_MSG       TRED
#	define TGREEN        ""
#	define OK_MSG        TGREEN
#	define TBLUE         ""
#	define TYELLOW       ""
#	define WARNING_MSG   TYELLOW
#	define TNORMAL       ""
#endif
#if defined(__cplusplus)
extern "C"{
#endif
extern char G_LOG_BUF[ 1024 ];
#if defined(__cplusplus)
}
#endif

/**
* @brief 当前时间的字符串
*/
std::string nowStr();

// 再LOG输出的时候如果需要输出源代码的信息，将SHOW_FILE_INFO配置为1
#if (SHOW_FILE_INFO == 1)
#	define __FILE_name__ __FILE__
#       define __LINE_no__ __LINE__
#	define LEFT_BRACE " [ "
#	define RIGHT_BRACE " ] "
#else
#	define __FILE_name__ ""
#       define __LINE_no__ ""
#	define LEFT_BRACE ""
#	define RIGHT_BRACE ""
#endif

#if defined( __DEBUG__ )
	void dump( const char* data , size_t s );
	void dump_2( const char* data , size_t s );

    #define __show_line( str ) do{                                                                  \
		std::cerr <<  nowStr() << " " << __FILE_name__ << LEFT_BRACE << __LINE_no__ << RIGHT_BRACE	\
        << TRED << str << TNORMAL << std::endl;								                        \
	}while(0)

    #define __show_line_1( format , ... ) do{               \
          std::cerr << nowStr() <<  " "				        \
            << __FILE_name__				                \
            << LEFT_BRACE					                \
            << __LINE_no__					                \
            << RIGHT_BRACE					                \
            << TRED ;					                    \
		sprintf( G_LOG_BUF , format , __VA_ARGS__ );		\
		std::cerr << G_LOG_BUF << TNORMAL << std::endl;		\
        G_LOG_BUF[ 0 ] = 0;					                \
         }while(0)

    #define __show_line_2() __show_line("")

    #define MSG( str , color ) do{                      \
	       std::cout<<  nowStr() <<  " "				\
            << __FILE_name__ << LEFT_BRACE              \
            << __LINE_no__ << RIGHT_BRACE               \
			<< color  << str << TNORMAL << std::endl;	\
	}while( 0 )

    #define MSG_1( format ,color, ... ) do{                     \
		std::cout <<  nowStr() <<  " "                          \
              << __FILE_name__ << LEFT_BRACE                    \
              << __LINE_no__ << RIGHT_BRACE  << color;	        \
		sprintf( G_LOG_BUF , format , __VA_ARGS__ );            \
		std::cout << G_LOG_BUF << TNORMAL << std::endl;         \
        G_LOG_BUF[ 0 ] = 0;					                    \
	}while(0)

        #define MSG_2() MSG("")
#else
	#define __show_line( str )
	#define __show_line_1( format , ... )
	#define __show_line_2()

        #define MSG( str , color )
        #define MSG_1( format , coor , ... )
        #define MSG_2()
#endif
#ifndef ERROR
#define ERROR( str ) __show_line( str )
#endif
#define ERROR_MSG( str ) __show_line( str )

#define WARN_MSG( str ) MSG( str , TYELLOW )


#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
    static const std::string PATH_DILMTER("\\");
#elif defined( __LINUX__ )
    static const std::string PATH_DILMTER("/");
#endif
/**
 * @brief 输出信息到指定的流
 * @param enable , 是否输出；true执行输出，false放弃输出
 * @param os, 要输出的流
 * @param msg , 要输出的信息
 */
void log_base( bool enable , std::ostream& os, const std::string& msg );
#if !defined( LOG )
#define LOG( enable , msg )     log_base( enable , std::cerr , msg )
#endif
/**
 * @brief 按照行输出信息，也就是完成输出消息后再自动添加一个换行
 */
void logln_base( bool , std::ostream& , const std::string& );
#define LOGLN( enable , msg )   logln_base( enable , std::cerr , msg )

typedef std::uint64_t hash_t;
constexpr hash_t __basis = 0xCBF29CE484222325ull;
constexpr hash_t __prime = 0x100000001B3ull;
extern hash_t hash_( char const *str );
extern hash_t hash_( char const *str , size_t len );
/////////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr hash_t hash_compile_time(char const* str, hash_t last_value = __basis)
{
	return *str ? hash_compile_time(str+1, (*str ^ last_value) * __prime) : last_value;
}

constexpr hash_t operator"" _hash( const char * data , size_t )
{
	return hash_compile_time( data );
}
/**
 * @brief GBK编码转换成UTF8编码
 * @param from[ I ],
 * @param len[ I ],
 * @param rst[ O ],
 * @param rlen[ O ],
 * @return
 */
extern int gbk2utf8( const char * from , size_t len , char ** rst , size_t & rlen );
/**
 * @brief UTF8编码转换成GBK编码
 * @param from[ I ],
 * @param len[ I ],
 * @param rst[ O ],
 * @param rlen[ O ]
 * @return
 */
extern size_t utf2gbk( const std::string& msg , char ** data );

/**
 * @brief 将数据按照字节方式改变大小端。长度必须是偶数的，否则返回错误
 * @param data[ I ]
 * @param len[ I ]
 * @return 成功操作返回true；否则返回false
*/
bool reverseByte(void* data, size_t len);
/**
 * @brief 查看进程列表
 * @param pattern[ I ]， 匹配模式字符串
 * @param fun 操作结果
 */
void ps( const std::string& pattern , std::function< void ( std::vector<std::string>&& ) > fun );

#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
std::string localIP( bool ipv4 );
# define access( a , b ) _access( a , b )
#else
/**
 * @brief 查询指定网卡IP地址
 * @param eth[ I ], 网卡名字
 * @return 成功操作返回IP地址
 */
std::string localIP( const std::string& eth );
#endif
/**
 * @brief 判断是否是IPv4字符串
 * @param ipstr[ I ], IP地址字符串。
 * @return 是IP地址返回true, 否则返回false
 */
bool isIPv4(const std::string& ipstr);
/**
 * @brief 判断是否是IPv6字符串
 * @param ipstr[ I ], IP地址字符串
 * @return 是IPv6字符串返回true, 否则返回false
 */
bool isIPv6(const std::string& ipstr);
/**
 * @brief 解析GMT时间字符串
 * @param gmt[ I ],GMT字符串
 * @return 返回转换结果
 */
time_t parseGMT( const std::string& gmt , int loc = 8);

/**
 * @brief 从路径中抽出文件名称。
 *     1  函数会检查本地文件是否存在，如果不存在根据最后路径是否还存在
 *     分隔符號，如果不存在分隔符號则直接给出最后一个名称。如果存在判断为目录
 *     2  如果文件存在，则会检查对象是否是目录
 * @param path[ I ], 路径名
 * @param name[ O ], 文件名称
 * @return 成功返回true, 否则返回false
*/
bool fileNameFromPath( const std::string& path , std::string& name );

/**
 * @brief 获取文件大小
 * @param path[ I ] 文件完整路径
 * @return 成功操作返回文件字节数量，否则返回0
*/
size_t getFileSize( const std::string& path );
/**
 * @brief 获取本机的IP地址
 * @warning 这个函数中没有针对网络库进行初始化，在windows下使用这个函数需要注意针对winsock进行初始化
 * @return
 */
std::string getLocalIP( bool ipv4 = true );
/**
 * @brief 将字符串按照指定的标记分割成多个字符串
 * @param str[ I ]， 要切割的字符串
 * @param c[ I ]， 分割标记
 * @param rst[ O ]， 分割结果
 * @return 成功返回字符串数量，否则返回-1
 */
int split( const std::string& str , char c , std::vector< std::string >& rst );
/**
 * @brief 判断是否是目录
 * @param dir[ I ],路径名称
 * @return 如果是目录返回true,否则返回false
 */
inline bool isDir( const std::string& dir )
{
	return fs::isDir( dir );
}
/**
 * @brief 从路径中抽取文件名。
 * @param path[ I ] 完整路径内容
 * @return 如果路径空内容，路径是是目录返回空字符串，否则返回文件名称
 */
inline std::string fileName( const std::string& path )
{
    if( path.empty() ) return {};
    if( fs::isDir( path ) ) return {};

    auto pos = path.rfind( PATH_DILMTER );
    if( pos == std::string::npos ) return path;

    std::string ret = path.substr( pos + 1);
    return ret;
}
/**
 * @brief 判断目录是否为空
 * @param dir[ I ]， 目录名称
 * @return 目录为空返回true，否则返回false
 */
inline bool dirEmpty( const std::string& dir )
{
	return fs::dirEmpty( dir );
}
/**
 * @brief removeFile
 * @param path
 * @param pattern
 * @param re
 * @return
 */
inline bool removeFile( const std::string& path , const std::string& pattern , bool re )
{
	auto rst = fs::removeFile( path , pattern , re );
	if( rst == fs::OK ) return true;

	return false;
}
/**
 * @brief 删除当前目录下的所有内容
 * @param path[ I ]， 要清理的目录
 * @return 成功操作返回true，否则返回false
 */
inline bool clearDir( const std::string& path )
{
	return removeFile( path , ".+" , true );
}
/**
 * @brief 读取CPU ID
 * @return
 */
extern std::string cpuID();

/**
 * @brief 执行外部程序，返回程序执行的标准输出结果
 * @param pszCmd[ I ], 外部程序
 * @return
 */
extern std::string ExeCmd(std::string pszCmd);
/**
 * @brief 异步执行外部程序，通过回调函数获取程序输出的标准流数据
 * @param pszCmd[ I ]， 外部程序
 * @param fun[ I ]， 结果回调函数
 * @return 成功启动外部程序返回true，否则返回false
 */
extern bool execCmd(std::string pszCmd , std::function< void ( const std::string& )> fun);

#endif

