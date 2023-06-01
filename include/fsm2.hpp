/**
 * @brief 状态机2.0
 * @version 2.0
 * @author 宋炜
 * @date
 */

#pragma once
#include <assert.h>

#include <string>
#include <map>
#include <atomic>
#include <functional>

#include "misc.hpp"

namespace wheels
{
	namespace fsm2{
		/// 方便继承或者特化后能够使用一个指针，定义一个基础空类
		struct __base{};
		// 当状态进入的时候调用的函数返回为AUTO_RUN，则自动转换状态否则需要外部条件触发。
		// 自动触发的状态
		static const bool AUTO_RUN = true;
		// 条件触发的状态
		static const bool TRIGER_RUN = false;

		/// 状态机定义， 这个状态机不支持子状态模式
		template< typename statType , typename dataType , typename realT = typename std::decay< dataType >::type >
		class fsm2 : public __base
		{
		public:

			enum emType{ START , END , NORMAL };

			enum emErrCode{
				ERR_ALLOC_MEM,
				ERR_STAT_ID_EMPTY,         // 节点ID不能为空字符串
				ERR_STAT_EXIST,            // 节点名称已经存在
				ERR_FSM_NOT_START,         // 状态机没有启动
				ERR_STAT_NOT_DEFINED,      // 状态节点没有定义
				ERR_STAT_FINISHED,         // 状态机已经结束运行
				ERR_STAT_OBJECT_NULL,	  // 状态描述对象空
				ERR_CAN_NOT_FIND_STAT,     // 找不到状态节点记录
				OK
			};
			struct stDataItem{
				std::vector< realT >    m_raw_data;

				/**
				 * @brief 读取指定位置的数据，根据具体类型转换数据类型。
				 * @tparam DT
				 * @param idx[ I ],
				 * @param d[ I ]
				 */
				template< typename DT >
				bool get(   DT& d ) const{
					if( m_raw_data.size() > 0 ){
						d = m_raw_data.back();
						return true;
					}

					return false;
				}

				template< typename DT >
				bool get( long idx ,   DT& d ) const{
					const realT * dp = m_raw_data.data();
					if( m_raw_data.size() > 0 ){
						d = *( dp + idx );
						return true;
					}

					return false;
				}
				/**
				 * @brief 获取原始数据内容指针
				 */
				const realT * getRaw()const{ return m_raw_data.data(); }
				/**
				 * @brief 获取原始数据内容记录数量。
				 * @note 这个是读取记录数量不是内存实际占用
				 */
				size_t size()const{ return m_raw_data.size(); }
			};
			/// @brief 进入状态处理函数。函数原型为：bool cbFun( const std::string& id , const stDataItem& data );
			/// @param const statType& [ I ], 当前状态ID
			/// @param  const stDataItem& [ I ], 当前已经处理的数据内容
			/// @return 如果函数执行后需要自动转换状态返回true，否则返回false
			using enterFun_t = std::function< bool ( const statType& , const stDataItem& ) > ;

			/// @brief 离开状态转换函数。函数原型为 std::string cbFun( const std::string& id , const stDataItem& data );
			/// @param id[ I ], 当前状态ID
			/// @param data[ I ]
			using leaveFun_t = std::function< statType ( const statType& , const stDataItem& ) > ;
			/// @brief 发生转换的时候调用这个函数
			using raiseFun_t = std::function< void ( const statType& , const statType& , const stDataItem& ) >;

			/**
			 * @brief 定义状态
			 */
			struct state: public __base
			{
				emType         m_type;
				statType       m_id;

				enterFun_t     cb_enter;
				leaveFun_t     cb_leave;

				state( const statType& id , emType type = NORMAL ): m_type( type ), m_id( id ){}
				~state(){}

				void type( emType type ){ m_type = type;}
				emType type(){ return m_type; }

				state& operator=( const state& b ){
					m_type = b.m_type;
					m_id = b.m_id;
					cb_enter = b.cb_enter;
					cb_leave = b.cb_leave;

					return *this;
				}

				state& operator=( state&& b ){
					m_type = b.m_type;
					m_id = std::move( b.m_id );
					cb_enter = b.cb_enter;
					cb_leave = b.cb_leave;

					return *this;
				}

			};

			struct trans : public __base
			{
				statType   m_from;
				statType   m_to;

				raiseFun_t  cb_raise;

				trans( const statType& f , const statType& to ): m_from( f ), m_to( to ){}
				trans( const state& b ): m_from( b.m_from ), m_to( b.m_to ){}
				trans( state&& b ): m_from( std::move( b.m_from )), m_to( std::move( b.m_to )) {}

				trans& operator=( const trans& b ){
					m_from = b.m_from;
					m_to = b.m_to;
					cb_raise = b.cb_raise;
					return *this;
				}

				trans& operator=( trans&& b ){
					m_from = std::move( b.m_from );
					m_to = std::move( b.m_to );
					cb_raise = b.cb_raise;

					return *this;
				}
				~trans(){}

			};

			using transTbl_t = std::map< statType , trans* > ;
			using statTbl_t     = std::map< statType , state * >;

		private:
			statType     	        __m_start;         // 起始状态
			statType     	        __m_end;           // 结束状态
			statType                __m_curr;          // 当前状态
			statType                __m_next;
			std::atomic<bool>       __m_is_running;    //
			stDataItem      	__m_data;          // 缓冲数据

			statTbl_t       	__m_stat;          // 状态表
			transTbl_t      	__m_trans;         // 状态表数据

		public:
			fsm2() {}
			/**
			 * @brief 创建状态机对象。同时初始化相关回调函数
			 * @param start[ I ], 起始状态名称
			 * @param end[ I ], 结束状态
			 * @param entfun[ I ], 进入起始状态触发函数
			 * @param leavefun[ I ], 初始状态离开回调函数
			 * @exception 起始状态为空字符串的时候抛出 ERR_STAT_ID_EMPTY
			 */
			fsm2( const statType& start , enterFun_t entfun ):
				__m_start( start ) , __m_curr( start ), __m_is_running( false )
				{
					if( start.empty() ) throw ERR_STAT_ID_EMPTY;

					state * s = new state( start , START );
					s->cb_enter = entfun;
					__m_stat.insert( std::make_pair( start , s ) );
				}

			fsm2( const statType& start , enterFun_t entfun , leaveFun_t leavefun ):
				__m_start( start ) , __m_curr( start ), __m_is_running( false )	{
				if( start.empty() ) throw ERR_STAT_ID_EMPTY;

				state * s = new state( start , START );
				s->cb_enter = entfun;
				s->cb_leave = leavefun;

				__m_stat.insert( std::make_pair( start , s ) );
			}

			fsm2( const statType& start , const statType& end )
				: __m_start( start ), __m_end( end ), __m_curr( start ), __m_is_running( false ){
				if( start.empty() ) throw ERR_STAT_ID_EMPTY;
				if( end.empty() ) throw ERR_STAT_ID_EMPTY;

				state *s = new state( start , START ) ,  * e = new state( end , END );

				__m_stat.insert( std::make_pair( start , s ) );
				__m_stat.insert( std::make_pair( end , e ) );
			}

			virtual ~fsm2()	{
				for( auto it : __m_trans ){
					if( it.second != nullptr ){
						delete it.second;
					}
				}

				for( auto it : __m_stat ){
					if( it.second != nullptr ){
						delete it.second;
					}
				}
			}
			/**
			 * @brief 添加节点
			 * @param stat[ I ], 节点ID
			 * @param ent[ I ], 当转换进入节点的时候触发的函数
			 * @return 成功操作返回OK, 否则返回错误代码
			 */
			emErrCode add( const statType& stat , enterFun_t ent ){
				emErrCode ret = OK;
				if( stat.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( stat ) ){
					__add_stat( stat , ent );
				}else{
					auto it = __m_stat.find( stat );
					if( it != __m_stat.end() ){
						it->second->cb_enter = ent;
					}
				}
				return ret;
			}
			/**
			 * @brief 添加节点，仅仅指定离开回调函数
			 * @param stat[ I ]
			 * @param leave[ I ]
			 * return 成功操作返回OK， 否则返回false
			 */
			emErrCode add( const statType& stat , leaveFun_t leave ){
				emErrCode ret = OK;
				if( stat.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( stat ) ){
					__add_stat( stat );
				}

				__set_leave( stat , leave );

				return ret;
			}
			/**
			 * @brief 添加状态节点, 并指定进入回调和转出回调
			 * @param stat[ I ],
			 * @param ent[ I ],
			 * @param leave[ I ],
			 */
			emErrCode add( const statType& stat, enterFun_t ent , leaveFun_t leave ){
				emErrCode ret = OK;
				if( stat.empty() ) return ERR_STAT_ID_EMPTY;
				if( __has_stat( stat ) == false ){
					__add_stat( stat , ent , leave );
				}
				return ret;
			}
			/**
			 * @brief 增加节点和转换。
			 * @param from[ I ]
			 * @param to[ I ]
			 * @param ent[ I ]
			 * @return
			 * @note 这个函数添加的状态进入函数用于目标节点
			 */
			emErrCode add( const statType& from , const statType& to , enterFun_t ent  ){
				if( !__has_stat( from )){
					__add_stat( from );
				}

				if( !__has_stat( to )){
					__add_stat( to , ent );
				}else{
					auto it = __m_stat.find( to );
					if( it != __m_stat.end() ){
						it->second->cb_enter = ent;
					}
				}

				if( !__has_trans( from , to )){
					__add_trans( from ,to );
				}

				return OK;
			}
			/**
			 * @brief 添加转换，但是不包含raise函数。这个函数主要用于已经配置了状态触发函数或者不需要状态触发函数的情况
			 * @param from[ I ]
			 * @param to[ I ]
			 */
			emErrCode add( const statType& from , const statType& to )	{
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( from ) ){
					__add_stat( from );
				}

				if( !__has_stat( to ) ){
					__add_stat( to );
				}

				if( __has_trans( from , to ) == false ){
					__add_trans( from , to );
				}

				return ret;
			}

			emErrCode add( const statType& from , const statType& to , raiseFun_t raise ){
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( from ) ){
					__add_stat( from );
				}

				if( !__has_stat( to ) ){
					__add_stat( to );
				}

				if( __has_trans( from , to ) == false ){
					__add_trans( from , to , raise );
				}else{
					auto it = __m_trans.find( from + to );
					if( it != __m_trans.end() ){
						it->second->cb_raise = raise;
					}
				}

				return ret;
			}

			emErrCode add( const statType& from , const statType& to , leaveFun_t leave  ){
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( __has_stat( from ) == false ){
					__add_stat( from );
				}
				__set_leave( from , leave );
				if( __has_stat( to ) == false ){
					__add_stat( to );
				}

				if( __has_trans( from , to ) == false ){
					__add_trans( from , to );
				}

				return ret;
			}

			emErrCode add( const statType& from , const statType& to , leaveFun_t leave  , raiseFun_t raise ){
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( __has_stat( from ) == false ){
					__add_stat( from );
				}
				__set_leave( from , leave );
				if( __has_stat( to ) == false ){
					__add_stat( to );
				}

				if( __has_trans( from , to ) == false ){
					__add_trans( from , to , raise );
				}else{
					auto it = __m_trans.find( from + to );
					if( it != __m_trans.end() ){
						it->second->cb_raise = raise;
					}
				}

				return ret;
			}

			/**
			 * @brief
			 */
			emErrCode add( const statType& from , const statType& to , enterFun_t ent , raiseFun_t raise ){
				emErrCode ret = OK;

				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;
				if( !__has_stat( from )){
					__add_stat( from );
				}
				if( !__has_stat( to ) ){
					__add_stat( to, ent );
				}else{
					auto it = __m_stat.find( to );
					if( it != __m_stat.end() ){
						it->second->cb_enter = ent;
					}
				}

				if( !__has_trans( from , to )){
					__add_trans( from , to , raise );
				}else{
					auto it = __m_trans.find( from + to );
					if( it != __m_trans.end() ){
						it->second->cb_raise = raise;
					}
				}
				return ret;
			}

			/**
			 * @brief 添加节点和转换
			 * @param from[ I ]
			 * @param to[ I ],
			 * @param ent[ I ], ent作用于to节点
			 * @param leave[ I ], leave作用于from节点
			 *
			 */
			emErrCode add( const statType& from , const statType& to , enterFun_t ent , leaveFun_t leave ){
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( from ) ){
					__add_stat( from );
				}

				__set_leave( from , leave );
				if( !__has_stat( to ) ){
					__add_stat( to , ent );
				}else{
					auto it = __m_stat.find( to );
					if( it != __m_stat.end() ){
						it->second->cb_enter = ent;
					}
				}

				return ret;
			}
			/**
			 * @brief
			 */
			emErrCode add( const statType& from , const statType to,  enterFun_t ent , leaveFun_t leave , raiseFun_t raise ){
				emErrCode ret = OK;
				if( from.empty() ) return ERR_STAT_ID_EMPTY;
				if( to.empty() ) return ERR_STAT_ID_EMPTY;

				if( !__has_stat( to ) ){
					__add_stat( to , ent );
				}

				if( !__has_trans( from , to ) ){
					__add_trans( from , to , raise );
				}

				__set_leave( from , leave );

				return ret;
			}

			/**
			 * @brief 启动执行.标记状态初始状态，如果初始状态包含了进入回调函数，则应该调用这个回调函数
			 */
			void start( ){
				if( __m_is_running == true ){  // 状态机正在运行
					ERROR_MSG( "状态机正在运行" );
					return;
				}
				// 标记状态机为运行状态
				__m_is_running = true;

				auto it = __m_stat.find( __m_start );
				if( it != __m_stat.end() && it->second->cb_enter ){
					bool r = it->second->cb_enter( __m_start , __m_data );
					__m_curr = __m_start;
					if( r == AUTO_RUN && it->second->cb_leave ){
						std::string next = it->second->cb_leave( __m_start , __m_data );
						transform( next );
					}
				}

				MSG( "状态机就绪" , TGREEN );
			}
			/**
			 * @brief 判断状态机是否正在运行
			 * @return 正在运行返回true，否则返回false
			 */
			bool isRunning(){
				return __m_is_running.load();
			}
			/**
			 * @brief 强制转换状态到指定状态。虽然是强制转换，但是仍然要求两个状态之间存在转换路径。这个
			 *   可以用户旁路触发的情况
			 */
			emErrCode transform( const statType& next )	{
				emErrCode ret = OK;

				auto it = __m_stat.find( __m_curr );
				if( it == __m_stat.end() ){ // 当前状态名称不是已经登记在状态表中的数据
					return ERR_STAT_NOT_DEFINED;
				}

				auto tIt = __m_trans.find( __m_curr + next );
				if( tIt != __m_trans.end() ){ // 执行转换操作。
					if( tIt->second->cb_raise ){
						tIt->second->cb_raise( __m_curr , next , __m_data );
					}
				}

				// 转换后需要检查下一个内容是否有进入操作函数，如果有这个操作需要
				// 调用这个函数
				__m_curr = next;
				it = __m_stat.find( __m_curr );
				if( it != __m_stat.end() && it->second->cb_enter ){
					bool rst = it->second->cb_enter( __m_curr , __m_data );
					if( rst == AUTO_RUN ){
						std::string n = it->second->cb_leave( __m_curr  , __m_data );
						// 如果有自动转换的节点，则递归调用进行状态转换
						ret = transform( n );
					}
				}

				return ret;
			}
			/**
			 * @brief 接受数据执行状态转换
			 * @param 接受到的数据
			 * @return 成功操作返回0K，否则返回错误代码
			 */
			emErrCode eat( const realT& data ){
				emErrCode ret = OK;
				if( __m_is_running == false ){
					return ERR_FSM_NOT_START;
				}
				// 记录数据内容。在整个运行期间，旧的数据持续填入缓冲。当
				// 一定的条件到达后再执行清理数据动作
				__m_data.m_raw_data.push_back( data );

				auto it = __m_stat.find( __m_curr );
				if( it == __m_stat.end() ){
					return ERR_CAN_NOT_FIND_STAT;
				}
				if( it->second == nullptr ){
					return ERR_STAT_OBJECT_NULL;
				}
				// 1） 离开上一个节点
				if( it->second->cb_leave){ // 除去结束节点外，每一个节点都有转出操作，在转出回调函数中判断下一个节点
					__m_next = it->second->cb_leave( __m_curr , __m_data );
				}else{
					ret = ERR_STAT_FINISHED;
					__m_is_running = false;
				}
				// 2） 执行状态准换过程，这个是离开上一个几点还未到下一个节点的动作
				auto tIt = __m_trans.find( __m_curr + __m_next );
				// 执行转换操作。
				if( tIt != __m_trans.end() &&  tIt->second->cb_raise ){ // 如果转换包含了raise函数，则执行raise函数通知外部进行处理
					tIt->second->cb_raise( __m_curr , __m_next , __m_data );
				}

				// 3）完成转换进入新的状态
				__m_curr = __m_next;
				// 转换后需要检查下一个内容是否有进入操作函数，如果有这个操作需要
				// 调用这个函数
				it = __m_stat.find( __m_curr );
				if( it != __m_stat.end() && it->second->cb_enter ){
					bool rst = it->second->cb_enter( __m_curr , __m_data );
					// 如果状态需要自动转换
					if( rst == AUTO_RUN ){
						std::string next = it->second->cb_leave( __m_curr  , __m_data );
						ret = transform( next );
					}
				}else if( it == __m_stat.end() ){
					return ERR_STAT_NOT_DEFINED;
				}

				return ret;

			}
			/**
			 * @brief 返回错误信息
			 */
			static std::string errMsg( emErrCode e ){
				std::string ret;
				switch( e ){
				case ERR_ALLOC_MEM:
					ret = "内存分配失败";
					break;
				case ERR_FSM_NOT_START:
					ret = "状态机没有启动";
				default:break;
				}
				return ret;
			}
			/**
			 * @brief 指定离开的回调函数
			 */
			emErrCode setLeave( const statType& id , leaveFun_t fun ){
				emErrCode ret = OK;
				auto it = __m_stat.find( id );
				if( it != __m_stat.end() ){
					it->second->cb_leave = fun;
				}else{
					ret = ERR_STAT_NOT_DEFINED;
				}
				return ret;
			}
			/**
			 * @brief 清理缓存数据内容
			 */
			inline void clearRaw(){
				__m_data.m_raw_data.erase( __m_data.m_raw_data.begin() , __m_data.m_raw_data.end() );
			}
			/**
			 * @brief 删除首元素
			 */
			inline void popRaw(){
				if( __m_data.m_raw_data.size() > 0 ){
					__m_data.m_raw_data.erase( __m_data.m_raw_data.begin() );
				}
			}
		private:
			/**
			 * @brief 判断节点是否存在
			 * @param id [ I ], 状态节点ID，
			 * @return 如果节点存在则返回true，否则返回false
			 */
			bool __has_stat( const statType& id ){
				return ( __m_stat.find( id ) != __m_stat.end() );
			}
			/**
			 * @brief 判断转换是否存在
			 * @param from[ I ]
			 * @param to[ I ]
			 * @return 如果节点存在则返回true，否则返回false
			 */
			bool __has_trans( const statType& from , const statType& end ){
				bool ret = false;
				ret = ( __m_trans.find( from + end ) != __m_trans.end() );
				return ret;
			}
			void __add_stat( const statType& name ){
				assert( !name.empty() );
				state * s = new state( name );
				__m_stat.insert( std::make_pair( name , s ) );
			}
			/**
			 * @brief 添加状态。
			 * @param name[ I ], 要添加的节点ID
			 * @param ent[ I ], 进入状态调用的回调函数
			 */
			void __add_stat( const statType& name , enterFun_t ent ){
				assert( !name.empty() );
				state * s = new state( name );
				if( ent ){
					s->cb_enter = ent;
				}

				__m_stat.insert( std::make_pair( name , s ) );
			}
			/**
			 */
			void __set_leave( const statType& name, leaveFun_t leave ){
				auto it = __m_stat.find( name );
				if( it != __m_stat.end() ){
					it->second->cb_leave = leave;
				}
			}
			/**
			 * @brief 添加状态
			 */
			void  __add_stat( const statType& name , enterFun_t ent , leaveFun_t leave )	{
				assert( !name.empty() );
				state * s( name );
				if( ent ){
					s->cb_enter = ent;
				}

				if( leave ){
					s->cb_leave = leave;
				}

				__m_stat.insert( std::make_pair( name , s ) );
			}
			/**
			 * @brief 添加转换。
			 * @param from[ I ]，转换的起始状态
			 * @param to[ I ], 转换的结束状态
			 * @param raise[ I ], 执行转换的时候调用的回调函数
			 */
			void  __add_trans( const statType& from , const statType& to , raiseFun_t raise )	{
				if( !__has_stat( from ) ){
					__add_stat( from );
				}
				if( !__has_stat( to ) ){
					__add_stat( to );
				}

				if( !__has_trans( from , to ) ){
					trans * __t = new trans( from , to );
					if( raise ){
						__t->cb_raise = raise;
					}
					std::string key = from + to;
					__m_trans.insert( std::make_pair( key , __t ) );
				}else{
					auto it = __m_trans.find( from + to );
					if( it != __m_trans.end() ){
						it->second->cb_raise = raise;
					}
				}
			}

			void __add_trans( const statType& from , const statType& to){
				trans * __t = new trans( from , to );

				std::string key = from + to;
				__m_trans.insert( std::make_pair( key , __t ) );
			}
		};
#if defined( FSM_USE_TYPE )
#    define onFsm2ENT( statType , type , fun ) [&]( const statType& s, const type::stDataItem& data )->bool{ return fun( s , data ); }
#    define onFsm2RAISE( statType , type , fun ) [&]( const statType& fs , const std::string& ts , const type::stDataItem& data){ fun( fs , ts , data ); }
#    define onFsm2LEAVE( statType , type , fun ) [&]( const statType& s, const type::stDataItem& data )->std::string{ return fun( s , data ); }
#else
#    define onFsm2ENT( fun ) [&]( const std::string& s, const fsm2::stDataItem& data )->bool{ return fun( s , data ); }
#    define onFsm2RAISE( fun ) [&]( const std::string& fs , const std::string& ts , const fsm2::stDataItem& data){ fun( fs , ts , data ); }
#    define onFsm2LEAVE( fun ) [&]( const std::string& s, const fsm2::stDataItem& data )->std::string{ return fun( s , data ); }
#endif
	} // namespace fsm2;
}// namespace wheels

