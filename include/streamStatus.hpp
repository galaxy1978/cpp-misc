/**
 * @brief 自定义一个流缓冲区，用来在方便在状态栏显示相关信息
 * @version 1.1
 * @date 2020-10-13 ~ 2021-2-22
 * @author 宋炜
*/

/// 2021-2-22 ADDED 添加LOG文件
#pragma once

#include <streambuf>
#include <sstream>
#include <fstream>
#include <memory>
#include <unordered_map>

#include <QStatusBar>
#include <QLabel>
#include <QColor>
#include <QMouseEvent>
#include <QDialog>

struct ssStatusPlugBase
{
	virtual ~ssStatusPlugBase(){}
	/**
	 * @brief 添加数据
	 * @param data
	 */
	virtual void push( const std::string& data ) = 0;
	/**
	 * @brief 运行模块
	 * @warning 模块不可以采用阻塞的方式实现这个函数
	 */
	virtual void run() = 0;
};

class ssStatusBar : public QStatusBar , public std::basic_streambuf< char >
{
public:
	enum emErrCode{
		ERR_CREATE_LOG = -1000,
		OK = 0
	};

	using ssStatusDcrt_t = std::unordered_map< std::string , std::unique_ptr< ssStatusPlugBase > >;
	using dcrt_t = ssStatusPlugBase;
private:
	std::string				__m_data;
	QLabel				      * __p_status;
	bool					__m_en_log;
	int					__m_log_type;

	std::shared_ptr< std::ofstream >	__pt_os;
	/// 采用装饰器模式为状态栏添加不同功能，所有的新增功能模块必须从
	/// ssStatusPlugBase继承而来
	ssStatusDcrt_t				__m_dcrts;
protected:
	virtual int overflow( int c ) final;
private:
	/**
	 * @brief 响应处理鼠标双击事件
	 * @param event[ I ]
	 */
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

public:
	ssStatusBar( QWidget * parent );
	virtual ~ssStatusBar(){}
	/**
	 * @brief 状态栏红绿灯设置
	 * @param color[ I ]， 背景色
	 * @param txt[ I ]，提示信息
	 */
	void setLabelColor( const std::string& color , const std::string& txt = "   ");
	/**
	 * @brief 添加装饰
	 * @param id[ I ]， 模块ID
	 * @param dcrt[ I ]， 模块指针
	 * @return 成功返回true，否则返回false
	 */
	bool dcrtIt( const std::string& id , dcrt_t * dcrt );
	/**
	 * @brief 移除装饰
	 * @param id[ I ]， 模块ID
	 */
	void unDcrtIt( const std::string& id );
};
