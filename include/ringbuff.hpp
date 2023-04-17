/**
 * @brief 环形定容量缓冲区模块。操作方式如下：
 *   1 在数据插入点不断的附加数据，如果数据已经占满缓冲区，则覆盖最老的数据
 *   2 在数据获取点获取数据
 *   这个缓冲区是一个定长的缓冲区， 操作过程中增加数据并不会引起缓冲区尺寸的调整，如果需要调整缓冲区长度必须进行手动调整。
 * @version 1.1
 * @date 2019-11-11
 * @author 宋炜
 *
 * @warning ****  为了提高内存检索的效率这个模块包含了x86兼容机的汇编代码，因此模块不能使用在其他指令集的硬件上  ****
 */

#ifndef __RINGBUFF_HPP__
#define __RINGBUFF_HPP__

#include <string>
#include <thread>
#include <mutex>

/// 2020-1-15 宋炜  增加index检索数据功能。以方便对数据进行检索，特别是对通讯中的数据进行分割标记的检索提供了方便
/// 2020-1-15 宋炜  增加length获取数据长度功能
/// 2020-1-15 宋炜  增加drop丢弃数据功能
namespace wheels
{
class rb
{
public:
	enum emErrCode{
		ERR_ALLOC_MEM = -1000,      // 内存分配失败
		ERR_NO_DATA,                // 当前缓冲区没有数据
		ERR_LESS_DATA,              // 当前数据长度少于请求长度
		ERR_PTHREAD_MUTEX_INIT,     // 初始化pthread_mutex错误
		ERR_SRC_NULL,               // 源数据指针空
		ERR_DST_NULL,               // 目标数据指针空
		ERR_DATA_NULL,              // 缓冲区指针空
		ERR_IDX_CAN_NOT_FIND,       // 检索数据的时候没有找到内容
		OK = 0
	};
private:
	void      * __p_data;               // 数据存储区

	size_t      __m_maxSize;            // 最大容量
	size_t      __m_curr_len;           // 当前占用量
	size_t      __m_curr_add_point;     // 当前插入点
	size_t      __m_curr_get_point;     // 当前取数据点
	/// 如果需要在多线程下运行， 每次存取数据可能运行在不同的线程中，因此每一个操作动作
	/// 都必须保证是线程安全的。在vxWorks中存在task和pthread两种方式，两种方式加锁
	/// 操作是不一样的。
	std::mutex  __m_mutex;
private:

public:
	/**
	 * @brief 构造对象，在默认的情况下构造一个64K BYTE的缓冲区，并且将缓冲区全部清除为0
	 * @param size[ I ], 要分配的内存大小
	 * @param autoZero[ I ], true初始化的时候讲内存设置为0，false不干涉原有的随机内容
	 * @exception 如果内存分配失败跑出ERR_ALLOC_MEM
	 */
	rb( size_t size = 65536 , bool autoZero = true );
	rb( const rb& b );

	virtual ~rb();

	rb& operator=( const rb& b );
	/**
	 * 添加数据
	 * @param data[ I ], 要添加的数据
	 * @param size[ I ], 要附加的数据的长度
	 * @param 成功操作返回__OK， 否则返回错误代码
	 */
	emErrCode append( const void * data , size_t size );
	/**
	 * @brief 拷贝数据到给定内存。
	 * @note 这个操作会消耗掉内存中的数据， 也就是会导致存取点和数据长度的调整
	 * @param data[ O ], 拷出内容保存指针
	 * @param size[ I ], 拷贝长度
	 * @return 成功操作返回__OK， 否则返回错误代码
	 */
	emErrCode get( void * data , size_t size );
	/**
	 * @brief 拷贝数据到给定内存。
	 * @note 这个操作不会消耗数据，也就是不会到存取点的调整。
	 * @param data[ O ], 拷出内容保存指针
	 * @param pos[ I ], 拷贝偏移量
	 * @param size[ I ], 拷贝长度
	 * @return
	 */
	emErrCode get( void * data , size_t pos , size_t size );
	/**
	 * @brief 清除内容。同时将其他参数都设置成默认的初始值
	 */
	void clear();
	/**
	 * @brief 重新分配内存大小。
	 * @param size[ I ], 请求内存大小
	 * @param autoZero[ I ], 内容是否清除，true清除内容
	 * @return 成功操作返回__OK, 如果内存分配失败返回ERR_ALLOC_MEM
	 */
	emErrCode resize( size_t size , bool autoZero = false );
	/**
	 * @brief 检索内容。这个函数检索出来的位置是相对于取数据起始位置的。
	 * @param data[ I ], 要检索的内容
	 * @param len[ I ], 检索内容长度
	 * @return 成功操作返回位置偏移量
	 * @exceptions 如果操作错误或者找不到数据则抛出相对应的emErrCode数据
	 */
	size_t index( const void * data , size_t len );
	/**
	 * @brief 获取当前数据的长度
	 * @return 返回数据的实际长度
	 */
	size_t length();
	/**
	 * @brief 放弃给定长度的数据
	 * @param len[ I ], 要放弃的数据长度。如果要放弃的数据长度大于等于实际数据长度，则
	 *    这个操作和clear相同
	 */
	void drop( size_t len );

	static std::string errMsg( emErrCode e );
};
}
#endif
