/**
 * @brief 责任链模式
 * @author 宋炜
 * @version 1.0
 * @date 2023-4-27
 */

#pragma once

/// 使用示例：
///
/// class myItfc{
///    virtual int do_something( int ) = 0;
///    定义接口
/// };

/// class myItfc1 : public myItfc{
///      接口实现1
/// };

/// class myItfc1 : public myItfc{
///      接口实现2
/// };

/// class myRsps : public rspsLink< myItfc >{
///        自己的定义和方法
/// }

/// myRsps rsps;
/// rsps.push_back( new myItfc1 );
/// rsps.push_back( new myItfc2 );
/// rsps.push_back( ... );
///
/// 按照责任链传递操作
/// int a = 1;
/// rsps.forward([&]( myRsps::stLinkItem& item ){
///       a = item->do_something( a );
/// });
namespace wheels
{
namespace dm
{
	template<typename itfcType >
	class rspsLink
	{
	public:
		using interface_type = itfcType;
		struct stLinkItem{
			interface_type    * p_iftc;

			interface_type * operator->(){ return p_itfc; }
			stLinkItem():p_itfc( nullptr ){}
			stLinkItem( interface_type * itfc ):p_itfc( itfc ){}
			stLinkItem( stLinkItem&& b ):p_itfc( b.p_itfc ){}

			stLinkItem& operator=( stLinkItem&& b ){
				p_itfc = b.p_itfc;
				return *this;
			}
		};

		using link_t = std::list< stLinkItem >;
	protected:
		link_t             __m_link;
		link_t::iterator   __m_curr_it;
	public:
		rspsLink(){}
		virtual ~rspsLink(){}

		/**
		 * @brief 执行一次链式传递,从链头部传递到尾部
		 * @param cb 传递过程的处理方法
		 * @return 成功调用返回true，否则返回false
		 * @note 当传递到最后的时候，函数会返回false同时将迭代器位置设为链首位置。
		 *   实际在应用的时候在回调函数中调用自定义的接口进行数据传递,例如：
		 *
		 *  forward( [=](rspsLink :: stLinkItem& curr ){
		 *         curr->do_something( ... );
		 *  });
		 */
		bool forward( std::function< void ( stLinkItem& curr ) > cb ){
			bool ret = true;
			if( __m_link.size() == 0 ) return false;
			if( __m_curr_it == __m_link.end() ){
				__m_curr_it = __m_link.begin();
				return false;
			}
			if( !cb ) return false;

			cb( *__m_curr_it );
			__m_curr_it ++;
			
			return ret;
		}
		/**
		 * @brief 执行一次链式传递,从链尾部传递到头
		 * @param cb 传递过程的处理方法
		 * @return 成功调用返回true，否则返回false
		 * @note 当传递到最后的时候，函数会返回false同时将迭代器位置设为链首位置。
		 *   实际在应用的时候在回调函数中调用自定义的接口进行数据传递,例如：
		 *
		 *  backward( [=](rspsLink :: stLinkItem& curr ){
		 *         curr->do_something( ... );
		 *  });
		 */
		bool backward( std::function< void ( stLinkItem &curr ) > cb ){
				bool ret = true;
			if( __m_link.size() == 0 ) return false;
			if( __m_curr_it == __m_link.end() ){
				__m_curr_it = __m_link.end();
				__m_curr_it --;
				return false;
			}
			if( !cb ) return false;

			cb( *__m_curr_it );
			__m_curr_it --;
		
		}
		/**
		 * 在指定位置之前添加处理接口
		 */
		iterator insert( iterator it , interface_type * rsps ){
			assert( rsps );
			stLinkItem item( rsps );
			iterator ret = __m_link.insert( it , item ) ;

			return ret;
		}
		/**
		 * @brief 添加处理接口
		 * @param 处理接口对象指针
		 */
		void push_back( interface_type * rsps ){
			assert( rsps );
			stLinkItem item( rsps );
			__m_link.push_back( item );
		}
		
		/**
		 * @brief 移除尾部对象
		 */
		stLinkItem&& pop_back(){
			stLinkItem item = __m_link.back();

			__m_link.pop_back();
			
			return std::move( item );
		}

		void erase( iterator it ){
			__m_link.erase( it );
		}
		
		void clear(){
			__m_link.erase( __m_link.begin() , __m_link.end() );
		}

		bool setFromFront(){
			if( __m_link.size() == 0 ) return false;

			__m_curr_it = __m_link.begin();

			return ret;
		}

		bool setFromEnd(){
			if( __m_link.size() == 0 ) return false;

			__m_curr_it = __m_link.end();

			return ret;
		}

		iterator begin(){ return __m_link.begin(); }
		iterator end(){ return __m_link.end(); }
		
	};
}
}
