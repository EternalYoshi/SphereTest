#include "TheRenderer.h"
#include <vector>
#include <functional>
#include <cmath>
#include "code\umvc3menu.h"
#include <atomic>
#include <mutex>
#include "glm\ext\vector_float3.hpp"
#include "glm\ext\matrix_transform.hpp"
#include "glm\ext\matrix_clip_space.hpp"
#include "glm\ext\matrix_projection.hpp"
#include "Globals.h"

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
//IDirect3DBaseTexture9* oldTexture;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
#define D3DX_PI 3.1415926535897932384626

struct TEXTUREVERTEX {
	float x, y, z, rhw;
	DWORD color;
	float u, v;
};

void ReleaseHitboxRenderTarget()
{
	if (g_pRenderTexture1) { g_pRenderTexture1->Release(); g_pRenderTexture1 = NULL; }
	if (g_pRenderSurface1) { g_pRenderSurface1->Release(); g_pRenderSurface1 = NULL; }
	if (g_pDepthStencilSurface1) { g_pDepthStencilSurface1->Release(); g_pDepthStencilSurface1 = NULL; }
	if (g_pRenderTexture2) { g_pRenderTexture2->Release(); g_pRenderTexture2 = NULL; }
	if (g_pRenderSurface2) { g_pRenderSurface2->Release(); g_pRenderSurface2 = NULL; }
	if (g_pDepthStencilSurface2) { g_pDepthStencilSurface2->Release(); g_pDepthStencilSurface2 = NULL; }
	g_renderTargetCreated1 = false;
	g_renderTargetCreated2 = false;
}

// Gets the resolution scale and plugs that into the globals.
void GetGameResolution(LPDIRECT3DDEVICE9 pDevice)
{	
	D3DVIEWPORT9 viewport;
	pDevice->GetViewport(&viewport);
	GameHeight = viewport.Height;
	GameWidth = viewport.Width;
}

void DrawHurtboxTexture(LPDIRECT3DDEVICE9 pDevice, float alpha)
{
	//GetGameResolution(pDevice);

	//Backups up the render state.
	DWORD alphaBlend, srcBlend, destBlend, fvf, zwriteenable;
	DWORD alphaOp, alphaArg1, alphaArg2, zenable;
	//IDirect3DBaseTexture9* oldTexture;
	//IDirect3DBaseTexture9* oldTexture2;

	//pDevice->GetRenderState(D3DRS_ZENABLE, &zenable);

	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
	pDevice->GetRenderState(D3DRS_SRCBLEND, &srcBlend);
	pDevice->GetRenderState(D3DRS_DESTBLEND, &destBlend);
	//pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &zwriteenable);
	pDevice->GetFVF(&fvf);

	pDevice->GetTexture(0, &oldTexture);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &alphaOp);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &alphaArg1);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &alphaArg2);

	//Attempt at Setting up textured quad with alpha enabled.
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pDevice->SetTexture(0, g_pRenderTexture1);
	pDevice->SetFVF(D3DFVF_TEXTUREVERTEX);

	BYTE alphaValue = (BYTE)(alpha * 255.0f);
	TEXTUREVERTEX vertices[] = {
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 0.0f},
		{GameWidth, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 0.0f},
		{GameWidth, GameHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 1.0f},
		{0.0f, GameHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 1.0f},
	};

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TEXTUREVERTEX));

	//Restores state.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
	pDevice->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	pDevice->SetRenderState(D3DRS_DESTBLEND, destBlend);
	//pDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwriteenable);

	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, alphaOp);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, alphaArg1);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, alphaArg2);

	pDevice->SetTexture(0, oldTexture);
	pDevice->SetFVF(fvf);

	if (oldTexture) oldTexture->Release();
	//if (oldTexture2) oldTexture2->Release();
}

void DrawHitboxTexture(LPDIRECT3DDEVICE9 pDevice, float alpha)
{
	//GetGameResolution(pDevice);

	//Backups up the render state.
	DWORD alphaBlend, srcBlend, destBlend, fvf, zwriteenable;
	DWORD alphaOp, alphaArg1, alphaArg2;
	//IDirect3DBaseTexture9* oldTexture;
	//IDirect3DBaseTexture9* oldTexture2;

	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
	pDevice->GetRenderState(D3DRS_SRCBLEND, &srcBlend);
	pDevice->GetRenderState(D3DRS_DESTBLEND, &destBlend);
	pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &zwriteenable);
	pDevice->GetFVF(&fvf);
	pDevice->GetTexture(0, &oldTexture2);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &alphaOp);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &alphaArg1);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &alphaArg2);

	//Setup for a transparent textured quad.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pDevice->SetTexture(0, g_pRenderTexture2);
	pDevice->SetFVF(D3DFVF_TEXTUREVERTEX);

	BYTE alphaValue = (BYTE)(alpha * 255.0f);
	TEXTUREVERTEX vertices[] = {
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 0.0f},
		{GameWidth, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 0.0f},
		{GameWidth, GameHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 1.0f},
		{0.0f, GameHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 1.0f},
	};

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TEXTUREVERTEX));

	//Restores the state.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
	pDevice->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	pDevice->SetRenderState(D3DRS_DESTBLEND, destBlend);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwriteenable);

	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, alphaOp);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, alphaArg1);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, alphaArg2);

	pDevice->SetTexture(0, oldTexture2);
	pDevice->SetFVF(fvf);

	//if (oldTexture) oldTexture->Release();
	if (oldTexture2) oldTexture2->Release();
}

bool CreateHitboxTarget(IDirect3DDevice9* device, int width, int height) {
	//This creates a texture with render target usage. Has a format with alpha channel as well.
	HRESULT hr = device->CreateTexture(
		width, height, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&g_pRenderTexture1,
		NULL
	);

	if (FAILED(hr)) return false;	
	g_pRenderTexture1->GetSurfaceLevel(0, &g_pRenderSurface1);

	//For the render target.
	hr = device->CreateDepthStencilSurface(
		width, height,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0,
		TRUE,
		&g_pDepthStencilSurface1,
		NULL
	);

	if (FAILED(hr)) return false;

	g_renderTargetCreated1 = true;
	return true;
}

bool CreateCapsuleTarget(IDirect3DDevice9* device, int width, int height) {
	HRESULT hr = device->CreateTexture(
		width, height, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&g_pRenderTexture2,
		NULL
	);

	if (FAILED(hr)) return false;

	g_pRenderTexture2->GetSurfaceLevel(0, &g_pRenderSurface2);

	hr = device->CreateDepthStencilSurface(
		width, height,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0,
		TRUE,
		&g_pDepthStencilSurface2,
		NULL
	);

	if (FAILED(hr)) return false;

	g_renderTargetCreated2 = true;
	return true;
}

struct SphereDataBuffer {
	//Player Character Hurtboxes.
	std::vector<PrimHitSphere> P1C1ActiveSpheres;
	std::vector<PrimHitSphere> P1C2ActiveSpheres;
	std::vector<PrimHitSphere> P1C3ActiveSpheres;
	std::vector<PrimHitSphere> P2C1ActiveSpheres;
	std::vector<PrimHitSphere> P2C2ActiveSpheres;
	std::vector<PrimHitSphere> P2C3ActiveSpheres;

	//Player Character Hitboxes.
	std::vector<PrimHitCapsule> P1C1ActiveHitSpheres;
	std::vector<PrimHitCapsule> P1C2ActiveHitSpheres;
	std::vector<PrimHitCapsule> P1C3ActiveHitSpheres;
	std::vector<PrimHitCapsule> P2C1ActiveHitSpheres;
	std::vector<PrimHitCapsule> P2C2ActiveHitSpheres;
	std::vector<PrimHitCapsule> P2C3ActiveHitSpheres;

	//Player Child Hit/Hurtspheres.
	std::vector<PrimHitSphere> P1C1ChildActiveSpheres;
	std::vector<PrimHitCapsule> P1C1Child1ActiveHitSpheres;

	std::vector<PrimHitSphere> P1C2ChildActiveSpheres;
	std::vector<PrimHitCapsule> P1C2Child1ActiveHitSpheres;

	std::vector<PrimHitSphere> P1C3ChildActiveSpheres;
	std::vector<PrimHitCapsule> P1C3Child1ActiveHitSpheres;

	std::vector<PrimHitSphere> P2C1ChildActiveSpheres;
	std::vector<PrimHitCapsule> P2C1Child1ActiveHitSpheres;

	std::vector<PrimHitSphere> P2C2ChildActiveSpheres;
	std::vector<PrimHitCapsule> P2C2Child1ActiveHitSpheres;

	std::vector<PrimHitSphere> P2C3ChildActiveSpheres;
	std::vector<PrimHitCapsule> P2C3Child1ActiveHitSpheres;

	//Shots.
	std::vector<PrimHitCapsule> P1ShotsActiveHitCapsules;
	std::vector<PrimHitCapsule> P2ShotsActiveHitCapsules;
	std::vector<PrimHitSphere> P1ShotsActiveHitSpheres;
	std::vector<PrimHitSphere> P2ShotsActiveHitSpheres;

	bool Valid = false;
};

static SphereDataBuffer FBuffers[2];
static std::atomic<int> WriteBuffer{ 0 };
static std::atomic<int> ReadBuffer{ 1 };
static std::mutex BufferMutex;

//Borrowed from HKHaan & Altimor's code.

//Restores device state to parameters saved in render_start.
std::function<void(IDirect3DDevice9*)> StopRendering;

void StartRendering(IDirect3DDevice9* device)
{
	//First the resolution scale.
	D3DVIEWPORT9 viewport;
	device->GetViewport(&viewport);

	//Default resolution I'm working with is 1600x900...

	//Back up device state.
	DWORD alpha_blend_enable, dest_blend, src_blend, dest_blend_alpha, src_blend_alpha, fvf, alpha_test_enable, alpharef, alphafunc;
	IDirect3DPixelShader9* pixel_shader;
	IDirect3DBaseTexture9* texture;
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &alpha_blend_enable);
	device->GetRenderState(D3DRS_ALPHATESTENABLE, &alpha_test_enable);
	device->GetRenderState(D3DRS_ALPHAREF, &alpharef);
	device->GetRenderState(D3DRS_ALPHAFUNC, &alphafunc);
	//device->GetRenderState(D3DRS_DESTBLEND, &dest_blend);
	//device->GetRenderState(D3DRS_SRCBLEND, &src_blend);
	//device->GetRenderState(D3DRS_DESTBLENDALPHA, &dest_blend_alpha);
	//device->GetRenderState(D3DRS_SRCBLENDALPHA, &src_blend_alpha);
	device->GetPixelShader(&pixel_shader);
	device->GetFVF(&fvf);
	device->GetTexture(0, &texture);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHAREF, 0x08);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	//device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	//device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
	device->SetPixelShader(nullptr);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetTexture(0, nullptr);

	StopRendering = [=](IDirect3DDevice9* device)
	{
		//Restore device state.
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, alpha_blend_enable);
		device->SetRenderState(D3DRS_ALPHATESTENABLE, alpha_test_enable);
		device->SetRenderState(D3DRS_ALPHAREF, alpharef);
		device->SetRenderState(D3DRS_ALPHAFUNC, alphafunc);
		//device->SetRenderState(D3DRS_DESTBLEND, dest_blend);
		//device->SetRenderState(D3DRS_SRCBLEND, src_blend);
		//device->SetRenderState(D3DRS_DESTBLENDALPHA, dest_blend_alpha);
		//device->SetRenderState(D3DRS_SRCBLENDALPHA, src_blend_alpha);
		device->SetPixelShader(pixel_shader);
		device->SetFVF(fvf);
		device->SetTexture(0, texture);

		if (texture != nullptr)
			texture->Release();
	};


}

void circleFilled(LPDIRECT3DDEVICE9 pDevice, float x, float y, float rad, int resolution, DWORD color)
{
	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	//Data for a Full circle.
	float pi = D3DX_PI;

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = color;

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x + rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y + rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].color = color;
	}

	//Rotation matrix.
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(0) * (circle[i].x - x) - sin(0) * (circle[i].y - y);
		circle[i].y = y + sin(0) * (circle[i].x - x) + cos(0) * (circle[i].y - y);
	}

	pDevice->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB->Unlock();

	pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	if (g_pVB != NULL) g_pVB->Release();
}

void ProcessHurtSpheres(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<PrimHitSphere>& ActiveSpheres)
{
	//Check if empty.
	if (ActiveSpheres.empty()) return;

	float pi = D3DX_PI;

	//Calculates the required vertex count.
	int verticesPerCircle = resolution + 2;
	int totalVertices = verticesPerCircle * ActiveSpheres.size();

	//Pre-allocating combined vertex array.
	std::vector<CUSTOMVERTEX> allVertices;
	allVertices.reserve(totalVertices);

	for (int v = 0; v < ActiveSpheres.size(); v++)
	{

		float x = ActiveSpheres[v].Position.X;
		float y = GameHeight - ActiveSpheres[v].Position.Y;
		float rad = ActiveSpheres[v].Radius;
		DWORD color = ActiveSpheres[v].Color;

		//The center vert.
		CUSTOMVERTEX center;
		center.x = x;
		center.y = y;
		center.z = 0;
		center.rhw = 1;
		center.color = color;
		allVertices.push_back(center);

		for (int i = 1; i < resolution + 2; i++)
		{
			CUSTOMVERTEX vert;
			vert.x = (float)(x + rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
			vert.y = (float)(y + rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
			vert.z = 0;
			vert.rhw = 1;
			vert.color = color;
			allVertices.push_back(vert);
		}

	}

	//Create a single vertex buffer for all the circles.
	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	pDevice->CreateVertexBuffer(totalVertices * sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY,
		D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
		D3DPOOL_DEFAULT,
		&pVB,
		NULL);

	VOID* pVertices;
	pVB->Lock(0, totalVertices * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &allVertices[0], totalVertices * sizeof(CUSTOMVERTEX));
	pVB->Unlock();

	pDevice->SetStreamSource(0, pVB, 0, sizeof(CUSTOMVERTEX));
	pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//Draws all circles in a call that uses multiple triangle fans.
	for (int i = 0; i < ActiveSpheres.size(); i++)
	{
		int startVertex = i * verticesPerCircle;
		pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, startVertex, resolution);
	}

	if (pVB != NULL) pVB->Release();


}

void ProcessHitCapsules(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<PrimHitCapsule>& ActiveSpheres)
{
	//Check if empty.
	if (ActiveSpheres.empty()) return;

	float pi = D3DX_PI;

	int verticesPerSphere = resolution + 2;
	//Need Rectangles to fill in the space between the two spheres to make the capsule shape.
	int verticesPerRect = 4;
	int verticesPerCapsule = (verticesPerSphere * 2) + verticesPerRect;
	int totalVertices = verticesPerCapsule * ActiveSpheres.size();

	//Pre-allocates the combined vertex array.
	std::vector<CUSTOMVERTEX> allVertices;
	allVertices.reserve(totalVertices);

	for (int v = 0; v < ActiveSpheres.size(); v++)
	{

		float x1 = ActiveSpheres[v].Position.X;
		float y1 = GameHeight - ActiveSpheres[v].Position.Y;
		float x2 = ActiveSpheres[v].Position2.X;
		float y2 = GameHeight - ActiveSpheres[v].Position2.Y;
		float rad = ActiveSpheres[v].Radius;
		DWORD color = ActiveSpheres[v].Color;

		//Calculates the directior vector for the rectanglular connection.
		float dx = x2 - x1;
		float dy = y2 - y1;
		float length = sqrt(dx * dx + dy * dy);

		//Normalized perpendicular vector.
		float perpX = -dy / length * rad;
		float perpY = dx / length * rad;

		//For the 1st Sphere.
		CUSTOMVERTEX center1;
		center1.x = x1;
		center1.y = y1;
		center1.z = 0;
		center1.rhw = 1;
		center1.color = color;
		allVertices.push_back(center1);

		for (int i = 1; i < resolution + 2; i++)
		{
			CUSTOMVERTEX vert;
			vert.x = (float)(x1 + rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
			vert.y = (float)(y1 + rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
			vert.z = 0;
			vert.rhw = 1;
			vert.color = color;
			allVertices.push_back(vert);
		}

		//For the 2nd Sphere.
		CUSTOMVERTEX center2;
		center2.x = x2;
		center2.y = y2;
		center2.z = 0;
		center2.rhw = 1;
		center2.color = color;
		allVertices.push_back(center2);

		for (int i = 1; i < resolution + 2; i++)
		{
			CUSTOMVERTEX vert;
			vert.x = (float)(x2 + rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
			vert.y = (float)(y2 + rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
			vert.z = 0;
			vert.rhw = 1;
			vert.color = color;
			allVertices.push_back(vert);
		}

		//Connecting Rectangle.
		CUSTOMVERTEX rect[4];

		rect[0].x = x1 + perpX;
		rect[0].y = y1 + perpY;
		rect[0].z = 0;
		rect[0].rhw = 1;
		rect[0].color = color;

		rect[1].x = x1 - perpX;
		rect[1].y = y1 - perpY;
		rect[1].z = 0;
		rect[1].rhw = 1;
		rect[1].color = color;

		rect[2].x = x2 + perpX;
		rect[2].y = y2 + perpY;
		rect[2].z = 0;
		rect[2].rhw = 1;
		rect[2].color = color;

		rect[3].x = x2 - perpX;
		rect[3].y = y2 - perpY;
		rect[3].z = 0;
		rect[3].rhw = 1;
		rect[3].color = color;

		for (int i = 0; i < 4; i++)
			allVertices.push_back(rect[i]);

	}

	//Creates single vertex buffer for all the above shapes.
	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	pDevice->CreateVertexBuffer(totalVertices * sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY,
		D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
		D3DPOOL_DEFAULT,
		&pVB,
		NULL);

	VOID* pVertices;
	pVB->Lock(0, totalVertices * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &allVertices[0], totalVertices * sizeof(CUSTOMVERTEX));
	pVB->Unlock();

	pDevice->SetStreamSource(0, pVB, 0, sizeof(CUSTOMVERTEX));
	pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//Draws all circles in a single call via multiple triangle fans.
	for (int i = 0; i < ActiveSpheres.size(); i++)
	{
		int startVertex = i * verticesPerCapsule;

		//First Sphere.
		pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, startVertex, resolution);

		//Second Sphere.
		pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, startVertex + verticesPerSphere, resolution);

		//Rectangles that connect the two.
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, startVertex + (verticesPerSphere * 2), 2);

	}

	if (pVB != NULL) pVB->Release();


}

PrimHitSphere GetHurtBoxScreenPos(Hurtbox HBox)
{
	//First the base coordinates of the hurtbox.

	float AverageX, AverageY, AverageZ;
	AverageX = (HBox.SecondaryX + HBox.DeFactoX) / 2;
	AverageY = (HBox.SecondaryY + HBox.DeFactoY) / 2;
	AverageZ = (HBox.SecondaryZ + HBox.DeFactoZ) / 2;

	glm::vec3 position(HBox.CollData.Coordinates.X, HBox.CollData.Coordinates.Y, HBox.CollData.Coordinates.Z);

	//Next the view matrix.
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),
		glm::vec3(0, 1, 0)
	);

	//Then the Projection and view.
	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, GameWidth / GameHeight, NearPlane, FarPlane);
	glm::vec4 View = glm::vec4(0, 0, GameWidth, GameHeight);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.CollData.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);
	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);

	float screenRadius = glm::distance(result, offsetResult) * 1.0f;

	PrimHitSphere NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(0, 128, 0, 255);
	NewBoxR.Radius = screenRadius;
	NewBoxR.Position.X = result.x;
	NewBoxR.Position.Y = result.y;
	NewBoxR.Position.Z = 0;

	//glm::vec4 viewPos = viewMatrix * glm::vec4(position, 1.0f);
	//printf("World Y: %.2f, View Y: %.2f, Screen Y: %.2f, Camera Y: %.2f\n",
	//	position.y, viewPos.y, result.y, CameraPos.y);

	return NewBoxR;
	//return HurtboxR(result.x, result.y, 0.0f, D3DCOLOR_RGBA(0, 250, 0, 70, 60, 0), (HBox.CollData.Radius * 2), true);

}

//For disambiguating between normal state and semi invul but throwable state.
PrimHitSphere GetBlueHurtBoxScreenPos(Hurtbox HBox)
{
	//First the base coordinates of the hurtbox.

	float AverageX, AverageY, AverageZ;
	AverageX = (HBox.SecondaryX + HBox.DeFactoX) / 2;
	AverageY = (HBox.SecondaryY + HBox.DeFactoY) / 2;
	AverageZ = (HBox.SecondaryZ + HBox.DeFactoZ) / 2;

	glm::vec3 position(HBox.CollData.Coordinates.X, HBox.CollData.Coordinates.Y, HBox.CollData.Coordinates.Z);

	//Next the view matrix.
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),
		glm::vec3(0, 1, 0)
	);

	//Then the Projection and view.
	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, GameWidth / GameHeight, NearPlane, FarPlane);
	glm::vec4 View = glm::vec4(0, 0, GameWidth, GameHeight);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.CollData.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);
	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);

	float screenRadius = glm::distance(result, offsetResult) * 1.0f;

	PrimHitSphere NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(0, 0, 150, 255);
	NewBoxR.Radius = screenRadius;
	NewBoxR.Position.X = result.x;
	NewBoxR.Position.Y = result.y;
	NewBoxR.Position.Z = 0;

	//glm::vec4 viewPos = viewMatrix * glm::vec4(position, 1.0f);
	//printf("World Y: %.2f, View Y: %.2f, Screen Y: %.2f, Camera Y: %.2f\n",
	//	position.y, viewPos.y, result.y, CameraPos.y);

	return NewBoxR;
	//return HurtboxR(result.x, result.y, 0.0f, D3DCOLOR_RGBA(0, 250, 0, 70, 60, 0), (HBox.CollData.Radius * 2), true);

}

//Because Shots can use Spheres as collisions.
PrimHitSphere GetRedSphereScreenPos(Hurtbox HBox)
{
	//First the base coordinates of the hurtbox.

	//float AverageX, AverageY, AverageZ;
	//AverageX = (HBox.SecondaryX + HBox.DeFactoX) / 2;
	//AverageY = (HBox.SecondaryY + HBox.DeFactoY) / 2;
	//AverageZ = (HBox.SecondaryZ + HBox.DeFactoZ) / 2;

	glm::vec3 position(HBox.CollData.Coordinates.X, HBox.CollData.Coordinates.Y, HBox.CollData.Coordinates.Z);

	//Next the view matrix.
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),
		glm::vec3(0, 1, 0)
	);

	//Then the Projection and view.
	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, GameWidth / GameHeight, NearPlane, FarPlane);
	glm::vec4 View = glm::vec4(0, 0, GameWidth, GameHeight);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.CollData.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);
	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);

	float screenRadius = glm::distance(result, offsetResult) * 1.0f;

	PrimHitSphere NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(200, 0, 0, 255);
	NewBoxR.Radius = screenRadius;
	NewBoxR.Position.X = result.x;
	NewBoxR.Position.Y = result.y;
	NewBoxR.Position.Z = 0;

	//glm::vec4 viewPos = viewMatrix * glm::vec4(position, 1.0f);
	//printf("World Y: %.2f, View Y: %.2f, Screen Y: %.2f, Camera Y: %.2f\n",
	//	position.y, viewPos.y, result.y, CameraPos.y);

	return NewBoxR;

}

PrimHitCapsule GetHitboxScreenPos(Hitbox HBox)
{

	//First the coordinates of the first part of the capsule.

	float AverageX, AverageY, AverageZ;
	AverageX = (HBox.CapsulePrimaryPos.x + HBox.ContainerPos.x) / 2;
	AverageY = (HBox.CapsulePrimaryPos.y + HBox.ContainerPos.y) / 2;
	AverageZ = (HBox.CapsulePrimaryPos.z + HBox.ContainerPos.z) / 2;

	glm::vec3 position(HBox.CapsulePrimaryPos.x, HBox.CapsulePrimaryPos.y, HBox.CapsulePrimaryPos.z);

	//Now for the second capsule point.

	AverageX = (HBox.CapsuleSecondPos.x + HBox.ContainerPos.x) / 2;
	AverageY = (HBox.CapsuleSecondPos.y + HBox.ContainerPos.y) / 2;
	AverageZ = (HBox.CapsuleSecondPos.z + HBox.ContainerPos.z) / 2;

	glm::vec3 positiontwo(HBox.CapsuleSecondPos.x, HBox.CapsuleSecondPos.y, HBox.CapsuleSecondPos.z);

	//Next the view matrix.
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),
		glm::vec3(0, 1, 0)
	);

	//Then the Projection and view.

	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, GameWidth / GameHeight, NearPlane, FarPlane);

	glm::vec4 View = glm::vec4(0, 0, GameWidth, GameHeight);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);

	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);
	glm::vec2 resulttwo = glm::project(positiontwo, viewMatrix, Projection, View);
	float screenRadius = glm::distance(result, offsetResult) * 1.0f;

	PrimHitCapsule NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(200, 0, 0, 255);
	NewBoxR.Radius = screenRadius;
	NewBoxR.Position.X = result.x;
	NewBoxR.Position.Y = result.y;
	NewBoxR.Position.Z = 0;
	NewBoxR.Position2.X = resulttwo.x;
	NewBoxR.Position2.Y = resulttwo.y;
	NewBoxR.Position2.Z = 0;

	return NewBoxR;
	//return HurtboxR(result.x, result.y, 0.0f, D3DCOLOR_RGBA(0, 250, 0, 70, 60, 0), (HBox.CollData.Radius * 2), true);

}

//This is meant to update the Hit/Hurtsphere data and is only meant to be called once per game frame somehow.
void UpdateSphereData(std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes, std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes, std::vector<Hurtbox> P2C3Hurtboxes, std::vector<Hurtbox> P1ShotHitSpheres, std::vector<Hitbox> P1ShotHitCapsule, std::vector<Hurtbox> P2ShotHitSpheres, std::vector<Hitbox> P2ShotHitCapsule)
{
	int write = WriteBuffer.load();

	std::lock_guard<std::mutex> lock(BufferMutex);

	SphereDataBuffer& data = FBuffers[write];

#pragma region Player 1 Character 1 Hurtboxes
	data.P1C1ActiveSpheres.clear();
	data.P1C1ActiveSpheres.resize(P1C1Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P1C1Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P1C1ChrStateBinary[25] != '1' && P1C1ChrStateBinary[29] != '1'
			&& P1C1CurAnmchrID != 0x85 && P1C1CurAnmchrID != 0x155 && P1C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C1Hurtboxes.size(); i++)
			{
				data.P1C1ActiveSpheres[i] = GetHurtBoxScreenPos(P1C1Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P1C1ChrStateBinary[25] == '1' && P1C1ChrStateBinary[29] != '1'
			&& P1C1CurAnmchrID != 0x85 && P1C1CurAnmchrID != 0x155 && P1C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C1Hurtboxes.size(); i++)
			{
				data.P1C1ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P1C1Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 2 Hurtboxes
	data.P1C2ActiveSpheres.clear();
	data.P1C2ActiveSpheres.resize(P1C2Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P1C2Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P1C2ChrStateBinary[25] != '1' && P1C2ChrStateBinary[29] != '1'
			&& P1C2CurAnmchrID != 0x85 && P1C2CurAnmchrID != 0x155 && P1C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C2Hurtboxes.size(); i++)
			{
				data.P1C2ActiveSpheres[i] = GetHurtBoxScreenPos(P1C2Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P1C2ChrStateBinary[25] == '1' && P1C2ChrStateBinary[29] != '1'
			&& P1C2CurAnmchrID != 0x85 && P1C2CurAnmchrID != 0x155 && P1C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C2Hurtboxes.size(); i++)
			{
				data.P1C2ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P1C2Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 3 Hurtboxes
	data.P1C3ActiveSpheres.clear();
	data.P1C3ActiveSpheres.resize(P1C3Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P1C3Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P1C3ChrStateBinary[25] != '1' && P1C3ChrStateBinary[29] != '1'
			&& P1C3CurAnmchrID != 0x85 && P1C3CurAnmchrID != 0x155 && P1C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C3Hurtboxes.size(); i++)
			{
				data.P1C3ActiveSpheres[i] = GetHurtBoxScreenPos(P1C3Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P1C3ChrStateBinary[25] == '1' && P1C3ChrStateBinary[29] != '1'
			&& P1C3CurAnmchrID != 0x85 && P1C3CurAnmchrID != 0x155 && P1C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C3Hurtboxes.size(); i++)
			{
				data.P1C3ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P1C3Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 1 Hurtboxes
	data.P2C1ActiveSpheres.clear();
	data.P2C1ActiveSpheres.resize(P2C1Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P2C1Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P2C1ChrStateBinary[25] != '1' && P2C1ChrStateBinary[29] != '1'
			&& P2C1CurAnmchrID != 0x85 && P2C1CurAnmchrID != 0x155 && P2C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C1Hurtboxes.size(); i++)
			{
				data.P2C1ActiveSpheres[i] = GetHurtBoxScreenPos(P2C1Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P2C1ChrStateBinary[25] == '1' && P2C1ChrStateBinary[29] != '1'
			&& P2C1CurAnmchrID != 0x85 && P2C1CurAnmchrID != 0x155 && P2C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C1Hurtboxes.size(); i++)
			{
				data.P2C1ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P2C1Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 2 Hurtboxes
	data.P2C2ActiveSpheres.clear();
	data.P2C2ActiveSpheres.resize(P2C2Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P2C2Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P2C2ChrStateBinary[25] != '1' && P2C2ChrStateBinary[29] != '1'
			&& P2C2CurAnmchrID != 0x85 && P2C2CurAnmchrID != 0x155 && P2C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C2Hurtboxes.size(); i++)
			{
				data.P2C2ActiveSpheres[i] = GetHurtBoxScreenPos(P2C2Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P2C2ChrStateBinary[25] == '1' && P2C2ChrStateBinary[29] != '1'
			&& P2C2CurAnmchrID != 0x85 && P2C2CurAnmchrID != 0x155 && P2C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C2Hurtboxes.size(); i++)
			{
				data.P2C2ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P2C2Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 3 Hurtboxes
	data.P2C3ActiveSpheres.clear();
	data.P2C3ActiveSpheres.resize(P2C3Hurtboxes.size());
	if (sAction && MatchFlag == 0 && P2C3Hurtboxes.size() > 0)
	{
		//Gotta use binary to use for checking character state. [23] checks for Active collisions, [25] for Invul but throwable,
		//and [29] for true invul.
		if (P2C3ChrStateBinary[25] != '1' && P2C3ChrStateBinary[29] != '1'
			&& P2C3CurAnmchrID != 0x85 && P2C3CurAnmchrID != 0x155 && P2C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C3Hurtboxes.size(); i++)
			{
				data.P2C3ActiveSpheres[i] = GetHurtBoxScreenPos(P2C3Hurtboxes[i]);
			}
		}
		//For that Semi Invul State.
		else if (P2C3ChrStateBinary[20] == '1' && P2C3ChrStateBinary[25] == '1' && P2C3ChrStateBinary[29] != '1'
			&& P2C3CurAnmchrID != 0x85 && P2C3CurAnmchrID != 0x155 && P2C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C3Hurtboxes.size(); i++)
			{
				data.P2C3ActiveSpheres[i] = GetBlueHurtBoxScreenPos(P2C3Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 1 H1tboxes
	data.P1C1ActiveHitSpheres.clear();
	data.P1C1ActiveHitSpheres.resize(P1C1HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if(P1C1HitboxCount && P1C1cAtkCtrl.AtiID != 0xffffffff && (P1C1cAtkCtrl.ATIActiveFrames == 0 || P1C1cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P1C1Hitboxes.size(); i++)
			{
				data.P1C1ActiveHitSpheres[i] = GetHitboxScreenPos(P1C1Hitboxes[i]);
			}
		}
		else if (P1C1HitboxCount && P1C1cAtkCtrl.AtiID != 0xffffffff && (P1C1cAtkCtrl.RemainingActiveFrames != 0 && P1C1cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P1C1Hitboxes.size(); i++)
			{
				data.P1C1ActiveHitSpheres[i] = GetHitboxScreenPos(P1C1Hitboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 2 H1tboxes
	data.P1C2ActiveHitSpheres.clear();
	data.P1C2ActiveHitSpheres.resize(P1C2HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P1C2HitboxCount && P1C2cAtkCtrl.AtiID != 0xffffffff && (P1C2cAtkCtrl.ATIActiveFrames == 0 || P1C2cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P1C2Hitboxes.size(); i++)
			{
				data.P1C2ActiveHitSpheres[i] = GetHitboxScreenPos(P1C2Hitboxes[i]);
			}
		}
		else if (P1C2HitboxCount && P1C2cAtkCtrl.AtiID != 0xffffffff && (P1C2cAtkCtrl.RemainingActiveFrames != 0 && P1C2cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P1C2Hitboxes.size(); i++)
			{
				data.P1C2ActiveHitSpheres[i] = GetHitboxScreenPos(P1C2Hitboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 3 H1tboxes
	data.P1C3ActiveHitSpheres.clear();
	data.P1C3ActiveHitSpheres.resize(P1C3HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P1C3HitboxCount && P1C3cAtkCtrl.AtiID != 0xffffffff && (P1C3cAtkCtrl.ATIActiveFrames == 0 || P1C3cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P1C3Hitboxes.size(); i++)
			{
				data.P1C3ActiveHitSpheres[i] = GetHitboxScreenPos(P1C3Hitboxes[i]);
			}
		}
		else if (P1C3HitboxCount && P1C3cAtkCtrl.AtiID != 0xffffffff && (P1C3cAtkCtrl.RemainingActiveFrames != 0 && P1C3cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P1C3Hitboxes.size(); i++)
			{
				data.P1C3ActiveHitSpheres[i] = GetHitboxScreenPos(P1C3Hitboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 1 H1tboxes
	data.P2C1ActiveHitSpheres.clear();
	data.P2C1ActiveHitSpheres.resize(P2C1HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P2C1HitboxCount && P2C1cAtkCtrl.AtiID != 0xffffffff && (P2C1cAtkCtrl.ATIActiveFrames == 0 || P2C1cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P2C1Hitboxes.size(); i++)
			{
				data.P2C1ActiveHitSpheres[i] = GetHitboxScreenPos(P2C1Hitboxes[i]);
			}
		}
		else if (P2C1HitboxCount && P2C1cAtkCtrl.AtiID != 0xffffffff && (P2C1cAtkCtrl.RemainingActiveFrames != 0 && P2C1cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P2C1Hitboxes.size(); i++)
			{
				data.P2C1ActiveHitSpheres[i] = GetHitboxScreenPos(P2C1Hitboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 2 H1tboxes
	data.P2C2ActiveHitSpheres.clear();
	data.P2C2ActiveHitSpheres.resize(P2C2HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P2C2HitboxCount && P2C2cAtkCtrl.AtiID != 0xffffffff && (P2C2cAtkCtrl.ATIActiveFrames == 0 || P2C2cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P2C2Hitboxes.size(); i++)
			{
				data.P2C2ActiveHitSpheres[i] = GetHitboxScreenPos(P2C2Hitboxes[i]);
			}
		}
		else if (P2C2HitboxCount && P2C2cAtkCtrl.AtiID != 0xffffffff && (P2C2cAtkCtrl.RemainingActiveFrames != 0 && P2C2cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P2C2Hitboxes.size(); i++)
			{
				data.P2C2ActiveHitSpheres[i] = GetHitboxScreenPos(P2C2Hitboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 3 H1tboxes
	data.P2C3ActiveHitSpheres.clear();
	data.P2C3ActiveHitSpheres.resize(P2C3HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P2C3HitboxCount && P2C3cAtkCtrl.AtiID != 0xffffffff && (P2C3cAtkCtrl.ATIActiveFrames == 0 || P2C3cAtkCtrl.ATIActiveFrames == -1))
		{
			for (int i = 0; i < P2C3Hitboxes.size(); i++)
			{
				data.P2C3ActiveHitSpheres[i] = GetHitboxScreenPos(P2C3Hitboxes[i]);
			}
		}
		else if (P2C3HitboxCount && P2C3cAtkCtrl.AtiID != 0xffffffff && (P2C3cAtkCtrl.RemainingActiveFrames != 0 && P2C3cAtkCtrl.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P2C3Hitboxes.size(); i++)
			{
				data.P2C3ActiveHitSpheres[i] = GetHitboxScreenPos(P2C3Hitboxes[i]);
			}
		}
	}
#pragma endregion

	//========Shot Stuff WIP.

	//EmptyShotLists();
	//GetShots();
	data.P1ShotsActiveHitCapsules.clear();
	data.P2ShotsActiveHitCapsules.clear();
	data.P1ShotsActiveHitSpheres.clear();
	data.P2ShotsActiveHitSpheres.clear();

	if (sAction && MatchFlag == 0)
	{
		//Player 1 Shots.
		if(P1ShotHitCapsules.size() > 0)
		{
			for (int n = 0; n < P1ShotHitCapsules.size(); n++)
			{
				data.P1ShotsActiveHitCapsules.push_back(GetHitboxScreenPos(P1ShotHitCapsules[n]));
			}
		}

		if (P1ShotHitSpheres.size() > 0)
		{
			for (int n = 0; n < P1ShotHitSpheres.size(); n++)
			{
				data.P1ShotsActiveHitSpheres.push_back(GetRedSphereScreenPos(P1ShotHitSpheres[n]));
			}
		}

		//Player 2 Shots.
		if (P2ShotHitCapsules.size() > 0)
		{
			for (int nn = 0; nn < P2ShotHitCapsules.size(); nn++)
			{
				data.P2ShotsActiveHitCapsules.push_back(GetHitboxScreenPos(P2ShotHitCapsules[nn]));
			}
		}

		if (P2ShotHitSpheres.size() > 0)
		{
			for (int n = 0; n < P2ShotHitSpheres.size(); n++)
			{
				data.P2ShotsActiveHitSpheres.push_back(GetRedSphereScreenPos(P2ShotHitSpheres[n]));
			}
		}

	}

	//========Child Character stuff.

#pragma region Player 1 Child Hurtboxes & Hitboxes
	if (Player1CharNodeTree && Player2CharNodeTree)
	{		
		//P1C1ActiveChildData = GetChildCharacterData(P1Character1Data, P1Character1ID, Player1CharNodeTree, P1C1ActiveChildData);
		data.P1C1ChildActiveSpheres.clear();
		data.P1C1Child1ActiveHitSpheres.clear();

		if (sAction && MatchFlag == 0)
		{
			for (int n = 0; n < P1C1ActiveChildData.size(); n++)
			{
				//Checks for Maya's Class value to be at 0 and won't render any hurtboxes if at 0. Next part of this statement is for Felciia's sisters & Dr. Strange's Afterimage.
				if ((P1C1ActiveChildData[n].VTableAddress == 0x140A7A830 && P1C1ActiveChildData[n].SomeClassValue == 0) 
					|| P1C1ActiveChildData[n].VTableAddress == 0x140a74eb0 || P1C1ActiveChildData[n].VTableAddress == 0x140a75380
					|| P1C1ActiveChildData[n].VTableAddress == 0x140a744c0 || P1C1ActiveChildData[n].VTableAddress == 0x140a84800
					|| P1C1ActiveChildData[n].VTableAddress == 0x140a7b7f0)
				{
					//data.P1C1ChildActiveSpheres.clear();
					//data.P1C1Child1ActiveHitSpheres.clear();
				}
				else
				{
					//Hope the binary that works for the main characters works for the child chars too.
					if (P1C1ActiveChildData[n].ChildCharStateBinary[25] != '1' && P1C1ActiveChildData[n].ChildCharStateBinary[29] != '1'
						&& P1C1ActiveChildData[n].CurAnmchrID != 0x85 && P1C1ActiveChildData[n].CurAnmchrID != 0x155 && P1C1ActiveChildData[n].CurAnmchrID != 0x88)
					{
						for (int i = 0; i < P1C1ActiveChildData[n].ChildActiveSpheres.size(); i++)
						{
							data.P1C1ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P1C1ActiveChildData[n].ChildActiveSpheres[i]));
						}
					}
				}

				if(P1C1ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P1C1ActiveChildData[n].WeirdFloat != 0 && P1C1ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
				{
					for (int i = 0; i < P1C1ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
					{
						data.P1C1Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P1C1ActiveChildData[n].Child1ActiveHitSpheres[i]));
					}
				}


			}





		}



	}
#pragma endregion
//
//#pragma region Player 1 Character 2 Child Hurtboxes & Hitboxes
//	if (Player1CharNodeTree && Player2CharNodeTree)
//	{
//		//P1C2ActiveChildData = GetChildCharacterData(P1Character2Data, P1Character2ID, Player1CharNodeTree, P1C2ActiveChildData);
//		data.P1C2ChildActiveSpheres.clear();
//		data.P1C2Child1ActiveHitSpheres.clear();
//
//		if (sAction && MatchFlag == 0)
//		{
//			for (int n = 0; n < P1C2ActiveChildData.size(); n++)
//			{
//				//Hope the binary that works for the main characters works for the child chars too.
//				if (P1C2ActiveChildData[n].ChildCharStateBinary[25] != '1' && P1C2ActiveChildData[n].ChildCharStateBinary[29] != '1'
//					&& P1C2ActiveChildData[n].CurAnmchrID != 0x85 && P1C2ActiveChildData[n].CurAnmchrID != 0x155 && P1C2ActiveChildData[n].CurAnmchrID != 0x88)
//				{
//					for (int i = 0; i < P1C2ActiveChildData[n].ChildActiveSpheres.size(); i++)
//					{
//						data.P1C2ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P1C2ActiveChildData[n].ChildActiveSpheres[i]));
//					}
//				}
//
//				if (P1C2ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P1C2ActiveChildData[n].WeirdFloat != 0 && P1C2ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
//				{
//					for (int i = 0; i < P1C2ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
//					{
//						data.P1C2Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P1C2ActiveChildData[n].Child1ActiveHitSpheres[i]));
//					}
//				}
//			}
//		}
//
//	}
//#pragma endregion
//
//#pragma region Player 1 Character 3 Child Hurtboxes & Hitboxes
//	if (Player1CharNodeTree && Player2CharNodeTree)
//	{
//		//P1C3ActiveChildData = GetChildCharacterData(P1Character3Data, P1Character3ID, Player1CharNodeTree, P1C3ActiveChildData);
//		data.P1C3ChildActiveSpheres.clear();
//		data.P1C3Child1ActiveHitSpheres.clear();
//
//		if (sAction && MatchFlag == 0)
//		{
//			for (int n = 0; n < P1C3ActiveChildData.size(); n++)
//			{
//				//Hope the binary that works for the main characters works for the child chars too.
//				if (P1C3ActiveChildData[n].ChildCharStateBinary[25] != '1' && P1C3ActiveChildData[n].ChildCharStateBinary[29] != '1'
//					&& P1C3ActiveChildData[n].CurAnmchrID != 0x85 && P1C3ActiveChildData[n].CurAnmchrID != 0x155 && P1C3ActiveChildData[n].CurAnmchrID != 0x88)
//				{
//					for (int i = 0; i < P1C3ActiveChildData[n].ChildActiveSpheres.size(); i++)
//					{
//						data.P1C3ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P1C3ActiveChildData[n].ChildActiveSpheres[i]));
//					}
//				}
//
//				if (P1C3ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P1C3ActiveChildData[n].WeirdFloat != 0 && P1C3ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
//				{
//					for (int i = 0; i < P1C3ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
//					{
//						data.P1C3Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P1C3ActiveChildData[n].Child1ActiveHitSpheres[i]));
//					}
//				}
//			}
//		}
//
//	}
//#pragma endregion

#pragma region Player 2 Child Hurtboxes & Hitboxes
	if (Player1CharNodeTree && Player2CharNodeTree)
	{
		//P2C1ActiveChildData = GetChildCharacterData(P2Character1Data, P2Character1ID, Player2CharNodeTree, P2C1ActiveChildData);
		data.P2C1ChildActiveSpheres.clear();
		data.P2C1Child1ActiveHitSpheres.clear();

		if (sAction && MatchFlag == 0)
		{
			for (int n = 0; n < P2C1ActiveChildData.size(); n++)
			{
				//Checks for Maya's Class value to be at 0 and won't render any hurtboxes if at 0. Next part of this statement is for Felciia's sisters & Dr. Strange's Afterimage.
				if (P2C1ActiveChildData[n].VTableAddress == 0x140A7A830 && P2C1ActiveChildData[n].SomeClassValue == 0
					|| P2C1ActiveChildData[n].VTableAddress == 0x140a74eb0 || P2C1ActiveChildData[n].VTableAddress == 0x140a75380
					|| P2C1ActiveChildData[n].VTableAddress == 0x140a744c0 || P2C1ActiveChildData[n].VTableAddress == 0x140a84800
					|| P2C1ActiveChildData[n].VTableAddress == 0x140a7b7f0)
				{
					//data.P1C1ChildActiveSpheres.clear();
					//data.P1C1Child1ActiveHitSpheres.clear();
				}
				else
				{
					//Hope the binary that works for the main characters works for the child chars too.
					if (P2C1ActiveChildData[n].ChildCharStateBinary[25] != '1' && P2C1ActiveChildData[n].ChildCharStateBinary[29] != '1'
						&& P2C1ActiveChildData[n].CurAnmchrID != 0x85 && P2C1ActiveChildData[n].CurAnmchrID != 0x155 && P2C1ActiveChildData[n].CurAnmchrID != 0x88)
					{
						for (int i = 0; i < P2C1ActiveChildData[n].ChildActiveSpheres.size(); i++)
						{
							data.P2C1ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P2C1ActiveChildData[n].ChildActiveSpheres[i]));
						}
					}
				}

				if (P2C1ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P2C1ActiveChildData[n].WeirdFloat != 0 && P2C1ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
				{
					for (int i = 0; i < P2C1ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
					{
						data.P2C1Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P2C1ActiveChildData[n].Child1ActiveHitSpheres[i]));
					}
				}


			}





		}



	}
#pragma endregion

//#pragma region Player 2 Character 2 Child Hurtboxes & Hitboxes
//	if (Player1CharNodeTree && Player2CharNodeTree)
//	{
//		//P2C2ActiveChildData = GetChildCharacterData(P2Character2Data, P2Character2ID, Player2CharNodeTree, P2C2ActiveChildData);
//		data.P2C2ChildActiveSpheres.clear();
//		data.P2C2Child1ActiveHitSpheres.clear();
//
//		if (sAction && MatchFlag == 0)
//		{
//			for (int n = 0; n < P2C2ActiveChildData.size(); n++)
//			{
//				//Hope the binary that works for the main characters works for the child chars too.
//				if (P2C2ActiveChildData[n].ChildCharStateBinary[25] != '1' && P2C2ActiveChildData[n].ChildCharStateBinary[29] != '1'
//					&& P2C2ActiveChildData[n].CurAnmchrID != 0x85 && P2C2ActiveChildData[n].CurAnmchrID != 0x155 && P2C2ActiveChildData[n].CurAnmchrID != 0x88)
//				{
//					for (int i = 0; i < P2C2ActiveChildData[n].ChildActiveSpheres.size(); i++)
//					{
//						data.P2C2ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P2C2ActiveChildData[n].ChildActiveSpheres[i]));
//					}
//				}
//
//				if (P2C2ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P2C2ActiveChildData[n].WeirdFloat != 0 && P2C2ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
//				{
//					for (int i = 0; i < P2C2ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
//					{
//						data.P2C2Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P2C2ActiveChildData[n].Child1ActiveHitSpheres[i]));
//					}
//				}
//			}
//		}
//
//	}
//#pragma endregion
//
//#pragma region Player 2 Character 3 Child Hurtboxes & Hitboxes
//	if (Player1CharNodeTree && Player2CharNodeTree)
//	{
//		//P2C3ActiveChildData = GetChildCharacterData(P2Character3Data, P2Character3ID, Player2CharNodeTree, P2C3ActiveChildData);
//		data.P2C3ChildActiveSpheres.clear();
//		data.P2C3Child1ActiveHitSpheres.clear();
//
//		if (sAction && MatchFlag == 0)
//		{
//			for (int n = 0; n < P2C3ActiveChildData.size(); n++)
//			{
//				//Hope the binary that works for the main characters works for the child chars too.
//				if (P2C3ActiveChildData[n].ChildCharStateBinary[25] != '1' && P2C3ActiveChildData[n].ChildCharStateBinary[29] != '1'
//					&& P2C3ActiveChildData[n].CurAnmchrID != 0x85 && P2C3ActiveChildData[n].CurAnmchrID != 0x155 && P2C3ActiveChildData[n].CurAnmchrID != 0x88)
//				{
//					for (int i = 0; i < P2C3ActiveChildData[n].ChildActiveSpheres.size(); i++)
//					{
//						data.P2C3ChildActiveSpheres.push_back(GetHurtBoxScreenPos(P2C3ActiveChildData[n].ChildActiveSpheres[i]));
//					}
//				}
//
//				if (P2C3ActiveChildData[n].ChildActiveATIChunk.FramesBeforeActive < 1 && P2C3ActiveChildData[n].WeirdFloat != 0 && P2C3ActiveChildData[n].ChildActiveATIChunk.AtiID != -1)
//				{
//					for (int i = 0; i < P2C3ActiveChildData[n].Child1ActiveHitSpheres.size(); i++)
//					{
//						data.P2C3Child1ActiveHitSpheres.push_back(GetHitboxScreenPos(P2C3ActiveChildData[n].Child1ActiveHitSpheres[i]));
//					}
//				}
//			}
//		}
//
//	}
//#pragma endregion


	data.Valid = true;

	//Swaps buffers.
	int oldWrite = write;
	int oldRead = ReadBuffer.load();
	WriteBuffer.store(oldRead);
	ReadBuffer.store(oldWrite);
}

void RenderSpheresFromBuffer(LPDIRECT3DDEVICE9 pDevice)
{
	int read = ReadBuffer.load();

	if (!FBuffers[read].Valid)
		return;

	const SphereDataBuffer& data = FBuffers[read];

	GetGameResolution(pDevice);

	//Renders all hurtboxes.
	ProcessHurtSpheres(pDevice, 32, data.P1C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C3ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C3ActiveSpheres);

	//Renders all hitboxes.
	ProcessHitCapsules(pDevice, 32, data.P1C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C3ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C3ActiveHitSpheres);

	//Render child spheres.
	ProcessHurtSpheres(pDevice, 32, data.P1C1ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C1Child1ActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C2ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C2Child1ActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C3ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C3Child1ActiveHitSpheres);

	ProcessHurtSpheres(pDevice, 32, data.P2C1ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C1Child1ActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C2ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C2Child1ActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C3ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C3Child1ActiveHitSpheres);

	//Render Shot HitSpheres.
	ProcessHitCapsules(pDevice, 32, data.P1ShotsActiveHitCapsules);
	ProcessHitCapsules(pDevice, 32, data.P2ShotsActiveHitCapsules);
	ProcessHurtSpheres(pDevice, 32, data.P1ShotsActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2ShotsActiveHitSpheres);

}

void RenderGreenSpheresFromBuffer(LPDIRECT3DDEVICE9 pDevice)
{
	int read = ReadBuffer.load();

	if (!FBuffers[read].Valid)
		return;

	const SphereDataBuffer& data = FBuffers[read];

	//Renders all hurtboxes.
	ProcessHurtSpheres(pDevice, 32, data.P1C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C3ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C3ActiveSpheres);

	//Render child spheres.
	ProcessHurtSpheres(pDevice, 32, data.P1C1ChildActiveSpheres);
	//ProcessHurtSpheres(pDevice, 32, data.P1C2ChildActiveSpheres);
	//ProcessHurtSpheres(pDevice, 32, data.P1C3ChildActiveSpheres);

	ProcessHurtSpheres(pDevice, 32, data.P2C1ChildActiveSpheres);
	//ProcessHurtSpheres(pDevice, 32, data.P2C2ChildActiveSpheres);
	//ProcessHurtSpheres(pDevice, 32, data.P2C3ChildActiveSpheres);

}

void RenderHitSpheresAndCapsulesFromBuffer(LPDIRECT3DDEVICE9 pDevice)
{
	int read = ReadBuffer.load();

	if (!FBuffers[read].Valid)
		return;

	const SphereDataBuffer& data = FBuffers[read];

	//Renders all hitboxes.
	ProcessHitCapsules(pDevice, 32, data.P1C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C3ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C3ActiveHitSpheres);

	//Render child spheres.
	ProcessHitCapsules(pDevice, 32, data.P1C1Child1ActiveHitSpheres);
	//ProcessHitCapsules(pDevice, 32, data.P1C2Child1ActiveHitSpheres);
	//ProcessHitCapsules(pDevice, 32, data.P1C3Child1ActiveHitSpheres);

	ProcessHitCapsules(pDevice, 32, data.P2C1Child1ActiveHitSpheres);
	//ProcessHitCapsules(pDevice, 32, data.P2C2Child1ActiveHitSpheres);
	//ProcessHitCapsules(pDevice, 32, data.P2C3Child1ActiveHitSpheres);

	//Render Shot HitSpheres.
	ProcessHitCapsules(pDevice, 32, data.P1ShotsActiveHitCapsules);
	ProcessHitCapsules(pDevice, 32, data.P2ShotsActiveHitCapsules);
	ProcessHurtSpheres(pDevice, 32, data.P1ShotsActiveHitSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2ShotsActiveHitSpheres);

}