/**
 * @brief HTTP授权用户接口
 * @version 1.0
 * @author 宋炜
 * @date 2017-10-20
 */

#pragma once

#include <string>

class credentialBase
{
public:
	enum emButtonType{ OK , CANCEL };
public:
	/**
	 * @brief 启动运行，一般可以弹出对话框输入
	 * @return 如果用户点击OK按钮返回OK ，否则返回CANCEL
	 */
	virtual enum emButtonType run() = 0;

	virtual enum emButtonType run( const std::string& info ) = 0;
	/**
	 * @brief 读取授权信息
	 * @param info[O]
	 */
	virtual void get( std::string& usr , std::string& pswd ) = 0;
};
