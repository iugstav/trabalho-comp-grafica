#ifndef POISSON_H
#define POISSON_H

#include <cmath>
void SolPoisson(int n, int m, float *sol) {
	float h, k, x, y;
	int indice = 0;

	h = 2.0 / float(n);
	k = 1.0 / float(m);
	for (int j = 1; j < m; ++j) {
		for (int i = 1; i < n; ++i) {
			x = float(i) * h;
			y = float(j) * k;
			sol[indice] = x * exp(y);
			++indice;
		}
	}
}

#endif
