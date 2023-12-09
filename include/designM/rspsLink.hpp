/**
 * @brief 责任链模式
 * @author 宋炜
 * @version 1.0
 * @date 2023-4-27
 */

#pragma once

#include <list>

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
        
        using link_t =  std::list< itfc_t * >;
        using iterator = typename link_t::iterator;
	protected:
        link_t             m_link__;
	public:
        rspsLink(){}
		virtual ~rspsLink(){}
		
		template< typename ...Params >
        void request( Params&&... params ){
            for( auto item : m_link__ ){
				auto * p = dynamic_cast< itfc_t *>( item );
				if( p == nullptr ) continue;
				
                bool rst = p->operation( std::forward<Params>( params)...);
                if( rst == false ){
                    break;
                }
            }
        }

        template< typename subType >
        void push_back( subType * rsps ){
            static_assert( std::is_base_of<private__::respItfc__ , subType >::value , "" );
            m_link__.push_back( rsps );
        }

		void erase( iterator it ){
            m_link__.erase( it );
		}
		
		void clear(){
            m_link__.erase( m_link__.begin() , m_link__.end() );
		}


        iterator begin(){ return m_link__.begin(); }
        iterator end(){ return m_link__.end(); }
		
	};
}}