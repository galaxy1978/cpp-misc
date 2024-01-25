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
#include <stack>
#include <memory>

namespace composite_private__
{
	struct stCompositeItfcBase__{};
}

#define DECLARE_COMPOSITE_ITFC( name )		\
struct name : public composite_private__::stCompositeItfcBase__ { \
	virtual ~name(){}
	
#define COMPOSITE_METHOD( name , ... )     virtual bool name( __VA_ARGS__ ) = 0;

#define END_COMPOSITE_ITFC()      };

#define DECLARE_COMPOSITE_ITFC_DEFAULT( itfcName , ... )		\
	DECLARE_COMPOSITE_ITFC( itfcName )							\
	COMPOSITE_METHOD( operation , __VA_ARGS__ )					\
	END_COMPOSITE_ITFC()
	
namespace wheels
{
namespace dm{
	
	template< typename DATA_TYPE >
	class composite
	{
	public:
		using type_t = typename std::remove_pointer<typename std::decay< DATA_TYPE >::type >::type;
		
		static_assert( std::is_base_of<composite_private__::stCompositeItfcBase__ , type_t>::value , "" );
		
        using composite_t = composite< type_t >;
		using children_t = std::vector< std::shared_ptr< composite< type_t > > >;
        using iterator = typename children_t::iterator;
	public:
		composite( ): p_obj__( nullptr ){}
		composite( std::shared_ptr< type_t > p ): p_obj__( p ){}
		composite( type_t&& data ){
            p_obj__ = std::make_shared<type_t>( data );
		}
		virtual ~composite(){}
		/**
		 * @brief 判断是否是叶子节点
		 */
		bool isLeaf(){ return m_children__.size() > 0; }
		/**
		 * @brief 添加子节点
		 */
		void add( std::shared_ptr< composite_t> child ){
			if( !child ) return;
            m_children__.push_back( child );
		}

		void remove( std::shared_ptr< composite_t> child ){
			if( !child ){
				return;
			}
			auto it = std::find( m_children__.begin() , m_children__.end() , child );

			if( it != m_children__.end() ){
				m_children__.erase( it );
			}
		}
        /**
         * @brief 先序遍历操作
         * @param func[ I ]， 回调函数，返回true继续遍历，否则结束遍历操作
         */
        void preOrderTraversal(std::function<bool (type_t*)> func) {
            if( isLeaf() ) return;
            std::stack<iterator> stack;
            stack.push(m_children__.begin());
            while (!stack.empty()) {
                auto it = stack.top();
                stack.pop();

                if( !it->get()) ) break;

                if (!it->get()->isLeaf()) {
                    stack.push(it->begin());
                    stack.push(std::next(it->begin()));
                }
            }
        }
		
		/**
         * @brief 先序遍历操作
         * @param func[ I ]， 回调函数，返回true继续遍历，否则结束遍历操作
         */
		template< typename ...PARAMS >
        void preOrderTraversalDefault( PARAMS&& ...params ) {
            if( isLeaf() ) return;
            std::stack<iterator> stack;
            stack.push(m_children__.begin());
            while (!stack.empty()) {
                auto it = stack.top();
                stack.pop();

                if( !it->get()->operation( std::forward<PARAMS>(params)... ) ) break;

                if (!it->get()->isLeaf()) {
                    stack.push(it->begin());
                    stack.push(std::next(it->begin()));
                }
            }
        }
		
		template< typename ...PARAMS >
		void for_eachDefault( PARAMS&&... params ){
			for( auto item : m_children__ ){
				bool rst = it->get()->operation( std::forward<PARAMS>(params)... ) );
				if( !rst ) break;
			}
		}
		
        /**
         * @brief 本层遍历
         * @param fun
         */
        void for_each( std::function< void ( type_t * ) > fun ){
			for( auto item : m_children__ ){
				bool rst = fun( item.get() );
				if( !rst ) break;
			}
		}
		
		type_t * get(){ return p_obj__.get(); }

	protected:
		children_t   m_children__;

		std::shared_ptr< type_t >     p_obj__;
	};

}}
