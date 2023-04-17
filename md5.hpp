
/**
  @brief ：定义MD5码的存储结构以方便对MD5码进行大小比较，相等判断等内容。这样在对系统变量进行
  保存后利用MD5进行排序，以提高系统变量的查找速度
  @author ：宋炜
  @date ：2015-7-18 ~ 2021-7-12
  @version 1.0
 */

 /// 2021-7-12 ADD 宋炜 添加分部分计算接口
#ifndef  __MD5_DATA_HPP__
#define __MD5_DATA_HPP__

#include <string>
#include <openssl/md5.h>

using namespace std;

class CMd5
{
	static const long FATAL_NO_MEM    = -1;
	static const long NULL_POINTER    = -2;
	static const long MD5_STRING_NULL = -3;
public:
	unsigned char m_data[ 16 ];
	unsigned long long *p_data_l;
	unsigned long long *p_data_h;
	int m_error;
private:
	MD5_CTX	 	* __p_ctx;
public:
	CMd5();
	CMd5( const CMd5& b);
	// CMd5& operator=( CMd5& md5 );
	CMd5& operator=(const CMd5& b );
	CMd5( const char *data , int len );
	CMd5( const string& data );
	virtual ~CMd5();

	int LastError(){ return m_error; }
	void Caculate( const char*data , int len );
	void Caculate( const string& data );
	void SetValue( const string& data ){ Caculate( data ); }

	int update( void * data , size_t len );
	std::string finish();

	const unsigned char *GetValue()const{ return m_data; }
	int Len(){ return 16; }
	void ToString( std::string& str )const;
	string Str()const;
	int FromString( const string& md5 );

	bool operator==( const CMd5& b) const;
	bool operator>(const CMd5& b) const ;
	bool operator<(const CMd5& b)const;
private:
	int do_digest( const char *data , int len );
};

#endif
