#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../util_linux/util_linux.h"

int main() {
	int sockfd, newsockfd;
	uint16_t portno;
	unsigned int clilen;
	char* buffer;
	struct sockaddr_in serv_addr, cli_addr;

	// Сперва вызываем функцию socket()
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//Если возникла ошибка открытия сокета то выходим из программы
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	// Инициализируем структуру сокета
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5001;	//указываем порт
	//заносим данные в стуктуру сокета
	serv_addr.sin_family = AF_INET; //семейство адресов
	serv_addr.sin_addr.s_addr = INADDR_ANY; //IP
	serv_addr.sin_port = htons(portno); //порт

	//указываем дополнительные опции сокета для повторного открытия нового сокета на том же порту
	//https://serverfault.com/questions/329845/how-to-forcibly-close-a-socket-in-time-wait
	if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &(int){ 1 }, sizeof(int)) < 0){
		closeSocket((int[]){sockfd}, 1, "ERROR on setsockopt");
	}

	// Привязываем адрес через функцию bind()
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeSocket((int[]){sockfd}, 1, "ERROR on binding");
	}

	//слушаем клиентов, стэк установлен на 5 соединений
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	//создаем новую ветку программы которая ожидает нажатия клавиши q и зыкрывает сервер
	if(fork() > 0){
		close(sockfd);
		while(getchar() != 'q'){
		}
		shutdown(sockfd, SHUT_RDWR);
		exit(0);
	}

	while(1) {

		// Принимаем соединение от клиента 
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		//в случае оишибки закрываем сервер
		if (newsockfd < 0) {
			closeSocket((int[]){sockfd, newsockfd}, 1, "ERROR on accept");
		}
		//создаем новый потом
		switch(fork()) {
			case -1:
				perror("ERROR on fork");
				break;
			case 0:
				//дочерний поток работает с клиентом
				close(sockfd);
				//если соединение было установлено то начинаем общение
				buffer = readAll((int[]){newsockfd, sockfd});

				printf("Here is the message: %s\n", buffer);

				//отправляет клиенту ответ
				sendAll((int[]){newsockfd, sockfd}, "I got your message");
				//зываершаем дочерний поток
				closeSocket((int[]){newsockfd}, 0, "");
				break;
			default:
				//родительский поток продалжает ожидать новых клиентов
				close(newsockfd);
			}

	}

	closeSocket((int[]){sockfd, newsockfd}, 0, "");

	return 0;
}


