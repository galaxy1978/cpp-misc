/**
 * @brief 红黑树的实现
 * @version 1.0
 * @author 宋炜
 * @date 
 */

#pragma once
#include <atomic>
#include <memory>
#include <list>
#include <iterator>
#include <iostream>

#if !defined( THREAD_SAFE )
#    define THREAD_SAFE 1
#endif

#if THREAD_SAFE
#    include <mutex>
#endif
namespace wheels
{
	template <typename keyType , typename dataType >
	class rbTree {
	public:
		// 颜色相关变量定义
		using color = bool;
		const static bool      RED = true;
		const static bool      BLACK   = false;
		using value_type = dataType;
		// 树节点定义
		struct stNode {
			keyType               m_key;
			dataType              m_val;
			color                 m_color;
			stNode              * p_left;
			stNode              * p_right;

			stNode(const keyType& k, const dataType& value , color red = BLACK ) : m_key( k ) , m_val( value ), m_color( red ), p_left(nullptr), p_right(nullptr) {}
			stNode( keyType&& k, dataType&& value , color red = BLACK ) : m_key( k ) , m_val( value ), m_color( red ), p_left(nullptr), p_right(nullptr) {}
			stNode(const stNode& b ):m_key( b.m_key ) , m_val( b.m_val ), m_color( b.m_color ), p_left(nullptr), p_right(nullptr){}

			stNode& operator=( const stNode& b ){
				m_key = b.m_key;
				m_val = b.m_val;
				m_color = b.m_color;
				return *this;
			}

			stNode& operator=( stNode&& b ){
				m_key = std::move( b.m_key );
				m_val = std::move( b.m_val );
				m_color = b.m_color;
				return *this;
			}
		};

		using node_t = stNode;
		// 迭代器定义
		class __stIterator : public std::iterator< std::forward_iterator_tag , stNode >{
		private:
			stNode                * __p_node;
			std::list< node_t * >   __m_stack;   // 迭代时候需要将数据进行压栈
		public:
			__stIterator( node_t * node ):__p_node( node ){}
			__stIterator( const __stIterator& b ): __p_node( b.__p_node ),__m_stack(b.__m_stack){}
			/**
			 * @brief 采用先序遍历迭代。
			 */
			__stIterator& operator++(){
				node_t * top = nullptr;
				if( __m_stack.size() > 0 ){
					top =__m_stack.back();
				}
				// 优先访问左子树
				if( ( top == nullptr ) ||                  // 允许空栈的时候访问第一个节点
				    // 如果访问左节点的时候如果发现了已经访问了右节点说明左节点已经被访问过
				    //                   不要访问已经访问过的左节点     不要访问已经访问过右节点的左节点   
				    ( top != nullptr &&  top->p_left != __p_node &&  top->p_right != __p_node &&   __p_node->p_left != nullptr ) ){
					__m_stack.push_back( __p_node );
					__p_node = __p_node->p_left;
					return *this;
				}
				// 访问右子树
				if( top != nullptr &&   top->p_right != __p_node &&  __p_node->p_right != nullptr ){
					__m_stack.push_back( __p_node );
					__p_node = __p_node->p_right;
					return *this;
				}
			
				__p_node = top;
				__m_stack.pop_back();
				if( __m_stack.size() > 0 ){ // 递归调用以弹出栈顶
					return operator++();
				}else{ // 满足end()条件
					__p_node = nullptr;
				}
			
				return *this;
			}
			
			__stIterator operator++(int){
				__stIterator tmp( *this );
				operator ++();
				return tmp;
			}
		
			bool operator==( const __stIterator& b ) const{ return __p_node == b.__p_node; }
			bool operator!=( const __stIterator& b ) const{ return __p_node != b.__p_node; }
		
			value_type& operator*() { return __p_node->m_val; }
			value_type* operator->() { return &__p_node->m_val; }
		};
		// 基本类型定义
		using iterator = __stIterator;
	private:
		stNode    * __p_root;            // 根节点
		size_t      __m_count;           // 节点数量
#if THREAD_SAFE
		std::mutex  __m_mutex;
#endif
	public:
		rbTree() : __p_root(nullptr) , __m_count(0) {}
		
		virtual ~rbTree(){
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif
			__release( __p_root );
			__p_root = nullptr;
		}
		/**
		 * @brief 清理内存
		 */
		void clear(){
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif
			__release( __p_root );
			__p_root = nullptr;
			__m_count = 0;
		}

		iterator begin(){ return iterator( __p_root ); }

		iterator end(){ return iterator( nullptr ); }
		/**
		 * @brief 查找数据
		 * @param key[ I ], 数据键名称
		 * @return 找到返回数据迭代器，否则返回end()
		 */
		iterator find( const keyType& key ){
			stNode* curr = __p_root;
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif			
			// 查找节点
			while (curr && curr->m_key != key ) {
				if (key < curr->m_key) {
					curr = curr->p_left;
				} else {
					curr = curr->p_right;
				}
			}

			return iterator( curr );
		}
		/**
		 * @brief 判断是否存在数据
		 * @param key[ I ],
		 */
		bool has( const keyType& key ){
			stNode* curr = __p_root;
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif
			// 查找节点
			while (curr && curr->m_key != key ) {
				if (key < curr->m_key) {
					curr = curr->p_left;
				} else {
					curr = curr->p_right;
				}
			}

			return ( curr != nullptr );
		}
		/**
		 * @brief 取数据内容。重载了下标运算符，方便获取数据
		 */
		dataType& operator[]( const keyType& key ){
			node_t * curr = __p_root;
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif
			// 查找节点
			while (curr && curr->m_key != key ) {
				if (key < curr->m_key) {
					curr = curr->p_left;
				} else {
					curr = curr->p_right;
				}
			}

			if( curr == nullptr ) return {};
		
			return curr->m_val;
		}
		/**
		 * @brief 插入节点
		 * @param key[ I ], 要插入的数据键
		 * @param val[ I ]， 要插入的数据的值
		 * @return 成功操作返回true,否则返回false
		 */
		bool insert(const keyType& key , const dataType& val) {
			bool ret = true;
			try{
#if THREAD_SAFE
				std::lock_guard< std::mutex > lck( __m_mutex );
#endif				
				if (!__p_root) {
					__p_root = new stNode( key , val , BLACK );
					return true;
				}

				stNode* curr = __p_root , * parent = nullptr;

				while (curr) {
					parent = curr;

					if (key < curr->m_key) {
						curr = curr->p_left;
					} else if( key > curr->m_key ){
						curr = curr->p_right;
					}else{ // 说明数据已经存在
						std::cerr << "数据已经存在" << std::endl;
						return false;
					}
				}
				
				if ( key < parent->m_key) {
					parent->p_left = new stNode( key , val , RED );
					__m_count ++;
				        __fix_insertion(parent->p_left);
				} else {
					parent->p_right = new stNode( key , val , RED );
					__m_count ++;
					__fix_insertion(parent->p_right );
				}
			}catch( std::bad_alloc& e ){
				std::cerr << e.what() << std::endl;
				ret = false;
			}

			return ret;
		}
		/**
		 * @brief 删除操作节点
		 */
		void erase(const keyType& key ) {
			stNode* curr = __p_root;
			stNode* parent = nullptr;
#if THREAD_SAFE
			std::lock_guard< std::mutex > lck( __m_mutex );
#endif
			// 查找节点
			while (curr && curr->m_key != key ) {
				parent = curr;

				if (key < curr->m_key) {
					curr = curr->p_left;
				} else {
					curr = curr->p_right;
				}
			}

			if (!curr) { return; }  // 节点不存在，不做任何操作

			if (!curr->p_left && !curr->p_right) { // 节点没有子树
				if (curr == __p_root) {
					__p_root = nullptr;
				} else if (curr == parent->p_left) {
					parent->p_left = nullptr;
					__fix_deletion(parent, nullptr);
				} else {
					parent->p_right = nullptr;
					__fix_deletion(parent, nullptr);
				}
			} else if (curr->p_left && curr->p_right) {
				stNode* succ = curr->p_right;
				parent = curr;

				while (succ->p_left) {
					parent = succ;
					succ = succ->p_left;
				}

				curr->m_key = succ->m_key;
				curr->m_val = succ->m_val;
			
				if (succ == parent->p_left) {
					parent->p_left = succ->p_right;
				} else {
					parent->p_right = succ->p_right;
				}

				__fix_deletion(parent, succ->p_right);
			} else {
				stNode* child = curr->p_left ? curr->p_left : curr->p_right;

				if (curr == __p_root ) { // 节点是根节点
					__p_root = __p_root->p_left ? __p_root->p_left : __p_root->p_right;
					__p_root->m_color = BLACK;
				} else if (curr == parent -> p_left) {
					parent->p_left = child;
					__fix_deletion(parent, child);
				} else {
					parent->p_right = child;
					__fix_deletion(parent, child);
				}
			}

			delete curr;
		}

	
	private:
		/**
		 * @brief 释放节点内存
		 * @param node[ I ] 要释放的节点对象指针
		 */
		void __release( node_t * node ){
			if( node == nullptr ) return;
			
			if( node->p_left ){
				__release( node->p_left );
			}

			if( node->p_right ){
				__release( node->p_right );
			}
			delete node;
		}
		/**
		 * @brief 插入后平衡操作
		 */
		void __fix_insertion( stNode* node) {
			if( node == nullptr ) return;
			node_t * parent = __get_parent( node );
			if( parent == nullptr ) return;
			node_t * grand = __get_parent( parent );
			
			while (parent != nullptr && parent->m_color == RED) {
				if ( parent == grand->p_left) { // 祖父的左孩子
					node_t *y = grand->p_right;
					if (y != nullptr && y->m_color == RED) {
						parent->m_color = BLACK;
						y->m_color = BLACK;
						grand->m_color = RED;
						node = grand;
					} else {
						if (node == parent->p_right) {
							node = parent;
							__rotate_left( node );
						}
						parent->m_color = BLACK;
						grand->m_color = RED;
						__rotate_right(grand);
					}
				} else {
					node_t *y = grand->p_left;
					if (y != nullptr && y->m_color == RED) {
						parent->m_color = BLACK;
						y->m_color = BLACK;
						grand->m_color = RED;
						node = grand;
					} else {
						if (node == parent->p_left) {
							node = parent;
							__rotate_right( node );
						}
						parent->m_color = BLACK;
						grand->m_color = RED;
						__rotate_left( grand );
					}
				}

				parent = __get_parent( node );
				grand = __get_parent( parent );
			}
			__p_root->m_color = BLACK;
		}

		/**
		 * @brief 删除后平衡操作
		 */
		void __fix_deletion( stNode* parent, stNode* node) {
			while (node != __p_root && !node->m_color ) {
				if (node == parent->p_left ) {
					stNode* sibling = parent->p_right;

					if (sibling-> m_color ) {
						sibling->m_color = BLACK;
						parent->m_color = RED;
						__rotate_left(parent);
						sibling = parent->p_right;
					}

					if (!sibling->p_left->m_color && !sibling->p_right->m_color) {
						sibling->m_color = RED;
						node = parent;
						parent = __get_parent(node);
					} else {
						if (!sibling -> p_right->m_color ) {
							sibling->p_left->m_color = BLACK;
							sibling->m_color = RED;
							__rotate_right(sibling);
							sibling = parent->p_right;
						}

						sibling->m_color = parent->m_color;
						parent->m_color = BLACK;
						sibling->p_right->m_color = BLACK;
						__rotate_left(parent);
						node = __p_root;
					}
				} else {
					stNode* sibling = parent->p_left;

					if (sibling->m_color) {
						sibling->m_color = BLACK;
						parent->m_color = RED;
						__rotate_right(parent);
						sibling = parent->p_left;
					}

					if (!sibling->p_left->m_color && !sibling->p_right->m_color) {
						sibling->m_color = RED;
						node = parent;
						parent = __get_parent(node);
					} else {
						if (!sibling->p_left->m_color) {
							sibling->p_right->m_color = BLACK;
							sibling->m_color = RED;
							__rotate_left(sibling);
							sibling = parent->p_left;
						}

						sibling->m_color = parent->m_color;
						parent->m_color = BLACK;
						sibling->p_left->m_color = BLACK;
						__rotate_right(parent);
						node = __p_root;
					}
				}
			}

			if (node) {
				node->m_color = BLACK;
			}
		}
		/**
		 * @brief 取双亲节点
		 */
		stNode* __get_parent( stNode* node) const {
			if (!node) {
				return nullptr;
			}

			if (node == __p_root) {
				return nullptr;
			}

			stNode* curr = __p_root;
			stNode* parent = nullptr;

			while (curr && curr != node) {
				parent = curr;

				if (node->m_key < curr->m_key ) {
					curr = curr->p_left;
				} else {
					curr = curr->p_right;
				}
			}

			return parent;
		}
		/**
		 * @brief 取叔叔节点
		 */
		stNode* __get_uncle( stNode* node) const {
			stNode* parent = __get_parent(node);
			stNode* grandparent = __get_parent(parent);

			if (!parent || !grandparent) {
				return nullptr;
			}

			if (parent == grandparent->p_left ) {
				return grandparent->p_right;
			} else {
				return grandparent->p_left;
			}
		}
		/**
		 * @brief 左旋
		 */
		void __rotate_left( stNode* node) {
			stNode* right = node->p_right;
			stNode* parent = __get_parent(node);

			if (node == __p_root ) {
				__p_root = node->p_right;
			} else if (node == parent->p_left ) {
				parent->p_left = node->p_right;
			} else {
				parent->p_right = node->p_right;
			}

			node->p_right = right->p_left;
			right->p_left = node;
		}
		/**
		 * @brief 右旋
		 */
		void __rotate_right( stNode* node) {
			stNode* left = node->p_left;
			stNode* parent = __get_parent(node);

			if (node == __p_root ) {
				__p_root = node->p_left;
			} else if (node == parent->p_left ) {
				parent->p_left = node->p_left;
			} else {
				parent->p_right = node->p_left;
			}

			node->p_left = left->p_right;
			left->p_right = node;
		}
	};
}
