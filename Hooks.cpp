 #include "global_count.h"
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "circlestrafer.h"
#include "CBulletListener.h"
#include "Interfaces.h"
#include "Skinchanger.h"
#include "autodefuse.h"
#include "RenderManager.h"
#include "lodepng.h"
#include "knifebot.h"
#include "Visuals.h"
#include <d3d9.h>
#include "EnginePrediction.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "hitmarker.h"
#include "laggycompensation.h"
#include <intrin.h>
#include "DamageIndicator.h"
#include "RageBot.h"
#include "LagCompensation2.h"
#include "position_adjust.h"
#include "EnginePrediction.h"
#include "lin_extp.h"
#include "radar.h"
#include "fakelag.h"
#include "experimental.h"
#include "killsay.h"
#include "backdrop.h"
#include "animations.h"
std::vector<impact_info> impacts;
std::vector<hitmarker_info> Xhitmarkers;
static CPredictionSystem* Prediction = new CPredictionSystem();
beam * c_beam = new beam();
backup_visuals * c_visuals = new backup_visuals();
CLagcompensation lagcompensation;
HANDLE worldmodel_handle;
C_BaseCombatWeapon* worldmodel;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif
#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif
Vector Globals::aim_point;
int bigboi::indicator;
bool bigboi::freestand;
bool round_change;
int bigboi::freestandval;
std::vector<trace_info> trace_logs;
int currentfov;
Vector LastAngleAA;
extern Vector LastAngleAA2;
Vector LastAngleAAFake;
Vector last_fake;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
bool Resolver::hitbaim;
bool Globals::Up2date;
int Globals::fired[65];
int Globals::hit[65];
extern float lineLBY;
extern float lineLBY2;

extern float current_desync;
extern float lineRealAngle;
//extern float lineFakeAngle;
extern float last_real;
extern float lspeed;
extern float pitchmeme;
extern float lby2;
extern float inaccuracy;

static bool fuckingcheck;

Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;

float bigboi::current_yaw;
#define STUDIO_RENDER					0x00000001
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots[65];
static int missedLogHits[65];
float fakeangle;
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef long(__stdcall *EndScene_t)(IDirect3DDevice9*);
typedef int(__thiscall* DoPostScreenEffects_t)(IClientModeShared*, int);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef long(__stdcall *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);
using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);

typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pEcx);
EndScene_t o_EndScene;
SceneEnd_t pSceneEnd;
Reset_t o_Reset;
DoPostScreenEffects_t o_DoPostScreenEffects;
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice);
int __stdcall Hooked_DoPostScreenEffects(int a1);
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);
typedef void(__thiscall* LockCursor)(void*);

LockCursor oLockCursor;

void	__stdcall Hooked_LockCursor()
{
	bool xd = options::menu.m_bIsOpen;
	if (xd) {
		interfaces::surface->unlockcursor();
		return;
	}
	oLockCursor(interfaces::surface);
}

namespace GlobalBREAK
{
	bool bVisualAimbotting = false;
	QAngle vecVisualAimbotAngs = QAngle(0.f, 0.f, 0.f);
	int ChokeAmount = 0;
	float flFakewalked = 0.f;
	bool NewRound = false;
	bool WeaponFire = false;
	QAngle fakeangleslocal;
	bool bRainbowCross = true;
	bool dohitmarker;
	float LastTimeWeFired = 0;
	int ShotsFiredLocally = 0;
	int ShotsHitPerEntity[65];
	bool HeadShottedEntity[65] = { false };
	float curFov = 0;
	bool bUsingFakeAngles[65];
	float HitMarkerAlpha = 0.f;
	int TicksOnGround = 0;
	int ticks_while_unducked = 0;
	char* breakmode;
	int AnimationPitchFix = 0;
	float hitchance;
	int NextPredictedLBYUpdate = 0;
	int breakangle;
	int prevChoked = 0;
	bool AAFlip = false;
	bool LEFT;
	bool RIGHT;
	bool BACK;
	char my_documents_folder[MAX_PATH];
	float smt = 0.f;
	QAngle visualAngles = QAngle(0.f, 0.f, 0.f);
	bool bSendPacket = false;
	bool bAimbotting = false;
	CUserCmd* userCMD = nullptr;
	char* szLastFunction = "<No function was called>";
	HMODULE hmDll = nullptr;
	bool bFakewalking = false;
	Vector vecUnpredictedVel = Vector(0, 0, 0);
	float flFakeLatencyAmount = 0.f;
	float flEstFakeLatencyOnServer = 0.f;
	matrix3x4_t traceHitboxbones[128];
	std::array<std::string, 64> resolverModes;
}
int ground_tick;
Vector OldOrigin;
namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel;
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender;
	Utilities::Memory::VMTManager VMTPrediction;
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
	Utilities::Memory::VMTManager VMTDIRECTX;
	Utilities::Memory::VMTManager VMTSurface;
	RecvVarProxyFn g_fnSequenceProxyFn = NULL;

};
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
	VMTDIRECTX.RestoreOriginal();
	VMTEventManager.RestoreOriginal();
	VMTSurface.RestoreOriginal();
	VMTRenderView.RestoreOriginal();
	VMTClient.RestoreOriginal();
}
void Hooks::Initialise()
{
	interfaces::engine->ExecuteClientCmd("clear");
	//--------------- D3D ---------------//
	VMTDIRECTX.Initialise((DWORD*)interfaces::g_pD3DDevice9);
	o_EndScene = (EndScene_t)VMTDIRECTX.HookMethod((DWORD)&EndScene_hooked, 42);
	VMTDIRECTX.Initialise((DWORD*)interfaces::g_pD3DDevice9);
	o_Reset = (Reset_t)VMTDIRECTX.HookMethod((DWORD)&Reset_hooked, 16);
	//--------------- NORMAL HOOKS ---------------//
	VMTClientMode.Initialise((DWORD*)interfaces::ClientMode);
	o_DoPostScreenEffects = (DoPostScreenEffects_t)VMTClientMode.HookMethod((DWORD)Hooked_DoPostScreenEffects, 44);
	VMTPanel.Initialise((DWORD*)interfaces::panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	VMTPrediction.Initialise((DWORD*)interfaces::prediction_dword);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	VMTModelRender.Initialise((DWORD*)interfaces::model_render);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	VMTClientMode.Initialise((DWORD*)interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);
	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);
	VMTClient.Initialise((DWORD*)interfaces::client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 37);
	VMTEventManager.Initialise((DWORD*)interfaces::event_manager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);
	VMTRenderView.Initialise((DWORD*)interfaces::render_view);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);
	VMTSurface.Initialise((DWORD*)interfaces::surface);
	oLockCursor = (LockCursor)VMTSurface.HookMethod((DWORD)Hooked_LockCursor, 67);
	for (ClientClass* pClass = interfaces::client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
	{
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) {
			RecvTable* pClassTable = pClass->m_pRecvTable;
			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) {
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];
				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;
				// Store the original proxy function.
				Hooks::g_fnSequenceProxyFn = (RecvVarProxyFn)pProp->m_ProxyFn;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = Hooks::SetViewModelSequence;

				break;
			}

			break;
		}
	}
	ConVar* nameVar = interfaces::cvar->FindVar("name");
	//--------------- NAME CVAR ---------------//
	if (nameVar)
	{
		*(int*)((DWORD)&nameVar->fnChangeCallback + 0xC) = 0;
	}
	//--------------- EVENT LOG ---------------//
	static auto y = interfaces::cvar->FindVar("sv_showanimstate"); //this probably isn't avaible in modern source
	y->SetValue(1);
	static auto developer = interfaces::cvar->FindVar("developer");
	developer->SetValue(1);
//	static auto con_filter_text_out = interfaces::cvar->FindVar("con_filter_text_out");
	static auto con_filter_enable = interfaces::cvar->FindVar("con_filter_enable");
	static auto con_filter_text = interfaces::cvar->FindVar("con_filter_text");
	static auto dogstfu = interfaces::cvar->FindVar("con_notifytime");
	dogstfu->SetValue(3);
	con_filter_text->SetValue(".     ");
//	con_filter_text_out->SetValue("");
	con_filter_enable->SetValue(2);

}

AnimatedClanTag *animatedClanTag = new AnimatedClanTag();

int __stdcall Hooked_DoPostScreenEffects(int a1)
{
	auto m_local = hackManager.pLocal();

	for (auto i = 0; i < interfaces::glow_manager->size; i++)
	{
		auto glow_object = &interfaces::glow_manager->m_GlowObjectDefinitions[i];
		IClientEntity *m_entity = glow_object->m_pEntity;

		if (!glow_object->m_pEntity || glow_object->IsUnused() || !m_local)
			continue;
		if (strstr(m_entity->GetClientClass()->m_pNetworkName, "Weapon"))
		{
			if (options::menu.visuals.OtherEntityGlow.getstate())
			{
				float m_flRed = options::menu.ColorsTab.GlowOtherEnt.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowOtherEnt.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowOtherEnt.GetValue()[2];
				
				glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
				glow_object->m_flGlowAlpha = 1.f;
				glow_object->m_bRenderWhenOccluded = true;
				glow_object->m_bRenderWhenUnoccluded = false;

				c_beam->glow = true;
			}
		}

		if (m_entity->isValidPlayer() && m_entity->cs_player())
		{
			if (m_entity == m_local && options::menu.visuals.Glowz_lcl.GetValue() > 0)
			{
				if (m_local->IsAlive() && options::menu.visuals.localmaterial.getindex() < 6)
				{
					float m_flRed = options::menu.ColorsTab.GlowLocal.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowLocal.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowLocal.GetValue()[2];
					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.Glowz_lcl.GetValue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;

				}

			}

			if (options::menu.visuals.GlowZ.GetValue() > 0 && m_entity->team() != m_local->team())
			{	
					float m_flRed = options::menu.ColorsTab.GlowEnemy.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowEnemy.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowEnemy.GetValue()[2];

					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.GlowZ.GetValue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
					//	glow_object->m_bPulsatingChams = 2;		
			}
			if (options::menu.visuals.team_glow.GetValue() > 0)
			{
				if (m_entity->team() == m_local->team() && m_entity != m_local)
				{
					float m_flRed = options::menu.ColorsTab.GlowTeam.GetValue()[0], m_flGreen = options::menu.ColorsTab.GlowTeam.GetValue()[1], m_flBlue = options::menu.ColorsTab.GlowTeam.GetValue()[2];

					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = options::menu.visuals.team_glow.GetValue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
					//	glow_object->m_bPulsatingChams = 1;
				}
			}
		}
	}
	return o_DoPostScreenEffects(interfaces::ClientMode, a1);
}
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	auto hr = o_Reset(pDevice, pPresentationParameters);
	if (hr >= 0)
	{
		bool gey;
		gey = true;
	}
	return hr;
}
struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};
#define M_PI 3.14159265358979323846
void CircleFilledRainbowColor(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color::FromHSB(hue, 1.f, 1.f);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
#define M_PI 3.14159265358979323846


void greyone(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color(15, 15, 15);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
void colorboy69(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = (Color)options::menu.ColorsTab.spreadcrosscol.GetValue();
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice)
{
	//this will probably get drawn even over the console and other CSGO hud elements, but whatever
	//this will also draw over the menu so we should disable it if the menu is open
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto g_LocalPlayer = pLocal;
	if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_LocalPlayer->GetWeapon2()) {
		int w, h;
		interfaces::engine->GetScreenSize(w, h); w /= 2; h /= 2;
		if (interfaces::engine->IsInGame() && interfaces::engine->IsConnected())
		{
			int w, h;
			static float rot = 0.f;
			interfaces::engine->GetScreenSize(w, h); w /= 2; h /= 2;
			C_BaseCombatWeapon* pWeapon = g_LocalPlayer->GetWeapon2();
			if (pWeapon)
			{
				short Index = (int)pWeapon->GetItemDefinitionIndex();
				
				if (g_LocalPlayer && Index != 42 && Index != 59 && Index != 500)
				{
					if (options::menu.visuals.SpreadCrosshair.getindex() != 0 && !options::menu.m_bIsOpen)
					{
						auto accuracy = pWeapon->GetInaccuracy() * (90 * 6.5);

						switch (options::menu.visuals.SpreadCrosshair.getindex())
						{
						case 1:
						{
							greyone(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 2:
						{
							colorboy69(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 3:
						{
							CircleFilledRainbowColor(w, h, accuracy, 0, 1, 50, pDevice);
						}
						break;

						case 4:
						{
							CircleFilledRainbowColor(w, h, accuracy, rot, 1, 50, pDevice);
						}
						break;
						}

						rot += 1.f;
						if (rot > 360.f)
							rot = 0.f;
					}
				}
			}
		}
	}


	return o_EndScene(pDevice);
}
void MovementCorrection(CUserCmd* userCMD, IClientEntity * local)
{
	if (!local)
		return;

	if (userCMD->forwardmove) {
		userCMD->buttons &= ~(userCMD->forwardmove < 0 ? IN_FORWARD : IN_BACK);
		userCMD->buttons |= (userCMD->forwardmove > 0 ? IN_FORWARD : IN_BACK);
	}
	if (userCMD->sidemove) {
		userCMD->buttons &= ~(userCMD->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
		userCMD->buttons |= (userCMD->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
	}

}
float clip(float n, float lower, float upper)
{
	return (std::max)(lower, (std::min)(n, upper));
}
int kek = 0;
int autism = 0;
int speed = 0;
static float testtimeToTick;
static float testServerTick;
static float testTickCount64 = 1;

float NormalizeYaw(float value)
{
	while (value > 180)
		value -= 360.f;
	while (value < -180)
		value += 360.f;
	return value;
}
float random_float(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(("vstdlib.dll")), ("RandomFloat"));
	return m_RandomFloat(min, max);
}
LinearExtrapolations linear_extraps;
std::string Tag = " fantailcommunity.xyz ";
std::string Tag2 = "";
void set_clan_tag(const char* tag, const char* clan_name)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(Utilities::Memory::FindPatternV2(XorStr("engine.dll"), XorStr("53 56 57 8B DA 8B F9 FF 15")));
	pSetClanTag(tag, clan_name);
}
void clan_changer()
{
	auto m_local = hackManager.pLocal();
	bool OOF = false;
	if (options::menu.ColorsTab.ClanTag.getstate())
	{
		if (!m_local || !m_local->IsAlive() || !interfaces::engine->IsInGame() || !interfaces::engine->connected())
		{
			if (!OOF)
			{
				Tag2 += Tag.at(0);
				Tag2.erase(0, 1);
				set_clan_tag(Tag2.c_str(), "mirror v6");
			}
			else
			{
				OOF = true;
			}
		}
		static size_t lastTime = 0;

		if (GetTickCount() > lastTime)
		{
			OOF = false;
			Tag += Tag.at(0);
			Tag.erase(0, 1);
			set_clan_tag(Tag.c_str(), "mirror v6");
			lastTime = GetTickCount() + 650;
		}
	}
}
struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};
std::deque<CIncomingSequence> sequences;
int32_t lastincomingsequencenumber;


bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	uintptr_t* FPointer; __asm { MOV FPointer, EBP }
	byte* SendPacket = (byte*)(*FPointer - 0x1C);
	GlobalBREAK::bSendPacket = *SendPacket;
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
	//	static bool boost_fps = false;
	//	static bool abc = false;
		
		c_misc->colour_modulation();

		pCmd->sidemove = pCmd->sidemove;
		pCmd->upmove = pCmd->upmove;
		pCmd->forwardmove = pCmd->forwardmove;

		GlobalBREAK::bSendPacket = *SendPacket;
		if (GetAsyncKeyState(options::menu.misc.manualleft.GetKey()))
		{
			bigboi::indicator = 1;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualright.GetKey()))
		{
			bigboi::indicator = 2;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualback.GetKey()))
		{
			bigboi::indicator = 3;
		}
		else if (GetAsyncKeyState(options::menu.misc.manualfront.GetKey()))
		{
			bigboi::indicator = 4;
		}

		defususmaximus(pCmd);

		GlobalBREAK::smt = frametime;
		GlobalBREAK::userCMD = pCmd;
		GlobalBREAK::vecUnpredictedVel = pLocal->GetVelocity();

		clan_changer();

		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;
		uintptr_t* framePtr;
		__asm mov framePtr, ebp;
		GlobalBREAK::bSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);

		if (pLocal->GetFlags() & FL_ONGROUND)
			GlobalBREAK::TicksOnGround++;
		else
			GlobalBREAK::TicksOnGround = 0;

		if (pLocal->GetFlags() & FL_DUCKING)
			GlobalBREAK::ticks_while_unducked = 0;
		else
			GlobalBREAK::ticks_while_unducked++;

		if (GlobalBREAK::bSendPacket)
			GlobalBREAK::prevChoked = interfaces::client_state->chokedcommands;
		if (!GlobalBREAK::bSendPacket)
			GlobalBREAK::visualAngles = QAngle(pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z);
		if (GlobalBREAK::TicksOnGround == 1)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 1 && pLocal->getFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 0 && pLocal->GetFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;

		globalsh.bSendPaket = true;

		if (options::menu.misc.FakeLagChoke.GetValue() > 0 || options::menu.misc.FakeLagChoke2.GetValue() > 0)
			globalsh.bSendPaket = false;
		if (interfaces::client_state->chokedcommands > 14 || (interfaces::client_state->chokedcommands == globalsh.ChokeAmount &&
			(options::menu.misc.FakeLagChoke.GetValue() > 0 || options::menu.misc.FakeLagChoke2.GetValue() > 0)))
			globalsh.bSendPaket = false;		globalsh.bSendPaket = (bool*)(*(DWORD*)pebp - 0x1C);
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		if (globalsh.bSendPaket)
			globalsh.prevChoked = interfaces::client_state->chokedcommands;

		IClientEntity* pEntity;
	//	Vector ClientAngles; interfaces::engine->get_viewangles(ClientAngles);

		if (options::menu.misc.SniperCrosshair.getstate() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pLocal->GetWeapon2() && pLocal->GetWeapon2()->m_bIsSniper()) {
				ConVar* cross = interfaces::cvar->FindVar("weapon_debug_spread_show");
				cross->nFlags &= ~FCVAR_CHEAT;
				cross->SetValue(3);
			}
		}
		else {
			ConVar* cross = interfaces::cvar->FindVar("weapon_debug_spread_show");
			cross->nFlags &= ~FCVAR_CHEAT;
			cross->SetValue(0);
		}
		if (pLocal) // isconnected and is ingame original check location
		{

			if (pLocal->GetFlags() & FL_ONGROUND && !(CMBacktracking::Get().current_record->m_nFlags & FL_ONGROUND)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = true;
			}
			if (pLocal->GetFlags() & FL_ONGROUND && interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			if (!pLocal->GetFlags() & FL_ONGROUND && !interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			if (interfaces::m_iInputSys->IsButtonDown(MOUSE_LEFT)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			for (int i = 1; i < 2; i++)
			{
				if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][i].flags)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}

			Prediction->StartPrediction(pCmd);
			{
				animfix->fix_local_player_animations();

			//	if (options::menu.aimbot.extrapolation.getstate())
				linear_extraps.run();

				Hacks::MoveHacks(pCmd, bSendPacket);

		//		if (options::menu.aimbot.delay_shot.getindex() > 2)
		//		{
		//			bt_2->pasted_backTrack(pCmd);
		//		}

			}
			Prediction->EndPrediction(pCmd);

			if (pCmd->forwardmove) 
			{
				pCmd->buttons &= ~(pCmd->forwardmove < 0 ? IN_FORWARD : IN_BACK);
				pCmd->buttons |= (pCmd->forwardmove > 0 ? IN_FORWARD : IN_BACK);
			}
			if (pCmd->sidemove) {
				pCmd->buttons &= ~(pCmd->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
				pCmd->buttons |= (pCmd->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
			}

		}

		IClientEntity* LocalPlayer = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
		float flServerTime = (float)(LocalPlayer->GetTickBase()  * interfaces::globals->interval_per_tick);
		static float next_time = 0;
		MovementCorrection(pCmd, LocalPlayer);
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (pLocal->IsAlive())
		{
			if (forward > 450) forward = 450;
			if (right > 450) right = 450;
			if (up > 450) up = 450;
			if (forward < -450) forward = -450;
			if (right < -450) right = -450;
			if (up < -450) up = -450;
			pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
			pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
			pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);
		}

		if (options::menu.misc.OtherSafeMode.getindex() < 3)
		{
			game_utils::NormaliseViewAngle(pCmd->viewangles);
			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}
			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log(" Re-calculating angles");
				game_utils::NormaliseViewAngle(pCmd->viewangles);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}
			if (pCmd->viewangles.x > 90)
			{
				pCmd->forwardmove = -pCmd->forwardmove;
			}
			if (pCmd->viewangles.x < -90)
			{
				pCmd->forwardmove = -pCmd->forwardmove;
			}
		
		if (!bSendPacket)
		{
			LastAngleAAReal = pCmd->viewangles;
			c_beam->real = pCmd->viewangles.y;
		}

		lineLBY = pLocal->GetLowerBodyYaw();
		lineLBY2 = LastAngleAAReal.y - pLocal->GetLowerBodyYaw();
		if (bSendPacket)
		{
			c_beam->cham_origin = pLocal->GetAbsOrigin();
			if (pCmd->command_number % 3)
				LastAngleAAFake = pCmd->viewangles;
		}

		switch (options::menu.visuals.optimize.getstate())
		{
			case true:
				c_misc->optimize();
				break;
		}

		if (pLocal && pLocal->IsAlive() && pWeapon != nullptr && !game_utils::IsGrenade(pWeapon) && !(pWeapon->isZeus() || pWeapon->IsC4()))
		{
			inaccuracy = pWeapon->GetInaccuracy() * 1000;
			lspeed = pLocal->GetVelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}

		if (!bSendPacket || bSendPacket)
		{
			c_beam->fake = pCmd->viewangles;
		}

		Vector fl = pLocal->GetAbsAngles();
		if (hackManager.pLocal()->GetBasePlayerAnimState())
		{
			fl.y = LastAngleAAFake.y;
		}
		fl.z = 0.f;

		c_beam->cham_angle = fl;
	}
	return false;
}
/*
static void drawThiccLine(int x1, int y1, int x2, int y2, int type, Color color) {
	if (type > 1) {
		Render::Line(x1, y1 - 1, x2, y2 - 1, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1, y1 + 1, x2, y2 + 1, color);
		Render::Line(x1, y1 - 2, x2, y2 - 2, color);
		Render::Line(x1, y1 + 2, x2, y2 + 2, color);
	}
	else {
		Render::Line(x1 - 1, y1, x2 - 1, y2, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1 + 1, y1, x2 + 1, y2, color);
		Render::Line(x1 - 2, y1, x2 - 2, y2, color);
		Render::Line(x1 + 2, y1, x2 + 2, y2, color);
	}
}
*/
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y | %d | %X", &tstruct);
	return buf;
}
Color urmamasuckmylargegenetalia(int speed, int offset)
{
	float hue = (float)((GetCurrentTime() + offset) % speed);
	hue /= speed;
	return Color::FromHSB(hue, 1.0F, 1.0F);
}
template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
static DWORD* deathNotice;
static void(__thiscall *ClearDeathNotices)(DWORD);


void things(IGameEvent* pEvent)
{
	if (!strcmp(pEvent->GetName(), "round_prestart") && interfaces::engine->IsInGame() && interfaces::engine->IsConnected() && hackManager.pLocal())
	{
		deathNotice = nullptr;
		fuckingcheck = true;
		c_misc->anotherpcheck = true;
	
	}
}
template<class T>
static T* FindHudElementX(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(game_utils::FindPattern1(("client_panorama.dll"), ("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(game_utils::FindPattern1(("client_panorama.dll"), ("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));
	return (T*)find_hud_element(pThis, name);
}


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (options::menu.visuals.Active.getstate() && options::menu.visuals.OtherNoScope.getstate() && strcmp("HudZoom", interfaces::panels->GetName(vguiPanel)) == 0)
		return;
	int w, h;
	int centerW, centerh, topH;
	interfaces::engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)interfaces::panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
		interfaces::m_iInputSys->EnableInput(!options::menu.m_bIsOpen);

		if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame() && pLocal->isValidPlayer())
		{

			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)interfaces::pinput + 0xEC);
			CUserCmd* pCmd = cmdlist;
			RECT scrn = Render::GetViewport();
			if (options::menu.misc.Radar.getstate())
				DrawRadar();
			if (globalsh.bSendPaket) {
				globalsh.prevChoked = interfaces::client_state->chokedcommands;
			}
			if (options::menu.visuals.LCIndicator.getindex() != 0)
			{
				if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
				{
					RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Lag Comp Status:");
					bool breaklagcomp = false;
					auto last_origin = pLocal->GetAbsOrigin2();
					if (pLocal->GetAbsOrigin2() != last_origin) 
					{
						if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->GetAbsOrigin2().Length2DSqr() > 4096) {
							breaklagcomp = true;
							last_origin = pLocal->GetAbsOrigin2();
						}
					}

					else if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->GetVelocity().Length2D() > 125 && GlobalBREAK::bSendPacket && GlobalBREAK::ChokeAmount == c_fakelag->break_lagcomp_mm_2() || GlobalBREAK::ChokeAmount == 5) {
						breaklagcomp = true;

					}
					else {
						breaklagcomp = false;
					}
					//	Render::Text(6, scrn.bottom - 88, breaklagcomp ? Color(0, 255, 30, 255) : Color(255, 0, 30, 255), Render::Fonts::LBY, "LC");

					if (options::menu.visuals.LCIndicator.getindex() == 1)
					{
						Render::Text(9, scrn.bottom - 88, breaklagcomp ? Color(0, 255, 30, 255) : Color(255, 0, 30, 255), Render::Fonts::LBY, "LC");
					}


					if (options::menu.visuals.LCIndicator.getindex() > 1)
					{
						if (breaklagcomp)
						{
							Render::Text(9, centerh + 115, Color(255, 255, 255, 255), Render::Fonts::xd, " Lag Comp Status:");
							Render::Text(TextSize_2.left + 11, centerh + 115, Color(0, 90, 250, 255), Render::Fonts::xd, "Active");
						}

						if (!breaklagcomp)
						{
							Render::Text(9, centerh + 115, Color(255, 255, 255, 255), Render::Fonts::xd, " Lag Comp Status:");
							Render::Text(TextSize_2.left + 11, centerh + 115, Color(255, 0, 80, 255), Render::Fonts::xd, "Normal");
						}
					}

				}
			}
			
			if (options::menu.visuals.killfeed.getstate())
			{
				if (hackManager.pLocal()) {
					if (!deathNotice) deathNotice = FindHudElementX<DWORD>("CCSGO_HudDeathNotice");
					if (deathNotice) {
						float* localDeathNotice = (float*)((DWORD)deathNotice + 0x50);
						if (localDeathNotice) *localDeathNotice = options::menu.visuals.killfeed.getstate() ? FLT_MAX : 1.5f;
						if (fuckingcheck && deathNotice - 20) {
							if (!ClearDeathNotices)
								ClearDeathNotices = (void(__thiscall*)(DWORD))game_utils::FindPattern1("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");
							if (ClearDeathNotices)
							{
								ClearDeathNotices(((DWORD)deathNotice - 20)); 
								fuckingcheck = false;
							}

						}
					}
				}
			}


			if (options::menu.visuals.FakeDuckIndicator.getindex() != 0)
			{
				switch (options::menu.visuals.FakeDuckIndicator.getindex())
				{
				case 1:
				{
					if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !c_fakelag->shot)
					{
						Render::Text(9, scrn.bottom - 54, Color(0, 250, 30, 255), Render::Fonts::LBY, "FD");
					}
					else
						Render::Text(9, scrn.bottom - 54, Color(255, 0, 30, 255), Render::Fonts::LBY, "FD");
				}
				break;

				case 2:
				{
					RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Fake Duck Status:");
					if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !c_fakelag->shot)
					{
						Render::Text(9, centerh + 130, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Duck Status:");
						Render::Text(TextSize_2.left + 11, centerh + 130, Color(0, 90, 250, 255), Render::Fonts::xd, "Active");
					}
					else
					{
						Render::Text(9, centerh + 130, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Duck Status:");
						Render::Text(TextSize_2.left + 11, centerh + 130, Color(255, 0, 80, 255), Render::Fonts::xd, "Inactive");
					}
				}
				break;
				}
			}
			if (options::menu.visuals.LBYIndicator.getindex() != 0)
			{

				RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "LBY");
				RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " LBY Status:");

				bool invalid_lby = (LastAngleAAReal.y - pLocal->GetLowerBodyYaw() >= -35 && LastAngleAAReal.y - pLocal->GetLowerBodyYaw() <= 35) || pLocal->IsMoving();
				switch (options::menu.visuals.LBYIndicator.getindex())
				{
				case 1:
				{
					if (invalid_lby)
					{
						Render::Text(9, scrn.bottom - 71, Color(255, 0, 30, 255), Render::Fonts::LBY, "LBY");
					}
					else
					{
						Render::Text(9, scrn.bottom - 71, Color(0, 250, 30, 255), Render::Fonts::LBY, "LBY");
					}
				}
				break;

				case 2:
				{
					if (invalid_lby)
					{
						Render::Text(9, centerh + 100, Color(255, 255, 255, 255), Render::Fonts::xd, " Lby Status:");
						Render::Text(TextSize_2.left + 11, centerh + 100, Color(255, 0, 80, 255), Render::Fonts::xd, "Normal");
					}
					else
					{
						Render::Text(9, centerh + 100, Color(255, 255, 255, 255), Render::Fonts::xd, " Lby Status:");
						Render::Text(TextSize_2.left + 11, centerh + 100, Color(0, 90, 250, 255), Render::Fonts::xd, "Broken");
					}
				}
				break;
				}
			}

			if (options::menu.visuals.fake_indicator.getindex() != 0)
			{
				float yaw_difference = LastAngleAAReal.y - LastAngleAAFake.y;
				bool fake_green = yaw_difference >= 35.f;
				bool fake_orange = yaw_difference < 35.f && yaw_difference > 20.f;

				switch (options::menu.visuals.fake_indicator.getindex())
				{
					case 1:
					{
						RECT TextSize = Render::GetTextSize(Render::Fonts::LBY, "FAKE");

						if (fake_green)
						{
							Render::Text(9, scrn.bottom - 105, Color(10, 255, 30, 255), Render::Fonts::LBY, "FAKE");
						}

						else if (fake_orange)
						{
							Render::Text(9, scrn.bottom - 105, Color(255, 150, 10, 255), Render::Fonts::LBY, "FAKE");
						}

						else
						{
							Render::Text(9, scrn.bottom - 105, Color(255, 0, 30, 255), Render::Fonts::LBY, "FAKE");
						}
					}
					break;

					case 2:
					{
						RECT TextSize_2 = Render::GetTextSize(Render::Fonts::LBYIndicator, " Desync Status:");
						switch (fake_green)
						{
							case true:
							{
								Render::Text(9, centerh + 145, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Status:");
								Render::Text(TextSize_2.left + 11, centerh + 145, Color(0, 90, 250, 255), Render::Fonts::xd, yaw_difference > 58 ? "Stretched" : "Optimal");
							}
							break;

							case false:
							{
								Render::Text(9, centerh + 145, Color(255, 255, 255, 255), Render::Fonts::xd, " Fake Status:");
								Render::Text(TextSize_2.left + 11, centerh + 145, Color(255, 0, 80, 255), Render::Fonts::xd, "Minimal");
							}
							break;
						}
					}
					break;
				}
			}

			if (options::menu.visuals.manualaa_type.getindex() > 0)
			{
				switch (options::menu.visuals.manualaa_type.getindex())
				{
					case 1:
					{
						c_visuals->single_arrow();
					}
					break;

					case 2:
					{
						c_visuals->all_arrows();
					}
					break;
				}
			}

			if (options::menu.visuals.cheatinfo.getstate())
			{
				char jew[64];
				float blob = interfaces::client_state->chokedcommands;
				float hc = options::menu.aimbot.AccuracyHitchance.GetValue();
				float md = options::menu.aimbot.AccuracyMinimumDamage.GetValue();
			
				sprintf_s(jew, " Pitch: %.1f", pitchmeme);
				Render::Text(9, (centerh - 20), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Yaw: %.1f", c_beam->real);
				Render::Text(9, (centerh), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Lby: %.1f", lineLBY);
				Render::Text(9, (centerh + 20), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Real / Lby Delta: %.1f", NormalizeYaw(lineLBY2));
				Render::Text(9, (centerh + 40), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

				sprintf_s(jew, " Speed: %.1f", lspeed);
				Render::Text(9, (centerh + 60), Color(250, 250, 250, 255), Render::Fonts::xd, jew);

			}

		}

		if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
		{
			Hacks::DrawHacks();

			if (options::menu.visuals.OtherHitmarker.getstate())
				hitmarker::singleton()->on_paint();
		}
		skinchanger.update_settings();

		if (options::menu.m_bIsOpen && options::menu.ColorsTab.menu_backdrop.getstate())
		{
			Drop::DrawBackDrop();
		}

		options::DoUIFrame();


	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}
bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = interfaces::prediction_dword;
	result = origFunc(ecxVal);
	if (options::menu.visuals.OtherNoVisualRecoil.getstate() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;
		float* m_LocalViewAngles = NULL;
		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}
		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();
		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}
	return result;
}

player_info_t GetInfo(int Index) {
	player_info_t Info;
	interfaces::engine->GetPlayerInfo(Index, &Info);
	return Info;
}
typedef void(__cdecl* MsgFn)(const char* msg, va_list);

bool warmup = false;

auto HitgroupToString = [](int hitgroup) -> std::string
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "HEAD";
	case HITGROUP_CHEST:
		return "CHEST";
	case HITGROUP_STOMACH:
		return "STOMACH";
	case HITGROUP_LEFTARM:
		return "LEFT ARM";
	case HITGROUP_RIGHTARM:
		return "RIGHT ARM";
	case HITGROUP_LEFTLEG:
		return "LEFT LEG";
	case HITGROUP_RIGHTLEG:
		return "RIGHT LEG";
	default:
		return "BODY";
	}
};
void Msg(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}

void hitsound()
{
	const char* _custom = "csgo\\sound\\mirror_custom.wav";
	switch (options::menu.misc.hitmarker_sound.getindex())
	{
	case 1: PlaySoundA(cod, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 2: interfaces::engine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
		break;
	case 3: PlaySoundA(bubble, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 4: PlaySoundA(bameware_sound, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 5: PlaySoundA(anime, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 6: PlaySoundA(hitler_wav, NULL, SND_ASYNC | SND_MEMORY);
		break;
	case 7: interfaces::engine->ExecuteClientCmd("play training\\bell_impact"); // buttons\light_power_on_switch_01.wav
		break;
	case 8: PlaySoundA(_custom, NULL, SND_ASYNC);
		break;
	}
}


void ConColorMsg(Color const &color, const char* buf, ...)
{
	using ConColFn = void(__stdcall*)(Color const &, const char*, ...);
	auto ConCol = reinterpret_cast<ConColFn>((GetProcAddress(GetModuleHandle("tier0.dll"), "?ConColorMsg@@YAXABVColor@@PBDZZ")));
	ConCol(color, buf);
}


struct bullet_impact_log
{
	bullet_impact_log(int userid, Vector fire_pos, Vector impact_pos, float impact_time) {
		this->uid = userid;
		this->fire_posit = fire_pos;
		this->impact_posit = impact_pos;
		this->impac_time = impact_time;
	}

	int uid;
	Vector fire_posit;
	Vector impact_posit;
	float impac_time;
};

std::vector< bullet_impact_log > bullet_logs;

void DrawBeam(Vector src, Vector end, Color Color) {
	int r, g, b, a;
	Color.GetColor(r, g, b, a);
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;

	beamInfo.m_pszModelName = "sprites/blueglow1.vmt";

	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 3.0;
	beamInfo.m_flWidth = 1.f;
	beamInfo.m_flEndWidth = 1.f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 1.f;
	beamInfo.m_flBrightness = a;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = r;
	beamInfo.m_flGreen = g;
	beamInfo.m_flBlue = b;
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;

	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;

	Beam_t* myBeam = interfaces::render_beams->CreateBeamPoints(beamInfo);

	if (myBeam)
		interfaces::render_beams->DrawBeam(myBeam);
}


void draw_hitboxes(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		interfaces::DebugOverlay->DrawPill(vMin, vMax, hitbox->m_flRadius, r, g, b, a, duration);
	}
}

bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{
	CBulletListener::singleton()->OnStudioRender();
	IClientEntity* localplayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		std::vector<dropdownboxitem> spike = options::menu.misc.killsay.items;
		things(Event);
		std::string event_name = Event->GetName();

		angle_correction ac;

		if (!strcmp(Event->GetName(), "round_prestart"))
		{
			skinchanger.set_viewmodel();
			fuckingcheck = false;
		}
		
		if (!strcmp(Event->GetName(), "round_start"))
		{
			skinchanger.set_viewmodel();
		}

		if (!strcmp(Event->GetName(), "player_death"))
		{
			skinchanger.apply_killcon(Event);	

			int deadfag = Event->GetInt("userid");
			int attackingfag = Event->GetInt("attacker");

			if (spike[1].GetSelected && interfaces::engine->GetPlayerForUserID(deadfag) == interfaces::engine->GetLocalPlayer() && interfaces::engine->GetPlayerForUserID(attackingfag) != interfaces::engine->GetLocalPlayer())
			{
				if (!deathmsg.empty()) {
					std::string msg = deathmsg[rand() % deathmsg.size()];
					std::string str;
					str.append("say ");
					str.append(msg);
					interfaces::engine->ClientCmd_Unrestricted(str.c_str());
				}
			}
			if (spike[0].GetSelected && interfaces::engine->GetPlayerForUserID(deadfag) != interfaces::engine->GetLocalPlayer() && interfaces::engine->GetPlayerForUserID(attackingfag) == interfaces::engine->GetLocalPlayer())
			{
				if (!killmsg.empty()) 
				{
					std::string msg = killmsg[rand() % killmsg.size()];
					std::string str;
					str.append("say ");
					str.append(msg);
					interfaces::engine->ClientCmd_Unrestricted(str.c_str());
				}
			}
		}
	
		if (!strcmp(Event->GetName(), "game_newmap"))
		{
			skinchanger.set_viewmodel();			

			ac.mirror_aesthetic_console();
			interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] changing map.     \n");

			if (options::menu.visuals.colmod.GetValue() < 100.f)
			{
				options::menu.visuals.colmodupdate.SetState(true);
				ac.mirror_aesthetic_console();
				interfaces::cvar->ConsoleColorPrintf(Color(250, 0, 200, 255), " [info] refreshed world modulation.     \n");
			}

		}

		if (!strcmp(Event->GetName(), "player_hurt"))
		{
			int attackerid = Event->GetInt("attacker");
			int entityid = interfaces::engine->GetPlayerForUserID(attackerid);
			if (entityid == interfaces::engine->GetLocalPlayer())
			{
				hitsound();
				int nUserID = Event->GetInt("attacker");
				int nDead = Event->GetInt("userid");
				int gaylol = Event->GetInt("hitgroup");

				if ((nUserID || nDead) && nUserID != nDead)
				{
					player_info_t killed_info = GetInfo(interfaces::engine->GetPlayerForUserID(nDead));
					player_info_t killer_info = GetInfo(interfaces::engine->GetPlayerForUserID(nUserID));
					IClientEntity* hurt = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetPlayerForUserID(Event->GetInt("userid")));

					if (options::menu.ColorsTab.DebugLagComp.getstate())
					{
						studiohdr_t* studio_hdr = interfaces::model_info->GetStudiomodel(hurt->GetModel());
						mstudiohitboxset_t* set = studio_hdr->GetHitboxSet(0);
						for (int i = 0; i < set->numhitboxes; i++)
						{
							mstudiobbox_t* hitbox = set->GetHitbox(i);
							if (!hitbox)
								continue;

							draw_hitboxes(hurt, 220, 220, 220, 255, 1, hitbox->m_flRadius);
						}
					}
					auto remaining_health = Event->GetString("health");
					int remainaing = Event->GetInt("health");
					auto dmg_to_health = Event->GetString("dmg_health");
					std::string szHitgroup = HitgroupToString(gaylol);
					interfaces::cvar->ConsoleColorPrintf(Color(220, 5, 250, 255), "Mirror: ");
					std::string One = "-";
					std::string Two = dmg_to_health;
					std::string Three = " in the ";
					std::string Four = szHitgroup;
					std::string gey = " of ";
					std::string yes = killed_info.name;
					std::string yyes = " [";
					std::string yyyes = " hp: ";
					std::string yyyyes = remaining_health;
					std::string yyyyyes = " ]";
					std::string newline = ".     \n";//no,i'm not retarded, i tried with stringstream but it didn't work.
					std::string uremam = One + Two + Three + Four + gey + yes + yyes + yyyes + yyyyes + yyyyyes + newline;

						
					switch (options::menu.visuals.logs.getindex())
					{
						case 1: Msg(uremam.c_str());
							break;
						case 2: ConColorMsg(Color(options::menu.ColorsTab.console_colour.GetValue()) ,uremam.c_str());
							break;
					}

					/*
					if (options::menu.visuals.bulletbeam.getstate())
					{
						auto shooter = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(
							interfaces::engine->GetPlayerForUserID(Event->GetInt("userid"))));

						int uid = Event->GetInt("userid");
						int x = Event->GetInt("x");
						int y = Event->GetInt("y");
						int z = Event->GetInt("z");

						if (interfaces::engine->GetPlayerForUserID(uid) == interfaces::engine->GetLocalPlayer())
						{
							Vector position(x, y, z);

							Vector CorrectedPos = Vector(shooter->GetEyePosition().x, shooter->GetEyePosition().y,
								shooter->GetEyePosition().z - 0.75);

							bullet_logs.push_back(bullet_impact_log(Event->GetInt("userid"), CorrectedPos, position,
								interfaces::globals->curtime));
						}
					}
					*/
				}
			}
		}

		/*
		if (options::menu.visuals.bulletbeam.getstate())
		{
			for (unsigned int i = 0; i < bullet_logs.size(); i++) {
				auto shooter = interfaces::ent_list->get_client_entity(
					interfaces::engine->GetPlayerForUserID(bullet_logs[i].uid));

				if (shooter && on_hit == true) {
					DrawBeam(bullet_logs[i].fire_posit, bullet_logs[i].impact_posit,
						Color(options::menu.ColorsTab.bullet_tracer.GetValue()));
				}

				bullet_logs.erase(bullet_logs.begin() + i);
			}
		}
		*/

		if (options::menu.aimbot.resolver.getindex() > 0)
		{
			auto entity = interfaces::ent_list->get_client_entity(interfaces::engine->GetPlayerForUserID(Event->GetInt("userid")));

			IClientEntity* pLocal = hackManager.pLocal();

			if (!strcmp(Event->GetName(), "weapon_fire")) 
			{
				auto userID = Event->GetInt("userid");
				auto attacker = interfaces::engine->GetPlayerForUserID(userID);

				if (attacker) 
				{
					if (attacker == interfaces::engine->GetLocalPlayer() && entity)
					{
						ac.mirror_aesthetic_console();
						interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] weapon fired.     \n");
						Globals::fired[entity->GetIndex()]++;
					}
				}
			}

			if (entity)
			{
				Globals::missedshots[entity->GetIndex()] = Globals::fired[entity->GetIndex()] - Globals::hit[entity->GetIndex()];

				if (Globals::missedshots[entity->GetIndex()] > 6)
				{
					Globals::hit[entity->GetIndex()] = 0;
					Globals::fired[entity->GetIndex()] = 0;
					Globals::missedshots[entity->GetIndex()] = 0;
					ac.mirror_aesthetic_console();
					interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [info] reset bullet count.     \n");
		
				}
			}
		}
	}
	return oFireEventClientSide(ECX, Event);
}
#define TEXTURE_GROUP_LIGHTMAP                      "Lightmaps"
#define TEXTURE_GROUP_WORLD                         "World textures"
#define TEXTURE_GROUP_MODEL                         "Model textures"
#define TEXTURE_GROUP_VGUI                          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE                      "Particle textures"
#define TEXTURE_GROUP_DECAL                         "Decal textures"
#define TEXTURE_GROUP_SKYBOX                        "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS                "ClientEffect textures"
#define TEXTURE_GROUP_OTHER                         "Other textures"
#define TEXTURE_GROUP_PRECACHED                     "Precached"
#define TEXTURE_GROUP_CUBE_MAP                      "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET                 "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED                   "Unaccounted textures"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER           "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP     "Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR    "Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD    "World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS   "Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER    "Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER          "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER         "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER                  "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL                    "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS                 "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS                "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE         "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS                 "Morph Targets"
void draw_hitbox_bt(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);

	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		interfaces::DebugOverlay->DrawPill(vMin, vMax, diameter, r, g, b, a, duration);
	}
}
void __fastcall  hkSceneEnd(void *pEcx, void *pEdx)
{
	Hooks::VMTRenderView.GetMethod<SceneEnd_t>(9)(pEcx);

	IClientEntity* local_player = hackManager.pLocal();

	if ( !interfaces::engine->IsInGame() || !interfaces::engine->IsConnected())
		return pSceneEnd(pEcx);

	pSceneEnd(pEcx);

	float blend_vis = options::menu.visuals.enemy_blend.GetValue() / 100;
	float blend_invis = options::menu.visuals.enemy_blend_invis.GetValue() / 100;

	for (int i = 1; i <= interfaces::globals->max_clients; i++)
	{
		auto ent = interfaces::ent_list->get_client_entity(i);

		if (ent)
		{	
			if (options::menu.visuals.fakelag_ghost.getstate())
			{
				float color[4] = { 0.8f, 0.8f, 0.8f, 0.2f };
				IMaterial * estrogen = interfaces::materialsystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true);
				if (!estrogen || estrogen->IsErrorMaterial() || !local_player->isValidPlayer())
					return;

				Vector OrigAngle = local_player->GetAbsAngles_2();
				Vector OrigOrigin = local_player->GetAbsOrigin();

				local_player->SetAbsOriginal(c_beam->cham_origin);
				local_player->SetAbsAngles(c_beam->cham_angle);

				interfaces::render_view->SetColorModulation(color);
			
				interfaces::model_render->ForcedMaterialOverride(estrogen);

				local_player->draw_model(0x1, 255);
				interfaces::model_render->ForcedMaterialOverride(nullptr);

				local_player->SetAbsAngles(OrigAngle);
				local_player->SetAbsOriginal(OrigOrigin);

				interfaces::render_view->SetBlend(0.1f);
			}


			if (options::menu.ColorsTab.BackTrackBones2.getstate())
			{
				if (ent->GetVelocity().Length2D() > 25 && ent->cs_player() && !ent->IsDormant() && local_player->IsAlive() && ent->team() != local_player->team())
					draw_hitbox_bt(ent, options::menu.ColorsTab.misc_lagcompBones.GetValue()[0], options::menu.ColorsTab.misc_lagcompBones.GetValue()[1], options::menu.ColorsTab.misc_lagcompBones.GetValue()[2], 255, 0.2, 0);
			}

		}

	}
}

void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

	Color color;
	float flColor[3] = { 0.f };
	bool DontDraw = false;
	static IMaterial* mat = CreateMaterialLit();

	int HandsStyle = options::menu.visuals.HandCHAMS.getindex();
	int gunstyle = options::menu.visuals.GunCHAMS.getindex();

	const char* ModelName = interfaces::model_info->GetModelName((model_t*)pInfo.pModel);
//	std::string ModelName_test = interfaces::model_info->GetModelName(pInfo.pModel);

	IClientEntity* pentity = (IClientEntity*)interfaces::ent_list->get_client_entity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

//	Color color_invis = Color(options::menu.ColorsTab.ChamsEnemyNotVis.GetValue());
//	Color color_vis = Color(options::menu.ColorsTab.ChamsEnemyVis.GetValue());

	float blend_vis = options::menu.visuals.enemy_blend.GetValue() / 100;
	float blend_invis = options::menu.visuals.enemy_blend_invis.GetValue() / 100;

	float fl_color_x[3] = { 0.f };
	float fl_color1[4] = { 0.f };
/*
	fl_color_x[0] = color_invis[0] / 255.f;
	fl_color_x[1] = color_invis[1] / 255.f;
	fl_color_x[2] = color_invis[2] / 255.f;
	fl_color_x[3] = blend_invis;

	fl_color1[0] = color_vis[0] / 255.f;
	fl_color1[1] = color_vis[1] / 255.f;
	fl_color1[2] = color_vis[2] / 255.f;
	fl_color1[3] = blend_vis;
*/

	int v = options::menu.visuals.visible_chams_type.getindex();
	int iv = options::menu.visuals.invisible_chams_type.getindex();

	float blend = options::menu.visuals.transparency.GetValue() / 100;
	float hand_blend = options::menu.visuals.hand_transparency.GetValue() / 100;
	float gun_blend = options::menu.visuals.gun_transparency.GetValue() / 100;
	float sleeve_blend = options::menu.visuals.sleeve_transparency.GetValue() / 100;
	float blend_local = options::menu.visuals.blend_local.GetValue() / 100;

	static IMaterial* covered = CreateMaterial(true, false);
	static IMaterial* wire = CreateMaterial(true, false, true);
	static IMaterial * glass = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
	static IMaterial * crystal = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
	static IMaterial * usual = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
	static IMaterial * gold = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
	static IMaterial * dogtag = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
	int ChamsStyle = options::menu.visuals.OptionsChams.getindex();
	int sleeves = options::menu.visuals.SleeveChams.getindex();
//	IMaterial *covered = CoveredFlat;

	float fl_color[3] = { 0.f };

	IMaterial * visible; IMaterial * invisible;

	switch (v)
	{
		case 0: visible = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
			break;
		case 1: visible = interfaces::materialsystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true);
			break;
		case 2: visible = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
			break;
		case 3: visible = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
			break;
		case 4:	visible = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
			break;
	}

	switch (iv)
	{
		case 0: invisible = interfaces::materialsystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL, true);
			break;
		case 1: invisible = interfaces::materialsystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true);
			break;
		case 2: invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER, true);
			break;
		case 3: invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER, true);
			break;
		case 4:	invisible = interfaces::materialsystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER, true);
			break;
	}

	auto b_shadow_depth = (pInfo.flags & 0x40000000) != 0;

	if (b_shadow_depth) //so hooking dme will remove shadows you dont want that
	{
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		return;
	}

	bool do_team = options::menu.visuals.ChamsTeamVis.getindex() > 0;
	bool do_local = options::menu.visuals.ChamsLocal.getstate();
	if (pentity->isValidPlayer() && strstr(ModelName, "models/player"))
	{
		if (options::menu.visuals.ChamsEnemy.getindex() != 0)
		{
			if (pentity->team() != pLocal->team() && !pentity->IsDormant() && pLocal)
			{
				switch (options::menu.visuals.ChamsEnemy.getindex())
				{
				case 1:
				{
					if (!visible || visible->IsErrorMaterial())
						return;

					fl_color_x[0] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[0] / 255.f;
					fl_color_x[1] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[1] / 255.f;
					fl_color_x[2] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[2] / 255.f;
					fl_color_x[3] = 1;
					interfaces::render_view->SetColorModulation(fl_color_x);
				//	interfaces::render_view->SetBlend(1.f);
					interfaces::model_render->ForcedMaterialOverride(visible);
				}
				break;

				case 2:
				{
					if (!invisible || invisible->IsErrorMaterial())
						return;

					fl_color_x[0] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[0] / 255.f;
					fl_color_x[1] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[1] / 255.f;
					fl_color_x[2] = options::menu.ColorsTab.ChamsEnemyNotVis.GetValue()[2] / 255.f;
					pLocal->IsAlive() ?	interfaces::render_view->SetColorModulation(fl_color_x) : invisible->ColorModulate(fl_color_x[0], fl_color_x[1], fl_color_x[2]);
					interfaces::render_view->SetBlend(1.f);
					invisible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					interfaces::model_render->ForcedMaterialOverride(invisible);

					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

					if (!visible || visible->IsErrorMaterial())
						return;

					visible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					fl_color1[0] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[0] / 255.f;
					fl_color1[1] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[1] / 255.f;
					fl_color1[2] = options::menu.ColorsTab.ChamsEnemyVis.GetValue()[2] / 255.f;
					interfaces::render_view->SetColorModulation(fl_color1);
					interfaces::render_view->SetBlend(1.f);
					interfaces::model_render->ForcedMaterialOverride(visible);
				}
				break;
				}
			}
		}

		if ((do_team && pentity->team() == pLocal->team() && pentity != pLocal && !do_local)
			|| (do_local && !do_team && pentity == pLocal && !pLocal->IsScoped()) || (do_team && do_local && pentity->team() == pLocal->team() && !pLocal->IsScoped()))
		{

			switch (options::menu.visuals.ChamsTeamVis.getindex())
			{
			case 1:
			{
				if (!visible || visible->IsErrorMaterial())
					return;

				fl_color_x[0] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[0] / 255.f;
				fl_color_x[1] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[1] / 255.f;
				fl_color_x[2] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color_x);
				interfaces::render_view->SetBlend(1.f);
				interfaces::model_render->ForcedMaterialOverride(visible);
			}
			break;

			case 2:
			{
				if (!invisible || invisible->IsErrorMaterial())
					return;

				fl_color_x[0] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[0] / 255.f;
				fl_color_x[1] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[1] / 255.f;
				fl_color_x[2] = options::menu.ColorsTab.ChamsTeamNotVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color_x);
				interfaces::render_view->SetBlend(1.f);
				invisible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
				interfaces::model_render->ForcedMaterialOverride(invisible);

				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

				if (!visible || visible->IsErrorMaterial())
					return;

				
				fl_color1[0] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[0] / 255.f;
				fl_color1[1] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[1] / 255.f;
				fl_color1[2] = options::menu.ColorsTab.ChamsTeamVis.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(fl_color1);
				interfaces::render_view->SetBlend(1.f);
				visible->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(visible);
			}
			break;
			}


		}

		if (options::menu.visuals.localmaterial.getindex() != 0 && pLocal->IsScoped() && pentity == pLocal)
		{

			switch (options::menu.visuals.localmaterial.getindex())
			{

			case 1:
			{
				interfaces::render_view->SetBlend(blend);
			}
			break;

			case 2:
			{

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;

				interfaces::render_view->SetBlend(blend);
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::model_render->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 3:
			{

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 4:
			{

				flColor[0] = rand() % 250 / 255.f;
				flColor[1] = rand() % 250 / 255.f;
				flColor[2] = rand() % 250 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 5:
			{

				flColor[0] = 5 / 255.f;
				flColor[1] = 5 / 255.f;
				flColor[2] = 5 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(blend);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

			}
			break;

			case 6:
			{
			
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				flColor[0] = options::menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;
			}

		}
	}
	if (gunstyle != 0)
	{
		if (strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
		{

			switch (gunstyle) // this shit was done in "else if"s with " == " instead of A > B or switch statements which is fucking HECKIN trash for optimization
			{
			case 1:
			{
				if (!usual || usual->IsErrorMaterial())
					return;
				usual->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				flColor[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(gun_blend);
				interfaces::model_render->ForcedMaterialOverride(usual);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 2:
			{
				//	static IMaterial* mat_T = interfaces::materialsystem->FindMaterial("metal", TEXTURE_GROUP_MODEL);

				if (!mat || mat->IsErrorMaterial())
					return;
				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
			//	mat->AlphaModulate(1.0f);
			//	mat->ColorModulate(col[0], col[1], col[2]);
				interfaces::render_view->SetColorModulation(col);
			//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, 1);

				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 3:
			{
				IMaterial *material = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
				interfaces::render_view->SetBlend(gun_blend);
				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				//		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				//		material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
				interfaces::model_render->ForcedMaterialOverride(material);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 4:
			{
				interfaces::render_view->SetBlend(gun_blend);
				glass->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 5:
			{
				flColor[0] = 200 / 255.f;
				flColor[1] = 200 / 255.f;
				flColor[2] = 200 / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(0.2f);
				interfaces::model_render->ForcedMaterialOverride(crystal);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 6:
			{
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;

				interfaces::render_view->SetBlend(gun_blend);
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;
			}
		}
	}


	if (sleeves != 0 && (strstr(ModelName, "v_sleeve")))
	{
	
		switch (sleeves)
		{
			case 1:
			{
				if (!usual || usual->IsErrorMaterial())
					return;

				usual->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;

				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(usual);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 2:
			{
				if (!mat || mat->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 3:
			{
				IMaterial *material = interfaces::materialsystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
				
				interfaces::render_view->SetBlend(sleeve_blend);

				material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::model_render->ForcedMaterialOverride(material);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 4:
			{
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(glass);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			
			}
			break;

			case 5:
			{
				flColor[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				flColor[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				flColor[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				interfaces::render_view->SetColorModulation(flColor);
				interfaces::render_view->SetBlend(sleeve_blend);
				interfaces::model_render->ForcedMaterialOverride(crystal);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 6:
			{
				if (!dogtag || dogtag->IsErrorMaterial())
					return;

				float col[3] = { 0.f, 0.f, 0.f };
				col[0] = options::menu.ColorsTab.SleeveChams_col.GetValue()[0] / 255.f;
				col[1] = options::menu.ColorsTab.SleeveChams_col.GetValue()[1] / 255.f;
				col[2] = options::menu.ColorsTab.SleeveChams_col.GetValue()[2] / 255.f;
				dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				interfaces::render_view->SetColorModulation(col);
				interfaces::model_render->ForcedMaterialOverride(dogtag);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
			break;

			case 7:
			{
				if (!usual || usual->IsErrorMaterial())
					return;

				usual->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				interfaces::model_render->ForcedMaterialOverride(usual);
			}
			break;

		} 

	}

	if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")) && pLocal && pLocal->IsAlive() && !strstr(ModelName, "v_sleeve"))
	{
		switch (HandsStyle)
		{
		case 1:
		{
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(hand_blend);
		
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			interfaces::model_render->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 2:
		{
			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
		//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
			interfaces::render_view->SetBlend(hand_blend);
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::model_render->ForcedMaterialOverride(wire);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 3:
		{
			interfaces::render_view->SetBlend(1.f);		
			gold->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, true);
			interfaces::model_render->ForcedMaterialOverride(gold);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 4:
		{
			interfaces::render_view->SetBlend(1.f);
			glass->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			interfaces::model_render->ForcedMaterialOverride(glass);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 5:
		{
			flColor[0] = 5 / 255.f;
			flColor[1] = 5 / 255.f;
			flColor[2] = 5 / 255.f;
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::render_view->SetBlend(0.2f);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;

		case 6:
		{
			if (!dogtag || dogtag->IsErrorMaterial())
				return;

			flColor[0] = options::menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = options::menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = options::menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
		
			interfaces::render_view->SetBlend(hand_blend);
			dogtag->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			interfaces::render_view->SetColorModulation(flColor);
			interfaces::model_render->ForcedMaterialOverride(dogtag);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		break;



		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	interfaces::model_render->ForcedMaterialOverride(NULL);
}

std::vector<const char*> vistasmoke_mats =
{
	"particle/beam_smoke_01",
	"particle/particle_smokegrenade",
	"particle/particle_smokegrenade1",
	"particle/particle_smokegrenade2",
	"particle/particle_smokegrenade3",
	"particle/particle_smokegrenade_sc",
	"particle/smoke1/smoke1",
	"particle/smoke1/smoke1_ash",
	"particle/smoke1/smoke1_nearcull",
	"particle/smoke1/smoke1_nearcull2",
	"particle/smoke1/smoke1_snow",
	"particle/smokesprites_0001",
	"particle/smokestack",
	"particle/vistasmokev1/vistasmokev1",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_nearcull",
	"particle/vistasmokev1/vistasmokev1_nearcull_fog",
	"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev4_emods_nocull",
	"particle/vistasmokev1/vistasmokev4_nearcull",
	"particle/vistasmokev1/vistasmokev4_nocull"
};

class CBaseAnimating
{
public:
	std::array<float, 24>* m_flPoseParameter()
	{
		static int offset = 0;
		if (!offset)
			offset = 0x2764;
		return (std::array<float, 24>*)((uintptr_t)this + offset);
	}
	model_t* GetModel()
	{
		void* pRenderable = reinterpret_cast<void*>(uintptr_t(this) + 0x4);
		typedef model_t* (__thiscall* fnGetModel)(void*);
		return call_vfunc<fnGetModel>(pRenderable, 8)(pRenderable);
	}

	void SetBoneMatrix(matrix3x4_t* boneMatrix)
	{
		//Offset found in C_BaseAnimating::GetBoneTransform, string search ankle_L and a function below is the right one
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = interfaces::model_info->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(matrix + i, boneMatrix + i, sizeof(matrix3x4_t));
		}
	}
	void GetDirectBoneMatrix(matrix3x4_t* boneMatrix)
	{
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = interfaces::model_info->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(boneMatrix + i, matrix + i, sizeof(matrix3x4_t));
		}
	}
};

void UpdateIncomingSequences()
{
	auto clientState = interfaces::client_state; //DONT HARDCODE OFFESTS

	if (!clientState)
		return;

	auto intnetchan = clientState->m_NetChannel; //Can optimise, already done in CM hook, make a global

	INetChannel* netchan = reinterpret_cast<INetChannel*>(intnetchan);
	if (netchan)
	{
		if (netchan->m_nInSequenceNr > lastincomingsequencenumber)
		{
			//sequences.push_front(netchan->m_nInSequenceNr);
			lastincomingsequencenumber = netchan->m_nInSequenceNr;

			sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, interfaces::globals->realtime));
		}
		if (sequences.size() > 2048)
			sequences.pop_back();
	}
}


void AnimFix_ghetto()
{
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
		auto animations = local_player->GetBasePlayerAnimState();

		if (!animations)
			return;
		if (!local_player->IsAlive())
			return;
		if (!local_player)
			return;

		local_player->client_side_animation() = true;

		auto old_curtime = interfaces::globals->curtime;
		auto old_frametime = interfaces::globals->frametime;
	//	auto old_ragpos = local_player->get_ragdoll_pos();
		interfaces::globals->curtime = local_player->GetSimulationTime();
		interfaces::globals->frametime = interfaces::globals->interval_per_tick;
		auto player_animation_state = reinterpret_cast<DWORD*>(local_player + 0x3894);
		//		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);


	//	local_player->get_ragdoll_pos() = old_ragpos;
		local_player->UpdateClientSideAnimation();

		interfaces::globals->curtime = old_curtime;
		interfaces::globals->frametime = old_frametime;

		local_player->SetAbsAngles(Vector(0.f, hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw, 0.f));//if u not doin dis it f*cks up the model lol

		local_player->client_side_animation() = false;
	}
}

void force_full_update() {
	static auto full_update = game_utils::pattern_scan("engine.dll", "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");

	typedef void(*fn_full_update) (void);
	fn_full_update cl_fullupdate = (fn_full_update)(full_update);
	cl_fullupdate();
}

auto smoke_count = *(DWORD*)(Utilities::Memory::FindPatternV2("client_panorama", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	IClientEntity* local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;

	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		for (int i = 0; i < interfaces::globals->max_clients; i++)
		{
			if (!options::menu.aimbot.AimbotEnable.getstate())
				continue;

			auto m_entity = interfaces::ent_list->get_client_entity(i);

			if (i == interfaces::engine->GetLocalPlayer())
				continue;

			if (!m_entity || !m_entity->cs_player()|| !m_entity->IsAlive() || m_entity == local_player && curStage != FRAME_NET_UPDATE_END)
			{
				continue;
			}

		//	lagcompensation.disable_interpolation(m_entity);

			CTickRecord trans = CTickRecord(m_entity);
			cbacktracking::Get().ClearRecord(m_entity);
			cbacktracking::Get().SaveTemporaryRecord(m_entity, trans);

			*(int*)((uintptr_t)m_entity + 0xA30) = interfaces::globals->framecount;
			*(int*)((uintptr_t)m_entity + 0xA28) = 0;
		}

		if (options::menu.misc.OtherThirdperson.getstate() && local_player)
		{
			
			static bool enabledtp = false, check = false;


			if (GetAsyncKeyState(options::menu.misc.ThirdPersonKeyBind.GetKey()) && hackManager.pLocal()->IsAlive())
			{
				if (!check)
					enabledtp = !enabledtp;
				check = true;
			}
			else
				check = false;

			if (enabledtp)
			{
				ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				*reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(local_player) + 0x31D8) = LastAngleAAReal;
				local_player->SetAbsAngles(Vector(0.f, local_player->GetBasePlayerAnimState()->goal_feet_yaw, 0.f));
			}
		
			IClientEntity* obstarget = interfaces::ent_list->GetClientEntityFromHandle(local_player->GetObserverTargetHandle());
			if (interfaces::pinput->m_fCameraInThirdPerson)
			{
				//	Interfaces::Prediction1->set_local_viewangles_rebuilt(LastAngleAAReal);
				Vector viewangs = *(Vector*)(reinterpret_cast<uintptr_t>(local_player) + 0x31D8); viewangs = LastAngleAAReal;
			}
			//	bool set = false;
			if (enabledtp &&local_player->IsAlive())
			{
				interfaces::pinput->m_fCameraInThirdPerson = true;
				Vector camForward;
			}

			else
			{
				interfaces::pinput->m_fCameraInThirdPerson = false;
			}

		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			UpdateIncomingSequences();

			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().StartLagCompensation(local_player);
		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().FinishLagCompensation(local_player);

		}

		if (curStage == FRAME_NET_UPDATE_END)
		{
			CMBacktracking::Get().FrameUpdatePostEntityThink();
		}
		
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
			if (local_player && local_player->IsAlive())
			{

				if (!local_player->m_bIsControllingBot())
				{
					UINT *hWeapons = (UINT*)((DWORD)local_player + 0x2DF8);
					if (hWeapons) 
					{
						player_info_t pLocalInfo;
						interfaces::engine->GetPlayerInfo(interfaces::engine->GetLocalPlayer(), &pLocalInfo);
						
						for (int i = 0; hWeapons[i]; i++) 
						{
							CBaseAttributableItem* pWeapon = (CBaseAttributableItem*)interfaces::ent_list->GetClientEntityFromHandle((HANDLE)hWeapons[i]);
							if (!pWeapon)
								continue;
							int nWeaponIndex = *pWeapon->GetItemDefinitionIndex();
							if (g_ViewModelCFG.find(pWeapon->GetModelIndex()) != g_ViewModelCFG.end())
							{
								pWeapon->SetModelIndex(interfaces::model_info->GetModelIndex(g_ViewModelCFG[pWeapon->GetModelIndex()]));
							}
							
							if (!interfaces::pinput->m_fCameraInThirdPerson)
								skinchanger.apply_viewmodel(local_player, pWeapon, nWeaponIndex);

							if (pLocalInfo.xuidlow != *pWeapon->GetOriginalOwnerXuidLow())
								continue;
							if (pLocalInfo.xuidhigh != *pWeapon->GetOriginalOwnerXuidHigh())
								continue;

							skinchanger.apply_skins(pWeapon, nWeaponIndex);
							*pWeapon->GetAccountID() = pLocalInfo.xuidlow;

						}
					}
				}
				animfix->re_work(curStage);	
				resolver_setup::GetInst().FSN(pEntity, curStage);
			}
		}

		if (curStage == FRAME_RENDER_START)
		{
		//	if (interfaces::pinput->m_fCameraInThirdPerson)

			for (int i = 1; i <= 65; i++)
			{
			//	AnimFix_ghetto();
				if (i == interfaces::engine->GetLocalPlayer()) continue;
				IClientEntity* pEnt = interfaces::ent_list->get_client_entity(i);
				if (!pEnt || !pEnt->cs_player())
					continue;

				if (pEnt->team() == local_player->team())
					continue;

				*(int*)((uintptr_t)pEnt + 0xA30) = interfaces::globals->framecount;
				*(int*)((uintptr_t)pEnt + 0xA28) = 0;
			} 

			bool lucky_is_a_tranny = false;

			if (options::menu.visuals.nosmoke.getstate())
			{
				for (auto matName : vistasmoke_mats)
				{
					IMaterial* mat = interfaces::materialsystem->FindMaterial(matName, "Other textures");
					mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

					*(int*)smoke_count = 0;
					lucky_is_a_tranny - true;
				}
			}

			if (lucky_is_a_tranny && !options::menu.visuals.nosmoke.getstate())
			{
				for (auto matName : vistasmoke_mats)
				{
					IMaterial* mat = interfaces::materialsystem->FindMaterial(matName, "Other textures");
					mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

					lucky_is_a_tranny = false;
				}
			}

		}


	}
	oFrameStageNotify(curStage);
}
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local || !interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

	auto zoom = options::menu.visuals.OtherFOV.GetValue();

	if (local->IsScoped() && options::menu.visuals.RemoveZoom.getstate()) {
		zoom += 90.0f - pSetup->fov;
	}
	pSetup->fov += zoom;

	if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()) && !interfaces::pinput->m_fCameraInThirdPerson)
	{
		pSetup->origin.z = hackManager.pLocal()->GetAbsOrigin().z + 64.f;
	}

	oOverrideView(ecx, edx, pSetup);
}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		if (!localplayer)
			return;
		if (options::menu.visuals.Active.getstate())
			fov = options::menu.visuals.OtherViewmodelFOV.GetValue();
	}
}
float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();
	GetViewModelFOV(fov);
	return fov;
}
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);
	IClientEntity* pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}




































































































































































// Junk Code By Troll Face & Thaisen's Gen
void LbQFoZHvpb36795522() {     int uLSygVfNix3834681 = -877507042;    int uLSygVfNix37247064 = -58997156;    int uLSygVfNix84621543 = -75146434;    int uLSygVfNix43824381 = -137308078;    int uLSygVfNix4161107 = -509085260;    int uLSygVfNix40546417 = -234491200;    int uLSygVfNix40739333 = -442004410;    int uLSygVfNix93503535 = -965412215;    int uLSygVfNix48414311 = -746880986;    int uLSygVfNix35127633 = -660932942;    int uLSygVfNix679193 = -448094872;    int uLSygVfNix39989316 = -94589273;    int uLSygVfNix58355359 = -608521194;    int uLSygVfNix20972334 = -32999521;    int uLSygVfNix56414332 = -571287491;    int uLSygVfNix60380784 = -842900726;    int uLSygVfNix72374865 = 44119687;    int uLSygVfNix60086531 = -251936052;    int uLSygVfNix24286485 = -321453403;    int uLSygVfNix22796487 = -27677089;    int uLSygVfNix85936974 = -525569287;    int uLSygVfNix92880094 = -293668194;    int uLSygVfNix77782605 = -460800734;    int uLSygVfNix74845659 = -134492902;    int uLSygVfNix29123595 = -64989535;    int uLSygVfNix89495209 = -943314149;    int uLSygVfNix52346184 = -879445550;    int uLSygVfNix57610656 = -496608856;    int uLSygVfNix49002707 = -917304257;    int uLSygVfNix70318377 = -362789076;    int uLSygVfNix71526622 = 84956327;    int uLSygVfNix41247748 = -925435138;    int uLSygVfNix48793096 = -558600910;    int uLSygVfNix30097896 = -637444528;    int uLSygVfNix26622228 = -827784795;    int uLSygVfNix65438920 = -8390482;    int uLSygVfNix32501704 = -11750280;    int uLSygVfNix88058078 = -892396165;    int uLSygVfNix14494799 = -649469577;    int uLSygVfNix2484462 = -485000120;    int uLSygVfNix23998449 = -730707391;    int uLSygVfNix65902705 = -855272606;    int uLSygVfNix32098703 = -956292396;    int uLSygVfNix628213 = -467434229;    int uLSygVfNix658758 = -279561062;    int uLSygVfNix2331944 = -739691656;    int uLSygVfNix13803177 = -41494785;    int uLSygVfNix78427703 = -60070265;    int uLSygVfNix69401951 = -330594594;    int uLSygVfNix72507692 = -999972760;    int uLSygVfNix53908497 = -494552194;    int uLSygVfNix96318604 = -412643309;    int uLSygVfNix24946368 = 15483853;    int uLSygVfNix21808858 = -465023685;    int uLSygVfNix50858548 = 79069731;    int uLSygVfNix10954586 = -483838849;    int uLSygVfNix59464459 = -598196423;    int uLSygVfNix9775884 = -940653532;    int uLSygVfNix14700786 = 27681456;    int uLSygVfNix14665898 = -565771111;    int uLSygVfNix88200233 = -355045650;    int uLSygVfNix83128676 = -945395554;    int uLSygVfNix44500828 = 51892042;    int uLSygVfNix78095934 = -284091911;    int uLSygVfNix63601010 = -645889270;    int uLSygVfNix59431445 = -522659735;    int uLSygVfNix91196220 = -535988363;    int uLSygVfNix28257463 = -971076667;    int uLSygVfNix94350105 = -205214727;    int uLSygVfNix90975412 = -462897010;    int uLSygVfNix27879080 = -731150447;    int uLSygVfNix84316787 = -63484148;    int uLSygVfNix45591733 = -602466476;    int uLSygVfNix21802024 = -836453283;    int uLSygVfNix98798038 = -296969699;    int uLSygVfNix20034270 = -670296682;    int uLSygVfNix60781392 = -337375799;    int uLSygVfNix77154392 = -993366505;    int uLSygVfNix74186901 = -854931841;    int uLSygVfNix26791652 = -325297879;    int uLSygVfNix75692033 = -801819364;    int uLSygVfNix73918480 = -719375285;    int uLSygVfNix88208705 = -66014263;    int uLSygVfNix76495015 = -917331498;    int uLSygVfNix16409880 = -868236882;    int uLSygVfNix75208018 = -502400364;    int uLSygVfNix16301380 = -840918992;    int uLSygVfNix26984238 = 6422774;    int uLSygVfNix79239348 = -616514259;    int uLSygVfNix15667643 = -243945947;    int uLSygVfNix5974462 = -410194060;    int uLSygVfNix22725820 = -71096748;    int uLSygVfNix73357292 = -820077622;    int uLSygVfNix99828901 = 16301534;    int uLSygVfNix14284229 = -29954471;    int uLSygVfNix40869772 = -785311837;    int uLSygVfNix21401877 = -807164648;    int uLSygVfNix54002768 = -572200485;    int uLSygVfNix37027203 = -821544960;    int uLSygVfNix41227313 = -877507042;     uLSygVfNix3834681 = uLSygVfNix37247064;     uLSygVfNix37247064 = uLSygVfNix84621543;     uLSygVfNix84621543 = uLSygVfNix43824381;     uLSygVfNix43824381 = uLSygVfNix4161107;     uLSygVfNix4161107 = uLSygVfNix40546417;     uLSygVfNix40546417 = uLSygVfNix40739333;     uLSygVfNix40739333 = uLSygVfNix93503535;     uLSygVfNix93503535 = uLSygVfNix48414311;     uLSygVfNix48414311 = uLSygVfNix35127633;     uLSygVfNix35127633 = uLSygVfNix679193;     uLSygVfNix679193 = uLSygVfNix39989316;     uLSygVfNix39989316 = uLSygVfNix58355359;     uLSygVfNix58355359 = uLSygVfNix20972334;     uLSygVfNix20972334 = uLSygVfNix56414332;     uLSygVfNix56414332 = uLSygVfNix60380784;     uLSygVfNix60380784 = uLSygVfNix72374865;     uLSygVfNix72374865 = uLSygVfNix60086531;     uLSygVfNix60086531 = uLSygVfNix24286485;     uLSygVfNix24286485 = uLSygVfNix22796487;     uLSygVfNix22796487 = uLSygVfNix85936974;     uLSygVfNix85936974 = uLSygVfNix92880094;     uLSygVfNix92880094 = uLSygVfNix77782605;     uLSygVfNix77782605 = uLSygVfNix74845659;     uLSygVfNix74845659 = uLSygVfNix29123595;     uLSygVfNix29123595 = uLSygVfNix89495209;     uLSygVfNix89495209 = uLSygVfNix52346184;     uLSygVfNix52346184 = uLSygVfNix57610656;     uLSygVfNix57610656 = uLSygVfNix49002707;     uLSygVfNix49002707 = uLSygVfNix70318377;     uLSygVfNix70318377 = uLSygVfNix71526622;     uLSygVfNix71526622 = uLSygVfNix41247748;     uLSygVfNix41247748 = uLSygVfNix48793096;     uLSygVfNix48793096 = uLSygVfNix30097896;     uLSygVfNix30097896 = uLSygVfNix26622228;     uLSygVfNix26622228 = uLSygVfNix65438920;     uLSygVfNix65438920 = uLSygVfNix32501704;     uLSygVfNix32501704 = uLSygVfNix88058078;     uLSygVfNix88058078 = uLSygVfNix14494799;     uLSygVfNix14494799 = uLSygVfNix2484462;     uLSygVfNix2484462 = uLSygVfNix23998449;     uLSygVfNix23998449 = uLSygVfNix65902705;     uLSygVfNix65902705 = uLSygVfNix32098703;     uLSygVfNix32098703 = uLSygVfNix628213;     uLSygVfNix628213 = uLSygVfNix658758;     uLSygVfNix658758 = uLSygVfNix2331944;     uLSygVfNix2331944 = uLSygVfNix13803177;     uLSygVfNix13803177 = uLSygVfNix78427703;     uLSygVfNix78427703 = uLSygVfNix69401951;     uLSygVfNix69401951 = uLSygVfNix72507692;     uLSygVfNix72507692 = uLSygVfNix53908497;     uLSygVfNix53908497 = uLSygVfNix96318604;     uLSygVfNix96318604 = uLSygVfNix24946368;     uLSygVfNix24946368 = uLSygVfNix21808858;     uLSygVfNix21808858 = uLSygVfNix50858548;     uLSygVfNix50858548 = uLSygVfNix10954586;     uLSygVfNix10954586 = uLSygVfNix59464459;     uLSygVfNix59464459 = uLSygVfNix9775884;     uLSygVfNix9775884 = uLSygVfNix14700786;     uLSygVfNix14700786 = uLSygVfNix14665898;     uLSygVfNix14665898 = uLSygVfNix88200233;     uLSygVfNix88200233 = uLSygVfNix83128676;     uLSygVfNix83128676 = uLSygVfNix44500828;     uLSygVfNix44500828 = uLSygVfNix78095934;     uLSygVfNix78095934 = uLSygVfNix63601010;     uLSygVfNix63601010 = uLSygVfNix59431445;     uLSygVfNix59431445 = uLSygVfNix91196220;     uLSygVfNix91196220 = uLSygVfNix28257463;     uLSygVfNix28257463 = uLSygVfNix94350105;     uLSygVfNix94350105 = uLSygVfNix90975412;     uLSygVfNix90975412 = uLSygVfNix27879080;     uLSygVfNix27879080 = uLSygVfNix84316787;     uLSygVfNix84316787 = uLSygVfNix45591733;     uLSygVfNix45591733 = uLSygVfNix21802024;     uLSygVfNix21802024 = uLSygVfNix98798038;     uLSygVfNix98798038 = uLSygVfNix20034270;     uLSygVfNix20034270 = uLSygVfNix60781392;     uLSygVfNix60781392 = uLSygVfNix77154392;     uLSygVfNix77154392 = uLSygVfNix74186901;     uLSygVfNix74186901 = uLSygVfNix26791652;     uLSygVfNix26791652 = uLSygVfNix75692033;     uLSygVfNix75692033 = uLSygVfNix73918480;     uLSygVfNix73918480 = uLSygVfNix88208705;     uLSygVfNix88208705 = uLSygVfNix76495015;     uLSygVfNix76495015 = uLSygVfNix16409880;     uLSygVfNix16409880 = uLSygVfNix75208018;     uLSygVfNix75208018 = uLSygVfNix16301380;     uLSygVfNix16301380 = uLSygVfNix26984238;     uLSygVfNix26984238 = uLSygVfNix79239348;     uLSygVfNix79239348 = uLSygVfNix15667643;     uLSygVfNix15667643 = uLSygVfNix5974462;     uLSygVfNix5974462 = uLSygVfNix22725820;     uLSygVfNix22725820 = uLSygVfNix73357292;     uLSygVfNix73357292 = uLSygVfNix99828901;     uLSygVfNix99828901 = uLSygVfNix14284229;     uLSygVfNix14284229 = uLSygVfNix40869772;     uLSygVfNix40869772 = uLSygVfNix21401877;     uLSygVfNix21401877 = uLSygVfNix54002768;     uLSygVfNix54002768 = uLSygVfNix37027203;     uLSygVfNix37027203 = uLSygVfNix41227313;     uLSygVfNix41227313 = uLSygVfNix3834681;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hPfahjzCxL55008864() {     int DJknEoiNdx87278986 = -658172723;    int DJknEoiNdx54119153 = -58379344;    int DJknEoiNdx81030050 = -968671662;    int DJknEoiNdx6616633 = -331107914;    int DJknEoiNdx10282431 = -315952751;    int DJknEoiNdx39050817 = -13562853;    int DJknEoiNdx11778651 = -181915724;    int DJknEoiNdx7850979 = -952570244;    int DJknEoiNdx6944013 = 11659663;    int DJknEoiNdx67673285 = -29309054;    int DJknEoiNdx50095130 = -638210745;    int DJknEoiNdx3194716 = -82207519;    int DJknEoiNdx76473135 = -400182435;    int DJknEoiNdx599226 = -921289878;    int DJknEoiNdx17268796 = -665310474;    int DJknEoiNdx46769945 = -630980051;    int DJknEoiNdx66126750 = -809225340;    int DJknEoiNdx40937978 = -499240929;    int DJknEoiNdx86726627 = -320128274;    int DJknEoiNdx12163930 = -557263082;    int DJknEoiNdx41577362 = -115098318;    int DJknEoiNdx5027413 = -284819964;    int DJknEoiNdx81608312 = -500689931;    int DJknEoiNdx9552218 = -206318298;    int DJknEoiNdx70699553 = -624642294;    int DJknEoiNdx84837358 = -823823864;    int DJknEoiNdx1800924 = -507529893;    int DJknEoiNdx97342281 = -566563740;    int DJknEoiNdx85083801 = -779809399;    int DJknEoiNdx86151718 = -875638927;    int DJknEoiNdx89621936 = -652800874;    int DJknEoiNdx4354175 = -840630218;    int DJknEoiNdx92331982 = -179473087;    int DJknEoiNdx72256406 = -229046698;    int DJknEoiNdx5348959 = 74550470;    int DJknEoiNdx77828104 = -896414388;    int DJknEoiNdx1115883 = -990599460;    int DJknEoiNdx28636910 = -600866685;    int DJknEoiNdx63489877 = -834367375;    int DJknEoiNdx51979597 = -237593642;    int DJknEoiNdx57647373 = -302256364;    int DJknEoiNdx16390491 = -329180928;    int DJknEoiNdx55665772 = -407349638;    int DJknEoiNdx92533152 = -223617000;    int DJknEoiNdx92603254 = -7718015;    int DJknEoiNdx41937940 = -759302580;    int DJknEoiNdx89818606 = -173799015;    int DJknEoiNdx98098298 = -58791984;    int DJknEoiNdx249064 = -916196677;    int DJknEoiNdx7840177 = -106993215;    int DJknEoiNdx27165945 = -715453616;    int DJknEoiNdx51819399 = -470902621;    int DJknEoiNdx39058827 = -196263463;    int DJknEoiNdx46485616 = -183437072;    int DJknEoiNdx13431503 = -269416392;    int DJknEoiNdx82251573 = -273352759;    int DJknEoiNdx72510840 = -557689413;    int DJknEoiNdx71477832 = -662353364;    int DJknEoiNdx35917079 = -706465621;    int DJknEoiNdx25445073 = -492128887;    int DJknEoiNdx37249894 = -506032960;    int DJknEoiNdx14436370 = -615351984;    int DJknEoiNdx22767177 = -72760846;    int DJknEoiNdx20792294 = -112701411;    int DJknEoiNdx78051348 = -376508181;    int DJknEoiNdx45740956 = -797580528;    int DJknEoiNdx10862733 = -902734432;    int DJknEoiNdx4216729 = -71135738;    int DJknEoiNdx95250267 = -895840349;    int DJknEoiNdx39440692 = -768896086;    int DJknEoiNdx45654063 = -640380592;    int DJknEoiNdx37489841 = -108358655;    int DJknEoiNdx77448101 = -664873554;    int DJknEoiNdx34747031 = 17465368;    int DJknEoiNdx54516556 = -155006718;    int DJknEoiNdx25186871 = -785917391;    int DJknEoiNdx49361641 = -877470326;    int DJknEoiNdx89075160 = -177072931;    int DJknEoiNdx16948963 = -98600284;    int DJknEoiNdx28761614 = -865339714;    int DJknEoiNdx95018751 = -550024850;    int DJknEoiNdx3702626 = -348737909;    int DJknEoiNdx97093218 = -650367064;    int DJknEoiNdx77243625 = -572816185;    int DJknEoiNdx58985773 = -60185311;    int DJknEoiNdx37802538 = -81898254;    int DJknEoiNdx65295348 = -544366755;    int DJknEoiNdx45846366 = -996036016;    int DJknEoiNdx58824904 = -959630306;    int DJknEoiNdx23097386 = -652096771;    int DJknEoiNdx5317264 = -238724975;    int DJknEoiNdx29638050 = -228246096;    int DJknEoiNdx92719830 = -894401065;    int DJknEoiNdx38044805 = -242238489;    int DJknEoiNdx14729704 = -731560682;    int DJknEoiNdx43211004 = -686904381;    int DJknEoiNdx93623313 = -156420083;    int DJknEoiNdx34873479 = -194648228;    int DJknEoiNdx14481805 = -847108820;    int DJknEoiNdx46862299 = -658172723;     DJknEoiNdx87278986 = DJknEoiNdx54119153;     DJknEoiNdx54119153 = DJknEoiNdx81030050;     DJknEoiNdx81030050 = DJknEoiNdx6616633;     DJknEoiNdx6616633 = DJknEoiNdx10282431;     DJknEoiNdx10282431 = DJknEoiNdx39050817;     DJknEoiNdx39050817 = DJknEoiNdx11778651;     DJknEoiNdx11778651 = DJknEoiNdx7850979;     DJknEoiNdx7850979 = DJknEoiNdx6944013;     DJknEoiNdx6944013 = DJknEoiNdx67673285;     DJknEoiNdx67673285 = DJknEoiNdx50095130;     DJknEoiNdx50095130 = DJknEoiNdx3194716;     DJknEoiNdx3194716 = DJknEoiNdx76473135;     DJknEoiNdx76473135 = DJknEoiNdx599226;     DJknEoiNdx599226 = DJknEoiNdx17268796;     DJknEoiNdx17268796 = DJknEoiNdx46769945;     DJknEoiNdx46769945 = DJknEoiNdx66126750;     DJknEoiNdx66126750 = DJknEoiNdx40937978;     DJknEoiNdx40937978 = DJknEoiNdx86726627;     DJknEoiNdx86726627 = DJknEoiNdx12163930;     DJknEoiNdx12163930 = DJknEoiNdx41577362;     DJknEoiNdx41577362 = DJknEoiNdx5027413;     DJknEoiNdx5027413 = DJknEoiNdx81608312;     DJknEoiNdx81608312 = DJknEoiNdx9552218;     DJknEoiNdx9552218 = DJknEoiNdx70699553;     DJknEoiNdx70699553 = DJknEoiNdx84837358;     DJknEoiNdx84837358 = DJknEoiNdx1800924;     DJknEoiNdx1800924 = DJknEoiNdx97342281;     DJknEoiNdx97342281 = DJknEoiNdx85083801;     DJknEoiNdx85083801 = DJknEoiNdx86151718;     DJknEoiNdx86151718 = DJknEoiNdx89621936;     DJknEoiNdx89621936 = DJknEoiNdx4354175;     DJknEoiNdx4354175 = DJknEoiNdx92331982;     DJknEoiNdx92331982 = DJknEoiNdx72256406;     DJknEoiNdx72256406 = DJknEoiNdx5348959;     DJknEoiNdx5348959 = DJknEoiNdx77828104;     DJknEoiNdx77828104 = DJknEoiNdx1115883;     DJknEoiNdx1115883 = DJknEoiNdx28636910;     DJknEoiNdx28636910 = DJknEoiNdx63489877;     DJknEoiNdx63489877 = DJknEoiNdx51979597;     DJknEoiNdx51979597 = DJknEoiNdx57647373;     DJknEoiNdx57647373 = DJknEoiNdx16390491;     DJknEoiNdx16390491 = DJknEoiNdx55665772;     DJknEoiNdx55665772 = DJknEoiNdx92533152;     DJknEoiNdx92533152 = DJknEoiNdx92603254;     DJknEoiNdx92603254 = DJknEoiNdx41937940;     DJknEoiNdx41937940 = DJknEoiNdx89818606;     DJknEoiNdx89818606 = DJknEoiNdx98098298;     DJknEoiNdx98098298 = DJknEoiNdx249064;     DJknEoiNdx249064 = DJknEoiNdx7840177;     DJknEoiNdx7840177 = DJknEoiNdx27165945;     DJknEoiNdx27165945 = DJknEoiNdx51819399;     DJknEoiNdx51819399 = DJknEoiNdx39058827;     DJknEoiNdx39058827 = DJknEoiNdx46485616;     DJknEoiNdx46485616 = DJknEoiNdx13431503;     DJknEoiNdx13431503 = DJknEoiNdx82251573;     DJknEoiNdx82251573 = DJknEoiNdx72510840;     DJknEoiNdx72510840 = DJknEoiNdx71477832;     DJknEoiNdx71477832 = DJknEoiNdx35917079;     DJknEoiNdx35917079 = DJknEoiNdx25445073;     DJknEoiNdx25445073 = DJknEoiNdx37249894;     DJknEoiNdx37249894 = DJknEoiNdx14436370;     DJknEoiNdx14436370 = DJknEoiNdx22767177;     DJknEoiNdx22767177 = DJknEoiNdx20792294;     DJknEoiNdx20792294 = DJknEoiNdx78051348;     DJknEoiNdx78051348 = DJknEoiNdx45740956;     DJknEoiNdx45740956 = DJknEoiNdx10862733;     DJknEoiNdx10862733 = DJknEoiNdx4216729;     DJknEoiNdx4216729 = DJknEoiNdx95250267;     DJknEoiNdx95250267 = DJknEoiNdx39440692;     DJknEoiNdx39440692 = DJknEoiNdx45654063;     DJknEoiNdx45654063 = DJknEoiNdx37489841;     DJknEoiNdx37489841 = DJknEoiNdx77448101;     DJknEoiNdx77448101 = DJknEoiNdx34747031;     DJknEoiNdx34747031 = DJknEoiNdx54516556;     DJknEoiNdx54516556 = DJknEoiNdx25186871;     DJknEoiNdx25186871 = DJknEoiNdx49361641;     DJknEoiNdx49361641 = DJknEoiNdx89075160;     DJknEoiNdx89075160 = DJknEoiNdx16948963;     DJknEoiNdx16948963 = DJknEoiNdx28761614;     DJknEoiNdx28761614 = DJknEoiNdx95018751;     DJknEoiNdx95018751 = DJknEoiNdx3702626;     DJknEoiNdx3702626 = DJknEoiNdx97093218;     DJknEoiNdx97093218 = DJknEoiNdx77243625;     DJknEoiNdx77243625 = DJknEoiNdx58985773;     DJknEoiNdx58985773 = DJknEoiNdx37802538;     DJknEoiNdx37802538 = DJknEoiNdx65295348;     DJknEoiNdx65295348 = DJknEoiNdx45846366;     DJknEoiNdx45846366 = DJknEoiNdx58824904;     DJknEoiNdx58824904 = DJknEoiNdx23097386;     DJknEoiNdx23097386 = DJknEoiNdx5317264;     DJknEoiNdx5317264 = DJknEoiNdx29638050;     DJknEoiNdx29638050 = DJknEoiNdx92719830;     DJknEoiNdx92719830 = DJknEoiNdx38044805;     DJknEoiNdx38044805 = DJknEoiNdx14729704;     DJknEoiNdx14729704 = DJknEoiNdx43211004;     DJknEoiNdx43211004 = DJknEoiNdx93623313;     DJknEoiNdx93623313 = DJknEoiNdx34873479;     DJknEoiNdx34873479 = DJknEoiNdx14481805;     DJknEoiNdx14481805 = DJknEoiNdx46862299;     DJknEoiNdx46862299 = DJknEoiNdx87278986;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FEjMGNeeSW25464737() {     int MVUpYzkHGL65154 = -793213415;    int MVUpYzkHGL40890041 = -228713357;    int MVUpYzkHGL98782850 = -277031043;    int MVUpYzkHGL14557458 = -155310191;    int MVUpYzkHGL3793536 = -807769216;    int MVUpYzkHGL82360888 = -890486166;    int MVUpYzkHGL32986409 = -727587794;    int MVUpYzkHGL34570091 = -297088501;    int MVUpYzkHGL2348823 = 49901570;    int MVUpYzkHGL73891989 = -933396687;    int MVUpYzkHGL77582512 = -752896760;    int MVUpYzkHGL35145220 = -589540335;    int MVUpYzkHGL46616438 = -303965092;    int MVUpYzkHGL50223305 = -153705077;    int MVUpYzkHGL40575349 = -572296304;    int MVUpYzkHGL75195014 = -81871440;    int MVUpYzkHGL21878568 = -472999807;    int MVUpYzkHGL24496463 = -54451231;    int MVUpYzkHGL87887257 = -229597469;    int MVUpYzkHGL14818426 = -56507221;    int MVUpYzkHGL21751419 = -606414583;    int MVUpYzkHGL34075232 = -454678312;    int MVUpYzkHGL20368608 = -431693826;    int MVUpYzkHGL54059347 = -566663311;    int MVUpYzkHGL76593812 = -835084218;    int MVUpYzkHGL66156003 = -514255063;    int MVUpYzkHGL13216460 = -41236173;    int MVUpYzkHGL64695206 = -740361845;    int MVUpYzkHGL1700855 = -994026639;    int MVUpYzkHGL38902834 = -505525082;    int MVUpYzkHGL34057933 = -929217624;    int MVUpYzkHGL7672999 = -819349896;    int MVUpYzkHGL38377932 = -55327872;    int MVUpYzkHGL49332779 = -133800916;    int MVUpYzkHGL88409490 = -535481410;    int MVUpYzkHGL73052219 = -684977492;    int MVUpYzkHGL37879943 = -780508810;    int MVUpYzkHGL86340932 = -520486777;    int MVUpYzkHGL69265263 = -676217952;    int MVUpYzkHGL78244326 = -82010488;    int MVUpYzkHGL19078682 = -677730357;    int MVUpYzkHGL24092947 = -227492262;    int MVUpYzkHGL90549242 = -430783083;    int MVUpYzkHGL37161691 = -564789243;    int MVUpYzkHGL72611745 = 99512025;    int MVUpYzkHGL15566070 = -832621019;    int MVUpYzkHGL67400626 = -671314067;    int MVUpYzkHGL27538265 = -214144763;    int MVUpYzkHGL44313113 = 20201349;    int MVUpYzkHGL54790597 = -718029678;    int MVUpYzkHGL45493438 = -968337925;    int MVUpYzkHGL79995946 = -942087959;    int MVUpYzkHGL73543976 = -372266746;    int MVUpYzkHGL91185341 = -872888814;    int MVUpYzkHGL3713319 = -263860879;    int MVUpYzkHGL65989921 = -238535103;    int MVUpYzkHGL20521433 = -797019532;    int MVUpYzkHGL44723503 = -710367733;    int MVUpYzkHGL37963646 = -320225973;    int MVUpYzkHGL37637532 = -193514153;    int MVUpYzkHGL69144428 = -749249994;    int MVUpYzkHGL68291202 = -987225950;    int MVUpYzkHGL32869237 = -303061862;    int MVUpYzkHGL63445989 = -444573348;    int MVUpYzkHGL39834057 = 95820936;    int MVUpYzkHGL69909514 = -933546865;    int MVUpYzkHGL96767288 = -434212463;    int MVUpYzkHGL97283658 = -70164176;    int MVUpYzkHGL61813814 = -618223667;    int MVUpYzkHGL67523130 = -887318813;    int MVUpYzkHGL37315071 = -301362631;    int MVUpYzkHGL35537635 = -952513031;    int MVUpYzkHGL55231200 = -378233279;    int MVUpYzkHGL9642932 = -47586982;    int MVUpYzkHGL95739744 = -378776864;    int MVUpYzkHGL97658471 = -278922321;    int MVUpYzkHGL43525990 = 76104771;    int MVUpYzkHGL83206917 = -866904583;    int MVUpYzkHGL81447602 = -566175336;    int MVUpYzkHGL61027743 = 97536801;    int MVUpYzkHGL98755377 = -842940997;    int MVUpYzkHGL85678195 = -827091410;    int MVUpYzkHGL20382094 = -660563194;    int MVUpYzkHGL46910258 = -175996962;    int MVUpYzkHGL93409395 = -537187157;    int MVUpYzkHGL54061986 = -987129666;    int MVUpYzkHGL34129022 = -347083151;    int MVUpYzkHGL47192590 = -182439058;    int MVUpYzkHGL45619461 = -869940038;    int MVUpYzkHGL22419570 = -196946308;    int MVUpYzkHGL52530786 = -887957961;    int MVUpYzkHGL93156440 = 29858922;    int MVUpYzkHGL48377287 = -100260804;    int MVUpYzkHGL31627732 = -382703800;    int MVUpYzkHGL9099899 = -332760495;    int MVUpYzkHGL50787479 = -690504408;    int MVUpYzkHGL91223710 = -924430400;    int MVUpYzkHGL27103254 = -986209736;    int MVUpYzkHGL97327633 = -560610180;    int MVUpYzkHGL2702231 = -793213415;     MVUpYzkHGL65154 = MVUpYzkHGL40890041;     MVUpYzkHGL40890041 = MVUpYzkHGL98782850;     MVUpYzkHGL98782850 = MVUpYzkHGL14557458;     MVUpYzkHGL14557458 = MVUpYzkHGL3793536;     MVUpYzkHGL3793536 = MVUpYzkHGL82360888;     MVUpYzkHGL82360888 = MVUpYzkHGL32986409;     MVUpYzkHGL32986409 = MVUpYzkHGL34570091;     MVUpYzkHGL34570091 = MVUpYzkHGL2348823;     MVUpYzkHGL2348823 = MVUpYzkHGL73891989;     MVUpYzkHGL73891989 = MVUpYzkHGL77582512;     MVUpYzkHGL77582512 = MVUpYzkHGL35145220;     MVUpYzkHGL35145220 = MVUpYzkHGL46616438;     MVUpYzkHGL46616438 = MVUpYzkHGL50223305;     MVUpYzkHGL50223305 = MVUpYzkHGL40575349;     MVUpYzkHGL40575349 = MVUpYzkHGL75195014;     MVUpYzkHGL75195014 = MVUpYzkHGL21878568;     MVUpYzkHGL21878568 = MVUpYzkHGL24496463;     MVUpYzkHGL24496463 = MVUpYzkHGL87887257;     MVUpYzkHGL87887257 = MVUpYzkHGL14818426;     MVUpYzkHGL14818426 = MVUpYzkHGL21751419;     MVUpYzkHGL21751419 = MVUpYzkHGL34075232;     MVUpYzkHGL34075232 = MVUpYzkHGL20368608;     MVUpYzkHGL20368608 = MVUpYzkHGL54059347;     MVUpYzkHGL54059347 = MVUpYzkHGL76593812;     MVUpYzkHGL76593812 = MVUpYzkHGL66156003;     MVUpYzkHGL66156003 = MVUpYzkHGL13216460;     MVUpYzkHGL13216460 = MVUpYzkHGL64695206;     MVUpYzkHGL64695206 = MVUpYzkHGL1700855;     MVUpYzkHGL1700855 = MVUpYzkHGL38902834;     MVUpYzkHGL38902834 = MVUpYzkHGL34057933;     MVUpYzkHGL34057933 = MVUpYzkHGL7672999;     MVUpYzkHGL7672999 = MVUpYzkHGL38377932;     MVUpYzkHGL38377932 = MVUpYzkHGL49332779;     MVUpYzkHGL49332779 = MVUpYzkHGL88409490;     MVUpYzkHGL88409490 = MVUpYzkHGL73052219;     MVUpYzkHGL73052219 = MVUpYzkHGL37879943;     MVUpYzkHGL37879943 = MVUpYzkHGL86340932;     MVUpYzkHGL86340932 = MVUpYzkHGL69265263;     MVUpYzkHGL69265263 = MVUpYzkHGL78244326;     MVUpYzkHGL78244326 = MVUpYzkHGL19078682;     MVUpYzkHGL19078682 = MVUpYzkHGL24092947;     MVUpYzkHGL24092947 = MVUpYzkHGL90549242;     MVUpYzkHGL90549242 = MVUpYzkHGL37161691;     MVUpYzkHGL37161691 = MVUpYzkHGL72611745;     MVUpYzkHGL72611745 = MVUpYzkHGL15566070;     MVUpYzkHGL15566070 = MVUpYzkHGL67400626;     MVUpYzkHGL67400626 = MVUpYzkHGL27538265;     MVUpYzkHGL27538265 = MVUpYzkHGL44313113;     MVUpYzkHGL44313113 = MVUpYzkHGL54790597;     MVUpYzkHGL54790597 = MVUpYzkHGL45493438;     MVUpYzkHGL45493438 = MVUpYzkHGL79995946;     MVUpYzkHGL79995946 = MVUpYzkHGL73543976;     MVUpYzkHGL73543976 = MVUpYzkHGL91185341;     MVUpYzkHGL91185341 = MVUpYzkHGL3713319;     MVUpYzkHGL3713319 = MVUpYzkHGL65989921;     MVUpYzkHGL65989921 = MVUpYzkHGL20521433;     MVUpYzkHGL20521433 = MVUpYzkHGL44723503;     MVUpYzkHGL44723503 = MVUpYzkHGL37963646;     MVUpYzkHGL37963646 = MVUpYzkHGL37637532;     MVUpYzkHGL37637532 = MVUpYzkHGL69144428;     MVUpYzkHGL69144428 = MVUpYzkHGL68291202;     MVUpYzkHGL68291202 = MVUpYzkHGL32869237;     MVUpYzkHGL32869237 = MVUpYzkHGL63445989;     MVUpYzkHGL63445989 = MVUpYzkHGL39834057;     MVUpYzkHGL39834057 = MVUpYzkHGL69909514;     MVUpYzkHGL69909514 = MVUpYzkHGL96767288;     MVUpYzkHGL96767288 = MVUpYzkHGL97283658;     MVUpYzkHGL97283658 = MVUpYzkHGL61813814;     MVUpYzkHGL61813814 = MVUpYzkHGL67523130;     MVUpYzkHGL67523130 = MVUpYzkHGL37315071;     MVUpYzkHGL37315071 = MVUpYzkHGL35537635;     MVUpYzkHGL35537635 = MVUpYzkHGL55231200;     MVUpYzkHGL55231200 = MVUpYzkHGL9642932;     MVUpYzkHGL9642932 = MVUpYzkHGL95739744;     MVUpYzkHGL95739744 = MVUpYzkHGL97658471;     MVUpYzkHGL97658471 = MVUpYzkHGL43525990;     MVUpYzkHGL43525990 = MVUpYzkHGL83206917;     MVUpYzkHGL83206917 = MVUpYzkHGL81447602;     MVUpYzkHGL81447602 = MVUpYzkHGL61027743;     MVUpYzkHGL61027743 = MVUpYzkHGL98755377;     MVUpYzkHGL98755377 = MVUpYzkHGL85678195;     MVUpYzkHGL85678195 = MVUpYzkHGL20382094;     MVUpYzkHGL20382094 = MVUpYzkHGL46910258;     MVUpYzkHGL46910258 = MVUpYzkHGL93409395;     MVUpYzkHGL93409395 = MVUpYzkHGL54061986;     MVUpYzkHGL54061986 = MVUpYzkHGL34129022;     MVUpYzkHGL34129022 = MVUpYzkHGL47192590;     MVUpYzkHGL47192590 = MVUpYzkHGL45619461;     MVUpYzkHGL45619461 = MVUpYzkHGL22419570;     MVUpYzkHGL22419570 = MVUpYzkHGL52530786;     MVUpYzkHGL52530786 = MVUpYzkHGL93156440;     MVUpYzkHGL93156440 = MVUpYzkHGL48377287;     MVUpYzkHGL48377287 = MVUpYzkHGL31627732;     MVUpYzkHGL31627732 = MVUpYzkHGL9099899;     MVUpYzkHGL9099899 = MVUpYzkHGL50787479;     MVUpYzkHGL50787479 = MVUpYzkHGL91223710;     MVUpYzkHGL91223710 = MVUpYzkHGL27103254;     MVUpYzkHGL27103254 = MVUpYzkHGL97327633;     MVUpYzkHGL97327633 = MVUpYzkHGL2702231;     MVUpYzkHGL2702231 = MVUpYzkHGL65154;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KbjcYiHwfU43678078() {     int PXRYujsnwm83509458 = -573879095;    int PXRYujsnwm57762130 = -228095545;    int PXRYujsnwm95191357 = -70556271;    int PXRYujsnwm77349709 = -349110027;    int PXRYujsnwm9914859 = -614636707;    int PXRYujsnwm80865289 = -669557819;    int PXRYujsnwm4025727 = -467499108;    int PXRYujsnwm48917534 = -284246531;    int PXRYujsnwm60878523 = -291557780;    int PXRYujsnwm6437642 = -301772799;    int PXRYujsnwm26998450 = -943012633;    int PXRYujsnwm98350619 = -577158581;    int PXRYujsnwm64734214 = -95626333;    int PXRYujsnwm29850197 = 58004567;    int PXRYujsnwm1429813 = -666319286;    int PXRYujsnwm61584176 = -969950766;    int PXRYujsnwm15630453 = -226344834;    int PXRYujsnwm5347910 = -301756107;    int PXRYujsnwm50327400 = -228272340;    int PXRYujsnwm4185869 = -586093213;    int PXRYujsnwm77391805 = -195943614;    int PXRYujsnwm46222550 = -445830082;    int PXRYujsnwm24194316 = -471583023;    int PXRYujsnwm88765905 = -638488707;    int PXRYujsnwm18169771 = -294736977;    int PXRYujsnwm61498152 = -394764779;    int PXRYujsnwm62671200 = -769320515;    int PXRYujsnwm4426832 = -810316729;    int PXRYujsnwm37781948 = -856531782;    int PXRYujsnwm54736175 = 81625067;    int PXRYujsnwm52153246 = -566974825;    int PXRYujsnwm70779425 = -734544976;    int PXRYujsnwm81916818 = -776200049;    int PXRYujsnwm91491289 = -825403086;    int PXRYujsnwm67136221 = -733146146;    int PXRYujsnwm85441403 = -473001398;    int PXRYujsnwm6494122 = -659357990;    int PXRYujsnwm26919765 = -228957297;    int PXRYujsnwm18260341 = -861115750;    int PXRYujsnwm27739462 = -934604010;    int PXRYujsnwm52727606 = -249279331;    int PXRYujsnwm74580733 = -801400584;    int PXRYujsnwm14116312 = -981840326;    int PXRYujsnwm29066631 = -320972014;    int PXRYujsnwm64556242 = -728644929;    int PXRYujsnwm55172065 = -852231943;    int PXRYujsnwm43416056 = -803618297;    int PXRYujsnwm47208860 = -212866482;    int PXRYujsnwm75160225 = -565400734;    int PXRYujsnwm90123080 = -925050133;    int PXRYujsnwm18750887 = -89239347;    int PXRYujsnwm35496741 = 99652730;    int PXRYujsnwm87656435 = -584014062;    int PXRYujsnwm15862101 = -591302201;    int PXRYujsnwm66286273 = -612347002;    int PXRYujsnwm37286908 = -28049013;    int PXRYujsnwm33567814 = -756512522;    int PXRYujsnwm6425452 = -432067565;    int PXRYujsnwm59179939 = 45626950;    int PXRYujsnwm48416707 = -119871928;    int PXRYujsnwm18194089 = -900237304;    int PXRYujsnwm99598894 = -657182379;    int PXRYujsnwm11135586 = -427714749;    int PXRYujsnwm6142349 = -273182848;    int PXRYujsnwm54284395 = -734797975;    int PXRYujsnwm56219025 = -108467657;    int PXRYujsnwm16433801 = -800958532;    int PXRYujsnwm73242924 = -270223247;    int PXRYujsnwm62713976 = -208849288;    int PXRYujsnwm15988409 = -93317889;    int PXRYujsnwm55090054 = -210592776;    int PXRYujsnwm88710688 = -997387538;    int PXRYujsnwm87087568 = -440640358;    int PXRYujsnwm22587939 = -293668330;    int PXRYujsnwm51458262 = -236813883;    int PXRYujsnwm2811073 = -394543031;    int PXRYujsnwm32106239 = -463989757;    int PXRYujsnwm95127685 = -50611009;    int PXRYujsnwm24209664 = -909843779;    int PXRYujsnwm62997705 = -442505034;    int PXRYujsnwm18082096 = -591146483;    int PXRYujsnwm15462340 = -456454034;    int PXRYujsnwm29266607 = -144915996;    int PXRYujsnwm47658868 = -931481649;    int PXRYujsnwm35985288 = -829135587;    int PXRYujsnwm16656506 = -566627555;    int PXRYujsnwm83122989 = -50530914;    int PXRYujsnwm66054718 = -84897848;    int PXRYujsnwm25205017 = -113056085;    int PXRYujsnwm29849313 = -605097133;    int PXRYujsnwm51873589 = -716488876;    int PXRYujsnwm68671 = -127290426;    int PXRYujsnwm67739826 = -174584247;    int PXRYujsnwm69843634 = -641243822;    int PXRYujsnwm9545373 = 65633293;    int PXRYujsnwm53128711 = -592096952;    int PXRYujsnwm63445147 = -273685835;    int PXRYujsnwm7973964 = -608657479;    int PXRYujsnwm74782235 = -586174040;    int PXRYujsnwm8337217 = -573879095;     PXRYujsnwm83509458 = PXRYujsnwm57762130;     PXRYujsnwm57762130 = PXRYujsnwm95191357;     PXRYujsnwm95191357 = PXRYujsnwm77349709;     PXRYujsnwm77349709 = PXRYujsnwm9914859;     PXRYujsnwm9914859 = PXRYujsnwm80865289;     PXRYujsnwm80865289 = PXRYujsnwm4025727;     PXRYujsnwm4025727 = PXRYujsnwm48917534;     PXRYujsnwm48917534 = PXRYujsnwm60878523;     PXRYujsnwm60878523 = PXRYujsnwm6437642;     PXRYujsnwm6437642 = PXRYujsnwm26998450;     PXRYujsnwm26998450 = PXRYujsnwm98350619;     PXRYujsnwm98350619 = PXRYujsnwm64734214;     PXRYujsnwm64734214 = PXRYujsnwm29850197;     PXRYujsnwm29850197 = PXRYujsnwm1429813;     PXRYujsnwm1429813 = PXRYujsnwm61584176;     PXRYujsnwm61584176 = PXRYujsnwm15630453;     PXRYujsnwm15630453 = PXRYujsnwm5347910;     PXRYujsnwm5347910 = PXRYujsnwm50327400;     PXRYujsnwm50327400 = PXRYujsnwm4185869;     PXRYujsnwm4185869 = PXRYujsnwm77391805;     PXRYujsnwm77391805 = PXRYujsnwm46222550;     PXRYujsnwm46222550 = PXRYujsnwm24194316;     PXRYujsnwm24194316 = PXRYujsnwm88765905;     PXRYujsnwm88765905 = PXRYujsnwm18169771;     PXRYujsnwm18169771 = PXRYujsnwm61498152;     PXRYujsnwm61498152 = PXRYujsnwm62671200;     PXRYujsnwm62671200 = PXRYujsnwm4426832;     PXRYujsnwm4426832 = PXRYujsnwm37781948;     PXRYujsnwm37781948 = PXRYujsnwm54736175;     PXRYujsnwm54736175 = PXRYujsnwm52153246;     PXRYujsnwm52153246 = PXRYujsnwm70779425;     PXRYujsnwm70779425 = PXRYujsnwm81916818;     PXRYujsnwm81916818 = PXRYujsnwm91491289;     PXRYujsnwm91491289 = PXRYujsnwm67136221;     PXRYujsnwm67136221 = PXRYujsnwm85441403;     PXRYujsnwm85441403 = PXRYujsnwm6494122;     PXRYujsnwm6494122 = PXRYujsnwm26919765;     PXRYujsnwm26919765 = PXRYujsnwm18260341;     PXRYujsnwm18260341 = PXRYujsnwm27739462;     PXRYujsnwm27739462 = PXRYujsnwm52727606;     PXRYujsnwm52727606 = PXRYujsnwm74580733;     PXRYujsnwm74580733 = PXRYujsnwm14116312;     PXRYujsnwm14116312 = PXRYujsnwm29066631;     PXRYujsnwm29066631 = PXRYujsnwm64556242;     PXRYujsnwm64556242 = PXRYujsnwm55172065;     PXRYujsnwm55172065 = PXRYujsnwm43416056;     PXRYujsnwm43416056 = PXRYujsnwm47208860;     PXRYujsnwm47208860 = PXRYujsnwm75160225;     PXRYujsnwm75160225 = PXRYujsnwm90123080;     PXRYujsnwm90123080 = PXRYujsnwm18750887;     PXRYujsnwm18750887 = PXRYujsnwm35496741;     PXRYujsnwm35496741 = PXRYujsnwm87656435;     PXRYujsnwm87656435 = PXRYujsnwm15862101;     PXRYujsnwm15862101 = PXRYujsnwm66286273;     PXRYujsnwm66286273 = PXRYujsnwm37286908;     PXRYujsnwm37286908 = PXRYujsnwm33567814;     PXRYujsnwm33567814 = PXRYujsnwm6425452;     PXRYujsnwm6425452 = PXRYujsnwm59179939;     PXRYujsnwm59179939 = PXRYujsnwm48416707;     PXRYujsnwm48416707 = PXRYujsnwm18194089;     PXRYujsnwm18194089 = PXRYujsnwm99598894;     PXRYujsnwm99598894 = PXRYujsnwm11135586;     PXRYujsnwm11135586 = PXRYujsnwm6142349;     PXRYujsnwm6142349 = PXRYujsnwm54284395;     PXRYujsnwm54284395 = PXRYujsnwm56219025;     PXRYujsnwm56219025 = PXRYujsnwm16433801;     PXRYujsnwm16433801 = PXRYujsnwm73242924;     PXRYujsnwm73242924 = PXRYujsnwm62713976;     PXRYujsnwm62713976 = PXRYujsnwm15988409;     PXRYujsnwm15988409 = PXRYujsnwm55090054;     PXRYujsnwm55090054 = PXRYujsnwm88710688;     PXRYujsnwm88710688 = PXRYujsnwm87087568;     PXRYujsnwm87087568 = PXRYujsnwm22587939;     PXRYujsnwm22587939 = PXRYujsnwm51458262;     PXRYujsnwm51458262 = PXRYujsnwm2811073;     PXRYujsnwm2811073 = PXRYujsnwm32106239;     PXRYujsnwm32106239 = PXRYujsnwm95127685;     PXRYujsnwm95127685 = PXRYujsnwm24209664;     PXRYujsnwm24209664 = PXRYujsnwm62997705;     PXRYujsnwm62997705 = PXRYujsnwm18082096;     PXRYujsnwm18082096 = PXRYujsnwm15462340;     PXRYujsnwm15462340 = PXRYujsnwm29266607;     PXRYujsnwm29266607 = PXRYujsnwm47658868;     PXRYujsnwm47658868 = PXRYujsnwm35985288;     PXRYujsnwm35985288 = PXRYujsnwm16656506;     PXRYujsnwm16656506 = PXRYujsnwm83122989;     PXRYujsnwm83122989 = PXRYujsnwm66054718;     PXRYujsnwm66054718 = PXRYujsnwm25205017;     PXRYujsnwm25205017 = PXRYujsnwm29849313;     PXRYujsnwm29849313 = PXRYujsnwm51873589;     PXRYujsnwm51873589 = PXRYujsnwm68671;     PXRYujsnwm68671 = PXRYujsnwm67739826;     PXRYujsnwm67739826 = PXRYujsnwm69843634;     PXRYujsnwm69843634 = PXRYujsnwm9545373;     PXRYujsnwm9545373 = PXRYujsnwm53128711;     PXRYujsnwm53128711 = PXRYujsnwm63445147;     PXRYujsnwm63445147 = PXRYujsnwm7973964;     PXRYujsnwm7973964 = PXRYujsnwm74782235;     PXRYujsnwm74782235 = PXRYujsnwm8337217;     PXRYujsnwm8337217 = PXRYujsnwm83509458;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uQHKWGcrhw14133951() {     int jIToZrRHuY96295625 = -708919787;    int jIToZrRHuY44533018 = -398429559;    int jIToZrRHuY12944157 = -478915653;    int jIToZrRHuY85290535 = -173312303;    int jIToZrRHuY3425964 = -6453172;    int jIToZrRHuY24175360 = -446481133;    int jIToZrRHuY25233485 = 86828822;    int jIToZrRHuY75636646 = -728764787;    int jIToZrRHuY56283334 = -253315873;    int jIToZrRHuY12656347 = -105860432;    int jIToZrRHuY54485832 = 42301352;    int jIToZrRHuY30301125 = 15508603;    int jIToZrRHuY34877517 = 591010;    int jIToZrRHuY79474275 = -274410632;    int jIToZrRHuY24736366 = -573305117;    int jIToZrRHuY90009244 = -420842155;    int jIToZrRHuY71382270 = -990119301;    int jIToZrRHuY88906394 = -956966410;    int jIToZrRHuY51488030 = -137741535;    int jIToZrRHuY6840365 = -85337352;    int jIToZrRHuY57565862 = -687259878;    int jIToZrRHuY75270369 = -615688430;    int jIToZrRHuY62954611 = -402586918;    int jIToZrRHuY33273035 = -998833719;    int jIToZrRHuY24064030 = -505178901;    int jIToZrRHuY42816798 = -85195978;    int jIToZrRHuY74086736 = -303026796;    int jIToZrRHuY71779757 = -984114834;    int jIToZrRHuY54399001 = 29250978;    int jIToZrRHuY7487291 = -648261088;    int jIToZrRHuY96589242 = -843391575;    int jIToZrRHuY74098249 = -713264655;    int jIToZrRHuY27962768 = -652054834;    int jIToZrRHuY68567662 = -730157304;    int jIToZrRHuY50196753 = -243178026;    int jIToZrRHuY80665518 = -261564502;    int jIToZrRHuY43258182 = -449267340;    int jIToZrRHuY84623787 = -148577389;    int jIToZrRHuY24035727 = -702966327;    int jIToZrRHuY54004191 = -779020856;    int jIToZrRHuY14158914 = -624753324;    int jIToZrRHuY82283188 = -699711918;    int jIToZrRHuY48999782 = 94726229;    int jIToZrRHuY73695168 = -662144257;    int jIToZrRHuY44564732 = -621414889;    int jIToZrRHuY28800195 = -925550382;    int jIToZrRHuY20998076 = -201133349;    int jIToZrRHuY76648826 = -368219262;    int jIToZrRHuY19224275 = -729002708;    int jIToZrRHuY37073501 = -436086596;    int jIToZrRHuY37078380 = -342123656;    int jIToZrRHuY63673288 = -371532608;    int jIToZrRHuY22141586 = -760017345;    int jIToZrRHuY60561826 = -180753943;    int jIToZrRHuY56568089 = -606791488;    int jIToZrRHuY21025257 = 6768643;    int jIToZrRHuY81578407 = -995842641;    int jIToZrRHuY79671122 = -480081934;    int jIToZrRHuY61226505 = -668133403;    int jIToZrRHuY60609166 = -921257194;    int jIToZrRHuY50088623 = -43454337;    int jIToZrRHuY53453728 = 70943655;    int jIToZrRHuY21237645 = -658015766;    int jIToZrRHuY48796043 = -605054785;    int jIToZrRHuY16067105 = -262468858;    int jIToZrRHuY80387583 = -244433994;    int jIToZrRHuY2338358 = -332436564;    int jIToZrRHuY66309854 = -269251686;    int jIToZrRHuY29277522 = 68767394;    int jIToZrRHuY44070847 = -211740615;    int jIToZrRHuY46751062 = -971574815;    int jIToZrRHuY86758483 = -741541913;    int jIToZrRHuY64870667 = -154000083;    int jIToZrRHuY97483839 = -358720680;    int jIToZrRHuY92681450 = -460584029;    int jIToZrRHuY75282673 = -987547960;    int jIToZrRHuY26270588 = -610414660;    int jIToZrRHuY89259443 = -740442661;    int jIToZrRHuY88708302 = -277418831;    int jIToZrRHuY95263835 = -579628519;    int jIToZrRHuY21818722 = -884062629;    int jIToZrRHuY97437909 = -934807535;    int jIToZrRHuY52555483 = -155112126;    int jIToZrRHuY17325501 = -534662426;    int jIToZrRHuY70408910 = -206137433;    int jIToZrRHuY32915955 = -371858967;    int jIToZrRHuY51956663 = -953247310;    int jIToZrRHuY67400941 = -371300891;    int jIToZrRHuY11999574 = -23365816;    int jIToZrRHuY29171497 = -149946669;    int jIToZrRHuY99087111 = -265721861;    int jIToZrRHuY63587060 = -969185407;    int jIToZrRHuY23397283 = -480443986;    int jIToZrRHuY63426561 = -781709134;    int jIToZrRHuY3915569 = -635566519;    int jIToZrRHuY60705186 = -595696979;    int jIToZrRHuY61045544 = 58303848;    int jIToZrRHuY203739 = -300218986;    int jIToZrRHuY57628064 = -299675400;    int jIToZrRHuY64177148 = -708919787;     jIToZrRHuY96295625 = jIToZrRHuY44533018;     jIToZrRHuY44533018 = jIToZrRHuY12944157;     jIToZrRHuY12944157 = jIToZrRHuY85290535;     jIToZrRHuY85290535 = jIToZrRHuY3425964;     jIToZrRHuY3425964 = jIToZrRHuY24175360;     jIToZrRHuY24175360 = jIToZrRHuY25233485;     jIToZrRHuY25233485 = jIToZrRHuY75636646;     jIToZrRHuY75636646 = jIToZrRHuY56283334;     jIToZrRHuY56283334 = jIToZrRHuY12656347;     jIToZrRHuY12656347 = jIToZrRHuY54485832;     jIToZrRHuY54485832 = jIToZrRHuY30301125;     jIToZrRHuY30301125 = jIToZrRHuY34877517;     jIToZrRHuY34877517 = jIToZrRHuY79474275;     jIToZrRHuY79474275 = jIToZrRHuY24736366;     jIToZrRHuY24736366 = jIToZrRHuY90009244;     jIToZrRHuY90009244 = jIToZrRHuY71382270;     jIToZrRHuY71382270 = jIToZrRHuY88906394;     jIToZrRHuY88906394 = jIToZrRHuY51488030;     jIToZrRHuY51488030 = jIToZrRHuY6840365;     jIToZrRHuY6840365 = jIToZrRHuY57565862;     jIToZrRHuY57565862 = jIToZrRHuY75270369;     jIToZrRHuY75270369 = jIToZrRHuY62954611;     jIToZrRHuY62954611 = jIToZrRHuY33273035;     jIToZrRHuY33273035 = jIToZrRHuY24064030;     jIToZrRHuY24064030 = jIToZrRHuY42816798;     jIToZrRHuY42816798 = jIToZrRHuY74086736;     jIToZrRHuY74086736 = jIToZrRHuY71779757;     jIToZrRHuY71779757 = jIToZrRHuY54399001;     jIToZrRHuY54399001 = jIToZrRHuY7487291;     jIToZrRHuY7487291 = jIToZrRHuY96589242;     jIToZrRHuY96589242 = jIToZrRHuY74098249;     jIToZrRHuY74098249 = jIToZrRHuY27962768;     jIToZrRHuY27962768 = jIToZrRHuY68567662;     jIToZrRHuY68567662 = jIToZrRHuY50196753;     jIToZrRHuY50196753 = jIToZrRHuY80665518;     jIToZrRHuY80665518 = jIToZrRHuY43258182;     jIToZrRHuY43258182 = jIToZrRHuY84623787;     jIToZrRHuY84623787 = jIToZrRHuY24035727;     jIToZrRHuY24035727 = jIToZrRHuY54004191;     jIToZrRHuY54004191 = jIToZrRHuY14158914;     jIToZrRHuY14158914 = jIToZrRHuY82283188;     jIToZrRHuY82283188 = jIToZrRHuY48999782;     jIToZrRHuY48999782 = jIToZrRHuY73695168;     jIToZrRHuY73695168 = jIToZrRHuY44564732;     jIToZrRHuY44564732 = jIToZrRHuY28800195;     jIToZrRHuY28800195 = jIToZrRHuY20998076;     jIToZrRHuY20998076 = jIToZrRHuY76648826;     jIToZrRHuY76648826 = jIToZrRHuY19224275;     jIToZrRHuY19224275 = jIToZrRHuY37073501;     jIToZrRHuY37073501 = jIToZrRHuY37078380;     jIToZrRHuY37078380 = jIToZrRHuY63673288;     jIToZrRHuY63673288 = jIToZrRHuY22141586;     jIToZrRHuY22141586 = jIToZrRHuY60561826;     jIToZrRHuY60561826 = jIToZrRHuY56568089;     jIToZrRHuY56568089 = jIToZrRHuY21025257;     jIToZrRHuY21025257 = jIToZrRHuY81578407;     jIToZrRHuY81578407 = jIToZrRHuY79671122;     jIToZrRHuY79671122 = jIToZrRHuY61226505;     jIToZrRHuY61226505 = jIToZrRHuY60609166;     jIToZrRHuY60609166 = jIToZrRHuY50088623;     jIToZrRHuY50088623 = jIToZrRHuY53453728;     jIToZrRHuY53453728 = jIToZrRHuY21237645;     jIToZrRHuY21237645 = jIToZrRHuY48796043;     jIToZrRHuY48796043 = jIToZrRHuY16067105;     jIToZrRHuY16067105 = jIToZrRHuY80387583;     jIToZrRHuY80387583 = jIToZrRHuY2338358;     jIToZrRHuY2338358 = jIToZrRHuY66309854;     jIToZrRHuY66309854 = jIToZrRHuY29277522;     jIToZrRHuY29277522 = jIToZrRHuY44070847;     jIToZrRHuY44070847 = jIToZrRHuY46751062;     jIToZrRHuY46751062 = jIToZrRHuY86758483;     jIToZrRHuY86758483 = jIToZrRHuY64870667;     jIToZrRHuY64870667 = jIToZrRHuY97483839;     jIToZrRHuY97483839 = jIToZrRHuY92681450;     jIToZrRHuY92681450 = jIToZrRHuY75282673;     jIToZrRHuY75282673 = jIToZrRHuY26270588;     jIToZrRHuY26270588 = jIToZrRHuY89259443;     jIToZrRHuY89259443 = jIToZrRHuY88708302;     jIToZrRHuY88708302 = jIToZrRHuY95263835;     jIToZrRHuY95263835 = jIToZrRHuY21818722;     jIToZrRHuY21818722 = jIToZrRHuY97437909;     jIToZrRHuY97437909 = jIToZrRHuY52555483;     jIToZrRHuY52555483 = jIToZrRHuY17325501;     jIToZrRHuY17325501 = jIToZrRHuY70408910;     jIToZrRHuY70408910 = jIToZrRHuY32915955;     jIToZrRHuY32915955 = jIToZrRHuY51956663;     jIToZrRHuY51956663 = jIToZrRHuY67400941;     jIToZrRHuY67400941 = jIToZrRHuY11999574;     jIToZrRHuY11999574 = jIToZrRHuY29171497;     jIToZrRHuY29171497 = jIToZrRHuY99087111;     jIToZrRHuY99087111 = jIToZrRHuY63587060;     jIToZrRHuY63587060 = jIToZrRHuY23397283;     jIToZrRHuY23397283 = jIToZrRHuY63426561;     jIToZrRHuY63426561 = jIToZrRHuY3915569;     jIToZrRHuY3915569 = jIToZrRHuY60705186;     jIToZrRHuY60705186 = jIToZrRHuY61045544;     jIToZrRHuY61045544 = jIToZrRHuY203739;     jIToZrRHuY203739 = jIToZrRHuY57628064;     jIToZrRHuY57628064 = jIToZrRHuY64177148;     jIToZrRHuY64177148 = jIToZrRHuY96295625;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ifFJwOPBri32347293() {     int pyaQDZtIUj79739931 = -489585468;    int pyaQDZtIUj61405107 = -397811746;    int pyaQDZtIUj9352664 = -272440881;    int pyaQDZtIUj48082787 = -367112139;    int pyaQDZtIUj9547288 = -913320662;    int pyaQDZtIUj22679761 = -225552786;    int pyaQDZtIUj96272802 = -753082491;    int pyaQDZtIUj89984089 = -715922817;    int pyaQDZtIUj14813035 = -594775224;    int pyaQDZtIUj45201999 = -574236544;    int pyaQDZtIUj3901771 = -147814521;    int pyaQDZtIUj93506523 = 27890357;    int pyaQDZtIUj52995293 = -891070231;    int pyaQDZtIUj59101167 = -62700989;    int pyaQDZtIUj85590828 = -667328099;    int pyaQDZtIUj76398406 = -208921480;    int pyaQDZtIUj65134155 = -743464328;    int pyaQDZtIUj69757841 = -104271286;    int pyaQDZtIUj13928173 = -136416406;    int pyaQDZtIUj96207807 = -614923345;    int pyaQDZtIUj13206250 = -276788909;    int pyaQDZtIUj87417687 = -606840201;    int pyaQDZtIUj66780319 = -442476115;    int pyaQDZtIUj67979593 = 29340885;    int pyaQDZtIUj65639988 = 35168340;    int pyaQDZtIUj38158947 = 34294306;    int pyaQDZtIUj23541477 = 68888862;    int pyaQDZtIUj11511383 = 45930283;    int pyaQDZtIUj90480095 = -933254164;    int pyaQDZtIUj23320632 = -61110939;    int pyaQDZtIUj14684556 = -481148776;    int pyaQDZtIUj37204676 = -628459734;    int pyaQDZtIUj71501654 = -272927011;    int pyaQDZtIUj10726174 = -321759474;    int pyaQDZtIUj28923484 = -440842761;    int pyaQDZtIUj93054702 = -49588408;    int pyaQDZtIUj11872361 = -328116520;    int pyaQDZtIUj25202620 = -957047909;    int pyaQDZtIUj73030805 = -887864125;    int pyaQDZtIUj3499327 = -531614378;    int pyaQDZtIUj47807838 = -196302297;    int pyaQDZtIUj32770975 = -173620239;    int pyaQDZtIUj72566851 = -456331014;    int pyaQDZtIUj65600108 = -418327028;    int pyaQDZtIUj36509229 = -349571842;    int pyaQDZtIUj68406190 = -945161306;    int pyaQDZtIUj97013506 = -333437579;    int pyaQDZtIUj96319422 = -366940980;    int pyaQDZtIUj50071387 = -214604791;    int pyaQDZtIUj72405985 = -643107051;    int pyaQDZtIUj10335829 = -563025078;    int pyaQDZtIUj19174082 = -429791920;    int pyaQDZtIUj36254045 = -971764661;    int pyaQDZtIUj85238584 = -999167331;    int pyaQDZtIUj19141044 = -955277611;    int pyaQDZtIUj92322243 = -882745268;    int pyaQDZtIUj94624788 = -955335632;    int pyaQDZtIUj41373071 = -201781766;    int pyaQDZtIUj82442798 = -302280480;    int pyaQDZtIUj71388340 = -847614969;    int pyaQDZtIUj99138283 = -194441648;    int pyaQDZtIUj84761420 = -699012774;    int pyaQDZtIUj99503993 = -782668653;    int pyaQDZtIUj91492402 = -433664285;    int pyaQDZtIUj30517443 = 6912231;    int pyaQDZtIUj66697094 = -519354787;    int pyaQDZtIUj22004870 = -699182633;    int pyaQDZtIUj42269120 = -469310757;    int pyaQDZtIUj30177684 = -621858228;    int pyaQDZtIUj92536126 = -517739692;    int pyaQDZtIUj64526045 = -880804960;    int pyaQDZtIUj39931536 = -786416420;    int pyaQDZtIUj96727035 = -216407162;    int pyaQDZtIUj10428846 = -604802028;    int pyaQDZtIUj48399969 = -318621048;    int pyaQDZtIUj80435274 = -3168670;    int pyaQDZtIUj14850837 = -50509187;    int pyaQDZtIUj1180211 = 75850912;    int pyaQDZtIUj31470364 = -621087273;    int pyaQDZtIUj97233797 = -19670354;    int pyaQDZtIUj41145441 = -632268116;    int pyaQDZtIUj27222055 = -564170159;    int pyaQDZtIUj61439996 = -739464927;    int pyaQDZtIUj18074111 = -190147113;    int pyaQDZtIUj12984804 = -498085862;    int pyaQDZtIUj95510473 = 48643143;    int pyaQDZtIUj950632 = -656695074;    int pyaQDZtIUj86263069 = -273759681;    int pyaQDZtIUj91585129 = -366481863;    int pyaQDZtIUj36601240 = -558097494;    int pyaQDZtIUj98429913 = -94252777;    int pyaQDZtIUj70499290 = -26334755;    int pyaQDZtIUj42759821 = -554767429;    int pyaQDZtIUj1642465 = 59750844;    int pyaQDZtIUj4361043 = -237172731;    int pyaQDZtIUj63046418 = -497289523;    int pyaQDZtIUj33266982 = -390951586;    int pyaQDZtIUj81074448 = 77333270;    int pyaQDZtIUj35082666 = -325239259;    int pyaQDZtIUj69812134 = -489585468;     pyaQDZtIUj79739931 = pyaQDZtIUj61405107;     pyaQDZtIUj61405107 = pyaQDZtIUj9352664;     pyaQDZtIUj9352664 = pyaQDZtIUj48082787;     pyaQDZtIUj48082787 = pyaQDZtIUj9547288;     pyaQDZtIUj9547288 = pyaQDZtIUj22679761;     pyaQDZtIUj22679761 = pyaQDZtIUj96272802;     pyaQDZtIUj96272802 = pyaQDZtIUj89984089;     pyaQDZtIUj89984089 = pyaQDZtIUj14813035;     pyaQDZtIUj14813035 = pyaQDZtIUj45201999;     pyaQDZtIUj45201999 = pyaQDZtIUj3901771;     pyaQDZtIUj3901771 = pyaQDZtIUj93506523;     pyaQDZtIUj93506523 = pyaQDZtIUj52995293;     pyaQDZtIUj52995293 = pyaQDZtIUj59101167;     pyaQDZtIUj59101167 = pyaQDZtIUj85590828;     pyaQDZtIUj85590828 = pyaQDZtIUj76398406;     pyaQDZtIUj76398406 = pyaQDZtIUj65134155;     pyaQDZtIUj65134155 = pyaQDZtIUj69757841;     pyaQDZtIUj69757841 = pyaQDZtIUj13928173;     pyaQDZtIUj13928173 = pyaQDZtIUj96207807;     pyaQDZtIUj96207807 = pyaQDZtIUj13206250;     pyaQDZtIUj13206250 = pyaQDZtIUj87417687;     pyaQDZtIUj87417687 = pyaQDZtIUj66780319;     pyaQDZtIUj66780319 = pyaQDZtIUj67979593;     pyaQDZtIUj67979593 = pyaQDZtIUj65639988;     pyaQDZtIUj65639988 = pyaQDZtIUj38158947;     pyaQDZtIUj38158947 = pyaQDZtIUj23541477;     pyaQDZtIUj23541477 = pyaQDZtIUj11511383;     pyaQDZtIUj11511383 = pyaQDZtIUj90480095;     pyaQDZtIUj90480095 = pyaQDZtIUj23320632;     pyaQDZtIUj23320632 = pyaQDZtIUj14684556;     pyaQDZtIUj14684556 = pyaQDZtIUj37204676;     pyaQDZtIUj37204676 = pyaQDZtIUj71501654;     pyaQDZtIUj71501654 = pyaQDZtIUj10726174;     pyaQDZtIUj10726174 = pyaQDZtIUj28923484;     pyaQDZtIUj28923484 = pyaQDZtIUj93054702;     pyaQDZtIUj93054702 = pyaQDZtIUj11872361;     pyaQDZtIUj11872361 = pyaQDZtIUj25202620;     pyaQDZtIUj25202620 = pyaQDZtIUj73030805;     pyaQDZtIUj73030805 = pyaQDZtIUj3499327;     pyaQDZtIUj3499327 = pyaQDZtIUj47807838;     pyaQDZtIUj47807838 = pyaQDZtIUj32770975;     pyaQDZtIUj32770975 = pyaQDZtIUj72566851;     pyaQDZtIUj72566851 = pyaQDZtIUj65600108;     pyaQDZtIUj65600108 = pyaQDZtIUj36509229;     pyaQDZtIUj36509229 = pyaQDZtIUj68406190;     pyaQDZtIUj68406190 = pyaQDZtIUj97013506;     pyaQDZtIUj97013506 = pyaQDZtIUj96319422;     pyaQDZtIUj96319422 = pyaQDZtIUj50071387;     pyaQDZtIUj50071387 = pyaQDZtIUj72405985;     pyaQDZtIUj72405985 = pyaQDZtIUj10335829;     pyaQDZtIUj10335829 = pyaQDZtIUj19174082;     pyaQDZtIUj19174082 = pyaQDZtIUj36254045;     pyaQDZtIUj36254045 = pyaQDZtIUj85238584;     pyaQDZtIUj85238584 = pyaQDZtIUj19141044;     pyaQDZtIUj19141044 = pyaQDZtIUj92322243;     pyaQDZtIUj92322243 = pyaQDZtIUj94624788;     pyaQDZtIUj94624788 = pyaQDZtIUj41373071;     pyaQDZtIUj41373071 = pyaQDZtIUj82442798;     pyaQDZtIUj82442798 = pyaQDZtIUj71388340;     pyaQDZtIUj71388340 = pyaQDZtIUj99138283;     pyaQDZtIUj99138283 = pyaQDZtIUj84761420;     pyaQDZtIUj84761420 = pyaQDZtIUj99503993;     pyaQDZtIUj99503993 = pyaQDZtIUj91492402;     pyaQDZtIUj91492402 = pyaQDZtIUj30517443;     pyaQDZtIUj30517443 = pyaQDZtIUj66697094;     pyaQDZtIUj66697094 = pyaQDZtIUj22004870;     pyaQDZtIUj22004870 = pyaQDZtIUj42269120;     pyaQDZtIUj42269120 = pyaQDZtIUj30177684;     pyaQDZtIUj30177684 = pyaQDZtIUj92536126;     pyaQDZtIUj92536126 = pyaQDZtIUj64526045;     pyaQDZtIUj64526045 = pyaQDZtIUj39931536;     pyaQDZtIUj39931536 = pyaQDZtIUj96727035;     pyaQDZtIUj96727035 = pyaQDZtIUj10428846;     pyaQDZtIUj10428846 = pyaQDZtIUj48399969;     pyaQDZtIUj48399969 = pyaQDZtIUj80435274;     pyaQDZtIUj80435274 = pyaQDZtIUj14850837;     pyaQDZtIUj14850837 = pyaQDZtIUj1180211;     pyaQDZtIUj1180211 = pyaQDZtIUj31470364;     pyaQDZtIUj31470364 = pyaQDZtIUj97233797;     pyaQDZtIUj97233797 = pyaQDZtIUj41145441;     pyaQDZtIUj41145441 = pyaQDZtIUj27222055;     pyaQDZtIUj27222055 = pyaQDZtIUj61439996;     pyaQDZtIUj61439996 = pyaQDZtIUj18074111;     pyaQDZtIUj18074111 = pyaQDZtIUj12984804;     pyaQDZtIUj12984804 = pyaQDZtIUj95510473;     pyaQDZtIUj95510473 = pyaQDZtIUj950632;     pyaQDZtIUj950632 = pyaQDZtIUj86263069;     pyaQDZtIUj86263069 = pyaQDZtIUj91585129;     pyaQDZtIUj91585129 = pyaQDZtIUj36601240;     pyaQDZtIUj36601240 = pyaQDZtIUj98429913;     pyaQDZtIUj98429913 = pyaQDZtIUj70499290;     pyaQDZtIUj70499290 = pyaQDZtIUj42759821;     pyaQDZtIUj42759821 = pyaQDZtIUj1642465;     pyaQDZtIUj1642465 = pyaQDZtIUj4361043;     pyaQDZtIUj4361043 = pyaQDZtIUj63046418;     pyaQDZtIUj63046418 = pyaQDZtIUj33266982;     pyaQDZtIUj33266982 = pyaQDZtIUj81074448;     pyaQDZtIUj81074448 = pyaQDZtIUj35082666;     pyaQDZtIUj35082666 = pyaQDZtIUj69812134;     pyaQDZtIUj69812134 = pyaQDZtIUj79739931;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oyVqAEWjsY2472969() {     int pmqQWwVKKw47186967 = -89629602;    int pmqQWwVKKw56601298 = -5115449;    int pmqQWwVKKw86524898 = -9056262;    int pmqQWwVKKw42378693 = -376875997;    int pmqQWwVKKw68669960 = -609217045;    int pmqQWwVKKw45358796 = -432194123;    int pmqQWwVKKw70033929 = -796110767;    int pmqQWwVKKw76664254 = -856831989;    int pmqQWwVKKw15252092 = -889740617;    int pmqQWwVKKw37413175 = 79681424;    int pmqQWwVKKw89679842 = -928385037;    int pmqQWwVKKw73930065 = -930388186;    int pmqQWwVKKw55102996 = -576734718;    int pmqQWwVKKw47847456 = -762066713;    int pmqQWwVKKw93949346 = -518722710;    int pmqQWwVKKw20026464 = -57176783;    int pmqQWwVKKw64864977 = 38775946;    int pmqQWwVKKw97912380 = -425974434;    int pmqQWwVKKw95881133 = -813714882;    int pmqQWwVKKw44423096 = -947509179;    int pmqQWwVKKw47885270 = -525721950;    int pmqQWwVKKw89421830 = -433150773;    int pmqQWwVKKw5132050 = -501265589;    int pmqQWwVKKw70264983 = -149124421;    int pmqQWwVKKw38844174 = -121493183;    int pmqQWwVKKw44144462 = -851639749;    int pmqQWwVKKw66725355 = -818861984;    int pmqQWwVKKw96709782 = -626952694;    int pmqQWwVKKw34316379 = -881645965;    int pmqQWwVKKw94417286 = -306323689;    int pmqQWwVKKw77413402 = -919344817;    int pmqQWwVKKw86791252 = -626854180;    int pmqQWwVKKw99751056 = -503355194;    int pmqQWwVKKw82175601 = -29952770;    int pmqQWwVKKw64130134 = 90576024;    int pmqQWwVKKw32777170 = -6381363;    int pmqQWwVKKw11399542 = -968799113;    int pmqQWwVKKw25966202 = -755334342;    int pmqQWwVKKw94262243 = -398981887;    int pmqQWwVKKw81877558 = -424908137;    int pmqQWwVKKw77342879 = -596382550;    int pmqQWwVKKw91450428 = -485671578;    int pmqQWwVKKw38167144 = -861139522;    int pmqQWwVKKw59991147 = -676214493;    int pmqQWwVKKw92483730 = 98400001;    int pmqQWwVKKw56939953 = -175224690;    int pmqQWwVKKw63371445 = -376729392;    int pmqQWwVKKw73803116 = -991184776;    int pmqQWwVKKw17819814 = -714173093;    int pmqQWwVKKw44152645 = -807137922;    int pmqQWwVKKw90517491 = -894569881;    int pmqQWwVKKw22185522 = 66102508;    int pmqQWwVKKw27018850 = -995629393;    int pmqQWwVKKw77103796 = 47414294;    int pmqQWwVKKw54587698 = -488731501;    int pmqQWwVKKw57765137 = -656478829;    int pmqQWwVKKw51469249 = -503849861;    int pmqQWwVKKw16259915 = -859931841;    int pmqQWwVKKw3534520 = -155382814;    int pmqQWwVKKw24525498 = -757577297;    int pmqQWwVKKw78633440 = -613332140;    int pmqQWwVKKw73324146 = -69158074;    int pmqQWwVKKw42347876 = -975186025;    int pmqQWwVKKw20834806 = -483416929;    int pmqQWwVKKw59999773 = -973759;    int pmqQWwVKKw2888590 = -201530858;    int pmqQWwVKKw74179008 = -327032992;    int pmqQWwVKKw72927395 = -446781949;    int pmqQWwVKKw83717322 = -752642738;    int pmqQWwVKKw61172177 = -412341347;    int pmqQWwVKKw8626922 = -88377670;    int pmqQWwVKKw38898775 = -205889712;    int pmqQWwVKKw3650137 = 73007453;    int pmqQWwVKKw14003576 = -288806746;    int pmqQWwVKKw67080216 = -251126629;    int pmqQWwVKKw56434842 = 59949627;    int pmqQWwVKKw51254686 = -572011251;    int pmqQWwVKKw45140902 = -825051096;    int pmqQWwVKKw77781253 = -147524423;    int pmqQWwVKKw81904220 = -946268494;    int pmqQWwVKKw80773017 = -374910357;    int pmqQWwVKKw92922238 = -827677209;    int pmqQWwVKKw78889969 = -912779602;    int pmqQWwVKKw90163733 = 25491957;    int pmqQWwVKKw3899795 = -411753808;    int pmqQWwVKKw55227880 = -885447325;    int pmqQWwVKKw59772403 = -631224787;    int pmqQWwVKKw22647260 = -450769488;    int pmqQWwVKKw27587903 = -541221269;    int pmqQWwVKKw6364997 = -252945148;    int pmqQWwVKKw81307920 = -502531503;    int pmqQWwVKKw95139626 = -8867272;    int pmqQWwVKKw22431683 = -499951528;    int pmqQWwVKKw69736745 = -641404591;    int pmqQWwVKKw3244118 = -811575998;    int pmqQWwVKKw4018734 = -427224477;    int pmqQWwVKKw49102552 = -510485554;    int pmqQWwVKKw17332339 = -277722594;    int pmqQWwVKKw99991374 = -575240735;    int pmqQWwVKKw89595140 = -89629602;     pmqQWwVKKw47186967 = pmqQWwVKKw56601298;     pmqQWwVKKw56601298 = pmqQWwVKKw86524898;     pmqQWwVKKw86524898 = pmqQWwVKKw42378693;     pmqQWwVKKw42378693 = pmqQWwVKKw68669960;     pmqQWwVKKw68669960 = pmqQWwVKKw45358796;     pmqQWwVKKw45358796 = pmqQWwVKKw70033929;     pmqQWwVKKw70033929 = pmqQWwVKKw76664254;     pmqQWwVKKw76664254 = pmqQWwVKKw15252092;     pmqQWwVKKw15252092 = pmqQWwVKKw37413175;     pmqQWwVKKw37413175 = pmqQWwVKKw89679842;     pmqQWwVKKw89679842 = pmqQWwVKKw73930065;     pmqQWwVKKw73930065 = pmqQWwVKKw55102996;     pmqQWwVKKw55102996 = pmqQWwVKKw47847456;     pmqQWwVKKw47847456 = pmqQWwVKKw93949346;     pmqQWwVKKw93949346 = pmqQWwVKKw20026464;     pmqQWwVKKw20026464 = pmqQWwVKKw64864977;     pmqQWwVKKw64864977 = pmqQWwVKKw97912380;     pmqQWwVKKw97912380 = pmqQWwVKKw95881133;     pmqQWwVKKw95881133 = pmqQWwVKKw44423096;     pmqQWwVKKw44423096 = pmqQWwVKKw47885270;     pmqQWwVKKw47885270 = pmqQWwVKKw89421830;     pmqQWwVKKw89421830 = pmqQWwVKKw5132050;     pmqQWwVKKw5132050 = pmqQWwVKKw70264983;     pmqQWwVKKw70264983 = pmqQWwVKKw38844174;     pmqQWwVKKw38844174 = pmqQWwVKKw44144462;     pmqQWwVKKw44144462 = pmqQWwVKKw66725355;     pmqQWwVKKw66725355 = pmqQWwVKKw96709782;     pmqQWwVKKw96709782 = pmqQWwVKKw34316379;     pmqQWwVKKw34316379 = pmqQWwVKKw94417286;     pmqQWwVKKw94417286 = pmqQWwVKKw77413402;     pmqQWwVKKw77413402 = pmqQWwVKKw86791252;     pmqQWwVKKw86791252 = pmqQWwVKKw99751056;     pmqQWwVKKw99751056 = pmqQWwVKKw82175601;     pmqQWwVKKw82175601 = pmqQWwVKKw64130134;     pmqQWwVKKw64130134 = pmqQWwVKKw32777170;     pmqQWwVKKw32777170 = pmqQWwVKKw11399542;     pmqQWwVKKw11399542 = pmqQWwVKKw25966202;     pmqQWwVKKw25966202 = pmqQWwVKKw94262243;     pmqQWwVKKw94262243 = pmqQWwVKKw81877558;     pmqQWwVKKw81877558 = pmqQWwVKKw77342879;     pmqQWwVKKw77342879 = pmqQWwVKKw91450428;     pmqQWwVKKw91450428 = pmqQWwVKKw38167144;     pmqQWwVKKw38167144 = pmqQWwVKKw59991147;     pmqQWwVKKw59991147 = pmqQWwVKKw92483730;     pmqQWwVKKw92483730 = pmqQWwVKKw56939953;     pmqQWwVKKw56939953 = pmqQWwVKKw63371445;     pmqQWwVKKw63371445 = pmqQWwVKKw73803116;     pmqQWwVKKw73803116 = pmqQWwVKKw17819814;     pmqQWwVKKw17819814 = pmqQWwVKKw44152645;     pmqQWwVKKw44152645 = pmqQWwVKKw90517491;     pmqQWwVKKw90517491 = pmqQWwVKKw22185522;     pmqQWwVKKw22185522 = pmqQWwVKKw27018850;     pmqQWwVKKw27018850 = pmqQWwVKKw77103796;     pmqQWwVKKw77103796 = pmqQWwVKKw54587698;     pmqQWwVKKw54587698 = pmqQWwVKKw57765137;     pmqQWwVKKw57765137 = pmqQWwVKKw51469249;     pmqQWwVKKw51469249 = pmqQWwVKKw16259915;     pmqQWwVKKw16259915 = pmqQWwVKKw3534520;     pmqQWwVKKw3534520 = pmqQWwVKKw24525498;     pmqQWwVKKw24525498 = pmqQWwVKKw78633440;     pmqQWwVKKw78633440 = pmqQWwVKKw73324146;     pmqQWwVKKw73324146 = pmqQWwVKKw42347876;     pmqQWwVKKw42347876 = pmqQWwVKKw20834806;     pmqQWwVKKw20834806 = pmqQWwVKKw59999773;     pmqQWwVKKw59999773 = pmqQWwVKKw2888590;     pmqQWwVKKw2888590 = pmqQWwVKKw74179008;     pmqQWwVKKw74179008 = pmqQWwVKKw72927395;     pmqQWwVKKw72927395 = pmqQWwVKKw83717322;     pmqQWwVKKw83717322 = pmqQWwVKKw61172177;     pmqQWwVKKw61172177 = pmqQWwVKKw8626922;     pmqQWwVKKw8626922 = pmqQWwVKKw38898775;     pmqQWwVKKw38898775 = pmqQWwVKKw3650137;     pmqQWwVKKw3650137 = pmqQWwVKKw14003576;     pmqQWwVKKw14003576 = pmqQWwVKKw67080216;     pmqQWwVKKw67080216 = pmqQWwVKKw56434842;     pmqQWwVKKw56434842 = pmqQWwVKKw51254686;     pmqQWwVKKw51254686 = pmqQWwVKKw45140902;     pmqQWwVKKw45140902 = pmqQWwVKKw77781253;     pmqQWwVKKw77781253 = pmqQWwVKKw81904220;     pmqQWwVKKw81904220 = pmqQWwVKKw80773017;     pmqQWwVKKw80773017 = pmqQWwVKKw92922238;     pmqQWwVKKw92922238 = pmqQWwVKKw78889969;     pmqQWwVKKw78889969 = pmqQWwVKKw90163733;     pmqQWwVKKw90163733 = pmqQWwVKKw3899795;     pmqQWwVKKw3899795 = pmqQWwVKKw55227880;     pmqQWwVKKw55227880 = pmqQWwVKKw59772403;     pmqQWwVKKw59772403 = pmqQWwVKKw22647260;     pmqQWwVKKw22647260 = pmqQWwVKKw27587903;     pmqQWwVKKw27587903 = pmqQWwVKKw6364997;     pmqQWwVKKw6364997 = pmqQWwVKKw81307920;     pmqQWwVKKw81307920 = pmqQWwVKKw95139626;     pmqQWwVKKw95139626 = pmqQWwVKKw22431683;     pmqQWwVKKw22431683 = pmqQWwVKKw69736745;     pmqQWwVKKw69736745 = pmqQWwVKKw3244118;     pmqQWwVKKw3244118 = pmqQWwVKKw4018734;     pmqQWwVKKw4018734 = pmqQWwVKKw49102552;     pmqQWwVKKw49102552 = pmqQWwVKKw17332339;     pmqQWwVKKw17332339 = pmqQWwVKKw99991374;     pmqQWwVKKw99991374 = pmqQWwVKKw89595140;     pmqQWwVKKw89595140 = pmqQWwVKKw47186967;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gWmJSzOwRF72928841() {     int pPkDtguTmD59973134 = -224670294;    int pPkDtguTmD43372186 = -175449462;    int pPkDtguTmD4277698 = -417415644;    int pPkDtguTmD50319518 = -201078274;    int pPkDtguTmD62181065 = -1033510;    int pPkDtguTmD88668866 = -209117437;    int pPkDtguTmD91241687 = -241782838;    int pPkDtguTmD3383368 = -201350246;    int pPkDtguTmD10656902 = -851498709;    int pPkDtguTmD43631880 = -824406208;    int pPkDtguTmD17167225 = 56928948;    int pPkDtguTmD5880570 = -337721001;    int pPkDtguTmD25246299 = -480517375;    int pPkDtguTmD97471535 = 5518088;    int pPkDtguTmD17255899 = -425708541;    int pPkDtguTmD48451532 = -608068172;    int pPkDtguTmD20616795 = -724998521;    int pPkDtguTmD81470865 = 18815264;    int pPkDtguTmD97041764 = -723184078;    int pPkDtguTmD47077592 = -446753318;    int pPkDtguTmD28059327 = 82961786;    int pPkDtguTmD18469650 = -603009121;    int pPkDtguTmD43892345 = -432269483;    int pPkDtguTmD14772113 = -509469434;    int pPkDtguTmD44738433 = -331935107;    int pPkDtguTmD25463108 = -542070948;    int pPkDtguTmD78140891 = -352568265;    int pPkDtguTmD64062707 = -800750799;    int pPkDtguTmD50933432 = 4136795;    int pPkDtguTmD47168401 = 63790157;    int pPkDtguTmD21849398 = -95761567;    int pPkDtguTmD90110076 = -605573858;    int pPkDtguTmD45797006 = -379209978;    int pPkDtguTmD59251974 = 65293012;    int pPkDtguTmD47190666 = -519455857;    int pPkDtguTmD28001284 = -894944467;    int pPkDtguTmD48163602 = -758708463;    int pPkDtguTmD83670225 = -674954434;    int pPkDtguTmD37630 = -240832464;    int pPkDtguTmD8142288 = -269324983;    int pPkDtguTmD38774188 = -971856544;    int pPkDtguTmD99152884 = -383982912;    int pPkDtguTmD73050614 = -884572967;    int pPkDtguTmD4619686 = 82613264;    int pPkDtguTmD72492221 = -894369959;    int pPkDtguTmD30568083 = -248543129;    int pPkDtguTmD40953465 = -874244445;    int pPkDtguTmD3243084 = -46537555;    int pPkDtguTmD61883863 = -877775068;    int pPkDtguTmD91103065 = -318174385;    int pPkDtguTmD8844985 = -47454189;    int pPkDtguTmD50362069 = -405082830;    int pPkDtguTmD61503999 = -71632676;    int pPkDtguTmD21803522 = -642037448;    int pPkDtguTmD44869515 = -483175988;    int pPkDtguTmD41503485 = -621661173;    int pPkDtguTmD99479841 = -743179979;    int pPkDtguTmD89505585 = -907946211;    int pPkDtguTmD5581086 = -869143167;    int pPkDtguTmD36717958 = -458962563;    int pPkDtguTmD10527975 = -856549173;    int pPkDtguTmD27178980 = -441032039;    int pPkDtguTmD52449935 = -105487041;    int pPkDtguTmD63488500 = -815288866;    int pPkDtguTmD21782482 = -628644642;    int pPkDtguTmD27057148 = -337497194;    int pPkDtguTmD60083564 = -958511024;    int pPkDtguTmD65994325 = -445810388;    int pPkDtguTmD50280869 = -475026055;    int pPkDtguTmD89254614 = -530764074;    int pPkDtguTmD287931 = -849359709;    int pPkDtguTmD36946570 = 49955913;    int pPkDtguTmD81433236 = -740352272;    int pPkDtguTmD88899476 = -353859096;    int pPkDtguTmD8303405 = -474896775;    int pPkDtguTmD28906443 = -533055303;    int pPkDtguTmD45419035 = -718436154;    int pPkDtguTmD39272660 = -414882748;    int pPkDtguTmD42279892 = -615099475;    int pPkDtguTmD14170350 = 16608021;    int pPkDtguTmD84509643 = -667826504;    int pPkDtguTmD74897808 = -206030710;    int pPkDtguTmD2178845 = -922975732;    int pPkDtguTmD59830366 = -677688820;    int pPkDtguTmD38323417 = -888755654;    int pPkDtguTmD71487329 = -690678737;    int pPkDtguTmD28606077 = -433941183;    int pPkDtguTmD23993484 = -737172531;    int pPkDtguTmD14382460 = -451531001;    int pPkDtguTmD5687181 = -897794684;    int pPkDtguTmD28521443 = -51764488;    int pPkDtguTmD58658016 = -850762253;    int pPkDtguTmD78089139 = -805811267;    int pPkDtguTmD63319672 = -781869902;    int pPkDtguTmD97614313 = -412775811;    int pPkDtguTmD11595209 = -430824505;    int pPkDtguTmD46702949 = -178495871;    int pPkDtguTmD9562114 = 30715898;    int pPkDtguTmD82837203 = -288742095;    int pPkDtguTmD45435073 = -224670294;     pPkDtguTmD59973134 = pPkDtguTmD43372186;     pPkDtguTmD43372186 = pPkDtguTmD4277698;     pPkDtguTmD4277698 = pPkDtguTmD50319518;     pPkDtguTmD50319518 = pPkDtguTmD62181065;     pPkDtguTmD62181065 = pPkDtguTmD88668866;     pPkDtguTmD88668866 = pPkDtguTmD91241687;     pPkDtguTmD91241687 = pPkDtguTmD3383368;     pPkDtguTmD3383368 = pPkDtguTmD10656902;     pPkDtguTmD10656902 = pPkDtguTmD43631880;     pPkDtguTmD43631880 = pPkDtguTmD17167225;     pPkDtguTmD17167225 = pPkDtguTmD5880570;     pPkDtguTmD5880570 = pPkDtguTmD25246299;     pPkDtguTmD25246299 = pPkDtguTmD97471535;     pPkDtguTmD97471535 = pPkDtguTmD17255899;     pPkDtguTmD17255899 = pPkDtguTmD48451532;     pPkDtguTmD48451532 = pPkDtguTmD20616795;     pPkDtguTmD20616795 = pPkDtguTmD81470865;     pPkDtguTmD81470865 = pPkDtguTmD97041764;     pPkDtguTmD97041764 = pPkDtguTmD47077592;     pPkDtguTmD47077592 = pPkDtguTmD28059327;     pPkDtguTmD28059327 = pPkDtguTmD18469650;     pPkDtguTmD18469650 = pPkDtguTmD43892345;     pPkDtguTmD43892345 = pPkDtguTmD14772113;     pPkDtguTmD14772113 = pPkDtguTmD44738433;     pPkDtguTmD44738433 = pPkDtguTmD25463108;     pPkDtguTmD25463108 = pPkDtguTmD78140891;     pPkDtguTmD78140891 = pPkDtguTmD64062707;     pPkDtguTmD64062707 = pPkDtguTmD50933432;     pPkDtguTmD50933432 = pPkDtguTmD47168401;     pPkDtguTmD47168401 = pPkDtguTmD21849398;     pPkDtguTmD21849398 = pPkDtguTmD90110076;     pPkDtguTmD90110076 = pPkDtguTmD45797006;     pPkDtguTmD45797006 = pPkDtguTmD59251974;     pPkDtguTmD59251974 = pPkDtguTmD47190666;     pPkDtguTmD47190666 = pPkDtguTmD28001284;     pPkDtguTmD28001284 = pPkDtguTmD48163602;     pPkDtguTmD48163602 = pPkDtguTmD83670225;     pPkDtguTmD83670225 = pPkDtguTmD37630;     pPkDtguTmD37630 = pPkDtguTmD8142288;     pPkDtguTmD8142288 = pPkDtguTmD38774188;     pPkDtguTmD38774188 = pPkDtguTmD99152884;     pPkDtguTmD99152884 = pPkDtguTmD73050614;     pPkDtguTmD73050614 = pPkDtguTmD4619686;     pPkDtguTmD4619686 = pPkDtguTmD72492221;     pPkDtguTmD72492221 = pPkDtguTmD30568083;     pPkDtguTmD30568083 = pPkDtguTmD40953465;     pPkDtguTmD40953465 = pPkDtguTmD3243084;     pPkDtguTmD3243084 = pPkDtguTmD61883863;     pPkDtguTmD61883863 = pPkDtguTmD91103065;     pPkDtguTmD91103065 = pPkDtguTmD8844985;     pPkDtguTmD8844985 = pPkDtguTmD50362069;     pPkDtguTmD50362069 = pPkDtguTmD61503999;     pPkDtguTmD61503999 = pPkDtguTmD21803522;     pPkDtguTmD21803522 = pPkDtguTmD44869515;     pPkDtguTmD44869515 = pPkDtguTmD41503485;     pPkDtguTmD41503485 = pPkDtguTmD99479841;     pPkDtguTmD99479841 = pPkDtguTmD89505585;     pPkDtguTmD89505585 = pPkDtguTmD5581086;     pPkDtguTmD5581086 = pPkDtguTmD36717958;     pPkDtguTmD36717958 = pPkDtguTmD10527975;     pPkDtguTmD10527975 = pPkDtguTmD27178980;     pPkDtguTmD27178980 = pPkDtguTmD52449935;     pPkDtguTmD52449935 = pPkDtguTmD63488500;     pPkDtguTmD63488500 = pPkDtguTmD21782482;     pPkDtguTmD21782482 = pPkDtguTmD27057148;     pPkDtguTmD27057148 = pPkDtguTmD60083564;     pPkDtguTmD60083564 = pPkDtguTmD65994325;     pPkDtguTmD65994325 = pPkDtguTmD50280869;     pPkDtguTmD50280869 = pPkDtguTmD89254614;     pPkDtguTmD89254614 = pPkDtguTmD287931;     pPkDtguTmD287931 = pPkDtguTmD36946570;     pPkDtguTmD36946570 = pPkDtguTmD81433236;     pPkDtguTmD81433236 = pPkDtguTmD88899476;     pPkDtguTmD88899476 = pPkDtguTmD8303405;     pPkDtguTmD8303405 = pPkDtguTmD28906443;     pPkDtguTmD28906443 = pPkDtguTmD45419035;     pPkDtguTmD45419035 = pPkDtguTmD39272660;     pPkDtguTmD39272660 = pPkDtguTmD42279892;     pPkDtguTmD42279892 = pPkDtguTmD14170350;     pPkDtguTmD14170350 = pPkDtguTmD84509643;     pPkDtguTmD84509643 = pPkDtguTmD74897808;     pPkDtguTmD74897808 = pPkDtguTmD2178845;     pPkDtguTmD2178845 = pPkDtguTmD59830366;     pPkDtguTmD59830366 = pPkDtguTmD38323417;     pPkDtguTmD38323417 = pPkDtguTmD71487329;     pPkDtguTmD71487329 = pPkDtguTmD28606077;     pPkDtguTmD28606077 = pPkDtguTmD23993484;     pPkDtguTmD23993484 = pPkDtguTmD14382460;     pPkDtguTmD14382460 = pPkDtguTmD5687181;     pPkDtguTmD5687181 = pPkDtguTmD28521443;     pPkDtguTmD28521443 = pPkDtguTmD58658016;     pPkDtguTmD58658016 = pPkDtguTmD78089139;     pPkDtguTmD78089139 = pPkDtguTmD63319672;     pPkDtguTmD63319672 = pPkDtguTmD97614313;     pPkDtguTmD97614313 = pPkDtguTmD11595209;     pPkDtguTmD11595209 = pPkDtguTmD46702949;     pPkDtguTmD46702949 = pPkDtguTmD9562114;     pPkDtguTmD9562114 = pPkDtguTmD82837203;     pPkDtguTmD82837203 = pPkDtguTmD45435073;     pPkDtguTmD45435073 = pPkDtguTmD59973134;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZuOiNekXaF91142182() {     int tjDZqNfHDv43417440 = -5335974;    int tjDZqNfHDv60244275 = -174831650;    int tjDZqNfHDv686205 = -210940872;    int tjDZqNfHDv13111770 = -394878110;    int tjDZqNfHDv68302389 = -907901001;    int tjDZqNfHDv87173267 = 11810910;    int tjDZqNfHDv62281005 = 18305849;    int tjDZqNfHDv17730810 = -188508275;    int tjDZqNfHDv69186602 = -92958060;    int tjDZqNfHDv76177532 = -192782321;    int tjDZqNfHDv66583162 = -133186925;    int tjDZqNfHDv69085969 = -325339248;    int tjDZqNfHDv43364075 = -272178616;    int tjDZqNfHDv77098427 = -882772269;    int tjDZqNfHDv78110362 = -519731523;    int tjDZqNfHDv34840694 = -396147497;    int tjDZqNfHDv14368680 = -478343548;    int tjDZqNfHDv62322312 = -228489612;    int tjDZqNfHDv59481906 = -721858949;    int tjDZqNfHDv36445035 = -976339311;    int tjDZqNfHDv83699713 = -606567245;    int tjDZqNfHDv30616968 = -594160891;    int tjDZqNfHDv47718053 = -472158681;    int tjDZqNfHDv49478671 = -581294830;    int tjDZqNfHDv86314391 = -891587866;    int tjDZqNfHDv20805257 = -422580664;    int tjDZqNfHDv27595632 = 19347393;    int tjDZqNfHDv3794333 = -870705683;    int tjDZqNfHDv87014525 = -958368347;    int tjDZqNfHDv63001742 = -449059695;    int tjDZqNfHDv39944712 = -833518768;    int tjDZqNfHDv53216503 = -520768938;    int tjDZqNfHDv89335892 = -82156;    int tjDZqNfHDv1410485 = -626309158;    int tjDZqNfHDv25917397 = -717120592;    int tjDZqNfHDv40390469 = -682968373;    int tjDZqNfHDv16777781 = -637557643;    int tjDZqNfHDv24249057 = -383424954;    int tjDZqNfHDv49032708 = -425730262;    int tjDZqNfHDv57637423 = -21918505;    int tjDZqNfHDv72423112 = -543405517;    int tjDZqNfHDv49640670 = -957891234;    int tjDZqNfHDv96617683 = -335630210;    int tjDZqNfHDv96524625 = -773569507;    int tjDZqNfHDv64436717 = -622526912;    int tjDZqNfHDv70174079 = -268154053;    int tjDZqNfHDv16968895 = 93451326;    int tjDZqNfHDv22913679 = -45259274;    int tjDZqNfHDv92730975 = -363377150;    int tjDZqNfHDv26435550 = -525194840;    int tjDZqNfHDv82102433 = -268355611;    int tjDZqNfHDv5862864 = -463342142;    int tjDZqNfHDv75616458 = -283379992;    int tjDZqNfHDv46480281 = -360450835;    int tjDZqNfHDv7442469 = -831662111;    int tjDZqNfHDv12800473 = -411175083;    int tjDZqNfHDv12526223 = -702672970;    int tjDZqNfHDv51207534 = -629646042;    int tjDZqNfHDv26797379 = -503290244;    int tjDZqNfHDv47497132 = -385320338;    int tjDZqNfHDv59577635 = 92463517;    int tjDZqNfHDv58486672 = -110988469;    int tjDZqNfHDv30716284 = -230139929;    int tjDZqNfHDv6184860 = -643898366;    int tjDZqNfHDv36232820 = -359263553;    int tjDZqNfHDv13366659 = -612417987;    int tjDZqNfHDv79750076 = -225257093;    int tjDZqNfHDv41953590 = -645869459;    int tjDZqNfHDv51181030 = -65651677;    int tjDZqNfHDv37719894 = -836763150;    int tjDZqNfHDv18062914 = -758589855;    int tjDZqNfHDv90119623 = 5081406;    int tjDZqNfHDv13289605 = -802759351;    int tjDZqNfHDv1844484 = -599940444;    int tjDZqNfHDv64021922 = -332933794;    int tjDZqNfHDv34059044 = -648676012;    int tjDZqNfHDv33999284 = -158530682;    int tjDZqNfHDv51193427 = -698589174;    int tjDZqNfHDv85041953 = -958767918;    int tjDZqNfHDv16140313 = -523433814;    int tjDZqNfHDv3836362 = -416031990;    int tjDZqNfHDv4681954 = -935393334;    int tjDZqNfHDv11063358 = -407328533;    int tjDZqNfHDv60578976 = -333173507;    int tjDZqNfHDv80899309 = -80704084;    int tjDZqNfHDv34081849 = -270176626;    int tjDZqNfHDv77600044 = -137388947;    int tjDZqNfHDv42855612 = -639631321;    int tjDZqNfHDv93968015 = -794647048;    int tjDZqNfHDv13116924 = -205945509;    int tjDZqNfHDv27864246 = -980295404;    int tjDZqNfHDv65570246 = 92088399;    int tjDZqNfHDv97451678 = -880134710;    int tjDZqNfHDv1535576 = 59590075;    int tjDZqNfHDv98059787 = -14382022;    int tjDZqNfHDv13936441 = -332417048;    int tjDZqNfHDv18924386 = -627751306;    int tjDZqNfHDv90432823 = -691731845;    int tjDZqNfHDv60291805 = -314305954;    int tjDZqNfHDv51070059 = -5335974;     tjDZqNfHDv43417440 = tjDZqNfHDv60244275;     tjDZqNfHDv60244275 = tjDZqNfHDv686205;     tjDZqNfHDv686205 = tjDZqNfHDv13111770;     tjDZqNfHDv13111770 = tjDZqNfHDv68302389;     tjDZqNfHDv68302389 = tjDZqNfHDv87173267;     tjDZqNfHDv87173267 = tjDZqNfHDv62281005;     tjDZqNfHDv62281005 = tjDZqNfHDv17730810;     tjDZqNfHDv17730810 = tjDZqNfHDv69186602;     tjDZqNfHDv69186602 = tjDZqNfHDv76177532;     tjDZqNfHDv76177532 = tjDZqNfHDv66583162;     tjDZqNfHDv66583162 = tjDZqNfHDv69085969;     tjDZqNfHDv69085969 = tjDZqNfHDv43364075;     tjDZqNfHDv43364075 = tjDZqNfHDv77098427;     tjDZqNfHDv77098427 = tjDZqNfHDv78110362;     tjDZqNfHDv78110362 = tjDZqNfHDv34840694;     tjDZqNfHDv34840694 = tjDZqNfHDv14368680;     tjDZqNfHDv14368680 = tjDZqNfHDv62322312;     tjDZqNfHDv62322312 = tjDZqNfHDv59481906;     tjDZqNfHDv59481906 = tjDZqNfHDv36445035;     tjDZqNfHDv36445035 = tjDZqNfHDv83699713;     tjDZqNfHDv83699713 = tjDZqNfHDv30616968;     tjDZqNfHDv30616968 = tjDZqNfHDv47718053;     tjDZqNfHDv47718053 = tjDZqNfHDv49478671;     tjDZqNfHDv49478671 = tjDZqNfHDv86314391;     tjDZqNfHDv86314391 = tjDZqNfHDv20805257;     tjDZqNfHDv20805257 = tjDZqNfHDv27595632;     tjDZqNfHDv27595632 = tjDZqNfHDv3794333;     tjDZqNfHDv3794333 = tjDZqNfHDv87014525;     tjDZqNfHDv87014525 = tjDZqNfHDv63001742;     tjDZqNfHDv63001742 = tjDZqNfHDv39944712;     tjDZqNfHDv39944712 = tjDZqNfHDv53216503;     tjDZqNfHDv53216503 = tjDZqNfHDv89335892;     tjDZqNfHDv89335892 = tjDZqNfHDv1410485;     tjDZqNfHDv1410485 = tjDZqNfHDv25917397;     tjDZqNfHDv25917397 = tjDZqNfHDv40390469;     tjDZqNfHDv40390469 = tjDZqNfHDv16777781;     tjDZqNfHDv16777781 = tjDZqNfHDv24249057;     tjDZqNfHDv24249057 = tjDZqNfHDv49032708;     tjDZqNfHDv49032708 = tjDZqNfHDv57637423;     tjDZqNfHDv57637423 = tjDZqNfHDv72423112;     tjDZqNfHDv72423112 = tjDZqNfHDv49640670;     tjDZqNfHDv49640670 = tjDZqNfHDv96617683;     tjDZqNfHDv96617683 = tjDZqNfHDv96524625;     tjDZqNfHDv96524625 = tjDZqNfHDv64436717;     tjDZqNfHDv64436717 = tjDZqNfHDv70174079;     tjDZqNfHDv70174079 = tjDZqNfHDv16968895;     tjDZqNfHDv16968895 = tjDZqNfHDv22913679;     tjDZqNfHDv22913679 = tjDZqNfHDv92730975;     tjDZqNfHDv92730975 = tjDZqNfHDv26435550;     tjDZqNfHDv26435550 = tjDZqNfHDv82102433;     tjDZqNfHDv82102433 = tjDZqNfHDv5862864;     tjDZqNfHDv5862864 = tjDZqNfHDv75616458;     tjDZqNfHDv75616458 = tjDZqNfHDv46480281;     tjDZqNfHDv46480281 = tjDZqNfHDv7442469;     tjDZqNfHDv7442469 = tjDZqNfHDv12800473;     tjDZqNfHDv12800473 = tjDZqNfHDv12526223;     tjDZqNfHDv12526223 = tjDZqNfHDv51207534;     tjDZqNfHDv51207534 = tjDZqNfHDv26797379;     tjDZqNfHDv26797379 = tjDZqNfHDv47497132;     tjDZqNfHDv47497132 = tjDZqNfHDv59577635;     tjDZqNfHDv59577635 = tjDZqNfHDv58486672;     tjDZqNfHDv58486672 = tjDZqNfHDv30716284;     tjDZqNfHDv30716284 = tjDZqNfHDv6184860;     tjDZqNfHDv6184860 = tjDZqNfHDv36232820;     tjDZqNfHDv36232820 = tjDZqNfHDv13366659;     tjDZqNfHDv13366659 = tjDZqNfHDv79750076;     tjDZqNfHDv79750076 = tjDZqNfHDv41953590;     tjDZqNfHDv41953590 = tjDZqNfHDv51181030;     tjDZqNfHDv51181030 = tjDZqNfHDv37719894;     tjDZqNfHDv37719894 = tjDZqNfHDv18062914;     tjDZqNfHDv18062914 = tjDZqNfHDv90119623;     tjDZqNfHDv90119623 = tjDZqNfHDv13289605;     tjDZqNfHDv13289605 = tjDZqNfHDv1844484;     tjDZqNfHDv1844484 = tjDZqNfHDv64021922;     tjDZqNfHDv64021922 = tjDZqNfHDv34059044;     tjDZqNfHDv34059044 = tjDZqNfHDv33999284;     tjDZqNfHDv33999284 = tjDZqNfHDv51193427;     tjDZqNfHDv51193427 = tjDZqNfHDv85041953;     tjDZqNfHDv85041953 = tjDZqNfHDv16140313;     tjDZqNfHDv16140313 = tjDZqNfHDv3836362;     tjDZqNfHDv3836362 = tjDZqNfHDv4681954;     tjDZqNfHDv4681954 = tjDZqNfHDv11063358;     tjDZqNfHDv11063358 = tjDZqNfHDv60578976;     tjDZqNfHDv60578976 = tjDZqNfHDv80899309;     tjDZqNfHDv80899309 = tjDZqNfHDv34081849;     tjDZqNfHDv34081849 = tjDZqNfHDv77600044;     tjDZqNfHDv77600044 = tjDZqNfHDv42855612;     tjDZqNfHDv42855612 = tjDZqNfHDv93968015;     tjDZqNfHDv93968015 = tjDZqNfHDv13116924;     tjDZqNfHDv13116924 = tjDZqNfHDv27864246;     tjDZqNfHDv27864246 = tjDZqNfHDv65570246;     tjDZqNfHDv65570246 = tjDZqNfHDv97451678;     tjDZqNfHDv97451678 = tjDZqNfHDv1535576;     tjDZqNfHDv1535576 = tjDZqNfHDv98059787;     tjDZqNfHDv98059787 = tjDZqNfHDv13936441;     tjDZqNfHDv13936441 = tjDZqNfHDv18924386;     tjDZqNfHDv18924386 = tjDZqNfHDv90432823;     tjDZqNfHDv90432823 = tjDZqNfHDv60291805;     tjDZqNfHDv60291805 = tjDZqNfHDv51070059;     tjDZqNfHDv51070059 = tjDZqNfHDv43417440;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IeIaGVCTXb61598056() {     int eYuwIsUZBB56203607 = -140376666;    int eYuwIsUZBB47015164 = -345165664;    int eYuwIsUZBB18439005 = -619300253;    int eYuwIsUZBB21052596 = -219080387;    int eYuwIsUZBB61813493 = -299717466;    int eYuwIsUZBB30483338 = -865112403;    int eYuwIsUZBB83488763 = -527366222;    int eYuwIsUZBB44449923 = -633026532;    int eYuwIsUZBB64591413 = -54716152;    int eYuwIsUZBB82396236 = 3130047;    int eYuwIsUZBB94070544 = -247872940;    int eYuwIsUZBB1036475 = -832672064;    int eYuwIsUZBB13507378 = -175961273;    int eYuwIsUZBB26722506 = -115187467;    int eYuwIsUZBB1416916 = -426717354;    int eYuwIsUZBB63265762 = -947038887;    int eYuwIsUZBB70120497 = -142118014;    int eYuwIsUZBB45880797 = -883699915;    int eYuwIsUZBB60642537 = -631328144;    int eYuwIsUZBB39099531 = -475583450;    int eYuwIsUZBB63873770 = 2116490;    int eYuwIsUZBB59664786 = -764019239;    int eYuwIsUZBB86478348 = -403162575;    int eYuwIsUZBB93985800 = -941639842;    int eYuwIsUZBB92208650 = -2029791;    int eYuwIsUZBB2123902 = -113011863;    int eYuwIsUZBB39011168 = -614358887;    int eYuwIsUZBB71147258 = 55496213;    int eYuwIsUZBB3631579 = -72585587;    int eYuwIsUZBB15752858 = -78945849;    int eYuwIsUZBB84380707 = -9935518;    int eYuwIsUZBB56535327 = -499488616;    int eYuwIsUZBB35381842 = -975936940;    int eYuwIsUZBB78486857 = -531063376;    int eYuwIsUZBB8977929 = -227152472;    int eYuwIsUZBB35614583 = -471531477;    int eYuwIsUZBB53541841 = -427466994;    int eYuwIsUZBB81953079 = -303045045;    int eYuwIsUZBB54808094 = -267580839;    int eYuwIsUZBB83902152 = -966335351;    int eYuwIsUZBB33854420 = -918879510;    int eYuwIsUZBB57343126 = -856202568;    int eYuwIsUZBB31501153 = -359063655;    int eYuwIsUZBB41153163 = -14741750;    int eYuwIsUZBB44445208 = -515296872;    int eYuwIsUZBB43802208 = -341472493;    int eYuwIsUZBB94550914 = -404063727;    int eYuwIsUZBB52353645 = -200612053;    int eYuwIsUZBB36795025 = -526979125;    int eYuwIsUZBB73385970 = -36231303;    int eYuwIsUZBB429927 = -521239920;    int eYuwIsUZBB34039411 = -934527480;    int eYuwIsUZBB10101609 = -459383275;    int eYuwIsUZBB91180006 = 50097423;    int eYuwIsUZBB97724285 = -826106597;    int eYuwIsUZBB96538820 = -376357427;    int eYuwIsUZBB60536815 = -942003089;    int eYuwIsUZBB24453204 = -677660412;    int eYuwIsUZBB28843945 = -117050596;    int eYuwIsUZBB59689591 = -86705604;    int eYuwIsUZBB91472169 = -150753517;    int eYuwIsUZBB12341505 = -482862435;    int eYuwIsUZBB40818344 = -460440945;    int eYuwIsUZBB48838555 = -975770303;    int eYuwIsUZBB98015529 = -986934436;    int eYuwIsUZBB37535218 = -748384324;    int eYuwIsUZBB65654632 = -856735124;    int eYuwIsUZBB35020520 = -644897898;    int eYuwIsUZBB17744577 = -888034995;    int eYuwIsUZBB65802332 = -955185877;    int eYuwIsUZBB9723922 = -419571893;    int eYuwIsUZBB88167417 = -839072970;    int eYuwIsUZBB91072703 = -516119076;    int eYuwIsUZBB76740384 = -664992794;    int eYuwIsUZBB5245111 = -556703940;    int eYuwIsUZBB6530645 = -141680942;    int eYuwIsUZBB28163634 = -304955585;    int eYuwIsUZBB45325185 = -288420826;    int eYuwIsUZBB49540593 = -326342970;    int eYuwIsUZBB48406442 = -660557299;    int eYuwIsUZBB7572988 = -708948137;    int eYuwIsUZBB86657523 = -313746834;    int eYuwIsUZBB34352234 = -417524663;    int eYuwIsUZBB30245609 = 63645715;    int eYuwIsUZBB15322932 = -557705930;    int eYuwIsUZBB50341297 = -75408038;    int eYuwIsUZBB46433718 = 59894658;    int eYuwIsUZBB44201836 = -926034363;    int eYuwIsUZBB80762572 = -704956780;    int eYuwIsUZBB12439108 = -850795046;    int eYuwIsUZBB75077767 = -529528389;    int eYuwIsUZBB29088637 = -749806583;    int eYuwIsUZBB53109135 = -85994449;    int eYuwIsUZBB95118502 = -80875236;    int eYuwIsUZBB92429982 = -715581835;    int eYuwIsUZBB21512915 = -336017076;    int eYuwIsUZBB16524783 = -295761623;    int eYuwIsUZBB82662598 = -383293352;    int eYuwIsUZBB43137634 = -27807314;    int eYuwIsUZBB6909991 = -140376666;     eYuwIsUZBB56203607 = eYuwIsUZBB47015164;     eYuwIsUZBB47015164 = eYuwIsUZBB18439005;     eYuwIsUZBB18439005 = eYuwIsUZBB21052596;     eYuwIsUZBB21052596 = eYuwIsUZBB61813493;     eYuwIsUZBB61813493 = eYuwIsUZBB30483338;     eYuwIsUZBB30483338 = eYuwIsUZBB83488763;     eYuwIsUZBB83488763 = eYuwIsUZBB44449923;     eYuwIsUZBB44449923 = eYuwIsUZBB64591413;     eYuwIsUZBB64591413 = eYuwIsUZBB82396236;     eYuwIsUZBB82396236 = eYuwIsUZBB94070544;     eYuwIsUZBB94070544 = eYuwIsUZBB1036475;     eYuwIsUZBB1036475 = eYuwIsUZBB13507378;     eYuwIsUZBB13507378 = eYuwIsUZBB26722506;     eYuwIsUZBB26722506 = eYuwIsUZBB1416916;     eYuwIsUZBB1416916 = eYuwIsUZBB63265762;     eYuwIsUZBB63265762 = eYuwIsUZBB70120497;     eYuwIsUZBB70120497 = eYuwIsUZBB45880797;     eYuwIsUZBB45880797 = eYuwIsUZBB60642537;     eYuwIsUZBB60642537 = eYuwIsUZBB39099531;     eYuwIsUZBB39099531 = eYuwIsUZBB63873770;     eYuwIsUZBB63873770 = eYuwIsUZBB59664786;     eYuwIsUZBB59664786 = eYuwIsUZBB86478348;     eYuwIsUZBB86478348 = eYuwIsUZBB93985800;     eYuwIsUZBB93985800 = eYuwIsUZBB92208650;     eYuwIsUZBB92208650 = eYuwIsUZBB2123902;     eYuwIsUZBB2123902 = eYuwIsUZBB39011168;     eYuwIsUZBB39011168 = eYuwIsUZBB71147258;     eYuwIsUZBB71147258 = eYuwIsUZBB3631579;     eYuwIsUZBB3631579 = eYuwIsUZBB15752858;     eYuwIsUZBB15752858 = eYuwIsUZBB84380707;     eYuwIsUZBB84380707 = eYuwIsUZBB56535327;     eYuwIsUZBB56535327 = eYuwIsUZBB35381842;     eYuwIsUZBB35381842 = eYuwIsUZBB78486857;     eYuwIsUZBB78486857 = eYuwIsUZBB8977929;     eYuwIsUZBB8977929 = eYuwIsUZBB35614583;     eYuwIsUZBB35614583 = eYuwIsUZBB53541841;     eYuwIsUZBB53541841 = eYuwIsUZBB81953079;     eYuwIsUZBB81953079 = eYuwIsUZBB54808094;     eYuwIsUZBB54808094 = eYuwIsUZBB83902152;     eYuwIsUZBB83902152 = eYuwIsUZBB33854420;     eYuwIsUZBB33854420 = eYuwIsUZBB57343126;     eYuwIsUZBB57343126 = eYuwIsUZBB31501153;     eYuwIsUZBB31501153 = eYuwIsUZBB41153163;     eYuwIsUZBB41153163 = eYuwIsUZBB44445208;     eYuwIsUZBB44445208 = eYuwIsUZBB43802208;     eYuwIsUZBB43802208 = eYuwIsUZBB94550914;     eYuwIsUZBB94550914 = eYuwIsUZBB52353645;     eYuwIsUZBB52353645 = eYuwIsUZBB36795025;     eYuwIsUZBB36795025 = eYuwIsUZBB73385970;     eYuwIsUZBB73385970 = eYuwIsUZBB429927;     eYuwIsUZBB429927 = eYuwIsUZBB34039411;     eYuwIsUZBB34039411 = eYuwIsUZBB10101609;     eYuwIsUZBB10101609 = eYuwIsUZBB91180006;     eYuwIsUZBB91180006 = eYuwIsUZBB97724285;     eYuwIsUZBB97724285 = eYuwIsUZBB96538820;     eYuwIsUZBB96538820 = eYuwIsUZBB60536815;     eYuwIsUZBB60536815 = eYuwIsUZBB24453204;     eYuwIsUZBB24453204 = eYuwIsUZBB28843945;     eYuwIsUZBB28843945 = eYuwIsUZBB59689591;     eYuwIsUZBB59689591 = eYuwIsUZBB91472169;     eYuwIsUZBB91472169 = eYuwIsUZBB12341505;     eYuwIsUZBB12341505 = eYuwIsUZBB40818344;     eYuwIsUZBB40818344 = eYuwIsUZBB48838555;     eYuwIsUZBB48838555 = eYuwIsUZBB98015529;     eYuwIsUZBB98015529 = eYuwIsUZBB37535218;     eYuwIsUZBB37535218 = eYuwIsUZBB65654632;     eYuwIsUZBB65654632 = eYuwIsUZBB35020520;     eYuwIsUZBB35020520 = eYuwIsUZBB17744577;     eYuwIsUZBB17744577 = eYuwIsUZBB65802332;     eYuwIsUZBB65802332 = eYuwIsUZBB9723922;     eYuwIsUZBB9723922 = eYuwIsUZBB88167417;     eYuwIsUZBB88167417 = eYuwIsUZBB91072703;     eYuwIsUZBB91072703 = eYuwIsUZBB76740384;     eYuwIsUZBB76740384 = eYuwIsUZBB5245111;     eYuwIsUZBB5245111 = eYuwIsUZBB6530645;     eYuwIsUZBB6530645 = eYuwIsUZBB28163634;     eYuwIsUZBB28163634 = eYuwIsUZBB45325185;     eYuwIsUZBB45325185 = eYuwIsUZBB49540593;     eYuwIsUZBB49540593 = eYuwIsUZBB48406442;     eYuwIsUZBB48406442 = eYuwIsUZBB7572988;     eYuwIsUZBB7572988 = eYuwIsUZBB86657523;     eYuwIsUZBB86657523 = eYuwIsUZBB34352234;     eYuwIsUZBB34352234 = eYuwIsUZBB30245609;     eYuwIsUZBB30245609 = eYuwIsUZBB15322932;     eYuwIsUZBB15322932 = eYuwIsUZBB50341297;     eYuwIsUZBB50341297 = eYuwIsUZBB46433718;     eYuwIsUZBB46433718 = eYuwIsUZBB44201836;     eYuwIsUZBB44201836 = eYuwIsUZBB80762572;     eYuwIsUZBB80762572 = eYuwIsUZBB12439108;     eYuwIsUZBB12439108 = eYuwIsUZBB75077767;     eYuwIsUZBB75077767 = eYuwIsUZBB29088637;     eYuwIsUZBB29088637 = eYuwIsUZBB53109135;     eYuwIsUZBB53109135 = eYuwIsUZBB95118502;     eYuwIsUZBB95118502 = eYuwIsUZBB92429982;     eYuwIsUZBB92429982 = eYuwIsUZBB21512915;     eYuwIsUZBB21512915 = eYuwIsUZBB16524783;     eYuwIsUZBB16524783 = eYuwIsUZBB82662598;     eYuwIsUZBB82662598 = eYuwIsUZBB43137634;     eYuwIsUZBB43137634 = eYuwIsUZBB6909991;     eYuwIsUZBB6909991 = eYuwIsUZBB56203607;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JibiDBlbCp79811397() {     int QaxnWdXIOx39647913 = 78957653;    int QaxnWdXIOx63887253 = -344547851;    int QaxnWdXIOx14847512 = -412825481;    int QaxnWdXIOx83844846 = -412880222;    int QaxnWdXIOx67934817 = -106584957;    int QaxnWdXIOx28987739 = -644184056;    int QaxnWdXIOx54528081 = -267277535;    int QaxnWdXIOx58797365 = -620184561;    int QaxnWdXIOx23121114 = -396175503;    int QaxnWdXIOx14941889 = -465246066;    int QaxnWdXIOx43486482 = -437988813;    int QaxnWdXIOx64241873 = -820290310;    int QaxnWdXIOx31625154 = 32377486;    int QaxnWdXIOx6349398 = 96522176;    int QaxnWdXIOx62271379 = -520740336;    int QaxnWdXIOx49654924 = -735118212;    int QaxnWdXIOx63872382 = -995463041;    int QaxnWdXIOx26732244 = -31004791;    int QaxnWdXIOx23082679 = -630003015;    int QaxnWdXIOx28466974 = 94830558;    int QaxnWdXIOx19514158 = -687412541;    int QaxnWdXIOx71812105 = -755171010;    int QaxnWdXIOx90304055 = -443051773;    int QaxnWdXIOx28692359 = 86534762;    int QaxnWdXIOx33784608 = -561682550;    int QaxnWdXIOx97466050 = 6478421;    int QaxnWdXIOx88465908 = -242443230;    int QaxnWdXIOx10878884 = -14458671;    int QaxnWdXIOx39712673 = 64909271;    int QaxnWdXIOx31586199 = -591795701;    int QaxnWdXIOx2476022 = -747692719;    int QaxnWdXIOx19641754 = -414683696;    int QaxnWdXIOx78920728 = -596809117;    int QaxnWdXIOx20645368 = -122665546;    int QaxnWdXIOx87704659 = -424817208;    int QaxnWdXIOx48003768 = -259555383;    int QaxnWdXIOx22156020 = -306316174;    int QaxnWdXIOx22531912 = -11515565;    int QaxnWdXIOx3803172 = -452478637;    int QaxnWdXIOx33397288 = -718928873;    int QaxnWdXIOx67503344 = -490428483;    int QaxnWdXIOx7830913 = -330110889;    int QaxnWdXIOx55068223 = -910120898;    int QaxnWdXIOx33058103 = -870924521;    int QaxnWdXIOx36389705 = -243453826;    int QaxnWdXIOx83408204 = -361083416;    int QaxnWdXIOx70566345 = -536367956;    int QaxnWdXIOx72024240 = -199333772;    int QaxnWdXIOx67642137 = -12581207;    int QaxnWdXIOx8718455 = -243251758;    int QaxnWdXIOx73687374 = -742141342;    int QaxnWdXIOx89540204 = -992786791;    int QaxnWdXIOx24214068 = -671130591;    int QaxnWdXIOx15856765 = -768315965;    int QaxnWdXIOx60297239 = -74592720;    int QaxnWdXIOx67835808 = -165871338;    int QaxnWdXIOx73583197 = -901496079;    int QaxnWdXIOx86155152 = -399360244;    int QaxnWdXIOx50060238 = -851197673;    int QaxnWdXIOx70468766 = -13063379;    int QaxnWdXIOx40521831 = -301740827;    int QaxnWdXIOx43649198 = -152818864;    int QaxnWdXIOx19084693 = -585093833;    int QaxnWdXIOx91534914 = -804379803;    int QaxnWdXIOx12465868 = -717553347;    int QaxnWdXIOx23844728 = 76694883;    int QaxnWdXIOx85321144 = -123481193;    int QaxnWdXIOx10979786 = -844956969;    int QaxnWdXIOx18644739 = -478660617;    int QaxnWdXIOx14267612 = -161184953;    int QaxnWdXIOx27498905 = -328802039;    int QaxnWdXIOx41340471 = -883947477;    int QaxnWdXIOx22929072 = -578526155;    int QaxnWdXIOx89685391 = -911074142;    int QaxnWdXIOx60963629 = -414740959;    int QaxnWdXIOx11683246 = -257301652;    int QaxnWdXIOx16743882 = -845050112;    int QaxnWdXIOx57245952 = -572127252;    int QaxnWdXIOx92302654 = -670011413;    int QaxnWdXIOx50376404 = -100599134;    int QaxnWdXIOx26899706 = -457153623;    int QaxnWdXIOx16441668 = 56890542;    int QaxnWdXIOx43236747 = 98122535;    int QaxnWdXIOx30994219 = -691838972;    int QaxnWdXIOx57898825 = -849654359;    int QaxnWdXIOx12935817 = -754905928;    int QaxnWdXIOx95427686 = -743553106;    int QaxnWdXIOx63063964 = -828493153;    int QaxnWdXIOx60348128 = 51927173;    int QaxnWdXIOx19868851 = -158945870;    int QaxnWdXIOx74420570 = -358059305;    int QaxnWdXIOx36000867 = -906955931;    int QaxnWdXIOx72471673 = -160317893;    int QaxnWdXIOx33334405 = -339415258;    int QaxnWdXIOx92875457 = -317188046;    int QaxnWdXIOx23854147 = -237609620;    int QaxnWdXIOx88746219 = -745017057;    int QaxnWdXIOx63533309 = -5741096;    int QaxnWdXIOx20592236 = -53371174;    int QaxnWdXIOx12544977 = 78957653;     QaxnWdXIOx39647913 = QaxnWdXIOx63887253;     QaxnWdXIOx63887253 = QaxnWdXIOx14847512;     QaxnWdXIOx14847512 = QaxnWdXIOx83844846;     QaxnWdXIOx83844846 = QaxnWdXIOx67934817;     QaxnWdXIOx67934817 = QaxnWdXIOx28987739;     QaxnWdXIOx28987739 = QaxnWdXIOx54528081;     QaxnWdXIOx54528081 = QaxnWdXIOx58797365;     QaxnWdXIOx58797365 = QaxnWdXIOx23121114;     QaxnWdXIOx23121114 = QaxnWdXIOx14941889;     QaxnWdXIOx14941889 = QaxnWdXIOx43486482;     QaxnWdXIOx43486482 = QaxnWdXIOx64241873;     QaxnWdXIOx64241873 = QaxnWdXIOx31625154;     QaxnWdXIOx31625154 = QaxnWdXIOx6349398;     QaxnWdXIOx6349398 = QaxnWdXIOx62271379;     QaxnWdXIOx62271379 = QaxnWdXIOx49654924;     QaxnWdXIOx49654924 = QaxnWdXIOx63872382;     QaxnWdXIOx63872382 = QaxnWdXIOx26732244;     QaxnWdXIOx26732244 = QaxnWdXIOx23082679;     QaxnWdXIOx23082679 = QaxnWdXIOx28466974;     QaxnWdXIOx28466974 = QaxnWdXIOx19514158;     QaxnWdXIOx19514158 = QaxnWdXIOx71812105;     QaxnWdXIOx71812105 = QaxnWdXIOx90304055;     QaxnWdXIOx90304055 = QaxnWdXIOx28692359;     QaxnWdXIOx28692359 = QaxnWdXIOx33784608;     QaxnWdXIOx33784608 = QaxnWdXIOx97466050;     QaxnWdXIOx97466050 = QaxnWdXIOx88465908;     QaxnWdXIOx88465908 = QaxnWdXIOx10878884;     QaxnWdXIOx10878884 = QaxnWdXIOx39712673;     QaxnWdXIOx39712673 = QaxnWdXIOx31586199;     QaxnWdXIOx31586199 = QaxnWdXIOx2476022;     QaxnWdXIOx2476022 = QaxnWdXIOx19641754;     QaxnWdXIOx19641754 = QaxnWdXIOx78920728;     QaxnWdXIOx78920728 = QaxnWdXIOx20645368;     QaxnWdXIOx20645368 = QaxnWdXIOx87704659;     QaxnWdXIOx87704659 = QaxnWdXIOx48003768;     QaxnWdXIOx48003768 = QaxnWdXIOx22156020;     QaxnWdXIOx22156020 = QaxnWdXIOx22531912;     QaxnWdXIOx22531912 = QaxnWdXIOx3803172;     QaxnWdXIOx3803172 = QaxnWdXIOx33397288;     QaxnWdXIOx33397288 = QaxnWdXIOx67503344;     QaxnWdXIOx67503344 = QaxnWdXIOx7830913;     QaxnWdXIOx7830913 = QaxnWdXIOx55068223;     QaxnWdXIOx55068223 = QaxnWdXIOx33058103;     QaxnWdXIOx33058103 = QaxnWdXIOx36389705;     QaxnWdXIOx36389705 = QaxnWdXIOx83408204;     QaxnWdXIOx83408204 = QaxnWdXIOx70566345;     QaxnWdXIOx70566345 = QaxnWdXIOx72024240;     QaxnWdXIOx72024240 = QaxnWdXIOx67642137;     QaxnWdXIOx67642137 = QaxnWdXIOx8718455;     QaxnWdXIOx8718455 = QaxnWdXIOx73687374;     QaxnWdXIOx73687374 = QaxnWdXIOx89540204;     QaxnWdXIOx89540204 = QaxnWdXIOx24214068;     QaxnWdXIOx24214068 = QaxnWdXIOx15856765;     QaxnWdXIOx15856765 = QaxnWdXIOx60297239;     QaxnWdXIOx60297239 = QaxnWdXIOx67835808;     QaxnWdXIOx67835808 = QaxnWdXIOx73583197;     QaxnWdXIOx73583197 = QaxnWdXIOx86155152;     QaxnWdXIOx86155152 = QaxnWdXIOx50060238;     QaxnWdXIOx50060238 = QaxnWdXIOx70468766;     QaxnWdXIOx70468766 = QaxnWdXIOx40521831;     QaxnWdXIOx40521831 = QaxnWdXIOx43649198;     QaxnWdXIOx43649198 = QaxnWdXIOx19084693;     QaxnWdXIOx19084693 = QaxnWdXIOx91534914;     QaxnWdXIOx91534914 = QaxnWdXIOx12465868;     QaxnWdXIOx12465868 = QaxnWdXIOx23844728;     QaxnWdXIOx23844728 = QaxnWdXIOx85321144;     QaxnWdXIOx85321144 = QaxnWdXIOx10979786;     QaxnWdXIOx10979786 = QaxnWdXIOx18644739;     QaxnWdXIOx18644739 = QaxnWdXIOx14267612;     QaxnWdXIOx14267612 = QaxnWdXIOx27498905;     QaxnWdXIOx27498905 = QaxnWdXIOx41340471;     QaxnWdXIOx41340471 = QaxnWdXIOx22929072;     QaxnWdXIOx22929072 = QaxnWdXIOx89685391;     QaxnWdXIOx89685391 = QaxnWdXIOx60963629;     QaxnWdXIOx60963629 = QaxnWdXIOx11683246;     QaxnWdXIOx11683246 = QaxnWdXIOx16743882;     QaxnWdXIOx16743882 = QaxnWdXIOx57245952;     QaxnWdXIOx57245952 = QaxnWdXIOx92302654;     QaxnWdXIOx92302654 = QaxnWdXIOx50376404;     QaxnWdXIOx50376404 = QaxnWdXIOx26899706;     QaxnWdXIOx26899706 = QaxnWdXIOx16441668;     QaxnWdXIOx16441668 = QaxnWdXIOx43236747;     QaxnWdXIOx43236747 = QaxnWdXIOx30994219;     QaxnWdXIOx30994219 = QaxnWdXIOx57898825;     QaxnWdXIOx57898825 = QaxnWdXIOx12935817;     QaxnWdXIOx12935817 = QaxnWdXIOx95427686;     QaxnWdXIOx95427686 = QaxnWdXIOx63063964;     QaxnWdXIOx63063964 = QaxnWdXIOx60348128;     QaxnWdXIOx60348128 = QaxnWdXIOx19868851;     QaxnWdXIOx19868851 = QaxnWdXIOx74420570;     QaxnWdXIOx74420570 = QaxnWdXIOx36000867;     QaxnWdXIOx36000867 = QaxnWdXIOx72471673;     QaxnWdXIOx72471673 = QaxnWdXIOx33334405;     QaxnWdXIOx33334405 = QaxnWdXIOx92875457;     QaxnWdXIOx92875457 = QaxnWdXIOx23854147;     QaxnWdXIOx23854147 = QaxnWdXIOx88746219;     QaxnWdXIOx88746219 = QaxnWdXIOx63533309;     QaxnWdXIOx63533309 = QaxnWdXIOx20592236;     QaxnWdXIOx20592236 = QaxnWdXIOx12544977;     QaxnWdXIOx12544977 = QaxnWdXIOx39647913;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rGiLxIvWBy80253500() {     int WYBMttWLjk74827668 = -917962860;    int WYBMttWLjk96433324 = -420829794;    int WYBMttWLjk60056378 = -3710093;    int WYBMttWLjk21712500 = -422949201;    int WYBMttWLjk50780074 = -721103102;    int WYBMttWLjk52375494 = -582282936;    int WYBMttWLjk99344241 = -930400445;    int WYBMttWLjk91936286 = -731122145;    int WYBMttWLjk32948891 = -528483564;    int WYBMttWLjk16284665 = -375268160;    int WYBMttWLjk88195118 = -589827157;    int WYBMttWLjk37803650 = -742890057;    int WYBMttWLjk36923723 = -468464017;    int WYBMttWLjk44744008 = -418448728;    int WYBMttWLjk77141100 = -92491028;    int WYBMttWLjk47771358 = -234881493;    int WYBMttWLjk13594793 = -16902758;    int WYBMttWLjk93266611 = -740886162;    int WYBMttWLjk45096670 = -56592069;    int WYBMttWLjk37563990 = -145023583;    int WYBMttWLjk20901898 = -359749739;    int WYBMttWLjk86378876 = -919803788;    int WYBMttWLjk95479277 = -91178417;    int WYBMttWLjk56049167 = -956882585;    int WYBMttWLjk46776425 = 33010255;    int WYBMttWLjk50513614 = -219641073;    int WYBMttWLjk76749283 = -332936290;    int WYBMttWLjk48739734 = -467744241;    int WYBMttWLjk50543840 = -947494774;    int WYBMttWLjk17404625 = -466546348;    int WYBMttWLjk57790144 = -718332386;    int WYBMttWLjk95777910 = -653652968;    int WYBMttWLjk20552925 = -800063181;    int WYBMttWLjk56827591 = -474864882;    int WYBMttWLjk64636518 = -839291586;    int WYBMttWLjk42092562 = -283748117;    int WYBMttWLjk40418424 = -829520097;    int WYBMttWLjk63944356 = -903498450;    int WYBMttWLjk22573093 = 17306170;    int WYBMttWLjk7974840 = 9861938;    int WYBMttWLjk69836356 = -181136244;    int WYBMttWLjk18344099 = -892538832;    int WYBMttWLjk35218525 = -261954672;    int WYBMttWLjk17898862 = -552495969;    int WYBMttWLjk363410 = -572107862;    int WYBMttWLjk24708647 = -804586280;    int WYBMttWLjk32747969 = -31012639;    int WYBMttWLjk70679300 = -602460186;    int WYBMttWLjk87507701 = -562136019;    int WYBMttWLjk32707198 = -309283594;    int WYBMttWLjk87624714 = -671546920;    int WYBMttWLjk77020751 = -412645663;    int WYBMttWLjk61565272 = -626991095;    int WYBMttWLjk10592765 = -101528664;    int WYBMttWLjk59351603 = -452842044;    int WYBMttWLjk88448792 = -998159073;    int WYBMttWLjk954047 = -229651377;    int WYBMttWLjk4007211 = -46827509;    int WYBMttWLjk74936075 = -355959456;    int WYBMttWLjk266461 = -401462029;    int WYBMttWLjk75626210 = -149346647;    int WYBMttWLjk50604508 = -362656204;    int WYBMttWLjk41392446 = -783627372;    int WYBMttWLjk15544267 = 38062783;    int WYBMttWLjk58494521 = -656935775;    int WYBMttWLjk92417207 = -936174190;    int WYBMttWLjk17250726 = -942826876;    int WYBMttWLjk80096132 = -993599135;    int WYBMttWLjk80107490 = -579157142;    int WYBMttWLjk35048538 = -808742911;    int WYBMttWLjk7352934 = -405361396;    int WYBMttWLjk49650436 = -113404309;    int WYBMttWLjk70693519 = -658192333;    int WYBMttWLjk37121831 = 33545993;    int WYBMttWLjk67727634 = -963887340;    int WYBMttWLjk2557800 = -467210908;    int WYBMttWLjk51160352 = -557849116;    int WYBMttWLjk77580415 = -538682449;    int WYBMttWLjk55685758 = -284774724;    int WYBMttWLjk22067778 = -162403465;    int WYBMttWLjk17765645 = -88628434;    int WYBMttWLjk6069983 = -730476104;    int WYBMttWLjk61232032 = -905608223;    int WYBMttWLjk17836643 = -538211180;    int WYBMttWLjk29779910 = -794999429;    int WYBMttWLjk80769392 = -205686724;    int WYBMttWLjk34212639 = 73338127;    int WYBMttWLjk9960161 = -598534517;    int WYBMttWLjk97475988 = 77977161;    int WYBMttWLjk76187725 = -841132513;    int WYBMttWLjk41138515 = 45903259;    int WYBMttWLjk36411213 = -682692589;    int WYBMttWLjk89008280 = -447538994;    int WYBMttWLjk22306632 = -581231801;    int WYBMttWLjk32348629 = -840791416;    int WYBMttWLjk19231848 = -818480041;    int WYBMttWLjk76951652 = -8911461;    int WYBMttWLjk19674258 = -200017456;    int WYBMttWLjk59404341 = -895560195;    int WYBMttWLjk7946202 = -917962860;     WYBMttWLjk74827668 = WYBMttWLjk96433324;     WYBMttWLjk96433324 = WYBMttWLjk60056378;     WYBMttWLjk60056378 = WYBMttWLjk21712500;     WYBMttWLjk21712500 = WYBMttWLjk50780074;     WYBMttWLjk50780074 = WYBMttWLjk52375494;     WYBMttWLjk52375494 = WYBMttWLjk99344241;     WYBMttWLjk99344241 = WYBMttWLjk91936286;     WYBMttWLjk91936286 = WYBMttWLjk32948891;     WYBMttWLjk32948891 = WYBMttWLjk16284665;     WYBMttWLjk16284665 = WYBMttWLjk88195118;     WYBMttWLjk88195118 = WYBMttWLjk37803650;     WYBMttWLjk37803650 = WYBMttWLjk36923723;     WYBMttWLjk36923723 = WYBMttWLjk44744008;     WYBMttWLjk44744008 = WYBMttWLjk77141100;     WYBMttWLjk77141100 = WYBMttWLjk47771358;     WYBMttWLjk47771358 = WYBMttWLjk13594793;     WYBMttWLjk13594793 = WYBMttWLjk93266611;     WYBMttWLjk93266611 = WYBMttWLjk45096670;     WYBMttWLjk45096670 = WYBMttWLjk37563990;     WYBMttWLjk37563990 = WYBMttWLjk20901898;     WYBMttWLjk20901898 = WYBMttWLjk86378876;     WYBMttWLjk86378876 = WYBMttWLjk95479277;     WYBMttWLjk95479277 = WYBMttWLjk56049167;     WYBMttWLjk56049167 = WYBMttWLjk46776425;     WYBMttWLjk46776425 = WYBMttWLjk50513614;     WYBMttWLjk50513614 = WYBMttWLjk76749283;     WYBMttWLjk76749283 = WYBMttWLjk48739734;     WYBMttWLjk48739734 = WYBMttWLjk50543840;     WYBMttWLjk50543840 = WYBMttWLjk17404625;     WYBMttWLjk17404625 = WYBMttWLjk57790144;     WYBMttWLjk57790144 = WYBMttWLjk95777910;     WYBMttWLjk95777910 = WYBMttWLjk20552925;     WYBMttWLjk20552925 = WYBMttWLjk56827591;     WYBMttWLjk56827591 = WYBMttWLjk64636518;     WYBMttWLjk64636518 = WYBMttWLjk42092562;     WYBMttWLjk42092562 = WYBMttWLjk40418424;     WYBMttWLjk40418424 = WYBMttWLjk63944356;     WYBMttWLjk63944356 = WYBMttWLjk22573093;     WYBMttWLjk22573093 = WYBMttWLjk7974840;     WYBMttWLjk7974840 = WYBMttWLjk69836356;     WYBMttWLjk69836356 = WYBMttWLjk18344099;     WYBMttWLjk18344099 = WYBMttWLjk35218525;     WYBMttWLjk35218525 = WYBMttWLjk17898862;     WYBMttWLjk17898862 = WYBMttWLjk363410;     WYBMttWLjk363410 = WYBMttWLjk24708647;     WYBMttWLjk24708647 = WYBMttWLjk32747969;     WYBMttWLjk32747969 = WYBMttWLjk70679300;     WYBMttWLjk70679300 = WYBMttWLjk87507701;     WYBMttWLjk87507701 = WYBMttWLjk32707198;     WYBMttWLjk32707198 = WYBMttWLjk87624714;     WYBMttWLjk87624714 = WYBMttWLjk77020751;     WYBMttWLjk77020751 = WYBMttWLjk61565272;     WYBMttWLjk61565272 = WYBMttWLjk10592765;     WYBMttWLjk10592765 = WYBMttWLjk59351603;     WYBMttWLjk59351603 = WYBMttWLjk88448792;     WYBMttWLjk88448792 = WYBMttWLjk954047;     WYBMttWLjk954047 = WYBMttWLjk4007211;     WYBMttWLjk4007211 = WYBMttWLjk74936075;     WYBMttWLjk74936075 = WYBMttWLjk266461;     WYBMttWLjk266461 = WYBMttWLjk75626210;     WYBMttWLjk75626210 = WYBMttWLjk50604508;     WYBMttWLjk50604508 = WYBMttWLjk41392446;     WYBMttWLjk41392446 = WYBMttWLjk15544267;     WYBMttWLjk15544267 = WYBMttWLjk58494521;     WYBMttWLjk58494521 = WYBMttWLjk92417207;     WYBMttWLjk92417207 = WYBMttWLjk17250726;     WYBMttWLjk17250726 = WYBMttWLjk80096132;     WYBMttWLjk80096132 = WYBMttWLjk80107490;     WYBMttWLjk80107490 = WYBMttWLjk35048538;     WYBMttWLjk35048538 = WYBMttWLjk7352934;     WYBMttWLjk7352934 = WYBMttWLjk49650436;     WYBMttWLjk49650436 = WYBMttWLjk70693519;     WYBMttWLjk70693519 = WYBMttWLjk37121831;     WYBMttWLjk37121831 = WYBMttWLjk67727634;     WYBMttWLjk67727634 = WYBMttWLjk2557800;     WYBMttWLjk2557800 = WYBMttWLjk51160352;     WYBMttWLjk51160352 = WYBMttWLjk77580415;     WYBMttWLjk77580415 = WYBMttWLjk55685758;     WYBMttWLjk55685758 = WYBMttWLjk22067778;     WYBMttWLjk22067778 = WYBMttWLjk17765645;     WYBMttWLjk17765645 = WYBMttWLjk6069983;     WYBMttWLjk6069983 = WYBMttWLjk61232032;     WYBMttWLjk61232032 = WYBMttWLjk17836643;     WYBMttWLjk17836643 = WYBMttWLjk29779910;     WYBMttWLjk29779910 = WYBMttWLjk80769392;     WYBMttWLjk80769392 = WYBMttWLjk34212639;     WYBMttWLjk34212639 = WYBMttWLjk9960161;     WYBMttWLjk9960161 = WYBMttWLjk97475988;     WYBMttWLjk97475988 = WYBMttWLjk76187725;     WYBMttWLjk76187725 = WYBMttWLjk41138515;     WYBMttWLjk41138515 = WYBMttWLjk36411213;     WYBMttWLjk36411213 = WYBMttWLjk89008280;     WYBMttWLjk89008280 = WYBMttWLjk22306632;     WYBMttWLjk22306632 = WYBMttWLjk32348629;     WYBMttWLjk32348629 = WYBMttWLjk19231848;     WYBMttWLjk19231848 = WYBMttWLjk76951652;     WYBMttWLjk76951652 = WYBMttWLjk19674258;     WYBMttWLjk19674258 = WYBMttWLjk59404341;     WYBMttWLjk59404341 = WYBMttWLjk7946202;     WYBMttWLjk7946202 = WYBMttWLjk74827668;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nFfYEwKqnj50709373() {     int ukxHPozsDX87613835 = 46996448;    int ukxHPozsDX83204213 = -591163808;    int ukxHPozsDX77809178 = -412069475;    int ukxHPozsDX29653326 = -247151478;    int ukxHPozsDX44291178 = -112919567;    int ukxHPozsDX95685564 = -359206249;    int ukxHPozsDX20552000 = -376072515;    int ukxHPozsDX18655399 = -75640402;    int ukxHPozsDX28353702 = -490241657;    int ukxHPozsDX22503370 = -179355793;    int ukxHPozsDX15682501 = -704513172;    int ukxHPozsDX69754155 = -150222873;    int ukxHPozsDX7067027 = -372246674;    int ukxHPozsDX94368087 = -750863926;    int ukxHPozsDX447653 = 523142;    int ukxHPozsDX76196426 = -785772882;    int ukxHPozsDX69346610 = -780677225;    int ukxHPozsDX76825097 = -296096464;    int ukxHPozsDX46257301 = 33938736;    int ukxHPozsDX40218486 = -744267723;    int ukxHPozsDX1075955 = -851066004;    int ukxHPozsDX15426696 = 10337864;    int ukxHPozsDX34239573 = -22182312;    int ukxHPozsDX556297 = -217227598;    int ukxHPozsDX52670684 = -177431670;    int ukxHPozsDX31832259 = 89927728;    int ukxHPozsDX88164819 = -966642570;    int ukxHPozsDX16092659 = -641542346;    int ukxHPozsDX67160893 = -61712014;    int ukxHPozsDX70155739 = -96432503;    int ukxHPozsDX2226140 = -994749136;    int ukxHPozsDX99096734 = -632372646;    int ukxHPozsDX66598874 = -675917965;    int ukxHPozsDX33903964 = -379619100;    int ukxHPozsDX47697050 = -349323466;    int ukxHPozsDX37316677 = -72311222;    int ukxHPozsDX77182484 = -619429448;    int ukxHPozsDX21648379 = -823118541;    int ukxHPozsDX28348479 = -924544407;    int ukxHPozsDX34239569 = -934554908;    int ukxHPozsDX31267664 = -556610238;    int ukxHPozsDX26046555 = -790850167;    int ukxHPozsDX70101994 = -285388117;    int ukxHPozsDX62527400 = -893668212;    int ukxHPozsDX80371899 = -464877822;    int ukxHPozsDX98336776 = -877904720;    int ukxHPozsDX10329989 = -528527692;    int ukxHPozsDX119267 = -757812965;    int ukxHPozsDX31571751 = -725737994;    int ukxHPozsDX79657618 = -920320057;    int ukxHPozsDX5952209 = -924431229;    int ukxHPozsDX5197299 = -883831001;    int ukxHPozsDX96050422 = -802994378;    int ukxHPozsDX55292490 = -790980406;    int ukxHPozsDX49633419 = -447286530;    int ukxHPozsDX72187140 = -963341417;    int ukxHPozsDX48964640 = -468981496;    int ukxHPozsDX77252881 = -94841878;    int ukxHPozsDX76982641 = 30280192;    int ukxHPozsDX12458920 = -102847295;    int ukxHPozsDX7520746 = -392563680;    int ukxHPozsDX4459342 = -734530170;    int ukxHPozsDX51494506 = 86071611;    int ukxHPozsDX58197962 = -293809154;    int ukxHPozsDX20277230 = -184606657;    int ukxHPozsDX16585767 = 27859473;    int ukxHPozsDX3155282 = -474304908;    int ukxHPozsDX73163062 = -992627574;    int ukxHPozsDX46671037 = -301540460;    int ukxHPozsDX63130976 = -927165637;    int ukxHPozsDX99013942 = -66343435;    int ukxHPozsDX47698231 = -957558684;    int ukxHPozsDX48476619 = -371552058;    int ukxHPozsDX12017732 = -31506357;    int ukxHPozsDX8950823 = -87657485;    int ukxHPozsDX75029400 = 39784163;    int ukxHPozsDX45324701 = -704274019;    int ukxHPozsDX71712173 = -128514100;    int ukxHPozsDX20184397 = -752349776;    int ukxHPozsDX54333907 = -299526950;    int ukxHPozsDX21502270 = -381544581;    int ukxHPozsDX88045552 = -108829605;    int ukxHPozsDX84520907 = -915804353;    int ukxHPozsDX87503275 = -141391958;    int ukxHPozsDX64203531 = -172001275;    int ukxHPozsDX97028840 = -10918136;    int ukxHPozsDX3046313 = -829378269;    int ukxHPozsDX11306385 = -884937559;    int ukxHPozsDX84270545 = -932332571;    int ukxHPozsDX75509909 = -385982050;    int ukxHPozsDX88352037 = -603329726;    int ukxHPozsDX99929603 = -424587570;    int ukxHPozsDX44665737 = -753398734;    int ukxHPozsDX15889559 = -721697112;    int ukxHPozsDX26718824 = -441991228;    int ukxHPozsDX26808323 = -822080068;    int ukxHPozsDX74552048 = -776921779;    int ukxHPozsDX11904033 = -991578963;    int ukxHPozsDX42250170 = -609061555;    int ukxHPozsDX63786133 = 46996448;     ukxHPozsDX87613835 = ukxHPozsDX83204213;     ukxHPozsDX83204213 = ukxHPozsDX77809178;     ukxHPozsDX77809178 = ukxHPozsDX29653326;     ukxHPozsDX29653326 = ukxHPozsDX44291178;     ukxHPozsDX44291178 = ukxHPozsDX95685564;     ukxHPozsDX95685564 = ukxHPozsDX20552000;     ukxHPozsDX20552000 = ukxHPozsDX18655399;     ukxHPozsDX18655399 = ukxHPozsDX28353702;     ukxHPozsDX28353702 = ukxHPozsDX22503370;     ukxHPozsDX22503370 = ukxHPozsDX15682501;     ukxHPozsDX15682501 = ukxHPozsDX69754155;     ukxHPozsDX69754155 = ukxHPozsDX7067027;     ukxHPozsDX7067027 = ukxHPozsDX94368087;     ukxHPozsDX94368087 = ukxHPozsDX447653;     ukxHPozsDX447653 = ukxHPozsDX76196426;     ukxHPozsDX76196426 = ukxHPozsDX69346610;     ukxHPozsDX69346610 = ukxHPozsDX76825097;     ukxHPozsDX76825097 = ukxHPozsDX46257301;     ukxHPozsDX46257301 = ukxHPozsDX40218486;     ukxHPozsDX40218486 = ukxHPozsDX1075955;     ukxHPozsDX1075955 = ukxHPozsDX15426696;     ukxHPozsDX15426696 = ukxHPozsDX34239573;     ukxHPozsDX34239573 = ukxHPozsDX556297;     ukxHPozsDX556297 = ukxHPozsDX52670684;     ukxHPozsDX52670684 = ukxHPozsDX31832259;     ukxHPozsDX31832259 = ukxHPozsDX88164819;     ukxHPozsDX88164819 = ukxHPozsDX16092659;     ukxHPozsDX16092659 = ukxHPozsDX67160893;     ukxHPozsDX67160893 = ukxHPozsDX70155739;     ukxHPozsDX70155739 = ukxHPozsDX2226140;     ukxHPozsDX2226140 = ukxHPozsDX99096734;     ukxHPozsDX99096734 = ukxHPozsDX66598874;     ukxHPozsDX66598874 = ukxHPozsDX33903964;     ukxHPozsDX33903964 = ukxHPozsDX47697050;     ukxHPozsDX47697050 = ukxHPozsDX37316677;     ukxHPozsDX37316677 = ukxHPozsDX77182484;     ukxHPozsDX77182484 = ukxHPozsDX21648379;     ukxHPozsDX21648379 = ukxHPozsDX28348479;     ukxHPozsDX28348479 = ukxHPozsDX34239569;     ukxHPozsDX34239569 = ukxHPozsDX31267664;     ukxHPozsDX31267664 = ukxHPozsDX26046555;     ukxHPozsDX26046555 = ukxHPozsDX70101994;     ukxHPozsDX70101994 = ukxHPozsDX62527400;     ukxHPozsDX62527400 = ukxHPozsDX80371899;     ukxHPozsDX80371899 = ukxHPozsDX98336776;     ukxHPozsDX98336776 = ukxHPozsDX10329989;     ukxHPozsDX10329989 = ukxHPozsDX119267;     ukxHPozsDX119267 = ukxHPozsDX31571751;     ukxHPozsDX31571751 = ukxHPozsDX79657618;     ukxHPozsDX79657618 = ukxHPozsDX5952209;     ukxHPozsDX5952209 = ukxHPozsDX5197299;     ukxHPozsDX5197299 = ukxHPozsDX96050422;     ukxHPozsDX96050422 = ukxHPozsDX55292490;     ukxHPozsDX55292490 = ukxHPozsDX49633419;     ukxHPozsDX49633419 = ukxHPozsDX72187140;     ukxHPozsDX72187140 = ukxHPozsDX48964640;     ukxHPozsDX48964640 = ukxHPozsDX77252881;     ukxHPozsDX77252881 = ukxHPozsDX76982641;     ukxHPozsDX76982641 = ukxHPozsDX12458920;     ukxHPozsDX12458920 = ukxHPozsDX7520746;     ukxHPozsDX7520746 = ukxHPozsDX4459342;     ukxHPozsDX4459342 = ukxHPozsDX51494506;     ukxHPozsDX51494506 = ukxHPozsDX58197962;     ukxHPozsDX58197962 = ukxHPozsDX20277230;     ukxHPozsDX20277230 = ukxHPozsDX16585767;     ukxHPozsDX16585767 = ukxHPozsDX3155282;     ukxHPozsDX3155282 = ukxHPozsDX73163062;     ukxHPozsDX73163062 = ukxHPozsDX46671037;     ukxHPozsDX46671037 = ukxHPozsDX63130976;     ukxHPozsDX63130976 = ukxHPozsDX99013942;     ukxHPozsDX99013942 = ukxHPozsDX47698231;     ukxHPozsDX47698231 = ukxHPozsDX48476619;     ukxHPozsDX48476619 = ukxHPozsDX12017732;     ukxHPozsDX12017732 = ukxHPozsDX8950823;     ukxHPozsDX8950823 = ukxHPozsDX75029400;     ukxHPozsDX75029400 = ukxHPozsDX45324701;     ukxHPozsDX45324701 = ukxHPozsDX71712173;     ukxHPozsDX71712173 = ukxHPozsDX20184397;     ukxHPozsDX20184397 = ukxHPozsDX54333907;     ukxHPozsDX54333907 = ukxHPozsDX21502270;     ukxHPozsDX21502270 = ukxHPozsDX88045552;     ukxHPozsDX88045552 = ukxHPozsDX84520907;     ukxHPozsDX84520907 = ukxHPozsDX87503275;     ukxHPozsDX87503275 = ukxHPozsDX64203531;     ukxHPozsDX64203531 = ukxHPozsDX97028840;     ukxHPozsDX97028840 = ukxHPozsDX3046313;     ukxHPozsDX3046313 = ukxHPozsDX11306385;     ukxHPozsDX11306385 = ukxHPozsDX84270545;     ukxHPozsDX84270545 = ukxHPozsDX75509909;     ukxHPozsDX75509909 = ukxHPozsDX88352037;     ukxHPozsDX88352037 = ukxHPozsDX99929603;     ukxHPozsDX99929603 = ukxHPozsDX44665737;     ukxHPozsDX44665737 = ukxHPozsDX15889559;     ukxHPozsDX15889559 = ukxHPozsDX26718824;     ukxHPozsDX26718824 = ukxHPozsDX26808323;     ukxHPozsDX26808323 = ukxHPozsDX74552048;     ukxHPozsDX74552048 = ukxHPozsDX11904033;     ukxHPozsDX11904033 = ukxHPozsDX42250170;     ukxHPozsDX42250170 = ukxHPozsDX63786133;     ukxHPozsDX63786133 = ukxHPozsDX87613835;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NUEkPprbzi68922715() {     int exmtIeWwGr71058141 = -833669233;    int exmtIeWwGr76302 = -590545995;    int exmtIeWwGr74217685 = -205594703;    int exmtIeWwGr92445577 = -440951313;    int exmtIeWwGr50412502 = 80212942;    int exmtIeWwGr94189965 = -138277902;    int exmtIeWwGr91591318 = -115983828;    int exmtIeWwGr33002842 = -62798431;    int exmtIeWwGr86883402 = -831701008;    int exmtIeWwGr55049021 = -647731905;    int exmtIeWwGr65098438 = -894629045;    int exmtIeWwGr32959555 = -137841119;    int exmtIeWwGr25184802 = -163907915;    int exmtIeWwGr73994979 = -539154283;    int exmtIeWwGr61302116 = -93499841;    int exmtIeWwGr62585588 = -573852207;    int exmtIeWwGr63098495 = -534022252;    int exmtIeWwGr57676543 = -543401341;    int exmtIeWwGr8697443 = 35263865;    int exmtIeWwGr29585929 = -173853715;    int exmtIeWwGr56716341 = -440595035;    int exmtIeWwGr27574014 = 19186094;    int exmtIeWwGr38065281 = -62071509;    int exmtIeWwGr35262855 = -289052994;    int exmtIeWwGr94246642 = -737084429;    int exmtIeWwGr27174408 = -890581988;    int exmtIeWwGr37619560 = -594726912;    int exmtIeWwGr55824284 = -711497230;    int exmtIeWwGr3241988 = 75782844;    int exmtIeWwGr85989080 = -609282354;    int exmtIeWwGr20321454 = -632506337;    int exmtIeWwGr62203161 = -547567726;    int exmtIeWwGr10137761 = -296790143;    int exmtIeWwGr76062474 = 28778730;    int exmtIeWwGr26423780 = -546988201;    int exmtIeWwGr49705861 = -960335128;    int exmtIeWwGr45796663 = -498278628;    int exmtIeWwGr62227211 = -531589061;    int exmtIeWwGr77343556 = -9442204;    int exmtIeWwGr83734704 = -687148430;    int exmtIeWwGr64916588 = -128159211;    int exmtIeWwGr76534340 = -264758488;    int exmtIeWwGr93669064 = -836445360;    int exmtIeWwGr54432340 = -649850983;    int exmtIeWwGr72316396 = -193034775;    int exmtIeWwGr37942772 = -897515644;    int exmtIeWwGr86345419 = -660831921;    int exmtIeWwGr19789863 = -756534684;    int exmtIeWwGr62418863 = -211340076;    int exmtIeWwGr14990102 = -27340512;    int exmtIeWwGr79209656 = -45332651;    int exmtIeWwGr60698093 = -942090313;    int exmtIeWwGr10162882 = 85258305;    int exmtIeWwGr79969249 = -509393794;    int exmtIeWwGr12206374 = -795772653;    int exmtIeWwGr43484127 = -752855327;    int exmtIeWwGr62011021 = -428474487;    int exmtIeWwGr38954830 = -916541710;    int exmtIeWwGr98198934 = -703866885;    int exmtIeWwGr23238095 = -29205071;    int exmtIeWwGr56570406 = -543550990;    int exmtIeWwGr35767034 = -404486599;    int exmtIeWwGr29760855 = -38581276;    int exmtIeWwGr894322 = -122418654;    int exmtIeWwGr34727568 = 84774431;    int exmtIeWwGr2895278 = -247061319;    int exmtIeWwGr22821794 = -841050977;    int exmtIeWwGr49122328 = -92686645;    int exmtIeWwGr47571199 = -992166082;    int exmtIeWwGr11596256 = -133164714;    int exmtIeWwGr16788926 = 24426420;    int exmtIeWwGr871284 = 97566809;    int exmtIeWwGr80332987 = -433959137;    int exmtIeWwGr24962739 = -277587705;    int exmtIeWwGr64669340 = 54305495;    int exmtIeWwGr80182001 = -75836547;    int exmtIeWwGr33904950 = -144368547;    int exmtIeWwGr83632941 = -412220527;    int exmtIeWwGr62946458 = 3981781;    int exmtIeWwGr56303870 = -839568785;    int exmtIeWwGr40828989 = -129750067;    int exmtIeWwGr17829698 = -838192229;    int exmtIeWwGr93405420 = -400157154;    int exmtIeWwGr88251885 = -896876645;    int exmtIeWwGr6779425 = -463949704;    int exmtIeWwGr59623360 = -690416025;    int exmtIeWwGr52040280 = -532826032;    int exmtIeWwGr30168512 = -787396349;    int exmtIeWwGr63856101 = -175448618;    int exmtIeWwGr82939652 = -794132875;    int exmtIeWwGr87694839 = -431860641;    int exmtIeWwGr6841834 = -581736918;    int exmtIeWwGr64028276 = -827722177;    int exmtIeWwGr54105462 = -980237134;    int exmtIeWwGr27164299 = -43597440;    int exmtIeWwGr29149555 = -723672612;    int exmtIeWwGr46773486 = -126177213;    int exmtIeWwGr92774742 = -614026707;    int exmtIeWwGr19704772 = -634625415;    int exmtIeWwGr69421119 = -833669233;     exmtIeWwGr71058141 = exmtIeWwGr76302;     exmtIeWwGr76302 = exmtIeWwGr74217685;     exmtIeWwGr74217685 = exmtIeWwGr92445577;     exmtIeWwGr92445577 = exmtIeWwGr50412502;     exmtIeWwGr50412502 = exmtIeWwGr94189965;     exmtIeWwGr94189965 = exmtIeWwGr91591318;     exmtIeWwGr91591318 = exmtIeWwGr33002842;     exmtIeWwGr33002842 = exmtIeWwGr86883402;     exmtIeWwGr86883402 = exmtIeWwGr55049021;     exmtIeWwGr55049021 = exmtIeWwGr65098438;     exmtIeWwGr65098438 = exmtIeWwGr32959555;     exmtIeWwGr32959555 = exmtIeWwGr25184802;     exmtIeWwGr25184802 = exmtIeWwGr73994979;     exmtIeWwGr73994979 = exmtIeWwGr61302116;     exmtIeWwGr61302116 = exmtIeWwGr62585588;     exmtIeWwGr62585588 = exmtIeWwGr63098495;     exmtIeWwGr63098495 = exmtIeWwGr57676543;     exmtIeWwGr57676543 = exmtIeWwGr8697443;     exmtIeWwGr8697443 = exmtIeWwGr29585929;     exmtIeWwGr29585929 = exmtIeWwGr56716341;     exmtIeWwGr56716341 = exmtIeWwGr27574014;     exmtIeWwGr27574014 = exmtIeWwGr38065281;     exmtIeWwGr38065281 = exmtIeWwGr35262855;     exmtIeWwGr35262855 = exmtIeWwGr94246642;     exmtIeWwGr94246642 = exmtIeWwGr27174408;     exmtIeWwGr27174408 = exmtIeWwGr37619560;     exmtIeWwGr37619560 = exmtIeWwGr55824284;     exmtIeWwGr55824284 = exmtIeWwGr3241988;     exmtIeWwGr3241988 = exmtIeWwGr85989080;     exmtIeWwGr85989080 = exmtIeWwGr20321454;     exmtIeWwGr20321454 = exmtIeWwGr62203161;     exmtIeWwGr62203161 = exmtIeWwGr10137761;     exmtIeWwGr10137761 = exmtIeWwGr76062474;     exmtIeWwGr76062474 = exmtIeWwGr26423780;     exmtIeWwGr26423780 = exmtIeWwGr49705861;     exmtIeWwGr49705861 = exmtIeWwGr45796663;     exmtIeWwGr45796663 = exmtIeWwGr62227211;     exmtIeWwGr62227211 = exmtIeWwGr77343556;     exmtIeWwGr77343556 = exmtIeWwGr83734704;     exmtIeWwGr83734704 = exmtIeWwGr64916588;     exmtIeWwGr64916588 = exmtIeWwGr76534340;     exmtIeWwGr76534340 = exmtIeWwGr93669064;     exmtIeWwGr93669064 = exmtIeWwGr54432340;     exmtIeWwGr54432340 = exmtIeWwGr72316396;     exmtIeWwGr72316396 = exmtIeWwGr37942772;     exmtIeWwGr37942772 = exmtIeWwGr86345419;     exmtIeWwGr86345419 = exmtIeWwGr19789863;     exmtIeWwGr19789863 = exmtIeWwGr62418863;     exmtIeWwGr62418863 = exmtIeWwGr14990102;     exmtIeWwGr14990102 = exmtIeWwGr79209656;     exmtIeWwGr79209656 = exmtIeWwGr60698093;     exmtIeWwGr60698093 = exmtIeWwGr10162882;     exmtIeWwGr10162882 = exmtIeWwGr79969249;     exmtIeWwGr79969249 = exmtIeWwGr12206374;     exmtIeWwGr12206374 = exmtIeWwGr43484127;     exmtIeWwGr43484127 = exmtIeWwGr62011021;     exmtIeWwGr62011021 = exmtIeWwGr38954830;     exmtIeWwGr38954830 = exmtIeWwGr98198934;     exmtIeWwGr98198934 = exmtIeWwGr23238095;     exmtIeWwGr23238095 = exmtIeWwGr56570406;     exmtIeWwGr56570406 = exmtIeWwGr35767034;     exmtIeWwGr35767034 = exmtIeWwGr29760855;     exmtIeWwGr29760855 = exmtIeWwGr894322;     exmtIeWwGr894322 = exmtIeWwGr34727568;     exmtIeWwGr34727568 = exmtIeWwGr2895278;     exmtIeWwGr2895278 = exmtIeWwGr22821794;     exmtIeWwGr22821794 = exmtIeWwGr49122328;     exmtIeWwGr49122328 = exmtIeWwGr47571199;     exmtIeWwGr47571199 = exmtIeWwGr11596256;     exmtIeWwGr11596256 = exmtIeWwGr16788926;     exmtIeWwGr16788926 = exmtIeWwGr871284;     exmtIeWwGr871284 = exmtIeWwGr80332987;     exmtIeWwGr80332987 = exmtIeWwGr24962739;     exmtIeWwGr24962739 = exmtIeWwGr64669340;     exmtIeWwGr64669340 = exmtIeWwGr80182001;     exmtIeWwGr80182001 = exmtIeWwGr33904950;     exmtIeWwGr33904950 = exmtIeWwGr83632941;     exmtIeWwGr83632941 = exmtIeWwGr62946458;     exmtIeWwGr62946458 = exmtIeWwGr56303870;     exmtIeWwGr56303870 = exmtIeWwGr40828989;     exmtIeWwGr40828989 = exmtIeWwGr17829698;     exmtIeWwGr17829698 = exmtIeWwGr93405420;     exmtIeWwGr93405420 = exmtIeWwGr88251885;     exmtIeWwGr88251885 = exmtIeWwGr6779425;     exmtIeWwGr6779425 = exmtIeWwGr59623360;     exmtIeWwGr59623360 = exmtIeWwGr52040280;     exmtIeWwGr52040280 = exmtIeWwGr30168512;     exmtIeWwGr30168512 = exmtIeWwGr63856101;     exmtIeWwGr63856101 = exmtIeWwGr82939652;     exmtIeWwGr82939652 = exmtIeWwGr87694839;     exmtIeWwGr87694839 = exmtIeWwGr6841834;     exmtIeWwGr6841834 = exmtIeWwGr64028276;     exmtIeWwGr64028276 = exmtIeWwGr54105462;     exmtIeWwGr54105462 = exmtIeWwGr27164299;     exmtIeWwGr27164299 = exmtIeWwGr29149555;     exmtIeWwGr29149555 = exmtIeWwGr46773486;     exmtIeWwGr46773486 = exmtIeWwGr92774742;     exmtIeWwGr92774742 = exmtIeWwGr19704772;     exmtIeWwGr19704772 = exmtIeWwGr69421119;     exmtIeWwGr69421119 = exmtIeWwGr71058141;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fjfFJFYPvA39378588() {     int eBSxwgeHzw83844308 = -968709924;    int eBSxwgeHzw86847190 = -760880009;    int eBSxwgeHzw91970484 = -613954085;    int eBSxwgeHzw386403 = -265153590;    int eBSxwgeHzw43923607 = -411603523;    int eBSxwgeHzw37500036 = 84798784;    int eBSxwgeHzw12799077 = -661655899;    int eBSxwgeHzw59721954 = -507316688;    int eBSxwgeHzw82288212 = -793459100;    int eBSxwgeHzw61267726 = -451819538;    int eBSxwgeHzw92585820 = 90684940;    int eBSxwgeHzw64910059 = -645173935;    int eBSxwgeHzw95328104 = -67690572;    int eBSxwgeHzw23619058 = -871569481;    int eBSxwgeHzw84608669 = -485671;    int eBSxwgeHzw91010657 = -24743597;    int eBSxwgeHzw18850313 = -197796719;    int eBSxwgeHzw41235029 = -98611643;    int eBSxwgeHzw9858074 = -974205330;    int eBSxwgeHzw32240425 = -773097854;    int eBSxwgeHzw36890398 = -931911299;    int eBSxwgeHzw56621833 = -150672254;    int eBSxwgeHzw76825576 = 6924596;    int eBSxwgeHzw79769984 = -649398006;    int eBSxwgeHzw140902 = -947526353;    int eBSxwgeHzw8493054 = -581013187;    int eBSxwgeHzw49035096 = -128433193;    int eBSxwgeHzw23177209 = -885295334;    int eBSxwgeHzw19859041 = -138434396;    int eBSxwgeHzw38740196 = -239168509;    int eBSxwgeHzw64757449 = -908923087;    int eBSxwgeHzw65521985 = -526287405;    int eBSxwgeHzw56183710 = -172644927;    int eBSxwgeHzw53138847 = -975975489;    int eBSxwgeHzw9484313 = -57020082;    int eBSxwgeHzw44929976 = -748898232;    int eBSxwgeHzw82560723 = -288187978;    int eBSxwgeHzw19931234 = -451209153;    int eBSxwgeHzw83118942 = -951292782;    int eBSxwgeHzw9999434 = -531565276;    int eBSxwgeHzw26347897 = -503633204;    int eBSxwgeHzw84236796 = -163069823;    int eBSxwgeHzw28552534 = -859878805;    int eBSxwgeHzw99060877 = -991023226;    int eBSxwgeHzw52324887 = -85804736;    int eBSxwgeHzw11570902 = -970834083;    int eBSxwgeHzw63927439 = -58346974;    int eBSxwgeHzw49229829 = -911887464;    int eBSxwgeHzw6482913 = -374942051;    int eBSxwgeHzw61940522 = -638376975;    int eBSxwgeHzw97537149 = -298216959;    int eBSxwgeHzw88874640 = -313275651;    int eBSxwgeHzw44648032 = -90744977;    int eBSxwgeHzw24668974 = -98845536;    int eBSxwgeHzw2488190 = -790217140;    int eBSxwgeHzw27222476 = -718037671;    int eBSxwgeHzw10021614 = -667804606;    int eBSxwgeHzw12200501 = -964556079;    int eBSxwgeHzw245502 = -317627238;    int eBSxwgeHzw35430554 = -830590337;    int eBSxwgeHzw88464940 = -786768024;    int eBSxwgeHzw89621867 = -776360565;    int eBSxwgeHzw39862914 = -268882292;    int eBSxwgeHzw43548016 = -454290591;    int eBSxwgeHzw96510276 = -542896451;    int eBSxwgeHzw27063836 = -383027656;    int eBSxwgeHzw8726350 = -372529008;    int eBSxwgeHzw42189258 = -91715084;    int eBSxwgeHzw14134746 = -714549400;    int eBSxwgeHzw39678694 = -251587440;    int eBSxwgeHzw8449934 = -736555619;    int eBSxwgeHzw98919078 = -746587567;    int eBSxwgeHzw58116086 = -147318862;    int eBSxwgeHzw99858639 = -342640055;    int eBSxwgeHzw5892529 = -169464650;    int eBSxwgeHzw52653602 = -668841477;    int eBSxwgeHzw28069299 = -290793450;    int eBSxwgeHzw77764698 = -2052179;    int eBSxwgeHzw27445098 = -463593271;    int eBSxwgeHzw88569999 = -976692270;    int eBSxwgeHzw44565615 = -422666214;    int eBSxwgeHzw99805267 = -216545730;    int eBSxwgeHzw16694296 = -410353284;    int eBSxwgeHzw57918518 = -500057422;    int eBSxwgeHzw41203046 = -940951550;    int eBSxwgeHzw75882809 = -495647437;    int eBSxwgeHzw20873954 = -335542428;    int eBSxwgeHzw31514736 = 26200608;    int eBSxwgeHzw50650658 = -85758349;    int eBSxwgeHzw82261836 = -338982411;    int eBSxwgeHzw34908362 = 18906373;    int eBSxwgeHzw70360223 = -323631900;    int eBSxwgeHzw19685733 = -33581916;    int eBSxwgeHzw47688389 = -20702445;    int eBSxwgeHzw21534494 = -744797252;    int eBSxwgeHzw36726029 = -727272640;    int eBSxwgeHzw44373883 = -894187531;    int eBSxwgeHzw85004517 = -305588214;    int eBSxwgeHzw2550602 = -348126775;    int eBSxwgeHzw25261051 = -968709924;     eBSxwgeHzw83844308 = eBSxwgeHzw86847190;     eBSxwgeHzw86847190 = eBSxwgeHzw91970484;     eBSxwgeHzw91970484 = eBSxwgeHzw386403;     eBSxwgeHzw386403 = eBSxwgeHzw43923607;     eBSxwgeHzw43923607 = eBSxwgeHzw37500036;     eBSxwgeHzw37500036 = eBSxwgeHzw12799077;     eBSxwgeHzw12799077 = eBSxwgeHzw59721954;     eBSxwgeHzw59721954 = eBSxwgeHzw82288212;     eBSxwgeHzw82288212 = eBSxwgeHzw61267726;     eBSxwgeHzw61267726 = eBSxwgeHzw92585820;     eBSxwgeHzw92585820 = eBSxwgeHzw64910059;     eBSxwgeHzw64910059 = eBSxwgeHzw95328104;     eBSxwgeHzw95328104 = eBSxwgeHzw23619058;     eBSxwgeHzw23619058 = eBSxwgeHzw84608669;     eBSxwgeHzw84608669 = eBSxwgeHzw91010657;     eBSxwgeHzw91010657 = eBSxwgeHzw18850313;     eBSxwgeHzw18850313 = eBSxwgeHzw41235029;     eBSxwgeHzw41235029 = eBSxwgeHzw9858074;     eBSxwgeHzw9858074 = eBSxwgeHzw32240425;     eBSxwgeHzw32240425 = eBSxwgeHzw36890398;     eBSxwgeHzw36890398 = eBSxwgeHzw56621833;     eBSxwgeHzw56621833 = eBSxwgeHzw76825576;     eBSxwgeHzw76825576 = eBSxwgeHzw79769984;     eBSxwgeHzw79769984 = eBSxwgeHzw140902;     eBSxwgeHzw140902 = eBSxwgeHzw8493054;     eBSxwgeHzw8493054 = eBSxwgeHzw49035096;     eBSxwgeHzw49035096 = eBSxwgeHzw23177209;     eBSxwgeHzw23177209 = eBSxwgeHzw19859041;     eBSxwgeHzw19859041 = eBSxwgeHzw38740196;     eBSxwgeHzw38740196 = eBSxwgeHzw64757449;     eBSxwgeHzw64757449 = eBSxwgeHzw65521985;     eBSxwgeHzw65521985 = eBSxwgeHzw56183710;     eBSxwgeHzw56183710 = eBSxwgeHzw53138847;     eBSxwgeHzw53138847 = eBSxwgeHzw9484313;     eBSxwgeHzw9484313 = eBSxwgeHzw44929976;     eBSxwgeHzw44929976 = eBSxwgeHzw82560723;     eBSxwgeHzw82560723 = eBSxwgeHzw19931234;     eBSxwgeHzw19931234 = eBSxwgeHzw83118942;     eBSxwgeHzw83118942 = eBSxwgeHzw9999434;     eBSxwgeHzw9999434 = eBSxwgeHzw26347897;     eBSxwgeHzw26347897 = eBSxwgeHzw84236796;     eBSxwgeHzw84236796 = eBSxwgeHzw28552534;     eBSxwgeHzw28552534 = eBSxwgeHzw99060877;     eBSxwgeHzw99060877 = eBSxwgeHzw52324887;     eBSxwgeHzw52324887 = eBSxwgeHzw11570902;     eBSxwgeHzw11570902 = eBSxwgeHzw63927439;     eBSxwgeHzw63927439 = eBSxwgeHzw49229829;     eBSxwgeHzw49229829 = eBSxwgeHzw6482913;     eBSxwgeHzw6482913 = eBSxwgeHzw61940522;     eBSxwgeHzw61940522 = eBSxwgeHzw97537149;     eBSxwgeHzw97537149 = eBSxwgeHzw88874640;     eBSxwgeHzw88874640 = eBSxwgeHzw44648032;     eBSxwgeHzw44648032 = eBSxwgeHzw24668974;     eBSxwgeHzw24668974 = eBSxwgeHzw2488190;     eBSxwgeHzw2488190 = eBSxwgeHzw27222476;     eBSxwgeHzw27222476 = eBSxwgeHzw10021614;     eBSxwgeHzw10021614 = eBSxwgeHzw12200501;     eBSxwgeHzw12200501 = eBSxwgeHzw245502;     eBSxwgeHzw245502 = eBSxwgeHzw35430554;     eBSxwgeHzw35430554 = eBSxwgeHzw88464940;     eBSxwgeHzw88464940 = eBSxwgeHzw89621867;     eBSxwgeHzw89621867 = eBSxwgeHzw39862914;     eBSxwgeHzw39862914 = eBSxwgeHzw43548016;     eBSxwgeHzw43548016 = eBSxwgeHzw96510276;     eBSxwgeHzw96510276 = eBSxwgeHzw27063836;     eBSxwgeHzw27063836 = eBSxwgeHzw8726350;     eBSxwgeHzw8726350 = eBSxwgeHzw42189258;     eBSxwgeHzw42189258 = eBSxwgeHzw14134746;     eBSxwgeHzw14134746 = eBSxwgeHzw39678694;     eBSxwgeHzw39678694 = eBSxwgeHzw8449934;     eBSxwgeHzw8449934 = eBSxwgeHzw98919078;     eBSxwgeHzw98919078 = eBSxwgeHzw58116086;     eBSxwgeHzw58116086 = eBSxwgeHzw99858639;     eBSxwgeHzw99858639 = eBSxwgeHzw5892529;     eBSxwgeHzw5892529 = eBSxwgeHzw52653602;     eBSxwgeHzw52653602 = eBSxwgeHzw28069299;     eBSxwgeHzw28069299 = eBSxwgeHzw77764698;     eBSxwgeHzw77764698 = eBSxwgeHzw27445098;     eBSxwgeHzw27445098 = eBSxwgeHzw88569999;     eBSxwgeHzw88569999 = eBSxwgeHzw44565615;     eBSxwgeHzw44565615 = eBSxwgeHzw99805267;     eBSxwgeHzw99805267 = eBSxwgeHzw16694296;     eBSxwgeHzw16694296 = eBSxwgeHzw57918518;     eBSxwgeHzw57918518 = eBSxwgeHzw41203046;     eBSxwgeHzw41203046 = eBSxwgeHzw75882809;     eBSxwgeHzw75882809 = eBSxwgeHzw20873954;     eBSxwgeHzw20873954 = eBSxwgeHzw31514736;     eBSxwgeHzw31514736 = eBSxwgeHzw50650658;     eBSxwgeHzw50650658 = eBSxwgeHzw82261836;     eBSxwgeHzw82261836 = eBSxwgeHzw34908362;     eBSxwgeHzw34908362 = eBSxwgeHzw70360223;     eBSxwgeHzw70360223 = eBSxwgeHzw19685733;     eBSxwgeHzw19685733 = eBSxwgeHzw47688389;     eBSxwgeHzw47688389 = eBSxwgeHzw21534494;     eBSxwgeHzw21534494 = eBSxwgeHzw36726029;     eBSxwgeHzw36726029 = eBSxwgeHzw44373883;     eBSxwgeHzw44373883 = eBSxwgeHzw85004517;     eBSxwgeHzw85004517 = eBSxwgeHzw2550602;     eBSxwgeHzw2550602 = eBSxwgeHzw25261051;     eBSxwgeHzw25261051 = eBSxwgeHzw83844308;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vuhtGxMmux57591929() {     int asJnxUbgNR67288614 = -749375605;    int asJnxUbgNR3719280 = -760262197;    int asJnxUbgNR88378991 = -407479312;    int asJnxUbgNR63178654 = -458953426;    int asJnxUbgNR50044931 = -218471014;    int asJnxUbgNR36004437 = -794272869;    int asJnxUbgNR83838394 = -401567212;    int asJnxUbgNR74069397 = -494474717;    int asJnxUbgNR40817913 = -34918451;    int asJnxUbgNR93813378 = -920195650;    int asJnxUbgNR42001759 = -99430933;    int asJnxUbgNR28115459 = -632792181;    int asJnxUbgNR13445881 = -959351813;    int asJnxUbgNR3245950 = -659859838;    int asJnxUbgNR45463133 = -94508654;    int asJnxUbgNR77399818 = -912822922;    int asJnxUbgNR12602198 = 48858254;    int asJnxUbgNR22086475 = -345916519;    int asJnxUbgNR72298215 = -972880201;    int asJnxUbgNR21607868 = -202683847;    int asJnxUbgNR92530785 = -521440330;    int asJnxUbgNR68769151 = -141824024;    int asJnxUbgNR80651284 = -32964601;    int asJnxUbgNR14476543 = -721223402;    int asJnxUbgNR41716859 = -407179112;    int asJnxUbgNR3835203 = -461522903;    int asJnxUbgNR98489836 = -856517535;    int asJnxUbgNR62908834 = -955250218;    int asJnxUbgNR55940134 = -939538;    int asJnxUbgNR54573537 = -752018361;    int asJnxUbgNR82852763 = -546680288;    int asJnxUbgNR28628412 = -441482485;    int asJnxUbgNR99722596 = -893517104;    int asJnxUbgNR95297357 = -567577659;    int asJnxUbgNR88211042 = -254684817;    int asJnxUbgNR57319160 = -536922138;    int asJnxUbgNR51174902 = -167037158;    int asJnxUbgNR60510066 = -159679673;    int asJnxUbgNR32114021 = -36190579;    int asJnxUbgNR59494569 = -284158798;    int asJnxUbgNR59996821 = -75182177;    int asJnxUbgNR34724583 = -736978144;    int asJnxUbgNR52119604 = -310936047;    int asJnxUbgNR90965817 = -747205997;    int asJnxUbgNR44269384 = -913961689;    int asJnxUbgNR51176898 = -990445007;    int asJnxUbgNR39942869 = -190651203;    int asJnxUbgNR68900424 = -910609182;    int asJnxUbgNR37330025 = -960544133;    int asJnxUbgNR97273006 = -845397430;    int asJnxUbgNR70794598 = -519118381;    int asJnxUbgNR44375435 = -371534962;    int asJnxUbgNR58760490 = -302492294;    int asJnxUbgNR49345733 = -917258923;    int asJnxUbgNR65061144 = -38703263;    int asJnxUbgNR98519462 = -507551582;    int asJnxUbgNR23067995 = -627297596;    int asJnxUbgNR73902449 = -686255911;    int asJnxUbgNR21461795 = 48225685;    int asJnxUbgNR46209729 = -756948112;    int asJnxUbgNR37514601 = -937755334;    int asJnxUbgNR20929560 = -446316994;    int asJnxUbgNR18129263 = -393535180;    int asJnxUbgNR86244375 = -282900091;    int asJnxUbgNR10960615 = -273515363;    int asJnxUbgNR13373347 = -657948449;    int asJnxUbgNR28392862 = -739275077;    int asJnxUbgNR18148523 = -291774155;    int asJnxUbgNR15034907 = -305175022;    int asJnxUbgNR88143972 = -557586516;    int asJnxUbgNR26224917 = -645785764;    int asJnxUbgNR52092131 = -791462074;    int asJnxUbgNR89972454 = -209725940;    int asJnxUbgNR12803647 = -588721404;    int asJnxUbgNR61611047 = -27501670;    int asJnxUbgNR57806203 = -784462186;    int asJnxUbgNR16649548 = -830887977;    int asJnxUbgNR89685466 = -285758605;    int asJnxUbgNR70207159 = -807261713;    int asJnxUbgNR90539961 = -416734105;    int asJnxUbgNR63892333 = -170871700;    int asJnxUbgNR29589412 = -945908354;    int asJnxUbgNR25578810 = -994706086;    int asJnxUbgNR58667128 = -155542109;    int asJnxUbgNR83778939 = -132899980;    int asJnxUbgNR38477328 = -75145326;    int asJnxUbgNR69867922 = -38990192;    int asJnxUbgNR50376864 = -976258182;    int asJnxUbgNR30236214 = -428874396;    int asJnxUbgNR89691579 = -747133236;    int asJnxUbgNR34251165 = -909624542;    int asJnxUbgNR77272453 = -480781248;    int asJnxUbgNR39048271 = -107905359;    int asJnxUbgNR85904292 = -279242468;    int asJnxUbgNR21979968 = -346403464;    int asJnxUbgNR39067261 = -628865183;    int asJnxUbgNR16595320 = -243442965;    int asJnxUbgNR65875228 = 71964043;    int asJnxUbgNR80005203 = -373690635;    int asJnxUbgNR30896037 = -749375605;     asJnxUbgNR67288614 = asJnxUbgNR3719280;     asJnxUbgNR3719280 = asJnxUbgNR88378991;     asJnxUbgNR88378991 = asJnxUbgNR63178654;     asJnxUbgNR63178654 = asJnxUbgNR50044931;     asJnxUbgNR50044931 = asJnxUbgNR36004437;     asJnxUbgNR36004437 = asJnxUbgNR83838394;     asJnxUbgNR83838394 = asJnxUbgNR74069397;     asJnxUbgNR74069397 = asJnxUbgNR40817913;     asJnxUbgNR40817913 = asJnxUbgNR93813378;     asJnxUbgNR93813378 = asJnxUbgNR42001759;     asJnxUbgNR42001759 = asJnxUbgNR28115459;     asJnxUbgNR28115459 = asJnxUbgNR13445881;     asJnxUbgNR13445881 = asJnxUbgNR3245950;     asJnxUbgNR3245950 = asJnxUbgNR45463133;     asJnxUbgNR45463133 = asJnxUbgNR77399818;     asJnxUbgNR77399818 = asJnxUbgNR12602198;     asJnxUbgNR12602198 = asJnxUbgNR22086475;     asJnxUbgNR22086475 = asJnxUbgNR72298215;     asJnxUbgNR72298215 = asJnxUbgNR21607868;     asJnxUbgNR21607868 = asJnxUbgNR92530785;     asJnxUbgNR92530785 = asJnxUbgNR68769151;     asJnxUbgNR68769151 = asJnxUbgNR80651284;     asJnxUbgNR80651284 = asJnxUbgNR14476543;     asJnxUbgNR14476543 = asJnxUbgNR41716859;     asJnxUbgNR41716859 = asJnxUbgNR3835203;     asJnxUbgNR3835203 = asJnxUbgNR98489836;     asJnxUbgNR98489836 = asJnxUbgNR62908834;     asJnxUbgNR62908834 = asJnxUbgNR55940134;     asJnxUbgNR55940134 = asJnxUbgNR54573537;     asJnxUbgNR54573537 = asJnxUbgNR82852763;     asJnxUbgNR82852763 = asJnxUbgNR28628412;     asJnxUbgNR28628412 = asJnxUbgNR99722596;     asJnxUbgNR99722596 = asJnxUbgNR95297357;     asJnxUbgNR95297357 = asJnxUbgNR88211042;     asJnxUbgNR88211042 = asJnxUbgNR57319160;     asJnxUbgNR57319160 = asJnxUbgNR51174902;     asJnxUbgNR51174902 = asJnxUbgNR60510066;     asJnxUbgNR60510066 = asJnxUbgNR32114021;     asJnxUbgNR32114021 = asJnxUbgNR59494569;     asJnxUbgNR59494569 = asJnxUbgNR59996821;     asJnxUbgNR59996821 = asJnxUbgNR34724583;     asJnxUbgNR34724583 = asJnxUbgNR52119604;     asJnxUbgNR52119604 = asJnxUbgNR90965817;     asJnxUbgNR90965817 = asJnxUbgNR44269384;     asJnxUbgNR44269384 = asJnxUbgNR51176898;     asJnxUbgNR51176898 = asJnxUbgNR39942869;     asJnxUbgNR39942869 = asJnxUbgNR68900424;     asJnxUbgNR68900424 = asJnxUbgNR37330025;     asJnxUbgNR37330025 = asJnxUbgNR97273006;     asJnxUbgNR97273006 = asJnxUbgNR70794598;     asJnxUbgNR70794598 = asJnxUbgNR44375435;     asJnxUbgNR44375435 = asJnxUbgNR58760490;     asJnxUbgNR58760490 = asJnxUbgNR49345733;     asJnxUbgNR49345733 = asJnxUbgNR65061144;     asJnxUbgNR65061144 = asJnxUbgNR98519462;     asJnxUbgNR98519462 = asJnxUbgNR23067995;     asJnxUbgNR23067995 = asJnxUbgNR73902449;     asJnxUbgNR73902449 = asJnxUbgNR21461795;     asJnxUbgNR21461795 = asJnxUbgNR46209729;     asJnxUbgNR46209729 = asJnxUbgNR37514601;     asJnxUbgNR37514601 = asJnxUbgNR20929560;     asJnxUbgNR20929560 = asJnxUbgNR18129263;     asJnxUbgNR18129263 = asJnxUbgNR86244375;     asJnxUbgNR86244375 = asJnxUbgNR10960615;     asJnxUbgNR10960615 = asJnxUbgNR13373347;     asJnxUbgNR13373347 = asJnxUbgNR28392862;     asJnxUbgNR28392862 = asJnxUbgNR18148523;     asJnxUbgNR18148523 = asJnxUbgNR15034907;     asJnxUbgNR15034907 = asJnxUbgNR88143972;     asJnxUbgNR88143972 = asJnxUbgNR26224917;     asJnxUbgNR26224917 = asJnxUbgNR52092131;     asJnxUbgNR52092131 = asJnxUbgNR89972454;     asJnxUbgNR89972454 = asJnxUbgNR12803647;     asJnxUbgNR12803647 = asJnxUbgNR61611047;     asJnxUbgNR61611047 = asJnxUbgNR57806203;     asJnxUbgNR57806203 = asJnxUbgNR16649548;     asJnxUbgNR16649548 = asJnxUbgNR89685466;     asJnxUbgNR89685466 = asJnxUbgNR70207159;     asJnxUbgNR70207159 = asJnxUbgNR90539961;     asJnxUbgNR90539961 = asJnxUbgNR63892333;     asJnxUbgNR63892333 = asJnxUbgNR29589412;     asJnxUbgNR29589412 = asJnxUbgNR25578810;     asJnxUbgNR25578810 = asJnxUbgNR58667128;     asJnxUbgNR58667128 = asJnxUbgNR83778939;     asJnxUbgNR83778939 = asJnxUbgNR38477328;     asJnxUbgNR38477328 = asJnxUbgNR69867922;     asJnxUbgNR69867922 = asJnxUbgNR50376864;     asJnxUbgNR50376864 = asJnxUbgNR30236214;     asJnxUbgNR30236214 = asJnxUbgNR89691579;     asJnxUbgNR89691579 = asJnxUbgNR34251165;     asJnxUbgNR34251165 = asJnxUbgNR77272453;     asJnxUbgNR77272453 = asJnxUbgNR39048271;     asJnxUbgNR39048271 = asJnxUbgNR85904292;     asJnxUbgNR85904292 = asJnxUbgNR21979968;     asJnxUbgNR21979968 = asJnxUbgNR39067261;     asJnxUbgNR39067261 = asJnxUbgNR16595320;     asJnxUbgNR16595320 = asJnxUbgNR65875228;     asJnxUbgNR65875228 = asJnxUbgNR80005203;     asJnxUbgNR80005203 = asJnxUbgNR30896037;     asJnxUbgNR30896037 = asJnxUbgNR67288614;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oQLBfDsNcK28047802() {     int loeHCyQyHg80074781 = -884416297;    int loeHCyQyHg90490167 = -930596210;    int loeHCyQyHg6131792 = -815838694;    int loeHCyQyHg71119479 = -283155703;    int loeHCyQyHg43556035 = -710287479;    int loeHCyQyHg79314507 = -571196182;    int loeHCyQyHg5046153 = -947239283;    int loeHCyQyHg788510 = -938992974;    int loeHCyQyHg36222724 = 3323457;    int loeHCyQyHg32084 = -724283283;    int loeHCyQyHg69489141 = -214116949;    int loeHCyQyHg60065964 = -40124997;    int loeHCyQyHg83589183 = -863134470;    int loeHCyQyHg52870028 = -992275036;    int loeHCyQyHg68769686 = -1494484;    int loeHCyQyHg5824888 = -363714311;    int loeHCyQyHg68354015 = -714916213;    int loeHCyQyHg5644960 = 98873179;    int loeHCyQyHg73458846 = -882349396;    int loeHCyQyHg24262364 = -801927986;    int loeHCyQyHg72704842 = 87243406;    int loeHCyQyHg97816970 = -311682372;    int loeHCyQyHg19411580 = 36031504;    int loeHCyQyHg58983672 = 18431586;    int loeHCyQyHg47611119 = -617621036;    int loeHCyQyHg85153847 = -151954102;    int loeHCyQyHg9905373 = -390223815;    int loeHCyQyHg30261760 = -29048323;    int loeHCyQyHg72557187 = -215156778;    int loeHCyQyHg7324653 = -381904515;    int loeHCyQyHg27288759 = -823097038;    int loeHCyQyHg31947236 = -420202163;    int loeHCyQyHg45768546 = -769371889;    int loeHCyQyHg72373730 = -472331877;    int loeHCyQyHg71271574 = -864716697;    int loeHCyQyHg52543275 = -325485242;    int loeHCyQyHg87938962 = 43053491;    int loeHCyQyHg18214089 = -79299765;    int loeHCyQyHg37889407 = -978041156;    int loeHCyQyHg85759298 = -128575644;    int loeHCyQyHg21428129 = -450656171;    int loeHCyQyHg42427039 = -635289478;    int loeHCyQyHg87003073 = -334369492;    int loeHCyQyHg35594356 = 11621760;    int loeHCyQyHg24277874 = -806731649;    int loeHCyQyHg24805027 = 36236553;    int loeHCyQyHg17524889 = -688166255;    int loeHCyQyHg98340390 = 34038038;    int loeHCyQyHg81394074 = -24146108;    int loeHCyQyHg44223427 = -356433893;    int loeHCyQyHg89122091 = -772002690;    int loeHCyQyHg72551982 = -842720300;    int loeHCyQyHg93245640 = -478495576;    int loeHCyQyHg94045458 = -506710665;    int loeHCyQyHg55342960 = -33147749;    int loeHCyQyHg82257810 = -472733925;    int loeHCyQyHg71078588 = -866627715;    int loeHCyQyHg47148119 = -734270280;    int loeHCyQyHg23508361 = -665534667;    int loeHCyQyHg58402188 = -458333378;    int loeHCyQyHg69409135 = -80972367;    int loeHCyQyHg74784393 = -818190960;    int loeHCyQyHg28231322 = -623836196;    int loeHCyQyHg28898071 = -614772028;    int loeHCyQyHg72743324 = -901186245;    int loeHCyQyHg37541905 = -793914786;    int loeHCyQyHg14297418 = -270753109;    int loeHCyQyHg11215453 = -290802593;    int loeHCyQyHg81598453 = -27558339;    int loeHCyQyHg16226411 = -676009243;    int loeHCyQyHg17885925 = -306767803;    int loeHCyQyHg50139926 = -535616449;    int loeHCyQyHg67755553 = 76914334;    int loeHCyQyHg87699547 = -653773753;    int loeHCyQyHg2834235 = -251271815;    int loeHCyQyHg30277804 = -277467116;    int loeHCyQyHg10813897 = -977312880;    int loeHCyQyHg83817223 = -975590257;    int loeHCyQyHg34705798 = -174836766;    int loeHCyQyHg22806092 = -553857590;    int loeHCyQyHg67628959 = -463787847;    int loeHCyQyHg11564982 = -324261854;    int loeHCyQyHg48867685 = 95097784;    int loeHCyQyHg28333761 = -858722886;    int loeHCyQyHg18202562 = -609901826;    int loeHCyQyHg54736777 = -980376738;    int loeHCyQyHg38701595 = -941706587;    int loeHCyQyHg51723088 = -162661224;    int loeHCyQyHg17030771 = -339184128;    int loeHCyQyHg89013764 = -291982773;    int loeHCyQyHg81464687 = -458857527;    int loeHCyQyHg40790843 = -222676229;    int loeHCyQyHg94705727 = -413765098;    int loeHCyQyHg79487219 = -419707779;    int loeHCyQyHg16350164 = 52396723;    int loeHCyQyHg46643736 = -632465211;    int loeHCyQyHg14195717 = 88546717;    int loeHCyQyHg58105003 = -719597465;    int loeHCyQyHg62851032 = -87191995;    int loeHCyQyHg86735969 = -884416297;     loeHCyQyHg80074781 = loeHCyQyHg90490167;     loeHCyQyHg90490167 = loeHCyQyHg6131792;     loeHCyQyHg6131792 = loeHCyQyHg71119479;     loeHCyQyHg71119479 = loeHCyQyHg43556035;     loeHCyQyHg43556035 = loeHCyQyHg79314507;     loeHCyQyHg79314507 = loeHCyQyHg5046153;     loeHCyQyHg5046153 = loeHCyQyHg788510;     loeHCyQyHg788510 = loeHCyQyHg36222724;     loeHCyQyHg36222724 = loeHCyQyHg32084;     loeHCyQyHg32084 = loeHCyQyHg69489141;     loeHCyQyHg69489141 = loeHCyQyHg60065964;     loeHCyQyHg60065964 = loeHCyQyHg83589183;     loeHCyQyHg83589183 = loeHCyQyHg52870028;     loeHCyQyHg52870028 = loeHCyQyHg68769686;     loeHCyQyHg68769686 = loeHCyQyHg5824888;     loeHCyQyHg5824888 = loeHCyQyHg68354015;     loeHCyQyHg68354015 = loeHCyQyHg5644960;     loeHCyQyHg5644960 = loeHCyQyHg73458846;     loeHCyQyHg73458846 = loeHCyQyHg24262364;     loeHCyQyHg24262364 = loeHCyQyHg72704842;     loeHCyQyHg72704842 = loeHCyQyHg97816970;     loeHCyQyHg97816970 = loeHCyQyHg19411580;     loeHCyQyHg19411580 = loeHCyQyHg58983672;     loeHCyQyHg58983672 = loeHCyQyHg47611119;     loeHCyQyHg47611119 = loeHCyQyHg85153847;     loeHCyQyHg85153847 = loeHCyQyHg9905373;     loeHCyQyHg9905373 = loeHCyQyHg30261760;     loeHCyQyHg30261760 = loeHCyQyHg72557187;     loeHCyQyHg72557187 = loeHCyQyHg7324653;     loeHCyQyHg7324653 = loeHCyQyHg27288759;     loeHCyQyHg27288759 = loeHCyQyHg31947236;     loeHCyQyHg31947236 = loeHCyQyHg45768546;     loeHCyQyHg45768546 = loeHCyQyHg72373730;     loeHCyQyHg72373730 = loeHCyQyHg71271574;     loeHCyQyHg71271574 = loeHCyQyHg52543275;     loeHCyQyHg52543275 = loeHCyQyHg87938962;     loeHCyQyHg87938962 = loeHCyQyHg18214089;     loeHCyQyHg18214089 = loeHCyQyHg37889407;     loeHCyQyHg37889407 = loeHCyQyHg85759298;     loeHCyQyHg85759298 = loeHCyQyHg21428129;     loeHCyQyHg21428129 = loeHCyQyHg42427039;     loeHCyQyHg42427039 = loeHCyQyHg87003073;     loeHCyQyHg87003073 = loeHCyQyHg35594356;     loeHCyQyHg35594356 = loeHCyQyHg24277874;     loeHCyQyHg24277874 = loeHCyQyHg24805027;     loeHCyQyHg24805027 = loeHCyQyHg17524889;     loeHCyQyHg17524889 = loeHCyQyHg98340390;     loeHCyQyHg98340390 = loeHCyQyHg81394074;     loeHCyQyHg81394074 = loeHCyQyHg44223427;     loeHCyQyHg44223427 = loeHCyQyHg89122091;     loeHCyQyHg89122091 = loeHCyQyHg72551982;     loeHCyQyHg72551982 = loeHCyQyHg93245640;     loeHCyQyHg93245640 = loeHCyQyHg94045458;     loeHCyQyHg94045458 = loeHCyQyHg55342960;     loeHCyQyHg55342960 = loeHCyQyHg82257810;     loeHCyQyHg82257810 = loeHCyQyHg71078588;     loeHCyQyHg71078588 = loeHCyQyHg47148119;     loeHCyQyHg47148119 = loeHCyQyHg23508361;     loeHCyQyHg23508361 = loeHCyQyHg58402188;     loeHCyQyHg58402188 = loeHCyQyHg69409135;     loeHCyQyHg69409135 = loeHCyQyHg74784393;     loeHCyQyHg74784393 = loeHCyQyHg28231322;     loeHCyQyHg28231322 = loeHCyQyHg28898071;     loeHCyQyHg28898071 = loeHCyQyHg72743324;     loeHCyQyHg72743324 = loeHCyQyHg37541905;     loeHCyQyHg37541905 = loeHCyQyHg14297418;     loeHCyQyHg14297418 = loeHCyQyHg11215453;     loeHCyQyHg11215453 = loeHCyQyHg81598453;     loeHCyQyHg81598453 = loeHCyQyHg16226411;     loeHCyQyHg16226411 = loeHCyQyHg17885925;     loeHCyQyHg17885925 = loeHCyQyHg50139926;     loeHCyQyHg50139926 = loeHCyQyHg67755553;     loeHCyQyHg67755553 = loeHCyQyHg87699547;     loeHCyQyHg87699547 = loeHCyQyHg2834235;     loeHCyQyHg2834235 = loeHCyQyHg30277804;     loeHCyQyHg30277804 = loeHCyQyHg10813897;     loeHCyQyHg10813897 = loeHCyQyHg83817223;     loeHCyQyHg83817223 = loeHCyQyHg34705798;     loeHCyQyHg34705798 = loeHCyQyHg22806092;     loeHCyQyHg22806092 = loeHCyQyHg67628959;     loeHCyQyHg67628959 = loeHCyQyHg11564982;     loeHCyQyHg11564982 = loeHCyQyHg48867685;     loeHCyQyHg48867685 = loeHCyQyHg28333761;     loeHCyQyHg28333761 = loeHCyQyHg18202562;     loeHCyQyHg18202562 = loeHCyQyHg54736777;     loeHCyQyHg54736777 = loeHCyQyHg38701595;     loeHCyQyHg38701595 = loeHCyQyHg51723088;     loeHCyQyHg51723088 = loeHCyQyHg17030771;     loeHCyQyHg17030771 = loeHCyQyHg89013764;     loeHCyQyHg89013764 = loeHCyQyHg81464687;     loeHCyQyHg81464687 = loeHCyQyHg40790843;     loeHCyQyHg40790843 = loeHCyQyHg94705727;     loeHCyQyHg94705727 = loeHCyQyHg79487219;     loeHCyQyHg79487219 = loeHCyQyHg16350164;     loeHCyQyHg16350164 = loeHCyQyHg46643736;     loeHCyQyHg46643736 = loeHCyQyHg14195717;     loeHCyQyHg14195717 = loeHCyQyHg58105003;     loeHCyQyHg58105003 = loeHCyQyHg62851032;     loeHCyQyHg62851032 = loeHCyQyHg86735969;     loeHCyQyHg86735969 = loeHCyQyHg80074781;}
// Junk Finished
