#include "Canvas.h"
#include <stdio.h>
#include <math.h>

Canvas::Canvas()
{
	m_width = 0;
	m_height = 0;
	m_isChanged = false;
}

Canvas::~Canvas()
{
}

void Canvas::Init(int width, int height)
{
	m_width = width;
	m_height = height;
	m_pixels.resize(width * height);
	Clear();
}

// 칠판 전체를 배경색으로 채운다
void Canvas::Clear()
{
	for (int i = 0; i < m_width * m_height; i++)
	{
		m_pixels[i] = BOARD_COLOR;
	}
	m_isChanged = true;
}

// 원 하나 찍기
void Canvas::DrawCircle(int cx, int cy, int radius, unsigned int color)
{
	for (int y = -radius; y <= radius; y++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			// 원 안에 있는 점만
			if (x * x + y * y > radius * radius)
				continue;

			int px = cx + x;
			int py = cy + y;

			// 화면 밖이면 안 그림
			if (px < 0 || px >= m_width || py < 0 || py >= m_height)
				continue;

			m_pixels[py * m_width + px] = color;
		}
	}
	m_isChanged = true;
}

// 두 점 사이를 원으로 채워서 선을 그린다
// (마우스를 빨리 움직이면 점이 띄엄띄엄 찍히는 문제 때문에 만듦)
void Canvas::DrawLine(int x0, int y0, int x1, int y1, int radius, unsigned int color)
{
	float dx = (float)(x1 - x0);
	float dy = (float)(y1 - y0);
	float length = sqrtf(dx * dx + dy * dy);

	int steps = (int)length;
	if (steps < 1)
		steps = 1;

	for (int i = 0; i <= steps; i++)
	{
		float t = (float)i / steps;
		int x = (int)(x0 + dx * t);
		int y = (int)(y0 + dy * t);
		DrawCircle(x, y, radius, color);
	}
}

// 파일 형식: [가로(int)][세로(int)][픽셀 데이터]
bool Canvas::Save(const char* fileName)
{
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "wb");
	if (fp == NULL)
		return false;

	fwrite(&m_width, sizeof(int), 1, fp);
	fwrite(&m_height, sizeof(int), 1, fp);
	fwrite(&m_pixels[0], sizeof(unsigned int), m_width * m_height, fp);

	fclose(fp);
	return true;
}

bool Canvas::Load(const char* fileName)
{
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "rb");
	if (fp == NULL)
		return false;

	int w = 0, h = 0;
	fread(&w, sizeof(int), 1, fp);
	fread(&h, sizeof(int), 1, fp);

	// 크기가 다르면 못 불러옴
	if (w != m_width || h != m_height)
	{
		fclose(fp);
		return false;
	}

	size_t count = fread(&m_pixels[0], sizeof(unsigned int), m_width * m_height, fp);
	fclose(fp);

	if (count != (size_t)(m_width * m_height))
	{
		// 파일이 잘못됐으면 그냥 지움
		Clear();
		return false;
	}

	m_isChanged = true;
	return true;
}
