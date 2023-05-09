# variant
#### 实现原理
- 1 使用一个纯虚函数的基类作为存储接口。提供相关存储访问接口和存储指针
- 2 使用一个模板类，并从纯虚基类继承，用来存储任意类型的数据
- 3 在上面描述的模板类之外再外覆一个普通类，这个类中实现可以暴露的接口
#### 主要功能
- 默认构造，拷贝构造，拷贝赋值，右值拷贝
- 静态工厂函数
- C++内建数据类型赋值重载，主要包括了基本的数字类型和字符串类型
- 支持==、< 、>、>=、 <=

#### 使用方法
- 包含需要的头文件
```
#include "container/variant.hpp"
using namespace wheels;
```
- 使用工厂函数生成对象,然后使用
```
using tuple = std::vector< variant >
tuple  var_t;

auto var_int = variant::make( 12 );
auto var_string = variant::make( std::string("abc") );

var_t.push_back( var_int );
var_t.push_back( var_string );

int a;
std::string b;

if( var_t[0].is<int>() ){
        a = var_t.get<int>();
}

if( var_t[1].is<std::string> ){
        b = var_t[1].get<decltype( b ) >();
}
```
