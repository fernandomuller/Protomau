#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <parser_Proto.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "TCPBaseSocket.h"

using namespace std;

#define PORTA 9999

void usage(const char *argv0);
void internet_tcp(const char *endereco);
void processa_resposta();
u_int32_t gera_hora();

int x,y;
int start = 1;
char  valor;

char CONF='0', SEND='1', ACK='2', STOP='3';

char buffer[sizeof(u_int8_t) * 1024];


time_t mytimer;

int tam_resposta, 
	intervalo_timer, 
	ativo = 1,
	sensor = 25; // Sensor que simula uma medição qualquer




int main(int argc, char *argv[]){

	TTeste pkt;
	// definindo os valores  
	pkt.set_id(1);
	pkt.set_tipo(1);
	pkt.set_counter(0);
	//pkt.set_data(time(NULL));
  	// verifica se os valores respeitam a especificação
  	pkt.check_constraints();
  	// mostra a estrutura de dados na tela
  	cout << "Estrutura de dados em memória (antes de codificação DER):" << endl;
  	pkt.show();

  	// cria o codificador
  	//ofstream out("pkt.data");
  	//TTeste::XerSerializer encoder(out);
  	
  	ostringstream out;
  	TTeste::DerSerializer encoder(out);

  	// codifica a estrutura de dados
  	encoder.serialize(pkt);
  	//out.close();

	TCPClientSocket sock;

  	sock.connect("127.0.0.1", 9999);
 	 // codifica a estrutura de dados
  	string conteudo = out.str();
  	cout<<"Enviou: "<<sock.send(conteudo)<<" bytes de "<<conteudo.size()<< endl;

  	sleep(1);

 	sock.close();
 	
	//------------------------------------------------------
	
	/*
  	// cria o decodificador
  	ifstream arq("pkt.data");
  	TTeste::XerDeserializer decoder(arq);

  	// tenta decodificar uma estrutura de dados
  	TTeste * other = decoder.deserialize();
  	arq.close();

  	cout << endl;

  	if (other) {
	    cout << "Estrutura de dados obtida da decodificação DER:" << endl;
    	other->show();    	
    	int id = other->get_id();
    	int tipo = other->get_tipo();
    	int counter = other->get_counter();
    	//mytimer = other->get_data();    	
    	cout << "id=" << id << endl;
    	cout << "tipo=" << tipo << endl;
    	cout << "counter=" << counter << endl;
    	//cout << "data=" << data << endl;
    	
    	//ASN1Oid & oid = other->get_oid_attr();
    	//cout << "oid=" << oid.str() << endl;
  	} else cerr << "Erro: não consegui decodificar a estrutura de dados ..." << endl;

  	// devem-se destruir explicitamente as estruturas de dados obtidas 
  	// do decodificador 
  	delete other; */
  
//******************************************************************

	//internet_tcp(argv[1]);

	/*if (sizeof(long int) != sizeof(time_t)) {
		printf("Erro ;)\n");
	}
	if (argc < 4) {
		usage(argv[0]);
	}//printf("\n Argv[3]: ");printf(" %s \n", argv[3]);
	
	intervalo_timer = atoi(argv[3]);
	data.valores.flag = CONF;
	data.valores.id = 1; // id sensores
	data.valores.valor = htons(sensor);
	data.valores.timestamp = htonl(intervalo_timer);	
	
	mytimer = time(NULL);	
	//printf("Timer iniciado: %ld \n\n", mytimer);

	while(ativo){
		if((time(NULL) - mytimer) >= intervalo_timer){	
			internet_tcp(argv[1]);
			data.valores.flag = '1'; 
			data.valores.valor = htons(sensor);
			data.valores.timestamp = htonl(gera_hora());
			mytimer += intervalo_timer;
		}
	}*/		
}

//------------------------------------------------------------------


u_int32_t gera_hora()
{
	time_t hora;
	time(&hora);
	//printf("Cliente gerou hora %s\n", ctime(&hora));
	return hora;
}
void usage(const char *argv0)
{
	printf("Uso: %s  <endereco> <n_sensores> <intervalo(segundos)>\n", argv0);
	exit(1);
}

// inspirado no livro "Unix Network Programming" de Richard W Stevens, p. 6
void internet_tcp(const char *endereco)
{/*
	int fd_conexao;
	struct sockaddr_in endereco_servidor;

	bzero(&endereco_servidor, sizeof(endereco_servidor));

	fd_conexao = socket(AF_INET, SOCK_STREAM, 0);

	endereco_servidor.sin_family = AF_INET;
	endereco_servidor.sin_port = htons(PORTA);

	if (inet_pton(AF_INET, endereco, &endereco_servidor.sin_addr) <= 0) {
		printf("Erro no endereço.\n");
		exit(1);
	}

	if (connect(fd_conexao, (struct sockaddr *) &endereco_servidor, 
				sizeof(endereco_servidor)) < 0) {
		printf("Impossível conectar com servidor.\n");
		exit(1);
	}
	// tanto read() como write() não garantem que vão ler/gravar
	// um buffer de uma vez só. Opcionalmente, eles podem retornar
	// um número de bytes gravados que é menor que o total
	// desejado. Por isso, temos de fazer um loop que vá insistindo
	// na leitura/gravação até chegar no total desejado...
	//
	// O interessante é que read()/write() são, mesmo assim, bloqueantes,
	// e retornam com pelo menos 1 byte lido ou gravado. Se eles 
	// retornam com 0 bytes lidos/gravados, em TCP/IP isso significa
	// que a conexão está sendo fechada.
	//
	// Como de praxe no Unix, se read()/write() retornam -1,
	// é porque ocorreu um erro.
	x = y = 0;
	if(start == 1){ // Envia mensagem CONF e não aguarda retorno
		while(x < sizeof(buffer)) {
			y = write(fd_conexao, buffer+x, 
				  sizeof(buffer)-x);
			if (y <= 0) {
				printf("Impossível mandar mensagem CONF.\n");
				exit(1);
			}else{printf("\nMensagem CONF enviada ao servidor.\n");}
			x += y;
		}
	// fecha a conexão no sentido de transmissão, para que
	// o servidor saiba que não temos mais nada a dizer.
		shutdown(fd_conexao, SHUT_WR);
		close(fd_conexao);
		start = 0;
	}else{// Envia mensagen SEND='1' e aguarda resposta ACK='2' ou STOP='3'
		while(x < sizeof(buffer)) {
			y = write(fd_conexao, buffer+x, 
				  sizeof(buffer)-x);
			if (y <= 0) {
				printf("Impossível mandar pergunta.\n");
				exit(1);
			}else{printf("\nMensagem SEND enviada ao servidor.\n");}
			x += y;
		}

	// fecha a conexão no sentido de transmissão, para que
	// o servidor saiba que não temos mais nada a dizer.
		shutdown(fd_conexao, SHUT_WR);
	// Novamente, insistimos em ler o tamanho do buffer, ou até que
	// read() retorne 0, o que significa que o servidor já disse
	// o que tinha a dizer e fechou a conexão.
		tam_resposta = y = 0;
		while (tam_resposta < sizeof(buffer)) {
			y = read(fd_conexao, buffer + tam_resposta, 
				 sizeof(buffer) - tam_resposta);
			if (y <= 0) {
				// servidor fechou a conexão, provavelmente
				// foi interrompido
				break;
			}
			tam_resposta += y;
		}
		processa_resposta();
		close(fd_conexao);
	}*/
}

void processa_resposta()
{
	/*char tipoflag = resposta.valores.flag;
	if (tam_resposta != sizeof(resposta.buffer)) {
		printf("\nresposta de tamanho diferente do esperado\n\n");
		return;
	}	
	if (tipoflag == STOP) {printf("flag: %s \n", &tipoflag);
		printf("\nEncerrar envio de dados! ! !\n");
		ativo = 0;
	} else if (tipoflag == ACK) {
		printf("ACK Recebido com sucesso! ! !\n");
	} else {
		printf("\nResposta do servidor não faz sentido.\n\n");
	}*/
}

