/**
 * @brief 解释器模式
 * @version 1.0
 * @author 宋炜
 */
 
#pragma once

#include <type_traits>
#include <memory>
#include <future>

namespace itprtor_private__
{
	struct interpretor_interface__{};
}
namespace wheels{namespace dm{
	
template< typename RET , typename ...PARAMS >
struct itptorItfc : public itprtor_private__::interpretor_interface__
{
    virtual RET execString(const std::string& str, PARAMS&& ...args) = 0;
    virtual RET execFile(const std::string& file, PARAMS&& ...args) = 0;
};

#define DECLARE_INTERPRETOR_ITFC( name ) \
struct name : public interpretor_interface__

#define INTERPRETOR_ITFC( RET , name , ... )  virtual RET name( __VA_ARGS__ ) = 0;

#define END_DECLARE_INTERPRETOR_ITFC()  };

// 实际解释器可以使用外部的脚本语言，比如lua，js和python等等
template < typename ITFC_TYPE , typename IMPL_TYPE >
class iterpretor
{
public:
	using itfc_t = typename std::remove_pointer< typename std::decay< ITFC_TYPE >::type >::type;
	using impl_t = typename std::remove_pointer< typename std::decay< IMPL_TYPE >::type >::type;
	static_assert( std::is_base_of< itprtor_private__::interpretor_interface__ , itfc_t >::value , "" );
	static_assert( std::is_base_of< itfc_t , impl_t >::value , "" );
protected:
	std::shared_ptr< impl_t >    pt_impl__;
protected:
	
public:
	iterpretor(){}
	virtual ~iterpretor(){}
	
    template< typename ...Params >
	static std::shared_ptr< iterpretor<itfc_t , impl_t > > 
	make_shared( Params&& ...args ){
		auto ret = std::make_shared<iterpretor< itfc_t , impl_t > >();
		ret->pt_impl__ = std::make_shared< impl_t >( std::forward< Params >(args)... );
		return ret;
	}
	
	template< typename Func_t , typename... Args >
    auto execStringAsync( const std::string& str , Func_t&& func ,  Args&& ...args )
		->std::future< typename std::result_of<Func_t( std::shared_ptr<impl_t> , const std::string& , Args...)>::type >
	{
		using rst_type = typename std::result_of<Func_t(std::shared_ptr<impl_t> , const std::string& , Args...)>::type;
        std::packaged_task< rst_type() > 
			task( std::bind( std::move(func) , pt_impl__ , str , std::forward<Args>(args)...) );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );

		thd.detach();

		return ret;
	}
	

    template< typename Func_t , typename... Args >
    auto execFileAsync( const std::string& file , Func_t&& func , Args&& ...args )
		->std::future< typename std::result_of<Func_t( std::shared_ptr<impl_t> , const std::string& , Args...)>::type >
	{
		using rst_type = typename std::result_of<Func_t( std::shared_ptr<impl_t> , const std::string& , Args...)>::type;
        std::packaged_task< rst_type() > task( std::bind( func , std::forward<Args>(args)...) );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );
		
		thd.detach();
				
		return ret;
	}
};
}}