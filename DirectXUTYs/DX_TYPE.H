/*                                                                           */
/*   DX_TYPE.h   ＤＸ関連の定数とか                                          */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DX_TYPE_H
#define PBGWIN_DX_TYPE_H		"DX_TYPE : Version 0.02"


// 更新履歴 //

// 2000/05/15 : 関数呼び出し規約 FINT 等を追加
// 2000/01/30 : バージョン文字列を出力するようにした
// 1999/12/02 : ＤＤ＿ＵＴＹから分離する


// ヘッダファイル //
#include <windows.h>
#include <d3d.h>
#include <stdio.h>


// 使用するライブラリ //
#pragma comment(lib,"dxguid.lib") // needed for DirectDraw and Direct3D
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"winmm.lib")


// ヘッダファイルのバージョンを表示する //
#pragma message("-----[" PBGWIN_DX_TYPE_H " / " __TIMESTAMP__ "]-----")

#if   DIRECTDRAW_VERSION == 0x0700
	#pragma message("      DirectDraw  : Version 7.00")
#elif DIRECTDRAW_VERSION == 0x0600
	#pragma message("      DirectDraw  : Version 6.00")
#elif DIRECTDRAW_VERSION == 0x0500
	#pragma message("      DirectDraw  : Version 5.00")
#elif DIRECTDRAW_VERSION == 0x0300
	#pragma message("      DirectDraw  : Version 3.00")
#endif

#pragma message("------------------------------")


// マクロ //
#define RELEASE(x)		if((x)!=NULL){(x)->Release();x=NULL;}
#define ZEROMEM(x)		memset(&x,0,sizeof(x))


// 型 //
typedef	LPDIRECTDRAWSURFACE	GRP;

#define PVOID	void __stdcall
#define PBOOL	BOOL __stdcall
#define PGRP	GRP  __stdcall
#define PINT	int  __stdcall

#define PVOIDFN(fn)		void (__stdcall fn)
#define PBOOLFN(fn)		BOOL (__stdcall fn)
#define PGRPFN(fn)		GRP  (__stdcall fn)

#define FVOID	void __fastcall
#define FBOOL	BOOL __fastcall
#define FGRP	GRP  __fastcall

#define FCHAR	char	__fastcall
#define FINT	int		__fastcall
#define FSHORT	short	__fastcall

#define FBYTE	BYTE	__fastcall
#define FWORD	WORD	__fastcall
#define FDWORD	DWORD	__fastcall

#define FVOIDFN(fn)		void (__fastcall fn)
#define FBOOLFN(fn)		BOOL (__fastcall fn)
#define FGRPFN(fn)		GRP  (__fastcall fn)

#define IVOID	__inline void
#define IBOOL	__inline BOOL
#define IGRP	__inline GRP
#define IINT	__inline INT
#define IBYTE	__inline BYTE
#define IWORD	__inline WORD
#define IDWORD	__inline DWORD

#define FIVOID	__forceinline void
#define FIBOOL	__forceinline BOOL
#define FIGRP	__forceinline GRP

#endif
