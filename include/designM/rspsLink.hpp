/**
 * @brief 责任链模式
 * @author 宋炜
 * @version 1.0
 * @date 2023-4-27
 */

#pragma once

#include <list>
#include <type_traits>

namespace private__
{
	struct respItfc__{};
}

#define DECLARE_RESPLINK_ITFC( name )    \
	struct name :public private__::respItfc__ {
		
#define RESPLINK_ITFC_MTD( ... )   virtual bool operation(  __VA_ARGS__ ) = 0;

#define END_DECLARE_RESPLINK_ITFC()   };
	
	
namespace wheels
{
namespace dm
{
    template< typename ITFC_TYPE >
	class rspsLink
	{
	public:
		using itfc_t = typename std::remove_pointer< typename std::decay< ITFC_TYPE >::type >::type;
		static_assert( std::is_base_of< private__::respItfc__ ,  itfc_t >::value , "" );
        
        using link_t =  std::list<  itfc_t* >;
        using iterator = typename link_t::iterator;
	protected:
        link_t             m_link__;
	public:
        rspsLink(){}
		virtual ~rspsLink(){}
		
		template< typename ...Params >
        void request( Params&&... params ){
            for( auto item : m_link__ ){
				auto * p = item ;
				if( p == nullptr ) continue;
				
                bool rst = p->operation( std::forward<Params>( params)...);
                if( rst == false ){
                    break;
                }
            }
        }
		
		template< typename Func_t , typename... Params >
		void requestCallback( Func_t && fun , Params&& ...args ){
			using result_type = typename std::result_of< Func_t( itfc_t* , Params&&... ) >::type;
			static_assert( std::is_same<result_type , bool >::value , "" );
			
			for( auto item : m_link__ ){
				auto * p =  item;
				if( p == nullptr ) continue;
				
				bool rst = fun( p , std::forward<Params>(args)... );
				if( rst == false ){
                    break;
                }
			}
		}

        template< typename subType >
        void push_back(  subType * rsps ){
			using type = typename std::remove_pointer< typename std::decay<subType>::type > :: type;
            static_assert( std::is_base_of<private__::respItfc__ , type >::value , "" );
            m_link__.push_back( rsps );
        }
		
		template< typename subType >
		void insert( iterator it , subType * rsps ){
			using type = typename std::remove_pointer< typename std::decay<subType>::type > :: type;
            static_assert( std::is_base_of<private__::respItfc__ , type >::value , "" );
            m_link__.insert( it , rsps );
		}

		void erase( iterator it ){
            m_link__.erase( it );
		}
		
		void erase( iterator start , iterator end ){
            m_link__.erase( start , end );
		}

        iterator begin(){ return m_link__.begin(); }
        iterator end(){ return m_link__.end(); }
		
	};
}}