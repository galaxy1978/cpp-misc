/**
 * @brief 文件系统接口封装
 * @version 1.0
 * @date 2020-9-30
 * @author 宋炜
 */

#pragma once

#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
#	if defined( __GNUC__ )
#		include <dirent.h>
#	else
#		include <windows.h>
#	endif
#elif defined( __LINUX__ )
#	include <dirent.h>
#endif

#include <sys/stat.h>
#include <string.h>
#include <functional>

#include <string>
#include "tree.hpp"
class fs
{
public:
	/// @brief 定义文件描述项目
	struct stFileItem{
		std::string    m_name;
		struct stat    m_stat;
		
		stFileItem(){}
		stFileItem( const stFileItem& b ){
			m_name = b.m_name;
			memcpy( &m_stat , &b.m_stat , sizeof( struct stat ));
		}
			
		stFileItem& operator=( const stFileItem& b ){
			m_name = b.m_name;
			memcpy( &m_stat , &b.m_stat , sizeof( struct stat ));
			return *this;
		}
				
	};

	using fileTree = tree< stFileItem >;

	enum emErrCode{
		ERR_ROOT_EMPTY = -1000,
		ERR_MISSING_TMPT,
		ERR_OPEN_DIR,
		ERR_PATH_NOT_DIR,
		ERR_ITER_DIR,              // 遍历目录失败
		OK = 0
	};
public:
	/**
	 * @brief 拷贝文件
	 * @param from
	 * @param to
	 * @return
	 */
	static bool copy( const std::string& from , const std::string& to );
	/**
	 * @brief 判断给定的路径是否是目录
	 * @param path [ I ] 文件路径
	 * @param item[ I ] 指定文件项目
	 * @return 如果是目录返回true, 否则返回false
	 */
	static bool isDir( const std::string& path );
	static bool isDir( const stFileItem& item );
	/**
	 * @brief 判断给定的路径是否是普通文件
	 * @param path [ I ] 文件路径
	 * @return 如果是普通文件返回true, 否则返回false
	 */
	static bool isFile( const std::string& path );
        /**
	 * @brief 判断指定目录是否是空目录
	 * @param path[ I ]， 指定目录路径
	 * @return 指定目录不空返回false，否则返回true
	 * @exceptions 指定目录为空字符串抛出-1，打开目录失败抛出-2，指定字符串是文件名字抛出-3
	 */
	static bool dirEmpty( const std::string& path );
        /**
	 * @brief 枚举文件，将指定目录按照原本的结构整理成一个可以以树形结构遍历的数据
	 * @param path[ I ]
	 * @param root[ O ]
	 * @return 成功操作返回true，否则返回false
	 */
	static emErrCode enumFile( const std::string& path , fileTree& root );
	static void enumFileAsync( const std::string& path , std::function< void ( emErrCode , fileTree& ) > fun );
        /**
	 * @brief 删除满足条件要求的文件或者目录
	 * @param path[ I ], 文件目录路径
	 * @param files[ I ]， 文件名匹配条件， 文件匹配条件以正则表达式作为匹配条件
	 * @param reg[ I ], 条件正则表达式
	 * @param re[ I ], 是否迭代删除
	 * @return 删除成功返回OK，否则返回错误代码
	 * 
	 */
	static emErrCode removeFile( const std::string& path ,
				     const std::string& reg , bool re = true );

	static void removeFileAsync( const std::string& path,
					  const std::string& files ,
					  std::function< void (emErrCode) > fun,
					  bool re = true );
	/**
	 * @brief 遍历指定路径中的文件
	 * @param path[ I ], 指定目录
	 * @param fun 执行的操作
	 *     bool fun( const stFileItem& item );
	 *     如果要停止遍历则返回false
	 */
	static emErrCode for_each( const std::string& path , std::function< bool ( emErrCode , const stFileItem& ) > fun );
};
