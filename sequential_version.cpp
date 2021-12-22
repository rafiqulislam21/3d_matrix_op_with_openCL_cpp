#include <stdio.h>
#include <math.h>

float A[8192][8192][3];

int main()
{
	//loop to fill in every row
	for(int i=0; i < 8192; i++)
	{
		//loop to fill in every column
		for(int j=0; j < 8192; j++)
		{
			//loop to fill in each 2D matrix
			for(int k=0; k < 3; k++)
			{
				//for matrix k=0
				if(k == 0)
				{
					A[i][j][k] = (float)i / ((float)j + 1.00);
				}
				//for matrix k=1
				else if(k == 1)
				{
					A[i][j][k] = 1.00;
				}
				//for matrix k=2
				else
				{
					A[i][j][k] = (float)j/((float)i+1.00);
				}
			}
		}
	}

	//Iteration count
	for (int t=0; t < 24; t++)
	{
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for(int i=1; i < 8191; i++)
		{
			//each column
			for(int j=0; j < 8192; j++)
			{
				//only matrix k=1 is updated
				A[i][j][1] = A[i][j][1] + (1 / (sqrt(A[i+1][j][0] + A[i-1][j][2])));
			}
		}
	}
}
