/**
 * @brief 责任链模式
 * @author 宋炜
 * @version 1.0
 * @date 2023-4-27
 */

#pragma once

#include <list>

namespace wheels
{
namespace dm
{
    template< typename ...Params >
	class rspsLink
	{
	public:
        struct itfc
        {
            virtual bool operation( Params&& ... args ) = 0;
        };

        using itfc_t = itfc;
        using link_t =  std::list< itfc * >;
        using iterator = typename link_t::iterator;
	protected:
        link_t             m_link__;
	public:
        rspsLink(){}
		virtual ~rspsLink(){}

        void request( Params&&... params ){
            for( auto item : m_link__ ){
                bool rst = item->operation( std::forward<Params>( params)...);
                if( rst == false ){
                    break;
                }
            }
        }

        template< typename subType >
        void push_back( subType * rsps ){
            static_assert( std::is_base_of<itfc_t , subType >::value , "" );
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
