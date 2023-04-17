/**
 * @brief 在TRP-CRM中大量的数据处理需要使用树形结构表，如BOM表，部门结构表，项目任务计划表等。
 * @version 1.0
 * @date 2011-2-10 ~ 2019-9-9
 * @author 宋炜
 *
 */

#pragma once

#include <memory>
#include <functional>
#include <iostream>
#include <assert.h>
#include <list>
#include <type_traits>

#include "typetraits.hpp"

template< typename type,
	  typename metaType = typename std::decay< type > :: type,
	  typename T = typename std::conditional< std::is_pointer< metaType >::value,
					 typename std::remove_pointer< metaType >::type,
					 metaType
					 >::type
	  >
class tree
{
public:
	/// @{
	enum emErrCode{
		OK,
		ERR_DATA_EMPTY,          // 当前节点数据是空指针
		ERR_ALLOC_MEM ,          // 内存分配失败
		ERR_TREE_EMPTY,          // 空树
		ERR_OUT_CHILDREN_RANGE,  // 操作超出子节点数量总数
		ERR_CAN_NOT_FIND,        // 找不到节点
		ERR_NULL_DATA_PT,        // 传入的数据指针是空指针
		ERR_ROOT_ASGN            // 对跟节点进行赋值
	};
	/// @}
	typedef typename type_traits< T >::value_type   value_type;
	typedef typename type_traits< T >::pointer      pointer;
	typedef typename type_traits< T >::refrence     reference;
	/**
	 * 定义树节点
	 */
	class treeNode{
		friend class tree;
	private:
		bool         m_root;           // 是否是根节点
		pointer      m_data;           // 具体的数据
		treeNode   * p_parent;         // 父亲节点
		treeNode   * p_children;       // 孩子节点
		treeNode   * p_sibling;        // 弟弟节点
		treeNode   * p_pre;            // 哥哥节点
	public:
		treeNode()
		:m_root( false ),
		m_data( nullptr ),
		p_parent( nullptr ),
		p_children( nullptr ),
		p_sibling( nullptr ),
		p_pre( nullptr )
		{}
		/**
		 * @brief 构造函数
		 * @param parent [ I ] 父亲节点指针
		 * @param value  [ I ] 数据
		 * @param is_root[ I ] 是否是根节点
		 * @exceptions 内存分配失败后抛出std::bad_alloc
		 */
		treeNode( bool /*is_root*/ , bool allocData = false )
			: m_root( false ),
			m_data( nullptr ),
			p_parent( nullptr ),
			p_children( nullptr ),
			p_sibling( nullptr ),
			p_pre( nullptr ){

			if( allocData == true ){
				m_data = new value_type();
			}
		}
		treeNode( const treeNode& b )
			: p_parent( nullptr ) , p_children( nullptr ) , p_sibling( nullptr ) , p_pre( nullptr )
		{
			m_root = b.m_root;
			m_data = new value_type();
			* m_data = *( b.m_data );
		}
		treeNode( reference data )
			: m_root( false ) , m_data( nullptr ) , p_parent( nullptr ) , p_children( nullptr ) , p_sibling( nullptr ) , p_pre( nullptr )
		{
			m_data = new value_type();
			* m_data = data;
		}

		treeNode( pointer data )
			: m_root( false ) , m_data( nullptr ) , p_parent( nullptr ) , p_children( nullptr ) , p_sibling( nullptr ) , p_pre( nullptr )
		{
			if( data == nullptr ) throw ERR_DATA_EMPTY;
			m_data = new value_type();
			*m_data = *data;
		}
		treeNode( value_type&& data )
			: m_root( false ) , m_data( nullptr ) , p_parent( nullptr ) , p_children( nullptr ) , p_sibling( nullptr ) , p_pre( nullptr )
	        {
			m_data = new value_type();

			*m_data = data;
		}

		treeNode( treeNode* parent , const reference data )
			: m_root( false ) , m_data( nullptr ) , p_parent( parent ) , p_children( nullptr ) , p_sibling( nullptr ) , p_pre( nullptr ){
			if( p_parent->p_children == nullptr ){
				p_parent->p_children = this;
			}else{
				treeNode * sib = p_parent->p_children;
				while( sib->p_sibling != nullptr ){
					sib = sib->p_sibling;
				}

				sib->p_sibling = this;
				p_pre = sib;
			}

			m_data = new value_type();
			*m_data = data;
		}
		/**
		 * @brief 克隆一个节点。这个节点的值和当前节点一致。
		 * @return 成功操作返回新节点的指针
		 * @exceptions 如果内存分配失败，则抛出ERR_ALLOC_MEM
		 * @note 这个操作只会复制节点值，不会复制节点全部关系。如果要完全克隆调用deepClone
		 */
		treeNode * clone( ){
			treeNode * ret = nullptr ;
			try{
				ret = new treeNode( m_data );
			}catch( std::bad_alloc& e ){
				throw ERR_ALLOC_MEM;
				std::cerr << __FILE__ << "  " << __LINE__ << "Allocate memory fail." <<std::endl;
			}
			return ret;
		}

		treeNode * deepClone(){
			treeNode * ret = nullptr ;
			try{
				ret = new treeNode( p_parent , *m_data );
				ret->p_children = p_children;
				ret->p_sibling = p_sibling;
				ret->p_pre = p_pre;
			}catch( std::bad_alloc& e ){
				throw ERR_ALLOC_MEM;
				std::cerr << __FILE__ << "  " << __LINE__ << "Allocate memory fail." <<std::endl;
			}

			return ret;
		}
		/**
		 * @brief 节点数据拷贝
		 * @param b[ I ] 要拷贝的数据节点
		 * @return 成功操作返回节点引用
		 * @exceptions 如果节点数值不存在，则抛出ERR_DATA_EMPTY
		 */
		treeNode& operator=( const treeNode& b ){
			if( m_data == nullptr ) throw ERR_DATA_EMPTY;
			* m_data = *b.m_data;
			p_parent = nullptr;
			p_children = nullptr;
			p_sibling = nullptr;
			p_pre = nullptr;
			return *this;
		}
		/**
		 * @brief 比对节点是否相等
		 * @param b[ I ] 要比对的数值
		 * @return 如果相等返回true, 否则返回false
		 * @exceptions 如果节点数值不存在，则抛出ERR_DATA_EMPTY
		 */
		bool operator==( const value_type& b ){
			bool ret = false;
			if( m_data == nullptr ) throw ERR_DATA_EMPTY;
			ret = ( *m_data == b );
			return ret;
		}

		bool operator==( const treeNode& b ){
			bool ret = false;
			if( m_data == nullptr ) throw ERR_DATA_EMPTY;
			if( b.m_data == nullptr ) throw ERR_DATA_EMPTY;
			ret = ( *m_data == *b.m_data );
			return ret;
		}
		/**
		 * @brief 去父亲节点
		 * @return 返回父亲节点指针，如果是根节点，则返回nullptr
		 */
		treeNode * parent(){ return p_parent; }
		/**
		 * @brief 指定父亲节点
		 * @param p[ I ] , 父亲节点指针
		 */
		void parent( treeNode* p ){ p_parent = p; }
		/**
		 * @brief 添加孩子节点。这个操作在孩子节点的最后追加内容
		 * @param node[ I ] , 要添加的孩子节点。
		 */
		void push_child( const treeNode & n ){
			treeNode * node = p_children , * new_node = nullptr;
			try{
				new_node = new treeNode( n );
				new_node->p_parent = this;
			}catch( std::bad_alloc& e ){
				std::cerr << "Allocate memory fail" << std::endl;
				return;
			}
			if( node == nullptr ){  // 如果节点没有孩子节点，则直接添加内容
				p_children = new_node;
				return;
			}

			while( node -> p_sibling != nullptr ){ node = node->p_sibling; }

			if( node ){
				node->p_sibling = new_node;
				new_node -> p_pre = node;
			}
		}

		void push_child( treeNode* node ){
			assert( node != nullptr );

			node->p_parent = this;
			if( p_children == nullptr ){
				p_children = node;
			}else{
				treeNode * tmp = p_children;
				while( tmp -> p_sibling != nullptr ){
					tmp = tmp->p_sibling;
				}

				tmp->p_sibling = node;
				node->p_pre = tmp;
			}
		}
		/**
		 * @brief 取弟弟节点指针
		 * @return 返回节点指针
		 */
		treeNode* sibling(){ return p_sibling; }
		/**
		 * @brief 指定弟弟节点指针
		 */
		void sibling( treeNode * node ){
			p_sibling = node;
			if( node ){
				node->p_pre = this;
				node->p_parent = p_parent;
			}
		}

		treeNode * children(){ return p_children; }
		void children( treeNode * node ){
			if( p_children != nullptr ){
				node->p_sibling = p_children;
			}
			p_children = node;
		}

		treeNode * pre( ){ return p_pre; }
		void pre( treeNode * node ) {
			p_pre = node;
			if( node != nullptr ){
				node->p_sibling = this;
				node->p_parent = p_parent;
			}
		}

		/**
		 * @brief
		 */
		const value_type& value() const{
			return *m_data;
		}
		/**
		 * @brief
		 */
		const pointer value_ptr()const{ return m_data; }

		treeNode * get()const{
			return const_cast< treeNode * >( this );
		}
	};//treeNode 定义结束
protected:
	treeNode     m_root;          // 根节点
	treeNode   * p_cur;           // 当前操作点指针
public:
	typedef treeNode     Node;
	typedef treeNode *   NodePtr;
public:
	/**
	 * @brief 对象构造
	 */
	tree():m_root( true ){
		p_cur = &m_root;
	}
	tree( bool allcRoot ): m_root( true , allcRoot )
	{
		p_cur = &m_root;
	}
	virtual ~tree(){
		clear();
	}

	treeNode& getRoot(){
		return m_root;
	}

	treeNode * getRootPtr(){
		return &m_root;
	}

	/**
	 * @brief 判断对象是否是空对象
	 * @return 如果是空对象则返回true，否则返回false
	 */
	bool empty() const{
		return ( m_root.p_children == nullptr );
	}

	bool have_children( const treeNode * p ) const{
		assert( p );
		return ( p->p_children != nullptr );
	}

	bool have_children( const treeNode& p ) const{
		return ( p.children() != nullptr );
	}

	bool have_sibling( const treeNode * p ) const{
		assert( p );
		return ( p->p_sibling != nullptr );
	}

	bool have_sibling( const treeNode& p ) const {
		return p.p_sibling != nullptr;
	}

	/**
	 * @brief 遍历树
	 * @param 访问节点的时候的回调函数
	 */
	emErrCode for_each( std::function< bool ( treeNode & ) > fun ){
		emErrCode ret = for_each( &m_root , [ this , fun ]( treeNode * n , bool , bool bkp )->bool{
		       if( n == nullptr ) return true;
		       if( n == &m_root ) return true;
		       if( bkp ) return true;
		       
		       bool ret1 = fun( *n );

		       return ret1;
		});

		return ret;

	}
	/**
	 * @brief 树的先根深度优先遍历
	 */
	emErrCode for_each( std::function< bool ( treeNode * )> fun ){
		emErrCode ret = for_each( &m_root , [ this , fun ]( treeNode * n , bool , bool )->bool{
		       if( n == nullptr ) return true;
		       bool ret1 = fun( n );

		       return ret1;
		});
		return ret;
	}
	/**
	 * @brief 后根遍历，采用后根遍历非常方便销毁对象
	 */
	emErrCode for_each_2( std::function< bool ( treeNode *) > fun ){
		emErrCode ret = OK;
		std::list< treeNode *>   sl;

		treeNode * p = &m_root , * old_p = nullptr;
		bool c = true , is_pop = false;

		if( have_children( p ) ){
			sl.push_back( p );
			p = p->children();
		}else{
			return ERR_TREE_EMPTY;
		}

		while( sl.size() > 0 && c == true ){
			if( have_children( p ) && p->children() != old_p && is_pop == false ){// 深入到孩子节点
				sl.push_back( p );
				p = p->children();
				is_pop = false;
				continue;
			}

			if( old_p != nullptr && have_sibling( p ) && p->sibling() != old_p && p->sibling() != old_p ->pre() ){// 深入到兄弟节点
				sl.push_back( p );
				is_pop = false;
				p = p->sibling();
				continue;
			}

			c = fun( p );
			old_p = p;
			p = sl.back();
			sl.pop_back();
			is_pop = true;
		}
		return ret;
	}
	/**
	 * @brief 后根遍历
	 * @param root[ I ] ， 遍历起始节点
	 * @param fun[ I ] ， 回调函数。回调函数的格式如下：
	 *     bool fun( treeNode * node , bool etc , bool bkp );
	 *     @param node[ I ] , 访问到的节点
	 *     @param etc[ I ] , 是否是访问子节点
	 *     @param bkp[ I ] , 是否是回退到父节点
	 *     @return 返回true继续执行遍历操作，否则结束便利操作
	 */
	emErrCode for_each( treeNode * root , std::function< bool ( treeNode * node , bool etc , bool bkp ) > fun ){
		emErrCode ret = OK;
		std::list< treeNode* >  sl ;
		treeNode * p = root , *old_p = nullptr;
		bool is_pop = false , c = false , et_child = true;

		if( root == nullptr ){ return ERR_TREE_EMPTY ; }

		c = fun( p , false , false );

		if( have_children( p ) ){
			sl.push_back( p );
			p = p->children();
		}
		old_p = p;
		while( sl.size() > 0 && c == true ){
			if( is_pop == false ){ // 访问节点
				c = fun( p , et_child , false );
			}else{ // 通知节点弹出
				c = fun( nullptr , false , old_p->parent() == p );
			}

			if( have_children( p ) && p->children() != old_p && is_pop == false ){ // 访问子节点
				sl.push_back( p );
				old_p = p;
				p = p->children();
				is_pop = false;
				et_child = true;
				continue;
			}
			if( have_sibling( p ) && p->sibling() != old_p ){ // 访问兄弟节点
				sl.push_back( p );
				old_p = p;
				p = p->sibling();
				is_pop = false;
				et_child = false;
				continue;
			}

			old_p = p;
			p = sl.back();
			sl.pop_back();
			is_pop = true;
		}
		return ret;
	}
     /**
      * @brief 后根遍历, 将节点以指针的方式传递给回调函数，并且以指定节点开始遍历
      * @param root
      * @param fun
      * @return 成功操作返回OK， 否则返回错误编码
      */
     emErrCode for_each_2( treeNode * root , std::function< bool ( treeNode *) > fun )
     {
         emErrCode ret = OK;
         std::list< treeNode * > s1;

         treeNode* p =  root , * old_p = nullptr ;
         bool c = true , is_pop = false;

         if( have_children( p ) ){// 在栈中压入根节点
             s1.push_back( p );
             p = p->p_children;
         }

         while( s1.size() > 0 && c == true ){

             if( p->p_children != old_p && have_children( p ) && is_pop == false ){ // 访问子节点
                 s1.push_back(  p );
                 p = p->p_children;
                 is_pop = false;
                 continue;
             }

             if( p->p_sibling != old_p && p->p_sibling != old_p->p_pre && have_sibling( p ) ){ // 访问兄弟节点
                 s1.push_back( p );
                 is_pop = false;
                 p = p->p_sibling;
                 continue;
             }
             c = fun( p );    // 访问节点
	     //old_p = p;
             p = s1.back();
             s1.pop_back();
             is_pop = true;
         }

         return ret;
     }
	/**
	 * @brief 查找给定值的节点。这个函数要求内容对象必须支持==
	 * @param value[ I ] , 要查找的目标值
	 * @return 成功返回节点指针，否则返回nullptr
	 */
	treeNode * find( const reference value ){
		emErrCode e = OK;
		treeNode * ret = nullptr , n( value );

		e = for_each( [ this , &ret , &n ]( treeNode * node )->bool{
			bool ret1 = true;
			if( *node == n ){
				ret1 = false;
				ret = node;
			}

			return ret1;
		});
		if( e != OK ){
			ret = nullptr;
		}
		return ret;
	}

	/**
	 * @brief 查找给定的值。这个函数采用比对函数的方式进行比对，方便了任何方式的比对操作。即使对象并不支持==操作符号
	 *   也可以完成比对操作
	 * @param value[ I ], 要查找的值
	 * @param fun[ I ] , 比对函数
	 *    bool fun( const treeNode& a , const treeNode& b );
	 *    如果是要查找的值则返回true ， 否则返回
	 */
	treeNode * find_if( const reference value , std::function< bool ( const treeNode& a , const treeNode& b )> fun ){
		treeNode * ret = nullptr;
		if( !fun ) return nullptr;
		treeNode n( value );
		emErrCode e = for_each( [ this , &n , &ret , fun ]( const treeNode* node )->bool{
			bool ret1 = true;
			if( fun( n , *node ) == true ){
				ret1 = false;
				ret = node;
			}
			return ret1;
		});

		if( e != OK ){
			ret = nullptr;
			throw e;
		}
		return ret;
	}

	treeNode * find_if( const treeNode& value , std::function< bool ( const treeNode& a , const treeNode& b ) > fun ){
		treeNode * ret = nullptr;
		if( !fun ) {
			std::cerr << __FILE__ << "\t[ " << __LINE__ << "] compare function is not avaliable." << std::endl;
			return nullptr;
		}

		emErrCode e = for_each( [ this , &ret, &value , fun ](treeNode * node )->bool{
		       bool ret1 = true;
		       if( fun( value , *node ) == true ){
			       ret1 = false;
			       ret = node;
		       }

		       return ret;
		});

		if( e != OK ){
			std::cerr << "" << std::endl;
			throw e;
		}
		return ret;
	}
	/**
	 * @brief current
	 * @return
	 */
	inline treeNode * current(){ return p_cur; }
	/**
	 * @brief 在给定的父节点下添加一个孩子节点。这个函数只会在父节点下的所有孩子节点最后添加
	 * @param parent[ I ] , 父节点
	 * @param value[ I ] , 要添加的值
	 * @return 成功返回OK，否则返回错误代码
	 */
	emErrCode insert( treeNode * parent , const reference value ){
		emErrCode ret = OK;
		assert( parent );
		treeNode * n = nullptr;
		try{
			n = new treeNode( value );
			p_cur = n;
		}catch( std::bad_alloc& ){
			ret = ERR_ALLOC_MEM;
			return ret;
		}

		parent->push_child( n );

		return ret;
	}
	/**
	 * @note 为了防止采用引用符号的获取的指针的应用操作，这个函数对传入的指针会重新进行内存的分配
	 *   所以外部如果使用了指针，必须自己进行内存的释放
	 */
	emErrCode insert( treeNode * parent , treeNode * node ){
		emErrCode ret = OK;
		assert( node != nullptr );

		treeNode * n = nullptr;
		try{
			n = new treeNode( node );
			parent->push_child( n );
			p_cur = n;
		}catch( std::bad_alloc& e ){
			ret = ERR_ALLOC_MEM;
			std::cerr << __FILE__ << "\t" << __LINE__ << "Allocate memory fail." << std::endl;
		}

		return ret;
	}
	/**
	 * @brief
	 * @param parent[ I ]
	 * @param pre[ I ]
	 * @param value[ I ]
	 * @return
	 */
	emErrCode insert( treeNode * parent , treeNode * pre , const reference value ){
		emErrCode ret = OK;
		assert( parent );
		assert( pre );

		treeNode * n = nullptr;
		try{
			n = new treeNode( value );

			if( pre == nullptr ){
				parent->push_child( n );
				n->parent( parent );
			}else{
				n->parent( parent );
				n->pre( pre );
				pre->sibling( n );
			}

			p_cur = n;
		}catch( std::bad_alloc& e ){
			ret = ERR_ALLOC_MEM;
			std::cerr << __FILE__ << "\t" << __LINE__ << "Allocate memory fail." << std::endl;
		}

		return ret;
	}
	/**
	 */
	emErrCode insert( treeNode * parent , treeNode * pre , treeNode * node ){
		emErrCode ret = OK;
		assert( parent );
		assert( pre );
		assert( node );
		try{
			treeNode * n = new treeNode( node );
			if( pre == nullptr ){
				parent->push_child( n );
				n->parent( parent );
			}else{
				n->parent( parent );
				n->pre( pre );
				pre->sibling( n );
			}

			p_cur = n;
		}catch( std::bad_alloc& e ){
			ret = ERR_ALLOC_MEM;
			std::cerr << __FILE__ << "\t" << __LINE__ << "Allocate memory fail." << std::endl;
		}

		return ret;
	}
	/**
	 * @brief 删除节点
	 */
	emErrCode remove( treeNode * node ){
		emErrCode ret = OK;
		bool set_pcur = false;
		assert( node );
		if( node -> sibling() ){
			( (treeNode *)node->sibling())->pre( node->pre() );
			p_cur = node->sibling();
			set_pcur = true;
		}
		if( node->pre() ){
			node->pre()->sibling( node->sibling() );
			if( set_pcur == false ){
				p_cur = node->pre();
				set_pcur = true;
			}
		}

		if( node->parent() && node->parent()->children() == node ){
			node->parent( node->sibling() );
			if( set_pcur == false ){
				p_cur = node->parent();
			}
		}

		delete node;

		return ret;
	}
	/**
	 * @brief 条件删除操作
	 * @param fun
	 * @return
	 */
	emErrCode remove_if( std::function< bool ( treeNode& node )> fun ){
		emErrCode ret = OK;
		bool has = false;

		ret = for_each( [ this , fun ,&has]( treeNode* n )->bool{
			bool rst = fun( *n );
			if( rst == true ){
				remove( n );
				has = true;
			}

			return true;
	        });

		if( has == false ){
			ret = ERR_CAN_NOT_FIND;
		}
		return ret;
	}
	/**
	 *@brief 释放内容
	 */
	emErrCode clear(){
		emErrCode e = for_each_2( [ this ]( treeNode * n )->bool{
			 bool ret1 = true;
			 if( n ){
				 remove( n );
			 }

			 return ret1;
		});
		m_root.p_children = nullptr;
		m_root.m_data = nullptr;
		m_root.p_sibling = nullptr;
		m_root.p_pre = nullptr;
		m_root.p_parent = nullptr;
		return e;
	}
};

