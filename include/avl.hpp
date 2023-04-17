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

namespace wheels
{
	/// @brief 容器定义
	template< typename keyType , typename dataType >
	class avl
	{
	public:
		/// @brief 容器存储结构定义
		/// @tparam T 要存储的数据类型，这个数据类型必须支持通用的比较操作符重载 < , > , ==。
		///       这几个操作符将用于添加数据，删除数据和查找数据操作
		///       keyT 需要支持 < , > , ==，如果类型不支持可能会导致程序失败
		struct __stAvlItem
		{
			int          m_deep;     // 节点深度
			keyType      m_key;
			dataType     m_data;     // 数据内容
		
		
			__stAvlItem*  p_left;    // 左子树
			__stAvlItem*  p_right;   // 右子树
			__stAvlItem*  p_parent;  // 父亲节点

			__stAvlItem():m_deep(1){}
			__stAvlItem( const key_type& key , const type& data , __stAvlItem<>* parent ):
				m_deep(1),
				m_key( key ),
				m_data( data ),
				p_left( nullptr ),
				p_right( nullptr ),
				p_parent( parent ){}
		
			__stAvlItem( const type& data ):m_bf(0 ) , m_data( data ){};
		};
		// 类型定义
		using type       = dataType;
		using value_type = type;
		using key_type   = keyType;
		using node_t     = __stAvlItem;
		/// 错误代码定义
		enum emErrCode{
			ERR_FIND_DATA = -1000,
			ERR_ALLOC_MEM,
			OK = 0
		};

		/// AVL的迭代器。
		class __avlIterator : public std::iterator< std::forword_iterator_tag , node_t >{
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

			dataT& operator*() { return __p_node->m_data; }
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
			if (node->__p_right != nullptr && node->__p_left != nullptr ) {
				ret = node->__p_left->__m_deep - node->__p_right->__m_deep;
			} else if ( node->__p_right != nullptr && node->__p_left == nullptr ) {
				ret = -1;
			} else if (node->__p_left != nullptr && node->__p_right == nullptr) {
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
			node_t * root, left, left_right;
			root = node;
			left = node.__p_left;
			if (left !=  nullptr ) {
				left_right = node->__p_left->__p_right;
			} else {
				left_right = nullptr;
			}

			if ( node.__p_parent != nullptr ) {
				if ( node->__p_parent->__p_left == node ) {
					node->__p_parent->__p_left = left;
				} else {
					node->__p_parent->__p_right = left;
				}
			} else {
				__p_root = left;
			}

			left->__p_parent = node->__p_parent;
			left->__p_right = root;
			root->__p_left = left_right;

			return ret;
		}
		/**
		 * @brief 执行LR类型的旋转操作
		 * @param node[ I ] 要执行的节点
		 */
		int __lr( node_t * node ){
			int ret = 0;
			node_t * left, left_right;
			left = node->__p_left;
			left_right = left->__p_right;
			// 左旋
			node->__p_left = left_right;
			left->__p_right = left_right->__p_left;
			left->__p_parent = left_right;
			left_right->__p_left = left;
			left->__p_parent = left_right;
			left_right->__p_parent = node;
			// 右旋
			node->__p_left = left_right->__p_right;
			left_right->__p_right = node;

			if ( node->__p_parent != nullptr ) {
				if ( node->__p_parent->__p_left == node ) {
					node->__p_parent->__p_left = left_right;
				} else {
					node->__p_parent->__p_right = left_right;
				}
			} else {
				__p_root = left_right;
				left_right->__p_parent = nullptr;
			}

			node->__p_parent = left_right;

			return ret
		}
		/**
		 * @brief 执行RR旋转
		 */
		int __rr( node_t * node ){
			int ret = 0;

			node_t * root, right, right_left;
			root = node;
			right = node->__p_right;
			right_left = node->__p_right->__p_left;

			if ( node->__p_parent != nullptr ) {
				if ( node->__p_parent->__p_left == node ) {
					node->__p_parent->__p_left = right;
				} else {
					node->__p_parent->__p_right = right;
				}
			} else {
				__p_root = node->__p_right;
			}
			right->__p_parent = node->__p_parent;
			root->__p_right = right_left;
			right->__p_left = root;

			return ret;
		}
		/**
		 * @brief 执行RL旋转
		 */
		int __rl( node_t * node ){
			int ret = 0;
			node_t * right, right_left, parent;

			// 右旋
			parent = node;
			right = node->__p_right;
			right_left = right->__p_left;

			parent->__p_right = right_left;
			right->__p_left = right_left->__p_right;
			right->__p_parent = right_left;
			right_left->__p_right = right;

			// 左旋
			parent = node->__p_parent;

			node_t * tmp = right_left->__p_left;
			right_left->__p_left = node;
			node->__p_right = tmp;
			right_left->__p_parent = parent;

			if ( parent != nullptr ) { // 处理根节点，如果是根节点发生了选装需要将根节点接上
				if ( parent->__p_left == node ) {
					parent->__p_left = right_left;
				} else {
					parent->__p_right = right_left;
				}
			} else {
				__p_root = right_left;
			}
			node->__p_parent = right_left;

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
		 * @brief 插入左节点
		 * @param node，根节点
		 * @param key[ I ], 数据键值
		 * @param data[ I ], 数据类型
		 * @param 成功插入操作并且没有完成平衡返回true, 其他情况返回false
		 */
		bool __insert_to_left( node_t * node, const key_type& key , const type& data ){
			bool ret = false;
			
			try{
				if( node.m_key > key ){
					if( node->p_left != nullptr ){
						ret = __insert_to_left( node->p_left , key , data );
					}else{
						node->p_left = new node_t( key , data , node );
						ret = true;
					}
				}else if( node.m_key < key ){
					if( node->p_right != nullptr ){
						ret = __insert_to_right( node->p_right , key , data );
					}else{
						node->p_right = new node_t( key , data , node );
						ret = true;
					}
				}
			}catch( std::bad_alloc& e ){
				ERROR_MSG( e.what());
			}

			if( ret ){
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
				}
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
		bool __insert_to_right( node_t * node, const key_type& key , const type& data ){
			bool ret = false;
			if( node->m_key > key ){
				if( node->p_left != nullptr ){
					ret = __insert_to_left( node->p_left , key , data );
				}else{
					node->p_left = new node_t( key , data , node );
					ret = true;
				}
			}else if( node->m_key < key ){
				if( node->p_right != nullptr ){
					ret = __insert_to_right( node->p_right , key , data );
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
				}
			}
			return ret;
		}
		/**
		 * @brief 调整重新计算深度
		 * @param node[ I ], 开始的节点指针
		 */
		void __adjust_deep( node * node ){
			if( node == nullptr ) return;

			size_t deep = 0;

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

			deep ++;
			node->__m_deep = deep;
		}
		/**
		 * @brief 删除节点内容
		 */
		void __del_node( node_t * node ){
			auto * parent = node->__p_parent;
			if ( node.__p_left != nullptr ) {
				auto * left_right = node->__p_left->__p_right;
				auto * right = node->__p_right;
				if ( right != nil ) {
					right.__p_parent = node->__p_left;
					right.__p_left = left_right;
				}
				left_right.__p_parent = right;
			}
			if ( parent.__p_left == node ){
				if ( node.__p_left != nullptr ) {
					parent.__p_left = node.__p_left;
				} else {
					parent.__p_left = node.__p_right;
				}
			} else {
				if ( node.__p_left != nullptr ){
					parent.__p_right = node.__p_left;
				} else {
					parent.__p_right = node.__p_right;
				}
			}
		}
			
		
		/**
		 * @brief 删除右节点
		 */
		bool __delete( node_t * node, key_typekey ){
			node_t * p = node;
			int factor = 0;
			bool ret = false;
			if ( p.__m_key < key ) {
				ret = __delete( p->p_left, key);
			} else if ( p.__m_key == key ) { // 删除根节点
				ret = __delete( p->p_right, key);
			} else { // 找到节点，执行删除操作
				__del_node(p);
				ret = true;
			}
			// 计算平衡因子
			if ( node->__p_left != nullptr && node->__p_right != nullptr ) {
				factor = node->__p_left->__m_deep - node->__p_right->__m_deep;
			} else if (node->__p_left != nullptr ){
				factor = node->__p_left->__m_deep;
			} else if ( node->__p_right != nullptr ) {
				factor = node->__p_right->__m_deep;
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

			if( root->m_key > key ){
				ret = __find( key , root->p_left );
			}else if( root->m_key < key ){
				ret = __find( key , root->p_right );
			}else{
				ret = node;
			}
			return ret;
		}
		/**
		 * @brief 中序遍历并整理成列表数据内容
		 * @param node[ I ], 根节点
		 * @param rst[ O ], 结果
		 */
		void __mid_travel( node_t * node , std::list< dataT >& rst ) {
			if ( node == nullptr ) {
				return;
			}

			if ( root.__p_left != nullptr ) {
				__mid_travel(node->p_left, rst);
			}
			
			rst.push_back( node->__m_data);

			if ( node->__p_right != nullptr ) {
				__mid_travel(node->p_right, rst);
			}
		}
	public:
		avl():__p_root( nullptr ) , __m_count(0), __m_deep(0){}
		/// 暂不实现拷贝构造和移动构造
		avl( const avl& b ) = delete;
		avl( avl&& b ) = delete;
		virtual ~avl(){
			__release_mem();
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
		bool insert( const key_type& key , const type& data ){
			bool ret = false;

			if( __p_root != nullptr ){
				auto * p_node = __p_root;
				if( p_node->m_key > key ){
					__insert_to_left( p_node , key , data );
					__m_count ++;
					__adjust_deep( __p_root );
					ret = true;
				}else if( p_node->m_key < key ){
					ret = __insert_to_right( p_node , key , data );
					__m_count ++;
					__adjust_deep( __p_root );
					ret = true;
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
		bool remove( keyType key ){
			bool ret = false;

			auto * p = __p_root;

			if ( p == nullptr ) {
				return false;
			}

			if (p->__m_key == key ) { // 删除根节点
				if ( p.__p_left != nullptr ) { // 优先使用左子树作为根节点
					avl.__p_root = p.__p_left;
					auto * tmp = p.__p_left.__p_right;
					avl.__p_root.__p_parent = nullptr;
					p.__p_left.__p_right = avl.__p_root.__p_right;
					avl.__p_root.__p_right.__p_left = tmp;
				} else if ( p.__p_right != nil ) { // 这种情况下说明今有两个节点
					avl.__p_root = p.__p_right;
					avl.__p_root.__p_parent = nil;
				}
				avl.__m_count--;
				avl.__m_deep--;
			}
			if (key < p.__m_key ){ // 在左子树删除节点
				ret = __avl_delete_left(avl, p.__p_left, key);
				p.__p_left.__m_deep--;
			} else { // 在右子树删除节点
				ret = __avl_delete_right(avl, p.__p_right, key);
				p.__p_right.__m_deep--;
			}

			if ( ret ){
				__adjust_avl_deep(avl.__p_root);
				avl.__m_count--;
				avl.__m_deep = avl.__p_root.__m_deep;
			}

			return ret;
		}
		/**
		 * @brief 查找节点
		 */
		node_t * find( const key_type & key ) {
			node_t * ret = nullptr;

			ret = __find( key , __p_root );
			return ret;
		}
		/**
		 * @brief 获取节点值
		 */
		const dataT& get( const key_type& key )const{
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
		dataT& operator[]( const key_type& key ){
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
		void order( std::list< dataT >& rst , bool inc_desc = true ){
			__mid_travel( __p_root , rst );
			if( !inc ){
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
