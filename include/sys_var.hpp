/**
 * @brief 系统变量接口头文件，将服务器的系统变量设置保存在在指定的文件中。
 * 当系统启动的时候这些数据会从文件中调入内存保存，方便各个模块获取与其相应的
 * 变量设置
 * @version 0.1
 * @date 2017-2-11
 * @author 宋炜
 */


#ifndef __SYS_VAR_HPP__
#define __SYS_VAR_HPP__

//#include "defines.hpp"
#include "cconffile.hpp"
#include <map>
#include <memory>

class CSysVar : public CConfFile
{
private:
	std::string file_name;

public:
	explicit CSysVar( const std::string& file );
	virtual ~CSysVar(  );
};

extern std::shared_ptr<CSysVar>  GetOrCreateSysVar( const std::string& file  );
extern std::shared_ptr<CSysVar>  GetOrCreateSysVar( );


#endif
