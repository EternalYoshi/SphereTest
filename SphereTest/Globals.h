#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H


#include <d3d9.h>

//inline IDirect3DTexture9* g_pRenderTexture = nullptr;
//inline IDirect3DSurface9* g_pRenderSurface = nullptr;
//inline IDirect3DSurface9* g_pOldRenderTarget = nullptr;
//inline IDirect3DSurface9* g_pOldDepthStencil = nullptr;

//extern LPDIRECT3DTEXTURE9 g_pCircleTexture;
//extern LPDIRECT3DSURFACE9 g_pCircleSurface;
//extern LPDIRECT3DSURFACE9 g_pOldRenderTarget;

inline float GameHeight;
inline float GameWidth;
inline int ToggleDisplayKey;

inline IDirect3DBaseTexture9* oldTexture;
inline IDirect3DBaseTexture9* oldTexture2;

//Attempted to make a render target Texture for the Green Stuff.
inline LPDIRECT3DTEXTURE9 g_pRenderTexture1 = NULL;
inline LPDIRECT3DSURFACE9 g_pRenderSurface1 = NULL;
inline LPDIRECT3DSURFACE9 g_pDepthStencilSurface1 = NULL;
inline bool g_renderTargetCreated1 = false;

//Attempted to make a render target Texture for the Red Stuff.
inline LPDIRECT3DTEXTURE9 g_pRenderTexture2 = NULL;
inline LPDIRECT3DSURFACE9 g_pRenderSurface2 = NULL;
inline LPDIRECT3DSURFACE9 g_pDepthStencilSurface2 = NULL;
inline bool g_renderTargetCreated2 = false;

inline float g_hitboxAlpha = 1.0f;

#endif