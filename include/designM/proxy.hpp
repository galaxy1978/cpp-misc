/**
 * @brief 代理模式
 * @version 1.0
 * @author 宋炜
*/

#pragma once
#include <type_traits>
#include <memory>

namespace private__
{
	struct itfcBase{};
}

namespace wheels{namespace dm {
	template< typename RET , typename ...PARAMS >
	struct proxyItfc : public private__::itfcBase
	{
		virtual RET agent( PARAM&& ...args ) = 0;
	};

	template< typename ITFC_TYPE , typename CONCREATE_TYPE >
	class proxy : public ITFC_TYPE
	{
	public:
		using itfc_t = typename std::remove_pointer< typename std::decay< ITFC_TYPE >::type >::type;
		using concrete_t  = typename std::remove_pointer< typename std::decay< CONCREATE_TYPE > :: type > :: type;
		
		static_assert( std::is_base_of<itfc_t , private__::itfcBase >::value , "" );
	protected:
		std::shared_ptr< concrete_t > pt_cncrt__;
	protected:
		std::shared_ptr< concrete_t > get__(){ 
			std::weak_ptr ret( pt_cncrt__ );
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
	};
}}

 // 使用方法：
/*	
	using itfc = proxyItfc< int , const std::string& >;
	
	class abc{
	public:
			int do( const std::string& a , int b ){}
	};
	
	
	class myProxy : public proxy< itfc , abc >{
		virtual int agent( const std::string& str ) override{
			int param1 = 0;
			param1 = 12;
			
			// ... ... 执行其他操作
			
			// 代理到具体类中
			auto ptr = get()__;
			if( ptr ){
				ptr->do( str , 12 );
			}
			return 0;
		}
	};
*/