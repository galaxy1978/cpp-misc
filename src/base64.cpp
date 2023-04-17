#include <openssl/evp.h>
#include <string>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <string.h>

char* Base64Encode( const void * src , size_t s  )
{
	BIO *bmem = NULL;
	BIO *b64 = NULL;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());

	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, src , s );
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	BIO_set_close(b64, BIO_NOCLOSE);

	char *buffer = (char *)malloc(bptr->length + 1);
	memcpy( buffer , bptr->data , bptr->length );
	buffer[ bptr->length ] = 0;

	BIO_free_all(b64);
	return buffer;
}

size_t Base64Decode( const std::string& src , void * rst )
{
	size_t ret = 0;

	ret = EVP_DecodeBlock( (unsigned char *)rst , (const unsigned char*)src.c_str() , src.length() );
	
	return ret;
}
