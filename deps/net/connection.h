#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <netinet/in.h>
#include <time.h>

//TCP连接状态
enum class ConnState {
    connected,
    listen,
	accept,
	close
};

//TCP连接
class Connection {
public:
    Connection();
    ~Connection();
    Connection(const Connection&)=delete;
    Connection& operator=(const Connection&)=delete;
public:
    void SetFd(int fd);
    int GetFd();
    void SetState(ConnState st);
    ConnState GetState();
    void SetCreateTime(time_t time);
    time_t GetCreateTime();
    void SetLastAccessTime(time_t time);
    time_t GetLastAccessTime();
    void SetPeerAddr(struct sockaddr_in addr);
    struct sockaddr_in GetPeerAddr();
    
    void SetCurRecvPkgLen(int len);
    int GetCurRecvPkgLen();
    void SetCurRecvedBytes(int n);
    int GetCurRecvedBytes();
    char* GetRecvBuf();

    void SetCurSendPkgLen(int len);
    int GetCurSendPkgLen();
    void SetCurSendedBytes(int n);
    int GetCurSendedBytes();
    char* GetSendBuf();

    bool HasRoomForRecvBuf(int n);
    bool MakeRoomForRecvBuf(int n);

    bool HasRoomForSendBuf(int n);
    bool MakeRoomForSendBuf(int n);

    void Destroy();
private:
	int m_fd;	                        //描述符id
	ConnState m_state;	                //TCP连接状态
	time_t m_createTime;	            //TCP连接创建时间（utc时间）
	time_t m_lastAccessTime;	        //TCP连接上次处理时间（utc时间）
    struct sockaddr_in m_peerAddr;	    //客终端端地址

    int m_curRecvPkgLen;	            //当前需要接收的业务包的长度
	int m_curRecvedBytes;	            //当前已接收的字节数
    char* m_recvBuf;                    //接收缓冲区
    int m_recvBufRoom;                  //接收缓冲区空间大小
    
    int m_curSendPkgLen;                //当前需要发送的业务包大小
	int m_curSendedBytes;	            //当前剩下需要发送的字节数
    char* m_sendBuf;                    //发送缓冲区
    int m_sendBufRoom;                  //接收缓冲区空间大小
};

#endif
