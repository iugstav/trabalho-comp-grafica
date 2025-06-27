#include "color.h"
#include "image.h"
#include <cmath>

void atkinsonDither(const std::vector<RGB> &inData, std::vector<RGB> &outData, int width, int height) {
	int npix = width * height;
	std::vector<Lab> buf(npix);
	for (int i = 0; i < npix; ++i)
		buf[i] = rgb2Lab(inData[i]);

	// Paleta em Lab
	auto palette = build_palette();
	outData.resize(npix);

	std::vector<RGB> levels;
	for (int i = 0; i < grayLevels; ++i) {
		unsigned char gray = static_cast<unsigned char>((255 * i) / (grayLevels - 1));
		levels.push_back({gray, gray, gray});
	}

	// Kernel de Atkinson
	const int dx[6] = {1, 2, -1, 0, 1, 0};
	const int dy[6] = {0, 0, 1, 1, 1, 2};

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;
			Lab oldLab = buf[idx];
			int pi = find_nearest_color(oldLab, palette);
			Lab best = palette[pi];

			// Convertendo paleta de volta a RGB
			outData[idx] = levels[pi];

			// Erro em Lab
			Lab err;
			err.L = oldLab.L - best.L;
			err.a = oldLab.a - best.a;
			err.b = oldLab.b - best.b;
			// Fixar pixel
			buf[idx] = best;
			// Difundir erro
			for (int k = 0; k < 6; ++k) {
				int nx = x + dx[k];
				int ny = y + dy[k];

				if (nx < 0 || nx >= width || ny < 0 || ny >= height)
					continue;

				int nidx = ny * width + nx;
				buf[nidx].L += err.L / 8.0f;
				buf[nidx].a += err.a / 8.0f;
				buf[nidx].b += err.b / 8.0f;
			}
		}
	}
}

int main(void) {
	std::vector<RGB> data;
	int width, height, maxValue;
	if (!readPPM("output.ppm", data, width, height, maxValue)) {
		return 1;
	}

	std::vector<RGB> output;
	atkinsonDither(data, output, width, height);
	if (!writePPM("dithering.ppm", output, width, height, maxValue)) {
		return 1;
	}

	return 0;
}
