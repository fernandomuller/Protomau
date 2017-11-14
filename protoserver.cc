// protoserver.cc

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/un.h>
#include <signal.h>

#include <parser_Proto.h>
#include <iostream>
#include <sstream>

#define PORTA 9999

void usage(const char *argv0);
u_int32_t gera_hora();
void processa_pedido();

void socket_tcp();

int x,y, valor;

char flag;
char buffer[sizeof(u_int8_t) * 1024];

// Tratador de sinal. Necessário para evitar que os filhos "zumbis"
// fiquem na memória para sempre.
void sigchld(int bla)
{
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
		// nao faz nada ;)
	}
	// Reinstala tratador de sinal (alguns Unixes desinstalam o
	// tratador de sinal do usuário quando este último é invocado).
	signal(SIGCHLD, sigchld);
}

int main(int argc, char *argv[])
{
	cout<<"\nIniciando SERVIDOR..."<<endl;
	
	signal(SIGCHLD, sigchld);

	if (sizeof(long int) != sizeof(time_t)) {
		printf("Erro ;)\n");
	}
	
	socket_tcp();
}

// inspirado no livro "Unix Network Programming" de Richard W Stevens, p. 13
void socket_tcp()
{
	// CONFIGURA O SOCKET...................................
	
	int pid, fd_escuta, fd_conexao;
	struct sockaddr_in endereco_servidor;
	fd_escuta = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_escuta < 0) {
		printf("Erro em socket()\n");
		exit(1);
	}
	bzero(&endereco_servidor, sizeof(endereco_servidor));
	endereco_servidor.sin_family = AF_INET;
	endereco_servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	endereco_servidor.sin_port = htons(PORTA);
	// Atrela o soquete ao endereço de servidor
	if (bind(fd_escuta, (struct sockaddr *) &endereco_servidor,
			sizeof(endereco_servidor)) == -1) {
		printf("Erro no bind()\n");
		exit(1);
	}
	listen(fd_escuta, 5);
	
	

	// O SERVIDOR FICA EM LOOP ATENDENDO CLIENTES..........
	cout<<"Aguardando conexões..."<<endl;
	while(1) {

		// Accept() é bloqueante, ou seja, o servidor irá dormir
		// até uma conexão entrar.
		
		fd_conexao = accept(fd_escuta, 0, 0);
		if (fd_conexao < 0) {
			continue;
		}
		// gera um processo-filho para tratar a conexao
		pid = fork();
		if (pid < 0) {// fork() falhou
			close(fd_conexao);
			continue;
		} else if (pid > 0) {// sou o pai
			close(fd_conexao);
			continue;
		}
		// se chegou aqui, sou o filho ;)
		//bzero(buffer, sizeof(buffer));

		y = 0;		
		y = recv(fd_conexao, buffer, 1024, 0);
		cout <<"\nMensagem recebida! Tamanho: "<<y<<" bytes..."<< endl;	
		string msg = string(buffer, y);
		
	// AQUI ENTRA O ASN1..............................
		
		stringstream inp;
 		TTeste::DerDeserializer decoder(inp);
 		inp.write(msg.c_str(), y);
 		
    	// tenta decodificar uma estrutura de dados
    	TTeste * other = decoder.deserialize();
      	if (other) {
        	cout<<"Estrutura obtida da decodificação DER:"<<endl;
        	other->show();
      	} else  break;
      	// destruir estruturas obtidas do decodificador 
      	delete other;
    	
    //DAQUI PRA FRENTE TRATA-SE OS DADOS E FECHA CONEXÃO......
    	
		flag = 1;
		if (flag ) {
			printf("Fechando última conexão...\n");
			close(fd_conexao);
		// perda de tempo pois o processo-filho vai morrer
		// close(fd_conexao);

		// fecha o processo-filho
			exit(0);
		} else if (flag == 0) {			
			/*while (x < sizeof(resposta.buffer)) {
				y = write(fd_conexao, resposta.buffer + x,
					   sizeof(resposta.buffer) - x);
				if (y <= 0) {
					break;
				}else{printf("Mensagem ACK enviada ao cliente.\n");}
				x += y;
			}*/			

			// perda de tempo pois o processo-filho vai morrer
			// close(fd_conexao);

			// fecha o processo-filho
			exit(0);
		} else {
			printf("Solicitação do cliente não faz sentido.\n");
		} 
		//return;

		
	}
}

u_int32_t gera_hora()
{
	time_t hora;
	time(&hora);
	printf("Servidor gerou hora %s\n", ctime(&hora));
	return hora;
}


