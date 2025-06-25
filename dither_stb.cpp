#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

void dithering(unsigned char* img, int width, int height){
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int i = y * width + x;
			int old = img[i];
			int new_pixel;
			if (old < 128){
				new_pixel = 0;
			} else {
				new_pixel = 255;
			}
			int erro = old - new_pixel;
			if (x+1 < width){
				img[i+1] = clamp(img[i+1]+erro*7/16, 0, 255);
			}
			if (y+1 < heigth){
				if (x>0){
                                	img[i+width-1] = clamp(img[i+width-1]+erro*3/16, 0, 255);
					img[i+width] = clamp(img[i+width]+erro*5/16, 0, 255);
					if (x+1<width){
						img[i+width+1] = clamp(img[i+width+1]+erro*1/16, 0, 255)
					}
                        	}
			}
		}
	}
}
int main() {
	string input_file = "entrada.jpg";
	string output_file = "saida.png";

	int width, height, channels;
	unsigned char* img = stbi_load(input_file.c_str(), &width, &height, &channels, 1);
	if (!img) {
        	cerr << "Erro ao carregar a imagem.\n" << input_file << "\n";
        	return 1;
	}

	cout << "Imagem carregada: " << input_file << "(" << width << " x " << height << ")\n";

	dithering(img, width, height);
	if (!stbi_write_png(output_file.c_str(), width, height, 1, img, width) {
        	cerr << "Erro ao salvar a imagem.\n" << output_file << "\n";
		stbi_image_free(img);
		return 2;
	}
	cout << "Dithering concluído com sucesso.\n";
	cout << "Imagem salva como: " << output_file << "\n";

	stbi_image_free(img);
	return 0;
}

