#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h> 
#include <fstream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <cstdint>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <sstream>

#include <parser_Proto01.h>
#include "TCPBaseSocket.h"






class protomau {
	private:		
	
	public:
		enum Evento {timeout, quadro};
		enum Estado {ocioso, inativo, ativo};
		
		Evento evento;
		Estado estado;
		
		int fd_sock;
		fd_set r;		
		
		time_t timer;
		timeval timer_val; 
		
		TCPClientSocket sock;    
			
		void init(int IDplaca, vector<TPsensor> VETsensores,void (*func)(vector<TPsensor> vs));
			
		//void envia( unsigned char *buffer, int bytes);
		//int recebe();		
		void (* fApp)(vector<TPsensor> vs);
		
		void protoinit(int IDplaca, vector<TPsensor> sensores);

		void assoc(TPlaca p);
		void configura(TPlaca p);
		void dados();

		void handle();

		TPDU recebePDU();
		void enviaPDU(TPDU pdu);

		TPlaca placa;
		vector<TMostra> vamostras;
		
};

