#include <iostream>
#include <tuple>
#include <functional>
#include <type_traits>


// 适配者类
class Adaptee1 {
public:
    virtual void specificRequest() = 0;
};

class Adaptee : public Adaptee1 {
public:
    virtual void specificRequest() override {
        std::cout << "Adaptee's specific request." << std::endl;
    }
};


template< int N , typename adapterType , typename tplType >
struct CALLER_HELPER__{
	static void call( adapterType * adpter , tplType& t  ){
		if( adpter && adpter->m_callback__ ){
			adpter->m_callback__( &std::get< N - 1 >( t ) );
			CALLER_HELPER__< N - 1 , adapterType , tplType>::call( adpter , t );
		}
	}
};
	
template< typename adapterType , typename tplType  >
struct CALLER_HELPER__< 0 ,  adapterType , tplType>{
	
	static void call( adapterType * adpter , tplType& t ){
		(void)adpter;
		(void)t;
	}
};

template < typename itfcType , typename... implType >
struct TYPE_CHK_HELPER__{};

template< typename itfcType , typename implType1 , typename... implType >
struct TYPE_CHK_HELPER__< itfcType , implType1 , implType...> //: public TYPE_CHK_HELPER__< itfcType , implType... >
{
	static_assert( std::is_base_of< itfcType , implType1 >::value , "" );
};

template< typename itfcType >
struct TYPE_CHK_HELPER__<itfcType> {};

// 适配器类
template <typename itfcType , typename... T>
class Adapter {
public:
    Adapter(const T&... adap) : m_adaptees__(adap...) {}
	
	void set( std::function< void ( itfcType* ) > fun ){ m_callback__ = fun; }
	
    void request() {
        CALLER_HELPER__< sizeof...(T) , Adapter<itfcType , T...> , std::tuple<T...> >::call( this , m_adaptees__ );
    }
	
	std::function< void ( itfcType* ) >  m_callback__;
protected:
    std::tuple<T...>    m_adaptees__;
	
	TYPE_CHK_HELPER__<itfcType , T...>   m_chk_helper__[0];
};


	
int main() {
    // 创建适配者
    Adaptee adaptee1;
    Adaptee adaptee2;

    // 创建适配器
    Adapter<Adaptee1 , Adaptee, Adaptee> adapter(adaptee1, adaptee2);
	
	adapter.set( [](Adaptee1* adtee ){
		adtee->specificRequest();
	} );
    // 使用适配器调用目标接口
    adapter.request();

    return 0;
}