#include <include/cef_app.h>
#include "ccefapp.hpp"

CCefApp :: CCefApp()
:CefApp(),CefBrowserProcessHandler()
{
	CefEnableHighDPISupport();

}
CCefApp :: ~CCefApp()
{

}

CefRefPtr<CefBrowserProcessHandler>
CCefApp :: GetBrowserProcessHandler()
{
	return CefApp::GetBrowserProcessHandler();
}
CefRefPtr< CefRenderProcessHandler >
CCefApp :: GetRenderProcessHandler()
{
	return CefApp::GetRenderProcessHandler();
}
CefRefPtr< CefResourceBundleHandler >
CCefApp :: GetResourceBundleHandler()
{
	return CefApp::GetResourceBundleHandler();
}

void CCefApp :: OnBeforeCommandLineProcessing(
	const CefString& /*process_type*/,
	CefRefPtr< CefCommandLine > command_line )
{
    command_line->AppendSwitch( "--gpu" );
	//command_line->AppendSwitch( "--proxy-server=192.168.6.33:80" );
	//command_line->AppendSwitch( "--enable-zero-copy" );
	//command_line->AppendSwitchWithValue("--gpu-rasterization-msaa-sample-count","16" );
}
void CCefApp :: OnRegisterCustomSchemes( CefRawPtr< CefSchemeRegistrar > /*registrar*/ )
{

}
void CCefApp :: OnContextInitialized()
{

}
