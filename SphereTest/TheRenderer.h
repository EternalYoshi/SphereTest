#pragma once
#include <d3d9.h>
#include <functional>
#include "code\umvc3menu.h"


//class TheRenderer
//{

extern std::function<void(IDirect3DDevice9*)> StopRendering;
void StartRendering(IDirect3DDevice9* pdevice);
//void StopRendering(IDirect3DDevice9* pdevice);
void DrawHurtboxTexture(LPDIRECT3DDEVICE9 pDevice, float alpha);
void DrawHitboxTexture(LPDIRECT3DDEVICE9 pDevice, float alpha);

void RenderTheSpheres(LPDIRECT3DDEVICE9 pDevice, std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes,
	std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes,
	std::vector<Hurtbox> P2C3Hurtboxes);
void RenderFromBuffer(LPDIRECT3DDEVICE9 pDevice, std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes,
	std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes,
	std::vector<Hurtbox> P2C3Hurtboxes);

//};
void ProcessHurtSpheres(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<PrimHitSphere>& ActiveSpheres);
void ProcessHitCapsules(LPDIRECT3DDEVICE9 pDevice, int resolution, const std::vector<PrimHitCapsule>& ActiveSpheres);

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw; // The transformed position for the vertex
	DWORD color;        // The vertex color
};
void UpdateSphereData(std::vector<Hurtbox> P1C1Hurtboxes, std::vector<Hurtbox> P1C2Hurtboxes,
	std::vector<Hurtbox> P1C3Hurtboxes, std::vector<Hurtbox> P2C1Hurtboxes, std::vector<Hurtbox> P2C2Hurtboxes,
	std::vector<Hurtbox> P2C3Hurtboxes);

void RenderSpheresFromBuffer(LPDIRECT3DDEVICE9 pDevice);