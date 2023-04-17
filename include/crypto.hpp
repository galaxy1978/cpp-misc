/**
 * @brief  加密解密的处理模块
 */

#ifndef CRYPTO_HPP
#define CRYPTO_HPP
#include <string>

/**
 * @brief AES 128加密算法。CBC
 * @param data[ I ], 要加密的数据
 * @param len[ I ], 要加密的数据长度
 * @param key[ I ], 加密密码，长度应该为16个字节
 * @return 加密后数据以16进制字符串的方式返回
 */
std::string encrypto( const uint8_t * data , size_t len , const char * key );
/**
 * @brief AES 128解密算法,CBC
 */
std::string decrypto( const uint8_t * data , size_t len , const char * key );
#endif // CRYPTO_HPP
