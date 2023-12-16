#include <iostream>  
#include <memory>
#include <vector>
#include <thread>
#include <type_traits>
#include <functional>
#include <string>
#include <future>

#include <lua.hpp>  
#include <lualib.h>  
#include <lauxlib.h>  
 
#include "designM/interpretor.hpp"
using namespace wheels;
using namespace dm;

using itfc_type = itptorItfc<int >;
// Lua解释器实现类  
class LuaInterpreter : public itfc_type {  
public:  
    LuaInterpreter() {}  
	virtual int execFile( const std::string& file ) override{ return 0; }
	
    virtual int execString(const std::string& str ) override {  
        auto * L = luaL_newstate(); // 初始化Lua状态机  
		std::cout << str << std::endl;
        luaL_openlibs( L ); // 加载标准库  
        if (luaL_loadstring(L, str.c_str()) || lua_pcall(L, 0, 1,0)) { // 加载并执行Lua脚本  
            std::cerr << "Failed to execute Lua script: " << lua_tostring(L, -1) << std::endl;  
            lua_close(L); // 清理Lua状态机  
            return -1; // 返回错误码  
        } else { // 执行成功，返回结果  
            int result = lua_tonumber(L, -1); // 获取结果，并关闭Lua状态机  
            lua_close(L);  
            return result;  
        }  
    }  
};  

using luaExecutor = iterpretor< itfc_type, LuaInterpreter >; // 创建Lua执行器对象  

int func( std::shared_ptr< luaExecutor::impl_t > pimpl , const std::string& str  )
{
	if( pimpl ){
			return pimpl->execString( str );
	}
	return -1;
}
  
int main() 
{    
	
    auto luaExecutorPtr = luaExecutor::make_shared(); // 创建执行器指针，用于异步执行Lua脚本 

    auto luaFuture = luaExecutorPtr->execStringAsync( "return 2 + 2" , func ); // 异步执行Lua脚本，并获取结果future  
    std::this_thread::sleep_for( std::chrono::seconds(1));
	
	std::cout << "Lua script result: " << luaFuture.get() << std::endl; // 获取结果并输出，等待异步执行完成  
	
	
    return 0;  
}