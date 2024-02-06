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
// 最简单情况下的接口类型定义，通常使用这个接口就够用了。但是在一些复杂得应用中
// 就需要使用后面的宏方式来声明接口
template< typename RET , typename ...PARAMS >
struct itptorItfc : public itprtor_private__::interpretor_interface__
{
	using ret_type = RET;
	
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
	/**
	 * @brief 构造解释器对象并
	*/
    template< typename ...Params >
	static std::shared_ptr< iterpretor<itfc_t , impl_t > > 
	make_shared( Params&& ...args ){
		auto ret = std::make_shared<iterpretor< itfc_t , impl_t > >();
		ret->pt_impl__ = std::make_shared< impl_t >( std::forward< Params >(args)... );
		return ret;
	}
	/**
	 * @brief 使用用默认的接口执行任务
	 * @tparam Args[ I ],传递给执行器的参数类型表
	 * @param str[ I ], 要执行的字符串
	 * @param args[ IO ], 参数表
	 * @return 返回接口定义的参数类型
	*/
	template< typename... Args >
    itfc_t::ret_type execStrAsync( const std::string& str ,  Args&& ...args )
	{
		
        std::packaged_task< itfc_t::ret_type > 
			task( std::bind( impl_t::execString, pt_impl__ , str , std::forward<Args>(args)...) );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );

		thd.detach();

		return ret;
	}
	/**
	 * @brief 使用用默认的接口执行任务
	 * @tparam Args[ I ],传递给执行器的参数类型表
	 * @param str[ I ], 要执行文件路径
	 * @param args[ IO ], 参数表
	 * @return 返回接口定义的参数类型
	*/
	template< typename... Args >
    itfc_t::ret_type execFAsync( const std::string& file , Args&& ...args )
	{
        std::packaged_task< itfc_t::ret_type > task( std::bind( impl_t::execFile , std::forward<Args>(args)...) );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );
		
		thd.detach();
				
		return ret;
	}
	
	/**
	 * @brief 异步执行字符串脚本
	 * @tparam Func_t,
	 * @tparam Args,
	 * @param str[ I ],
	 * @param func[ I ],
	 * @param args[ OI ],
	*/
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
	
	/**
	 * @brief 异步的执行脚本文件
	 * @tparam Func_t,
	 * @tparam Args,
	 * @param file[ I ],
	 * @param func[ I ],
     * @param args[ IO ],
	*/
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