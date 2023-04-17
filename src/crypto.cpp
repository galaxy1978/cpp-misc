#include <assert.h>
#include <stdlib.h>

#include <string>
#include <iostream>

#include "openssl/aes.h"

#include "crypto.hpp"
#include "misc.hpp"

std::string encrypto( const uint8_t * data , size_t len , const char * key )
{
	assert( data );
	assert( key );

	std::string ret;
	AES_KEY aes;
	char tmpIV[] = "2624750004598718";
	uint8_t iv[ AES_BLOCK_SIZE ];

	for( int i = 0; i < 16; i ++ ){
		iv[ i ] = tmpIV[ i ];
	}

	int rst = AES_set_encrypt_key( (uint8_t*)key , 128 , &aes );
	if( rst < 0 ){
		ERROR_MSG( " set aes key fail." );
		throw -1;
	}

	uint8_t * __dat = ( uint8_t* )malloc( len * 2 + 1);
	if( __dat == nullptr ){
		ERROR_MSG( " Allocate memory fail."  );

		throw -2;
	}

	// 执行加密
	AES_cbc_encrypt( data  , __dat , len , &aes , iv , AES_ENCRYPT );
	char * __rst_data = ( char *)malloc( len * 2 + 1);
	if( __rst_data == nullptr ){
		ERROR_MSG( " Allocate memory fail."  );
		free( __dat );
		__dat = nullptr;
		throw -2;
	}
	for( size_t i = 0; i < len; i ++ ){
		sprintf( __rst_data + i * 2, "%02X", __dat[ i ] );
	}
	ret.assign( __rst_data );
	if( __rst_data ) free( __rst_data );
	if( __dat ) free( __dat );
	return ret;
}

std::string decrypto( const uint8_t * data , size_t len , const char * key )
{
	assert( data );
	assert( key );

	std::string ret;
	AES_KEY aes;
	char tmpIV[] = "2624750004598718";
	uint8_t iv[ AES_BLOCK_SIZE ];
	uint8_t real_data[ len / 2 ];
	for( size_t i = 0; i < len; i += 2 ){
		unsigned char __tmp_data[ 3 ];
		__tmp_data[ 0 ] = data[ i ];
		__tmp_data[ 1 ] = data[ i + 1 ];
		__tmp_data[ 2 ] = 0;
		real_data[ i / 2 ] = ( uint8_t )strtoul( (char*)__tmp_data , nullptr , 16 );
	}
	for( int i = 0; i < 16; i ++ ){
		iv[ i ] = tmpIV[ i ];
	}

	int rst = AES_set_decrypt_key( (uint8_t*)key , 128 , &aes );
	if( rst < 0 ){
		ERROR_MSG( "set aes key fail."  );
		throw -1;
	}

	uint8_t * __dat = ( uint8_t* )malloc( len / 2 + 1 );
	if( __dat == nullptr ){
		ERROR_MSG( "Allocate memory fail."  );
		throw -2;
	}
	__dat[ len ] = 0;
	AES_cbc_encrypt( real_data , __dat , len / 2 , &aes , iv , AES_DECRYPT );

	ret.assign( ( char *)__dat );
	free( __dat );
	return ret;
}
