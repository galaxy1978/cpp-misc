#include "timer.hpp"
#include <iostream>
#include <sys/time.h>
//#include "defines.hpp"
#include "misc.hpp"

void CTimer::Stop(  )
{
	m_is_run = false;
}

void CTimer::Start( long ms , bool oneShot )
{
	if( m_is_run == false ){
		m_is_run = true;
		if( ms > 0 ){
			m_tick = ms;
		}
		m_is_one_shot = oneShot;
		//运行记时线程
		if( m_cb ){
			std::thread thd( std::bind( &CTimer::thd_fun , this ) );
			thd.detach();
		}
	}
}

void CTimer::thd_fun(  )                    //记时线程,在后台运行
{
	long __t = m_tick;
	bool __run = m_is_run , __one_shot = m_is_one_shot;
	bool __fun_ok  = m_cb ? true : false;

	struct timeval  __s_time , __e_time;

	gettimeofday( &__s_time , NULL );
	while( __run == true ){
		if( m_reset == true ){
			gettimeofday( &__s_time , NULL );
			m_reset = false;
		}

		gettimeofday( &__e_time , NULL );
		long    __diff_ms = (__e_time.tv_sec - __s_time.tv_sec ) * 1000  + ( __e_time.tv_usec - __s_time.tv_usec ) / 1000;

		if( __diff_ms < __t && __t > 0 ){//比较时钟
			__run = m_is_run;
			std::this_thread::yield();
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ));
			continue;
		}
		if( __one_shot == true && __fun_ok ){//单次触发
			m_is_run = false;

			std::thread thd( [this]{
					std::this_thread::yield();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					m_cb();
			} );
			thd.detach();
		}else{//连续触发
			if( __fun_ok ){
				m_is_run = false;

				std::thread __thd( m_cb );
				__thd.join();
			}

			Start( -1 , false );        //重新启动
		}
		__run = m_is_run;
	}
}

void CTimer::SetOwner( std::function<void(  )> fun )
{
	m_cb = fun;
}

bool CTimer::IsRuning(  )
{
	bool ret = false;
	//m_mutex.lock(  );
	ret = m_is_run;
	//m_mutex.unlock(  );
	return ret;
}
