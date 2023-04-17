#include "misc.hpp"

#include "details/linuxTimer.hpp"

/**
 * @brief 
 */
void linux_on_timer( sigval info )
{
	linuxTimer * obj = (linuxTimer *)info.sival_ptr;
	if( obj == nullptr ){
		return;
	}

	obj->__on_sys_timer();
}


void linuxTimer :: __on_sys_timer()
{
	if( __m_is_one_shot == true ){
		Stop();
		__m_is_run = false;
		__m_id = 0;
	}
	if( __m_cb ){
		__m_cb();
	}
}

linuxTimer :: linuxTimer():
	__m_is_one_shot( false ),
	__m_tick( 0 ),
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (timer_t)-1 )
{
	emErrCode e = __init_timer();
	if( e != OK ){
		ERROR_MSG( errMsg( e ) );
		throw e;
	}
}

linuxTimer :: linuxTimer( long ms , bool oneShot ) :
	__m_is_one_shot( oneShot ),
	__m_tick( ms ) ,
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (timer_t)-1 )
{
	emErrCode e = __init_timer();
	if( e != OK ){
		ERROR_MSG( errMsg( e ) );
		throw e;
	}
}

linuxTimer :: linuxTimer( std::function<void()> fun , long ms , bool oneShot ):
	__m_is_one_shot( oneShot ),
	__m_tick( ms ) ,
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (timer_t)-1 ),
	__m_cb( fun )
{
	emErrCode e = __init_timer();
	if( e != OK ){
		ERROR_MSG( errMsg( e ) );
		throw e;
	}
}

iTimer :: emErrCode
linuxTimer :: Start( long ms , bool oneShot )
{
	emErrCode ret = OK;
	if( ms > 0 ){
		__m_tick = ms;
	}
	if( !__m_cb ){
		return ERR_MISSING_INFO_FUN;
	}

	if( __m_id.load() == nullptr ){
		ret = __init_timer();
	}
	if( __m_tick.load() <= 0 ){
		return ERR_TICK;
	}
	__m_is_one_shot = oneShot;
	if( ret == OK ){
		
		__m_new.it_value.tv_sec = __m_tick.load() / 1000;
		__m_new.it_value.tv_nsec = ( __m_tick.load() % 1000 ) * 1000;
		
		if( oneShot == false ){ // 如果是连续触发，则配置冷却时间为非0值
			__m_new.it_interval.tv_sec = __m_new.it_value.tv_sec;
			__m_new.it_interval.tv_nsec = __m_new.it_value.tv_nsec;
		}else{
			__m_new.it_interval.tv_sec = 0;
			__m_new.it_interval.tv_nsec = 0;
		}
		
		int rst = timer_settime( __m_id , 0 , &__m_new , nullptr );
		if( rst < 0 ){
			ret = ERR_START_TIMER;
		}else{
			__m_is_run = true;
		}
	}

	return ret;
}

void linuxTimer :: SetOwner( std::function<void(  )> fun )
{
	__m_cb = fun;
}

iTimer :: emErrCode
linuxTimer :: Stop(  )
{
	emErrCode ret = OK;
	if( __m_id == nullptr ){
		return OK;
	}
	int rst = timer_delete( __m_id.load() );
	if( rst != 0 ){
		ret = ERR_STOP_TIMER;
	}else{
		__m_id = nullptr;

		memset( &__m_evt , 0 , sizeof( struct sigevent ) );

		__m_is_run = false;
	}
	return ret;
}

bool linuxTimer :: IsRuning(  )
{
	bool ret = false;
	ret = __m_is_run.load();
	return ret;
}

iTimer :: emErrCode
linuxTimer :: __init_timer()
{
	emErrCode ret = OK;

    memset( &__m_evt , 0 , sizeof( struct sigevent ) );
	
	__m_evt.sigev_notify = SIGEV_THREAD;
	__m_evt.sigev_value.sival_ptr = this;
	__m_evt.sigev_notify_function = linux_on_timer;
	timer_t id;
	int rst = timer_create( CLOCK_REALTIME , &__m_evt , &id );
	if( rst != 0 ){
		ret = ERR_CREATE_TIMER;
		__m_id = 0;
	}else{
		__m_id = id;
	}

	return ret;
}

iTimer :: emErrCode
linuxTimer :: Reset()
{
	auto rst = Stop();
	if( rst == OK ){
		rst = Start( -1, __m_is_one_shot.load() );
	}

	return rst;
}
