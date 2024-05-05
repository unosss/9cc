#include <stdlib.h>


void alloc(int **P, int w, int x, int y, int z){
	int *heap;
	heap = (int*)malloc(sizeof(int) * 5);
	*P = heap;
	heap[1] = w;
	heap[2] = x;
	heap[3] = y;
	heap[4] = z;	
}
