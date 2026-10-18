// 칠판 그리기 (DirectX 11)
// - 칠판 위에서 마우스를 누른 채로 움직이면 그림이 그려짐
// - 저장 버튼 : save.dat 파일로 저장
// - 불러오기 버튼 : save.dat 파일을 읽어서 칠판에 다시 표시

#include <windows.h>
#include "Graphics.h"
#include "Canvas.h"

// 창 크기
#define WINDOW_WIDTH  1100
#define WINDOW_HEIGHT 640

// 칠판 위치, 크기
#define BOARD_X      160
#define BOARD_Y      10
#define BOARD_WIDTH  920
#define BOARD_HEIGHT 580

// 버튼 ID
#define ID_BTN_SAVE     101
#define ID_BTN_LOAD     102
#define ID_BTN_CLEAR    103
#define ID_BTN_WHITE    201
#define ID_BTN_YELLOW   202
#define ID_BTN_RED      203
#define ID_BTN_BLUE     204
#define ID_BTN_ERASER   205
#define ID_BTN_SMALL    301
#define ID_BTN_MEDIUM   302
#define ID_BTN_BIG      303

#define SAVE_FILE "save.dat"

// 전역 변수
HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_hBoard;		// 칠판 (DirectX가 그려지는 창)
HWND g_hStatus;		// 현재 상태 표시 글자
HFONT g_hFont;		// 버튼 글꼴

Graphics g_graphics;
Canvas g_canvas;

bool g_isDrawing = false;
int g_lastX = 0;
int g_lastY = 0;

unsigned int g_color = MAKE_COLOR(255, 255, 255);
int g_brushSize = 4;
const wchar_t* g_colorName = L"흰색";
const wchar_t* g_sizeName = L"보통";

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BoardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 상태 글자 갱신
void UpdateStatus()
{
	wchar_t text[100];
	wsprintf(text, L"색: %s\n굵기: %s", g_colorName, g_sizeName);
	SetWindowText(g_hStatus, text);
}

// 버튼 만들기
HWND MakeButton(const wchar_t* text, int x, int y, int w, int h, int id)
{
	HWND hBtn = CreateWindow(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h, g_hWnd, (HMENU)(INT_PTR)id, g_hInst, NULL);
	return hBtn;
}

void CreateControls()
{
	int x = 15;
	int w = 130;

	// 기본 글꼴이 안 예뻐서 맑은 고딕으로 바꿈
	g_hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGUL_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");

	CreateWindow(L"STATIC", L"[ 색상 ]", WS_CHILD | WS_VISIBLE, x, 15, w, 20, g_hWnd, NULL, g_hInst, NULL);
	MakeButton(L"흰색", x, 35, w, 28, ID_BTN_WHITE);
	MakeButton(L"노랑", x, 65, w, 28, ID_BTN_YELLOW);
	MakeButton(L"빨강", x, 95, w, 28, ID_BTN_RED);
	MakeButton(L"파랑", x, 125, w, 28, ID_BTN_BLUE);
	MakeButton(L"지우개", x, 155, w, 28, ID_BTN_ERASER);

	CreateWindow(L"STATIC", L"[ 굵기 ]", WS_CHILD | WS_VISIBLE, x, 200, w, 20, g_hWnd, NULL, g_hInst, NULL);
	MakeButton(L"얇게", x, 220, w, 28, ID_BTN_SMALL);
	MakeButton(L"보통", x, 250, w, 28, ID_BTN_MEDIUM);
	MakeButton(L"굵게", x, 280, w, 28, ID_BTN_BIG);

	CreateWindow(L"STATIC", L"[ 파일 ]", WS_CHILD | WS_VISIBLE, x, 325, w, 20, g_hWnd, NULL, g_hInst, NULL);
	MakeButton(L"저장", x, 345, w, 35, ID_BTN_SAVE);
	MakeButton(L"불러오기", x, 385, w, 35, ID_BTN_LOAD);
	MakeButton(L"전체 지우기", x, 440, w, 28, ID_BTN_CLEAR);

	g_hStatus = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, x, 490, w, 40, g_hWnd, NULL, g_hInst, NULL);

	// 버튼, 글자 전부 폰트 적용
	HWND hChild = GetWindow(g_hWnd, GW_CHILD);
	while (hChild)
	{
		SendMessage(hChild, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		hChild = GetWindow(hChild, GW_HWNDNEXT);
	}

	UpdateStatus();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	// 메인 창 클래스
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"MyPaintClass";
	RegisterClassEx(&wc);

	// 칠판 창 클래스
	WNDCLASSEX wcBoard;
	ZeroMemory(&wcBoard, sizeof(wcBoard));
	wcBoard.cbSize = sizeof(WNDCLASSEX);
	wcBoard.lpfnWndProc = BoardProc;
	wcBoard.hInstance = hInstance;
	wcBoard.hCursor = LoadCursor(NULL, IDC_CROSS);
	wcBoard.lpszClassName = L"BoardClass";
	RegisterClassEx(&wcBoard);

	// 창 크기 고정 (크기 조절 안 됨)
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, style, FALSE);

	g_hWnd = CreateWindow(L"MyPaintClass", L"칠판 그리기", style,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, hInstance, NULL);

	g_hBoard = CreateWindow(L"BoardClass", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
		BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, g_hWnd, NULL, hInstance, NULL);

	// 테두리 때문에 실제 그리는 영역 크기를 다시 구함
	RECT boardRect;
	GetClientRect(g_hBoard, &boardRect);
	int boardW = boardRect.right;
	int boardH = boardRect.bottom;

	CreateControls();

	// DirectX 초기화
	if (!g_graphics.Init(g_hBoard, boardW, boardH))
		return 0;

	g_canvas.Init(boardW, boardH);
	if (!g_graphics.CreateCanvasTexture(boardW, boardH))
	{
		MessageBox(g_hWnd, L"텍스처 생성 실패", L"에러", MB_OK);
		return 0;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	// 메인 루프
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// 그림이 바뀌었을 때만 텍스처 업데이트
			if (g_canvas.m_isChanged)
			{
				g_graphics.UpdateCanvasTexture(g_canvas.GetPixels(), g_canvas.GetWidth(), g_canvas.GetHeight());
				g_canvas.m_isChanged = false;
			}
			g_graphics.Render();
		}
	}

	g_graphics.Release();
	DeleteObject(g_hFont);
	return (int)msg.wParam;
}

// 칠판 창 (마우스로 그리기)
LRESULT CALLBACK BoardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x = (short)LOWORD(lParam);
	int y = (short)HIWORD(lParam);

	switch (message)
	{
	case WM_LBUTTONDOWN:
		g_isDrawing = true;
		g_lastX = x;
		g_lastY = y;
		g_canvas.DrawCircle(x, y, g_brushSize, g_color);
		SetCapture(hWnd);	// 칠판 밖으로 나가도 마우스 계속 받기
		return 0;

	case WM_MOUSEMOVE:
		if (g_isDrawing)
		{
			g_canvas.DrawLine(g_lastX, g_lastY, x, y, g_brushSize, g_color);
			g_lastX = x;
			g_lastY = y;
		}
		return 0;

	case WM_LBUTTONUP:
		g_isDrawing = false;
		ReleaseCapture();
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// 메인 창 (버튼 처리)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// 색상
		case ID_BTN_WHITE:
			g_color = MAKE_COLOR(255, 255, 255);
			g_colorName = L"흰색";
			break;
		case ID_BTN_YELLOW:
			g_color = MAKE_COLOR(255, 230, 80);
			g_colorName = L"노랑";
			break;
		case ID_BTN_RED:
			g_color = MAKE_COLOR(255, 90, 90);
			g_colorName = L"빨강";
			break;
		case ID_BTN_BLUE:
			g_color = MAKE_COLOR(120, 190, 255);
			g_colorName = L"파랑";
			break;
		case ID_BTN_ERASER:
			g_color = BOARD_COLOR;	// 배경색으로 칠하면 지우개
			g_colorName = L"지우개";
			break;

		// 굵기
		case ID_BTN_SMALL:
			g_brushSize = 2;
			g_sizeName = L"얇게";
			break;
		case ID_BTN_MEDIUM:
			g_brushSize = 4;
			g_sizeName = L"보통";
			break;
		case ID_BTN_BIG:
			g_brushSize = 10;
			g_sizeName = L"굵게";
			break;

		// 파일
		case ID_BTN_SAVE:
			if (g_canvas.Save(SAVE_FILE))
				MessageBox(hWnd, L"저장했습니다.", L"저장", MB_OK);
			else
				MessageBox(hWnd, L"저장 실패!", L"저장", MB_OK);
			break;
		case ID_BTN_LOAD:
			if (g_canvas.Load(SAVE_FILE))
				MessageBox(hWnd, L"불러왔습니다.", L"불러오기", MB_OK);
			else
				MessageBox(hWnd, L"저장된 파일이 없습니다.", L"불러오기", MB_OK);
			break;
		case ID_BTN_CLEAR:
			g_canvas.Clear();
			break;
		}
		UpdateStatus();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
