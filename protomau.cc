#include <iostream>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <cstdint>
#include <cstring>

#include "protomau.h"

using namespace std;

/*protomau::protomau(Enquadramento & e): enquad(e) {
    estado = Ocioso;
    rseq = 0;
    tseq = 0;
}*/

void protomau::init(int IDplaca, vector<TPsensor> VETsensores, void (*funcaoApp)(vector<TPsensor> vs)){
		
		int N = VETsensores.size();
		
		fApp = funcaoApp;
		
		timer = time(NULL);	

		printf("Atualizando valor dos sensores...\n");	  	
	  	fApp(VETsensores);	  	
	  	printf("Atualizado!!! Timer: %ld \n\n", timer);
	  	
	  	//Define valores de amostras e 
	  	//insere nos registros do sensor
	  	vector<TMostra> va[N];
	  	TMostra a[N];	  		  		  	    	
	   	for(int x=0; x<N; x++){
	   		VETsensores[x].set_valor((x+1)*10);
	   		a[x].set_timestamp(time(NULL));
	   		a[x].set_valor(VETsensores[x].get_valor());
	   		//VETsensores[x].get_regs(va[x]);
	   		va[x].push_back(a[x]);
	   		VETsensores[x].set_regs(va[x]);
	   	}
		
		placa.set_idPlaca(IDplaca);
		placa.set_datapoints(VETsensores);   		 	  	
  		placa.show();
  	
		estado = inativo;
		evento = timeout;
		
		printf("\nInit... Estado MEF: %d. Evento: %d",estado,evento);
	   	
	    sock.connect("127.0.0.1", 9999);	   
	   	if(sock.isConnected())cout << "\nConectado!";
	   	
		fd_sock = sock.get_descriptor();
		int op_sock = fcntl(fd_sock, F_GETFL);
		fcntl(fd_sock, F_SETFL, op_sock | O_NONBLOCK);
 		
 		cout << "\n**  Iniciando Máquina de Estados...  **";
    	evento = timeout;
        //handle();
        dados();
        sleep(2);
        exit(0);
}

void protomau::handle(){
	printf ("\n\nhandle() Estado MEF: %d Evento: %d",estado,evento);	
		
	char buffer[1024];
	int n = 0;
	if(estado==1){cout << "  **  INATIVO  **";
	}else{cout << "  **  ATIVO  **";} 
		
	//int y = sock.recv(buffer, 1);
	//printf ("\n\nRecebeu: %d " , y);
	
 	switch (evento)
 	{
        case timeout://----------------------------------------
		    	if(estado==inativo){
		    	//cout << "\n** inativo**********";
		    		assoc(placa);//Envia mensagem Associação
		    		timer_val = {5,0};
		    	}else{
		    	//cout << "*********ativo**********";
		    		dados();//Envia mensagem Dados
		    		timer_val = {5,0};
		    	}        		           
           			//timer_val = {5,0};					
				FD_ZERO(&r);
				FD_SET(fd_sock, &r);						   		   
				n = select(fd_sock+1, &r, NULL, NULL, &timer_val);
				
				printf ("\n\nn: %d .  " , n);
				
				if (FD_ISSET(fd_sock, &r)) {
				 cout << "*********Socket ??? **********";
						//Trata recepção msg Conf	
						 
						evento = quadro;
						handle(); 
						break;       				
				}   
				//handle();										  		    
				if (n <= 0) { 
				cout << "\n*********Timeout ???**********";
						evento = timeout;							
						handle();
						break;
				} 									
            break;

        case quadro://-------------------------------------------------
            
            //Trata recepção msg Conf
            		
            		//string recv(int max_bytes);
  					int rx = sock.recv(buffer, 1024);
  					string msg = string(buffer, rx);
		
			// AQUI ENTRA O ASN1..............................
		
					stringstream inp;
			 		TPDU::DerDeserializer decoder(inp);
			 		inp.write(msg.c_str(), rx);								
					TPDU * other = decoder.deserialize();					
					
				  	if (other) {
						cout<<"Estrutura obtida da decodificação DER:"<<endl;
						other->show();
						TPDU::Choice_payload & carga = other->get_payload();	
						//TConfiguracao config = carga.get_conf();
						//configura();
						/*
						Configuracao ::= SEQUENCE {
							pEnvio INTEGER,
							pAmostragem INTEGER,
							pAmostragemEsp SEQUENCE OF Emprego,
							alarme SEQUENCE OF Alarme
						}*/	
						
						//config.set_idPlaca(p.get_idPlaca());
						
				  	} //else  break;
				  	// destruir estruturas obtidas do decodificador 
				  	delete other;		 		
				 	
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
        //default :
       		printf ("Evento invalido!\n");
    }	
   cout << "*********fim **********";
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
  	
  	//printf("\n**  Fim teste msg Assoc\n");
  	//sleep(1);
	//exit(1);  	
 	//sock.close();
}

void protomau::dados(){

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
				//v[i].show();
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
	
	printf("\n**  Fim teste msg Dados\n");
	exit(1);

}


