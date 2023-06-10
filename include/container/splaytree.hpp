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
	// 这是一个伸展树的实现，主要是方便了高频访问数据的访问。尽量保证高频访问的数据访问的时候达到O(1)的时间复杂度
	// 后续软仓中会上传示例代码和完整的代码
	template< typename keyType , typename dataType ,
		  typename midKeyType = typename std::decay< keyType>::type,       // 对key类型进行处理，提取实际的类型
		  typename midDataType = typename std::decay< dataType >::type >   // 对数据类型进行处理，提取实际的类型
	class splayTree
	{
	public:
		// 进一步提取实际的类型，移除指针修饰
		using key_type = typename std::conditional< std::is_pointer< midKeyType > ::value ,
							    typename std::remove_pointer< midKeyType >::type ,
							    midKeyType >::type;
		using value_type = typename std::conditional< std::is_pointer< midDataType > ::value ,
							      typename std::remove_pointer< midDataType >::type ,
							      midDataType >::type;

		// 检查提供的数据类型是否满足要求
		static_assert( std::is_integral< key_type >::value || std::is_enum< key_type >::value || std::is_class< key_type>::value , "key类型必须是整数类型，枚举类型或者类" );
		// 树节点数据结构定义
		struct stNode {
			keyType        m_key;
			value_type     m_data;
			stNode       * p_left;
			stNode       * p_right;
			stNode       * p_parent;

			stNode(const key_type& k, const value_type& data , stNode* l = nullptr, stNode* r = nullptr, stNode* p = nullptr) :
				m_key(k), m_data( data ), p_left(l), p_right(r), p_parent(p) {}

			stNode( const stNode& b ):
				m_key( b.m_key ), m_data( b.m_data ), p_left( nullptr ), p_right( nullptr ), p_parent( nullptr ) {}
			stNode( stNode&& b ):
				m_key( std::move( b.m_key ) ), m_data( std::move( b.m_data ) ), p_left( nullptr ), p_right( nullptr ), p_parent( nullptr ) {}
			stNode& operator=( const stNode& b ){
				m_key = b.m_key;
				m_data = b.m_data;

				p_left = nullptr;
				p_right = nullptr;
				p_parent = nullptr;

				return *this;
			}

			stNode& operator=( stNode&& b ){
				m_key = std::move( b.m_key );
				m_data = std::move( b.m_data );

				p_left = nullptr;
				p_right = nullptr;
				p_parent = nullptr;

				return *this;
			}
		};

		using node_t = stNode;

		// 定义迭代器，方便后面使用迭代器进行访问，
		// 因为使用foward_iterator
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
			// 重载->和*
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

			// 进行遍历的++操作符号重载，这里没有重载--，
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

		// 定义一个满足通常STL习惯的迭代器名字
		using iterator = iterator__;

	private:
		node_t        * p_root__;            // 根节点
		size_t          m_count__;
	private:
		/**
		 * @brief 左旋，进行伸展旋转的左旋
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
		 * @brief 右旋，
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
		 * @brief 伸展，实际执行伸展的操作，吸取网友提醒，将私有方法或者变量标记采用__后缀
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
		 * @brief 查找节点，根据key进行查找，
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
		// 公有的接口
		splayTree() : p_root__(nullptr) , m_count__(0){}
		splayTree( splayTree&& b ):p_root__( b.p_root__ ),m_count__( b.m_count__ ){
			b.p_root__ = nullptr;
			b.m_count = 0;
		}
		
		virtual ~splayTree(){
			clear();
		}

		splayTree& operator( splayTree&& b ){
			p_root__ = b.p_root__;
			m_count__ = b.m_count__;

			b.p_root__ = nullptr;
			b.m_count__ = nullptr;

			return *this;
		}
		// 迭代器的接口
		iterator begin(){ return iterator(p_root__); }
		iterator end(){ return iterator( nullptr ); }
		/**
		 * @brief 读取key内容
		 */
		const key_type& key( iterator it ){ return it.key(); }
		/**
		 * @brief 插入节点。在这个程序的实现中没有采用递归的方式实现。但是实际递归的方式更容易立即，虽然这种方式效率可能会比较低
		 * @param key[ I ], 节点的key
		 * @param data，实际的数据内容
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
		 * @brief 删除指定件节点,按照key进行删除操作
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
		 * @brief 判断是否存在指定key的数据，如果存在指定的key返回true
		 * @param key
		 * @return 
		 */
		bool has(const key_type& key) {
			return find_node__(key) != nullptr;
		}

		/**
		 * @brief 清楚数据
		 */
		void clear() {
			while (p_root__ != nullptr) {
				remove(p_root__->key);
			}
		}
		/**
		 * 遍历树，通过回调函数进行处理。如果回调函数返回false，则结束遍历
		 */
		void for_each(std::function< bool (const key_type&, const value_type&)> func) {
			if (p_root__ == nullptr) { return; }
			std::stack<node_t*> stk;   // 依然没有采用递归的方式
			node_t* p = p_root__;
			while (p != nullptr || !stk.empty()) {
				while (p != nullptr) {
					stk.push(p);
					p = p->p_left;
				}
				if (!stk.empty()) {
					p = stk.top();
					stk.pop();
					// 在治理
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
