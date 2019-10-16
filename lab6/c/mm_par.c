#include <stdio.h>
#include <string.h>
#include <omp.h>

#define N (2048)

float sum;
float a[N][N];
float b[N][N];
float c_trans[N][N];

void matmul()
{
	int	i, j, k;
	#pragma omp parallel private(i,j,k)
	#pragma omp for schedule(static, N/omp_get_num_procs())
	for (i = 0; i < N; i += 1) {
		for (j = 0; j < N; j += 1) {
			a[i][j] = 0;
			for (k = 0; k < N; k += 1) {
				a[i][j] += b[i][k] * c_trans[j][k];
			}
		}
	}
}

void init()
{

	int	i, j;
	#pragma omp parallel private(i,j)
	#pragma omp for schedule(static, N/omp_get_num_procs())
	for (i = 0; i < N; i += 1) {
		for (j = 0; j < N; j += 1) {
			b[i][j] = 12 + i * j * 13;
			c_trans[j][i] = -13 + i + j * 21;
		}
	}
}

void check()
{
	int	i, j;

	for (i = 0; i < N; i += 1)
		for (j = 0; j < N; j += 1)
			sum += a[i][j];
	printf("sum = %lf\n", sum);
}

int main()
{

	int nthread = 8;
	omp_set_num_threads(nthread);
	init();
	matmul();
	check();

	return 0;
}
