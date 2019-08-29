
#ifndef tcpserver__h
#define tcpserver__h

#ifdef __cplusplus
extern "C" {
#endif
#define SERVERPORT  8890
#define QUEUE_SIZE   10
#define BUFFER_SIZE 2048
typedef struct {
    void *dispatch_thread;
} tcp_ctx_t;
void *start_socket_server();

#ifdef __cplusplus
}
#endif

#endif
