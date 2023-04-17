#include <sstream>
#include <regex>
#include "str_template.h"
bool CStrTmpt :: isFloat( const std::string& str )
{
	bool        ret = false;
	std::regex reg(R"([+-]{0,1}\d*\.\d+$)");

	ret = std::regex_match( str , reg );
	return ret;
}

float CStrTmpt :: toFloat( const std::string& str )
{
	float ret = 0.0;
	std::regex reg(R"(^[+-]{0,1}\d*\.\d+$)");
	if( std::regex_match( str, reg ) ){
		std::stringstream ss;
		ss << str;
		ss >> ret;
	}else{
		throw ret;
	}
	return ret;
}

bool CStrTmpt :: isReal( const std::string& str )
{
	bool        ret = false;
	std::regex reg(R"([+-]{0,1}\d*\.\d+$)");

	ret = std::regex_match( str , reg );
	return ret;
}
double CStrTmpt :: toReal( const std::string& str )
{
	double ret = 0.0;
	std::regex reg(R"(^[+-]{0,1}\d*\.\d+$)");
	if( std::regex_match( str, reg ) ){
		std::stringstream ss;
		ss << str;
		ss >> ret;
	}else{
		throw ret;
	}
	return ret;
}

bool CStrTmpt :: isDigit( const std::string& str )
{
	bool ret = false;
        std::regex reg( R"(^[+-]{0,1}\d+$)");
        ret = std::regex_match( str , reg );
	return ret;
}

long CStrTmpt :: toDigit( const std::string& str )
{
	long ret;
	std::regex reg( R"(^[+-]{0,1}\d+$)" );
	if( std::regex_match( str , reg ) ){
		std::stringstream ss;
		ss << str;
		ss >> ret;
	}else{
		throw ret;
	}

	return ret;
}

bool CStrTmpt :: isHex( const std::string& str )
{
	bool ret = false;
	std::regex reg( R"(^0[Xx][0-9a-bA-B]+$)") ,
		reg1( R"(^[0-9A-Ba-b]+[hH]$)");

	ret = ( std::regex_match( str , reg ) || std::regex_match( str , reg1));

	return ret;
}
uint32_t CStrTmpt :: toHex( const std::string& str )
{
	uint32_t ret = 0;
	std::regex reg( R"(^0[Xx][0-9a-bA-B]+$)") ,
		reg1( R"(^[0-9A-Ba-b]+[hH]$)");

	if( std::regex_match( str , reg ) ){
                std::stringstream ss;
                ss << str;
                ss >> std::hex >> ret;
	}else if( std::regex_match( str , reg1)){
		std::stringstream ss;
		std::string str1 = str.substr( 0, str.length() - 1 );
		ss << "0x" << str1;
		ss >> ret;
	}else{
		throw ret;
	}

	return ret;
}

bool CStrTmpt :: isDate( const std::string& str )
{
	bool ret = false;
	std::regex reg( R"(^\d{4}-\d{2}-\d{2}$)");

	ret = std::regex_match( str , reg );

	return ret;
}
time_t CStrTmpt :: toDate( const std::string& str )
{
	time_t ret = 0;
	std::regex reg( R"(^\d{4}-\d{2}-\d{2}$)");

	if( std::regex_match( str , reg ) ){

	}else throw ret;

	return ret;
}

bool CStrTmpt :: isTime( const std::string& str )
{
	bool ret = false;

	return ret;
}
time_t CStrTmpt :: toTime( const std::string& str )
{
        time_t ret = 0;

        return ret;
}

bool CStrTmpt :: isDateTime( const std::string& str )
{
        bool ret = false;

        return ret;
}
time_t CStrTmpt :: toDateTime( const std::string& str )
{
        time_t ret = 0;

        return ret;
}
