#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h>
#include <stdlib.h>

void execute();

/****************************************************************************************/

void detectTetrimino(char grid[20][10], char* type, int* x, int* y, int* state);

/****************************************************************************************/

int softDrop(char grid[20][10], char *type, int *x, int *y, int *state);

/****************************************************************************************/

void clearLines(char grid[20][10]);

/****************************************************************************************/

int checkTop(char grid[20][10]);

/****************************************************************************************/

void initializeGrid(char grid[20][10]) {
	int i, j;

	for(i = 0; i < 20; i++) {
		for(j = 0; j < 10; j++) {
			grid[i][j] = '_';
		}
	}
}

/****************************************************************************************/

void printGrid(char grid[20][10]) {
	int i, j;

	for(i = 0; i < 20; i++) {
		for(j = 0; j < 10; j++) {
			printf("%c ", grid[i][j]);
		}
		printf("\r\n");
	}
}

/****************************************************************************************/

void generateTetrimino(char grid[20][10], char type) {
	int positionX = 4;	

	if ( type == 'O' ) {
		grid[0][positionX] = 'O';
		grid[1][positionX] = 'O';
		grid[0][positionX+1] = 'O';
		grid[1][positionX+1] = 'O';
	}
	else if ( type == 'I' ) {
		grid[0][positionX] = 'I';
		grid[0][positionX+1] = 'I';
	}
	else if ( type == 'C' ) {
		grid[1][positionX] = 'C';
		grid[0][positionX+1] = 'C';
		grid[1][positionX+1] = 'C';
	}
}

/****************************************************************************************/

#endif
