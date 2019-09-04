#ifndef SMP_MD5_H
#define SMP_MD5_H
typedef unsigned int md5_word_t;
typedef unsigned char md5_byte_t;
typedef struct md5_state_s
{
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
}md5_state_t;

//  用md5算法取散列值
//  参数:	szsour 源字符串
//			ilen   源字符串长度
//			szdest 目的串(16字节)
void md5(md5_byte_t *szsour, int ilen, md5_byte_t *szdest);
void md5_str(md5_byte_t *szsour, int ilen, md5_byte_t *szdest);

#endif //SMP_MD5_H
