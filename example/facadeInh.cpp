#include <iostream>
#include <functional>

template <typename retType, typename... subTypes>
struct facadeInh{};

template <typename retType, typename subType1, typename... subTypes>
struct facadeInh<retType, subType1, subTypes...> : public subType1, public facadeInh<retType, subTypes...>
{
};

template <typename retType>
struct facadeInh<retType>
{
	template <typename... Args>
    retType run(std::function<retType(facadeInh<retType> * )> func)
    {
        return func(this);
    }
};

// 定义子系统类
struct SubSystem1
{
    int add(int a, int b)
    {
        return a + b;
    }
};

struct SubSystem2
{
    int multiply(int a, int b)
    {
        return a * b;
    }
};

// 创建外观类
template<typename... subTypes>
class MathFacade : public facadeInh<int, subTypes...> {};

int main()
{
    // 创建外观类对象
    MathFacade<SubSystem1, SubSystem2> facade;

    // 使用外观类对象调用子系统接口
    int result1 = facade.run([](facadeInh<int>* f)->int {
		MathFacade<SubSystem1, SubSystem2> * facade  = static_cast<  MathFacade<SubSystem1, SubSystem2> * >( f );
        return facade->add(2, 3);
    });

    int result2 = facade.run([](facadeInh<int>* f)->int{
		MathFacade<SubSystem1, SubSystem2> * facade  = static_cast<  MathFacade<SubSystem1, SubSystem2> * >( f );
        return facade->multiply(4, 5);
    });

    // 输出结果
    std::cout << "Result 1: " << result1 << std::endl;
    std::cout << "Result 2: " << result2 << std::endl;
    
    return 0;
}