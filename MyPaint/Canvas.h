#pragma once
#include <vector>

// 색상 만들기 (R8G8B8A8 형식)
#define MAKE_COLOR(r, g, b) ((unsigned int)(r) | ((unsigned int)(g) << 8) | ((unsigned int)(b) << 16) | 0xFF000000)

// 칠판 배경색 (초록색)
#define BOARD_COLOR MAKE_COLOR(40, 80, 60)

class Canvas
{
public:
	Canvas();
	~Canvas();

	void Init(int width, int height);
	void Clear();

	// 그리기 함수
	void DrawCircle(int cx, int cy, int radius, unsigned int color);
	void DrawLine(int x0, int y0, int x1, int y1, int radius, unsigned int color);

	// 저장 / 불러오기
	bool Save(const char* fileName);
	bool Load(const char* fileName);

	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	unsigned int* GetPixels() { return &m_pixels[0]; }

	bool m_isChanged;	// 텍스처를 다시 올려야 하는지

private:
	int m_width;
	int m_height;
	std::vector<unsigned int> m_pixels;
};
