#include <windows.h>

#include "misc.hpp"

#include "details/winTimer.hpp"

std::map< MMRESULT , winTimer * >   winTimer :: gTimerRecord;

void CALLBACK on_win_timer(UINT id , UINT msg , DWORD_PTR , void * duser, DWORD_PTR dw1 , DWORD_PTR dw2 )
{
	auto it = winTimer::gTimerRecord.find( id );
	if( it != winTimer::gTimerRecord.end() ){
		winTimer * obj = it->second;
		if( obj ){
			obj->__on_sys_timer();
		}
	}
}

winTimer :: winTimer():
	__m_is_one_shot( false ),
	__m_tick( -1 ),
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (MMRESULT)-1 ){}

winTimer :: winTimer( long ms , bool oneShot ):
	__m_is_one_shot( oneShot ),
	__m_tick( ms ) ,
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (MMRESULT )-1){}

winTimer :: winTimer( std::function<void()> fun , long ms , bool oneShot  ):
	__m_is_one_shot( oneShot ),
	__m_tick( ms ) ,
	__m_is_run( false ) ,
	__m_reset( false ),
	__m_id( (MMRESULT )-1),
	__m_cb( fun ){}

iTimer :: emErrCode
winTimer :: Start( long ms , bool oneShot )
{
	emErrCode ret = OK;
	if( !__m_cb ){
		return ERR_MISSING_INFO_FUN;
	} 
	if( ms > 0 ){
		__m_tick = ms;
	}
	__m_is_one_shot = oneShot;
	UINT evt = TIME_PERIODIC;
	if( __m_is_one_shot == true ){
		evt = TIME_ONESHOT;
	}
	if( __m_tick.load() <= 0 ){
		return ERR_TICK;
	}
	__m_id = timeSetEvent( __m_tick.load() , 1 , (LPTIMECALLBACK)on_win_timer , 0 , evt );

	if( __m_id == 0 ){
		ret = ERR_CREATE_TIMER;
		ERROR_MSG( errMsg( ret ) );
	}else{
		gTimerRecord.insert( std::make_pair( __m_id , this ) );
	}
	return ret;
}

void winTimer :: SetOwner( std::function<void(  )> fun )
{
	__m_cb = fun;
}

iTimer :: emErrCode
winTimer :: Stop()	
{
	emErrCode ret = OK;
	auto e = timeKillEvent( __m_id );

	if( e == TIMERR_NOERROR ){
		gTimerRecord.erase( gTimerRecord.find( __m_id ) );
		__m_id = (MMRESULT)-1;
		__m_is_run = false;
	}else{
		ret = ERR_STOP_TIMER;
	}

	return ret;
}

bool winTimer :: IsRuning(  )
{
	bool ret = __m_is_run.load();
	return ret;
}

iTimer :: emErrCode
winTimer :: Reset()
{
	auto ret = Stop();
	
	if( ret == OK ){
		ret = Start( -1, __m_is_one_shot.load() );
	}

	return ret;
}

void winTimer :: __on_sys_timer()
{
	if( __m_cb ){
		__m_cb();
	}
}
