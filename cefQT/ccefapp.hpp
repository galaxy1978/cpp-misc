/**
* @brief 使用libcef作为网页处理器。使用wxApp作为APP管理类。将这两个综合起来
* @version 1.1
* @date 2018-7-23 ~ 2020-11-20
* @author 宋炜
*/
#ifndef CCEFAPP_H
#define CCEFAPP_H

#include <include/cef_app.h>
class CCefApp : public CefApp ,  public CefBrowserProcessHandler
{
public:
	CCefApp();
	virtual ~CCefApp();

	/**
	 * @brief
	 * @return
	 */
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;
	virtual CefRefPtr< CefRenderProcessHandler > GetRenderProcessHandler() override;
	virtual CefRefPtr< CefResourceBundleHandler > GetResourceBundleHandler() override;

	virtual void OnBeforeCommandLineProcessing( const CefString& process_type, CefRefPtr< CefCommandLine > command_line ) override;
	virtual void OnRegisterCustomSchemes( CefRawPtr< CefSchemeRegistrar > registrar ) override;
	virtual void OnContextInitialized() override;
protected:

private:
	IMPLEMENT_REFCOUNTING(CCefApp);
};

#endif // CCEFAPP_H

