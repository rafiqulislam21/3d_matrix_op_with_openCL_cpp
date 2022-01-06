#include <stdio.h>
#include <math.h>
#include <time.h>

const int depth = 3;
const int row = 8; //8192
const int column = 8;

float A[depth][row][column];


void showTimeDifference(clock_t t1, clock_t t2){
	// auto start = chrono::high_resolution_clock::now();
    double timeTaken = double(t2 - t1)/CLOCKS_PER_SEC;;
	printf("========================= Time taken = %.3f s ==========================", timeTaken);
}

int main()
{
	clock_t s_t1 = clock();
	//loop to fill in every row
	for (int k = 0; k < depth; k++)
	{
		//loop to fill in every column
		for (int i = 0; i < row; i++)
		{
			//loop to fill in each 2D matrix
			for (int j = 0; j < column; j++)
			{
				//for matrix k=0
				if (k == 0)
				{
					A[k][i][j] = (float)i / ((float)j + 1.00);
				}
				//for matrix k=1
				else if (k == 1)
				{
					A[k][i][j] = 1.00;
				}
				//for matrix k=2
				else
				{
					A[k][i][j] = (float)j / ((float)i + 1.00);
				}
			}
		}
	}

	

	//Iteration count
	for (int t = 0; t < 24; t++)
	{
		// printf("Iteration = %d :\n", t);
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for (int i = 1; i < row; i++)
		{
			//each column
			for (int j = 0; j < column; j++)
			{
				//only matrix k=1 is updated
				A[1][i][j] = A[1][i][j] + (1 / (sqrt(A[0][i + 1][j] + A[2][i - 1][j])));
				// printf("%.2f \t", A[1][i][j]);
			}
			// printf("\n");
		}
		// printf("\n");
	}

	//Display array====================================================================================
	for (int k = 0; k < depth; k++)
	{
		printf("Baseline matrix k = %d (i=0..10, j =0..10):\n", k);
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for (int i = 0; i < row; i++)
		{
			//each column
			for (int j = 0; j < column; j++)
			{
				//only matrix k=1 is updated
				printf("%.2f \t", A[k][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	//Display array=======================================================================================
	clock_t s_t2 = clock();
	// showing total time taken to sequential executation
	printf("Sequential Code: \n");
	showTimeDifference(s_t1, s_t2);
}
