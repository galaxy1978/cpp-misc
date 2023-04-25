# cppMisc

#### 介绍
我自己常用的基础算法，接口的封装。主要包含了常用的设计模式，容器和一些常用的功能性接口。

- 设计模式目前已经实现里单例模式，装饰模式，工厂模式和桥接模式，位于designM目录。

- 容器目前实现了AVL


#### 安装教程
单例模式使用说明：
   业务实现类应该继承于singleton,并在实现文件中使用宏 IMP_SINGLETON
   
   在头文件中：
   #include "designM/singleton.hpp"
   
   class newSingleClass : public wheels::dm::singleton< newSingleClass > {
	   
	   // 您自己的实现代码
   };
   
   在实现文件中：
   IMP_SINGLETON(newSingleClass);
   
   // 您自己的实现代码
   
   应用时：
   创建对象，
   newSingleClass * p = newSingleClass::create();
   
   获取对象：
   auto * p = newSingleClass::get();