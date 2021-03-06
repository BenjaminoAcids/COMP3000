#include "MVM.h"

int main()
{

	struct timespec start, end;
	uint64_t diff;
	double gflops;
	int outcome;

	initialization_MVM();
	
	clock_gettime(CLOCK_MONOTONIC, &start);

	for(int i = 0; i < TIMES; i++)
	{
		//MVM_AVX();
		//MVM_OMP();
		//MVM_SSE();
		//MVM_SIMD();
		//MVM_regBlock_2();
		//MVM_regBlock_8();
		//MVM_regBlock_13();
		//MVM_regBlock_16();
		//MVM_Looptiling_Default();
		MVM_Looptiling();
		//MVM_AVX_TILE();
		//MVM_SSE_REG_8();
		//MVM_SSE_REG_8_OMP(4);
		//MVM_SSE_REG_16();
		//MVM_SSE_REG_16_OMP(4);
		//MVM_AVX_REG_4();
		//MVM_AVX_REG_8();
		//MVM_AVX_REG_8_OMP(4);
		//MVM_AVX_REG_13();
		//MVM_AVX_REG_16();
		//MVM_AVX_REG_16_OMP(4);
		//MVM_AVX_REG_OMP();
		//MVM_AVX_REG_OMP_TILE();
		//MVM_Test();
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	gflops = (double) ARITHMETICAL_OPS / (diff / TIMES);
	printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
	printf("elapsed time = %llu mseconds\n", (long long unsigned int) diff/1000000);
	printf("%f GigaFLOPS achieved\n", gflops);
	
	outcome = Compare_MVM();	

	if (outcome == 0)
		printf("\n\n\r -----  output is correct -----\n\r");
	else
		printf("\n\n\r ----- output is INcorrect -----\n\r");

	return 0;

}

void initialization_MVM() {

	float e = 0.1234, p = 0.7264, r = 0.11;

	for (unsigned int i = 0; i != M; i++)
		for (unsigned int j = 0; j != M; j++)
			A1[i][j] = ((i - j) % 9) + p;

	for (unsigned int j = 0; j != M; j++) {
		Y[j] = 0.0;
		test1[j] = 0.0;
		X[j] = (j % 7) + r;
	}
}


unsigned short int MVM_AVX() {

	
	float tmp;
	int i, j;
	

	__m256 ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, num0, num1, num2, num3, num4, num5;
	__m128 xmm1, xmm2;
	
	for (i = 0; i < M; i++) 
	{
		num1 = _mm256_setzero_ps();

		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			num5 = _mm256_load_ps(X + j);
			num0 = _mm256_load_ps(&A1[i][j]);
			num1 = _mm256_fmadd_ps(num0, num5, num1);
		}

		ymm2 = _mm256_permute2f128_ps(num1, num1, 1);
		num1 = _mm256_add_ps(num1, ymm2);
		num1 = _mm256_hadd_ps(num1, num1);
		num1 = _mm256_hadd_ps(num1, num1);
		xmm2 = _mm256_extractf128_ps(num1, 0);
		_mm_store_ss(Y + i, xmm2);


	}

	return 1;
}

unsigned short int MVM_OMP()
{

	int i, j;
	
	#pragma omp parallel for private(i,j) shared(A1,X) reduction(+:Y)
	for (i = 0; i < M; i++) 
	{	
		for (j = 0; j < M; j++) 
		{
			Y[i] += A1[i][j] * X[j];
		}
	}

	return 1;
} 

unsigned short int MVM_SSE()
{
__m128 num0, num1, num2, num3, num4, num5, num6;

	for (int i = 0; i < M; i++) {

		num3 = _mm_setzero_ps();
		for (int j = 0; j < M; j += 4) { 

			num0 = _mm_load_ps(&A1[i][j]);
			num1 = _mm_load_ps(&X[j]);
			num3 = _mm_fmadd_ps(num0, num1, num3);
		}

		num4 = _mm_hadd_ps(num3, num3);
		num4 = _mm_hadd_ps(num4, num4);

		_mm_store_ss(&Y[i], num4);
	}

	return 1;
}


unsigned short int MVM_SIMD()
{
	int i, j;
	
	#pragma omp parallel for private(i,j) shared(A1,X) reduction(+:Y)
	for (i = 0; i < M; i++) 
	{
		#pragma omp simd reduction(+:Y) aligned(Y,A1,X:64)
		for (j = 0; j < M; j++) 
		{
			Y[i] += A1[i][j] * X[j];
		}
	}

	return 1;
}

unsigned short int MVM_regBlock_2()
{
	double y0, y1, x0; 	
	
	for (int i = 0; i < M; i+=2) {
		y0 = Y[i];
		y1 = Y[i + 1];
		for (int j = 0; j < M; j++) {
			x0 = X[j];
			y0 += A1[i][j] * x0;
			y1 += A1[i+1][j] * x0;
		}
		Y[i] += y0;
		Y[i+1] +=y1;
	}

	return 1;
}

unsigned short int MVM_regBlock_8()
{
	double y0, y1, y2, y3, y4, y5, y6, y7, x0; 	
	
	for (int i = 0; i < M; i+=8) {
		y0 = Y[i];
		y1 = Y[i + 1];
		y2 = Y[i + 2];
		y3 = Y[i + 3];
		y4 = Y[i + 4];
		y5 = Y[i + 5];
		y6 = Y[i + 6];
		y7 = Y[i + 7];
		
		for (int j = 0; j < M; j++) {
			x0 = X[j];
			y0 += A1[i][j] * x0;
			y1 += A1[i+1][j] * x0;
			y2 += A1[i+2][j] * x0;
			y3 += A1[i+3][j] * x0;
			y4 += A1[i+4][j] * x0;
			y5 += A1[i+5][j] * x0;
			y6 += A1[i+6][j] * x0;
			y7 += A1[i+7][j] * x0;
		}
		Y[i] += y0;
		Y[i+1] +=y1;
		Y[i+2] +=y2;
		Y[i+3] +=y3;
		Y[i+4] +=y4;
		Y[i+5] +=y5;
		Y[i+6] +=y6;
		Y[i+7] +=y7;
	}

	return 1;
}

unsigned short int MVM_regBlock_13()
{
	double y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, x0; 	
	
	for (int i = 0; i < M; i+=13) {
		y0 = Y[i];
		y1 = Y[i + 1];
		y2 = Y[i + 2];
		y3 = Y[i + 3];
		y4 = Y[i + 4];
		y5 = Y[i + 5];
		y6 = Y[i + 6];
		y7 = Y[i + 7];
		y8 = Y[i + 8];
		y9 = Y[i + 9];
		y10 = Y[i + 10];
		y11 = Y[i + 11];
		y12 = Y[i + 12];
		
		for (int j = 0; j < M; j++) {
			x0 = X[j];
			y0 += A1[i][j] * x0;
			y1 += A1[i+1][j] * x0;
			y2 += A1[i+2][j] * x0;
			y3 += A1[i+3][j] * x0;
			y4 += A1[i+4][j] * x0;
			y5 += A1[i+5][j] * x0;
			y6 += A1[i+6][j] * x0;
			y7 += A1[i+7][j] * x0;
			y8 += A1[i+8][j] * x0;
			y9 += A1[i+9][j] * x0;
			y10 += A1[i+10][j] * x0;
			y11 += A1[i+11][j] * x0;
			y12 += A1[i+12][j] * x0;
		}
		Y[i] += y0;
		Y[i+1] +=y1;
		Y[i+2] +=y2;
		Y[i+3] +=y3;
		Y[i+4] +=y4;
		Y[i+5] +=y5;
		Y[i+6] +=y6;
		Y[i+7] +=y7;
		Y[i+8] +=y8;
		Y[i+9] +=y9;
		Y[i+10] +=y10;
		Y[i+11] +=y11;
		Y[i+12] +=y12;
	}

	return 1;
}

unsigned short int MVM_regBlock_16()
{
	double y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, y13, y14, y15, y16, x0; 	
	
	for (int i = 0; i < M; i+=16) {
		y0 = Y[i];
		y1 = Y[i + 1];
		y2 = Y[i + 2];
		y3 = Y[i + 3];
		y4 = Y[i + 4];
		y5 = Y[i + 5];
		y6 = Y[i + 6];
		y7 = Y[i + 7];
		y8 = Y[i + 8];
		y9 = Y[i + 9];
		y10 = Y[i + 10];
		y11 = Y[i + 11];
		y12 = Y[i + 12];
		y13 = Y[i + 13];
		y14 = Y[i + 14];
		y15 = Y[i + 15];
		
		for (int j = 0; j < M; j++) {
			x0 = X[j];
			y0 += A1[i][j] * x0;
			y1 += A1[i+1][j] * x0;
			y2 += A1[i+2][j] * x0;
			y3 += A1[i+3][j] * x0;
			y4 += A1[i+4][j] * x0;
			y5 += A1[i+5][j] * x0;
			y6 += A1[i+6][j] * x0;
			y7 += A1[i+7][j] * x0;
			y8 += A1[i+8][j] * x0;
			y9 += A1[i+9][j] * x0;
			y10 += A1[i+10][j] * x0;
			y11 += A1[i+11][j] * x0;
			y12 += A1[i+12][j] * x0;
			y13 += A1[i+13][j] * x0;
			y14 += A1[i+14][j] * x0;
			y15 += A1[i+15][j] * x0;
		}
		Y[i] += y0;
		Y[i+1] +=y1;
		Y[i+2] +=y2;
		Y[i+3] +=y3;
		Y[i+4] +=y4;
		Y[i+5] +=y5;
		Y[i+6] +=y6;
		Y[i+7] +=y7;
		Y[i+8] +=y8;
		Y[i+9] +=y9;
		Y[i+10] +=y10;
		Y[i+11] +=y11;
		Y[i+12] +=y12;
		Y[i+13] +=y13;
		Y[i+14] +=y14;
		Y[i+15] +=y15;
	}

	return 1;
}

unsigned short int MVM_Looptiling_Default()
{
	for(int ii = 0; ii < M; ii += TILEA)
	{
		for(int jj = 0; jj <M; jj += TILEB)
		{
			for(int i = ii; i < MIN(M, ii + TILEA); i++)
			{
				for(int j = jj; j < MIN(M, jj + TILEB); j++)
				{
					Y[i] += A1[i][j] * X[j];
				}
			}
		}
	}
	
	return 1;
}

unsigned short int MVM_Looptiling()
{

	//int i, j, ii, jj;
	
	
	#pragma omp parallel 
	{
		float temp1[M] __attribute__((aligned(64))) = {0};
		#pragma omp for collapse(2)
		for(int ii = 0; ii < M; ii += TILEA)
		{
			for(int jj = 0; jj < M; jj += TILEB)
			{
				
				for(int i = ii; i < MIN(M, ii + TILEA); i++) 
				{
					#pragma omp simd reduction(+:temp1) aligned(temp1,A1,X:64)
					for(int j = jj; j < MIN(M, jj + TILEB); j++) 
					{
						temp1[i] += A1[i][j] * X[j];
					}
				}
			}
		}
		
		for(int i = 0; i < M; i++)
		{
			#pragma omp atomic
			Y[i] += temp1[i];
		}
	}
	
	return 1;
}

unsigned short int MVM_AVX_TILE()
{
	double y0, y1, y2, y3, y4, y5, y6, y7, x0; 
	
	for(int ii = 0; ii < M; ii += TILEA)
	{
		for(int jj = 0; jj <M; jj += TILEB)
		{
			for(int i = ii; i < MIN(M, ii + TILEA); i+8)
			{
				y0 = Y[i];
				y1 = Y[i + 1];
				y2 = Y[i + 2];
				y3 = Y[i + 3];
				y4 = Y[i + 4];
				y5 = Y[i + 5];
				y6 = Y[i + 6];
				y7 = Y[i + 7];
				
				for(int j = jj; j < MIN(M, jj + TILEB); j++)
				{
					x0 = X[j];
					y0 += A1[i][j] * x0;
					y1 += A1[i+1][j] * x0;
					y2 += A1[i+2][j] * x0;
					y3 += A1[i+3][j] * x0;
					y4 += A1[i+4][j] * x0;
					y5 += A1[i+5][j] * x0;
					y6 += A1[i+6][j] * x0;
					y7 += A1[i+7][j] * x0;
				}
				
				Y[i] += y0;
				Y[i+1] +=y1;
				Y[i+2] +=y2;
				Y[i+3] +=y3;
				Y[i+4] +=y4;
				Y[i+5] +=y5;
				Y[i+6] +=y6;
				Y[i+7] +=y7;
			}	
		}
	}
	
	return 1;
}

unsigned short int MVM_SSE_REG_8()
{
	__m128 a0, a1, a3, a4, b0, b3, b4, c0, c3, c4, d0, d3, d4, e0, e3, e4, f0, f3, f4, g0, g3, 			g4, h0, h3, h4;

	for (int i = 0; i < M; i+=8) {

		a3 = _mm_setzero_ps();
		b3 = _mm_setzero_ps();
		c3 = _mm_setzero_ps();
		d3 = _mm_setzero_ps();
		e3 = _mm_setzero_ps();
		f3 = _mm_setzero_ps();
		g3 = _mm_setzero_ps();
		h3 = _mm_setzero_ps();
		
		for (int j = 0; j < M; j += 4) { 
			a1 = _mm_load_ps(&X[j]);
			
			a0 = _mm_load_ps(&A1[i][j]);
			a3 = _mm_fmadd_ps(a0, a1, a3);
			
			b0 = _mm_load_ps(&A1[i+1][j]);
			b3 = _mm_fmadd_ps(b0, a1, b3);
			
			c0 = _mm_load_ps(&A1[i+2][j]);
			c3 = _mm_fmadd_ps(c0, a1, c3);
			
			d0 = _mm_load_ps(&A1[i+3][j]);
			d3 = _mm_fmadd_ps(d0, a1, d3);
			
			e0 = _mm_load_ps(&A1[i+4][j]);
			e3 = _mm_fmadd_ps(e0, a1, e3);
			
			f0 = _mm_load_ps(&A1[i+5][j]);
			f3 = _mm_fmadd_ps(f0, a1, f3);
			
			g0 = _mm_load_ps(&A1[i+6][j]);
			g3 = _mm_fmadd_ps(g0, a1, g3);
			
			h0 = _mm_load_ps(&A1[i+7][j]);
			h3 = _mm_fmadd_ps(h0, a1, h3);
		}

		a4 = _mm_hadd_ps(a3, a3);
		a4 = _mm_hadd_ps(a4, a4);
		_mm_store_ss(&Y[i], a4);
		
		b4 = _mm_hadd_ps(b3, b3);
		b4 = _mm_hadd_ps(b4, b4);
		_mm_store_ss(&Y[i+1], b4);
		
		c4 = _mm_hadd_ps(c3, c3);
		c4 = _mm_hadd_ps(c4, c4);
		_mm_store_ss(&Y[i+2], c4);
		
		d4 = _mm_hadd_ps(d3, d3);
		d4 = _mm_hadd_ps(d4, d4);
		_mm_store_ss(&Y[i+3], d4);
		
		e4 = _mm_hadd_ps(e3, e3);
		e4 = _mm_hadd_ps(e4, e4);
		_mm_store_ss(&Y[i+4], e4);
		
		f4 = _mm_hadd_ps(f3, f3);
		f4 = _mm_hadd_ps(f4, f4);
		_mm_store_ss(&Y[i+5], f4);
		
		g4 = _mm_hadd_ps(g3, g3);
		g4 = _mm_hadd_ps(g4, g4);
		_mm_store_ss(&Y[i+6], g4);
		
		h4 = _mm_hadd_ps(h3, h3);
		h4 = _mm_hadd_ps(h4, h4);
		_mm_store_ss(&Y[i+7], h4);
	}

	return 1;
}

unsigned short int MVM_SSE_REG_8_OMP(int noThreads)
{
	int i, j;
	
	__m128 a0, a1, a3, a4, b0, b3, b4, c0, c3, c4, d0, d3, d4, e0, e3, e4, f0, f3, f4, g0, g3, 			g4, h0, h3, h4;
	
	omp_set_num_threads(noThreads);
	#pragma omp parallel
	{
	#pragma omp for private(i,j) schedule(dynamic)
	for (i = 0; i < M; i+=8) {

		a3 = _mm_setzero_ps();
		b3 = _mm_setzero_ps();
		c3 = _mm_setzero_ps();
		d3 = _mm_setzero_ps();
		e3 = _mm_setzero_ps();
		f3 = _mm_setzero_ps();
		g3 = _mm_setzero_ps();
		h3 = _mm_setzero_ps();
		
		#pragma omp simd aligned(X,A1:64)
		for (j = 0; j < M; j += 4) { 
			a1 = _mm_load_ps(&X[j]);
			
			a0 = _mm_load_ps(&A1[i][j]);
			a3 = _mm_fmadd_ps(a0, a1, a3);
			
			b0 = _mm_load_ps(&A1[i+1][j]);
			b3 = _mm_fmadd_ps(b0, a1, b3);
			
			c0 = _mm_load_ps(&A1[i+2][j]);
			c3 = _mm_fmadd_ps(c0, a1, c3);
			
			d0 = _mm_load_ps(&A1[i+3][j]);
			d3 = _mm_fmadd_ps(d0, a1, d3);
			
			e0 = _mm_load_ps(&A1[i+4][j]);
			e3 = _mm_fmadd_ps(e0, a1, e3);
			
			f0 = _mm_load_ps(&A1[i+5][j]);
			f3 = _mm_fmadd_ps(f0, a1, f3);
			
			g0 = _mm_load_ps(&A1[i+6][j]);
			g3 = _mm_fmadd_ps(g0, a1, g3);
			
			h0 = _mm_load_ps(&A1[i+7][j]);
			h3 = _mm_fmadd_ps(h0, a1, h3);
		}

		a4 = _mm_hadd_ps(a3, a3);
		a4 = _mm_hadd_ps(a4, a4);
		_mm_store_ss(&Y[i], a4);
		
		b4 = _mm_hadd_ps(b3, b3);
		b4 = _mm_hadd_ps(b4, b4);
		_mm_store_ss(&Y[i+1], b4);
		
		c4 = _mm_hadd_ps(c3, c3);
		c4 = _mm_hadd_ps(c4, c4);
		_mm_store_ss(&Y[i+2], c4);
		
		d4 = _mm_hadd_ps(d3, d3);
		d4 = _mm_hadd_ps(d4, d4);
		_mm_store_ss(&Y[i+3], d4);
		
		e4 = _mm_hadd_ps(e3, e3);
		e4 = _mm_hadd_ps(e4, e4);
		_mm_store_ss(&Y[i+4], e4);
		
		f4 = _mm_hadd_ps(f3, f3);
		f4 = _mm_hadd_ps(f4, f4);
		_mm_store_ss(&Y[i+5], f4);
		
		g4 = _mm_hadd_ps(g3, g3);
		g4 = _mm_hadd_ps(g4, g4);
		_mm_store_ss(&Y[i+6], g4);
		
		h4 = _mm_hadd_ps(h3, h3);
		h4 = _mm_hadd_ps(h4, h4);
		_mm_store_ss(&Y[i+7], h4);
	}
	}

	return 1;
}

unsigned short int MVM_SSE_REG_16()
{
	__m128 a0, a1, a3, a4, b0, b3, b4, c0, c3, c4, d0, d3, d4, e0, e3, e4, f0, f3, f4, g0, g3, 			g4, h0, h3, h4, i0, i3, i4, j0, j3, j4, k0, k3, k4, l0, l3, l4, m0, m3, m4, n0, n3, 		n4, o0, o3, o4, p0, p3, p4;

	for (int i = 0; i < M; i+=16) {

		a3 = _mm_setzero_ps();
		b3 = _mm_setzero_ps();
		c3 = _mm_setzero_ps();
		d3 = _mm_setzero_ps();
		e3 = _mm_setzero_ps();
		f3 = _mm_setzero_ps();
		g3 = _mm_setzero_ps();
		h3 = _mm_setzero_ps();
		i3 = _mm_setzero_ps();
		j3 = _mm_setzero_ps();
		k3 = _mm_setzero_ps();
		l3 = _mm_setzero_ps();
		m3 = _mm_setzero_ps();
		n3 = _mm_setzero_ps(); 
		o3 = _mm_setzero_ps(); 
		p3 = _mm_setzero_ps();
		
		for (int j = 0; j < M; j += 4) { 
			a1 = _mm_load_ps(&X[j]);
			
			a0 = _mm_load_ps(&A1[i][j]);
			a3 = _mm_fmadd_ps(a0, a1, a3);
			
			b0 = _mm_load_ps(&A1[i+1][j]);
			b3 = _mm_fmadd_ps(b0, a1, b3);
			
			c0 = _mm_load_ps(&A1[i+2][j]);
			c3 = _mm_fmadd_ps(c0, a1, c3);
			
			d0 = _mm_load_ps(&A1[i+3][j]);
			d3 = _mm_fmadd_ps(d0, a1, d3);
			
			e0 = _mm_load_ps(&A1[i+4][j]);
			e3 = _mm_fmadd_ps(e0, a1, e3);
			
			f0 = _mm_load_ps(&A1[i+5][j]);
			f3 = _mm_fmadd_ps(f0, a1, f3);
			
			g0 = _mm_load_ps(&A1[i+6][j]);
			g3 = _mm_fmadd_ps(g0, a1, g3);
			
			h0 = _mm_load_ps(&A1[i+7][j]);
			h3 = _mm_fmadd_ps(h0, a1, h3);
			
			i0 = _mm_load_ps(&A1[i+8][j]);
			i3 = _mm_fmadd_ps(i0, a1, i3);
			
			j0 = _mm_load_ps(&A1[i+9][j]);
			j3 = _mm_fmadd_ps(j0, a1, j3);
			
			k0 = _mm_load_ps(&A1[i+10][j]);
			k3 = _mm_fmadd_ps(k0, a1, k3);
			
			l0 = _mm_load_ps(&A1[i+11][j]);
			l3 = _mm_fmadd_ps(l0, a1, l3);
			
			m0 = _mm_load_ps(&A1[i+12][j]);
			m3 = _mm_fmadd_ps(m0, a1, m3);
			
			n0 = _mm_load_ps(&A1[i+13][j]);
			n3 = _mm_fmadd_ps(n0, a1, n3);
			
			o0 = _mm_load_ps(&A1[i+14][j]);
			o3 = _mm_fmadd_ps(o0, a1, o3);
			
			p0 = _mm_load_ps(&A1[i+15][j]);
			p3 = _mm_fmadd_ps(p0, a1, p3);
		}

		a4 = _mm_hadd_ps(a3, a3);
		a4 = _mm_hadd_ps(a4, a4);
		_mm_store_ss(&Y[i], a4);
		
		b4 = _mm_hadd_ps(b3, b3);
		b4 = _mm_hadd_ps(b4, b4);
		_mm_store_ss(&Y[i+1], b4);
		
		c4 = _mm_hadd_ps(c3, c3);
		c4 = _mm_hadd_ps(c4, c4);
		_mm_store_ss(&Y[i+2], c4);
		
		d4 = _mm_hadd_ps(d3, d3);
		d4 = _mm_hadd_ps(d4, d4);
		_mm_store_ss(&Y[i+3], d4);
		
		e4 = _mm_hadd_ps(e3, e3);
		e4 = _mm_hadd_ps(e4, e4);
		_mm_store_ss(&Y[i+4], e4);
		
		f4 = _mm_hadd_ps(f3, f3);
		f4 = _mm_hadd_ps(f4, f4);
		_mm_store_ss(&Y[i+5], f4);
		
		g4 = _mm_hadd_ps(g3, g3);
		g4 = _mm_hadd_ps(g4, g4);
		_mm_store_ss(&Y[i+6], g4);
		
		h4 = _mm_hadd_ps(h3, h3);
		h4 = _mm_hadd_ps(h4, h4);
		_mm_store_ss(&Y[i+7], h4);
		
		i4 = _mm_hadd_ps(i3, i3);
		i4 = _mm_hadd_ps(i4, i4);
		_mm_store_ss(&Y[i+8], i4);
		
		j4 = _mm_hadd_ps(j3, j3);
		j4 = _mm_hadd_ps(j4, j4);
		_mm_store_ss(&Y[i+9], j4);
		
		k4 = _mm_hadd_ps(k3, k3);
		k4 = _mm_hadd_ps(k4, k4);
		_mm_store_ss(&Y[i+10], k4);
		
		l4 = _mm_hadd_ps(l3, l3);
		l4 = _mm_hadd_ps(l4, l4);
		_mm_store_ss(&Y[i+11], l4);
		
		m4 = _mm_hadd_ps(m3, m3);
		m4 = _mm_hadd_ps(m4, m4);
		_mm_store_ss(&Y[i+12], m4);
		
		n4 = _mm_hadd_ps(n3, n3);
		n4 = _mm_hadd_ps(n4, n4);
		_mm_store_ss(&Y[i+13], n4);
		
		o4 = _mm_hadd_ps(o3, o3);
		o4 = _mm_hadd_ps(o4, o4);
		_mm_store_ss(&Y[i+14], o4);
		
		p4 = _mm_hadd_ps(p3, p3);
		p4 = _mm_hadd_ps(p4, p4);
		_mm_store_ss(&Y[i+15], p4);
	}

	return 1;
}

unsigned short int MVM_SSE_REG_16_OMP(int noThreads)
{
	int i, j;
	
	__m128 a0, a1, a3, a4, b0, b3, b4, c0, c3, c4, d0, d3, d4, e0, e3, e4, f0, f3, f4, g0, g3, 			g4, h0, h3, h4, i0, i3, i4, j0, j3, j4, k0, k3, k4, l0, l3, l4, m0, m3, m4, n0, n3, 		n4, o0, o3, o4, p0, p3, p4;
	
	omp_set_num_threads(noThreads);
	#pragma omp parallel
	{
	#pragma omp for private(i,j) schedule(dynamic)
	for (i = 0; i < M; i+=16) {

		a3 = _mm_setzero_ps();
		b3 = _mm_setzero_ps();
		c3 = _mm_setzero_ps();
		d3 = _mm_setzero_ps();
		e3 = _mm_setzero_ps();
		f3 = _mm_setzero_ps();
		g3 = _mm_setzero_ps();
		h3 = _mm_setzero_ps();
		i3 = _mm_setzero_ps();
		j3 = _mm_setzero_ps();
		k3 = _mm_setzero_ps();
		l3 = _mm_setzero_ps();
		m3 = _mm_setzero_ps();
		n3 = _mm_setzero_ps(); 
		o3 = _mm_setzero_ps(); 
		p3 = _mm_setzero_ps();
		
		#pragma omp simd aligned(X,A1:64)
		for (j = 0; j < M; j += 4) { 
			a1 = _mm_load_ps(&X[j]);
			
			a0 = _mm_load_ps(&A1[i][j]);
			a3 = _mm_fmadd_ps(a0, a1, a3);
			
			b0 = _mm_load_ps(&A1[i+1][j]);
			b3 = _mm_fmadd_ps(b0, a1, b3);
			
			c0 = _mm_load_ps(&A1[i+2][j]);
			c3 = _mm_fmadd_ps(c0, a1, c3);
			
			d0 = _mm_load_ps(&A1[i+3][j]);
			d3 = _mm_fmadd_ps(d0, a1, d3);
			
			e0 = _mm_load_ps(&A1[i+4][j]);
			e3 = _mm_fmadd_ps(e0, a1, e3);
			
			f0 = _mm_load_ps(&A1[i+5][j]);
			f3 = _mm_fmadd_ps(f0, a1, f3);
			
			g0 = _mm_load_ps(&A1[i+6][j]);
			g3 = _mm_fmadd_ps(g0, a1, g3);
			
			h0 = _mm_load_ps(&A1[i+7][j]);
			h3 = _mm_fmadd_ps(h0, a1, h3);
			
			i0 = _mm_load_ps(&A1[i+8][j]);
			i3 = _mm_fmadd_ps(i0, a1, i3);
			
			j0 = _mm_load_ps(&A1[i+9][j]);
			j3 = _mm_fmadd_ps(j0, a1, j3);
			
			k0 = _mm_load_ps(&A1[i+10][j]);
			k3 = _mm_fmadd_ps(k0, a1, k3);
			
			l0 = _mm_load_ps(&A1[i+11][j]);
			l3 = _mm_fmadd_ps(l0, a1, l3);
			
			m0 = _mm_load_ps(&A1[i+12][j]);
			m3 = _mm_fmadd_ps(m0, a1, m3);
			
			n0 = _mm_load_ps(&A1[i+13][j]);
			n3 = _mm_fmadd_ps(n0, a1, n3);
			
			o0 = _mm_load_ps(&A1[i+14][j]);
			o3 = _mm_fmadd_ps(o0, a1, o3);
			
			p0 = _mm_load_ps(&A1[i+15][j]);
			p3 = _mm_fmadd_ps(p0, a1, p3);
		}

		a4 = _mm_hadd_ps(a3, a3);
		a4 = _mm_hadd_ps(a4, a4);
		_mm_store_ss(&Y[i], a4);
		
		b4 = _mm_hadd_ps(b3, b3);
		b4 = _mm_hadd_ps(b4, b4);
		_mm_store_ss(&Y[i+1], b4);
		
		c4 = _mm_hadd_ps(c3, c3);
		c4 = _mm_hadd_ps(c4, c4);
		_mm_store_ss(&Y[i+2], c4);
		
		d4 = _mm_hadd_ps(d3, d3);
		d4 = _mm_hadd_ps(d4, d4);
		_mm_store_ss(&Y[i+3], d4);
		
		e4 = _mm_hadd_ps(e3, e3);
		e4 = _mm_hadd_ps(e4, e4);
		_mm_store_ss(&Y[i+4], e4);
		
		f4 = _mm_hadd_ps(f3, f3);
		f4 = _mm_hadd_ps(f4, f4);
		_mm_store_ss(&Y[i+5], f4);
		
		g4 = _mm_hadd_ps(g3, g3);
		g4 = _mm_hadd_ps(g4, g4);
		_mm_store_ss(&Y[i+6], g4);
		
		h4 = _mm_hadd_ps(h3, h3);
		h4 = _mm_hadd_ps(h4, h4);
		_mm_store_ss(&Y[i+7], h4);
		
		i4 = _mm_hadd_ps(i3, i3);
		i4 = _mm_hadd_ps(i4, i4);
		_mm_store_ss(&Y[i+8], i4);
		
		j4 = _mm_hadd_ps(j3, j3);
		j4 = _mm_hadd_ps(j4, j4);
		_mm_store_ss(&Y[i+9], j4);
		
		k4 = _mm_hadd_ps(k3, k3);
		k4 = _mm_hadd_ps(k4, k4);
		_mm_store_ss(&Y[i+10], k4);
		
		l4 = _mm_hadd_ps(l3, l3);
		l4 = _mm_hadd_ps(l4, l4);
		_mm_store_ss(&Y[i+11], l4);
		
		m4 = _mm_hadd_ps(m3, m3);
		m4 = _mm_hadd_ps(m4, m4);
		_mm_store_ss(&Y[i+12], m4);
		
		n4 = _mm_hadd_ps(n3, n3);
		n4 = _mm_hadd_ps(n4, n4);
		_mm_store_ss(&Y[i+13], n4);
		
		o4 = _mm_hadd_ps(o3, o3);
		o4 = _mm_hadd_ps(o4, o4);
		_mm_store_ss(&Y[i+14], o4);
		
		p4 = _mm_hadd_ps(p3, p3);
		p4 = _mm_hadd_ps(p4, p4);
		_mm_store_ss(&Y[i+15], p4);
	}
	}
	
	return 1;
}

unsigned short int MVM_AVX_REG_4()
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5;
	__m128 xmm1, a4, b4, c4, d4;
	
	for (i = 0; i < M; i+=4) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();

		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);


	}

	return 1;
}

unsigned short int MVM_AVX_REG_8()
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5,
		 e0, e1, e2, e5, f0, f1, f2, f5, g0, g1, g2, g5, h0, h1, h2, h5;
	__m128	a4, b4, c4, d4, e4, f4, g4, h4;
	
	for (i = 0; i < M; i+=8) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		e1 = _mm256_setzero_ps();
		f1 = _mm256_setzero_ps();
		g1 = _mm256_setzero_ps();
		h1 = _mm256_setzero_ps();

		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
			
			
			e0 = _mm256_load_ps(&A1[i + 4][j]);
			e1 = _mm256_fmadd_ps(e0, a5, e1);
			
			
			f0 = _mm256_load_ps(&A1[i + 5][j]);
			f1 = _mm256_fmadd_ps(f0, a5, f1);
			
			
			g0 = _mm256_load_ps(&A1[i + 6][j]);
			g1 = _mm256_fmadd_ps(g0, a5, g1);
			
			
			h0 = _mm256_load_ps(&A1[i + 7][j]);
			h1 = _mm256_fmadd_ps(h0, a5, h1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		e2 = _mm256_permute2f128_ps(e1, e1, 1);
		e1 = _mm256_add_ps(e1, e2);
		e1 = _mm256_hadd_ps(e1, e1);
		e1 = _mm256_hadd_ps(e1, e1);
		e4 = _mm256_extractf128_ps(e1, 0);
		_mm_store_ss(Y + (i+4), e4);
		
		f2 = _mm256_permute2f128_ps(f1, f1, 1);
		f1 = _mm256_add_ps(f1, f2);
		f1 = _mm256_hadd_ps(f1, f1);
		f1 = _mm256_hadd_ps(f1, f1);
		f4 = _mm256_extractf128_ps(f1, 0);
		_mm_store_ss(Y + (i+5), f4);
		
		g2 = _mm256_permute2f128_ps(g1, g1, 1);
		g1 = _mm256_add_ps(g1, g2);
		g1 = _mm256_hadd_ps(g1, g1);
		g1 = _mm256_hadd_ps(g1, g1);
		g4 = _mm256_extractf128_ps(g1, 0);
		_mm_store_ss(Y + (i+6), g4);
		
		h2 = _mm256_permute2f128_ps(h1, h1, 1);
		h1 = _mm256_add_ps(h1, h2);
		h1 = _mm256_hadd_ps(h1, h1);
		h1 = _mm256_hadd_ps(h1, h1);
		h4 = _mm256_extractf128_ps(h1, 0);
		_mm_store_ss(Y + (i+7), h4);


	}

	return 1;
}

unsigned short int MVM_AVX_REG_8_OMP(int noThreads)
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5,
		 e0, e1, e2, e5, f0, f1, f2, f5, g0, g1, g2, g5, h0, h1, h2, h5;
	__m128	a4, b4, c4, d4, e4, f4, g4, h4;
	
	omp_set_num_threads(noThreads);
	#pragma omp parallel
	{
	#pragma omp for private(i,j) schedule(dynamic)
	for (i = 0; i < M; i+=8) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		e1 = _mm256_setzero_ps();
		f1 = _mm256_setzero_ps();
		g1 = _mm256_setzero_ps();
		h1 = _mm256_setzero_ps();
		
		#pragma omp simd aligned(X,A1:64)
		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
			
			
			e0 = _mm256_load_ps(&A1[i + 4][j]);
			e1 = _mm256_fmadd_ps(e0, a5, e1);
			
			
			f0 = _mm256_load_ps(&A1[i + 5][j]);
			f1 = _mm256_fmadd_ps(f0, a5, f1);
			
			
			g0 = _mm256_load_ps(&A1[i + 6][j]);
			g1 = _mm256_fmadd_ps(g0, a5, g1);
			
			
			h0 = _mm256_load_ps(&A1[i + 7][j]);
			h1 = _mm256_fmadd_ps(h0, a5, h1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		e2 = _mm256_permute2f128_ps(e1, e1, 1);
		e1 = _mm256_add_ps(e1, e2);
		e1 = _mm256_hadd_ps(e1, e1);
		e1 = _mm256_hadd_ps(e1, e1);
		e4 = _mm256_extractf128_ps(e1, 0);
		_mm_store_ss(Y + (i+4), e4);
		
		f2 = _mm256_permute2f128_ps(f1, f1, 1);
		f1 = _mm256_add_ps(f1, f2);
		f1 = _mm256_hadd_ps(f1, f1);
		f1 = _mm256_hadd_ps(f1, f1);
		f4 = _mm256_extractf128_ps(f1, 0);
		_mm_store_ss(Y + (i+5), f4);
		
		g2 = _mm256_permute2f128_ps(g1, g1, 1);
		g1 = _mm256_add_ps(g1, g2);
		g1 = _mm256_hadd_ps(g1, g1);
		g1 = _mm256_hadd_ps(g1, g1);
		g4 = _mm256_extractf128_ps(g1, 0);
		_mm_store_ss(Y + (i+6), g4);
		
		h2 = _mm256_permute2f128_ps(h1, h1, 1);
		h1 = _mm256_add_ps(h1, h2);
		h1 = _mm256_hadd_ps(h1, h1);
		h1 = _mm256_hadd_ps(h1, h1);
		h4 = _mm256_extractf128_ps(h1, 0);
		_mm_store_ss(Y + (i+7), h4);


	}
	}

	return 1;
}

unsigned short int MVM_AVX_REG_13()
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5,
		 e0, e1, e2, e5, f0, f1, f2, f5, g0, g1, g2, g5, h0, h1, h2, h5,
		 i0, i1, i2, i5, j0, j1, j2, j5, k0, k1, k2, k5, l0, l1, l2, l5, 
		 m0, m1, m2, m5;
	__m128	a4, b4, c4, d4, e4, f4, g4, h4, i4, j4, k4, l4, m4;
	
	for (i = 0; i < M; i+=13) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		e1 = _mm256_setzero_ps();
		f1 = _mm256_setzero_ps();
		g1 = _mm256_setzero_ps();
		h1 = _mm256_setzero_ps();
		i1 = _mm256_setzero_ps();
		j1 = _mm256_setzero_ps();
		k1 = _mm256_setzero_ps();
		l1 = _mm256_setzero_ps();
		m1 = _mm256_setzero_ps();

		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
			
			
			e0 = _mm256_load_ps(&A1[i + 4][j]);
			e1 = _mm256_fmadd_ps(e0, a5, e1);
			
			
			f0 = _mm256_load_ps(&A1[i + 5][j]);
			f1 = _mm256_fmadd_ps(f0, a5, f1);
			
			
			g0 = _mm256_load_ps(&A1[i + 6][j]);
			g1 = _mm256_fmadd_ps(g0, a5, g1);
			
			
			h0 = _mm256_load_ps(&A1[i + 7][j]);
			h1 = _mm256_fmadd_ps(h0, a5, h1);
			
			
			i0 = _mm256_load_ps(&A1[i + 8][j]);
			i1 = _mm256_fmadd_ps(i0, a5, i1);
			
			
			j0 = _mm256_load_ps(&A1[i + 9][j]);
			j1 = _mm256_fmadd_ps(j0, a5, j1);
			
			
			k0 = _mm256_load_ps(&A1[i + 10][j]);
			k1 = _mm256_fmadd_ps(k0, a5, k1);
			
			
			l0 = _mm256_load_ps(&A1[i + 11][j]);
			l1 = _mm256_fmadd_ps(l0, a5, l1);
			
			
			m0 = _mm256_load_ps(&A1[i + 12][j]);
			m1 = _mm256_fmadd_ps(m0, a5, m1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		e2 = _mm256_permute2f128_ps(e1, e1, 1);
		e1 = _mm256_add_ps(e1, e2);
		e1 = _mm256_hadd_ps(e1, e1);
		e1 = _mm256_hadd_ps(e1, e1);
		e4 = _mm256_extractf128_ps(e1, 0);
		_mm_store_ss(Y + (i+4), e4);
		
		f2 = _mm256_permute2f128_ps(f1, f1, 1);
		f1 = _mm256_add_ps(f1, f2);
		f1 = _mm256_hadd_ps(f1, f1);
		f1 = _mm256_hadd_ps(f1, f1);
		f4 = _mm256_extractf128_ps(f1, 0);
		_mm_store_ss(Y + (i+5), f4);
		
		g2 = _mm256_permute2f128_ps(g1, g1, 1);
		g1 = _mm256_add_ps(g1, g2);
		g1 = _mm256_hadd_ps(g1, g1);
		g1 = _mm256_hadd_ps(g1, g1);
		g4 = _mm256_extractf128_ps(g1, 0);
		_mm_store_ss(Y + (i+6), g4);
		
		h2 = _mm256_permute2f128_ps(h1, h1, 1);
		h1 = _mm256_add_ps(h1, h2);
		h1 = _mm256_hadd_ps(h1, h1);
		h1 = _mm256_hadd_ps(h1, h1);
		h4 = _mm256_extractf128_ps(h1, 0);
		_mm_store_ss(Y + (i+7), h4);
		
		i2 = _mm256_permute2f128_ps(i1, i1, 1);
		i1 = _mm256_add_ps(i1, i2);
		i1 = _mm256_hadd_ps(i1, i1);
		i1 = _mm256_hadd_ps(i1, i1);
		i4 = _mm256_extractf128_ps(i1, 0);
		_mm_store_ss(Y + (i+8), i4);
		
		j2 = _mm256_permute2f128_ps(j1, j1, 1);
		j1 = _mm256_add_ps(j1, j2);
		j1 = _mm256_hadd_ps(j1, j1);
		j1 = _mm256_hadd_ps(j1, j1);
		j4 = _mm256_extractf128_ps(j1, 0);
		_mm_store_ss(Y + (i+9), j4);
		
		k2 = _mm256_permute2f128_ps(k1, k1, 1);
		k1 = _mm256_add_ps(k1, k2);
		k1 = _mm256_hadd_ps(k1, k1);
		k1 = _mm256_hadd_ps(k1, k1);
		k4 = _mm256_extractf128_ps(k1, 0);
		_mm_store_ss(Y + (i+10), k4);
		
		l2 = _mm256_permute2f128_ps(l1, l1, 1);
		l1 = _mm256_add_ps(l1, l2);
		l1 = _mm256_hadd_ps(l1, l1);
		l1 = _mm256_hadd_ps(l1, l1);
		l4 = _mm256_extractf128_ps(l1, 0);
		_mm_store_ss(Y + (i+11), l4);
		
		m2 = _mm256_permute2f128_ps(m1, m1, 1);
		m1 = _mm256_add_ps(m1, m2);
		m1 = _mm256_hadd_ps(m1, m1);
		m1 = _mm256_hadd_ps(m1, m1);
		m4 = _mm256_extractf128_ps(m1, 0);
		_mm_store_ss(Y + (i+12), m4);


	}

	return 1;
}

unsigned short int MVM_AVX_REG_16()
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5,
		 e0, e1, e2, e5, f0, f1, f2, f5, g0, g1, g2, g5, h0, h1, h2, h5,
		 i0, i1, i2, i5, j0, j1, j2, j5, k0, k1, k2, k5, l0, l1, l2, l5, 
		 m0, m1, m2, m5, n0, n1, n2, n5, o0, o1, o2, o5, p0, p1, p2, p5;
	__m128	a4, b4, c4, d4, e4, f4, g4, h4, i4, j4, k4, l4, m4, n4, o4, p4;
	
	for (i = 0; i < M; i+=16) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		e1 = _mm256_setzero_ps();
		f1 = _mm256_setzero_ps();
		g1 = _mm256_setzero_ps();
		h1 = _mm256_setzero_ps();
		i1 = _mm256_setzero_ps();
		j1 = _mm256_setzero_ps();
		k1 = _mm256_setzero_ps();
		l1 = _mm256_setzero_ps();
		m1 = _mm256_setzero_ps();
		n1 = _mm256_setzero_ps();
		o1 = _mm256_setzero_ps();
		p1 = _mm256_setzero_ps();

		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
			
			
			e0 = _mm256_load_ps(&A1[i + 4][j]);
			e1 = _mm256_fmadd_ps(e0, a5, e1);
			
			
			f0 = _mm256_load_ps(&A1[i + 5][j]);
			f1 = _mm256_fmadd_ps(f0, a5, f1);
			
			
			g0 = _mm256_load_ps(&A1[i + 6][j]);
			g1 = _mm256_fmadd_ps(g0, a5, g1);
			
			
			h0 = _mm256_load_ps(&A1[i + 7][j]);
			h1 = _mm256_fmadd_ps(h0, a5, h1);
			
			
			i0 = _mm256_load_ps(&A1[i + 8][j]);
			i1 = _mm256_fmadd_ps(i0, a5, i1);
			
			
			j0 = _mm256_load_ps(&A1[i + 9][j]);
			j1 = _mm256_fmadd_ps(j0, a5, j1);
			
			
			k0 = _mm256_load_ps(&A1[i + 10][j]);
			k1 = _mm256_fmadd_ps(k0, a5, k1);
			
			
			l0 = _mm256_load_ps(&A1[i + 11][j]);
			l1 = _mm256_fmadd_ps(l0, a5, l1);
			
			
			m0 = _mm256_load_ps(&A1[i + 12][j]);
			m1 = _mm256_fmadd_ps(m0, a5, m1);
			
			n0 = _mm256_load_ps(&A1[i + 13][j]);
			n1 = _mm256_fmadd_ps(n0, a5, n1);
			
			o0 = _mm256_load_ps(&A1[i + 14][j]);
			o1 = _mm256_fmadd_ps(o0, a5, o1);
			
			p0 = _mm256_load_ps(&A1[i + 15][j]);
			p1 = _mm256_fmadd_ps(p0, a5, p1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		e2 = _mm256_permute2f128_ps(e1, e1, 1);
		e1 = _mm256_add_ps(e1, e2);
		e1 = _mm256_hadd_ps(e1, e1);
		e1 = _mm256_hadd_ps(e1, e1);
		e4 = _mm256_extractf128_ps(e1, 0);
		_mm_store_ss(Y + (i+4), e4);
		
		f2 = _mm256_permute2f128_ps(f1, f1, 1);
		f1 = _mm256_add_ps(f1, f2);
		f1 = _mm256_hadd_ps(f1, f1);
		f1 = _mm256_hadd_ps(f1, f1);
		f4 = _mm256_extractf128_ps(f1, 0);
		_mm_store_ss(Y + (i+5), f4);
		
		g2 = _mm256_permute2f128_ps(g1, g1, 1);
		g1 = _mm256_add_ps(g1, g2);
		g1 = _mm256_hadd_ps(g1, g1);
		g1 = _mm256_hadd_ps(g1, g1);
		g4 = _mm256_extractf128_ps(g1, 0);
		_mm_store_ss(Y + (i+6), g4);
		
		h2 = _mm256_permute2f128_ps(h1, h1, 1);
		h1 = _mm256_add_ps(h1, h2);
		h1 = _mm256_hadd_ps(h1, h1);
		h1 = _mm256_hadd_ps(h1, h1);
		h4 = _mm256_extractf128_ps(h1, 0);
		_mm_store_ss(Y + (i+7), h4);
		
		i2 = _mm256_permute2f128_ps(i1, i1, 1);
		i1 = _mm256_add_ps(i1, i2);
		i1 = _mm256_hadd_ps(i1, i1);
		i1 = _mm256_hadd_ps(i1, i1);
		i4 = _mm256_extractf128_ps(i1, 0);
		_mm_store_ss(Y + (i+8), i4);
		
		j2 = _mm256_permute2f128_ps(j1, j1, 1);
		j1 = _mm256_add_ps(j1, j2);
		j1 = _mm256_hadd_ps(j1, j1);
		j1 = _mm256_hadd_ps(j1, j1);
		j4 = _mm256_extractf128_ps(j1, 0);
		_mm_store_ss(Y + (i+9), j4);
		
		k2 = _mm256_permute2f128_ps(k1, k1, 1);
		k1 = _mm256_add_ps(k1, k2);
		k1 = _mm256_hadd_ps(k1, k1);
		k1 = _mm256_hadd_ps(k1, k1);
		k4 = _mm256_extractf128_ps(k1, 0);
		_mm_store_ss(Y + (i+10), k4);
		
		l2 = _mm256_permute2f128_ps(l1, l1, 1);
		l1 = _mm256_add_ps(l1, l2);
		l1 = _mm256_hadd_ps(l1, l1);
		l1 = _mm256_hadd_ps(l1, l1);
		l4 = _mm256_extractf128_ps(l1, 0);
		_mm_store_ss(Y + (i+11), l4);
		
		m2 = _mm256_permute2f128_ps(m1, m1, 1);
		m1 = _mm256_add_ps(m1, m2);
		m1 = _mm256_hadd_ps(m1, m1);
		m1 = _mm256_hadd_ps(m1, m1);
		m4 = _mm256_extractf128_ps(m1, 0);
		_mm_store_ss(Y + (i+12), m4);
		
		n2 = _mm256_permute2f128_ps(n1, n1, 1);
		n1 = _mm256_add_ps(n1, n2);
		n1 = _mm256_hadd_ps(n1, n1);
		n1 = _mm256_hadd_ps(n1, n1);
		n4 = _mm256_extractf128_ps(n1, 0);
		_mm_store_ss(Y + (i+13), n4);
		
		o2 = _mm256_permute2f128_ps(o1, o1, 1);
		o1 = _mm256_add_ps(o1, o2);
		o1 = _mm256_hadd_ps(o1, o1);
		o1 = _mm256_hadd_ps(o1, o1);
		o4 = _mm256_extractf128_ps(o1, 0);
		_mm_store_ss(Y + (i+14), o4);
		
		p2 = _mm256_permute2f128_ps(p1, p1, 1);
		p1 = _mm256_add_ps(p1, p2);
		p1 = _mm256_hadd_ps(p1, p1);
		p1 = _mm256_hadd_ps(p1, p1);
		p4 = _mm256_extractf128_ps(p1, 0);
		_mm_store_ss(Y + (i+15), p4);


	}

	return 1;
}

unsigned short int MVM_AVX_REG_16_OMP(int noThreads)
{
	float tmp;
	int i, j;
	

	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5,
		 e0, e1, e2, e5, f0, f1, f2, f5, g0, g1, g2, g5, h0, h1, h2, h5,
		 i0, i1, i2, i5, j0, j1, j2, j5, k0, k1, k2, k5, l0, l1, l2, l5, 
		 m0, m1, m2, m5, n0, n1, n2, n5, o0, o1, o2, o5, p0, p1, p2, p5;
	__m128	a4, b4, c4, d4, e4, f4, g4, h4, i4, j4, k4, l4, m4, n4, o4, p4;
	
	omp_set_num_threads(noThreads);
	#pragma omp parallel
	{
	#pragma omp for private(i,j) schedule(dynamic)
	for (i = 0; i < M; i+=16) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		e1 = _mm256_setzero_ps();
		f1 = _mm256_setzero_ps();
		g1 = _mm256_setzero_ps();
		h1 = _mm256_setzero_ps();
		i1 = _mm256_setzero_ps();
		j1 = _mm256_setzero_ps();
		k1 = _mm256_setzero_ps();
		l1 = _mm256_setzero_ps();
		m1 = _mm256_setzero_ps();
		n1 = _mm256_setzero_ps();
		o1 = _mm256_setzero_ps();
		p1 = _mm256_setzero_ps();
		
		#pragma omp simd aligned(X,A1:64)
		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
			
			
			e0 = _mm256_load_ps(&A1[i + 4][j]);
			e1 = _mm256_fmadd_ps(e0, a5, e1);
			
			
			f0 = _mm256_load_ps(&A1[i + 5][j]);
			f1 = _mm256_fmadd_ps(f0, a5, f1);
			
			
			g0 = _mm256_load_ps(&A1[i + 6][j]);
			g1 = _mm256_fmadd_ps(g0, a5, g1);
			
			
			h0 = _mm256_load_ps(&A1[i + 7][j]);
			h1 = _mm256_fmadd_ps(h0, a5, h1);
			
			
			i0 = _mm256_load_ps(&A1[i + 8][j]);
			i1 = _mm256_fmadd_ps(i0, a5, i1);
			
			
			j0 = _mm256_load_ps(&A1[i + 9][j]);
			j1 = _mm256_fmadd_ps(j0, a5, j1);
			
			
			k0 = _mm256_load_ps(&A1[i + 10][j]);
			k1 = _mm256_fmadd_ps(k0, a5, k1);
			
			
			l0 = _mm256_load_ps(&A1[i + 11][j]);
			l1 = _mm256_fmadd_ps(l0, a5, l1);
			
			
			m0 = _mm256_load_ps(&A1[i + 12][j]);
			m1 = _mm256_fmadd_ps(m0, a5, m1);
			
			n0 = _mm256_load_ps(&A1[i + 13][j]);
			n1 = _mm256_fmadd_ps(n0, a5, n1);
			
			o0 = _mm256_load_ps(&A1[i + 14][j]);
			o1 = _mm256_fmadd_ps(o0, a5, o1);
			
			p0 = _mm256_load_ps(&A1[i + 15][j]);
			p1 = _mm256_fmadd_ps(p0, a5, p1);
		}

		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		e2 = _mm256_permute2f128_ps(e1, e1, 1);
		e1 = _mm256_add_ps(e1, e2);
		e1 = _mm256_hadd_ps(e1, e1);
		e1 = _mm256_hadd_ps(e1, e1);
		e4 = _mm256_extractf128_ps(e1, 0);
		_mm_store_ss(Y + (i+4), e4);
		
		f2 = _mm256_permute2f128_ps(f1, f1, 1);
		f1 = _mm256_add_ps(f1, f2);
		f1 = _mm256_hadd_ps(f1, f1);
		f1 = _mm256_hadd_ps(f1, f1);
		f4 = _mm256_extractf128_ps(f1, 0);
		_mm_store_ss(Y + (i+5), f4);
		
		g2 = _mm256_permute2f128_ps(g1, g1, 1);
		g1 = _mm256_add_ps(g1, g2);
		g1 = _mm256_hadd_ps(g1, g1);
		g1 = _mm256_hadd_ps(g1, g1);
		g4 = _mm256_extractf128_ps(g1, 0);
		_mm_store_ss(Y + (i+6), g4);
		
		h2 = _mm256_permute2f128_ps(h1, h1, 1);
		h1 = _mm256_add_ps(h1, h2);
		h1 = _mm256_hadd_ps(h1, h1);
		h1 = _mm256_hadd_ps(h1, h1);
		h4 = _mm256_extractf128_ps(h1, 0);
		_mm_store_ss(Y + (i+7), h4);
		
		i2 = _mm256_permute2f128_ps(i1, i1, 1);
		i1 = _mm256_add_ps(i1, i2);
		i1 = _mm256_hadd_ps(i1, i1);
		i1 = _mm256_hadd_ps(i1, i1);
		i4 = _mm256_extractf128_ps(i1, 0);
		_mm_store_ss(Y + (i+8), i4);
		
		j2 = _mm256_permute2f128_ps(j1, j1, 1);
		j1 = _mm256_add_ps(j1, j2);
		j1 = _mm256_hadd_ps(j1, j1);
		j1 = _mm256_hadd_ps(j1, j1);
		j4 = _mm256_extractf128_ps(j1, 0);
		_mm_store_ss(Y + (i+9), j4);
		
		k2 = _mm256_permute2f128_ps(k1, k1, 1);
		k1 = _mm256_add_ps(k1, k2);
		k1 = _mm256_hadd_ps(k1, k1);
		k1 = _mm256_hadd_ps(k1, k1);
		k4 = _mm256_extractf128_ps(k1, 0);
		_mm_store_ss(Y + (i+10), k4);
		
		l2 = _mm256_permute2f128_ps(l1, l1, 1);
		l1 = _mm256_add_ps(l1, l2);
		l1 = _mm256_hadd_ps(l1, l1);
		l1 = _mm256_hadd_ps(l1, l1);
		l4 = _mm256_extractf128_ps(l1, 0);
		_mm_store_ss(Y + (i+11), l4);
		
		m2 = _mm256_permute2f128_ps(m1, m1, 1);
		m1 = _mm256_add_ps(m1, m2);
		m1 = _mm256_hadd_ps(m1, m1);
		m1 = _mm256_hadd_ps(m1, m1);
		m4 = _mm256_extractf128_ps(m1, 0);
		_mm_store_ss(Y + (i+12), m4);
		
		n2 = _mm256_permute2f128_ps(n1, n1, 1);
		n1 = _mm256_add_ps(n1, n2);
		n1 = _mm256_hadd_ps(n1, n1);
		n1 = _mm256_hadd_ps(n1, n1);
		n4 = _mm256_extractf128_ps(n1, 0);
		_mm_store_ss(Y + (i+13), n4);
		
		o2 = _mm256_permute2f128_ps(o1, o1, 1);
		o1 = _mm256_add_ps(o1, o2);
		o1 = _mm256_hadd_ps(o1, o1);
		o1 = _mm256_hadd_ps(o1, o1);
		o4 = _mm256_extractf128_ps(o1, 0);
		_mm_store_ss(Y + (i+14), o4);
		
		p2 = _mm256_permute2f128_ps(p1, p1, 1);
		p1 = _mm256_add_ps(p1, p2);
		p1 = _mm256_hadd_ps(p1, p1);
		p1 = _mm256_hadd_ps(p1, p1);
		p4 = _mm256_extractf128_ps(p1, 0);
		_mm_store_ss(Y + (i+15), p4);


	}
	}

	return 1;
}

unsigned short int MVM_AVX_REG_OMP()
{
	float tmp;
	int i, j;
	
	
	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5;
	__m128 xmm1, a4, b4, c4, d4;
	
	#pragma omp parallel
	{
	#pragma omp for private(i,j) schedule(dynamic)//schedule(static, 4)
	for (i = 0; i < M; i+=4) 
	{
		a1 = _mm256_setzero_ps();
		b1 = _mm256_setzero_ps();
		c1 = _mm256_setzero_ps();
		d1 = _mm256_setzero_ps();
		
		
		#pragma omp simd aligned(X,A1:64)
		for (j = 0; j < M;/*((M / 8) * 8);*/ j += 8) 
		{

			a5 = _mm256_load_ps(X + j);
			a0 = _mm256_load_ps(&A1[i][j]);
			a1 = _mm256_fmadd_ps(a0, a5, a1);
			
			
			b0 = _mm256_load_ps(&A1[i + 1][j]);
			b1 = _mm256_fmadd_ps(b0, a5, b1);
			
			
			c0 = _mm256_load_ps(&A1[i + 2][j]);
			c1 = _mm256_fmadd_ps(c0, a5, c1);
			
			
			d0 = _mm256_load_ps(&A1[i + 3][j]);
			d1 = _mm256_fmadd_ps(d0, a5, d1);
		}
		
		a2 = _mm256_permute2f128_ps(a1, a1, 1);
		a1 = _mm256_add_ps(a1, a2);
		a1 = _mm256_hadd_ps(a1, a1);
		a1 = _mm256_hadd_ps(a1, a1);
		a4 = _mm256_extractf128_ps(a1, 0);
		_mm_store_ss(Y + i, a4);
		
		b2 = _mm256_permute2f128_ps(b1, b1, 1);
		b1 = _mm256_add_ps(b1, b2);
		b1 = _mm256_hadd_ps(b1, b1);
		b1 = _mm256_hadd_ps(b1, b1);
		b4 = _mm256_extractf128_ps(b1, 0);
		_mm_store_ss(Y + (i+1), b4);
		
		c2 = _mm256_permute2f128_ps(c1, c1, 1);
		c1 = _mm256_add_ps(c1, c2);
		c1 = _mm256_hadd_ps(c1, c1);
		c1 = _mm256_hadd_ps(c1, c1);
		c4 = _mm256_extractf128_ps(c1, 0);
		_mm_store_ss(Y + (i+2), c4);
		
		d2 = _mm256_permute2f128_ps(d1, d1, 1);
		d1 = _mm256_add_ps(d1, d2);
		d1 = _mm256_hadd_ps(d1, d1);
		d1 = _mm256_hadd_ps(d1, d1);
		d4 = _mm256_extractf128_ps(d1, 0);
		_mm_store_ss(Y + (i+3), d4);
		
		


	}
	}
	return 1;
}

unsigned short int MVM_AVX_REG_OMP_TILE()
{

	
	int i, j, ii, jj;
	float temp[M] __attribute__((aligned(64))) = {0};
	
	__m256  a0, a1, a2, a5, b0, b1, b2, b5, c0, c1, c2, c5, d0, d1, d2, d5;
	__m128 xmm1, a4, b4, c4, d4;

	#pragma omp parallel private(i, j, ii, jj, temp)
	{
		
		#pragma omp for collapse(2)
		for(ii = 0; ii < M; ii += TILEA)
		{
			for(jj = 0; jj < M; jj += TILEB)
			{
				
				for(i = ii; i < MIN(M, ii + TILEA); i+=4) 
				{
					a1 = _mm256_setzero_ps();
					b1 = _mm256_setzero_ps();
					c1 = _mm256_setzero_ps();
					d1 = _mm256_setzero_ps();
					
					#pragma omp simd aligned(A1,X,temp:64)
					for(j = jj; j < MIN(M, jj + TILEB); j+=8) 
					{
						a5 = _mm256_load_ps(X + j);
						a0 = _mm256_load_ps(&A1[i][j]);
						a1 = _mm256_fmadd_ps(a0, a5, a1);
						
						b5 = _mm256_load_ps(X + j);
						b0 = _mm256_load_ps(&A1[i + 1][j]);
						b1 = _mm256_fmadd_ps(b0, b5, b1);
						
						c5 = _mm256_load_ps(X + j);
						c0 = _mm256_load_ps(&A1[i + 2][j]);
						c1 = _mm256_fmadd_ps(c0, c5, c1);
						
						d5 = _mm256_load_ps(X + j);
						d0 = _mm256_load_ps(&A1[i + 3][j]);
						d1 = _mm256_fmadd_ps(d0, d5, d1);
					}
					
					a2 = _mm256_permute2f128_ps(a1, a1, 1);
					a1 = _mm256_add_ps(a1, a2);
					a1 = _mm256_hadd_ps(a1, a1);
					a1 = _mm256_hadd_ps(a1, a1);
					a4 = _mm256_extractf128_ps(a1, 0);
					_mm_store_ss(temp + i, a4);
					
					b2 = _mm256_permute2f128_ps(b1, b1, 1);
					b1 = _mm256_add_ps(b1, b2);
					b1 = _mm256_hadd_ps(b1, b1);
					b1 = _mm256_hadd_ps(b1, b1);
					b4 = _mm256_extractf128_ps(b1, 0);
					_mm_store_ss(temp + (i+1), b4);
					
					c2 = _mm256_permute2f128_ps(c1, c1, 1);
					c1 = _mm256_add_ps(c1, c2);
					c1 = _mm256_hadd_ps(c1, c1);
					c1 = _mm256_hadd_ps(c1, c1);
					c4 = _mm256_extractf128_ps(c1, 0);
					_mm_store_ss(temp + (i+2), c4);
					
					d2 = _mm256_permute2f128_ps(d1, d1, 1);
					d1 = _mm256_add_ps(d1, d2);
					d1 = _mm256_hadd_ps(d1, d1);
					d1 = _mm256_hadd_ps(d1, d1);
					d4 = _mm256_extractf128_ps(d1, 0);
					_mm_store_ss(temp + (i+3), d4);
					

				}
			}
		}
		
		for(int i = 0; i < M; i++)
		{
			#pragma omp atomic
			Y[i] += temp[i];
		}
		
	}
	
	return 1;
}



unsigned short int Compare_MVM() {

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			test1[i] += A1[i][j] * X[j];
		}
	}

	for (int j = 0; j < M; j++)
		if (equal(Y[j], test1[j]) == 1) {
			//printf("\n j=%d\n", j);
			return 1;
		}

	return 0;
}


unsigned short int equal(float const a, float const b) {
	float temp = a - b;
	//printf("\n %f  %f", a, b);
	if (fabs(temp) < EPSILON)
		return 0; //success
	else
		return 1;
}
