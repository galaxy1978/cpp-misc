/**
 * @brief IOT Box主窗口.采用libcef集成webkit的方式实现网页前端任务
 * @version 1.0
 * @date 2017-10-20
 * @author
*/
#if defined( __USE_LIBCEF__)
#ifndef CCEFMAINWIN_H
#define CCEFMAINWIN_H

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>

#include "frame.hpp"

class CcefMainWin : public rcFrame
{
public:
    static const long ID_MENU_OPEN ;       // 打开文件
    static const long ID_MENU_CLOSE;       // 关闭当前窗口
    static const long ID_MENU_NEW_WIN;     // 打开新窗口
    static const long ID_MENU_NEW_TAB ;    // 打开新标签
    static const long ID_MENU_EXIT;        // 退出
    static const long ID_MENU_HELP;        // 帮助
    static const long ID_MENU_ABOUT;       // 关于
    static const long ID_MENU_CLOSE_ALL ;  // 关闭所有窗口
    static const long ID_MENU_FORWARD;     // 前进
    static const long ID_MENU_BACKWARD;    // 后退
    static const long ID_MAIN_TOOLBAR;     //
    static const long ID_MAIN_TOOLBAR2;    //
    static const long ID_ADDRESS;          //
private:
    wxMenuBar       * p_menu_bar;          // 菜单栏
    wxAuiNotebook   * p_aui_book;          // AUI nootbook，每一个页面就是一个浏览器窗口
public:
    CcefMainWin();
    virtual ~CcefMainWin();

    void ShowMenu( bool show = true );
    void HideMenu() { ShowMenu( false ); }

    void ShowRibbon( bool show = true );
    void HideRibbon( ){ ShowRibbon( false ); }

    // 事件处理部分
    // .1 菜单事件
    void OnMenuOpenFile( wxCommandEvent& evt );
    void OnMenuCloseTab( wxCommandEvent& evt );
    void OnMenuCloseAll( wxCommandEvent& evt );
    void OnMenuNewWin  ( wxCommandEvent& evt );
    void OnMenuNewTab  ( wxCommandEvent& evt );
    void OnMenuExit    ( wxCommandEvent& evt );
    void OnMenuHelp    ( wxCommandEvent& evt );
    void OnMenuAbout   ( wxCommandEvent& evt );
    void OnMenuForward ( wxCommandEvent& evt );
    void OnMenuBackward( wxCommandEvent& evt );

    void OnSize( wxSizeEvent &event );
protected:

private:
    void init();
    /**
     * @brief 装入配置文件。按照配置文件的配置变量初始化窗口内容
    */
    void __load_config();
    wxDECLARE_EVENT_TABLE();
};


#endif // CCEFMAINWIN_H
#endif
