/**
 * @brief 控制LOG文件，记录文件内容。
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <thread>
#include <mutex>

#include "ary_str.hpp"
#include "misc.hpp"

namespace wheels
{
class logFile : public std::streambuf
{
public:
	enum emErrCode{
		ERR_INIT_DIR = -1000,
		ERR_INIT_FILE,
		OK = 0
	};
	struct stFile{
		bool         m_is_current;
		std::shared_ptr< std::ofstream >  pt_ofs; 
		size_t       m_curr_line;
		std::string  m_file_name;

		stFile( const std::string& file ):
			m_is_current( false ),
			pt_ofs( nullptr ),
			m_curr_line( 0 ),
			m_file_name( file ){}

		stFile( const stFile& file ):
			m_is_current( file.m_is_current ),
			pt_ofs( std::move( file.pt_ofs ) ),
			m_curr_line( file.m_curr_line ),
			m_file_name( file.m_file_name ){}

		stFile& operator=( const stFile& file ){
			m_file_name = file.m_file_name;
			m_curr_line = file.m_curr_line;
			pt_ofs = std::move( file.pt_ofs );
			m_is_current = file.m_is_current;

			return *this;
		}
	};
	static const size_t      MAX_LINE_LEN;
private:
	std::mutex               __m_mutex;
	size_t                   __m_max_line;
	size_t                   __m_max_files;
	std::ofstream          * __p_current_file;
	std::string              __m_file_name;
	std::string              __m_data_buf;
	// 记录标准流的旧缓冲区。如果需要重定向标准流，这个就非常重要。
	// 以用来在结束重定向后恢复。
	std::streambuf         * __p_stdout;
	std::streambuf         * __p_stderr;

	// LOG文件保存的目录，如果没有配置是当前目录下的log目录
	std::string              __m_log_root; 
	                                        
	std::vector< stFile >    __m_file;
private:
	int overflow( int c ) final;

	bool __new_file();
	/**
	 * @brief 生成文件名称
	 */
	std::string __make_file_name();
	/**
	 * @brief 构建新的文件
	 */
	bool __create_new_log( const std::string& file );
	/**
	 * @brief 查找最后的文件，统计文件内容长度
	 */
	std::string __find_old_file( const std::string& path );
        /**
	 * @brief 初始化目录，如果指定目录下没有log目录则创建一个
	 * @param path[ I ], 根路径；不包含log目录
	 */
	bool __init_dir( const std::string& path );
	/**
	 * @brief 清理内存中的文件列表
	 */
	void __clear();
	/**
	 * @brief 按文件名对文件进行排序。
	 */
	void __sort_file();
public:
	/**
	 * @brief ctor 
	 * @param logroot[ I ], log文件的根目录，如果没有配置则自动配置成当前路径下log目录
	 * @param stdo[ I ], 是否重定向标准输出流
	 * @param stde[ I ]， 是否重定向标准错误流
	 * @note logroot并不包含log目录，程序启动后会自动在根目录下创建log目录
	 */
	logFile( const std::string& logroot , bool stdo = true , bool stde = true );
	~logFile();
	/**
	 * @brief 配置LOG生成控制参数
	 * @param maxF[ I ] 最大文件数量
	 * @param maxLine[ I ] 每个文件的最大行数
	 * @param logroot[ I ] log目录的父目录名，可以是相对路径
	 */
	void configure( size_t maxF , size_t maxLine , const std::string& logroot );
	/**
	 * @brief 增加数据内容。将数据内容写入当前的LOG文件
	 * @param data[ I ],要登记成日志的数据
	 */
	void add( const std::string& data );
};
}
