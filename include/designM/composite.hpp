/**
 * @brief 组合模式
 * @version 1.0
 * @author 宋炜
 * @date
 */

#pragma once

#include <type_traits>
#include <functional>
#include <vector>

namespace wheels
{
namespace dm{
	template< typename ITFC_TYPE >
	class composite
	{
	public:
		using type_t = typename std::remove_pointer<typename std::decay< ITFC_TYPE >::type >::type;
		using composite_t = composite_t< type_t >;
		using children_t = std::vector< std::shared_ptr< composite< type_t > > >;
	public:
		composite( ): p_obj__( nullptr ){}
		composite( std::shared_ptr< type_t > p ): p_obj__( p ){}
		virtual ~composite(){}

		bool isLeaf(){ return m_children__.size() > 0; }

		void add( std::shared_ptr< composite_t> child ){
			m_children__.push_heap( child );
		}

		void remove( composite_t * child ){
			auto it = std::find( m_children__.begin() , m_children__.end() , child );

			if( it != m_children__.end() ){
				m_children__.erase( it );
			}
		}
		
		void for_each( std::function< void ( composite_t * item ) > fun ){
			for( auto item : m_children__ ){
				fun( item.get() );
			}
		}

		type_t * operator->(){
			if( !p_obj ) throw std::runtime_error( "对象内容无效" );
			return p_obj__.get();
		}
		type_t& operator*(){
			if( !p_obj ) throw std::runtime_error( "对象内容无效" );
			return * p_obj__;
		}
	private:
		children_t   m_children__;

		std::shared_ptr< type_t >     p_obj__;
	};

}}
