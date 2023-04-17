#if defined( WIN32 ) || defined( __WIN32 ) || defined( __WIN64 ) 
#	include <windows.h>
#	include <io.h>
#	include <direct.h>
#endif
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <list>
#include <regex>

#include "fs.hpp"
#include "tree.hpp"
#include "ary_str.hpp"
#include "misc.hpp"

bool fs :: dirEmpty( const std::string& path )
{
	bool ret = true;
	if( path.empty() ){
		ERROR_MSG( "目录名称为空" );
		throw -1;
	}

	if( isDir( path ) == false ){
		ERROR_MSG( path + "不是文件目录" );
		throw -3;
	}
	DIR * dirp = nullptr;

	dirp = opendir( path.c_str() );
	if( dirp != nullptr ){
		dirent * dp = nullptr;
		do{
			dp = readdir( dirp );
			if( dp != nullptr ){
				std::string name( dp->d_name );
				if( name == "." || name == ".." ){
					continue;
				}
				if( dp != nullptr ){
					ret = false;
					closedir( dirp );
					break;
				}
			}
		}while( dp != nullptr );
	}else{
		ERROR_MSG( std::string("找不到指定目录： " ) + path );
		throw -2;
	}

	return ret;
}

bool fs :: copy( const std::string& from , const std::string& to )
{
	bool ret = false;
#if defined( WIN32 ) || defined( __WIN32 ) || defined( __MINGW32__ ) || defined( __MINGW64__ ) || defined( __WIN64 ) || defined( __WIN__ ) || defined( __WXMSW__ )
	ret = CopyFileA( from.c_str() , to.c_str() , FALSE );
#else
	std::string cmd("cp ");
	cmd += from + " " + to;
	int rst = system( cmd.c_str() );
	if( rst != 0 ){
		ERROR_MSG("拷贝文件失败");
	}else{
		ret = true;
	}
#endif
	return ret;
}

bool fs :: isDir( const std::string& path )
{
	bool ret = false;
	if( access( path.c_str() , 0 ) != 0 ){ return ret;}
	struct stat buf;
	stat( path.c_str(), &buf );
#if defined( WIN32 ) || defined( __WIN32 ) || defined( __MINGW32__ ) || defined( __MINGW64__ ) || defined( __WIN64 ) || defined( __WIN__ ) || defined( __WXMSW__ )
	if(_S_IFDIR & buf.st_mode){
		ret = true;
	}
#elif defined( __LINUX__ )
	if(S_ISDIR( buf.st_mode )){
		ret = true;
	}
#endif // defined
	return ret;
}

bool fs :: isFile( const std::string& path )
{
	bool ret = false;
	if( access( path.c_str() , 0 ) != 0 ){ return ret;}
	struct stat buf;
	stat( path.c_str(), &buf );
#if defined( WIN32 ) || defined( __WIN32 ) || defined( __WIN64 )
	if(_S_IFREG & buf.st_mode){
		ret = true;
	}
#elif defined( __LINUX__ )
	if(S_ISREG( buf.st_mode )){
		ret = true;
	}
#endif // defined
	return ret;
}

fs :: emErrCode
fs :: enumFile( const std::string& path , fileTree& tree )
{
	emErrCode ret = OK;
	if( path.empty() ){ERROR_MSG( "没有给出根路径" );return ERR_ROOT_EMPTY;}
	if( isDir( path) == false ){ERROR_MSG( "给定路径不是目录" );return ERR_PATH_NOT_DIR;}

	DIR * dirp = opendir( path.c_str() );		// 要枚举的根目录

	std::list< DIR * > dirList;			// 目录对象栈
	dirList.push_back( dirp );
	std::list<fileTree::treeNode*> treeStack;	// 树结构干节点栈
	std::list<std::string>	dirs;			// 目录名称栈

	fileTree::treeNode* root = tree.getRootPtr();

	if( dirp != nullptr ){
		dirent * dp = nullptr;
		/// 循环遍历目录，遇到目录则下钻枚举；遇到空则上退枚举
		do{
			dp = readdir( dirp );
			if( dp != nullptr ){
				std::string f_n( dp->d_name ), p;
				if( f_n == "." || f_n==".."){ // 放弃掉 "."和".."
					continue;
				}
				// 组和目录表形成合法的路径
				for( auto i : dirs ){
					p += "\\" + i;
				}
				p = path + p + "\\" + f_n;
				MSG( p , TNORMAL );
				// 获取文件的属性参数
				stFileItem item;
				stat( p.c_str() , &item.m_stat );
				item.m_name = f_n;
				// 记录到树形表中
				tree.insert( root , item );
				// 针对目录调整遍历路径
				if( isDir( p.c_str() ) == true ){
					dirs.push_back( f_n );
					dirList.push_back( dirp );
					treeStack.push_back( root );
					root = tree.current();
					// 准备执行下钻的目录内容遍历
					dirp = opendir( p.c_str() );
					if( dirp == nullptr ){
						ERROR_MSG( "打开目录失败" );
						ret = ERR_OPEN_DIR;
						return ret;
					}
					continue;
				}
			}else{ // 上退处理
				closedir( dirp );
				dirp = dirList.back();
				root = treeStack.back();
				if( dirs.size() > 0 )
					dirs.pop_back();
				if( treeStack.size() > 0 )
					treeStack.pop_back();
				if( dirList.size() > 0 )
					dirList.pop_back();
			}
		}while( dp != nullptr || dirList.size() > 0 );
	}else{
		ERROR_MSG( "打开目录失败" );
		ret = ERR_OPEN_DIR;
	}
	return ret;
}

void fs :: enumFileAsync( const std::string& path , std::function< void ( emErrCode e , fileTree & ) > fun )
{
	std::thread thd([&]{
	        auto cb = fun;
		fileTree t;
		auto e = enumFile( path , t );
		if( cb ){
			cb( e , t );
		}
	});

	thd.detach();
}

fs :: emErrCode
fs :: removeFile( const std::string& __path , const std::string& files , bool re )
{
	emErrCode ret = OK;
	std::string path = __path;
	if( path.empty() ){ERROR_MSG( "没有给出根路径" );return ERR_ROOT_EMPTY;}
	if( files.empty() ){ERROR_MSG( "没有给出匹配条件" );return ERR_MISSING_TMPT;}
	if( isDir( path) == false ){ERROR_MSG( "给定路径不是目录" );return ERR_PATH_NOT_DIR;}
	
	DIR * dirp = opendir( path.c_str() );

	std::list< std::string>  pathStack;
	std::list< DIR * > dirList;
	dirList.push_back( dirp );
	if( dirp != nullptr ){
        if( path.back() != '/' && path.back() != '\\'){
              path += PATH_DILMTER;
        }
		std::regex reg( files );
		dirent * dp = nullptr;
		do{
			dp = readdir( dirp );
			if( dp != nullptr ){
				std::string fn( dp->d_name ), p;
				if( fn == "." || fn == ".." ) continue;
				for( auto i : pathStack ){
					p = p + PATH_DILMTER + i;
				}
                if( p.find( PATH_DILMTER ) == 0 ){
                    p = p.substr( 1 );
                }
				if( p.empty() == false ){
					p = path + p + PATH_DILMTER + fn;
				}else{
					p = path + p + fn;
				}
				// 判断是否是目录，如果是目录则将原来的目录结构加入到栈中
				// 否则判断文件是否匹配给出的正则表达式，删除掉和正则表达式匹配的文件
				if( isDir( p ) && re == true ){
					dirList.push_back(dirp);
					pathStack.push_back( dp->d_name );
					dirp = opendir( p.c_str() );
				}else if( std::regex_search( dp->d_name , reg ) == true ){
					int rst = remove( p.c_str() );
					if( rst != 0 ){
						ERROR_MSG( std::string( "删除" ) + p + "失败" );
					}else{
						MSG(std::string("删除") + p +  "成功", TNORMAL );
					}
				}
			}else{// 如果当前目录下的文件已经全部
				if( pathStack.size() > 0 && std::regex_search( pathStack.back() , reg ) == true ){
					// 如果目录匹配要求的条件，则将目录也删除掉
					std::string p;
					for( auto i : pathStack ){
                        p = p + PATH_DILMTER + i;
					}
                    if( p.find( PATH_DILMTER ) == 0 ){
                        p = p.substr( 1 );
                    }
					p = path + p;
#if defined( _MSC_VER )
					int rst = _rmdir(p.c_str());
#else
					int rst = rmdir( p.c_str() );
#endif
					if( rst != 0 ){
						ERROR_MSG( std::string( "删除" ) + p + "失败" );
					}else{
						MSG(std::string("删除") + p +  "成功", TNORMAL );
					}
				}
				closedir( dirp );
				dirp = dirList.back();
				if( pathStack.size() > 0 )
					pathStack.pop_back();
				dirList.pop_back();
			}
		}while( dp != nullptr || dirList.size() != 0 );
	}else{
		ERROR_MSG( "打开目录失败" );
		ret = ERR_OPEN_DIR;
	}
	
	return ret;
}

void fs :: removeFileAsync( const std::string& path , const std::string& reg , std::function< void ( emErrCode ) > fun , bool re )
{
	std::thread thd(
	[&]{
		auto cb = fun;
		auto e = removeFile( path , reg , re );

		if( cb ){
			fun( e );
		}
	});

	thd.detach();
}

fs :: emErrCode
fs :: for_each( const std::string& path , std::function< bool ( emErrCode , const stFileItem& ) > fun )
{
	emErrCode ret = OK;
	fileTree tre;
	ret = enumFile( path , tre );
	if( ret == OK ){
		auto rst = tre.for_each( tre.getRootPtr() , [&](fileTree :: treeNode * node , bool , bool bkp)->bool{
		       bool ret = true;
		       if( node == tre.getRootPtr() ) return ret;
		       if( !bkp ){
			       ret = fun( OK , node->value() );
		       }
		       return ret;
		});

		if( rst!= fileTree::OK ){
			ret = ERR_ITER_DIR;
		}
	}
	return ret;
}
