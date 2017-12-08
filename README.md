# Protomau - O protocolo mau humorado

Faça a instalação do compilador ASN1 seguindo os passos do link https://wiki.sj.ifsc.edu.br/wiki/index.php/Compilador_ASN1.

Após instalar o compilador siga os passos abaixo:

1- Copie os arquivos abaixo para a pasta asn1pp-master criada na instalação do compilador ASN1.
	
	protomau.h
	protomau.cc
	protocliente.cc
	protoserver.cc	
	Proto.asn1
	Makefile.build
	TCPBaseSocket.cc
	TCPBaseSocket.h

2- Edite o arquivo Makefile.build, conforme o arquivo executável que vc quer compilar.
	Exemplo: Para compilar o arquivo do servidor (protoserver.cc), edite as variáveis como mostrado abaixo.

	MYOBJS=protoserver.o protomau.o TCPBaseSocket.o
	PROG=server
	ASN1SRC:=Proto.asn1 

2- Execute o seguinte comando para compilar a especificação ASN1.
	
	$ make build	
	
3-Execute o comando abaixo para compilar o arquivo do programa principal (protoserver.cc).

	$ make all

Ao compilar a especificação e o programa principal, é criada uma pasta chamada “build”, onde encontra-se o arquivo executável.

OBS: Antes de compilar o arquivo protocliente.cc, renomeie a pasta build para build_server, por exemplo. E edite o arquivo Makefile.build conforme o arquivo executável que vc quer compilar. Nesse caso, protocliente.cc.

	MYOBJS=protocliente.o protomau.o TCPBaseSocket.o
	PROG=cliente
	ASN1SRC:=Proto.asn1 

4- Repita os passos 2 e 3 para compilar o programa do cliente. O executável (cliente) será criado na nova pasta “build” criada no comando ‘$make build’ a seguir.

	$ make build
	$ make all
