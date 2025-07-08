#ifndef POISSON_H
#define POISSON_H

#include <cmath>
#include <iostream>
#include <vector>

#define TOL 1e-7f
#define MAX_IT 10000

void SolPoisson(int n, int m, float *sol) {
	// passo da malha
	float h = 4.0f / n;
	float k = 2.0f / m;
	float h2 = 1.0f / (h * h), k2 = 1.0f / (k * k);
	int N = n + 1, M = m + 1;

	// aloca U incluindo contornos
	std::vector<std::vector<float>> U(N, std::vector<float>(M));

	// 1) inicializa fronteiras
	for (int j = 0; j < M; ++j) {
		float y = j * k;
		U[0][j] = 0.0f; // u(0,y)
		U[n][j] = 2.0f * expf(y); // u(2,y)
	}
	for (int i = 0; i < N; ++i) {
		float x = i * h;
		U[i][0] = x; // u(x,0)
		U[i][m] = expf(x); // u(x,1)
	}

	// 2) chute inicial nos interiores (por exemplo média dos vizinhos de contorno)
	for (int i = 1; i < n; ++i)
		for (int j = 1; j < m; ++j)
			U[i][j] = (U[0][j] + U[n][j] + U[i][0] + U[i][m]) * 0.25f;

	// 3) Gauss–Seidel iterativo
	for (int iter = 0; iter < MAX_IT; ++iter) {
		float maxDiff = 0.0f;
		for (int i = 1; i < n; ++i) {
			for (int j = 1; j < m; ++j) {
				float old = U[i][j];
				// f = 0 para Laplace; se f≠0, subtrair f/(h^-2+k^-2)
				U[i][j] = (h2 * (U[i + 1][j] + U[i - 1][j]) + k2 * (U[i][j + 1] + U[i][j - 1])) /
				    (2.0f * (h2 + k2));
				maxDiff = std::max(maxDiff, fabsf(U[i][j] - old));
			}
		}
		if (maxDiff < TOL) {
			std::cout << "Convergiu em " << iter << " iterações (erro " << maxDiff << ")\n";
			break;
		}
	}

	// 4) copia só os interiores para o array sol[]
	int idx = 0;
	for (int j = 1; j < m; ++j)
		for (int i = 1; i < n; ++i)
			sol[idx++] = U[i][j];
}

#endif
