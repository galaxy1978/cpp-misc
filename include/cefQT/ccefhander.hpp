/**
 * @brief libcef 回调处理模块
 * @version 1.1
 * @date 2020-3-22 ~ 2021-1-12
 * @author 宋炜
 */
#ifndef CCEFHANDER_H
#define CCEFHANDER_H
#include <list>
#include <string>
#include <QTableWidget>
#include <include/cef_display_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/cef_load_handler.h>
#include <include/cef_request_handler.h>
#include <include/cef_context_menu_handler.h>
#include <include/cef_download_handler.h>
#include <include/cef_browser.h>
#include <include/views/cef_browser_view.h>
#include <include/base/cef_bind.h>
#include <include/cef_client.h>
#include <include/cef_auth_callback.h>

#include "credential.hpp"

#if defined( wstring )
#	undef wstring
#endif

typedef std::string  wstring;


class CCefHander: public CefClient,
                  public CefDisplayHandler,
                  public CefLifeSpanHandler,
                  public CefLoadHandler,
                  public CefRequestHandler,
                  public CefContextMenuHandler,
                  public CefDownloadHandler
{
public:
        CCefHander();
        virtual ~CCefHander();
	/**
	 * @brief 指定用户授权接口对象
	 * @param c
	 */
	void setCredential( credentialBase * c );

	void setParent( QWidget * parent ){ if( !p_wid ) p_wid = parent; }
	//自定义方法
	CefRefPtr<CefBrowser> GetBrowser() { return m_pBrowser; }
	CefRefPtr<CefFrame>   GetMainFram() { return m_pBrowser.get()?m_pBrowser->GetMainFrame():NULL; }
	HWND    GetBrowserHostWnd() { return m_pBrowser.get()?m_pBrowser->GetHost()->GetWindowHandle():NULL; }
	void    SetHomePage(const std::string& strUrl) { m_strHomePage=strUrl; }
	const std::string& GetHomePage()const { return m_strHomePage; }
	std::string GetLoadingUrl();
	void    Navigate(const std::string& strUrl);
	void    CreateBrowser(HWND hParentWnd, const RECT& rect);
	bool    IsClosing() const { return m_bIsClose; }

	//凡是继承了的处理功能都在这里返回this指针
	virtual CefRefPtr<CefDisplayHandler>      GetDisplayHandler() override     { return this; }
	virtual CefRefPtr<CefLifeSpanHandler>     GetLifeSpanHandler() override    { return this; }
	virtual CefRefPtr<CefLoadHandler>         GetLoadHandler()  override       { return this; }
	virtual CefRefPtr<CefContextMenuHandler>  GetContextMenuHandler() override { return this; }
	virtual CefRefPtr<CefDownloadHandler>     GetDownloadHandler() override    { return this; }
	virtual CefRefPtr<CefRequestHandler>      GetRequestHandler() override     { return this; }
	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
	virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) override;
	virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;

	// CefLifeSpanHandler methods:
	virtual bool OnBeforePopup( CefRefPtr<CefBrowser> browser,
				    CefRefPtr<CefFrame> frame,
				    const CefString& target_url,
				    const CefString& target_frame_name,
				    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
				    bool user_gesture,
				    const CefPopupFeatures& popupFeatures,
				    CefWindowInfo& windowInfo,
				    CefRefPtr<CefClient>& client,
				    CefBrowserSettings& settings,
				    CefRefPtr<CefDictionaryValue>& extra_info,
				    bool* no_javascript_access) override;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

	// CefLoadHandler methods:
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
				 CefRefPtr<CefFrame> frame,
				 TransitionType transition_type) override;
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);
	//
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> /* browser */,
				    CefRefPtr<CefFrame> /* frame */,
				    CefRefPtr<CefRequest> /* request*/,
				    bool /* user_gesture*/,
				    bool /*is_redirect*/) override
	{
		//return true;
		return false;
	}

	virtual bool OnBeforeResourceLoad(CefRefPtr<CefBrowser> /*browser*/,
					  CefRefPtr<CefFrame> /* frame */,
					  CefRefPtr<CefRequest> /*request */ ) {
		return false;
	}


	//菜单处理
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
					 CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

	virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
					  CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) override;

	//下载处理
	virtual void OnBeforeDownload( CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
				       const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) override;

	virtual void OnDownloadUpdated( CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
					CefRefPtr<CefDownloadItemCallback> callback) override;
	virtual bool OnSelectClientCertificate(
	    CefRefPtr<CefBrowser> /*browser*/,
	    bool /*isProxy*/,
	    const CefString& /*host*/,
	    int /*port */,
	    const X509CertificateList& /*certificates*/,
	    CefRefPtr<CefSelectClientCertificateCallback> /*callback*/)override;
	virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,	const CefString& origin_url,bool isProxy,const CefString& host,int port,const CefString& realm,const CefString& scheme,CefRefPtr<CefAuthCallback> callback)final;
private:
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;
	CefRefPtr<CefBrowser> m_pBrowser;
	bool    m_bIsClose;
	std::string m_strHomePage;
	int  m_nBrowserCount;


	IMPLEMENT_REFCOUNTING( CCefHander );
	//    IMPLEMENT_LOCKING( CCefHander );
	credentialBase   * p_credential;
	QWidget		 * p_wid;
};

void InitCefHandler();

CefRefPtr< CCefHander >  & GetCefHandler();
#endif // CCEFHANDER_H
