#if defined( __WIN32 )
#    include <windows.h>
#endif

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/base/cef_bind.h>

#include "ccefbrowserview.hpp"
#include "ccefhander.hpp"
#include "credential.hpp"

static wchar_t pCharset[] = L"UTF-8";

CcefBrowserView::CcefBrowserView( const std::string& url ,CefRefPtr<CCefHander> client , QWidget * parent , credentialBase * c ):
    QWidget( parent ),__pt_client( client )
{
	CefWindowInfo   __cef_win_info;

	WId __h = this->winId();
	__m_wnd = __h; 
	RECT r;
	r.top = 0;
	r.left = 0;
	r.bottom = 768;
	r.right = 1366;
	__cef_win_info.SetAsChild( (cef_window_handle_t)__h , r );

	CefBrowserSettings settings;

	settings.default_encoding.str = pCharset;
	settings.default_encoding.length = wcslen(pCharset);
	client->setCredential( c );
	client->setParent( parent );
	CefBrowserHost::CreateBrowser ( __cef_win_info, client, url.c_str(), settings, NULL, NULL);
}

CcefBrowserView::~CcefBrowserView()
{
	/*CefRefPtr< CefBrowser > __brs = GetCefHandler()->GetBrowser();
	if( __brs ){
		__brs->GetHost()->CloseBrowser(true);
	}*/
}

void CcefBrowserView::setCefSize( int w , int h )
{
    setGeometry( 0 , 0 , w , h );
#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WINNT )
	HWND  __c = ::GetWindow( (HWND)__m_wnd , GW_CHILD );
	if( __c ){
        ::MoveWindow( ( HWND)__m_wnd , 0, 0, w , h , TRUE);
		::MoveWindow( __c , 0, 0, w, h, TRUE);
	}
#elif defined( __LINUX__ )

#endif
}

void CcefBrowserView::back()
{
	if( __pt_client ){
		CefRefPtr<CefBrowser> b = __pt_client->GetBrowser();

		if( b ){
			b->GoBack();
		}
	}
}
void CcefBrowserView::forward()
{
	if( __pt_client ){
		CefRefPtr<CefBrowser> b = __pt_client->GetBrowser();

		if( b ){
			b->GoForward();
		}
	}
}

bool CcefBrowserView::canGoBack()
{
	if( __pt_client ){
		CefRefPtr<CefBrowser> b = __pt_client->GetBrowser();

		if( b ){
			return b->CanGoBack();
		}
	}

	return false;
}
bool CcefBrowserView::canGoForward()
{
	if( __pt_client ){
		CefRefPtr<CefBrowser> b = __pt_client->GetBrowser();

		if( b ){
			return b->CanGoForward();
		}
	}

	return false;
}
