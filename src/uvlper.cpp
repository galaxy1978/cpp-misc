#include <thread>
#include "uvlper.hpp"

uv_loop_t * looper :: pt_loop = nullptr;

looper :: looper()
{
    m_is_run = false;
    pt_loop = uv_default_loop();
    if( pt_loop )
        pt_loop -> data = this;
}

looper :: ~looper()
{
    if( pt_loop ){
	uv_stop( pt_loop );
	m_is_run = false;
	std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));

    }
}

void looper :: run( uv_run_mode mode )
{
    m_is_run = true;
    
    std::thread *thd = new std::thread([ = ]{
	    do{
                    uv_run( pt_loop , mode );

                    std::this_thread::yield();
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ));
	    }while( m_is_run.load() );
    });
    
    thd->detach();
}

void looper :: stop()
{
    m_is_run = false;
    if( pt_loop )
        uv_stop( pt_loop );
}

looper * CreateLooper()
{
    looper * ret = nullptr;
    if( looper::pt_loop == nullptr ){
        ret = new looper();
    }
    if( ret )
        ret->run( UV_RUN_DEFAULT );
    return ret;
}

looper * GetLooper()
{
      looper * ret = nullptr;
      if( looper :: pt_loop )
	    ret = ( looper *)( looper::pt_loop->data );
      return ret;
}
