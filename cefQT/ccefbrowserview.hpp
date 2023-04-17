/**
* @brief 浏览器窗口
* @version 1.0
* @date 2017-9-24
* @author 宋炜
*/
#ifndef CCEFBROWSERVIEW_H
#define CCEFBROWSERVIEW_H
#include <QWindow>
#include <QWidget>

#include <include/cef_browser.h>
#include <include/views/cef_browser_view.h>
#include <include/base/cef_bind.h>
#include <include/cef_client.h>

#include "ccefhander.hpp"
#include "credential.hpp"

class CcefBrowserView : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @brief
	 * @param url
	 * @param client
	 * @param parent
	 */
	CcefBrowserView( const std::string& url ,CefRefPtr<CCefHander> client, QWidget *parent , credentialBase * credent = nullptr ) ;
	virtual ~CcefBrowserView();
	/**
	 * @brief setCefSize
	 * @param w
	 * @param h
	 */
	void setCefSize( int w , int h );
	/**
	 * @brief 浏览器的前进或者后退操作
	 */
	void back();
	void forward();

	bool canGoBack();
	bool canGoForward();
private:
	WId			__m_wnd;
	CefRefPtr<CCefHander>   __pt_client;
    int         __m_w;
    int         __m_h;
};

#endif // CCEFBROWSERVIEW_H
