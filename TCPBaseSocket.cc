/* 
 * File:   TCPBaseSocket.cpp
 * Author: msobral
 * 
 * Created on 6 de Julho de 2016, 16:14
 */

#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <exception>
#include <sstream>
#include "TCPBaseSocket.h"

using namespace std;

sockaddr_in TCPBaseSocket::make_addr(const string& addr, unsigned short port) const {
    sockaddr_in myaddr;
    hostent * haddr = gethostbyname(addr.c_str());

    if (not haddr) throw TCPBaseSocket::SocketException(errno);
    
    myaddr.sin_family = AF_INET;
    memcpy(&myaddr.sin_addr, haddr->h_addr_list[0], haddr->h_length);
    myaddr.sin_port = htons(port);    
    
    return myaddr;
}

TCPBaseSocket::TCPBaseSocket() :sd(0) {
}

TCPBaseSocket::TCPBaseSocket(const TCPBaseSocket& orig) {
    sd = orig.sd;
}

TCPBaseSocket::TCPBaseSocket(const string& addr, unsigned short port) {
    sockaddr_in myaddr = make_addr(addr, port);

    if ((sd = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
        throw TCPBaseSocket::SocketException(errno);
    }

    int opt = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {
        throw TCPBaseSocket::SocketException(errno);        
    }
/*    if (fcntl(sd, F_SETFL, O_NONBLOCK) < 0) {
        throw TCPBaseSocket::SocketException(errno);        
    }
*/    
    if (bind(sd, (sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
        throw TCPBaseSocket::SocketException(errno);        
    }
}

TCPBaseSocket::TCPBaseSocket(int socket_descriptor) : sd(socket_descriptor) {
    if (fcntl(sd, F_SETFL, O_NONBLOCK) < 0) {
        throw TCPBaseSocket::SocketException(errno);        
    }
}

TCPBaseSocket::~TCPBaseSocket() {
    if (sd) ::close(sd);
}

void TCPBaseSocket::get_peer(string& addr, unsigned short& port) {
    sockaddr_in paddr;
    socklen_t len = sizeof(paddr);
    
    if (getpeername(sd, (sockaddr*)&paddr, &len) < 0) {
        throw TCPBaseSocket::SocketException(errno);
    }

    addr = inet_ntoa(paddr.sin_addr);
    port = ntohs(paddr.sin_port);
}

void TCPBaseSocket::get_self(string& addr, unsigned short& port) {
    sockaddr_in paddr;
    socklen_t len = sizeof(paddr);
    
    if (getsockname(sd, (sockaddr*)&paddr, &len) < 0) {
        throw TCPBaseSocket::SocketException(errno);
    }

    addr = inet_ntoa(paddr.sin_addr);
    port = ntohs(paddr.sin_port);
}

int TCPBaseSocket::send(const string& data) {
    const char * buffer = data.c_str();
    int len = data.size();
        
    return send(buffer, len);
}

int TCPBaseSocket::send(const char* buffer, int num_bytes) {
    if (not sd) throw TCPBaseSocket::SocketException(EINVAL); 
    
    if (not num_bytes) return 0;
    
    int n = ::send(sd, buffer, num_bytes, 0);
    
    if (n < 0) throw TCPBaseSocket::SocketException(errno);
    
    return n;
}

string TCPBaseSocket::recv(int max_bytes) {
    char buffer[max_bytes];
    buffer[0] = 0;
    
    int n = recv(buffer, max_bytes);
    return string(buffer, n);    
}

int TCPBaseSocket::recv(char* buffer, int max_bytes) {
    if (not sd) throw TCPBaseSocket::SocketException(EINVAL); 
    
    int n = ::recv(sd, buffer, max_bytes, 0);
    if (n < 0) {
        if (errno != EAGAIN) throw TCPBaseSocket::SocketException(errno);
        n = 0;
    }
    return n;    
}

bool TCPBaseSocket::isConnected(){
    char x;
    struct timeval tv= {0,0};
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(sd, &socks);
            
    int n = select(sd+1, &socks, NULL, NULL, &tv);
    if (n) {
        ssize_t r = ::recv(sd, &x, 1, MSG_DONTWAIT|MSG_PEEK);
        return (r > 0);
    }
    return true;
}

TCPClientSocket::TCPClientSocket() : TCPBaseSocket("0.0.0.0", 0) {    
}

TCPClientSocket::TCPClientSocket(const string& addr, unsigned short port) : TCPBaseSocket(addr, port) {
}

void TCPClientSocket::connect(const string& addr, unsigned short port) {
    sockaddr_in servaddr = make_addr(addr, port);
    
    if (::connect(sd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        throw TCPBaseSocket::SocketException(errno);
    }
}

TCPServerSocket::TCPServerSocket(const string& addr, unsigned short port) : TCPBaseSocket(addr, port){
    if (::listen(sd,5) < 0) throw TCPBaseSocket::SocketException(errno);
}

TCPServerSocket::TCPServerSocket(unsigned short port) : TCPBaseSocket("0.0.0.0", port) {
    if (::listen(sd,5) < 0) throw TCPBaseSocket::SocketException(errno);
}

TCPServerSocket::~TCPServerSocket() {
    list<Connection*>::iterator it;

    check_disconnected();
    
    for (it = conns.begin(); it != conns.end(); it++) {
        delete *it;
    }
}

Connection & TCPServerSocket::accept() {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    int con = ::accept(sd, (sockaddr*)&addr, &len);
    if (con < 0) throw TCPBaseSocket::SocketException(errno);
    Connection * novo = new Connection(con);
    conns.push_back(novo);
    return *novo;
}

int TCPServerSocket::get_num_connections() const {
    return conns.size();
}

void TCPServerSocket::close_connection(Connection& sock) {
    list<Connection*>::iterator it;
    for (it = conns.begin(); it != conns.end(); it++) {
        if (*it == &sock) {
            Connection * ptr = *it;
            conns.erase(it);
            delete ptr;
            break;
        }
    }
    
}

Connection & TCPServerSocket::wait() {
    return wait(0);
}

Connection & TCPServerSocket::wait(long timeout_ms) {
    if (not sd) throw TCPBaseSocket::SocketException(EINVAL);
    timeval tv, * tv_ptr = NULL;
    
    if (timeout_ms) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = 1000*(timeout_ms % 1000);
        tv_ptr = &tv;
    }
    
    check_disconnected();
    
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(sd, &socks);    
    int maior = sd;
    
    list<Connection*>::iterator it;
    for (it = conns.begin(); it != conns.end(); it++) {
        Connection * ptr = *it;
        int sockd = ptr->get_descriptor();
        FD_SET(sockd, &socks);
        if (sockd > maior) maior = sockd;
    }
    
    int n = select(maior+1, &socks, NULL, NULL, tv_ptr);

    if (not n) throw TCPServerSocket::TimeoutException();
    
    /*for (it = conns.begin(); it != conns.end(); it++) {
        TCPBaseSocket * sock = *it;
        int sockd = sock->get_descriptor();
        if (FD_ISSET(sockd, &socks)) {
            string addr;
            unsigned short port;
            
            sock->get_peer(addr,port);
            if (not sock->isConnected()) {
                throw TCPServerSocket::DisconnectedException(addr,port);
            }
        }
    }    */

    if (FD_ISSET(sd, &socks)) {
        return accept();
    }

    for (it = conns.begin(); it != conns.end(); it++) {
        Connection * sock = *it;
        int sockd = sock->get_descriptor();
        if (FD_ISSET(sockd, &socks)) {
            if (not sock->isConnected()) {
                string addr;
                unsigned short port;

                sock->get_peer(addr,port);
                throw TCPServerSocket::DisconnectedException(addr,port);
            }      
            if (sock->isNew()) sock->set_used();
            return *sock;
        }
    }    
            
}

void TCPServerSocket::check_disconnected() {
    list<Connection*>::iterator it;
    for (it = conns.begin(); it != conns.end(); it++) {
        Connection * ptr = *it;
        if (not ptr->isConnected()) {
            it = conns.erase(it);
            delete ptr;
        }
    }    
}

Connection::Connection(int socket_descriptor) : 
  TCPBaseSocket(socket_descriptor), novo(true) {
}
