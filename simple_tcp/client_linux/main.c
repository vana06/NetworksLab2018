#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../util_linux/util_linux.h"


int main(int argc, char *argv[]) {
	int sockfd;
	uint16_t portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char* buffer = (char*)malloc(256);
	//обазательно в качестве аргументов указать IP/имя хоста и номер порта
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portno = (uint16_t) atoi(argv[2]);

	// создаем сокет
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//Если возникла ошибка открытия сокета то выходим из программы
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	//получаем ip адрес по имени хоста
	server = gethostbyname(argv[1]);
	
	if (server == NULL) {
		closeSocket((int[]){sockfd}, 0, "ERROR, no such host\n");
	}
	// Инициализируем структуру сокета
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//заносим данные в стуктуру сокета
	serv_addr.sin_family = AF_INET;
	bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
	serv_addr.sin_port = htons(portno);

	//соединяемся с сервером
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeSocket((int[]){sockfd}, 1, "ERROR connecting");
	}

	//ожидаем ввода сообщния от польщователя для последующей отправки на сервер
	printf("Please enter the message: ");
	fgets(buffer, 255, stdin);

	//отправляем сообщение на сервер
	sendAll((int[]){sockfd}, buffer);

	//читаем ответ с сервера
	buffer = readAll((int[]){sockfd});

	printf("%s\n", buffer);

	free(buffer);
	buffer = NULL;

	closeSocket((int[]){sockfd}, 0, "");

	return 0;
}
