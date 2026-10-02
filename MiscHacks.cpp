#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "IClientMode.h"
#include <chrono>
#include <algorithm>
#include <time.h>
#include "Hooks.h"
#include "position_adjust.h"
#include "RageBot.h"
#include "Autowall.h"
#include "Resolver.h"
CMiscHacks* c_misc = new CMiscHacks;
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
inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}
inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}
Vector AutoStrafeView;
void CMiscHacks::Init()
{
}
void CMiscHacks::Draw()
{
	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

}

void set_name(const char* name)
{
	ConVar* nameConvar = interfaces::cvar->FindVar(("name"));
	*(int*)((DWORD)&nameConvar->fnChangeCallback + 0xC) = NULL;
	nameConvar->SetValueChar(name);
}

inline float FastSqrt(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}
#define square( x ) ( x * x )

void angleVectors(const Vector& angles, Vector& forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void CMiscHacks::zeusbot(CUserCmd *m_pcmd)
{
	auto m_local = hackManager.pLocal();

	if (m_local->GetFlags() & FL_FROZEN)
		return;
	if (!m_local->IsAlive())
		return;
	if (!m_local->GetWeapon2())
		return;

	do_zeus = false;

	for (int y = 0; y <= 360; y += 360.f / 6.f) {
		for (int x = -89; x <= 89; x += 179.f / 6.f) {
			Vector ang = Vector(x, y, 0);
			Vector dir;
			angleVectors(ang, dir);
			trace_t trace;
			UTIL_TraceLine(m_local->GetEyePosition(), m_local->GetEyePosition() + (dir * 555), MASK_SHOT, m_local, 0, &trace);

			if (trace.m_pEnt == nullptr)
				continue;
			if (trace.m_pEnt == m_local)
				continue;
			if (!trace.m_pEnt->IsAlive())
				continue;
		
			if (trace.m_pEnt->team() == m_local->team())
				continue;
			if (trace.m_pEnt->IsDormant())
				continue;

			player_info_t info;
			if (!(interfaces::engine->GetPlayerInfo(trace.m_pEnt->GetIndex(), &info)))
				continue;

			do_zeus = true;

			m_pcmd->viewangles = Vector(x, y, 0);
			m_pcmd->buttons |= IN_ATTACK;

			do_zeus = false;

			return;
		}
	}
}

void CMiscHacks::namespam()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < .5)
		return;
	const char* result;
	std::vector <std::string> names;
	if (interfaces::engine->IsInGame() && interfaces::engine->IsConnected()) {
		for (int i = 1; i < interfaces::globals->max_clients; i++)
		{
			IClientEntity *entity = interfaces::ent_list->get_client_entity(i);

			player_info_t pInfo;

			if (entity && hackManager.pLocal()->team() == entity->team())
			{
				ClientClass* cClass = (ClientClass*)entity->GetClientClass();

				if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
				{
					if (interfaces::engine->GetPlayerInfo(i, &pInfo))
					{
						if (!strstr(pInfo.name, "GOTV"))
							names.push_back(pInfo.name);
					}
				}
			}
		}
	}

	set_name("\n\xAD\xAD\xAD");
	int randomIndex = rand() % names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", names[randomIndex].c_str());
	result = buffer;



	set_name(result);
	start_t = clock();

}
/*
template<class T>
static T* FindHudElement(const char* name)
{

static auto pThis = *reinterpret_cast<DWORD**>(Utilities::Memory::FindPatternV2("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 8D 58") + 1);

static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39"));

if (find_hud_element != nullptr)
{
return (T*)find_hud_element(pThis, name);
}

}
void preservefeed(IGameEvent* Event)
{
if (hackManager.pLocal()->IsAlive())
{
static DWORD* _death_notice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");

if (_death_notice == nullptr)
return;

static void(__thiscall *_clear_notices)(DWORD) = (void(__thiscall*)(DWORD))Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

if (round_change)
{

_death_notice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
if (_death_notice - 20)
_clear_notices(((DWORD)_death_notice - 20));
round_change = false;
}

if (_death_notice)
*(float*)((DWORD)_death_notice + 0x50) = Options::Menu.VisualsTab.killfeed.GetState() ? 100 : 1;
}
}*/
void CMiscHacks::buybot_primary()
{
	bool is_ct = hackManager.pLocal()->team() == TEAM_CS_CT;
	switch (options::menu.misc.buybot_primary.getindex())
	{
	case 1: is_ct ? (interfaces::engine->ExecuteClientCmd("buy scar20;")) : (interfaces::engine->ExecuteClientCmd("buy g3sg1;"));
		break;
	case 2: interfaces::engine->ExecuteClientCmd("buy ssg08;");
		break;
	case 3: interfaces::engine->ExecuteClientCmd("buy awp;");
		break;
	case 4: is_ct ? (interfaces::engine->ExecuteClientCmd("buy m4a1; buy m4a1_silencer")) : (interfaces::engine->ExecuteClientCmd("buy ak47;"));
		break;
	case 5: is_ct ? (interfaces::engine->ExecuteClientCmd("buy aug;")) : (interfaces::engine->ExecuteClientCmd("buy sg556"));
		break;
	case 6: is_ct ? (interfaces::engine->ExecuteClientCmd("buy mp9")) : (interfaces::engine->ExecuteClientCmd("buy mac-10;"));
		break;
	}

}

void CMiscHacks::buybot_secondary()
{
	switch (options::menu.misc.buybot_secondary.getindex())
	{
	case 1: interfaces::engine->ExecuteClientCmd("buy elite;");
		break;
	case 2: interfaces::engine->ExecuteClientCmd("buy deagle;");
		break;
	case 3: interfaces::engine->ExecuteClientCmd("buy fn57");
		break;
	}

}

void CMiscHacks::buybot_otr()
{
	std::vector<dropdownboxitem> otr_list = options::menu.misc.buybot_otr.items;

	if (otr_list[0].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy vest; buy vesthelm;");
	}

	if (otr_list[1].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy hegrenade;");
	}

	if (otr_list[2].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy flashbang;");
	}

	if (otr_list[3].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy smokegrenade;");
	}

	if (otr_list[4].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy molotov;");
	}

	if (otr_list[5].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy taser;");
	}

	if (otr_list[6].GetSelected)
	{
		interfaces::engine->ExecuteClientCmd("buy defuser;");
	}

}

void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	IGameEvent* Event;
	if (!hackManager.pLocal()->IsAlive())
	{
		_done = false;
		return;
	}

	// ------- Oi thundercunt, this is needed for the weapon configs ------- //

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

//	preset_cfg();
	if (pWeapon != nullptr)
	{
		if (GetAsyncKeyState(options::menu.misc.minimal_walk.GetKey()))
		{
			MinimalWalk(pCmd, get_gun(pWeapon));
		}

		if (options::menu.aimbot.AimbotEnable.getstate())
			zeusbot(pCmd);

		if (game_utils::AutoSniper(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_auto.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_auto.GetValue());

		}

		if (game_utils::IsPistol(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_pistol.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_pistol.GetValue());

		}

		if (pWeapon->is_scout())
		{

			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_scout.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_scout.GetValue());

		}

		if (pWeapon->is_awp())
		{
			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_awp.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_awp.GetValue());

		}

		if (game_utils::IsRifle(pWeapon) || game_utils::IsShotgun(pWeapon) || game_utils::IsMachinegun(pWeapon))
		{

			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_otr.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_otr.GetValue());

		}

		if (game_utils::IsMP(pWeapon))
		{
			options::menu.aimbot.AccuracyHitchance.SetValue((float)options::menu.aimbot.hc_smg.GetValue());
			options::menu.aimbot.AccuracyMinimumDamage.SetValue((float)options::menu.aimbot.md_smg.GetValue());
		}

		if (game_utils::IsZeus(pWeapon))
		{
			options::menu.aimbot.AccuracyHitchance.SetValue(5);
			options::menu.aimbot.AccuracyMinimumDamage.SetValue(21);
		} 
	}

	if (options::menu.misc.infinite_duck.getstate())
	{
		pCmd->buttons |= IN_BULLRUSH;
	}

	if (options::menu.visuals.override_viewmodel.getstate())
		viewmodel_x_y_z();

	//	if (Options::Menu.RageBotTab.AimbotEnable.GetState())
	//		AutoPistol(pCmd);

//	RankReveal(pCmd);

	if (pLocal->movetype() == MOVETYPE_LADDER || pLocal->movetype() == MOVETYPE_NOCLIP)
		return;

	if (options::menu.misc.OtherAutoJump.getstate())
		AutoJump(pCmd);
	if (options::menu.misc.airduck_type.getindex() != 0)
	{
		airduck(pCmd);
	}
	interfaces::engine->get_viewangles(AutoStrafeView);
	if (options::menu.misc.OtherAutoStrafe.getstate())
	{
		strafer(pCmd);
	}

	if (GetAsyncKeyState(options::menu.misc.fw.GetKey()) && !options::menu.m_bIsOpen)
	{
		FakeWalk0(pCmd, bSendPacket);
	}

	fake_crouch(pCmd, bSendPacket, pLocal);

	if (options::menu.visuals.DisablePostProcess.getstate())
		PostProcces();

	if (!_done && hackManager.pLocal()->IsAlive())
	{
		if (options::menu.misc.buybot_primary.getindex() != 0)
			buybot_primary();

		if (options::menu.misc.buybot_secondary.getindex() != 0)
			buybot_secondary();

		buybot_otr();

		_done = true;
	}

	if (options::menu.misc.NameChanger.getstate())
	{
		namespam();
	}
}
int CMiscHacks::GetFPS()
{
	static int fps = 0;
	static int count = 0;
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	static auto last = high_resolution_clock::now();
	count++;
	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}
	return fps;
}
float curtime_fixedx(CUserCmd* ucmd) {
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}

void VectorAnglesXXX(Vector forward, Vector &angles)
{
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);

		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector CalcAngleFakewalk(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesXXX(dst - src, ret);
	return ret;
}

void rotate_movement(float yaw, CUserCmd* cmd)
{
	Vector viewangles;
	QAngle yamom;
	interfaces::engine->get_viewangles(viewangles);
	float rotation = DEG2RAD(viewangles.y - yaw);
	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);
	float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
	float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);
	cmd->forwardmove = new_forwardmove;
	cmd->sidemove = new_sidemove;
}

float fakewalk_curtime(CUserCmd* ucmd)
{
	auto local_player = hackManager.pLocal();

	if (!local_player)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted)
	{
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}
void CMiscHacks::FakeWalk0(CUserCmd* pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	globalsh.fakewalk = true;
	static int iChoked = -1;
	iChoked++;
	if (pCmd->forwardmove > 0)
	{
		pCmd->buttons |= IN_BACK;
		pCmd->buttons &= ~IN_FORWARD;
	}
	if (pCmd->forwardmove < 0)
	{
		pCmd->buttons |= IN_FORWARD;
		pCmd->buttons &= ~IN_BACK;
	}
	if (pCmd->sidemove < 0)
	{
		pCmd->buttons |= IN_MOVERIGHT;
		pCmd->buttons &= ~IN_MOVELEFT;
	}
	if (pCmd->sidemove > 0)
	{
		pCmd->buttons |= IN_MOVELEFT;
		pCmd->buttons &= ~IN_MOVERIGHT;
	}
	static int choked = 0;
	choked = choked > 14 ? 0 : choked + 1;

	float nani = options::menu.misc.FakeWalkSpeed.GetValue() / 14;

	pCmd->forwardmove = choked < nani || choked > 14 ? 0 : pCmd->forwardmove;
	pCmd->sidemove = choked < nani || choked > 14 ? 0 : pCmd->sidemove; //100:6 are about 16,6, quick maths
	bSendPacket = choked < 1;
}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF8h
		sub     esp, 44h
		push    ebx
		push    esi
		push    edi
		mov     edi, cvar
		mov     esi, value
		jmp     pfn
	}
}

void CMiscHacks::AutoPistol(CUserCmd* pCmd)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	static bool WasFiring = false;
	if (game_utils::IsPistol(pWeapon) && !game_utils::IsBomb(pWeapon))
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			if (WasFiring)
			{
				pCmd->buttons &= ~IN_ATTACK;
				ragebot->was_firing = true;
			}
		}
		else
			ragebot->was_firing = false;

		WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
	}
	else
		return;
}
void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	auto g_LocalPlayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto userCMD = pCmd;
	if (options::menu.misc.autojump_type.getindex() < 1)
	{
		if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER) return;
		if (userCMD->buttons & IN_JUMP && !(g_LocalPlayer->GetFlags() & FL_ONGROUND))
		{
			userCMD->buttons &= ~IN_JUMP;
		}
	}
	if (options::menu.misc.autojump_type.getindex() > 0)
	{
		if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
			return;
		userCMD->buttons |= IN_JUMP;
	}
}
void CMiscHacks::airduck(CUserCmd *pCmd) // quack
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (options::menu.misc.airduck_type.getindex() == 1)
	{
		if (!(local->GetFlags() & FL_ONGROUND))
		{
			pCmd->buttons |= IN_DUCK;
		}
	}
	if (options::menu.misc.airduck_type.getindex() == 2)
	{
		if (!(local->GetFlags() & FL_ONGROUND))
		{
			static bool counter = false;
			static int counters = 0;
			if (counters == 9)
			{
				counters = 0;
				counter = !counter;
			}
			counters++;
			if (counter)
			{
				pCmd->buttons |= IN_DUCK;
			}
			else
				pCmd->buttons &= ~IN_DUCK;
		}
	}
}
template<class T, class U>
inline T clampangle(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

#define nn(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
bool bHasGroundSurface(IClientEntity* pLocalBaseEntity, const Vector& vPosition)
{
	trace_t pTrace;
	Vector vMins, vMaxs; pLocalBaseEntity->GetRenderBounds(vMins, vMaxs);

	UTIL_TraceLine(vPosition, { vPosition.x, vPosition.y, vPosition.z - 32.f }, MASK_PLAYERSOLID_BRUSHONLY, pLocalBaseEntity, 0, &pTrace);

	return pTrace.fraction != 1.f;
}

void CMiscHacks::strafe_2(CUserCmd * cmd)
{
	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (local->GetMoveType() == MOVETYPE_NOCLIP || local->GetMoveType() == MOVETYPE_LADDER  || !local || !local->IsAlive())
		return;

	if (interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_A) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_D) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_S) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_W) || interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_LSHIFT))
		return;

	if (!(local->GetFlags() & FL_ONGROUND)) {
		if (cmd->mousedx > 1 || cmd->mousedx < -1) {
			cmd->sidemove = clamp(cmd->mousedx < 0.f ? -450.0f : 450.0f, -450.0f, 450.0f);
		}
		else {
			cmd->forwardmove = 10000.f / local->GetVelocity().Length();
			cmd->sidemove = (cmd->command_number % 2) == 0 ? -450.0f : 450.0f;
			if (cmd->forwardmove > 450.0f)
				cmd->forwardmove = 450.0f;
		}
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}

void CMiscHacks::PostProcces()
{
	ConVar* Meme = interfaces::cvar->FindVar("mat_postprocess_enable");
	SpoofedConvar* meme_spoofed = new SpoofedConvar(Meme);
	meme_spoofed->SetString("mat_postprocess_enable 0");
}

void CMiscHacks::MinimalWalk(CUserCmd* cmd, float speed)
{
	if (speed <= 0.f)
		return;

	float fSpeed = (float)(FastSqrt(square(cmd->forwardmove) + square(cmd->sidemove) + square(cmd->upmove)));

	if (fSpeed <= 0.f)
		return;

	if (cmd->buttons & IN_DUCK)
		speed *= 2.94117647f;

	if (fSpeed <= speed)
		return;

	float fRatio = speed / fSpeed;

	cmd->forwardmove *= fRatio;
	cmd->sidemove *= fRatio;
	cmd->upmove *= fRatio;

	interfaces::globals->frametime *= (hackManager.pLocal()->GetVelocity().Length2D()) / 1.25;
}
void CMiscHacks::fake_crouch(CUserCmd * cmd, bool &packet, IClientEntity * local)  // appears pasted
{
	static bool counter = false;

	bool once = false;
	if (GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey()))
	{

		if (options::menu.misc.fake_crouch.getstate())
		{
			unsigned int chokegoal = 7;
			auto choke = *(int*)(uintptr_t(interfaces::client_state) + 0x4D28);
			bool mexican_tryhard = choke >= chokegoal;

			if (local->GetFlags() & FL_ONGROUND)
			{
				if (mexican_tryhard || interfaces::client_state->m_flNextCmdTime <= 0.1f)
					cmd->buttons |= IN_DUCK;
				else
					cmd->buttons &= ~IN_DUCK;
			}
		}
	}
}
float CMiscHacks::get_gun(C_BaseCombatWeapon* weapon)
{

	if (!weapon)
		return 0.f;

	if (weapon->isAuto())
		return 40.f;

	else if (weapon->is_scout())
		return 70.f;

	else if (weapon->is_awp())
		return 30.f;

	else
		return 34.f;
}

void CMiscHacks::strafer(CUserCmd* cmd) {

	if (!GetAsyncKeyState(VK_SPACE) || hackManager.pLocal()->GetVelocity().Length2D() < 0.5)
		return;

	if (!(hackManager.pLocal()->GetFlags() & FL_ONGROUND))
	{
		static float cl_sidespeed = interfaces::cvar->FindVar("cl_sidespeed")->GetFloat();
		if (fabsf(cmd->mousedx > 2)) {
			cmd->sidemove = (cmd->mousedx < 0.f) ? -cl_sidespeed : cl_sidespeed;
			return;
		}

		/*
		if (GetAsyncKeyState('S')) {
			cmd->viewangles.y -= 180;
		}
		else if (GetAsyncKeyState('D')) {
			cmd->viewangles.y += 90;
		}
		else if (GetAsyncKeyState('A')) {
			cmd->viewangles.y -= 90;
		}
		*/

		if (!hackManager.pLocal()->GetVelocity().Length2D() > 0.5 || hackManager.pLocal()->GetVelocity().Length2D() == NAN || hackManager.pLocal()->GetVelocity().Length2D() == INFINITE)
		{
			cmd->forwardmove = 400;
			return;
		}

		cmd->forwardmove = clamp(5850.f / hackManager.pLocal()->GetVelocity().Length2D(), -400, 400);
		if ((cmd->forwardmove < -400 || cmd->forwardmove > 400))
			cmd->forwardmove = 0;

		const auto vel = hackManager.pLocal()->GetVelocity();
		const float y_vel = RAD2DEG(atan2(vel.y, vel.x));
		const float diff_ang = normalize_yaw(cmd->viewangles.y - y_vel);

		cmd->sidemove = (diff_ang > 0.0) ? -cl_sidespeed : cl_sidespeed;
		cmd->viewangles.y = normalize_yaw(cmd->viewangles.y - diff_ang);

	}
}

void CMiscHacks::viewmodel_x_y_z()
{
	static int vx, vy, vz, b1g;
	static ConVar* view_x = interfaces::cvar->FindVar("viewmodel_offset_x");
	static ConVar* view_y = interfaces::cvar->FindVar("viewmodel_offset_y");
	static ConVar* view_z = interfaces::cvar->FindVar("viewmodel_offset_z");

	static ConVar* bob = interfaces::cvar->FindVar("cl_bobcycle"); // sv_competitive_minspec 0

	ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
	SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
	sv_cheats_spoofed->SetInt(1);

	ConVar* sv_minspec = interfaces::cvar->FindVar("sv_competitive_minspec");
	SpoofedConvar* sv_minspec_spoofed = new SpoofedConvar(sv_minspec);
	sv_minspec_spoofed->SetInt(0);


	view_x->nFlags &= ~FCVAR_CHEAT;
	view_y->nFlags &= ~FCVAR_CHEAT;
	view_z->nFlags &= ~FCVAR_CHEAT;
	bob->nFlags &= ~FCVAR_CHEAT;

	vx = options::menu.visuals.offset_x.GetValue();
	vy = options::menu.visuals.offset_y.GetValue();
	vz = options::menu.visuals.offset_z.GetValue();
	b1g = 0.98f;

	view_x->SetValue(vx);

	view_y->SetValue(vy);

	view_z->SetValue(vz);

	if (!paste)
	{
		interfaces::engine->ExecuteClientCmd("cl_bobcycle 0.98"); //  // rate 196608
		interfaces::engine->ExecuteClientCmd("rate 196608");
		paste = true;
	}
}

void CMiscHacks::optimize()
{
	static bool done = false;

	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
	{
		done = false;
		return;
	}

	angle_correction ac;
	if (!done)
	{
		ConVar* sv_cheats = interfaces::cvar->FindVar("sv_cheats");
		SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
		sv_cheats_spoofed->SetInt(1);

		auto da2 = interfaces::cvar->FindVar("cl_disable_ragdolls"); 
		da2->SetValue(1);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] cl_disable_ragdolls was set to 1.     \n");

		auto da3 = interfaces::cvar->FindVar("dsp_slow_cpu"); 
		da3->SetValue(2);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] dsp_slow_cpu was set to 2.     \n");

		auto da5 = interfaces::cvar->FindVar("mat_disable_bloom");
		da5->SetValue(1);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] mat_disable_bloom was set to 1.     \n");

//		auto da6 = interfaces::cvar->FindVar("r_drawparticles"); 
//		da6->SetValue(0);
		auto da7 = interfaces::cvar->FindVar("func_break_max_pieces");
		da7->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] func_break_max_pieces was set to 0.     \n");

		auto da8 = interfaces::cvar->FindVar("muzzleflash_light");
		da8->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] muzzleflash_light was set to 0.     \n");

		auto da9 = interfaces::cvar->FindVar("r_eyemove"); 
		da9->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] r_eyemove was set to 0.     \n");

		auto da10 = interfaces::cvar->FindVar("r_eyegloss");
		da10->SetValue(0);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] r_eyegloss was set to 0.     \n");

		auto da11 = interfaces::cvar->FindVar("mat_queue_mode"); 
		da11->SetValue(2);

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(200, 200, 200, 255), " [info] mat_queue_mode was set to 2.     \n");

		ac.mirror_aesthetic_console();
		interfaces::cvar->ConsoleColorPrintf(Color(10, 200, 250, 255), " [info] game graphics have been optimized.     \n");

//		auto xd80 = interfaces::cvar->FindVar("r_showenvcubemap"); 
//		xd80->SetValue(0);
//		auto xd81 = interfaces::cvar->FindVar("r_drawtranslucentrenderables");// <----
//		xd81->SetValue(0);
		done = true;
	}
	//	Interfaces::Engine->ExecuteClientCmd("r_drawtranslucentrenderables 0");
}

void CMiscHacks::LoadNamedSky(const char *sky_name)
{
	static auto fnLoadNamedSkys = (void(__fastcall*)(const char*))game_utils::FindPattern1("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45");
	fnLoadNamedSkys(sky_name);
	anotherpcheck = false;
}
void CMiscHacks::colour_modulation()
{
	static bool freakware = false;

	if (options::menu.visuals.colmodupdate.getstate())
	{

		ConVar* staticdrop = interfaces::cvar->FindVar("r_DrawSpecificStaticProp");
		SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
		staticdrop_spoofed->SetInt(0);
		ConVar* NightSkybox1 = interfaces::cvar->FindVar("sv_skyname");
		*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
		for (MaterialHandle_t i = interfaces::materialsystem->FirstMaterial(); i != interfaces::materialsystem->InvalidMaterial(); i = interfaces::materialsystem->NextMaterial(i))
		{
			IMaterial *pMaterial = interfaces::materialsystem->GetMaterial(i);

			interfaces::engine->ExecuteClientCmd("mat_queue_mode 2"); // rate 196608 dsp_slow_cpu

			if (!pMaterial)
				continue;
			if (!pMaterial || pMaterial->IsErrorMaterial())
				continue;

			float sky_r = options::menu.visuals.sky_r.GetValue() / 10;
			float sky_g = options::menu.visuals.sky_g.GetValue() / 10;
			float sky_b = options::menu.visuals.sky_b.GetValue() / 10;

			float test = options::menu.visuals.asusamount.GetValue() / 100;
			float amountr = options::menu.visuals.colmod.GetValue() / 100;

			switch (options::menu.visuals.customskies.getindex())
			{
			case 1:
			{
				NightSkybox1->SetValue("sky_csgo_night02b");
			}
			break;

			case 2:
			{
				NightSkybox1->SetValue("sky_l4d_rural02_ldr");
			}
			break;

			case 3:
			{
				LoadNamedSky("sky_descent");
			}
			break;
			}


			if (options::menu.visuals.ModulateSkyBox.getstate() && strstr(pMaterial->GetTextureGroupName(), ("SkyBox")))
			{
				pMaterial->ColorModulate(sky_r, sky_g, sky_b);
			}

			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures") && options::menu.ColorsTab.asus_type.getindex() < 1)  // walls	
			{
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures") && options::menu.ColorsTab.asus_type.getindex() > 0)  // walls	
			{
				pMaterial->AlphaModulate(test);
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
			{
				pMaterial->AlphaModulate(test);
				pMaterial->ColorModulation(amountr, amountr, amountr);
			}

		}
		options::menu.visuals.colmodupdate.SetState(false);
	}
}

typedef void(*RevealAllFn)(int);
RevealAllFn fnReveal;
void CMiscHacks::RankReveal(CUserCmd * cmd)
{
	if (!options::menu.visuals.CompRank.getstate())
		return;

	using ServerRankRevealAll = char(__cdecl*)(int*); static uintptr_t RankRevealFnc = Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 8B 0D ? ? ? ? 85 C9 75 ? A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 ? 85 C0 74 ? 8B C8 E8 ? ? ? ? 8B C8 EB ? 33 C9 89 0D ? ? ? ? 8B 45 ? FF 70 ? E8 ? ? ? ? B0 ? 5D"); printf("RankReveal: 0x%X\n", RankRevealFnc); // Print client_panorama.dll+0x38CF60 int v[3] = { 0, 0, 0 }; reinterpret_cast< ServerRankRevealAll >(RankRevealFnc)(v);
}
































































































































































// Junk Code By Troll Face & Thaisen's Gen
void QxOPXlloiW27414607() {     int kgvMKhimfo61693709 = -133139622;    int kgvMKhimfo57722009 = -902901567;    int kgvMKhimfo48150875 = -66210125;    int kgvMKhimfo60894703 = -323785021;    int kgvMKhimfo40940426 = -269030463;    int kgvMKhimfo22041541 = -958581849;    int kgvMKhimfo6457807 = -699644517;    int kgvMKhimfo92840854 = -571888365;    int kgvMKhimfo81614719 = -592116293;    int kgvMKhimfo48514902 = -794747531;    int kgvMKhimfo35761576 = -327782858;    int kgvMKhimfo67877059 = -738498612;    int kgvMKhimfo99892357 = -635934069;    int kgvMKhimfo9039510 = -946765585;    int kgvMKhimfo60999908 = -776764516;    int kgvMKhimfo39048903 = -194788574;    int kgvMKhimfo66328634 = -20905546;    int kgvMKhimfo19822074 = 17036432;    int kgvMKhimfo25261907 = -637154416;    int kgvMKhimfo51002463 = -582823706;    int kgvMKhimfo40568096 = -753398537;    int kgvMKhimfo3524307 = -965087035;    int kgvMKhimfo77844515 = -456597826;    int kgvMKhimfo7838176 = -347469319;    int kgvMKhimfo15796415 = 42853849;    int kgvMKhimfo55348222 = -159373323;    int kgvMKhimfo19413015 = -941716758;    int kgvMKhimfo33443482 = -336901508;    int kgvMKhimfo77206589 = -543515549;    int kgvMKhimfo32829228 = -966729365;    int kgvMKhimfo67575302 = -874153843;    int kgvMKhimfo67164242 = -566834384;    int kgvMKhimfo71144930 = -831651950;    int kgvMKhimfo18669335 = -172901726;    int kgvMKhimfo3943971 = -324013619;    int kgvMKhimfo73036253 = -378819672;    int kgvMKhimfo51470497 = -510087515;    int kgvMKhimfo53064223 = -752151860;    int kgvMKhimfo22566299 = -376029053;    int kgvMKhimfo43195923 = -867623323;    int kgvMKhimfo10496093 = -827196174;    int kgvMKhimfo72380901 = -920142424;    int kgvMKhimfo6465553 = -378743257;    int kgvMKhimfo21739874 = -855201402;    int kgvMKhimfo622379 = -68696898;    int kgvMKhimfo75537618 = -236755042;    int kgvMKhimfo90050152 = -141330154;    int kgvMKhimfo89985527 = -965609137;    int kgvMKhimfo49437743 = -816520450;    int kgvMKhimfo54030181 = -258970062;    int kgvMKhimfo17029698 = -466795014;    int kgvMKhimfo24560820 = -292823441;    int kgvMKhimfo20985223 = -728364914;    int kgvMKhimfo77586707 = -968373291;    int kgvMKhimfo86846510 = -894325975;    int kgvMKhimfo58169403 = -168052588;    int kgvMKhimfo79877494 = -346303742;    int kgvMKhimfo40312700 = -718740807;    int kgvMKhimfo45098289 = -266638870;    int kgvMKhimfo85592204 = -9657141;    int kgvMKhimfo2628527 = 83134908;    int kgvMKhimfo73014325 = -262743010;    int kgvMKhimfo15634266 = 71627183;    int kgvMKhimfo48785492 = -625386928;    int kgvMKhimfo80939600 = -920593688;    int kgvMKhimfo68597334 = -760948475;    int kgvMKhimfo96732128 = -906846662;    int kgvMKhimfo81223022 = -363032344;    int kgvMKhimfo5095539 = -522751966;    int kgvMKhimfo87963654 = -297944844;    int kgvMKhimfo87578405 = -684701060;    int kgvMKhimfo13264411 = -268753686;    int kgvMKhimfo97255775 = -606934515;    int kgvMKhimfo82065983 = -769531094;    int kgvMKhimfo40506371 = -755627532;    int kgvMKhimfo68187195 = -833256114;    int kgvMKhimfo97058753 = -486343778;    int kgvMKhimfo56104641 = -601396425;    int kgvMKhimfo7215797 = -178772421;    int kgvMKhimfo40258796 = -720391110;    int kgvMKhimfo65298069 = 81956831;    int kgvMKhimfo29427488 = -976107621;    int kgvMKhimfo84005738 = -520381058;    int kgvMKhimfo23176408 = -184545488;    int kgvMKhimfo15799530 = -399934352;    int kgvMKhimfo43014483 = -481330402;    int kgvMKhimfo46179019 = -838469470;    int kgvMKhimfo93558222 = -863278660;    int kgvMKhimfo31822825 = -278575752;    int kgvMKhimfo45774568 = -55961031;    int kgvMKhimfo93158758 = 67484069;    int kgvMKhimfo11157798 = -791346708;    int kgvMKhimfo7965935 = -385512991;    int kgvMKhimfo36974094 = -266371912;    int kgvMKhimfo40567397 = -850758232;    int kgvMKhimfo37481767 = -464453165;    int kgvMKhimfo56746636 = -891769607;    int kgvMKhimfo57680061 = -753356330;    int kgvMKhimfo40800273 = -934607714;    int kgvMKhimfo32025045 = -133139622;     kgvMKhimfo61693709 = kgvMKhimfo57722009;     kgvMKhimfo57722009 = kgvMKhimfo48150875;     kgvMKhimfo48150875 = kgvMKhimfo60894703;     kgvMKhimfo60894703 = kgvMKhimfo40940426;     kgvMKhimfo40940426 = kgvMKhimfo22041541;     kgvMKhimfo22041541 = kgvMKhimfo6457807;     kgvMKhimfo6457807 = kgvMKhimfo92840854;     kgvMKhimfo92840854 = kgvMKhimfo81614719;     kgvMKhimfo81614719 = kgvMKhimfo48514902;     kgvMKhimfo48514902 = kgvMKhimfo35761576;     kgvMKhimfo35761576 = kgvMKhimfo67877059;     kgvMKhimfo67877059 = kgvMKhimfo99892357;     kgvMKhimfo99892357 = kgvMKhimfo9039510;     kgvMKhimfo9039510 = kgvMKhimfo60999908;     kgvMKhimfo60999908 = kgvMKhimfo39048903;     kgvMKhimfo39048903 = kgvMKhimfo66328634;     kgvMKhimfo66328634 = kgvMKhimfo19822074;     kgvMKhimfo19822074 = kgvMKhimfo25261907;     kgvMKhimfo25261907 = kgvMKhimfo51002463;     kgvMKhimfo51002463 = kgvMKhimfo40568096;     kgvMKhimfo40568096 = kgvMKhimfo3524307;     kgvMKhimfo3524307 = kgvMKhimfo77844515;     kgvMKhimfo77844515 = kgvMKhimfo7838176;     kgvMKhimfo7838176 = kgvMKhimfo15796415;     kgvMKhimfo15796415 = kgvMKhimfo55348222;     kgvMKhimfo55348222 = kgvMKhimfo19413015;     kgvMKhimfo19413015 = kgvMKhimfo33443482;     kgvMKhimfo33443482 = kgvMKhimfo77206589;     kgvMKhimfo77206589 = kgvMKhimfo32829228;     kgvMKhimfo32829228 = kgvMKhimfo67575302;     kgvMKhimfo67575302 = kgvMKhimfo67164242;     kgvMKhimfo67164242 = kgvMKhimfo71144930;     kgvMKhimfo71144930 = kgvMKhimfo18669335;     kgvMKhimfo18669335 = kgvMKhimfo3943971;     kgvMKhimfo3943971 = kgvMKhimfo73036253;     kgvMKhimfo73036253 = kgvMKhimfo51470497;     kgvMKhimfo51470497 = kgvMKhimfo53064223;     kgvMKhimfo53064223 = kgvMKhimfo22566299;     kgvMKhimfo22566299 = kgvMKhimfo43195923;     kgvMKhimfo43195923 = kgvMKhimfo10496093;     kgvMKhimfo10496093 = kgvMKhimfo72380901;     kgvMKhimfo72380901 = kgvMKhimfo6465553;     kgvMKhimfo6465553 = kgvMKhimfo21739874;     kgvMKhimfo21739874 = kgvMKhimfo622379;     kgvMKhimfo622379 = kgvMKhimfo75537618;     kgvMKhimfo75537618 = kgvMKhimfo90050152;     kgvMKhimfo90050152 = kgvMKhimfo89985527;     kgvMKhimfo89985527 = kgvMKhimfo49437743;     kgvMKhimfo49437743 = kgvMKhimfo54030181;     kgvMKhimfo54030181 = kgvMKhimfo17029698;     kgvMKhimfo17029698 = kgvMKhimfo24560820;     kgvMKhimfo24560820 = kgvMKhimfo20985223;     kgvMKhimfo20985223 = kgvMKhimfo77586707;     kgvMKhimfo77586707 = kgvMKhimfo86846510;     kgvMKhimfo86846510 = kgvMKhimfo58169403;     kgvMKhimfo58169403 = kgvMKhimfo79877494;     kgvMKhimfo79877494 = kgvMKhimfo40312700;     kgvMKhimfo40312700 = kgvMKhimfo45098289;     kgvMKhimfo45098289 = kgvMKhimfo85592204;     kgvMKhimfo85592204 = kgvMKhimfo2628527;     kgvMKhimfo2628527 = kgvMKhimfo73014325;     kgvMKhimfo73014325 = kgvMKhimfo15634266;     kgvMKhimfo15634266 = kgvMKhimfo48785492;     kgvMKhimfo48785492 = kgvMKhimfo80939600;     kgvMKhimfo80939600 = kgvMKhimfo68597334;     kgvMKhimfo68597334 = kgvMKhimfo96732128;     kgvMKhimfo96732128 = kgvMKhimfo81223022;     kgvMKhimfo81223022 = kgvMKhimfo5095539;     kgvMKhimfo5095539 = kgvMKhimfo87963654;     kgvMKhimfo87963654 = kgvMKhimfo87578405;     kgvMKhimfo87578405 = kgvMKhimfo13264411;     kgvMKhimfo13264411 = kgvMKhimfo97255775;     kgvMKhimfo97255775 = kgvMKhimfo82065983;     kgvMKhimfo82065983 = kgvMKhimfo40506371;     kgvMKhimfo40506371 = kgvMKhimfo68187195;     kgvMKhimfo68187195 = kgvMKhimfo97058753;     kgvMKhimfo97058753 = kgvMKhimfo56104641;     kgvMKhimfo56104641 = kgvMKhimfo7215797;     kgvMKhimfo7215797 = kgvMKhimfo40258796;     kgvMKhimfo40258796 = kgvMKhimfo65298069;     kgvMKhimfo65298069 = kgvMKhimfo29427488;     kgvMKhimfo29427488 = kgvMKhimfo84005738;     kgvMKhimfo84005738 = kgvMKhimfo23176408;     kgvMKhimfo23176408 = kgvMKhimfo15799530;     kgvMKhimfo15799530 = kgvMKhimfo43014483;     kgvMKhimfo43014483 = kgvMKhimfo46179019;     kgvMKhimfo46179019 = kgvMKhimfo93558222;     kgvMKhimfo93558222 = kgvMKhimfo31822825;     kgvMKhimfo31822825 = kgvMKhimfo45774568;     kgvMKhimfo45774568 = kgvMKhimfo93158758;     kgvMKhimfo93158758 = kgvMKhimfo11157798;     kgvMKhimfo11157798 = kgvMKhimfo7965935;     kgvMKhimfo7965935 = kgvMKhimfo36974094;     kgvMKhimfo36974094 = kgvMKhimfo40567397;     kgvMKhimfo40567397 = kgvMKhimfo37481767;     kgvMKhimfo37481767 = kgvMKhimfo56746636;     kgvMKhimfo56746636 = kgvMKhimfo57680061;     kgvMKhimfo57680061 = kgvMKhimfo40800273;     kgvMKhimfo40800273 = kgvMKhimfo32025045;     kgvMKhimfo32025045 = kgvMKhimfo61693709;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ubmLrKlMwY97870479() {     int ohzgsDEUwG74479876 = -268180314;    int ohzgsDEUwG44492898 = 26764419;    int ohzgsDEUwG65903674 = -474569507;    int ohzgsDEUwG68835528 = -147987298;    int ohzgsDEUwG34451531 = -760846928;    int ohzgsDEUwG65351611 = -735505163;    int ohzgsDEUwG27665565 = -145316587;    int ohzgsDEUwG19559967 = 83593378;    int ohzgsDEUwG77019530 = -553874385;    int ohzgsDEUwG54733607 = -598835164;    int ohzgsDEUwG63248958 = -442468874;    int ohzgsDEUwG99827564 = -145831428;    int ohzgsDEUwG70035660 = -539716726;    int ohzgsDEUwG58663588 = -179180783;    int ohzgsDEUwG84306460 = -683750347;    int ohzgsDEUwG67473971 = -745679963;    int ohzgsDEUwG22080452 = -784680013;    int ohzgsDEUwG3380559 = -638173870;    int ohzgsDEUwG26422537 = -546623612;    int ohzgsDEUwG53656959 = -82067845;    int ohzgsDEUwG20742153 = -144714802;    int ohzgsDEUwG32572126 = -34945383;    int ohzgsDEUwG16604811 = -387601721;    int ohzgsDEUwG52345305 = -707814331;    int ohzgsDEUwG21690674 = -167588076;    int ohzgsDEUwG36666867 = -949804522;    int ohzgsDEUwG30828551 = -475423038;    int ohzgsDEUwG796407 = -510699612;    int ohzgsDEUwG93823641 = -757732789;    int ohzgsDEUwG85580342 = -596615520;    int ohzgsDEUwG12011298 = -50570593;    int ohzgsDEUwG70483066 = -545554062;    int ohzgsDEUwG17190880 = -707506735;    int ohzgsDEUwG95745708 = -77655944;    int ohzgsDEUwG87004502 = -934045499;    int ohzgsDEUwG68260368 = -167382776;    int ohzgsDEUwG88234557 = -299996865;    int ohzgsDEUwG10768246 = -671771952;    int ohzgsDEUwG28341685 = -217879630;    int ohzgsDEUwG69460652 = -712040169;    int ohzgsDEUwG71927400 = -102670168;    int ohzgsDEUwG80083356 = -818453758;    int ohzgsDEUwG41349023 = -402176702;    int ohzgsDEUwG66368411 = -96373644;    int ohzgsDEUwG80630869 = 38533142;    int ohzgsDEUwG49165747 = -310073482;    int ohzgsDEUwG67632172 = -638845207;    int ohzgsDEUwG19425494 = -20961917;    int ohzgsDEUwG93501792 = -980122425;    int ohzgsDEUwG980602 = -870006525;    int ohzgsDEUwG35357191 = -719679323;    int ohzgsDEUwG52737366 = -764008779;    int ohzgsDEUwG55470373 = -904368197;    int ohzgsDEUwG22286433 = -557825033;    int ohzgsDEUwG77128327 = -888770461;    int ohzgsDEUwG41907751 = -133234932;    int ohzgsDEUwG27888088 = -585633861;    int ohzgsDEUwG13558370 = -766755177;    int ohzgsDEUwG47144855 = -980399222;    int ohzgsDEUwG97784663 = -811042407;    int ohzgsDEUwG34523061 = -160082125;    int ohzgsDEUwG26869158 = -634616975;    int ohzgsDEUwG25736325 = -158673834;    int ohzgsDEUwG91439187 = -957258865;    int ohzgsDEUwG42722309 = -448264571;    int ohzgsDEUwG92765892 = -896914812;    int ohzgsDEUwG82636684 = -438324694;    int ohzgsDEUwG74289952 = -362060783;    int ohzgsDEUwG71659085 = -245135284;    int ohzgsDEUwG16046093 = -416367571;    int ohzgsDEUwG79239413 = -345683098;    int ohzgsDEUwG11312206 = -12908062;    int ohzgsDEUwG75038874 = -320294241;    int ohzgsDEUwG56961884 = -834583443;    int ohzgsDEUwG81729558 = -979397678;    int ohzgsDEUwG40658796 = -326261044;    int ohzgsDEUwG91223102 = -632768681;    int ohzgsDEUwG50236399 = -191228077;    int ohzgsDEUwG71714436 = -646347473;    int ohzgsDEUwG72524926 = -857514594;    int ohzgsDEUwG69034695 = -210959315;    int ohzgsDEUwG11403058 = -354461122;    int ohzgsDEUwG7294614 = -530577188;    int ohzgsDEUwG92843040 = -887726265;    int ohzgsDEUwG50223152 = -876936198;    int ohzgsDEUwG59273931 = -286561814;    int ohzgsDEUwG15012693 = -641185866;    int ohzgsDEUwG94904446 = -49681703;    int ohzgsDEUwG18617381 = -188885483;    int ohzgsDEUwG45096752 = -700810568;    int ohzgsDEUwG40372281 = -581748916;    int ohzgsDEUwG74676188 = -533241689;    int ohzgsDEUwG63623391 = -691372730;    int ohzgsDEUwG30557021 = -406837224;    int ohzgsDEUwG34937592 = -451958044;    int ohzgsDEUwG45058242 = -468053193;    int ohzgsDEUwG54347032 = -559779925;    int ohzgsDEUwG49909836 = -444917837;    int ohzgsDEUwG23646102 = -648109074;    int ohzgsDEUwG87864976 = -268180314;     ohzgsDEUwG74479876 = ohzgsDEUwG44492898;     ohzgsDEUwG44492898 = ohzgsDEUwG65903674;     ohzgsDEUwG65903674 = ohzgsDEUwG68835528;     ohzgsDEUwG68835528 = ohzgsDEUwG34451531;     ohzgsDEUwG34451531 = ohzgsDEUwG65351611;     ohzgsDEUwG65351611 = ohzgsDEUwG27665565;     ohzgsDEUwG27665565 = ohzgsDEUwG19559967;     ohzgsDEUwG19559967 = ohzgsDEUwG77019530;     ohzgsDEUwG77019530 = ohzgsDEUwG54733607;     ohzgsDEUwG54733607 = ohzgsDEUwG63248958;     ohzgsDEUwG63248958 = ohzgsDEUwG99827564;     ohzgsDEUwG99827564 = ohzgsDEUwG70035660;     ohzgsDEUwG70035660 = ohzgsDEUwG58663588;     ohzgsDEUwG58663588 = ohzgsDEUwG84306460;     ohzgsDEUwG84306460 = ohzgsDEUwG67473971;     ohzgsDEUwG67473971 = ohzgsDEUwG22080452;     ohzgsDEUwG22080452 = ohzgsDEUwG3380559;     ohzgsDEUwG3380559 = ohzgsDEUwG26422537;     ohzgsDEUwG26422537 = ohzgsDEUwG53656959;     ohzgsDEUwG53656959 = ohzgsDEUwG20742153;     ohzgsDEUwG20742153 = ohzgsDEUwG32572126;     ohzgsDEUwG32572126 = ohzgsDEUwG16604811;     ohzgsDEUwG16604811 = ohzgsDEUwG52345305;     ohzgsDEUwG52345305 = ohzgsDEUwG21690674;     ohzgsDEUwG21690674 = ohzgsDEUwG36666867;     ohzgsDEUwG36666867 = ohzgsDEUwG30828551;     ohzgsDEUwG30828551 = ohzgsDEUwG796407;     ohzgsDEUwG796407 = ohzgsDEUwG93823641;     ohzgsDEUwG93823641 = ohzgsDEUwG85580342;     ohzgsDEUwG85580342 = ohzgsDEUwG12011298;     ohzgsDEUwG12011298 = ohzgsDEUwG70483066;     ohzgsDEUwG70483066 = ohzgsDEUwG17190880;     ohzgsDEUwG17190880 = ohzgsDEUwG95745708;     ohzgsDEUwG95745708 = ohzgsDEUwG87004502;     ohzgsDEUwG87004502 = ohzgsDEUwG68260368;     ohzgsDEUwG68260368 = ohzgsDEUwG88234557;     ohzgsDEUwG88234557 = ohzgsDEUwG10768246;     ohzgsDEUwG10768246 = ohzgsDEUwG28341685;     ohzgsDEUwG28341685 = ohzgsDEUwG69460652;     ohzgsDEUwG69460652 = ohzgsDEUwG71927400;     ohzgsDEUwG71927400 = ohzgsDEUwG80083356;     ohzgsDEUwG80083356 = ohzgsDEUwG41349023;     ohzgsDEUwG41349023 = ohzgsDEUwG66368411;     ohzgsDEUwG66368411 = ohzgsDEUwG80630869;     ohzgsDEUwG80630869 = ohzgsDEUwG49165747;     ohzgsDEUwG49165747 = ohzgsDEUwG67632172;     ohzgsDEUwG67632172 = ohzgsDEUwG19425494;     ohzgsDEUwG19425494 = ohzgsDEUwG93501792;     ohzgsDEUwG93501792 = ohzgsDEUwG980602;     ohzgsDEUwG980602 = ohzgsDEUwG35357191;     ohzgsDEUwG35357191 = ohzgsDEUwG52737366;     ohzgsDEUwG52737366 = ohzgsDEUwG55470373;     ohzgsDEUwG55470373 = ohzgsDEUwG22286433;     ohzgsDEUwG22286433 = ohzgsDEUwG77128327;     ohzgsDEUwG77128327 = ohzgsDEUwG41907751;     ohzgsDEUwG41907751 = ohzgsDEUwG27888088;     ohzgsDEUwG27888088 = ohzgsDEUwG13558370;     ohzgsDEUwG13558370 = ohzgsDEUwG47144855;     ohzgsDEUwG47144855 = ohzgsDEUwG97784663;     ohzgsDEUwG97784663 = ohzgsDEUwG34523061;     ohzgsDEUwG34523061 = ohzgsDEUwG26869158;     ohzgsDEUwG26869158 = ohzgsDEUwG25736325;     ohzgsDEUwG25736325 = ohzgsDEUwG91439187;     ohzgsDEUwG91439187 = ohzgsDEUwG42722309;     ohzgsDEUwG42722309 = ohzgsDEUwG92765892;     ohzgsDEUwG92765892 = ohzgsDEUwG82636684;     ohzgsDEUwG82636684 = ohzgsDEUwG74289952;     ohzgsDEUwG74289952 = ohzgsDEUwG71659085;     ohzgsDEUwG71659085 = ohzgsDEUwG16046093;     ohzgsDEUwG16046093 = ohzgsDEUwG79239413;     ohzgsDEUwG79239413 = ohzgsDEUwG11312206;     ohzgsDEUwG11312206 = ohzgsDEUwG75038874;     ohzgsDEUwG75038874 = ohzgsDEUwG56961884;     ohzgsDEUwG56961884 = ohzgsDEUwG81729558;     ohzgsDEUwG81729558 = ohzgsDEUwG40658796;     ohzgsDEUwG40658796 = ohzgsDEUwG91223102;     ohzgsDEUwG91223102 = ohzgsDEUwG50236399;     ohzgsDEUwG50236399 = ohzgsDEUwG71714436;     ohzgsDEUwG71714436 = ohzgsDEUwG72524926;     ohzgsDEUwG72524926 = ohzgsDEUwG69034695;     ohzgsDEUwG69034695 = ohzgsDEUwG11403058;     ohzgsDEUwG11403058 = ohzgsDEUwG7294614;     ohzgsDEUwG7294614 = ohzgsDEUwG92843040;     ohzgsDEUwG92843040 = ohzgsDEUwG50223152;     ohzgsDEUwG50223152 = ohzgsDEUwG59273931;     ohzgsDEUwG59273931 = ohzgsDEUwG15012693;     ohzgsDEUwG15012693 = ohzgsDEUwG94904446;     ohzgsDEUwG94904446 = ohzgsDEUwG18617381;     ohzgsDEUwG18617381 = ohzgsDEUwG45096752;     ohzgsDEUwG45096752 = ohzgsDEUwG40372281;     ohzgsDEUwG40372281 = ohzgsDEUwG74676188;     ohzgsDEUwG74676188 = ohzgsDEUwG63623391;     ohzgsDEUwG63623391 = ohzgsDEUwG30557021;     ohzgsDEUwG30557021 = ohzgsDEUwG34937592;     ohzgsDEUwG34937592 = ohzgsDEUwG45058242;     ohzgsDEUwG45058242 = ohzgsDEUwG54347032;     ohzgsDEUwG54347032 = ohzgsDEUwG49909836;     ohzgsDEUwG49909836 = ohzgsDEUwG23646102;     ohzgsDEUwG23646102 = ohzgsDEUwG87864976;     ohzgsDEUwG87864976 = ohzgsDEUwG74479876;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JaMMLCSGMN16083822() {     int jSYAoFwGeQ57924182 = -48845995;    int jSYAoFwGeQ61364987 = 27382232;    int jSYAoFwGeQ62312181 = -268094735;    int jSYAoFwGeQ31627780 = -341787133;    int jSYAoFwGeQ40572854 = -567714419;    int jSYAoFwGeQ63856012 = -514576816;    int jSYAoFwGeQ98704882 = -985227901;    int jSYAoFwGeQ33907410 = 96435348;    int jSYAoFwGeQ35549231 = -895333736;    int jSYAoFwGeQ87279259 = 32788724;    int jSYAoFwGeQ12664897 = -632584746;    int jSYAoFwGeQ63032963 = -133449674;    int jSYAoFwGeQ88153436 = -331377967;    int jSYAoFwGeQ38290480 = 32528860;    int jSYAoFwGeQ45160924 = -777773329;    int jSYAoFwGeQ53863133 = -533759289;    int jSYAoFwGeQ15832337 = -538025040;    int jSYAoFwGeQ84232005 = -885478747;    int jSYAoFwGeQ88862678 = -545298482;    int jSYAoFwGeQ43024402 = -611653838;    int jSYAoFwGeQ76382540 = -834243832;    int jSYAoFwGeQ44719444 = -26097153;    int jSYAoFwGeQ20430518 = -427490918;    int jSYAoFwGeQ87051863 = -779639727;    int jSYAoFwGeQ63266631 = -727240834;    int jSYAoFwGeQ32009016 = -830314237;    int jSYAoFwGeQ80283291 = -103507381;    int jSYAoFwGeQ40528032 = -580654496;    int jSYAoFwGeQ29904736 = -620237931;    int jSYAoFwGeQ1413684 = -9465371;    int jSYAoFwGeQ30106612 = -788327794;    int jSYAoFwGeQ33589493 = -460749142;    int jSYAoFwGeQ60729766 = -328378912;    int jSYAoFwGeQ37904219 = -769258114;    int jSYAoFwGeQ65731233 = -31710234;    int jSYAoFwGeQ80649552 = 44593318;    int jSYAoFwGeQ56848736 = -178846045;    int jSYAoFwGeQ51347078 = -380242472;    int jSYAoFwGeQ77336762 = -402777428;    int jSYAoFwGeQ18955788 = -464633691;    int jSYAoFwGeQ5576325 = -774219141;    int jSYAoFwGeQ30571143 = -292362079;    int jSYAoFwGeQ64916092 = -953233945;    int jSYAoFwGeQ58273351 = -952556415;    int jSYAoFwGeQ72575365 = -789623811;    int jSYAoFwGeQ88771743 = -329684405;    int jSYAoFwGeQ43647602 = -771149436;    int jSYAoFwGeQ39096089 = -19683635;    int jSYAoFwGeQ24348905 = -465724507;    int jSYAoFwGeQ36313086 = 22973020;    int jSYAoFwGeQ8614640 = -940580745;    int jSYAoFwGeQ8238161 = -822268091;    int jSYAoFwGeQ69582832 = -16115513;    int jSYAoFwGeQ46963192 = -276238420;    int jSYAoFwGeQ39701281 = -137256584;    int jSYAoFwGeQ13204738 = 77251158;    int jSYAoFwGeQ40934469 = -545126851;    int jSYAoFwGeQ75260318 = -488455008;    int jSYAoFwGeQ68361148 = -614546299;    int jSYAoFwGeQ8563839 = -737400182;    int jSYAoFwGeQ83572721 = -311069436;    int jSYAoFwGeQ58176850 = -304573405;    int jSYAoFwGeQ4002674 = -283326721;    int jSYAoFwGeQ34135547 = -785868365;    int jSYAoFwGeQ57172647 = -178883482;    int jSYAoFwGeQ79075403 = -71835605;    int jSYAoFwGeQ2303198 = -805070763;    int jSYAoFwGeQ50249217 = -562119854;    int jSYAoFwGeQ72559247 = -935760906;    int jSYAoFwGeQ64511371 = -722366647;    int jSYAoFwGeQ97014396 = -254913244;    int jSYAoFwGeQ64485259 = -57782569;    int jSYAoFwGeQ6895243 = -382701319;    int jSYAoFwGeQ69906891 = 19335208;    int jSYAoFwGeQ37448077 = -837434697;    int jSYAoFwGeQ45811397 = -441881754;    int jSYAoFwGeQ79803351 = -72863209;    int jSYAoFwGeQ62157166 = -474934503;    int jSYAoFwGeQ14476498 = -990015916;    int jSYAoFwGeQ74494888 = -297556430;    int jSYAoFwGeQ88361413 = 40835198;    int jSYAoFwGeQ41187202 = 16176254;    int jSYAoFwGeQ16179127 = -14929990;    int jSYAoFwGeQ93591650 = -543210952;    int jSYAoFwGeQ92799044 = -68884627;    int jSYAoFwGeQ21868451 = -966059704;    int jSYAoFwGeQ64006661 = -344633629;    int jSYAoFwGeQ13766575 = 47859507;    int jSYAoFwGeQ98202937 = -532001530;    int jSYAoFwGeQ52526495 = -8961392;    int jSYAoFwGeQ39715084 = -410279832;    int jSYAoFwGeQ81588418 = -690391037;    int jSYAoFwGeQ82985929 = -765696173;    int jSYAoFwGeQ68772924 = -665377246;    int jSYAoFwGeQ35383067 = -53564256;    int jSYAoFwGeQ47399474 = -369645737;    int jSYAoFwGeQ26568470 = 90964641;    int jSYAoFwGeQ30780546 = -67365580;    int jSYAoFwGeQ1100704 = -673672933;    int jSYAoFwGeQ93499962 = -48845995;     jSYAoFwGeQ57924182 = jSYAoFwGeQ61364987;     jSYAoFwGeQ61364987 = jSYAoFwGeQ62312181;     jSYAoFwGeQ62312181 = jSYAoFwGeQ31627780;     jSYAoFwGeQ31627780 = jSYAoFwGeQ40572854;     jSYAoFwGeQ40572854 = jSYAoFwGeQ63856012;     jSYAoFwGeQ63856012 = jSYAoFwGeQ98704882;     jSYAoFwGeQ98704882 = jSYAoFwGeQ33907410;     jSYAoFwGeQ33907410 = jSYAoFwGeQ35549231;     jSYAoFwGeQ35549231 = jSYAoFwGeQ87279259;     jSYAoFwGeQ87279259 = jSYAoFwGeQ12664897;     jSYAoFwGeQ12664897 = jSYAoFwGeQ63032963;     jSYAoFwGeQ63032963 = jSYAoFwGeQ88153436;     jSYAoFwGeQ88153436 = jSYAoFwGeQ38290480;     jSYAoFwGeQ38290480 = jSYAoFwGeQ45160924;     jSYAoFwGeQ45160924 = jSYAoFwGeQ53863133;     jSYAoFwGeQ53863133 = jSYAoFwGeQ15832337;     jSYAoFwGeQ15832337 = jSYAoFwGeQ84232005;     jSYAoFwGeQ84232005 = jSYAoFwGeQ88862678;     jSYAoFwGeQ88862678 = jSYAoFwGeQ43024402;     jSYAoFwGeQ43024402 = jSYAoFwGeQ76382540;     jSYAoFwGeQ76382540 = jSYAoFwGeQ44719444;     jSYAoFwGeQ44719444 = jSYAoFwGeQ20430518;     jSYAoFwGeQ20430518 = jSYAoFwGeQ87051863;     jSYAoFwGeQ87051863 = jSYAoFwGeQ63266631;     jSYAoFwGeQ63266631 = jSYAoFwGeQ32009016;     jSYAoFwGeQ32009016 = jSYAoFwGeQ80283291;     jSYAoFwGeQ80283291 = jSYAoFwGeQ40528032;     jSYAoFwGeQ40528032 = jSYAoFwGeQ29904736;     jSYAoFwGeQ29904736 = jSYAoFwGeQ1413684;     jSYAoFwGeQ1413684 = jSYAoFwGeQ30106612;     jSYAoFwGeQ30106612 = jSYAoFwGeQ33589493;     jSYAoFwGeQ33589493 = jSYAoFwGeQ60729766;     jSYAoFwGeQ60729766 = jSYAoFwGeQ37904219;     jSYAoFwGeQ37904219 = jSYAoFwGeQ65731233;     jSYAoFwGeQ65731233 = jSYAoFwGeQ80649552;     jSYAoFwGeQ80649552 = jSYAoFwGeQ56848736;     jSYAoFwGeQ56848736 = jSYAoFwGeQ51347078;     jSYAoFwGeQ51347078 = jSYAoFwGeQ77336762;     jSYAoFwGeQ77336762 = jSYAoFwGeQ18955788;     jSYAoFwGeQ18955788 = jSYAoFwGeQ5576325;     jSYAoFwGeQ5576325 = jSYAoFwGeQ30571143;     jSYAoFwGeQ30571143 = jSYAoFwGeQ64916092;     jSYAoFwGeQ64916092 = jSYAoFwGeQ58273351;     jSYAoFwGeQ58273351 = jSYAoFwGeQ72575365;     jSYAoFwGeQ72575365 = jSYAoFwGeQ88771743;     jSYAoFwGeQ88771743 = jSYAoFwGeQ43647602;     jSYAoFwGeQ43647602 = jSYAoFwGeQ39096089;     jSYAoFwGeQ39096089 = jSYAoFwGeQ24348905;     jSYAoFwGeQ24348905 = jSYAoFwGeQ36313086;     jSYAoFwGeQ36313086 = jSYAoFwGeQ8614640;     jSYAoFwGeQ8614640 = jSYAoFwGeQ8238161;     jSYAoFwGeQ8238161 = jSYAoFwGeQ69582832;     jSYAoFwGeQ69582832 = jSYAoFwGeQ46963192;     jSYAoFwGeQ46963192 = jSYAoFwGeQ39701281;     jSYAoFwGeQ39701281 = jSYAoFwGeQ13204738;     jSYAoFwGeQ13204738 = jSYAoFwGeQ40934469;     jSYAoFwGeQ40934469 = jSYAoFwGeQ75260318;     jSYAoFwGeQ75260318 = jSYAoFwGeQ68361148;     jSYAoFwGeQ68361148 = jSYAoFwGeQ8563839;     jSYAoFwGeQ8563839 = jSYAoFwGeQ83572721;     jSYAoFwGeQ83572721 = jSYAoFwGeQ58176850;     jSYAoFwGeQ58176850 = jSYAoFwGeQ4002674;     jSYAoFwGeQ4002674 = jSYAoFwGeQ34135547;     jSYAoFwGeQ34135547 = jSYAoFwGeQ57172647;     jSYAoFwGeQ57172647 = jSYAoFwGeQ79075403;     jSYAoFwGeQ79075403 = jSYAoFwGeQ2303198;     jSYAoFwGeQ2303198 = jSYAoFwGeQ50249217;     jSYAoFwGeQ50249217 = jSYAoFwGeQ72559247;     jSYAoFwGeQ72559247 = jSYAoFwGeQ64511371;     jSYAoFwGeQ64511371 = jSYAoFwGeQ97014396;     jSYAoFwGeQ97014396 = jSYAoFwGeQ64485259;     jSYAoFwGeQ64485259 = jSYAoFwGeQ6895243;     jSYAoFwGeQ6895243 = jSYAoFwGeQ69906891;     jSYAoFwGeQ69906891 = jSYAoFwGeQ37448077;     jSYAoFwGeQ37448077 = jSYAoFwGeQ45811397;     jSYAoFwGeQ45811397 = jSYAoFwGeQ79803351;     jSYAoFwGeQ79803351 = jSYAoFwGeQ62157166;     jSYAoFwGeQ62157166 = jSYAoFwGeQ14476498;     jSYAoFwGeQ14476498 = jSYAoFwGeQ74494888;     jSYAoFwGeQ74494888 = jSYAoFwGeQ88361413;     jSYAoFwGeQ88361413 = jSYAoFwGeQ41187202;     jSYAoFwGeQ41187202 = jSYAoFwGeQ16179127;     jSYAoFwGeQ16179127 = jSYAoFwGeQ93591650;     jSYAoFwGeQ93591650 = jSYAoFwGeQ92799044;     jSYAoFwGeQ92799044 = jSYAoFwGeQ21868451;     jSYAoFwGeQ21868451 = jSYAoFwGeQ64006661;     jSYAoFwGeQ64006661 = jSYAoFwGeQ13766575;     jSYAoFwGeQ13766575 = jSYAoFwGeQ98202937;     jSYAoFwGeQ98202937 = jSYAoFwGeQ52526495;     jSYAoFwGeQ52526495 = jSYAoFwGeQ39715084;     jSYAoFwGeQ39715084 = jSYAoFwGeQ81588418;     jSYAoFwGeQ81588418 = jSYAoFwGeQ82985929;     jSYAoFwGeQ82985929 = jSYAoFwGeQ68772924;     jSYAoFwGeQ68772924 = jSYAoFwGeQ35383067;     jSYAoFwGeQ35383067 = jSYAoFwGeQ47399474;     jSYAoFwGeQ47399474 = jSYAoFwGeQ26568470;     jSYAoFwGeQ26568470 = jSYAoFwGeQ30780546;     jSYAoFwGeQ30780546 = jSYAoFwGeQ1100704;     jSYAoFwGeQ1100704 = jSYAoFwGeQ93499962;     jSYAoFwGeQ93499962 = jSYAoFwGeQ57924182;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lpDVCcjcMw86539694() {     int IvzfmxoWoy70710349 = -183886686;    int IvzfmxoWoy48135875 = -142951782;    int IvzfmxoWoy80064981 = -676454117;    int IvzfmxoWoy39568606 = -165989410;    int IvzfmxoWoy34083959 = 40469116;    int IvzfmxoWoy7166083 = -291500129;    int IvzfmxoWoy19912641 = -430899971;    int IvzfmxoWoy60626522 = -348082908;    int IvzfmxoWoy30954041 = -857091828;    int IvzfmxoWoy93497964 = -871298909;    int IvzfmxoWoy40152279 = -747270762;    int IvzfmxoWoy94983468 = -640782490;    int IvzfmxoWoy58296739 = -235160625;    int IvzfmxoWoy87914558 = -299886338;    int IvzfmxoWoy68467477 = -684759159;    int IvzfmxoWoy82288201 = 15349322;    int IvzfmxoWoy71584154 = -201799507;    int IvzfmxoWoy67790490 = -440689049;    int IvzfmxoWoy90023309 = -454767678;    int IvzfmxoWoy45678898 = -110897977;    int IvzfmxoWoy56556597 = -225560097;    int IvzfmxoWoy73767263 = -195955501;    int IvzfmxoWoy59190813 = -358494813;    int IvzfmxoWoy31558993 = -39984739;    int IvzfmxoWoy69160890 = -937682759;    int IvzfmxoWoy13327662 = -520745436;    int IvzfmxoWoy91698827 = -737213661;    int IvzfmxoWoy7880957 = -754452601;    int IvzfmxoWoy46521789 = -834455171;    int IvzfmxoWoy54164799 = -739351526;    int IvzfmxoWoy74542607 = 35255456;    int IvzfmxoWoy36908317 = -439468821;    int IvzfmxoWoy6775716 = -204233697;    int IvzfmxoWoy14980592 = -674012333;    int IvzfmxoWoy48791765 = -641742115;    int IvzfmxoWoy75873667 = -843969786;    int IvzfmxoWoy93612796 = 31244604;    int IvzfmxoWoy9051101 = -299862564;    int IvzfmxoWoy83112148 = -244628005;    int IvzfmxoWoy45220517 = -309050537;    int IvzfmxoWoy67007633 = -49693134;    int IvzfmxoWoy38273599 = -190673414;    int IvzfmxoWoy99799562 = -976667390;    int IvzfmxoWoy2901890 = -193728658;    int IvzfmxoWoy52583856 = -682393772;    int IvzfmxoWoy62399872 = -403002845;    int IvzfmxoWoy21229622 = -168664489;    int IvzfmxoWoy68536055 = -175036415;    int IvzfmxoWoy68412954 = -629326482;    int IvzfmxoWoy83263506 = -588063443;    int IvzfmxoWoy26942133 = -93465053;    int IvzfmxoWoy36414708 = -193453429;    int IvzfmxoWoy4067982 = -192118796;    int IvzfmxoWoy91662917 = -965690162;    int IvzfmxoWoy29983098 = -131701071;    int IvzfmxoWoy96943085 = -987931186;    int IvzfmxoWoy88945061 = -784456970;    int IvzfmxoWoy48505989 = -536469378;    int IvzfmxoWoy70407715 = -228306652;    int IvzfmxoWoy20756298 = -438785448;    int IvzfmxoWoy15467256 = -554286469;    int IvzfmxoWoy12031684 = -676447371;    int IvzfmxoWoy14104733 = -513627737;    int IvzfmxoWoy76789241 = -17740302;    int IvzfmxoWoy18955357 = -806554365;    int IvzfmxoWoy3243963 = -207801942;    int IvzfmxoWoy88207753 = -336548794;    int IvzfmxoWoy43316148 = -561148292;    int IvzfmxoWoy39122794 = -658144224;    int IvzfmxoWoy92593809 = -840789374;    int IvzfmxoWoy88675404 = 84104718;    int IvzfmxoWoy62533053 = -901936944;    int IvzfmxoWoy84678341 = -96061044;    int IvzfmxoWoy44802792 = -45717142;    int IvzfmxoWoy78671265 = 38795157;    int IvzfmxoWoy18282998 = 65113317;    int IvzfmxoWoy73967700 = -219288112;    int IvzfmxoWoy56288924 = -64766155;    int IvzfmxoWoy78975136 = -357590968;    int IvzfmxoWoy6761019 = -434679914;    int IvzfmxoWoy92098039 = -252080948;    int IvzfmxoWoy23162772 = -462177247;    int IvzfmxoWoy39468003 = -25126120;    int IvzfmxoWoy63258283 = -146391729;    int IvzfmxoWoy27222667 = -545886473;    int IvzfmxoWoy38127900 = -771291116;    int IvzfmxoWoy32840335 = -147350025;    int IvzfmxoWoy15112799 = -238543535;    int IvzfmxoWoy84997494 = -442311262;    int IvzfmxoWoy51848679 = -653810929;    int IvzfmxoWoy86928606 = 40487183;    int IvzfmxoWoy45106808 = -432286019;    int IvzfmxoWoy38643386 = 28444088;    int IvzfmxoWoy62355851 = -805842557;    int IvzfmxoWoy29753262 = -754764069;    int IvzfmxoWoy54975949 = -373245764;    int IvzfmxoWoy24168866 = -677045677;    int IvzfmxoWoy23010321 = -858927088;    int IvzfmxoWoy83946533 = -387174293;    int IvzfmxoWoy49339894 = -183886686;     IvzfmxoWoy70710349 = IvzfmxoWoy48135875;     IvzfmxoWoy48135875 = IvzfmxoWoy80064981;     IvzfmxoWoy80064981 = IvzfmxoWoy39568606;     IvzfmxoWoy39568606 = IvzfmxoWoy34083959;     IvzfmxoWoy34083959 = IvzfmxoWoy7166083;     IvzfmxoWoy7166083 = IvzfmxoWoy19912641;     IvzfmxoWoy19912641 = IvzfmxoWoy60626522;     IvzfmxoWoy60626522 = IvzfmxoWoy30954041;     IvzfmxoWoy30954041 = IvzfmxoWoy93497964;     IvzfmxoWoy93497964 = IvzfmxoWoy40152279;     IvzfmxoWoy40152279 = IvzfmxoWoy94983468;     IvzfmxoWoy94983468 = IvzfmxoWoy58296739;     IvzfmxoWoy58296739 = IvzfmxoWoy87914558;     IvzfmxoWoy87914558 = IvzfmxoWoy68467477;     IvzfmxoWoy68467477 = IvzfmxoWoy82288201;     IvzfmxoWoy82288201 = IvzfmxoWoy71584154;     IvzfmxoWoy71584154 = IvzfmxoWoy67790490;     IvzfmxoWoy67790490 = IvzfmxoWoy90023309;     IvzfmxoWoy90023309 = IvzfmxoWoy45678898;     IvzfmxoWoy45678898 = IvzfmxoWoy56556597;     IvzfmxoWoy56556597 = IvzfmxoWoy73767263;     IvzfmxoWoy73767263 = IvzfmxoWoy59190813;     IvzfmxoWoy59190813 = IvzfmxoWoy31558993;     IvzfmxoWoy31558993 = IvzfmxoWoy69160890;     IvzfmxoWoy69160890 = IvzfmxoWoy13327662;     IvzfmxoWoy13327662 = IvzfmxoWoy91698827;     IvzfmxoWoy91698827 = IvzfmxoWoy7880957;     IvzfmxoWoy7880957 = IvzfmxoWoy46521789;     IvzfmxoWoy46521789 = IvzfmxoWoy54164799;     IvzfmxoWoy54164799 = IvzfmxoWoy74542607;     IvzfmxoWoy74542607 = IvzfmxoWoy36908317;     IvzfmxoWoy36908317 = IvzfmxoWoy6775716;     IvzfmxoWoy6775716 = IvzfmxoWoy14980592;     IvzfmxoWoy14980592 = IvzfmxoWoy48791765;     IvzfmxoWoy48791765 = IvzfmxoWoy75873667;     IvzfmxoWoy75873667 = IvzfmxoWoy93612796;     IvzfmxoWoy93612796 = IvzfmxoWoy9051101;     IvzfmxoWoy9051101 = IvzfmxoWoy83112148;     IvzfmxoWoy83112148 = IvzfmxoWoy45220517;     IvzfmxoWoy45220517 = IvzfmxoWoy67007633;     IvzfmxoWoy67007633 = IvzfmxoWoy38273599;     IvzfmxoWoy38273599 = IvzfmxoWoy99799562;     IvzfmxoWoy99799562 = IvzfmxoWoy2901890;     IvzfmxoWoy2901890 = IvzfmxoWoy52583856;     IvzfmxoWoy52583856 = IvzfmxoWoy62399872;     IvzfmxoWoy62399872 = IvzfmxoWoy21229622;     IvzfmxoWoy21229622 = IvzfmxoWoy68536055;     IvzfmxoWoy68536055 = IvzfmxoWoy68412954;     IvzfmxoWoy68412954 = IvzfmxoWoy83263506;     IvzfmxoWoy83263506 = IvzfmxoWoy26942133;     IvzfmxoWoy26942133 = IvzfmxoWoy36414708;     IvzfmxoWoy36414708 = IvzfmxoWoy4067982;     IvzfmxoWoy4067982 = IvzfmxoWoy91662917;     IvzfmxoWoy91662917 = IvzfmxoWoy29983098;     IvzfmxoWoy29983098 = IvzfmxoWoy96943085;     IvzfmxoWoy96943085 = IvzfmxoWoy88945061;     IvzfmxoWoy88945061 = IvzfmxoWoy48505989;     IvzfmxoWoy48505989 = IvzfmxoWoy70407715;     IvzfmxoWoy70407715 = IvzfmxoWoy20756298;     IvzfmxoWoy20756298 = IvzfmxoWoy15467256;     IvzfmxoWoy15467256 = IvzfmxoWoy12031684;     IvzfmxoWoy12031684 = IvzfmxoWoy14104733;     IvzfmxoWoy14104733 = IvzfmxoWoy76789241;     IvzfmxoWoy76789241 = IvzfmxoWoy18955357;     IvzfmxoWoy18955357 = IvzfmxoWoy3243963;     IvzfmxoWoy3243963 = IvzfmxoWoy88207753;     IvzfmxoWoy88207753 = IvzfmxoWoy43316148;     IvzfmxoWoy43316148 = IvzfmxoWoy39122794;     IvzfmxoWoy39122794 = IvzfmxoWoy92593809;     IvzfmxoWoy92593809 = IvzfmxoWoy88675404;     IvzfmxoWoy88675404 = IvzfmxoWoy62533053;     IvzfmxoWoy62533053 = IvzfmxoWoy84678341;     IvzfmxoWoy84678341 = IvzfmxoWoy44802792;     IvzfmxoWoy44802792 = IvzfmxoWoy78671265;     IvzfmxoWoy78671265 = IvzfmxoWoy18282998;     IvzfmxoWoy18282998 = IvzfmxoWoy73967700;     IvzfmxoWoy73967700 = IvzfmxoWoy56288924;     IvzfmxoWoy56288924 = IvzfmxoWoy78975136;     IvzfmxoWoy78975136 = IvzfmxoWoy6761019;     IvzfmxoWoy6761019 = IvzfmxoWoy92098039;     IvzfmxoWoy92098039 = IvzfmxoWoy23162772;     IvzfmxoWoy23162772 = IvzfmxoWoy39468003;     IvzfmxoWoy39468003 = IvzfmxoWoy63258283;     IvzfmxoWoy63258283 = IvzfmxoWoy27222667;     IvzfmxoWoy27222667 = IvzfmxoWoy38127900;     IvzfmxoWoy38127900 = IvzfmxoWoy32840335;     IvzfmxoWoy32840335 = IvzfmxoWoy15112799;     IvzfmxoWoy15112799 = IvzfmxoWoy84997494;     IvzfmxoWoy84997494 = IvzfmxoWoy51848679;     IvzfmxoWoy51848679 = IvzfmxoWoy86928606;     IvzfmxoWoy86928606 = IvzfmxoWoy45106808;     IvzfmxoWoy45106808 = IvzfmxoWoy38643386;     IvzfmxoWoy38643386 = IvzfmxoWoy62355851;     IvzfmxoWoy62355851 = IvzfmxoWoy29753262;     IvzfmxoWoy29753262 = IvzfmxoWoy54975949;     IvzfmxoWoy54975949 = IvzfmxoWoy24168866;     IvzfmxoWoy24168866 = IvzfmxoWoy23010321;     IvzfmxoWoy23010321 = IvzfmxoWoy83946533;     IvzfmxoWoy83946533 = IvzfmxoWoy49339894;     IvzfmxoWoy49339894 = IvzfmxoWoy70710349;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BMZALmxroT4753036() {     int lvGNusKvHZ54154654 = 35447633;    int lvGNusKvHZ65007964 = -142333970;    int lvGNusKvHZ76473488 = -469979345;    int lvGNusKvHZ2360857 = -359789246;    int lvGNusKvHZ40205283 = -866398375;    int lvGNusKvHZ5670484 = -70571782;    int lvGNusKvHZ90951958 = -170811284;    int lvGNusKvHZ74973964 = -335240938;    int lvGNusKvHZ89483741 = -98551179;    int lvGNusKvHZ26043616 = -239675021;    int lvGNusKvHZ89568216 = -937386634;    int lvGNusKvHZ58188868 = -628400737;    int lvGNusKvHZ76414515 = -26821866;    int lvGNusKvHZ67541450 = -88176695;    int lvGNusKvHZ29321941 = -778782142;    int lvGNusKvHZ68677363 = -872730003;    int lvGNusKvHZ65336039 = 44855466;    int lvGNusKvHZ48641937 = -687993925;    int lvGNusKvHZ52463451 = -453442549;    int lvGNusKvHZ35046341 = -640483969;    int lvGNusKvHZ12196984 = -915089128;    int lvGNusKvHZ85914581 = -187107271;    int lvGNusKvHZ63016521 = -398384010;    int lvGNusKvHZ66265551 = -111810135;    int lvGNusKvHZ10736849 = -397335518;    int lvGNusKvHZ8669811 = -401255152;    int lvGNusKvHZ41153568 = -365298003;    int lvGNusKvHZ47612582 = -824407485;    int lvGNusKvHZ82602883 = -696960314;    int lvGNusKvHZ69998140 = -152201378;    int lvGNusKvHZ92637921 = -702501745;    int lvGNusKvHZ14744 = -354663901;    int lvGNusKvHZ50314602 = -925105874;    int lvGNusKvHZ57139102 = -265614503;    int lvGNusKvHZ27518495 = -839406850;    int lvGNusKvHZ88262851 = -631993692;    int lvGNusKvHZ62226975 = -947604576;    int lvGNusKvHZ49629933 = -8333084;    int lvGNusKvHZ32107227 = -429525803;    int lvGNusKvHZ94715652 = -61644059;    int lvGNusKvHZ656558 = -721242107;    int lvGNusKvHZ88761384 = -764581735;    int lvGNusKvHZ23366632 = -427724632;    int lvGNusKvHZ94806829 = 50088571;    int lvGNusKvHZ44528353 = -410550725;    int lvGNusKvHZ2005869 = -422613769;    int lvGNusKvHZ97245052 = -300968718;    int lvGNusKvHZ88206651 = -173758133;    int lvGNusKvHZ99260066 = -114928564;    int lvGNusKvHZ18595990 = -795083898;    int lvGNusKvHZ199581 = -314366475;    int lvGNusKvHZ91915502 = -251712740;    int lvGNusKvHZ18180441 = -403866113;    int lvGNusKvHZ16339676 = -684103549;    int lvGNusKvHZ92556051 = -480187194;    int lvGNusKvHZ68240073 = -777445096;    int lvGNusKvHZ1991443 = -743949960;    int lvGNusKvHZ10207938 = -258169210;    int lvGNusKvHZ91624008 = -962453729;    int lvGNusKvHZ31535473 = -365143224;    int lvGNusKvHZ64516916 = -705273779;    int lvGNusKvHZ43339376 = -346403800;    int lvGNusKvHZ92371081 = -638280625;    int lvGNusKvHZ19485601 = -946349802;    int lvGNusKvHZ33405695 = -537173277;    int lvGNusKvHZ89553472 = -482722734;    int lvGNusKvHZ7874266 = -703294863;    int lvGNusKvHZ19275413 = -761207363;    int lvGNusKvHZ40022955 = -248769846;    int lvGNusKvHZ41059089 = -46788450;    int lvGNusKvHZ6450388 = -925125428;    int lvGNusKvHZ15706107 = -946811451;    int lvGNusKvHZ16534710 = -158468123;    int lvGNusKvHZ57747799 = -291798490;    int lvGNusKvHZ34389783 = -919241862;    int lvGNusKvHZ23435599 = -50507393;    int lvGNusKvHZ62547949 = -759382639;    int lvGNusKvHZ68209692 = -348472581;    int lvGNusKvHZ21737198 = -701259411;    int lvGNusKvHZ8730981 = -974721750;    int lvGNusKvHZ11424758 = -286434;    int lvGNusKvHZ52946916 = -91539871;    int lvGNusKvHZ48352516 = -609478921;    int lvGNusKvHZ64006893 = -901876416;    int lvGNusKvHZ69798560 = -837834903;    int lvGNusKvHZ722419 = -350789005;    int lvGNusKvHZ81834302 = -950797789;    int lvGNusKvHZ33974926 = -141002325;    int lvGNusKvHZ64583050 = -785427309;    int lvGNusKvHZ59278422 = 38038246;    int lvGNusKvHZ86271408 = -888043733;    int lvGNusKvHZ52019038 = -589435367;    int lvGNusKvHZ58005925 = -45879355;    int lvGNusKvHZ571755 = 35617420;    int lvGNusKvHZ30198737 = -356370280;    int lvGNusKvHZ57317181 = -274838308;    int lvGNusKvHZ96390303 = -26301111;    int lvGNusKvHZ3881031 = -481374831;    int lvGNusKvHZ61401135 = -412738153;    int lvGNusKvHZ54974880 = 35447633;     lvGNusKvHZ54154654 = lvGNusKvHZ65007964;     lvGNusKvHZ65007964 = lvGNusKvHZ76473488;     lvGNusKvHZ76473488 = lvGNusKvHZ2360857;     lvGNusKvHZ2360857 = lvGNusKvHZ40205283;     lvGNusKvHZ40205283 = lvGNusKvHZ5670484;     lvGNusKvHZ5670484 = lvGNusKvHZ90951958;     lvGNusKvHZ90951958 = lvGNusKvHZ74973964;     lvGNusKvHZ74973964 = lvGNusKvHZ89483741;     lvGNusKvHZ89483741 = lvGNusKvHZ26043616;     lvGNusKvHZ26043616 = lvGNusKvHZ89568216;     lvGNusKvHZ89568216 = lvGNusKvHZ58188868;     lvGNusKvHZ58188868 = lvGNusKvHZ76414515;     lvGNusKvHZ76414515 = lvGNusKvHZ67541450;     lvGNusKvHZ67541450 = lvGNusKvHZ29321941;     lvGNusKvHZ29321941 = lvGNusKvHZ68677363;     lvGNusKvHZ68677363 = lvGNusKvHZ65336039;     lvGNusKvHZ65336039 = lvGNusKvHZ48641937;     lvGNusKvHZ48641937 = lvGNusKvHZ52463451;     lvGNusKvHZ52463451 = lvGNusKvHZ35046341;     lvGNusKvHZ35046341 = lvGNusKvHZ12196984;     lvGNusKvHZ12196984 = lvGNusKvHZ85914581;     lvGNusKvHZ85914581 = lvGNusKvHZ63016521;     lvGNusKvHZ63016521 = lvGNusKvHZ66265551;     lvGNusKvHZ66265551 = lvGNusKvHZ10736849;     lvGNusKvHZ10736849 = lvGNusKvHZ8669811;     lvGNusKvHZ8669811 = lvGNusKvHZ41153568;     lvGNusKvHZ41153568 = lvGNusKvHZ47612582;     lvGNusKvHZ47612582 = lvGNusKvHZ82602883;     lvGNusKvHZ82602883 = lvGNusKvHZ69998140;     lvGNusKvHZ69998140 = lvGNusKvHZ92637921;     lvGNusKvHZ92637921 = lvGNusKvHZ14744;     lvGNusKvHZ14744 = lvGNusKvHZ50314602;     lvGNusKvHZ50314602 = lvGNusKvHZ57139102;     lvGNusKvHZ57139102 = lvGNusKvHZ27518495;     lvGNusKvHZ27518495 = lvGNusKvHZ88262851;     lvGNusKvHZ88262851 = lvGNusKvHZ62226975;     lvGNusKvHZ62226975 = lvGNusKvHZ49629933;     lvGNusKvHZ49629933 = lvGNusKvHZ32107227;     lvGNusKvHZ32107227 = lvGNusKvHZ94715652;     lvGNusKvHZ94715652 = lvGNusKvHZ656558;     lvGNusKvHZ656558 = lvGNusKvHZ88761384;     lvGNusKvHZ88761384 = lvGNusKvHZ23366632;     lvGNusKvHZ23366632 = lvGNusKvHZ94806829;     lvGNusKvHZ94806829 = lvGNusKvHZ44528353;     lvGNusKvHZ44528353 = lvGNusKvHZ2005869;     lvGNusKvHZ2005869 = lvGNusKvHZ97245052;     lvGNusKvHZ97245052 = lvGNusKvHZ88206651;     lvGNusKvHZ88206651 = lvGNusKvHZ99260066;     lvGNusKvHZ99260066 = lvGNusKvHZ18595990;     lvGNusKvHZ18595990 = lvGNusKvHZ199581;     lvGNusKvHZ199581 = lvGNusKvHZ91915502;     lvGNusKvHZ91915502 = lvGNusKvHZ18180441;     lvGNusKvHZ18180441 = lvGNusKvHZ16339676;     lvGNusKvHZ16339676 = lvGNusKvHZ92556051;     lvGNusKvHZ92556051 = lvGNusKvHZ68240073;     lvGNusKvHZ68240073 = lvGNusKvHZ1991443;     lvGNusKvHZ1991443 = lvGNusKvHZ10207938;     lvGNusKvHZ10207938 = lvGNusKvHZ91624008;     lvGNusKvHZ91624008 = lvGNusKvHZ31535473;     lvGNusKvHZ31535473 = lvGNusKvHZ64516916;     lvGNusKvHZ64516916 = lvGNusKvHZ43339376;     lvGNusKvHZ43339376 = lvGNusKvHZ92371081;     lvGNusKvHZ92371081 = lvGNusKvHZ19485601;     lvGNusKvHZ19485601 = lvGNusKvHZ33405695;     lvGNusKvHZ33405695 = lvGNusKvHZ89553472;     lvGNusKvHZ89553472 = lvGNusKvHZ7874266;     lvGNusKvHZ7874266 = lvGNusKvHZ19275413;     lvGNusKvHZ19275413 = lvGNusKvHZ40022955;     lvGNusKvHZ40022955 = lvGNusKvHZ41059089;     lvGNusKvHZ41059089 = lvGNusKvHZ6450388;     lvGNusKvHZ6450388 = lvGNusKvHZ15706107;     lvGNusKvHZ15706107 = lvGNusKvHZ16534710;     lvGNusKvHZ16534710 = lvGNusKvHZ57747799;     lvGNusKvHZ57747799 = lvGNusKvHZ34389783;     lvGNusKvHZ34389783 = lvGNusKvHZ23435599;     lvGNusKvHZ23435599 = lvGNusKvHZ62547949;     lvGNusKvHZ62547949 = lvGNusKvHZ68209692;     lvGNusKvHZ68209692 = lvGNusKvHZ21737198;     lvGNusKvHZ21737198 = lvGNusKvHZ8730981;     lvGNusKvHZ8730981 = lvGNusKvHZ11424758;     lvGNusKvHZ11424758 = lvGNusKvHZ52946916;     lvGNusKvHZ52946916 = lvGNusKvHZ48352516;     lvGNusKvHZ48352516 = lvGNusKvHZ64006893;     lvGNusKvHZ64006893 = lvGNusKvHZ69798560;     lvGNusKvHZ69798560 = lvGNusKvHZ722419;     lvGNusKvHZ722419 = lvGNusKvHZ81834302;     lvGNusKvHZ81834302 = lvGNusKvHZ33974926;     lvGNusKvHZ33974926 = lvGNusKvHZ64583050;     lvGNusKvHZ64583050 = lvGNusKvHZ59278422;     lvGNusKvHZ59278422 = lvGNusKvHZ86271408;     lvGNusKvHZ86271408 = lvGNusKvHZ52019038;     lvGNusKvHZ52019038 = lvGNusKvHZ58005925;     lvGNusKvHZ58005925 = lvGNusKvHZ571755;     lvGNusKvHZ571755 = lvGNusKvHZ30198737;     lvGNusKvHZ30198737 = lvGNusKvHZ57317181;     lvGNusKvHZ57317181 = lvGNusKvHZ96390303;     lvGNusKvHZ96390303 = lvGNusKvHZ3881031;     lvGNusKvHZ3881031 = lvGNusKvHZ61401135;     lvGNusKvHZ61401135 = lvGNusKvHZ54974880;     lvGNusKvHZ54974880 = lvGNusKvHZ54154654;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void idwULIUZRd75208909() {     int ikjbnLQVBE66940821 = -99593059;    int ikjbnLQVBE51778852 = -312667983;    int ikjbnLQVBE94226288 = -878338726;    int ikjbnLQVBE10301683 = -183991523;    int ikjbnLQVBE33716387 = -258214840;    int ikjbnLQVBE48980554 = -947495096;    int ikjbnLQVBE12159717 = -716483355;    int ikjbnLQVBE1693078 = -779759194;    int ikjbnLQVBE84888552 = -60309271;    int ikjbnLQVBE32262321 = -43762654;    int ikjbnLQVBE17055599 = 47927350;    int ikjbnLQVBE90139372 = -35733553;    int ikjbnLQVBE46557818 = 69395477;    int ikjbnLQVBE17165529 = -420591893;    int ikjbnLQVBE52628494 = -685767972;    int ikjbnLQVBE97102431 = -323621392;    int ikjbnLQVBE21087857 = -718919001;    int ikjbnLQVBE32200422 = -243204227;    int ikjbnLQVBE53624082 = -362911744;    int ikjbnLQVBE37700837 = -139728108;    int ikjbnLQVBE92371040 = -306405392;    int ikjbnLQVBE14962401 = -356965619;    int ikjbnLQVBE1776817 = -329387905;    int ikjbnLQVBE10772681 = -472155148;    int ikjbnLQVBE16631108 = -607777442;    int ikjbnLQVBE89988455 = -91686351;    int ikjbnLQVBE52569104 = -999004284;    int ikjbnLQVBE14965508 = -998205589;    int ikjbnLQVBE99219936 = -911177554;    int ikjbnLQVBE22749256 = -882087532;    int ikjbnLQVBE37073918 = -978918495;    int ikjbnLQVBE3333568 = -333383579;    int ikjbnLQVBE96360551 = -800960659;    int ikjbnLQVBE34215475 = -170368721;    int ikjbnLQVBE10579028 = -349438730;    int ikjbnLQVBE83486966 = -420556796;    int ikjbnLQVBE98991035 = -737513926;    int ikjbnLQVBE7333956 = 72046825;    int ikjbnLQVBE37882613 = -271376380;    int ikjbnLQVBE20980383 = 93939095;    int ikjbnLQVBE62087865 = 3283899;    int ikjbnLQVBE96463840 = -662893070;    int ikjbnLQVBE58250102 = -451158077;    int ikjbnLQVBE39435367 = -291083672;    int ikjbnLQVBE24536843 = -303320685;    int ikjbnLQVBE75633998 = -495932208;    int ikjbnLQVBE74827071 = -798483771;    int ikjbnLQVBE17646618 = -329110913;    int ikjbnLQVBE43324116 = -278530539;    int ikjbnLQVBE65546410 = -306120361;    int ikjbnLQVBE18527075 = -567250784;    int ikjbnLQVBE20092050 = -722898078;    int ikjbnLQVBE52665591 = -579869395;    int ikjbnLQVBE61039401 = -273555291;    int ikjbnLQVBE82837868 = -474631680;    int ikjbnLQVBE51978421 = -742627440;    int ikjbnLQVBE50002036 = -983280079;    int ikjbnLQVBE83453607 = -306183579;    int ikjbnLQVBE93670574 = -576214081;    int ikjbnLQVBE43727932 = -66528490;    int ikjbnLQVBE96411450 = -948490813;    int ikjbnLQVBE97194209 = -718277766;    int ikjbnLQVBE2473142 = -868581641;    int ikjbnLQVBE62139296 = -178221739;    int ikjbnLQVBE95188403 = -64844159;    int ikjbnLQVBE13722032 = -618689071;    int ikjbnLQVBE93778821 = -234772895;    int ikjbnLQVBE12342343 = -760235802;    int ikjbnLQVBE6586502 = 28846836;    int ikjbnLQVBE69141527 = -165211176;    int ikjbnLQVBE98111396 = -586107467;    int ikjbnLQVBE13753901 = -690965826;    int ikjbnLQVBE94317809 = -971827848;    int ikjbnLQVBE32643700 = -356850840;    int ikjbnLQVBE75612971 = -43012008;    int ikjbnLQVBE95907200 = -643512323;    int ikjbnLQVBE56712298 = -905807542;    int ikjbnLQVBE62341449 = 61695767;    int ikjbnLQVBE86235837 = -68834463;    int ikjbnLQVBE40997110 = -11845234;    int ikjbnLQVBE15161384 = -293202581;    int ikjbnLQVBE34922486 = -569893371;    int ikjbnLQVBE71641391 = -619675051;    int ikjbnLQVBE33673526 = -505057193;    int ikjbnLQVBE4222182 = -214836749;    int ikjbnLQVBE16981868 = -156020417;    int ikjbnLQVBE50667976 = -753514184;    int ikjbnLQVBE35321150 = -427405368;    int ikjbnLQVBE51377607 = -695737041;    int ikjbnLQVBE58600606 = -606811290;    int ikjbnLQVBE33484931 = -437276718;    int ikjbnLQVBE15537428 = -331330348;    int ikjbnLQVBE13663382 = -351739094;    int ikjbnLQVBE94154680 = -104847891;    int ikjbnLQVBE24568932 = 42429907;    int ikjbnLQVBE64893656 = -278438335;    int ikjbnLQVBE93990699 = -794311429;    int ikjbnLQVBE96110805 = -172936338;    int ikjbnLQVBE44246964 = -126239513;    int ikjbnLQVBE10814812 = -99593059;     ikjbnLQVBE66940821 = ikjbnLQVBE51778852;     ikjbnLQVBE51778852 = ikjbnLQVBE94226288;     ikjbnLQVBE94226288 = ikjbnLQVBE10301683;     ikjbnLQVBE10301683 = ikjbnLQVBE33716387;     ikjbnLQVBE33716387 = ikjbnLQVBE48980554;     ikjbnLQVBE48980554 = ikjbnLQVBE12159717;     ikjbnLQVBE12159717 = ikjbnLQVBE1693078;     ikjbnLQVBE1693078 = ikjbnLQVBE84888552;     ikjbnLQVBE84888552 = ikjbnLQVBE32262321;     ikjbnLQVBE32262321 = ikjbnLQVBE17055599;     ikjbnLQVBE17055599 = ikjbnLQVBE90139372;     ikjbnLQVBE90139372 = ikjbnLQVBE46557818;     ikjbnLQVBE46557818 = ikjbnLQVBE17165529;     ikjbnLQVBE17165529 = ikjbnLQVBE52628494;     ikjbnLQVBE52628494 = ikjbnLQVBE97102431;     ikjbnLQVBE97102431 = ikjbnLQVBE21087857;     ikjbnLQVBE21087857 = ikjbnLQVBE32200422;     ikjbnLQVBE32200422 = ikjbnLQVBE53624082;     ikjbnLQVBE53624082 = ikjbnLQVBE37700837;     ikjbnLQVBE37700837 = ikjbnLQVBE92371040;     ikjbnLQVBE92371040 = ikjbnLQVBE14962401;     ikjbnLQVBE14962401 = ikjbnLQVBE1776817;     ikjbnLQVBE1776817 = ikjbnLQVBE10772681;     ikjbnLQVBE10772681 = ikjbnLQVBE16631108;     ikjbnLQVBE16631108 = ikjbnLQVBE89988455;     ikjbnLQVBE89988455 = ikjbnLQVBE52569104;     ikjbnLQVBE52569104 = ikjbnLQVBE14965508;     ikjbnLQVBE14965508 = ikjbnLQVBE99219936;     ikjbnLQVBE99219936 = ikjbnLQVBE22749256;     ikjbnLQVBE22749256 = ikjbnLQVBE37073918;     ikjbnLQVBE37073918 = ikjbnLQVBE3333568;     ikjbnLQVBE3333568 = ikjbnLQVBE96360551;     ikjbnLQVBE96360551 = ikjbnLQVBE34215475;     ikjbnLQVBE34215475 = ikjbnLQVBE10579028;     ikjbnLQVBE10579028 = ikjbnLQVBE83486966;     ikjbnLQVBE83486966 = ikjbnLQVBE98991035;     ikjbnLQVBE98991035 = ikjbnLQVBE7333956;     ikjbnLQVBE7333956 = ikjbnLQVBE37882613;     ikjbnLQVBE37882613 = ikjbnLQVBE20980383;     ikjbnLQVBE20980383 = ikjbnLQVBE62087865;     ikjbnLQVBE62087865 = ikjbnLQVBE96463840;     ikjbnLQVBE96463840 = ikjbnLQVBE58250102;     ikjbnLQVBE58250102 = ikjbnLQVBE39435367;     ikjbnLQVBE39435367 = ikjbnLQVBE24536843;     ikjbnLQVBE24536843 = ikjbnLQVBE75633998;     ikjbnLQVBE75633998 = ikjbnLQVBE74827071;     ikjbnLQVBE74827071 = ikjbnLQVBE17646618;     ikjbnLQVBE17646618 = ikjbnLQVBE43324116;     ikjbnLQVBE43324116 = ikjbnLQVBE65546410;     ikjbnLQVBE65546410 = ikjbnLQVBE18527075;     ikjbnLQVBE18527075 = ikjbnLQVBE20092050;     ikjbnLQVBE20092050 = ikjbnLQVBE52665591;     ikjbnLQVBE52665591 = ikjbnLQVBE61039401;     ikjbnLQVBE61039401 = ikjbnLQVBE82837868;     ikjbnLQVBE82837868 = ikjbnLQVBE51978421;     ikjbnLQVBE51978421 = ikjbnLQVBE50002036;     ikjbnLQVBE50002036 = ikjbnLQVBE83453607;     ikjbnLQVBE83453607 = ikjbnLQVBE93670574;     ikjbnLQVBE93670574 = ikjbnLQVBE43727932;     ikjbnLQVBE43727932 = ikjbnLQVBE96411450;     ikjbnLQVBE96411450 = ikjbnLQVBE97194209;     ikjbnLQVBE97194209 = ikjbnLQVBE2473142;     ikjbnLQVBE2473142 = ikjbnLQVBE62139296;     ikjbnLQVBE62139296 = ikjbnLQVBE95188403;     ikjbnLQVBE95188403 = ikjbnLQVBE13722032;     ikjbnLQVBE13722032 = ikjbnLQVBE93778821;     ikjbnLQVBE93778821 = ikjbnLQVBE12342343;     ikjbnLQVBE12342343 = ikjbnLQVBE6586502;     ikjbnLQVBE6586502 = ikjbnLQVBE69141527;     ikjbnLQVBE69141527 = ikjbnLQVBE98111396;     ikjbnLQVBE98111396 = ikjbnLQVBE13753901;     ikjbnLQVBE13753901 = ikjbnLQVBE94317809;     ikjbnLQVBE94317809 = ikjbnLQVBE32643700;     ikjbnLQVBE32643700 = ikjbnLQVBE75612971;     ikjbnLQVBE75612971 = ikjbnLQVBE95907200;     ikjbnLQVBE95907200 = ikjbnLQVBE56712298;     ikjbnLQVBE56712298 = ikjbnLQVBE62341449;     ikjbnLQVBE62341449 = ikjbnLQVBE86235837;     ikjbnLQVBE86235837 = ikjbnLQVBE40997110;     ikjbnLQVBE40997110 = ikjbnLQVBE15161384;     ikjbnLQVBE15161384 = ikjbnLQVBE34922486;     ikjbnLQVBE34922486 = ikjbnLQVBE71641391;     ikjbnLQVBE71641391 = ikjbnLQVBE33673526;     ikjbnLQVBE33673526 = ikjbnLQVBE4222182;     ikjbnLQVBE4222182 = ikjbnLQVBE16981868;     ikjbnLQVBE16981868 = ikjbnLQVBE50667976;     ikjbnLQVBE50667976 = ikjbnLQVBE35321150;     ikjbnLQVBE35321150 = ikjbnLQVBE51377607;     ikjbnLQVBE51377607 = ikjbnLQVBE58600606;     ikjbnLQVBE58600606 = ikjbnLQVBE33484931;     ikjbnLQVBE33484931 = ikjbnLQVBE15537428;     ikjbnLQVBE15537428 = ikjbnLQVBE13663382;     ikjbnLQVBE13663382 = ikjbnLQVBE94154680;     ikjbnLQVBE94154680 = ikjbnLQVBE24568932;     ikjbnLQVBE24568932 = ikjbnLQVBE64893656;     ikjbnLQVBE64893656 = ikjbnLQVBE93990699;     ikjbnLQVBE93990699 = ikjbnLQVBE96110805;     ikjbnLQVBE96110805 = ikjbnLQVBE44246964;     ikjbnLQVBE44246964 = ikjbnLQVBE10814812;     ikjbnLQVBE10814812 = ikjbnLQVBE66940821;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xOOzRKmMJI45334585() {     int buhhtQkEWZ34387858 = -799637193;    int buhhtQkEWZ46975043 = 80028315;    int buhhtQkEWZ71398522 = -614954108;    int buhhtQkEWZ4597589 = -193755381;    int buhhtQkEWZ92839060 = 45888777;    int buhhtQkEWZ71659590 = -54136434;    int buhhtQkEWZ85920842 = -759511631;    int buhhtQkEWZ88373243 = -920668367;    int buhhtQkEWZ85327609 = -355274664;    int buhhtQkEWZ24473498 = -489844685;    int buhhtQkEWZ2833671 = -732643165;    int buhhtQkEWZ70562914 = -994012095;    int buhhtQkEWZ48665521 = -716269010;    int buhhtQkEWZ5911819 = -19957618;    int buhhtQkEWZ60987011 = -537162583;    int buhhtQkEWZ40730489 = -171876695;    int buhhtQkEWZ20818679 = 63321274;    int buhhtQkEWZ60354961 = -564907375;    int buhhtQkEWZ35577043 = 59789780;    int buhhtQkEWZ85916125 = -472313942;    int buhhtQkEWZ27050062 = -555338433;    int buhhtQkEWZ16966543 = -183276192;    int buhhtQkEWZ40128547 = -388177378;    int buhhtQkEWZ13058071 = -650620454;    int buhhtQkEWZ89835293 = -764438965;    int buhhtQkEWZ95973971 = -977620407;    int buhhtQkEWZ95752982 = -786755130;    int buhhtQkEWZ163908 = -571088566;    int buhhtQkEWZ43056219 = -859569354;    int buhhtQkEWZ93845910 = -27300281;    int buhhtQkEWZ99802763 = -317114536;    int buhhtQkEWZ52920144 = -331778024;    int buhhtQkEWZ24609954 = 68611159;    int buhhtQkEWZ5664904 = -978562016;    int buhhtQkEWZ45785678 = -918019945;    int buhhtQkEWZ23209434 = -377349751;    int buhhtQkEWZ98518216 = -278196519;    int buhhtQkEWZ8097539 = -826239609;    int buhhtQkEWZ59114051 = -882494142;    int buhhtQkEWZ99358614 = -899354664;    int buhhtQkEWZ91622906 = -396796354;    int buhhtQkEWZ55143294 = -974944408;    int buhhtQkEWZ23850395 = -855966586;    int buhhtQkEWZ33826406 = -548971137;    int buhhtQkEWZ80511344 = -955348842;    int buhhtQkEWZ64167761 = -825995592;    int buhhtQkEWZ41185011 = -841775584;    int buhhtQkEWZ95130312 = -953354708;    int buhhtQkEWZ11072543 = -778098841;    int buhhtQkEWZ37293071 = -470151232;    int buhhtQkEWZ98708737 = -898795587;    int buhhtQkEWZ23103489 = -227003651;    int buhhtQkEWZ43430396 = -603734127;    int buhhtQkEWZ52904613 = -326973667;    int buhhtQkEWZ18284523 = -8085570;    int buhhtQkEWZ17421315 = -516361002;    int buhhtQkEWZ6846497 = -531794308;    int buhhtQkEWZ58340452 = -964333654;    int buhhtQkEWZ14762295 = -429316416;    int buhhtQkEWZ96865089 = 23509183;    int buhhtQkEWZ75906607 = -267381304;    int buhhtQkEWZ85756935 = -88423065;    int buhhtQkEWZ45317024 = 38900987;    int buhhtQkEWZ91481698 = -227974383;    int buhhtQkEWZ24670734 = -72730149;    int buhhtQkEWZ49913527 = -300865142;    int buhhtQkEWZ45952960 = -962623254;    int buhhtQkEWZ43000618 = -737706994;    int buhhtQkEWZ60126140 = -101937673;    int buhhtQkEWZ37777577 = -59812832;    int buhhtQkEWZ42212273 = -893680177;    int buhhtQkEWZ12721141 = -110439118;    int buhhtQkEWZ1240911 = -682413233;    int buhhtQkEWZ36218429 = -40855557;    int buhhtQkEWZ94293218 = 24482411;    int buhhtQkEWZ71906767 = -580394026;    int buhhtQkEWZ93116148 = -327309606;    int buhhtQkEWZ6302141 = -839206241;    int buhhtQkEWZ32546726 = -695271613;    int buhhtQkEWZ25667533 = -938443374;    int buhhtQkEWZ54788960 = -35844823;    int buhhtQkEWZ622671 = -833400422;    int buhhtQkEWZ89091364 = -792989726;    int buhhtQkEWZ5763149 = -289418123;    int buhhtQkEWZ95137173 = -128504695;    int buhhtQkEWZ76699274 = 9889114;    int buhhtQkEWZ9489748 = -728043898;    int buhhtQkEWZ71705341 = -604415175;    int buhhtQkEWZ87380380 = -870476447;    int buhhtQkEWZ28364363 = -301658944;    int buhhtQkEWZ16362938 = -845555444;    int buhhtQkEWZ40177764 = -313862865;    int buhhtQkEWZ93335243 = -296923193;    int buhhtQkEWZ62248961 = -806003326;    int buhhtQkEWZ23452007 = -531973360;    int buhhtQkEWZ5865972 = -208373289;    int buhhtQkEWZ9826271 = -913845396;    int buhhtQkEWZ32368696 = -527992203;    int buhhtQkEWZ9155672 = -376240988;    int buhhtQkEWZ30597818 = -799637193;     buhhtQkEWZ34387858 = buhhtQkEWZ46975043;     buhhtQkEWZ46975043 = buhhtQkEWZ71398522;     buhhtQkEWZ71398522 = buhhtQkEWZ4597589;     buhhtQkEWZ4597589 = buhhtQkEWZ92839060;     buhhtQkEWZ92839060 = buhhtQkEWZ71659590;     buhhtQkEWZ71659590 = buhhtQkEWZ85920842;     buhhtQkEWZ85920842 = buhhtQkEWZ88373243;     buhhtQkEWZ88373243 = buhhtQkEWZ85327609;     buhhtQkEWZ85327609 = buhhtQkEWZ24473498;     buhhtQkEWZ24473498 = buhhtQkEWZ2833671;     buhhtQkEWZ2833671 = buhhtQkEWZ70562914;     buhhtQkEWZ70562914 = buhhtQkEWZ48665521;     buhhtQkEWZ48665521 = buhhtQkEWZ5911819;     buhhtQkEWZ5911819 = buhhtQkEWZ60987011;     buhhtQkEWZ60987011 = buhhtQkEWZ40730489;     buhhtQkEWZ40730489 = buhhtQkEWZ20818679;     buhhtQkEWZ20818679 = buhhtQkEWZ60354961;     buhhtQkEWZ60354961 = buhhtQkEWZ35577043;     buhhtQkEWZ35577043 = buhhtQkEWZ85916125;     buhhtQkEWZ85916125 = buhhtQkEWZ27050062;     buhhtQkEWZ27050062 = buhhtQkEWZ16966543;     buhhtQkEWZ16966543 = buhhtQkEWZ40128547;     buhhtQkEWZ40128547 = buhhtQkEWZ13058071;     buhhtQkEWZ13058071 = buhhtQkEWZ89835293;     buhhtQkEWZ89835293 = buhhtQkEWZ95973971;     buhhtQkEWZ95973971 = buhhtQkEWZ95752982;     buhhtQkEWZ95752982 = buhhtQkEWZ163908;     buhhtQkEWZ163908 = buhhtQkEWZ43056219;     buhhtQkEWZ43056219 = buhhtQkEWZ93845910;     buhhtQkEWZ93845910 = buhhtQkEWZ99802763;     buhhtQkEWZ99802763 = buhhtQkEWZ52920144;     buhhtQkEWZ52920144 = buhhtQkEWZ24609954;     buhhtQkEWZ24609954 = buhhtQkEWZ5664904;     buhhtQkEWZ5664904 = buhhtQkEWZ45785678;     buhhtQkEWZ45785678 = buhhtQkEWZ23209434;     buhhtQkEWZ23209434 = buhhtQkEWZ98518216;     buhhtQkEWZ98518216 = buhhtQkEWZ8097539;     buhhtQkEWZ8097539 = buhhtQkEWZ59114051;     buhhtQkEWZ59114051 = buhhtQkEWZ99358614;     buhhtQkEWZ99358614 = buhhtQkEWZ91622906;     buhhtQkEWZ91622906 = buhhtQkEWZ55143294;     buhhtQkEWZ55143294 = buhhtQkEWZ23850395;     buhhtQkEWZ23850395 = buhhtQkEWZ33826406;     buhhtQkEWZ33826406 = buhhtQkEWZ80511344;     buhhtQkEWZ80511344 = buhhtQkEWZ64167761;     buhhtQkEWZ64167761 = buhhtQkEWZ41185011;     buhhtQkEWZ41185011 = buhhtQkEWZ95130312;     buhhtQkEWZ95130312 = buhhtQkEWZ11072543;     buhhtQkEWZ11072543 = buhhtQkEWZ37293071;     buhhtQkEWZ37293071 = buhhtQkEWZ98708737;     buhhtQkEWZ98708737 = buhhtQkEWZ23103489;     buhhtQkEWZ23103489 = buhhtQkEWZ43430396;     buhhtQkEWZ43430396 = buhhtQkEWZ52904613;     buhhtQkEWZ52904613 = buhhtQkEWZ18284523;     buhhtQkEWZ18284523 = buhhtQkEWZ17421315;     buhhtQkEWZ17421315 = buhhtQkEWZ6846497;     buhhtQkEWZ6846497 = buhhtQkEWZ58340452;     buhhtQkEWZ58340452 = buhhtQkEWZ14762295;     buhhtQkEWZ14762295 = buhhtQkEWZ96865089;     buhhtQkEWZ96865089 = buhhtQkEWZ75906607;     buhhtQkEWZ75906607 = buhhtQkEWZ85756935;     buhhtQkEWZ85756935 = buhhtQkEWZ45317024;     buhhtQkEWZ45317024 = buhhtQkEWZ91481698;     buhhtQkEWZ91481698 = buhhtQkEWZ24670734;     buhhtQkEWZ24670734 = buhhtQkEWZ49913527;     buhhtQkEWZ49913527 = buhhtQkEWZ45952960;     buhhtQkEWZ45952960 = buhhtQkEWZ43000618;     buhhtQkEWZ43000618 = buhhtQkEWZ60126140;     buhhtQkEWZ60126140 = buhhtQkEWZ37777577;     buhhtQkEWZ37777577 = buhhtQkEWZ42212273;     buhhtQkEWZ42212273 = buhhtQkEWZ12721141;     buhhtQkEWZ12721141 = buhhtQkEWZ1240911;     buhhtQkEWZ1240911 = buhhtQkEWZ36218429;     buhhtQkEWZ36218429 = buhhtQkEWZ94293218;     buhhtQkEWZ94293218 = buhhtQkEWZ71906767;     buhhtQkEWZ71906767 = buhhtQkEWZ93116148;     buhhtQkEWZ93116148 = buhhtQkEWZ6302141;     buhhtQkEWZ6302141 = buhhtQkEWZ32546726;     buhhtQkEWZ32546726 = buhhtQkEWZ25667533;     buhhtQkEWZ25667533 = buhhtQkEWZ54788960;     buhhtQkEWZ54788960 = buhhtQkEWZ622671;     buhhtQkEWZ622671 = buhhtQkEWZ89091364;     buhhtQkEWZ89091364 = buhhtQkEWZ5763149;     buhhtQkEWZ5763149 = buhhtQkEWZ95137173;     buhhtQkEWZ95137173 = buhhtQkEWZ76699274;     buhhtQkEWZ76699274 = buhhtQkEWZ9489748;     buhhtQkEWZ9489748 = buhhtQkEWZ71705341;     buhhtQkEWZ71705341 = buhhtQkEWZ87380380;     buhhtQkEWZ87380380 = buhhtQkEWZ28364363;     buhhtQkEWZ28364363 = buhhtQkEWZ16362938;     buhhtQkEWZ16362938 = buhhtQkEWZ40177764;     buhhtQkEWZ40177764 = buhhtQkEWZ93335243;     buhhtQkEWZ93335243 = buhhtQkEWZ62248961;     buhhtQkEWZ62248961 = buhhtQkEWZ23452007;     buhhtQkEWZ23452007 = buhhtQkEWZ5865972;     buhhtQkEWZ5865972 = buhhtQkEWZ9826271;     buhhtQkEWZ9826271 = buhhtQkEWZ32368696;     buhhtQkEWZ32368696 = buhhtQkEWZ9155672;     buhhtQkEWZ9155672 = buhhtQkEWZ30597818;     buhhtQkEWZ30597818 = buhhtQkEWZ34387858;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XlcEBxUiUK63547926() {     int UEvmigPvEr17832163 = -580302874;    int UEvmigPvEr63847132 = 80646127;    int UEvmigPvEr67807029 = -408479336;    int UEvmigPvEr67389840 = -387555216;    int UEvmigPvEr98960384 = -860978714;    int UEvmigPvEr70163990 = -933208087;    int UEvmigPvEr56960160 = -499422944;    int UEvmigPvEr2720686 = -907826396;    int UEvmigPvEr43857310 = -696734015;    int UEvmigPvEr57019149 = -958220797;    int UEvmigPvEr52249608 = -922759038;    int UEvmigPvEr33768313 = -981630341;    int UEvmigPvEr66783297 = -507930251;    int UEvmigPvEr85538710 = -908247975;    int UEvmigPvEr21841475 = -631185565;    int UEvmigPvEr27119651 = 40043980;    int UEvmigPvEr14570564 = -790023753;    int UEvmigPvEr41206408 = -812212252;    int UEvmigPvEr98017185 = 61114909;    int UEvmigPvEr75283568 = 98100065;    int UEvmigPvEr82690448 = -144867464;    int UEvmigPvEr29113861 = -174427962;    int UEvmigPvEr43954255 = -428066575;    int UEvmigPvEr47764628 = -722445850;    int UEvmigPvEr31411252 = -224091724;    int UEvmigPvEr91316119 = -858130122;    int UEvmigPvEr45207723 = -414839472;    int UEvmigPvEr39895533 = -641043450;    int UEvmigPvEr79137313 = -722074496;    int UEvmigPvEr9679252 = -540150133;    int UEvmigPvEr17898077 = 45128263;    int UEvmigPvEr16026571 = -246973104;    int UEvmigPvEr68148841 = -652261019;    int UEvmigPvEr47823414 = -570164186;    int UEvmigPvEr24512409 = -15684681;    int UEvmigPvEr35598618 = -165373657;    int UEvmigPvEr67132395 = -157045699;    int UEvmigPvEr48676370 = -534710129;    int UEvmigPvEr8109129 = 32608060;    int UEvmigPvEr48853749 = -651948186;    int UEvmigPvEr25271831 = 31654673;    int UEvmigPvEr5631081 = -448852730;    int UEvmigPvEr47417464 = -307023829;    int UEvmigPvEr25731346 = -305153908;    int UEvmigPvEr72455841 = -683505795;    int UEvmigPvEr3773757 = -845606515;    int UEvmigPvEr17200441 = -974079814;    int UEvmigPvEr14800908 = -952076427;    int UEvmigPvEr41919655 = -263700924;    int UEvmigPvEr72625554 = -677171687;    int UEvmigPvEr71966185 = -19697009;    int UEvmigPvEr78604283 = -285262962;    int UEvmigPvEr57542855 = -815481443;    int UEvmigPvEr77581372 = -45387054;    int UEvmigPvEr80857477 = -356571693;    int UEvmigPvEr88718302 = -305874912;    int UEvmigPvEr19892878 = -491287298;    int UEvmigPvEr20042401 = -686033486;    int UEvmigPvEr35978589 = -63463493;    int UEvmigPvEr7644265 = 97151408;    int UEvmigPvEr24956268 = -418368615;    int UEvmigPvEr17064628 = -858379495;    int UEvmigPvEr23583373 = -85751900;    int UEvmigPvEr34178058 = -56583883;    int UEvmigPvEr39121072 = -903349061;    int UEvmigPvEr36223038 = -575785935;    int UEvmigPvEr65619472 = -229369323;    int UEvmigPvEr18959884 = -937766065;    int UEvmigPvEr61026302 = -792563295;    int UEvmigPvEr86242856 = -365811908;    int UEvmigPvEr59987256 = -802910322;    int UEvmigPvEr65894193 = -155313625;    int UEvmigPvEr33097279 = -744820312;    int UEvmigPvEr49163436 = -286936906;    int UEvmigPvEr50011737 = -933554608;    int UEvmigPvEr77059368 = -696014735;    int UEvmigPvEr81696396 = -867404134;    int UEvmigPvEr18222909 = -22912668;    int UEvmigPvEr75308787 = 61059944;    int UEvmigPvEr27637495 = -378485209;    int UEvmigPvEr74115679 = -884050309;    int UEvmigPvEr30406815 = -462763046;    int UEvmigPvEr97975877 = -277342527;    int UEvmigPvEr6511759 = 55097190;    int UEvmigPvEr37713066 = -420453124;    int UEvmigPvEr39293794 = -669608775;    int UEvmigPvEr58483716 = -431491661;    int UEvmigPvEr90567468 = -506873965;    int UEvmigPvEr66965936 = -113592494;    int UEvmigPvEr35794106 = -709809769;    int UEvmigPvEr15705741 = -674086359;    int UEvmigPvEr47089994 = -471012213;    int UEvmigPvEr12697782 = -371246636;    int UEvmigPvEr464865 = 35456652;    int UEvmigPvEr23897482 = -133579572;    int UEvmigPvEr8207204 = -109965833;    int UEvmigPvEr82047708 = -263100830;    int UEvmigPvEr13239406 = -150439946;    int UEvmigPvEr86610273 = -401804848;    int UEvmigPvEr36232804 = -580302874;     UEvmigPvEr17832163 = UEvmigPvEr63847132;     UEvmigPvEr63847132 = UEvmigPvEr67807029;     UEvmigPvEr67807029 = UEvmigPvEr67389840;     UEvmigPvEr67389840 = UEvmigPvEr98960384;     UEvmigPvEr98960384 = UEvmigPvEr70163990;     UEvmigPvEr70163990 = UEvmigPvEr56960160;     UEvmigPvEr56960160 = UEvmigPvEr2720686;     UEvmigPvEr2720686 = UEvmigPvEr43857310;     UEvmigPvEr43857310 = UEvmigPvEr57019149;     UEvmigPvEr57019149 = UEvmigPvEr52249608;     UEvmigPvEr52249608 = UEvmigPvEr33768313;     UEvmigPvEr33768313 = UEvmigPvEr66783297;     UEvmigPvEr66783297 = UEvmigPvEr85538710;     UEvmigPvEr85538710 = UEvmigPvEr21841475;     UEvmigPvEr21841475 = UEvmigPvEr27119651;     UEvmigPvEr27119651 = UEvmigPvEr14570564;     UEvmigPvEr14570564 = UEvmigPvEr41206408;     UEvmigPvEr41206408 = UEvmigPvEr98017185;     UEvmigPvEr98017185 = UEvmigPvEr75283568;     UEvmigPvEr75283568 = UEvmigPvEr82690448;     UEvmigPvEr82690448 = UEvmigPvEr29113861;     UEvmigPvEr29113861 = UEvmigPvEr43954255;     UEvmigPvEr43954255 = UEvmigPvEr47764628;     UEvmigPvEr47764628 = UEvmigPvEr31411252;     UEvmigPvEr31411252 = UEvmigPvEr91316119;     UEvmigPvEr91316119 = UEvmigPvEr45207723;     UEvmigPvEr45207723 = UEvmigPvEr39895533;     UEvmigPvEr39895533 = UEvmigPvEr79137313;     UEvmigPvEr79137313 = UEvmigPvEr9679252;     UEvmigPvEr9679252 = UEvmigPvEr17898077;     UEvmigPvEr17898077 = UEvmigPvEr16026571;     UEvmigPvEr16026571 = UEvmigPvEr68148841;     UEvmigPvEr68148841 = UEvmigPvEr47823414;     UEvmigPvEr47823414 = UEvmigPvEr24512409;     UEvmigPvEr24512409 = UEvmigPvEr35598618;     UEvmigPvEr35598618 = UEvmigPvEr67132395;     UEvmigPvEr67132395 = UEvmigPvEr48676370;     UEvmigPvEr48676370 = UEvmigPvEr8109129;     UEvmigPvEr8109129 = UEvmigPvEr48853749;     UEvmigPvEr48853749 = UEvmigPvEr25271831;     UEvmigPvEr25271831 = UEvmigPvEr5631081;     UEvmigPvEr5631081 = UEvmigPvEr47417464;     UEvmigPvEr47417464 = UEvmigPvEr25731346;     UEvmigPvEr25731346 = UEvmigPvEr72455841;     UEvmigPvEr72455841 = UEvmigPvEr3773757;     UEvmigPvEr3773757 = UEvmigPvEr17200441;     UEvmigPvEr17200441 = UEvmigPvEr14800908;     UEvmigPvEr14800908 = UEvmigPvEr41919655;     UEvmigPvEr41919655 = UEvmigPvEr72625554;     UEvmigPvEr72625554 = UEvmigPvEr71966185;     UEvmigPvEr71966185 = UEvmigPvEr78604283;     UEvmigPvEr78604283 = UEvmigPvEr57542855;     UEvmigPvEr57542855 = UEvmigPvEr77581372;     UEvmigPvEr77581372 = UEvmigPvEr80857477;     UEvmigPvEr80857477 = UEvmigPvEr88718302;     UEvmigPvEr88718302 = UEvmigPvEr19892878;     UEvmigPvEr19892878 = UEvmigPvEr20042401;     UEvmigPvEr20042401 = UEvmigPvEr35978589;     UEvmigPvEr35978589 = UEvmigPvEr7644265;     UEvmigPvEr7644265 = UEvmigPvEr24956268;     UEvmigPvEr24956268 = UEvmigPvEr17064628;     UEvmigPvEr17064628 = UEvmigPvEr23583373;     UEvmigPvEr23583373 = UEvmigPvEr34178058;     UEvmigPvEr34178058 = UEvmigPvEr39121072;     UEvmigPvEr39121072 = UEvmigPvEr36223038;     UEvmigPvEr36223038 = UEvmigPvEr65619472;     UEvmigPvEr65619472 = UEvmigPvEr18959884;     UEvmigPvEr18959884 = UEvmigPvEr61026302;     UEvmigPvEr61026302 = UEvmigPvEr86242856;     UEvmigPvEr86242856 = UEvmigPvEr59987256;     UEvmigPvEr59987256 = UEvmigPvEr65894193;     UEvmigPvEr65894193 = UEvmigPvEr33097279;     UEvmigPvEr33097279 = UEvmigPvEr49163436;     UEvmigPvEr49163436 = UEvmigPvEr50011737;     UEvmigPvEr50011737 = UEvmigPvEr77059368;     UEvmigPvEr77059368 = UEvmigPvEr81696396;     UEvmigPvEr81696396 = UEvmigPvEr18222909;     UEvmigPvEr18222909 = UEvmigPvEr75308787;     UEvmigPvEr75308787 = UEvmigPvEr27637495;     UEvmigPvEr27637495 = UEvmigPvEr74115679;     UEvmigPvEr74115679 = UEvmigPvEr30406815;     UEvmigPvEr30406815 = UEvmigPvEr97975877;     UEvmigPvEr97975877 = UEvmigPvEr6511759;     UEvmigPvEr6511759 = UEvmigPvEr37713066;     UEvmigPvEr37713066 = UEvmigPvEr39293794;     UEvmigPvEr39293794 = UEvmigPvEr58483716;     UEvmigPvEr58483716 = UEvmigPvEr90567468;     UEvmigPvEr90567468 = UEvmigPvEr66965936;     UEvmigPvEr66965936 = UEvmigPvEr35794106;     UEvmigPvEr35794106 = UEvmigPvEr15705741;     UEvmigPvEr15705741 = UEvmigPvEr47089994;     UEvmigPvEr47089994 = UEvmigPvEr12697782;     UEvmigPvEr12697782 = UEvmigPvEr464865;     UEvmigPvEr464865 = UEvmigPvEr23897482;     UEvmigPvEr23897482 = UEvmigPvEr8207204;     UEvmigPvEr8207204 = UEvmigPvEr82047708;     UEvmigPvEr82047708 = UEvmigPvEr13239406;     UEvmigPvEr13239406 = UEvmigPvEr86610273;     UEvmigPvEr86610273 = UEvmigPvEr36232804;     UEvmigPvEr36232804 = UEvmigPvEr17832163;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gKhxFgLfQA34003799() {     int AMrQqdtzyx30618330 = -715343565;    int AMrQqdtzyx50618020 = -89687887;    int AMrQqdtzyx85559829 = -816838717;    int AMrQqdtzyx75330665 = -211757493;    int AMrQqdtzyx92471488 = -252795179;    int AMrQqdtzyx13474062 = -710131400;    int AMrQqdtzyx78167918 = 54904985;    int AMrQqdtzyx29439799 = -252344653;    int AMrQqdtzyx39262120 = -658492107;    int AMrQqdtzyx63237854 = -762308430;    int AMrQqdtzyx79736990 = 62554947;    int AMrQqdtzyx65718818 = -388963157;    int AMrQqdtzyx36926600 = -411712908;    int AMrQqdtzyx35162789 = -140663173;    int AMrQqdtzyx45148028 = -538171396;    int AMrQqdtzyx55544719 = -510847409;    int AMrQqdtzyx70322381 = -453798220;    int AMrQqdtzyx24764893 = -367422554;    int AMrQqdtzyx99177815 = -948354287;    int AMrQqdtzyx77938064 = -501144074;    int AMrQqdtzyx62864505 = -636183728;    int AMrQqdtzyx58161680 = -344286310;    int AMrQqdtzyx82714550 = -359070470;    int AMrQqdtzyx92271758 = 17209138;    int AMrQqdtzyx37305511 = -434533648;    int AMrQqdtzyx72634765 = -548561321;    int AMrQqdtzyx56623259 = 51454247;    int AMrQqdtzyx7248458 = -814841555;    int AMrQqdtzyx95754366 = -936291737;    int AMrQqdtzyx62430367 = -170036287;    int AMrQqdtzyx62334073 = -231288487;    int AMrQqdtzyx19345395 = -225692783;    int AMrQqdtzyx14194790 = -528115803;    int AMrQqdtzyx24899787 = -474918405;    int AMrQqdtzyx7572941 = -625716561;    int AMrQqdtzyx30822733 = 46063239;    int AMrQqdtzyx3896456 = 53044951;    int AMrQqdtzyx6380393 = -454330220;    int AMrQqdtzyx13884515 = -909242517;    int AMrQqdtzyx75118479 = -496365032;    int AMrQqdtzyx86703139 = -343819320;    int AMrQqdtzyx13333537 = -347164064;    int AMrQqdtzyx82300933 = -330457274;    int AMrQqdtzyx70359884 = -646326151;    int AMrQqdtzyx52464332 = -576275755;    int AMrQqdtzyx77401886 = -918924955;    int AMrQqdtzyx94782460 = -371594866;    int AMrQqdtzyx44240874 = -7429207;    int AMrQqdtzyx85983704 = -427302898;    int AMrQqdtzyx19575975 = -188208150;    int AMrQqdtzyx90293679 = -272581318;    int AMrQqdtzyx6780831 = -756448300;    int AMrQqdtzyx92028004 = -991484726;    int AMrQqdtzyx22281097 = -734838796;    int AMrQqdtzyx71139293 = -351016180;    int AMrQqdtzyx72456650 = -271057256;    int AMrQqdtzyx67903470 = -730617417;    int AMrQqdtzyx93288071 = -734047855;    int AMrQqdtzyx38025155 = -777223845;    int AMrQqdtzyx19836724 = -704233858;    int AMrQqdtzyx56850802 = -661585648;    int AMrQqdtzyx70919460 = -130253460;    int AMrQqdtzyx33685432 = -316052917;    int AMrQqdtzyx76831753 = -388455820;    int AMrQqdtzyx903782 = -431019943;    int AMrQqdtzyx60391596 = -711752271;    int AMrQqdtzyx51524028 = -860847355;    int AMrQqdtzyx12026814 = -936794504;    int AMrQqdtzyx27589848 = -514946613;    int AMrQqdtzyx14325295 = -484234635;    int AMrQqdtzyx51648264 = -463892361;    int AMrQqdtzyx63941988 = -999468001;    int AMrQqdtzyx10880378 = -458180037;    int AMrQqdtzyx24059337 = -351989255;    int AMrQqdtzyx91234925 = -57324754;    int AMrQqdtzyx49530969 = -189019665;    int AMrQqdtzyx75860746 = 86170963;    int AMrQqdtzyx12354667 = -712744320;    int AMrQqdtzyx39807426 = -406515108;    int AMrQqdtzyx59903625 = -515608694;    int AMrQqdtzyx77852305 = -76966456;    int AMrQqdtzyx12382385 = -941116546;    int AMrQqdtzyx21264754 = -287538657;    int AMrQqdtzyx76178391 = -648083587;    int AMrQqdtzyx72136688 = -897454970;    int AMrQqdtzyx55553242 = -474840187;    int AMrQqdtzyx27317390 = -234208057;    int AMrQqdtzyx91913692 = -793277008;    int AMrQqdtzyx53760493 = -23902226;    int AMrQqdtzyx35116290 = -254659305;    int AMrQqdtzyx62919263 = -223319345;    int AMrQqdtzyx10608385 = -212907194;    int AMrQqdtzyx68355238 = -677106375;    int AMrQqdtzyx94047791 = -105008659;    int AMrQqdtzyx18267677 = -834779384;    int AMrQqdtzyx15783679 = -113565860;    int AMrQqdtzyx79648104 = 68888852;    int AMrQqdtzyx5469181 = -942001454;    int AMrQqdtzyx69456103 = -115306208;    int AMrQqdtzyx92072735 = -715343565;     AMrQqdtzyx30618330 = AMrQqdtzyx50618020;     AMrQqdtzyx50618020 = AMrQqdtzyx85559829;     AMrQqdtzyx85559829 = AMrQqdtzyx75330665;     AMrQqdtzyx75330665 = AMrQqdtzyx92471488;     AMrQqdtzyx92471488 = AMrQqdtzyx13474062;     AMrQqdtzyx13474062 = AMrQqdtzyx78167918;     AMrQqdtzyx78167918 = AMrQqdtzyx29439799;     AMrQqdtzyx29439799 = AMrQqdtzyx39262120;     AMrQqdtzyx39262120 = AMrQqdtzyx63237854;     AMrQqdtzyx63237854 = AMrQqdtzyx79736990;     AMrQqdtzyx79736990 = AMrQqdtzyx65718818;     AMrQqdtzyx65718818 = AMrQqdtzyx36926600;     AMrQqdtzyx36926600 = AMrQqdtzyx35162789;     AMrQqdtzyx35162789 = AMrQqdtzyx45148028;     AMrQqdtzyx45148028 = AMrQqdtzyx55544719;     AMrQqdtzyx55544719 = AMrQqdtzyx70322381;     AMrQqdtzyx70322381 = AMrQqdtzyx24764893;     AMrQqdtzyx24764893 = AMrQqdtzyx99177815;     AMrQqdtzyx99177815 = AMrQqdtzyx77938064;     AMrQqdtzyx77938064 = AMrQqdtzyx62864505;     AMrQqdtzyx62864505 = AMrQqdtzyx58161680;     AMrQqdtzyx58161680 = AMrQqdtzyx82714550;     AMrQqdtzyx82714550 = AMrQqdtzyx92271758;     AMrQqdtzyx92271758 = AMrQqdtzyx37305511;     AMrQqdtzyx37305511 = AMrQqdtzyx72634765;     AMrQqdtzyx72634765 = AMrQqdtzyx56623259;     AMrQqdtzyx56623259 = AMrQqdtzyx7248458;     AMrQqdtzyx7248458 = AMrQqdtzyx95754366;     AMrQqdtzyx95754366 = AMrQqdtzyx62430367;     AMrQqdtzyx62430367 = AMrQqdtzyx62334073;     AMrQqdtzyx62334073 = AMrQqdtzyx19345395;     AMrQqdtzyx19345395 = AMrQqdtzyx14194790;     AMrQqdtzyx14194790 = AMrQqdtzyx24899787;     AMrQqdtzyx24899787 = AMrQqdtzyx7572941;     AMrQqdtzyx7572941 = AMrQqdtzyx30822733;     AMrQqdtzyx30822733 = AMrQqdtzyx3896456;     AMrQqdtzyx3896456 = AMrQqdtzyx6380393;     AMrQqdtzyx6380393 = AMrQqdtzyx13884515;     AMrQqdtzyx13884515 = AMrQqdtzyx75118479;     AMrQqdtzyx75118479 = AMrQqdtzyx86703139;     AMrQqdtzyx86703139 = AMrQqdtzyx13333537;     AMrQqdtzyx13333537 = AMrQqdtzyx82300933;     AMrQqdtzyx82300933 = AMrQqdtzyx70359884;     AMrQqdtzyx70359884 = AMrQqdtzyx52464332;     AMrQqdtzyx52464332 = AMrQqdtzyx77401886;     AMrQqdtzyx77401886 = AMrQqdtzyx94782460;     AMrQqdtzyx94782460 = AMrQqdtzyx44240874;     AMrQqdtzyx44240874 = AMrQqdtzyx85983704;     AMrQqdtzyx85983704 = AMrQqdtzyx19575975;     AMrQqdtzyx19575975 = AMrQqdtzyx90293679;     AMrQqdtzyx90293679 = AMrQqdtzyx6780831;     AMrQqdtzyx6780831 = AMrQqdtzyx92028004;     AMrQqdtzyx92028004 = AMrQqdtzyx22281097;     AMrQqdtzyx22281097 = AMrQqdtzyx71139293;     AMrQqdtzyx71139293 = AMrQqdtzyx72456650;     AMrQqdtzyx72456650 = AMrQqdtzyx67903470;     AMrQqdtzyx67903470 = AMrQqdtzyx93288071;     AMrQqdtzyx93288071 = AMrQqdtzyx38025155;     AMrQqdtzyx38025155 = AMrQqdtzyx19836724;     AMrQqdtzyx19836724 = AMrQqdtzyx56850802;     AMrQqdtzyx56850802 = AMrQqdtzyx70919460;     AMrQqdtzyx70919460 = AMrQqdtzyx33685432;     AMrQqdtzyx33685432 = AMrQqdtzyx76831753;     AMrQqdtzyx76831753 = AMrQqdtzyx903782;     AMrQqdtzyx903782 = AMrQqdtzyx60391596;     AMrQqdtzyx60391596 = AMrQqdtzyx51524028;     AMrQqdtzyx51524028 = AMrQqdtzyx12026814;     AMrQqdtzyx12026814 = AMrQqdtzyx27589848;     AMrQqdtzyx27589848 = AMrQqdtzyx14325295;     AMrQqdtzyx14325295 = AMrQqdtzyx51648264;     AMrQqdtzyx51648264 = AMrQqdtzyx63941988;     AMrQqdtzyx63941988 = AMrQqdtzyx10880378;     AMrQqdtzyx10880378 = AMrQqdtzyx24059337;     AMrQqdtzyx24059337 = AMrQqdtzyx91234925;     AMrQqdtzyx91234925 = AMrQqdtzyx49530969;     AMrQqdtzyx49530969 = AMrQqdtzyx75860746;     AMrQqdtzyx75860746 = AMrQqdtzyx12354667;     AMrQqdtzyx12354667 = AMrQqdtzyx39807426;     AMrQqdtzyx39807426 = AMrQqdtzyx59903625;     AMrQqdtzyx59903625 = AMrQqdtzyx77852305;     AMrQqdtzyx77852305 = AMrQqdtzyx12382385;     AMrQqdtzyx12382385 = AMrQqdtzyx21264754;     AMrQqdtzyx21264754 = AMrQqdtzyx76178391;     AMrQqdtzyx76178391 = AMrQqdtzyx72136688;     AMrQqdtzyx72136688 = AMrQqdtzyx55553242;     AMrQqdtzyx55553242 = AMrQqdtzyx27317390;     AMrQqdtzyx27317390 = AMrQqdtzyx91913692;     AMrQqdtzyx91913692 = AMrQqdtzyx53760493;     AMrQqdtzyx53760493 = AMrQqdtzyx35116290;     AMrQqdtzyx35116290 = AMrQqdtzyx62919263;     AMrQqdtzyx62919263 = AMrQqdtzyx10608385;     AMrQqdtzyx10608385 = AMrQqdtzyx68355238;     AMrQqdtzyx68355238 = AMrQqdtzyx94047791;     AMrQqdtzyx94047791 = AMrQqdtzyx18267677;     AMrQqdtzyx18267677 = AMrQqdtzyx15783679;     AMrQqdtzyx15783679 = AMrQqdtzyx79648104;     AMrQqdtzyx79648104 = AMrQqdtzyx5469181;     AMrQqdtzyx5469181 = AMrQqdtzyx69456103;     AMrQqdtzyx69456103 = AMrQqdtzyx92072735;     AMrQqdtzyx92072735 = AMrQqdtzyx30618330;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ThghyRTRho52217141() {     int DzKkPhRrbO14062636 = -496009246;    int DzKkPhRrbO67490109 = -89070074;    int DzKkPhRrbO81968336 = -610363945;    int DzKkPhRrbO38122917 = -405557329;    int DzKkPhRrbO98592812 = -59662670;    int DzKkPhRrbO11978462 = -489203053;    int DzKkPhRrbO49207236 = -785006328;    int DzKkPhRrbO43787241 = -239502682;    int DzKkPhRrbO97791820 = -999951458;    int DzKkPhRrbO95783506 = -130684542;    int DzKkPhRrbO29152928 = -127560926;    int DzKkPhRrbO28924218 = -376581403;    int DzKkPhRrbO55044376 = -203374149;    int DzKkPhRrbO14789681 = 71046470;    int DzKkPhRrbO6002491 = -632194378;    int DzKkPhRrbO41933881 = -298926735;    int DzKkPhRrbO64074266 = -207143247;    int DzKkPhRrbO5616339 = -614727430;    int DzKkPhRrbO61617958 = -947029157;    int DzKkPhRrbO67305507 = 69269933;    int DzKkPhRrbO18504893 = -225712759;    int DzKkPhRrbO70308998 = -335438080;    int DzKkPhRrbO86540257 = -398959667;    int DzKkPhRrbO26978316 = -54616258;    int DzKkPhRrbO78881469 = -994186407;    int DzKkPhRrbO67976914 = -429071037;    int DzKkPhRrbO6078000 = -676630095;    int DzKkPhRrbO46980083 = -884796439;    int DzKkPhRrbO31835461 = -798796879;    int DzKkPhRrbO78263708 = -682886139;    int DzKkPhRrbO80429387 = -969045688;    int DzKkPhRrbO82451821 = -140887862;    int DzKkPhRrbO57733677 = -148987980;    int DzKkPhRrbO67058297 = -66520575;    int DzKkPhRrbO86299671 = -823381296;    int DzKkPhRrbO43211917 = -841960667;    int DzKkPhRrbO72510634 = -925804229;    int DzKkPhRrbO46959225 = -162800740;    int DzKkPhRrbO62879593 = 5859685;    int DzKkPhRrbO24613614 = -248958554;    int DzKkPhRrbO20352064 = 84631706;    int DzKkPhRrbO63821322 = -921072385;    int DzKkPhRrbO5868004 = -881514516;    int DzKkPhRrbO62264824 = -402508922;    int DzKkPhRrbO44408829 = -304432708;    int DzKkPhRrbO17007882 = -938535879;    int DzKkPhRrbO70797890 = -503899096;    int DzKkPhRrbO63911469 = -6150925;    int DzKkPhRrbO16830817 = 87095019;    int DzKkPhRrbO54908459 = -395228605;    int DzKkPhRrbO63551127 = -493482740;    int DzKkPhRrbO62281625 = -814707612;    int DzKkPhRrbO6140464 = -103232042;    int DzKkPhRrbO46957856 = -453252183;    int DzKkPhRrbO33712248 = -699502303;    int DzKkPhRrbO43753638 = -60571166;    int DzKkPhRrbO80949851 = -690110407;    int DzKkPhRrbO54990020 = -455747687;    int DzKkPhRrbO59241448 = -411370922;    int DzKkPhRrbO30615899 = -630591634;    int DzKkPhRrbO5900463 = -812572959;    int DzKkPhRrbO2227154 = -900209890;    int DzKkPhRrbO11951781 = -440705804;    int DzKkPhRrbO19528113 = -217065320;    int DzKkPhRrbO15354120 = -161638855;    int DzKkPhRrbO46701107 = -986673064;    int DzKkPhRrbO71190540 = -127593423;    int DzKkPhRrbO87986079 = -36853575;    int DzKkPhRrbO28490010 = -105572235;    int DzKkPhRrbO62790574 = -790233711;    int DzKkPhRrbO69423247 = -373122506;    int DzKkPhRrbO17115041 = 55657492;    int DzKkPhRrbO42736746 = -520587116;    int DzKkPhRrbO37004344 = -598070604;    int DzKkPhRrbO46953443 = 84638227;    int DzKkPhRrbO54683570 = -304640375;    int DzKkPhRrbO64440994 = -453923564;    int DzKkPhRrbO24275434 = -996450746;    int DzKkPhRrbO82569487 = -750183551;    int DzKkPhRrbO61873587 = 44349471;    int DzKkPhRrbO97179023 = -925171942;    int DzKkPhRrbO42166530 = -570479170;    int DzKkPhRrbO30149267 = -871891459;    int DzKkPhRrbO76927001 = -303568274;    int DzKkPhRrbO14712582 = -89403400;    int DzKkPhRrbO18147762 = -54338076;    int DzKkPhRrbO76311357 = 62344179;    int DzKkPhRrbO10775821 = -695735798;    int DzKkPhRrbO33346049 = -367018273;    int DzKkPhRrbO42546033 = -662810130;    int DzKkPhRrbO62262065 = -51850260;    int DzKkPhRrbO17520615 = -370056542;    int DzKkPhRrbO87717777 = -751429818;    int DzKkPhRrbO32263695 = -363548682;    int DzKkPhRrbO18713151 = -436385596;    int DzKkPhRrbO18124911 = -15158404;    int DzKkPhRrbO51869542 = -380366582;    int DzKkPhRrbO86339891 = -564449197;    int DzKkPhRrbO46910705 = -140870068;    int DzKkPhRrbO97707721 = -496009246;     DzKkPhRrbO14062636 = DzKkPhRrbO67490109;     DzKkPhRrbO67490109 = DzKkPhRrbO81968336;     DzKkPhRrbO81968336 = DzKkPhRrbO38122917;     DzKkPhRrbO38122917 = DzKkPhRrbO98592812;     DzKkPhRrbO98592812 = DzKkPhRrbO11978462;     DzKkPhRrbO11978462 = DzKkPhRrbO49207236;     DzKkPhRrbO49207236 = DzKkPhRrbO43787241;     DzKkPhRrbO43787241 = DzKkPhRrbO97791820;     DzKkPhRrbO97791820 = DzKkPhRrbO95783506;     DzKkPhRrbO95783506 = DzKkPhRrbO29152928;     DzKkPhRrbO29152928 = DzKkPhRrbO28924218;     DzKkPhRrbO28924218 = DzKkPhRrbO55044376;     DzKkPhRrbO55044376 = DzKkPhRrbO14789681;     DzKkPhRrbO14789681 = DzKkPhRrbO6002491;     DzKkPhRrbO6002491 = DzKkPhRrbO41933881;     DzKkPhRrbO41933881 = DzKkPhRrbO64074266;     DzKkPhRrbO64074266 = DzKkPhRrbO5616339;     DzKkPhRrbO5616339 = DzKkPhRrbO61617958;     DzKkPhRrbO61617958 = DzKkPhRrbO67305507;     DzKkPhRrbO67305507 = DzKkPhRrbO18504893;     DzKkPhRrbO18504893 = DzKkPhRrbO70308998;     DzKkPhRrbO70308998 = DzKkPhRrbO86540257;     DzKkPhRrbO86540257 = DzKkPhRrbO26978316;     DzKkPhRrbO26978316 = DzKkPhRrbO78881469;     DzKkPhRrbO78881469 = DzKkPhRrbO67976914;     DzKkPhRrbO67976914 = DzKkPhRrbO6078000;     DzKkPhRrbO6078000 = DzKkPhRrbO46980083;     DzKkPhRrbO46980083 = DzKkPhRrbO31835461;     DzKkPhRrbO31835461 = DzKkPhRrbO78263708;     DzKkPhRrbO78263708 = DzKkPhRrbO80429387;     DzKkPhRrbO80429387 = DzKkPhRrbO82451821;     DzKkPhRrbO82451821 = DzKkPhRrbO57733677;     DzKkPhRrbO57733677 = DzKkPhRrbO67058297;     DzKkPhRrbO67058297 = DzKkPhRrbO86299671;     DzKkPhRrbO86299671 = DzKkPhRrbO43211917;     DzKkPhRrbO43211917 = DzKkPhRrbO72510634;     DzKkPhRrbO72510634 = DzKkPhRrbO46959225;     DzKkPhRrbO46959225 = DzKkPhRrbO62879593;     DzKkPhRrbO62879593 = DzKkPhRrbO24613614;     DzKkPhRrbO24613614 = DzKkPhRrbO20352064;     DzKkPhRrbO20352064 = DzKkPhRrbO63821322;     DzKkPhRrbO63821322 = DzKkPhRrbO5868004;     DzKkPhRrbO5868004 = DzKkPhRrbO62264824;     DzKkPhRrbO62264824 = DzKkPhRrbO44408829;     DzKkPhRrbO44408829 = DzKkPhRrbO17007882;     DzKkPhRrbO17007882 = DzKkPhRrbO70797890;     DzKkPhRrbO70797890 = DzKkPhRrbO63911469;     DzKkPhRrbO63911469 = DzKkPhRrbO16830817;     DzKkPhRrbO16830817 = DzKkPhRrbO54908459;     DzKkPhRrbO54908459 = DzKkPhRrbO63551127;     DzKkPhRrbO63551127 = DzKkPhRrbO62281625;     DzKkPhRrbO62281625 = DzKkPhRrbO6140464;     DzKkPhRrbO6140464 = DzKkPhRrbO46957856;     DzKkPhRrbO46957856 = DzKkPhRrbO33712248;     DzKkPhRrbO33712248 = DzKkPhRrbO43753638;     DzKkPhRrbO43753638 = DzKkPhRrbO80949851;     DzKkPhRrbO80949851 = DzKkPhRrbO54990020;     DzKkPhRrbO54990020 = DzKkPhRrbO59241448;     DzKkPhRrbO59241448 = DzKkPhRrbO30615899;     DzKkPhRrbO30615899 = DzKkPhRrbO5900463;     DzKkPhRrbO5900463 = DzKkPhRrbO2227154;     DzKkPhRrbO2227154 = DzKkPhRrbO11951781;     DzKkPhRrbO11951781 = DzKkPhRrbO19528113;     DzKkPhRrbO19528113 = DzKkPhRrbO15354120;     DzKkPhRrbO15354120 = DzKkPhRrbO46701107;     DzKkPhRrbO46701107 = DzKkPhRrbO71190540;     DzKkPhRrbO71190540 = DzKkPhRrbO87986079;     DzKkPhRrbO87986079 = DzKkPhRrbO28490010;     DzKkPhRrbO28490010 = DzKkPhRrbO62790574;     DzKkPhRrbO62790574 = DzKkPhRrbO69423247;     DzKkPhRrbO69423247 = DzKkPhRrbO17115041;     DzKkPhRrbO17115041 = DzKkPhRrbO42736746;     DzKkPhRrbO42736746 = DzKkPhRrbO37004344;     DzKkPhRrbO37004344 = DzKkPhRrbO46953443;     DzKkPhRrbO46953443 = DzKkPhRrbO54683570;     DzKkPhRrbO54683570 = DzKkPhRrbO64440994;     DzKkPhRrbO64440994 = DzKkPhRrbO24275434;     DzKkPhRrbO24275434 = DzKkPhRrbO82569487;     DzKkPhRrbO82569487 = DzKkPhRrbO61873587;     DzKkPhRrbO61873587 = DzKkPhRrbO97179023;     DzKkPhRrbO97179023 = DzKkPhRrbO42166530;     DzKkPhRrbO42166530 = DzKkPhRrbO30149267;     DzKkPhRrbO30149267 = DzKkPhRrbO76927001;     DzKkPhRrbO76927001 = DzKkPhRrbO14712582;     DzKkPhRrbO14712582 = DzKkPhRrbO18147762;     DzKkPhRrbO18147762 = DzKkPhRrbO76311357;     DzKkPhRrbO76311357 = DzKkPhRrbO10775821;     DzKkPhRrbO10775821 = DzKkPhRrbO33346049;     DzKkPhRrbO33346049 = DzKkPhRrbO42546033;     DzKkPhRrbO42546033 = DzKkPhRrbO62262065;     DzKkPhRrbO62262065 = DzKkPhRrbO17520615;     DzKkPhRrbO17520615 = DzKkPhRrbO87717777;     DzKkPhRrbO87717777 = DzKkPhRrbO32263695;     DzKkPhRrbO32263695 = DzKkPhRrbO18713151;     DzKkPhRrbO18713151 = DzKkPhRrbO18124911;     DzKkPhRrbO18124911 = DzKkPhRrbO51869542;     DzKkPhRrbO51869542 = DzKkPhRrbO86339891;     DzKkPhRrbO86339891 = DzKkPhRrbO46910705;     DzKkPhRrbO46910705 = DzKkPhRrbO97707721;     DzKkPhRrbO97707721 = DzKkPhRrbO14062636;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xDdntPvvCs52659244() {     int abFJAbbKdJ49242392 = -392929760;    int abFJAbbKdJ36182 = -165352017;    int abFJAbbKdJ27177203 = -201248557;    int abFJAbbKdJ75990570 = -415626307;    int abFJAbbKdJ81438069 = -674180815;    int abFJAbbKdJ35366217 = -427301933;    int abFJAbbKdJ94023397 = -348129238;    int abFJAbbKdJ76926162 = -350440266;    int abFJAbbKdJ7619599 = -32259520;    int abFJAbbKdJ97126282 = -40706637;    int abFJAbbKdJ73861564 = -279399270;    int abFJAbbKdJ2485995 = -299181150;    int abFJAbbKdJ60342945 = -704215651;    int abFJAbbKdJ53184291 = -443924434;    int abFJAbbKdJ20872212 = -203945070;    int abFJAbbKdJ40050315 = -898690016;    int abFJAbbKdJ13796676 = -328582964;    int abFJAbbKdJ72150707 = -224608801;    int abFJAbbKdJ83631949 = -373618211;    int abFJAbbKdJ76402523 = -170584208;    int abFJAbbKdJ19892633 = -998049958;    int abFJAbbKdJ84875769 = -500070858;    int abFJAbbKdJ91715479 = -47086312;    int abFJAbbKdJ54335125 = 1966394;    int abFJAbbKdJ91873285 = -399493603;    int abFJAbbKdJ21024477 = -655190532;    int abFJAbbKdJ94361374 = -767123155;    int abFJAbbKdJ84840933 = -238082009;    int abFJAbbKdJ42666628 = -711200923;    int abFJAbbKdJ64082133 = -557636786;    int abFJAbbKdJ35743509 = -939685355;    int abFJAbbKdJ58587978 = -379857134;    int abFJAbbKdJ99365873 = -352242044;    int abFJAbbKdJ3240520 = -418719911;    int abFJAbbKdJ63231529 = -137855674;    int abFJAbbKdJ37300712 = -866153402;    int abFJAbbKdJ90773038 = -349008153;    int abFJAbbKdJ88371669 = 45216375;    int abFJAbbKdJ81649513 = -624355508;    int abFJAbbKdJ99191165 = -620167743;    int abFJAbbKdJ22685075 = -706076055;    int abFJAbbKdJ74334508 = -383500328;    int abFJAbbKdJ86018305 = -233348291;    int abFJAbbKdJ47105583 = -84080370;    int abFJAbbKdJ8382534 = -633086745;    int abFJAbbKdJ58308325 = -282038743;    int abFJAbbKdJ32979515 = 1456221;    int abFJAbbKdJ62566529 = -409277339;    int abFJAbbKdJ36696382 = -462459792;    int abFJAbbKdJ78897202 = -461260440;    int abFJAbbKdJ77488467 = -422888318;    int abFJAbbKdJ49762172 = -234566484;    int abFJAbbKdJ43491669 = -59092547;    int abFJAbbKdJ41693856 = -886464883;    int abFJAbbKdJ32766612 = 22248374;    int abFJAbbKdJ64366622 = -892858902;    int abFJAbbKdJ8320702 = -18265706;    int abFJAbbKdJ72842078 = -103214952;    int abFJAbbKdJ84117285 = 83867295;    int abFJAbbKdJ60413592 = 81009716;    int abFJAbbKdJ41004843 = -660178778;    int abFJAbbKdJ9182465 = -10047230;    int abFJAbbKdJ34259534 = -639239343;    int abFJAbbKdJ43537465 = -474622734;    int abFJAbbKdJ61382773 = -101021282;    int abFJAbbKdJ15273587 = -899542137;    int abFJAbbKdJ3120122 = -946939107;    int abFJAbbKdJ57102426 = -185495741;    int abFJAbbKdJ89952762 = -206068760;    int abFJAbbKdJ83571500 = -337791669;    int abFJAbbKdJ49277276 = -449681863;    int abFJAbbKdJ25425007 = -273799340;    int abFJAbbKdJ90501193 = -600253294;    int abFJAbbKdJ84440783 = -753450469;    int abFJAbbKdJ53717448 = -464508154;    int abFJAbbKdJ45558124 = -514549630;    int abFJAbbKdJ98857464 = -166722568;    int abFJAbbKdJ44609897 = -963005942;    int abFJAbbKdJ45952592 = -364946861;    int abFJAbbKdJ33564961 = -17454861;    int abFJAbbKdJ88044962 = -556646753;    int abFJAbbKdJ31794845 = -257845816;    int abFJAbbKdJ48144552 = -775622217;    int abFJAbbKdJ63769425 = -149940483;    int abFJAbbKdJ86593666 = -34748469;    int abFJAbbKdJ85981337 = -605118872;    int abFJAbbKdJ15096310 = -220764588;    int abFJAbbKdJ57672017 = -465777161;    int abFJAbbKdJ70473908 = -340968285;    int abFJAbbKdJ98864907 = -244996773;    int abFJAbbKdJ28980010 = -747887696;    int abFJAbbKdJ17930961 = -145793201;    int abFJAbbKdJ4254385 = 61349080;    int abFJAbbKdJ21235922 = -605365224;    int abFJAbbKdJ58186322 = -959988965;    int abFJAbbKdJ13502611 = -596028825;    int abFJAbbKdJ40074974 = -744260986;    int abFJAbbKdJ42480840 = -758725557;    int abFJAbbKdJ85722810 = -983059089;    int abFJAbbKdJ93108946 = -392929760;     abFJAbbKdJ49242392 = abFJAbbKdJ36182;     abFJAbbKdJ36182 = abFJAbbKdJ27177203;     abFJAbbKdJ27177203 = abFJAbbKdJ75990570;     abFJAbbKdJ75990570 = abFJAbbKdJ81438069;     abFJAbbKdJ81438069 = abFJAbbKdJ35366217;     abFJAbbKdJ35366217 = abFJAbbKdJ94023397;     abFJAbbKdJ94023397 = abFJAbbKdJ76926162;     abFJAbbKdJ76926162 = abFJAbbKdJ7619599;     abFJAbbKdJ7619599 = abFJAbbKdJ97126282;     abFJAbbKdJ97126282 = abFJAbbKdJ73861564;     abFJAbbKdJ73861564 = abFJAbbKdJ2485995;     abFJAbbKdJ2485995 = abFJAbbKdJ60342945;     abFJAbbKdJ60342945 = abFJAbbKdJ53184291;     abFJAbbKdJ53184291 = abFJAbbKdJ20872212;     abFJAbbKdJ20872212 = abFJAbbKdJ40050315;     abFJAbbKdJ40050315 = abFJAbbKdJ13796676;     abFJAbbKdJ13796676 = abFJAbbKdJ72150707;     abFJAbbKdJ72150707 = abFJAbbKdJ83631949;     abFJAbbKdJ83631949 = abFJAbbKdJ76402523;     abFJAbbKdJ76402523 = abFJAbbKdJ19892633;     abFJAbbKdJ19892633 = abFJAbbKdJ84875769;     abFJAbbKdJ84875769 = abFJAbbKdJ91715479;     abFJAbbKdJ91715479 = abFJAbbKdJ54335125;     abFJAbbKdJ54335125 = abFJAbbKdJ91873285;     abFJAbbKdJ91873285 = abFJAbbKdJ21024477;     abFJAbbKdJ21024477 = abFJAbbKdJ94361374;     abFJAbbKdJ94361374 = abFJAbbKdJ84840933;     abFJAbbKdJ84840933 = abFJAbbKdJ42666628;     abFJAbbKdJ42666628 = abFJAbbKdJ64082133;     abFJAbbKdJ64082133 = abFJAbbKdJ35743509;     abFJAbbKdJ35743509 = abFJAbbKdJ58587978;     abFJAbbKdJ58587978 = abFJAbbKdJ99365873;     abFJAbbKdJ99365873 = abFJAbbKdJ3240520;     abFJAbbKdJ3240520 = abFJAbbKdJ63231529;     abFJAbbKdJ63231529 = abFJAbbKdJ37300712;     abFJAbbKdJ37300712 = abFJAbbKdJ90773038;     abFJAbbKdJ90773038 = abFJAbbKdJ88371669;     abFJAbbKdJ88371669 = abFJAbbKdJ81649513;     abFJAbbKdJ81649513 = abFJAbbKdJ99191165;     abFJAbbKdJ99191165 = abFJAbbKdJ22685075;     abFJAbbKdJ22685075 = abFJAbbKdJ74334508;     abFJAbbKdJ74334508 = abFJAbbKdJ86018305;     abFJAbbKdJ86018305 = abFJAbbKdJ47105583;     abFJAbbKdJ47105583 = abFJAbbKdJ8382534;     abFJAbbKdJ8382534 = abFJAbbKdJ58308325;     abFJAbbKdJ58308325 = abFJAbbKdJ32979515;     abFJAbbKdJ32979515 = abFJAbbKdJ62566529;     abFJAbbKdJ62566529 = abFJAbbKdJ36696382;     abFJAbbKdJ36696382 = abFJAbbKdJ78897202;     abFJAbbKdJ78897202 = abFJAbbKdJ77488467;     abFJAbbKdJ77488467 = abFJAbbKdJ49762172;     abFJAbbKdJ49762172 = abFJAbbKdJ43491669;     abFJAbbKdJ43491669 = abFJAbbKdJ41693856;     abFJAbbKdJ41693856 = abFJAbbKdJ32766612;     abFJAbbKdJ32766612 = abFJAbbKdJ64366622;     abFJAbbKdJ64366622 = abFJAbbKdJ8320702;     abFJAbbKdJ8320702 = abFJAbbKdJ72842078;     abFJAbbKdJ72842078 = abFJAbbKdJ84117285;     abFJAbbKdJ84117285 = abFJAbbKdJ60413592;     abFJAbbKdJ60413592 = abFJAbbKdJ41004843;     abFJAbbKdJ41004843 = abFJAbbKdJ9182465;     abFJAbbKdJ9182465 = abFJAbbKdJ34259534;     abFJAbbKdJ34259534 = abFJAbbKdJ43537465;     abFJAbbKdJ43537465 = abFJAbbKdJ61382773;     abFJAbbKdJ61382773 = abFJAbbKdJ15273587;     abFJAbbKdJ15273587 = abFJAbbKdJ3120122;     abFJAbbKdJ3120122 = abFJAbbKdJ57102426;     abFJAbbKdJ57102426 = abFJAbbKdJ89952762;     abFJAbbKdJ89952762 = abFJAbbKdJ83571500;     abFJAbbKdJ83571500 = abFJAbbKdJ49277276;     abFJAbbKdJ49277276 = abFJAbbKdJ25425007;     abFJAbbKdJ25425007 = abFJAbbKdJ90501193;     abFJAbbKdJ90501193 = abFJAbbKdJ84440783;     abFJAbbKdJ84440783 = abFJAbbKdJ53717448;     abFJAbbKdJ53717448 = abFJAbbKdJ45558124;     abFJAbbKdJ45558124 = abFJAbbKdJ98857464;     abFJAbbKdJ98857464 = abFJAbbKdJ44609897;     abFJAbbKdJ44609897 = abFJAbbKdJ45952592;     abFJAbbKdJ45952592 = abFJAbbKdJ33564961;     abFJAbbKdJ33564961 = abFJAbbKdJ88044962;     abFJAbbKdJ88044962 = abFJAbbKdJ31794845;     abFJAbbKdJ31794845 = abFJAbbKdJ48144552;     abFJAbbKdJ48144552 = abFJAbbKdJ63769425;     abFJAbbKdJ63769425 = abFJAbbKdJ86593666;     abFJAbbKdJ86593666 = abFJAbbKdJ85981337;     abFJAbbKdJ85981337 = abFJAbbKdJ15096310;     abFJAbbKdJ15096310 = abFJAbbKdJ57672017;     abFJAbbKdJ57672017 = abFJAbbKdJ70473908;     abFJAbbKdJ70473908 = abFJAbbKdJ98864907;     abFJAbbKdJ98864907 = abFJAbbKdJ28980010;     abFJAbbKdJ28980010 = abFJAbbKdJ17930961;     abFJAbbKdJ17930961 = abFJAbbKdJ4254385;     abFJAbbKdJ4254385 = abFJAbbKdJ21235922;     abFJAbbKdJ21235922 = abFJAbbKdJ58186322;     abFJAbbKdJ58186322 = abFJAbbKdJ13502611;     abFJAbbKdJ13502611 = abFJAbbKdJ40074974;     abFJAbbKdJ40074974 = abFJAbbKdJ42480840;     abFJAbbKdJ42480840 = abFJAbbKdJ85722810;     abFJAbbKdJ85722810 = abFJAbbKdJ93108946;     abFJAbbKdJ93108946 = abFJAbbKdJ49242392;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WtIaWLhHXq23115117() {     int xWHhRsdhGt62028559 = -527970451;    int xWHhRsdhGt86807069 = -335686031;    int xWHhRsdhGt44930002 = -609607939;    int xWHhRsdhGt83931396 = -239828584;    int xWHhRsdhGt74949173 = -65997280;    int xWHhRsdhGt78676288 = -204225246;    int xWHhRsdhGt15231156 = -893801308;    int xWHhRsdhGt3645275 = -794958523;    int xWHhRsdhGt3024409 = 5982388;    int xWHhRsdhGt3344987 = -944794270;    int xWHhRsdhGt1348947 = -394085286;    int xWHhRsdhGt34436499 = -806513966;    int xWHhRsdhGt30486249 = -607998308;    int xWHhRsdhGt2808371 = -776339632;    int xWHhRsdhGt44178765 = -110930901;    int xWHhRsdhGt68475384 = -349581405;    int xWHhRsdhGt69548493 = 7642569;    int xWHhRsdhGt55709192 = -879819103;    int xWHhRsdhGt84792579 = -283087407;    int xWHhRsdhGt79057019 = -769828347;    int xWHhRsdhGt66690 = -389366223;    int xWHhRsdhGt13923589 = -669929206;    int xWHhRsdhGt30475776 = 21909793;    int xWHhRsdhGt98842254 = -358378618;    int xWHhRsdhGt97767544 = -609935527;    int xWHhRsdhGt2343123 = -345621731;    int xWHhRsdhGt5776911 = -300829435;    int xWHhRsdhGt52193858 = -411880113;    int xWHhRsdhGt59283681 = -925418163;    int xWHhRsdhGt16833249 = -187522941;    int xWHhRsdhGt80179505 = -116102105;    int xWHhRsdhGt61906802 = -358576812;    int xWHhRsdhGt45411822 = -228096828;    int xWHhRsdhGt80316892 = -323474129;    int xWHhRsdhGt46292062 = -747887555;    int xWHhRsdhGt32524826 = -654716506;    int xWHhRsdhGt27537099 = -138917503;    int xWHhRsdhGt46075692 = -974403716;    int xWHhRsdhGt87424899 = -466206085;    int xWHhRsdhGt25455895 = -464584588;    int xWHhRsdhGt84116383 = 18449952;    int xWHhRsdhGt82036964 = -281811663;    int xWHhRsdhGt20901775 = -256781736;    int xWHhRsdhGt91734120 = -425252613;    int xWHhRsdhGt88391023 = -525856705;    int xWHhRsdhGt31936454 = -355357182;    int xWHhRsdhGt10561535 = -496058831;    int xWHhRsdhGt92006495 = -564630119;    int xWHhRsdhGt80760431 = -626061767;    int xWHhRsdhGt25847623 = 27703097;    int xWHhRsdhGt95815960 = -675772626;    int xWHhRsdhGt77938719 = -705751822;    int xWHhRsdhGt77976819 = -235095829;    int xWHhRsdhGt86393581 = -475916625;    int xWHhRsdhGt23048428 = 27803887;    int xWHhRsdhGt48104970 = -858041246;    int xWHhRsdhGt56331294 = -257595825;    int xWHhRsdhGt46087748 = -151229322;    int xWHhRsdhGt86163851 = -629893057;    int xWHhRsdhGt72606051 = -720375550;    int xWHhRsdhGt72899377 = -903395811;    int xWHhRsdhGt63037297 = -381921195;    int xWHhRsdhGt44361594 = -869540360;    int xWHhRsdhGt86191160 = -806494671;    int xWHhRsdhGt23165482 = -728692165;    int xWHhRsdhGt39442145 = 64491526;    int xWHhRsdhGt89024677 = -478417138;    int xWHhRsdhGt50169356 = -184524180;    int xWHhRsdhGt56516308 = 71547922;    int xWHhRsdhGt11653939 = -456214396;    int xWHhRsdhGt40938285 = -110663902;    int xWHhRsdhGt23472802 = -17953715;    int xWHhRsdhGt68284293 = -313613019;    int xWHhRsdhGt59336684 = -818502819;    int xWHhRsdhGt94940636 = -688278300;    int xWHhRsdhGt18029725 = -7554560;    int xWHhRsdhGt93021813 = -313147471;    int xWHhRsdhGt38741655 = -552837594;    int xWHhRsdhGt10451231 = -832521913;    int xWHhRsdhGt65831090 = -154578345;    int xWHhRsdhGt91781587 = -849562900;    int xWHhRsdhGt13770415 = -736199317;    int xWHhRsdhGt71433427 = -785818347;    int xWHhRsdhGt33436058 = -853121260;    int xWHhRsdhGt21017288 = -511750315;    int xWHhRsdhGt2240786 = -410350284;    int xWHhRsdhGt83929983 = -23480984;    int xWHhRsdhGt59018241 = -752180204;    int xWHhRsdhGt57268465 = -251278016;    int xWHhRsdhGt98187091 = -889846310;    int xWHhRsdhGt76193532 = -297120681;    int xWHhRsdhGt81449351 = -987688182;    int xWHhRsdhGt59911841 = -244510659;    int xWHhRsdhGt14818849 = -745830536;    int xWHhRsdhGt52556517 = -561188778;    int xWHhRsdhGt21079086 = -599628853;    int xWHhRsdhGt37675371 = -412271303;    int xWHhRsdhGt34710615 = -450287065;    int xWHhRsdhGt68568639 = -696560449;    int xWHhRsdhGt48948879 = -527970451;     xWHhRsdhGt62028559 = xWHhRsdhGt86807069;     xWHhRsdhGt86807069 = xWHhRsdhGt44930002;     xWHhRsdhGt44930002 = xWHhRsdhGt83931396;     xWHhRsdhGt83931396 = xWHhRsdhGt74949173;     xWHhRsdhGt74949173 = xWHhRsdhGt78676288;     xWHhRsdhGt78676288 = xWHhRsdhGt15231156;     xWHhRsdhGt15231156 = xWHhRsdhGt3645275;     xWHhRsdhGt3645275 = xWHhRsdhGt3024409;     xWHhRsdhGt3024409 = xWHhRsdhGt3344987;     xWHhRsdhGt3344987 = xWHhRsdhGt1348947;     xWHhRsdhGt1348947 = xWHhRsdhGt34436499;     xWHhRsdhGt34436499 = xWHhRsdhGt30486249;     xWHhRsdhGt30486249 = xWHhRsdhGt2808371;     xWHhRsdhGt2808371 = xWHhRsdhGt44178765;     xWHhRsdhGt44178765 = xWHhRsdhGt68475384;     xWHhRsdhGt68475384 = xWHhRsdhGt69548493;     xWHhRsdhGt69548493 = xWHhRsdhGt55709192;     xWHhRsdhGt55709192 = xWHhRsdhGt84792579;     xWHhRsdhGt84792579 = xWHhRsdhGt79057019;     xWHhRsdhGt79057019 = xWHhRsdhGt66690;     xWHhRsdhGt66690 = xWHhRsdhGt13923589;     xWHhRsdhGt13923589 = xWHhRsdhGt30475776;     xWHhRsdhGt30475776 = xWHhRsdhGt98842254;     xWHhRsdhGt98842254 = xWHhRsdhGt97767544;     xWHhRsdhGt97767544 = xWHhRsdhGt2343123;     xWHhRsdhGt2343123 = xWHhRsdhGt5776911;     xWHhRsdhGt5776911 = xWHhRsdhGt52193858;     xWHhRsdhGt52193858 = xWHhRsdhGt59283681;     xWHhRsdhGt59283681 = xWHhRsdhGt16833249;     xWHhRsdhGt16833249 = xWHhRsdhGt80179505;     xWHhRsdhGt80179505 = xWHhRsdhGt61906802;     xWHhRsdhGt61906802 = xWHhRsdhGt45411822;     xWHhRsdhGt45411822 = xWHhRsdhGt80316892;     xWHhRsdhGt80316892 = xWHhRsdhGt46292062;     xWHhRsdhGt46292062 = xWHhRsdhGt32524826;     xWHhRsdhGt32524826 = xWHhRsdhGt27537099;     xWHhRsdhGt27537099 = xWHhRsdhGt46075692;     xWHhRsdhGt46075692 = xWHhRsdhGt87424899;     xWHhRsdhGt87424899 = xWHhRsdhGt25455895;     xWHhRsdhGt25455895 = xWHhRsdhGt84116383;     xWHhRsdhGt84116383 = xWHhRsdhGt82036964;     xWHhRsdhGt82036964 = xWHhRsdhGt20901775;     xWHhRsdhGt20901775 = xWHhRsdhGt91734120;     xWHhRsdhGt91734120 = xWHhRsdhGt88391023;     xWHhRsdhGt88391023 = xWHhRsdhGt31936454;     xWHhRsdhGt31936454 = xWHhRsdhGt10561535;     xWHhRsdhGt10561535 = xWHhRsdhGt92006495;     xWHhRsdhGt92006495 = xWHhRsdhGt80760431;     xWHhRsdhGt80760431 = xWHhRsdhGt25847623;     xWHhRsdhGt25847623 = xWHhRsdhGt95815960;     xWHhRsdhGt95815960 = xWHhRsdhGt77938719;     xWHhRsdhGt77938719 = xWHhRsdhGt77976819;     xWHhRsdhGt77976819 = xWHhRsdhGt86393581;     xWHhRsdhGt86393581 = xWHhRsdhGt23048428;     xWHhRsdhGt23048428 = xWHhRsdhGt48104970;     xWHhRsdhGt48104970 = xWHhRsdhGt56331294;     xWHhRsdhGt56331294 = xWHhRsdhGt46087748;     xWHhRsdhGt46087748 = xWHhRsdhGt86163851;     xWHhRsdhGt86163851 = xWHhRsdhGt72606051;     xWHhRsdhGt72606051 = xWHhRsdhGt72899377;     xWHhRsdhGt72899377 = xWHhRsdhGt63037297;     xWHhRsdhGt63037297 = xWHhRsdhGt44361594;     xWHhRsdhGt44361594 = xWHhRsdhGt86191160;     xWHhRsdhGt86191160 = xWHhRsdhGt23165482;     xWHhRsdhGt23165482 = xWHhRsdhGt39442145;     xWHhRsdhGt39442145 = xWHhRsdhGt89024677;     xWHhRsdhGt89024677 = xWHhRsdhGt50169356;     xWHhRsdhGt50169356 = xWHhRsdhGt56516308;     xWHhRsdhGt56516308 = xWHhRsdhGt11653939;     xWHhRsdhGt11653939 = xWHhRsdhGt40938285;     xWHhRsdhGt40938285 = xWHhRsdhGt23472802;     xWHhRsdhGt23472802 = xWHhRsdhGt68284293;     xWHhRsdhGt68284293 = xWHhRsdhGt59336684;     xWHhRsdhGt59336684 = xWHhRsdhGt94940636;     xWHhRsdhGt94940636 = xWHhRsdhGt18029725;     xWHhRsdhGt18029725 = xWHhRsdhGt93021813;     xWHhRsdhGt93021813 = xWHhRsdhGt38741655;     xWHhRsdhGt38741655 = xWHhRsdhGt10451231;     xWHhRsdhGt10451231 = xWHhRsdhGt65831090;     xWHhRsdhGt65831090 = xWHhRsdhGt91781587;     xWHhRsdhGt91781587 = xWHhRsdhGt13770415;     xWHhRsdhGt13770415 = xWHhRsdhGt71433427;     xWHhRsdhGt71433427 = xWHhRsdhGt33436058;     xWHhRsdhGt33436058 = xWHhRsdhGt21017288;     xWHhRsdhGt21017288 = xWHhRsdhGt2240786;     xWHhRsdhGt2240786 = xWHhRsdhGt83929983;     xWHhRsdhGt83929983 = xWHhRsdhGt59018241;     xWHhRsdhGt59018241 = xWHhRsdhGt57268465;     xWHhRsdhGt57268465 = xWHhRsdhGt98187091;     xWHhRsdhGt98187091 = xWHhRsdhGt76193532;     xWHhRsdhGt76193532 = xWHhRsdhGt81449351;     xWHhRsdhGt81449351 = xWHhRsdhGt59911841;     xWHhRsdhGt59911841 = xWHhRsdhGt14818849;     xWHhRsdhGt14818849 = xWHhRsdhGt52556517;     xWHhRsdhGt52556517 = xWHhRsdhGt21079086;     xWHhRsdhGt21079086 = xWHhRsdhGt37675371;     xWHhRsdhGt37675371 = xWHhRsdhGt34710615;     xWHhRsdhGt34710615 = xWHhRsdhGt68568639;     xWHhRsdhGt68568639 = xWHhRsdhGt48948879;     xWHhRsdhGt48948879 = xWHhRsdhGt62028559;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MSXBbPIdGw41328458() {     int iuoZtAzHjD45472864 = -308636132;    int iuoZtAzHjD3679159 = -335068219;    int iuoZtAzHjD41338509 = -403133167;    int iuoZtAzHjD46723647 = -433628420;    int iuoZtAzHjD81070497 = -972864771;    int iuoZtAzHjD77180689 = 16703101;    int iuoZtAzHjD86270473 = -633712622;    int iuoZtAzHjD17992718 = -782116552;    int iuoZtAzHjD61554109 = -335476963;    int iuoZtAzHjD35890639 = -313170382;    int iuoZtAzHjD50764885 = -584201158;    int iuoZtAzHjD97641898 = -794132212;    int iuoZtAzHjD48604024 = -399659549;    int iuoZtAzHjD82435262 = -564629989;    int iuoZtAzHjD5033229 = -204953883;    int iuoZtAzHjD54864545 = -137660730;    int iuoZtAzHjD63300378 = -845702458;    int iuoZtAzHjD36560639 = -27123980;    int iuoZtAzHjD47232722 = -281762277;    int iuoZtAzHjD68424462 = -199414340;    int iuoZtAzHjD55707076 = 21104746;    int iuoZtAzHjD26070907 = -661080976;    int iuoZtAzHjD34301483 = -17979404;    int iuoZtAzHjD33548813 = -430204014;    int iuoZtAzHjD39343503 = -69588286;    int iuoZtAzHjD97685271 = -226131446;    int iuoZtAzHjD55231650 = 71086222;    int iuoZtAzHjD91925483 = -481834997;    int iuoZtAzHjD95364774 = -787923305;    int iuoZtAzHjD32666590 = -700372793;    int iuoZtAzHjD98274819 = -853859306;    int iuoZtAzHjD25013229 = -273771892;    int iuoZtAzHjD88950709 = -948969006;    int iuoZtAzHjD22475404 = 84923701;    int iuoZtAzHjD25018792 = -945552290;    int iuoZtAzHjD44914011 = -442740412;    int iuoZtAzHjD96151277 = -17766683;    int iuoZtAzHjD86654524 = -682874236;    int iuoZtAzHjD36419978 = -651103883;    int iuoZtAzHjD74951030 = -217178111;    int iuoZtAzHjD17765308 = -653099021;    int iuoZtAzHjD32524751 = -855719984;    int iuoZtAzHjD44468845 = -807838978;    int iuoZtAzHjD83639060 = -181435384;    int iuoZtAzHjD80335520 = -254013658;    int iuoZtAzHjD71542450 = -374968106;    int iuoZtAzHjD86576964 = -628363061;    int iuoZtAzHjD11677092 = -563351837;    int iuoZtAzHjD11607544 = -111663849;    int iuoZtAzHjD61180107 = -179317359;    int iuoZtAzHjD69073409 = -896674049;    int iuoZtAzHjD33439514 = -764011133;    int iuoZtAzHjD92089277 = -446843146;    int iuoZtAzHjD11070340 = -194330012;    int iuoZtAzHjD85621382 = -320682236;    int iuoZtAzHjD19401957 = -647555156;    int iuoZtAzHjD69377675 = -217088815;    int iuoZtAzHjD7789697 = -972929153;    int iuoZtAzHjD7380145 = -264040134;    int iuoZtAzHjD83385226 = -646733325;    int iuoZtAzHjD21949039 = 45616878;    int iuoZtAzHjD94344989 = -51877625;    int iuoZtAzHjD22627943 = -994193247;    int iuoZtAzHjD28887520 = -635104171;    int iuoZtAzHjD37615820 = -459311076;    int iuoZtAzHjD25751656 = -210429267;    int iuoZtAzHjD8691190 = -845163207;    int iuoZtAzHjD26128621 = -384583251;    int iuoZtAzHjD57416470 = -619077700;    int iuoZtAzHjD60119218 = -762213472;    int iuoZtAzHjD58713268 = -19894048;    int iuoZtAzHjD76645854 = -62828222;    int iuoZtAzHjD140662 = -376020098;    int iuoZtAzHjD72281691 = 35415833;    int iuoZtAzHjD50659155 = -546315319;    int iuoZtAzHjD23182326 = -123175270;    int iuoZtAzHjD81602062 = -853241999;    int iuoZtAzHjD50662422 = -836544020;    int iuoZtAzHjD53213292 = -76190356;    int iuoZtAzHjD67801052 = -694620181;    int iuoZtAzHjD11108307 = -597768386;    int iuoZtAzHjD43554559 = -365561941;    int iuoZtAzHjD80317940 = -270171148;    int iuoZtAzHjD34184668 = -508605947;    int iuoZtAzHjD63593181 = -803698745;    int iuoZtAzHjD64835305 = 10151827;    int iuoZtAzHjD32923952 = -826928747;    int iuoZtAzHjD77880369 = -654638994;    int iuoZtAzHjD36854021 = -594394063;    int iuoZtAzHjD5616835 = -197997134;    int iuoZtAzHjD75536335 = -125651597;    int iuoZtAzHjD88361581 = -44837530;    int iuoZtAzHjD79274379 = -318834102;    int iuoZtAzHjD53034752 = 95629442;    int iuoZtAzHjD53001992 = -162794989;    int iuoZtAzHjD23420318 = -501221397;    int iuoZtAzHjD9896808 = -861526738;    int iuoZtAzHjD15581326 = -72734808;    int iuoZtAzHjD46023241 = -722124309;    int iuoZtAzHjD54583865 = -308636132;     iuoZtAzHjD45472864 = iuoZtAzHjD3679159;     iuoZtAzHjD3679159 = iuoZtAzHjD41338509;     iuoZtAzHjD41338509 = iuoZtAzHjD46723647;     iuoZtAzHjD46723647 = iuoZtAzHjD81070497;     iuoZtAzHjD81070497 = iuoZtAzHjD77180689;     iuoZtAzHjD77180689 = iuoZtAzHjD86270473;     iuoZtAzHjD86270473 = iuoZtAzHjD17992718;     iuoZtAzHjD17992718 = iuoZtAzHjD61554109;     iuoZtAzHjD61554109 = iuoZtAzHjD35890639;     iuoZtAzHjD35890639 = iuoZtAzHjD50764885;     iuoZtAzHjD50764885 = iuoZtAzHjD97641898;     iuoZtAzHjD97641898 = iuoZtAzHjD48604024;     iuoZtAzHjD48604024 = iuoZtAzHjD82435262;     iuoZtAzHjD82435262 = iuoZtAzHjD5033229;     iuoZtAzHjD5033229 = iuoZtAzHjD54864545;     iuoZtAzHjD54864545 = iuoZtAzHjD63300378;     iuoZtAzHjD63300378 = iuoZtAzHjD36560639;     iuoZtAzHjD36560639 = iuoZtAzHjD47232722;     iuoZtAzHjD47232722 = iuoZtAzHjD68424462;     iuoZtAzHjD68424462 = iuoZtAzHjD55707076;     iuoZtAzHjD55707076 = iuoZtAzHjD26070907;     iuoZtAzHjD26070907 = iuoZtAzHjD34301483;     iuoZtAzHjD34301483 = iuoZtAzHjD33548813;     iuoZtAzHjD33548813 = iuoZtAzHjD39343503;     iuoZtAzHjD39343503 = iuoZtAzHjD97685271;     iuoZtAzHjD97685271 = iuoZtAzHjD55231650;     iuoZtAzHjD55231650 = iuoZtAzHjD91925483;     iuoZtAzHjD91925483 = iuoZtAzHjD95364774;     iuoZtAzHjD95364774 = iuoZtAzHjD32666590;     iuoZtAzHjD32666590 = iuoZtAzHjD98274819;     iuoZtAzHjD98274819 = iuoZtAzHjD25013229;     iuoZtAzHjD25013229 = iuoZtAzHjD88950709;     iuoZtAzHjD88950709 = iuoZtAzHjD22475404;     iuoZtAzHjD22475404 = iuoZtAzHjD25018792;     iuoZtAzHjD25018792 = iuoZtAzHjD44914011;     iuoZtAzHjD44914011 = iuoZtAzHjD96151277;     iuoZtAzHjD96151277 = iuoZtAzHjD86654524;     iuoZtAzHjD86654524 = iuoZtAzHjD36419978;     iuoZtAzHjD36419978 = iuoZtAzHjD74951030;     iuoZtAzHjD74951030 = iuoZtAzHjD17765308;     iuoZtAzHjD17765308 = iuoZtAzHjD32524751;     iuoZtAzHjD32524751 = iuoZtAzHjD44468845;     iuoZtAzHjD44468845 = iuoZtAzHjD83639060;     iuoZtAzHjD83639060 = iuoZtAzHjD80335520;     iuoZtAzHjD80335520 = iuoZtAzHjD71542450;     iuoZtAzHjD71542450 = iuoZtAzHjD86576964;     iuoZtAzHjD86576964 = iuoZtAzHjD11677092;     iuoZtAzHjD11677092 = iuoZtAzHjD11607544;     iuoZtAzHjD11607544 = iuoZtAzHjD61180107;     iuoZtAzHjD61180107 = iuoZtAzHjD69073409;     iuoZtAzHjD69073409 = iuoZtAzHjD33439514;     iuoZtAzHjD33439514 = iuoZtAzHjD92089277;     iuoZtAzHjD92089277 = iuoZtAzHjD11070340;     iuoZtAzHjD11070340 = iuoZtAzHjD85621382;     iuoZtAzHjD85621382 = iuoZtAzHjD19401957;     iuoZtAzHjD19401957 = iuoZtAzHjD69377675;     iuoZtAzHjD69377675 = iuoZtAzHjD7789697;     iuoZtAzHjD7789697 = iuoZtAzHjD7380145;     iuoZtAzHjD7380145 = iuoZtAzHjD83385226;     iuoZtAzHjD83385226 = iuoZtAzHjD21949039;     iuoZtAzHjD21949039 = iuoZtAzHjD94344989;     iuoZtAzHjD94344989 = iuoZtAzHjD22627943;     iuoZtAzHjD22627943 = iuoZtAzHjD28887520;     iuoZtAzHjD28887520 = iuoZtAzHjD37615820;     iuoZtAzHjD37615820 = iuoZtAzHjD25751656;     iuoZtAzHjD25751656 = iuoZtAzHjD8691190;     iuoZtAzHjD8691190 = iuoZtAzHjD26128621;     iuoZtAzHjD26128621 = iuoZtAzHjD57416470;     iuoZtAzHjD57416470 = iuoZtAzHjD60119218;     iuoZtAzHjD60119218 = iuoZtAzHjD58713268;     iuoZtAzHjD58713268 = iuoZtAzHjD76645854;     iuoZtAzHjD76645854 = iuoZtAzHjD140662;     iuoZtAzHjD140662 = iuoZtAzHjD72281691;     iuoZtAzHjD72281691 = iuoZtAzHjD50659155;     iuoZtAzHjD50659155 = iuoZtAzHjD23182326;     iuoZtAzHjD23182326 = iuoZtAzHjD81602062;     iuoZtAzHjD81602062 = iuoZtAzHjD50662422;     iuoZtAzHjD50662422 = iuoZtAzHjD53213292;     iuoZtAzHjD53213292 = iuoZtAzHjD67801052;     iuoZtAzHjD67801052 = iuoZtAzHjD11108307;     iuoZtAzHjD11108307 = iuoZtAzHjD43554559;     iuoZtAzHjD43554559 = iuoZtAzHjD80317940;     iuoZtAzHjD80317940 = iuoZtAzHjD34184668;     iuoZtAzHjD34184668 = iuoZtAzHjD63593181;     iuoZtAzHjD63593181 = iuoZtAzHjD64835305;     iuoZtAzHjD64835305 = iuoZtAzHjD32923952;     iuoZtAzHjD32923952 = iuoZtAzHjD77880369;     iuoZtAzHjD77880369 = iuoZtAzHjD36854021;     iuoZtAzHjD36854021 = iuoZtAzHjD5616835;     iuoZtAzHjD5616835 = iuoZtAzHjD75536335;     iuoZtAzHjD75536335 = iuoZtAzHjD88361581;     iuoZtAzHjD88361581 = iuoZtAzHjD79274379;     iuoZtAzHjD79274379 = iuoZtAzHjD53034752;     iuoZtAzHjD53034752 = iuoZtAzHjD53001992;     iuoZtAzHjD53001992 = iuoZtAzHjD23420318;     iuoZtAzHjD23420318 = iuoZtAzHjD9896808;     iuoZtAzHjD9896808 = iuoZtAzHjD15581326;     iuoZtAzHjD15581326 = iuoZtAzHjD46023241;     iuoZtAzHjD46023241 = iuoZtAzHjD54583865;     iuoZtAzHjD54583865 = iuoZtAzHjD45472864;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eYpfWsjkhN11784331() {     int MviyuWJbtG58259031 = -443676824;    int MviyuWJbtG90450046 = -505402232;    int MviyuWJbtG59091309 = -811492549;    int MviyuWJbtG54664473 = -257830697;    int MviyuWJbtG74581602 = -364681236;    int MviyuWJbtG20490760 = -860220212;    int MviyuWJbtG7478232 = -79384692;    int MviyuWJbtG44711830 = -126634809;    int MviyuWJbtG56958920 = -297235055;    int MviyuWJbtG42109344 = -117258015;    int MviyuWJbtG78252267 = -698887174;    int MviyuWJbtG29592404 = -201465028;    int MviyuWJbtG18747327 = -303442206;    int MviyuWJbtG32059341 = -897045187;    int MviyuWJbtG28339782 = -111939714;    int MviyuWJbtG83289614 = -688552119;    int MviyuWJbtG19052196 = -509476925;    int MviyuWJbtG20119124 = -682334282;    int MviyuWJbtG48393352 = -191231473;    int MviyuWJbtG71078958 = -798658479;    int MviyuWJbtG35881133 = -470211518;    int MviyuWJbtG55118726 = -830939324;    int MviyuWJbtG73061778 = 51016701;    int MviyuWJbtG78055942 = -790549026;    int MviyuWJbtG45237762 = -280030211;    int MviyuWJbtG79003916 = 83437354;    int MviyuWJbtG66647187 = -562620058;    int MviyuWJbtG59278409 = -655633102;    int MviyuWJbtG11981828 = 97859454;    int MviyuWJbtG85417705 = -330258947;    int MviyuWJbtG42710815 = -30276056;    int MviyuWJbtG28332053 = -252491571;    int MviyuWJbtG34996658 = -824823790;    int MviyuWJbtG99551776 = -919830517;    int MviyuWJbtG8079325 = -455584170;    int MviyuWJbtG40138125 = -231303516;    int MviyuWJbtG32915338 = -907676034;    int MviyuWJbtG44358547 = -602494328;    int MviyuWJbtG42195364 = -492954460;    int MviyuWJbtG1215760 = -61594956;    int MviyuWJbtG79196615 = 71426986;    int MviyuWJbtG40227207 = -754031318;    int MviyuWJbtG79352314 = -831272423;    int MviyuWJbtG28267599 = -522607627;    int MviyuWJbtG60344011 = -146783618;    int MviyuWJbtG45170580 = -448286546;    int MviyuWJbtG64158984 = -25878113;    int MviyuWJbtG41117058 = -718704617;    int MviyuWJbtG55671593 = -275265824;    int MviyuWJbtG8130528 = -790353821;    int MviyuWJbtG87400902 = -49558357;    int MviyuWJbtG61616061 = -135196471;    int MviyuWJbtG26574428 = -622846428;    int MviyuWJbtG55770065 = -883781754;    int MviyuWJbtG75903198 = -315126722;    int MviyuWJbtG3140306 = -612737500;    int MviyuWJbtG17388269 = -456418934;    int MviyuWJbtG81035367 = 79056477;    int MviyuWJbtG9426711 = -977800487;    int MviyuWJbtG95577685 = -348118591;    int MviyuWJbtG53843573 = -197600155;    int MviyuWJbtG48199823 = -423751591;    int MviyuWJbtG32730002 = -124494264;    int MviyuWJbtG71541214 = -966976108;    int MviyuWJbtG99398528 = 13018041;    int MviyuWJbtG49920214 = -346395603;    int MviyuWJbtG94595745 = -376641239;    int MviyuWJbtG19195551 = -383611690;    int MviyuWJbtG23980017 = -341461018;    int MviyuWJbtG88201656 = -880636198;    int MviyuWJbtG50374276 = -780876086;    int MviyuWJbtG74693649 = -906982598;    int MviyuWJbtG77923760 = -89379823;    int MviyuWJbtG47177592 = -29636517;    int MviyuWJbtG91882343 = -770085465;    int MviyuWJbtG95653926 = -716180200;    int MviyuWJbtG75766412 = -999666902;    int MviyuWJbtG44794180 = -426375672;    int MviyuWJbtG17711932 = -543765408;    int MviyuWJbtG67183 = -831743665;    int MviyuWJbtG14844932 = -890684533;    int MviyuWJbtG25530129 = -843915442;    int MviyuWJbtG3606816 = -280367278;    int MviyuWJbtG3851301 = -111786725;    int MviyuWJbtG98016802 = -180700591;    int MviyuWJbtG81094754 = -895079585;    int MviyuWJbtG1757625 = -629645143;    int MviyuWJbtG79226593 = -941042036;    int MviyuWJbtG23648578 = -504703795;    int MviyuWJbtG4939019 = -842846671;    int MviyuWJbtG22749857 = -774884582;    int MviyuWJbtG51879971 = -886732511;    int MviyuWJbtG34931836 = -624693842;    int MviyuWJbtG46617679 = -44835869;    int MviyuWJbtG47372187 = -863994802;    int MviyuWJbtG30996793 = -504821424;    int MviyuWJbtG7497205 = -529537055;    int MviyuWJbtG7811100 = -864296315;    int MviyuWJbtG28869070 = -435625669;    int MviyuWJbtG10423797 = -443676824;     MviyuWJbtG58259031 = MviyuWJbtG90450046;     MviyuWJbtG90450046 = MviyuWJbtG59091309;     MviyuWJbtG59091309 = MviyuWJbtG54664473;     MviyuWJbtG54664473 = MviyuWJbtG74581602;     MviyuWJbtG74581602 = MviyuWJbtG20490760;     MviyuWJbtG20490760 = MviyuWJbtG7478232;     MviyuWJbtG7478232 = MviyuWJbtG44711830;     MviyuWJbtG44711830 = MviyuWJbtG56958920;     MviyuWJbtG56958920 = MviyuWJbtG42109344;     MviyuWJbtG42109344 = MviyuWJbtG78252267;     MviyuWJbtG78252267 = MviyuWJbtG29592404;     MviyuWJbtG29592404 = MviyuWJbtG18747327;     MviyuWJbtG18747327 = MviyuWJbtG32059341;     MviyuWJbtG32059341 = MviyuWJbtG28339782;     MviyuWJbtG28339782 = MviyuWJbtG83289614;     MviyuWJbtG83289614 = MviyuWJbtG19052196;     MviyuWJbtG19052196 = MviyuWJbtG20119124;     MviyuWJbtG20119124 = MviyuWJbtG48393352;     MviyuWJbtG48393352 = MviyuWJbtG71078958;     MviyuWJbtG71078958 = MviyuWJbtG35881133;     MviyuWJbtG35881133 = MviyuWJbtG55118726;     MviyuWJbtG55118726 = MviyuWJbtG73061778;     MviyuWJbtG73061778 = MviyuWJbtG78055942;     MviyuWJbtG78055942 = MviyuWJbtG45237762;     MviyuWJbtG45237762 = MviyuWJbtG79003916;     MviyuWJbtG79003916 = MviyuWJbtG66647187;     MviyuWJbtG66647187 = MviyuWJbtG59278409;     MviyuWJbtG59278409 = MviyuWJbtG11981828;     MviyuWJbtG11981828 = MviyuWJbtG85417705;     MviyuWJbtG85417705 = MviyuWJbtG42710815;     MviyuWJbtG42710815 = MviyuWJbtG28332053;     MviyuWJbtG28332053 = MviyuWJbtG34996658;     MviyuWJbtG34996658 = MviyuWJbtG99551776;     MviyuWJbtG99551776 = MviyuWJbtG8079325;     MviyuWJbtG8079325 = MviyuWJbtG40138125;     MviyuWJbtG40138125 = MviyuWJbtG32915338;     MviyuWJbtG32915338 = MviyuWJbtG44358547;     MviyuWJbtG44358547 = MviyuWJbtG42195364;     MviyuWJbtG42195364 = MviyuWJbtG1215760;     MviyuWJbtG1215760 = MviyuWJbtG79196615;     MviyuWJbtG79196615 = MviyuWJbtG40227207;     MviyuWJbtG40227207 = MviyuWJbtG79352314;     MviyuWJbtG79352314 = MviyuWJbtG28267599;     MviyuWJbtG28267599 = MviyuWJbtG60344011;     MviyuWJbtG60344011 = MviyuWJbtG45170580;     MviyuWJbtG45170580 = MviyuWJbtG64158984;     MviyuWJbtG64158984 = MviyuWJbtG41117058;     MviyuWJbtG41117058 = MviyuWJbtG55671593;     MviyuWJbtG55671593 = MviyuWJbtG8130528;     MviyuWJbtG8130528 = MviyuWJbtG87400902;     MviyuWJbtG87400902 = MviyuWJbtG61616061;     MviyuWJbtG61616061 = MviyuWJbtG26574428;     MviyuWJbtG26574428 = MviyuWJbtG55770065;     MviyuWJbtG55770065 = MviyuWJbtG75903198;     MviyuWJbtG75903198 = MviyuWJbtG3140306;     MviyuWJbtG3140306 = MviyuWJbtG17388269;     MviyuWJbtG17388269 = MviyuWJbtG81035367;     MviyuWJbtG81035367 = MviyuWJbtG9426711;     MviyuWJbtG9426711 = MviyuWJbtG95577685;     MviyuWJbtG95577685 = MviyuWJbtG53843573;     MviyuWJbtG53843573 = MviyuWJbtG48199823;     MviyuWJbtG48199823 = MviyuWJbtG32730002;     MviyuWJbtG32730002 = MviyuWJbtG71541214;     MviyuWJbtG71541214 = MviyuWJbtG99398528;     MviyuWJbtG99398528 = MviyuWJbtG49920214;     MviyuWJbtG49920214 = MviyuWJbtG94595745;     MviyuWJbtG94595745 = MviyuWJbtG19195551;     MviyuWJbtG19195551 = MviyuWJbtG23980017;     MviyuWJbtG23980017 = MviyuWJbtG88201656;     MviyuWJbtG88201656 = MviyuWJbtG50374276;     MviyuWJbtG50374276 = MviyuWJbtG74693649;     MviyuWJbtG74693649 = MviyuWJbtG77923760;     MviyuWJbtG77923760 = MviyuWJbtG47177592;     MviyuWJbtG47177592 = MviyuWJbtG91882343;     MviyuWJbtG91882343 = MviyuWJbtG95653926;     MviyuWJbtG95653926 = MviyuWJbtG75766412;     MviyuWJbtG75766412 = MviyuWJbtG44794180;     MviyuWJbtG44794180 = MviyuWJbtG17711932;     MviyuWJbtG17711932 = MviyuWJbtG67183;     MviyuWJbtG67183 = MviyuWJbtG14844932;     MviyuWJbtG14844932 = MviyuWJbtG25530129;     MviyuWJbtG25530129 = MviyuWJbtG3606816;     MviyuWJbtG3606816 = MviyuWJbtG3851301;     MviyuWJbtG3851301 = MviyuWJbtG98016802;     MviyuWJbtG98016802 = MviyuWJbtG81094754;     MviyuWJbtG81094754 = MviyuWJbtG1757625;     MviyuWJbtG1757625 = MviyuWJbtG79226593;     MviyuWJbtG79226593 = MviyuWJbtG23648578;     MviyuWJbtG23648578 = MviyuWJbtG4939019;     MviyuWJbtG4939019 = MviyuWJbtG22749857;     MviyuWJbtG22749857 = MviyuWJbtG51879971;     MviyuWJbtG51879971 = MviyuWJbtG34931836;     MviyuWJbtG34931836 = MviyuWJbtG46617679;     MviyuWJbtG46617679 = MviyuWJbtG47372187;     MviyuWJbtG47372187 = MviyuWJbtG30996793;     MviyuWJbtG30996793 = MviyuWJbtG7497205;     MviyuWJbtG7497205 = MviyuWJbtG7811100;     MviyuWJbtG7811100 = MviyuWJbtG28869070;     MviyuWJbtG28869070 = MviyuWJbtG10423797;     MviyuWJbtG10423797 = MviyuWJbtG58259031;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AViAMEQaUK29997673() {     int cLmEdmmfFl41703337 = -224342504;    int cLmEdmmfFl7322136 = -504784420;    int cLmEdmmfFl55499816 = -605017776;    int cLmEdmmfFl17456725 = -451630533;    int cLmEdmmfFl80702926 = -171548727;    int cLmEdmmfFl18995161 = -639291866;    int cLmEdmmfFl78517549 = -919296005;    int cLmEdmmfFl59059273 = -113792838;    int cLmEdmmfFl15488621 = -638694406;    int cLmEdmmfFl74654996 = -585634127;    int cLmEdmmfFl27668205 = -889003046;    int cLmEdmmfFl92797802 = -189083274;    int cLmEdmmfFl36865103 = -95103447;    int cLmEdmmfFl11686233 = -685335544;    int cLmEdmmfFl89194244 = -205962696;    int cLmEdmmfFl69678776 = -476631445;    int cLmEdmmfFl12804081 = -262821952;    int cLmEdmmfFl970571 = -929639158;    int cLmEdmmfFl10833495 = -189906343;    int cLmEdmmfFl60446401 = -228244471;    int cLmEdmmfFl91521520 = -59740549;    int cLmEdmmfFl67266044 = -822091095;    int cLmEdmmfFl76887486 = 11127504;    int cLmEdmmfFl12762501 = -862374422;    int cLmEdmmfFl86813720 = -839682970;    int cLmEdmmfFl74346065 = -897072361;    int cLmEdmmfFl16101927 = -190704400;    int cLmEdmmfFl99010034 = -725587986;    int cLmEdmmfFl48062922 = -864645688;    int cLmEdmmfFl1251047 = -843108799;    int cLmEdmmfFl60806129 = -768033257;    int cLmEdmmfFl91438479 = -167686651;    int cLmEdmmfFl78535545 = -445695967;    int cLmEdmmfFl41710287 = -511432687;    int cLmEdmmfFl86806054 = -653248905;    int cLmEdmmfFl52527309 = -19327422;    int cLmEdmmfFl1529517 = -786525214;    int cLmEdmmfFl84937379 = -310964848;    int cLmEdmmfFl91190442 = -677852257;    int cLmEdmmfFl50710895 = -914188479;    int cLmEdmmfFl12845540 = -600121988;    int cLmEdmmfFl90714992 = -227939640;    int cLmEdmmfFl2919385 = -282329666;    int cLmEdmmfFl20172539 = -278790398;    int cLmEdmmfFl52288507 = -974940572;    int cLmEdmmfFl84776575 = -467897469;    int cLmEdmmfFl40174415 = -158182343;    int cLmEdmmfFl60787653 = -717426335;    int cLmEdmmfFl86518705 = -860867907;    int cLmEdmmfFl43463011 = -997374277;    int cLmEdmmfFl60658350 = -270459779;    int cLmEdmmfFl17116856 = -193455783;    int cLmEdmmfFl40686887 = -834593745;    int cLmEdmmfFl80446824 = -602195142;    int cLmEdmmfFl38476153 = -663612845;    int cLmEdmmfFl74437292 = -402251410;    int cLmEdmmfFl30434650 = -415911924;    int cLmEdmmfFl42737316 = -742643355;    int cLmEdmmfFl30643004 = -611947564;    int cLmEdmmfFl6356861 = -274476366;    int cLmEdmmfFl2893234 = -348587466;    int cLmEdmmfFl79507515 = -93708020;    int cLmEdmmfFl10996351 = -249147151;    int cLmEdmmfFl14237574 = -795585608;    int cLmEdmmfFl13848867 = -817600870;    int cLmEdmmfFl36229725 = -621316396;    int cLmEdmmfFl14262258 = -743387308;    int cLmEdmmfFl95154816 = -583670761;    int cLmEdmmfFl24880179 = 67913361;    int cLmEdmmfFl36666935 = -86635275;    int cLmEdmmfFl68149259 = -690106232;    int cLmEdmmfFl27866702 = -951857105;    int cLmEdmmfFl9780129 = -151786902;    int cLmEdmmfFl60122599 = -275717865;    int cLmEdmmfFl47600861 = -628122484;    int cLmEdmmfFl806528 = -831800909;    int cLmEdmmfFl64346660 = -439761429;    int cLmEdmmfFl56714947 = -710082099;    int cLmEdmmfFl60473993 = -887433851;    int cLmEdmmfFl2037145 = -271785501;    int cLmEdmmfFl34171651 = -638890019;    int cLmEdmmfFl55314274 = -473278066;    int cLmEdmmfFl12491329 = -864720080;    int cLmEdmmfFl4599911 = -867271412;    int cLmEdmmfFl40592696 = -472649020;    int cLmEdmmfFl43689274 = -474577475;    int cLmEdmmfFl50751593 = -333092906;    int cLmEdmmfFl98088720 = -843500826;    int cLmEdmmfFl3234135 = -847819842;    int cLmEdmmfFl12368762 = -150997496;    int cLmEdmmfFl22092660 = -603415498;    int cLmEdmmfFl58792201 = 56118141;    int cLmEdmmfFl54294375 = -699017285;    int cLmEdmmfFl84833581 = -303375891;    int cLmEdmmfFl47817662 = -465601014;    int cLmEdmmfFl33338025 = -406413968;    int cLmEdmmfFl79718641 = -978792490;    int cLmEdmmfFl88681810 = -486744059;    int cLmEdmmfFl6323672 = -461189528;    int cLmEdmmfFl16058783 = -224342504;     cLmEdmmfFl41703337 = cLmEdmmfFl7322136;     cLmEdmmfFl7322136 = cLmEdmmfFl55499816;     cLmEdmmfFl55499816 = cLmEdmmfFl17456725;     cLmEdmmfFl17456725 = cLmEdmmfFl80702926;     cLmEdmmfFl80702926 = cLmEdmmfFl18995161;     cLmEdmmfFl18995161 = cLmEdmmfFl78517549;     cLmEdmmfFl78517549 = cLmEdmmfFl59059273;     cLmEdmmfFl59059273 = cLmEdmmfFl15488621;     cLmEdmmfFl15488621 = cLmEdmmfFl74654996;     cLmEdmmfFl74654996 = cLmEdmmfFl27668205;     cLmEdmmfFl27668205 = cLmEdmmfFl92797802;     cLmEdmmfFl92797802 = cLmEdmmfFl36865103;     cLmEdmmfFl36865103 = cLmEdmmfFl11686233;     cLmEdmmfFl11686233 = cLmEdmmfFl89194244;     cLmEdmmfFl89194244 = cLmEdmmfFl69678776;     cLmEdmmfFl69678776 = cLmEdmmfFl12804081;     cLmEdmmfFl12804081 = cLmEdmmfFl970571;     cLmEdmmfFl970571 = cLmEdmmfFl10833495;     cLmEdmmfFl10833495 = cLmEdmmfFl60446401;     cLmEdmmfFl60446401 = cLmEdmmfFl91521520;     cLmEdmmfFl91521520 = cLmEdmmfFl67266044;     cLmEdmmfFl67266044 = cLmEdmmfFl76887486;     cLmEdmmfFl76887486 = cLmEdmmfFl12762501;     cLmEdmmfFl12762501 = cLmEdmmfFl86813720;     cLmEdmmfFl86813720 = cLmEdmmfFl74346065;     cLmEdmmfFl74346065 = cLmEdmmfFl16101927;     cLmEdmmfFl16101927 = cLmEdmmfFl99010034;     cLmEdmmfFl99010034 = cLmEdmmfFl48062922;     cLmEdmmfFl48062922 = cLmEdmmfFl1251047;     cLmEdmmfFl1251047 = cLmEdmmfFl60806129;     cLmEdmmfFl60806129 = cLmEdmmfFl91438479;     cLmEdmmfFl91438479 = cLmEdmmfFl78535545;     cLmEdmmfFl78535545 = cLmEdmmfFl41710287;     cLmEdmmfFl41710287 = cLmEdmmfFl86806054;     cLmEdmmfFl86806054 = cLmEdmmfFl52527309;     cLmEdmmfFl52527309 = cLmEdmmfFl1529517;     cLmEdmmfFl1529517 = cLmEdmmfFl84937379;     cLmEdmmfFl84937379 = cLmEdmmfFl91190442;     cLmEdmmfFl91190442 = cLmEdmmfFl50710895;     cLmEdmmfFl50710895 = cLmEdmmfFl12845540;     cLmEdmmfFl12845540 = cLmEdmmfFl90714992;     cLmEdmmfFl90714992 = cLmEdmmfFl2919385;     cLmEdmmfFl2919385 = cLmEdmmfFl20172539;     cLmEdmmfFl20172539 = cLmEdmmfFl52288507;     cLmEdmmfFl52288507 = cLmEdmmfFl84776575;     cLmEdmmfFl84776575 = cLmEdmmfFl40174415;     cLmEdmmfFl40174415 = cLmEdmmfFl60787653;     cLmEdmmfFl60787653 = cLmEdmmfFl86518705;     cLmEdmmfFl86518705 = cLmEdmmfFl43463011;     cLmEdmmfFl43463011 = cLmEdmmfFl60658350;     cLmEdmmfFl60658350 = cLmEdmmfFl17116856;     cLmEdmmfFl17116856 = cLmEdmmfFl40686887;     cLmEdmmfFl40686887 = cLmEdmmfFl80446824;     cLmEdmmfFl80446824 = cLmEdmmfFl38476153;     cLmEdmmfFl38476153 = cLmEdmmfFl74437292;     cLmEdmmfFl74437292 = cLmEdmmfFl30434650;     cLmEdmmfFl30434650 = cLmEdmmfFl42737316;     cLmEdmmfFl42737316 = cLmEdmmfFl30643004;     cLmEdmmfFl30643004 = cLmEdmmfFl6356861;     cLmEdmmfFl6356861 = cLmEdmmfFl2893234;     cLmEdmmfFl2893234 = cLmEdmmfFl79507515;     cLmEdmmfFl79507515 = cLmEdmmfFl10996351;     cLmEdmmfFl10996351 = cLmEdmmfFl14237574;     cLmEdmmfFl14237574 = cLmEdmmfFl13848867;     cLmEdmmfFl13848867 = cLmEdmmfFl36229725;     cLmEdmmfFl36229725 = cLmEdmmfFl14262258;     cLmEdmmfFl14262258 = cLmEdmmfFl95154816;     cLmEdmmfFl95154816 = cLmEdmmfFl24880179;     cLmEdmmfFl24880179 = cLmEdmmfFl36666935;     cLmEdmmfFl36666935 = cLmEdmmfFl68149259;     cLmEdmmfFl68149259 = cLmEdmmfFl27866702;     cLmEdmmfFl27866702 = cLmEdmmfFl9780129;     cLmEdmmfFl9780129 = cLmEdmmfFl60122599;     cLmEdmmfFl60122599 = cLmEdmmfFl47600861;     cLmEdmmfFl47600861 = cLmEdmmfFl806528;     cLmEdmmfFl806528 = cLmEdmmfFl64346660;     cLmEdmmfFl64346660 = cLmEdmmfFl56714947;     cLmEdmmfFl56714947 = cLmEdmmfFl60473993;     cLmEdmmfFl60473993 = cLmEdmmfFl2037145;     cLmEdmmfFl2037145 = cLmEdmmfFl34171651;     cLmEdmmfFl34171651 = cLmEdmmfFl55314274;     cLmEdmmfFl55314274 = cLmEdmmfFl12491329;     cLmEdmmfFl12491329 = cLmEdmmfFl4599911;     cLmEdmmfFl4599911 = cLmEdmmfFl40592696;     cLmEdmmfFl40592696 = cLmEdmmfFl43689274;     cLmEdmmfFl43689274 = cLmEdmmfFl50751593;     cLmEdmmfFl50751593 = cLmEdmmfFl98088720;     cLmEdmmfFl98088720 = cLmEdmmfFl3234135;     cLmEdmmfFl3234135 = cLmEdmmfFl12368762;     cLmEdmmfFl12368762 = cLmEdmmfFl22092660;     cLmEdmmfFl22092660 = cLmEdmmfFl58792201;     cLmEdmmfFl58792201 = cLmEdmmfFl54294375;     cLmEdmmfFl54294375 = cLmEdmmfFl84833581;     cLmEdmmfFl84833581 = cLmEdmmfFl47817662;     cLmEdmmfFl47817662 = cLmEdmmfFl33338025;     cLmEdmmfFl33338025 = cLmEdmmfFl79718641;     cLmEdmmfFl79718641 = cLmEdmmfFl88681810;     cLmEdmmfFl88681810 = cLmEdmmfFl6323672;     cLmEdmmfFl6323672 = cLmEdmmfFl16058783;     cLmEdmmfFl16058783 = cLmEdmmfFl41703337;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LPLCwlvmrl453546() {     int SYOtiwdwxr54489504 = -359383196;    int SYOtiwdwxr94093024 = -675118434;    int SYOtiwdwxr73252616 = 86622842;    int SYOtiwdwxr25397550 = -275832810;    int SYOtiwdwxr74214030 = -663365192;    int SYOtiwdwxr62305231 = -416215179;    int SYOtiwdwxr99725307 = -364968076;    int SYOtiwdwxr85778385 = -558311095;    int SYOtiwdwxr10893431 = -600452498;    int SYOtiwdwxr80873700 = -389721760;    int SYOtiwdwxr55155587 = 96310938;    int SYOtiwdwxr24748308 = -696416090;    int SYOtiwdwxr7008406 = 1113895;    int SYOtiwdwxr61310311 = 82249257;    int SYOtiwdwxr12500798 = -112948527;    int SYOtiwdwxr98103844 = 72477166;    int SYOtiwdwxr68555898 = 73403581;    int SYOtiwdwxr84529055 = -484849461;    int SYOtiwdwxr11994125 = -99375539;    int SYOtiwdwxr63100897 = -827488610;    int SYOtiwdwxr71695577 = -551056813;    int SYOtiwdwxr96313863 = -991949443;    int SYOtiwdwxr15647782 = 80123609;    int SYOtiwdwxr57269630 = -122719435;    int SYOtiwdwxr92707979 = 49875106;    int SYOtiwdwxr55664711 = -587503560;    int SYOtiwdwxr27517463 = -824410681;    int SYOtiwdwxr66362959 = -899386090;    int SYOtiwdwxr64679975 = 21137072;    int SYOtiwdwxr54002162 = -472994953;    int SYOtiwdwxr5242125 = 55549993;    int SYOtiwdwxr94757303 = -146406329;    int SYOtiwdwxr24581494 = -321550752;    int SYOtiwdwxr18786660 = -416186906;    int SYOtiwdwxr69866586 = -163280786;    int SYOtiwdwxr47751424 = -907890526;    int SYOtiwdwxr38293577 = -576434564;    int SYOtiwdwxr42641402 = -230584940;    int SYOtiwdwxr96965828 = -519702834;    int SYOtiwdwxr76975625 = -758605324;    int SYOtiwdwxr74276848 = -975595981;    int SYOtiwdwxr98417448 = -126250974;    int SYOtiwdwxr37802854 = -305763111;    int SYOtiwdwxr64801076 = -619962641;    int SYOtiwdwxr32296998 = -867710532;    int SYOtiwdwxr58404705 = -541215909;    int SYOtiwdwxr17756435 = -655697395;    int SYOtiwdwxr90227619 = -872779115;    int SYOtiwdwxr30582755 = 75530119;    int SYOtiwdwxr90413431 = -508410740;    int SYOtiwdwxr78985844 = -523344088;    int SYOtiwdwxr45293403 = -664641121;    int SYOtiwdwxr75172037 = 89402973;    int SYOtiwdwxr25146550 = -191646884;    int SYOtiwdwxr28757969 = -658057332;    int SYOtiwdwxr58175640 = -367433754;    int SYOtiwdwxr78445242 = -655242043;    int SYOtiwdwxr15982986 = -790657724;    int SYOtiwdwxr32689571 = -225707916;    int SYOtiwdwxr18549320 = 24138368;    int SYOtiwdwxr34787768 = -591804499;    int SYOtiwdwxr33362349 = -465581986;    int SYOtiwdwxr21098411 = -479448167;    int SYOtiwdwxr56891269 = -27457545;    int SYOtiwdwxr75631576 = -345271753;    int SYOtiwdwxr60398283 = -757282733;    int SYOtiwdwxr166814 = -274865339;    int SYOtiwdwxr88221746 = -582699200;    int SYOtiwdwxr91443724 = -754469957;    int SYOtiwdwxr64749373 = -205058001;    int SYOtiwdwxr59810267 = -351088270;    int SYOtiwdwxr25914497 = -696011480;    int SYOtiwdwxr87563227 = -965146627;    int SYOtiwdwxr35018500 = -340770215;    int SYOtiwdwxr88824049 = -851892630;    int SYOtiwdwxr73278128 = -324805839;    int SYOtiwdwxr58511010 = -586186332;    int SYOtiwdwxr50846705 = -299913750;    int SYOtiwdwxr24972632 = -255008903;    int SYOtiwdwxr34303275 = -408908985;    int SYOtiwdwxr37908277 = -931806166;    int SYOtiwdwxr37289844 = -951631566;    int SYOtiwdwxr35780205 = -874916210;    int SYOtiwdwxr74266543 = -470452189;    int SYOtiwdwxr75016318 = -949650866;    int SYOtiwdwxr59948722 = -279808887;    int SYOtiwdwxr19585267 = -135809302;    int SYOtiwdwxr99434944 = -29903869;    int SYOtiwdwxr90028691 = -758129574;    int SYOtiwdwxr11690946 = -795847032;    int SYOtiwdwxr69306182 = -152648483;    int SYOtiwdwxr22310591 = -785776841;    int SYOtiwdwxr9951832 = 95122976;    int SYOtiwdwxr78416508 = -443841203;    int SYOtiwdwxr42187857 = -66800826;    int SYOtiwdwxr40914500 = -410013996;    int SYOtiwdwxr77319038 = -646802807;    int SYOtiwdwxr80911585 = -178305566;    int SYOtiwdwxr89169500 = -174690888;    int SYOtiwdwxr71898714 = -359383196;     SYOtiwdwxr54489504 = SYOtiwdwxr94093024;     SYOtiwdwxr94093024 = SYOtiwdwxr73252616;     SYOtiwdwxr73252616 = SYOtiwdwxr25397550;     SYOtiwdwxr25397550 = SYOtiwdwxr74214030;     SYOtiwdwxr74214030 = SYOtiwdwxr62305231;     SYOtiwdwxr62305231 = SYOtiwdwxr99725307;     SYOtiwdwxr99725307 = SYOtiwdwxr85778385;     SYOtiwdwxr85778385 = SYOtiwdwxr10893431;     SYOtiwdwxr10893431 = SYOtiwdwxr80873700;     SYOtiwdwxr80873700 = SYOtiwdwxr55155587;     SYOtiwdwxr55155587 = SYOtiwdwxr24748308;     SYOtiwdwxr24748308 = SYOtiwdwxr7008406;     SYOtiwdwxr7008406 = SYOtiwdwxr61310311;     SYOtiwdwxr61310311 = SYOtiwdwxr12500798;     SYOtiwdwxr12500798 = SYOtiwdwxr98103844;     SYOtiwdwxr98103844 = SYOtiwdwxr68555898;     SYOtiwdwxr68555898 = SYOtiwdwxr84529055;     SYOtiwdwxr84529055 = SYOtiwdwxr11994125;     SYOtiwdwxr11994125 = SYOtiwdwxr63100897;     SYOtiwdwxr63100897 = SYOtiwdwxr71695577;     SYOtiwdwxr71695577 = SYOtiwdwxr96313863;     SYOtiwdwxr96313863 = SYOtiwdwxr15647782;     SYOtiwdwxr15647782 = SYOtiwdwxr57269630;     SYOtiwdwxr57269630 = SYOtiwdwxr92707979;     SYOtiwdwxr92707979 = SYOtiwdwxr55664711;     SYOtiwdwxr55664711 = SYOtiwdwxr27517463;     SYOtiwdwxr27517463 = SYOtiwdwxr66362959;     SYOtiwdwxr66362959 = SYOtiwdwxr64679975;     SYOtiwdwxr64679975 = SYOtiwdwxr54002162;     SYOtiwdwxr54002162 = SYOtiwdwxr5242125;     SYOtiwdwxr5242125 = SYOtiwdwxr94757303;     SYOtiwdwxr94757303 = SYOtiwdwxr24581494;     SYOtiwdwxr24581494 = SYOtiwdwxr18786660;     SYOtiwdwxr18786660 = SYOtiwdwxr69866586;     SYOtiwdwxr69866586 = SYOtiwdwxr47751424;     SYOtiwdwxr47751424 = SYOtiwdwxr38293577;     SYOtiwdwxr38293577 = SYOtiwdwxr42641402;     SYOtiwdwxr42641402 = SYOtiwdwxr96965828;     SYOtiwdwxr96965828 = SYOtiwdwxr76975625;     SYOtiwdwxr76975625 = SYOtiwdwxr74276848;     SYOtiwdwxr74276848 = SYOtiwdwxr98417448;     SYOtiwdwxr98417448 = SYOtiwdwxr37802854;     SYOtiwdwxr37802854 = SYOtiwdwxr64801076;     SYOtiwdwxr64801076 = SYOtiwdwxr32296998;     SYOtiwdwxr32296998 = SYOtiwdwxr58404705;     SYOtiwdwxr58404705 = SYOtiwdwxr17756435;     SYOtiwdwxr17756435 = SYOtiwdwxr90227619;     SYOtiwdwxr90227619 = SYOtiwdwxr30582755;     SYOtiwdwxr30582755 = SYOtiwdwxr90413431;     SYOtiwdwxr90413431 = SYOtiwdwxr78985844;     SYOtiwdwxr78985844 = SYOtiwdwxr45293403;     SYOtiwdwxr45293403 = SYOtiwdwxr75172037;     SYOtiwdwxr75172037 = SYOtiwdwxr25146550;     SYOtiwdwxr25146550 = SYOtiwdwxr28757969;     SYOtiwdwxr28757969 = SYOtiwdwxr58175640;     SYOtiwdwxr58175640 = SYOtiwdwxr78445242;     SYOtiwdwxr78445242 = SYOtiwdwxr15982986;     SYOtiwdwxr15982986 = SYOtiwdwxr32689571;     SYOtiwdwxr32689571 = SYOtiwdwxr18549320;     SYOtiwdwxr18549320 = SYOtiwdwxr34787768;     SYOtiwdwxr34787768 = SYOtiwdwxr33362349;     SYOtiwdwxr33362349 = SYOtiwdwxr21098411;     SYOtiwdwxr21098411 = SYOtiwdwxr56891269;     SYOtiwdwxr56891269 = SYOtiwdwxr75631576;     SYOtiwdwxr75631576 = SYOtiwdwxr60398283;     SYOtiwdwxr60398283 = SYOtiwdwxr166814;     SYOtiwdwxr166814 = SYOtiwdwxr88221746;     SYOtiwdwxr88221746 = SYOtiwdwxr91443724;     SYOtiwdwxr91443724 = SYOtiwdwxr64749373;     SYOtiwdwxr64749373 = SYOtiwdwxr59810267;     SYOtiwdwxr59810267 = SYOtiwdwxr25914497;     SYOtiwdwxr25914497 = SYOtiwdwxr87563227;     SYOtiwdwxr87563227 = SYOtiwdwxr35018500;     SYOtiwdwxr35018500 = SYOtiwdwxr88824049;     SYOtiwdwxr88824049 = SYOtiwdwxr73278128;     SYOtiwdwxr73278128 = SYOtiwdwxr58511010;     SYOtiwdwxr58511010 = SYOtiwdwxr50846705;     SYOtiwdwxr50846705 = SYOtiwdwxr24972632;     SYOtiwdwxr24972632 = SYOtiwdwxr34303275;     SYOtiwdwxr34303275 = SYOtiwdwxr37908277;     SYOtiwdwxr37908277 = SYOtiwdwxr37289844;     SYOtiwdwxr37289844 = SYOtiwdwxr35780205;     SYOtiwdwxr35780205 = SYOtiwdwxr74266543;     SYOtiwdwxr74266543 = SYOtiwdwxr75016318;     SYOtiwdwxr75016318 = SYOtiwdwxr59948722;     SYOtiwdwxr59948722 = SYOtiwdwxr19585267;     SYOtiwdwxr19585267 = SYOtiwdwxr99434944;     SYOtiwdwxr99434944 = SYOtiwdwxr90028691;     SYOtiwdwxr90028691 = SYOtiwdwxr11690946;     SYOtiwdwxr11690946 = SYOtiwdwxr69306182;     SYOtiwdwxr69306182 = SYOtiwdwxr22310591;     SYOtiwdwxr22310591 = SYOtiwdwxr9951832;     SYOtiwdwxr9951832 = SYOtiwdwxr78416508;     SYOtiwdwxr78416508 = SYOtiwdwxr42187857;     SYOtiwdwxr42187857 = SYOtiwdwxr40914500;     SYOtiwdwxr40914500 = SYOtiwdwxr77319038;     SYOtiwdwxr77319038 = SYOtiwdwxr80911585;     SYOtiwdwxr80911585 = SYOtiwdwxr89169500;     SYOtiwdwxr89169500 = SYOtiwdwxr71898714;     SYOtiwdwxr71898714 = SYOtiwdwxr54489504;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FjTyDIPpoS18666887() {     int VhOTrvGZVd37933810 = -140048877;    int VhOTrvGZVd10965114 = -674500621;    int VhOTrvGZVd69661122 = -806902386;    int VhOTrvGZVd88189801 = -469632645;    int VhOTrvGZVd80335354 = -470232683;    int VhOTrvGZVd60809632 = -195286832;    int VhOTrvGZVd70764625 = -104879389;    int VhOTrvGZVd125829 = -545469124;    int VhOTrvGZVd69423132 = -941911849;    int VhOTrvGZVd13419353 = -858097872;    int VhOTrvGZVd4571525 = -93804935;    int VhOTrvGZVd87953707 = -684034337;    int VhOTrvGZVd25126182 = -890547346;    int VhOTrvGZVd40937204 = -806041100;    int VhOTrvGZVd73355261 = -206971509;    int VhOTrvGZVd84493006 = -815602159;    int VhOTrvGZVd62307783 = -779941446;    int VhOTrvGZVd65380502 = -732154337;    int VhOTrvGZVd74434267 = -98050410;    int VhOTrvGZVd52468340 = -257074603;    int VhOTrvGZVd27335964 = -140585844;    int VhOTrvGZVd8461182 = -983101213;    int VhOTrvGZVd19473490 = 40234412;    int VhOTrvGZVd91976188 = -194544831;    int VhOTrvGZVd34283938 = -509777653;    int VhOTrvGZVd51006860 = -468013276;    int VhOTrvGZVd76972203 = -452495023;    int VhOTrvGZVd6094585 = -969340974;    int VhOTrvGZVd761069 = -941368070;    int VhOTrvGZVd69835503 = -985844805;    int VhOTrvGZVd23337439 = -682207208;    int VhOTrvGZVd57863730 = -61601409;    int VhOTrvGZVd68120381 = 57577071;    int VhOTrvGZVd60945170 = -7789076;    int VhOTrvGZVd48593317 = -360945521;    int VhOTrvGZVd60140608 = -695914432;    int VhOTrvGZVd6907756 = -455283744;    int VhOTrvGZVd83220233 = 60944540;    int VhOTrvGZVd45960906 = -704600632;    int VhOTrvGZVd26470760 = -511198847;    int VhOTrvGZVd7925773 = -547144954;    int VhOTrvGZVd48905235 = -700159296;    int VhOTrvGZVd61369924 = -856820354;    int VhOTrvGZVd56706017 = -376145412;    int VhOTrvGZVd24241495 = -595867485;    int VhOTrvGZVd98010700 = -560826833;    int VhOTrvGZVd93771864 = -788001625;    int VhOTrvGZVd9898215 = -871500833;    int VhOTrvGZVd61429867 = -510071964;    int VhOTrvGZVd25745916 = -715431195;    int VhOTrvGZVd52243292 = -744245510;    int VhOTrvGZVd794197 = -722900432;    int VhOTrvGZVd89284495 = -122344344;    int VhOTrvGZVd49823308 = 89939729;    int VhOTrvGZVd91330923 = 93456545;    int VhOTrvGZVd29472628 = -156947665;    int VhOTrvGZVd91491623 = -614735034;    int VhOTrvGZVd77684934 = -512357556;    int VhOTrvGZVd53905864 = -959854993;    int VhOTrvGZVd29328495 = 97780592;    int VhOTrvGZVd83837428 = -742791809;    int VhOTrvGZVd64670041 = -135538415;    int VhOTrvGZVd99364759 = -604101055;    int VhOTrvGZVd99587628 = -956067045;    int VhOTrvGZVd90081914 = -75890664;    int VhOTrvGZVd46707794 = 67796474;    int VhOTrvGZVd19833326 = -641611408;    int VhOTrvGZVd64181012 = -782758271;    int VhOTrvGZVd92343886 = -345095579;    int VhOTrvGZVd13214653 = -511057077;    int VhOTrvGZVd77585250 = -260318416;    int VhOTrvGZVd79087549 = -740885987;    int VhOTrvGZVd19419596 = 72446295;    int VhOTrvGZVd47963507 = -586851563;    int VhOTrvGZVd44542567 = -709929649;    int VhOTrvGZVd78430729 = -440426549;    int VhOTrvGZVd47091258 = -26280860;    int VhOTrvGZVd62767472 = -583620177;    int VhOTrvGZVd67734693 = -598677346;    int VhOTrvGZVd36273237 = -948950820;    int VhOTrvGZVd57234995 = -680011652;    int VhOTrvGZVd67073988 = -580994190;    int VhOTrvGZVd44664718 = -359269011;    int VhOTrvGZVd75015153 = -125936876;    int VhOTrvGZVd17592211 = -141599296;    int VhOTrvGZVd22543242 = -959306776;    int VhOTrvGZVd68579234 = -939257066;    int VhOTrvGZVd18297073 = 67637341;    int VhOTrvGZVd69614247 = -1245621;    int VhOTrvGZVd19120690 = -103997857;    int VhOTrvGZVd68648985 = 18820601;    int VhOTrvGZVd29222821 = -942926189;    int VhOTrvGZVd29314370 = 20799533;    int VhOTrvGZVd16632412 = -702381225;    int VhOTrvGZVd42633332 = -768407038;    int VhOTrvGZVd43255731 = -311606539;    int VhOTrvGZVd49540476 = 3941759;    int VhOTrvGZVd61782295 = -900753309;    int VhOTrvGZVd66624102 = -200254748;    int VhOTrvGZVd77533700 = -140048877;     VhOTrvGZVd37933810 = VhOTrvGZVd10965114;     VhOTrvGZVd10965114 = VhOTrvGZVd69661122;     VhOTrvGZVd69661122 = VhOTrvGZVd88189801;     VhOTrvGZVd88189801 = VhOTrvGZVd80335354;     VhOTrvGZVd80335354 = VhOTrvGZVd60809632;     VhOTrvGZVd60809632 = VhOTrvGZVd70764625;     VhOTrvGZVd70764625 = VhOTrvGZVd125829;     VhOTrvGZVd125829 = VhOTrvGZVd69423132;     VhOTrvGZVd69423132 = VhOTrvGZVd13419353;     VhOTrvGZVd13419353 = VhOTrvGZVd4571525;     VhOTrvGZVd4571525 = VhOTrvGZVd87953707;     VhOTrvGZVd87953707 = VhOTrvGZVd25126182;     VhOTrvGZVd25126182 = VhOTrvGZVd40937204;     VhOTrvGZVd40937204 = VhOTrvGZVd73355261;     VhOTrvGZVd73355261 = VhOTrvGZVd84493006;     VhOTrvGZVd84493006 = VhOTrvGZVd62307783;     VhOTrvGZVd62307783 = VhOTrvGZVd65380502;     VhOTrvGZVd65380502 = VhOTrvGZVd74434267;     VhOTrvGZVd74434267 = VhOTrvGZVd52468340;     VhOTrvGZVd52468340 = VhOTrvGZVd27335964;     VhOTrvGZVd27335964 = VhOTrvGZVd8461182;     VhOTrvGZVd8461182 = VhOTrvGZVd19473490;     VhOTrvGZVd19473490 = VhOTrvGZVd91976188;     VhOTrvGZVd91976188 = VhOTrvGZVd34283938;     VhOTrvGZVd34283938 = VhOTrvGZVd51006860;     VhOTrvGZVd51006860 = VhOTrvGZVd76972203;     VhOTrvGZVd76972203 = VhOTrvGZVd6094585;     VhOTrvGZVd6094585 = VhOTrvGZVd761069;     VhOTrvGZVd761069 = VhOTrvGZVd69835503;     VhOTrvGZVd69835503 = VhOTrvGZVd23337439;     VhOTrvGZVd23337439 = VhOTrvGZVd57863730;     VhOTrvGZVd57863730 = VhOTrvGZVd68120381;     VhOTrvGZVd68120381 = VhOTrvGZVd60945170;     VhOTrvGZVd60945170 = VhOTrvGZVd48593317;     VhOTrvGZVd48593317 = VhOTrvGZVd60140608;     VhOTrvGZVd60140608 = VhOTrvGZVd6907756;     VhOTrvGZVd6907756 = VhOTrvGZVd83220233;     VhOTrvGZVd83220233 = VhOTrvGZVd45960906;     VhOTrvGZVd45960906 = VhOTrvGZVd26470760;     VhOTrvGZVd26470760 = VhOTrvGZVd7925773;     VhOTrvGZVd7925773 = VhOTrvGZVd48905235;     VhOTrvGZVd48905235 = VhOTrvGZVd61369924;     VhOTrvGZVd61369924 = VhOTrvGZVd56706017;     VhOTrvGZVd56706017 = VhOTrvGZVd24241495;     VhOTrvGZVd24241495 = VhOTrvGZVd98010700;     VhOTrvGZVd98010700 = VhOTrvGZVd93771864;     VhOTrvGZVd93771864 = VhOTrvGZVd9898215;     VhOTrvGZVd9898215 = VhOTrvGZVd61429867;     VhOTrvGZVd61429867 = VhOTrvGZVd25745916;     VhOTrvGZVd25745916 = VhOTrvGZVd52243292;     VhOTrvGZVd52243292 = VhOTrvGZVd794197;     VhOTrvGZVd794197 = VhOTrvGZVd89284495;     VhOTrvGZVd89284495 = VhOTrvGZVd49823308;     VhOTrvGZVd49823308 = VhOTrvGZVd91330923;     VhOTrvGZVd91330923 = VhOTrvGZVd29472628;     VhOTrvGZVd29472628 = VhOTrvGZVd91491623;     VhOTrvGZVd91491623 = VhOTrvGZVd77684934;     VhOTrvGZVd77684934 = VhOTrvGZVd53905864;     VhOTrvGZVd53905864 = VhOTrvGZVd29328495;     VhOTrvGZVd29328495 = VhOTrvGZVd83837428;     VhOTrvGZVd83837428 = VhOTrvGZVd64670041;     VhOTrvGZVd64670041 = VhOTrvGZVd99364759;     VhOTrvGZVd99364759 = VhOTrvGZVd99587628;     VhOTrvGZVd99587628 = VhOTrvGZVd90081914;     VhOTrvGZVd90081914 = VhOTrvGZVd46707794;     VhOTrvGZVd46707794 = VhOTrvGZVd19833326;     VhOTrvGZVd19833326 = VhOTrvGZVd64181012;     VhOTrvGZVd64181012 = VhOTrvGZVd92343886;     VhOTrvGZVd92343886 = VhOTrvGZVd13214653;     VhOTrvGZVd13214653 = VhOTrvGZVd77585250;     VhOTrvGZVd77585250 = VhOTrvGZVd79087549;     VhOTrvGZVd79087549 = VhOTrvGZVd19419596;     VhOTrvGZVd19419596 = VhOTrvGZVd47963507;     VhOTrvGZVd47963507 = VhOTrvGZVd44542567;     VhOTrvGZVd44542567 = VhOTrvGZVd78430729;     VhOTrvGZVd78430729 = VhOTrvGZVd47091258;     VhOTrvGZVd47091258 = VhOTrvGZVd62767472;     VhOTrvGZVd62767472 = VhOTrvGZVd67734693;     VhOTrvGZVd67734693 = VhOTrvGZVd36273237;     VhOTrvGZVd36273237 = VhOTrvGZVd57234995;     VhOTrvGZVd57234995 = VhOTrvGZVd67073988;     VhOTrvGZVd67073988 = VhOTrvGZVd44664718;     VhOTrvGZVd44664718 = VhOTrvGZVd75015153;     VhOTrvGZVd75015153 = VhOTrvGZVd17592211;     VhOTrvGZVd17592211 = VhOTrvGZVd22543242;     VhOTrvGZVd22543242 = VhOTrvGZVd68579234;     VhOTrvGZVd68579234 = VhOTrvGZVd18297073;     VhOTrvGZVd18297073 = VhOTrvGZVd69614247;     VhOTrvGZVd69614247 = VhOTrvGZVd19120690;     VhOTrvGZVd19120690 = VhOTrvGZVd68648985;     VhOTrvGZVd68648985 = VhOTrvGZVd29222821;     VhOTrvGZVd29222821 = VhOTrvGZVd29314370;     VhOTrvGZVd29314370 = VhOTrvGZVd16632412;     VhOTrvGZVd16632412 = VhOTrvGZVd42633332;     VhOTrvGZVd42633332 = VhOTrvGZVd43255731;     VhOTrvGZVd43255731 = VhOTrvGZVd49540476;     VhOTrvGZVd49540476 = VhOTrvGZVd61782295;     VhOTrvGZVd61782295 = VhOTrvGZVd66624102;     VhOTrvGZVd66624102 = VhOTrvGZVd77533700;     VhOTrvGZVd77533700 = VhOTrvGZVd37933810;}
// Junk Finished
