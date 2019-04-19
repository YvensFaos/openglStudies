#pragma once

#define printMatrix(matrix) {int _i = -1; \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("----- ----- ----- -----\n");}