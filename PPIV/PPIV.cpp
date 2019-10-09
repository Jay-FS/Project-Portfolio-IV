// PPIV.cpp : Defines the entry point for the application.
//
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
#include <fstream>

#include "framework.h"
#include "PPIV.h"

// Including my own vertex and pixel shader headers
#include "VertexShader.csh"
#include "PixelShader.csh"

// Texture Loading
#include "DDSTextureLoader.h"

#define MAX_LOADSTRING 100

using namespace DirectX;
using namespace std;

// Structs

struct My_Vertex 
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct My_Mesh
{
	vector<My_Vertex> vertexList;
	vector<int> indicesList;
	FLOAT scale = 50.f;
};

// VRAM Constant Buffer
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vOutputColor;
};


// funtime random color
#define RAND_COLOR XMFLOAT4(rand()/float(RAND_MAX),rand()/float(RAND_MAX),rand()/float(RAND_MAX),1.0f)


// Global DirectX Objects
const char* meshName = "stump.mesh";

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
ID3D11Buffer* iBuf;				// Index Buffer
ID3D11Buffer* cBuf;				// Constant Buffer
ID3D11VertexShader* vShader;	//HLSL
ID3D11PixelShader* pShader;		//HLSL
ID3D11PixelShader* pShader1;		//HLSL

//Texture variables

ID3D11ShaderResourceView* textureRV;
ID3D11ShaderResourceView* textureRVAO; // ambient oclusion
ID3D11ShaderResourceView* textureRVNM; // Normal Map
ID3D11SamplerState* samplLinear;

//Rasterizer
ID3D11RasterizerState* rState;

//My Meshes
My_Mesh stairs;

// Matricies

XMMATRIX WorldMatrix;
XMMATRIX ViewMatrix;
XMMATRIX ProjectionMatrix;

// Scaling Things
float scaleBy = 1.0f;
float sizeScale = 1.0f;

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


void LoadMesh(const char*, My_Mesh&);
void CleanupDevice();
void Render();
BOOL InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
void ZeroCameraValues(void);



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
    if (!InitInstance (hInstance, nCmdShow))
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

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PPIV));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PPIV);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hr = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, 
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

   //Setting the vertex buffer
   hr = myDev->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, &pShader);
   if (FAILED(hr))
	   return FALSE;
#pragma endregion

#pragma region Mesh Data

   //Importing mesh from binary file
   LoadMesh(meshName, stairs);

   // load it onto  the card
   D3D11_BUFFER_DESC bDesc;
   ZeroMemory(&bDesc, sizeof(bDesc));

   //setting buffer desc
   bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   bDesc.ByteWidth = sizeof(My_Vertex) * stairs.vertexList.size();
   bDesc.CPUAccessFlags = 0;
   bDesc.MiscFlags = 0;
   bDesc.StructureByteStride = 0;
   bDesc.Usage = D3D11_USAGE_DEFAULT;

   //setting the subData for the vertex buffer
   D3D11_SUBRESOURCE_DATA subData;
   ZeroMemory(&subData, sizeof(subData));

   subData.pSysMem = stairs.vertexList.data();
   hr = myDev->CreateBuffer(&bDesc, &subData, &vBuf);
   if (FAILED(hr))
	   return FALSE;

   // Set vertex buffer
   UINT stride = sizeof(My_Vertex);
   UINT offset = 0;
   myCon->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);


   // Create index Buffer
   bDesc.Usage = D3D11_USAGE_DEFAULT;
   bDesc.ByteWidth = sizeof(uint32_t) * stairs.indicesList.size();        // 36 vertices needed for 12 triangles in a triangle list
   bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   bDesc.CPUAccessFlags = 0;
   subData.pSysMem = stairs.indicesList.data();
   hr = myDev->CreateBuffer(&bDesc, &subData, &iBuf);
   if (FAILED(hr))
	   return FALSE;

   // Set index buffer
   myCon->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);

   // Set primitive topology
   myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

#pragma region Texture

   // Load the Texture
   hr = CreateDDSTextureFromFile(myDev, L"stumpNM.dds", nullptr, &textureRV); // Name of texture
   if (FAILED(hr))
	   return FALSE;
   hr = CreateDDSTextureFromFile(myDev, L"stumpAO.dds", nullptr, &textureRVAO); // Name of texture
   if (FAILED(hr))
	   return FALSE;
   hr = CreateDDSTextureFromFile(myDev, L"stump.dds", nullptr, &textureRVNM); // Name of texture
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

   // Create the Constant buffer
   bDesc.Usage = D3D11_USAGE_DEFAULT;
   bDesc.ByteWidth = sizeof(ConstantBuffer);
   bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   bDesc.CPUAccessFlags = 0;
   hr = myDev->CreateBuffer(&bDesc, nullptr, &cBuf);
   if (FAILED(hr))
	   return FALSE;

	   // Initialize the world matrices
   WorldMatrix = XMMatrixIdentity();

   // Initialize the view matrix
   XMVECTOR Eye = XMVectorSet(0.0f, 4.0f, -10.0f, 0.0f);
   XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
   XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
   ViewMatrix = XMMatrixInverse(nullptr, XMMatrixLookAtLH(Eye, At, Up));

   ProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, swap.BufferDesc.Width / (FLOAT)swap.BufferDesc.Height, 0.01f, 100.0f);

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

void LoadMesh(const char* meshFileName, My_Mesh& mesh) {

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
		mesh.vertexList[i].Pos.x *= sizeScale;
		mesh.vertexList[i].Pos.y *= sizeScale;
		mesh.vertexList[i].Pos.z *= sizeScale;
	}

	file.close();
}

void CleanupDevice()
{
	//release all the D3D11 interfaces
	if(myCon) myCon->Release();
	if (samplLinear) samplLinear->Release();
	if (textureRV) textureRV->Release();
	if (textureRVAO) textureRVAO->Release();
	if (textureRVNM) textureRVNM->Release();
	if(vBuf) vBuf->Release();
	if(iBuf) iBuf->Release();
	if(vLayout) vLayout->Release();
	if(vShader) vShader->Release();
	if(pShader) pShader->Release();
	if(depthStencil) depthStencil->Release();
	if(myRtv) myRtv->Release();
	if(depthView) depthView->Release();
	if(cBuf) cBuf->Release();
	if(myDev) myDev->Release();
	if(rState) rState->Release();
	if(mySwap) mySwap->Release();
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

	// temp light and colors
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(Colors::White),
		XMFLOAT4(Colors::Blue)
	};

	// Rotate the second light around the origin
	XMMATRIX mRotate = XMMatrixRotationY(-1.0f * t);
	XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&vLightDirs[1], vLightDir);


	//clear the backbuffer
	myCon->ClearRenderTargetView(myRtv, Colors::Black);

	//clear the depth buffer to max depth (1.0)
	myCon->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

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

	// Update matrix variables for the Constant buffer
	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(WorldMatrix);
	cb1.mView = XMMatrixTranspose(XMMatrixInverse(nullptr, ViewMatrix));
	cb1.mProjection = XMMatrixTranspose(ProjectionMatrix);
	cb1.vLightDir[0] = vLightDirs[0];
	cb1.vLightDir[1] = vLightDirs[1];
	cb1.vLightColor[0] = vLightColors[0];
	cb1.vLightColor[1] = vLightColors[1];
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	myCon->UpdateSubresource(cBuf, 0, nullptr, &cb1, 0, 0);

	//setting up the pipeline
	ID3D11RenderTargetView* tempRtv[] = { myRtv };
	myCon->OMSetRenderTargets(1, tempRtv, depthView); // z buffer is the third param remove using nullptr
	// rasterizer
	myCon->RSSetViewports(1, &myPort);
	// Input Assembler
	myCon->IASetInputLayout(vLayout);

	//Rendering the Shape

	//Vertex shader stage
	myCon->VSSetShader(vShader, nullptr, 0);
	myCon->VSSetConstantBuffers(0, 1, &cBuf);

	//Pixel Shader Stage
	myCon->PSSetShader(pShader, nullptr, 0);
	myCon->VSSetConstantBuffers(0, 1, &cBuf);

	//Texture Stage
	myCon->PSSetShaderResources(0, 1, &textureRV);
	myCon->PSSetShaderResources(0, 1, &textureRVAO);
	myCon->PSSetShaderResources(0, 1, &textureRVNM);
	myCon->PSSetSamplers(0, 1, &samplLinear);

	//Draw Model
	myCon->DrawIndexed(stairs.indicesList.size(), 0, 0);
	//myCon->DrawInstanced(mesh.vertexList.size(), 1, 0, 0);
	
	// Render each light
	for (int m = 0; m < 2; m++)
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(2.0f * XMLoadFloat4(&vLightDirs[m]));
		XMMATRIX mLightScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
		mLight = mLightScale * mLight;

		// Update the world variable to reflect the current light
		cb1.mWorld = XMMatrixTranspose(mLight);
		cb1.vOutputColor = vLightColors[m];
		myCon->UpdateSubresource(cBuf, 0, nullptr, &cb1, 0, 0);
		myCon->PSSetShader(pShader, nullptr, 0);
		myCon->PSSetConstantBuffers(0, 1, &cBuf);
		//myCon->DrawIndexed(stairs.indicesList.size(), 0, 0);
		//myCon->DrawInstanced(stairs.vertexList.size(), 1, 0, 0);
	}

	// presents the back buffer to the front buffer and should always be last
	mySwap->Present(0,0);

}
