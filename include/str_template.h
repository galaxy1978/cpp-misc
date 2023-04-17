/**
 * @brief 字符串模式匹配类
 * 字符串中没有提供对特殊格式的字符串的匹配功能。在LTRP中经常需要对特定字符串的格式进行判断或者内容提取，如对浮点的判断，对IP地址的判断
 *       对Email地址的判断甚至对物料命名的格式进行判断等任务。因此需要开发这个模块利用正则表达式完成这样的任务。
 *
 * @version 1.0.1.20
 * @date 2011-7-14 ~ 2020-12-25
 * @author 宋炜
*/

/**
 * 2019-6-22 修改模块，去掉wx的相关内容，重新使用STL实现，方便进行MVC分离设计
 * 2020-12-25 去掉DLL_EXPORT描述
 */
#ifndef __TMP_STR_H__
#define __TMP_STR_H__

#include <time.h>
#include <regex>
class CStrTmpt
{
public:
	CStrTmpt() = delete;
	CStrTmpt( const std::string& str ) = delete;
	~CStrTmpt() = delete;
	/**
	 * @brief 判断是否是浮点数
	 * @param str
	 * @return bool , 是浮点数则返回true, 否则返回false
	 */
	static bool isFloat( const std::string& str );
	/**
	 * @brief 将字符串转换成浮点数
	 * @param str
	 * @return 成功操作返回转换后的值
	 */
	static float toFloat( const std::string& str );

	static bool isReal( const std::string& str );
	static double toReal( const std::string& str );

	static bool isDigit( const std::string& str );
	static long toDigit( const std::string& str );

	static bool isHex( const std::string& str );
	static uint32_t toHex( const std::string& str );

	static bool isDate( const std::string& str );
	static time_t toDate( const std::string& str );

	static bool isTime( const std::string& str );
	static time_t toTime( const std::string& str );

	static bool isDateTime( const std::string& str );
	static time_t toDateTime( const std::string& str );
};
#endif
