#include <iostream>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

#define FACADE_START_DECLARE_SUB_ITFC( name ) \
struct name{ \
	virtual ~name(){}

#define FACADE_END_DECLARE_SUB_ITFC  };

#define FACADE_ADD_ITFC( RET , NAME , ... ) \
virtual RET NAME( __VA_ARGS__ ) = 0;
	

template< typename itfcType ,  typename... subTypes >
class facadeItfc
{
public:
	template< int N , typename tupleParams >
	struct INIT_HELPER__
	{
		static void init__( std::vector< itfcType * >& vec , tupleParams& t ){
			vec[ N - 1 ] = std::get< N - 1 >( t );
			
			INIT_HELPER__< N - 1 , tupleParams >::init__( vec , t );
		}
	};
	
	template< typename tupleParams >
	struct INIT_HELPER__<0 , tupleParams >{
		static void init__( std::vector< itfcType * >& vec , tupleParams& t ){}
	};
protected:
    std::vector< itfcType * >  m_subs__;
public:
    facadeItfc(subTypes*... subs):m_subs__( sizeof...(subTypes) ) {
		std::tuple< subTypes*... > t = std::make_tuple( subs... );
		INIT_HELPER__< sizeof...( subTypes ) ,decltype(t) >::init__( m_subs__ , t );
	}

    void run(std::function< void ( itfcType * )> fun) {
		for( int i = 0; i < sizeof...( subTypes ); i ++ ){
			if( m_subs__[ i ] != nullptr ){
				fun( m_subs__[ i ] );
			}
		}
    }

};

// 定义子接口和子接口实现类
FACADE_START_DECLARE_SUB_ITFC(MySubInterfaceA)
    FACADE_ADD_ITFC(void, subInterfaceFunction, int);
FACADE_END_DECLARE_SUB_ITFC

FACADE_START_DECLARE_SUB_ITFC(MySubInterfaceB)
    FACADE_ADD_ITFC(void, subInterfaceFunction, int);
FACADE_END_DECLARE_SUB_ITFC

// 子接口实现类A
struct MySubInterfaceAImpl : public MySubInterfaceA {
    virtual void subInterfaceFunction(int arg) override {
        std::cout << "MySubInterfaceAImpl::subInterfaceFunction called with argument: " << arg << std::endl;
    }
};

// 子接口实现类B
struct MySubInterfaceBImpl : public MySubInterface {
    virtual void subInterfaceFunction(int arg) override {
        std::cout << "MySubInterfaceBImpl::subInterfaceFunction called with argument: " << arg << std::endl;
    }
};


int main() {
    // 创建子接口实现类的对象
    MySubInterfaceAImpl subInterfaceA;
    MySubInterfaceBImpl subInterfaceB;

    // 创建外观对象，并将子接口实现类的对象传入
    facadeItfc<MySubInterfaceA, MySubInterfaceAImpl, MySubInterfaceBImpl> facade(  &subInterfaceA ,  &subInterfaceB );

	auto fun = []( MySubInterfaceA* arg ){
		arg->subInterfaceFunction( 123);
	};

    // 调用外观对象的run函数，并传入执行函数和参数
    facade.run( fun );

    return 0;
}