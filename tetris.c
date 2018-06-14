#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "tetris.h"
#include <termios.h>

char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

void old_settings() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
}

typedef struct {
	char* type;
	int* state;
	int* c_x;
	int* c_y;		
}	Info;

pthread_mutex_t mymutex;
char grid[20][10];
int isFalling = 1;
int point = 0;

int hardDrop(char grid[20][10], char* type, int* x, int* y, int* state)
{
	int i,a=0;
	while(i)
	{
		i=softDrop(grid, type, x, y, state);
		a+=1;
	}
	return (a-1);
}


int rotateCW(char grid[20][10], char* type, int* state, int* x, int* y)
{
	int a=0;
	if(*type=='I')
	{
		if(*state==1)
		{
			if(grid[*y+1][*x+1]!='X')
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				*x+=1;
				grid[*y][*x]='I';
				grid[*y+1][*x]='I';
				a+=1;
				*state=2;
			}
		}
		else if(*state==2)
		{
			if(*x!=9 && grid[*y][*x+1]!='X')
			{
				grid[*y][*x]='_';
				grid[*y+1][*x]='_';
				grid[*y][*x]='I';
				grid[*y][*x+1]='I';
				a+=1;
				*state=1;
			}
		}
	}
	else if(*type=='C')
	{
		if(*state==1)
		{
			if(grid[*y+1][*x]!='X')
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				grid[*y+1][*x+1]='_';
				grid[*y][*x+1]='C';
				grid[*y+1][*x]='C';
				grid[*y+1][*x+1]='C';
				a+=1;
				*state=4;
			}
		}
		else if(*state==2)
		{
			if(grid[*y][*x+1]!='X')
			{
				grid[*y][*x]='_';
				grid[*y+1][*x]='_';
				grid[*y+1][*x+1]='_';
				grid[*y][*x]='C';
				grid[*y][*x+1]='C';
				grid[*y+1][*x]='C';
				a+=1;
				*state=3;
			}
		}
		else if(*state==3)
		{
			if(grid[*y+1][*x+1]!='X')
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				grid[*y+1][*x]='_';
				grid[*y][*x]='C';
				grid[*y][*x+1]='C';
				grid[*y+1][*x+1]='C';
				a+=1;
				*state=1;
			}
		}
		else if(*state==4)
		{
			if(grid[*y][*x]!='X')
			{
				grid[*y][*x+1]='_';
				grid[*y+1][*x]='_';
				grid[*y+1][*x+1]='_';
				grid[*y][*x]='C';
				grid[*y+1][*x]='C';
				grid[*y+1][*x+1]='C';
				a+=1;
				*state=2;
			}
		}
	}
	return a;
}

void detectTetrimino(char grid[20][10], char* type, int* x, int* y, int* state)
{
	int i,j,c=0;
	*type='_';
	*x=-1;
	*y=-1;
	*state=-1;

	for(i=0;i<20;i++)
	{
		for(j=0;j<10;j++)
		{
			if(grid[i][j]!='_' && grid[i][j]!='X')
			{
				if(grid[i][j]=='O')
				{
					*type='O';
					*x=j;
					*y=i;
					*state=1;
					c=1;
					break;
				}
				else if(grid[i][j]=='I')
				{
					if(grid[i][j+1]=='I')
					{
						*type='I';
						*x=j;
						*y=i;
						*state=1;
						c=1;
						break;
					}
					else if(grid[i+1][j]=='I')
					{
						*type='I';
						*x=j;
						*y=i;
						*state=2;
						c=1;
						break;
					}
				}
				else if(grid[i][j]=='C')
				{
					if(grid[i][j+1]=='C' && grid[i+1][j+1]=='C')
					{
						*type='C';
						*x=j;
						*y=i;
						*state=1;
						c=1;
						break;
					}
					else if(grid[i+1][j]=='C' && grid[i+1][j+1]=='C')
					{
						*type='C';
						*x=j;
						*y=i;
						*state=2;
						c=1;
						break;
					}
					else if(grid[i][j+1]=='C' && grid[i+1][j]=='C')
					{
						*type='C';
						*x=j;
						*y=i;
						*state=3;
						c=1;
						break;
					}
					else if(grid[i+1][j]=='C' && grid[i+1][j-1]=='C')
					{
						*type='C';
						*x=j-1;
						*y=i;
						*state=4;
						c=1;
						break;
					}
				}
			}
		}
		if(c==1)
			break;
	}
}

int softDrop(char grid[20][10], char *type, int *x, int *y, int *state)
{
	if(*type=='O')
	{
		if(grid[*y+2][*x]=='X' || grid[*y+2][*x+1]=='X' || *y==18)
		{
			grid[*y][*x]='X';
			grid[*y][*x+1]='X';
			grid[*y+1][*x]='X';
			grid[*y+1][*x+1]='X';			
			return 0;
		}
		else
		{
			grid[*y][*x]='_';
			grid[*y][*x+1]='_';
			grid[*y+1][*x]='_';
			grid[*y+1][*x+1]='_';
			(*y)++;
			grid[*y][*x]='O';
			grid[*y][*x+1]='O';
			grid[*y+1][*x]='O';
			grid[*y+1][*x+1]='O';
		}
	}
	else if(*type=='I')
	{
		if(*state==1)
		{
			if(grid[*y+1][*x]=='X' || grid[*y+1][*x+1]=='X' || *y==19)
			{
				grid[*y][*x]='X';
				grid[*y][*x+1]='X';
				return 0;
			}
			else
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				(*y)++;
				grid[*y][*x]='I';
				grid[*y][*x+1]='I';
			}
		}
		else if(*state==2)
		{
			if(grid[*y+2][*x]=='X' || *y==18)
			{
				grid[*y][*x]='X';
				grid[*y+1][*x]='X';
				return 0;
			}
			else
			{
				grid[*y][*x]='_';
				grid[*y+1][*x]='_';
				(*y)++;
				grid[*y][*x]='I';
				grid[*y+1][*x]='I';
			}
		}
	}
	else if(*type=='C')
	{
		if(*state==1)
		{
			if(grid[*y+2][*x+1]=='X' || grid[*y+1][*x]=='X' || *y==18)
			{
				grid[*y][*x]='X';
				grid[*y][*x+1]='X';
				grid[*y+1][*x+1]='X';			
				return 0;
			}
			else
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				grid[*y+1][*x+1]='_';				
				(*y)++;
				grid[*y][*x]='C';
				grid[*y][*x+1]='C';
				grid[*y+1][*x+1]='C';
			}
		}
		else if(*state==2)
		{
			if(grid[*y+2][*x]=='X' || grid[*y+2][*x+1]=='X' || *y==18)
			{
				grid[*y][*x]='X';
				grid[*y+1][*x]='X';
				grid[*y+1][*x+1]='X';			
				return 0;
			}
			else
			{
				grid[*y][*x]='_';
				grid[*y+1][*x]='_';
				grid[*y+1][*x+1]='_';
				(*y)++;
				grid[*y][*x]='C';
				grid[*y+1][*x]='C';
				grid[*y+1][*x+1]='C';
			}
		}
		else if(*state==3)
		{
			if(grid[*y+2][*x]=='X' || grid[*y+1][*x+1]=='X' || *y==18)
			{
				grid[*y][*x]='X';
				grid[*y][*x+1]='X';
				grid[*y+1][*x]='X';			
				return 0;
			}
			else
			{
				grid[*y][*x]='_';
				grid[*y][*x+1]='_';
				grid[*y+1][*x]='_';
				(*y)++;
				grid[*y][*x]='C';
				grid[*y][*x+1]='C';
				grid[*y+1][*x]='C';
			}
		}
		else if(*state==4)
		{
			if(grid[*y+2][*x]=='X' || grid[*y+2][*x+1]=='X' || *y==18)
			{
				grid[*y][*x+1]='X';
				grid[*y+1][*x]='X';
				grid[*y+1][*x+1]='X';			
				return 0;
			}
			else
			{
				grid[*y][*x+1]='_';
				grid[*y+1][*x]='_';
				grid[*y+1][*x+1]='_';
				(*y)++;
				grid[*y][*x+1]='C';
				grid[*y+1][*x]='C';
				grid[*y+1][*x+1]='C';
			}
		}
	}
	return 1;
}

void* align(void * information)
{
	pthread_mutex_lock(&mymutex);
	Info* info = (Info *) information;
	char* type = info->type;
	int* x = info->c_x;
	int* y = info->c_y;
	int* state = info->state;
	char dir;

	dir = getch();
	fflush(stdin);

	do
	{
		if(dir=='s' || dir=='S')
			isFalling = softDrop(grid, type, x, y, state);

		else if(dir=='w' || dir=='W')
		{
			if(isFalling==1)
				rotateCW(grid, type, state, x, y);
		}

		else if(dir=='x' || dir=='X')
			hardDrop(grid, type, x, y, state);

		else if(dir=='d' || dir=='D')
		{
			if(*type=='O')
			{
				if(grid[*y][*x]!='X' && *x<8 && grid[*y][*x+2]!='X' && grid[*y+1][*x+2]!='X')
				{
					grid[*y][*x]='_';
					grid[*y][*x+1]='_';
					grid[*y+1][*x]='_';
					grid[*y+1][*x+1]='_';
					(*x)++;
					grid[*y][*x]='O';
					grid[*y][*x+1]='O';
					grid[*y+1][*x]='O';
					grid[*y+1][*x+1]='O';
				}
			}
			else if(*type=='I')
			{
				if(*state==1)
				{
					if(grid[*y][*x]!='X' && *x<8 && grid[*y][*x+2]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						(*x)++;
						grid[*y][*x+1]='I';
						grid[*y][*x]='I';
					}
				}
				else if(*state==2)
				{
					if(grid[*y][*x]!='X' && *x<9 && grid[*y][*x+1]!='X' && grid[*y+1][*x+1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y+1][*x]='_';
						(*x)++;
						grid[*y][*x]='I';
						grid[*y+1][*x]='I';
					}
				}
			}
			else if(*type=='C')
			{
				if(*state==1)
				{
					if(grid[*y][*x]!='X' && *x<8 && grid[*y][*x+2]!='X' && grid[*y+1][*x+2]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x+1]='_';
						(*x)++;
						grid[*y][*x]='C';
						grid[*y][*x+1]='C';
						grid[*y+1][*x+1]='C';					
					}
				}
				else if(*state==2)
				{
					if(grid[*y][*x]!='X' && *x<8 && grid[*y][*x+1]!='X' && grid[*y+1][*x+2]!='X')
					{
						grid[*y][*x]='_';
						grid[*y+1][*x]='_';
						grid[*y+1][*x+1]='_';
						(*x)++;
						grid[*y][*x]='C';
						grid[*y+1][*x]='C';
						grid[*y+1][*x+1]='C';					
					}
				}
				else if(*state==3)
				{
					if(grid[*y][*x]!='X' && *x<8 && grid[*y][*x+2]!='X' && grid[*y+1][*x+1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x]='_';
						(*x)++;
						grid[*y][*x]='C';
						grid[*y][*x+1]='C';
						grid[*y+1][*x]='C';					
					}
				}
				else if(*state==4)
				{
					if(grid[*y][*x+1]!='X' && *x<8 && grid[*y][*x+2]!='X' && grid[*y+1][*x+2]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x]='_';
						grid[*y+1][*x+1]='_';
						grid[*y][*x+1]='_';
						grid[*y][*x+2]='C';
						grid[*y+1][*x+1]='C';
						grid[*y+1][*x+2]='C';
						(*x)++;					
					}
				}
			}
		}
		else if(dir=='a' || dir=='A')
		{
			if(*type=='O')
			{
				if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X' && grid[*y+1][*x-1]!='X')
				{
					grid[*y][*x]='_';
					grid[*y][*x+1]='_';
					grid[*y+1][*x]='_';
					grid[*y+1][*x+1]='_';
					(*x)--;
					grid[*y][*x]='O';
					grid[*y][*x+1]='O';
					grid[*y+1][*x]='O';
					grid[*y+1][*x+1]='O';
				}
			}
			else if(*type=='I')
			{
				if(*state==1)
				{
					if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X')
					{
						grid[*y][*x+1]='_';
						grid[*y][*x]='_';
						(*x)--;
						grid[*y][*x]='I';
						grid[*y][*x+1]='I';
					}
				}
				else if(*state==2)
				{
					if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X' && grid[*y+1][*x-1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y+1][*x]='_';
						(*x)--;
						grid[*y][*x]='I';
						grid[*y+1][*x]='I';
					}
				}
			}
			else if(*type=='C')
			{
				if(*state==1)
				{
					if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X' && grid[*y+1][*x]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x+1]='_';
						(*x)--;
						grid[*y][*x]='C';
						grid[*y][*x+1]='C';
						grid[*y+1][*x+1]='C';					
					}
				}
				else if(*state==2)
				{
					if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X' && grid[*y+1][*x-1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y+1][*x]='_';
						grid[*y+1][*x+1]='_';
						(*x)--;
						grid[*y][*x]='C';
						grid[*y+1][*x]='C';
						grid[*y+1][*x+1]='C';					
					}
				}
				else if(*state==3)
				{
					if(grid[*y][*x]!='X' && *x>0 && grid[*y][*x-1]!='X' && grid[*y+1][*x-1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x]='_';
						(*x)--;
						grid[*y][*x]='C';
						grid[*y][*x+1]='C';
						grid[*y+1][*x]='C';					
					}
				}
				else if(*state==4)
				{
					if(grid[*y][*x+1]!='X' && *x>0 && grid[*y][*x]!='X' && grid[*y+1][*x-1]!='X')
					{
						grid[*y][*x]='_';
						grid[*y][*x+1]='_';
						grid[*y+1][*x]='_';
						grid[*y+1][*x+1]='_';
						grid[*y][*x-1]='_';
						grid[*y][*x]='C';
						grid[*y+1][*x-1]='C';
						grid[*y+1][*x]='C';
						(*x)--;					
					}
				}
			}
		}

	} while (dir=getch());

	pthread_mutex_unlock(&mymutex);
}

void clearLines(char grid[20][10])
{
	int i,j,k;
	for(i=0;i<20;i++)
	{
		for(j=0;j<10;j++)
		{
			if(grid[i][j]!='X')
				break;
		}
		if(j==10)
		{
			for(k=i-1;k>-1;k--)
			{
				for(j=9;j>-1;j--)
				{
					grid[k+1][j]=grid[k][j];
				}
			}
			for(j=0;j<10;j++)
			{
				grid[0][j]='_';
			}
			point++;
		}
	}
}

int checkTop(char grid[20][10])
{
	int i;
	for(i=0;i<10;i++)
	{
		if(grid[0][i]=='X')
			return 1;
	}
	return 0;
}

void execute() {}

int main() {
	
	char type;
	int x, y, state,i;
	int randomTetrimino;
	char stepChar;

	Info information;
	pthread_attr_t attr;

	pthread_mutex_init(&mymutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	time_t t;
	srand((unsigned) time(&t));

	initializeGrid(grid);
	generateTetrimino(grid, 'O');
	detectTetrimino(grid, &type, &x, &y, &state);
	isFalling = 1;
	i=0;

	while( !(checkTop(grid) ) ) {
		isFalling = softDrop(grid, &type, &x, &y, &state);
				
		if ( isFalling == 0 ) {
			clearLines(grid);
			randomTetrimino = rand() % 10;
			if ( randomTetrimino > 6 ) {
				generateTetrimino(grid, 'C');
			}
			else if ( randomTetrimino < 3 ) {
				generateTetrimino(grid, 'I');
			}
			else {
				generateTetrimino(grid, 'O');
			}
			detectTetrimino(grid, &type, &x, &y, &state);
			isFalling = 1;
		}
		information.type = &type;
		information.state = &state;
		information.c_x = &x;
		information.c_y = &y;	
		
		pthread_t thread;
		pthread_create(&thread, &attr, align, (void *) &information);		

		printf("X : %d - Y : %d - State : %d => POINT : %d\n", x, y, state, point);

		printGrid(grid);
		printf("========================================\n");
		
			
		usleep(500000);
		i++;
	}


	old_settings();

	return 0;
}
