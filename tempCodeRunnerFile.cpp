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