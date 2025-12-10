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
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
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

void DrawHurtboxTexture(LPDIRECT3DDEVICE9 pDevice, float alpha)
{
	// Backups up the state.
	DWORD alphaBlend, srcBlend, destBlend, fvf, zwriteenable;
	DWORD alphaOp, alphaArg1, alphaArg2;
	//IDirect3DBaseTexture9* oldTexture;
	//IDirect3DBaseTexture9* oldTexture2;

	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
	pDevice->GetRenderState(D3DRS_SRCBLEND, &srcBlend);
	pDevice->GetRenderState(D3DRS_DESTBLEND, &destBlend);
	pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &zwriteenable);
	pDevice->GetFVF(&fvf);
	pDevice->GetTexture(0, &oldTexture);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &alphaOp);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &alphaArg1);
	pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &alphaArg2);

	// Setting up textured quad with alpha enabled.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pDevice->SetTexture(0, g_pRenderTexture1);
	pDevice->SetFVF(D3DFVF_TEXTUREVERTEX);

	BYTE alphaValue = (BYTE)(alpha * 255.0f);
	TEXTUREVERTEX vertices[] = {
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 0.0f},
		{1600.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 0.0f},
		{1600.0f, 900.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 1.0f},
		{0.0f, 900.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 1.0f},
	};

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TEXTUREVERTEX));

	//Restores state.
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
	pDevice->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	pDevice->SetRenderState(D3DRS_DESTBLEND, destBlend);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwriteenable);


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
	// Backup state
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

	// Set up for textured quad with alpha
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
		{1600.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 0.0f},
		{1600.0f, 900.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 1.0f, 1.0f},
		{0.0f, 900.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(alphaValue, 255, 255, 255), 0.0f, 1.0f},
	};

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TEXTUREVERTEX));



	// Restore state
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
	// Create texture with render target usage
	HRESULT hr = device->CreateTexture(
		width, height, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, // Format with alpha channel
		D3DPOOL_DEFAULT,
		&g_pRenderTexture1,
		NULL
	);

	if (FAILED(hr)) return false;

	// Get the surface
	g_pRenderTexture1->GetSurfaceLevel(0, &g_pRenderSurface1);

	// Create depth stencil surface for the render target
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
	// Create texture with render target usage
	HRESULT hr = device->CreateTexture(
		width, height, 1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, // Format with alpha channel
		D3DPOOL_DEFAULT,
		&g_pRenderTexture2,
		NULL
	);

	if (FAILED(hr)) return false;

	// Get the surface
	g_pRenderTexture2->GetSurfaceLevel(0, &g_pRenderSurface2);

	// Create depth stencil surface for the render target
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

// Sphere data buffer structure
struct SphereDataBuffer {
	//Player Character Hurtboxes.
	std::vector<HurtboxR> P1C1ActiveSpheres;
	std::vector<HurtboxR> P1C2ActiveSpheres;
	std::vector<HurtboxR> P1C3ActiveSpheres;
	std::vector<HurtboxR> P2C1ActiveSpheres;
	std::vector<HurtboxR> P2C2ActiveSpheres;
	std::vector<HurtboxR> P2C3ActiveSpheres;

	//Player Character Hitboxes.
	std::vector<HitboxR> P1C1ActiveHitSpheres;
	std::vector<HitboxR> P1C2ActiveHitSpheres;
	std::vector<HitboxR> P1C3ActiveHitSpheres;
	std::vector<HitboxR> P2C1ActiveHitSpheres;
	std::vector<HitboxR> P2C2ActiveHitSpheres;
	std::vector<HitboxR> P2C3ActiveHitSpheres;

	//Player Child Hit/Hurtspheres.
	std::vector<HurtboxR> P1C1ChildActiveSpheres;
	std::vector<HitboxR> P1C1Child1ActiveHitSpheres;
	bool Valid = false;
};

static SphereDataBuffer FBuffers[2];
static std::atomic<int> WriteBuffer{ 0 };
static std::atomic<int> ReadBuffer{ 1 };
static std::mutex BufferMutex;

//Borrowed from HKHaan & Altimor's code.

// Restores device state to parameters saved in render_start.
std::function<void(IDirect3DDevice9*)> StopRendering;

void StartRendering(IDirect3DDevice9* device)
{
	//First the resolution scale.
	D3DVIEWPORT9 viewport;
	device->GetViewport(&viewport);

	//Default resolution I'm working with is 1600x900...

	// Back up device state.
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
		// Restore device state.
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
	float pi = D3DX_PI;        // Full circle

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

	// Rotate matrix
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

void ProcessHurtSpheres(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<HurtboxR>& ActiveSpheres)
{
	//Check if empty.
	if (ActiveSpheres.empty()) return;

	float pi = D3DX_PI;        // Full circle

	// Calculate total vertices needed
	int verticesPerCircle = resolution + 2;
	int totalVertices = verticesPerCircle * ActiveSpheres.size();

	// Pre-allocate combined vertex array
	std::vector<CUSTOMVERTEX> allVertices;
	allVertices.reserve(totalVertices);

	for (int v = 0; v < ActiveSpheres.size(); v++)
	{

		float x = ActiveSpheres[v].Position.X;
		float y = 900.0f - ActiveSpheres[v].Position.Y;
		float rad = ActiveSpheres[v].Radius;
		DWORD color = ActiveSpheres[v].Color;

		// Center vertex
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

	// Create single vertex buffer for all circles
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

	// Draw all circles in one call using multiple triangle fans
	for (int i = 0; i < ActiveSpheres.size(); i++)
	{
		int startVertex = i * verticesPerCircle;
		pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, startVertex, resolution);
	}

	if (pVB != NULL) pVB->Release();


}

void ProcessHitCapsules(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<HitboxR>& ActiveSpheres)
{
	//Check if empty.
	if (ActiveSpheres.empty()) return;

	float pi = D3DX_PI;        // Full circle

	// Calculate total vertices needed
	int verticesPerSphere = resolution + 2;
	//Need Rectangles to fill in the space between the two spheres to make the capsule shape.
	int verticesPerRect = 4;
	int verticesPerCapsule = (verticesPerSphere * 2) + verticesPerRect;
	int totalVertices = verticesPerCapsule * ActiveSpheres.size();

	// Pre-allocate combined vertex array
	std::vector<CUSTOMVERTEX> allVertices;
	allVertices.reserve(totalVertices);

	for (int v = 0; v < ActiveSpheres.size(); v++)
	{

		float x1 = ActiveSpheres[v].Position.X;
		float y1 = 900.0f - ActiveSpheres[v].Position.Y;
		float x2 = ActiveSpheres[v].Position2.X;
		float y2 = 900.0f - ActiveSpheres[v].Position2.Y;
		float rad = ActiveSpheres[v].Radius;
		DWORD color = ActiveSpheres[v].Color;

		//Calculates the directior vector for the rectanglular connection.
		float dx = x2 - x1;
		float dy = y2 - y1;
		float length = sqrt(dx * dx + dy * dy);

		// Perpendicular vector (normalized)
		float perpX = -dy / length * rad;
		float perpY = dx / length * rad;

		// For the 1st Sphere.
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

	// Create single vertex buffer for all circles
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

	// Draw all circles in one call using multiple triangle fans
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

HurtboxR GetHurtBoxScreenPos(Hurtbox HBox)
{
	//First the base coordinates of the hurtbox.

	float AverageX, AverageY, AverageZ;
	AverageX = (HBox.SecondaryX + HBox.DeFactoX) / 2;
	AverageY = (HBox.SecondaryY + HBox.DeFactoY) / 2;
	AverageZ = (HBox.SecondaryZ + HBox.DeFactoZ) / 2;

	glm::vec3 position(HBox.CollData.Coordinates.X, HBox.CollData.Coordinates.Y, HBox.CollData.Coordinates.Z);

	//Next the view matrix.
	glm::mat4 viewMatrix = glm::lookAt(
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),    // CameraPos
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),	 // TargetPos
		glm::vec3(0, 1, 0)									 // CameraUp
	);

	//Then the Projection and view.
	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, 1600.0f / 900.0f, NearPlane, FarPlane);
	glm::vec4 View = glm::vec4(0, 0, 1600, 900);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.CollData.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);
	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);
	
	float screenRadius = glm::distance(result, offsetResult) * 1.0f;
	
	HurtboxR NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(0, 128, 0, 200, 60, 0);
	NewBoxR.Radius = screenRadius;
	NewBoxR.Position.X = result.x;
	NewBoxR.Position.Y = result.y;
	NewBoxR.Position.Z = 0;

	return NewBoxR;
	//return HurtboxR(result.x, result.y, 0.0f, D3DCOLOR_RGBA(0, 250, 0, 70, 60, 0), (HBox.CollData.Radius * 2), true);

}

HitboxR GetHitboxScreenPos(Hitbox HBox)
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
		glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z),    // CameraPos
		glm::vec3(TargetPos.x, TargetPos.y, TargetPos.z),	 // TargetPos
		glm::vec3(0, 1, 0)									 // CameraUp
	);

	//Then the Projection and view.

	float FovR = glm::radians(mFOV);
	glm::highp_mat4 Projection = glm::perspective<float>(FovR, 1600.0f / 900.0f, NearPlane, FarPlane);

	glm::vec4 View = glm::vec4(0, 0, 1600, 900);

	//Now for the radius to adapt to zoom levels.
	glm::vec3 offsetPosition = position + glm::vec3(HBox.Radius, 0, 0);
	glm::vec2 offsetResult = glm::project(offsetPosition, viewMatrix, Projection, View);

	glm::vec2 result = glm::project(position, viewMatrix, Projection, View);
	glm::vec2 resulttwo = glm::project(positiontwo, viewMatrix, Projection, View);
	float screenRadius = glm::distance(result, offsetResult) * 1.0f;

	HitboxR NewBoxR;
	NewBoxR.Enabled = true;
	NewBoxR.Color = D3DCOLOR_RGBA(200, 0, 0, 200, 60, 0);
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

// This is meant to update the Hit/Hurtsphere data and is only meant to be called once per game frame somehow.
void UpdateSphereData(std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes, std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes, std::vector<Hurtbox> P2C3Hurtboxes)
{
	int write = WriteBuffer.load();

	std::lock_guard<std::mutex> lock(BufferMutex);

	SphereDataBuffer& data = FBuffers[write];

#pragma region Player 1 Character 1 Hurtboxes
	data.P1C1ActiveSpheres.clear();
	data.P1C1ActiveSpheres.resize(P1C1Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P1C1VulnState != 4 && P1C1VulnState != 0x1C && P1C1CharState != 0x280C && P1C1CharState != 0x100 && P1C1CharState != 0x20000
			&& P1C1CurAnmchrID != 0x85 && P1C1CurAnmchrID != 0x155 && P1C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C1Hurtboxes.size(); i++)
			{
				data.P1C1ActiveSpheres[i] = GetHurtBoxScreenPos(P1C1Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 2 Hurtboxes
	data.P1C2ActiveSpheres.clear();
	data.P1C2ActiveSpheres.resize(P1C2Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P1C2VulnState != 4 && P1C2VulnState != 0x1C && P1C2CharState != 0x280C && P1C2CharState != 0x100 && P1C2CharState != 0x20000
			&& P1C2CurAnmchrID != 0x85 && P1C2CurAnmchrID != 0x155 && P1C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C2Hurtboxes.size(); i++)
			{
				data.P1C2ActiveSpheres[i] = GetHurtBoxScreenPos(P1C2Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 3 Hurtboxes
	data.P1C3ActiveSpheres.clear();
	data.P1C3ActiveSpheres.resize(P1C3Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P1C3VulnState != 4 && P1C3VulnState != 0x1C && P1C3CharState != 0x280C && P1C3CharState != 0x100 && P1C3CharState != 0x20000
			&& P1C3CurAnmchrID != 0x85 && P1C3CurAnmchrID != 0x155 && P1C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P1C3Hurtboxes.size(); i++)
			{
				data.P1C3ActiveSpheres[i] = GetHurtBoxScreenPos(P1C3Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 1 Hurtboxes
	data.P2C1ActiveSpheres.clear();
	data.P2C1ActiveSpheres.resize(P2C1Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P2C1VulnState != 4 && P2C1VulnState != 0x1C && P2C1CharState != 0x280C && P2C1CharState != 0x100 && P2C1CharState != 0x20000
			&& P2C1CurAnmchrID != 0x85 && P2C1CurAnmchrID != 0x155 && P2C1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C1Hurtboxes.size(); i++)
			{
				data.P2C1ActiveSpheres[i] = GetHurtBoxScreenPos(P2C1Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 2 Hurtboxes
	data.P2C2ActiveSpheres.clear();
	data.P2C2ActiveSpheres.resize(P2C2Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P2C2VulnState != 4 && P2C2VulnState != 0x1C && P2C2CharState != 0x280C && P2C2CharState != 0x100 && P2C2CharState != 0x20000
			&& P2C2CurAnmchrID != 0x85 && P2C2CurAnmchrID != 0x155 && P2C2CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C2Hurtboxes.size(); i++)
			{
				data.P2C2ActiveSpheres[i] = GetHurtBoxScreenPos(P2C2Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 2 Character 3 Hurtboxes
	data.P2C3ActiveSpheres.clear();
	data.P2C3ActiveSpheres.resize(P2C3Hurtboxes.size());
	if (sAction && MatchFlag == 0)
	{
		if (P2C3VulnState != 4 && P2C3VulnState != 0x1C && P2C3CharState != 0x280C && P2C3CharState != 0x100 && P2C3CharState != 0x20000
			&& P2C3CurAnmchrID != 0x85 && P2C3CurAnmchrID != 0x155 && P2C3CurAnmchrID != 0x88)
		{
			for (int i = 0; i < P2C3Hurtboxes.size(); i++)
			{
				data.P2C3ActiveSpheres[i] = GetHurtBoxScreenPos(P2C3Hurtboxes[i]);
			}
		}
	}
#pragma endregion

#pragma region Player 1 Character 1 H1tboxes
	data.P1C1ActiveHitSpheres.clear();
	data.P1C1ActiveHitSpheres.resize(P1C1HitboxCount);
	if (sAction && MatchFlag == 0)
	{
		if (P1C1HitboxCount && (P1C1ActiveATIChunk.RemainingActiveFrames != 0 && P1C1ActiveATIChunk.FramesBeforeActive < 1))
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
		if (P1C2HitboxCount && (P1C2ActiveATIChunk.RemainingActiveFrames != 0 && P1C2ActiveATIChunk.FramesBeforeActive < 1))
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
		if (P1C3HitboxCount && (P1C3ActiveATIChunk.RemainingActiveFrames != 0 && P1C3ActiveATIChunk.FramesBeforeActive < 1))
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
		if (P2C1HitboxCount && (P2C1ActiveATIChunk.RemainingActiveFrames != 0 && P2C1ActiveATIChunk.FramesBeforeActive < 1))
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
		if (P2C2HitboxCount && (P2C2ActiveATIChunk.RemainingActiveFrames != 0 && P2C2ActiveATIChunk.FramesBeforeActive < 1))
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
		if (P2C3HitboxCount && (P2C3ActiveATIChunk.RemainingActiveFrames != 0 && P2C3ActiveATIChunk.FramesBeforeActive < 1))
		{
			for (int i = 0; i < P2C3Hitboxes.size(); i++)
			{
				data.P2C3ActiveHitSpheres[i] = GetHitboxScreenPos(P2C3Hitboxes[i]);
			}
		}
	}
#pragma endregion

	//========Shot Stuff Coming soon.

	//========Child Character stuff.

#pragma region Player 1 Character 1 Child Hurtboxes & Hitboxes
	if (Player1CharNodeTree && Player2CharNodeTree)
	{

		EmptyTheChildLists();
		GetChildCharacterData(P1Character1Data, P1Character1ID, Player1CharNodeTree);

		data.P1C1ChildActiveSpheres.clear();
		data.P1C1ChildActiveSpheres.resize(ChildHurtboxes.size());
		if (sAction && MatchFlag == 0)
		{

			if (P1Child1TempVulnState != 4 && P1Child1TempVulnState != 0x1C && P1Child1TempCharState != 0x280C
				&& P1Child1TempCharState != 0x100 && P1Child1TempCharState != 0x20000
				&& P1Child1CurAnmchrID != 0x85 && P1Child1CurAnmchrID != 0x155 && P1Child1CurAnmchrID != 0x88)
			{
				for (int i = 0; i < ChildHurtboxes.size(); i++)
				{
					data.P1C1ChildActiveSpheres[i] = GetHurtBoxScreenPos(ChildHurtboxes[i]);
				}
			}
		}

		data.P1C1Child1ActiveHitSpheres.clear();
		data.P1C1Child1ActiveHitSpheres.resize(ChildHitboxes.size());
		if (sAction && MatchFlag == 0)
		{
			if (ChildHitboxes.size() > 0)
			{
				for (int i = 0; i < ChildHitboxes.size(); i++)
				{
					data.P1C1Child1ActiveHitSpheres[i] = GetHitboxScreenPos(ChildHitboxes[i]);
				}
			}
		}



	}
#pragma endregion


	data.Valid = true;

	// Swap buffers
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

	// Render all hurtboxes
	ProcessHurtSpheres(pDevice, 32, data.P1C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P1C3ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C1ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C2ActiveSpheres);
	ProcessHurtSpheres(pDevice, 32, data.P2C3ActiveSpheres);

	// Render all hitboxes
	ProcessHitCapsules(pDevice, 32, data.P1C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C3ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C1ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C2ActiveHitSpheres);
	ProcessHitCapsules(pDevice, 32, data.P2C3ActiveHitSpheres);

	// Render child spheres
	ProcessHurtSpheres(pDevice, 32, data.P1C1ChildActiveSpheres);
	ProcessHitCapsules(pDevice, 32, data.P1C1Child1ActiveHitSpheres);
}

void RenderTheSpheres(LPDIRECT3DDEVICE9 pDevice, std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes, std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes, std::vector<Hurtbox> P2C3Hurtboxes)
{

#pragma region Player 1 Character 1 Hurtboxes
	P1C1ActiveSpheres.clear();
	P1C1ActiveSpheres.resize(P1C1Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Dead, or Invul. Second line checks anmchr state for certain states.
	if (P1C1VulnState != 4 && P1C1VulnState != 0x1C && P1C1CharState != 0x280C && P1C1CharState != 0x100 && P1C1CharState != 0x20000
		&& P1C1CurAnmchrID != 0x85 && P1C1CurAnmchrID != 0x155 && P1C1CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P1C1Hurtboxes.size(); i++)
		{
			P1C1ActiveSpheres[i] = GetHurtBoxScreenPos(P1C1Hurtboxes[i]);
		}
	}

	ProcessHurtSpheres(pDevice, 32, P1C1ActiveSpheres);

#pragma endregion

#pragma region Player 1 Character 2 Hurtboxes
	P1C2ActiveSpheres.clear();
	P1C2ActiveSpheres.resize(P1C2Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Tagged out, Dead, or Invul.
	if (P1C2VulnState != 4 && P1C2VulnState != 0x1C && P1C2CharState != 0x280C && P1C2CharState != 0x100 && P1C2CharState != 0x20000
		&& P1C2CurAnmchrID != 0x85 && P1C2CurAnmchrID != 0x155 && P1C2CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P1C2Hurtboxes.size(); i++)
		{
			P1C2ActiveSpheres[i] = GetHurtBoxScreenPos(P1C2Hurtboxes[i]);
		}

		//for (int i = 0; i < P1C2ActiveSpheres.size(); i++)
		//{
		//	if (P1C2ActiveSpheres[i].Enabled)
		//	{
		//		float newY = 900.0f - P1C2ActiveSpheres[i].Position.Y;
		//		circleFilled(pDevice, P1C2ActiveSpheres[i].Position.X, newY, P1C2ActiveSpheres[i].Radius, 16, P1C2ActiveSpheres[i].Color);
		//	}
		//}

		ProcessHurtSpheres(pDevice, 32, P1C2ActiveSpheres);
	}
#pragma endregion

#pragma region Player 1 Character 3 Hurtboxes
	P1C3ActiveSpheres.clear();
	P1C3ActiveSpheres.resize(P1C3Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Dead, or Invul.
	if (P1C3VulnState != 4 && P1C3VulnState != 0x1C && P1C3CharState != 0x280C && P1C3CharState != 0x100 && P1C3CharState != 0x20000
		&& P1C3CurAnmchrID != 0x85 && P1C3CurAnmchrID != 0x155 && P1C3CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P1C3Hurtboxes.size(); i++)
		{
			P1C3ActiveSpheres[i] = GetHurtBoxScreenPos(P1C3Hurtboxes[i]);
		}

		//for (int i = 0; i < P1C3ActiveSpheres.size(); i++)
		//{
		//	if (P1C3ActiveSpheres[i].Enabled)
		//	{
		//		float newY = 900.0f - P1C3ActiveSpheres[i].Position.Y;
		//		circleFilled(pDevice, P1C3ActiveSpheres[i].Position.X, newY, P1C3ActiveSpheres[i].Radius, 16, P1C3ActiveSpheres[i].Color);
		//	}
		//}

		ProcessHurtSpheres(pDevice, 32, P1C3ActiveSpheres);

	}
#pragma endregion

#pragma region Player 2 Character 1 Hurtboxes

	P2C1ActiveSpheres.clear();
	P2C1ActiveSpheres.resize(P2C1Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Dead, or Invul.
	if (P2C1VulnState != 4 && P2C1VulnState != 0x1C && P2C1CharState != 0x280C && P2C1CharState != 0x100 && P2C1CharState != 0x20000
		&& P2C1CurAnmchrID != 0x85 && P2C1CurAnmchrID != 0x155 && P2C1CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P2C1Hurtboxes.size(); i++)
		{
			P2C1ActiveSpheres[i] = GetHurtBoxScreenPos(P2C1Hurtboxes[i]);
		}

		//for (int i = 0; i < P2C1ActiveSpheres.size(); i++)
		//{
		//	if (P2C1ActiveSpheres[i].Enabled)
		//	{
		//		float newY = 900.0f - P2C1ActiveSpheres[i].Position.Y;
		//		circleFilled(pDevice, P2C1ActiveSpheres[i].Position.X, newY, P2C1ActiveSpheres[i].Radius, 16, P2C1ActiveSpheres[i].Color);
		//	}
		//}

		ProcessHurtSpheres(pDevice, 32, P2C1ActiveSpheres);

	}
#pragma endregion

#pragma region Player 2 Character 2 Hurtboxes

	P2C2ActiveSpheres.clear();
	P2C2ActiveSpheres.resize(P2C2Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Dead, or Invul.
	if (P2C2VulnState != 4 && P2C2VulnState != 0x1C && P2C2CharState != 0x280C && P2C2CharState != 0x100 && P2C2CharState != 0x20000
		&& P2C2CurAnmchrID != 0x85 && P2C2CurAnmchrID != 0x155 && P2C2CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P2C2Hurtboxes.size(); i++)
		{
			P2C2ActiveSpheres[i] = GetHurtBoxScreenPos(P2C2Hurtboxes[i]);
		}

		//for (int i = 0; i < P2C2ActiveSpheres.size(); i++)
		//{
		//	if (P2C2ActiveSpheres[i].Enabled)
		//	{
		//		float newY = 900.0f - P2C2ActiveSpheres[i].Position.Y;
		//		circleFilled(pDevice, P2C2ActiveSpheres[i].Position.X, newY, P2C2ActiveSpheres[i].Radius, 16, P2C2ActiveSpheres[i].Color);
		//	}
		//}

		ProcessHurtSpheres(pDevice, 32, P2C2ActiveSpheres);

	}
#pragma endregion

#pragma region Player 2 Character 3 Hurtboxes

	P2C3ActiveSpheres.clear();
	P2C3ActiveSpheres.resize(P2C3Hurtboxes.size());
	//Meant to turn off display for character when Snapped out, Dead, or Invul.
	if (P2C3VulnState != 4 && P2C3VulnState != 0x1C && P2C3CharState != 0x280C && P2C3CharState != 0x100 && P2C3CharState != 0x20000
		&& P2C3CurAnmchrID != 0x85 && P2C3CurAnmchrID != 0x155 && P2C3CurAnmchrID != 0x88)
	{
		for (int i = 0; i < P2C3Hurtboxes.size(); i++)
		{
			P2C3ActiveSpheres[i] = GetHurtBoxScreenPos(P2C3Hurtboxes[i]);
		}

		//for (int i = 0; i < P2C3ActiveSpheres.size(); i++)
		//{
		//	if (P2C3ActiveSpheres[i].Enabled)
		//	{
		//		float newY = 900.0f - P2C3ActiveSpheres[i].Position.Y;
		//		circleFilled(pDevice, P2C3ActiveSpheres[i].Position.X, newY, P2C3ActiveSpheres[i].Radius, 16, P2C3ActiveSpheres[i].Color);
		//	}
		//}

		ProcessHurtSpheres(pDevice, 32, P2C3ActiveSpheres);

	}
#pragma endregion

#pragma region Player 1 Character 1 Hitboxes Attempt

	P1C1ActiveHitSpheres.clear();
	P1C1ActiveHitSpheres.resize(P1C1HitboxCount);

	//This is designed so that hitboxes only display during active frames.
	if (P1C1HitboxCount && (P1C1ActiveATIChunk.RemainingActiveFrames != 0 && P1C1ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P1C1Hitboxes.size(); i++)
		{
			P1C1ActiveHitSpheres[i] = GetHitboxScreenPos(P1C1Hitboxes[i]);
		}
	}
	ProcessHitCapsules(pDevice, 32, P1C1ActiveHitSpheres);

#pragma endregion

#pragma region Player 1 Character 2 Hitboxes Attempt

	P1C2ActiveHitSpheres.clear();
	P1C2ActiveHitSpheres.resize(P1C2HitboxCount);

	if (P1C2HitboxCount && (P1C2ActiveATIChunk.RemainingActiveFrames != 0 && P1C2ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P1C2Hitboxes.size(); i++)
		{
			P1C2ActiveHitSpheres[i] = GetHitboxScreenPos(P1C2Hitboxes[i]);
		}
	}

	ProcessHitCapsules(pDevice, 32, P1C2ActiveHitSpheres);
#pragma endregion

#pragma region Player 1 Character 3 Hitboxes Attempt

	P1C3ActiveHitSpheres.clear();
	P1C3ActiveHitSpheres.resize(P1C3HitboxCount);

	if (P1C3HitboxCount && (P1C3ActiveATIChunk.RemainingActiveFrames != 0 && P1C3ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P1C3Hitboxes.size(); i++)
		{
			P1C3ActiveHitSpheres[i] = GetHitboxScreenPos(P1C3Hitboxes[i]);
		}
	}

	ProcessHitCapsules(pDevice, 32, P1C3ActiveHitSpheres);
#pragma endregion

#pragma region Player 2 Character 1 Hitboxes Attempt

	P2C1ActiveHitSpheres.clear();
	P2C1ActiveHitSpheres.resize(P2C1HitboxCount);

	if (P2C1HitboxCount && (P2C1ActiveATIChunk.RemainingActiveFrames != 0 && P2C1ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P2C1Hitboxes.size(); i++)
		{
			P2C1ActiveHitSpheres[i] = GetHitboxScreenPos(P2C1Hitboxes[i]);
		}
	}

	ProcessHitCapsules(pDevice, 32, P2C1ActiveHitSpheres);
#pragma endregion

#pragma region Player 2 Character 2 Hitboxes Attempt

	P2C2ActiveHitSpheres.clear();
	P2C2ActiveHitSpheres.resize(P2C2HitboxCount);

	if (P2C2HitboxCount && (P2C2ActiveATIChunk.RemainingActiveFrames != 0 && P2C2ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P2C2Hitboxes.size(); i++)
		{
			P2C2ActiveHitSpheres[i] = GetHitboxScreenPos(P2C2Hitboxes[i]);
		}
	}

	ProcessHitCapsules(pDevice, 32, P2C2ActiveHitSpheres);
#pragma endregion

#pragma region Player 2 Character 3 Hitboxes Attempt

	P2C3ActiveHitSpheres.clear();
	P2C3ActiveHitSpheres.resize(P2C3HitboxCount);

	if (P2C3HitboxCount && (P2C3ActiveATIChunk.RemainingActiveFrames != 0 && P2C3ActiveATIChunk.FramesBeforeActive < 1))
	{
		for (int i = 0; i < P2C3Hitboxes.size(); i++)
		{
			P2C3ActiveHitSpheres[i] = GetHitboxScreenPos(P2C3Hitboxes[i]);
		}
	}

	ProcessHitCapsules(pDevice, 32, P2C3ActiveHitSpheres);
#pragma endregion

#pragma region Player 1 Character 1 Child Hurtboxes & Hitboxes(If applicable)

	if (Player1CharNodeTree && Player2CharNodeTree)
	{
		//For the hurtboxes.
		P1C1ChildActiveSpheres.clear();
		P1C1ChildActiveSpheres.resize(ChildHurtboxes.size());

		//Meant to turn off display for character when Snapped out, Dead, or Invul. Second line checks anmchr state for certain states.
		if (P1Child1TempVulnState != 4 && P1Child1TempVulnState != 0x1C && P1Child1TempCharState != 0x280C
			&& P1Child1TempCharState != 0x100 && P1Child1TempCharState != 0x20000
			&& P1Child1CurAnmchrID != 0x85 && P1Child1CurAnmchrID != 0x155 && P1Child1CurAnmchrID != 0x88)
		{
			for (int i = 0; i < ChildHurtboxes.size(); i++)
			{
				P1C1ChildActiveSpheres[i] = GetHurtBoxScreenPos(ChildHurtboxes[i]);
			}
		}

		ProcessHurtSpheres(pDevice, 32, P1C1ChildActiveSpheres);

		//For Hitboxes.

		EmptyTheChildLists();
		GetChildCharacterData(P1Character1Data, P1Character1ID, Player1CharNodeTree);

		P1C1Child1ActiveHitSpheres.clear();
		P1C1Child1ActiveHitSpheres.resize(ChildHitboxes.size());

		if (ChildHitboxes.size() > 0)
		{


			for (int i = 0; i < ChildHitboxes.size(); i++)
			{
				P1C1Child1ActiveHitSpheres[i] = GetHitboxScreenPos(ChildHitboxes[i]);
			}
		}
		//For Child hitboxes. Test with this first.
		ProcessHitCapsules(pDevice, 32, P1C1Child1ActiveHitSpheres);


	}

#pragma endregion


}