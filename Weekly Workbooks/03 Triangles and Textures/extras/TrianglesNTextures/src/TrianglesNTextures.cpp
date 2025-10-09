#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

#define STROKED 0
#define FILLED 1

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

void line(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
	float xDiff = (to.x-from.x);
	float yDiff = (to.y-from.y);
	float numSteps = std::max(abs(xDiff), abs(yDiff));
	if (numSteps == 0) {
		uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
		window.setPixelColour(round(from.x), round(from.y), uintColour);
		return;
	}
	float xStepSize = xDiff/numSteps;
	float yStepSize = yDiff/numSteps;
	// repeated addition instead of multiplication (out of bounds)
	// float x = from.x;
	// float y = from.y;
	// for (int i = 0; i <= numSteps; i++) {
	// 	x += xStepSize;
	// 	y += yStepSize;
	// 	uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	// 	window.setPixelColour(int(x), int(y), uintColour);
	// }

	for (float i=0.0; i<=numSteps; i++) {
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;
		uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
		window.setPixelColour(round(x), round(y), uintColour);
	}

}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	line(window, triangle.v0(), triangle.v1(), colour);
	line(window, triangle.v1(), triangle.v2(), colour);
	line(window, triangle.v2(), triangle.v0(), colour);
}

Colour randomColour() {
	return Colour(rand() % 256, rand() % 256, rand() % 256);
}

CanvasTriangle randomTriangle () {
	CanvasPoint v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	return CanvasTriangle(v0, v1, v2);
}

CanvasPoint pointOnLineY(float y, CanvasPoint p1, CanvasPoint p2) {
	float proportion = (y - p1.y) / (p2.y - p1.y);
	float x = proportion * (p2.x - p1.x) + p1.x;
	return CanvasPoint(x, y);
}

std::vector<CanvasTriangle> splitTriangle(CanvasTriangle triangle) {
	CanvasTriangle top, bottom;
	std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
	std::sort(points.begin(),points.end(),
		[](CanvasPoint p1, CanvasPoint p2) {
		return (p1.y < p2.y);
	});
	// for each y from p1.y to p2.y, we want to print a line
	// the line goes from proportionalPoint(yVal, p1, p2) to proportionalPoint(yVal, p1, p3)
	// proportionalPoint(Y, p1, p2) returns a point(X,Y) on a line

	CanvasPoint newPoint = pointOnLineY(points[1].y, points[0], points[2]);

	top.v0() = points[0];
	top.v1() = points[1];
	top.v2() = newPoint;

	bottom.v0() = points[1];
	bottom.v1() = newPoint;
	bottom.v2() = points[2];

	return {top, bottom};
}

int getHeight(CanvasTriangle triangle) {
	return std::max({triangle.v0().y, triangle.v1().y, triangle.v2().y}) - std::min({triangle.v0().y, triangle.v1().y, triangle.v2().y});
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	std::vector<CanvasTriangle> triangleSegments = splitTriangle(triangle);
	CanvasTriangle topTriangle = triangleSegments[0];
	int height = getHeight(topTriangle);
	// std::cout << height << std::endl;
	// std::cout << triangleSegment.v0() << std::endl;
	for (int i = 0; i < height; i++) {
		int yVal = topTriangle.v0().y + i;
		CanvasPoint p1 = pointOnLineY(yVal, topTriangle.v0(), topTriangle.v1());
		CanvasPoint p2 = pointOnLineY(yVal, topTriangle.v0(), topTriangle.v2());
		line(window,p1,p2,colour);
	}

	CanvasTriangle bottomTriangle = triangleSegments[1];
	height = getHeight(bottomTriangle);
	for (int i = 0; i < height; i++) {
		int yVal = bottomTriangle.v0().y + i;
		CanvasPoint p1 = pointOnLineY(yVal, bottomTriangle.v2(), bottomTriangle.v1());
		CanvasPoint p2 = pointOnLineY(yVal, bottomTriangle.v2(), bottomTriangle.v0());
		line(window,p1,p2,colour);
	}

	Colour white = Colour(255,255,255);
	drawStrokedTriangle(window, triangle, white);
}

void drawRandomTriangle(DrawingWindow &window, int type = STROKED) {
	Colour randColour = randomColour();
		CanvasTriangle randTriangle = randomTriangle();
	if (type == STROKED) {
		drawStrokedTriangle(window, randTriangle, randColour);
	}else {
		drawFilledTriangle(window, randTriangle, randColour);
	}
}

void draw(DrawingWindow &window) {
	// window.clearPixels();
	//	Task 2: Line Drawing
	CanvasPoint centre = CanvasPoint(WIDTH/2, HEIGHT/2);
	CanvasPoint top = CanvasPoint(WIDTH/2, 0);
	CanvasPoint bottom = CanvasPoint(WIDTH/2, HEIGHT-1);
	CanvasPoint topLeft = CanvasPoint(0, 0);
	CanvasPoint topRight = CanvasPoint(WIDTH-1, 0);
	CanvasPoint middleLeftThird = CanvasPoint(WIDTH/3, HEIGHT/2);
	CanvasPoint middleRightThird = CanvasPoint(2*WIDTH/3, HEIGHT/2);

	Colour white = Colour(255,255,255);
	line(window,  topLeft, centre, white);
	line(window,  topRight, centre, white);
	line(window,  top, bottom, white);
	line(window,  middleLeftThird,  middleRightThird, white);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) drawRandomTriangle(window);
		else if (event.key.keysym.sym == SDLK_f) drawRandomTriangle(window, FILLED);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
