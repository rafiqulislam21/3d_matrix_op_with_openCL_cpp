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