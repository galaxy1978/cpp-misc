#if defined __TIMER_OLD__
#    include "timer_old.cpp"
#else
#include "timer.hpp"
using namespace wheels;
#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WIN32 ) || defined( WINNT )
#    include "details/winTimer.hpp"
#else
#    include "details/linuxTimer.hpp"
#endif

CTimer :: CTimer():
	__pt_imp( new

#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WIN32 ) || defined( WINNT )	
		  winTimer()
#elif defined( __LINUX__ )
		  linuxTimer()
#endif
	){}

CTimer :: CTimer( long ms , bool oneShot ):
	__pt_imp( new 
#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WIN32 ) || defined( WINNT )	
		  winTimer( ms , oneShot )
#elif defined( __LINUX__ )
		  linuxTimer( ms , oneShot )
#endif
	){}

CTimer :: CTimer( std::function< void () > fun , long ms , bool oneShot ):
	__pt_imp( new
#if defined( __WIN32 ) || defined( __WIN64 ) || defined( WIN32 ) || defined( WINNT )	
		  winTimer( fun , ms , oneShot )
#elif defined( __LINUX__ )
		  linuxTimer( fun , ms , oneShot )
#endif										     
	){}

CTimer :: ~CTimer(){}

iTimer :: emErrCode
CTimer :: Start( long ms , bool oneShot )
{
	return __pt_imp->Start( ms , oneShot );
}

void CTimer :: SetOwner( std::function<void(  )> fun )
{
	__pt_imp->SetOwner( fun );
}

iTimer :: emErrCode
CTimer :: Stop(  )
{
	return __pt_imp->Stop();
}

bool CTimer :: IsRuning(  )
{
	return __pt_imp->IsRuning();
}

iTimer :: emErrCode
CTimer :: Reset()
{
	return __pt_imp->Reset();
}
#endif
