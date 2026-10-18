#pragma once
#include <windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

struct Vertex
{
	float x, y, z;
	float u, v;
};

class Graphics
{
public:
	Graphics();
	~Graphics();

	bool Init(HWND hWnd, int width, int height);
	void Release();

	bool CreateCanvasTexture(int width, int height);
	void UpdateCanvasTexture(unsigned int* pixels, int width, int height);
	void Render();

private:
	bool CreateShader();
	bool CreateQuad();

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRTV;

	ID3D11VertexShader* m_pVS;
	ID3D11PixelShader* m_pPS;
	ID3D11InputLayout* m_pLayout;
	ID3D11Buffer* m_pVB;

	// 칠판 텍스처
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11SamplerState* m_pSampler;

	int m_width;
	int m_height;
};
