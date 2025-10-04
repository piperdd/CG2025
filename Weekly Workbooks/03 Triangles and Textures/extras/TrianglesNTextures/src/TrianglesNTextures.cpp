#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	std::vector<float> result;
	float spacing;
	spacing = (to - from) / (numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++) {
		result.push_back(from+spacing*i);
	}
	return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	std::vector<glm::vec3> result;
	glm::vec3 spacing;
	spacing = (to - from) / float(numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++) {
		result.push_back(from+spacing * float(i));
	}
	return result;
}

std::vector<std::vector<glm::vec3>> colourInterpolation2D() {
	glm::vec3 topLeft(255, 0, 0);        // red
	glm::vec3 topRight(0, 0, 255);       // blue
	glm::vec3 bottomRight(0, 255, 0);    // green
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	std::vector<glm::vec3> firstCol, lastCol;
	std::vector<std::vector<glm::vec3>> canvas;
	std::vector<glm::vec3> temp;
	firstCol = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
	lastCol = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);
	for (size_t y = 0; y < HEIGHT; y++) {
		temp = interpolateThreeElementValues(firstCol[y],lastCol[y],WIDTH);
		canvas.push_back(temp);
	}
	return canvas;
}

// we have convertToBarycentricCoordinates() : x, y -> u, v, w
// we want a function, barycentricToRGB(): u, v, w -> r, g, b
glm::vec3 barycentricToRGB(glm::vec3 pos) {
	return pos * float(255);
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	// we want to "standardise"/scale the values of 0-256 to the size of the window
	// float scalingFactor = WIDTH/256
	// input 319 -> output 255
	// window max = 319
	// window min = 0
	// rgb max = 255
	// rgb min = 0
	// formula rgb = 319 / (319-0) * 255

	// std::vector<std::vector<glm::vec3>> canvas = colourInterpolation2D();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			// RED NOISE
			// float red = rand() % 256;
			// float green = 0.0;
			// float blue = 0.0;

			// GREYSCALE INTERPOLATION
			// float red = (window.width-x-1) * 255 / 319;
			// float green = (window.width-x-1) * 255 / 319;
			// float blue = (window.width-x-1) * 255 / 319;

			// 2D COLOUR INTERPOLATION
			// float red = canvas[y][x].x;
			// float green = canvas[y][x].y;
			// float blue = canvas[y][x].z;

			// BARYCENTRIC TRIANGULAR INTERPOLATION
			glm::vec2 v0(0, HEIGHT), v1(WIDTH/2, 0),v2(WIDTH, HEIGHT);
			glm::vec3 pixelColour = barycentricToRGB(convertToBarycentricCoordinates(v0, v1, v2, glm::vec2(x,y)));
			float minVal = std::min({pixelColour.x, pixelColour.y, pixelColour.z});

			float red = 0;
			float green = 0;
			float blue = 0;
			if (minVal >= 0) {
				red = pixelColour.x;
				green = pixelColour.y;
				blue = pixelColour.z;
			}

			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	glm::vec3 from(1.0, 4.0, 9.2);
	glm::vec3 to(4.0, 1.0, 9.8);
	std::vector<glm::vec3> resultVec3;
	resultVec3 = interpolateThreeElementValues(from, to, 4);
	for(size_t i=0; i<resultVec3.size(); i++) {
		std::cout << resultVec3[i].x << " " << resultVec3[i].y << " " << resultVec3[i].z << " ";
		std::cout << std::endl;
	}

	std::vector<float> result;
	result = interpolateSingleFloats(2.2, 8.5, 7);
	for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
	std::cout << std::endl;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
