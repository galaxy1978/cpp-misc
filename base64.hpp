/**
 * @brief base64编解码
 * @version 1.0
 * @date 2018-6-23
 * @author 宋炜
*/

#ifndef __BASE64_HPP__
#define __BASE64_HPP__
/**
 * @brief base64编码
 * @param src, 要编码的数据
 * @param s , 数据长度
 * @param rst, 编码结果
 * @return
 * @exception
 */
void Base64Encode( const void *src , size_t s , std::string& rst );
/**
 * @brief base64解码
 * @param src , 要解码的数据
 * @param rst , 解码结果
 * @retrun 结果数据的长度
 */
size_t Base64Decode( const std::string& src , void* rst );
#endif // __BASE64_HPP__
