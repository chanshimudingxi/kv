#include "connection.h"
#include <cstring>

Connection::Connection():
    m_fd(-1),m_state(ConnState::close),m_createTime(0),m_lastAccessTime(0),
    m_curRecvPkgLen(0),m_curRecvedBytes(0),m_recvBuf(nullptr),m_recvBufRoom(0),
    m_curSendPkgLen(0),m_curSendedBytes(0),m_sendBuf(nullptr),m_sendBufRoom(0)
{}

Connection::~Connection(){}

void Connection::SetFd(int fd){
    m_fd = fd;
}

int Connection::GetFd(){
    return m_fd;
}

void Connection::SetState(ConnState st){
    m_state = st;
}

ConnState Connection::GetState(){
    return m_state;
}

void Connection::SetCreateTime(time_t time){
    m_createTime = time;
}

time_t Connection::GetCreateTime(){
    return m_createTime;
}

void Connection::SetLastAccessTime(time_t time){
    m_lastAccessTime = time;
}

time_t Connection::GetLastAccessTime(){
    return m_lastAccessTime;
}

void Connection::SetPeerAddr(struct sockaddr_in addr){
    m_peerAddr = addr;
}

struct sockaddr_in Connection::GetPeerAddr(){
    return m_peerAddr;
}
    
void Connection::SetCurRecvPkgLen(int len){
    m_curRecvPkgLen = len;
}

int Connection::GetCurRecvPkgLen(){
    return m_curRecvPkgLen;
}

void Connection::SetCurRecvedBytes(int n){
    m_curRecvedBytes = n;
}

int Connection::GetCurRecvedBytes(){
    return m_curRecvedBytes;
}

char* Connection::GetRecvBuf(){
    return m_recvBuf;
}

bool Connection::HasRoomForRecvBuf(int n){
    if(m_recvBufRoom >= n){
        return true;
    }
    return false;
}

//原来的数据要进行拷贝(内存扩展)
bool Connection::MakeRoomForRecvBuf(int n){
    if(n <= m_recvBufRoom && m_recvBufRoom <= 10*n){
        return true;
    }

    if(nullptr == m_recvBuf){
        m_recvBuf = new char[n];
        if(nullptr == m_recvBuf){
            return false;
        }
    }
    else{
        char* tmp = new char[n];
        if(nullptr == tmp){
            return false;
        }
        memcpy(tmp,m_recvBuf,m_curRecvedBytes);
        delete [] m_recvBuf;
        m_recvBuf = tmp;
    }
    m_recvBufRoom = n;
    return true;
}

void Connection::SetCurSendPkgLen(int len){
    m_curSendPkgLen = len;
}

int Connection::GetCurSendPkgLen(){
    return m_curSendPkgLen;
}

void Connection::SetCurSendedBytes(int n){
    m_curSendedBytes = n;
}

int Connection::GetCurSendedBytes(){
    return m_curSendedBytes;
}

char* Connection::GetSendBuf(){
    return m_sendBuf;
}

bool Connection::HasRoomForSendBuf(int n){
    if(m_sendBufRoom >= n){
        return true;
    }
    return false;
}

//原来的数据不要进行拷贝(内存申请)
bool Connection::MakeRoomForSendBuf(int n){
    if(n <= m_sendBufRoom && m_sendBufRoom <= 2*n){
        return true;
    }

    if(nullptr == m_sendBuf){
        m_sendBuf = new char[n];
        if(nullptr == m_sendBuf){
            return false;
        }
    }
    else{
        char* tmp = new char[n];
        if(nullptr == tmp){
            return false;
        }
        delete [] m_sendBuf;
        m_sendBuf = tmp;
    }
    m_sendBufRoom = n;
    return true;
}

void Connection::Destroy(){
    m_fd = -1;
    m_state = ConnState::close;
    m_createTime = 0;
    m_lastAccessTime = 0;

    if(nullptr != m_recvBuf){
        delete [] m_recvBuf;
        m_recvBuf = nullptr;
    }
    m_recvBufRoom = 0;
    m_curRecvedBytes = 0;
    m_curRecvPkgLen = 0;

    if(nullptr != m_sendBuf){
        delete [] m_sendBuf;
        m_sendBuf = nullptr;
    }
    m_sendBufRoom = 0;
    m_curSendPkgLen = 0;
    m_curSendedBytes = 0;
}
