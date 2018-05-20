#pragma once
#include <string>
#include "FreeImage.h"

class Renderer{
	public:
		Renderer(int w, int h);
		~Renderer();
		void createImage(BYTE* pixels, std::string outputFileName);

	private:
		int width;
		int height;
};

