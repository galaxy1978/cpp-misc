#include <sstream>
#include <regex>

#include "url.hpp"
#include "ary_str.hpp"
#include "misc.hpp"

URL::urlDict
urlEncodeDict[]={
                 { 0x00 , true },
                 { 0x01 , true },
                 { 0x02 , true },
                 { 0x03 , true },
                 { 0x04 , true },
		 { 0x05 , true },
                 { 0x06 , true },
                 { 0x06 , true },
		 { 0x07 , true },
                 { 0x08 , true },
                 { 0x09 , true },
                 { 0x0a , true },
                 { 0x0b , true },
                 { 0x0c , true },
                 { 0x0d , true },
                 { 0x0e , true },
                 { 0x0f , true },
                 { 0x10 , true },
                 { 0x11 , true },
                 { 0x12 , true },
                 { 0x13 , true },
                 { 0x14 , true },
                 { 0x15 , true },
                 { 0x16 , true },
                 { 0x17 , true },
                 { 0x18 , true },
                 { 0x19 , true },
                 { 0x20 , true },
                 { 0x21 , true },
                 { 0x22 , true },
                 { 0x23 , true },
                 { 0x24 , true },
                 { 0x25 , true },
                 { 0x26 , true },
                 { 0x27 , true },
                 { 0x28 , true },
                 { 0x29 , true },
                 { 0x30 , true },
                 { 0x31 , true },
                 { ' ' , true },
                 { '!' , true },
                 { '#' , true },
                 { '$' , true },
                 { '%' , false },
                 { '&' , false },
                 { '\'' , true },
                 { '(' , false },
                 { ')' , false },
                 { '*' , false },
                 { '+' , true },
                 { ',' , true },
                 { '-' , false },
                 { '.' , false },
                 { '/' , false },
                 { '0' , false },
                 { '1' , false },
                 { '2' , false },
                 { '3' , false },
                 { '4' , false },
                 { '5' , false },
                 { '6' , false },
                 { '7' , false },
                 { '8' , false },
                 { '9' , false },
                 { ':' , false },
                 { ';' , true },
                 { '<' , true },
                 { '=' , false },
                 { '>' , true },
                 { '?' , true },
                 { '@' , true },
                 { 'A' , false },
                 { 'B' , false },
                 { 'C' , false },
                 { 'D' , false },
                 { 'E' , false },
                 { 'F' , false },
                 { 'G' , false },
                 { 'H' , false },
                 { 'I' , false },
                 { 'J' , false },
                 { 'K' , false },
                 { 'L' , false },
                 { 'M' , false },
                 { 'N' , false },
                 { 'O' , false },
                 { 'P' , false },
                 { 'Q' , false },
                 { 'R' , false },
                 { 'S' , false },
                 { 'T' , false },
                 { 'U' , false },
                 { 'V' , false },
                 { 'W' , false },
                 { 'X' , false },
                 { 'Y' , false },
                 { 'Z' , false },
                 { '[' , true },
                 { '\\' , true },
                 { ']' , true },
                 { '^' , true },
                 { '_' , false },
                 { '`' , false },
                 { 'a' , false },
                 { 'b' , false },
                 { 'c' , false },
                 { 'd' , false },
                 { 'e' , false },
                 { 'f' , false },
                 { 'g' , false },
                 { 'h' , false },
                 { 'i' , false },
                 { 'j' , false },
                 { 'k' , false },
                 { 'l' , false },
                 { 'm' , false },
                 { 'n' , false },
                 { 'o' , false },
                 { 'p' , false },
                 { 'q' , false },
                 { 'r' , false },
                 { 's' , false },
                 { 't' , false },
                 { 'u' , false },
                 { 'v' , false },
                 { 'w' , false },
                 { 'x' , false },
                 { 'y' , false },
                 { 'z' , false },
                 { '{' , true },
                 { '|' , true },
                 { '}' , true },
                 { '~' , true },
                 { 0x7f , true }
};

URL ::
Param ::Param()
{

}

URL ::
Param ::Param( const std::string& query )
{
	bool rst = parse( query );
	if( rst == false ){
		throw URL :: ERR_PARSE_QUERY;
	}
}

URL :: Param ::Param( const Param& b )
{
	m_dict = b.m_dict;
}

URL :: Param :: Param( Param&& b )
{
	m_dict = std::move( b.m_dict );
}

URL :: Param :: ~Param(){}

URL ::Param&
URL :: Param ::operator=( const Param& b )
{
	m_dict = b.m_dict;
	return *this;
}
URL ::Param&
URL :: Param :: operator=( Param&& b )
{
	m_dict = std::move( b.m_dict );
	return *this;
}

bool URL ::
Param :: parse( const std::string& query )
{
	bool ret = false;
	if( query.empty() ) return false;

	std::string str = query;
	std::regex reg( "(\\w|%[0-9A-Fa-f]{2,4}|-|_)+=(\\w|%[0-9A-Fa-f]{2,4}|-|_)+" );

	std::smatch  sm;
	int count = 0;

	while( std::regex_search( str , sm , reg ) == true ){
		count ++;

		std::string str1 = sm[ 0 ], key , param;
		key = std::move( str1.substr( 0 , str1.find( "=" ) ) );
		param = std::move( str1.substr( str1.find( "=" ) + 1 ) );

		m_dict.insert( std::pair< std::string , std::string>( key , param ) );

		str = std::move( std::regex_replace( str , reg , "$1" ) );
	}
	ret = ( count > 0 );
	return ret;
}

const std::string URL ::
Param ::operator[ ]( const std::string& name )
{
	std::string ret;
	if( name.empty() ) throw URL :: ERR_EMPTY_PARAM_KEY;
	if( m_dict.find( name ) == m_dict.end() ) throw URL :: ERR_KEY_NOT_EXIST;

	ret = m_dict[ name ];

	return ret;
}

size_t URL ::
Param ::size()
{
    	return m_dict.size();
}
////////////////////////////////////////////////////////////////////////////////////////////

URL :: URL( const URL& b ):
	m_url( b.m_url ),
	m_protocol( b.m_protocol ),
	m_user( b.m_user ),
	m_passwd( b.m_passwd ),
	m_host( b.m_host ),
	m_port( b.m_port ),
	m_query( b.m_query ),
	m_path( b.m_path ),
	m_hash( b.m_hash ),
	m_param( b.m_param ){}

URL :: URL( URL&& b ):
	m_url( std::move( b.m_url )),
	m_protocol( std::move(b.m_protocol )),
	m_user( std::move(b.m_user )),
	m_passwd( std::move(b.m_passwd )),
	m_host( std::move(b.m_host )),
	m_port( std::move(b.m_port )),
	m_query( std::move(b.m_query )),
	m_path( std::move(b.m_path )),
	m_hash( std::move(b.m_hash )),
	m_param( std::move(b.m_param ) ){}

URL& URL :: operator=( const URL& b )
{
	m_url = b.m_url;
	m_protocol = b.m_protocol;
	m_user = b.m_user;
	m_passwd = b.m_passwd;
	m_host = b.m_host;
	m_port = b.m_port;
	m_query = b.m_query;
	m_path = b.m_path;
	m_hash = b.m_hash;
	m_param = b.m_param;

	return *this;
}
URL& URL :: operator=( URL&& b )
{
	m_url = std::move( b.m_url );
	m_protocol = std::move(b.m_protocol );
	m_user = std::move(b.m_user );
	m_passwd = std::move(b.m_passwd );
	m_host = std::move(b.m_host );
	m_port = std::move(b.m_port );
	m_query = std::move(b.m_query );
	m_path = std::move(b.m_path );
	m_hash = std::move(b.m_hash );
	m_param = std::move(b.m_param );

	return *this;
}

bool URL :: parse_protocol( const std::string& url )
{
	bool ret = false;
	std::regex reg( "^\\w+(?=://)" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_protocol = sm[ 0 ];
		ret = true;
	}

	return ret;
}

bool URL :: parse_user( const std::string& url )
{
	bool ret = false;
	std::regex reg( "//\\w+(?=[:@])" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_user = sm[ 0 ];
		m_user = m_user.substr( 2 );
		ret = true;
	}

	return ret;
}

bool URL :: parse_password( const std::string& url )
{
	bool ret = false;
	std::regex reg( R"(\w+:\w+(?=@))" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_passwd = sm[ 0 ];
		m_passwd = m_passwd.substr( m_passwd.find( ':') + 1 );
		ret = true;
	}

	return ret;
}

bool URL :: parse_host( const std::string& url )
{
	bool ret = false;

	std::regex reg( R"((?://)(.*@){0,1}(\w|\.|_|-)+)" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_host = sm[ 0 ];
		size_t pos = m_host.find( '@' );
		if( pos != std::string::npos ){
			m_host = m_host.substr( pos + 1 );
		}else{
			m_host = m_host.substr( 2 );
		}
		ret = true;
	}

	return ret;
}

bool URL :: parse_port( const std::string& url )
{
	bool ret = false;

	std::regex reg( R"(:\w+(?=/|$))" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_port = sm[ 0 ];
		m_port = m_port.substr( 1 );
		ret = true;
	}else{
		m_port = "-1";
	}

	return ret;
}

bool URL :: parse_query( const std::string& url )
{
	bool ret = false;

	std::regex reg( "\\?(\\w|%|_)+=(\\w|%|-|_)+(&(\\w|%|_)+=(\\w|\%|-|_)+)*");
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_query = sm[ 0 ];
		m_query = m_query.substr( 1 );
		ret = m_param( m_query );
		if( !ret ){
		m_param.clear();
		m_query.clear();
		}
	}else{
		m_param.clear();
		m_query.clear();
	}

	return ret;
}

bool URL :: parse_path( const std::string& url )
{
	bool ret = true;

	std::regex reg( "\\w/(\\w|_|\%)*(/(\\w|_|%)+)*(\\.(\\w|_|%)+)?/?(?=[#?]|$)");
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_path = sm[ 0 ];
		m_path = m_path.substr( 1 );
		ret = true;

	}
	return ret;
}

bool URL :: parse_hash( const std::string& url )
{
	bool ret = false;
	std::regex reg( R"(#(\w|_|\.)+)" );
	std::smatch sm;

	if( std::regex_search( url , sm , reg ) == true ){
		m_hash = sm[ 0 ];
		m_hash = m_hash.substr( 1 );
		ret = true;

	}else m_hash.clear();
	return ret;
}

URL :: URL():m_port( "-1" ){}

URL :: URL( const std::string& url )
{
	m_url.clear();
	if( url.empty() ) throw ERR_EMPTY_URL;
	m_url = url;
	parse_protocol( url );
	parse_user( url );
	parse_password( url );
	parse_host( url );
	parse_port( url );
	parse_path( url );
	parse_query( url );
	parse_hash( url );
}

URL ::  ~URL(){}

const std::string URL :: Protocol() const
{
    	return m_protocol;
}

void URL :: Protocol( const std::string& p )

{
	m_protocol = p;
}
const std::string URL :: AuthUser() const
{
    	return m_user;
}

void URL :: AuthUser( const std::string& user )
{
    	m_user = user;
}

const std::string URL :: AuthPswd() const
{
    	return m_passwd;
}

void URL :: AuthPswd( const std::string& pswd )
{
	m_passwd = pswd;
}
const std::string URL :: HostName() const
{
    	return m_host;
}

void URL :: HostName( const std::string& host )
{
        m_host = host;
}
void URL :: HostName( std::string&& host )
{
        m_host = host;
}
int URL :: HostPort()const
{
	int ret = -1;
	ret = atoi( m_port.c_str() );
    	return ret;
}

void URL :: HostPort( int port )
{
        std::stringstream ss;
        ss << port;
        m_port = ss.str();
}

const std::string URL :: Path() const
{
    	return m_path;
}

const std::string URL :: Query() const
{
    return m_query;
}

const std::string URL :: Parameters( const std::string& key )
{
	if( key.empty() ) throw ERR_EMPTY_PARAM_KEY;
	if( m_param.find( key ) == m_param.end() ) throw ERR_KEY_NOT_EXIST;

	return m_param[ key ];
}

void URL :: Params( Param& param )
{
    	param = m_param;
}

const std::string URL ::  Hash() const
{
    return m_hash;
}

std::string URL :: Encode( const std::string& from )
{
	std::string ret("");
        for( auto c : from ){
                if( c < 0 ){
                        unsigned char d = (unsigned char )c;
                        unsigned char dh =  d >> 4 , dl = d & 0x0f;
                        if( dh < 10 ) dh += 0x30;
                        else dh = ( dh - 10 ) + 0x41;

                        if( dl < 10 ) dl += 0x30;
                        else dl = ( dl - 10 ) + 0x41;
						ret += '%';
                        ret += ( char )dh;
                        ret += ( char )dl;
                }else{
                        int idx = ( int )c;
                        if( urlEncodeDict[ idx ].m_en == true ){
                                unsigned char d = (unsigned char )c;
                                unsigned char dh =  ( d >> 4 ) & 0xf , dl = d & 0x0f;
                                if( dh < 10 ) dh += 0x30;
                                else dh = ( dh - 10 ) + 0x41;
                                
                                if( dl < 10 ) dl += 0x30;
                                else dl = ( dl - 10 ) + 0x41;
								ret += '%';
                                ret += ( char )dh;
                                ret += ( char )dl;
                        }else{
                                ret += c;
                        }
                }
        }
	return ret;
}

std::string URL :: Decode( const std::string& from )
{
	std::string ret("");

        for( size_t i = 0; i < from.length(); i ++ ){
                if( from[ i ] != '%' ){
                        ret += from[ i ];
                }else{
                        std::string str = std::move( from.substr( i + 1 , 2 ) );
                        i += 3;

                        char c ,c1;
                        if( str[ 0 ] >= 'A' ){
                                c = str[ 0 ] - 0x41 + 10;
                                c <<= 4;
                        }else{
                                c = str[ 0 ] - 0x30;
                        }

                        if( str[ 1 ] >= 'A' ){
                                c1 = str[ 1 ] - 0x41 + 10;
                                c |= c1;
                        }else{
                                c1 = str[ 1 ] - 0x30;
                                c |= c1; 
                        }

                        ret += c;
                }
        }
	return ret;
}

bool URL :: Parse( const std::string& url )
{
	if( url.empty() ) return false;

	bool ret = parse_protocol( url );
	parse_user( url );
	parse_password( url );
	bool ret1 = parse_host( url );
	parse_port( url );
	bool ret2 = parse_path( url );
	parse_query( url );
	parse_hash( url );

	return ret && ret1 && ret2;
}

std::string URL :: toString()
{
	std::string ret;
	//protocol://[user:password@]hostname[:port]/website/path/[file][?query][#fragment]
	if( m_url.empty() ){
		std::string p = m_protocol;
		std::transform( p.begin(), p.end() , p.begin() , ::tolower );
		switch( hash_( p.c_str() ) ){
		case "ftp"_hash:
		case "ftps"_hash:
		case "http"_hash:
		case "https"_hash:
			m_url = p + "://";
			break;
		case "file"_hash:
			m_url = p + ":///";
			break;

		}

		m_url += m_host;
		if( ! m_port.empty() ) m_url += ":" + m_port;
		if( m_path.at( 0 ) != '/' )
			m_url += "/" + m_path;
		else m_url += m_path;
		if( m_query.empty() == false )
			m_url += "?" + m_query;
		if( m_hash.empty() == false )
			m_url += "#" + m_hash;

	}
	ret = m_url;

	return ret;
}

const std::string URL :: File() const
{
	std::string ret;
	size_t pos = m_path.rfind( '/' );
	if( pos != std::string::npos ){
		ret = m_path.substr( pos + 1 );
	}

	return ret;
}
