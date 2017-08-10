#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <omp.h>

char board[9] = {}; 
//int pc_turn = 0;
int flag = 0;
pthread_t pcTR, mainTR;
int turn_sing = 1, kill_your_self = 1;
pthread_mutex_t lock;
int pcFlag = 0, gameFlag = 0;
void clearboard(){
	for(int i = 0; i < 9; i++)
	{
		board[i] = '-';
	}
}

bool has_wonQ(char player){
 
	int wins[][3] = {{0,1,2}, {3,4,5}, {6,7,8}, {0,3,6}, {1,4,7}, {2,5,8}, {0,4,8}, {2,4,6}};

	for(int i = 0; i<8; i++)
	{
		int count = 0;
		for(int j = 0; j<3; j++)
		{
			if(board[wins[i][j]] == player) 
				count++; 
		}
		if(count == 3)
		{
      			return true;
    		}
  	}
  	return false;
}

void print_board(string indent)
{
		cout << endl;
	        cout<<indent<<"-"<<board[6]<<"-|-"<<board[7]<<"-|-"<<board[8]<<"-\n";
		cout<<indent<<"-"<<board[3]<<"-|-"<<board[4]<<"-|-"<<board[5]<<"-\n";
		cout<<indent<<"-"<<board[0]<<"-|-"<<board[1]<<"-|-"<<board[2]<<"-\n";
}

int get_move(int num)
{
		srand(time(NULL));
		
		int move;
		if(num == 1)
		{
			cout<<"\nПоле ходов: \n";
			cout<<"-7-|-8-|-9-" << endl;
			cout<<"-4-|-5-|-6-" << endl;
			cout<<"-1-|-2-|-3-" << endl;
			cout<<"\n";

			print_board("");
			cout << "\nВаш выбор: "; 
			cin >> move;
			while(move > 9 || move < 1 || board[move-1] != '-')
			{
				cout<<"Пожалуйста введите другое значение (1-9): \n";
				cin >> move;
			}
		}
		else
		{
			move = rand()%9 + 1;
			while(move > 9 || move < 1 || board[move-1] != '-')
				move = rand()%9 + 1;
		}
		return move;
}

void *pcThread(void *arg) {
	sleep(1);
	while(kill_your_self != 0){
		//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if(kill_your_self == 0)
			break;
		if(has_wonQ('X') || has_wonQ('O'))
			break;
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		//pthread_testcancel();
		if(turn_sing < 9){
			pthread_mutex_lock(&lock);
			if(kill_your_self == 0)
			break;
			int move = 0 ;
			move = get_move(2);
             		board[move-1]='O';
			//turn_sing++;
			turn_sing = turn_sing + 1;
			pcFlag = 1;
		pthread_mutex_unlock(&lock);
		if(pcFlag == 1){
			sleep(2);
			pcFlag = 0;
		}
		}
		else
		if(kill_your_self == 0)
			break;
		
	}
	pthread_exit(NULL);
	return NULL;
}


void *play_and_get_winner(void *arg)
{
	kill_your_self = 1;
	flag = 0;
	int qw = 0;
	while(flag != 1)
	{
		//sleep(1);
		if(has_wonQ('O'))
			{
				system("clear");
                		cout << "Извини, но ты проиграл!\n";
				kill_your_self = 0;
                		flag = 1;
				qw = 1;
				break;
				//pthread_exit(0);
				//sleep(2);

             	}
		if(turn_sing == 10 && flag != 1)
		{
			system("clear");
		        cout << "Ничья!\n";
			kill_your_self = 0;
		        flag = 1;
			qw = 1;
			break;
			//sleep(2);
			//pthread_exit(0);
		}
		
		pthread_mutex_lock(&lock);
		if(turn_sing == 1)
			pcFlag = 1;	
		system("clear");         
	        int move = 0 ;
	        system("clear");
			move = get_move(1);
	        	board[move-1]='X';
             		if(has_wonQ('X'))
			{
				system("clear");
                		cout << "Поздравляем!Ты победил!\n";
				kill_your_self = 0;
                		flag = 1;
				turn_sing = turn_sing + 1;
				pthread_mutex_unlock(&lock);	
				break;
				//sleep(2);
				//pthread_exit(0);
				//pthread_exit(&pcTR);
             		}
			turn_sing = turn_sing + 1;
			gameFlag = 1;
          	pthread_mutex_unlock(&lock);	
		if(gameFlag == 1){
			sleep(1);
			gameFlag = 0;
		}
	      	
	}
	//pthread_exit(NULL);
	return NULL;
}

int mainfun(){
	int join_work;
	void * result;
	turn_sing = 1;
	kill_your_self = 1;
	
	clearboard();
	pthread_mutex_init(&lock, NULL);
	//#pragma omp parallel
        //{	
		pthread_create(&mainTR, NULL, play_and_get_winner, NULL);
		pthread_create(&pcTR, NULL, pcThread, NULL);
	//}
	
	pthread_join(pcTR, NULL);
	//if(result == PTHREAD_CANCELED)
	//	cout<<"Через CANCEL";
	
	pthread_join(mainTR, NULL);
	pthread_mutex_destroy(&lock);
	cout<<endl;
	//pthread_join(pcTR, NULL);
        print_board("");
          
	return EXIT_SUCCESS;
}
