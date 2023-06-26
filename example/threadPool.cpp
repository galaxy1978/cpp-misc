#include <iostream>
#include <vector>
#include <functional>
#include <future>
#include "threadPool.hpp"
class a
{
public:
	int taskFunction(int id) {
		std::cout << "Task " << id << " started" << std::endl;
		// 模拟一些工作
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Task " << id << " finished" << std::endl;
		
		return id * 2 ;
	}
};

int main() {
	a a1;
	threadPool pool(4); // 创建一个线程池，有4个工作线程
	std::vector< std::future< int > >  rst( 8 );
	
	// 将任务提交到线程池
	for (int i = 0; i < 8; ++i) {
		rst[ i ] = pool.enqueue( std::bind( &a::taskFunction, &a1 , std::placeholders::_1) , i );
	}
	
	pool.start( true );
	
	// 等待所有任务完成
	std::this_thread::sleep_for(std::chrono::seconds(5));
    
    for( int i = 0; i < 8; i ++ ){
        std::cout << "thread " << i << " result: " << rst[ i ].get() << std::endl;
    }
	return 0;
}