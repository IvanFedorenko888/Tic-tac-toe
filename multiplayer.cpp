#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#define BUFFER_LENGTH 10240

using namespace std;
#include "Client.h"// заголовочный файл, который хранит структуру клиента
#include "Session.h"// хранит структуру сессий
#include "sing_pl.cpp"// одиночный решим H2M

int wwM = 1, ww = 1, eg = 1, multi = 1; // флаги
char U; // прием Y - начать игру заного, N закончить сетевую игру
int connectToServerTCP(string host, string port); //подключение к серверу
bool sendToServerMoveTCP(int socket, string message);//отправка хода серверу
bool requestToChat(int socket, string name);//запрос на игру и получение хода 1 - первый, 2 - второй.
bool has_wonQM(char player); // кто победил
void print_boardM(string indent);// показ поля
int get_moveM(int socket);// делаем ход
void clearboardM();// чистим стол, ставим в массив "-"
void *requestEnemy (void *args);// поток постоянного запроса сообщений от сервера и их парс, если "["или"]", то ход, если "{" "}" чат
bool RequestToSee(int socket);
void *Lets_see (void *args);

string FIRST, SECOND;
int sock, ch = 0; //сокет
bool turn;// ход. В зависимости от того, кто подключился первый и занял в сессии firstPlayer, у того будет - true, иначе - false. После сделанного хода меняем у первого на - false и т.д
int you, ph = 0;// у первого you = 1, у второго you = 2. ch - флаг
char boardM[9] = {}; // игровое поле
pthread_t checkThread, seeThread;// поток
int main(int argc, char **argv)
{
	int selection;// выбор игры
	while(ww!=0){
		system("clear"); // чистим терминал
		cout<<"Добро пожаловать в игру 'Крестики-Нолики!'"<<endl;
		cout<<"------------------------------------------"<<endl;
		cout<<"Одиночная игра - (1)"<<endl;
		cout<<"Сетевая игра - (2)"<<endl;
		cout<<"Режим зрителя - (3)"<<endl;
		cout<<"Выйти из игры - (0)"<<endl;		
		cout<<"Ваш выбор: "; cin>>selection;
		switch(selection)
		{
			case 1:{ 
				eg = 1;
				while(eg != 0)// если N, меняем флаг 
				{
					mainfun();
					while(1)//если Y, то break этот цикл и начинаем цикл while(eg != 0) заново
					{
						cout<<"Хотите сыграть еще раз? Yes - (Y), No - (N)"; cin>>U;
						if(U == 'Y')
							break;
						if(U == 'N')
						{
							eg = 0;
							break;
						}
					}
				}
				
				break;		
				}
			case 2:{
				multi = 1;
				while(multi != 0)
				{
					string host = string("127.0.0.1");// наш адрес
					string port = string("1231");// наш порт

					sock = connectToServerTCP(host, port);// подключаемся к серверу
					if(sock < 0)
						return -1;// ошибка сокета
					else
					{
						cout << "Введите ваше имя: ";
						string name;
						cin >> name;
						requestToChat(sock, name); //делаем запрос на сетевую игру
						cout << "Установлено соединение с сервером по адресу " << host << ":" << port << endl;
						int turnI = 1;//количество сделаных ходов
						clearboardM();// чистим поле
						pthread_create(&checkThread, NULL, requestEnemy, NULL);//создание потока прослушивания сервера
						while(!has_wonQM('X') && !has_wonQM('O') && turnI != 10) {
						
						if(turn == true)
						{
							
							system("clear"); 
							//char mess[80]={};
							int move = 0;
				
							move = get_moveM(sock);// делаем ход
							if(you == 1)
							{
								boardM[move-1]='X';// ставим на поле крестик
								if(has_wonQM('X')){// если выйграл X , то поздравление
									system("clear"); 
									print_boardM("");//вывод поля
                							cout << "Поздравляем!Ты победил!\n";
									string message = to_string(move);
									sendToServerMoveTCP(sock, message); 
            							}
								/*else{//если проиграл
									system("clear"); 
									print_board("");//вывод поля
									cout << "Извини, но ты проиграл!\n";
								}*/
							}
							else
							{
								boardM[move-1]='O';//если выиграл O, то поздравление
								if(has_wonQM('O')){
									system("clear"); 
									print_boardM("");//вывод поля
                							cout << "Поздравляем!Ты победил!\n";
									string message = to_string(move);
									sendToServerMoveTCP(sock, message); 
             							}
								/*else{// если проиграл
									system("clear"); 
									print_board("");//вывод поля
									cout << "Извини, но ты проиграл!\n";
								}*/
							}
							string message = to_string(move);
							sendToServerMoveTCP(sock, message); //отправляем серверу наш ход
							turn = false;// меняем нас на ожидание хода противника
						}
						else//ожидание хода противника
						{ 
							ch = 0;
				
							while(ch != 1)
							{
								int Vib;
								system("clear"); 
								cout<<"\nПоле ходов: \n";
								cout<<"-7-|-8-|-9-" << endl;
								cout<<"-4-|-5-|-6-" << endl;
								cout<<"-1-|-2-|-3-" << endl;
								cout<<"\n";
	
								print_boardM("");//выводим поле
								cout<<"1 - Обновить."<<endl;
								cout<<"2 - Чат."<<endl;
								cout<<"Ваш выбор: "; cin>>Vib;
								switch(Vib)
								{
									case 1:{
										break;	//проходим if и условия, т.к может игрок отправил нам сообщение	
									}
									case 2:{//здесь осуществляется отправка сообщения
										string chat;
										char buffer[80] = {};
										sprintf(buffer, "%d", sock);
										cout<<"Введите ваше сообщение: "; cin>>chat;
										string chatMSG = chat + "{" + string(buffer) + "}";
										send(sock, chatMSG.c_str(), chatMSG.size(), 0);
										break;
									}
								}	
							}
							if(you != 1)// если мы второй игрок, то проверяем выиграли ли мы, если да то поздравление, иначе поражение
							{
								if(turnI != 9)
								if(!has_wonQM('O')){ 
									system("clear"); 
									print_boardM("");
									cout << "Извини, но ты проиграл!\n";
            							}
								/*else{
									system("clear"); 
									print_boardM("");
                							cout << "Поздравляем!Ты победил!\n";
								}*/
							}
							else// тоже самое для первого игрока
							{
								if(!has_wonQM('X')){
									system("clear");
									print_boardM("");
									cout << "Извини, но ты проиграл!\n";
             							}
								/*else{
									system("clear"); 
									print_boardM("");
                							cout << "Поздравляем!Ты победил!\n";
								}*/
							}
							turn = true;
						}
						turnI++;//увеличиваем ход
						
						
						}			
					if(turnI == 10){// ничья
						system("clear");
						print_boardM("");
        					cout << "\t Ничья!\n"; 
      					}
					}
					multi = 1;
					while(1)
					{
						cout<<"Хотите сыграть еще раз? Yes - (Y), No - (N)"; cin>>U;
						if(U == 'Y')
							break;
						if(U == 'N')
						{
							multi = 0;
							break;
						}
					}
					}
					
				break;
				}
			case 3:{
				string host = string("127.0.0.1");// наш адрес
				string port = string("1231");// наш порт

				sock = connectToServerTCP(host, port);// подключаемся к серверу
				if(sock < 0)
						return -1;// ошибка сокета
				else
				{
				
					if(RequestToSee(sock) == false)
					{
						cout<<"Нет доступных сессий!";		
					}
					else{
						cout << "Установлено соединение с сервером по адресу " << host << ":" << port << endl;
						clearboardM();// чистим поле
						pthread_create(&seeThread, NULL, Lets_see, NULL);//создание потока прослушивания сервера
						while(1)
						{
							system("clear"); 
							cout<<"X - "<<FIRST<<" _____ O - "<<SECOND<<endl;
							print_boardM("");
							if(has_wonQM('O')){ 
									system("clear"); 
									print_boardM("");
									cout << "Победил игрок с именем "<<SECOND<<"!\n";
									ph = 1; 
									break;
            							}
							if(has_wonQM('X')){ 
									system("clear"); 
									print_boardM("");
									cout << "Победил игрок с именем "<<FIRST<<"!\n";
									ph = 1;
									break;
            							}
							sleep(1);
						}
						pthread_join(seeThread, NULL);
					}
				}
				break;
			}
			case 0:{// выходим из приложения
					ww = 0;
					break;
				}
		}
	}
	
	return 0;
}

void *Lets_see (void *args) {
	while(true){
		int cell;
		char reply[80] = {};
		if(recv(sock, &reply, BUFFER_LENGTH, 0) < 0)
		{
			cout << "Не удалось получить данные от сервера." << endl;
		}
		else {
			string message = string(reply);
			int begin 	= message.find('[');///////////////////
			int end 	= message.find(']');///////////////////здесь парсим
			if(begin > 0 && end > 0){// если значение >0, то выполнится условие, иначе будет -1 и мы примим сообщение как чат
				string numMove = message.substr(0, begin);// вытаскиваем отсюда номер хода
				string who_Move = message.substr(begin+1, end - begin - 1);// вытаскиваем отсюда номер хода
				cell = stoi(numMove); //вот тут будет храниться айди того клиента, кто это сообщение отправил
				if(who_Move == "X")
				{
					boardM[cell-1]='X';//записываем в поле X
				}
				else
				{
					boardM[cell-1]='O';//записываем в поле O
				}
				
			}	
			if(ph == 1)
				break;
		}
	}
	pthread_exit(NULL);
	return NULL;
}

bool RequestToSee(int socket)
{
	string message = "Request_to_see:";
	if(send(socket, message.c_str(), message.size(), 0) < 0)//отправляем сообщение о том, что хотим играть
	{
		cout << "Не удалось отправить данные серверу" << endl;
		return -1;
	}
	char see[80] = {};
	if(recv(socket, &see, BUFFER_LENGTH, 0) <0)
	{
		cout << "Не удалось получить данные от сервера." << endl;
		return -1;
	}
	else
		cout << "От сервера получено: \"" << see << "\"" << endl;
	string done = string(see);	
	int begin 	= done.find('@');
	int end 	= done.find('&');
	string SEE_MAN = done.substr(0, begin);// получили информацию о ходе
	FIRST = done.substr(begin+1, end - begin - 1);
	SECOND = done.substr(end+1, done.length()-1);
	if(SEE_MAN == "See_man")
		return true;
	else
		return false;
	
}

void clearboardM(){// заполняем поле "-"
  for(int i = 0; i<9; i++){
    boardM[i] = '-';
  }
}

bool has_wonQM(char player){// кто победил
  
  int wins[][3] = {{0,1,2}, {3,4,5}, {6,7,8}, {0,3,6}, {1,4,7}, // комбинации выиграшей
                    {2,5,8}, {0,4,8}, {2,4,6}};

  for(int i = 0; i<8; i++){
    int count = 0;
    for(int j = 0; j<3; j++){
      if(boardM[wins[i][j]] == player)// player = X или O, в зависимости от того, что отправили 
        count++; 
    }
    if(count == 3){// если найдено 3 одинаковых символа, X или O, по возвращаем победу.
      return true;
    }
  }
  return false;
}

void print_boardM(string indent){// вывод игрового поля
        cout << endl;
        cout<<indent<<"-"<<boardM[6]<<"-|-"<<boardM[7]<<"-|-"<<boardM[8]<<"-\n";
	cout<<indent<<"-"<<boardM[3]<<"-|-"<<boardM[4]<<"-|-"<<boardM[5]<<"-\n";
	cout<<indent<<"-"<<boardM[0]<<"-|-"<<boardM[1]<<"-|-"<<boardM[2]<<"-\n";
}

int get_moveM(int socket){//функция хода
	srand(time(NULL));
	int move = 0, chose;
	
	while(move == 0){
	string chat;
	system("clear"); 
	cout<<"\n Поле ходов: \n";
	cout<<"-7-|-8-|-9-" << endl;
	cout<<"-4-|-5-|-6-" << endl;
	cout<<"-1-|-2-|-3-" << endl;
	cout<<"\n";

	print_boardM("");// выводим поле
	
	cout<<"1 - Ход."<<endl;
	cout<<"2 - Чат."<<endl;
	cout<<"Ваш выбор: ";	cin>>chose;
	switch(chose)
	{
			case 1:{
				system("clear"); 
				cout<<"\n Поле ходов: \n";
				cout<<"-7-|-8-|-9-" << endl;
				cout<<"-4-|-5-|-6-" << endl;
				cout<<"-1-|-2-|-3-" << endl;
				cout<<"\n";

				print_boardM("");
				cout << "\nВаш ход: "; 
				cin >> move; // вводим наш ход
				while(move > 9 || move < 1 || boardM[move-1] != '-'){// если выбранная ячейка занята или превышает значение поля, то просим ввести заново
					cout<<"Пожалуйста введите другое значение (1-9): \n";
					cin >> move;
				}
				break;		
			}
			case 2:{
				char buffer[80] = {};
				sprintf(buffer, "%d", socket);
				system("clear"); 
				cout<<"\n Поле ходов: \n";
				cout<<"-7-|-8-|-9-" << endl;
				cout<<"-4-|-5-|-6-" << endl;
				cout<<"-1-|-2-|-3-" << endl;
				cout<<"\n";

				print_boardM("");
				cout<<"Введите ваше сообщение: "; cin>>chat;// вводим сообщение
				string chatMSG = chat + "{" + string(buffer) + "}";// {номер сокета сервера}
				if(send(socket, chatMSG.c_str(), chatMSG.size(), 0) < 0)// отправляем серверу
				{
					cout << "Не удалось отправить данные серверу" << endl;
				}	
				break;
			}
		}
	}
	return move;
}

void *requestEnemy (void *args) {// поток прослушивания сервера
		
	while(true){
		int cell;
		char reply[80] = {};
		if(recv(sock, &reply, BUFFER_LENGTH, 0) < 0)
		{
			cout << "Не удалось получить данные от сервера." << endl;
		}
		else {
			string message = string(reply);
			int begin 	= message.find('[');///////////////////
			int end 	= message.find(']');///////////////////здесь парсим
			if(begin > 0 && end > 0){// если значение >0, то выполнится условие, иначе будет -1 и мы примим сообщение как чат
				string numString = message.substr(begin - 1, end - begin - 1);// вытаскиваем отсюда номер хода
				cell = stoi(numString); //вот тут будет храниться айди того клиента, кто это сообщение отправил
				if(you != 1)
				{
					boardM[cell-1]='X';//записываем в поле X
				}
				else
				{
					boardM[cell-1]='O';//записываем в поле O
				}
				ch = 1;
			}
			int begin1 	= message.find('{');///////////////////
			int end1 	= message.find('}');///////////////////здесь парсим
			if(begin1 > 0 && end1 > 0)
			{
				string messe = "Chat: " + message;// чат
				cout << messe << endl; 
			}
			
		}
	}
}

int connectToServerTCP(string host, string port)
{
	struct sockaddr_in sin;// структура адреса сокета
	int s;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)atoi(port.c_str())); // указываем в структуру порт сервера	
	
	s = socket(AF_INET, SOCK_STREAM, 0);// создаем сокет TCP
	if(s < 0)
	{
		cout << "Ошибка создания сокета" << endl;
		return -1;
	}
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)// кодключаемся к серверу
	{
		cout << "Не удалось подключится к серверу" << endl;
		return -1;			
	}
	return s;
}

bool requestToChat(int socket, string name)
{
	string message = "Request_to_game:" + name;
	if(send(socket, message.c_str(), message.size(), 0) < 0)//отправляем сообщение о том, что хотим играть
	{
		cout << "Не удалось отправить данные серверу" << endl;
		return -1;
	}
	
	char game[80] = {};
	wwM = 1;
	while(wwM!=0){// пока к нам не подключится второй игрок будем слушать
	if(recv(socket, &game, BUFFER_LENGTH, 0) <0)
	{
		cout << "Не удалось получить данные от сервера." << endl;
		return -1;
	}
	else
		cout << "От сервера получено: \"" << game << "\"" << endl;
	string message = string(game);
	int begin 	= message.find('[');
	int end 	= message.find(']');
	string turnString = message.substr(begin + 1, end - begin - 1);// получили информацию о ходе
	
	cout<<"turn:"<<turnString<<endl;
	if(turnString == "1"){
		turn = true;
		you = 1;
	}
	else{
		turn = false;
		you = 2;
	}
	string forGame = message.substr(0, begin);//получаем сообщение Enjoy_game!
	cout<<"forGame:"<<forGame<<endl;
	if(forGame == "Enjoy_game!")
	{
		wwM = 0;// заканчиваем цикл ожидания второго игрока
	}
	}
	sleep(3);
	return true;
}

bool sendToServerMoveTCP(int socket, string message)
{
	if(send(socket, message.c_str(), message.size(), 0) < 0)// передаем сообщение серверу
	{
		cout << "Не удалось отправить данные серверу" << endl;
		return false;
	}
	return true;
}
