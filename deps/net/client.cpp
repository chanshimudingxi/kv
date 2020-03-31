#include "client.h"
#include "util.h"
#include "log.h"

Client::Client(){
    m_timeout = DEFAULT_TIMEOUT;
    m_port = 0;
    m_pconn = nullptr;
}

Client::~Client(){}

void Client::SetTimeout(int timeout){
    if(timeout > 0 && timeout < DEFAULT_TIMEOUT){    
        m_timeout = timeout;
    }
}

void Client::SetAddress(std::string ip, int port){
    m_ip = ip;
    m_port = port;
}

int Client::GetFd(){
    if(nullptr != m_pconn)
    {
        return m_pconn->GetFd();
    }
    return -1;
}

bool Client::Connect(){
    if(m_pconn == nullptr){
        LOG_DEBUG("create new connection");
        m_pconn = new Connection();
    }

    if(m_pconn->GetState() != ConnState::close){
        LOG_ERROR("already connect");
        return true;
    }

    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1){
        LOG_ERROR("%s",strerror(errno));
        return false; 
    }
    m_pconn->SetFd(fd);
    m_pconn->SetCreateTime(time(NULL));
    m_pconn->SetLastAccessTime(m_pconn->GetCreateTime());
    LOG_DEBUG("fd:%d create",fd);

    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("fd:%d %s",fd,strerror(errno));
        Destroy();
        return false;
    }

    struct sockaddr_in peerAddr;
    bzero(&peerAddr,sizeof(peerAddr));
    peerAddr.sin_family=AF_INET;
    peerAddr.sin_port=htons(m_port);
    peerAddr.sin_addr.s_addr=inet_addr(m_ip.c_str());
    m_pconn->SetPeerAddr(peerAddr);

    LOG_DEBUG("fd:%d start connect",fd);
    if(connect(fd,(struct sockaddr *)(&peerAddr),sizeof(struct sockaddr))==-1){
        if (errno == EINPROGRESS){
            fd_set wset;
            struct timeval tval;
            int n;
            FD_ZERO(&wset);  
            FD_SET(fd, &wset);
            tval.tv_sec = 0;  
            tval.tv_usec = m_timeout*1000;
            if ((n = select(fd+1, NULL, &wset, NULL, &tval)) == -1) {  
                LOG_ERROR("fd:%d %s",fd,strerror(errno)); 
                Destroy();
                return false;
            }
               
            if(n == 0){
                LOG_ERROR("fd:%d connect timeout");
                Destroy();
                return false;
            }

            if (FD_ISSET(fd, &wset)){
                int error;
                socklen_t errorLen;
                if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &errorLen) == -1){
                    LOG_ERROR("fd:%d %s",fd,strerror(errno));
                    Destroy();
                    return false;
                }
                
                if(error){
                    LOG_ERROR("fd:%d %s",fd,strerror(error));
                    Destroy();
                    return false;
                }
            }
            else{
                LOG_ERROR("fd:%d connect timeout",fd);
                Destroy();
                return false;
            }
        }
        else{
            LOG_ERROR("fd:%d %s",fd,strerror(errno));
            Destroy();
            return false;
        }
    }

    m_pconn->SetState(ConnState::connected);          
    LOG_DEBUG("fd:%d connect succ",fd);
    return true;
}

bool Client::Write(){
    if(m_pconn == nullptr){
        LOG_ERROR("connection destroy");
        return false;
    }

    if(m_pconn->GetState() != ConnState::connected){
        LOG_ERROR("not connected");
        return false;
    }
    
    if(m_pconn->GetCurSendPkgLen() <= m_pconn->GetCurSendedBytes()){
        LOG_DEBUG("fd:%d no sending bytes",m_pconn->GetFd());
        return false;
    }
        
    m_pconn->SetLastAccessTime(time(NULL));

    while(true)
    {
        int n = send(m_pconn->GetFd(),m_pconn->GetSendBuf()+m_pconn->GetCurSendedBytes(),
            m_pconn->GetCurSendPkgLen()-m_pconn->GetCurSendedBytes(),0);

        if (n == -1) {
            if(errno == EAGAIN){
                LOG_DEBUG("fd:%d need send again",m_pconn->GetFd());
                break;
            }
            else{
                LOG_ERROR("fd:%d %s",m_pconn->GetFd(),strerror(errno));
                Destroy();
                return false;
            }
        }
        else{ 
            m_pconn->SetCurSendedBytes(m_pconn->GetCurSendedBytes()+n);

            if(m_pconn->GetCurSendPkgLen() > m_pconn->GetCurSendedBytes()){
                LOG_DEBUG("fd:%d pkg_len:%d cur_send_size:%d total_send_size:%d",
                    m_pconn->GetFd(),m_pconn->GetCurSendPkgLen(),n,m_pconn->GetCurSendedBytes());
            }
            //数据已经发送完毕
            else{
                LOG_DEBUG("fd:%d finish sending pkg_len:%d",m_pconn->GetFd(),m_pconn->GetCurSendPkgLen());
                break;
            }
        }
    }

    return true;
}

bool Client::SendPkg(const void* pkg,int pkgLen){
    //首先把上次没发完的继续发完
    if(m_pconn != nullptr){
        fd_set read_fds;
        fd_set write_fds;
        fd_set error_fds;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = m_timeout*1000;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&error_fds);

        int fd = GetFd();
        FD_SET(fd, &read_fds);
        FD_SET(fd, &write_fds);
        FD_SET(fd, &error_fds);
        
        int ret = select(fd+1, &read_fds, &write_fds, &error_fds, &timeout);
        if (ret == 0){
            LOG_ERROR("select timeout");
        }

        if(ret == -1){
            LOG_ERROR("select fail ret: %d. %s",ret, strerror(errno));
        }

        if(FD_ISSET(fd, &read_fds)){
            LOG_DEBUG("fd:%d read",fd);
            Read();
        }
        if(FD_ISSET(fd, &write_fds)){
            LOG_DEBUG("fd:%d write",fd);
            Write();
        }
        if(FD_ISSET(fd, &error_fds)){
            LOG_ERROR("fd:%d error",fd);
            Destroy();
        }
    }

    if(m_pconn == nullptr){
        LOG_ERROR("connection destroy");
        if(!Connect()){
            LOG_DEBUG("Connect() failed");
            return false;
        }
        else{
            LOG_DEBUG("Connect() success");
        }
    }

    if(nullptr == pkg || pkgLen <=0){
        LOG_ERROR("fd:%d no sending bytes",m_pconn->GetFd());
        return false;
    }

    //每次只能等待一个包发完，因为如果没有发完，说明内核发送缓冲区不够用.
    //没有必要再缓冲起来，因为资源已经不够，不如把这个信息反馈回业务方
    if(m_pconn->GetCurSendPkgLen() > m_pconn->GetCurSendedBytes()){
        LOG_ERROR("fd:%d sending busy",m_pconn->GetFd());
        return false;
    }
        
    if(m_pconn->GetState() != ConnState::connected){
        LOG_DEBUG("fd:%d bug",m_pconn->GetFd());
        return false;
    }
    
    if(!m_pconn->HasRoomForSendBuf(pkgLen)){
        if(!m_pconn->MakeRoomForSendBuf(pkgLen)){
            return false;
        }
    }
    m_pconn->SetCurSendPkgLen(pkgLen);
    m_pconn->SetCurSendedBytes(0);
    memcpy(m_pconn->GetSendBuf(),(char*)pkg,pkgLen);
    m_pconn->SetLastAccessTime(time(NULL));
    
    while(true)
    {
        int n = send(m_pconn->GetFd(),m_pconn->GetSendBuf(),m_pconn->GetCurSendPkgLen(),0);
        if (n == -1) {
            if(errno == EAGAIN){
                LOG_DEBUG("fd:%d need send again",m_pconn->GetFd());
                break;
            }
            else{
                LOG_ERROR("fd:%d %s",m_pconn->GetFd(),strerror(errno));
                Destroy();
                return false;
            }
        }
        else{
            m_pconn->SetCurSendedBytes(m_pconn->GetCurSendedBytes()+n);
            if(m_pconn->GetCurSendPkgLen() > m_pconn->GetCurSendedBytes()){
                LOG_DEBUG("fd:%d pkg_len:%d cur_send_size:%d total_send_size:%d",
                        m_pconn->GetFd(),m_pconn->GetCurSendPkgLen(),n,m_pconn->GetCurSendedBytes());
            }
            else{
                LOG_DEBUG("fd:%d finish sending pkg_len:%d",m_pconn->GetFd(),m_pconn->GetCurSendPkgLen());
                break;
            }
        }
    }
    return true;   
}

bool Client::Read(){
    if(m_pconn == nullptr){
        LOG_ERROR("connection destroy");
        return false;
    }

    if(m_pconn->GetState() != ConnState::connected){
        LOG_ERROR("not connected");
        return false;
    }
    
    while(true){
        int n =0;
        if(m_pconn->GetCurRecvPkgLen() == 0){
            if(!m_pconn->HasRoomForRecvBuf(RecvPkgHeadLen())){
                if(!m_pconn->MakeRoomForRecvBuf(RecvPkgHeadLen())){
                    LOG_ERROR("fd:%d no more room",m_pconn->GetFd());
                    return false;
                }
            }
            LOG_DEBUG("cur_recv_size:%d",m_pconn->GetCurRecvedBytes());
            n = recv(m_pconn->GetFd(), m_pconn->GetRecvBuf()+m_pconn->GetCurRecvedBytes(), 
                RecvPkgHeadLen() - m_pconn->GetCurRecvedBytes(), 0);
        }
        else{
            if(!m_pconn->HasRoomForRecvBuf(m_pconn->GetCurRecvPkgLen())){
                if(!m_pconn->MakeRoomForRecvBuf(m_pconn->GetCurRecvPkgLen())){
                    LOG_ERROR("fd:%d no more room",m_pconn->GetFd());
                    return false;
                }
            }
            n = recv(m_pconn->GetFd(),m_pconn->GetRecvBuf()+m_pconn->GetCurRecvedBytes(),
                    m_pconn->GetCurRecvPkgLen()-m_pconn->GetCurRecvedBytes(),0);
        }
       
        if(n == -1){
            if(errno == EAGAIN){
                return true;
            }
            LOG_ERROR("fd:%d %s",m_pconn->GetFd(),strerror(errno));
            Destroy();
            return false;
        }

        if(n == 0){
            LOG_ERROR("fd:%d peer close",m_pconn->GetFd());
            Destroy();
            return false;
        }
        
        m_pconn->SetCurRecvedBytes(m_pconn->GetCurRecvedBytes() + n);
        m_pconn->SetLastAccessTime(time(NULL));
        LOG_DEBUG("%s",DumpHex(m_pconn->GetRecvBuf(),m_pconn->GetCurRecvedBytes()).c_str());

        //设置包长度
	    if (m_pconn->GetCurRecvPkgLen() == 0 && m_pconn->GetCurRecvedBytes()== RecvPkgHeadLen()) {
            int pkgLen = 0;
		    if (!HandleRecvPkgHead(m_pconn->GetRecvBuf(), m_pconn->GetCurRecvedBytes(), &pkgLen)) {
                LOG_ERROR("fd:%d set recv_pkg length failed",m_pconn->GetFd());
                Destroy();
                return false;
		    }
            m_pconn->SetCurRecvPkgLen(pkgLen);
            LOG_DEBUG("fd:%d total_recv_size:%d recv_pkg_len:%d",
                    m_pconn->GetFd(),m_pconn->GetCurRecvedBytes(),m_pconn->GetCurRecvPkgLen());
	    }   
       
        //检查包长度
	    if (m_pconn->GetCurRecvPkgLen() < 0 || m_pconn->GetCurRecvPkgLen() > RecvPkgLenMaxLimit()) {
            LOG_ERROR("fd:%d invalid recv_pkg_len:%d",m_pconn->GetFd(),m_pconn->GetCurRecvPkgLen());
            Destroy();
		    return false;
	    }

	    //接收的字节数达到设置上限但却没有设置包长度，说明包有问题
	    if (m_pconn->GetCurRecvPkgLen() == 0 && m_pconn->GetCurRecvedBytes() == RecvPkgHeadLen()) {
	        LOG_ERROR("fd:%d invalid recv_pkg");
            Destroy();
            return false;
        }

        //接收到一个完整的包
        if(m_pconn->GetCurRecvPkgLen() > 0 && m_pconn->GetCurRecvPkgLen() == m_pconn->GetCurRecvedBytes()){
            if(!HandleRecvPkg(m_pconn->GetRecvBuf(),m_pconn->GetCurRecvPkgLen())){
                LOG_ERROR("fd:%d handle pkg failed",m_pconn->GetFd());
                Destroy();
                return false;
            }
            m_pconn->SetCurRecvedBytes(0);
            m_pconn->SetCurRecvPkgLen(0);
            return true;
        }
    } 

    return true;
}

void Client::Destroy(){
    if(m_pconn != nullptr){
    
        close(m_pconn->GetFd());

        m_pconn->Destroy();

        delete m_pconn;        
        m_pconn = nullptr;
    }
}