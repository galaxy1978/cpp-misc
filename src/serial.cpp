/// 错误信息接口在这个文件中
#include "details/iSerial.cpp"

/// 分别引入不同平台实现代码。
/// 注意：
///    windows 平台的代码没有在visual C++中进行编译和测试。不过mingw gcc是经过测试验证的
#if defined( WIN32) || defined( __WIN64 ) || defined( WINNT )
#    include "details/winSerial.cpp"
#elif defined( __LINUX__ )
#    include "details/linuxSerial.cpp"
#endif

