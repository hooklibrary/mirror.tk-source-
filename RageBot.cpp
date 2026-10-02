#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include "position_adjust.h"
#include <iostream>
#include <time.h>
#include "UTIL Functions.h"
#include "xostr.h"
#include <chrono>
#include "Hooks.h"
#include "global_count.h"
#include "laggycompensation.h"
#include "MD5.cpp"
//#include "otr_awall.h"
#include "antiaim.h"
#include "fakelag.h"
#include "experimental.h"
#include "lin_extp.h"
#include "MiscHacks.h"
#include "newbacktrack.h"
//float bigboi::current_yaw;

float current_desync;
Vector LastAngleAA2;
static bool dir = false;
static bool back = false;
static bool up = false;
static bool jitter = false;

static bool backup = false;
static bool default_aa = true;
static bool panic = false;
float hitchance_custom;
#define TICK_INTERVAL			(interfaces::globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
CAimbot * ragebot = new CAimbot;
c_newbacktrack * new_backtrack = new c_newbacktrack;
extra * ext = new extra;
void CAimbot::Init()
{
	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

void CAimbot::Draw()
{
}
float curtime_fixed(CUserCmd* ucmd) {
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

void RandomSeed(UINT seed)
{
	typedef void(*RandomSeed_t)(UINT);
	static RandomSeed_t m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed");
	m_RandomSeed(seed);
	return;
}

void CAimbot::auto_revolver(CUserCmd* m_pcmd) // credits: https://steamcommunity.com/id/x-87
{
	auto m_local = hackManager.pLocal();
	auto m_weapon = m_local->GetWeapon2();
	if (!m_weapon)
		return;

	if (!shot_this_tick && *m_weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
	{

		float flPostponeFireReady = m_weapon->GetFireReadyTime();
		if (flPostponeFireReady > 0 && flPostponeFireReady - 1 < interfaces::globals->curtime)
		{
			m_pcmd->buttons &= ~IN_ATTACK;
		}
		static int delay = 0;
		delay++;

		if (delay <= 15)
			m_pcmd->buttons |= IN_ATTACK;
		else
			delay = 0;

	}
	else
	{
		if (*m_weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			static int delay = 0;
			delay++;

			if (delay <= 15)
				m_pcmd->buttons |= IN_ATTACK;
			else
				delay = 0;
		}
	}
}


bool IsAbleToShoot(IClientEntity* pLocal)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * interfaces::globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}
float CAimbot::hitchance()
{
	float hitchance = 101;
	auto m_local = hackManager.pLocal();
	auto pWeapon = m_local->GetWeapon2();
	if (pWeapon)
	{
		if (options::menu.aimbot.AccuracyHitchance.GetValue() > 0)
		{
			float inaccuracy = pWeapon->GetInaccuracy();
			if (inaccuracy == 0) inaccuracy = 0.0000001;
			inaccuracy = 1 / inaccuracy;
			hitchance = inaccuracy;
		}
		return hitchance;
	}
}
bool CAimbot::CanOpenFire(IClientEntity * local)
{

	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(local->GetActiveWeaponHandle());
	float flServerTime = (float)local->GetTickBase() * interfaces::globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();
	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;
	return !(flNextPrimaryAttack > flServerTime);
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

float GetLerpTimeX()
{
	int ud_rate = interfaces::cvar->FindVar("cl_updaterate")->GetFloat();
	ConVar *min_ud_rate = interfaces::cvar->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = interfaces::cvar->FindVar("sv_maxupdaterate");
	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetFloat();
	float ratio = interfaces::cvar->FindVar("cl_interp_ratio")->GetFloat();
	if (ratio == 0)
		ratio = 1.0f;
	float lerp = interfaces::cvar->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = interfaces::cvar->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = interfaces::cvar->FindVar("sv_client_max_interp_ratio");
	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());
	return max(lerp, (ratio / ud_rate));
}
void CAimbot::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocalEntity = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame() || !pLocalEntity->isValidPlayer())
		return;

	c_fakelag->Fakelag(pCmd, bSendPacket);

	if (!pWeapon)
		return;

	if (options::menu.misc.AntiAimEnable.getstate())
	{
		static int ChokedPackets = 1;
		//	std::vector<dropdownboxitem> spike = options::menu.MiscTab.fl_spike.items;

		if ((ChokedPackets < 1 && pLocalEntity->GetHealth() <= 0.f && !(pWeapon->IsKnife() || pWeapon->IsC4())))
		{
			bSendPacket = false;
		}

		else
		{		
			if (pLocalEntity->IsAlive() && pLocalEntity->GetMoveType() != MOVETYPE_LADDER)
			{
				c_antiaim->DoAntiAim(pCmd, bSendPacket);
			}
			ChokedPackets = 1;
		}
	}

	if (options::menu.aimbot.AimbotEnable.getstate())
	{
		DoAimbot(pCmd, bSendPacket);
		DoNoRecoil(pCmd);
		auto_revolver(pCmd);
	}

	if (options::menu.misc.OtherSafeMode.getindex() == 1)
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 39.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 39.f;
			pCmd->viewangles = LastAngle + AddAngs;
			game_utils::NormaliseViewAngle(pCmd->viewangles);
		}
	}
	LastAngle = pCmd->viewangles;
}
inline float FastSqrt(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}
#define square( x ) ( x * x )
void ClampMovement(CUserCmd* pCommand, float fMaxSpeed)
{
	if (fMaxSpeed <= 0.f)
		return;
	float fSpeed = (float)(FastSqrt(square(pCommand->forwardmove) + square(pCommand->sidemove) + square(pCommand->upmove)));
	if (fSpeed <= 0.f)
		return;
	if (pCommand->buttons & IN_DUCK)
		fMaxSpeed *= 2.94117647f;
	if (fSpeed <= fMaxSpeed)
		return;
	float fRatio = fMaxSpeed / fSpeed;
	pCommand->forwardmove *= fRatio;
	pCommand->sidemove *= fRatio;
	pCommand->upmove *= fRatio;
}

void CAimbot::sim_time_delay(IClientEntity* entity)
{
	float old_sim[65] = { 0.f };
	float current_sim[65] = { entity->GetSimulationTime() };

	if (entity->m_flOldSimulationTime() != current_sim[entity->GetIndex()])
	{
		can_shoot = true;
		shot_refined = true;
	//	old_sim[entity->GetIndex()] = current_sim[entity->GetIndex()];
	}
	else
	{
		can_shoot = false;
		shot_refined = false;
	}
}

void CAimbot::delay_shot(IClientEntity* entity, CUserCmd* pcmd)
{
	float old_sim[65] = { 0.f };
	float current_sim[65] = { entity->GetSimulationTime() };

	bool lag_comp;

	int index = options::menu.aimbot.delay_shot.getindex();

	switch (index)
	{
	case 1:
	{
		sim_time_delay(entity);
	}
	break;

	case 2: // bameware
	{
		Vector vec_position[65], origin_delta[65];

		if (entity->m_VecORIGIN() != vec_position[entity->GetIndex()])
		{
			origin_delta[entity->GetIndex()] = entity->m_VecORIGIN() - vec_position[entity->GetIndex()];
			vec_position[entity->GetIndex()] = entity->m_VecORIGIN();

			lag_comp = fabs(origin_delta[entity->GetIndex()].Length()) > 64;
		}

		if (lag_comp && entity->GetVelocity().Length2D() > 300)
		{
			can_shoot = false;
		}
		else
			can_shoot = true;
	}
	break;

	case 3:
	{
		can_shoot = true;
	}
	break;
	}

}

namespace debug_helpers // will not be used for now
{
	auto hitbox_to_String = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case 0:
		{
			return "HEAD";
		}
		case 1:
			return "NECK";
		case 2:
			return "PELVIS";
		case 3:
			return "STOMACH";
		case 5:
			return "LOWER CHEST";
		case 6:
			return "UPPER CHEST";
		case 7:
			return "THIGHS";
		case 8:
			return"THIGHS";
		case 11:
			return "LEGS";
		case 12:
			return "LEGS";
		case 16:
			return "ARMS";
		case 17:
			return "ARMS";
		default:
			return "BODY";
		}
	};


}

/*
void CAimbot::mirror_console_debug(IClientEntity * the_nignog)
{
bool gay = backtracking->good_tick(TIME_TO_TICKS(pTarget->GetSimulationTime() + backtracking->GetLerpTime()));
bool delayshot = options::menu.aimbot.delay_shot.getindex() != 0;
bool sw = the_nignog->GetVelocity().Length2D() < 50.f && the_nignog->GetVelocity().Length2D() > 25.f;

int c = hitchance() / 1.5;
int s = the_nignog->GetVelocity().Length2D();
int h = options::menu.aimbot.AccuracyHitchance.GetValue();
std::string EVENT_2 = " [ Mirror Event ] Shot at hitbox [ ";
std::string one = debug_helpers::hitbox_to_String(HitBox);;
std::string two = " ] with an actual hit chance of [ ";
std::string three = std::to_string(c);
std::string four = " ] and a configured value of [ ";
std::string nn = std::to_string(h);
std::string ffs = " ]";
//----------------------------------------------
std::string EVENT_1 = " [ Mirror Info ] ";
std::string five = " Fake: ";
std::string six = resolver->enemy_fake ? "TRUE." : "FALSE.";
std::string seven = " Valid Tick: ";
std::string eight = gay ? "TRUE." : "FALSE.";
std::string nine = " Delay Shot: ";
std::string ten = delayshot ? "TRUE." : "FALSE.";
std::string eleven = " Enemy Speed: [ ";
std::string twelve = std::to_string(s);
std::string thirteen = " ]";
std::string fourteen = " Enemy Health: [ ";
std::string fifteen = std::to_string(the_nignog->GetHealth());
std::string sixteen = " ]";
std::string newline = ".     \n";

//----------------------------------------------
std::string EVENT_3 = " [ Mirror Setting ] ";
std::string desync1 = " Desync: ";
std::string desync2 = resolver->has_desync ? "TRUE." : "FALSE.";
std::string slow1 = " Slow Walk: ";
std::string slow2 = sw ? "TRUE." : "FALSE.";
std::string r1 = " Primary Resolver: ";
std::string r2 = options::menu.aimbot.resolver.getindex() == 2 ? "TRUE" : "FALSE";
//----------------------------------------------
std::string EVENT_4 = " [ Mirror Resolver ] ";
std::string resolver_stage_text = " Enemy Stage: ";
//	std::string resolver_stage_index = debug_helpers::stage_to_string(a_c->resolver_stage);
std::string resolver_flag_text = " Enemy Flag: ";
//	std::string resolver_flag_index = debug_helpers::resolver_flag_to_string(a_c->resolver_flag[the_nignog->GetIndex()]);

std::string homo = "         ";

std::string uremam = EVENT_2 + one + two + three + four + nn + ffs + newline;
std::string ruined_is_gay = EVENT_1 + five + six + seven + eight + nine + ten + eleven + twelve + thirteen + fourteen + fifteen + sixteen + newline;
std::string no_muslim = EVENT_3 + desync1 + desync2 + slow1 + slow2 + r1 + r2 + newline;
//	std::string i_hate_myself = EVENT_4 + resolver_stage_text + resolver_stage_index + resolver_flag_text + resolver_flag_index + newline;
std::string killme = homo + newline;

interfaces::cvar->ConsoleColorPrintf(Color(250, 100, 250, 255), uremam.c_str());
interfaces::cvar->ConsoleColorPrintf(Color(250, 100, 250, 255), ruined_is_gay.c_str());
interfaces::cvar->ConsoleColorPrintf(Color(250, 100, 250, 255), no_muslim.c_str());
//	interfaces::cvar->ConsoleColorPrintf(Color(250, 100, 250, 255), i_hate_myself.c_str()); Doesn't work i guess
interfaces::cvar->ConsoleColorPrintf(Color(100, 100, 100, 100), killme.c_str());
}
*/

void CAimbot::faxzee_extrapolation(IClientEntity * pTarget, Vector AP)
{
	pTarget->GetPredicted(AP);

	bt_2->ShotBackTrackStoreFSN(pTarget);

	{
		Vector position = pTarget->GetAbsOriginlol();
		Vector extr_position = position;

		float old_simtime = CMBacktracking::Get().current_record[pTarget->GetIndex()].m_flSimulationTime;
		float simtime = pTarget->GetSimulationTime();

		cm_backtrack->ExtrapolatePosition(pTarget, extr_position, simtime, pTarget->GetVelocity());

		AP -= position;
		AP += extr_position;
	}

	bt_2->ShotBackTrackBeforeAimbot(pTarget);

	bt_2->RestoreTemporaryRecord(pTarget);

	cm_backtrack->StartLagCompensation(pTarget);

	new_backtracking->RestoreTemporaryRecord(pTarget);
}


typedef void(__cdecl* MsgFn)(const char* msg, va_list);

void gamer_message(const char* msg, ...)
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

inline int time_to_ticks(float time) {
	return static_cast< int >(time / interfaces::globals->interval_per_tick + 0.5f);
}

float CAimbot::interpolation_time() { // interpolation niggas

	static const auto                 cl_updaterate = interfaces::cvar->FindVar("cl_updaterate");
	static const auto              sv_minupdaterate = interfaces::cvar->FindVar("sv_minupdaterate");
	static const auto              sv_maxupdaterate = interfaces::cvar->FindVar("sv_maxupdaterate");
	static const auto               cl_interp_ratio = interfaces::cvar->FindVar("cl_interp_ratio");
	static const auto                     cl_interp = interfaces::cvar->FindVar("cl_interp");
	static const auto    sv_client_min_interp_ratio = interfaces::cvar->FindVar("sv_client_min_interp_ratio");
	static const auto    sv_client_max_interp_ratio = interfaces::cvar->FindVar("sv_client_max_interp_ratio");

	const auto update_rate = clamp(cl_updaterate->GetFloat(),
		sv_minupdaterate->GetFloat(),
		sv_maxupdaterate->GetFloat());

	const auto base_interp_ratio = (cl_interp_ratio->GetFloat() == 0.0f) ? 1.0f : cl_interp_ratio->GetFloat();

	const auto interp_ratio = clamp(base_interp_ratio,
		sv_client_min_interp_ratio->GetFloat(),
		sv_client_max_interp_ratio->GetFloat());

	const auto interp_latency = (interp_ratio / update_rate);

	return std::max<float>(cl_interp->GetFloat(), interp_latency);
}

void CAimbot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	bool vac_kick = options::menu.misc.OtherSafeMode.getindex() == 1;
	bool current_lagpred = options::menu.aimbot.lag_pred.getindex() > 1;
	IClientEntity* pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	bool FindNewTarget = true;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	CMBacktracking gladbacktrack;
	if (!pLocal)
		return;

	if (pWeapon)
	{
		if ((!pWeapon->isZeus() && pWeapon->GetAmmoInClip() < 1) || pWeapon->IsKnife() || pWeapon->IsC4() || pWeapon->IsGrenade())
			return;
	}
	else
		return;

	if (IsLocked && TargetID > -0 && HitBox >= 0)
	{
		pTarget = interfaces::ent_list->get_client_entity(TargetID);
		if (pTarget && TargetMeetsRequirements(pTarget, pLocal))
		{
			HitBox = HitScan(pTarget, pLocal);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View; interfaces::engine->get_viewangles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);

				if (FoV < vac_kick ? 39.f : options::menu.aimbot.AimbotFov.GetValue())
					FindNewTarget = false;
			}
		}
	}

	if (FindNewTarget)
	{
		Globals::Shots = 0;
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		TargetID = get_target_fov(pLocal);

		if (TargetID >= 0)
		{
			pTarget = interfaces::ent_list->get_client_entity(TargetID);
		}

	}

	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget, pLocal);

		if (!CanOpenFire(pLocal))
			return;

		//	bool IsAtPeakOfJump = fabs(pLocal->GetVelocity().z) <= 5.0f;

		Vector AP;
		AP = options::menu.aimbot.Multienable.getstate() ? GetHitboxPosition(pTarget, HitBox) : hitbox_location(pTarget, HitBox);

		shot_this_tick = false;

		switch (options::menu.aimbot.lag_pred.getindex())
		{
		case 1: pTarget->GetPredicted(AP);
			break;
		case 2:
			faxzee_extrapolation(pTarget, AP);
			break;
		}

		switch (options::menu.misc.QuickStop.getindex())
		{
		case 1:
		{
			if (pLocal->GetFlags() & FL_ONGROUND)
			{
				ClampMovement(pCmd, c_misc->get_gun(pWeapon));
			}
		}
		break;

		case 2:
		{
			if (pLocal->GetVelocity().Length2D() > 40.f)
			{
				c_misc->MinimalWalk(pCmd, c_misc->get_gun(pWeapon));
			}
		}
		break;
		}

		//	Globals::missedshots[pTarget->GetIndex()] = Globals::fired[pTarget->GetIndex()] - Globals::hit[pTarget->GetIndex()];

		float hc = hitchance();
		if (game_utils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && options::menu.aimbot.AccuracyAutoScope.getstate())
		{
			pCmd->buttons |= IN_ATTACK2;
		}

		int tickdiff;
		tickdiff = TIME_TO_TICKS(interfaces::engine->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + interfaces::engine->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING));

		auto simtime = pTarget->get_simulation_time();
		simtime += interpolation_time();

		if (pWeapon->isZeus27() && c_misc->do_zeus)
		{
			if (AimAtPoint(pLocal, AP, pCmd, bSendPacket))
			{
				for (int t = 0; t < (bt_2->ticks + 1); ++t)
				{
					if (bt_2->ticks < 1) // let's go for backtrack ticks
						continue;

					sim_time_delay(pTarget); // make sure simtime is correct to help in hitting

					if (!shot_refined) // no u
						continue;

					pCmd->buttons |= IN_ATTACK; // bang
					if (backtracking->IsTickValid(pTarget->GetSimulationTime() + backtracking->GetLerpTime())) // is it a valid tick?
						pCmd->tick_count = TIME_TO_TICKS(simtime);

				}
			}
			return;
		}

		if ((hc >= options::menu.aimbot.AccuracyHitchance.GetValue() * 1.5))
		{
			if (options::menu.misc.QuickCrouch.getstate())
				pCmd->buttons |= IN_DUCK;

			if (pLocal->get_animation_state()->m_fDuckAmount > 0.1 && !(GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey())) && !interfaces::m_iInputSys->IsButtonDown(KEY_LCONTROL))
				return;

			if (AimAtPoint(pLocal, AP, pCmd, bSendPacket))
			{
				if (options::menu.aimbot.AimbotAutoFire.getstate() && !(pCmd->buttons & IN_ATTACK))
				{

					//				if ((GetAsyncKeyState(options::menu.misc.fake_crouch_key.GetKey())) && pLocal->get_animation_state()->m_fDuckAmount > 0.3)
					//					return;

					if (options::menu.aimbot.delay_shot.getindex() != 0)
					{
						delay_shot(pTarget, pCmd);

						switch (can_shoot)
						{
						case true:
						{
							switch (options::menu.aimbot.lag_pred.getindex())
							{
							case 1: // old mirror vibes
							{

								cbacktracking::Get().ShotBackTrackAimbotStart(pTarget);
								cbacktracking::Get().RestoreTemporaryRecord(pTarget);
								cbacktracking::Get().ShotBackTrackedTick(pTarget);

								c_fakelag->shot = true;

								pCmd->buttons |= IN_ATTACK;
								if (backtracking->IsTickValid(pTarget->GetSimulationTime() + backtracking->GetLerpTime()))
									pCmd->tick_count = TIME_TO_TICKS(pTarget->GetSimulationTime()) + TIME_TO_TICKS(backtracking->GetLerpTime()) + tickdiff;

								c_fakelag->shot = false;
								shot_this_tick = true;
							}
							break;

							case 2:
							{
								//		if (options::menu.aimbot.delay_shot.getindex() > 2)
								//			new_backtracking->ShotBackTrackAimbotStart(pTarget);

								c_fakelag->shot = true;
								Globals::fired[pTarget->GetIndex()]++;

								for (int t = 0; t < (bt_2->ticks + 1); ++t)
								{
									if (bt_2->ticks < 1) // let's go for backtrack ticks
										continue;

									sim_time_delay(pTarget); // make sure simtime is correct to help in hitting

									if (!shot_refined) // no u
										continue;

									pCmd->buttons |= IN_ATTACK; // bang
									if (backtracking->IsTickValid(pTarget->GetSimulationTime() + backtracking->GetLerpTime())) // is it a valid tick?
										pCmd->tick_count = TIME_TO_TICKS(simtime);

								}

								c_fakelag->shot = false;
								shot_this_tick = true;
							}
							break;

							}
						}
						break;
						}

					}
					else
					{
						c_fakelag->shot = true;

						if (options::menu.misc.QuickCrouch.getstate())
							pCmd->buttons |= IN_DUCK;

						pCmd->buttons |= IN_ATTACK;
						Globals::fired[pTarget->GetIndex()]++;

						c_fakelag->shot = false;
						shot_this_tick = true;
					}

					was_firing_test = true;

					if (!(pCmd->buttons |= IN_ATTACK))
					{
						shot_this_tick = false;
					}

				}
				else if (pCmd->buttons & IN_ATTACK || (pCmd->buttons & IN_ATTACK2 && !game_utils::AutoSniper(pWeapon)))
				{

					Globals::fired[pTarget->GetIndex()]++;
					c_fakelag->shot = false;
					was_firing_test = false;

					return;
				}

				if (*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() != 64)
				{
					static bool WasFiring = false;
					if (*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 31)
					{
						if (pCmd->buttons & IN_ATTACK)
						{
							if (WasFiring)
							{
								pCmd->buttons &= ~IN_ATTACK;
								//	was_firing = true;
							}
						}
						//		else
						//			was_firing = false;

						WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
					}
				}
			}
		}

	}

	if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) {
		Globals::Shots += 1;
	}
	//	missed_shot_log(pCmd, pWeapon);
}


float VectorDistance(const Vector& v1, const Vector& v2)
{
	return FastSqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

bool CAimbot::TargetMeetsRequirements(IClientEntity* pEntity, IClientEntity* local)
{
	//	auto local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
//	ClientClass *pClientClass = pEntity->GetClientClass();
	if (pEntity->isValidPlayer())
	{
		player_info_t pinfo;
		if (pEntity->cs_player() && interfaces::engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(local->GetActiveWeaponHandle());

			float Distance = VectorDistance(local->GetEyePosition(), pEntity->GetEyePosition());

			if (pWeapon && pWeapon->GetCSWpnData()->range < Distance)
				return false;

			if (!pEntity->is_dormant())
			{
				// Team Check
				if (options::menu.misc.OtherSafeMode.getindex() != 3 ? pEntity->team() != local->team() : pEntity->GetIndex() != local->GetIndex())
				{
					// Spawn And Dormant Check
					if (!pEntity->has_gungame_immunity())
					{
						if (!(pEntity->GetFlags() & FL_FROZEN)) // ice age niggas
						{
							return true;
						}
					}

				}
			}
			
		}
	}
	return false;
}

float CAimbot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{

	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);

	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}
int CAimbot::get_target_fov(IClientEntity* pLocal)
{
	int target = -1;
	float min_fov = 180.f;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; interfaces::engine->get_viewangles(View);

	for (int i = 0; i < 65; i++)
	{
		IClientEntity *pEntity = interfaces::ent_list->get_client_entity(i);
		if (TargetMeetsRequirements(pEntity, pLocal))
		{
			if (pEntity->GetOrigin() == Vector(0, 0, 0))
				continue;

			int NewHitBox = HitScan(pEntity, pLocal);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov <= min_fov && fov <= 180.f)
				{
					min_fov = fov;
					target = i;
				}
			}
		}
	}

	return target;
}
float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;
	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);
	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}
bool CAimbot::should_baim(IClientEntity* pEntity)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pEntity->GetActiveWeaponHandle());

	if (!pWeapon)
		return false;

	int health = options::menu.aimbot.BaimIfUnderXHealth.GetValue();
	int miss[65] = { Globals::missedshots[pEntity->GetIndex()] };

	bool nn[65] = { miss[pEntity->GetIndex()] > 2 };

	if ((GetAsyncKeyState(options::menu.aimbot.bigbaim.GetKey()) ||
		(options::menu.aimbot.baim_fakewalk.getstate() && (enemy_is_slow_walking(pEntity)))
		|| (options::menu.aimbot.baim_fake.getstate() && nn[pEntity->GetIndex()])
		|| (options::menu.aimbot.baim_inair.getstate() && !(pEntity->GetFlags() & FL_ONGROUND))
		|| pEntity->GetHealth() <= health)
		|| game_utils::IsZeus(pWeapon))
	{
		return true;
	}
	else
		return false;

	return false;
}


void CAimbot::missed_shot_log(CUserCmd * pcmd, C_BaseCombatWeapon* pWeapon)
{

	if (!pTarget)
		return;

	static int missedshots[65];
	missedshots[pTarget->GetIndex()] = Globals::missedshots[pTarget->GetIndex()];

	if (CanOpenFire(hackManager.pLocal()))
		return;

	bool reset = false;
	valid_hitchance = hitchance() <= options::menu.aimbot.AccuracyHitchance.GetValue() * 1.5;

	if ( (missedshots[pTarget->GetIndex()] > 0 && Globals::fired[pTarget->GetIndex()] != missedshots[pTarget->GetIndex()] && pWeapon->GetInaccuracy() > 0 && pcmd->buttons & IN_ATTACK) || (Globals::fired[pTarget->GetIndex()] > missedshots[pTarget->GetIndex()] && !reset) )
	{
		
		interfaces::cvar->ConsoleColorPrintf(Color(160, 5, 240, 255), "Mirror: ");

		std::string one = "missed due to innacuracy";
		std::string newline = ".     \n";
		std::string uremam = one + newline;

		gamer_message(uremam.c_str());

		Globals::fired[pTarget->GetIndex()] = 0;
		Globals::hit[pTarget->GetIndex()] = 0;
		Globals::missedshots[pTarget->GetIndex()] = 0.f;
		missedshots[pTarget->GetIndex()] = 0;

		reset = true;
	}

	IGameEvent* event;

	if (event == nullptr)
		return;
	int user = event->GetInt("userid");

	if ((Globals::fired[pTarget->GetIndex()] != Globals::missedshots[pTarget->GetIndex()] || ext->current_flag[interfaces::engine->GetPlayerForUserID(user)] == correction_flags::DESYNC) && pTarget->GetVelocity().Length2D() < 120 )
	{
		interfaces::cvar->ConsoleColorPrintf(Color(160, 5, 240, 255), "Mirror: ");

		std::string one = "missed due to bad resolve";
		std::string newline = ".     \n";
		std::string uremam = one + newline;
		gamer_message(uremam.c_str());
	}
}


bool CAimbot::enemy_is_slow_walking(IClientEntity * entity)
{
	C_BaseCombatWeapon* weapon = entity->GetWeapon2();
	if (!weapon)
		return false;

	float speed = entity->GetVelocity().Length2D();

	if (speed < 50.f && speed >= 25.f) // if it's more or less the same.
	{
		return true;
	}
	else
		return false;

}

/*
int CAimbot::automatic_hitscan(IClientEntity * entity)
{
	int hp = entity->GetHealth();
	int speed = entity->GetVelocity().Length();

	if (entity == nullptr)
		return 0;

#pragma region " 1 = head, pelvis | 2 = head, stomach, pelvis | 3 - full scan | 4- body only | 5 - head, lower body, legs "

	if (speed >= 210)
	{
		if (hp >= 70)
		{
			return 1;
		}

		if (hp < 70 && hp >= 50)
		{
			return 2;
		}

		if (hp < 50)
		{
			return 3;
		}
	}

	if (speed < 210 && speed >= 150)
	{
		if (hp >= 50)
		{
			return 2;
		}

		if (hp < 50)
		{
			return 3;
		}
	}

	if (speed < 150)
	{
		if (hp >= 70)
		{
			return 5;
		}

		if (hp < 70 && hp > 50)
		{
			return 3;
		}

		if (hp <= 50)
		{
			return 4;
		}
	}
}
*/

std::vector<int> CAimbot::head_hitscan()
{
	std::vector<int> hitbox;
	hitbox.push_back((int)csgo_hitboxes::head);
	//	hitbox.push_back((int)csgo_hitboxes::neck);

	return hitbox;
}


/*
std::vector<int> CAimbot::upperbody_hitscan()
{
std::vector<int> hitbox;
hitbox.push_back((int)csgo_hitboxes::upper_chest);
hitbox.push_back((int)csgo_hitboxes::thorax);

return hitbox;
}
*/
std::vector<int> CAimbot::lowerbody_hitscan()
{
	std::vector<int> hitbox;
	hitbox.push_back((int)csgo_hitboxes::lower_chest);
	hitbox.push_back((int)csgo_hitboxes::pelvis);
	hitbox.push_back((int)csgo_hitboxes::stomach);
	hitbox.push_back((int)csgo_hitboxes::left_thigh);
	hitbox.push_back((int)csgo_hitboxes::right_thigh);

	return hitbox;
}
/*
std::vector<int> CAimbot::arms_hitscan()
{
std::vector<int> hitbox;
hitbox.push_back((int)csgo_hitboxes::right_upper_arm);
hitbox.push_back((int)csgo_hitboxes::left_upper_arm);
hitbox.push_back((int)csgo_hitboxes::right_lower_arm);
hitbox.push_back((int)csgo_hitboxes::left_lower_arm);
hitbox.push_back((int)csgo_hitboxes::right_hand);
hitbox.push_back((int)csgo_hitboxes::left_hand);

return hitbox;
}

std::vector<int> CAimbot::legs_hitscan()
{
std::vector<int> hitbox;
hitbox.push_back((int)csgo_hitboxes::left_calf);
hitbox.push_back((int)csgo_hitboxes::left_foot);
hitbox.push_back((int)csgo_hitboxes::right_calf);
hitbox.push_back((int)csgo_hitboxes::right_foot);

return hitbox;
}
*/

std::vector<int> CAimbot::awp_minimal_hitscan()
{
	std::vector<int> scan_hitboxes;
	scan_hitboxes.push_back((int)csgo_hitboxes::stomach);
	scan_hitboxes.push_back((int)csgo_hitboxes::lower_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::pelvis);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_foot);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_foot);

	return scan_hitboxes;
}

std::vector<int> CAimbot::minimal_hitscan()
{
	std::vector<int> scan_hitboxes;
	scan_hitboxes.push_back((int)csgo_hitboxes::head);
	scan_hitboxes.push_back((int)csgo_hitboxes::head);
	scan_hitboxes.push_back((int)csgo_hitboxes::upper_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::pelvis);

	scan_hitboxes.push_back((int)csgo_hitboxes::right_foot);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_foot);

	return scan_hitboxes;
}

std::vector<int> CAimbot::essential_hitscan()
{
	std::vector<int> scan_hitboxes;
	scan_hitboxes.push_back((int)csgo_hitboxes::head);
	scan_hitboxes.push_back((int)csgo_hitboxes::upper_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::lower_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_upper_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_upper_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::pelvis);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_thigh);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_thigh);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_foot);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_foot);

	return scan_hitboxes;
}

std::vector<int> CAimbot::maximal_hitscan()
{
	std::vector<int> scan_hitboxes; 
	scan_hitboxes.push_back((int)csgo_hitboxes::head);
	scan_hitboxes.push_back((int)csgo_hitboxes::head);
	scan_hitboxes.push_back((int)csgo_hitboxes::upper_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::lower_chest);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_upper_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_upper_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_lower_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_lower_arm);
	scan_hitboxes.push_back((int)csgo_hitboxes::stomach);
	scan_hitboxes.push_back((int)csgo_hitboxes::pelvis);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_thigh);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_thigh);
	scan_hitboxes.push_back((int)csgo_hitboxes::right_foot);
	scan_hitboxes.push_back((int)csgo_hitboxes::left_foot);

	return scan_hitboxes;
}

int CAimbot::HitScan(IClientEntity* pEntity, IClientEntity* pLocal)
{

	float health = options::menu.aimbot.BaimIfUnderXHealth.GetValue();

	std::vector<int> scan_hitboxes;
	//	std::vector<dropdownboxitem> auto_list = options::menu.aimbot.target_auto.items;
	//	std::vector<dropdownboxitem> scout_list = options::menu.aimbot.target_scout.items;
	//	std::vector<dropdownboxitem> awp_list = options::menu.aimbot.target_awp.items;
	//	std::vector<dropdownboxitem> pistol_list = options::menu.aimbot.target_pistol.items;
	//	std::vector<dropdownboxitem> smg_list = options::menu.aimbot.target_smg.items;
	//	std::vector<dropdownboxitem> otr_list = options::menu.aimbot.target_otr.items;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (pWeapon != nullptr)
	{
		switch (should_baim(pEntity))
		{
			case true:
			{
				scan_hitboxes = lowerbody_hitscan();
			}
			break;

			case false:
			{
				if (pWeapon->isAuto())
				{

					switch (options::menu.aimbot.target_auto2.getindex())
					{
					case 1:
						scan_hitboxes = minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}
				if (pWeapon->is_scout())
				{
					switch (options::menu.aimbot.target_scout2.getindex())
					{
					case 1:
						scan_hitboxes = minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}

				if (pWeapon->is_awp())
				{
					switch (options::menu.aimbot.target_awp2.getindex())
					{
					case 1:
						scan_hitboxes = awp_minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}

				if (pWeapon->isPistol()) // head
				{

					switch (options::menu.aimbot.target_pistol2.getindex())
					{
					case 1:
						scan_hitboxes = minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}

				if (game_utils::IsMP(pWeapon))
				{

					switch (options::menu.aimbot.target_smg2.getindex())
					{
					case 1:
						scan_hitboxes = minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}

				if (game_utils::IsRifle(pWeapon) || game_utils::IsShotgun(pWeapon) || game_utils::IsMachinegun(pWeapon))
				{

					switch (options::menu.aimbot.target_otr2.getindex())
					{
					case 1:
						scan_hitboxes = minimal_hitscan();
						break;
					case 2:
						scan_hitboxes = essential_hitscan();
						break;
					case 3:
						scan_hitboxes = maximal_hitscan();
						break;
					}

				}
			}
			break;
		}	

		for (auto HitBoxID : scan_hitboxes)
		{
			Vector Point, lol;
			Point = hitbox_location(pEntity, HitBoxID);

			float dmg = 0.f;

			if (backup_awall->can_hit(Point, &dmg))
			{
				return HitBoxID;
			}

		}
		return -1;
	}
}

void CAimbot::DoNoRecoil(CUserCmd *pCmd)
{
	Vector AimPunch = hackManager.pLocal()->localPlayerExclusive()->GetAimPunchAngle();
	if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
	{
		pCmd->viewangles -= AimPunch * 2.00;
		game_utils::NormaliseViewAngle(pCmd->viewangles);
	}
}

void CAimbot::aimAtPlayer(CUserCmd *pCmd, IClientEntity* pLocal)
{
	//	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->range;

	IClientEntity* target = nullptr;

	for (int i = 0; i <= 65; i++)
	{
		IClientEntity *pEntity = interfaces::ent_list->get_client_entity(i);
		if (TargetMeetsRequirements(pEntity, pLocal))
		{
			if (TargetID != -1)
				target = interfaces::ent_list->get_client_entity(TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();
			Vector CurPos = target->GetEyePosition() + target->GetAbsOrigin();

			float temp_dist = eye_position.DistTo(target_position);
			QAngle angle = QAngle(0, 0, 0);
			float lowest = 99999999.f;
			if (CurPos.DistToSqr(eye_position) < lowest)
			{
				lowest = CurPos.DistTo(eye_position);
				CalcAngle(eye_position, target_position, angle);
			}
		}
	}
}

bool CAimbot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;
	if (point.Length() == 0) return ReturnValue;
	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();
	CalcAngle(src, point, angles);
	game_utils::NormaliseViewAngle(angles);
	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}
	IsLocked = true;
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle;
	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, interfaces::ent_list->get_client_entity(TargetID), 0);
	Vector AddAngs = angles - LastAimstepAngle;
	if (fovLeft > 29.0f && options::menu.misc.OtherSafeMode.getindex() == 1)
	{
		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 29.0f;
		LastAimstepAngle += AddAngs;
		game_utils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}

	else
	{
		ReturnValue = true;
	}
	//	if (Options::Menu.aimbot_tab.AimbotSilentAim.GetState())
	//	{
	pCmd->viewangles = angles;
	//	}
	//	if (!Options::Menu.aimbot_tab.AimbotSilentAim.GetState())
	//	{
	//		Interfaces::Engine->SetViewAngles(angles);
	//	}

	if (options::menu.aimbot.AimbotSilentAim.getstate())
	{
		Vector oViewangles = pCmd->viewangles;
		float oForwardmove = pCmd->forwardmove;
		float oSidemove = pCmd->sidemove;

		static auto choked = 0.f;

		//So we dont kill ourselfs
		if (choked < 1 && !c_fakelag->shot && HitBox == -1)
		{
			bSendPacket = false;
			choked++;
		}

		if (c_fakelag->shot)
			bSendPacket = true;

		if (bSendPacket)
		{

			pCmd->viewangles = oViewangles;
			pCmd->forwardmove = oForwardmove;
			pCmd->sidemove = oSidemove;
			choked++;
		}
	}

	return ReturnValue;
}































































































































































// Junk Code By Troll Face & Thaisen's Gen
void OXuBTKcziZ78996743() {     int dvEytMFGIe357309 = -517521518;    int dvEytMFGIe44471414 = 52207229;    int dvEytMFGIe88334036 = -437556279;    int dvEytMFGIe78753438 = -325310623;    int dvEytMFGIe59553343 = -462139273;    int dvEytMFGIe25585140 = -715869558;    int dvEytMFGIe61732982 = -500117685;    int dvEytMFGIe25134630 = -422030424;    int dvEytMFGIe28558322 = -878829635;    int dvEytMFGIe94172898 = -314447848;    int dvEytMFGIe30414400 = -484122001;    int dvEytMFGIe33568238 = 39895366;    int dvEytMFGIe15846686 = -311819145;    int dvEytMFGIe57281117 = -24791479;    int dvEytMFGIe93555926 = -478544924;    int dvEytMFGIe11490787 = -652328465;    int dvEytMFGIe16286575 = -139305503;    int dvEytMFGIe11721221 = -823854685;    int dvEytMFGIe25567057 = -983607303;    int dvEytMFGIe55411102 = -119165243;    int dvEytMFGIe74111693 = -70419325;    int dvEytMFGIe66337453 = -456698062;    int dvEytMFGIe11961973 = -603283681;    int dvEytMFGIe33195268 = -272229523;    int dvEytMFGIe14734569 = -600374514;    int dvEytMFGIe90658458 = -160300519;    int dvEytMFGIe44910496 = -255427828;    int dvEytMFGIe96755731 = -338914473;    int dvEytMFGIe12181008 = -363576768;    int dvEytMFGIe6438080 = -214418857;    int dvEytMFGIe30501684 = -736371975;    int dvEytMFGIe99912144 = -669708516;    int dvEytMFGIe38058900 = -695781354;    int dvEytMFGIe42333308 = -92931928;    int dvEytMFGIe12570010 = -653479433;    int dvEytMFGIe44867889 = -715818571;    int dvEytMFGIe45146619 = 77305830;    int dvEytMFGIe56308533 = -720634116;    int dvEytMFGIe10258711 = -471516204;    int dvEytMFGIe24192522 = 42799527;    int dvEytMFGIe74485942 = -580333714;    int dvEytMFGIe31549566 = 27974555;    int dvEytMFGIe79215598 = -613869587;    int dvEytMFGIe73988473 = -173621318;    int dvEytMFGIe40618394 = -651826297;    int dvEytMFGIe39371018 = -803952446;    int dvEytMFGIe69168580 = -698094500;    int dvEytMFGIe95842354 = -960022230;    int dvEytMFGIe10023435 = 33547003;    int dvEytMFGIe15240597 = -868974885;    int dvEytMFGIe85808082 = -656098890;    int dvEytMFGIe46906357 = -971589937;    int dvEytMFGIe53917224 = -388343779;    int dvEytMFGIe91940647 = -667344912;    int dvEytMFGIe4885051 = -718303145;    int dvEytMFGIe34019855 = 39176543;    int dvEytMFGIe32509442 = -344509090;    int dvEytMFGIe55138769 = -65326756;    int dvEytMFGIe64018870 = -724936110;    int dvEytMFGIe68894885 = -201838755;    int dvEytMFGIe80674644 = -360441731;    int dvEytMFGIe64977251 = -61203213;    int dvEytMFGIe12953622 = 41546344;    int dvEytMFGIe22120243 = -564410778;    int dvEytMFGIe63671214 = -578075874;    int dvEytMFGIe30502255 = -814413486;    int dvEytMFGIe95509338 = -264323281;    int dvEytMFGIe73513377 = -118887217;    int dvEytMFGIe44711107 = -371312046;    int dvEytMFGIe48688037 = -762726353;    int dvEytMFGIe66344167 = -629634295;    int dvEytMFGIe59978042 = -418671388;    int dvEytMFGIe1462510 = -252338482;    int dvEytMFGIe1374535 = -926406831;    int dvEytMFGIe80925159 = -538831529;    int dvEytMFGIe42562128 = 1606120;    int dvEytMFGIe87121854 = -842828476;    int dvEytMFGIe37973499 = -329662364;    int dvEytMFGIe92576873 = -620403226;    int dvEytMFGIe75363550 = -796422069;    int dvEytMFGIe21489878 = -462206019;    int dvEytMFGIe49068141 = -295405598;    int dvEytMFGIe86732296 = -272461476;    int dvEytMFGIe96940411 = -494601883;    int dvEytMFGIe20629998 = -558319968;    int dvEytMFGIe83595327 = -764782038;    int dvEytMFGIe45994921 = -181364738;    int dvEytMFGIe46118253 = 71563557;    int dvEytMFGIe37448258 = -374628784;    int dvEytMFGIe78550154 = -592655977;    int dvEytMFGIe12358448 = -271309482;    int dvEytMFGIe90007850 = -857367414;    int dvEytMFGIe92289662 = -995698006;    int dvEytMFGIe41363826 = -169677449;    int dvEytMFGIe43517877 = -596758742;    int dvEytMFGIe9508692 = -419130502;    int dvEytMFGIe18595944 = 86428210;    int dvEytMFGIe57095356 = 50541191;    int dvEytMFGIe10317259 = -595545444;    int dvEytMFGIe10116140 = -517521518;     dvEytMFGIe357309 = dvEytMFGIe44471414;     dvEytMFGIe44471414 = dvEytMFGIe88334036;     dvEytMFGIe88334036 = dvEytMFGIe78753438;     dvEytMFGIe78753438 = dvEytMFGIe59553343;     dvEytMFGIe59553343 = dvEytMFGIe25585140;     dvEytMFGIe25585140 = dvEytMFGIe61732982;     dvEytMFGIe61732982 = dvEytMFGIe25134630;     dvEytMFGIe25134630 = dvEytMFGIe28558322;     dvEytMFGIe28558322 = dvEytMFGIe94172898;     dvEytMFGIe94172898 = dvEytMFGIe30414400;     dvEytMFGIe30414400 = dvEytMFGIe33568238;     dvEytMFGIe33568238 = dvEytMFGIe15846686;     dvEytMFGIe15846686 = dvEytMFGIe57281117;     dvEytMFGIe57281117 = dvEytMFGIe93555926;     dvEytMFGIe93555926 = dvEytMFGIe11490787;     dvEytMFGIe11490787 = dvEytMFGIe16286575;     dvEytMFGIe16286575 = dvEytMFGIe11721221;     dvEytMFGIe11721221 = dvEytMFGIe25567057;     dvEytMFGIe25567057 = dvEytMFGIe55411102;     dvEytMFGIe55411102 = dvEytMFGIe74111693;     dvEytMFGIe74111693 = dvEytMFGIe66337453;     dvEytMFGIe66337453 = dvEytMFGIe11961973;     dvEytMFGIe11961973 = dvEytMFGIe33195268;     dvEytMFGIe33195268 = dvEytMFGIe14734569;     dvEytMFGIe14734569 = dvEytMFGIe90658458;     dvEytMFGIe90658458 = dvEytMFGIe44910496;     dvEytMFGIe44910496 = dvEytMFGIe96755731;     dvEytMFGIe96755731 = dvEytMFGIe12181008;     dvEytMFGIe12181008 = dvEytMFGIe6438080;     dvEytMFGIe6438080 = dvEytMFGIe30501684;     dvEytMFGIe30501684 = dvEytMFGIe99912144;     dvEytMFGIe99912144 = dvEytMFGIe38058900;     dvEytMFGIe38058900 = dvEytMFGIe42333308;     dvEytMFGIe42333308 = dvEytMFGIe12570010;     dvEytMFGIe12570010 = dvEytMFGIe44867889;     dvEytMFGIe44867889 = dvEytMFGIe45146619;     dvEytMFGIe45146619 = dvEytMFGIe56308533;     dvEytMFGIe56308533 = dvEytMFGIe10258711;     dvEytMFGIe10258711 = dvEytMFGIe24192522;     dvEytMFGIe24192522 = dvEytMFGIe74485942;     dvEytMFGIe74485942 = dvEytMFGIe31549566;     dvEytMFGIe31549566 = dvEytMFGIe79215598;     dvEytMFGIe79215598 = dvEytMFGIe73988473;     dvEytMFGIe73988473 = dvEytMFGIe40618394;     dvEytMFGIe40618394 = dvEytMFGIe39371018;     dvEytMFGIe39371018 = dvEytMFGIe69168580;     dvEytMFGIe69168580 = dvEytMFGIe95842354;     dvEytMFGIe95842354 = dvEytMFGIe10023435;     dvEytMFGIe10023435 = dvEytMFGIe15240597;     dvEytMFGIe15240597 = dvEytMFGIe85808082;     dvEytMFGIe85808082 = dvEytMFGIe46906357;     dvEytMFGIe46906357 = dvEytMFGIe53917224;     dvEytMFGIe53917224 = dvEytMFGIe91940647;     dvEytMFGIe91940647 = dvEytMFGIe4885051;     dvEytMFGIe4885051 = dvEytMFGIe34019855;     dvEytMFGIe34019855 = dvEytMFGIe32509442;     dvEytMFGIe32509442 = dvEytMFGIe55138769;     dvEytMFGIe55138769 = dvEytMFGIe64018870;     dvEytMFGIe64018870 = dvEytMFGIe68894885;     dvEytMFGIe68894885 = dvEytMFGIe80674644;     dvEytMFGIe80674644 = dvEytMFGIe64977251;     dvEytMFGIe64977251 = dvEytMFGIe12953622;     dvEytMFGIe12953622 = dvEytMFGIe22120243;     dvEytMFGIe22120243 = dvEytMFGIe63671214;     dvEytMFGIe63671214 = dvEytMFGIe30502255;     dvEytMFGIe30502255 = dvEytMFGIe95509338;     dvEytMFGIe95509338 = dvEytMFGIe73513377;     dvEytMFGIe73513377 = dvEytMFGIe44711107;     dvEytMFGIe44711107 = dvEytMFGIe48688037;     dvEytMFGIe48688037 = dvEytMFGIe66344167;     dvEytMFGIe66344167 = dvEytMFGIe59978042;     dvEytMFGIe59978042 = dvEytMFGIe1462510;     dvEytMFGIe1462510 = dvEytMFGIe1374535;     dvEytMFGIe1374535 = dvEytMFGIe80925159;     dvEytMFGIe80925159 = dvEytMFGIe42562128;     dvEytMFGIe42562128 = dvEytMFGIe87121854;     dvEytMFGIe87121854 = dvEytMFGIe37973499;     dvEytMFGIe37973499 = dvEytMFGIe92576873;     dvEytMFGIe92576873 = dvEytMFGIe75363550;     dvEytMFGIe75363550 = dvEytMFGIe21489878;     dvEytMFGIe21489878 = dvEytMFGIe49068141;     dvEytMFGIe49068141 = dvEytMFGIe86732296;     dvEytMFGIe86732296 = dvEytMFGIe96940411;     dvEytMFGIe96940411 = dvEytMFGIe20629998;     dvEytMFGIe20629998 = dvEytMFGIe83595327;     dvEytMFGIe83595327 = dvEytMFGIe45994921;     dvEytMFGIe45994921 = dvEytMFGIe46118253;     dvEytMFGIe46118253 = dvEytMFGIe37448258;     dvEytMFGIe37448258 = dvEytMFGIe78550154;     dvEytMFGIe78550154 = dvEytMFGIe12358448;     dvEytMFGIe12358448 = dvEytMFGIe90007850;     dvEytMFGIe90007850 = dvEytMFGIe92289662;     dvEytMFGIe92289662 = dvEytMFGIe41363826;     dvEytMFGIe41363826 = dvEytMFGIe43517877;     dvEytMFGIe43517877 = dvEytMFGIe9508692;     dvEytMFGIe9508692 = dvEytMFGIe18595944;     dvEytMFGIe18595944 = dvEytMFGIe57095356;     dvEytMFGIe57095356 = dvEytMFGIe10317259;     dvEytMFGIe10317259 = dvEytMFGIe10116140;     dvEytMFGIe10116140 = dvEytMFGIe357309;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RqOXqqyATy49452617() {     int tnOILpJbCy13143476 = -652562210;    int tnOILpJbCy31242303 = -118126784;    int tnOILpJbCy6086837 = -845915661;    int tnOILpJbCy86694263 = -149512900;    int tnOILpJbCy53064448 = -953955738;    int tnOILpJbCy68895211 = -492792872;    int tnOILpJbCy82940740 = 54210245;    int tnOILpJbCy51853743 = -866548680;    int tnOILpJbCy23963133 = -840587728;    int tnOILpJbCy391604 = -118535481;    int tnOILpJbCy57901782 = -598808017;    int tnOILpJbCy65518742 = -467437450;    int tnOILpJbCy85989988 = -215601803;    int tnOILpJbCy6905196 = -357206677;    int tnOILpJbCy16862479 = -385530754;    int tnOILpJbCy39915855 = -103219854;    int tnOILpJbCy72038393 = -903079970;    int tnOILpJbCy95279705 = -379064987;    int tnOILpJbCy26727687 = -893076499;    int tnOILpJbCy58065598 = -718409382;    int tnOILpJbCy54285750 = -561735589;    int tnOILpJbCy95385272 = -626556410;    int tnOILpJbCy50722268 = -534287576;    int tnOILpJbCy77702397 = -632574535;    int tnOILpJbCy20628828 = -810816439;    int tnOILpJbCy71977104 = -950731718;    int tnOILpJbCy56326032 = -889134108;    int tnOILpJbCy64108656 = -512712578;    int tnOILpJbCy28798061 = -577794008;    int tnOILpJbCy59189195 = -944305012;    int tnOILpJbCy74937680 = 87211275;    int tnOILpJbCy3230969 = -648428194;    int tnOILpJbCy84104848 = -571636138;    int tnOILpJbCy19409682 = 2313853;    int tnOILpJbCy95630541 = -163511314;    int tnOILpJbCy40092004 = -504381675;    int tnOILpJbCy81910679 = -812603521;    int tnOILpJbCy14012556 = -640254207;    int tnOILpJbCy16034097 = -313366781;    int tnOILpJbCy50457251 = -901617319;    int tnOILpJbCy35917251 = -955807707;    int tnOILpJbCy39252021 = -970336780;    int tnOILpJbCy14099069 = -637303031;    int tnOILpJbCy18617011 = -514793561;    int tnOILpJbCy20626885 = -544596258;    int tnOILpJbCy12999148 = -877270885;    int tnOILpJbCy46750600 = -95609553;    int tnOILpJbCy25282321 = -15375010;    int tnOILpJbCy54087484 = -130054972;    int tnOILpJbCy62191017 = -380011348;    int tnOILpJbCy4135576 = -908983198;    int tnOILpJbCy75082904 = -342775275;    int tnOILpJbCy88402373 = -564347061;    int tnOILpJbCy36640372 = -256796654;    int tnOILpJbCy95166866 = -712747632;    int tnOILpJbCy17758203 = 73994199;    int tnOILpJbCy80520034 = -583839209;    int tnOILpJbCy28384439 = -113341126;    int tnOILpJbCy66065436 = -338696462;    int tnOILpJbCy81087344 = 96775979;    int tnOILpJbCy12569179 = -603658764;    int tnOILpJbCy18832084 = -433077178;    int tnOILpJbCy23055682 = -188754673;    int tnOILpJbCy64773937 = -896282716;    int tnOILpJbCy25453924 = -105746757;    int tnOILpJbCy54670814 = -950379823;    int tnOILpJbCy81413894 = -895801312;    int tnOILpJbCy66580307 = -117915656;    int tnOILpJbCy11274654 = -93695364;    int tnOILpJbCy76770475 = -881149079;    int tnOILpJbCy58005175 = -290616334;    int tnOILpJbCy58025837 = -162825764;    int tnOILpJbCy79245609 = 34301793;    int tnOILpJbCy76270435 = -991459180;    int tnOILpJbCy22148348 = -762601675;    int tnOILpJbCy15033729 = -591398810;    int tnOILpJbCy81286204 = -989253379;    int tnOILpJbCy32105257 = 80505985;    int tnOILpJbCy57075512 = 12021722;    int tnOILpJbCy7629680 = -933545554;    int tnOILpJbCy25226504 = -755122166;    int tnOILpJbCy31043711 = -773759099;    int tnOILpJbCy10021173 = -282657606;    int tnOILpJbCy66607044 = -97782660;    int tnOILpJbCy55053619 = 64678186;    int tnOILpJbCy99854776 = -570013450;    int tnOILpJbCy14828595 = 15918866;    int tnOILpJbCy47464477 = -214839485;    int tnOILpJbCy24242815 = -284938516;    int tnOILpJbCy77872339 = -137505514;    int tnOILpJbCy59571969 = -920542467;    int tnOILpJbCy53526240 = -599262395;    int tnOILpJbCy47947119 = -201557745;    int tnOILpJbCy34946752 = -310142760;    int tnOILpJbCy37888073 = -197958555;    int tnOILpJbCy17085167 = -422730529;    int tnOILpJbCy16196340 = -681582107;    int tnOILpJbCy49325131 = -741020316;    int tnOILpJbCy93163087 = -309046804;    int tnOILpJbCy65956071 = -652562210;     tnOILpJbCy13143476 = tnOILpJbCy31242303;     tnOILpJbCy31242303 = tnOILpJbCy6086837;     tnOILpJbCy6086837 = tnOILpJbCy86694263;     tnOILpJbCy86694263 = tnOILpJbCy53064448;     tnOILpJbCy53064448 = tnOILpJbCy68895211;     tnOILpJbCy68895211 = tnOILpJbCy82940740;     tnOILpJbCy82940740 = tnOILpJbCy51853743;     tnOILpJbCy51853743 = tnOILpJbCy23963133;     tnOILpJbCy23963133 = tnOILpJbCy391604;     tnOILpJbCy391604 = tnOILpJbCy57901782;     tnOILpJbCy57901782 = tnOILpJbCy65518742;     tnOILpJbCy65518742 = tnOILpJbCy85989988;     tnOILpJbCy85989988 = tnOILpJbCy6905196;     tnOILpJbCy6905196 = tnOILpJbCy16862479;     tnOILpJbCy16862479 = tnOILpJbCy39915855;     tnOILpJbCy39915855 = tnOILpJbCy72038393;     tnOILpJbCy72038393 = tnOILpJbCy95279705;     tnOILpJbCy95279705 = tnOILpJbCy26727687;     tnOILpJbCy26727687 = tnOILpJbCy58065598;     tnOILpJbCy58065598 = tnOILpJbCy54285750;     tnOILpJbCy54285750 = tnOILpJbCy95385272;     tnOILpJbCy95385272 = tnOILpJbCy50722268;     tnOILpJbCy50722268 = tnOILpJbCy77702397;     tnOILpJbCy77702397 = tnOILpJbCy20628828;     tnOILpJbCy20628828 = tnOILpJbCy71977104;     tnOILpJbCy71977104 = tnOILpJbCy56326032;     tnOILpJbCy56326032 = tnOILpJbCy64108656;     tnOILpJbCy64108656 = tnOILpJbCy28798061;     tnOILpJbCy28798061 = tnOILpJbCy59189195;     tnOILpJbCy59189195 = tnOILpJbCy74937680;     tnOILpJbCy74937680 = tnOILpJbCy3230969;     tnOILpJbCy3230969 = tnOILpJbCy84104848;     tnOILpJbCy84104848 = tnOILpJbCy19409682;     tnOILpJbCy19409682 = tnOILpJbCy95630541;     tnOILpJbCy95630541 = tnOILpJbCy40092004;     tnOILpJbCy40092004 = tnOILpJbCy81910679;     tnOILpJbCy81910679 = tnOILpJbCy14012556;     tnOILpJbCy14012556 = tnOILpJbCy16034097;     tnOILpJbCy16034097 = tnOILpJbCy50457251;     tnOILpJbCy50457251 = tnOILpJbCy35917251;     tnOILpJbCy35917251 = tnOILpJbCy39252021;     tnOILpJbCy39252021 = tnOILpJbCy14099069;     tnOILpJbCy14099069 = tnOILpJbCy18617011;     tnOILpJbCy18617011 = tnOILpJbCy20626885;     tnOILpJbCy20626885 = tnOILpJbCy12999148;     tnOILpJbCy12999148 = tnOILpJbCy46750600;     tnOILpJbCy46750600 = tnOILpJbCy25282321;     tnOILpJbCy25282321 = tnOILpJbCy54087484;     tnOILpJbCy54087484 = tnOILpJbCy62191017;     tnOILpJbCy62191017 = tnOILpJbCy4135576;     tnOILpJbCy4135576 = tnOILpJbCy75082904;     tnOILpJbCy75082904 = tnOILpJbCy88402373;     tnOILpJbCy88402373 = tnOILpJbCy36640372;     tnOILpJbCy36640372 = tnOILpJbCy95166866;     tnOILpJbCy95166866 = tnOILpJbCy17758203;     tnOILpJbCy17758203 = tnOILpJbCy80520034;     tnOILpJbCy80520034 = tnOILpJbCy28384439;     tnOILpJbCy28384439 = tnOILpJbCy66065436;     tnOILpJbCy66065436 = tnOILpJbCy81087344;     tnOILpJbCy81087344 = tnOILpJbCy12569179;     tnOILpJbCy12569179 = tnOILpJbCy18832084;     tnOILpJbCy18832084 = tnOILpJbCy23055682;     tnOILpJbCy23055682 = tnOILpJbCy64773937;     tnOILpJbCy64773937 = tnOILpJbCy25453924;     tnOILpJbCy25453924 = tnOILpJbCy54670814;     tnOILpJbCy54670814 = tnOILpJbCy81413894;     tnOILpJbCy81413894 = tnOILpJbCy66580307;     tnOILpJbCy66580307 = tnOILpJbCy11274654;     tnOILpJbCy11274654 = tnOILpJbCy76770475;     tnOILpJbCy76770475 = tnOILpJbCy58005175;     tnOILpJbCy58005175 = tnOILpJbCy58025837;     tnOILpJbCy58025837 = tnOILpJbCy79245609;     tnOILpJbCy79245609 = tnOILpJbCy76270435;     tnOILpJbCy76270435 = tnOILpJbCy22148348;     tnOILpJbCy22148348 = tnOILpJbCy15033729;     tnOILpJbCy15033729 = tnOILpJbCy81286204;     tnOILpJbCy81286204 = tnOILpJbCy32105257;     tnOILpJbCy32105257 = tnOILpJbCy57075512;     tnOILpJbCy57075512 = tnOILpJbCy7629680;     tnOILpJbCy7629680 = tnOILpJbCy25226504;     tnOILpJbCy25226504 = tnOILpJbCy31043711;     tnOILpJbCy31043711 = tnOILpJbCy10021173;     tnOILpJbCy10021173 = tnOILpJbCy66607044;     tnOILpJbCy66607044 = tnOILpJbCy55053619;     tnOILpJbCy55053619 = tnOILpJbCy99854776;     tnOILpJbCy99854776 = tnOILpJbCy14828595;     tnOILpJbCy14828595 = tnOILpJbCy47464477;     tnOILpJbCy47464477 = tnOILpJbCy24242815;     tnOILpJbCy24242815 = tnOILpJbCy77872339;     tnOILpJbCy77872339 = tnOILpJbCy59571969;     tnOILpJbCy59571969 = tnOILpJbCy53526240;     tnOILpJbCy53526240 = tnOILpJbCy47947119;     tnOILpJbCy47947119 = tnOILpJbCy34946752;     tnOILpJbCy34946752 = tnOILpJbCy37888073;     tnOILpJbCy37888073 = tnOILpJbCy17085167;     tnOILpJbCy17085167 = tnOILpJbCy16196340;     tnOILpJbCy16196340 = tnOILpJbCy49325131;     tnOILpJbCy49325131 = tnOILpJbCy93163087;     tnOILpJbCy93163087 = tnOILpJbCy65956071;     tnOILpJbCy65956071 = tnOILpJbCy13143476;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nxqELUoHZT67665958() {     int vYdtKmmxWk96587781 = -433227891;    int vYdtKmmxWk48114392 = -117508972;    int vYdtKmmxWk2495343 = -639440888;    int vYdtKmmxWk49486515 = -343312736;    int vYdtKmmxWk59185772 = -760823229;    int vYdtKmmxWk67399611 = -271864525;    int vYdtKmmxWk53980058 = -785701069;    int vYdtKmmxWk66201185 = -853706710;    int vYdtKmmxWk82492833 = -82047078;    int vYdtKmmxWk32937256 = -586911593;    int vYdtKmmxWk7317720 = -788923889;    int vYdtKmmxWk28724142 = -455055697;    int vYdtKmmxWk4107765 = -7263044;    int vYdtKmmxWk86532087 = -145497034;    int vYdtKmmxWk77716942 = -479553737;    int vYdtKmmxWk26305017 = -991299180;    int vYdtKmmxWk65790277 = -656424997;    int vYdtKmmxWk76131152 = -626369864;    int vYdtKmmxWk89167829 = -891751369;    int vYdtKmmxWk47433040 = -147995374;    int vYdtKmmxWk9926137 = -151264620;    int vYdtKmmxWk7532591 = -617708180;    int vYdtKmmxWk54547976 = -574176773;    int vYdtKmmxWk12408955 = -704399931;    int vYdtKmmxWk62204785 = -270469197;    int vYdtKmmxWk67319253 = -831241434;    int vYdtKmmxWk5780773 = -517218451;    int vYdtKmmxWk3840282 = -582667461;    int vYdtKmmxWk64879155 = -440299150;    int vYdtKmmxWk75022536 = -357154864;    int vYdtKmmxWk93032993 = -650545925;    int vYdtKmmxWk66337395 = -563623274;    int vYdtKmmxWk27643736 = -192508316;    int vYdtKmmxWk61568192 = -689288317;    int vYdtKmmxWk74357272 = -361176049;    int vYdtKmmxWk52481188 = -292405581;    int vYdtKmmxWk50524858 = -691452700;    int vYdtKmmxWk54591388 = -348724727;    int vYdtKmmxWk65029175 = -498264578;    int vYdtKmmxWk99952386 = -654210841;    int vYdtKmmxWk69566175 = -527356680;    int vYdtKmmxWk89739807 = -444245101;    int vYdtKmmxWk37666138 = -88360274;    int vYdtKmmxWk10521952 = -270976332;    int vYdtKmmxWk12571382 = -272753211;    int vYdtKmmxWk52605144 = -896881809;    int vYdtKmmxWk22766030 = -227913782;    int vYdtKmmxWk44952917 = -14096728;    int vYdtKmmxWk84934596 = -715657054;    int vYdtKmmxWk97523501 = -587031803;    int vYdtKmmxWk77393024 = -29884620;    int vYdtKmmxWk30583698 = -401034586;    int vYdtKmmxWk2514833 = -776094378;    int vYdtKmmxWk61317131 = 24789959;    int vYdtKmmxWk57739821 = 38766245;    int vYdtKmmxWk89055190 = -815519711;    int vYdtKmmxWk93566415 = -543332199;    int vYdtKmmxWk90086387 = -935040958;    int vYdtKmmxWk87281729 = 27156461;    int vYdtKmmxWk91866519 = -929581796;    int vYdtKmmxWk61618839 = -754646075;    int vYdtKmmxWk50139776 = -103033608;    int vYdtKmmxWk1322031 = -313407560;    int vYdtKmmxWk7470297 = -724892215;    int vYdtKmmxWk39904262 = -936365668;    int vYdtKmmxWk40980325 = -125300616;    int vYdtKmmxWk1080407 = -162547381;    int vYdtKmmxWk42539573 = -317974727;    int vYdtKmmxWk12174816 = -784320986;    int vYdtKmmxWk25235755 = -87148156;    int vYdtKmmxWk75780158 = -199846480;    int vYdtKmmxWk11198890 = -207700271;    int vYdtKmmxWk11101978 = -28105286;    int vYdtKmmxWk89215442 = -137540529;    int vYdtKmmxWk77866865 = -620638694;    int vYdtKmmxWk20186330 = -707019520;    int vYdtKmmxWk69866453 = -429347906;    int vYdtKmmxWk44026025 = -203200442;    int vYdtKmmxWk99837573 = -331646721;    int vYdtKmmxWk9599642 = -373587389;    int vYdtKmmxWk44553223 = -503327652;    int vYdtKmmxWk60827856 = -403121723;    int vYdtKmmxWk18905686 = -867010408;    int vYdtKmmxWk67355654 = -853267347;    int vYdtKmmxWk97629512 = -227270244;    int vYdtKmmxWk62449295 = -149511340;    int vYdtKmmxWk63822563 = -787528897;    int vYdtKmmxWk66326604 = -117298275;    int vYdtKmmxWk3828371 = -628054563;    int vYdtKmmxWk85302082 = -545656338;    int vYdtKmmxWk58914772 = -749073383;    int vYdtKmmxWk60438470 = -756411743;    int vYdtKmmxWk67309658 = -275881188;    int vYdtKmmxWk73162655 = -568682783;    int vYdtKmmxWk38333547 = -899564767;    int vYdtKmmxWk19426399 = -324323073;    int vYdtKmmxWk88417777 = -30837542;    int vYdtKmmxWk30195841 = -363468059;    int vYdtKmmxWk70617689 = -334610664;    int vYdtKmmxWk71591057 = -433227891;     vYdtKmmxWk96587781 = vYdtKmmxWk48114392;     vYdtKmmxWk48114392 = vYdtKmmxWk2495343;     vYdtKmmxWk2495343 = vYdtKmmxWk49486515;     vYdtKmmxWk49486515 = vYdtKmmxWk59185772;     vYdtKmmxWk59185772 = vYdtKmmxWk67399611;     vYdtKmmxWk67399611 = vYdtKmmxWk53980058;     vYdtKmmxWk53980058 = vYdtKmmxWk66201185;     vYdtKmmxWk66201185 = vYdtKmmxWk82492833;     vYdtKmmxWk82492833 = vYdtKmmxWk32937256;     vYdtKmmxWk32937256 = vYdtKmmxWk7317720;     vYdtKmmxWk7317720 = vYdtKmmxWk28724142;     vYdtKmmxWk28724142 = vYdtKmmxWk4107765;     vYdtKmmxWk4107765 = vYdtKmmxWk86532087;     vYdtKmmxWk86532087 = vYdtKmmxWk77716942;     vYdtKmmxWk77716942 = vYdtKmmxWk26305017;     vYdtKmmxWk26305017 = vYdtKmmxWk65790277;     vYdtKmmxWk65790277 = vYdtKmmxWk76131152;     vYdtKmmxWk76131152 = vYdtKmmxWk89167829;     vYdtKmmxWk89167829 = vYdtKmmxWk47433040;     vYdtKmmxWk47433040 = vYdtKmmxWk9926137;     vYdtKmmxWk9926137 = vYdtKmmxWk7532591;     vYdtKmmxWk7532591 = vYdtKmmxWk54547976;     vYdtKmmxWk54547976 = vYdtKmmxWk12408955;     vYdtKmmxWk12408955 = vYdtKmmxWk62204785;     vYdtKmmxWk62204785 = vYdtKmmxWk67319253;     vYdtKmmxWk67319253 = vYdtKmmxWk5780773;     vYdtKmmxWk5780773 = vYdtKmmxWk3840282;     vYdtKmmxWk3840282 = vYdtKmmxWk64879155;     vYdtKmmxWk64879155 = vYdtKmmxWk75022536;     vYdtKmmxWk75022536 = vYdtKmmxWk93032993;     vYdtKmmxWk93032993 = vYdtKmmxWk66337395;     vYdtKmmxWk66337395 = vYdtKmmxWk27643736;     vYdtKmmxWk27643736 = vYdtKmmxWk61568192;     vYdtKmmxWk61568192 = vYdtKmmxWk74357272;     vYdtKmmxWk74357272 = vYdtKmmxWk52481188;     vYdtKmmxWk52481188 = vYdtKmmxWk50524858;     vYdtKmmxWk50524858 = vYdtKmmxWk54591388;     vYdtKmmxWk54591388 = vYdtKmmxWk65029175;     vYdtKmmxWk65029175 = vYdtKmmxWk99952386;     vYdtKmmxWk99952386 = vYdtKmmxWk69566175;     vYdtKmmxWk69566175 = vYdtKmmxWk89739807;     vYdtKmmxWk89739807 = vYdtKmmxWk37666138;     vYdtKmmxWk37666138 = vYdtKmmxWk10521952;     vYdtKmmxWk10521952 = vYdtKmmxWk12571382;     vYdtKmmxWk12571382 = vYdtKmmxWk52605144;     vYdtKmmxWk52605144 = vYdtKmmxWk22766030;     vYdtKmmxWk22766030 = vYdtKmmxWk44952917;     vYdtKmmxWk44952917 = vYdtKmmxWk84934596;     vYdtKmmxWk84934596 = vYdtKmmxWk97523501;     vYdtKmmxWk97523501 = vYdtKmmxWk77393024;     vYdtKmmxWk77393024 = vYdtKmmxWk30583698;     vYdtKmmxWk30583698 = vYdtKmmxWk2514833;     vYdtKmmxWk2514833 = vYdtKmmxWk61317131;     vYdtKmmxWk61317131 = vYdtKmmxWk57739821;     vYdtKmmxWk57739821 = vYdtKmmxWk89055190;     vYdtKmmxWk89055190 = vYdtKmmxWk93566415;     vYdtKmmxWk93566415 = vYdtKmmxWk90086387;     vYdtKmmxWk90086387 = vYdtKmmxWk87281729;     vYdtKmmxWk87281729 = vYdtKmmxWk91866519;     vYdtKmmxWk91866519 = vYdtKmmxWk61618839;     vYdtKmmxWk61618839 = vYdtKmmxWk50139776;     vYdtKmmxWk50139776 = vYdtKmmxWk1322031;     vYdtKmmxWk1322031 = vYdtKmmxWk7470297;     vYdtKmmxWk7470297 = vYdtKmmxWk39904262;     vYdtKmmxWk39904262 = vYdtKmmxWk40980325;     vYdtKmmxWk40980325 = vYdtKmmxWk1080407;     vYdtKmmxWk1080407 = vYdtKmmxWk42539573;     vYdtKmmxWk42539573 = vYdtKmmxWk12174816;     vYdtKmmxWk12174816 = vYdtKmmxWk25235755;     vYdtKmmxWk25235755 = vYdtKmmxWk75780158;     vYdtKmmxWk75780158 = vYdtKmmxWk11198890;     vYdtKmmxWk11198890 = vYdtKmmxWk11101978;     vYdtKmmxWk11101978 = vYdtKmmxWk89215442;     vYdtKmmxWk89215442 = vYdtKmmxWk77866865;     vYdtKmmxWk77866865 = vYdtKmmxWk20186330;     vYdtKmmxWk20186330 = vYdtKmmxWk69866453;     vYdtKmmxWk69866453 = vYdtKmmxWk44026025;     vYdtKmmxWk44026025 = vYdtKmmxWk99837573;     vYdtKmmxWk99837573 = vYdtKmmxWk9599642;     vYdtKmmxWk9599642 = vYdtKmmxWk44553223;     vYdtKmmxWk44553223 = vYdtKmmxWk60827856;     vYdtKmmxWk60827856 = vYdtKmmxWk18905686;     vYdtKmmxWk18905686 = vYdtKmmxWk67355654;     vYdtKmmxWk67355654 = vYdtKmmxWk97629512;     vYdtKmmxWk97629512 = vYdtKmmxWk62449295;     vYdtKmmxWk62449295 = vYdtKmmxWk63822563;     vYdtKmmxWk63822563 = vYdtKmmxWk66326604;     vYdtKmmxWk66326604 = vYdtKmmxWk3828371;     vYdtKmmxWk3828371 = vYdtKmmxWk85302082;     vYdtKmmxWk85302082 = vYdtKmmxWk58914772;     vYdtKmmxWk58914772 = vYdtKmmxWk60438470;     vYdtKmmxWk60438470 = vYdtKmmxWk67309658;     vYdtKmmxWk67309658 = vYdtKmmxWk73162655;     vYdtKmmxWk73162655 = vYdtKmmxWk38333547;     vYdtKmmxWk38333547 = vYdtKmmxWk19426399;     vYdtKmmxWk19426399 = vYdtKmmxWk88417777;     vYdtKmmxWk88417777 = vYdtKmmxWk30195841;     vYdtKmmxWk30195841 = vYdtKmmxWk70617689;     vYdtKmmxWk70617689 = vYdtKmmxWk71591057;     vYdtKmmxWk71591057 = vYdtKmmxWk96587781;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UAhomDlZol38121831() {     int ZYuKiUQcyU9373949 = -568268582;    int ZYuKiUQcyU34885280 = -287842985;    int ZYuKiUQcyU20248143 = 52199730;    int ZYuKiUQcyU57427341 = -167515013;    int ZYuKiUQcyU52696876 = -152639694;    int ZYuKiUQcyU10709683 = -48787838;    int ZYuKiUQcyU75187816 = -231373139;    int ZYuKiUQcyU92920298 = -198224966;    int ZYuKiUQcyU77897643 = -43805171;    int ZYuKiUQcyU39155960 = -390999226;    int ZYuKiUQcyU34805102 = -903609905;    int ZYuKiUQcyU60674647 = -962388513;    int ZYuKiUQcyU74251067 = 88954299;    int ZYuKiUQcyU36156166 = -477912233;    int ZYuKiUQcyU1023496 = -386539567;    int ZYuKiUQcyU54730085 = -442190569;    int ZYuKiUQcyU21542096 = -320199464;    int ZYuKiUQcyU59689637 = -181580166;    int ZYuKiUQcyU90328459 = -801220565;    int ZYuKiUQcyU50087537 = -747239513;    int ZYuKiUQcyU90100194 = -642580884;    int ZYuKiUQcyU36580410 = -787566528;    int ZYuKiUQcyU93308271 = -505180668;    int ZYuKiUQcyU56916085 = 35255056;    int ZYuKiUQcyU68099044 = -480911122;    int ZYuKiUQcyU48637898 = -521672633;    int ZYuKiUQcyU17196309 = -50924731;    int ZYuKiUQcyU71193207 = -756465566;    int ZYuKiUQcyU81496208 = -654516390;    int ZYuKiUQcyU27773652 = 12958982;    int ZYuKiUQcyU37468990 = -926962675;    int ZYuKiUQcyU69656219 = -542342953;    int ZYuKiUQcyU73689684 = -68363100;    int ZYuKiUQcyU38644565 = -594042535;    int ZYuKiUQcyU57417804 = -971207929;    int ZYuKiUQcyU47705303 = -80968686;    int ZYuKiUQcyU87288918 = -481362051;    int ZYuKiUQcyU12295411 = -268344819;    int ZYuKiUQcyU70804561 = -340115155;    int ZYuKiUQcyU26217116 = -498627687;    int ZYuKiUQcyU30997483 = -902830674;    int ZYuKiUQcyU97442263 = -342556435;    int ZYuKiUQcyU72549608 = -111793719;    int ZYuKiUQcyU55150489 = -612148575;    int ZYuKiUQcyU92579871 = -165523171;    int ZYuKiUQcyU26233273 = -970200249;    int ZYuKiUQcyU348050 = -725428835;    int ZYuKiUQcyU74392883 = -169449508;    int ZYuKiUQcyU28998646 = -879259029;    int ZYuKiUQcyU44473922 = -98068266;    int ZYuKiUQcyU95720517 = -282768929;    int ZYuKiUQcyU58760245 = -872219924;    int ZYuKiUQcyU36999983 = -952097660;    int ZYuKiUQcyU6016857 = -664661783;    int ZYuKiUQcyU48021637 = 44321759;    int ZYuKiUQcyU72793538 = -780702055;    int ZYuKiUQcyU41577009 = -782662318;    int ZYuKiUQcyU63332058 = -983055327;    int ZYuKiUQcyU89328296 = -686603892;    int ZYuKiUQcyU4058979 = -630967062;    int ZYuKiUQcyU93513373 = -997863108;    int ZYuKiUQcyU3994610 = -474907574;    int ZYuKiUQcyU11424090 = -543708577;    int ZYuKiUQcyU50123992 = 43235847;    int ZYuKiUQcyU1686971 = -464036551;    int ZYuKiUQcyU65148883 = -261266953;    int ZYuKiUQcyU86984962 = -794025413;    int ZYuKiUQcyU35606503 = -317003166;    int ZYuKiUQcyU78738362 = -506704304;    int ZYuKiUQcyU53318193 = -205570882;    int ZYuKiUQcyU67441167 = -960828518;    int ZYuKiUQcyU9246685 = 48145354;    int ZYuKiUQcyU88885076 = -841465011;    int ZYuKiUQcyU64111343 = -202592879;    int ZYuKiUQcyU19090054 = -844408840;    int ZYuKiUQcyU92657930 = -200024449;    int ZYuKiUQcyU64030802 = -575772809;    int ZYuKiUQcyU38157782 = -893032094;    int ZYuKiUQcyU64336213 = -799221773;    int ZYuKiUQcyU41865772 = -510710874;    int ZYuKiUQcyU48289848 = -796243799;    int ZYuKiUQcyU42803426 = -881475223;    int ZYuKiUQcyU42194561 = -877206538;    int ZYuKiUQcyU37022287 = -456448124;    int ZYuKiUQcyU32053134 = -704272090;    int ZYuKiUQcyU78708744 = 45257248;    int ZYuKiUQcyU32656236 = -590245293;    int ZYuKiUQcyU67672828 = -403701318;    int ZYuKiUQcyU90622927 = -538364294;    int ZYuKiUQcyU84624266 = -90505875;    int ZYuKiUQcyU6128295 = -298306368;    int ZYuKiUQcyU23956861 = -498306724;    int ZYuKiUQcyU22967115 = -581740928;    int ZYuKiUQcyU66745582 = -709148094;    int ZYuKiUQcyU32703743 = -500764579;    int ZYuKiUQcyU27002874 = -327923101;    int ZYuKiUQcyU86018173 = -798847859;    int ZYuKiUQcyU22425616 = -55029567;    int ZYuKiUQcyU53463519 = -48112024;    int ZYuKiUQcyU27430989 = -568268582;     ZYuKiUQcyU9373949 = ZYuKiUQcyU34885280;     ZYuKiUQcyU34885280 = ZYuKiUQcyU20248143;     ZYuKiUQcyU20248143 = ZYuKiUQcyU57427341;     ZYuKiUQcyU57427341 = ZYuKiUQcyU52696876;     ZYuKiUQcyU52696876 = ZYuKiUQcyU10709683;     ZYuKiUQcyU10709683 = ZYuKiUQcyU75187816;     ZYuKiUQcyU75187816 = ZYuKiUQcyU92920298;     ZYuKiUQcyU92920298 = ZYuKiUQcyU77897643;     ZYuKiUQcyU77897643 = ZYuKiUQcyU39155960;     ZYuKiUQcyU39155960 = ZYuKiUQcyU34805102;     ZYuKiUQcyU34805102 = ZYuKiUQcyU60674647;     ZYuKiUQcyU60674647 = ZYuKiUQcyU74251067;     ZYuKiUQcyU74251067 = ZYuKiUQcyU36156166;     ZYuKiUQcyU36156166 = ZYuKiUQcyU1023496;     ZYuKiUQcyU1023496 = ZYuKiUQcyU54730085;     ZYuKiUQcyU54730085 = ZYuKiUQcyU21542096;     ZYuKiUQcyU21542096 = ZYuKiUQcyU59689637;     ZYuKiUQcyU59689637 = ZYuKiUQcyU90328459;     ZYuKiUQcyU90328459 = ZYuKiUQcyU50087537;     ZYuKiUQcyU50087537 = ZYuKiUQcyU90100194;     ZYuKiUQcyU90100194 = ZYuKiUQcyU36580410;     ZYuKiUQcyU36580410 = ZYuKiUQcyU93308271;     ZYuKiUQcyU93308271 = ZYuKiUQcyU56916085;     ZYuKiUQcyU56916085 = ZYuKiUQcyU68099044;     ZYuKiUQcyU68099044 = ZYuKiUQcyU48637898;     ZYuKiUQcyU48637898 = ZYuKiUQcyU17196309;     ZYuKiUQcyU17196309 = ZYuKiUQcyU71193207;     ZYuKiUQcyU71193207 = ZYuKiUQcyU81496208;     ZYuKiUQcyU81496208 = ZYuKiUQcyU27773652;     ZYuKiUQcyU27773652 = ZYuKiUQcyU37468990;     ZYuKiUQcyU37468990 = ZYuKiUQcyU69656219;     ZYuKiUQcyU69656219 = ZYuKiUQcyU73689684;     ZYuKiUQcyU73689684 = ZYuKiUQcyU38644565;     ZYuKiUQcyU38644565 = ZYuKiUQcyU57417804;     ZYuKiUQcyU57417804 = ZYuKiUQcyU47705303;     ZYuKiUQcyU47705303 = ZYuKiUQcyU87288918;     ZYuKiUQcyU87288918 = ZYuKiUQcyU12295411;     ZYuKiUQcyU12295411 = ZYuKiUQcyU70804561;     ZYuKiUQcyU70804561 = ZYuKiUQcyU26217116;     ZYuKiUQcyU26217116 = ZYuKiUQcyU30997483;     ZYuKiUQcyU30997483 = ZYuKiUQcyU97442263;     ZYuKiUQcyU97442263 = ZYuKiUQcyU72549608;     ZYuKiUQcyU72549608 = ZYuKiUQcyU55150489;     ZYuKiUQcyU55150489 = ZYuKiUQcyU92579871;     ZYuKiUQcyU92579871 = ZYuKiUQcyU26233273;     ZYuKiUQcyU26233273 = ZYuKiUQcyU348050;     ZYuKiUQcyU348050 = ZYuKiUQcyU74392883;     ZYuKiUQcyU74392883 = ZYuKiUQcyU28998646;     ZYuKiUQcyU28998646 = ZYuKiUQcyU44473922;     ZYuKiUQcyU44473922 = ZYuKiUQcyU95720517;     ZYuKiUQcyU95720517 = ZYuKiUQcyU58760245;     ZYuKiUQcyU58760245 = ZYuKiUQcyU36999983;     ZYuKiUQcyU36999983 = ZYuKiUQcyU6016857;     ZYuKiUQcyU6016857 = ZYuKiUQcyU48021637;     ZYuKiUQcyU48021637 = ZYuKiUQcyU72793538;     ZYuKiUQcyU72793538 = ZYuKiUQcyU41577009;     ZYuKiUQcyU41577009 = ZYuKiUQcyU63332058;     ZYuKiUQcyU63332058 = ZYuKiUQcyU89328296;     ZYuKiUQcyU89328296 = ZYuKiUQcyU4058979;     ZYuKiUQcyU4058979 = ZYuKiUQcyU93513373;     ZYuKiUQcyU93513373 = ZYuKiUQcyU3994610;     ZYuKiUQcyU3994610 = ZYuKiUQcyU11424090;     ZYuKiUQcyU11424090 = ZYuKiUQcyU50123992;     ZYuKiUQcyU50123992 = ZYuKiUQcyU1686971;     ZYuKiUQcyU1686971 = ZYuKiUQcyU65148883;     ZYuKiUQcyU65148883 = ZYuKiUQcyU86984962;     ZYuKiUQcyU86984962 = ZYuKiUQcyU35606503;     ZYuKiUQcyU35606503 = ZYuKiUQcyU78738362;     ZYuKiUQcyU78738362 = ZYuKiUQcyU53318193;     ZYuKiUQcyU53318193 = ZYuKiUQcyU67441167;     ZYuKiUQcyU67441167 = ZYuKiUQcyU9246685;     ZYuKiUQcyU9246685 = ZYuKiUQcyU88885076;     ZYuKiUQcyU88885076 = ZYuKiUQcyU64111343;     ZYuKiUQcyU64111343 = ZYuKiUQcyU19090054;     ZYuKiUQcyU19090054 = ZYuKiUQcyU92657930;     ZYuKiUQcyU92657930 = ZYuKiUQcyU64030802;     ZYuKiUQcyU64030802 = ZYuKiUQcyU38157782;     ZYuKiUQcyU38157782 = ZYuKiUQcyU64336213;     ZYuKiUQcyU64336213 = ZYuKiUQcyU41865772;     ZYuKiUQcyU41865772 = ZYuKiUQcyU48289848;     ZYuKiUQcyU48289848 = ZYuKiUQcyU42803426;     ZYuKiUQcyU42803426 = ZYuKiUQcyU42194561;     ZYuKiUQcyU42194561 = ZYuKiUQcyU37022287;     ZYuKiUQcyU37022287 = ZYuKiUQcyU32053134;     ZYuKiUQcyU32053134 = ZYuKiUQcyU78708744;     ZYuKiUQcyU78708744 = ZYuKiUQcyU32656236;     ZYuKiUQcyU32656236 = ZYuKiUQcyU67672828;     ZYuKiUQcyU67672828 = ZYuKiUQcyU90622927;     ZYuKiUQcyU90622927 = ZYuKiUQcyU84624266;     ZYuKiUQcyU84624266 = ZYuKiUQcyU6128295;     ZYuKiUQcyU6128295 = ZYuKiUQcyU23956861;     ZYuKiUQcyU23956861 = ZYuKiUQcyU22967115;     ZYuKiUQcyU22967115 = ZYuKiUQcyU66745582;     ZYuKiUQcyU66745582 = ZYuKiUQcyU32703743;     ZYuKiUQcyU32703743 = ZYuKiUQcyU27002874;     ZYuKiUQcyU27002874 = ZYuKiUQcyU86018173;     ZYuKiUQcyU86018173 = ZYuKiUQcyU22425616;     ZYuKiUQcyU22425616 = ZYuKiUQcyU53463519;     ZYuKiUQcyU53463519 = ZYuKiUQcyU27430989;     ZYuKiUQcyU27430989 = ZYuKiUQcyU9373949;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ziGkxiopPU56335173() {     int GbetFjjefA92818253 = -348934263;    int GbetFjjefA51757369 = -287225173;    int GbetFjjefA16656650 = -841325498;    int GbetFjjefA20219593 = -361314849;    int GbetFjjefA58818200 = 40492815;    int GbetFjjefA9214083 = -927859491;    int GbetFjjefA46227134 = 28715547;    int GbetFjjefA7267741 = -185382996;    int GbetFjjefA36427344 = -385264522;    int GbetFjjefA71701612 = -859375338;    int GbetFjjefA84221040 = 6274222;    int GbetFjjefA23880046 = -950006759;    int GbetFjjefA92368843 = -802706942;    int GbetFjjefA15783058 = -266202589;    int GbetFjjefA61877959 = -480562549;    int GbetFjjefA41119247 = -230269894;    int GbetFjjefA15293981 = -73544491;    int GbetFjjefA40541084 = -428885042;    int GbetFjjefA52768602 = -799895436;    int GbetFjjefA39454979 = -176825506;    int GbetFjjefA45740581 = -232109915;    int GbetFjjefA48727728 = -778718298;    int GbetFjjefA97133978 = -545069865;    int GbetFjjefA91622642 = -36570340;    int GbetFjjefA9675003 = 59436119;    int GbetFjjefA43980047 = -402182348;    int GbetFjjefA66651048 = -779009073;    int GbetFjjefA10924833 = -826420450;    int GbetFjjefA17577303 = -517021532;    int GbetFjjefA43606993 = -499890870;    int GbetFjjefA55564304 = -564719876;    int GbetFjjefA32762646 = -457538033;    int GbetFjjefA17228572 = -789235278;    int GbetFjjefA80803075 = -185644705;    int GbetFjjefA36144535 = -68872665;    int GbetFjjefA60094487 = -968992591;    int GbetFjjefA55903097 = -360211231;    int GbetFjjefA52874243 = 23184661;    int GbetFjjefA19799639 = -525012953;    int GbetFjjefA75712251 = -251221209;    int GbetFjjefA64646407 = -474379647;    int GbetFjjefA47930049 = -916464757;    int GbetFjjefA96116677 = -662850962;    int GbetFjjefA47055429 = -368331346;    int GbetFjjefA84524368 = -993680124;    int GbetFjjefA65839269 = -989811172;    int GbetFjjefA76363480 = -857733064;    int GbetFjjefA94063478 = -168171226;    int GbetFjjefA59845758 = -364861111;    int GbetFjjefA79806405 = -305088722;    int GbetFjjefA68977965 = -503670351;    int GbetFjjefA14261040 = -930479236;    int GbetFjjefA51112442 = -63844977;    int GbetFjjefA30693616 = -383075170;    int GbetFjjefA10594592 = -304164364;    int GbetFjjefA44090525 = -570215965;    int GbetFjjefA54623390 = -742155308;    int GbetFjjefA25034007 = -704755159;    int GbetFjjefA10544590 = -320750969;    int GbetFjjefA14838154 = -557324837;    int GbetFjjefA42563035 = -48850419;    int GbetFjjefA35302302 = -144864003;    int GbetFjjefA89690438 = -668361464;    int GbetFjjefA92820351 = -885373652;    int GbetFjjefA16137309 = -194655463;    int GbetFjjefA51458394 = -536187745;    int GbetFjjefA6651475 = -60771482;    int GbetFjjefA11565769 = -517062237;    int GbetFjjefA79638523 = -97329925;    int GbetFjjefA1783472 = -511569958;    int GbetFjjefA85216149 = -870058664;    int GbetFjjefA62419737 = 3270847;    int GbetFjjefA20741445 = -903872089;    int GbetFjjefA77056350 = -448674227;    int GbetFjjefA74808572 = -702445859;    int GbetFjjefA97810531 = -315645159;    int GbetFjjefA52611051 = -15867337;    int GbetFjjefA50078550 = -76738520;    int GbetFjjefA7098275 = -42890216;    int GbetFjjefA43835734 = 49247291;    int GbetFjjefA67616567 = -544449285;    int GbetFjjefA72587570 = -510837847;    int GbetFjjefA51079074 = -361559339;    int GbetFjjefA37770897 = -111932811;    int GbetFjjefA74629027 = -996220519;    int GbetFjjefA41303264 = -634240641;    int GbetFjjefA81650204 = -293693056;    int GbetFjjefA86534956 = -306160108;    int GbetFjjefA70208483 = -881480341;    int GbetFjjefA92054009 = -498656700;    int GbetFjjefA5471098 = -126837284;    int GbetFjjefA30869091 = -655456072;    int GbetFjjefA42329653 = -656064371;    int GbetFjjefA4961486 = -967688116;    int GbetFjjefA33149217 = -102370791;    int GbetFjjefA29344106 = -229515644;    int GbetFjjefA58239611 = -148103293;    int GbetFjjefA3296327 = -777477310;    int GbetFjjefA30918121 = -73675884;    int GbetFjjefA33065975 = -348934263;     GbetFjjefA92818253 = GbetFjjefA51757369;     GbetFjjefA51757369 = GbetFjjefA16656650;     GbetFjjefA16656650 = GbetFjjefA20219593;     GbetFjjefA20219593 = GbetFjjefA58818200;     GbetFjjefA58818200 = GbetFjjefA9214083;     GbetFjjefA9214083 = GbetFjjefA46227134;     GbetFjjefA46227134 = GbetFjjefA7267741;     GbetFjjefA7267741 = GbetFjjefA36427344;     GbetFjjefA36427344 = GbetFjjefA71701612;     GbetFjjefA71701612 = GbetFjjefA84221040;     GbetFjjefA84221040 = GbetFjjefA23880046;     GbetFjjefA23880046 = GbetFjjefA92368843;     GbetFjjefA92368843 = GbetFjjefA15783058;     GbetFjjefA15783058 = GbetFjjefA61877959;     GbetFjjefA61877959 = GbetFjjefA41119247;     GbetFjjefA41119247 = GbetFjjefA15293981;     GbetFjjefA15293981 = GbetFjjefA40541084;     GbetFjjefA40541084 = GbetFjjefA52768602;     GbetFjjefA52768602 = GbetFjjefA39454979;     GbetFjjefA39454979 = GbetFjjefA45740581;     GbetFjjefA45740581 = GbetFjjefA48727728;     GbetFjjefA48727728 = GbetFjjefA97133978;     GbetFjjefA97133978 = GbetFjjefA91622642;     GbetFjjefA91622642 = GbetFjjefA9675003;     GbetFjjefA9675003 = GbetFjjefA43980047;     GbetFjjefA43980047 = GbetFjjefA66651048;     GbetFjjefA66651048 = GbetFjjefA10924833;     GbetFjjefA10924833 = GbetFjjefA17577303;     GbetFjjefA17577303 = GbetFjjefA43606993;     GbetFjjefA43606993 = GbetFjjefA55564304;     GbetFjjefA55564304 = GbetFjjefA32762646;     GbetFjjefA32762646 = GbetFjjefA17228572;     GbetFjjefA17228572 = GbetFjjefA80803075;     GbetFjjefA80803075 = GbetFjjefA36144535;     GbetFjjefA36144535 = GbetFjjefA60094487;     GbetFjjefA60094487 = GbetFjjefA55903097;     GbetFjjefA55903097 = GbetFjjefA52874243;     GbetFjjefA52874243 = GbetFjjefA19799639;     GbetFjjefA19799639 = GbetFjjefA75712251;     GbetFjjefA75712251 = GbetFjjefA64646407;     GbetFjjefA64646407 = GbetFjjefA47930049;     GbetFjjefA47930049 = GbetFjjefA96116677;     GbetFjjefA96116677 = GbetFjjefA47055429;     GbetFjjefA47055429 = GbetFjjefA84524368;     GbetFjjefA84524368 = GbetFjjefA65839269;     GbetFjjefA65839269 = GbetFjjefA76363480;     GbetFjjefA76363480 = GbetFjjefA94063478;     GbetFjjefA94063478 = GbetFjjefA59845758;     GbetFjjefA59845758 = GbetFjjefA79806405;     GbetFjjefA79806405 = GbetFjjefA68977965;     GbetFjjefA68977965 = GbetFjjefA14261040;     GbetFjjefA14261040 = GbetFjjefA51112442;     GbetFjjefA51112442 = GbetFjjefA30693616;     GbetFjjefA30693616 = GbetFjjefA10594592;     GbetFjjefA10594592 = GbetFjjefA44090525;     GbetFjjefA44090525 = GbetFjjefA54623390;     GbetFjjefA54623390 = GbetFjjefA25034007;     GbetFjjefA25034007 = GbetFjjefA10544590;     GbetFjjefA10544590 = GbetFjjefA14838154;     GbetFjjefA14838154 = GbetFjjefA42563035;     GbetFjjefA42563035 = GbetFjjefA35302302;     GbetFjjefA35302302 = GbetFjjefA89690438;     GbetFjjefA89690438 = GbetFjjefA92820351;     GbetFjjefA92820351 = GbetFjjefA16137309;     GbetFjjefA16137309 = GbetFjjefA51458394;     GbetFjjefA51458394 = GbetFjjefA6651475;     GbetFjjefA6651475 = GbetFjjefA11565769;     GbetFjjefA11565769 = GbetFjjefA79638523;     GbetFjjefA79638523 = GbetFjjefA1783472;     GbetFjjefA1783472 = GbetFjjefA85216149;     GbetFjjefA85216149 = GbetFjjefA62419737;     GbetFjjefA62419737 = GbetFjjefA20741445;     GbetFjjefA20741445 = GbetFjjefA77056350;     GbetFjjefA77056350 = GbetFjjefA74808572;     GbetFjjefA74808572 = GbetFjjefA97810531;     GbetFjjefA97810531 = GbetFjjefA52611051;     GbetFjjefA52611051 = GbetFjjefA50078550;     GbetFjjefA50078550 = GbetFjjefA7098275;     GbetFjjefA7098275 = GbetFjjefA43835734;     GbetFjjefA43835734 = GbetFjjefA67616567;     GbetFjjefA67616567 = GbetFjjefA72587570;     GbetFjjefA72587570 = GbetFjjefA51079074;     GbetFjjefA51079074 = GbetFjjefA37770897;     GbetFjjefA37770897 = GbetFjjefA74629027;     GbetFjjefA74629027 = GbetFjjefA41303264;     GbetFjjefA41303264 = GbetFjjefA81650204;     GbetFjjefA81650204 = GbetFjjefA86534956;     GbetFjjefA86534956 = GbetFjjefA70208483;     GbetFjjefA70208483 = GbetFjjefA92054009;     GbetFjjefA92054009 = GbetFjjefA5471098;     GbetFjjefA5471098 = GbetFjjefA30869091;     GbetFjjefA30869091 = GbetFjjefA42329653;     GbetFjjefA42329653 = GbetFjjefA4961486;     GbetFjjefA4961486 = GbetFjjefA33149217;     GbetFjjefA33149217 = GbetFjjefA29344106;     GbetFjjefA29344106 = GbetFjjefA58239611;     GbetFjjefA58239611 = GbetFjjefA3296327;     GbetFjjefA3296327 = GbetFjjefA30918121;     GbetFjjefA30918121 = GbetFjjefA33065975;     GbetFjjefA33065975 = GbetFjjefA92818253;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void shScEsWxCh26460849() {     int TfyMuInGHx60265290 = 51021603;    int TfyMuInGHx46953560 = -994528875;    int TfyMuInGHx93828884 = -577940879;    int TfyMuInGHx14515499 = -371078707;    int TfyMuInGHx17940874 = -755403568;    int TfyMuInGHx31893119 = -34500829;    int TfyMuInGHx19988261 = -14312729;    int TfyMuInGHx93947906 = -326292168;    int TfyMuInGHx36866401 = -680229915;    int TfyMuInGHx63912789 = -205457370;    int TfyMuInGHx69999112 = -774296293;    int TfyMuInGHx4303588 = -808285301;    int TfyMuInGHx94476547 = -488371429;    int TfyMuInGHx4529348 = -965568314;    int TfyMuInGHx70236476 = -331957160;    int TfyMuInGHx84747304 = -78525197;    int TfyMuInGHx15024802 = -391304217;    int TfyMuInGHx68695622 = -750588190;    int TfyMuInGHx34721563 = -377193912;    int TfyMuInGHx87670268 = -509411340;    int TfyMuInGHx80419601 = -481042957;    int TfyMuInGHx50731871 = -605028871;    int TfyMuInGHx35485710 = -603859339;    int TfyMuInGHx93908032 = -215035646;    int TfyMuInGHx82879188 = -97225404;    int TfyMuInGHx49965562 = -188116404;    int TfyMuInGHx9834928 = -566759919;    int TfyMuInGHx96123232 = -399303427;    int TfyMuInGHx61413585 = -465413333;    int TfyMuInGHx14703648 = -745103619;    int TfyMuInGHx18293150 = 97084082;    int TfyMuInGHx82349222 = -455932478;    int TfyMuInGHx45477974 = 80336540;    int TfyMuInGHx52252503 = -993838000;    int TfyMuInGHx71351185 = -637453880;    int TfyMuInGHx99816954 = -925785546;    int TfyMuInGHx55430278 = 99106176;    int TfyMuInGHx53637825 = -875101772;    int TfyMuInGHx41031077 = -36130716;    int TfyMuInGHx54090483 = -144514968;    int TfyMuInGHx94181448 = -874459900;    int TfyMuInGHx6609503 = -128516096;    int TfyMuInGHx61716970 = 32340530;    int TfyMuInGHx41446468 = -626218811;    int TfyMuInGHx40498870 = -545708281;    int TfyMuInGHx54373032 = -219874556;    int TfyMuInGHx42721419 = -901024878;    int TfyMuInGHx71547173 = -792415022;    int TfyMuInGHx27594185 = -864429414;    int TfyMuInGHx51553066 = -469119592;    int TfyMuInGHx49159629 = -835215154;    int TfyMuInGHx17272480 = -434584809;    int TfyMuInGHx41877247 = -87709708;    int TfyMuInGHx22558827 = -436493546;    int TfyMuInGHx46041247 = -937618254;    int TfyMuInGHx9533420 = -343949527;    int TfyMuInGHx11467851 = -290669537;    int TfyMuInGHx99920851 = -262905234;    int TfyMuInGHx31636310 = -173853303;    int TfyMuInGHx67975311 = -467287165;    int TfyMuInGHx22058191 = -467740910;    int TfyMuInGHx23865028 = -615009302;    int TfyMuInGHx32534321 = -860878835;    int TfyMuInGHx22162754 = -935126296;    int TfyMuInGHx45619639 = -202541453;    int TfyMuInGHx87649889 = -218363816;    int TfyMuInGHx58825613 = -788621841;    int TfyMuInGHx42224044 = -494533429;    int TfyMuInGHx33178162 = -228114435;    int TfyMuInGHx70419522 = -406171614;    int TfyMuInGHx29317027 = -77631374;    int TfyMuInGHx61386977 = -516202445;    int TfyMuInGHx27664546 = -614457475;    int TfyMuInGHx80631080 = -132678945;    int TfyMuInGHx93488819 = -634951440;    int TfyMuInGHx73810098 = -252526862;    int TfyMuInGHx89014900 = -537369401;    int TfyMuInGHx94039241 = -977640528;    int TfyMuInGHx53409163 = -669327366;    int TfyMuInGHx28506157 = -877350849;    int TfyMuInGHx7244144 = -287091526;    int TfyMuInGHx38287754 = -774344898;    int TfyMuInGHx68529047 = -534874014;    int TfyMuInGHx9860520 = -996293741;    int TfyMuInGHx65544018 = -909888465;    int TfyMuInGHx1020671 = -468331109;    int TfyMuInGHx40471976 = -268222770;    int TfyMuInGHx22919147 = -483169915;    int TfyMuInGHx6211257 = 43780253;    int TfyMuInGHx61817766 = -193504353;    int TfyMuInGHx88349104 = -535116009;    int TfyMuInGHx55509427 = -637988590;    int TfyMuInGHx22001515 = -601248469;    int TfyMuInGHx73055766 = -568843551;    int TfyMuInGHx32032292 = -676774058;    int TfyMuInGHx70316421 = -159450598;    int TfyMuInGHx74075182 = -267637261;    int TfyMuInGHx39554217 = -32533175;    int TfyMuInGHx95826828 = -323677359;    int TfyMuInGHx52848981 = 51021603;     TfyMuInGHx60265290 = TfyMuInGHx46953560;     TfyMuInGHx46953560 = TfyMuInGHx93828884;     TfyMuInGHx93828884 = TfyMuInGHx14515499;     TfyMuInGHx14515499 = TfyMuInGHx17940874;     TfyMuInGHx17940874 = TfyMuInGHx31893119;     TfyMuInGHx31893119 = TfyMuInGHx19988261;     TfyMuInGHx19988261 = TfyMuInGHx93947906;     TfyMuInGHx93947906 = TfyMuInGHx36866401;     TfyMuInGHx36866401 = TfyMuInGHx63912789;     TfyMuInGHx63912789 = TfyMuInGHx69999112;     TfyMuInGHx69999112 = TfyMuInGHx4303588;     TfyMuInGHx4303588 = TfyMuInGHx94476547;     TfyMuInGHx94476547 = TfyMuInGHx4529348;     TfyMuInGHx4529348 = TfyMuInGHx70236476;     TfyMuInGHx70236476 = TfyMuInGHx84747304;     TfyMuInGHx84747304 = TfyMuInGHx15024802;     TfyMuInGHx15024802 = TfyMuInGHx68695622;     TfyMuInGHx68695622 = TfyMuInGHx34721563;     TfyMuInGHx34721563 = TfyMuInGHx87670268;     TfyMuInGHx87670268 = TfyMuInGHx80419601;     TfyMuInGHx80419601 = TfyMuInGHx50731871;     TfyMuInGHx50731871 = TfyMuInGHx35485710;     TfyMuInGHx35485710 = TfyMuInGHx93908032;     TfyMuInGHx93908032 = TfyMuInGHx82879188;     TfyMuInGHx82879188 = TfyMuInGHx49965562;     TfyMuInGHx49965562 = TfyMuInGHx9834928;     TfyMuInGHx9834928 = TfyMuInGHx96123232;     TfyMuInGHx96123232 = TfyMuInGHx61413585;     TfyMuInGHx61413585 = TfyMuInGHx14703648;     TfyMuInGHx14703648 = TfyMuInGHx18293150;     TfyMuInGHx18293150 = TfyMuInGHx82349222;     TfyMuInGHx82349222 = TfyMuInGHx45477974;     TfyMuInGHx45477974 = TfyMuInGHx52252503;     TfyMuInGHx52252503 = TfyMuInGHx71351185;     TfyMuInGHx71351185 = TfyMuInGHx99816954;     TfyMuInGHx99816954 = TfyMuInGHx55430278;     TfyMuInGHx55430278 = TfyMuInGHx53637825;     TfyMuInGHx53637825 = TfyMuInGHx41031077;     TfyMuInGHx41031077 = TfyMuInGHx54090483;     TfyMuInGHx54090483 = TfyMuInGHx94181448;     TfyMuInGHx94181448 = TfyMuInGHx6609503;     TfyMuInGHx6609503 = TfyMuInGHx61716970;     TfyMuInGHx61716970 = TfyMuInGHx41446468;     TfyMuInGHx41446468 = TfyMuInGHx40498870;     TfyMuInGHx40498870 = TfyMuInGHx54373032;     TfyMuInGHx54373032 = TfyMuInGHx42721419;     TfyMuInGHx42721419 = TfyMuInGHx71547173;     TfyMuInGHx71547173 = TfyMuInGHx27594185;     TfyMuInGHx27594185 = TfyMuInGHx51553066;     TfyMuInGHx51553066 = TfyMuInGHx49159629;     TfyMuInGHx49159629 = TfyMuInGHx17272480;     TfyMuInGHx17272480 = TfyMuInGHx41877247;     TfyMuInGHx41877247 = TfyMuInGHx22558827;     TfyMuInGHx22558827 = TfyMuInGHx46041247;     TfyMuInGHx46041247 = TfyMuInGHx9533420;     TfyMuInGHx9533420 = TfyMuInGHx11467851;     TfyMuInGHx11467851 = TfyMuInGHx99920851;     TfyMuInGHx99920851 = TfyMuInGHx31636310;     TfyMuInGHx31636310 = TfyMuInGHx67975311;     TfyMuInGHx67975311 = TfyMuInGHx22058191;     TfyMuInGHx22058191 = TfyMuInGHx23865028;     TfyMuInGHx23865028 = TfyMuInGHx32534321;     TfyMuInGHx32534321 = TfyMuInGHx22162754;     TfyMuInGHx22162754 = TfyMuInGHx45619639;     TfyMuInGHx45619639 = TfyMuInGHx87649889;     TfyMuInGHx87649889 = TfyMuInGHx58825613;     TfyMuInGHx58825613 = TfyMuInGHx42224044;     TfyMuInGHx42224044 = TfyMuInGHx33178162;     TfyMuInGHx33178162 = TfyMuInGHx70419522;     TfyMuInGHx70419522 = TfyMuInGHx29317027;     TfyMuInGHx29317027 = TfyMuInGHx61386977;     TfyMuInGHx61386977 = TfyMuInGHx27664546;     TfyMuInGHx27664546 = TfyMuInGHx80631080;     TfyMuInGHx80631080 = TfyMuInGHx93488819;     TfyMuInGHx93488819 = TfyMuInGHx73810098;     TfyMuInGHx73810098 = TfyMuInGHx89014900;     TfyMuInGHx89014900 = TfyMuInGHx94039241;     TfyMuInGHx94039241 = TfyMuInGHx53409163;     TfyMuInGHx53409163 = TfyMuInGHx28506157;     TfyMuInGHx28506157 = TfyMuInGHx7244144;     TfyMuInGHx7244144 = TfyMuInGHx38287754;     TfyMuInGHx38287754 = TfyMuInGHx68529047;     TfyMuInGHx68529047 = TfyMuInGHx9860520;     TfyMuInGHx9860520 = TfyMuInGHx65544018;     TfyMuInGHx65544018 = TfyMuInGHx1020671;     TfyMuInGHx1020671 = TfyMuInGHx40471976;     TfyMuInGHx40471976 = TfyMuInGHx22919147;     TfyMuInGHx22919147 = TfyMuInGHx6211257;     TfyMuInGHx6211257 = TfyMuInGHx61817766;     TfyMuInGHx61817766 = TfyMuInGHx88349104;     TfyMuInGHx88349104 = TfyMuInGHx55509427;     TfyMuInGHx55509427 = TfyMuInGHx22001515;     TfyMuInGHx22001515 = TfyMuInGHx73055766;     TfyMuInGHx73055766 = TfyMuInGHx32032292;     TfyMuInGHx32032292 = TfyMuInGHx70316421;     TfyMuInGHx70316421 = TfyMuInGHx74075182;     TfyMuInGHx74075182 = TfyMuInGHx39554217;     TfyMuInGHx39554217 = TfyMuInGHx95826828;     TfyMuInGHx95826828 = TfyMuInGHx52848981;     TfyMuInGHx52848981 = TfyMuInGHx60265290;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ncXGotRVHb96916721() {     int OoMkHToWuG73051457 = -84019089;    int OoMkHToWuG33724448 = -64862889;    int OoMkHToWuG11581684 = -986300261;    int OoMkHToWuG22456324 = -195280983;    int OoMkHToWuG11451978 = -147220033;    int OoMkHToWuG75203189 = -911424143;    int OoMkHToWuG41196019 = -559984799;    int OoMkHToWuG20667019 = -770810425;    int OoMkHToWuG32271212 = -641988007;    int OoMkHToWuG70131494 = -9545002;    int OoMkHToWuG97486494 = -888982308;    int OoMkHToWuG36254092 = -215618117;    int OoMkHToWuG64619850 = -392154086;    int OoMkHToWuG54153426 = -197983513;    int OoMkHToWuG93543029 = -238942991;    int OoMkHToWuG13172373 = -629416586;    int OoMkHToWuG70776619 = -55078684;    int OoMkHToWuG52254108 = -305798492;    int OoMkHToWuG35882194 = -286663107;    int OoMkHToWuG90324764 = -8655479;    int OoMkHToWuG60593658 = -972359221;    int OoMkHToWuG79779689 = -774887219;    int OoMkHToWuG74246005 = -534863233;    int OoMkHToWuG38415163 = -575380658;    int OoMkHToWuG88773447 = -307667328;    int OoMkHToWuG31284208 = -978547603;    int OoMkHToWuG21250464 = -100466200;    int OoMkHToWuG63476157 = -573101531;    int OoMkHToWuG78030638 = -679630573;    int OoMkHToWuG67454763 = -374989773;    int OoMkHToWuG62729145 = -179332668;    int OoMkHToWuG85668046 = -434652156;    int OoMkHToWuG91523923 = -895518245;    int OoMkHToWuG29328876 = -898592219;    int OoMkHToWuG54411717 = -147485760;    int OoMkHToWuG95041069 = -714348650;    int OoMkHToWuG92194338 = -790803174;    int OoMkHToWuG11341848 = -794721864;    int OoMkHToWuG46806463 = -977981293;    int OoMkHToWuG80355212 = 11068187;    int OoMkHToWuG55612757 = -149933893;    int OoMkHToWuG14311959 = -26827430;    int OoMkHToWuG96600440 = 8907085;    int OoMkHToWuG86075005 = -967391054;    int OoMkHToWuG20507361 = -438478241;    int OoMkHToWuG28001161 = -293192995;    int OoMkHToWuG20303439 = -298539930;    int OoMkHToWuG987140 = -947767802;    int OoMkHToWuG71658234 = 71968612;    int OoMkHToWuG98503486 = 19843945;    int OoMkHToWuG67487122 = 11900538;    int OoMkHToWuG45449027 = -905770147;    int OoMkHToWuG76362396 = -263712991;    int OoMkHToWuG67258552 = -25945288;    int OoMkHToWuG36323063 = -932062740;    int OoMkHToWuG93271767 = -309131871;    int OoMkHToWuG59478443 = -529999656;    int OoMkHToWuG73166521 = -310919603;    int OoMkHToWuG33682876 = -887613656;    int OoMkHToWuG80167770 = -168672431;    int OoMkHToWuG53952725 = -710957943;    int OoMkHToWuG77719861 = -986883268;    int OoMkHToWuG42636380 = 8820148;    int OoMkHToWuG64816449 = -166998234;    int OoMkHToWuG7402349 = -830212335;    int OoMkHToWuG11818448 = -354330153;    int OoMkHToWuG44730169 = -320099873;    int OoMkHToWuG35290974 = -493561868;    int OoMkHToWuG99741708 = 49502247;    int OoMkHToWuG98501960 = -524594341;    int OoMkHToWuG20978035 = -838613413;    int OoMkHToWuG59434772 = -260356820;    int OoMkHToWuG5447645 = -327817200;    int OoMkHToWuG55526981 = -197731294;    int OoMkHToWuG34712008 = -858721586;    int OoMkHToWuG46281700 = -845531792;    int OoMkHToWuG83179249 = -683794304;    int OoMkHToWuG88170999 = -567472180;    int OoMkHToWuG17907802 = -36902418;    int OoMkHToWuG60772286 = 85525667;    int OoMkHToWuG10980770 = -580007673;    int OoMkHToWuG20263324 = -152698399;    int OoMkHToWuG91817923 = -545070144;    int OoMkHToWuG79527152 = -599474518;    int OoMkHToWuG99967640 = -286890312;    int OoMkHToWuG17280119 = -273562521;    int OoMkHToWuG9305650 = -70939166;    int OoMkHToWuG24265371 = -769572958;    int OoMkHToWuG93005813 = -966529479;    int OoMkHToWuG61139950 = -838353890;    int OoMkHToWuG35562626 = -84348995;    int OoMkHToWuG19027817 = -379883571;    int OoMkHToWuG77658971 = -907108209;    int OoMkHToWuG66638693 = -709308863;    int OoMkHToWuG26402487 = -277973871;    int OoMkHToWuG77892896 = -163050626;    int OoMkHToWuG71675578 = 64352422;    int OoMkHToWuG31783991 = -824094682;    int OoMkHToWuG78672657 = -37178719;    int OoMkHToWuG8688913 = -84019089;     OoMkHToWuG73051457 = OoMkHToWuG33724448;     OoMkHToWuG33724448 = OoMkHToWuG11581684;     OoMkHToWuG11581684 = OoMkHToWuG22456324;     OoMkHToWuG22456324 = OoMkHToWuG11451978;     OoMkHToWuG11451978 = OoMkHToWuG75203189;     OoMkHToWuG75203189 = OoMkHToWuG41196019;     OoMkHToWuG41196019 = OoMkHToWuG20667019;     OoMkHToWuG20667019 = OoMkHToWuG32271212;     OoMkHToWuG32271212 = OoMkHToWuG70131494;     OoMkHToWuG70131494 = OoMkHToWuG97486494;     OoMkHToWuG97486494 = OoMkHToWuG36254092;     OoMkHToWuG36254092 = OoMkHToWuG64619850;     OoMkHToWuG64619850 = OoMkHToWuG54153426;     OoMkHToWuG54153426 = OoMkHToWuG93543029;     OoMkHToWuG93543029 = OoMkHToWuG13172373;     OoMkHToWuG13172373 = OoMkHToWuG70776619;     OoMkHToWuG70776619 = OoMkHToWuG52254108;     OoMkHToWuG52254108 = OoMkHToWuG35882194;     OoMkHToWuG35882194 = OoMkHToWuG90324764;     OoMkHToWuG90324764 = OoMkHToWuG60593658;     OoMkHToWuG60593658 = OoMkHToWuG79779689;     OoMkHToWuG79779689 = OoMkHToWuG74246005;     OoMkHToWuG74246005 = OoMkHToWuG38415163;     OoMkHToWuG38415163 = OoMkHToWuG88773447;     OoMkHToWuG88773447 = OoMkHToWuG31284208;     OoMkHToWuG31284208 = OoMkHToWuG21250464;     OoMkHToWuG21250464 = OoMkHToWuG63476157;     OoMkHToWuG63476157 = OoMkHToWuG78030638;     OoMkHToWuG78030638 = OoMkHToWuG67454763;     OoMkHToWuG67454763 = OoMkHToWuG62729145;     OoMkHToWuG62729145 = OoMkHToWuG85668046;     OoMkHToWuG85668046 = OoMkHToWuG91523923;     OoMkHToWuG91523923 = OoMkHToWuG29328876;     OoMkHToWuG29328876 = OoMkHToWuG54411717;     OoMkHToWuG54411717 = OoMkHToWuG95041069;     OoMkHToWuG95041069 = OoMkHToWuG92194338;     OoMkHToWuG92194338 = OoMkHToWuG11341848;     OoMkHToWuG11341848 = OoMkHToWuG46806463;     OoMkHToWuG46806463 = OoMkHToWuG80355212;     OoMkHToWuG80355212 = OoMkHToWuG55612757;     OoMkHToWuG55612757 = OoMkHToWuG14311959;     OoMkHToWuG14311959 = OoMkHToWuG96600440;     OoMkHToWuG96600440 = OoMkHToWuG86075005;     OoMkHToWuG86075005 = OoMkHToWuG20507361;     OoMkHToWuG20507361 = OoMkHToWuG28001161;     OoMkHToWuG28001161 = OoMkHToWuG20303439;     OoMkHToWuG20303439 = OoMkHToWuG987140;     OoMkHToWuG987140 = OoMkHToWuG71658234;     OoMkHToWuG71658234 = OoMkHToWuG98503486;     OoMkHToWuG98503486 = OoMkHToWuG67487122;     OoMkHToWuG67487122 = OoMkHToWuG45449027;     OoMkHToWuG45449027 = OoMkHToWuG76362396;     OoMkHToWuG76362396 = OoMkHToWuG67258552;     OoMkHToWuG67258552 = OoMkHToWuG36323063;     OoMkHToWuG36323063 = OoMkHToWuG93271767;     OoMkHToWuG93271767 = OoMkHToWuG59478443;     OoMkHToWuG59478443 = OoMkHToWuG73166521;     OoMkHToWuG73166521 = OoMkHToWuG33682876;     OoMkHToWuG33682876 = OoMkHToWuG80167770;     OoMkHToWuG80167770 = OoMkHToWuG53952725;     OoMkHToWuG53952725 = OoMkHToWuG77719861;     OoMkHToWuG77719861 = OoMkHToWuG42636380;     OoMkHToWuG42636380 = OoMkHToWuG64816449;     OoMkHToWuG64816449 = OoMkHToWuG7402349;     OoMkHToWuG7402349 = OoMkHToWuG11818448;     OoMkHToWuG11818448 = OoMkHToWuG44730169;     OoMkHToWuG44730169 = OoMkHToWuG35290974;     OoMkHToWuG35290974 = OoMkHToWuG99741708;     OoMkHToWuG99741708 = OoMkHToWuG98501960;     OoMkHToWuG98501960 = OoMkHToWuG20978035;     OoMkHToWuG20978035 = OoMkHToWuG59434772;     OoMkHToWuG59434772 = OoMkHToWuG5447645;     OoMkHToWuG5447645 = OoMkHToWuG55526981;     OoMkHToWuG55526981 = OoMkHToWuG34712008;     OoMkHToWuG34712008 = OoMkHToWuG46281700;     OoMkHToWuG46281700 = OoMkHToWuG83179249;     OoMkHToWuG83179249 = OoMkHToWuG88170999;     OoMkHToWuG88170999 = OoMkHToWuG17907802;     OoMkHToWuG17907802 = OoMkHToWuG60772286;     OoMkHToWuG60772286 = OoMkHToWuG10980770;     OoMkHToWuG10980770 = OoMkHToWuG20263324;     OoMkHToWuG20263324 = OoMkHToWuG91817923;     OoMkHToWuG91817923 = OoMkHToWuG79527152;     OoMkHToWuG79527152 = OoMkHToWuG99967640;     OoMkHToWuG99967640 = OoMkHToWuG17280119;     OoMkHToWuG17280119 = OoMkHToWuG9305650;     OoMkHToWuG9305650 = OoMkHToWuG24265371;     OoMkHToWuG24265371 = OoMkHToWuG93005813;     OoMkHToWuG93005813 = OoMkHToWuG61139950;     OoMkHToWuG61139950 = OoMkHToWuG35562626;     OoMkHToWuG35562626 = OoMkHToWuG19027817;     OoMkHToWuG19027817 = OoMkHToWuG77658971;     OoMkHToWuG77658971 = OoMkHToWuG66638693;     OoMkHToWuG66638693 = OoMkHToWuG26402487;     OoMkHToWuG26402487 = OoMkHToWuG77892896;     OoMkHToWuG77892896 = OoMkHToWuG71675578;     OoMkHToWuG71675578 = OoMkHToWuG31783991;     OoMkHToWuG31783991 = OoMkHToWuG78672657;     OoMkHToWuG78672657 = OoMkHToWuG8688913;     OoMkHToWuG8688913 = OoMkHToWuG73051457;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rylYYqvTJE15130063() {     int OgdoVxQIAd56495762 = -964684770;    int OgdoVxQIAd50596537 = -64245077;    int OgdoVxQIAd7990191 = -779825489;    int OgdoVxQIAd85248575 = -389080819;    int OgdoVxQIAd17573302 = 45912476;    int OgdoVxQIAd73707590 = -690495796;    int OgdoVxQIAd12235337 = -299896112;    int OgdoVxQIAd35014462 = -757968454;    int OgdoVxQIAd90800912 = -983447358;    int OgdoVxQIAd2677146 = -477921115;    int OgdoVxQIAd46902432 = 20901819;    int OgdoVxQIAd99459491 = -203236363;    int OgdoVxQIAd82737626 = -183815327;    int OgdoVxQIAd33780318 = 13726130;    int OgdoVxQIAd54397493 = -332965973;    int OgdoVxQIAd99561534 = -417495911;    int OgdoVxQIAd64528504 = -908423711;    int OgdoVxQIAd33105554 = -553103368;    int OgdoVxQIAd98322335 = -285337978;    int OgdoVxQIAd79692207 = -538241472;    int OgdoVxQIAd16234046 = -561888252;    int OgdoVxQIAd91927007 = -766038989;    int OgdoVxQIAd78071712 = -574752431;    int OgdoVxQIAd73121720 = -647206054;    int OgdoVxQIAd30349406 = -867320087;    int OgdoVxQIAd26626357 = -859057318;    int OgdoVxQIAd70705204 = -828550542;    int OgdoVxQIAd3207783 = -643056415;    int OgdoVxQIAd14111733 = -542135715;    int OgdoVxQIAd83288104 = -887839625;    int OgdoVxQIAd80824459 = -917089868;    int OgdoVxQIAd48774473 = -349847236;    int OgdoVxQIAd35062810 = -516390422;    int OgdoVxQIAd71487387 = -490194389;    int OgdoVxQIAd33138448 = -345150495;    int OgdoVxQIAd7430254 = -502372556;    int OgdoVxQIAd60808517 = -669652354;    int OgdoVxQIAd51920680 = -503192384;    int OgdoVxQIAd95801541 = -62879091;    int OgdoVxQIAd29850348 = -841525336;    int OgdoVxQIAd89261681 = -821482867;    int OgdoVxQIAd64799745 = -600735751;    int OgdoVxQIAd20167510 = -542150158;    int OgdoVxQIAd77979946 = -723573825;    int OgdoVxQIAd12451858 = -166635194;    int OgdoVxQIAd67607157 = -312803919;    int OgdoVxQIAd96318868 = -430844160;    int OgdoVxQIAd20657735 = -946489520;    int OgdoVxQIAd2505347 = -513633471;    int OgdoVxQIAd33835970 = -187176510;    int OgdoVxQIAd40744571 = -209000885;    int OgdoVxQIAd949821 = -964029458;    int OgdoVxQIAd90474855 = -475460308;    int OgdoVxQIAd91935311 = -844358675;    int OgdoVxQIAd98896017 = -180548863;    int OgdoVxQIAd64568754 = -98645781;    int OgdoVxQIAd72524824 = -489492646;    int OgdoVxQIAd34868470 = -32619435;    int OgdoVxQIAd54899170 = -521760733;    int OgdoVxQIAd90946945 = -95030206;    int OgdoVxQIAd3002387 = -861945254;    int OgdoVxQIAd9027554 = -656839698;    int OgdoVxQIAd20902729 = -115832739;    int OgdoVxQIAd7512809 = 4392267;    int OgdoVxQIAd21852687 = -560831247;    int OgdoVxQIAd98127958 = -629250946;    int OgdoVxQIAd64396681 = -686845942;    int OgdoVxQIAd11250240 = -693620939;    int OgdoVxQIAd641871 = -641123375;    int OgdoVxQIAd46967239 = -830593417;    int OgdoVxQIAd38753018 = -747843558;    int OgdoVxQIAd12607825 = -305231327;    int OgdoVxQIAd37304013 = -390224278;    int OgdoVxQIAd68471987 = -443812643;    int OgdoVxQIAd90430525 = -716758605;    int OgdoVxQIAd51434301 = -961152501;    int OgdoVxQIAd71759498 = -123888831;    int OgdoVxQIAd91767 = -851178607;    int OgdoVxQIAd60669863 = -380570860;    int OgdoVxQIAd62742249 = -454516169;    int OgdoVxQIAd30307488 = -328213159;    int OgdoVxQIAd50047469 = -882061022;    int OgdoVxQIAd702437 = -29422945;    int OgdoVxQIAd80275762 = -254959205;    int OgdoVxQIAd42543534 = -578838741;    int OgdoVxQIAd79874638 = -953060411;    int OgdoVxQIAd58299618 = -874386929;    int OgdoVxQIAd43127499 = -672031748;    int OgdoVxQIAd72591369 = -209645526;    int OgdoVxQIAd68569693 = -146504715;    int OgdoVxQIAd34905429 = 87120090;    int OgdoVxQIAd25940047 = -537032919;    int OgdoVxQIAd97021510 = -981431652;    int OgdoVxQIAd4854597 = -967848885;    int OgdoVxQIAd26847962 = -979580082;    int OgdoVxQIAd80234128 = -64643170;    int OgdoVxQIAd43897016 = -384903013;    int OgdoVxQIAd12654702 = -446542425;    int OgdoVxQIAd56127259 = -62742578;    int OgdoVxQIAd14323899 = -964684770;     OgdoVxQIAd56495762 = OgdoVxQIAd50596537;     OgdoVxQIAd50596537 = OgdoVxQIAd7990191;     OgdoVxQIAd7990191 = OgdoVxQIAd85248575;     OgdoVxQIAd85248575 = OgdoVxQIAd17573302;     OgdoVxQIAd17573302 = OgdoVxQIAd73707590;     OgdoVxQIAd73707590 = OgdoVxQIAd12235337;     OgdoVxQIAd12235337 = OgdoVxQIAd35014462;     OgdoVxQIAd35014462 = OgdoVxQIAd90800912;     OgdoVxQIAd90800912 = OgdoVxQIAd2677146;     OgdoVxQIAd2677146 = OgdoVxQIAd46902432;     OgdoVxQIAd46902432 = OgdoVxQIAd99459491;     OgdoVxQIAd99459491 = OgdoVxQIAd82737626;     OgdoVxQIAd82737626 = OgdoVxQIAd33780318;     OgdoVxQIAd33780318 = OgdoVxQIAd54397493;     OgdoVxQIAd54397493 = OgdoVxQIAd99561534;     OgdoVxQIAd99561534 = OgdoVxQIAd64528504;     OgdoVxQIAd64528504 = OgdoVxQIAd33105554;     OgdoVxQIAd33105554 = OgdoVxQIAd98322335;     OgdoVxQIAd98322335 = OgdoVxQIAd79692207;     OgdoVxQIAd79692207 = OgdoVxQIAd16234046;     OgdoVxQIAd16234046 = OgdoVxQIAd91927007;     OgdoVxQIAd91927007 = OgdoVxQIAd78071712;     OgdoVxQIAd78071712 = OgdoVxQIAd73121720;     OgdoVxQIAd73121720 = OgdoVxQIAd30349406;     OgdoVxQIAd30349406 = OgdoVxQIAd26626357;     OgdoVxQIAd26626357 = OgdoVxQIAd70705204;     OgdoVxQIAd70705204 = OgdoVxQIAd3207783;     OgdoVxQIAd3207783 = OgdoVxQIAd14111733;     OgdoVxQIAd14111733 = OgdoVxQIAd83288104;     OgdoVxQIAd83288104 = OgdoVxQIAd80824459;     OgdoVxQIAd80824459 = OgdoVxQIAd48774473;     OgdoVxQIAd48774473 = OgdoVxQIAd35062810;     OgdoVxQIAd35062810 = OgdoVxQIAd71487387;     OgdoVxQIAd71487387 = OgdoVxQIAd33138448;     OgdoVxQIAd33138448 = OgdoVxQIAd7430254;     OgdoVxQIAd7430254 = OgdoVxQIAd60808517;     OgdoVxQIAd60808517 = OgdoVxQIAd51920680;     OgdoVxQIAd51920680 = OgdoVxQIAd95801541;     OgdoVxQIAd95801541 = OgdoVxQIAd29850348;     OgdoVxQIAd29850348 = OgdoVxQIAd89261681;     OgdoVxQIAd89261681 = OgdoVxQIAd64799745;     OgdoVxQIAd64799745 = OgdoVxQIAd20167510;     OgdoVxQIAd20167510 = OgdoVxQIAd77979946;     OgdoVxQIAd77979946 = OgdoVxQIAd12451858;     OgdoVxQIAd12451858 = OgdoVxQIAd67607157;     OgdoVxQIAd67607157 = OgdoVxQIAd96318868;     OgdoVxQIAd96318868 = OgdoVxQIAd20657735;     OgdoVxQIAd20657735 = OgdoVxQIAd2505347;     OgdoVxQIAd2505347 = OgdoVxQIAd33835970;     OgdoVxQIAd33835970 = OgdoVxQIAd40744571;     OgdoVxQIAd40744571 = OgdoVxQIAd949821;     OgdoVxQIAd949821 = OgdoVxQIAd90474855;     OgdoVxQIAd90474855 = OgdoVxQIAd91935311;     OgdoVxQIAd91935311 = OgdoVxQIAd98896017;     OgdoVxQIAd98896017 = OgdoVxQIAd64568754;     OgdoVxQIAd64568754 = OgdoVxQIAd72524824;     OgdoVxQIAd72524824 = OgdoVxQIAd34868470;     OgdoVxQIAd34868470 = OgdoVxQIAd54899170;     OgdoVxQIAd54899170 = OgdoVxQIAd90946945;     OgdoVxQIAd90946945 = OgdoVxQIAd3002387;     OgdoVxQIAd3002387 = OgdoVxQIAd9027554;     OgdoVxQIAd9027554 = OgdoVxQIAd20902729;     OgdoVxQIAd20902729 = OgdoVxQIAd7512809;     OgdoVxQIAd7512809 = OgdoVxQIAd21852687;     OgdoVxQIAd21852687 = OgdoVxQIAd98127958;     OgdoVxQIAd98127958 = OgdoVxQIAd64396681;     OgdoVxQIAd64396681 = OgdoVxQIAd11250240;     OgdoVxQIAd11250240 = OgdoVxQIAd641871;     OgdoVxQIAd641871 = OgdoVxQIAd46967239;     OgdoVxQIAd46967239 = OgdoVxQIAd38753018;     OgdoVxQIAd38753018 = OgdoVxQIAd12607825;     OgdoVxQIAd12607825 = OgdoVxQIAd37304013;     OgdoVxQIAd37304013 = OgdoVxQIAd68471987;     OgdoVxQIAd68471987 = OgdoVxQIAd90430525;     OgdoVxQIAd90430525 = OgdoVxQIAd51434301;     OgdoVxQIAd51434301 = OgdoVxQIAd71759498;     OgdoVxQIAd71759498 = OgdoVxQIAd91767;     OgdoVxQIAd91767 = OgdoVxQIAd60669863;     OgdoVxQIAd60669863 = OgdoVxQIAd62742249;     OgdoVxQIAd62742249 = OgdoVxQIAd30307488;     OgdoVxQIAd30307488 = OgdoVxQIAd50047469;     OgdoVxQIAd50047469 = OgdoVxQIAd702437;     OgdoVxQIAd702437 = OgdoVxQIAd80275762;     OgdoVxQIAd80275762 = OgdoVxQIAd42543534;     OgdoVxQIAd42543534 = OgdoVxQIAd79874638;     OgdoVxQIAd79874638 = OgdoVxQIAd58299618;     OgdoVxQIAd58299618 = OgdoVxQIAd43127499;     OgdoVxQIAd43127499 = OgdoVxQIAd72591369;     OgdoVxQIAd72591369 = OgdoVxQIAd68569693;     OgdoVxQIAd68569693 = OgdoVxQIAd34905429;     OgdoVxQIAd34905429 = OgdoVxQIAd25940047;     OgdoVxQIAd25940047 = OgdoVxQIAd97021510;     OgdoVxQIAd97021510 = OgdoVxQIAd4854597;     OgdoVxQIAd4854597 = OgdoVxQIAd26847962;     OgdoVxQIAd26847962 = OgdoVxQIAd80234128;     OgdoVxQIAd80234128 = OgdoVxQIAd43897016;     OgdoVxQIAd43897016 = OgdoVxQIAd12654702;     OgdoVxQIAd12654702 = OgdoVxQIAd56127259;     OgdoVxQIAd56127259 = OgdoVxQIAd14323899;     OgdoVxQIAd14323899 = OgdoVxQIAd56495762;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dqueYwFhoT85585935() {     int iuYXbEVLdH69281929 = 274539;    int iuYXbEVLdH37367425 = -234579090;    int iuYXbEVLdH25742991 = -88184871;    int iuYXbEVLdH93189401 = -213283096;    int iuYXbEVLdH11084407 = -445903989;    int iuYXbEVLdH17017661 = -467419109;    int iuYXbEVLdH33443095 = -845568183;    int iuYXbEVLdH61733574 = -102486711;    int iuYXbEVLdH86205722 = -945205450;    int iuYXbEVLdH8895851 = -282008747;    int iuYXbEVLdH74389814 = -93784197;    int iuYXbEVLdH31409997 = -710569179;    int iuYXbEVLdH52880929 = -87597984;    int iuYXbEVLdH83404396 = -318689068;    int iuYXbEVLdH77704046 = -239951804;    int iuYXbEVLdH27986604 = -968387301;    int iuYXbEVLdH20280322 = -572198177;    int iuYXbEVLdH16664040 = -108313671;    int iuYXbEVLdH99482965 = -194807174;    int iuYXbEVLdH82346703 = -37485611;    int iuYXbEVLdH96408102 = 46795484;    int iuYXbEVLdH20974827 = -935897337;    int iuYXbEVLdH16832008 = -505756325;    int iuYXbEVLdH17628851 = 92448933;    int iuYXbEVLdH36243665 = 22237989;    int iuYXbEVLdH7945002 = -549488518;    int iuYXbEVLdH82120740 = -362256822;    int iuYXbEVLdH70560708 = -816854520;    int iuYXbEVLdH30728786 = -756352955;    int iuYXbEVLdH36039220 = -517725780;    int iuYXbEVLdH25260456 = -93506618;    int iuYXbEVLdH52093297 = -328566915;    int iuYXbEVLdH81108759 = -392245207;    int iuYXbEVLdH48563760 = -394948607;    int iuYXbEVLdH16198980 = -955182376;    int iuYXbEVLdH2654369 = -290935660;    int iuYXbEVLdH97572577 = -459561704;    int iuYXbEVLdH9624703 = -422812475;    int iuYXbEVLdH1576928 = 95270332;    int iuYXbEVLdH56115077 = -685942181;    int iuYXbEVLdH50692989 = -96956860;    int iuYXbEVLdH72502201 = -499047086;    int iuYXbEVLdH55050979 = -565583603;    int iuYXbEVLdH22608484 = 35253933;    int iuYXbEVLdH92460347 = -59405155;    int iuYXbEVLdH41235287 = -386122359;    int iuYXbEVLdH73900888 = -928359212;    int iuYXbEVLdH50097701 = -1842300;    int iuYXbEVLdH46569396 = -677235445;    int iuYXbEVLdH80786391 = -798212973;    int iuYXbEVLdH59072064 = -461885193;    int iuYXbEVLdH29126368 = -335214796;    int iuYXbEVLdH24960006 = -651463590;    int iuYXbEVLdH36635037 = -433810417;    int iuYXbEVLdH89177833 = -174993350;    int iuYXbEVLdH48307102 = -63828125;    int iuYXbEVLdH20535417 = -728822765;    int iuYXbEVLdH8114141 = -80633805;    int iuYXbEVLdH56945736 = -135521085;    int iuYXbEVLdH3139405 = -896415472;    int iuYXbEVLdH34896921 = -5162287;    int iuYXbEVLdH62882386 = 71286337;    int iuYXbEVLdH31004789 = -346133756;    int iuYXbEVLdH50166503 = -327479671;    int iuYXbEVLdH83635395 = -88502129;    int iuYXbEVLdH22296517 = -765217282;    int iuYXbEVLdH50301237 = -218323973;    int iuYXbEVLdH4317170 = -692649377;    int iuYXbEVLdH67205417 = -363506692;    int iuYXbEVLdH75049677 = -949016144;    int iuYXbEVLdH30414026 = -408825597;    int iuYXbEVLdH10655620 = -49385702;    int iuYXbEVLdH15087112 = -103584003;    int iuYXbEVLdH43367888 = -508864993;    int iuYXbEVLdH31653714 = -940528751;    int iuYXbEVLdH23905902 = -454157431;    int iuYXbEVLdH65923847 = -270313734;    int iuYXbEVLdH94223524 = -441010258;    int iuYXbEVLdH25168503 = -848145913;    int iuYXbEVLdH95008378 = -591639653;    int iuYXbEVLdH34044114 = -621129306;    int iuYXbEVLdH32023039 = -260414523;    int iuYXbEVLdH23991312 = -39619075;    int iuYXbEVLdH49942395 = -958139983;    int iuYXbEVLdH76967155 = 44159413;    int iuYXbEVLdH96134087 = -758291823;    int iuYXbEVLdH27133292 = -677103325;    int iuYXbEVLdH44473723 = -958434790;    int iuYXbEVLdH59385926 = -119955258;    int iuYXbEVLdH67891877 = -791354251;    int iuYXbEVLdH82118951 = -562112895;    int iuYXbEVLdH89458436 = -278927900;    int iuYXbEVLdH52678967 = -187291391;    int iuYXbEVLdH98437522 = -8314196;    int iuYXbEVLdH21218157 = -580779895;    int iuYXbEVLdH87810602 = -68243197;    int iuYXbEVLdH41497412 = -52913330;    int iuYXbEVLdH4884477 = -138103933;    int iuYXbEVLdH38973089 = -876243938;    int iuYXbEVLdH70163830 = 274539;     iuYXbEVLdH69281929 = iuYXbEVLdH37367425;     iuYXbEVLdH37367425 = iuYXbEVLdH25742991;     iuYXbEVLdH25742991 = iuYXbEVLdH93189401;     iuYXbEVLdH93189401 = iuYXbEVLdH11084407;     iuYXbEVLdH11084407 = iuYXbEVLdH17017661;     iuYXbEVLdH17017661 = iuYXbEVLdH33443095;     iuYXbEVLdH33443095 = iuYXbEVLdH61733574;     iuYXbEVLdH61733574 = iuYXbEVLdH86205722;     iuYXbEVLdH86205722 = iuYXbEVLdH8895851;     iuYXbEVLdH8895851 = iuYXbEVLdH74389814;     iuYXbEVLdH74389814 = iuYXbEVLdH31409997;     iuYXbEVLdH31409997 = iuYXbEVLdH52880929;     iuYXbEVLdH52880929 = iuYXbEVLdH83404396;     iuYXbEVLdH83404396 = iuYXbEVLdH77704046;     iuYXbEVLdH77704046 = iuYXbEVLdH27986604;     iuYXbEVLdH27986604 = iuYXbEVLdH20280322;     iuYXbEVLdH20280322 = iuYXbEVLdH16664040;     iuYXbEVLdH16664040 = iuYXbEVLdH99482965;     iuYXbEVLdH99482965 = iuYXbEVLdH82346703;     iuYXbEVLdH82346703 = iuYXbEVLdH96408102;     iuYXbEVLdH96408102 = iuYXbEVLdH20974827;     iuYXbEVLdH20974827 = iuYXbEVLdH16832008;     iuYXbEVLdH16832008 = iuYXbEVLdH17628851;     iuYXbEVLdH17628851 = iuYXbEVLdH36243665;     iuYXbEVLdH36243665 = iuYXbEVLdH7945002;     iuYXbEVLdH7945002 = iuYXbEVLdH82120740;     iuYXbEVLdH82120740 = iuYXbEVLdH70560708;     iuYXbEVLdH70560708 = iuYXbEVLdH30728786;     iuYXbEVLdH30728786 = iuYXbEVLdH36039220;     iuYXbEVLdH36039220 = iuYXbEVLdH25260456;     iuYXbEVLdH25260456 = iuYXbEVLdH52093297;     iuYXbEVLdH52093297 = iuYXbEVLdH81108759;     iuYXbEVLdH81108759 = iuYXbEVLdH48563760;     iuYXbEVLdH48563760 = iuYXbEVLdH16198980;     iuYXbEVLdH16198980 = iuYXbEVLdH2654369;     iuYXbEVLdH2654369 = iuYXbEVLdH97572577;     iuYXbEVLdH97572577 = iuYXbEVLdH9624703;     iuYXbEVLdH9624703 = iuYXbEVLdH1576928;     iuYXbEVLdH1576928 = iuYXbEVLdH56115077;     iuYXbEVLdH56115077 = iuYXbEVLdH50692989;     iuYXbEVLdH50692989 = iuYXbEVLdH72502201;     iuYXbEVLdH72502201 = iuYXbEVLdH55050979;     iuYXbEVLdH55050979 = iuYXbEVLdH22608484;     iuYXbEVLdH22608484 = iuYXbEVLdH92460347;     iuYXbEVLdH92460347 = iuYXbEVLdH41235287;     iuYXbEVLdH41235287 = iuYXbEVLdH73900888;     iuYXbEVLdH73900888 = iuYXbEVLdH50097701;     iuYXbEVLdH50097701 = iuYXbEVLdH46569396;     iuYXbEVLdH46569396 = iuYXbEVLdH80786391;     iuYXbEVLdH80786391 = iuYXbEVLdH59072064;     iuYXbEVLdH59072064 = iuYXbEVLdH29126368;     iuYXbEVLdH29126368 = iuYXbEVLdH24960006;     iuYXbEVLdH24960006 = iuYXbEVLdH36635037;     iuYXbEVLdH36635037 = iuYXbEVLdH89177833;     iuYXbEVLdH89177833 = iuYXbEVLdH48307102;     iuYXbEVLdH48307102 = iuYXbEVLdH20535417;     iuYXbEVLdH20535417 = iuYXbEVLdH8114141;     iuYXbEVLdH8114141 = iuYXbEVLdH56945736;     iuYXbEVLdH56945736 = iuYXbEVLdH3139405;     iuYXbEVLdH3139405 = iuYXbEVLdH34896921;     iuYXbEVLdH34896921 = iuYXbEVLdH62882386;     iuYXbEVLdH62882386 = iuYXbEVLdH31004789;     iuYXbEVLdH31004789 = iuYXbEVLdH50166503;     iuYXbEVLdH50166503 = iuYXbEVLdH83635395;     iuYXbEVLdH83635395 = iuYXbEVLdH22296517;     iuYXbEVLdH22296517 = iuYXbEVLdH50301237;     iuYXbEVLdH50301237 = iuYXbEVLdH4317170;     iuYXbEVLdH4317170 = iuYXbEVLdH67205417;     iuYXbEVLdH67205417 = iuYXbEVLdH75049677;     iuYXbEVLdH75049677 = iuYXbEVLdH30414026;     iuYXbEVLdH30414026 = iuYXbEVLdH10655620;     iuYXbEVLdH10655620 = iuYXbEVLdH15087112;     iuYXbEVLdH15087112 = iuYXbEVLdH43367888;     iuYXbEVLdH43367888 = iuYXbEVLdH31653714;     iuYXbEVLdH31653714 = iuYXbEVLdH23905902;     iuYXbEVLdH23905902 = iuYXbEVLdH65923847;     iuYXbEVLdH65923847 = iuYXbEVLdH94223524;     iuYXbEVLdH94223524 = iuYXbEVLdH25168503;     iuYXbEVLdH25168503 = iuYXbEVLdH95008378;     iuYXbEVLdH95008378 = iuYXbEVLdH34044114;     iuYXbEVLdH34044114 = iuYXbEVLdH32023039;     iuYXbEVLdH32023039 = iuYXbEVLdH23991312;     iuYXbEVLdH23991312 = iuYXbEVLdH49942395;     iuYXbEVLdH49942395 = iuYXbEVLdH76967155;     iuYXbEVLdH76967155 = iuYXbEVLdH96134087;     iuYXbEVLdH96134087 = iuYXbEVLdH27133292;     iuYXbEVLdH27133292 = iuYXbEVLdH44473723;     iuYXbEVLdH44473723 = iuYXbEVLdH59385926;     iuYXbEVLdH59385926 = iuYXbEVLdH67891877;     iuYXbEVLdH67891877 = iuYXbEVLdH82118951;     iuYXbEVLdH82118951 = iuYXbEVLdH89458436;     iuYXbEVLdH89458436 = iuYXbEVLdH52678967;     iuYXbEVLdH52678967 = iuYXbEVLdH98437522;     iuYXbEVLdH98437522 = iuYXbEVLdH21218157;     iuYXbEVLdH21218157 = iuYXbEVLdH87810602;     iuYXbEVLdH87810602 = iuYXbEVLdH41497412;     iuYXbEVLdH41497412 = iuYXbEVLdH4884477;     iuYXbEVLdH4884477 = iuYXbEVLdH38973089;     iuYXbEVLdH38973089 = iuYXbEVLdH70163830;     iuYXbEVLdH70163830 = iuYXbEVLdH69281929;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WaiMlCSHlD3799278() {     int JaCbhfYeYo52726235 = -880391142;    int JaCbhfYeYo54239514 = -233961278;    int JaCbhfYeYo22151498 = -981710098;    int JaCbhfYeYo55981652 = -407082932;    int JaCbhfYeYo17205731 = -252771480;    int JaCbhfYeYo15522062 = -246490762;    int JaCbhfYeYo4482413 = -585479496;    int JaCbhfYeYo76081017 = -89644740;    int JaCbhfYeYo44735424 = -186664801;    int JaCbhfYeYo41441503 = -750384860;    int JaCbhfYeYo23805752 = -283900069;    int JaCbhfYeYo94615395 = -698187426;    int JaCbhfYeYo70998705 = -979259225;    int JaCbhfYeYo63031288 = -106979425;    int JaCbhfYeYo38558509 = -333974786;    int JaCbhfYeYo14375765 = -756466626;    int JaCbhfYeYo14032207 = -325543204;    int JaCbhfYeYo97515485 = -355618547;    int JaCbhfYeYo61923108 = -193482044;    int JaCbhfYeYo71714145 = -567071603;    int JaCbhfYeYo52048489 = -642733547;    int JaCbhfYeYo33122145 = -927049107;    int JaCbhfYeYo20657716 = -545645523;    int JaCbhfYeYo52335408 = 20623537;    int JaCbhfYeYo77819623 = -537414770;    int JaCbhfYeYo3287151 = -429998233;    int JaCbhfYeYo31575480 = 9658835;    int JaCbhfYeYo10292334 = -886809404;    int JaCbhfYeYo66809880 = -618858098;    int JaCbhfYeYo51872561 = 69424369;    int JaCbhfYeYo43355769 = -831263819;    int JaCbhfYeYo15199724 = -243761994;    int JaCbhfYeYo24647646 = -13117384;    int JaCbhfYeYo90722270 = 13449223;    int JaCbhfYeYo94925710 = -52847111;    int JaCbhfYeYo15043553 = -78959566;    int JaCbhfYeYo66186756 = -338410884;    int JaCbhfYeYo50203535 = -131282995;    int JaCbhfYeYo50572005 = -89627465;    int JaCbhfYeYo5610213 = -438535704;    int JaCbhfYeYo84341913 = -768505833;    int JaCbhfYeYo22989987 = 27044593;    int JaCbhfYeYo78618049 = -16640846;    int JaCbhfYeYo14513424 = -820928838;    int JaCbhfYeYo84404844 = -887562108;    int JaCbhfYeYo80841282 = -405733282;    int JaCbhfYeYo49916319 = 39336558;    int JaCbhfYeYo69768297 = -564018;    int JaCbhfYeYo77416508 = -162837528;    int JaCbhfYeYo16118875 = 94766571;    int JaCbhfYeYo32329512 = -682786615;    int JaCbhfYeYo84627162 = -393474108;    int JaCbhfYeYo39072465 = -863210907;    int JaCbhfYeYo61311795 = -152223804;    int JaCbhfYeYo51750788 = -523479473;    int JaCbhfYeYo19604090 = -953342035;    int JaCbhfYeYo33581799 = -688315756;    int JaCbhfYeYo69816089 = -902333636;    int JaCbhfYeYo78162029 = -869668162;    int JaCbhfYeYo13918580 = -822773247;    int JaCbhfYeYo83946581 = -156149598;    int JaCbhfYeYo94190079 = -698670093;    int JaCbhfYeYo9271138 = -470786643;    int JaCbhfYeYo92862862 = -156089170;    int JaCbhfYeYo98085733 = -919121041;    int JaCbhfYeYo8606028 = 59861925;    int JaCbhfYeYo69967750 = -585070042;    int JaCbhfYeYo80276434 = -892708448;    int JaCbhfYeYo68105578 = 45867686;    int JaCbhfYeYo23514957 = -155015220;    int JaCbhfYeYo48189009 = -318055742;    int JaCbhfYeYo63828672 = -94260209;    int JaCbhfYeYo46943480 = -165991082;    int JaCbhfYeYo56312895 = -754946341;    int JaCbhfYeYo87372232 = -798565770;    int JaCbhfYeYo29058503 = -569778141;    int JaCbhfYeYo54504096 = -810408262;    int JaCbhfYeYo6144292 = -724716685;    int JaCbhfYeYo67930564 = -91814355;    int JaCbhfYeYo96978340 = -31681488;    int JaCbhfYeYo53370832 = -369334792;    int JaCbhfYeYo61807183 = -989777147;    int JaCbhfYeYo32875825 = -623971877;    int JaCbhfYeYo50691005 = -613624670;    int JaCbhfYeYo19543049 = -247789016;    int JaCbhfYeYo58728606 = -337789712;    int JaCbhfYeYo76127259 = -380551088;    int JaCbhfYeYo63335850 = -860893580;    int JaCbhfYeYo38971482 = -463071305;    int JaCbhfYeYo75321620 = -99505076;    int JaCbhfYeYo81461754 = -390643811;    int JaCbhfYeYo96370666 = -436077248;    int JaCbhfYeYo72041505 = -261614834;    int JaCbhfYeYo36653426 = -266854219;    int JaCbhfYeYo21663632 = -182386106;    int JaCbhfYeYo90151834 = 30164259;    int JaCbhfYeYo13718850 = -502168764;    int JaCbhfYeYo85755186 = -860551676;    int JaCbhfYeYo16427691 = -901807798;    int JaCbhfYeYo75798816 = -880391142;     JaCbhfYeYo52726235 = JaCbhfYeYo54239514;     JaCbhfYeYo54239514 = JaCbhfYeYo22151498;     JaCbhfYeYo22151498 = JaCbhfYeYo55981652;     JaCbhfYeYo55981652 = JaCbhfYeYo17205731;     JaCbhfYeYo17205731 = JaCbhfYeYo15522062;     JaCbhfYeYo15522062 = JaCbhfYeYo4482413;     JaCbhfYeYo4482413 = JaCbhfYeYo76081017;     JaCbhfYeYo76081017 = JaCbhfYeYo44735424;     JaCbhfYeYo44735424 = JaCbhfYeYo41441503;     JaCbhfYeYo41441503 = JaCbhfYeYo23805752;     JaCbhfYeYo23805752 = JaCbhfYeYo94615395;     JaCbhfYeYo94615395 = JaCbhfYeYo70998705;     JaCbhfYeYo70998705 = JaCbhfYeYo63031288;     JaCbhfYeYo63031288 = JaCbhfYeYo38558509;     JaCbhfYeYo38558509 = JaCbhfYeYo14375765;     JaCbhfYeYo14375765 = JaCbhfYeYo14032207;     JaCbhfYeYo14032207 = JaCbhfYeYo97515485;     JaCbhfYeYo97515485 = JaCbhfYeYo61923108;     JaCbhfYeYo61923108 = JaCbhfYeYo71714145;     JaCbhfYeYo71714145 = JaCbhfYeYo52048489;     JaCbhfYeYo52048489 = JaCbhfYeYo33122145;     JaCbhfYeYo33122145 = JaCbhfYeYo20657716;     JaCbhfYeYo20657716 = JaCbhfYeYo52335408;     JaCbhfYeYo52335408 = JaCbhfYeYo77819623;     JaCbhfYeYo77819623 = JaCbhfYeYo3287151;     JaCbhfYeYo3287151 = JaCbhfYeYo31575480;     JaCbhfYeYo31575480 = JaCbhfYeYo10292334;     JaCbhfYeYo10292334 = JaCbhfYeYo66809880;     JaCbhfYeYo66809880 = JaCbhfYeYo51872561;     JaCbhfYeYo51872561 = JaCbhfYeYo43355769;     JaCbhfYeYo43355769 = JaCbhfYeYo15199724;     JaCbhfYeYo15199724 = JaCbhfYeYo24647646;     JaCbhfYeYo24647646 = JaCbhfYeYo90722270;     JaCbhfYeYo90722270 = JaCbhfYeYo94925710;     JaCbhfYeYo94925710 = JaCbhfYeYo15043553;     JaCbhfYeYo15043553 = JaCbhfYeYo66186756;     JaCbhfYeYo66186756 = JaCbhfYeYo50203535;     JaCbhfYeYo50203535 = JaCbhfYeYo50572005;     JaCbhfYeYo50572005 = JaCbhfYeYo5610213;     JaCbhfYeYo5610213 = JaCbhfYeYo84341913;     JaCbhfYeYo84341913 = JaCbhfYeYo22989987;     JaCbhfYeYo22989987 = JaCbhfYeYo78618049;     JaCbhfYeYo78618049 = JaCbhfYeYo14513424;     JaCbhfYeYo14513424 = JaCbhfYeYo84404844;     JaCbhfYeYo84404844 = JaCbhfYeYo80841282;     JaCbhfYeYo80841282 = JaCbhfYeYo49916319;     JaCbhfYeYo49916319 = JaCbhfYeYo69768297;     JaCbhfYeYo69768297 = JaCbhfYeYo77416508;     JaCbhfYeYo77416508 = JaCbhfYeYo16118875;     JaCbhfYeYo16118875 = JaCbhfYeYo32329512;     JaCbhfYeYo32329512 = JaCbhfYeYo84627162;     JaCbhfYeYo84627162 = JaCbhfYeYo39072465;     JaCbhfYeYo39072465 = JaCbhfYeYo61311795;     JaCbhfYeYo61311795 = JaCbhfYeYo51750788;     JaCbhfYeYo51750788 = JaCbhfYeYo19604090;     JaCbhfYeYo19604090 = JaCbhfYeYo33581799;     JaCbhfYeYo33581799 = JaCbhfYeYo69816089;     JaCbhfYeYo69816089 = JaCbhfYeYo78162029;     JaCbhfYeYo78162029 = JaCbhfYeYo13918580;     JaCbhfYeYo13918580 = JaCbhfYeYo83946581;     JaCbhfYeYo83946581 = JaCbhfYeYo94190079;     JaCbhfYeYo94190079 = JaCbhfYeYo9271138;     JaCbhfYeYo9271138 = JaCbhfYeYo92862862;     JaCbhfYeYo92862862 = JaCbhfYeYo98085733;     JaCbhfYeYo98085733 = JaCbhfYeYo8606028;     JaCbhfYeYo8606028 = JaCbhfYeYo69967750;     JaCbhfYeYo69967750 = JaCbhfYeYo80276434;     JaCbhfYeYo80276434 = JaCbhfYeYo68105578;     JaCbhfYeYo68105578 = JaCbhfYeYo23514957;     JaCbhfYeYo23514957 = JaCbhfYeYo48189009;     JaCbhfYeYo48189009 = JaCbhfYeYo63828672;     JaCbhfYeYo63828672 = JaCbhfYeYo46943480;     JaCbhfYeYo46943480 = JaCbhfYeYo56312895;     JaCbhfYeYo56312895 = JaCbhfYeYo87372232;     JaCbhfYeYo87372232 = JaCbhfYeYo29058503;     JaCbhfYeYo29058503 = JaCbhfYeYo54504096;     JaCbhfYeYo54504096 = JaCbhfYeYo6144292;     JaCbhfYeYo6144292 = JaCbhfYeYo67930564;     JaCbhfYeYo67930564 = JaCbhfYeYo96978340;     JaCbhfYeYo96978340 = JaCbhfYeYo53370832;     JaCbhfYeYo53370832 = JaCbhfYeYo61807183;     JaCbhfYeYo61807183 = JaCbhfYeYo32875825;     JaCbhfYeYo32875825 = JaCbhfYeYo50691005;     JaCbhfYeYo50691005 = JaCbhfYeYo19543049;     JaCbhfYeYo19543049 = JaCbhfYeYo58728606;     JaCbhfYeYo58728606 = JaCbhfYeYo76127259;     JaCbhfYeYo76127259 = JaCbhfYeYo63335850;     JaCbhfYeYo63335850 = JaCbhfYeYo38971482;     JaCbhfYeYo38971482 = JaCbhfYeYo75321620;     JaCbhfYeYo75321620 = JaCbhfYeYo81461754;     JaCbhfYeYo81461754 = JaCbhfYeYo96370666;     JaCbhfYeYo96370666 = JaCbhfYeYo72041505;     JaCbhfYeYo72041505 = JaCbhfYeYo36653426;     JaCbhfYeYo36653426 = JaCbhfYeYo21663632;     JaCbhfYeYo21663632 = JaCbhfYeYo90151834;     JaCbhfYeYo90151834 = JaCbhfYeYo13718850;     JaCbhfYeYo13718850 = JaCbhfYeYo85755186;     JaCbhfYeYo85755186 = JaCbhfYeYo16427691;     JaCbhfYeYo16427691 = JaCbhfYeYo75798816;     JaCbhfYeYo75798816 = JaCbhfYeYo52726235;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void urYZOhvLGv4241381() {     int NpKRZATEyU87905991 = -777311656;    int NpKRZATEyU86785586 = -310243221;    int NpKRZATEyU67360364 = -572594711;    int NpKRZATEyU93849305 = -417151910;    int NpKRZATEyU50987 = -867289625;    int NpKRZATEyU38909817 = -184589642;    int NpKRZATEyU49298573 = -148602406;    int NpKRZATEyU9219938 = -200582324;    int NpKRZATEyU54563201 = -318972862;    int NpKRZATEyU42784279 = -660406954;    int NpKRZATEyU68514388 = -435738413;    int NpKRZATEyU68177172 = -620787172;    int NpKRZATEyU76297274 = -380100727;    int NpKRZATEyU1425900 = -621950328;    int NpKRZATEyU53428230 = 94274522;    int NpKRZATEyU12492199 = -256229907;    int NpKRZATEyU63754617 = -446982921;    int NpKRZATEyU64049854 = 34500082;    int NpKRZATEyU83937099 = -720071098;    int NpKRZATEyU80811162 = -806925745;    int NpKRZATEyU53436229 = -315070746;    int NpKRZATEyU47688917 = 8318115;    int NpKRZATEyU25832938 = -193772167;    int NpKRZATEyU79692217 = 77206190;    int NpKRZATEyU90811439 = 57278034;    int NpKRZATEyU56334714 = -656117728;    int NpKRZATEyU19858855 = -80834225;    int NpKRZATEyU48153183 = -240094974;    int NpKRZATEyU77641047 = -531262142;    int NpKRZATEyU37690986 = -905326278;    int NpKRZATEyU98669891 = -801903487;    int NpKRZATEyU91335881 = -482731266;    int NpKRZATEyU66279842 = -216371447;    int NpKRZATEyU26904493 = -338750113;    int NpKRZATEyU71857569 = -467321489;    int NpKRZATEyU9132347 = -103152301;    int NpKRZATEyU84449160 = -861614808;    int NpKRZATEyU91615979 = 76734120;    int NpKRZATEyU69341926 = -719842658;    int NpKRZATEyU80187764 = -809744893;    int NpKRZATEyU86674925 = -459213594;    int NpKRZATEyU33503173 = -535383350;    int NpKRZATEyU58768351 = -468474620;    int NpKRZATEyU99354182 = -502500287;    int NpKRZATEyU48378549 = -116216144;    int NpKRZATEyU22141726 = -849236147;    int NpKRZATEyU12097943 = -555308125;    int NpKRZATEyU68423357 = -403690432;    int NpKRZATEyU97282073 = -712392340;    int NpKRZATEyU40107618 = 28734736;    int NpKRZATEyU46266852 = -612192193;    int NpKRZATEyU72107709 = -913332980;    int NpKRZATEyU76423669 = -819071411;    int NpKRZATEyU56047795 = -585436504;    int NpKRZATEyU50805151 = -901728797;    int NpKRZATEyU40217074 = -685629771;    int NpKRZATEyU60952648 = -16471054;    int NpKRZATEyU87668147 = -549800901;    int NpKRZATEyU3037867 = -374429945;    int NpKRZATEyU43716273 = -111171898;    int NpKRZATEyU19050962 = -3755417;    int NpKRZATEyU1145390 = -908507433;    int NpKRZATEyU31578891 = -669320183;    int NpKRZATEyU16872216 = -413646584;    int NpKRZATEyU44114387 = -858503468;    int NpKRZATEyU77178507 = -953007148;    int NpKRZATEyU1897331 = -304415725;    int NpKRZATEyU49392781 = 58649385;    int NpKRZATEyU29568331 = -54628840;    int NpKRZATEyU44295883 = -802573178;    int NpKRZATEyU28043039 = -394615099;    int NpKRZATEyU72138638 = -423717042;    int NpKRZATEyU94707928 = -245657260;    int NpKRZATEyU3749336 = -910326206;    int NpKRZATEyU94136237 = -247712151;    int NpKRZATEyU19933056 = -779687397;    int NpKRZATEyU88920566 = -523207265;    int NpKRZATEyU26478755 = -691271881;    int NpKRZATEyU31313668 = -806577666;    int NpKRZATEyU68669714 = -93485820;    int NpKRZATEyU44236771 = -809604;    int NpKRZATEyU51435498 = -677143793;    int NpKRZATEyU50871110 = -527702635;    int NpKRZATEyU37533429 = -459996878;    int NpKRZATEyU91424133 = -193134086;    int NpKRZATEyU26562182 = -888570508;    int NpKRZATEyU14912212 = -663659856;    int NpKRZATEyU10232048 = -630934944;    int NpKRZATEyU76099342 = -437021317;    int NpKRZATEyU31640495 = -781691719;    int NpKRZATEyU48179698 = 13318753;    int NpKRZATEyU96781013 = -211813907;    int NpKRZATEyU88578112 = -548835936;    int NpKRZATEyU25625653 = -508670761;    int NpKRZATEyU61136803 = -705989476;    int NpKRZATEyU85529535 = -550706162;    int NpKRZATEyU1924283 = -866063168;    int NpKRZATEyU41896136 = 45171964;    int NpKRZATEyU55239796 = -643996819;    int NpKRZATEyU71200041 = -777311656;     NpKRZATEyU87905991 = NpKRZATEyU86785586;     NpKRZATEyU86785586 = NpKRZATEyU67360364;     NpKRZATEyU67360364 = NpKRZATEyU93849305;     NpKRZATEyU93849305 = NpKRZATEyU50987;     NpKRZATEyU50987 = NpKRZATEyU38909817;     NpKRZATEyU38909817 = NpKRZATEyU49298573;     NpKRZATEyU49298573 = NpKRZATEyU9219938;     NpKRZATEyU9219938 = NpKRZATEyU54563201;     NpKRZATEyU54563201 = NpKRZATEyU42784279;     NpKRZATEyU42784279 = NpKRZATEyU68514388;     NpKRZATEyU68514388 = NpKRZATEyU68177172;     NpKRZATEyU68177172 = NpKRZATEyU76297274;     NpKRZATEyU76297274 = NpKRZATEyU1425900;     NpKRZATEyU1425900 = NpKRZATEyU53428230;     NpKRZATEyU53428230 = NpKRZATEyU12492199;     NpKRZATEyU12492199 = NpKRZATEyU63754617;     NpKRZATEyU63754617 = NpKRZATEyU64049854;     NpKRZATEyU64049854 = NpKRZATEyU83937099;     NpKRZATEyU83937099 = NpKRZATEyU80811162;     NpKRZATEyU80811162 = NpKRZATEyU53436229;     NpKRZATEyU53436229 = NpKRZATEyU47688917;     NpKRZATEyU47688917 = NpKRZATEyU25832938;     NpKRZATEyU25832938 = NpKRZATEyU79692217;     NpKRZATEyU79692217 = NpKRZATEyU90811439;     NpKRZATEyU90811439 = NpKRZATEyU56334714;     NpKRZATEyU56334714 = NpKRZATEyU19858855;     NpKRZATEyU19858855 = NpKRZATEyU48153183;     NpKRZATEyU48153183 = NpKRZATEyU77641047;     NpKRZATEyU77641047 = NpKRZATEyU37690986;     NpKRZATEyU37690986 = NpKRZATEyU98669891;     NpKRZATEyU98669891 = NpKRZATEyU91335881;     NpKRZATEyU91335881 = NpKRZATEyU66279842;     NpKRZATEyU66279842 = NpKRZATEyU26904493;     NpKRZATEyU26904493 = NpKRZATEyU71857569;     NpKRZATEyU71857569 = NpKRZATEyU9132347;     NpKRZATEyU9132347 = NpKRZATEyU84449160;     NpKRZATEyU84449160 = NpKRZATEyU91615979;     NpKRZATEyU91615979 = NpKRZATEyU69341926;     NpKRZATEyU69341926 = NpKRZATEyU80187764;     NpKRZATEyU80187764 = NpKRZATEyU86674925;     NpKRZATEyU86674925 = NpKRZATEyU33503173;     NpKRZATEyU33503173 = NpKRZATEyU58768351;     NpKRZATEyU58768351 = NpKRZATEyU99354182;     NpKRZATEyU99354182 = NpKRZATEyU48378549;     NpKRZATEyU48378549 = NpKRZATEyU22141726;     NpKRZATEyU22141726 = NpKRZATEyU12097943;     NpKRZATEyU12097943 = NpKRZATEyU68423357;     NpKRZATEyU68423357 = NpKRZATEyU97282073;     NpKRZATEyU97282073 = NpKRZATEyU40107618;     NpKRZATEyU40107618 = NpKRZATEyU46266852;     NpKRZATEyU46266852 = NpKRZATEyU72107709;     NpKRZATEyU72107709 = NpKRZATEyU76423669;     NpKRZATEyU76423669 = NpKRZATEyU56047795;     NpKRZATEyU56047795 = NpKRZATEyU50805151;     NpKRZATEyU50805151 = NpKRZATEyU40217074;     NpKRZATEyU40217074 = NpKRZATEyU60952648;     NpKRZATEyU60952648 = NpKRZATEyU87668147;     NpKRZATEyU87668147 = NpKRZATEyU3037867;     NpKRZATEyU3037867 = NpKRZATEyU43716273;     NpKRZATEyU43716273 = NpKRZATEyU19050962;     NpKRZATEyU19050962 = NpKRZATEyU1145390;     NpKRZATEyU1145390 = NpKRZATEyU31578891;     NpKRZATEyU31578891 = NpKRZATEyU16872216;     NpKRZATEyU16872216 = NpKRZATEyU44114387;     NpKRZATEyU44114387 = NpKRZATEyU77178507;     NpKRZATEyU77178507 = NpKRZATEyU1897331;     NpKRZATEyU1897331 = NpKRZATEyU49392781;     NpKRZATEyU49392781 = NpKRZATEyU29568331;     NpKRZATEyU29568331 = NpKRZATEyU44295883;     NpKRZATEyU44295883 = NpKRZATEyU28043039;     NpKRZATEyU28043039 = NpKRZATEyU72138638;     NpKRZATEyU72138638 = NpKRZATEyU94707928;     NpKRZATEyU94707928 = NpKRZATEyU3749336;     NpKRZATEyU3749336 = NpKRZATEyU94136237;     NpKRZATEyU94136237 = NpKRZATEyU19933056;     NpKRZATEyU19933056 = NpKRZATEyU88920566;     NpKRZATEyU88920566 = NpKRZATEyU26478755;     NpKRZATEyU26478755 = NpKRZATEyU31313668;     NpKRZATEyU31313668 = NpKRZATEyU68669714;     NpKRZATEyU68669714 = NpKRZATEyU44236771;     NpKRZATEyU44236771 = NpKRZATEyU51435498;     NpKRZATEyU51435498 = NpKRZATEyU50871110;     NpKRZATEyU50871110 = NpKRZATEyU37533429;     NpKRZATEyU37533429 = NpKRZATEyU91424133;     NpKRZATEyU91424133 = NpKRZATEyU26562182;     NpKRZATEyU26562182 = NpKRZATEyU14912212;     NpKRZATEyU14912212 = NpKRZATEyU10232048;     NpKRZATEyU10232048 = NpKRZATEyU76099342;     NpKRZATEyU76099342 = NpKRZATEyU31640495;     NpKRZATEyU31640495 = NpKRZATEyU48179698;     NpKRZATEyU48179698 = NpKRZATEyU96781013;     NpKRZATEyU96781013 = NpKRZATEyU88578112;     NpKRZATEyU88578112 = NpKRZATEyU25625653;     NpKRZATEyU25625653 = NpKRZATEyU61136803;     NpKRZATEyU61136803 = NpKRZATEyU85529535;     NpKRZATEyU85529535 = NpKRZATEyU1924283;     NpKRZATEyU1924283 = NpKRZATEyU41896136;     NpKRZATEyU41896136 = NpKRZATEyU55239796;     NpKRZATEyU55239796 = NpKRZATEyU71200041;     NpKRZATEyU71200041 = NpKRZATEyU87905991;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CnnAGkyHNm74697253() {     int ngxkfmYthL692159 = -912352347;    int ngxkfmYthL73556474 = -480577235;    int ngxkfmYthL85113164 = -980954092;    int ngxkfmYthL1790132 = -241354187;    int ngxkfmYthL93562091 = -259106090;    int ngxkfmYthL82219887 = 38487045;    int ngxkfmYthL70506331 = -694274476;    int ngxkfmYthL35939051 = -645100581;    int ngxkfmYthL49968011 = -280730955;    int ngxkfmYthL49002983 = -464494587;    int ngxkfmYthL96001770 = -550424429;    int ngxkfmYthL127678 = -28119988;    int ngxkfmYthL46440577 = -283883384;    int ngxkfmYthL51049978 = -954365527;    int ngxkfmYthL76734783 = -912711309;    int ngxkfmYthL40917268 = -807121296;    int ngxkfmYthL19506435 = -110757388;    int ngxkfmYthL47608339 = -620710220;    int ngxkfmYthL85097730 = -629540294;    int ngxkfmYthL83465658 = -306169884;    int ngxkfmYthL33610286 = -806387010;    int ngxkfmYthL76736736 = -161540233;    int ngxkfmYthL64593233 = -124776062;    int ngxkfmYthL24199347 = -283138822;    int ngxkfmYthL96705698 = -153163890;    int ngxkfmYthL37653359 = -346548927;    int ngxkfmYthL31274392 = -714540505;    int ngxkfmYthL15506109 = -413893078;    int ngxkfmYthL94258099 = -745479382;    int ngxkfmYthL90442101 = -535212433;    int ngxkfmYthL43105888 = 21679763;    int ngxkfmYthL94654704 = -461450945;    int ngxkfmYthL12325792 = -92226232;    int ngxkfmYthL3980866 = -243504331;    int ngxkfmYthL54918101 = 22646630;    int ngxkfmYthL4356462 = -991715405;    int ngxkfmYthL21213221 = -651524158;    int ngxkfmYthL49320002 = -942885972;    int ngxkfmYthL75117312 = -561693235;    int ngxkfmYthL6452494 = -654161738;    int ngxkfmYthL48106233 = -834687587;    int ngxkfmYthL41205629 = -433694684;    int ngxkfmYthL93651820 = -491908065;    int ngxkfmYthL43982721 = -843672530;    int ngxkfmYthL28387040 = -8986104;    int ngxkfmYthL95769854 = -922554586;    int ngxkfmYthL89679962 = 47176823;    int ngxkfmYthL97863323 = -559043212;    int ngxkfmYthL41346123 = -875994314;    int ngxkfmYthL87058038 = -582301727;    int ngxkfmYthL64594346 = -865076502;    int ngxkfmYthL284257 = -284518317;    int ngxkfmYthL10908820 = -995074694;    int ngxkfmYthL747521 = -174888246;    int ngxkfmYthL41086967 = -896173283;    int ngxkfmYthL23955422 = -650812115;    int ngxkfmYthL8963242 = -255801173;    int ngxkfmYthL60913817 = -597815271;    int ngxkfmYthL5084433 = 11809703;    int ngxkfmYthL55908732 = -912557163;    int ngxkfmYthL50945496 = -246972450;    int ngxkfmYthL55000223 = -180381398;    int ngxkfmYthL41680951 = -899621199;    int ngxkfmYthL59525910 = -745518522;    int ngxkfmYthL5897096 = -386174351;    int ngxkfmYthL1347066 = 11026515;    int ngxkfmYthL87801886 = -935893757;    int ngxkfmYthL42459711 = 59620946;    int ngxkfmYthL96131876 = -877012158;    int ngxkfmYthL72378321 = -920995904;    int ngxkfmYthL19704047 = -55597138;    int ngxkfmYthL70186432 = -167871417;    int ngxkfmYthL72491027 = 40983014;    int ngxkfmYthL78645236 = -975378556;    int ngxkfmYthL35359425 = -471482297;    int ngxkfmYthL92404657 = -272692326;    int ngxkfmYthL83084915 = -669632169;    int ngxkfmYthL20610513 = -281103533;    int ngxkfmYthL95812307 = -174152718;    int ngxkfmYthL935844 = -230609305;    int ngxkfmYthL47973397 = -293725750;    int ngxkfmYthL33411068 = -55497294;    int ngxkfmYthL74159985 = -537898765;    int ngxkfmYthL7200062 = -63177656;    int ngxkfmYthL25847755 = -670135932;    int ngxkfmYthL42821631 = -693801920;    int ngxkfmYthL83745885 = -466376251;    int ngxkfmYthL11578271 = -917337986;    int ngxkfmYthL62893898 = -347331049;    int ngxkfmYthL30962679 = -326541255;    int ngxkfmYthL95393220 = -635914232;    int ngxkfmYthL60299403 = 46291112;    int ngxkfmYthL44235569 = -854695675;    int ngxkfmYthL19208580 = -649136072;    int ngxkfmYthL55506998 = -307189288;    int ngxkfmYthL93106010 = -554306190;    int ngxkfmYthL99524678 = -534073486;    int ngxkfmYthL34125911 = -746389544;    int ngxkfmYthL38085625 = -357498179;    int ngxkfmYthL27039974 = -912352347;     ngxkfmYthL692159 = ngxkfmYthL73556474;     ngxkfmYthL73556474 = ngxkfmYthL85113164;     ngxkfmYthL85113164 = ngxkfmYthL1790132;     ngxkfmYthL1790132 = ngxkfmYthL93562091;     ngxkfmYthL93562091 = ngxkfmYthL82219887;     ngxkfmYthL82219887 = ngxkfmYthL70506331;     ngxkfmYthL70506331 = ngxkfmYthL35939051;     ngxkfmYthL35939051 = ngxkfmYthL49968011;     ngxkfmYthL49968011 = ngxkfmYthL49002983;     ngxkfmYthL49002983 = ngxkfmYthL96001770;     ngxkfmYthL96001770 = ngxkfmYthL127678;     ngxkfmYthL127678 = ngxkfmYthL46440577;     ngxkfmYthL46440577 = ngxkfmYthL51049978;     ngxkfmYthL51049978 = ngxkfmYthL76734783;     ngxkfmYthL76734783 = ngxkfmYthL40917268;     ngxkfmYthL40917268 = ngxkfmYthL19506435;     ngxkfmYthL19506435 = ngxkfmYthL47608339;     ngxkfmYthL47608339 = ngxkfmYthL85097730;     ngxkfmYthL85097730 = ngxkfmYthL83465658;     ngxkfmYthL83465658 = ngxkfmYthL33610286;     ngxkfmYthL33610286 = ngxkfmYthL76736736;     ngxkfmYthL76736736 = ngxkfmYthL64593233;     ngxkfmYthL64593233 = ngxkfmYthL24199347;     ngxkfmYthL24199347 = ngxkfmYthL96705698;     ngxkfmYthL96705698 = ngxkfmYthL37653359;     ngxkfmYthL37653359 = ngxkfmYthL31274392;     ngxkfmYthL31274392 = ngxkfmYthL15506109;     ngxkfmYthL15506109 = ngxkfmYthL94258099;     ngxkfmYthL94258099 = ngxkfmYthL90442101;     ngxkfmYthL90442101 = ngxkfmYthL43105888;     ngxkfmYthL43105888 = ngxkfmYthL94654704;     ngxkfmYthL94654704 = ngxkfmYthL12325792;     ngxkfmYthL12325792 = ngxkfmYthL3980866;     ngxkfmYthL3980866 = ngxkfmYthL54918101;     ngxkfmYthL54918101 = ngxkfmYthL4356462;     ngxkfmYthL4356462 = ngxkfmYthL21213221;     ngxkfmYthL21213221 = ngxkfmYthL49320002;     ngxkfmYthL49320002 = ngxkfmYthL75117312;     ngxkfmYthL75117312 = ngxkfmYthL6452494;     ngxkfmYthL6452494 = ngxkfmYthL48106233;     ngxkfmYthL48106233 = ngxkfmYthL41205629;     ngxkfmYthL41205629 = ngxkfmYthL93651820;     ngxkfmYthL93651820 = ngxkfmYthL43982721;     ngxkfmYthL43982721 = ngxkfmYthL28387040;     ngxkfmYthL28387040 = ngxkfmYthL95769854;     ngxkfmYthL95769854 = ngxkfmYthL89679962;     ngxkfmYthL89679962 = ngxkfmYthL97863323;     ngxkfmYthL97863323 = ngxkfmYthL41346123;     ngxkfmYthL41346123 = ngxkfmYthL87058038;     ngxkfmYthL87058038 = ngxkfmYthL64594346;     ngxkfmYthL64594346 = ngxkfmYthL284257;     ngxkfmYthL284257 = ngxkfmYthL10908820;     ngxkfmYthL10908820 = ngxkfmYthL747521;     ngxkfmYthL747521 = ngxkfmYthL41086967;     ngxkfmYthL41086967 = ngxkfmYthL23955422;     ngxkfmYthL23955422 = ngxkfmYthL8963242;     ngxkfmYthL8963242 = ngxkfmYthL60913817;     ngxkfmYthL60913817 = ngxkfmYthL5084433;     ngxkfmYthL5084433 = ngxkfmYthL55908732;     ngxkfmYthL55908732 = ngxkfmYthL50945496;     ngxkfmYthL50945496 = ngxkfmYthL55000223;     ngxkfmYthL55000223 = ngxkfmYthL41680951;     ngxkfmYthL41680951 = ngxkfmYthL59525910;     ngxkfmYthL59525910 = ngxkfmYthL5897096;     ngxkfmYthL5897096 = ngxkfmYthL1347066;     ngxkfmYthL1347066 = ngxkfmYthL87801886;     ngxkfmYthL87801886 = ngxkfmYthL42459711;     ngxkfmYthL42459711 = ngxkfmYthL96131876;     ngxkfmYthL96131876 = ngxkfmYthL72378321;     ngxkfmYthL72378321 = ngxkfmYthL19704047;     ngxkfmYthL19704047 = ngxkfmYthL70186432;     ngxkfmYthL70186432 = ngxkfmYthL72491027;     ngxkfmYthL72491027 = ngxkfmYthL78645236;     ngxkfmYthL78645236 = ngxkfmYthL35359425;     ngxkfmYthL35359425 = ngxkfmYthL92404657;     ngxkfmYthL92404657 = ngxkfmYthL83084915;     ngxkfmYthL83084915 = ngxkfmYthL20610513;     ngxkfmYthL20610513 = ngxkfmYthL95812307;     ngxkfmYthL95812307 = ngxkfmYthL935844;     ngxkfmYthL935844 = ngxkfmYthL47973397;     ngxkfmYthL47973397 = ngxkfmYthL33411068;     ngxkfmYthL33411068 = ngxkfmYthL74159985;     ngxkfmYthL74159985 = ngxkfmYthL7200062;     ngxkfmYthL7200062 = ngxkfmYthL25847755;     ngxkfmYthL25847755 = ngxkfmYthL42821631;     ngxkfmYthL42821631 = ngxkfmYthL83745885;     ngxkfmYthL83745885 = ngxkfmYthL11578271;     ngxkfmYthL11578271 = ngxkfmYthL62893898;     ngxkfmYthL62893898 = ngxkfmYthL30962679;     ngxkfmYthL30962679 = ngxkfmYthL95393220;     ngxkfmYthL95393220 = ngxkfmYthL60299403;     ngxkfmYthL60299403 = ngxkfmYthL44235569;     ngxkfmYthL44235569 = ngxkfmYthL19208580;     ngxkfmYthL19208580 = ngxkfmYthL55506998;     ngxkfmYthL55506998 = ngxkfmYthL93106010;     ngxkfmYthL93106010 = ngxkfmYthL99524678;     ngxkfmYthL99524678 = ngxkfmYthL34125911;     ngxkfmYthL34125911 = ngxkfmYthL38085625;     ngxkfmYthL38085625 = ngxkfmYthL27039974;     ngxkfmYthL27039974 = ngxkfmYthL692159;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FkDerDfQPk92910594() {     int cuIddOrRhr84136463 = -693018028;    int cuIddOrRhr90428563 = -479959422;    int cuIddOrRhr81521670 = -774479320;    int cuIddOrRhr64582383 = -435154023;    int cuIddOrRhr99683415 = -65973581;    int cuIddOrRhr80724288 = -840584608;    int cuIddOrRhr41545650 = -434185790;    int cuIddOrRhr50286493 = -632258610;    int cuIddOrRhr8497713 = -622190305;    int cuIddOrRhr81548635 = -932870699;    int cuIddOrRhr45417709 = -740540301;    int cuIddOrRhr63333077 = -15738234;    int cuIddOrRhr64558353 = -75544625;    int cuIddOrRhr30676870 = -742655884;    int cuIddOrRhr37589247 = 93265709;    int cuIddOrRhr27306430 = -595200621;    int cuIddOrRhr13258320 = -964102415;    int cuIddOrRhr28459786 = -868015097;    int cuIddOrRhr47537872 = -628215164;    int cuIddOrRhr72833101 = -835755876;    int cuIddOrRhr89250673 = -395916041;    int cuIddOrRhr88884054 = -152692003;    int cuIddOrRhr68418941 = -164665259;    int cuIddOrRhr58905904 = -354964218;    int cuIddOrRhr38281657 = -712816649;    int cuIddOrRhr32995508 = -227058643;    int cuIddOrRhr80729131 = -342624848;    int cuIddOrRhr55237734 = -483847962;    int cuIddOrRhr30339194 = -607984524;    int cuIddOrRhr6275443 = 51937715;    int cuIddOrRhr61201201 = -716077438;    int cuIddOrRhr57761132 = -376646024;    int cuIddOrRhr55864678 = -813098409;    int cuIddOrRhr46139376 = -935106501;    int cuIddOrRhr33644831 = -175018105;    int cuIddOrRhr16745646 = -779739311;    int cuIddOrRhr89827399 = -530373338;    int cuIddOrRhr89898833 = -651356492;    int cuIddOrRhr24112390 = -746591033;    int cuIddOrRhr55947629 = -406755261;    int cuIddOrRhr81755157 = -406236561;    int cuIddOrRhr91693415 = 92396994;    int cuIddOrRhr17218891 = 57034692;    int cuIddOrRhr35887661 = -599855301;    int cuIddOrRhr20331536 = -837143058;    int cuIddOrRhr35375851 = -942165510;    int cuIddOrRhr65695393 = -85127407;    int cuIddOrRhr17533919 = -557764930;    int cuIddOrRhr72193235 = -361596397;    int cuIddOrRhr22390523 = -789322182;    int cuIddOrRhr37851794 = 14022076;    int cuIddOrRhr55785051 = -342777629;    int cuIddOrRhr25021279 = -106822010;    int cuIddOrRhr25424280 = -993301633;    int cuIddOrRhr3659922 = -144659406;    int cuIddOrRhr95252409 = -440326025;    int cuIddOrRhr22009623 = -215294163;    int cuIddOrRhr22615766 = -319515103;    int cuIddOrRhr26300726 = -722337374;    int cuIddOrRhr66687907 = -838914939;    int cuIddOrRhr99995156 = -397959761;    int cuIddOrRhr86307915 = -950337828;    int cuIddOrRhr19947299 = 75725914;    int cuIddOrRhr2222270 = -574128021;    int cuIddOrRhr20347434 = -116793262;    int cuIddOrRhr87656576 = -263894278;    int cuIddOrRhr7468399 = -202639826;    int cuIddOrRhr18418977 = -140438125;    int cuIddOrRhr97032038 = -467637779;    int cuIddOrRhr20843601 = -126994980;    int cuIddOrRhr37479030 = 35172716;    int cuIddOrRhr23359486 = -212745924;    int cuIddOrRhr4347396 = -21424064;    int cuIddOrRhr91590243 = -121459904;    int cuIddOrRhr91077943 = -329519316;    int cuIddOrRhr97557258 = -388313036;    int cuIddOrRhr71665164 = -109726696;    int cuIddOrRhr32531280 = -564809959;    int cuIddOrRhr38574369 = -517821161;    int cuIddOrRhr2905807 = -770651140;    int cuIddOrRhr67300115 = -41931236;    int cuIddOrRhr63195213 = -784859918;    int cuIddOrRhr83044498 = -22251566;    int cuIddOrRhr7948672 = -818662343;    int cuIddOrRhr68423648 = -962084361;    int cuIddOrRhr5416151 = -273299809;    int cuIddOrRhr32739853 = -169824015;    int cuIddOrRhr30440399 = -819796776;    int cuIddOrRhr42479455 = -690447096;    int cuIddOrRhr38392422 = -734692080;    int cuIddOrRhr94736023 = -464445148;    int cuIddOrRhr67211633 = -110858236;    int cuIddOrRhr63598108 = -929019118;    int cuIddOrRhr57424483 = -907676095;    int cuIddOrRhr55952472 = 91204500;    int cuIddOrRhr95447242 = -455898733;    int cuIddOrRhr71746116 = -983328920;    int cuIddOrRhr14996621 = -368837287;    int cuIddOrRhr15540227 = -383062039;    int cuIddOrRhr32674960 = -693018028;     cuIddOrRhr84136463 = cuIddOrRhr90428563;     cuIddOrRhr90428563 = cuIddOrRhr81521670;     cuIddOrRhr81521670 = cuIddOrRhr64582383;     cuIddOrRhr64582383 = cuIddOrRhr99683415;     cuIddOrRhr99683415 = cuIddOrRhr80724288;     cuIddOrRhr80724288 = cuIddOrRhr41545650;     cuIddOrRhr41545650 = cuIddOrRhr50286493;     cuIddOrRhr50286493 = cuIddOrRhr8497713;     cuIddOrRhr8497713 = cuIddOrRhr81548635;     cuIddOrRhr81548635 = cuIddOrRhr45417709;     cuIddOrRhr45417709 = cuIddOrRhr63333077;     cuIddOrRhr63333077 = cuIddOrRhr64558353;     cuIddOrRhr64558353 = cuIddOrRhr30676870;     cuIddOrRhr30676870 = cuIddOrRhr37589247;     cuIddOrRhr37589247 = cuIddOrRhr27306430;     cuIddOrRhr27306430 = cuIddOrRhr13258320;     cuIddOrRhr13258320 = cuIddOrRhr28459786;     cuIddOrRhr28459786 = cuIddOrRhr47537872;     cuIddOrRhr47537872 = cuIddOrRhr72833101;     cuIddOrRhr72833101 = cuIddOrRhr89250673;     cuIddOrRhr89250673 = cuIddOrRhr88884054;     cuIddOrRhr88884054 = cuIddOrRhr68418941;     cuIddOrRhr68418941 = cuIddOrRhr58905904;     cuIddOrRhr58905904 = cuIddOrRhr38281657;     cuIddOrRhr38281657 = cuIddOrRhr32995508;     cuIddOrRhr32995508 = cuIddOrRhr80729131;     cuIddOrRhr80729131 = cuIddOrRhr55237734;     cuIddOrRhr55237734 = cuIddOrRhr30339194;     cuIddOrRhr30339194 = cuIddOrRhr6275443;     cuIddOrRhr6275443 = cuIddOrRhr61201201;     cuIddOrRhr61201201 = cuIddOrRhr57761132;     cuIddOrRhr57761132 = cuIddOrRhr55864678;     cuIddOrRhr55864678 = cuIddOrRhr46139376;     cuIddOrRhr46139376 = cuIddOrRhr33644831;     cuIddOrRhr33644831 = cuIddOrRhr16745646;     cuIddOrRhr16745646 = cuIddOrRhr89827399;     cuIddOrRhr89827399 = cuIddOrRhr89898833;     cuIddOrRhr89898833 = cuIddOrRhr24112390;     cuIddOrRhr24112390 = cuIddOrRhr55947629;     cuIddOrRhr55947629 = cuIddOrRhr81755157;     cuIddOrRhr81755157 = cuIddOrRhr91693415;     cuIddOrRhr91693415 = cuIddOrRhr17218891;     cuIddOrRhr17218891 = cuIddOrRhr35887661;     cuIddOrRhr35887661 = cuIddOrRhr20331536;     cuIddOrRhr20331536 = cuIddOrRhr35375851;     cuIddOrRhr35375851 = cuIddOrRhr65695393;     cuIddOrRhr65695393 = cuIddOrRhr17533919;     cuIddOrRhr17533919 = cuIddOrRhr72193235;     cuIddOrRhr72193235 = cuIddOrRhr22390523;     cuIddOrRhr22390523 = cuIddOrRhr37851794;     cuIddOrRhr37851794 = cuIddOrRhr55785051;     cuIddOrRhr55785051 = cuIddOrRhr25021279;     cuIddOrRhr25021279 = cuIddOrRhr25424280;     cuIddOrRhr25424280 = cuIddOrRhr3659922;     cuIddOrRhr3659922 = cuIddOrRhr95252409;     cuIddOrRhr95252409 = cuIddOrRhr22009623;     cuIddOrRhr22009623 = cuIddOrRhr22615766;     cuIddOrRhr22615766 = cuIddOrRhr26300726;     cuIddOrRhr26300726 = cuIddOrRhr66687907;     cuIddOrRhr66687907 = cuIddOrRhr99995156;     cuIddOrRhr99995156 = cuIddOrRhr86307915;     cuIddOrRhr86307915 = cuIddOrRhr19947299;     cuIddOrRhr19947299 = cuIddOrRhr2222270;     cuIddOrRhr2222270 = cuIddOrRhr20347434;     cuIddOrRhr20347434 = cuIddOrRhr87656576;     cuIddOrRhr87656576 = cuIddOrRhr7468399;     cuIddOrRhr7468399 = cuIddOrRhr18418977;     cuIddOrRhr18418977 = cuIddOrRhr97032038;     cuIddOrRhr97032038 = cuIddOrRhr20843601;     cuIddOrRhr20843601 = cuIddOrRhr37479030;     cuIddOrRhr37479030 = cuIddOrRhr23359486;     cuIddOrRhr23359486 = cuIddOrRhr4347396;     cuIddOrRhr4347396 = cuIddOrRhr91590243;     cuIddOrRhr91590243 = cuIddOrRhr91077943;     cuIddOrRhr91077943 = cuIddOrRhr97557258;     cuIddOrRhr97557258 = cuIddOrRhr71665164;     cuIddOrRhr71665164 = cuIddOrRhr32531280;     cuIddOrRhr32531280 = cuIddOrRhr38574369;     cuIddOrRhr38574369 = cuIddOrRhr2905807;     cuIddOrRhr2905807 = cuIddOrRhr67300115;     cuIddOrRhr67300115 = cuIddOrRhr63195213;     cuIddOrRhr63195213 = cuIddOrRhr83044498;     cuIddOrRhr83044498 = cuIddOrRhr7948672;     cuIddOrRhr7948672 = cuIddOrRhr68423648;     cuIddOrRhr68423648 = cuIddOrRhr5416151;     cuIddOrRhr5416151 = cuIddOrRhr32739853;     cuIddOrRhr32739853 = cuIddOrRhr30440399;     cuIddOrRhr30440399 = cuIddOrRhr42479455;     cuIddOrRhr42479455 = cuIddOrRhr38392422;     cuIddOrRhr38392422 = cuIddOrRhr94736023;     cuIddOrRhr94736023 = cuIddOrRhr67211633;     cuIddOrRhr67211633 = cuIddOrRhr63598108;     cuIddOrRhr63598108 = cuIddOrRhr57424483;     cuIddOrRhr57424483 = cuIddOrRhr55952472;     cuIddOrRhr55952472 = cuIddOrRhr95447242;     cuIddOrRhr95447242 = cuIddOrRhr71746116;     cuIddOrRhr71746116 = cuIddOrRhr14996621;     cuIddOrRhr14996621 = cuIddOrRhr15540227;     cuIddOrRhr15540227 = cuIddOrRhr32674960;     cuIddOrRhr32674960 = cuIddOrRhr84136463;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MHTUBiYJVS63366468() {     int qSNFmdvzSs96922630 = -828058720;    int qSNFmdvzSs77199451 = -650293436;    int qSNFmdvzSs99274470 = -82838702;    int qSNFmdvzSs72523208 = -259356300;    int qSNFmdvzSs93194519 = -557790046;    int qSNFmdvzSs24034359 = -617507921;    int qSNFmdvzSs62753407 = -979857860;    int qSNFmdvzSs77005606 = 23223133;    int qSNFmdvzSs3902523 = -583948398;    int qSNFmdvzSs87767340 = -736958332;    int qSNFmdvzSs72905090 = -855226317;    int qSNFmdvzSs95283581 = -523071050;    int qSNFmdvzSs34701656 = 20672717;    int qSNFmdvzSs80300948 = 24928918;    int qSNFmdvzSs60895800 = -913720122;    int qSNFmdvzSs55731498 = -46092011;    int qSNFmdvzSs69010137 = -627876882;    int qSNFmdvzSs12018271 = -423225399;    int qSNFmdvzSs48698502 = -537684360;    int qSNFmdvzSs75487597 = -335000015;    int qSNFmdvzSs69424730 = -887232306;    int qSNFmdvzSs17931874 = -322550351;    int qSNFmdvzSs7179237 = -95669154;    int qSNFmdvzSs3413035 = -715309230;    int qSNFmdvzSs44175916 = -923258574;    int qSNFmdvzSs14314154 = 82510158;    int qSNFmdvzSs92144667 = -976331128;    int qSNFmdvzSs22590659 = -657646067;    int qSNFmdvzSs46956247 = -822201764;    int qSNFmdvzSs59026558 = -677948439;    int qSNFmdvzSs5637198 = -992494188;    int qSNFmdvzSs61079955 = -355365703;    int qSNFmdvzSs1910628 = -688953194;    int qSNFmdvzSs23215750 = -839860720;    int qSNFmdvzSs16705364 = -785049985;    int qSNFmdvzSs11969761 = -568302415;    int qSNFmdvzSs26591460 = -320282689;    int qSNFmdvzSs47602857 = -570976583;    int qSNFmdvzSs29887776 = -588441610;    int qSNFmdvzSs82212358 = -251172106;    int qSNFmdvzSs43186466 = -781710554;    int qSNFmdvzSs99395871 = -905914340;    int qSNFmdvzSs52102360 = 33601247;    int qSNFmdvzSs80516198 = -941027543;    int qSNFmdvzSs340027 = -729913018;    int qSNFmdvzSs9003980 = 84516051;    int qSNFmdvzSs43277413 = -582642459;    int qSNFmdvzSs46973885 = -713117710;    int qSNFmdvzSs16257285 = -525198371;    int qSNFmdvzSs69340943 = -300358645;    int qSNFmdvzSs56179287 = -238862232;    int qSNFmdvzSs83961598 = -813962967;    int qSNFmdvzSs59506429 = -282825293;    int qSNFmdvzSs70124004 = -582753375;    int qSNFmdvzSs93941737 = -139103893;    int qSNFmdvzSs78990757 = -405508369;    int qSNFmdvzSs70020215 = -454624282;    int qSNFmdvzSs95861436 = -367529472;    int qSNFmdvzSs28347292 = -336097727;    int qSNFmdvzSs78880366 = -540300205;    int qSNFmdvzSs31889691 = -641176794;    int qSNFmdvzSs40162749 = -222211794;    int qSNFmdvzSs30049359 = -154575103;    int qSNFmdvzSs44875965 = -905999959;    int qSNFmdvzSs82130143 = -744464145;    int qSNFmdvzSs11825136 = -399860614;    int qSNFmdvzSs93372954 = -834117857;    int qSNFmdvzSs11485907 = -139466563;    int qSNFmdvzSs63595585 = -190021097;    int qSNFmdvzSs48926039 = -245417707;    int qSNFmdvzSs29140038 = -725809322;    int qSNFmdvzSs21407280 = 43099701;    int qSNFmdvzSs82130494 = -834783789;    int qSNFmdvzSs66486144 = -186512254;    int qSNFmdvzSs32301132 = -553289462;    int qSNFmdvzSs70028859 = -981317966;    int qSNFmdvzSs65829513 = -256151599;    int qSNFmdvzSs26663038 = -154641611;    int qSNFmdvzSs3073008 = -985396213;    int qSNFmdvzSs35171936 = -907774625;    int qSNFmdvzSs71036741 = -334847383;    int qSNFmdvzSs45170783 = -163213418;    int qSNFmdvzSs6333375 = -32447696;    int qSNFmdvzSs77615304 = -421843120;    int qSNFmdvzSs2847271 = -339086207;    int qSNFmdvzSs21675599 = -78531221;    int qSNFmdvzSs1573527 = 27459589;    int qSNFmdvzSs31786623 = -6199819;    int qSNFmdvzSs29274012 = -600756827;    int qSNFmdvzSs37714606 = -279541617;    int qSNFmdvzSs41949546 = -13678133;    int qSNFmdvzSs30730024 = -952753217;    int qSNFmdvzSs19255565 = -134878857;    int qSNFmdvzSs51007410 = 51858594;    int qSNFmdvzSs50322668 = -609995312;    int qSNFmdvzSs3023717 = -459498761;    int qSNFmdvzSs69346512 = -651339238;    int qSNFmdvzSs7226396 = -60398794;    int qSNFmdvzSs98386055 = -96563399;    int qSNFmdvzSs88514891 = -828058720;     qSNFmdvzSs96922630 = qSNFmdvzSs77199451;     qSNFmdvzSs77199451 = qSNFmdvzSs99274470;     qSNFmdvzSs99274470 = qSNFmdvzSs72523208;     qSNFmdvzSs72523208 = qSNFmdvzSs93194519;     qSNFmdvzSs93194519 = qSNFmdvzSs24034359;     qSNFmdvzSs24034359 = qSNFmdvzSs62753407;     qSNFmdvzSs62753407 = qSNFmdvzSs77005606;     qSNFmdvzSs77005606 = qSNFmdvzSs3902523;     qSNFmdvzSs3902523 = qSNFmdvzSs87767340;     qSNFmdvzSs87767340 = qSNFmdvzSs72905090;     qSNFmdvzSs72905090 = qSNFmdvzSs95283581;     qSNFmdvzSs95283581 = qSNFmdvzSs34701656;     qSNFmdvzSs34701656 = qSNFmdvzSs80300948;     qSNFmdvzSs80300948 = qSNFmdvzSs60895800;     qSNFmdvzSs60895800 = qSNFmdvzSs55731498;     qSNFmdvzSs55731498 = qSNFmdvzSs69010137;     qSNFmdvzSs69010137 = qSNFmdvzSs12018271;     qSNFmdvzSs12018271 = qSNFmdvzSs48698502;     qSNFmdvzSs48698502 = qSNFmdvzSs75487597;     qSNFmdvzSs75487597 = qSNFmdvzSs69424730;     qSNFmdvzSs69424730 = qSNFmdvzSs17931874;     qSNFmdvzSs17931874 = qSNFmdvzSs7179237;     qSNFmdvzSs7179237 = qSNFmdvzSs3413035;     qSNFmdvzSs3413035 = qSNFmdvzSs44175916;     qSNFmdvzSs44175916 = qSNFmdvzSs14314154;     qSNFmdvzSs14314154 = qSNFmdvzSs92144667;     qSNFmdvzSs92144667 = qSNFmdvzSs22590659;     qSNFmdvzSs22590659 = qSNFmdvzSs46956247;     qSNFmdvzSs46956247 = qSNFmdvzSs59026558;     qSNFmdvzSs59026558 = qSNFmdvzSs5637198;     qSNFmdvzSs5637198 = qSNFmdvzSs61079955;     qSNFmdvzSs61079955 = qSNFmdvzSs1910628;     qSNFmdvzSs1910628 = qSNFmdvzSs23215750;     qSNFmdvzSs23215750 = qSNFmdvzSs16705364;     qSNFmdvzSs16705364 = qSNFmdvzSs11969761;     qSNFmdvzSs11969761 = qSNFmdvzSs26591460;     qSNFmdvzSs26591460 = qSNFmdvzSs47602857;     qSNFmdvzSs47602857 = qSNFmdvzSs29887776;     qSNFmdvzSs29887776 = qSNFmdvzSs82212358;     qSNFmdvzSs82212358 = qSNFmdvzSs43186466;     qSNFmdvzSs43186466 = qSNFmdvzSs99395871;     qSNFmdvzSs99395871 = qSNFmdvzSs52102360;     qSNFmdvzSs52102360 = qSNFmdvzSs80516198;     qSNFmdvzSs80516198 = qSNFmdvzSs340027;     qSNFmdvzSs340027 = qSNFmdvzSs9003980;     qSNFmdvzSs9003980 = qSNFmdvzSs43277413;     qSNFmdvzSs43277413 = qSNFmdvzSs46973885;     qSNFmdvzSs46973885 = qSNFmdvzSs16257285;     qSNFmdvzSs16257285 = qSNFmdvzSs69340943;     qSNFmdvzSs69340943 = qSNFmdvzSs56179287;     qSNFmdvzSs56179287 = qSNFmdvzSs83961598;     qSNFmdvzSs83961598 = qSNFmdvzSs59506429;     qSNFmdvzSs59506429 = qSNFmdvzSs70124004;     qSNFmdvzSs70124004 = qSNFmdvzSs93941737;     qSNFmdvzSs93941737 = qSNFmdvzSs78990757;     qSNFmdvzSs78990757 = qSNFmdvzSs70020215;     qSNFmdvzSs70020215 = qSNFmdvzSs95861436;     qSNFmdvzSs95861436 = qSNFmdvzSs28347292;     qSNFmdvzSs28347292 = qSNFmdvzSs78880366;     qSNFmdvzSs78880366 = qSNFmdvzSs31889691;     qSNFmdvzSs31889691 = qSNFmdvzSs40162749;     qSNFmdvzSs40162749 = qSNFmdvzSs30049359;     qSNFmdvzSs30049359 = qSNFmdvzSs44875965;     qSNFmdvzSs44875965 = qSNFmdvzSs82130143;     qSNFmdvzSs82130143 = qSNFmdvzSs11825136;     qSNFmdvzSs11825136 = qSNFmdvzSs93372954;     qSNFmdvzSs93372954 = qSNFmdvzSs11485907;     qSNFmdvzSs11485907 = qSNFmdvzSs63595585;     qSNFmdvzSs63595585 = qSNFmdvzSs48926039;     qSNFmdvzSs48926039 = qSNFmdvzSs29140038;     qSNFmdvzSs29140038 = qSNFmdvzSs21407280;     qSNFmdvzSs21407280 = qSNFmdvzSs82130494;     qSNFmdvzSs82130494 = qSNFmdvzSs66486144;     qSNFmdvzSs66486144 = qSNFmdvzSs32301132;     qSNFmdvzSs32301132 = qSNFmdvzSs70028859;     qSNFmdvzSs70028859 = qSNFmdvzSs65829513;     qSNFmdvzSs65829513 = qSNFmdvzSs26663038;     qSNFmdvzSs26663038 = qSNFmdvzSs3073008;     qSNFmdvzSs3073008 = qSNFmdvzSs35171936;     qSNFmdvzSs35171936 = qSNFmdvzSs71036741;     qSNFmdvzSs71036741 = qSNFmdvzSs45170783;     qSNFmdvzSs45170783 = qSNFmdvzSs6333375;     qSNFmdvzSs6333375 = qSNFmdvzSs77615304;     qSNFmdvzSs77615304 = qSNFmdvzSs2847271;     qSNFmdvzSs2847271 = qSNFmdvzSs21675599;     qSNFmdvzSs21675599 = qSNFmdvzSs1573527;     qSNFmdvzSs1573527 = qSNFmdvzSs31786623;     qSNFmdvzSs31786623 = qSNFmdvzSs29274012;     qSNFmdvzSs29274012 = qSNFmdvzSs37714606;     qSNFmdvzSs37714606 = qSNFmdvzSs41949546;     qSNFmdvzSs41949546 = qSNFmdvzSs30730024;     qSNFmdvzSs30730024 = qSNFmdvzSs19255565;     qSNFmdvzSs19255565 = qSNFmdvzSs51007410;     qSNFmdvzSs51007410 = qSNFmdvzSs50322668;     qSNFmdvzSs50322668 = qSNFmdvzSs3023717;     qSNFmdvzSs3023717 = qSNFmdvzSs69346512;     qSNFmdvzSs69346512 = qSNFmdvzSs7226396;     qSNFmdvzSs7226396 = qSNFmdvzSs98386055;     qSNFmdvzSs98386055 = qSNFmdvzSs88514891;     qSNFmdvzSs88514891 = qSNFmdvzSs96922630;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zkSEwEFmDS81579809() {     int ilPjYVbyGN80366936 = -608724400;    int ilPjYVbyGN94071540 = -649675623;    int ilPjYVbyGN95682977 = -976363930;    int ilPjYVbyGN35315460 = -453156136;    int ilPjYVbyGN99315843 = -364657537;    int ilPjYVbyGN22538760 = -396579575;    int ilPjYVbyGN33792726 = -719769173;    int ilPjYVbyGN91353048 = 36065104;    int ilPjYVbyGN62432223 = -925407749;    int ilPjYVbyGN20312992 = -105334444;    int ilPjYVbyGN22321029 = 54657810;    int ilPjYVbyGN58488981 = -510689297;    int ilPjYVbyGN52819432 = -870988524;    int ilPjYVbyGN59927840 = -863361439;    int ilPjYVbyGN21750263 = 92256896;    int ilPjYVbyGN42120660 = -934171336;    int ilPjYVbyGN62762022 = -381221909;    int ilPjYVbyGN92869717 = -670530275;    int ilPjYVbyGN11138645 = -536359230;    int ilPjYVbyGN64855040 = -864586008;    int ilPjYVbyGN25065117 = -476761336;    int ilPjYVbyGN30079192 = -313702122;    int ilPjYVbyGN11004944 = -135558351;    int ilPjYVbyGN38119592 = -787134626;    int ilPjYVbyGN85751874 = -382911332;    int ilPjYVbyGN9656303 = -897999557;    int ilPjYVbyGN41599408 = -604415470;    int ilPjYVbyGN62322284 = -727600951;    int ilPjYVbyGN83037341 = -684706907;    int ilPjYVbyGN74859899 = -90798291;    int ilPjYVbyGN23732511 = -630251389;    int ilPjYVbyGN24186383 = -270560783;    int ilPjYVbyGN45449514 = -309825371;    int ilPjYVbyGN65374260 = -431462890;    int ilPjYVbyGN95432093 = -982714720;    int ilPjYVbyGN24358945 = -356326321;    int ilPjYVbyGN95205638 = -199131869;    int ilPjYVbyGN88181688 = -279447103;    int ilPjYVbyGN78882854 = -773339408;    int ilPjYVbyGN31707494 = -3765629;    int ilPjYVbyGN76835390 = -353259527;    int ilPjYVbyGN49883657 = -379822662;    int ilPjYVbyGN75669430 = -517455995;    int ilPjYVbyGN72421138 = -697210314;    int ilPjYVbyGN92284523 = -458069971;    int ilPjYVbyGN48609976 = 64905127;    int ilPjYVbyGN19292843 = -714946689;    int ilPjYVbyGN66644480 = -711839428;    int ilPjYVbyGN47104397 = -10800454;    int ilPjYVbyGN4673427 = -507379100;    int ilPjYVbyGN29436736 = -459763655;    int ilPjYVbyGN39462393 = -872222279;    int ilPjYVbyGN73618887 = -494572609;    int ilPjYVbyGN94800763 = -301166763;    int ilPjYVbyGN56514692 = -487590016;    int ilPjYVbyGN50287745 = -195022279;    int ilPjYVbyGN83066596 = -414117273;    int ilPjYVbyGN57563385 = -89229304;    int ilPjYVbyGN49563586 = 29755196;    int ilPjYVbyGN89659541 = -466657980;    int ilPjYVbyGN80939351 = -792164105;    int ilPjYVbyGN71470441 = -992168223;    int ilPjYVbyGN8315708 = -279227990;    int ilPjYVbyGN87572324 = -734609458;    int ilPjYVbyGN96580481 = -475083056;    int ilPjYVbyGN98134646 = -674781407;    int ilPjYVbyGN13039467 = -100863926;    int ilPjYVbyGN87445172 = -339525634;    int ilPjYVbyGN64495747 = -880646719;    int ilPjYVbyGN97391318 = -551416783;    int ilPjYVbyGN46915021 = -635039468;    int ilPjYVbyGN74580333 = -1774806;    int ilPjYVbyGN13986863 = -897190868;    int ilPjYVbyGN79431151 = -432593602;    int ilPjYVbyGN88019649 = -411326481;    int ilPjYVbyGN75181460 = 3061325;    int ilPjYVbyGN54409762 = -796246127;    int ilPjYVbyGN38583806 = -438348037;    int ilPjYVbyGN45835069 = -229064656;    int ilPjYVbyGN37141898 = -347816460;    int ilPjYVbyGN90363459 = -83052869;    int ilPjYVbyGN74954927 = -892576042;    int ilPjYVbyGN15217888 = -616800498;    int ilPjYVbyGN78363914 = -77327807;    int ilPjYVbyGN45423163 = -631034637;    int ilPjYVbyGN84270118 = -758029111;    int ilPjYVbyGN50567495 = -775988174;    int ilPjYVbyGN50648751 = 91341391;    int ilPjYVbyGN8859568 = -943872874;    int ilPjYVbyGN45144349 = -687692442;    int ilPjYVbyGN41292348 = -942209049;    int ilPjYVbyGN37642254 = -9902565;    int ilPjYVbyGN38618103 = -209202300;    int ilPjYVbyGN89223313 = -206681428;    int ilPjYVbyGN50768142 = -211601524;    int ilPjYVbyGN5364949 = -361091305;    int ilPjYVbyGN41567950 = -594672;    int ilPjYVbyGN88097105 = -782846538;    int ilPjYVbyGN75840657 = -122127259;    int ilPjYVbyGN94149877 = -608724400;     ilPjYVbyGN80366936 = ilPjYVbyGN94071540;     ilPjYVbyGN94071540 = ilPjYVbyGN95682977;     ilPjYVbyGN95682977 = ilPjYVbyGN35315460;     ilPjYVbyGN35315460 = ilPjYVbyGN99315843;     ilPjYVbyGN99315843 = ilPjYVbyGN22538760;     ilPjYVbyGN22538760 = ilPjYVbyGN33792726;     ilPjYVbyGN33792726 = ilPjYVbyGN91353048;     ilPjYVbyGN91353048 = ilPjYVbyGN62432223;     ilPjYVbyGN62432223 = ilPjYVbyGN20312992;     ilPjYVbyGN20312992 = ilPjYVbyGN22321029;     ilPjYVbyGN22321029 = ilPjYVbyGN58488981;     ilPjYVbyGN58488981 = ilPjYVbyGN52819432;     ilPjYVbyGN52819432 = ilPjYVbyGN59927840;     ilPjYVbyGN59927840 = ilPjYVbyGN21750263;     ilPjYVbyGN21750263 = ilPjYVbyGN42120660;     ilPjYVbyGN42120660 = ilPjYVbyGN62762022;     ilPjYVbyGN62762022 = ilPjYVbyGN92869717;     ilPjYVbyGN92869717 = ilPjYVbyGN11138645;     ilPjYVbyGN11138645 = ilPjYVbyGN64855040;     ilPjYVbyGN64855040 = ilPjYVbyGN25065117;     ilPjYVbyGN25065117 = ilPjYVbyGN30079192;     ilPjYVbyGN30079192 = ilPjYVbyGN11004944;     ilPjYVbyGN11004944 = ilPjYVbyGN38119592;     ilPjYVbyGN38119592 = ilPjYVbyGN85751874;     ilPjYVbyGN85751874 = ilPjYVbyGN9656303;     ilPjYVbyGN9656303 = ilPjYVbyGN41599408;     ilPjYVbyGN41599408 = ilPjYVbyGN62322284;     ilPjYVbyGN62322284 = ilPjYVbyGN83037341;     ilPjYVbyGN83037341 = ilPjYVbyGN74859899;     ilPjYVbyGN74859899 = ilPjYVbyGN23732511;     ilPjYVbyGN23732511 = ilPjYVbyGN24186383;     ilPjYVbyGN24186383 = ilPjYVbyGN45449514;     ilPjYVbyGN45449514 = ilPjYVbyGN65374260;     ilPjYVbyGN65374260 = ilPjYVbyGN95432093;     ilPjYVbyGN95432093 = ilPjYVbyGN24358945;     ilPjYVbyGN24358945 = ilPjYVbyGN95205638;     ilPjYVbyGN95205638 = ilPjYVbyGN88181688;     ilPjYVbyGN88181688 = ilPjYVbyGN78882854;     ilPjYVbyGN78882854 = ilPjYVbyGN31707494;     ilPjYVbyGN31707494 = ilPjYVbyGN76835390;     ilPjYVbyGN76835390 = ilPjYVbyGN49883657;     ilPjYVbyGN49883657 = ilPjYVbyGN75669430;     ilPjYVbyGN75669430 = ilPjYVbyGN72421138;     ilPjYVbyGN72421138 = ilPjYVbyGN92284523;     ilPjYVbyGN92284523 = ilPjYVbyGN48609976;     ilPjYVbyGN48609976 = ilPjYVbyGN19292843;     ilPjYVbyGN19292843 = ilPjYVbyGN66644480;     ilPjYVbyGN66644480 = ilPjYVbyGN47104397;     ilPjYVbyGN47104397 = ilPjYVbyGN4673427;     ilPjYVbyGN4673427 = ilPjYVbyGN29436736;     ilPjYVbyGN29436736 = ilPjYVbyGN39462393;     ilPjYVbyGN39462393 = ilPjYVbyGN73618887;     ilPjYVbyGN73618887 = ilPjYVbyGN94800763;     ilPjYVbyGN94800763 = ilPjYVbyGN56514692;     ilPjYVbyGN56514692 = ilPjYVbyGN50287745;     ilPjYVbyGN50287745 = ilPjYVbyGN83066596;     ilPjYVbyGN83066596 = ilPjYVbyGN57563385;     ilPjYVbyGN57563385 = ilPjYVbyGN49563586;     ilPjYVbyGN49563586 = ilPjYVbyGN89659541;     ilPjYVbyGN89659541 = ilPjYVbyGN80939351;     ilPjYVbyGN80939351 = ilPjYVbyGN71470441;     ilPjYVbyGN71470441 = ilPjYVbyGN8315708;     ilPjYVbyGN8315708 = ilPjYVbyGN87572324;     ilPjYVbyGN87572324 = ilPjYVbyGN96580481;     ilPjYVbyGN96580481 = ilPjYVbyGN98134646;     ilPjYVbyGN98134646 = ilPjYVbyGN13039467;     ilPjYVbyGN13039467 = ilPjYVbyGN87445172;     ilPjYVbyGN87445172 = ilPjYVbyGN64495747;     ilPjYVbyGN64495747 = ilPjYVbyGN97391318;     ilPjYVbyGN97391318 = ilPjYVbyGN46915021;     ilPjYVbyGN46915021 = ilPjYVbyGN74580333;     ilPjYVbyGN74580333 = ilPjYVbyGN13986863;     ilPjYVbyGN13986863 = ilPjYVbyGN79431151;     ilPjYVbyGN79431151 = ilPjYVbyGN88019649;     ilPjYVbyGN88019649 = ilPjYVbyGN75181460;     ilPjYVbyGN75181460 = ilPjYVbyGN54409762;     ilPjYVbyGN54409762 = ilPjYVbyGN38583806;     ilPjYVbyGN38583806 = ilPjYVbyGN45835069;     ilPjYVbyGN45835069 = ilPjYVbyGN37141898;     ilPjYVbyGN37141898 = ilPjYVbyGN90363459;     ilPjYVbyGN90363459 = ilPjYVbyGN74954927;     ilPjYVbyGN74954927 = ilPjYVbyGN15217888;     ilPjYVbyGN15217888 = ilPjYVbyGN78363914;     ilPjYVbyGN78363914 = ilPjYVbyGN45423163;     ilPjYVbyGN45423163 = ilPjYVbyGN84270118;     ilPjYVbyGN84270118 = ilPjYVbyGN50567495;     ilPjYVbyGN50567495 = ilPjYVbyGN50648751;     ilPjYVbyGN50648751 = ilPjYVbyGN8859568;     ilPjYVbyGN8859568 = ilPjYVbyGN45144349;     ilPjYVbyGN45144349 = ilPjYVbyGN41292348;     ilPjYVbyGN41292348 = ilPjYVbyGN37642254;     ilPjYVbyGN37642254 = ilPjYVbyGN38618103;     ilPjYVbyGN38618103 = ilPjYVbyGN89223313;     ilPjYVbyGN89223313 = ilPjYVbyGN50768142;     ilPjYVbyGN50768142 = ilPjYVbyGN5364949;     ilPjYVbyGN5364949 = ilPjYVbyGN41567950;     ilPjYVbyGN41567950 = ilPjYVbyGN88097105;     ilPjYVbyGN88097105 = ilPjYVbyGN75840657;     ilPjYVbyGN75840657 = ilPjYVbyGN94149877;     ilPjYVbyGN94149877 = ilPjYVbyGN80366936;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YIkseycxOp52035682() {     int SYbFSfktxk93153103 = -743765092;    int SYbFSfktxk80842429 = -820009637;    int SYbFSfktxk13435778 = -284723311;    int SYbFSfktxk43256285 = -277358412;    int SYbFSfktxk92826948 = -856474002;    int SYbFSfktxk65848830 = -173502888;    int SYbFSfktxk55000484 = -165441244;    int SYbFSfktxk18072162 = -408453153;    int SYbFSfktxk57837034 = -887165841;    int SYbFSfktxk26531697 = 90577923;    int SYbFSfktxk49808411 = -60028205;    int SYbFSfktxk90439486 = 81977887;    int SYbFSfktxk22962735 = -774771181;    int SYbFSfktxk9551919 = -95776637;    int SYbFSfktxk45056816 = -914728934;    int SYbFSfktxk70545728 = -385062725;    int SYbFSfktxk18513840 = -44996376;    int SYbFSfktxk76428202 = -225740577;    int SYbFSfktxk12299275 = -445828426;    int SYbFSfktxk67509536 = -363830147;    int SYbFSfktxk5239174 = -968077601;    int SYbFSfktxk59127011 = -483560470;    int SYbFSfktxk49765240 = -66562246;    int SYbFSfktxk82626722 = -47479639;    int SYbFSfktxk91646133 = -593353257;    int SYbFSfktxk90974947 = -588430756;    int SYbFSfktxk53014944 = -138121750;    int SYbFSfktxk29675209 = -901399055;    int SYbFSfktxk99654394 = -898924147;    int SYbFSfktxk27611015 = -820684445;    int SYbFSfktxk68168507 = -906668139;    int SYbFSfktxk27505206 = -249280461;    int SYbFSfktxk91495463 = -185680155;    int SYbFSfktxk42450633 = -336217108;    int SYbFSfktxk78492625 = -492746601;    int SYbFSfktxk19583060 = -144889425;    int SYbFSfktxk31969699 = 10958781;    int SYbFSfktxk45885712 = -199067195;    int SYbFSfktxk84658240 = -615189985;    int SYbFSfktxk57972223 = -948182474;    int SYbFSfktxk38266698 = -728733520;    int SYbFSfktxk57586113 = -278133996;    int SYbFSfktxk10552900 = -540889440;    int SYbFSfktxk17049677 = 61617443;    int SYbFSfktxk72293013 = -350839931;    int SYbFSfktxk22238106 = -8413313;    int SYbFSfktxk96874862 = -112461741;    int SYbFSfktxk96084446 = -867192208;    int SYbFSfktxk91168446 = -174402428;    int SYbFSfktxk51623847 = -18415563;    int SYbFSfktxk47764229 = -712647963;    int SYbFSfktxk67638940 = -243407617;    int SYbFSfktxk8104038 = -670575892;    int SYbFSfktxk39500489 = -990618505;    int SYbFSfktxk46796508 = -482034502;    int SYbFSfktxk34026093 = -160204623;    int SYbFSfktxk31077190 = -653447392;    int SYbFSfktxk30809056 = -137243673;    int SYbFSfktxk51610152 = -684005156;    int SYbFSfktxk1852001 = -168043246;    int SYbFSfktxk12833886 = 64618862;    int SYbFSfktxk25325275 = -264042189;    int SYbFSfktxk18417767 = -509529007;    int SYbFSfktxk30226020 = 33518604;    int SYbFSfktxk58363190 = -2753939;    int SYbFSfktxk22303205 = -810747744;    int SYbFSfktxk98944022 = -732341958;    int SYbFSfktxk80512102 = -338554073;    int SYbFSfktxk31059294 = -603030037;    int SYbFSfktxk25473757 = -669839510;    int SYbFSfktxk38576030 = -296021506;    int SYbFSfktxk72628128 = -845929182;    int SYbFSfktxk91769962 = -610550593;    int SYbFSfktxk54327051 = -497645952;    int SYbFSfktxk29242838 = -635096627;    int SYbFSfktxk47653061 = -589943605;    int SYbFSfktxk48574111 = -942671030;    int SYbFSfktxk32715563 = -28179689;    int SYbFSfktxk10333709 = -696639708;    int SYbFSfktxk69408028 = -484939945;    int SYbFSfktxk94100085 = -375969016;    int SYbFSfktxk56930497 = -270929543;    int SYbFSfktxk38506763 = -626996628;    int SYbFSfktxk48030547 = -780508584;    int SYbFSfktxk79846785 = -8036483;    int SYbFSfktxk529567 = -563260523;    int SYbFSfktxk19401169 = -578704570;    int SYbFSfktxk51994974 = -195061651;    int SYbFSfktxk95654124 = -854182606;    int SYbFSfktxk44466533 = -232541978;    int SYbFSfktxk88505870 = -491442034;    int SYbFSfktxk1160644 = -851797546;    int SYbFSfktxk94275559 = -515062039;    int SYbFSfktxk82806239 = -347146739;    int SYbFSfktxk45138338 = -912801337;    int SYbFSfktxk12941424 = -364691332;    int SYbFSfktxk39168346 = -768604990;    int SYbFSfktxk80326880 = -474408045;    int SYbFSfktxk58686486 = -935628619;    int SYbFSfktxk49989809 = -743765092;     SYbFSfktxk93153103 = SYbFSfktxk80842429;     SYbFSfktxk80842429 = SYbFSfktxk13435778;     SYbFSfktxk13435778 = SYbFSfktxk43256285;     SYbFSfktxk43256285 = SYbFSfktxk92826948;     SYbFSfktxk92826948 = SYbFSfktxk65848830;     SYbFSfktxk65848830 = SYbFSfktxk55000484;     SYbFSfktxk55000484 = SYbFSfktxk18072162;     SYbFSfktxk18072162 = SYbFSfktxk57837034;     SYbFSfktxk57837034 = SYbFSfktxk26531697;     SYbFSfktxk26531697 = SYbFSfktxk49808411;     SYbFSfktxk49808411 = SYbFSfktxk90439486;     SYbFSfktxk90439486 = SYbFSfktxk22962735;     SYbFSfktxk22962735 = SYbFSfktxk9551919;     SYbFSfktxk9551919 = SYbFSfktxk45056816;     SYbFSfktxk45056816 = SYbFSfktxk70545728;     SYbFSfktxk70545728 = SYbFSfktxk18513840;     SYbFSfktxk18513840 = SYbFSfktxk76428202;     SYbFSfktxk76428202 = SYbFSfktxk12299275;     SYbFSfktxk12299275 = SYbFSfktxk67509536;     SYbFSfktxk67509536 = SYbFSfktxk5239174;     SYbFSfktxk5239174 = SYbFSfktxk59127011;     SYbFSfktxk59127011 = SYbFSfktxk49765240;     SYbFSfktxk49765240 = SYbFSfktxk82626722;     SYbFSfktxk82626722 = SYbFSfktxk91646133;     SYbFSfktxk91646133 = SYbFSfktxk90974947;     SYbFSfktxk90974947 = SYbFSfktxk53014944;     SYbFSfktxk53014944 = SYbFSfktxk29675209;     SYbFSfktxk29675209 = SYbFSfktxk99654394;     SYbFSfktxk99654394 = SYbFSfktxk27611015;     SYbFSfktxk27611015 = SYbFSfktxk68168507;     SYbFSfktxk68168507 = SYbFSfktxk27505206;     SYbFSfktxk27505206 = SYbFSfktxk91495463;     SYbFSfktxk91495463 = SYbFSfktxk42450633;     SYbFSfktxk42450633 = SYbFSfktxk78492625;     SYbFSfktxk78492625 = SYbFSfktxk19583060;     SYbFSfktxk19583060 = SYbFSfktxk31969699;     SYbFSfktxk31969699 = SYbFSfktxk45885712;     SYbFSfktxk45885712 = SYbFSfktxk84658240;     SYbFSfktxk84658240 = SYbFSfktxk57972223;     SYbFSfktxk57972223 = SYbFSfktxk38266698;     SYbFSfktxk38266698 = SYbFSfktxk57586113;     SYbFSfktxk57586113 = SYbFSfktxk10552900;     SYbFSfktxk10552900 = SYbFSfktxk17049677;     SYbFSfktxk17049677 = SYbFSfktxk72293013;     SYbFSfktxk72293013 = SYbFSfktxk22238106;     SYbFSfktxk22238106 = SYbFSfktxk96874862;     SYbFSfktxk96874862 = SYbFSfktxk96084446;     SYbFSfktxk96084446 = SYbFSfktxk91168446;     SYbFSfktxk91168446 = SYbFSfktxk51623847;     SYbFSfktxk51623847 = SYbFSfktxk47764229;     SYbFSfktxk47764229 = SYbFSfktxk67638940;     SYbFSfktxk67638940 = SYbFSfktxk8104038;     SYbFSfktxk8104038 = SYbFSfktxk39500489;     SYbFSfktxk39500489 = SYbFSfktxk46796508;     SYbFSfktxk46796508 = SYbFSfktxk34026093;     SYbFSfktxk34026093 = SYbFSfktxk31077190;     SYbFSfktxk31077190 = SYbFSfktxk30809056;     SYbFSfktxk30809056 = SYbFSfktxk51610152;     SYbFSfktxk51610152 = SYbFSfktxk1852001;     SYbFSfktxk1852001 = SYbFSfktxk12833886;     SYbFSfktxk12833886 = SYbFSfktxk25325275;     SYbFSfktxk25325275 = SYbFSfktxk18417767;     SYbFSfktxk18417767 = SYbFSfktxk30226020;     SYbFSfktxk30226020 = SYbFSfktxk58363190;     SYbFSfktxk58363190 = SYbFSfktxk22303205;     SYbFSfktxk22303205 = SYbFSfktxk98944022;     SYbFSfktxk98944022 = SYbFSfktxk80512102;     SYbFSfktxk80512102 = SYbFSfktxk31059294;     SYbFSfktxk31059294 = SYbFSfktxk25473757;     SYbFSfktxk25473757 = SYbFSfktxk38576030;     SYbFSfktxk38576030 = SYbFSfktxk72628128;     SYbFSfktxk72628128 = SYbFSfktxk91769962;     SYbFSfktxk91769962 = SYbFSfktxk54327051;     SYbFSfktxk54327051 = SYbFSfktxk29242838;     SYbFSfktxk29242838 = SYbFSfktxk47653061;     SYbFSfktxk47653061 = SYbFSfktxk48574111;     SYbFSfktxk48574111 = SYbFSfktxk32715563;     SYbFSfktxk32715563 = SYbFSfktxk10333709;     SYbFSfktxk10333709 = SYbFSfktxk69408028;     SYbFSfktxk69408028 = SYbFSfktxk94100085;     SYbFSfktxk94100085 = SYbFSfktxk56930497;     SYbFSfktxk56930497 = SYbFSfktxk38506763;     SYbFSfktxk38506763 = SYbFSfktxk48030547;     SYbFSfktxk48030547 = SYbFSfktxk79846785;     SYbFSfktxk79846785 = SYbFSfktxk529567;     SYbFSfktxk529567 = SYbFSfktxk19401169;     SYbFSfktxk19401169 = SYbFSfktxk51994974;     SYbFSfktxk51994974 = SYbFSfktxk95654124;     SYbFSfktxk95654124 = SYbFSfktxk44466533;     SYbFSfktxk44466533 = SYbFSfktxk88505870;     SYbFSfktxk88505870 = SYbFSfktxk1160644;     SYbFSfktxk1160644 = SYbFSfktxk94275559;     SYbFSfktxk94275559 = SYbFSfktxk82806239;     SYbFSfktxk82806239 = SYbFSfktxk45138338;     SYbFSfktxk45138338 = SYbFSfktxk12941424;     SYbFSfktxk12941424 = SYbFSfktxk39168346;     SYbFSfktxk39168346 = SYbFSfktxk80326880;     SYbFSfktxk80326880 = SYbFSfktxk58686486;     SYbFSfktxk58686486 = SYbFSfktxk49989809;     SYbFSfktxk49989809 = SYbFSfktxk93153103;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DaCtDdmSCv70249024() {     int kuNZfRcDzr76597409 = -524430773;    int kuNZfRcDzr97714517 = -819391825;    int kuNZfRcDzr9844285 = -78248539;    int kuNZfRcDzr6048537 = -471158248;    int kuNZfRcDzr98948271 = -663341493;    int kuNZfRcDzr64353231 = 47425459;    int kuNZfRcDzr26039802 = 94647443;    int kuNZfRcDzr32419604 = -395611182;    int kuNZfRcDzr16366735 = -128625192;    int kuNZfRcDzr59077349 = -377798189;    int kuNZfRcDzr99224348 = -250144078;    int kuNZfRcDzr53644885 = 94359641;    int kuNZfRcDzr41080511 = -566432422;    int kuNZfRcDzr89178811 = -984066994;    int kuNZfRcDzr5911280 = 91248083;    int kuNZfRcDzr56934890 = -173142050;    int kuNZfRcDzr12265725 = -898341403;    int kuNZfRcDzr57279649 = -473045454;    int kuNZfRcDzr74739417 = -444503297;    int kuNZfRcDzr56876979 = -893416139;    int kuNZfRcDzr60879561 = -557606632;    int kuNZfRcDzr71274329 = -474712240;    int kuNZfRcDzr53590947 = -106451443;    int kuNZfRcDzr17333280 = -119305035;    int kuNZfRcDzr33222092 = -53006016;    int kuNZfRcDzr86317096 = -468940472;    int kuNZfRcDzr2469685 = -866206093;    int kuNZfRcDzr69406834 = -971353939;    int kuNZfRcDzr35735488 = -761429289;    int kuNZfRcDzr43444356 = -233534297;    int kuNZfRcDzr86263820 = -544425340;    int kuNZfRcDzr90611633 = -164475541;    int kuNZfRcDzr35034350 = -906552333;    int kuNZfRcDzr84609143 = 72180722;    int kuNZfRcDzr57219356 = -690411336;    int kuNZfRcDzr31972244 = 67086669;    int kuNZfRcDzr583878 = -967890399;    int kuNZfRcDzr86464543 = 92462285;    int kuNZfRcDzr33653319 = -800087783;    int kuNZfRcDzr7467359 = -700775996;    int kuNZfRcDzr71915622 = -300282494;    int kuNZfRcDzr8073900 = -852042317;    int kuNZfRcDzr34119970 = 8053317;    int kuNZfRcDzr8954617 = -794565328;    int kuNZfRcDzr64237510 = -78996885;    int kuNZfRcDzr61844101 = -28024236;    int kuNZfRcDzr72890292 = -244765970;    int kuNZfRcDzr15755043 = -865913927;    int kuNZfRcDzr22015559 = -760004511;    int kuNZfRcDzr86956331 = -225436018;    int kuNZfRcDzr21021677 = -933549385;    int kuNZfRcDzr23139735 = -301666928;    int kuNZfRcDzr22216497 = -882323208;    int kuNZfRcDzr64177248 = -709031892;    int kuNZfRcDzr9369463 = -830520625;    int kuNZfRcDzr5323080 = 50281466;    int kuNZfRcDzr44123571 = -612940382;    int kuNZfRcDzr92511004 = -958943505;    int kuNZfRcDzr72826445 = -318152233;    int kuNZfRcDzr12631176 = -94401021;    int kuNZfRcDzr61883547 = -86368449;    int kuNZfRcDzr56632967 = 66001382;    int kuNZfRcDzr96684115 = -634181894;    int kuNZfRcDzr72922379 = -895090895;    int kuNZfRcDzr72813528 = -833372850;    int kuNZfRcDzr8612716 = 14331463;    int kuNZfRcDzr18610535 = 911973;    int kuNZfRcDzr56471367 = -538613144;    int kuNZfRcDzr31959455 = -193655659;    int kuNZfRcDzr73939035 = -975838586;    int kuNZfRcDzr56351012 = -205251652;    int kuNZfRcDzr25801181 = -890803689;    int kuNZfRcDzr23626331 = -672957672;    int kuNZfRcDzr67272058 = -743727301;    int kuNZfRcDzr84961356 = -493133646;    int kuNZfRcDzr52805662 = -705564315;    int kuNZfRcDzr37154360 = -382765557;    int kuNZfRcDzr44636331 = -311886116;    int kuNZfRcDzr53095770 = 59691849;    int kuNZfRcDzr71377990 = 75018220;    int kuNZfRcDzr13426804 = -124174502;    int kuNZfRcDzr86714642 = 99707833;    int kuNZfRcDzr47391276 = -111349429;    int kuNZfRcDzr48779157 = -435993271;    int kuNZfRcDzr22422679 = -299984912;    int kuNZfRcDzr63124086 = -142758412;    int kuNZfRcDzr68395136 = -282152333;    int kuNZfRcDzr70857102 = -97520441;    int kuNZfRcDzr75239680 = -97298653;    int kuNZfRcDzr51896276 = -640692803;    int kuNZfRcDzr87848673 = -319972949;    int kuNZfRcDzr8072874 = 91053106;    int kuNZfRcDzr13638099 = -589385482;    int kuNZfRcDzr21022143 = -605686762;    int kuNZfRcDzr45583812 = -514407548;    int kuNZfRcDzr15282656 = -266283876;    int kuNZfRcDzr11389784 = -117860424;    int kuNZfRcDzr61197590 = -96855788;    int kuNZfRcDzr36141088 = -961192478;    int kuNZfRcDzr55624795 = -524430773;     kuNZfRcDzr76597409 = kuNZfRcDzr97714517;     kuNZfRcDzr97714517 = kuNZfRcDzr9844285;     kuNZfRcDzr9844285 = kuNZfRcDzr6048537;     kuNZfRcDzr6048537 = kuNZfRcDzr98948271;     kuNZfRcDzr98948271 = kuNZfRcDzr64353231;     kuNZfRcDzr64353231 = kuNZfRcDzr26039802;     kuNZfRcDzr26039802 = kuNZfRcDzr32419604;     kuNZfRcDzr32419604 = kuNZfRcDzr16366735;     kuNZfRcDzr16366735 = kuNZfRcDzr59077349;     kuNZfRcDzr59077349 = kuNZfRcDzr99224348;     kuNZfRcDzr99224348 = kuNZfRcDzr53644885;     kuNZfRcDzr53644885 = kuNZfRcDzr41080511;     kuNZfRcDzr41080511 = kuNZfRcDzr89178811;     kuNZfRcDzr89178811 = kuNZfRcDzr5911280;     kuNZfRcDzr5911280 = kuNZfRcDzr56934890;     kuNZfRcDzr56934890 = kuNZfRcDzr12265725;     kuNZfRcDzr12265725 = kuNZfRcDzr57279649;     kuNZfRcDzr57279649 = kuNZfRcDzr74739417;     kuNZfRcDzr74739417 = kuNZfRcDzr56876979;     kuNZfRcDzr56876979 = kuNZfRcDzr60879561;     kuNZfRcDzr60879561 = kuNZfRcDzr71274329;     kuNZfRcDzr71274329 = kuNZfRcDzr53590947;     kuNZfRcDzr53590947 = kuNZfRcDzr17333280;     kuNZfRcDzr17333280 = kuNZfRcDzr33222092;     kuNZfRcDzr33222092 = kuNZfRcDzr86317096;     kuNZfRcDzr86317096 = kuNZfRcDzr2469685;     kuNZfRcDzr2469685 = kuNZfRcDzr69406834;     kuNZfRcDzr69406834 = kuNZfRcDzr35735488;     kuNZfRcDzr35735488 = kuNZfRcDzr43444356;     kuNZfRcDzr43444356 = kuNZfRcDzr86263820;     kuNZfRcDzr86263820 = kuNZfRcDzr90611633;     kuNZfRcDzr90611633 = kuNZfRcDzr35034350;     kuNZfRcDzr35034350 = kuNZfRcDzr84609143;     kuNZfRcDzr84609143 = kuNZfRcDzr57219356;     kuNZfRcDzr57219356 = kuNZfRcDzr31972244;     kuNZfRcDzr31972244 = kuNZfRcDzr583878;     kuNZfRcDzr583878 = kuNZfRcDzr86464543;     kuNZfRcDzr86464543 = kuNZfRcDzr33653319;     kuNZfRcDzr33653319 = kuNZfRcDzr7467359;     kuNZfRcDzr7467359 = kuNZfRcDzr71915622;     kuNZfRcDzr71915622 = kuNZfRcDzr8073900;     kuNZfRcDzr8073900 = kuNZfRcDzr34119970;     kuNZfRcDzr34119970 = kuNZfRcDzr8954617;     kuNZfRcDzr8954617 = kuNZfRcDzr64237510;     kuNZfRcDzr64237510 = kuNZfRcDzr61844101;     kuNZfRcDzr61844101 = kuNZfRcDzr72890292;     kuNZfRcDzr72890292 = kuNZfRcDzr15755043;     kuNZfRcDzr15755043 = kuNZfRcDzr22015559;     kuNZfRcDzr22015559 = kuNZfRcDzr86956331;     kuNZfRcDzr86956331 = kuNZfRcDzr21021677;     kuNZfRcDzr21021677 = kuNZfRcDzr23139735;     kuNZfRcDzr23139735 = kuNZfRcDzr22216497;     kuNZfRcDzr22216497 = kuNZfRcDzr64177248;     kuNZfRcDzr64177248 = kuNZfRcDzr9369463;     kuNZfRcDzr9369463 = kuNZfRcDzr5323080;     kuNZfRcDzr5323080 = kuNZfRcDzr44123571;     kuNZfRcDzr44123571 = kuNZfRcDzr92511004;     kuNZfRcDzr92511004 = kuNZfRcDzr72826445;     kuNZfRcDzr72826445 = kuNZfRcDzr12631176;     kuNZfRcDzr12631176 = kuNZfRcDzr61883547;     kuNZfRcDzr61883547 = kuNZfRcDzr56632967;     kuNZfRcDzr56632967 = kuNZfRcDzr96684115;     kuNZfRcDzr96684115 = kuNZfRcDzr72922379;     kuNZfRcDzr72922379 = kuNZfRcDzr72813528;     kuNZfRcDzr72813528 = kuNZfRcDzr8612716;     kuNZfRcDzr8612716 = kuNZfRcDzr18610535;     kuNZfRcDzr18610535 = kuNZfRcDzr56471367;     kuNZfRcDzr56471367 = kuNZfRcDzr31959455;     kuNZfRcDzr31959455 = kuNZfRcDzr73939035;     kuNZfRcDzr73939035 = kuNZfRcDzr56351012;     kuNZfRcDzr56351012 = kuNZfRcDzr25801181;     kuNZfRcDzr25801181 = kuNZfRcDzr23626331;     kuNZfRcDzr23626331 = kuNZfRcDzr67272058;     kuNZfRcDzr67272058 = kuNZfRcDzr84961356;     kuNZfRcDzr84961356 = kuNZfRcDzr52805662;     kuNZfRcDzr52805662 = kuNZfRcDzr37154360;     kuNZfRcDzr37154360 = kuNZfRcDzr44636331;     kuNZfRcDzr44636331 = kuNZfRcDzr53095770;     kuNZfRcDzr53095770 = kuNZfRcDzr71377990;     kuNZfRcDzr71377990 = kuNZfRcDzr13426804;     kuNZfRcDzr13426804 = kuNZfRcDzr86714642;     kuNZfRcDzr86714642 = kuNZfRcDzr47391276;     kuNZfRcDzr47391276 = kuNZfRcDzr48779157;     kuNZfRcDzr48779157 = kuNZfRcDzr22422679;     kuNZfRcDzr22422679 = kuNZfRcDzr63124086;     kuNZfRcDzr63124086 = kuNZfRcDzr68395136;     kuNZfRcDzr68395136 = kuNZfRcDzr70857102;     kuNZfRcDzr70857102 = kuNZfRcDzr75239680;     kuNZfRcDzr75239680 = kuNZfRcDzr51896276;     kuNZfRcDzr51896276 = kuNZfRcDzr87848673;     kuNZfRcDzr87848673 = kuNZfRcDzr8072874;     kuNZfRcDzr8072874 = kuNZfRcDzr13638099;     kuNZfRcDzr13638099 = kuNZfRcDzr21022143;     kuNZfRcDzr21022143 = kuNZfRcDzr45583812;     kuNZfRcDzr45583812 = kuNZfRcDzr15282656;     kuNZfRcDzr15282656 = kuNZfRcDzr11389784;     kuNZfRcDzr11389784 = kuNZfRcDzr61197590;     kuNZfRcDzr61197590 = kuNZfRcDzr36141088;     kuNZfRcDzr36141088 = kuNZfRcDzr55624795;     kuNZfRcDzr55624795 = kuNZfRcDzr76597409;}
// Junk Finished
