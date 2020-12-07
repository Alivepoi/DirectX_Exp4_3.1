//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: cube.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Renders a spinning cube in wireframe mode.  Demonstrates vertex and 
//       index buffers, world and view transformations, render states and
//       drawing commands.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

/*
项目1： DirectX材质和光照实验
在例程Cube的基础上，完成以下步骤：
1）实现平行光照明。
2）黄色材质设定。
3）增加一个蓝色点光源。
在实验过程中注意灯光设置的方向、颜色等参数设定。
 */
#include "d3dUtility.h"
#include <vector>
 //
 // Globals
 //

IDirect3DDevice9* Device = 0;

const int Width = 640;
const int Height = 480;

IDirect3DVertexBuffer9* VB = 0;
IDirect3DIndexBuffer9* IB = 0;
D3DMATERIAL9 Mtrl_;

ID3DXMesh* Mesh = 0;
//创建一个ID3DMesh对象 并将XFile中的几何数据加载到里面去
std::vector<D3DMATERIAL9>       Mtrls(0);   //存储材质
std::vector<IDirect3DTexture9*> Textures(0); //存储纹理

//
// Classes and Structures
//

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z)
	{
		_x = x;  _y = y;  _z = z;
	}
	float _x, _y, _z;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL;

//
// Framework Functions
//
bool Setup()
{
	//加载飞机对象
	///////////////////////////////////////////////////////////////////////////////////////////////
	HRESULT hr = 0;

	//
	// Load the XFile data.
	//

	ID3DXBuffer* adjBuffer = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD        numMtrls = 0;

	hr = D3DXLoadMeshFromX(
		"bigship1.x",  //文件名
		D3DXMESH_MANAGED,  //创建网格的时候所使用的创建标记。D3DXMESH_MANAFED网格数据将被存储在托管内存池中。
		Device,     //设备指针
		&adjBuffer,   //返回一个ID3DXBuffer 对象，该对象包含了一个描述了了该网格对象的邻接信息的DWORD 类型的数组
		&mtrlBuffer,    //返回一个ID3DXBuffer 对象，该对象包含了一个存储了该网格材质数据的D3DXMATERIAL 类型的数组
		0,                   //返回一个ID3DXBuffer 对象，这里指定参数为0.将其忽略
		&numMtrls,       //返回网格中材质的数目。mtrlBuffer中输出的D3DXMATERIAL数组中的元素个数
		&Mesh);        //返回ID3DXMesh对象

	if (FAILED(hr))
	{
		::MessageBox(0, "D3DXLoadMeshFromX() - FAILED", 0, 0);
		return false;
	}

	//
	// Extract the materials, and load textures.
	//遍历D3DMATERIAL 数组中的元素，并加载网格所引用的纹理数据

	if (mtrlBuffer != 0 && numMtrls != 0)
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

		for (int i = 0; i < numMtrls; i++)
		{
			// the MatD3D property doesn't have an ambient value set
			// when its loaded, so set it now:
			mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;

			// save the ith material
			Mtrls.push_back(mtrls[i].MatD3D);

			// check if the ith material has an associative texture
			if (mtrls[i].pTextureFilename != 0)
			{
				// yes, load the texture for the ith subset
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					mtrls[i].pTextureFilename,
					&tex);

				// save the loaded texture
				Textures.push_back(tex);
			}
			else
			{
				// no texture for the ith subset
				Textures.push_back(0);
			}
		}
	}
	d3d::Release<ID3DXBuffer*>(mtrlBuffer); // done w/ buffer

	//
	// Optimize the mesh.
	//

	hr = Mesh->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT |
		D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(),
		0, 0, 0);

	d3d::Release<ID3DXBuffer*>(adjBuffer); // done w/ buffer

	if (FAILED(hr))
	{
		::MessageBox(0, "OptimizeInplace() - FAILED", 0, 0);
		return false;
	}

	//
// Set texture filters.
//

	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	///////////////////////////////////////////////////////////////////////////////////////////////

	//
	// Create vertex and index buffers.
	//

	Device->CreateVertexBuffer(
		16 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Device->CreateIndexBuffer(
		72 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		0);

	//
	// Fill the buffers with the cube data.
	//

	// define unique vertices:
	Vertex* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	// vertices of a unit cube
	vertices[0] = Vertex(-1.0f, -1.0f, -1.0f);
	vertices[1] = Vertex(-1.0f, 1.0f, -1.0f);
	vertices[2] = Vertex(1.0f, 1.0f, -1.0f);
	vertices[3] = Vertex(1.0f, -1.0f, -1.0f);
	vertices[4] = Vertex(-1.0f, -1.0f, 1.0f);
	vertices[5] = Vertex(-1.0f, 1.0f, 1.0f);
	vertices[6] = Vertex(1.0f, 1.0f, 1.0f);
	vertices[7] = Vertex(1.0f, -1.0f, 1.0f);


	VB->Unlock();

	// define the triangles of the cube:
	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	// front side
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	// back side
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	// left side
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	// right side
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	// top
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	// bottom
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;

	//indices[36] = 6; indices[37] = 7; indices[38] = 8;

	IB->Unlock();

	//设置方向光源
	D3DXVECTOR3 dir(1.0f, -0.0f, 0.25f);
	D3DXCOLOR   c = d3d::WHITE;
	D3DLIGHT9 dirLight = d3d::InitDirectionalLight(&dir, &c);

	//设置点光源
	D3DXVECTOR3 point_(1.0f, 1.0f, 1.0f);
	D3DXCOLOR  p = d3d::RED;
	D3DLIGHT9 pointLight = d3d::InitPointLight(&point_, &p);

	////设置并启用灯光 
	Device->SetLight(0, &dirLight);
	Device->SetLight(1, &pointLight);
	Device->LightEnable(0, true);
	Device->LightEnable(1, true);

	////设置渲染状态
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, false);


	//
	// Position and aim the camera.
	//
	Mtrl_ = d3d::YELLOW_MTRL;

	D3DXVECTOR3 position(0.0f, 0.0f, 7.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);

	Device->SetTransform(D3DTS_VIEW, &V);

	//
	// Set the projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.5f, // 90 - degree
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//
	// Switch to wireframe mode.
	//

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	return true;
}

void Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(VB);
	d3d::Release<IDirect3DIndexBuffer9*>(IB);
}

bool Display(float timeDelta)
{
	if (Device)
	{


		//
		// spin the cube:
		//
		D3DXMATRIX Rx, Ry, scale_;

		// rotate 45 degrees on x-axis
		D3DXMatrixRotationX(&Rx, 3.14f / 4.0f);

		// incremement y-rotation angle each frame
		static float y = 0.0f;
		D3DXMatrixRotationY(&Ry, y);
		y += timeDelta;

		// reset angle to zero when angle reaches 2*PI
		if (y >= 6.28f)
			y = 0.0f;

		// combine x- and y-axis rotation transformations.

		D3DXMatrixScaling(&scale_, 0.3, 0.3, 0.3);
		D3DXMATRIX p = Rx * Ry * scale_;

		Device->SetTransform(D3DTS_WORLD, &p);

		//
		// draw the scene:
		//
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();


		//bigship.x
		for (int i = 0; i < Mtrls.size(); i++)
		{
			Device->SetMaterial(&Mtrls[i]);
			Device->SetTexture(0, Textures[i]);
			Mesh->DrawSubset(i);
		}


		D3DXMATRIX V_;//
		D3DXMatrixTranslation(&V_, 5, 0, 0);
		D3DXMATRIX _V = Rx * Ry * V_;
		Device->SetTransform(D3DTS_WORLD, &_V);


		Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
		Device->SetIndices(IB);
		Device->SetFVF(Vertex::FVF);
		Device->SetMaterial(&Mtrl_);
		// Draw cube.
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}


//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}
