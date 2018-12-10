#include "util_linux.h"

//функция закрытия сокетов и завершение программы
void closeSocket(int socks[], int error, char* errorMsg){
	if(strcmp(errorMsg,"") != 0){
		perror(errorMsg);
	}
	int lenght = (int)(sizeof(socks)/sizeof(socks[0]));
	for(int i = 0; i < lenght; i++) {
		shutdown(socks[i], SHUT_RDWR);
		close(socks[i]);
	}
	exit(error);
}
//функция чтения сообщения
//данная функция сперва считывает 4 байта в которых хранится длина последующего сообщения
//затем считывается нужное количество байт указанное в предыдущем сообщении
char* readAll(int socks[]){
	char *buffer = (char*)calloc(256, sizeof(char));
	if(buffer == NULL){
		closeSocket(socks, 1, "ERROR allocation");
	}
	char strLenght[4];

	int n = read(socks[0], strLenght, 4);
	if (n < 0) {
		closeSocket(socks, 1, "ERROR reading from socket");
	}
	int lenght = atoi(strLenght);

	int recieved = 0;
	while(recieved < lenght){
		n = read(socks[0], buffer, 256);
		recieved += n;
		if (n < 0) {
			closeSocket(socks, 1, "ERROR reading from socket");
		}
	}

	return buffer;
}
//функция записи сообщения
//сперва отправляет 4 байта длины последующего сообщения
//затем отправляет само сообщение
void sendAll(int socks[], char* buffer){
	int messageLength = strlen(buffer);
	char toSend[4 + 256];

	snprintf(toSend, 4 + 256, "%04d%s", messageLength, buffer);

	int n = write(socks[0], toSend, strlen(toSend));
	if (n < 0) {
		closeSocket(socks, 1, "ERROR writing to socket");
	}
}
