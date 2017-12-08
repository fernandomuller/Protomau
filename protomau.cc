#include "protomau.h"

void protomau::initClient(int IDplaca, vector<TPsensor> VETsensores, void (*funcaoApp)(vector<TPsensor> vs)){
		
		int N = VETsensores.size();
		
		fApp = funcaoApp;
		
		timer = time(NULL);	

		printf("Atualizando valor dos sensores...\n");	  	
	  	fApp(VETsensores);	   	
	  	
	  	//Define valores de amostras e 
	  	//insere nos registros do sensor
	  	vector<TMostra> va[N];
	  	TMostra a[N];	  		  		  	    	
	   	for(int x=0; x<N; x++){
	   		VETsensores[x].set_valor((x+1)*10);
	   		a[x].set_timestamp(time(NULL));
	   		a[x].set_valor(VETsensores[x].get_valor());
	   		va[x].push_back(a[x]);
	   		VETsensores[x].set_regs(va[x]);
	   	}
		
		placa.set_idPlaca(IDplaca);
		placa.set_datapoints(VETsensores);   		 	  	
  		placa.show();
  	
		estado = inativo;
		evento = timeout;
	   	
	    sock.connect("127.0.0.1", 9999);

	   	if(sock.isConnected())cout << "\nConectado!";	   		
			
		fd_sock = sock.get_descriptor();
		int op_sock = fcntl(fd_sock, F_GETFL);
		fcntl(fd_sock, F_SETFL, op_sock | O_NONBLOCK);
 		
 		cout << "\n**  Iniciando Máquina de Estados...  **";
        handle();
}

void protomau::initServer(){

	int y;
	char buffer[sizeof(u_int8_t) * 1024];

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
	// Accept() é bloqueante,o servidor irá dormir
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
		// se chegou aqui, sou o filho 

		y = 0;	
		while(true){
			y= 0;
			y = recv(fd_conexao, buffer, 1024, 0);
			if(y>0){
				cout<<"\nMensagem recebida! Tamanho: "<<y<<" bytes..."<< endl;	
				string msg = string(buffer, y);
		
			// AQUI ENTRA O ASN1 para RX   ........................
		
				stringstream inp;
			 	TPDU::DerDeserializer decoder(inp);
			 	inp.write(msg.c_str(), y);
				TPDU  *pdu_rx = decoder.deserialize();
				
				if (pdu_rx) {
					cout<<"Estrutura obtida da decodificação DER:"<<endl;
					pdu_rx->show();
				}  else{
				  		cout<<"\nImpossível decodificar estrutura\n"<<endl;
				}							
				
				TPDU::Choice_payload & carga = pdu_rx->get_payload();	
				
				if(carga.get_choice() == payload_PR_assoc ){
				 	printf("\nPDU: Associacao\n");
				 	TAssociacao associa = carga.get_assoc();				 	
				 	
				 	vector<TEmprego> ve;	
					associa.get_sensores(ve);
					
					int N = ve.size();	
					
					vector<TAlarme> va;
					TAlarme a[N];

					if(N>0){
						for(int i = 0; i < N; i++){
							a[i].set_id(ve[i].get_id());
							a[i].set_valorMaximo(50 + i);
							a[i].set_valorMinimo(5 + i);				
							va.push_back(a[i]);	
						}		
					}
					
				// AQUI ENTRA O ASN1 TX   ...............
								 
					TPDU pdu_tx; 
					TPDU::Choice_payload & pay = pdu_tx.get_payload();	
					TConfiguracao config = pay.get_conf();
						
					config.set_pEnvio(10);
					config.set_pAmostragem(2);
					config.set_alarme(va);
									
					pdu_tx.check_constraints();	
					
					//pdu_tx.show();
	
				  	ostringstream out;
				  	TPDU::DerSerializer encoder(out);				  	
				  	encoder.serialize(pdu_tx); 	
				  	string conteudo = out.str();
				  	int c = conteudo.size();  							 
			 
					int tx = send(fd_conexao, conteudo.c_str(),c,0);
			
				  	cout<<"\nEnviou: "<<tx<<" bytes de "<<c<<endl;
				 	
				 	
				}else if(carga.get_choice() == payload_PR_dados ){
				 	printf("\nPDU: Dados\n");
				 	TDados data = carga.get_dados();
				 					 	
				 	vector<TValores>  v_valores;					
					data.get_val(v_valores);
									
					int t = v_valores.size();							

					vector<TMostra> v_amostras[t];	
				
					std::ofstream out("regs.txt", std::ios_base::app | std::ios_base::out);
					
					if(out == NULL){
						printf("Erro, nao foi possivel abrir o arquivo\n");
					}else{	

						for(int y=0; y<t; y++){	
												
							v_valores[y].get_registros(v_amostras[y]);
						
							int k = v_amostras[y].size();
						
							for(int u = 0; u<k; u++){
								int ts = v_amostras[y][u].get_timestamp();
								int val = v_amostras[y][u].get_valor();
								out<<"id: "<< v_valores[y].get_idSensor();
								out<< " , timestamp: "<< ts <<" , valor: "<<val;
							}	
							out << "\r";				
						}
						out << "\n\r";
						printf("\nGravou registros em arquivo! \n");
						out.close();	
					}												 	
				}			
				sleep(1);	
				delete pdu_rx;										
			}
			y=0;				
		}				
	}


}

void protomau::amostrar(){

		vector<TPsensor> VETsensores;
		placa.get_datapoints(VETsensores);
		int N = VETsensores.size();
		
		printf("Atualizando valor dos sensores...\n");	  	
	  	fApp(VETsensores);   	
	  	
	  	vector<TMostra> va[N];
	  	TMostra a[N];
	  		  		  		  	    	
	   	for(int x=0; x<N; x++){
	   		a[x].set_timestamp(time(NULL));
	   		a[x].set_valor(VETsensores[x].get_valor());
	   		va[x].push_back(a[x]);
	   		VETsensores[x].set_regs(va[x]);
	   	}		
		placa.set_datapoints(VETsensores);   		 	  	
  		//placa.show();


}

void protomau::handle(){
			
	char buffer[sizeof(u_int8_t) * 1024];
	int n = 0;	
	
 	switch (evento)
 	{
        case timeout:
		    	if(estado==inativo){	
		    		assoc(placa);//Envia mensagem Associação
		    		timer_val = {5,0};
		    	}else{
		    		dados();//Envia mensagem Dados
		    		timer_val = {5,0};
		    	}        		      
		    						
				FD_ZERO(&r);
				FD_SET(fd_sock, &r);						   		   
				n = select(fd_sock+1, &r, NULL, NULL, &timer_val);
				
				printf ("\n\nn: %d .  " , n);
				
				if (FD_ISSET(fd_sock, &r)) {
				 	cout << "\nTrata recepção msg Conf...\n";
					evento = quadro;
					handle(); 
					//break;       				
				}   
													  		    
				if (n <= 0) { 
					cout << "\nTrata Timeout...\n";
					evento = timeout;							
					handle();
					//break;
				} 									
            break;

        case quadro://---------------------------------------------
            
            //Trata recepção msg Conf
            		cout << "\nQuadro recebido!\n";
  					int rx = sock.recv(buffer, 1024); 				
  					string msg = string(buffer, rx);  				
					cout << "\nRecebeu: "<<rx<<" bytes\n";
			
			// AQUI ENTRA O ASN1.............................	
			 
					stringstream inp;
			 		TPDU::DerDeserializer decoder(inp);
			 		inp.write(msg.c_str(), rx);						
					TPDU * other = decoder.deserialize();			
					
				  	if (other) {
						other->show();
						TPDU::Choice_payload & carga = other->get_payload();	
						if(carga.get_choice() == payload_PR_conf){
						 	printf("\nPDU: Configuracao\n");
						 	TConfiguracao config = carga.get_conf();
						 	estado = ativo;
						}						
						
				  	} else{
				  		cout<<"\nImpossível decodificar estrutura\n"<<endl;
				  	}
				  	
				 	timer_val = {5,0};	
				 							
					FD_ZERO(&r);
					FD_SET(fd_sock, &r);					   		   
					n = select(fd_sock+1, &r, NULL, NULL, &timer_val);
						
					if (FD_ISSET(fd_sock, &r)) {  
							evento = quadro;          	 
							handle(); 
							break;       				
					}     
									  		    
					if (n <= 0) { 
							evento = timeout;
							handle();
							break;
					} 
         
            break;
    }	
}

void protomau::assoc(TPlaca p){	

  	cout << "\nEnviando msg Associação...\n";
  	
  	TPDU pdu; 	   	
	TPDU::Choice_payload & carga = pdu.get_payload();	
	TAssociacao associa = carga.get_assoc();
	
	associa.set_idPlaca(p.get_idPlaca());
	vector<TPsensor> v_sensores;
	placa.get_datapoints(v_sensores);
	
	int N = v_sensores.size();
	
	vector<TEmprego> ve;
	TEmprego e[N];
	
	if(N>0){
			for(int i = 0; i < N; i++){
				e[i].set_id(v_sensores[i].get_idSensor());
				e[i].set_tipo(v_sensores[i].get_tipo());			
				ve.push_back(e[i]);	
		   }		
	}
	
	associa.set_sensores(ve); 
	  	  	
	pdu.check_constraints();	
	pdu.show();
	
  	ostringstream out;
  	TPDU::DerSerializer encoder(out);
  	
  	encoder.serialize(pdu);  	
	
  	string conteudo = out.str();
  	cout<<"Enviou: "<<sock.send(conteudo)<<" bytes de "<<conteudo.size()<< endl;
  	
}

void protomau::dados(){
	
	amostrar();
	// Monta o PDU com uma mensagem de dados
  	TPDU pdu; 	   	
	TPDU::Choice_payload & carga = pdu.get_payload();	
	TDados data = carga.get_dados();
		
	vector<TPsensor> v_sensores;
	placa.get_datapoints(v_sensores);
	
	int t = v_sensores.size();
	
	vector<TValores>  v_valores;	
	TValores v[t];		
	vector<TMostra> v_amostras[t];			

	printf("\nMontando PDU Dados com %d sensores.\n", t);  	
  	
	if(t>0){
			for(int i = 0; i < t; i++){
				v[i].set_idSensor(v_sensores[i].get_idSensor());	
				v_sensores[i].get_regs(v_amostras[i]);
				v[i].set_registros(v_amostras[i]);
				v_valores.push_back(v[i]);	
		   }		
	}
	
	data.set_val(v_valores);
		
	pdu.check_constraints();
	pdu.show();
	
	ostringstream out;
  	TPDU::DerSerializer encoder(out);
  	
  	encoder.serialize(pdu);  	
	
  	string conteudo = out.str();
  	cout<<"Enviou: "<<sock.send(conteudo)<<" bytes de "<<conteudo.size()<< endl;
  	
}


