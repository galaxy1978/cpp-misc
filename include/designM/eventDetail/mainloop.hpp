/**
 * @brief 主循环.
 * @version 1.0
 * @author 宋炜
 * @date 2023-5-18
 */

#pragma once

#include <atomic>
#if CMD_USE_SINGLETON
#    include "designM/singleton.hpp"
#endif
namespace wheels
{
	namespace dm
	{
		/// @brief
		/// @tparam cmdType 命令类型，要求这个类型支持拷贝构造，赋值拷贝;如果属实类则应该是可以
		///    作为方便检索的基础类型，比如整型数据。
		template< typename cmdType >
		class mainLoop
#if CMD_USE_SINGLETON
			: public singleton< mainLoop >
#endif
		{
			static_assert( std::is_integral< cmdType >::value || ( 
					       std::is_class< cmdType >::value && 
					       std::is_copy_assignable< cmdType >::value &&
					       std::is_copy_constructible< cmdType > ::value 
					       ) , "" );
		private:
			std::atomic< bool >   __m_is_running;
			std::atomic< long >   __m_over_time;
			std::unique_ptr< dispatcher< cmdType > >   __pt_dsptch;
		public:
			mainLoop( long ovt ): __m_is_running( false ) , __m_over_time( 10 ){
#if CMD_USE_SINGLETON
				dispatcher<cmdType>::create();
#else
				__pt_dsptch.reset( new dispatcher );
#endif
			}
			virtual ~mainLoop(){
				stop();
			}
			/**
			 * @brief 启动主循环
			 */
			bool exec(){
				if( __m_is_running == true ) return true;
				
				__m_is_running = true;
#if CMD_USE_SINGLETON
				dispatcher * p_dsptch = dispatcher::get();
#else
				dispatcher * p_dsptch = __pt_dsptch.get();
#endif
				if( p_dsptch == nullptr ) return false;
				
				while( __m_is_running.load() ){
					p_dsptch->dispatch( __m_over_time.load() );
				}

				return true;
			}

			void stop(){
				if( __m_is_running == false ) return;
				
				__m_is_running = false;
#if CMD_USE_SINGLETON
				dispatcher * p_dsptch = dispatcher::get();
#else
				dispatcher * p_dsptch = __pt_dsptch.get();
#endif
				
				if( p_dsptch ){
					p_dsptch->clear();
				}
			}

			bool isRunning(){ return __m_is_running.load(); }
			
			dispatcher * getDispatch(){
#if CMD_USE_SINGLETON
				dispatcher * p_dsptch = dispatcher::get();
#else
				dispatcher * p_dsptch = __pt_dsptch.get();
#endif
				return p_dsptch;
			}
		};
#if CMD_USE_SINGLETON
#define IMP_CMD( cmdType )  IMP_SINGLETON( mainLoop< cmdType > ); \
		IMP_SINGLETON( dispatcher< cmdType> );
#endif
	}
}
