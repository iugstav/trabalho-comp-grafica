#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <limits>
#include <vector>

typedef struct {
	float L;
	float a;
	float b;
} Lab;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGB;

Lab rgb2Lab(const RGB &pixel) {
	auto linearize = [](float c) -> float {
		if (c <= 0.04045f)
			return c / 12.92f;
		return std::pow((c + 0.055f) / 1.055f, 2.4f); // [TODO]: optimize
	};

	// normaliza cada componente rgb pro espaço [0,1]
	float r_norm = pixel.r / 255.0f;
	float g_norm = pixel.g / 255.0f;
	float b_norm = pixel.b / 255.0f;
	// lineariza os valores normalizados para inverter a correção de gama
	float r = linearize(r_norm);
	float g = linearize(g_norm);
	float b = linearize(b_norm);

	// aplica a matriz de transformação para o espaço de cores CIE XYZ
	float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
	float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
	float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

	// normalização pela referência white point d65
	float x_d65 = X / 0.95047f;
	float y_d65 = Y;
	float z_d65 = Z / 1.08883f;

	// função de ajuste não-linear para Lab
	const float delta = 6.0f / 29.0f;
	auto f = [&](float t) -> float {
		if (t > delta * delta * delta)
			return std::cbrt(t); // [TODO]: otimizar
		return (t / (3 * delta * delta)) + (4.0f / 29.0f); // [TODO]: otimizar
	};

	// conversão de XYZ para Lab
	Lab lab;
	lab.L = 116.0f * f(y_d65) - 16.0f;
	lab.a = 500.0f * (f(x_d65) - f(y_d65));
	lab.b = 200.0f * (f(y_d65) - f(z_d65));
	return lab;
}

// constrói com base em `grayLevels` a paleta de tons de cinza
std::vector<RGB> build_gray_Levels(const int levels) {
	std::vector<RGB> l;
	for (int i = 0; i < levels; ++i) {
		unsigned char gray = static_cast<unsigned char>((255 * i) / (levels - 1));
		l.push_back({gray, gray, gray});
	}

	return l;
}

// encontra o índice de cor mais próximo no espaço Lab
int find_nearest_color(const Lab &pixel, const std::vector<Lab> &palette) {
	int best_idx = 0;
	float best_distance = std::numeric_limits<float>::max(), dist;
	for (int i = 0; i < int(palette.size()); ++i) {
		float dL = pixel.L - palette[i].L;
		float da = pixel.a - palette[i].a;
		float db = pixel.b - palette[i].b;

		dist = dL * dL + da * da + db * db;
		if (dist < best_distance) {
			best_idx = i;
			best_distance = dist;
		}
	}

	return best_idx;
}

#endif
