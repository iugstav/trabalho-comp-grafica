#ifndef IMAGE_H
#define IMAGE_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pixel;

bool readPPM(const std::string &filename, int &width, int &height, int &maxValue, std::vector<unsigned char> &pixels) {
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

	// Handle newline after header for P6 format
	file.get();

	// Error checking for header values
	if (magicNumber != "P6" || width <= 0 || height <= 0 || maxValue <= 0 || maxValue > 255) {
		std::cerr << "Error: Invalid PPM header in " << filename << std::endl;
		file.close();
		return false;
	}

	size_t imgSize = static_cast<size_t>(width) * height * 3;
	pixels.resize(width * height * 3); // Resize vector to hold all pixels

	file.read(reinterpret_cast<char *>(pixels.data()), imgSize);
	if (!file) {
		std::cerr << "Erro ao ler dados da imagem, esperado " << imgSize << " bytes\n";
		return false;
	}

	file.close();
	return true;
}

bool writePPM(const std::string &filename, const std::vector<unsigned char> &data, int width, int height, int maxval) {
	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		std::cerr << "Erro ao abrir arquivo de saída: " << filename << "\n";
		return false;
	}
	out << "P6\n" << width << " " << height << "\n" << maxval << "\n";
	size_t imgSize = static_cast<size_t>(width) * height * 3;
	out.write(reinterpret_cast<const char *>(data.data()), imgSize);
	if (!out) {
		std::cerr << "Erro ao escrever dados da imagem\n";
		return false;
	}
	return true;
}

#endif
