#include <direct.h>

#include <iostream>
#include <thread>
#include <sstream>

#include <QMessageBox>

#include "ccefhander.hpp"
#include <include/cef_app.h>
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

CefRefPtr< CCefHander >     pt_handler;

CCefHander::CCefHander():p_credential( nullptr ),p_wid( nullptr )
{
	m_nBrowserCount = 0;
	pt_handler = this;
}

CCefHander::~CCefHander()
{
	pt_handler = nullptr;
}

void CCefHander::setCredential( credentialBase * c )
{
	if( c == nullptr ) return;
	if( p_credential == nullptr ){
		p_credential = c;
	}
}

void CCefHander::CloseAllBrowsers(bool force_close)
{
	if (!CefCurrentlyOn(TID_UI)){
		// Execute on the UI thread.
		//CefPostTask(TID_UI, NewCefRunnableMethod(this, &CCefHander::CloseAllBrowsers, force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}

std::string CCefHander::GetLoadingUrl()
{
	CefRefPtr<CefFrame> pMainFram=GetMainFram();
	return pMainFram.get()?pMainFram->GetURL():"";
}

void CCefHander::Navigate( const wstring& strUrl )
{
	CefRefPtr<CefFrame> pMainFram=GetMainFram();
	if ( pMainFram.get() )
		pMainFram->LoadURL(strUrl.c_str());
}

void CCefHander::CreateBrowser( HWND hParentWnd, const RECT& rect )
{
	CefWindowInfo info;
	CefBrowserSettings settings;
	//static wchar_t* pCharset = L"GB2312";
	//settings.default_encoding.str = pCharset;
	//settings.default_encoding.length = wcslen(pCharset);
	info.SetAsChild(hParentWnd, rect);
	CefBrowserHost::CreateBrowser(info,
				  CefRefPtr<CCefHander>(this),
				  m_strHomePage.c_str(),
				  settings,
				  NULL,
				  NULL);
}

//****************************************************
//菜单加载接口
void CCefHander::OnBeforeContextMenu( CefRefPtr<CefBrowser> /*browser*/,
				      CefRefPtr<CefFrame> /*frame*/,
				      CefRefPtr<CefContextMenuParams> params,
				      CefRefPtr<CefMenuModel> model )
{
    //在这里，我添加了自己想要的菜单
	CEF_REQUIRE_UI_THREAD();
	cef_context_menu_type_flags_t flag =   params->GetTypeFlags();
	if ( flag & CM_TYPEFLAG_PAGE ){//普通页面的右键消息
		model->SetLabel( MENU_ID_BACK,		L"返回");
		model->SetLabel( MENU_ID_FORWARD,	L"前进");
		model->AddSeparator();
		model->SetLabel( MENU_ID_VIEW_SOURCE,	L"查看源代码");
		model->SetLabel( MENU_ID_PRINT,		L"打印");
		model->AddItem( MENU_ID_RELOAD,		L"刷新");
		//model->AddSeparator();
		//model->AddItem( MENU_ID_FIND ,		L"查找...");
	}
	if ( flag & CM_TYPEFLAG_EDITABLE) {//编辑框的右键消息
		model->SetLabel( MENU_ID_UNDO,		L"撤销");
		model->SetLabel( MENU_ID_REDO,		L"重做");
		model->SetLabel( MENU_ID_CUT,		L"剪切");
		model->SetLabel( MENU_ID_COPY,		L"复制");
		model->SetLabel( MENU_ID_PASTE,		L"粘贴");
		model->SetLabel( MENU_ID_DELETE,	L"删除");
		model->SetLabel( MENU_ID_SELECT_ALL,	L"全选");
	}
}

bool CCefHander::OnContextMenuCommand( CefRefPtr<CefBrowser> /*browser*/,
				       CefRefPtr<CefFrame> /*frame*/,
				       CefRefPtr<CefContextMenuParams> /*params*/,
				       int command_id,
				       EventFlags /*event_flags*/ )
{
	bool ret = false;
	switch( command_id ){
	case MENU_ID_FIND:

	break;
	default:break;
	}

	return ret;
}


//****************************************************
//网页加载状态回调接口
void CCefHander::OnLoadStart( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> /*frame*/,
			      TransitionType /*transition_type */ )
{
	if( !m_pBrowser ){
		m_pBrowser = browser;
	}
}

void CCefHander::OnLoadEnd( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> /*frame*/, int /*httpStatusCode*/ )
{
	browser->GetHost()->SetZoomLevel( 0.0 );

}

void CCefHander::OnLoadError(CefRefPtr<CefBrowser> /*browser*/,
			     CefRefPtr<CefFrame> frame,
			      ErrorCode errorCode,
			      const CefString& /*errorText*/,
			      const CefString& /*failedUrl*/)
{
	CEF_REQUIRE_UI_THREAD();
	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;
	char pwd[ 500 ];
	getcwd( pwd , 500 );
	std::stringstream ss;
	ss << "file:///" << pwd << "/etc/404.html";
	std::string file = ss.str();
	frame->LoadURL( file.c_str() );

}

//****************************************************
//状态改变回调接口
void CCefHander::OnTitleChange( CefRefPtr<CefBrowser> /*browser*/, const CefString& /*title*/ )
{

}

void CCefHander::OnAddressChange( CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, const CefString& /*url*/ )
{

}

bool CCefHander::OnTooltip( CefRefPtr<CefBrowser> /*browser*/, CefString& /*text*/ )
{
	return false;
}

void CCefHander::OnStatusMessage( CefRefPtr<CefBrowser> /*browser*/, const CefString& /*value*/ )
{

}

//****************************************************
//网页生命周期回调接口
bool CCefHander::OnBeforePopup( CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefFrame> frame,
				const CefString&  target_url ,
				const CefString& /*target_frame_name */,
				CefLifeSpanHandler::WindowOpenDisposition /* target_disposition */,
				bool /* user_gesture */,
				const CefPopupFeatures& /* popupFeatures */,
				CefWindowInfo& /* windowInfo */,
				CefRefPtr<CefClient>& /* client */,
				CefBrowserSettings& /* settings */,
				CefRefPtr<CefDictionaryValue>& /* extra_info */,
				bool* /* no_javascript_access*/ )
{
	frame->LoadURL( target_url );
	m_pBrowser = browser;
	//return false;//创建新窗口
	return true; //禁止创建新的窗口
}

void CCefHander::OnAfterCreated(CefRefPtr<CefBrowser> /*browser*/)
{
    CEF_REQUIRE_UI_THREAD();
    // Add to the list of existing browsers.
    //browser_list_.push_back(browser);
	//    AutoLock scopLock(this);
    //if ( ! m_pBrowser.get() )
     //   m_pBrowser=browser;
    //m_nBrowserCount++;
}

bool CCefHander::DoClose(CefRefPtr<CefBrowser> /*browser */)
{
	CEF_REQUIRE_UI_THREAD();
	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		m_bIsClose = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void CCefHander::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();
	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}
	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void CCefHander::OnBeforeDownload( CefRefPtr<CefBrowser> /*browser*/,
				   CefRefPtr<CefDownloadItem> /*download_item*/,
				   const CefString& /*suggested_name */,
				   CefRefPtr<CefBeforeDownloadCallback> /* callback */ )
{

}

void CCefHander::OnDownloadUpdated(CefRefPtr<CefBrowser> /*browser*/,
		CefRefPtr<CefDownloadItem> download_item,
		CefRefPtr<CefDownloadItemCallback> callback )
{
    //取消CEF内部下载文件，使用默认浏览器打开链接去下载，下载过程就不需要我们关心了，毕竟不是做浏览器
    callback->Cancel();
    CefString strUrl = download_item->GetURL();
    ShellExecute(NULL, L"open", strUrl.c_str(), NULL, NULL, SW_SHOW);
}

void InitCefHandler()
{
	if( pt_handler ) return;

	new CCefHander();
}

CefRefPtr< CCefHander >& GetCefHandler()
{
	return pt_handler;
}

bool CCefHander::OnSelectClientCertificate(
	    CefRefPtr<CefBrowser> /*browser*/,
	    bool isProxy,
	    const CefString& /*host*/,
	    int /*port */,
	    const X509CertificateList& /*certificates*/,
	    CefRefPtr<CefSelectClientCertificateCallback> /*callback*/ )
{
	if( isProxy ){}

	auto e = p_credential->run();
	if( e == credentialBase :: OK ){
		std::string usr , pswd;
		p_credential->get( usr , pswd );
	}
	return true;
}


bool
CCefHander::GetAuthCredentials(CefRefPtr<CefBrowser> /*browser*/,const CefString& origin_url,bool isProxy ,const CefString& host,int port,
			const CefString& realm,	const CefString& scheme,CefRefPtr<CefAuthCallback> callback)
{
	if( p_credential == nullptr ){
		return false;
	}
	if( isProxy == false ){
		std::thread thd([=]{
			std::stringstream ss;
			ss << "目标主机需要您输入登录信息，请确认相关信息\n后输入用户和密码。\n"
			   << "\n原始地址：" << origin_url.ToString()
			   << "\n主机：   " << host.ToString() << ":" << port
			   << "\n域：     " << realm.ToString()
			   << "\n方案：   " << scheme.ToString();

			auto e = p_credential->run( ss.str() );
			if( e == credentialBase :: OK ){
				std::string usr , pswd;
				p_credential->get( usr , pswd );
				callback->Continue( usr , pswd );
			}
		});
		thd.detach();
	}

	return true;
}
