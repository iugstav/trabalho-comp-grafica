#include "image.h"
#include <cmath>

void atkinsonDither(const std::vector<unsigned char> &inData, std::vector<unsigned char> &outData, int width,
		    int height) {

	size_t pixelsNum = static_cast<size_t>(width) * height;
	std::vector<float> gray(pixelsNum);

	// algoritmo de conversão usando o padrão Rec. 601
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			size_t idx = (static_cast<size_t>(j) * width + i);
			size_t base = idx * 3;
			unsigned char r = inData[base];
			unsigned char g = inData[base + 1];
			unsigned char b = inData[base + 2];
			gray[idx] = 0.299f * r + 0.587f * g + 0.114f * b;
		}
	}

	// dithering
	float oldVal, newVal, err;
	int idx;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			idx = y * width + x;
			oldVal = gray[idx];
			newVal = (oldVal < 128.0f ? 0.0f : 255.0f);
			err = oldVal - newVal;
			gray[idx] = newVal;

			const float factor = err * 0.125f; // divide por 8
			// (x+1, y)
			if (x + 1 < width) {
				gray[y * width + (x + 1)] += factor;
			}
			// (x+2, y)
			if (x + 2 < width) {
				gray[y * width + (x + 2)] += factor;
			}
			// (x-1, y+1)
			if (y + 1 < height && x - 1 >= 0) {
				gray[(y + 1) * width + (x - 1)] += factor;
			}
			// (x, y+1)
			if (y + 1 < height) {
				gray[(y + 1) * width + x] += factor;
			}
			// (x+1, y+1)
			if (y + 1 < height && x + 1 < width) {
				gray[(y + 1) * width + (x + 1)] += factor;
			}
			// (x, y+2)
			if (y + 2 < height) {
				gray[(y + 2) * width + x] += factor;
			}
		}
	}
	// Monta saída: cada pixel se torna (v,v,v) com v = 0 ou 255, clip se necessário
	outData.resize(pixelsNum * 3);
	for (size_t i = 0; i < pixelsNum; ++i) {
		float v = gray[i];
		if (v < 0.0f)
			v = 0.0f;
		if (v > 255.0f)
			v = 255.0f;
		unsigned char u = static_cast<unsigned char>(v < 128.0f ? 0 : 255);
		outData[3 * i] = u;
		outData[3 * i + 1] = u;
		outData[3 * i + 2] = u;
	}
}

int main(void) {
	std::vector<unsigned char> data;
	int width, height, maxValue;
	if (!readPPM("output.ppm", width, height, maxValue, data)) {
		return 1;
	}

	std::vector<unsigned char> output;
	atkinsonDither(data, output, width, height);
	if (!writePPM("dithering.ppm", output, width, height, maxValue)) {
		return 1;
	}

	return 0;
}
