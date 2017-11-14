/* 
 * File:   TCPBaseSocket.h
 * Author: msobral
 *
 * Created on 6 de Julho de 2016, 16:14
 */

#ifndef TCPBASESOCKET_H
#define	TCPBASESOCKET_H

#include <string>
#include <list>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// classe TCPBaseSocket: não deve ser instanciada diretamente ..
// pois é a classe base para TCPClientSocket e TCPServerSocket
class TCPBaseSocket {
 public:     
  TCPBaseSocket(const string & addr, unsigned short port);
  TCPBaseSocket(int socket_descriptor);
  TCPBaseSocket();
  TCPBaseSocket(const TCPBaseSocket& orig);
  virtual ~TCPBaseSocket();
  
  // obtém endereço IP e port do socket da outra ponta 
  // da conexão
  void get_peer(string & addr, unsigned short & port);
  
  // obtém endereço IP e port deste socket
  void get_self(string & addr, unsigned short & port);
  
  // envia os bytes contidos em "data"
  int send(const string & data);
  
  // envia os "numbytes" bytes contidos em "buffer"
  int send(const char* buffer, int num_bytes);

  // recebe até "max_bytes", e retorna-os como uma string
  string recv(int max_bytes);
  
  // recebe até "max_bytes", e grava-os em "buffer"
  int recv(char * buffer, int max_bytes);

  // retorna o descritor deste socket
  int get_descriptor() { return sd;}
  
  // testa se o socket está conectado
  bool isConnected();
  
  // termina a conexão ... porém o destructor a termina
  // automaticamente se necessário
  virtual void close(){::close(sd);}
  
  // classe SocketException: usada para informar erros
  // de socket em geral. O código de erro
  // segue os erros padrões da variável errno
  class SocketException: public exception {
  public:
      SocketException(int error) : erro(error) {}
      ~SocketException() throw() {}
      const char * what() { return strerror(erro);}
      int get_errno() { return erro;}
  private:
      int erro;
  };
  
 protected:
  int sd;
  
  sockaddr_in make_addr(const string & addr, unsigned short port) const;
};

// classe TCPClientSocket: implementa um socket TCP cliente
class TCPClientSocket : public TCPBaseSocket {
 public:
  // cria um socket TCP cliente associado a um endereço IP e port
  TCPClientSocket(const string & addr, unsigned short port);
  
  // cria um socket TCP cliente associado a um endereço IP e port
  // escolhidos pelo sistema operacional
  TCPClientSocket();
  virtual ~TCPClientSocket() {}
  
  // conecta a um servidor dado por um IP e port
  void connect(const string & addr, unsigned short port);
};

/*class TCPConnectedSocket : public TCPBaseSocket {
  TCPConnectedSocket(int socket_descriptor);
  virtual ~TCPConnectedSocket() {}
  bool isNew() const { return novo;}
  void set_used() { novo = false;}
 private:
  bool novo;
};*/

  class Connection : public TCPBaseSocket {
   public:
    bool isNew() const { return novo;}
    
    friend class TCPServerSocket;
   private:
    Connection(int socket_descriptor);
    virtual ~Connection() {}
    void set_used() { novo = false;}

    bool novo;
  };

// classe TCPServerSocket: implementa um socket TCP servidor
class TCPServerSocket : public TCPBaseSocket {
public:
  // cria um socket TCP servidor associado a um endereço IP e port    
  TCPServerSocket(const string & addr, unsigned short port);
  
  // cria um socket TCP cliente associado endereço IP 
  // escolhido pelo sistema operacional, e ao port informado
  TCPServerSocket(unsigned short port);
  
  virtual ~TCPServerSocket();
  
  // aceita uma conexão. Isso retorna um outro socket,
  // que deve ser usado para envio e recepção
  Connection & accept();
  
  // espera por novas conexões OU por dados disponíveis 
  // nas conexões existentes por no máximo "timeout_ms" milissegundos. 
  // Retorna o primeiro socket que tiver
  // dados para serem lidos. Novas conexões têm prioridade em relação
  // a dados existentes ...
  Connection & wait(long timeout_ms);

  // espera indefinidamente por novas conexões OU por dados disponíveis 
  // nas conexões existentes. Retorna o primeiro socket que tiver
  // dados para serem lidos. Novas conexões têm prioridade em relação
  // a dados existentes ...
  Connection & wait();

  // encerra uma das conexões 
  // isso é opcional: se um socket for encerrado explicitamente
  // com seu método "close", o socket servidor que o originou
  /// irá detectar isso quando apropriado ...
  void close_connection(Connection & sock);
  
  // retorna a quantidade de conexões existentes
  int get_num_connections() const;

  // verifica sockets desconectados, removendos da lista
  void check_disconnected();
  
  // classe para informar timeout na forma de uma exceção
  class TimeoutException: public exception {
  public:
      TimeoutException() {}
      ~TimeoutException() throw() {}
      const char * what() { return "timeout";}
  };
  
  // classe para informar que uma ou mais conexões foram 
  // terminadas
  class DisconnectedException: public exception {
  public:
      DisconnectedException(const string & addr, unsigned short port) : addr(addr),port(port){}
      ~DisconnectedException() throw() {}
      const char * what() { return "disconnected";}
      string get_addr() { return addr;}
      unsigned short get_port() { return port;}
  private:
      string addr;
      unsigned short port;
  };

protected:
    list<Connection*> conns;
};

#endif	/* TCPBASESOCKET_H */

