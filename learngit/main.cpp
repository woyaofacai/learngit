#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include <stdio.h>
#include "Utils.h"

#pragma comment(lib,"d3d9.lib")  
#pragma comment(lib,"d3dx9.lib")  
#pragma comment(lib,"dxguid.lib")  
#pragma comment(lib, "winmm.lib")

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
bool Game_Init(HWND hwnd);
void Game_Update(float);
void Game_Render();
void Game_Exit();
D3DCOLOR getRandomColor();

LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPD3DXMESH g_pMeshes[4];
D3DXMATRIX g_WorldMats[4];
D3DLIGHT9 g_Lights[3];
D3DMATERIAL9 g_Mtrls[4];
D3DXMATRIX g_matRot;
bool g_bLightEnabled[3];
LPD3DXFONT g_pFont;


int WINAPI WinMain (HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int iCmdShow)

{
	HWND hwnd; 
	MSG msg; 
	WNDCLASS wndclass; 
	static TCHAR szClassName[ ] = TEXT("WindowsApp");
	/* 第一步：注册窗口类 */
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = szClassName;
	wndclass.lpfnWndProc = WindowProcedure; 
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; 
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;

	if (!RegisterClass (&wndclass))
		return 0;

	RECT rc = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	/* 第二步：创建窗口 */
	hwnd = CreateWindow(
		szClassName, 
		TEXT("MyApp"), 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT, 
		rc.right - rc.left,   
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	/* 第三步：显示窗口 */
	ShowWindow (hwnd, iCmdShow);
	UpdateWindow(hwnd);

	Game_Init(hwnd);

	float lastTime = timeGetTime() * 0.001f;
	float currentTime, delta;
	/* 第四步：消息循环 */
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			currentTime = timeGetTime() * 0.001f;
			delta = currentTime - lastTime;
			lastTime = currentTime;
			Game_Update(delta);
			Game_Render();
		}
	}

	Game_Exit();

	return msg.wParam;
}

/*窗口过程*/
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int num;
	switch (message)  
	{
	case WM_DESTROY:
		PostQuitMessage (0); 
		break;
	case WM_KEYUP:
		{
			if(wParam == '1' || wParam == '2' || wParam == '3')
			{
				num = wParam - '1';
				g_bLightEnabled[num] = !g_bLightEnabled[num];
				g_pd3dDevice->LightEnable(num, g_bLightEnabled[num]);
			}
		} 

	default:
		return DefWindowProc (hwnd, message, wParam, lParam);
	}
	return 0;
}

bool Game_Init(HWND hwnd)
{
	srand(timeGetTime());

	LPDIRECT3D9 d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	D3DCAPS9 d3dcap;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcap);

	DWORD vp;
	if( d3dcap.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth            = SCREEN_WIDTH;
	d3dpp.BackBufferHeight           = SCREEN_HEIGHT;
	d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount            = 1;
	d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality         = 0;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD; 
	d3dpp.hDeviceWindow              = hwnd;
	d3dpp.Windowed                   = true;
	d3dpp.EnableAutoDepthStencil     = true; 
	d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
	d3dpp.Flags                      = 0;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr;
	hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
		hwnd, vp, &d3dpp, &g_pd3dDevice);

	d3d9->Release();

	if(FAILED(hr))
		return false;

	D3DXCreateFont(g_pd3dDevice, 20, 0, 0, 1, FALSE, DEFAULT_CHARSET,   
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("宋体"), &g_pFont);


	D3DXCreateTeapot(g_pd3dDevice, &g_pMeshes[0], 0);
	D3DXCreateSphere(g_pd3dDevice, 1.0f, 20, 20, &g_pMeshes[1], 0);
	D3DXCreateTorus(g_pd3dDevice, 0.5f, 1.0f, 20, 20, &g_pMeshes[2], 0);
	D3DXCreateCylinder(g_pd3dDevice, 0.5f, 0.5f, 2.0f, 20, 20, &g_pMeshes[3], 0);

	D3DXMatrixTranslation(&g_WorldMats[0],  0.0f,  2.0f, 0.0f);
	D3DXMatrixTranslation(&g_WorldMats[1],  0.0f, -2.0f, 0.0f);
	D3DXMatrixTranslation(&g_WorldMats[2], -3.0f,  0.0f, 0.0f);
	D3DXMatrixTranslation(&g_WorldMats[3],  3.0f,  0.0f, 0.0f);

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

	//定义光源
	::ZeroMemory(&g_Lights[0],sizeof(g_Lights[0]));  
	g_Lights[0].Type = D3DLIGHT_POINT;	//点光源  
	g_Lights[0].Ambient     = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);  
	g_Lights[0].Diffuse      = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[0].Specular     =D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[0].Position      = D3DXVECTOR3(-10.0f, 10.0f, 0);
	g_Lights[0].Attenuation0  = 1.0f;
	g_Lights[0].Attenuation1  = 0.0f; 
	g_Lights[0].Attenuation2  = 0.0f;
	g_Lights[0].Range         = 1000.0f;   
	g_pd3dDevice->SetLight(0, &g_Lights[0]); //设置光源  
	g_pd3dDevice->LightEnable(0, false); //启用光照  

	::ZeroMemory(&g_Lights[1],sizeof(g_Lights[1]));  
	g_Lights[1].Type = D3DLIGHT_DIRECTIONAL;	//平行光 
	g_Lights[1].Ambient     = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);  
	g_Lights[1].Diffuse      = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[1].Specular     = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[1].Direction      = D3DXVECTOR3(0.71f, -0.71f, 0.0f);
	g_pd3dDevice->SetLight(1, &g_Lights[1]); //设置光源  
	g_pd3dDevice->LightEnable(1, false); //启用光照  

	::ZeroMemory(&g_Lights[2],sizeof(g_Lights[2]));  
	g_Lights[2].Type          = D3DLIGHT_SPOT;   //聚光灯
	g_Lights[2].Ambient     = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);  
	g_Lights[2].Diffuse      = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[2].Specular     = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	g_Lights[2].Direction = D3DXVECTOR3(0.71f, -0.71f, 0.0f);
	g_Lights[2].Position  = D3DXVECTOR3(-10.0f, 10.0f, 0.0f);
	g_Lights[2].Attenuation0  = 1.0f;   
	g_Lights[2].Attenuation1  = 0.0f;   
	g_Lights[2].Attenuation2  = 0.0f;   
	g_Lights[2].Range         = 1000.0f;  
	g_Lights[2].Falloff       = 0.1f;  
	g_Lights[2].Phi           = D3DX_PI / 3.0f;  
	g_Lights[2].Theta         = D3DX_PI / 6.0f;  
	g_pd3dDevice->SetLight(2, &g_Lights[1]); //设置光源  
	g_pd3dDevice->LightEnable(2, false); //启用光照  


	//定义和设置材质
	g_Mtrls[0].Ambient  = D3DXCOLOR(0.5f, 0.1f, 0.1f, 1.0f);  
	g_Mtrls[0].Diffuse  = D3DXCOLOR(1.0f, 0.2f, 0.2f, 1.0f);  
	g_Mtrls[0].Specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);  
	g_Mtrls[0].Emissive = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);  
	g_Mtrls[0].Power = 30;

	g_Mtrls[1].Ambient  = D3DXCOLOR(0.1f, 0.5f, 0.1f, 1.0f);  
	g_Mtrls[1].Diffuse  = D3DXCOLOR(0.2f, 1.0f, 0.2f, 1.0f);  
	g_Mtrls[1].Specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);  
	g_Mtrls[1].Emissive = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);  
	g_Mtrls[1].Power = 30;

	g_Mtrls[2].Ambient  = D3DXCOLOR(0.1f, 0.1f, 0.5f, 1.0f);  
	g_Mtrls[2].Diffuse  = D3DXCOLOR(0.2f, 0.2f, 1.0f, 1.0f);  
	g_Mtrls[2].Specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);  
	g_Mtrls[2].Emissive = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);  
	g_Mtrls[2].Power = 30;

	g_Mtrls[3].Ambient  = D3DXCOLOR(0.5f, 0.5f, 0.1f, 1.0f);  
	g_Mtrls[3].Diffuse  = D3DXCOLOR(1.0f, 1.0f, 0.2f, 1.0f);  
	g_Mtrls[3].Specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);  
	g_Mtrls[3].Emissive = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);  
	g_Mtrls[3].Power = 30;

	g_bLightEnabled[0] = g_bLightEnabled[1] = g_bLightEnabled[2] = false;

	return true;
}

D3DCOLOR getRandomColor()
{
	return D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256);
}

void Game_Update(float delta)
{
	static float rad = 0.0f;
	rad += delta * 3.0f;
	if(rad > 6.28f) rad -= 6.28f;
	D3DXMatrixRotationY(&g_matRot, rad);

	D3DXMATRIX matView;   
	D3DXVECTOR3 vEye(0.0f, 0.0f, -10.0f);
	D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &vEye, &vAt, &vUp);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matProj; 
	D3DXMatrixPerspectiveFovLH(
		&matProj, 
		D3DX_PI / 4.0f,                 
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,    
		1.0f, 
		1000.0f
		); 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

}

void Game_Render()
{
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0); 
	g_pd3dDevice->BeginScene();

	for (int i = 0; i < 4; i++)
	{
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &(g_WorldMats[i] * g_matRot));
		g_pd3dDevice->SetMaterial(&g_Mtrls[i]);
		g_pMeshes[i]->DrawSubset(0);
	}

	TCHAR szTipText[100];
	_stprintf(szTipText, TEXT("通过数字键1,2,3控制三种灯光的开关\n1 点光源: %s\n2 平行光: %s\n3 聚光灯: %s\n"),
		g_bLightEnabled[0] ? TEXT("On") : TEXT("Off"),
		g_bLightEnabled[1] ? TEXT("On") : TEXT("Off"),
		g_bLightEnabled[2] ? TEXT("On") : TEXT("Off"));


	RECT rect = {10, 10, SCREEN_WIDTH, SCREEN_HEIGHT};

	g_pFont->DrawText(0, szTipText, -1, 
		&rect, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(255, 255, 0));

	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void Game_Exit()
{
	int i;
	for (i = 0; i < 4; i++)
	{
		g_pMeshes[i]->Release();
		g_pMeshes[i] = NULL;
	}

	if(g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}
}
