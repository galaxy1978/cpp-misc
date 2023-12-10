/**
 * @brief 代理模式
 * @version 1.1
 * @author 宋炜
*/

#pragma once
#include <type_traits>
#include <memory>
#include <future>
#include <thread>
#include <functional>

namespace private__
{
	struct itfcBase{};
}

namespace wheels{namespace dm {
	template< typename RET , typename ...PARAMS >
	struct proxyItfc : public private__::itfcBase
	{
        virtual RET agent( PARAMS&& ...args ) = 0;
	};

	#define DECLARE_PROXY_ITFC( name )    \
	struct name : public private__:: itfcBase{
		
	#define PROXY_ITFC_MTD( ret , name , ... )   virtual ret name(  __VA_ARGS__ ) = 0;

	#define END_PROXY_ITFC()   };
	
	template< typename ITFC_TYPE , typename CONCREATE_TYPE >
	class proxy : public ITFC_TYPE
	{
	public:
		using itfc_t = typename std::remove_pointer< typename std::decay< ITFC_TYPE >::type >::type;
		using concrete_t  = typename std::remove_pointer< typename std::decay< CONCREATE_TYPE > :: type > :: type;
	protected:
		std::shared_ptr< concrete_t > pt_cncrt__;

	protected:
		std::shared_ptr< concrete_t > get__(){ 
			std::weak_ptr<concrete_t> ret( pt_cncrt__ );
			if( !ret.expired() ){
				return ret.lock();
			}
			
			return std::shared_ptr<concrete_t>( nullptr );
		}
	public:
		proxy(){}
		proxy( std::shared_ptr< concrete_t > ptr ):pt_cncrt__( ptr ){}
		virtual ~proxy(){}

		template< typename ...Args >
		bool create( Args&& ...args ){
			bool ret = true;
			try{
				pt_cncrt__ = std::make_shared<concrete_t>( std::forward<Args>( args )... );
				if( !pt_cncrt__ ){
					ret = false;
				}
			}catch( std::bad_alloc& ){
				ret = false;
			}catch( ... ){
				ret = false;
			}
			
			return ret;
		}		
		/**
		 * @brief 代理函数，将实际代理处理放在回调函数中进行。并使用异步执行的方式运行
		 * @tparam RET，函数对象的返回值
		 * @tparam PARAMS，可变的回调函数对象
		 * @param cb, 实际的代理处理函数对象，这个函数对象的参数也是可变的
		 * @return 返回std::future对象
		*/
		template< typename Func_t , typename ...PARAMS >
        auto agentCall( Func_t&& cb , PARAMS&& ...args  ) 
			-> std::future<
				typename std::result_of<
						Func_t(std::shared_ptr< concrete_t > ,PARAMS&&...)
					>::type
			   >
        {
			using return_type = typename std::result_of<
					Func_t( std::shared_ptr< concrete_t > , PARAMS&&...)
					>::type;
			
			std::packaged_task< return_type() > task(std::bind(std::forward<Func_t>(cb), pt_cncrt__ , std::forward<PARAMS>(args)...) );
			
            auto ret = task.get_future();

            std::thread thd( std::move( task ) );
			thd.detach();
			return ret;
		}
	};
}}