# cppMisc
#### changelog
-  2023-11-3 新增了中介者模式
-  2023-11-2 新增了备忘录模式，组合模式和代理模式
-  2023-7-15 新增了INI文𢓐处理模块
-  2023-7-14 新增了适配器模式 include/designM/adaptor.hpp
-  2023-7-7  新增了外观模式，include/designM/facade.hpp"分别以接口和继承的两种方式实现 , 示例在example目录下
-  2023-7-4  新增了建造者模式，include/designM/build.hpp , 示例在example目录下
-  2023-6-30 新增了抽象工厂模式 include/designM/absFactory.hpp，示例在example目录下
-  2023-6-26 新增了线程池 include/threadPool.hpp
-  2023-5-28 新增红黑树
-  2023-5-7  新增variant，任意数据类型， 文件路径container/variant.hpp。这个模块是一个纯头文件的模板类，支持在其他的stl容器中使用这个模块
-  2023-5-9  新增variant简单介绍文档
#### 介绍
我自己常用的基础算法，接口的封装。主要包含了常用的设计模式，容器和一些常用的功能性接口。

- 设计模式目前已经实现里单例模式，装饰模式，工厂模式和桥接模式，位于include/designM目录。设计模式实现都是采用纯头文件的方式可以直接引用

- 容器目前实现了AVL, variant , 红黑树rbTree。这部分代码位于include/container目录中


#### 安装教程
- 单例模式使用说明：
   业务实现类应该继承于singleton,并在实现文件中使用宏 IMP_SINGLETON
   
   在头文件中：

```
   #include "designM/singleton.hpp"
   
   class newSingleClass : public wheels::dm::singleton< newSingleClass > {
	   
	   // 您自己的实现代码
   };
```

   
   在实现文件中：

```
   IMP_SINGLETON(newSingleClass);
   
   // 您自己的实现代码
   
```

   应用时：
   创建对象,create支持可变参数方式，因此可以应用于有不同参数的构造函数
```
    newSingleClass * p = newSingleClass::create();

    newSingleClass * p = newSingleClass::create( 12 , "abc" );
```
   
   获取对象：
  ```auto * p = newSingleClass::get();```
- 工厂模式：工厂模式采用两种方式实现，一种是采用类继承的方式；一种是采用模板函数的方式
1. 采用模板函数的方式适用于公有的构造函数的类，例如：

```
class a{
public:
	a();
	a( int c );
};

a * pa = factory< a >( 23 );
a * pa2 = Factory< a >( []( Factory::emErrCode err){
	 // 错误处理
	} , 23 );
```

这里注意，工厂函数考虑了创建没有考虑销毁，在使用完成对象后要自己调用delete完成对象的销毁
2. 采用类继承的方式适用于采用私有构造函数的方式，例如

```
class a : public Factory< a >{
public:
         a();
         a( int c );
};

a * pa = a::factory( 23 );
```
- 责任链模式
 使用示例：


```
class myItfc{
    virtual int do_something( int ) = 0;
    定义接口
 };

 class myItfc1 : public myItfc{
      接口实现1
 };

 class myItfc1 : public myItfc{
      接口实现2
 };

 class myRsps : public rspsLink< myItfc >{
        自己的定义和方法
 }

 myRsps rsps;
 rsps.push_back( new myItfc1 );
 rsps.push_back( new myItfc2 );
 rsps.push_back( ... );

/// 按照责任链传递操作
 int a = 1;
 rsps.forward([&]( myRsps::stLinkItem& item ){
       a = item->do_something( a );
 });
```
