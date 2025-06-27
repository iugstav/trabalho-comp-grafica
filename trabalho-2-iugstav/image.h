#ifndef IMAGE_H
#define IMAGE_H

#include "color.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

bool readPPM(const std::string &filename, std::vector<RGB> &pixels, int &width, int &height, int &maxValue) {
	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return false;
	}

	std::string magicNumber;
	file >> magicNumber;

	if (magicNumber != "P6") { // Binary format
		std::cerr << "Error: invalid format. Expected P6, found: " << magicNumber << std::endl;
		return false;
	}

	// Skip comments
	while (file.peek() == '#') {
		file.ignore(256, '\n');
	}

	file >> width >> height;
	file >> maxValue;
	file.get();

	// Error checking for header values
	if (width <= 0 || height <= 0 || maxValue <= 0 || maxValue > 255) {
		std::cerr << "Error: Invalid PPM header in " << filename << std::endl;
		file.close();
		return false;
	}

	int npix = width * height;
	std::vector<unsigned char> raw(npix * 3);
	file.read(reinterpret_cast<char *>(raw.data()), raw.size());
	if (!file) {
		return false;
	}

	pixels.resize(npix);
	for (int i = 0; i < npix; ++i) {
		pixels[i] = {raw[3 * i], raw[3 * i + 1], raw[3 * i + 2]};
	}

	file.close();
	return true;
}

bool writePPM(const std::string &filename, const std::vector<RGB> &data, int width, int height, int maxval) {
	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		std::cerr << "Erro ao abrir arquivo de saída: " << filename << "\n";
		return false;
	}
	out << "P6\n" << width << " " << height << "\n" << maxval << "\n";

	int pixels_num = width * height;
	for (int i = 0; i < pixels_num; ++i) {
		out.put(data[i].r);
		out.put(data[i].g);
		out.put(data[i].b);
	}

	return true;
}

#endif
