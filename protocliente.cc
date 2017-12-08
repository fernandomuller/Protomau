#include "protomau.h"

//	Simula a leitura dos sensores
void atualizasensores(vector<TPsensor> vs){	

		int t =vs.size();
		printf("Lendo %d sensores...Atualizando...\n", t);		
		if(t>0){
				int id = 0;
				for(int i = 0; i < t; i++){
					id = vs[i].get_idSensor();
					printf("Atualizando sensor id= %d. ", id);
					vs[i].set_valor(vs[i].get_valor()+id);
				  	cout<<"Valor atualizado = "<<vs[i].get_valor()<<endl;
			   }		
		}		
}

int main(int argc, char *argv[]){  	
				
	protomau proto;	   	
  	 	
  	//Define sensores e valores iniciais(id e tipo)
  	  	
  	int N = 2;//Número de sensores
  	int idPlaca = 1001;
  	
  	vector<TPsensor> sensores; 
  	TPsensor sensor[N];  
  	  	
   	for(int x=0; x<N; x++){
   		sensor[x].set_idSensor(x+1);
   		sensor[x].set_tipo((x+1)*10);
   		sensores.push_back(sensor[x]);
   	}  	
  	
  	proto.initClient(idPlaca, sensores, atualizasensores); 
			
}


