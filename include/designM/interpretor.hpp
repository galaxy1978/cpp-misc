/**
 * @brief 解释器模式
 * @version 1.0
 * @author 宋炜
 */
 
#pragma once

#include <type_traits>
#include <memory>
#include <future>

namespace wheels{namespace dm{
tempalte< typename RET , typename ...PARAMS >
struct itptorItfc
{
	virtual RET execString(const std::string& str, PARAMS&& ...args) = 0; 
	virtual RET execFile(const std::string& file, PARAMS&& ...args) = 0; 
};
// 实际解释器可以使用外部的脚本语言，比如lua，js和python等等
template < typename ITFC_TYPE , typename IMPL_TYPE , typename RET , typename ...PARAMS>
class iterpretor
{
public:
	using itfc_t = typename std::remove_pointer< typename std::decay< ITFC_TYPE >::type >::type;
	using impl_t = typename std::remove_pointer< typename std::decay< IMPL_TYPE >::type >::type;
	
	static_assert( std::is_base_of<itfc_t , itptorItfc<RET , PARAMS... > >::value , "" );
	static_assert( std::is_base_of<impl_t , itfc_t >::value , "" );
protected:
	std::shared_ptr< impl_t >    pt_impl__;
protected:
	iterpretor(){}
public:
	virtual ~iterpretor(){}
	
	tempalte< typename ...Params >
	static std::shared_ptr< iterpretor<itfc_t , impl_t , RET , PARAMS...> > 
	make_shared( Params&& ...args ){
		auto ret = std::make_shared<iterpretor< itfc_t , impl_t , RET , PARAMS...> >();
		ret->pt_impl__ = std::make_shared< impl_t >( std::forward< Params >(args)... );
		return ret;
	}
	
	std::future< RET > execString( const std::string& str , bool async , PARAMS&& ...args ){
		std::packaged_task< RET (PARAMS...) > task( [=]->RET{
			return pt_impl__->execString( str , std::forward<PARAMS>(args)...);
		} );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );
		if( async ){
			thd.detach();
		}else{
			thd.join();
		}
		
		return ret;
	}
	

	std::future< RET > execFile( const std::string& file , bool async , PARAMS&& ...args ){
		std::packaged_task< RET (PARAMS...) > task( [=]->RET{
			return pt_impl__->execFile( file , std::forward<PARAMS>(args)...);
		} );
		auto ret = task.get_future();
		
		std::thread thd( std::move(task) );
		if( async ){
			thd.detach();
		}else{
			thd.join();
		}
		
		return ret;
	}
};
}}