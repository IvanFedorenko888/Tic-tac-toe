#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#define BUFFER_LENGTH 10240

using namespace std;
#include "Client.h"
#include "Session.h"

int tsock;
int clientCount;
vector<Client> clients; 
vector<Session> sessions;
int createSocket(string port, int len);// создание сокета
void serverOff(int signal);// отключение сервера
void *clientThread(void *args);// функция потока клиентов
int vot = 1;
int main()
{
	signal(SIGINT, serverOff);
	struct sockaddr_in clientAddress;
	unsigned int clientAddressSize = sizeof(clientAddress); // размер адреса клиентов
	
	tsock = createSocket("1231", 5);// создание сокета
	if(tsock < 0)
		return -1;
	
	fd_set rfds;
	int nfds = tsock;
	FD_ZERO(&rfds);
	clientCount = 0;

	while(1)
	{
		FD_SET(tsock, &rfds);
		cout << "Ожидание подключения по протоколу TCP..." << endl;
		if (select(nfds + 1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0))
		{
			if (FD_ISSET(tsock, &rfds))
			{
				int tempSocket = accept(tsock, (struct sockaddr*) &clientAddress, &clientAddressSize);
				if(tempSocket < 0)
					cout << "Ошибка принятия подключения" << endl;
				else
				{	
					/////////создание клиента, запись его в вектор структуры Client////////////
					Client newClient;//
					newClient.identifier = ++clientCount;//
					newClient.socket = tempSocket;//
					newClient.clientAddress = clientAddress;//
					clients.push_back(newClient);//
					cout << " Подключен клиент с id = " << newClient.identifier << "!" << endl;//
					/////////////////////////////////////
					pthread_create(&newClient.thread, NULL, clientThread, &newClient);// создание потока клиента
				}
			}
		}
	}
	return 0;
}

void *clientThread(void *args) {
	Client client = *((Client *)args);
	int vot = 1;
	while(1)
	{
		
		char msg[BUFFER_LENGTH] = {};
		int msgLength = recv(client.socket, &msg, BUFFER_LENGTH, 0);// прием сообщения от клиента
		
		if(msgLength > 0)
		{
			
			string msge = string(msg);
			int begin = msge.find(':');
			string req = msge.substr(0, begin);
			if(vot == 1){
				string name_cl = msge.substr(begin+1,msge.length()-1);
				int c = clients.size();
				for(int i = 0; i < c; i++)
				{
					if(clients.at(i).socket == client.socket)
						clients.at(i).name = name_cl;
				}
				vot = 0;
			}
			if(req == "Request_to_see"){
				int a = sessions.size();
				for(int i = 0; i < a; i++)
				{
					if(sessions.at(i).seePlayer == 0)//если место второго игрока не занято, а у первого свободно, то присоединяем его к первому и отправляем сообщения для начала игры
					{
						sleep(2);
						string viewer = "See_man";
						sessions.at(i).seePlayer = client.socket;
						int c = clients.size();
						string FIRST, SECOND;
						for(int j = 0; j < c; j++)
						{
							if(clients.at(j).socket == sessions.at(i).firstPlayer)
								FIRST = clients.at(j).name;
							if(clients.at(j).socket == sessions.at(i).secondPlayer)
								SECOND = clients.at(j).name;
						}
						string mess_viewer = viewer + "@" + FIRST + "&" + SECOND; 
						send(sessions.at(i).seePlayer, mess_viewer.c_str(), mess_viewer.size(), 0); 	
						cout<<"Сообщение отправлено зрителю: "<<sessions.at(i).seePlayer<<endl;
					}
				}
			}
			if (req == "Request_to_game") { //если получили запрос на игру
				if(sessions.size() == 0)// если вектор сессий пуст, то создаем сессию
				{
						Session newSession;
						newSession.firstPlayer = client.socket;
						newSession.secondPlayer = 0;
						newSession.seePlayer = 0;
						sessions.push_back(newSession);
						cout<<"Сессия создана!"<<endl;
				}
				else{
				int a = sessions.size();
				for(int i = 0; i < a; i++)
				{			
					//cout<<"Количество сессий: "<<sessions.size()<<endl;
					
					if(sessions.at(i).secondPlayer == 0 && sessions.at(i).firstPlayer != 0)//если место второго игрока не занято, а у первого свободно, то присоединяем его к первому и отправляем сообщения для начала игры
					{
						sleep(2);
						string game = "Enjoy_game!";
						sessions.at(i).secondPlayer = client.socket;
						string messg = game + "[" + "1" + "]"; 
						send(sessions.at(i).firstPlayer, messg.c_str(), messg.size(), 0); 	
						cout<<"Сообщение отправлено игроку: "<<	sessions.at(i).firstPlayer<<endl;
						messg = game + "[" + "0" + "]"; 
						send(sessions.at(i).secondPlayer, messg.c_str(), messg.size(), 0);
						cout<<"Сообщение отправлено игроку: "<<	sessions.at(i).secondPlayer<<endl;
					}
					else{//создаем новую сессию
						Session newSessions;
						newSessions.firstPlayer = client.socket;
						newSessions.secondPlayer = 0;
						newSessions.seePlayer = 0;
						sessions.push_back(newSessions);
						cout<<"Сессия создана!"<<endl;
					}
				}
				}
				
			} 
			else { //иначе это ход
				string message = string(msg);
				int begin 	= message.find('{');
				int end 	= message.find('}');
				cout<<begin<<" "<<end<<endl;
				if(begin > 0 && end > 0)
				{
					string sockString = message.substr(begin + 1, end - begin - 1);
					string my_mess = message.substr(0, begin);
					int sockINT = stoi(sockString);
					int c = clients.size();
					string name_client;
					for(int i = 0; i < c; i++)
					{
						if(clients.at(i).socket == client.socket)
							name_client = clients.at(i).name;
					}
					for(int i = 0; i < sessions.size(); i++){
					Session otherSession = sessions.at(i);
					if(otherSession.firstPlayer == client.socket){//если сообщение чата отправил первый игрок, то отправляем его второму
						string for_mess = my_mess + "{" + name_client + "}";
						send(otherSession.secondPlayer, for_mess.c_str(), for_mess.size(), 0);
						cout<<"Сообщение чата "<<otherSession.secondPlayer<<" игроку отправлено."<<endl;
					}
					if(otherSession.secondPlayer == client.socket){//если сообщение чата отправил второму игрок, то отправляем его первый
						string for_mess = my_mess + "{" + name_client + "}";
						send(otherSession.firstPlayer, for_mess.c_str(), for_mess.size(), 0);
						cout<<"Сообщение чата "<<otherSession.firstPlayer<<" игроку отправлено."<<endl;
					}
					}
				}
				else
				for(int i = 0; i < sessions.size(); i++){
					Session otherSession = sessions.at(i);
					if(otherSession.firstPlayer == client.socket){// тоже самое, что в чате
						string moveF = string(msg) + "["+"X"+"]";
						send(otherSession.secondPlayer, moveF.c_str(), moveF.size(), 0);
						cout<<"Ход второму игроку отправлено."<<endl;
						if(otherSession.seePlayer != 0){
							send(otherSession.seePlayer, moveF.c_str(), moveF.size(), 0);
							cout<<"Ход зрителю отправлен."<<endl;
						}
					}
					if(otherSession.secondPlayer == client.socket){
						string moveS = string(msg) + "["+"O"+"]";
						send(otherSession.firstPlayer, moveS.c_str(), moveS.size(), 0);
						cout<<"Ход первому игроку отправлено."<<endl;
						if(otherSession.seePlayer != 0){
							send(otherSession.seePlayer, moveS.c_str(), moveS.size(), 0);
							cout<<"Ход зрителю отправлен."<<endl;
						}
					}
				}
			     }
			}
	}
	return NULL;
}



int createSocket(string port, int len)
{
	int s;
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons((unsigned short)atoi(port.c_str()));

	s = socket(AF_INET, SOCK_STREAM, 0);// создаем сокет TCP
	if(s < 0)
	{
		cout << "Ошибка создания сокета" << endl;
		return -1;
	}
	if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)// проверяем не занят ли сокет
	{
		cout << "Ошибка связывания сокета" << endl;
		return -1;
	}
	if(listen(s, len) < 0)// слушаем клиентов которые хотят подключиться
	{
		cout << "Ошибка прослушивания сокета" << endl;
		return -1;
	}
	return s;
}

void serverOff(int signal)// отключаем сервер
{
	cout << "Сервер был аварийно выключен!" << endl;
	close(tsock);
	exit(0);
}
