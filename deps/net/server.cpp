#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <time.h>
#include "log.h"

Server::Server(int maxFdCount){
    m_maxFdCount = maxFdCount;
    m_timeout = 0;
    m_listenBacklog = DEFAULT_LISTEN_BACKLOG;
    m_maxLoopWaitTime = DEFAULT_LOOP_WAIT_TIME;
    m_listenPort = DEFAULT_LISTEN_PORT;

    m_connContainer = NULL;

    m_epfd = 0;
    m_events = NULL;
}

Server::~Server(){}

Connection* Server::GetConnection(int fd){
    return m_connContainer->Get(fd);
}

void Server::SetTimeout(int timeout){
    m_timeout = timeout>0 ? timeout :0;
}

void Server::SetListenBacklog(int backlog){
    m_listenBacklog = backlog; 
}

void Server::SetListenPort(int port){
    m_listenPort = port;
}

bool Server::Init() {
    //设置程序能够处理的最大描述符个数
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
        LOG_ERROR("%s",strerror(errno));
        return false;
    }
	rlim.rlim_max = rlim.rlim_cur = m_maxFdCount;
	if (setrlimit(RLIMIT_NOFILE, &rlim) == -1) {
		LOG_ERROR("%s",strerror(errno));
        return false;
	}

    //初始化连接容器
    m_connContainer = new ConnectionContainer(m_maxFdCount);
    if(m_connContainer == NULL){
        return false;
    }

	//系统多路复用描述符初始化
	m_events = new struct epoll_event[m_maxFdCount];
	m_epfd = epoll_create(m_maxFdCount);
	if (m_epfd == -1) {
        LOG_ERROR("%s",strerror(errno));
		return false;
	}
    
    //监听
    if(!Listen()){
        return false;
    }
    return true;
}

bool Server::Listen() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
        LOG_ERROR("%s", strerror(errno));
		return false;
	}

	int flags = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		LOG_ERROR("%s", strerror(errno));
        return false;
	}

    int reuse = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		LOG_ERROR("%s",strerror(errno));
        return false;
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_listenPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		LOG_ERROR("%s",strerror(errno));
        return false;
	}

	if (listen(fd, m_listenBacklog) == -1) {
		LOG_ERROR("%s",strerror(errno));
        return false;
	}

	struct epoll_event event;
	event.events = EPOLLIN;//错误事件会自动加上
	event.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event) == -1) {
		LOG_ERROR("%s",strerror(errno));
        return false;
	}

	Connection *c = new Connection();
    c->SetFd(fd);
    c->SetCreateTime(time(NULL));
    c->SetLastAccessTime(c->GetCreateTime());
    c->SetState(ConnState::listen);
	if (!m_connContainer->Set(fd, c)) {
		LOG_ERROR("container set fd:%d failed",fd);
        return false;
	}

    LOG_DEBUG("port:%d listen_fd:%d container_size:%d",m_listenPort,fd,m_connContainer->Size());
    return true;
}

//能够正常处理就返回true
bool Server::Accept(int fd) {
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	int addrLen = sizeof(addr);
	int afd = accept(fd, (struct sockaddr*)(&addr),(socklen_t*)&addrLen);
	if (-1 == afd) {
        LOG_ERROR("listen_fd:%d %s", fd, strerror(errno));
		Destroy(afd);
        return false;
	}

	int flags = fcntl(afd, F_GETFL, 0);
	if (fcntl(afd, F_SETFL, flags | O_NONBLOCK) == -1) {
		LOG_ERROR("listen_fd:%d accept_fd:%d %s",fd, afd, strerror(errno));
        Destroy(afd);
        return true;
	}

	Connection *c = new Connection();
    c->SetFd(afd);
	c->SetPeerAddr(addr);
    c->SetCreateTime(time(NULL));
	c->SetLastAccessTime(c->GetCreateTime());
	c->SetState(ConnState::accept);
	c->SetCurRecvedBytes(0);
    c->SetCurRecvPkgLen(0);
	c->SetCurSendedBytes(0);
    c->SetCurSendPkgLen(0);
	if(!m_connContainer->Set(afd,c)){
        LOG_ERROR("listen_fd:%d accept_fd:%d container set failed",fd,afd);
        Destroy(afd);
        return true;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = afd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, afd, &event) == -1) {
        LOG_ERROR("listen_fd:%d accept_fd:%d %s",fd, afd, strerror(errno));
        Destroy(afd);
        return true;
	}

    if(!HandleAccept(c)){
        LOG_ERROR("listen_fd:%d accept_fd:%d",fd, afd);
        Destroy(afd); 
        return true;
    }

    LOG_DEBUG("listen_fd:%d accept_fd:%d container_size:%d",fd,afd,m_connContainer->Size());
	return true;
}

bool Server::Read(int fd) {
    Connection *c = m_connContainer->Get(fd);
    if(nullptr == c){
        LOG_ERROR("recv_fd:%d noexist in container",fd);
        Destroy(fd);
        return true;
    }

    //尽可能的把包读完
    while(1){
        int n = 0;
        //包含包长度信息字段的最小包大小
        if(c->GetCurRecvPkgLen() == 0)
        {
            //缓冲区大小设置为最小包大小
            if(!c->HasRoomForRecvBuf(RecvPkgHeadLen())){
                if(!c->MakeRoomForRecvBuf(RecvPkgHeadLen())){
                    LOG_ERROR("recv_fd:%d no more room",fd);
                    return true;
                }
            }
            n = recv(fd, c->GetRecvBuf() + c->GetCurRecvedBytes(), RecvPkgHeadLen() - c->GetCurRecvedBytes(), 0);
        }
        else
        {
            //缓冲区大小扩大到包的大小
            if(!c->HasRoomForRecvBuf(c->GetCurRecvPkgLen())){
                if(!c->MakeRoomForRecvBuf(c->GetCurRecvPkgLen())){
                    LOG_ERROR("recv_fd:%d no more room",fd);
                    return true;
                }
            }
            //尽量保证每次处理并且只处理一个包
            n = recv(fd, c->GetRecvBuf() + c->GetCurRecvedBytes(), c->GetCurRecvPkgLen() - c->GetCurRecvedBytes(), 0);
        }

	    if (n == -1) {
            if(errno != EAGAIN){
                LOG_ERROR("recv_fd:%d %s",fd, strerror(errno));
                Destroy(fd);
            }
            return true;
	    }

	    if (n == 0) {
            LOG_DEBUG("recv_fd:%d total_recv_size:%d recv_pkg_len:%d peer close",
                    fd,c->GetCurRecvPkgLen(),c->GetCurRecvedBytes());
            Destroy(fd);
		    return true;
	    }

	    c->SetCurRecvedBytes(c->GetCurRecvedBytes()+n);
	    c->SetLastAccessTime(time(NULL));
        LOG_DEBUG("recv_fd:%d total_recv_size:%d recv_pkg_len:%d cur_recv_size:%d",
                fd,c->GetCurRecvedBytes(),c->GetCurRecvPkgLen(),n);
	
        //设置包长度
	    if (c->GetCurRecvPkgLen() == 0 && c->GetCurRecvedBytes() >= RecvPkgHeadLen()) {
            int pkgLen=0;
		    if (!HandleRecvPkgHead(c, c->GetRecvBuf(), c->GetCurRecvedBytes(), &pkgLen)) {
                LOG_ERROR("recv_fd:%d set pkg length failed",fd);
                Destroy(fd);
			    return true;
		    }
            c->SetCurRecvPkgLen(pkgLen);
            LOG_DEBUG("recv_fd:%d total_recv_size:%d recv_pkg_len:%d",fd,c->GetCurRecvedBytes(),c->GetCurRecvPkgLen());
	    }
        //检查包长度是否在合理范围
	    if (c->GetCurRecvPkgLen() < 0 || c->GetCurRecvPkgLen() > RecvPkgLenMaxLimit()) {
            LOG_ERROR("recv_fd:%d invalid recv_pkg_len:%d",fd,c->GetCurRecvPkgLen());
            Destroy(fd);
		    return true;
	    }
        //接收的字节数达到上限但却没有设置包长度，说明包有问题
	    if (c->GetCurRecvPkgLen() == 0 && c->GetCurRecvedBytes() >= RecvPkgHeadLen()) {
            LOG_ERROR("recv_fd:%d pkg length not setted",fd);
            Destroy(fd);
		    return true;
	    }
	
        //接收到一个完整的包
	    if (c->GetCurRecvPkgLen() > 0 && c->GetCurRecvPkgLen() == c->GetCurRecvedBytes()) {
		    if (!HandleRecvPkg(c, c->GetRecvBuf(), c->GetCurRecvPkgLen())) {
                LOG_ERROR("recv_fd:%d handle pkg failed",fd);
                Destroy(fd);
			    return true;
		    }
            //重置连接接收状态
		    c->SetCurRecvedBytes(0);
            c->SetCurRecvPkgLen(0);
            return true;
        }
    }
        	
	return true;
}

bool Server::Write(int fd) {
    Connection *c = m_connContainer->Get(fd);
    if(nullptr == c){
        LOG_ERROR("send_fd:%d noexist in container",fd);
        Destroy(fd);
        return true;
    }

    if(c->GetCurSendPkgLen() <= c->GetCurSendedBytes()){
        LOG_ERROR("send_fd:%d no sending bytes",fd);
        Destroy(fd);
        return true;
    }

	c->SetLastAccessTime(time(NULL));

	int n = send(fd,c->GetSendBuf()+c->GetCurSendedBytes(),c->GetCurSendPkgLen()-c->GetCurSendedBytes(),0);
    if (n == -1) {
        if(errno == EAGAIN){
            struct epoll_event event;
	        event.events = EPOLLOUT|EPOLLIN;
	        event.data.fd = fd;
	        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
                LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
                Destroy(fd);
                return true;
	        }
        }
        else{
            LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
            Destroy(fd);
            return true;
        }
	}
    else{ 
        c->SetCurSendedBytes(c->GetCurSendedBytes()+n);

        if(c->GetCurSendPkgLen() > c->GetCurSendedBytes()){
            LOG_DEBUG("send_fd:%d pkg_len:%d cur_send_size:%d total_send_size:%d",
                fd,c->GetCurSendPkgLen(),n,c->GetCurSendedBytes());
            struct epoll_event event;
            event.events = EPOLLOUT|EPOLLIN;
            event.data.fd = fd;
            if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
                LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
                Destroy(fd);
                return true;
            }
        }
        else{
            struct epoll_event event;
            event.events = EPOLLIN; 
            event.data.fd = fd;
            if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1){
                LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
                Destroy(fd);
                return true;
            }
            LOG_DEBUG("send_fd:%d finish sending pkg_len:%d",fd,c->GetCurSendPkgLen());
        }
    }

    return true;
}

bool Server::SendPkg(int fd, const void* pkg, int pkgLen) {
    Connection *c = m_connContainer->Get(fd);
    if(nullptr == c){
        LOG_ERROR("send_fd:%d noexist in container",fd);
        Destroy(fd);
        return false;
    }

    if(nullptr == pkg || pkgLen <= 0){
        LOG_ERROR("send_fd:%d no sending bytes",fd);
        return false;
    }
   
    if(c->GetCurSendPkgLen() > c->GetCurSendedBytes()){
        LOG_ERROR("send_fd:%d sending busy",fd);
        return false;
    }

    if(!c->HasRoomForSendBuf(pkgLen)){
        if(!c->MakeRoomForSendBuf(pkgLen)){
            return false;
        }
    }
    c->SetLastAccessTime(time(NULL));
    c->SetCurSendPkgLen(pkgLen);
    c->SetCurSendedBytes(0);
    memcpy(c->GetSendBuf(),(char*)pkg,pkgLen);
  
    int n = send(fd,c->GetSendBuf(),c->GetCurSendPkgLen(),0);
	if (n == -1) {
        if(errno == EAGAIN){
            struct epoll_event event;
	        event.events = EPOLLOUT|EPOLLIN;
	        event.data.fd = fd;
	        if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
                LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
                Destroy(fd);
                return false;
	        }
        }
        else{
            LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
            Destroy(fd);
            return false;
        }
	}
    else{
        c->SetCurSendedBytes(n);

        if(c->GetCurSendPkgLen() > c->GetCurSendedBytes()){
            LOG_DEBUG("send_fd:%d pkg_len:%d cur_send_size:%d total_send_size:%d",
                    fd,c->GetCurSendPkgLen(),n,c->GetCurSendedBytes());
            struct epoll_event event;
            event.events = EPOLLOUT|EPOLLIN;
            event.data.fd = fd;
            if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event) == -1) {
                LOG_ERROR("send_fd:%d %s",fd,strerror(errno));
                Destroy(fd);
                return false;
            }
        }
        else{
            LOG_DEBUG("send_fd:%d finish sending pkg_len:%d",fd,c->GetCurSendPkgLen());
        }
    }

    return true; 
}

bool Server::Destroy(int fd) {
    Connection* c = m_connContainer->Get(fd);
    if(nullptr != c){
        //销毁连接
        c->Destroy();
        //delete new出来的对象
        delete c;
    }
    //连接容器中删除描述符
    m_connContainer->Remove(fd);
    //事件容器中删除描述符 
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);        
    //关闭描述符 
    close(fd);
    
    LOG_DEBUG("close_fd:%d, container_size:%d",fd,m_connContainer->Size());
    return true;
}

bool Server::NeedKeepRunning() {
    //事件容器里拿出所有描述符
    int ready = epoll_wait(m_epfd, m_events, m_maxFdCount, m_maxLoopWaitTime);
	if (ready == -1) {
        LOG_ERROR("%s",strerror(errno));
        return false;
	}
    //处理每个描述符
	for (int i = 0; i < ready; ++i) {
		int fd = m_events[i].data.fd;
        Connection *c = m_connContainer->Get(fd);//连接容器里获取描述符对应的连接
		//出现连接容器和事件容器里描述符不一致,出现逻辑bug
        if (nullptr == c) {
            LOG_ERROR("fd:%d noexist in container",fd);
		    return false;
        }

		if (c->GetState() == ConnState::listen) {
			if (EPOLLERR == (m_events[i].events & EPOLLERR)) {
                //监听描述符本身出错
                LOG_ERROR("listen fd error");
                return false;
			}
			else if (EPOLLIN == (m_events[i].events & EPOLLIN)) {
                if(!Accept(fd)){
                    //无法再处理新连接
                    LOG_ERROR("can't accept new connection");
                    return false;
                }
			}
            //未知事件
            else{
                LOG_ERROR("fd:%d events:%d",fd,m_events[i].events);
                return false;
            }
		}
		else if (c->GetState() == ConnState::accept) {
            if (EPOLLERR == (m_events[i].events & EPOLLERR)) {
                LOG_ERROR("fd:%d events:%d",fd,m_events[i].events);
				Destroy(fd);
			}				
			else if(EPOLLIN == (m_events[i].events & EPOLLIN) || EPOLLOUT == (m_events[i].events & EPOLLOUT)){
                if (EPOLLIN == (m_events[i].events & EPOLLIN)) {
					if(!Read(fd)){
                        //无法再读
                        LOG_ERROR("read failed");
                        return false;
                    }
                }
                if (EPOLLOUT == (m_events[i].events & EPOLLOUT)) {
					if(!Write(fd)){
                        //无法再写
                        LOG_ERROR("write failed");
                        return false;
                    }
				}
			}
            //未知事件
            else{
                LOG_ERROR("fd:%d events:%d",fd,m_events[i].events);
                Destroy(fd);
            }
		}
        //逻辑bug
		else{
            LOG_ERROR("fd:%d unknow state",fd);
            return false;
        }
	}
  
    //设置了业务层面的心跳超时检测
    if(m_timeout>0){
        static int checkFd=0; 
        time_t now = time(NULL);
        for(int i=0;i<1000&&i<m_maxFdCount;++i){
            checkFd = checkFd<m_maxFdCount-1 ?checkFd+1 :0;//利用了操作系统分配描述符的规律，描述符id在[0~maxFdCount)之间
            Connection* c = m_connContainer->Get(checkFd);
            if(nullptr!=c && c->GetState() == ConnState::accept){
                if(now - c->GetLastAccessTime() > m_timeout){
                    LOG_ERROR("fd:%d idle_time:%d > timeout:%d",c->GetFd(), now - c->GetLastAccessTime(),m_timeout);
                    Destroy(c->GetFd()); 
                }
            }
        }
    }
    return true;
}

bool Server::EnableTcpKeepAlive(int fd, int aliveTime, int interval, int count){
    int keepalive = 1;
    if(-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive, sizeof(keepalive))){
        LOG_ERROR("%s",strerror(errno));
        return false;
    }

    if(-1 == setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&aliveTime, sizeof(aliveTime))){
        LOG_ERROR("%s",strerror(errno));
        return false;
    }
    
    if(-1 == setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(interval))){
        LOG_ERROR("%s",strerror(errno));
        return false;
    }
    
    if(-1 == setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&count, sizeof(count))){
        LOG_ERROR("%s",strerror(errno));
        return false;
    }

    return true;
}

bool Server::EnableTcpNoDelay(int fd){
    int nodelay = 1; 
    if(-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&nodelay, sizeof(nodelay))){
        LOG_ERROR("%s",strerror(errno));
        return false;
    }
    return true;
}
