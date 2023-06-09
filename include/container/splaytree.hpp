/**
 * @brief 伸展树实现
 * @version 1.0
 * @author 宋炜
 * @date 2023-4-18
 */
#pragma once

#include <iterator>
#include <iostream>
#include <type_traits>
#include <list>
#include <stack>
#include <functional>

namespace wheels
{
	template< typename keyType , typename dataType , typename midKeyType = typename std::decay< keyType>::type, typename midDataType = typename std::decay< dataType >::type >
	class splayTree
	{
	public:
		using key_type = typename std::conditional< std::is_pointer< midKeyType > ::value ,
							    typename std::remove_pointer< midKeyType >::type ,
							    midKeyType >::type;
		using value_type = typename std::conditional< std::is_pointer< midDataType > ::value ,
							      typename std::remove_pointer< midDataType >::type ,
							      midDataType >::type;

		static_assert( std::is_integral< key_type >::value || std::is_enum< key_type >::value || std::is_class< key_type>::value , "key类型必须是整数类型，枚举类型或者类" );

		struct stNode {
			keyType        m_key;
			value_type     m_data;
			stNode       * p_left;
			stNode       * p_right;
			stNode       * p_parent;

			stNode(const key_type& k, const value_type& data , stNode* l = nullptr, stNode* r = nullptr, stNode* p = nullptr) :
				m_key(k), m_data( data ), p_left(l), p_right(r), p_parent(p) {}
		};

		using node_t = stNode;

		// 定义迭代器
		class iterator__ : public std::iterator< std::forward_iterator_tag , node_t >{
			friend class splayTree;
		private:
			node_t * p_node__;
			std::list< node_t* > m_stack__;
		public:
			iterator__():p_node__( nullptr ){}
			iterator__( node_t * n ): p_node__( n ){}
			iterator__( const iterator__& b ):p_node__( b.p_node__ ){}

			iterator__& operator=( const iterator__& b ){
				p_node__ = b.p_node__;
				m_stack__ = b.m_stack__;
				return *this;
			}

			iterator__& operator=( iterator__&& b ){
				p_node__ = b.p_node__;
				m_stack__ = std::move( b.m_stack__ );

				return *this;
			}

			bool operator ==( const iterator__& b ){
				return p_node__ == b.p_node__;
			}

			bool operator != ( const iterator__& b ){
				return p_node__ != b.p_node__;
			}

			node_t * operator->(){
				if( p_node__ == nullptr ){
					throw std::runtime_error( "data pointer is null" );
				}
				return &p_node__->__m_data;
			}

			value_type& operator*(){
				if( p_node__ == nullptr ){
					throw std::runtime_error( "data pointer is null" );
				}
				return p_node__->__m_data;
			}

			iterator__& operator++(){
				node_t * top = nullptr;
				if( m_stack__.size() > 0 ){
					top =m_stack__.back();
				}
				// 优先访问左子树
				if( ( top == nullptr ) ||                  // 允许空栈的时候访问第一个节点
				    // 如果访问左节点的时候如果发现了已经访问了右节点说明左节点已经被访问过
				    //                   不要访问已经访问过的左节点     不要访问已经访问过右节点的左节点
				    ( top != nullptr &&  top->p_left != p_node__ &&  top->p_right != p_node__ &&   p_node__->p_left != nullptr ) ){
					m_stack__.push_back( p_node__ );
					p_node__ = p_node__->p_left;
					return *this;
				}
				// 访问右子树
				if( top != nullptr &&   top->p_right != p_node__ &&  p_node__->p_right != nullptr ){
					m_stack__.push_back( p_node__ );
					p_node__ = p_node__->p_right;
					return *this;
				}

				p_node__ = top;
				m_stack__.pop_back();
				if( m_stack__.size() > 0 ){ // 递归调用以弹出栈顶
					return operator++();
				}else{ // 满足end()条件
					p_node__ = nullptr;
				}

				return *this;
			}
			const key_type& key(){
				if( p_node__ == nullptr ){
					throw std::runtime_error( "data pointer is null" );
				}

				return p_node__->m_key;
			}
		};


		using iterator = iterator__;

	private:
		node_t        * p_root__;
		size_t          m_count__;
	private:
		/**
		 * @brief 左旋
		 */
		void rotate_left__( node_t * x) {
			node_t* y = x->p_right;
			x->p_right = y->p_left;
			if (y->p_left != nullptr) {
				y->p_left->p_parent = x;
			}

			y->parent = x->p_parent;

			if (x->p_parent == nullptr) {
				p_root__ = y;
			}else if (x == x->p_parent->p_left) {
				x->p_parent->p_left = y;
			}else {
				x->p_parent->p_right = y;
			}
			y->p_left = x;
			x->p_parent = y;
		}
		/**
		 * @brief 右旋
		 */
		void rotate_right__(node_t* x) {
			node_t* y = x->p_left;
			x->p_left = y->p_right;
			if (y->p_right != nullptr) {
				y->p_right->p_parent = x;
			}

			y->p_parent = x->p_parent;

			if (x->p_parent == nullptr) {
				p_root__ = y;
			}else if (x == x->p_parent->p_left) {
				x->p_parent->p_left = y;
			}else {
				x->p_parent->p_right = y;
			}
			y->p_right = x;
			x->p_parent = y;
		}
		/**
		 * @brief 伸展
		 */
		void splay__(node_t* x) {
			while (x->p_parent != nullptr) {
				if (x->p_parent->p_parent == nullptr) {
					if (x == x->p_parent->p_left) {
						rotate_right__(x->p_parent);
					}else {
						rotate_left__(x->p_parent);
					}
				}
				else if (x == x->p_parent->p_left && x->p_parent == x->p_parent->p_parent->p_left) {
					rotate_right__(x->p_parent->p_parent);
					rotate_right__(x->p_parent);
				}else if (x == x->p_parent->p_right && x->p_parent == x->p_parent->p_parent->p_right) {
					rotate_left__(x->p_parent->p_parent);
					rotate_left__(x->p_parent);
				}else if (x == x->p_parent->p_right && x->p_parent == x->p_parent->p_parent->p_left) {
					rotate_left__(x->p_parent);
					rotate_right__(x->p_parent);
				}else {
					rotate_right__(x->p_parent);
					rotate_left__(x->p_parent);
				}
			}
		}
		/**
		 * @brief 查找节点
		 * @param key[ I ], 节点键
		 * @return 成功找到返回节点指针否则返回nullptr
		 */
		node_t* find_node__( const key_type& key) {
			node_t* x = p_root__;
			while (x != nullptr) {
				if (key < x->key) {
					x = x->p_left;
				}else if (key > x->key) {
					x = x->p_right;
				}else {
					return x;
				}
			}
			return nullptr;
		}

	public:
		splayTree() : p_root__(nullptr) , m_count__(0){}
		virtual ~splayTree(){
			clear();
		}

		iterator begin(){ return iterator(p_root__); }
		iterator end(){ return iterator( nullptr ); }
		/**
		 * @brief 读取key内容
		 */
		const key_type& key( iterator it ){ return it.key(); }
		/**
		 */
		bool insert(const key_type& key , const value_type& data ) {
			bool ret = true;
			if (p_root__ == nullptr) {
				try{
					p_root__ = new node_t(key , data );
					m_count__ = 1;
				}catch( std::bad_alloc& e ){
					std::cerr << e.what() << std::endl;
					ret = false;
				}
				return ret ;
			}
			node_t * x = p_root__;
			node_t * y = nullptr;
			while (x != nullptr) {
				y = x;
				if (key < x->key) {
					x = x->p_left;
				}else if (key > x->key) {
					x = x->p_right;
				}else {
					return false;
				}
			}
			try{
				x = new node_t(key);
				m_count__ ++;
				x->p_parent = y;
				if (key < y->key) {
					y->p_left = x;
				}else {
					y->p_right = x;
				}
				splay__(x);
			}catch( std::bad_alloc& e ){
				std::cerr << e.what() << std::endl;
				ret = false;
			}

			return ret;
		}
		/**
		 * @brief 删除指定件节点
		 */
		void remove(const key_type& key) {
			node_t* x = find_node__(key);
			if (x == nullptr) {
				return;
			}
			splay__(x);
			if (x->p_left == nullptr) {
				p_root__ = x->p_right;
				if (x->p_right != nullptr) {
					x->p_right->p_parent = nullptr;
				}
			}else if (x->p_right == nullptr) {
				p_root__ = x->p_left;
				if (x->p_left != nullptr) {
					x->p_left->p_parent = nullptr;
				}
			}else {
				node_t* y = x->p_left;
				while (y->p_right != nullptr) {
					y = y->p_right;
				}
				splay__(y);
				y->p_right = x->p_right;
				x->p_right->p_parent = y;
				p_root__ = y;
				y->p_parent = nullptr;
			}
			m_count__ --;
			delete x;
		}
		/**
		 * @brief 判断是否存在指定key的数据
		 * @param key
		 * @return 
		 */
		bool has(const key_type& key) {
			return find_node__(key) != nullptr;
		}

		void clear() {
			while (p_root__ != nullptr) {
				remove(p_root__->key);
			}
		}
		/**
		 * 
		 */
		void for_each(std::function< bool (const key_type&, const value_type&)> func) {
			if (p_root__ == nullptr) { return; }
			std::stack<node_t*> stk;
			node_t* p = p_root__;
			while (p != nullptr || !stk.empty()) {
				while (p != nullptr) {
					stk.push(p);
					p = p->p_left;
				}
				if (!stk.empty()) {
					p = stk.top();
					stk.pop();
					bool rst = func(p->key, p->data);
					if( rst == false ){
						break;
					}
					p = p->p_right;
				}
			}
		}
	};
}
