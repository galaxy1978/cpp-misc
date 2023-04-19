/**
 * @brief 平衡二叉树容器
 * @author 宋炜
 * @date 2022-12-14
 * @version 1.0
 */

#pragma once

#include <type_traits>
#include <functional>
#include <memory>
#include <vector>
#include <iterator>
#include <list>

#include "misc.hpp"

namespace wheels
{
	/// @brief 容器定义
	template< typename keyType , typename dataType >
	class avl
	{
	public:
		using value_type = dataType;
		using key_type   = keyType;

		/// @brief 容器存储结构定义
		/// @tparam T 要存储的数据类型，这个数据类型必须支持通用的比较操作符重载 < , > , ==。
		///       这几个操作符将用于添加数据，删除数据和查找数据操作
		///       keyT 需要支持 < , > , ==，如果类型不支持可能会导致程序失败
		struct __stAvlItem
		{
			int            m_deep;     // 节点深度
			key_type       m_key;
			value_type     m_data;     // 数据内容
		
		
			__stAvlItem*  p_left;    // 左子树
			__stAvlItem*  p_right;   // 右子树
			__stAvlItem*  p_parent;  // 父亲节点

			__stAvlItem():m_deep(1){}
			__stAvlItem( const key_type& key , const value_type& data , __stAvlItem* parent ):
				m_deep(1),
				m_key( key ),
				m_data( data ),
				p_left( nullptr ),
				p_right( nullptr ),
				p_parent( parent ){}
		
		};
		// 类型定义
		using node_t     = __stAvlItem;
		/// 错误代码定义
		enum emErrCode{
			ERR_FIND_DATA = -1000,
			ERR_ALLOC_MEM,
			OK = 0
		};

		/// AVL的迭代器。
		class __avlIterator : public std::iterator< std::forward_iterator_tag , node_t >{
			friend class avl;
		private:
			node_t                  * __p_node;
			std::vector< node_t * >   __m_stack;   // 迭代时候需要将数据进行压栈
		public:
			__avlIterator( node_t * node ):__p_node( node ){}
			__avlIterator( const __avlIterator& b ): __p_node( b.__p_node ),__m_stack(b.__m_stack){}
			/**
			 * @brief 采用先序遍历迭代。
			 */
			__avlIterator& operator++(){
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

			__avlIterator operator++(node_t){
				__avlIterator tmp( *this );
				operator ++();
				return tmp;
			}

			bool operator==( const __avlIterator& b ) const{ return __p_node == b.__p_node; }
			bool operator!=( const __avlIterator& b ) const{ return __p_node != b.__p_node; }

			value_type& operator*() { return __p_node->m_data; }
		};

		using iterator = __avlIterator;
	private:
		node_t  * __p_root;    // 根节点
		size_t    __m_count;   // 节点数量
		size_t    __m_deep;    // 树深度
	private:
		/**
		 * @brief 释放内存操作。由于容器采用十字链表的方式保存数据，在释放内存的时候需要逐个释放。这个
		 *   函数采用递归的方式进行释放操作
		 * @param node[ I ], 要释放内存的树的根节点。
		 */
		void __release_mem( node_t * node ){
			if( node == nullptr ) return;

			if( node->p_left != nullptr ){
				__release_mem( node->p_left );
				node->p_left = nullptr;
			}

			if( node->p_right != nullptr ){
				__release_mem( node->p_right );
				node->p_right = nullptr;
			}

			if( node->p_parent != nullptr ){
				if( node->p_parent->p_left == node ){
					node->p_parent->p_left = nullptr;
				}else{
					node->p_parent->p_right = nullptr;
				}
			}else{
				__p_root = nullptr;				
			}
			delete node;
		}

		/**
		 * @brief 计算平衡因子。平衡因子如果左子树的深度大则其值大于0，右子树深度大则其值小于0
		 * @param node[ I ], 要计算的节点指针
		 * @return 返回平衡因子的计算结果
		 */
		int __factor( node_t * node ){
			int ret = 0;
			if (node->p_right != nullptr && node->p_left != nullptr ) {
				ret = node->p_left->m_deep - node->p_right->m_deep;
			} else if ( node->p_right != nullptr && node->p_left == nullptr ) {
				ret = -1;
			} else if (node->p_left != nullptr && node->p_right == nullptr) {
				ret = 1;
			}
			return ret;
		}
		/**
		 * @brief 计算旋转类型
		 * @param node[ I ]，要计算的节点
		 * @param factor[ I ], 节点的平衡因子
		 * @return 返回旋转类型-3，RR；-2 RL； 2 LR; 3 LL
		 */
		int __rotate_type( node_t * node , int factor ){
			int ret = 0;

			if( factor == 2 ){
				int f2 = __factor( node->p_left );

				if( f2 >= 0 ){
					ret = 3;
				}else {
					ret = 2;
				}
			}else if( factor == -2 ){
				int f2 = __factor( node->p_right );
				if( f2 <= 0 ){
					ret = -3;
				}else{
					ret = -2;
				}
			}

			return ret;
		}
		/**
		 * @brief 执行LL类型旋转操作
		 * @param node[ I ]要旋转的节点
		 * @return 
		 */
		int __ll( node_t * node ){
			int ret = 0;
			node_t * root, *left, *left_right;
			root = node;
			left = node->p_left;
			if (left !=  nullptr ) {
				left_right = node->p_left->p_right;
			} else {
				left_right = nullptr;
			}

			if ( node->p_parent != nullptr ) {
				if ( node->p_parent->p_left == node ) {
					node->p_parent->p_left = left;
				} else {
					node->p_parent->p_right = left;
				}
			} else {
				__p_root = left;
			}

			left->p_parent = node->p_parent;
			left->p_right = root;
			root->p_left = left_right;

			return ret;
		}
		/**
		 * @brief 执行LR类型的旋转操作
		 * @param node[ I ] 要执行的节点
		 */
		int __lr( node_t * node ){
			int ret = 0;
			node_t * left, *left_right;
			left = node->p_left;
			left_right = left->p_right;
			// 左旋
			node->p_left = left_right;
			left->p_right = left_right->p_left;
			left->p_parent = left_right;
			left_right->p_left = left;
			left->p_parent = left_right;
			left_right->p_parent = node;
			// 右旋
			node->p_left = left_right->p_right;
			left_right->p_right = node;

			if ( node->p_parent != nullptr ) {
				if ( node->p_parent->p_left == node ) {
					node->p_parent->p_left = left_right;
				} else {
					node->p_parent->p_right = left_right;
				}
			} else {
				__p_root = left_right;
				left_right->p_parent = nullptr;
			}

			node->p_parent = left_right;

			return ret;
		}
		/**
		 * @brief 执行RR旋转
		 */
		int __rr( node_t * node ){
			int ret = 0;

			node_t * root , * right, * right_left;
			root = node;
			right = node->p_right;
			right_left = node->p_right->p_left;

			if ( node->p_parent != nullptr ) {
				if ( node->p_parent->p_left == node ) {
					node->p_parent->p_left = right;
				} else {
					node->p_parent->p_right = right;
				}
			} else {
				__p_root = node->p_right;
			}
			right->p_parent = node->p_parent;
			root->p_right = right_left;
			right->p_left = root;
			root->p_parent = right;

			return ret;
		}
		/**
		 * @brief 执行RL旋转
		 */
		int __rl( node_t * node ){
			int ret = 0;
			node_t * right, * right_left, *parent;

			// 右旋
			parent = node;
			right = node->p_right;
			right_left = right->p_left;

			parent->p_right = right_left;
			right->p_left = right_left->p_right;
			right->p_parent = right_left;
			right_left->p_right = right;

			// 左旋
			parent = node->p_parent;

			node_t * tmp = right_left->p_left;
			right_left->p_left = node;
			node->p_right = tmp;
			right_left->p_parent = parent;

			if ( parent != nullptr ) { // 处理根节点，如果是根节点发生了选装需要将根节点接上
				if ( parent->p_left == node ) {
					parent->p_left = right_left;
				} else {
					parent->p_right = right_left;
				}
			} else {
				__p_root = right_left;
			}
			node->p_parent = right_left;

			return ret;
		}
		
		/**
		 * @brief 调整平衡，使失衡的树平衡。平衡后会导致树的深度标记发生变化，需要重新
		 *   计算调整树深度
		 * @param node[ I ], 要做平衡操作的根节点
		 * @param factor[ I ]，平衡因子
		 */
		int __balance(node_t * node , int factor ){
			int ret = 0;

			int f = __rotate_type( node , factor );

			switch( f ){
			case -3:
				ret = __rr( node );
				break;
			case -2:
				ret = __rl( node );
				break;
			case 2:
				ret = __lr( node );
				break;
			case 3:
				ret = __ll( node );
				break;
			}
			return ret;
		}
		/**
		 * @brief 插入右节点
		 * @param node，根节点
		 * @param key[ I ], 数据键值
		 * @param data[ I ], 数据类型
		 * @param 成功插入操作并且没有完成平衡返回true, 其他情况返回false
		 */
		bool __insert( node_t * node, const key_type& key , const value_type& data ){
			bool ret = false;
			if( node->m_key > key ){
				if( node->p_left != nullptr ){
					ret = __insert( node->p_left , key , data );
				}else{
					node->p_left = new node_t( key , data , node );
					ret = true;
				}
			}else if( node->m_key < key ){
				if( node->p_right != nullptr ){
					ret = __insert( node->p_right , key , data );
				}else{
					node->p_right = new node_t( key , data , node );
					ret = true;
				}
			}

			if( ret == true ){
				node->m_deep ++;
				
				int factor = 0;
				if( node->p_left != nullptr && node->p_right != nullptr ){
					factor = node->p_left->m_deep - node->p_right->m_deep;
				}else if( node->p_left != nullptr ){
					factor = node->p_left->m_deep;
				}else if( node->p_right != nullptr ){
					factor = - node->p_right->m_deep;
				}

				if( factor < -1 || factor > 1 ){
					__balance( node , factor );
					ret = false;
				}
			}
			return ret;
		}
		/**
		 * @brief 调整重新计算深度
		 * @param node[ I ], 开始的节点指针
		 */
		void __adjust_deep( node_t * node ){
			if( node == nullptr ) return;

			auto deep = node->m_deep;

			if( node->p_left != nullptr ){
				__adjust_deep( node->p_left );
				deep = node->p_left->m_deep;
			}

			if( node->p_right != nullptr ){
				__adjust_deep( node->p_right );

				if( deep < node->p_right->m_deep ){
					deep = node->p_right->m_deep;
				}
			}

			if( node->p_right == nullptr && node->p_left == nullptr ){
				deep = 1;
			}else{
				deep ++;
			}
			node->m_deep = deep;
		}
		/**
		 * @brief 查找前驱节点
		 * @param node[ I ]
		 * @return 返回前驱节点的指针
		 */
		node_t * __find_pre_node( node_t * node ){
			node_t * left = node->p_left , * right = left->p_right;

			if( right != nullptr ){
				while( right->p_right != nullptr ){
					right = right->p_right;
				}				
			}else{
				right = left;
			}

			return right;			
		}
		/**
		 * @brief 删除节点内容
		 */
		node_t * __del_node( node_t * node ){
			auto * parent = node->p_parent;
			// 如果左子树不空，将左子树提升作为父节点的子节点
			if ( node->p_left == nullptr && node->p_right == nullptr ) {// 左右子树都没有,直接删除
				if( parent != nullptr ){
					if( parent->p_left == node ){
						parent->p_left = nullptr;
						if( parent->p_right == nullptr ){
							parent->m_deep --;
						}
					}else{
						parent->p_right = nullptr;
						if( parent->p_left == nullptr ){
							parent->m_deep --;
						}
					}
				}else{
					__p_root = nullptr;
					__m_deep = 0;
				}
			}else if( node->p_left == nullptr && node->p_right != nullptr ){// 存在右子树不存在左子树
				if( parent != nullptr ){
					if( parent->p_left ==node ){
						parent->p_left = node->p_right;
					}else{
						parent->p_right = node->p_right;
					}
					node->p_right->p_parent = parent;
					parent->m_deep --;
				}else{
					__p_root = node->p_right;
					
				}								
			}else if( node->p_left != nullptr && node->p_right == nullptr ){// 左右子树都存在
				if( parent != nullptr ){
					if( parent->p_left == node ){
						parent->p_left = node->p_left;
					}else{
						parent->p_right = node->p_left;
					}
					node->p_left->p_parent = parent;
					parent->m_deep --;
				}else{
					__p_root = node->p_left;
				}
			}else{
				node_t * pre_node = __find_pre_node( node );
				node->m_data = pre_node->m_data;
				node->m_key = pre_node->m_key;
				parent = pre_node->p_parent;

				if( parent->p_left == node ){
					parent->p_left = nullptr;
					if( parent->p_right == nullptr ){
						parent->m_deep --;
					}
				}else{
					parent->p_right = nullptr;
					if( parent->p_left == nullptr ){
						parent->m_deep --;
					}
				}
				delete pre_node;
				__m_count --;
				return parent;
			}
			delete node;
			__m_count --;

			return parent;
		}
			
		
		/**
		 * @brief 删除节点
		 * @param node[ I ]
		 * @param key[ I ]
		 * @return 删除后没有执行数据平衡操作返回true，完成平衡操作后返回false
		 */
		bool __delete( node_t * node, const key_type& key ){
			node_t * p = node;
			int factor = 0;
			bool ret = false;
			if ( p->m_key > key ) {
				ret = __delete( p->p_left, key);
			} else if ( p->m_key < key ) { // 删除根节点
				ret = __delete( p->p_right, key);
			} else { // 找到节点，执行删除操作
				node = __del_node(p);
				ret = true;
				return ret;
			}
			// 计算平衡因子
			if( node == nullptr ){
				ret = false;
			}else if ( node->p_left != nullptr && node->p_right != nullptr ) {
				factor = node->p_left->m_deep - node->p_right->m_deep;
			} else if (node->p_left != nullptr ){
				factor = node->p_left->m_deep;
			} else if ( node->p_right != nullptr ) {
				factor = -node->p_right->m_deep;
			}
			
			if ( ret && (factor < -1 || factor > 1) ){
				__balance( node, factor);
				ret = false;
			}

			return ret;
		}
		/**
		 * @brief 实际执行查找操作
		 */
		node_t * __find( const key_type& key , node_t * root ){
			node_t * ret = nullptr;

			if( root == nullptr ){
				return nullptr;
			}

			if( root->m_key == key ){
				return root;
			}else if( root->m_key > key ){
				ret = __find( key , root->p_left );
			}
			
			return  __find( key , root->p_right );			
		}
		/**
		 * @brief 中序遍历并整理成列表数据内容
		 * @param node[ I ], 根节点
		 * @param rst[ O ], 结果
		 */
		void __mid_travel( node_t * node , std::list< value_type >& rst ) {
			if ( node == nullptr ) {
				return;
			}

			if ( node->p_left != nullptr ) {
				__mid_travel(node->p_left, rst);
			}
			
			rst.push_back( node->m_data);

			if ( node->p_right != nullptr ) {
				__mid_travel(node->p_right, rst);
			}
		}
	public:
		avl():__p_root( nullptr ) , __m_count(0), __m_deep(0){}
		/// 暂不实现拷贝构造和移动构造
		avl( const avl& b ) = delete;
		avl( avl&& b ) = delete;
		virtual ~avl(){
			__release_mem( __p_root );
		}
		/**
		 * @brief 获取当前节点数量
		 * @return 返回节点
		 */
		size_t count(){ return __m_count; }
		/**
		 * @brief 获取深度
		 */
		size_t deep(){ return __m_deep; }
		/**
		 * @brief 插入节点.
		 * @param key[ I ]，节点索引
		 * @param data[ I ]， 节点数据内容
		 * @return 成功操作返回true，否则返回false
		 */
		bool insert( const key_type& key , const value_type& data ){
			bool ret = false;

			if( __p_root != nullptr ){
				auto * p_node = __p_root;
				if( key != p_node->m_key ){
					ret = __insert( p_node , key , data );
					__m_count ++;
					if( ret == false ){
						__adjust_deep( __p_root );
						__m_deep = __p_root->m_deep;
					}
					ret = true;
				}else{
					ERROR_MSG( "数据已经存在" );
					return ret;
				}
			}else{
				try{
					__p_root = new node_t( key , data , nullptr );
					__m_deep = 1;
					__m_count = 1;
					ret = true;
				}catch( std::bad_alloc& e ){
					ERROR_MSG( e.what() );
				}
			}
			return ret;
		}
		/**
		 * @brief 删除节点
		 */
		bool remove( const key_type& key ){
			bool ret = true;

			auto * p = __p_root;

			if ( p == nullptr ) {
				return false;
			}

			__delete( p , key );
			
			return ret;
		}
		
		inline bool erase( const key_type & key ){
			if( __p_root ){
				__delete( __p_root , key );
				return true;
			}

			return false;
		}


		node_t * root(){ return __p_root; }
		/**
		 * @brief 查找节点
		 */
		iterator find( const key_type & key ) {
			node_t * ret = nullptr;

			ret = __find( key , __p_root );
			
			return {ret};
		}
		/**
		 * @brief 获取节点值
		 */
		const value_type& get( const key_type& key )const{
			node_t * ret = nullptr;

			ret = __find( key , __p_root );
			if( ret == nullptr ){
				throw ERR_FIND_DATA;
			}
			return *ret;
		}
		/**
		 * @brief 获取节点值。对于[]进行的重载，这个方式可以修改节点数据内容
		 * @param key[ I ],键名
		 * @return 返回节点的可读写引用
		 */
		value_type& operator[]( const key_type& key ){
			node_t * ret = nullptr;

			ret = __find( key , __p_root );

			if( ret == nullptr ){
				throw ERR_FIND_DATA;
			}

			return ret->m_data;
		}
		/**
		 * @brief 对数据进行排序
		 * @param rst[ O ], 排序的结果
		 * @param inc_desc[ I ], 排序方式，true升序，false降序
		 */
		void order( std::list< value_type >& rst , bool inc_desc = true ){
			__mid_travel( __p_root , rst );
			if( !inc_desc ){
				rst.reverse();
			}
		}
		/**
		 * @brief 
		 */
		iterator begin(){ return iterator(__p_root );}
		iterator end(){ return iterator( nullptr ); }
	};
}
