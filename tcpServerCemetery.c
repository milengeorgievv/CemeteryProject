#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>
//#include "stdafx.h"
#include <ctype.h>
#include <time.h>



#define PORT 1234

void getInfoByName(int newSocket, char buffer[1024]);

void reserveByRowAndColumn(int newSocket);

int compareDates(char firstDate[11], char secondDate[11]);

char* trim(char* input);

int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[2048]= {'\0'};
	pid_t childpid;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 1234);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		if((childpid = fork()) == 0){
			close(sockfd);
			while(1){
				recv(newSocket, buffer, 1024, 0);
				//Choose one of the options below:\n1.Show information for name\n2.Reserve for 15 years or forever for row and column\n0.Exit\n
				switch(atoi(buffer)) {//NEEDS CHANGE TODO
					case 1:
						bzero(buffer, sizeof(buffer));
						getInfoByName(newSocket, buffer);
					break;
					case 2:
						reserveByRowAndColumn(newSocket);
					break;
					case 0:
						printf("Disconnected from %s:%d\n", 
							inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
					default:
						printf("Unrecognized command from the menu!!!");
						send(newSocket, "Unrecognized command from the menu!!!", 
						strlen("Unrecognized command from the menu!!!"), 0); //in client print menu
					break;
			
				}
				if(strcmp(buffer, "0") == 0){
					break;
				}
				bzero(buffer, sizeof(buffer));
			}
		}

	}

	close(newSocket);


	return 0;
}

void getInfoByName(int newSocket, char buffer[1024]) {
	send(newSocket, "Write name to search for", strlen("Write name to search for"), 0);
	recv(newSocket, buffer, 1024, 0);
	//read from file and get info for name
	int fd;
	char currBuffer[70] = {'\0'};
	char name[38] = {'\0'};
	char dateOfFuneral[11] = {'\0'};
	char dateTillReserved[11] = {'\0'};
	char row[4] = {'\0'};
	char column[4] = {'\0'};
	char returnMessage[1024] = {'\0'};
	char currentMessage[128] = {'\0'};
	fd = open ("testCemetery.txt", O_RDONLY);
	if(fd != -1) {
		while(read(fd, currBuffer, 70) == 70) {
			strncpy(name, currBuffer, 38);
			bool isFound = strstr(name, buffer)?true: false;
			if(!isFound) {
				bzero(currBuffer, sizeof(currBuffer));
				continue;
			} else {
				strncpy(dateOfFuneral, currBuffer+39, 10);
				strncpy(dateTillReserved, currBuffer+50, 10);
				strncpy(row, currBuffer+61, 3);
				strncpy(column, currBuffer+65, 3);
				strcpy(returnMessage, ("Name: %s, Date of funeral: %s, Reserved until: %s, Row: %s, Column: %s", 						name,dateOfFuneral, dateTillReserved, row, column));// right info but doesnt save right in variable
				//strcat(returnMessage, currentMessage); //("Name: %s, Date of funeral: %s, Reserved until: %s, Row: %s, Column: %s", NEEDS FIXING TODO
				//name,dateOfFuneral, dateTillReserved, row, column));
				printf("%s-%s-%s-%s-%s\n",name,dateOfFuneral, dateTillReserved, row, column);
			}
		
			bzero(currBuffer, sizeof(currBuffer));
		}
		close(fd);
	}
	//check if variable is empty there is noone with that name TODO
	
	
	send(newSocket, returnMessage, strlen(returnMessage), 0);
	
	
}

void reserveByRowAndColumn(int newSocket) {
	char currRow[4] = {'\0'};
	char currColumn[4] = {'\0'};
	char currName [38] = {'\0'};
	char currLength[2] = {'\0'};
	send(newSocket, "Write row to reserve for", strlen("Write row to reserve for"), 0);
	recv(newSocket, currRow, 1024, 0);
	send(newSocket, "Write column to reserve for", strlen("Write column to reserve for"), 0);
	recv(newSocket, currColumn, 1024, 0);
	send(newSocket, "Write name to reserve for  ", strlen("Write name to reserve for  "), 0);
	recv(newSocket, currName, 1024, 0);
	send(newSocket, "Write length of reservation: 1. 15 years; 2. forever", strlen("Write length of reservation: 1. 15 years; 2. forever"), 0);
	recv(newSocket, currLength, 1024, 0);
	printf("%s",trim(currLength));
	int fd;
	char currBuffer[70] = {'\0'};
	char name[38] = {'\0'};
	char dateOfFuneral[11] = {'\0'};
	char dateTillReserved[11] = {'\0'};
	char row[4] = {'\0'};
	char column[4] = {'\0'};
	char returnMessage[1024] = {'\0'};
	char currentMessage[128] = {'\0'};
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char currentDate[11] = {tm.tm_mday,'/',tm.tm_mon,'/',tm.tm_year};
	char latestDate[11] = {"01/01/1900"};
	char currLine[70] = {'\0'};
	bool isFree = true;
	fd = open ("testCemetery.txt", O_RDWR);
	if(fd != -1) {
		while(read(fd, currBuffer, 70) == 70) {
			strncpy(row, currBuffer+61, 3);
			strncpy(column, currBuffer+65, 3);
			bool isFound = strcmp(trim(currRow), trim(row)) == 0 && strcmp(trim(currColumn), trim(column)) == 0 ?true: false;
			
			if(!isFound) {
				bzero(currBuffer, sizeof(currBuffer));
				continue;
			} else {
				//check if the date is past due and if not continue
				if(compareDates(latestDate, trim(dateTillReserved)) <= 0) {
				//if dateTillReserved is later 
				} else {
					strcpy(latestDate, dateTillReserved);
				// latestDate = dateTillReserved
				}
				//if yes write in file and save 
				isFree = false;
			}
				
			bzero(currBuffer, sizeof(currBuffer));
		}
		if(!isFree) {
			if(compareDates(latestDate, currentDate) <= 0) {
			//if currentDate is later then write in file and saveS
			//write to file
			char day[3] = {'\0'};
			char month[3] = {'\0'};
			char year[3] = {'\0'};
				if(strcmp(trim(currLength), "1") == 0) {
					//bzero(currLine, sizeof(currLine));
					sprintf(day, "%d", tm.tm_mday);
					sprintf(month, "%d", tm.tm_mon);
					sprintf(year, "%d", tm.tm_year);
					strncat(currLine, currName, strlen(currName));
					strncat(currLine, "|", 2);
					strncat(currLine, currentDate, strlen(currentDate) + 1);
					strncat(currLine, "|", 2);
					strncat(currLine, day, 3);
					strncat(currLine, "/", 2);
					strncat(currLine, month, 3);
					strncat(currLine, "/", 2);
					strncat(currLine, year, 5);
					strncat(currLine, "|", 2);
					strncat(currLine, currRow, strlen(currRow) + 1);
					strncat(currLine, "|", 2);
					strncat(currLine, currColumn, strlen(currColumn) + 1);
					strncat(currLine, "|", 2);
					write(fd, currLine, sizeof(currLine));	
				} else if(strcmp(trim(currLength), "2") == 0){
					//bzero(currLine, sizeof(currLine));
					strncat(currLine, currName, strlen(currName) + 1);
					strncat(currLine, "|", 2);
					strncat(currLine, currentDate, strlen(currentDate) + 1);
					strncat(currLine, "|", 2);
					strncat(currLine, "forever   ", 11);
					strncat(currLine, "|", 2);
					strncat(currLine, currRow, strlen(currRow) + 1);
					strncat(currLine, "|", 2);
					strncat(currLine, currColumn, strlen(currColumn) + 1);
					strncat(currLine, "|", 2);
					write(fd, currLine, sizeof(currLine));	
				}
				send(newSocket, "Your reservation is successful.", strlen("Your reservation is successful."), 0);
			} else {
				send(newSocket, "It is already reserved.", strlen("It is already reserved."), 0);
			}
		} else { 
			// VQRNI DANNI NO PROBLEM S DATITE
			//svoboden e groba
			//write to file
			//if 1 currentDate + 15; if 2 - forever
			if(strcmp(trim(currLength), "1") == 0) {
				//bzero(currLine, sizeof(currLine));
				strncat(currLine, currName, strlen(currName));
				strncat(currLine, "|", 2);
				strncat(currLine, currentDate, strlen(currentDate) + 1);
				/*strncat(currLine, "|", 2);
				strncat(currLine, tm.tm_mday, 3);
				strncat(currLine, "/", 2);
				strncat(currLine, tm.tm_mon, 3);
				strncat(currLine, "/", 2);
				strncat(currLine, (tm.tm_year + 15), 5);*/
				strncat(currLine, "|", 2);
				strncat(currLine, currRow, strlen(currRow) + 1);
				strncat(currLine, "|", 2);
				strncat(currLine, currColumn, strlen(currColumn) + 1);
				strncat(currLine, "|", 2);
				write(fd, currLine, sizeof(currLine));	
			} else if(strcmp(trim(currLength), "2") == 0){
				//bzero(currLine, sizeof(currLine));
				strncat(currLine, currName, strlen(currName) + 1);
				strncat(currLine, "|", 2);
				strncat(currLine, currentDate, strlen(currentDate) + 1);
				strncat(currLine, "|", 2);
				strncat(currLine, "forever   ", 11);
				strncat(currLine, "|", 2);
				strncat(currLine, currRow, strlen(currRow) + 1);
				strncat(currLine, "|", 2);
				strncat(currLine, currColumn, strlen(currColumn) + 1);
				strncat(currLine, "|", 2);
				write(fd, currLine, sizeof(currLine));	
			}					
			send(newSocket, "Your reservation is successful.", strlen("Your reservation is successful."), 0);
		}
		close(fd);
	}	
}

int compareDates(char firstDate[11], char secondDate[11]) {
	char firstYear[5] = {'\0'};
	char firstMonth[3] = {'\0'};
	char firstDay[3] = {'\0'};
	char secondYear[5] = {'\0'};
	char secondMonth[3] = {'\0'};
	char secondDay[3] = {'\0'};
	char forever[10] = {'\0'};
	strcat(forever, "forever");
	if(strstr(firstDate, forever)) {
		return 10;
	}
	if(strstr(secondDate, forever)) {
		return -10;
	}
	
	strncpy(firstYear, firstDate, 4);
	strncpy(firstMonth, firstDate+5, 2);
	strncpy(firstDay, firstDate+8, 2);
	strncpy(secondYear, secondDate, 4);
	strncpy(secondMonth, secondDate+5, 2);
	strncpy(secondDay, secondDate+8, 2);
	

	if(strcmp(firstYear, secondYear) != 0) {
		return strcmp(firstYear, secondYear);
	}
	if(strcmp(firstMonth, secondMonth) != 0) {
		return strcmp(firstMonth, secondMonth);
	} 
	if(strcmp(firstDay, secondDay) !=0) {
		return strcmp(firstDay, secondDay);
	}
	return 0;	
}


char *ltrim(char *s) 
{     
    while(isspace(*s)) s++;     
    return s; 
}  

char *rtrim(char *s) 
{     
    char* back;
    int len = strlen(s);

    if(len == 0)
        return(s); 

    back = s + len;     
    while(isspace(*--back));     
    *(back+1) = '\0';     
    return s; 
}  

char *trim(char *s) 
{     
    return rtrim(ltrim(s));  
} 


