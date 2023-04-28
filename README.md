# cppMisc

#### 介绍
我自己常用的基础算法，接口的封装。主要包含了常用的设计模式，容器和一些常用的功能性接口。

- 设计模式目前已经实现里单例模式，装饰模式，工厂模式和桥接模式，位于designM目录。

- 容器目前实现了AVL


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
   创建对象
   ```newSingleClass * p = newSingleClass::create();```
   
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
//包含有错误通知回调的工厂
a * pa2 = a::factory( []( Factory::emErrCode err){
            // 错误处理
} , 23 );
```
