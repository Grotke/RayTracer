#include "Renderer.h"

Renderer::Renderer(int w, int h) : width(w), height(h) {
	FreeImage_Initialise();
}


Renderer::~Renderer() {
	FreeImage_DeInitialise();
}

void Renderer::createImage(BYTE* pixels, string outputFileName) {
	FIBITMAP* img = FreeImage_ConvertFromRawBits(pixels, width, height, width * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
	FreeImage_Save(FIF_PNG, img, outputFileName.c_str(), 0);
}
