/**
 * @brief INI 文件读取解析修改保存
 * @version 1.1
 * @author 宋炜
 * @date 2021-7-16 ~ 2023-7-14
 */

/// 2023-7-14  增加修改变量功能
/// 2023-7-14  增加保存文件功能

#pragma once
// 这个实现是很早前的一个实现，最近对代码进行了修改，增加了部分内容。
// 基本实现了文件的解析，获取键值内容，修改，增加变量、小节。文件的序列化保存操作
#include <string>
#include <sstream>
#include <type_traits>
#include <map>

// 这个是我实现的的一个字符串数组
#include "ary_str.hpp"
// 这个是一个RAW 内存管理
#include "mallocSharedPtr.hpp"
namespace wheels{
class iniRead
{
public:
	// 定义几个错误代码
	enum emErrCode{
		ERR_ALLOC_MEM = -1000,
		ERR_CAN_NOT_FIND_KEY,
		ERR_CAN_NOT_FIND_SECTION,
		ERR_EMPTY_FILE,
		ERR_SECTION_NOT_EXIST,
		ERR_FIND_ASSIGN,
		ERR_KEY_DUPLICATED,
		OK = 0
	};
	// 定义保存变量的内存结构，这里使用map来存储
	using item_t = std::map< std::string , std::string >;
	// 定义保存小节的内存结构，同样使用map，在一个小节中可能会有多个变量。所以使用item_t作为存储
	using section_t = std::map< std::string , item_t >;
private:
	item_t      m_root_items__;            // 项目，这个实现允许没有小节直接写变量。这部分变量保存在这里
	section_t   m_root_sections__;         // 小节。用来存储所有的小节内容。

	std::string m_curr_section__;        // 当前操作的小节名称，就是当前在解析中的小节的名字
private:
	/**
	 * @brief 将数据分割成不同的行，并过滤掉回车符号和换行符号
	 */
	size_t split_line__( wheels::mallocSharedPtr<char>& buff , wheels::ArrayString& rst );
	/**
	 * @brief 解析文件，构造DOM树
	 * @param file[ I ]， ini文件路径
	 * @return 成功操作返回OK , 否则返回错误代码
	 */
	emErrCode parse__( wheels::mallocSharedPtr< char >& buff );
	/**
	 * @brief 处理小节中的变量
	 * @param str[ I ],
	 * @return 成功操作返回OK，否则返回错误代码
	 */
	emErrCode process_item__( const std::string& str );
	/**
	 * @brief 处理小节外的变量
	 * @param str[ I ],
	 * @return 成功操作返回OK，否则返回错误代码
	 */
	emErrCode process_root_item__( const std::string& str );
	/**
	 * @brief 判断是否是注释
	 * @param 
	 * @return 
	 */
	bool is_comment__( const std::string& str );
	/**
	 * @brief 判断是否是小节
	 * @param
	 * @return 是小节返回true
	 */
	bool is_section__( const std::string& str );
	/**
	 * @brief 判断是否是变量
	 * @param
	 * @return 
	 */
	bool is_item__( const std::string& str );
		
public:
	// 构造函数，提供文件的名字。自动载入和解析
	iniRead( const std::string& file );
	~iniRead();

	/**
	 * 获取键值内容。这个接口使用模板函数实现，避免了大量的重载。
	 * @tparam real_t 返回值类型的原始类型
	 * @return 如果找到键名，返回对应的数据
	 * @exceptions 如果找不到变量则抛出异常
	 */
	template< typename real_t >
	real_t get( const std::string& path , real_t& rst )
	{
		// 检查变量类型，应该是基础的数据类型和字符串类型
		static_assert( std::is_fundamental< real_t >::value || std::is_same< real_t , std::string >::value , "Unsupported data type." );
		real_t ret;
		// 函数参数的path是变量的完整名称，比如小节system中的color描述的格式为:  system/color,
		// 使用 / 分隔小节和变量名称
		std::string sec , key;
		size_t pos = path.find("/");           // 将路径分隔开来，小节存在sec，变量名存在key中
		if( pos != std::string::npos ){
			sec = path.substr( 0 , pos );
			key = path.substr( pos + 1 );
		}

		if( sec.empty() ){  // 如果小节名称是空的，则表明是文件中存在着没有个小节的自由变量。
			auto it = m_root_items__.find( key );  // 针对这些自由变量在这里处理
			if( it != m_root_items__.end() ){
				// 利用std::stringstream 进行数据的转换操作
				std::stringstream ss;
				ss.str( it->second );

				ss >> ret;
			}else{
				throw std::runtime_error( "找不到指定名称的变量" );
			}
		}else{  // 针对存在小节的情况的处理
			auto sit = m_root_sections__.find( sec );
			if( sit == m_root_sections__.end() ) throw std::runtime_error( "找不到小节名" );

			auto it = sit->second.find( key );
			if( it == sit->second.end() ) throw std::runtime_error( "找不到变量名" );
			// 找到数据后针对数据进行数据转换操作
			std::stringstream ss;
			ss.str( it->second );
			ss >> ret;
		}

		rst = ret;
		return ret;
	}

	/**
	 * @brief 修改值。这个方法同样使用模板函数实现，用来支持不同类型的数据。基本的实现方式和get是一样的
	          但是这个方法，如果找不到目标变量会添加新的变量，如果没有小节也会添加新的小节
	 */
	template< typename T >
	void set( const std::string& path , const T& data ){
		static_assert( std::is_fundamental< T >::value || std::is_same< T , std::string >::value , "Unsupported data type." );
		T ret;

		std::string sec , key;
		size_t pos = path.find("/");
		if( pos != std::string::npos ){
			sec = path.substr( 0 , pos );
			key = path.substr( pos + 1 );
		}

		if( sec.empty() ){
			auto it = m_root_items__.find( key );
			std::stringstream ss;
			ss << data;
			if( it != m_root_items__.end() ){
				it->second = ss.str();
			}else{// 添加新的自由变量
				m_root_items__.insert( std::make_pair( key , ss.str() ) ); 
			}
		}else{
			std::stringstream ss;
			ss << data;
			
			auto sit = m_root_sections__.find( sec );
			if( sit == m_root_sections__.end() ){ // 添加新的小节
				item_t i;
				i.insert( std::make_pair( key , ss.str() ) );
				m_root_sections__.insert( std::make_pair( sec , i ) );
				return;
			}
			auto it = sit->second.find( key );
			
			if( it != sit->second.end() ){
				it->second = ss.str();
			}else{ // 添加新的变量
				sit->second.insert( std::make_pair( key , ss.str() ) );
			}
		}
	}
	/**
	 * @brief 保存文件。
	 * @param file[ I ], 文件名称
	 * @return 成功操作返回true，否则返回false
	 */
	bool save( const std::string& file );

};
}
