#include <iostream>
#include <thread>
#include <conditional_variable>
#include <mutex>

#include "designM/command.hpp"
#include "container/variant.hpp"

using namespace wheels;
using namespace dm;

enum class emEVT
{
	EVT_A,
	EVT_B, 
	EVT_C
}

IMP_CMD( emEVT );

using evtLoop_t = mainLoop< emEVT >;
using evtDispt_t = dispatcher< emEVT >;

std::mutex    gMutex;
std::conditional_variable  cv;

int main()
{
	// 
	auto * p_loop = evtLoop_t::create( 10 );
	if( p_loop == nullptr ){
		return -1;
	}
	
	auto * p_dispt = p_loop->getDispatch();
	if( !p_dispt ){
		return -2;
	}
	
	p_dispt->connect( emEVT::EVT_A , []( const variant& data ){ 
		int d = data.get< int >();
		std::cout << d << std::endl;
	});
	
	p_dispt->connect( emEVT::EVT_A , []( const variant& data ){ 
		std::string d = data.get< std::string >();
		std::cout << d << std::endl;
	});
	
	p_dispt->connect( emEVT::EVT_A , []( const variant& data ){ 
		float d = data.get< float >();
		std::cout << d << std::endl;
		
		cv.notify_all();
	});
	
	p_loop->exec();
	
	p_dispt->emit( emEVT::EVT_A , 11 );
	p_dispt->emit( emEVT::EVT_B , std::string( "abcdef" ) );
	p_dispt->emit( emEVT::EVT_C , 11.34 );
	
	{
		std::unique_lock< std::mutex > lck( gMutex );
		cv.wait( lck );
	}

    p_loop->stop();
	return 0;
}