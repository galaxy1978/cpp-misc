/**
 * @brief 线程池实现模块
 * @author 宋炜
 * @date 2023-6-26
 * @version 1.0
 */
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <future>
namespace wheels
{
	class threadPool
	{
	public:
	
	private:
		std::vector<std::thread>             m_works__;              // 线程池
		std::queue<std::function<void()> >   m_tasks__;              // 任务队列
		std::mutex                           m_queue_mutex__;        // 
		std::condition_variable              m_condition__;         
		std::atomic< bool >                  m_stop__;               // 是否已经停止
		std::atomic< int >                   m_count__;              // 线程池容量
	private:
		/**
		 * @brief 实际处理任务的方法。
		 */
		void run_task__(){
			while( !m_stop__.load() ) {
				std::function<void()> task;   // 实际要执行的任务
			
				{
					std::unique_lock<std::mutex> lock(m_queue_mutex__);
					m_condition__.wait(lock, [&] {
						return m_stop__ || !m_tasks__.empty();
					});
					// 结束执行，这里使用执行完所有的已经排队的线程后才可以退出执行
					if (m_stop__ && m_tasks__.empty()) {
						return;
					}
					// 拿取任务
					task = std::move(m_tasks__.front());
					m_tasks__.pop();
				}
				// 执行任务
				task();
			}
		}
	public:
		threadPool(size_t numThreads) : m_stop__( true ) , m_count__( numThreads ){}
		~threadPool() { stop(); }
		/**
		 * @brief 排队任务
		 */
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
			using return_type = typename std::result_of<F(Args...)>::type;
			
			auto task = std::make_shared<std::packaged_task<return_type()>>( std::bind(std::forward<F>(f), std::forward<Args>(args)...) );

			std::future<return_type> res = task->get_future();

			{
				std::unique_lock<std::mutex> lock(m_queue_mutex__);
			
				m_tasks__.emplace([task]() { (*task)(); });
			}

			m_condition__.notify_one();
			return res;
		}

		
		/**
		 * @brief 清理掉还没有执行的任务
		 */
		void clearNotRunning(){
			std::unique_lock< std::mutex > lock( m_queue_mutex__ );
			while( !m_tasks__.empty() ){
				m_tasks__.pop();
			}
		}
		/**
		 * @brief 启动或者停止线程
		 * @param sw[ I ], true启动线程池；false结束线程池
		 */
		void start( bool sw = true ){
			if( sw ){
				if( !m_stop__.load() ) return;   // 线程池正在运行
				m_stop__ = !sw;
				for (size_t i = 0; i < m_count__.load(); ++i) {
					m_works__.emplace_back( std::bind( &threadPool::run_task__ , this ) );
				}
				m_condition__.notify_all();
			}else{
				clearNotRunning();
				m_stop__ = !sw;
				m_condition__.notify_all();
				for (std::thread& worker : m_works__) {
#if defined( __POSIX__ ) || defined( __LINUX__ )
					pthread_t id = worker.native_handle();
					pthread_cancel( id );
#else
					HANDLE id = worker.native_handle();
					TerminateThread( id , 0 );
#endif
				}
			}

				 
		}
		/**
		 * @brief 获取线程池容量
		 */
		int count(){ return m_count__.load(); }
		/**
		 * @brief 指定线程池容量，修改前线程池需要处于停止状态
		 * @return 成功操作返回true，否则返回false
		 */
		bool count( int count ){
			if( !m_stop__.load() ){
				return false;
			}

			m_count__ = count;
			return ret;
		}
	       
		inline void stop(){ start( false ); }

	};
}
