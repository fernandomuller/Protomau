#include <iostream>
#include <fcntl.h> 
#include <fstream>
#include <sstream>

#include <parser_Proto.h>
#include "TCPBaseSocket.h"

#define PORTA 9999

class protomau {		
	
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
			
		void initClient(int IDplaca, vector<TPsensor> VETsensores,void (*func)(vector<TPsensor> vs));
		void initServer();	
		
		void (* fApp)(vector<TPsensor> vs);
		
		void handle();
		void amostrar();
		void assoc(TPlaca p);
		void dados();		

		TPlaca placa;
		
};

