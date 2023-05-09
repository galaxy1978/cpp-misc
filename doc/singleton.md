# singleton 单例模式
#### 实现原理
- 1 静态指针std::unique_ptr存储对象
- 2 私有化构造函数，移除拷贝函数
- 3 暴露出工厂接口和获取对象指针接口
- 4 利用模板参数的逗号展开方式适配任意参数个数的构造函数
#### 主要功能
- 支持任意个参数的构造函数

#### 使用方法
- 包含需要的头文件
```
#include "designM/singleton.hpp"
```
- 自己的类声明需要继承singleton
```
   class newSingleClass : public wheels::dm::singleton< newSingleClass > {
	   // 您自己的实现代码
   };
```   
- 在实现文件中：
```
   IMP_SINGLETON(newSingleClass);
   
   // 您自己的实现代码
   
   应用时：
   创建对象，
   newSingleClass * p = newSingleClass::create();
   
   获取对象：
   auto * p = newSingleClass::get();    b = var_t[1].get<decltype( b ) >();
```
}
```
