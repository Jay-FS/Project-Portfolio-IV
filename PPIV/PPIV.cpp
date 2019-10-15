// PPIV.cpp : Defines the entry point for the application.
//
#pragma region Headers
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>
#pragma comment (lib, "d3d11.lib")

//Keyboard and Mouse
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

#include <vector>
#include <random>
#include <fstream>

#include "framework.h"
#include "PPIV.h"

// Including shader headers
#include "VertexShader.csh"
#include "PixelShader.csh"
#include "GrassPixelShader.csh"
#include "GeometryShader.csh"

// Texture Loading
#include "DDSTextureLoader.h"

#define MAX_LOADSTRING 100

using namespace DirectX;
using namespace std;

#pragma endregion



// Structs

struct My_Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 Tangent;
	XMFLOAT4 Binormal;
};

struct My_Mesh
{
	vector<My_Vertex> vertexList;
	vector<int> indicesList;
};

// VRAM Constant Buffer
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

//Lighting Constant Buffer
struct LightBuffer
{
	XMFLOAT4 lightDir[2];
	XMFLOAT4 lightColor[4];
	XMFLOAT4 lightPos;
	FLOAT lightRadius;
	XMFLOAT3 padding;
	XMFLOAT4 coneDir;
	FLOAT coneSize;
	FLOAT coneRatio;
	FLOAT innerConeRatio;
	FLOAT outerConeRatio;
	//BOOL hasNM;
};

//Instancing Constant Buffer
struct InstanceBuffer
{
	XMMATRIX positions[25];
};


#pragma region Variables
// funtime random color
#define RAND_COLOR XMFLOAT4(rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX),1.0f)

// Global DirectX Objects
const char* meshName = "stumpTanBinorm.mesh";
const char* meshName2 = "LightBulb.mesh";
const char* meshName3 = "UvReverseCube.mesh";
const char* meshName4 = "GrassPlane.mesh";

//Mouse And Keyboard
IDirectInputDevice8* DIKey;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseState;
LPDIRECTINPUT8 DirectInput;

float moveX;
float moveY;
float moveZ;

float rotateX;
float rotateY;

float speed = 0;

XMMATRIX XRotation;
XMMATRIX YRotation;
XMMATRIX camView;


// For init
ID3D11Device* myDev;
IDXGISwapChain* mySwap;
ID3D11DeviceContext* myCon;

// For Drawing 
ID3D11RenderTargetView* myRtv;
D3D11_VIEWPORT myPort;

ID3D11Texture2D* depthStencil;	// z Buffer
ID3D11DepthStencilView* depthView;
ID3D11InputLayout* vLayout;		// vertex layout
ID3D11Buffer* vBuf;				// vertex buffer
ID3D11Buffer* vBufLight;		// vertex buffer for lightbulb
ID3D11Buffer* vBufCube;			// vertex buffer for cube / skybox
ID3D11Buffer* vBufGrassPlane;	// vertex buffer for grass plane
ID3D11Buffer* iBuf;				// Index Buffer
ID3D11Buffer* iBufLight;		// Index Buffer for lightbulb
ID3D11Buffer* iBufCube;			// Index Buffer for cube / skybox
ID3D11Buffer* iBufGrassPlane;	// Index Buffer for grass planen
ID3D11Buffer* cBuf;				// Constant Buffer
ID3D11Buffer* lBuf;				// Light Buffer
ID3D11Buffer* instBuf;			// Instance Buffer
ID3D11VertexShader* vShader;	//HLSL
ID3D11PixelShader* pShader;		//HLSL
ID3D11PixelShader* pShaderGrass;//HLSL
ID3D11GeometryShader* gShader;  //HLSL

//Texture variables

ID3D11ShaderResourceView* textureRV;
ID3D11ShaderResourceView* textureRVAO; // ambient oclusion
ID3D11ShaderResourceView* textureRVNM; // Normal Map
ID3D11ShaderResourceView* textureSky; // SkyBox
ID3D11ShaderResourceView* textureGrass; // Grass
ID3D11SamplerState* samplLinear;

//Rasterizer
ID3D11RasterizerState* rState;

//My Meshes
My_Mesh stump;
My_Mesh lightBulb;
My_Mesh Skybox;
My_Mesh GrassPlane;

// Matricies

XMMATRIX WorldMatrix;
XMMATRIX ViewMatrix;
XMMATRIX ProjectionMatrix;

// Scaling Things
float scaleBy = 0.2f;

// Timing
double countSeconds = 0.0;
__int64 CounterStart = 0;
__int64 oldFrameTime = 0;
int frameCount = 0;
int fps = 0;
double frameTime;
double GetTime();
double GetFrameTime();
void StartTimer();


void LoadMesh(const char*, My_Mesh&, float);
void CleanupDevice();
void Render();
BOOL InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
void ZeroCameraValues(void);

#pragma endregion

#pragma region WinVars


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PPIV, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//Initializing Direct Input
	if (!InitDirectInput(hInstance))
	{
		return FALSE;
	}


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PPIV));

	MSG msg = { 0 };

	// Main message loop:
	while (msg.message != WM_QUIT) //GetMessage(&msg, nullptr, 0, 0)) not for games / realtime simulations
	{

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			frameCount++;
			if (GetTime() > 1.0f)
			{
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}

			frameTime = GetFrameTime();
			DetectInput(frameTime);
			Render();
		}

	}
	CleanupDevice();

	return (int)msg.wParam;
}

#pragma endregion
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PPIV));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PPIV);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

#pragma region Window Things

	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// MY CODE

	//getting size of the window
	RECT myWinR;
	GetClientRect(hWnd, &myWinR);


	// attaching d3d11 to the window
	D3D_FEATURE_LEVEL dx11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swap;
	ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap.BufferCount = 1;
	swap.OutputWindow = hWnd;
	swap.Windowed = true;
	swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //color things
	swap.BufferDesc.Width = myWinR.right - myWinR.left;
	swap.BufferDesc.Height = myWinR.bottom - myWinR.top;
	swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // refer to api
	swap.SampleDesc.Count = 1;

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
		&dx11, 1, D3D11_SDK_VERSION, &swap, &mySwap, &myDev, 0, &myCon);
	if (FAILED(hr))
		return FALSE;

	ID3D11Resource* backbuffer;
	hr = mySwap->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	if (FAILED(hr))
		return FALSE;

	myDev->CreateRenderTargetView(backbuffer, NULL, &myRtv);
	backbuffer->Release();
	if (FAILED(hr))
		return FALSE;

	//Create a depth stencil texture aka a z buffer
	CD3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = swap.BufferDesc.Width;
	depthDesc.Height = swap.BufferDesc.Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	hr = myDev->CreateTexture2D(&depthDesc, nullptr, &depthStencil);
	if (FAILED(hr))
		return FALSE;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = depthDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = myDev->CreateDepthStencilView(depthStencil, &descDSV, &depthView);
	if (FAILED(hr))
		return FALSE;

	myCon->OMSetRenderTargets(1, &myRtv, depthView);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.AntialiasedLineEnable = TRUE;
	rasterDesc.MultisampleEnable = TRUE;

	hr = myDev->CreateRasterizerState(&rasterDesc, &rState);
	if (FAILED(hr))
		return FALSE;
	myCon->RSSetState(rState);

	//the viewport
	myPort.Width = (FLOAT)swap.BufferDesc.Width;
	myPort.Height = (FLOAT)swap.BufferDesc.Height;
	myPort.TopLeftX = 0;
	myPort.TopLeftY = 0;
	myPort.MinDepth = 0;
	myPort.MaxDepth = 1; //ndc depth from 0-1

#pragma endregion

   // Define the input layout
	D3D11_INPUT_ELEMENT_DESC vDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(vDesc);

	hr = myDev->CreateInputLayout(vDesc, numElements, VertexShader, sizeof(VertexShader), &vLayout);

	// set the input layout
	myCon->IASetInputLayout(vLayout);
#pragma region Shaders

	//vertex and pixel shaders
	hr = myDev->CreateVertexShader(VertexShader, sizeof(VertexShader), nullptr, &vShader);
	if (FAILED(hr))
		return FALSE;

	hr = myDev->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, &pShader);
	if (FAILED(hr))
		return FALSE;

	hr = myDev->CreatePixelShader(GrassPixelShader, sizeof(GrassPixelShader), nullptr, &pShaderGrass);
	if (FAILED(hr))
		return FALSE;

	hr = myDev->CreateGeometryShader(GeometryShader, sizeof(GeometryShader), nullptr, &gShader);
	if (FAILED(hr))
		return FALSE;
#pragma endregion

#pragma region Stump Model
	LoadMesh(meshName, stump, 1);

	// load it onto  the card
	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));

	//setting buffer desc
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(My_Vertex) * stump.vertexList.size();
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	//setting the subData for the vertex buffer
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&subData, sizeof(subData));

	subData.pSysMem = stump.vertexList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &vBuf);
	if (FAILED(hr))
		return FALSE;

	// Create index Buffer
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.ByteWidth = sizeof(uint32_t) * stump.indicesList.size();
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.CPUAccessFlags = 0;
	subData.pSysMem = stump.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBuf);
	if (FAILED(hr))
		return FALSE;
#pragma endregion

#pragma region LightBulb Model

	LoadMesh(meshName2, lightBulb, 0.0005f);
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(My_Vertex) * lightBulb.vertexList.size();
	subData.pSysMem = lightBulb.vertexList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &vBufLight);
	if (FAILED(hr))
		return FALSE;

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(uint32_t) * lightBulb.indicesList.size();
	subData.pSysMem = lightBulb.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBufLight);
	if (FAILED(hr))
		return FALSE;

#pragma endregion

#pragma region SkyBox / Cube

	LoadMesh(meshName3, Skybox, 1);
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(My_Vertex) * Skybox.vertexList.size();
	subData.pSysMem = Skybox.vertexList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &vBufCube);
	if (FAILED(hr))
		return FALSE;

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(uint32_t) * Skybox.indicesList.size();
	subData.pSysMem = Skybox.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBufCube);
	if (FAILED(hr))
		return FALSE;

#pragma endregion

#pragma region Grass Plane

	LoadMesh(meshName4, GrassPlane, 1);
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(My_Vertex) * GrassPlane.vertexList.size();
	subData.pSysMem = GrassPlane.vertexList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &vBufGrassPlane);
	if (FAILED(hr))
		return FALSE;

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(uint32_t) * GrassPlane.indicesList.size();
	subData.pSysMem = GrassPlane.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBufGrassPlane);
	if (FAILED(hr))
		return FALSE;
#pragma endregion


	// Set primitive topology
	myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#pragma region Texture

	// Load the Texture
	hr = CreateDDSTextureFromFile(myDev, L"stump.dds", nullptr, &textureRV); // Name of texture
	if (FAILED(hr))
		return FALSE;
	hr = CreateDDSTextureFromFile(myDev, L"stumpAO.dds", nullptr, &textureRVAO); // Name of texture
	if (FAILED(hr))
		return FALSE;
	hr = CreateDDSTextureFromFile(myDev, L"stumpNM.dds", nullptr, &textureRVNM); // Name of texture
	if (FAILED(hr))
		return FALSE;
	hr = CreateDDSTextureFromFile(myDev, L"Grass.dds", nullptr, &textureGrass); // Name of texture
	if (FAILED(hr))
		return FALSE;

	// Create the sample state
	D3D11_SAMPLER_DESC sDesc = {};
	sDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sDesc.MinLOD = 0;
	sDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = myDev->CreateSamplerState(&sDesc, &samplLinear);
	if (FAILED(hr))
		return FALSE;

#pragma endregion

#pragma region Constant Buffers

	// Create the Constant buffer
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.ByteWidth = sizeof(ConstantBuffer);
	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.CPUAccessFlags = 0;
	hr = myDev->CreateBuffer(&bDesc, nullptr, &cBuf);
	if (FAILED(hr))
		return FALSE;

	// Create Light Constant Buffer
	bDesc.ByteWidth = sizeof(LightBuffer);
	hr = myDev->CreateBuffer(&bDesc, nullptr, &lBuf);
	if (FAILED(hr))
		return FALSE;

	// Create Instance Constant Buffer
	bDesc.ByteWidth = sizeof(InstanceBuffer);
	hr = myDev->CreateBuffer(&bDesc, nullptr, &instBuf);
	if (FAILED(hr))
		return FALSE;

	// Placing Different instances
	InstanceBuffer instb1;

	std::random_device rd;
	std::mt19937 gen(rd());
	// Instance buffer positions
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::uniform_real_distribution<> dist(0, 1);
			instb1.positions[(i * 5) + j] = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.2f, 0.2f, 0.2f));
			instb1.positions[(i * 5) + j] = XMMatrixMultiply(instb1.positions[(i * 5) + j], XMMatrixRotationY(dist(gen) * 2 * XM_PI));
			instb1.positions[(i * 5) + j] = XMMatrixTranspose(XMMatrixMultiply(instb1.positions[(i * 5) + j], XMMatrixTranslation(-5.0f + (i * 8.0f), 0.0f, -5.0f + (j * 8.0f))));
		}
	}
	myCon->UpdateSubresource(instBuf, 0, nullptr, &instb1, 0, 0);

#pragma endregion

#pragma region Matricies

	// Initialize the world matrices
	WorldMatrix = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 4.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	ViewMatrix = XMMatrixInverse(nullptr, XMMatrixLookAtLH(Eye, At, Up));

	ProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, swap.BufferDesc.Width / (FLOAT)swap.BufferDesc.Height, 0.01f, 100.0f);

#pragma endregion
	
	return TRUE;
}

BOOL InitDirectInput(HINSTANCE hInstance)
{
	// Initializing the Keyboard and Mouse
	HRESULT hr;
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKey, NULL);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DIKey->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DIKey->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr))
	{
		return FALSE;
	}
	hr = DIMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		return FALSE;
	}
	return TRUE;
}

#pragma region Time

void StartTimer()
{
	LARGE_INTEGER freqCnt;
	QueryPerformanceFrequency(&freqCnt);

	countSeconds = double(freqCnt.QuadPart);
	QueryPerformanceCounter(&freqCnt);
	CounterStart = freqCnt.QuadPart;
}

double GetTime()
{
	LARGE_INTEGER curTime;

	QueryPerformanceCounter(&curTime);
	return double(curTime.QuadPart - CounterStart) / countSeconds;
}

double GetFrameTime()
{
	LARGE_INTEGER curTime;
	__int64 tickCount;
	QueryPerformanceCounter(&curTime);
	tickCount = curTime.QuadPart - oldFrameTime;
	oldFrameTime = curTime.QuadPart;
	if (tickCount < 0.0f)
		tickCount = 0.0f;
	return float(tickCount) / countSeconds;
}
#pragma endregion


void DetectInput(double time)
{
	DIMOUSESTATE mouseStateCurr;

	BYTE keyboardState[256];

	DIKey->Acquire();
	DIMouse->Acquire();
	DIKey->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseStateCurr);

	speed = 5.0f * time;

#pragma region KeyBoard

	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(hWnd, WM_DESTROY, 0, 0);
	if (keyboardState[DIK_W] & 0x80) // move forward
	{
		moveZ += speed;
	}
	if (keyboardState[DIK_S] & 0x80) // move backwards
	{
		moveZ -= speed;
	}
	if (keyboardState[DIK_D] & 0x80) // move right
	{
		moveX += speed;
	}
	if (keyboardState[DIK_A] & 0x80) // move left
	{
		moveX -= speed;
	}
	if (keyboardState[DIK_Q] & 0x80) // move up
	{
		moveY += speed;
	}
	if (keyboardState[DIK_E] & 0x80) // move down
	{
		moveY -= speed;
	}
#pragma endregion

	//Mouse
	if (mouseStateCurr.lX != mouseState.lX) // Look left and right
	{
		rotateY += (mouseStateCurr.lX * 0.005f);
	}
	if (mouseStateCurr.lY != mouseState.lY) // Look up and down
	{
		rotateX += (mouseStateCurr.lY * 0.005f);
	}

	mouseState = mouseStateCurr;
}

void ZeroCameraValues(void)
{
	moveZ = moveX = moveY = 0;
	rotateY = rotateX = 0;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void LoadMesh(const char* meshFileName, My_Mesh& mesh, float size) {

	fstream file(meshFileName, std::ios_base::in | ios_base::binary);
	assert(file.is_open());

	uint32_t player_index_count;
	file.read((char*)&player_index_count, sizeof(uint32_t));
	mesh.indicesList.resize(player_index_count);

	file.read((char*)mesh.indicesList.data(), sizeof(uint32_t) * player_index_count);

	uint32_t player_vertex_count;

	file.read((char*)&player_vertex_count, sizeof(uint32_t));
	mesh.vertexList.resize(player_vertex_count);

	file.read((char*)mesh.vertexList.data(), sizeof(My_Vertex) * player_vertex_count);

	for (int i = 0; i < mesh.vertexList.size(); i++)
	{
		mesh.vertexList[i].Pos.x *= size;
		mesh.vertexList[i].Pos.y *= size;
		mesh.vertexList[i].Pos.z *= size;

		mesh.vertexList[i].Tex.y = 1.0 - mesh.vertexList[i].Tex.y;
	}

	file.close();
}

void CleanupDevice()
{
	//release all the D3D11 interfaces
	if (myCon) myCon->Release();
	if (samplLinear) samplLinear->Release();
	if (textureRV) textureRV->Release();
	if (textureRVAO) textureRVAO->Release();
	if (textureRVNM) textureRVNM->Release();
	if (textureSky) textureSky->Release();
	if (textureGrass) textureGrass->Release();
	if (vBuf) vBuf->Release();
	if (vBufLight) vBufLight->Release();
	if (vBufCube) vBufCube->Release();
	if (vBufGrassPlane) vBufGrassPlane->Release();
	if (iBuf) iBuf->Release();
	if (iBufLight) iBufLight->Release();
	if (iBufCube) iBufCube->Release();
	if (iBufGrassPlane) iBufGrassPlane->Release();
	if (vLayout) vLayout->Release();
	if (vShader) vShader->Release();
	if (pShader) pShader->Release();
	if (pShaderGrass) pShaderGrass->Release();
	if (gShader) gShader->Release();
	if (depthStencil) depthStencil->Release();
	if (myRtv) myRtv->Release();
	if (depthView) depthView->Release();
	if (cBuf) cBuf->Release();
	if (lBuf) lBuf->Release();
	if (instBuf) instBuf->Release();
	if (myDev) myDev->Release();
	if (rState) rState->Release();
	if (mySwap) mySwap->Release();
	if (DirectInput) DirectInput->Release();
	if (DIKey) DIKey->Unacquire();
	if (DIMouse) DIMouse->Unacquire();
}

void Render()
{
	// Update our time
	static float t = 0.0f;
	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	t = (timeCur - timeStart) / 1000.0f;
	// Rotate cube around the origin
	//WorldMatrix = XMMatrixRotationY(t);

	//clear the backbuffer
	myCon->ClearRenderTargetView(myRtv, Colors::Black);

	//clear the depth buffer to max depth (1.0)
	myCon->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#pragma region Mouse and Keyboard
	// Moving the view with key and mouse

	XRotation = XMMatrixRotationX(rotateX);
	YRotation = XMMatrixRotationY(rotateY);

	XMMATRIX translation = XMMatrixTranslation(moveX, moveY, moveZ);
	ViewMatrix = XMMatrixMultiply(translation, ViewMatrix);
	XMVECTOR pos = ViewMatrix.r[3];
	ViewMatrix.r[3] = XMVectorSet(0, 0, 0, 1.0f);
	ViewMatrix = XMMatrixMultiply(XRotation, ViewMatrix);
	ViewMatrix = XMMatrixMultiply(ViewMatrix, YRotation);
	ViewMatrix.r[3] = pos;

	ZeroCameraValues();
#pragma endregion

	XMMATRIX scaled = XMMatrixScaling(scaleBy, scaleBy, scaleBy);

	// Update matrix variables for the Constant buffer
	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(XMMatrixMultiply(WorldMatrix, scaled));
	cb1.mView = XMMatrixTranspose(XMMatrixInverse(nullptr, ViewMatrix));
	cb1.mProjection = XMMatrixTranspose(ProjectionMatrix);
	myCon->UpdateSubresource(cBuf, 0, nullptr, &cb1, 0, 0);

	//setting up the pipeline
	ID3D11RenderTargetView* tempRtv[] = { myRtv };
	myCon->OMSetRenderTargets(1, tempRtv, depthView); // z buffer is the third param remove using nullptr
	// rasterizer
	myCon->RSSetViewports(1, &myPort);
	// Input Assembler
	myCon->IASetInputLayout(vLayout);


	//Rendering the Shape
#pragma region Shaders

	//Vertex shader stage
	myCon->VSSetShader(vShader, nullptr, 0);
	myCon->VSSetConstantBuffers(0, 1, &cBuf);
	myCon->VSSetConstantBuffers(1, 1, &instBuf);

	//Geometry shader stage
	myCon->GSSetShader(nullptr, nullptr, 0);
	myCon->GSSetConstantBuffers(0, 1, &cBuf);

	//Pixel Shader Stage
	myCon->PSSetShader(pShader, nullptr, 0);
	myCon->PSSetConstantBuffers(0, 1, &lBuf);

	//Texture Stage
	myCon->PSSetShaderResources(0, 1, &textureRV);
	myCon->PSSetShaderResources(1, 1, &textureRVAO);
	myCon->PSSetShaderResources(2, 1, &textureRVNM);
	myCon->PSSetSamplers(0, 1, &samplLinear);

#pragma endregion

#pragma region Draw Models
	UINT stride = sizeof(My_Vertex);
	UINT offset = 0;

	//Draw Stump
	// Set vertex buffer
	myCon->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
	// Set index buffer
	myCon->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);
	//myCon->DrawIndexed(stump.indicesList.size(), 0, 0);

	//myCon->DrawIndexedInstanced(stump.indicesList.size(), 2, 0, 0, 0);
	myCon->DrawInstanced(stump.vertexList.size(), 25, 0, 0);


	//Draw Plane
	cb1.mWorld = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(50.0f, 50.0f, 50.0f));
	cb1.mWorld = XMMatrixTranspose(XMMatrixMultiply(cb1.mWorld, XMMatrixTranslation(0.0f, -2.0f, 0.0f)));
	myCon->UpdateSubresource(cBuf, 0, nullptr, &cb1, 0, 0);
	//myCon->PSSetShader(pShaderGrass, nullptr, 0);
	myCon->PSSetConstantBuffers(0, 1, &lBuf);
	myCon->PSSetShaderResources(0, 1, &textureGrass);
	myCon->PSSetSamplers(0, 1, &samplLinear);

	myCon->IASetVertexBuffers(0, 1, &vBufGrassPlane, &stride, &offset);
	myCon->IASetIndexBuffer(iBufGrassPlane, DXGI_FORMAT_R32_UINT, 0);
	myCon->DrawIndexed(GrassPlane.indicesList.size(), 0, 0);

#pragma endregion


#pragma region Lights

	const int numLights = 4;
	// temp light and colors
	XMFLOAT4 vLightDirs[numLights] =
	{
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		XMFLOAT4(0.0f, -0.5f, 0.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[numLights] =
	{
		XMFLOAT4(Colors::Green),
		XMFLOAT4(Colors::Yellow),
		XMFLOAT4(Colors::Red),
		XMFLOAT4(Colors::Blue),
	};

	XMFLOAT4 vLightPos[numLights] =
	{
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),
	};

	// Rotate the second light around the origin
	XMMATRIX mRotate = XMMatrixRotationY(1.0f * t);
	XMVECTOR vLightPos1 = XMLoadFloat4(&vLightPos[1]);
	vLightPos1 = XMVector3TransformCoord(vLightPos1, mRotate);  // XMVector3TransformCoord(vLightDir, mRotate); // for pos
	XMStoreFloat4(&vLightPos[1], vLightPos1);

	vLightDirs[0].x *= sin(t * 1.0f);
	vLightColors[0].y *= cos(t * 1.0f);

	// Update matrix vars for the Light Buffer
	XMMATRIX mLight = XMMatrixTranslationFromVector(15.0f * XMLoadFloat4(&vLightPos[1]));
	XMMATRIX mLightScale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	mLight = mLightScale * mLight;
	   	
	cb1.mWorld = XMMatrixTranspose(mLight);
	myCon->UpdateSubresource(cBuf, 0, nullptr, &cb1, 0, 0);

	LightBuffer lb1 = {};
	lb1.lightPos.x = mLight.r[3].m128_f32[0];
	lb1.lightPos.y = mLight.r[3].m128_f32[1];
	lb1.lightPos.z = mLight.r[3].m128_f32[2];
	lb1.lightPos.w = mLight.r[3].m128_f32[3];

	lb1.lightDir[0] = vLightDirs[0];		
	lb1.lightDir[1] = vLightDirs[1];
	lb1.lightColor[0] = vLightColors[0];
	lb1.lightColor[1] = vLightColors[1];
	lb1.lightColor[2] = vLightColors[2];
	lb1.lightColor[3] = vLightColors[3];
	lb1.lightRadius = 100.0f;
	lb1.coneSize = 2.0f;
	lb1.coneDir = vLightDirs[1];
	lb1.innerConeRatio = 0.95f;
	lb1.outerConeRatio = 0.8f;

	myCon->UpdateSubresource(lBuf, 0, nullptr, &lb1, 0, 0);
	myCon->PSSetShader(pShader, nullptr, 0);
	myCon->PSSetConstantBuffers(0, 1, &lBuf);

	//Draw Model
	// Set vertex buffer
	myCon->IASetVertexBuffers(0, 1, &vBufLight, &stride, &offset);
	// Set index buffer
	myCon->IASetIndexBuffer(iBufLight, DXGI_FORMAT_R32_UINT, 0);
	myCon->DrawIndexed(lightBulb.indicesList.size(), 0, 0);

#pragma endregion

	// presents the back buffer to the front buffer and should always be last
	mySwap->Present(0, 0);

}
