#include "Graphics.h"
#include <d3dcompiler.h>

Graphics::Graphics()
{
	m_pDevice = NULL;
	m_pContext = NULL;
	m_pSwapChain = NULL;
	m_pRTV = NULL;
	m_pVS = NULL;
	m_pPS = NULL;
	m_pLayout = NULL;
	m_pVB = NULL;
	m_pTexture = NULL;
	m_pSRV = NULL;
	m_pSampler = NULL;
	m_width = 0;
	m_height = 0;
}

Graphics::~Graphics()
{
	Release();
}

bool Graphics::Init(HWND hWnd, int width, int height)
{
	m_width = width;
	m_height = height;

	// 스왑체인 설정
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// 디바이스, 스왑체인 생성
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
		&featureLevel, 1, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, NULL, &m_pContext);

	if (FAILED(hr))
	{
		// 그래픽카드가 안 되면 WARP(소프트웨어)로 다시 시도
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0,
			&featureLevel, 1, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, NULL, &m_pContext);
		if (FAILED(hr))
		{
			MessageBox(hWnd, L"디바이스 생성 실패", L"에러", MB_OK);
			return false;
		}
	}

	// 렌더 타겟 뷰 생성
	ID3D11Texture2D* pBackBuffer = NULL;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTV);
	pBackBuffer->Release();
	if (FAILED(hr))
		return false;

	m_pContext->OMSetRenderTargets(1, &m_pRTV, NULL);

	// 뷰포트
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pContext->RSSetViewports(1, &vp);

	if (!CreateShader())
	{
		MessageBox(hWnd, L"셰이더 파일(Shader.hlsl)을 찾을 수 없거나 컴파일 실패", L"에러", MB_OK);
		return false;
	}

	if (!CreateQuad())
		return false;

	return true;
}

bool Graphics::CreateShader()
{
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pError = NULL;

	// 버텍스 셰이더
	HRESULT hr = D3DCompileFromFile(L"Shader.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &pVSBlob, &pError);
	if (FAILED(hr))
	{
		if (pError)
		{
			OutputDebugStringA((char*)pError->GetBufferPointer());
			pError->Release();
		}
		return false;
	}

	m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVS);

	// 입력 레이아웃
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	m_pDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pLayout);
	pVSBlob->Release();

	// 픽셀 셰이더
	hr = D3DCompileFromFile(L"Shader.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &pPSBlob, &pError);
	if (FAILED(hr))
	{
		if (pError)
		{
			OutputDebugStringA((char*)pError->GetBufferPointer());
			pError->Release();
		}
		return false;
	}

	m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPS);
	pPSBlob->Release();

	return true;
}

// 화면 전체를 덮는 사각형
bool Graphics::CreateQuad()
{
	Vertex vertices[] =
	{
		{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },	// 왼쪽 위
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },	// 오른쪽 위
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },	// 왼쪽 아래
		{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },	// 오른쪽 아래
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	HRESULT hr = m_pDevice->CreateBuffer(&bd, &initData, &m_pVB);
	if (FAILED(hr))
		return false;

	return true;
}

// 칠판 그림을 담을 텍스처 (CPU에서 매번 써야 하니까 DYNAMIC)
bool Graphics::CreateCanvasTexture(int width, int height)
{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DYNAMIC;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_pDevice->CreateTexture2D(&td, NULL, &m_pTexture);
	if (FAILED(hr))
		return false;

	hr = m_pDevice->CreateShaderResourceView(m_pTexture, NULL, &m_pSRV);
	if (FAILED(hr))
		return false;

	// 샘플러
	D3D11_SAMPLER_DESC samp;
	ZeroMemory(&samp, sizeof(samp));
	samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samp.MaxLOD = D3D11_FLOAT32_MAX;

	hr = m_pDevice->CreateSamplerState(&samp, &m_pSampler);
	if (FAILED(hr))
		return false;

	return true;
}

// CPU에 있는 픽셀 데이터를 텍스처로 복사
void Graphics::UpdateCanvasTexture(unsigned int* pixels, int width, int height)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = m_pContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
		return;

	// 한 줄씩 복사 (RowPitch가 가로 크기랑 다를 수 있어서)
	for (int y = 0; y < height; y++)
	{
		BYTE* dest = (BYTE*)mapped.pData + y * mapped.RowPitch;
		memcpy(dest, pixels + y * width, width * 4);
	}

	m_pContext->Unmap(m_pTexture, 0);
}

void Graphics::Render()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pContext->ClearRenderTargetView(m_pRTV, clearColor);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
	m_pContext->IASetInputLayout(m_pLayout);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pContext->VSSetShader(m_pVS, NULL, 0);
	m_pContext->PSSetShader(m_pPS, NULL, 0);
	m_pContext->PSSetShaderResources(0, 1, &m_pSRV);
	m_pContext->PSSetSamplers(0, 1, &m_pSampler);

	m_pContext->Draw(4, 0);

	m_pSwapChain->Present(1, 0);
}

void Graphics::Release()
{
	if (m_pContext)
		m_pContext->ClearState();

	SAFE_RELEASE(m_pSampler);
	SAFE_RELEASE(m_pSRV);
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pLayout);
	SAFE_RELEASE(m_pPS);
	SAFE_RELEASE(m_pVS);
	SAFE_RELEASE(m_pRTV);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pContext);
	SAFE_RELEASE(m_pDevice);
}
