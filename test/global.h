#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/* 充许在特定fd上请求连接的客户队列数 */
#define MAX_LISTEN_QUEUE 100001

/* max buf len */
#define MAX_BUF_LEN 1024

/* eh max MTU 1500 */
#define ETH_MAX_MTU 1500

/* MIN_USER_PWD_LEN */
#define MIN_USER_PWD_LEN 6

/* LOGIN */
#define LOGIN 0x0001

/* REGISTER */
#define REGISTER 0x0000

//void bzero(void *base, unsigned int size);

void m_tolower(char *str);

void thread_wait(const int sec);

#endif /* __GLOBAL_H__ */
