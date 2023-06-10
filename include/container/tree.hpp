/**
 * @brief 任意树的实现
 * @author 宋炜
 * @date 2022-6-10
 * @version 2.0
 */

#pragma once
#include <iostream>
#include <memory>
#include <type_traits>

namespace wheels
{
	template <typename dataType , typename midType = typename std::decay< dataType >::type >
	class tree {
	public:
		using value_type = std::conditional< std::is_pointer< midType >:: value , typename std::remove_pointer< midType >::type , midType >
	public:
		
		class treeNode {
		public:
			value_type     m_data;
			treeNode     * p_first_child;
			treeNode     * p_next_sibling;
			treeNode     * p_parent;

			explicit treeNode(const value_type& data) : m_data(data), p_first_child( nullptr ),p_next_sibling(nullptr),p_parent( nullptr) {}
			
			void addChild(treeNode* child) {
				if (!p_first_child) {
					p_first_child = child;
				} else {
					treeNode* last_child = p_first_child;
					while (last_child->p_next_sibling) {
						last_child = last_child->p_next_sibling;
					}
					last_child->p_next_sibling = child;
				}
				child->p_parent = this;
			}

			void removeChild(treeNode* child) {
				if (!p_first_child) { return; }

				if (p_first_child == child) {
					first_child = p_first_child->p_next_sibling;
					return;
				}

				treeNode* prev_child = p_first_child;
				while (prev_child->p_next_sibling && prev_child->p_next_sibling != child) {
					prev_child = prev_child->next_sibling;
				}

				if (prev_child->p_next_sibling == child) {
					prev_child->p_next_sibling = child->p_next_sibling;
					child->p_next_sibling = nullptr;
				}
			}

		private:
			treeNode* get_parent__( treeNode* node ) {
				return node->p_parent;
			}

			friend class tree<value_type >;

		};
		// 迭代器实现
		class iterator__ {
		private:
			treeNode               * p_node__;
			std::stack<treeNode*>    m_stack__;
		public:
			explicit iterator__(treeNode* node) : node_(node) {
				if( node != nullptr ){
					m_stack__.push( node );
				}
			}

			iterator__( const treeNode& b ): p_node__( b.p_node__ ) , m_stack__( b.m_stack__ ){}
			iterator__( treeNode&& b ): p_node__( b.p_node__ ) , m_stack__( std::move( b.m_stack__ ) ){
				b.p_node__ = nullptr;
			}
			
			
			value_type& operator*() const {
				if( !p_node__ ) throw std::runtime_error( "数据指针为空" );
				return p_node__->data;
			}

			value_type* operator->() const {
				if( !p_node__ ) throw std::runtime_error( "数据指针为空" );
				return &p_node__->data;
			}

			// 采用先序遍历的方式
			iterator__& operator++() {
				if( !p_node__ ) return {nullptr};
				
				if( !m_stack__.empty() ){
				        p_node__ = m_stack__.top();
					m_stack__.pop();

					node_t* child = node->p_first_child;
					while (child) {
						s.push(child);
						child = child->p_next_sibling;
					}

					
				}
				return *this;
			}

			iterator__ operator++(int) {
				iterator it(*this);
				++(*this);
				return it;
			}

			bool operator==(const iterator__& other) const {
				return node_ == other.node_;
			}
			
			bool operator!=(const iterator__& other) const {
				return !(*this == other);
			}
		};
		
		using node_t = treeNode;
		
	private:
		node_t     * p_root__;      // 根节点
		size_t       m_count__;     // 节点数量
	public:
		tree():p_root__( nullptr ) , m_count__( 0 ){}
		
		explicit tree(const value_type& data) : p_root( new treeNode(data) ), __m_count( 1 ) {}

		tree( tree&& b ):p_root__( b.p_root__ ), 
		
		virtual ~tree(){ release__(); }

		size_t count(){ return m_count__; }
		
		node_t *insert(const value_type& data, node_t * parent) {
			try{
				node_t * node = new treeNode( data );

				if( parent != nullptr ){
					parent->addChild( node );
				}else{
					p_root__ = node;
				}
				
				m_count__ ++;

				return node;
			}catch( std::bad_alloc& e ){
				std::cerr << e.what() << std::endl;
			}

			return nullptr;
		}
		
		node_t * insert( value_type&& data , node_t * parent ){
			try{
				node_t * node = new treeNode( data );
				if( parent == nullptr ){
					p_root__ = node;
				}else{
					parent->addChild( node );
				}
				m_count__ ++;

				return node;
			}catch( std::bad_alloc& e ){
				std::cerr << e.what() << std::endl;
			}
			return nullptr;
		}
		

		iterator insert( value_type&& data , iterator parent ){
			try{
				node_t * p = parent.p_node__ , * node = new treeNode(data);
				if( p != nullptr ){
					p->addChild( node );
				}else{
					p_root__ = node;
				}
				m_count__ ++;

				return iterator( node );
			}catch( std::bad_alloc& e ){
				std::cerr << e.what() << std::endl;
				ret = false;
			}
			return iterator( nullptr );
		}
		
		void erase( iterator it ){
			node_t * node = it.p_node__;
			if( node ){
				erase__( node );
			}
		}

		void erase( iterator from , iterator end ){
			for( auto it = from; it != end; it ++ ){
				erase( it );
			}
		}

		iterator begin() {
			return iterator( p_root__ );
		}

		iterator end() {
			return iterator(nullptr);
		}

		inline void traversePreOrder( std::function< bool ( node_t * ) > visit ){
			traverse_pre_rder__( visit );
		}
		
		inline void traversePostOrder(std::function<void(node_t *)> visit) {
			if( visit ){
				traverse_post_order__( visit );
			}
		}
		/**
		 */
		void traverseInOrder(std::function<void(node_t *)> visit) {
			std::stack<node_t *> s;
			node_t * node = p_root__;
			
			while (node || !s.empty()) {
				if (node) {// 将当前节点及其所有左子节点压入栈中
					s.push(node);
					node = node->p_first_child;
				} else {// 访问栈顶节点，然后转向右子树
					node = s.top();
					s.pop();
					visit(node);
					node = node->p_next_sibling;
				}
			}
		}
private:
		/**
		 * @brief 删除节点
		 */
		void erase__(node_t* node) {
			if( node == nullptr ) return;
			
			if ( !node->p_first_child ) {
				treeNode* parent = get_parent__( node );
				if (parent) {
					parent->removeChild( node );
				} else {
					p_root__ = nullptr;
				}

				delete node;
				m_count__ --;
			} else {
				std::cerr << "Cannot remove a node with children or node can not be nullptr\n";
			}
		}
		
	        void release__(){
			traverse_post_order__( [&]( node_t * node ){
						       node_t * parent = get_parent( node );
						       if( parent != nullptr ){
							       parent->removeChild( node );
						       }else{							       
							       p_node__ = nullptr;
						       }

						       delete p_node__;
					       } );
		}

		void traverse_post_order__(std::function< bool (node_t *)> visit) {
			node_t * last_visited = nullptr;
			std::stack<node_t *> s;
			s.push( p_root__ );
			
			while (!s.empty()) {
				node_t * node = s.top();
				if (!last_visited || last_visited->parent == node) {
					// 第一次访问该节点，将其子节点压入栈中
					node_t * child = node->p_first_child;
					while (child) {
						s.push(child);
						child = child->p_next_sibling;
					}
				} else {
					// 第二次访问该节点，访问该节点并弹出栈
					bool rst = visit(node);
					if( rst == false ) break;
					s.pop();
				}
				last_visited = node;
			}
		}
		/**
		 * @brief 先序遍历
		 * 
		 */
		void traverse_pre_order__(std::function< bool (node_t*)> visit) {
			std::stack<node_t*> s;
			s.push( p_root__ );
			
			while (!s.empty()) {
				node_t* node = s.top();
				s.pop();

				bool rst = visit(node);
				if( rst == false ) break;
				
				node_t* child = node->p_first_child;
				while (child) {
					s.push(child);
					child = child->p_next_sibling;
				}
			}
		}
		/**
		 * @brief 获取父节点
		 */
		node_t * get_parent__( node_t * node) {
			if( node == nullptr) return nullptr;
			
			return node->p_parent;
		}
		friend class treeNode;

	};
}
