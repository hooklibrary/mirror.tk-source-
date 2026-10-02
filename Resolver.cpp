#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"
#include "RenderManager.h"
#include "position_adjust.h"
#include "LagCompensation2.h"
#include "laggycompensation.h"
#include "global_count.h"
#include "position_adjust.h"
#include "Autowall.h"
#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif
#include "antiaim.h"
resolver_setup * resolver = new resolver_setup();

namespace global_count
{
	int hits[65] = { 0.f };
	int shots_fired[65] = { 0.f };
	int missed_shots[64] = { 0.f };
	bool didhit[64] = { 0.f };
	bool on_fire;

	int missed;
	int hit;
}

void calculate_angle(Vector src, Vector dst, Vector &angles)
{
	Vector delta = src - dst;
	vec_t hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.0f;
	if (delta.x >= 0.0) angles.y += 180.0f;
}
void NormalizeNumX(Vector &vIn, Vector &vOut)
{
	float flLen = vIn.Length();
	if (flLen == 0) {
		vOut.Init(0, 0, 1);
		return;
	}
	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}

inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}
void resolver_setup::preso(IClientEntity * player)
{
	switch (options::menu.aimbot.preso.getindex())
	{
	case 1:
	{
		player->GetEyeAnglesXY()->x = 89;
		//	resolver->resolved_pitch = 89.f;
	}
	break;
	case 2:
	{
		player->GetEyeAnglesXY()->x = -89;
		//	resolver->resolved_pitch = -89.f;
	}
	break;
	case 3:
	{
		player->GetEyeAnglesXY()->x = 0;
		//	resolver->resolved_pitch = 0.f;
	}
	break;
	case 4:
	{

		auto index = player->GetIndex();

		static bool shoot[65];
		static bool untrusted[65];
		static float shoot_time[65];
		static float base_pitch;

		const auto local = hackManager.pLocal();
		if (!local)
			return;

		for (auto i = 0; i < interfaces::engine->GetMaxClients(); ++i)
		{
			static float simtime[65];

			const auto eye = player->GetEyeAnglesXY();

			

			auto pitch = 89.f;

			if (eye->x > 89.f) // you sir, are not trusted
			{
				pitch = 89.f;
			}
			
			if (eye->x < -89.f)
			{
				pitch = -89.f;				
			}		
	
			if (simtime[index] != player->GetSimulationTime())
			{
				base_pitch = eye->x;
			}

			if (player->GetWeapon2())
			{
				if (shoot_time[index] != player->GetWeapon2()->last_shottime()) // check if they shot
				{
					pitch = eye->x;
					shoot[index] = true;

					shoot_time[index] = player->GetWeapon2()->last_shottime(); // suck a Sloavky dick
					//hhh... i miss daniel
				}
				else
				{
					pitch = base_pitch;
					shoot[index] = false;
				}
			}
			else
			{
				shoot[index] = false;
				shoot_time[index] = 0.f; // reset
			}

			player->GetEyeAnglesXY()->x = pitch;
		}
	}
	break;

	}

}

player_info_t GetInfo2(int Index) {
	player_info_t Info;
	interfaces::engine->GetPlayerInfo(Index, &Info);
	return Info;
}

int IClientEntity::sequence_activity(IClientEntity* pEntity, int sequence)
{
	const model_t* pModel = pEntity->GetModel();
	if (!pModel)
		return 0;

	auto hdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(pEntity, hdr, sequence);
}

float NormalizeFloatToAngle(float input)
{
	for (auto i = 0; i < 3; i++) {
		while (input < -180.0f) input += 360.0f;
		while (input > 180.0f) input -= 360.0f;
	}
	return input;
}

float override_yaw(IClientEntity* player, IClientEntity* local) {
	Vector eye_pos, pos_enemy;
	CalcAngle(player->GetEyePosition(), local->GetEyePosition(), eye_pos);

	if (Render::TransformScreen(player->GetOrigin(), pos_enemy))
	{
		if (GUI.GetMouse().x < pos_enemy.x)
			return (eye_pos.y - 90);
		else if (GUI.GetMouse().x > pos_enemy.x)
			return (eye_pos.y + 90);
	}

}

#define M_PI 3.14159265358979323846
void VectorAnglesBrute(const Vector& forward, Vector &angles)
{
	float tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0) pitch = 270; else pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0) yaw += 360; tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]); pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
		if (pitch < 0) pitch += 360;
	} angles[0] = pitch; angles[1] = yaw; angles[2] = 0;
}

Vector calc_angle_trash(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesBrute(dst - src, ret);
	return ret;
}

int total_missed[64];
int total_hit[64];
IGameEvent* event = nullptr;
extra s_extra;
void angle_correction::missed_due_to_desync(IGameEvent* event) {

	if (event == nullptr)
		return;
	int user = event->GetInt("userid");
	int attacker = event->GetInt("attacker");
	bool player_hurt[64], hit_entity[64];

	if (interfaces::engine->GetPlayerForUserID(user) != interfaces::engine->GetLocalPlayer()
		&& interfaces::engine->GetPlayerForUserID(attacker) == interfaces::engine->GetLocalPlayer()) {
		player_hurt[interfaces::engine->GetPlayerForUserID(user)] = true;
	}

	if (interfaces::engine->GetPlayerForUserID(user) != interfaces::engine->GetLocalPlayer())
	{
		Vector bullet_impact_location = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
		if (Globals::aim_point != bullet_impact_location) return;
		hit_entity[interfaces::engine->GetPlayerForUserID(user)] = true;
	}

	if (!player_hurt[interfaces::engine->GetPlayerForUserID(user)] && hit_entity[interfaces::engine->GetPlayerForUserID(user)]) {
		s_extra.current_flag[interfaces::engine->GetPlayerForUserID(user)] = correction_flags::DESYNC;
		++total_missed[interfaces::engine->GetPlayerForUserID(user)];
	}
	if (player_hurt[interfaces::engine->GetPlayerForUserID(user)] && hit_entity[interfaces::engine->GetPlayerForUserID(user)]) {
		++total_hit[interfaces::engine->GetPlayerForUserID(user)];
	}
}

int IClientEntity::GetSequenceActivity(int sequence)
{
	auto hdr = interfaces::model_info->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	static auto getSequenceActivity = (DWORD)(Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 7D 08 FF 56 8B F1 74"));
	static auto GetSequenceActivity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(getSequenceActivity);

	return GetSequenceActivity(this, hdr, sequence);
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

float lerp_time()
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

float NormalizeYaw180(float yaw)
{
	if (yaw > 180)
		yaw -= (round(yaw / 360) * 360.f);
	else if (yaw < -180)
		yaw += (round(yaw / 360) * -360.f);

	return yaw;
}
bool delta_58(float first, float second)
{
	if (first - second < 58.f && first - second > -58.f)
	{
		return true;
	}
	return false;
}
Vector CalcAngleToEnt(const Vector& vecSource, const Vector& vecDestination)
{
	Vector qAngles;
	Vector delta = Vector((vecSource[0] - vecDestination[0]), (vecSource[1] - vecDestination[1]), (vecSource[2] - vecDestination[2]));
	float hyp = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]);
	qAngles[0] = (float)(atan(delta[2] / hyp) * (180.0f / M_PI));
	qAngles[1] = (float)(atan(delta[1] / delta[0]) * (180.0f / M_PI));
	qAngles[2] = 0.f;
	if (delta[0] >= 0.f)
		qAngles[1] += 180.f;

	return qAngles;
}
float feet_yaw_delta(float first, float second)
{
	return first - second;
}
bool delta_35(float first, float second)
{
	if (first - second <= 35.f && first - second >= -35.f)
	{
		return true;
	}
	return false;
}
bool delta_20(float first, float second)
{
	if (first - second <= 20.f && first - second >= -20.f)
	{
		return true;
	}
	return false;
}
void angle_correction::ac_smart(IClientEntity* pEnt)
{
	static float oldSimtime[65];
	static float storedSimtime[65];
	static float ShotTime[65];
	static float SideTime[65][3];
	static int LastDesyncSide[65];
	static bool Delaying[65];
	static AnimationLayer StoredLayers[64][15];
	static CBaseAnimState * StoredAnimState[65];
	static float StoredPosParams[65][24];
	static Vector oldEyeAngles[65];
	static float oldGoalfeetYaw[65];
	float* PosParams = (float*)((uintptr_t)pEnt + 0x2774);
	bool update = false;
	bool shot = false;

	const auto local = hackManager.pLocal();
	if (!local)
		return;

	static bool jittering[65];

	auto* AnimState = pEnt->get_animation_state();

	if (!AnimState || !pEnt->AnimOverlays() || !PosParams)
		return;

	auto RemapVal = [](float val, float A, float B, float C, float D) -> float
	{
		if (A == B)
			return val >= B ? D : C;
		return C + (D - C) * (val - A) / (B - A);
	};

	//	pEnt->GetEyeAnglesXY()->z = 0.f;


	if (is_slow_walking(pEnt))
	{
		s_extra.current_flag[pEnt->GetIndex()] = correction_flags::SLOW_WALK;
		resolver->enemy_slowwalk = true;
	}
	else
		resolver->enemy_slowwalk = false;
	if (total_missed[pEnt->GetIndex()] > 4)
	{
		resolver->enemy_fake[pEnt->GetIndex()] = false;
		total_missed[pEnt->GetIndex()] = 0;
	}
	if (total_missed[pEnt->GetIndex()] > 1 && total_missed[pEnt->GetIndex()] <= 4)
	{
		resolver->enemy_fake[pEnt->GetIndex()] = true;
	}

	if (storedSimtime[pEnt->GetIndex()] != pEnt->GetSimulationTime())
	{
		jittering[pEnt->GetIndex()] = false;
		pEnt->ClientAnimations(true);
		pEnt->UpdateClientSideAnimation();

		memcpy(StoredPosParams[pEnt->GetIndex()], PosParams, sizeof(float) * 24);
		memcpy(StoredLayers[pEnt->GetIndex()], pEnt->AnimOverlays(), (sizeof(AnimationLayer) * 15));

		oldGoalfeetYaw[pEnt->GetIndex()] = AnimState->goal_feet_yaw;

		if (pEnt->GetWeapon2() && !pEnt->IsKnifeorNade())
		{
			if (ShotTime[pEnt->GetIndex()] != pEnt->GetWeapon2()->GetLastShotTime())
			{
				shot = true;
				ShotTime[pEnt->GetIndex()] = pEnt->GetWeapon2()->GetLastShotTime();
			}
			else
				shot = false;
		}
		else
		{
			shot = false;
			ShotTime[pEnt->GetIndex()] = 0.f;
		}

		float angToLocal = NormalizeYaw180(CalcAngleToEnt(local->GetOrigin(), pEnt->GetOrigin()).y);

		float Back = NormalizeYaw180(angToLocal);
		float DesyncFix = 0;
		float Resim = NormalizeYaw180((0.24f / (pEnt->GetSimulationTime() - oldSimtime[pEnt->GetIndex()]))*(oldEyeAngles[pEnt->GetIndex()].y - pEnt->GetEyeAngles().y));

		if (Resim > 58.f)
			Resim = 58.f;
		if (Resim < -58.f)
			Resim = -58.f;

		if (pEnt->GetVelocity().Length2D() > 0.5f && !shot)
		{
			float Delta = NormalizeYaw180(NormalizeYaw180(CalcAngleToEnt(Vector(0, 0, 0), pEnt->GetVelocity()).y) - NormalizeYaw180(NormalizeYaw180(AnimState->goal_feet_yaw + RemapVal(PosParams[11], 0, 1, -60, 60)) + Resim));

			int CurrentSide = 0;

			if (Delta < 0)
			{
				CurrentSide = 1;
				SideTime[pEnt->GetIndex()][1] = interfaces::globals->curtime;
			}
			else if (Delta > 0)
			{
				CurrentSide = 2;
				SideTime[pEnt->GetIndex()][2] = interfaces::globals->curtime;
			}

			if (LastDesyncSide[pEnt->GetIndex()] == 1)
			{
				Resim += (58.f - Resim);
				DesyncFix += (58.f - Resim);
			}
			if (LastDesyncSide[pEnt->GetIndex()] == 2)
			{
				Resim += (-58.f - Resim);
				DesyncFix += (-58.f - Resim);
			}

			if (LastDesyncSide[pEnt->GetIndex()] != CurrentSide)
			{
				Delaying[pEnt->GetIndex()] = true;

				if (0.5f < (interfaces::globals->curtime - SideTime[pEnt->GetIndex()][LastDesyncSide[pEnt->GetIndex()]]))
				{
					LastDesyncSide[pEnt->GetIndex()] = CurrentSide;
					Delaying[pEnt->GetIndex()] = false;
				}
			}

			if (!Delaying[pEnt->GetIndex()])
				LastDesyncSide[pEnt->GetIndex()] = CurrentSide;
		}
		else if (!shot)
		{
			float Brute = pEnt->GetLowerBodyYaw();

			float Delta = NormalizeYaw180(NormalizeYaw180(Brute - NormalizeYaw180(NormalizeYaw180(AnimState->goal_feet_yaw + RemapVal(PosParams[11], 0, 1, -60, 60))) + Resim));

			if (Delta > 58.f)
				Delta = 58.f;
			if (Delta < -58.f)
				Delta = -58.f;

			Resim += Delta;
			DesyncFix += Delta;

			if (Resim > 58.f)
				Resim = 58.f;
			if (Resim < -58.f)
				Resim = -58.f;
		}

		float Equalized = NormalizeYaw180(NormalizeYaw180(AnimState->goal_feet_yaw + RemapVal(PosParams[11], 0, 1, -59.f, 59.f)) + Resim);

		float JitterDelta = fabs(NormalizeYaw180(oldEyeAngles[pEnt->GetIndex()].y - pEnt->GetEyeAngles().y));

		if (JitterDelta >= 70.f && !shot)
			jittering[pEnt->GetIndex()] = true;

		if (pEnt->team() != local->team() && (pEnt->GetFlags() & FL_ONGROUND))
		{
			if (jittering[pEnt->GetIndex()])
				AnimState->goal_feet_yaw = NormalizeYaw180(pEnt->GetEyeAngles().y + DesyncFix);
			else
				AnimState->goal_feet_yaw = Equalized;

			pEnt->SetLowerBodyYaw(AnimState->goal_feet_yaw);
		}

		StoredAnimState[pEnt->GetIndex()] = AnimState;

		oldEyeAngles[pEnt->GetIndex()] = pEnt->GetEyeAngles();

		oldSimtime[pEnt->GetIndex()] = storedSimtime[pEnt->GetIndex()];

		storedSimtime[pEnt->GetIndex()] = pEnt->GetSimulationTime();

		update = true;
	}

	pEnt->ClientAnimations(false);

	if (pEnt != local && pEnt->team() != local->team() && (pEnt->GetFlags() & FL_ONGROUND))
		pEnt->SetLowerBodyYaw(AnimState->goal_feet_yaw);
	else
		pEnt->SetAbsAngles(Vector(0, pEnt->GetEyeAngles().y, 0));

	AnimState = StoredAnimState[pEnt->GetIndex()];

	memcpy((void*)PosParams, &StoredPosParams[pEnt->GetIndex()], (sizeof(float) * 24));
	memcpy(pEnt->AnimOverlays(), StoredLayers[pEnt->GetIndex()], (sizeof(AnimationLayer) * 15));

	if (pEnt != local && pEnt->team() != local->team() && (pEnt->GetFlags() & FL_ONGROUND) && jittering[pEnt->GetIndex()])
		pEnt->SetAbsAngles(Vector(0, pEnt->GetEyeAngles().y, 0));
	else
		pEnt->SetAbsAngles(Vector(0, oldGoalfeetYaw[pEnt->GetIndex()], 0));

	*reinterpret_cast<int*>(uintptr_t(pEnt) + 0xA30) = interfaces::globals->framecount;
	*reinterpret_cast<int*>(uintptr_t(pEnt) + 0xA28) = 0;
}

bool angle_correction::is_slow_walking(IClientEntity* entity) {
	float velocity_2D[64], old_velocity_2D[64];

	if (entity->GetVelocity().Length2D() != velocity_2D[entity->GetIndex()] && entity->GetVelocity().Length2D() != NULL) {
		old_velocity_2D[entity->GetIndex()] = velocity_2D[entity->GetIndex()];
		velocity_2D[entity->GetIndex()] = entity->GetVelocity().Length2D();
	}

	if (velocity_2D[entity->GetIndex()] > 0.1) {
		int tick_counter[64];

		if (velocity_2D[entity->GetIndex()] == old_velocity_2D[entity->GetIndex()])
			++tick_counter[entity->GetIndex()];
		else
			tick_counter[entity->GetIndex()] = 0;

		while (tick_counter[entity->GetIndex()] > (1 / interfaces::globals->interval_per_tick) * fabsf(0.1f))// should give use 100ms in ticks if their speed stays the same for that long they are definetely up to something..
			return true;

	}
	return false;
}

#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)

float NormalizeX(float yaw)
{
	if (yaw != yaw)
		yaw = 0.f;

	return fmod(yaw + 180.f, 360.f) - 180.f;
}

float approach(float cur, float target, float inc) {
	inc = abs(inc);

	if (cur < target)
		return min(cur + inc, target);
	if (cur > target)
		return max(cur - inc, target);

	return target;
}

float angle_difference(float a, float b) {
	auto diff = NormalizeYaw180(a - b);

	if (diff < 180)
		return diff;
	return diff - 360;
}

float approach_angle(float cur, float target, float inc) {
	auto diff = angle_difference(target, cur);
	return approach(cur, cur + diff, inc);
}

int IClientEntity::get_sequence_act(int sequence)
{
	auto hdr = interfaces::model_info->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast< int(__fastcall*)(void*, studiohdr_t*, int) >((DWORD)
		game_utils::pattern_scan(GetModuleHandle("client_panorama.dll"),
			"55 8B EC 53 8B 5D 08 56 8B F1 83"/*"55 8B EC 83 7D 08 FF 56 8B F1 74"*/));
	return get_sequence_activity(this, hdr, sequence);
}
bool angle_correction::solve_desync_simple(IClientEntity* e)
{
	if (!e || e->IsDormant() || !e->IsAlive())
		return false;

	for (size_t i = 0; i < e->GetNumAnimOverlays(); i++)
	{
		auto layer = e->get_anim_overlay_index(i);
		if (!layer)
			continue;

		if (e->get_sequence_act(layer->m_nSequence) == 979 && layer->m_flWeight == 0.0f && (layer->m_flCycle == 0.0f || layer->m_flCycle != layer->m_flPrevCycle) || layer->m_flWeight == 1.0f && layer->m_flCycle != layer->m_flPrevCycle)
		{
			return true;
		}
	}
	return false;
}
bool angle_correction::breaking_lby_animations(IClientEntity* e)
{
	if (!e || e->is_dormant() || !e->IsAlive())
		return false;

	for (size_t i = 0; i < e->GetNumAnimOverlays(); i++)
	{
		auto layer = e->get_anim_overlay_index(i);

		if (!layer )
			continue;
		if (e->get_sequence_act(layer->m_nSequence) == 979)
		{
			if (layer->m_flCycle != layer->m_flCycle || layer->m_flWeight == 1.f)
				return true;
		}
	}

	return false;
}

HANDLE _out = NULL, _old_out = NULL;
bool ConsolePrint(const char* fmt, ...)
{
	if (!_out)
		return false;

	char buf[1024];
	va_list va;

	va_start(va, fmt);
	_vsnprintf_s(buf, 1024, fmt, va);
	va_end(va);//im missing 5 chromosomes

	return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
}

float angle_correction::max_delta(IClientEntity* player, CBaseAnimState* anim_state) {
	if (!player || !anim_state || !player->GetBasePlayerAnimState())
		return 0.f;

	auto ducking_speed = anim_state->speed_2d / (player->max_speed() * 0.340f);

	auto speed_fraction = max(0.0f, min(anim_state->m_flFeetSpeedForwardsOrSideWays, 1.0f));

	auto fl_yaw_modifier = ((anim_state->m_flStopToFullRunningFraction * -0.3f) - 0.2f) * speed_fraction + 1.0f;

	if (anim_state->m_fDuckAmount > 0) {
		auto fl_ducking_speed = clamp(ducking_speed, 0.0f, 1.0f);
		fl_yaw_modifier = fl_yaw_modifier + ((anim_state->m_fDuckAmount * fl_ducking_speed) * (0.5f -
			fl_yaw_modifier));
	}

	auto delta = anim_state->velocity_subtract_y * fl_yaw_modifier;

	return delta;
}

float flAngleMod(float flAngle)
{
	return((360.0f / 65536.0f) * ((int32_t)(flAngle * (65536.0f / 360.0f)) & 65535));
}
float ApproachAngle(float target, float value, float speed)
{
	target = flAngleMod(target);
	value = flAngleMod(value);

	float delta = target - value;

	// Speed is assumed to be positive
	if (speed < 0)
		speed = -speed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

player_info_t GetInfo_x(int Index) {
	player_info_t Info;
	interfaces::engine->GetPlayerInfo(Index, &Info);
	return Info;
}


void angle_correction::mirror_aesthetic_console()
{
	interfaces::cvar->ConsoleColorPrintf(Color(250, 250, 250, 255), "[");
	interfaces::cvar->ConsoleColorPrintf(Color(210, 10, 250, 255), "mir");
	interfaces::cvar->ConsoleColorPrintf(Color(10, 190, 250, 255), "ror");
	interfaces::cvar->ConsoleColorPrintf(Color(250, 250, 250, 255), "]");
}

void angle_correction::resolve_mirror_primary(IClientEntity * player)
{

	float moving_lby[65] = { FLT_MAX };
	float old_eyes[65] = { FLT_MAX };
	float delta[65] = { FLT_MAX };
	float old_lby[65] = { FLT_MAX };
	float update_time[65] = { FLT_MAX };
	float original_lby[65] = { FLT_MAX };
	bool lby_flick[65] = { false };
	bool fake = false;
	bool did_move[65] = { false };

	bool is_moving = player->GetVelocity().Length2D() > 0.1f && (player->GetFlags() & FL_ONGROUND);

	if (!player || !(player->GetFlags() & FL_ONGROUND))
		return;

	const auto animation = player->get_animation_state();

	if (!animation)
		return;

	if ((!delta_35(player->GetEyeAnglesXY()->y, player->GetLowerBodyYaw()) && Globals::missedshots[player->GetIndex()] > 3))
	{
		resolver->enemy_fake[player->GetIndex()] = true;
		fake = true;
	}
	else
	{
		resolver->enemy_fake[player->GetIndex()] = false;
		fake = false;
	}

	if (delta_35(player->GetEyeAnglesXY()->y, player->GetLowerBodyYaw()) && ragebot->valid_hitchance)
	{
			resolver->has_desync[player->GetIndex()] = true;
	}
	else
		resolver->has_desync[player->GetIndex()] = false;

	if (is_moving)
	{
		if (moving_lby[player->GetIndex()] != player->GetLowerBodyYaw())
			moving_lby[player->GetIndex()] = player->GetLowerBodyYaw();

		old_lby[player->GetIndex()] = player->GetLowerBodyYaw();
		did_move[player->GetIndex()] = true;
	}


	if (player->GetVelocity().Length2D() < 1.1f && (player->GetFlags() & FL_ONGROUND))
	{
		if (original_lby[player->GetIndex()] == FLT_MAX)
		{
			original_lby[player->GetIndex()] = player->GetLowerBodyYaw();
		}

		if (old_lby[player->GetIndex()] != player->GetLowerBodyYaw())
		{
			delta[player->GetIndex()] = old_lby[player->GetIndex()] - player->GetLowerBodyYaw();
			lby_flick[player->GetIndex()] = true;
			old_lby[player->GetIndex()] = player->GetLowerBodyYaw();
		}
	}
	else
	{
		if (original_lby[player->GetIndex()] != player->GetLowerBodyYaw())
			original_lby[player->GetIndex()] = player->GetLowerBodyYaw();
	}

#pragma region i want to die.


	if (player->GetVelocity().Length2D() < 1.1f)
	{
		if (breaking_lby_animations(player))
		{
			if (old_lby[player->GetIndex()] != player->GetLowerBodyYaw() && original_lby[player->GetIndex()] != FLT_MAX)
			{
				if (Globals::missedshots[player->GetIndex()] < 3)
				{
					if (lby_flick[player->GetIndex()])
					{
						player->GetEyeAnglesXY()->y = player->GetLowerBodyYaw();
						//	mirror_aesthetic_console();
						//	interfaces::cvar->ConsoleColorPrintf(Color(10, 250, 200, 255), " [debug] resolver stage Lby Update.     \n");
					}
					else
						player->GetEyeAnglesXY()->y = old_lby[player->GetIndex()];
				}
				else
				{
					if (animation)
						player->GetEyeAnglesXY()->y -= max_delta(player, animation);

				}
			}

			else
			{

				if (Globals::missedshots[player->GetIndex()] < 3 && animation)
				{
					player->GetEyeAnglesXY()->y -= max_delta(player, animation);

				}

				else
				{
					if (original_lby[player->GetIndex()] != FLT_MAX) // experimental. You can replace this with your brute if you want. do NOT base if off lby in this case
					{
						player->GetEyeAnglesXY()->y = original_lby[player->GetIndex()];
					}
				}
			}

		}

		if (solve_desync_simple(player)) // experimental. Low delta lby break related animations.
		{	
			if ((delta[player->GetIndex()] < 110.f && delta[player->GetIndex()] >= 90.f) || (delta[player->GetIndex()] > -110.f && delta[player->GetIndex()] <= -90.f))
			{
				player->GetEyeAnglesXY()->y = player->GetLowerBodyYaw() - delta[player->GetIndex()];
			}
		}
	}

	else
	{
		if (animation)
			player->GetEyeAnglesXY()->y -= max_delta(player, animation);		
	}

}

void angle_correction::AnimationFix(IClientEntity* pEnt)
{
	if (pEnt != hackManager.pLocal())
	{
		auto player_index = pEnt->GetIndex() - 1;

		pEnt->ClientAnimations(true);

		auto old_curtime = interfaces::globals->curtime;
		auto old_frametime = interfaces::globals->frametime;

		interfaces::globals->curtime = pEnt->GetSimulationTime();
		interfaces::globals->frametime = interfaces::globals->interval_per_tick;

		auto player_animation_state = pEnt->get_animation_state();
		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);
		if (player_animation_state != nullptr && player_model_time != nullptr)
			if (*player_model_time == interfaces::globals->framecount)
				*player_model_time = interfaces::globals->framecount - 1;


		pEnt->UpdateClientSideAnimation();

		interfaces::globals->curtime = old_curtime;
		interfaces::globals->frametime = old_frametime;

		//pEnt->SetAbsAngles(Vector(0, player_animation_state->m_flGoalFeetYaw, 0));

		pEnt->ClientAnimations(false);
	}

}


void update_state(CBaseAnimState * state, Vector angles) {
	using Fn = void(__vectorcall*)(void *, void *, float, float, float, void *);
	static auto fn = reinterpret_cast<Fn>(game_utils::pattern_scan("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));
	fn(state, nullptr, 0.0f, angles[1], angles[0], nullptr);
}

void HandleBackUpResolve(IClientEntity* pEnt) {

	

	const auto player_animation_state = pEnt->get_animation_state();

	if (!player_animation_state)
		return;

	float m_flLastClientSideAnimationUpdateTimeDelta = fabs(player_animation_state->m_iLastClientSideAnimationUpdateFramecount - player_animation_state->m_flLastClientSideAnimationUpdateTime);

	auto v48 = 0.f;

	if (player_animation_state->m_flFeetSpeedForwardsOrSideWays >= 0.0f)
	{
		v48 = fminf(player_animation_state->m_flFeetSpeedForwardsOrSideWays, 1.0f);
	}
	else
	{
		v48 = 0.0f;
	}

	float v49 = ((player_animation_state->m_flStopToFullRunningFraction * -0.30000001) - 0.19999999) * v48;

	float flYawModifier = v49 + 1.0;

	if (player_animation_state->m_fDuckAmount > 0.0)
	{
		float v53 = 0.0f;

		if (player_animation_state->m_flFeetSpeedUnknownForwardOrSideways >= 0.0)
		{
			v53 = fminf(player_animation_state->m_flFeetSpeedUnknownForwardOrSideways, 1.0);
		}
		else
		{
			v53 = 0.0f;
		}
	}

	float flMaxYawModifier = player_animation_state->pad10[516] * flYawModifier;
	float flMinYawModifier = player_animation_state->pad10[512] * flYawModifier;

	float newFeetYaw = 0.f;

	auto eyeYaw = player_animation_state->m_flEyeYaw;

	auto lbyYaw = player_animation_state->goal_feet_yaw;

	float eye_feet_delta = fabs(eyeYaw - lbyYaw);

	if (eye_feet_delta <= flMaxYawModifier)
	{
		if (flMinYawModifier > eye_feet_delta)
		{
			newFeetYaw = fabs(flMinYawModifier) + eyeYaw;
		}
	}
	else
	{
		newFeetYaw = eyeYaw - fabs(flMaxYawModifier);
	}

	float v136 = fmod(newFeetYaw, 360.0);

	if (v136 > 180.0)
	{
		v136 = v136 - 360.0;
	}

	if (v136 < 180.0)
	{
		v136 = v136 + 360.0;
	}

	player_animation_state->goal_feet_yaw = v136;

	/*static int stored_yaw = 0;

	if (pEnt->GetEyeAnglesPointer()->y != stored_yaw) {
	if ((pEnt->GetEyeAnglesPointer()->y - stored_yaw > 120)) { // Arbitrary high angle value.
	if (pEnt->GetEyeAnglesPointer()->y - stored_yaw > 120) {
	pEnt->GetEyeAnglesPointer()->y = pEnt->GetEyeAnglesPointer()->y - (pEnt->GetEyeAnglesPointer()->y - stored_yaw);
	}

	stored_yaw = pEnt->GetEyeAnglesPointer()->y;
	}
	}*/
	//if (pEnt->GetVelocity().Length2D() > 0.1f)
	//{
	//	player_animation_state->m_flGoalFeetYaw = ApproachAngle(pEnt->GetLowerBodyYaw(), player_animation_state->m_flGoalFeetYaw, (player_animation_state->m_flStopToFullRunningFraction * 20.0f) + 30.0f *player_animation_state->m_flLastClientSideAnimationUpdateTime);
	//}
	//else
	//{
	//	player_animation_state->m_flGoalFeetYaw = ApproachAngle(pEnt->GetLowerBodyYaw(), player_animation_state->m_flGoalFeetYaw, (m_flLastClientSideAnimationUpdateTimeDelta * 100.0f));
	//}
	//if (Globals::MissedShots[pEnt->EntIndex()] > 3) {
	//	switch (Globals::MissedShots[pEnt->EntIndex()] % 4) {
	//	case 0: pEnt->GetEyeAnglesPointer()->y = pEnt->GetEyeAnglesPointer()->y + 45; break;
	//	case 1: pEnt->GetEyeAnglesPointer()->y = pEnt->GetEyeAnglesPointer()->y - 45; break;
	//	case 2: pEnt->GetEyeAnglesPointer()->y = pEnt->GetEyeAnglesPointer()->y - 30; break;
	//	case 3: pEnt->GetEyeAnglesPointer()->y = pEnt->GetEyeAnglesPointer()->y + 30; break;
	//	}
	//}
}

void resolver_setup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{

	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;
	if (!options::menu.aimbot.AimbotEnable.getstate() || options::menu.aimbot.resolver.getindex() < 1)
		return;

	angle_correction ac;

	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < 65; i++)
		{
			pEntity = (IClientEntity*)interfaces::ent_list->get_client_entity(i);

			if (!pEntity->isValidPlayer() || pEntity->team() == hackManager.pLocal()->team() || pEntity->IsDormant())
				continue;

			if (pEntity->GetOrigin() == Vector(0, 0, 0))
				continue;

			if (options::menu.aimbot.preso.getindex() > 0)
			{
				resolver_setup::preso(pEntity);
			}

			switch (options::menu.aimbot.resolver.getindex())
			{
			case 1:
			{			
				ac.ac_smart(pEntity);			
			}
			break;

			case 2:
			{				
				ac.resolve_mirror_primary(pEntity); // spin on my dick like a beyblade, uh				
			}
			break;
			}
		}

		if (GetAsyncKeyState(options::menu.aimbot.flip180.GetKey()))
		{
			pEntity->GetEyeAnglesXY()->y -= 180.f;
		}
		//	if (Options::Menu.RageBotTab.resolver.GetIndex() > 1)
		//		ac.ac_smart(pEntity);				
	}

	if (options::menu.aimbot.resolver.getindex() > 2)
	{
		for (int i = 1; i < interfaces::engine->GetMaxClients(); ++i)
		{
			IClientEntity* pPlayerEntity = interfaces::ent_list->get_client_entity(i);

			if (!pPlayerEntity->isValidPlayer()
				|| pPlayerEntity->team() == hackManager.pLocal()->team())
				continue;
			if (pPlayerEntity->IsDormant())
			{
				continue;
			}

			if (stage == FRAME_RENDER_START)
			{
				ac.AnimationFix(pPlayerEntity);
			}

			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
				HandleBackUpResolve(pPlayerEntity);
			}

			if (stage == FRAME_NET_UPDATE_END && pPlayerEntity != hackManager.pLocal())
			{
				auto VarMap = reinterpret_cast<uintptr_t>(pPlayerEntity) + 36;
				auto VarMapSize = *reinterpret_cast<int*>(VarMap + 20);

				for (auto index = 0; index < VarMapSize; index++)
					*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(VarMap) + index * 12) = 0;
			}
		}
	}
}































































































































































// Junk Code By Troll Face & Thaisen's Gen
void KmrhYSRNWB592651() {     int SKJqADJDAy16627319 = -40023592;    int SKJqADJDAy85445635 = -186736045;    int SKJqADJDAy1061132 = -526377202;    int SKJqADJDAy66685346 = -140969525;    int SKJqADJDAy88832109 = -532546403;    int SKJqADJDAy49051055 = -311981701;    int SKJqADJDAy93399755 = -183140014;    int SKJqADJDAy51008598 = -605753155;    int SKJqADJDAy61078958 = -994993009;    int SKJqADJDAy44706824 = -828213704;    int SKJqADJDAy7845970 = -603308816;    int SKJqADJDAy57648144 = -866443726;    int SKJqADJDAy96645748 = -490645376;    int SKJqADJDAy16752193 = -20261668;    int SKJqADJDAy34548777 = -515560470;    int SKJqADJDAy14241305 = 39003536;    int SKJqADJDAy72273924 = -900040210;    int SKJqADJDAy20644484 = -510074733;    int SKJqADJDAy5018846 = -712940332;    int SKJqADJDAy53377220 = -14896777;    int SKJqADJDAy86441607 = -206419178;    int SKJqADJDAy43631648 = 46465341;    int SKJqADJDAy79664504 = 67153214;    int SKJqADJDAy75702680 = -613917392;    int SKJqADJDAy6575165 = -948737606;    int SKJqADJDAy54239778 = -725539419;    int SKJqADJDAy93540138 = -112352118;    int SKJqADJDAy89560056 = -61439973;    int SKJqADJDAy52941313 = -485451182;    int SKJqADJDAy46979622 = -317243857;    int SKJqADJDAy82549940 = -904367188;    int SKJqADJDAy59842714 = -512333055;    int SKJqADJDAy9386622 = -232511479;    int SKJqADJDAy6891432 = -665517013;    int SKJqADJDAy27324722 = -78502751;    int SKJqADJDAy17834845 = -817187840;    int SKJqADJDAy57324397 = -802006252;    int SKJqADJDAy75844421 = -816753578;    int SKJqADJDAy84956588 = -878638738;    int SKJqADJDAy56876298 = -719985280;    int SKJqADJDAy97574089 = 81762514;    int SKJqADJDAy87907499 = -559791858;    int SKJqADJDAy6698813 = -420595586;    int SKJqADJDAy86024852 = -151642029;    int SKJqADJDAy96649195 = -799071621;    int SKJqADJDAy35532105 = 99034575;    int SKJqADJDAy63687403 = -57729216;    int SKJqADJDAy32484089 = -706661689;    int SKJqADJDAy94807611 = -930432707;    int SKJqADJDAy99412689 = -923984336;    int SKJqADJDAy58976620 = -68881495;    int SKJqADJDAy9947895 = -501682899;    int SKJqADJDAy83983169 = -268465421;    int SKJqADJDAy56258312 = -622555575;    int SKJqADJDAy14151044 = -158475478;    int SKJqADJDAy72995671 = 13511066;    int SKJqADJDAy5781132 = -153889259;    int SKJqADJDAy25358451 = -912459810;    int SKJqADJDAy60110181 = -192231919;    int SKJqADJDAy34592332 = -807006984;    int SKJqADJDAy55510917 = -99629584;    int SKJqADJDAy3839699 = -21700042;    int SKJqADJDAy98067284 = -20301973;    int SKJqADJDAy14099336 = -577749153;    int SKJqADJDAy62156884 = -923846516;    int SKJqADJDAy48003256 = 9024239;    int SKJqADJDAy48261523 = -533932248;    int SKJqADJDAy89754316 = -825128363;    int SKJqADJDAy89427470 = -941758918;    int SKJqADJDAy16713932 = -698372631;    int SKJqADJDAy56916908 = -158990213;    int SKJqADJDAy96429502 = 16713367;    int SKJqADJDAy35687896 = -631435995;    int SKJqADJDAy48142547 = -992955053;    int SKJqADJDAy55803131 = 3340708;    int SKJqADJDAy98534107 = -646627320;    int SKJqADJDAy36932836 = -532939073;    int SKJqADJDAy93639651 = -781204758;    int SKJqADJDAy79053484 = -814845772;    int SKJqADJDAy71043060 = -947772182;    int SKJqADJDAy90552374 = -567810204;    int SKJqADJDAy61056050 = -405690429;    int SKJqADJDAy94752445 = -131007266;    int SKJqADJDAy53528624 = -561466847;    int SKJqADJDAy88003001 = -148362362;    int SKJqADJDAy72602045 = -302684290;    int SKJqADJDAy75859544 = -143867634;    int SKJqADJDAy53128310 = -609955904;    int SKJqADJDAy92740388 = -407041536;    int SKJqADJDAy54329051 = 7986183;    int SKJqADJDAy12053714 = -563298582;    int SKJqADJDAy31965946 = -889546442;    int SKJqADJDAy15734241 = -524521659;    int SKJqADJDAy50364257 = 28368245;    int SKJqADJDAy1365382 = -520355696;    int SKJqADJDAy93734390 = -896537445;    int SKJqADJDAy89840215 = -439489886;    int SKJqADJDAy92599477 = -842846434;    int SKJqADJDAy23867968 = -227795513;    int SKJqADJDAy48645940 = -40023592;     SKJqADJDAy16627319 = SKJqADJDAy85445635;     SKJqADJDAy85445635 = SKJqADJDAy1061132;     SKJqADJDAy1061132 = SKJqADJDAy66685346;     SKJqADJDAy66685346 = SKJqADJDAy88832109;     SKJqADJDAy88832109 = SKJqADJDAy49051055;     SKJqADJDAy49051055 = SKJqADJDAy93399755;     SKJqADJDAy93399755 = SKJqADJDAy51008598;     SKJqADJDAy51008598 = SKJqADJDAy61078958;     SKJqADJDAy61078958 = SKJqADJDAy44706824;     SKJqADJDAy44706824 = SKJqADJDAy7845970;     SKJqADJDAy7845970 = SKJqADJDAy57648144;     SKJqADJDAy57648144 = SKJqADJDAy96645748;     SKJqADJDAy96645748 = SKJqADJDAy16752193;     SKJqADJDAy16752193 = SKJqADJDAy34548777;     SKJqADJDAy34548777 = SKJqADJDAy14241305;     SKJqADJDAy14241305 = SKJqADJDAy72273924;     SKJqADJDAy72273924 = SKJqADJDAy20644484;     SKJqADJDAy20644484 = SKJqADJDAy5018846;     SKJqADJDAy5018846 = SKJqADJDAy53377220;     SKJqADJDAy53377220 = SKJqADJDAy86441607;     SKJqADJDAy86441607 = SKJqADJDAy43631648;     SKJqADJDAy43631648 = SKJqADJDAy79664504;     SKJqADJDAy79664504 = SKJqADJDAy75702680;     SKJqADJDAy75702680 = SKJqADJDAy6575165;     SKJqADJDAy6575165 = SKJqADJDAy54239778;     SKJqADJDAy54239778 = SKJqADJDAy93540138;     SKJqADJDAy93540138 = SKJqADJDAy89560056;     SKJqADJDAy89560056 = SKJqADJDAy52941313;     SKJqADJDAy52941313 = SKJqADJDAy46979622;     SKJqADJDAy46979622 = SKJqADJDAy82549940;     SKJqADJDAy82549940 = SKJqADJDAy59842714;     SKJqADJDAy59842714 = SKJqADJDAy9386622;     SKJqADJDAy9386622 = SKJqADJDAy6891432;     SKJqADJDAy6891432 = SKJqADJDAy27324722;     SKJqADJDAy27324722 = SKJqADJDAy17834845;     SKJqADJDAy17834845 = SKJqADJDAy57324397;     SKJqADJDAy57324397 = SKJqADJDAy75844421;     SKJqADJDAy75844421 = SKJqADJDAy84956588;     SKJqADJDAy84956588 = SKJqADJDAy56876298;     SKJqADJDAy56876298 = SKJqADJDAy97574089;     SKJqADJDAy97574089 = SKJqADJDAy87907499;     SKJqADJDAy87907499 = SKJqADJDAy6698813;     SKJqADJDAy6698813 = SKJqADJDAy86024852;     SKJqADJDAy86024852 = SKJqADJDAy96649195;     SKJqADJDAy96649195 = SKJqADJDAy35532105;     SKJqADJDAy35532105 = SKJqADJDAy63687403;     SKJqADJDAy63687403 = SKJqADJDAy32484089;     SKJqADJDAy32484089 = SKJqADJDAy94807611;     SKJqADJDAy94807611 = SKJqADJDAy99412689;     SKJqADJDAy99412689 = SKJqADJDAy58976620;     SKJqADJDAy58976620 = SKJqADJDAy9947895;     SKJqADJDAy9947895 = SKJqADJDAy83983169;     SKJqADJDAy83983169 = SKJqADJDAy56258312;     SKJqADJDAy56258312 = SKJqADJDAy14151044;     SKJqADJDAy14151044 = SKJqADJDAy72995671;     SKJqADJDAy72995671 = SKJqADJDAy5781132;     SKJqADJDAy5781132 = SKJqADJDAy25358451;     SKJqADJDAy25358451 = SKJqADJDAy60110181;     SKJqADJDAy60110181 = SKJqADJDAy34592332;     SKJqADJDAy34592332 = SKJqADJDAy55510917;     SKJqADJDAy55510917 = SKJqADJDAy3839699;     SKJqADJDAy3839699 = SKJqADJDAy98067284;     SKJqADJDAy98067284 = SKJqADJDAy14099336;     SKJqADJDAy14099336 = SKJqADJDAy62156884;     SKJqADJDAy62156884 = SKJqADJDAy48003256;     SKJqADJDAy48003256 = SKJqADJDAy48261523;     SKJqADJDAy48261523 = SKJqADJDAy89754316;     SKJqADJDAy89754316 = SKJqADJDAy89427470;     SKJqADJDAy89427470 = SKJqADJDAy16713932;     SKJqADJDAy16713932 = SKJqADJDAy56916908;     SKJqADJDAy56916908 = SKJqADJDAy96429502;     SKJqADJDAy96429502 = SKJqADJDAy35687896;     SKJqADJDAy35687896 = SKJqADJDAy48142547;     SKJqADJDAy48142547 = SKJqADJDAy55803131;     SKJqADJDAy55803131 = SKJqADJDAy98534107;     SKJqADJDAy98534107 = SKJqADJDAy36932836;     SKJqADJDAy36932836 = SKJqADJDAy93639651;     SKJqADJDAy93639651 = SKJqADJDAy79053484;     SKJqADJDAy79053484 = SKJqADJDAy71043060;     SKJqADJDAy71043060 = SKJqADJDAy90552374;     SKJqADJDAy90552374 = SKJqADJDAy61056050;     SKJqADJDAy61056050 = SKJqADJDAy94752445;     SKJqADJDAy94752445 = SKJqADJDAy53528624;     SKJqADJDAy53528624 = SKJqADJDAy88003001;     SKJqADJDAy88003001 = SKJqADJDAy72602045;     SKJqADJDAy72602045 = SKJqADJDAy75859544;     SKJqADJDAy75859544 = SKJqADJDAy53128310;     SKJqADJDAy53128310 = SKJqADJDAy92740388;     SKJqADJDAy92740388 = SKJqADJDAy54329051;     SKJqADJDAy54329051 = SKJqADJDAy12053714;     SKJqADJDAy12053714 = SKJqADJDAy31965946;     SKJqADJDAy31965946 = SKJqADJDAy15734241;     SKJqADJDAy15734241 = SKJqADJDAy50364257;     SKJqADJDAy50364257 = SKJqADJDAy1365382;     SKJqADJDAy1365382 = SKJqADJDAy93734390;     SKJqADJDAy93734390 = SKJqADJDAy89840215;     SKJqADJDAy89840215 = SKJqADJDAy92599477;     SKJqADJDAy92599477 = SKJqADJDAy23867968;     SKJqADJDAy23867968 = SKJqADJDAy48645940;     SKJqADJDAy48645940 = SKJqADJDAy16627319;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HTAGBLBygd18805992() {     int HintIzjgjy71625 = -920689273;    int HintIzjgjy2317725 = -186118232;    int HintIzjgjy97469638 = -319902430;    int HintIzjgjy29477598 = -334769361;    int HintIzjgjy94953433 = -339413894;    int HintIzjgjy47555456 = -91053354;    int HintIzjgjy64439073 = 76948673;    int HintIzjgjy65356040 = -592911184;    int HintIzjgjy19608659 = -236452360;    int HintIzjgjy77252476 = -196589816;    int HintIzjgjy57261907 = -793424688;    int HintIzjgjy20853543 = -854061972;    int HintIzjgjy14763524 = -282306617;    int HintIzjgjy96379084 = -908552025;    int HintIzjgjy95403239 = -609583452;    int HintIzjgjy630467 = -849075790;    int HintIzjgjy66025809 = -653385237;    int HintIzjgjy1495931 = -757379609;    int HintIzjgjy67458988 = -711615202;    int HintIzjgjy42744663 = -544482769;    int HintIzjgjy42081994 = -895948209;    int HintIzjgjy55778966 = 55313571;    int HintIzjgjy83490211 = 27264016;    int HintIzjgjy10409239 = -685742788;    int HintIzjgjy48151123 = -408390365;    int HintIzjgjy49581927 = -606049135;    int HintIzjgjy42994878 = -840436460;    int HintIzjgjy29291682 = -131394857;    int HintIzjgjy89022407 = -347956325;    int HintIzjgjy62812963 = -830093708;    int HintIzjgjy645254 = -542124389;    int HintIzjgjy22949141 = -427528135;    int HintIzjgjy52925508 = -953383656;    int HintIzjgjy49049942 = -257119183;    int HintIzjgjy6051453 = -276167486;    int HintIzjgjy30224029 = -605211746;    int HintIzjgjy25938576 = -680855432;    int HintIzjgjy16423254 = -525224098;    int HintIzjgjy33951666 = 36463464;    int HintIzjgjy6371434 = -472578802;    int HintIzjgjy31223014 = -589786459;    int HintIzjgjy38395286 = -33700180;    int HintIzjgjy30265882 = -971652829;    int HintIzjgjy77929792 = 92175200;    int HintIzjgjy88593692 = -527228574;    int HintIzjgjy75138101 = 79423652;    int HintIzjgjy39702834 = -190033445;    int HintIzjgjy52154684 = -705383407;    int HintIzjgjy25654724 = -416034790;    int HintIzjgjy34745174 = -31004791;    int HintIzjgjy32234069 = -289782918;    int HintIzjgjy65448689 = -559942210;    int HintIzjgjy98095628 = -480212738;    int HintIzjgjy80935071 = -340968963;    int HintIzjgjy76723998 = -506961601;    int HintIzjgjy44292658 = -876002845;    int HintIzjgjy18827513 = -113382249;    int HintIzjgjy87060399 = -634159642;    int HintIzjgjy81326474 = -926378996;    int HintIzjgjy45371506 = -733364760;    int HintIzjgjy4560578 = -250616895;    int HintIzjgjy35147392 = -791656471;    int HintIzjgjy76333633 = -144954860;    int HintIzjgjy56795695 = -406358652;    int HintIzjgjy76607222 = -654465427;    int HintIzjgjy34312766 = -265896554;    int HintIzjgjy67928035 = -900678317;    int HintIzjgjy65713582 = 74812566;    int HintIzjgjy90327632 = -532384540;    int HintIzjgjy65179210 = 95628293;    int HintIzjgjy74691891 = -68220358;    int HintIzjgjy49602555 = -28161140;    int HintIzjgjy67544264 = -693843074;    int HintIzjgjy61087554 = -139036401;    int HintIzjgjy11521649 = -954696311;    int HintIzjgjy3686709 = -762248030;    int HintIzjgjy25513084 = 26966400;    int HintIzjgjy5560420 = 35088816;    int HintIzjgjy21815546 = -58514215;    int HintIzjgjy73013022 = -387814017;    int HintIzjgjy9879093 = -316015690;    int HintIzjgjy90840194 = -35053053;    int HintIzjgjy3636959 = -715360067;    int HintIzjgjy54277234 = -216951534;    int HintIzjgjy30578895 = -440310791;    int HintIzjgjy35196565 = -982182180;    int HintIzjgjy24853513 = -947315398;    int HintIzjgjy71990437 = -512414694;    int HintIzjgjy72325944 = -750157583;    int HintIzjgjy61758794 = -400164641;    int HintIzjgjy11396517 = -391829498;    int HintIzjgjy38878176 = 53304210;    int HintIzjgjy35096779 = -598845102;    int HintIzjgjy88580159 = -230171777;    int HintIzjgjy1810857 = -121961908;    int HintIzjgjy96075622 = -798129988;    int HintIzjgjy62061653 = -888745320;    int HintIzjgjy73470187 = -465294178;    int HintIzjgjy1322570 = -253359373;    int HintIzjgjy54280926 = -920689273;     HintIzjgjy71625 = HintIzjgjy2317725;     HintIzjgjy2317725 = HintIzjgjy97469638;     HintIzjgjy97469638 = HintIzjgjy29477598;     HintIzjgjy29477598 = HintIzjgjy94953433;     HintIzjgjy94953433 = HintIzjgjy47555456;     HintIzjgjy47555456 = HintIzjgjy64439073;     HintIzjgjy64439073 = HintIzjgjy65356040;     HintIzjgjy65356040 = HintIzjgjy19608659;     HintIzjgjy19608659 = HintIzjgjy77252476;     HintIzjgjy77252476 = HintIzjgjy57261907;     HintIzjgjy57261907 = HintIzjgjy20853543;     HintIzjgjy20853543 = HintIzjgjy14763524;     HintIzjgjy14763524 = HintIzjgjy96379084;     HintIzjgjy96379084 = HintIzjgjy95403239;     HintIzjgjy95403239 = HintIzjgjy630467;     HintIzjgjy630467 = HintIzjgjy66025809;     HintIzjgjy66025809 = HintIzjgjy1495931;     HintIzjgjy1495931 = HintIzjgjy67458988;     HintIzjgjy67458988 = HintIzjgjy42744663;     HintIzjgjy42744663 = HintIzjgjy42081994;     HintIzjgjy42081994 = HintIzjgjy55778966;     HintIzjgjy55778966 = HintIzjgjy83490211;     HintIzjgjy83490211 = HintIzjgjy10409239;     HintIzjgjy10409239 = HintIzjgjy48151123;     HintIzjgjy48151123 = HintIzjgjy49581927;     HintIzjgjy49581927 = HintIzjgjy42994878;     HintIzjgjy42994878 = HintIzjgjy29291682;     HintIzjgjy29291682 = HintIzjgjy89022407;     HintIzjgjy89022407 = HintIzjgjy62812963;     HintIzjgjy62812963 = HintIzjgjy645254;     HintIzjgjy645254 = HintIzjgjy22949141;     HintIzjgjy22949141 = HintIzjgjy52925508;     HintIzjgjy52925508 = HintIzjgjy49049942;     HintIzjgjy49049942 = HintIzjgjy6051453;     HintIzjgjy6051453 = HintIzjgjy30224029;     HintIzjgjy30224029 = HintIzjgjy25938576;     HintIzjgjy25938576 = HintIzjgjy16423254;     HintIzjgjy16423254 = HintIzjgjy33951666;     HintIzjgjy33951666 = HintIzjgjy6371434;     HintIzjgjy6371434 = HintIzjgjy31223014;     HintIzjgjy31223014 = HintIzjgjy38395286;     HintIzjgjy38395286 = HintIzjgjy30265882;     HintIzjgjy30265882 = HintIzjgjy77929792;     HintIzjgjy77929792 = HintIzjgjy88593692;     HintIzjgjy88593692 = HintIzjgjy75138101;     HintIzjgjy75138101 = HintIzjgjy39702834;     HintIzjgjy39702834 = HintIzjgjy52154684;     HintIzjgjy52154684 = HintIzjgjy25654724;     HintIzjgjy25654724 = HintIzjgjy34745174;     HintIzjgjy34745174 = HintIzjgjy32234069;     HintIzjgjy32234069 = HintIzjgjy65448689;     HintIzjgjy65448689 = HintIzjgjy98095628;     HintIzjgjy98095628 = HintIzjgjy80935071;     HintIzjgjy80935071 = HintIzjgjy76723998;     HintIzjgjy76723998 = HintIzjgjy44292658;     HintIzjgjy44292658 = HintIzjgjy18827513;     HintIzjgjy18827513 = HintIzjgjy87060399;     HintIzjgjy87060399 = HintIzjgjy81326474;     HintIzjgjy81326474 = HintIzjgjy45371506;     HintIzjgjy45371506 = HintIzjgjy4560578;     HintIzjgjy4560578 = HintIzjgjy35147392;     HintIzjgjy35147392 = HintIzjgjy76333633;     HintIzjgjy76333633 = HintIzjgjy56795695;     HintIzjgjy56795695 = HintIzjgjy76607222;     HintIzjgjy76607222 = HintIzjgjy34312766;     HintIzjgjy34312766 = HintIzjgjy67928035;     HintIzjgjy67928035 = HintIzjgjy65713582;     HintIzjgjy65713582 = HintIzjgjy90327632;     HintIzjgjy90327632 = HintIzjgjy65179210;     HintIzjgjy65179210 = HintIzjgjy74691891;     HintIzjgjy74691891 = HintIzjgjy49602555;     HintIzjgjy49602555 = HintIzjgjy67544264;     HintIzjgjy67544264 = HintIzjgjy61087554;     HintIzjgjy61087554 = HintIzjgjy11521649;     HintIzjgjy11521649 = HintIzjgjy3686709;     HintIzjgjy3686709 = HintIzjgjy25513084;     HintIzjgjy25513084 = HintIzjgjy5560420;     HintIzjgjy5560420 = HintIzjgjy21815546;     HintIzjgjy21815546 = HintIzjgjy73013022;     HintIzjgjy73013022 = HintIzjgjy9879093;     HintIzjgjy9879093 = HintIzjgjy90840194;     HintIzjgjy90840194 = HintIzjgjy3636959;     HintIzjgjy3636959 = HintIzjgjy54277234;     HintIzjgjy54277234 = HintIzjgjy30578895;     HintIzjgjy30578895 = HintIzjgjy35196565;     HintIzjgjy35196565 = HintIzjgjy24853513;     HintIzjgjy24853513 = HintIzjgjy71990437;     HintIzjgjy71990437 = HintIzjgjy72325944;     HintIzjgjy72325944 = HintIzjgjy61758794;     HintIzjgjy61758794 = HintIzjgjy11396517;     HintIzjgjy11396517 = HintIzjgjy38878176;     HintIzjgjy38878176 = HintIzjgjy35096779;     HintIzjgjy35096779 = HintIzjgjy88580159;     HintIzjgjy88580159 = HintIzjgjy1810857;     HintIzjgjy1810857 = HintIzjgjy96075622;     HintIzjgjy96075622 = HintIzjgjy62061653;     HintIzjgjy62061653 = HintIzjgjy73470187;     HintIzjgjy73470187 = HintIzjgjy1322570;     HintIzjgjy1322570 = HintIzjgjy54280926;     HintIzjgjy54280926 = HintIzjgjy71625;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VrbrJtIyZG89261865() {     int SZntBEzVUZ12857792 = 44270035;    int SZntBEzVUZ89088612 = -356452246;    int SZntBEzVUZ15222438 = -728261811;    int SZntBEzVUZ37418423 = -158971637;    int SZntBEzVUZ88464537 = -831230359;    int SZntBEzVUZ90865526 = -967976668;    int SZntBEzVUZ85646831 = -468723398;    int SZntBEzVUZ92075153 = 62570559;    int SZntBEzVUZ15013469 = -198210452;    int SZntBEzVUZ83471180 = -677449;    int SZntBEzVUZ84749289 = -908110704;    int SZntBEzVUZ52804048 = -261394788;    int SZntBEzVUZ84906826 = -186089274;    int SZntBEzVUZ46003163 = -140967223;    int SZntBEzVUZ18709793 = -516569283;    int SZntBEzVUZ29055536 = -299967179;    int SZntBEzVUZ21777627 = -317159704;    int SZntBEzVUZ85054415 = -312589911;    int SZntBEzVUZ68619618 = -621084398;    int SZntBEzVUZ45399159 = -43726909;    int SZntBEzVUZ22256051 = -287264473;    int SZntBEzVUZ84826785 = -114544777;    int SZntBEzVUZ22250507 = 96260122;    int SZntBEzVUZ54916368 = 53912199;    int SZntBEzVUZ54045382 = -618832289;    int SZntBEzVUZ30900572 = -296480334;    int SZntBEzVUZ54410414 = -374142740;    int SZntBEzVUZ96644606 = -305192961;    int SZntBEzVUZ5639461 = -562173565;    int SZntBEzVUZ15564079 = -459979863;    int SZntBEzVUZ45081250 = -818541139;    int SZntBEzVUZ26267965 = -406247813;    int SZntBEzVUZ98971457 = -829238440;    int SZntBEzVUZ26126315 = -161873402;    int SZntBEzVUZ89111984 = -886199366;    int SZntBEzVUZ25448144 = -393774850;    int SZntBEzVUZ62702636 = -470764782;    int SZntBEzVUZ74127276 = -444844190;    int SZntBEzVUZ39727052 = -905387113;    int SZntBEzVUZ32636164 = -316995648;    int SZntBEzVUZ92654321 = -965260452;    int SZntBEzVUZ46097742 = 67988486;    int SZntBEzVUZ65149352 = -995086274;    int SZntBEzVUZ22558330 = -248997043;    int SZntBEzVUZ68602183 = -419998534;    int SZntBEzVUZ48766230 = 6105212;    int SZntBEzVUZ17284854 = -687548497;    int SZntBEzVUZ81594650 = -860736187;    int SZntBEzVUZ69718772 = -579636764;    int SZntBEzVUZ81695594 = -642041254;    int SZntBEzVUZ50561562 = -542667226;    int SZntBEzVUZ93625235 = 68872452;    int SZntBEzVUZ32580779 = -656216020;    int SZntBEzVUZ25634796 = 69579295;    int SZntBEzVUZ67005814 = -501406088;    int SZntBEzVUZ28031006 = -841185189;    int SZntBEzVUZ66838105 = -352712368;    int SZntBEzVUZ60306069 = -682174011;    int SZntBEzVUZ83373040 = -540139349;    int SZntBEzVUZ57563966 = -434750026;    int SZntBEzVUZ36455112 = -493833928;    int SZntBEzVUZ89002224 = -63530437;    int SZntBEzVUZ86435692 = -375255877;    int SZntBEzVUZ99449389 = -738230590;    int SZntBEzVUZ38389931 = -182136310;    int SZntBEzVUZ58481325 = -401862891;    int SZntBEzVUZ53832591 = -432156348;    int SZntBEzVUZ58780512 = 75784127;    int SZntBEzVUZ56891178 = -254767858;    int SZntBEzVUZ93261648 = -22794433;    int SZntBEzVUZ66352899 = -829202397;    int SZntBEzVUZ47650350 = -872315515;    int SZntBEzVUZ45327363 = -407202799;    int SZntBEzVUZ35983455 = -204088751;    int SZntBEzVUZ52744837 = -78466457;    int SZntBEzVUZ76158309 = -255252960;    int SZntBEzVUZ19677434 = -119458503;    int SZntBEzVUZ99692177 = -654742836;    int SZntBEzVUZ86314185 = -526089267;    int SZntBEzVUZ5279152 = -524937502;    int SZntBEzVUZ13615719 = -608931837;    int SZntBEzVUZ72815764 = -513406554;    int SZntBEzVUZ26925834 = -725556197;    int SZntBEzVUZ23943867 = -920132311;    int SZntBEzVUZ65002517 = -917312637;    int SZntBEzVUZ51456014 = -787413592;    int SZntBEzVUZ93687186 = -750031794;    int SZntBEzVUZ73336661 = -798817736;    int SZntBEzVUZ59120501 = -660467315;    int SZntBEzVUZ61080978 = 54985822;    int SZntBEzVUZ58610039 = 58937517;    int SZntBEzVUZ2396567 = -788590772;    int SZntBEzVUZ90754235 = -904704841;    int SZntBEzVUZ82163086 = -370637088;    int SZntBEzVUZ96181051 = -823161720;    int SZntBEzVUZ3652098 = -801730016;    int SZntBEzVUZ59662049 = -556755638;    int SZntBEzVUZ65699962 = -156855685;    int SZntBEzVUZ84168399 = 33139267;    int SZntBEzVUZ10120859 = 44270035;     SZntBEzVUZ12857792 = SZntBEzVUZ89088612;     SZntBEzVUZ89088612 = SZntBEzVUZ15222438;     SZntBEzVUZ15222438 = SZntBEzVUZ37418423;     SZntBEzVUZ37418423 = SZntBEzVUZ88464537;     SZntBEzVUZ88464537 = SZntBEzVUZ90865526;     SZntBEzVUZ90865526 = SZntBEzVUZ85646831;     SZntBEzVUZ85646831 = SZntBEzVUZ92075153;     SZntBEzVUZ92075153 = SZntBEzVUZ15013469;     SZntBEzVUZ15013469 = SZntBEzVUZ83471180;     SZntBEzVUZ83471180 = SZntBEzVUZ84749289;     SZntBEzVUZ84749289 = SZntBEzVUZ52804048;     SZntBEzVUZ52804048 = SZntBEzVUZ84906826;     SZntBEzVUZ84906826 = SZntBEzVUZ46003163;     SZntBEzVUZ46003163 = SZntBEzVUZ18709793;     SZntBEzVUZ18709793 = SZntBEzVUZ29055536;     SZntBEzVUZ29055536 = SZntBEzVUZ21777627;     SZntBEzVUZ21777627 = SZntBEzVUZ85054415;     SZntBEzVUZ85054415 = SZntBEzVUZ68619618;     SZntBEzVUZ68619618 = SZntBEzVUZ45399159;     SZntBEzVUZ45399159 = SZntBEzVUZ22256051;     SZntBEzVUZ22256051 = SZntBEzVUZ84826785;     SZntBEzVUZ84826785 = SZntBEzVUZ22250507;     SZntBEzVUZ22250507 = SZntBEzVUZ54916368;     SZntBEzVUZ54916368 = SZntBEzVUZ54045382;     SZntBEzVUZ54045382 = SZntBEzVUZ30900572;     SZntBEzVUZ30900572 = SZntBEzVUZ54410414;     SZntBEzVUZ54410414 = SZntBEzVUZ96644606;     SZntBEzVUZ96644606 = SZntBEzVUZ5639461;     SZntBEzVUZ5639461 = SZntBEzVUZ15564079;     SZntBEzVUZ15564079 = SZntBEzVUZ45081250;     SZntBEzVUZ45081250 = SZntBEzVUZ26267965;     SZntBEzVUZ26267965 = SZntBEzVUZ98971457;     SZntBEzVUZ98971457 = SZntBEzVUZ26126315;     SZntBEzVUZ26126315 = SZntBEzVUZ89111984;     SZntBEzVUZ89111984 = SZntBEzVUZ25448144;     SZntBEzVUZ25448144 = SZntBEzVUZ62702636;     SZntBEzVUZ62702636 = SZntBEzVUZ74127276;     SZntBEzVUZ74127276 = SZntBEzVUZ39727052;     SZntBEzVUZ39727052 = SZntBEzVUZ32636164;     SZntBEzVUZ32636164 = SZntBEzVUZ92654321;     SZntBEzVUZ92654321 = SZntBEzVUZ46097742;     SZntBEzVUZ46097742 = SZntBEzVUZ65149352;     SZntBEzVUZ65149352 = SZntBEzVUZ22558330;     SZntBEzVUZ22558330 = SZntBEzVUZ68602183;     SZntBEzVUZ68602183 = SZntBEzVUZ48766230;     SZntBEzVUZ48766230 = SZntBEzVUZ17284854;     SZntBEzVUZ17284854 = SZntBEzVUZ81594650;     SZntBEzVUZ81594650 = SZntBEzVUZ69718772;     SZntBEzVUZ69718772 = SZntBEzVUZ81695594;     SZntBEzVUZ81695594 = SZntBEzVUZ50561562;     SZntBEzVUZ50561562 = SZntBEzVUZ93625235;     SZntBEzVUZ93625235 = SZntBEzVUZ32580779;     SZntBEzVUZ32580779 = SZntBEzVUZ25634796;     SZntBEzVUZ25634796 = SZntBEzVUZ67005814;     SZntBEzVUZ67005814 = SZntBEzVUZ28031006;     SZntBEzVUZ28031006 = SZntBEzVUZ66838105;     SZntBEzVUZ66838105 = SZntBEzVUZ60306069;     SZntBEzVUZ60306069 = SZntBEzVUZ83373040;     SZntBEzVUZ83373040 = SZntBEzVUZ57563966;     SZntBEzVUZ57563966 = SZntBEzVUZ36455112;     SZntBEzVUZ36455112 = SZntBEzVUZ89002224;     SZntBEzVUZ89002224 = SZntBEzVUZ86435692;     SZntBEzVUZ86435692 = SZntBEzVUZ99449389;     SZntBEzVUZ99449389 = SZntBEzVUZ38389931;     SZntBEzVUZ38389931 = SZntBEzVUZ58481325;     SZntBEzVUZ58481325 = SZntBEzVUZ53832591;     SZntBEzVUZ53832591 = SZntBEzVUZ58780512;     SZntBEzVUZ58780512 = SZntBEzVUZ56891178;     SZntBEzVUZ56891178 = SZntBEzVUZ93261648;     SZntBEzVUZ93261648 = SZntBEzVUZ66352899;     SZntBEzVUZ66352899 = SZntBEzVUZ47650350;     SZntBEzVUZ47650350 = SZntBEzVUZ45327363;     SZntBEzVUZ45327363 = SZntBEzVUZ35983455;     SZntBEzVUZ35983455 = SZntBEzVUZ52744837;     SZntBEzVUZ52744837 = SZntBEzVUZ76158309;     SZntBEzVUZ76158309 = SZntBEzVUZ19677434;     SZntBEzVUZ19677434 = SZntBEzVUZ99692177;     SZntBEzVUZ99692177 = SZntBEzVUZ86314185;     SZntBEzVUZ86314185 = SZntBEzVUZ5279152;     SZntBEzVUZ5279152 = SZntBEzVUZ13615719;     SZntBEzVUZ13615719 = SZntBEzVUZ72815764;     SZntBEzVUZ72815764 = SZntBEzVUZ26925834;     SZntBEzVUZ26925834 = SZntBEzVUZ23943867;     SZntBEzVUZ23943867 = SZntBEzVUZ65002517;     SZntBEzVUZ65002517 = SZntBEzVUZ51456014;     SZntBEzVUZ51456014 = SZntBEzVUZ93687186;     SZntBEzVUZ93687186 = SZntBEzVUZ73336661;     SZntBEzVUZ73336661 = SZntBEzVUZ59120501;     SZntBEzVUZ59120501 = SZntBEzVUZ61080978;     SZntBEzVUZ61080978 = SZntBEzVUZ58610039;     SZntBEzVUZ58610039 = SZntBEzVUZ2396567;     SZntBEzVUZ2396567 = SZntBEzVUZ90754235;     SZntBEzVUZ90754235 = SZntBEzVUZ82163086;     SZntBEzVUZ82163086 = SZntBEzVUZ96181051;     SZntBEzVUZ96181051 = SZntBEzVUZ3652098;     SZntBEzVUZ3652098 = SZntBEzVUZ59662049;     SZntBEzVUZ59662049 = SZntBEzVUZ65699962;     SZntBEzVUZ65699962 = SZntBEzVUZ84168399;     SZntBEzVUZ84168399 = SZntBEzVUZ10120859;     SZntBEzVUZ10120859 = SZntBEzVUZ12857792;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RerbuXxiwc7475207() {     int LAyAozJcVX96302097 = -836395646;    int LAyAozJcVX5960702 = -355834433;    int LAyAozJcVX11630945 = -521787039;    int LAyAozJcVX210675 = -352771473;    int LAyAozJcVX94585861 = -638097850;    int LAyAozJcVX89369927 = -747048321;    int LAyAozJcVX56686149 = -208634711;    int LAyAozJcVX6422596 = 75412530;    int LAyAozJcVX73543169 = -539669803;    int LAyAozJcVX16016833 = -469053561;    int LAyAozJcVX34165227 = 1773424;    int LAyAozJcVX16009448 = -249013034;    int LAyAozJcVX3024603 = 22249485;    int LAyAozJcVX25630055 = 70742420;    int LAyAozJcVX79564256 = -610592265;    int LAyAozJcVX15444697 = -88046504;    int LAyAozJcVX15529512 = -70504731;    int LAyAozJcVX65905862 = -559894788;    int LAyAozJcVX31059761 = -619759269;    int LAyAozJcVX34766602 = -573312901;    int LAyAozJcVX77896438 = -976793504;    int LAyAozJcVX96974103 = -105696547;    int LAyAozJcVX26076215 = 56370924;    int LAyAozJcVX89622926 = -17913197;    int LAyAozJcVX95621340 = -78485048;    int LAyAozJcVX26242721 = -176990050;    int LAyAozJcVX3865155 = -2227083;    int LAyAozJcVX36376232 = -375147845;    int LAyAozJcVX41720555 = -424678707;    int LAyAozJcVX31397420 = -972829714;    int LAyAozJcVX63176563 = -456298340;    int LAyAozJcVX89374391 = -321442893;    int LAyAozJcVX42510344 = -450110618;    int LAyAozJcVX68284825 = -853475572;    int LAyAozJcVX67838715 = 16135899;    int LAyAozJcVX37837328 = -181798756;    int LAyAozJcVX31316815 = -349613962;    int LAyAozJcVX14706109 = -153314710;    int LAyAozJcVX88722130 = 9715089;    int LAyAozJcVX82131298 = -69589170;    int LAyAozJcVX26303246 = -536809426;    int LAyAozJcVX96585527 = -505919836;    int LAyAozJcVX88716421 = -446143517;    int LAyAozJcVX14463270 = -5179814;    int LAyAozJcVX60546680 = -148155487;    int LAyAozJcVX88372226 = -13505712;    int LAyAozJcVX93300283 = -819852727;    int LAyAozJcVX1265246 = -859457905;    int LAyAozJcVX565885 = -65238847;    int LAyAozJcVX17028078 = -849061710;    int LAyAozJcVX23819011 = -763568648;    int LAyAozJcVX49126030 = 10613140;    int LAyAozJcVX46693237 = -867963337;    int LAyAozJcVX50311555 = -748834092;    int LAyAozJcVX29578769 = -849892211;    int LAyAozJcVX99327993 = -630699099;    int LAyAozJcVX79884487 = -312205358;    int LAyAozJcVX22008018 = -403873843;    int LAyAozJcVX4589334 = -174286426;    int LAyAozJcVX68343140 = -361107801;    int LAyAozJcVX85504772 = -644821239;    int LAyAozJcVX20309917 = -833486866;    int LAyAozJcVX64702041 = -499908764;    int LAyAozJcVX42145749 = -566840089;    int LAyAozJcVX52840269 = 87244779;    int LAyAozJcVX44790836 = -676783684;    int LAyAozJcVX73499103 = -798902417;    int LAyAozJcVX34739778 = -124274944;    int LAyAozJcVX57791340 = -945393479;    int LAyAozJcVX41726928 = -328793510;    int LAyAozJcVX84127882 = -738432543;    int LAyAozJcVX823404 = -917190022;    int LAyAozJcVX77183731 = -469609877;    int LAyAozJcVX48928462 = -450170099;    int LAyAozJcVX8463356 = 63496524;    int LAyAozJcVX81310910 = -370873669;    int LAyAozJcVX8257683 = -659553031;    int LAyAozJcVX11612945 = -938449263;    int LAyAozJcVX29076247 = -869757710;    int LAyAozJcVX7249114 = 35020663;    int LAyAozJcVX32942438 = -357137323;    int LAyAozJcVX2599910 = -142769178;    int LAyAozJcVX35810347 = -209908999;    int LAyAozJcVX24692477 = -575616998;    int LAyAozJcVX7578410 = -109261066;    int LAyAozJcVX14050533 = -366911481;    int LAyAozJcVX42681154 = -453479557;    int LAyAozJcVX92198789 = -701276526;    int LAyAozJcVX38706057 = 96416638;    int LAyAozJcVX68510721 = -353165003;    int LAyAozJcVX57952841 = -869593398;    int LAyAozJcVX9308797 = -945740120;    int LAyAozJcVX10116775 = -979028284;    int LAyAozJcVX20378990 = -629177111;    int LAyAozJcVX96626526 = -424767932;    int LAyAozJcVX5993330 = -703322560;    int LAyAozJcVX31883487 = 93988928;    int LAyAozJcVX46570672 = -879303428;    int LAyAozJcVX61623001 = 7575407;    int LAyAozJcVX15755845 = -836395646;     LAyAozJcVX96302097 = LAyAozJcVX5960702;     LAyAozJcVX5960702 = LAyAozJcVX11630945;     LAyAozJcVX11630945 = LAyAozJcVX210675;     LAyAozJcVX210675 = LAyAozJcVX94585861;     LAyAozJcVX94585861 = LAyAozJcVX89369927;     LAyAozJcVX89369927 = LAyAozJcVX56686149;     LAyAozJcVX56686149 = LAyAozJcVX6422596;     LAyAozJcVX6422596 = LAyAozJcVX73543169;     LAyAozJcVX73543169 = LAyAozJcVX16016833;     LAyAozJcVX16016833 = LAyAozJcVX34165227;     LAyAozJcVX34165227 = LAyAozJcVX16009448;     LAyAozJcVX16009448 = LAyAozJcVX3024603;     LAyAozJcVX3024603 = LAyAozJcVX25630055;     LAyAozJcVX25630055 = LAyAozJcVX79564256;     LAyAozJcVX79564256 = LAyAozJcVX15444697;     LAyAozJcVX15444697 = LAyAozJcVX15529512;     LAyAozJcVX15529512 = LAyAozJcVX65905862;     LAyAozJcVX65905862 = LAyAozJcVX31059761;     LAyAozJcVX31059761 = LAyAozJcVX34766602;     LAyAozJcVX34766602 = LAyAozJcVX77896438;     LAyAozJcVX77896438 = LAyAozJcVX96974103;     LAyAozJcVX96974103 = LAyAozJcVX26076215;     LAyAozJcVX26076215 = LAyAozJcVX89622926;     LAyAozJcVX89622926 = LAyAozJcVX95621340;     LAyAozJcVX95621340 = LAyAozJcVX26242721;     LAyAozJcVX26242721 = LAyAozJcVX3865155;     LAyAozJcVX3865155 = LAyAozJcVX36376232;     LAyAozJcVX36376232 = LAyAozJcVX41720555;     LAyAozJcVX41720555 = LAyAozJcVX31397420;     LAyAozJcVX31397420 = LAyAozJcVX63176563;     LAyAozJcVX63176563 = LAyAozJcVX89374391;     LAyAozJcVX89374391 = LAyAozJcVX42510344;     LAyAozJcVX42510344 = LAyAozJcVX68284825;     LAyAozJcVX68284825 = LAyAozJcVX67838715;     LAyAozJcVX67838715 = LAyAozJcVX37837328;     LAyAozJcVX37837328 = LAyAozJcVX31316815;     LAyAozJcVX31316815 = LAyAozJcVX14706109;     LAyAozJcVX14706109 = LAyAozJcVX88722130;     LAyAozJcVX88722130 = LAyAozJcVX82131298;     LAyAozJcVX82131298 = LAyAozJcVX26303246;     LAyAozJcVX26303246 = LAyAozJcVX96585527;     LAyAozJcVX96585527 = LAyAozJcVX88716421;     LAyAozJcVX88716421 = LAyAozJcVX14463270;     LAyAozJcVX14463270 = LAyAozJcVX60546680;     LAyAozJcVX60546680 = LAyAozJcVX88372226;     LAyAozJcVX88372226 = LAyAozJcVX93300283;     LAyAozJcVX93300283 = LAyAozJcVX1265246;     LAyAozJcVX1265246 = LAyAozJcVX565885;     LAyAozJcVX565885 = LAyAozJcVX17028078;     LAyAozJcVX17028078 = LAyAozJcVX23819011;     LAyAozJcVX23819011 = LAyAozJcVX49126030;     LAyAozJcVX49126030 = LAyAozJcVX46693237;     LAyAozJcVX46693237 = LAyAozJcVX50311555;     LAyAozJcVX50311555 = LAyAozJcVX29578769;     LAyAozJcVX29578769 = LAyAozJcVX99327993;     LAyAozJcVX99327993 = LAyAozJcVX79884487;     LAyAozJcVX79884487 = LAyAozJcVX22008018;     LAyAozJcVX22008018 = LAyAozJcVX4589334;     LAyAozJcVX4589334 = LAyAozJcVX68343140;     LAyAozJcVX68343140 = LAyAozJcVX85504772;     LAyAozJcVX85504772 = LAyAozJcVX20309917;     LAyAozJcVX20309917 = LAyAozJcVX64702041;     LAyAozJcVX64702041 = LAyAozJcVX42145749;     LAyAozJcVX42145749 = LAyAozJcVX52840269;     LAyAozJcVX52840269 = LAyAozJcVX44790836;     LAyAozJcVX44790836 = LAyAozJcVX73499103;     LAyAozJcVX73499103 = LAyAozJcVX34739778;     LAyAozJcVX34739778 = LAyAozJcVX57791340;     LAyAozJcVX57791340 = LAyAozJcVX41726928;     LAyAozJcVX41726928 = LAyAozJcVX84127882;     LAyAozJcVX84127882 = LAyAozJcVX823404;     LAyAozJcVX823404 = LAyAozJcVX77183731;     LAyAozJcVX77183731 = LAyAozJcVX48928462;     LAyAozJcVX48928462 = LAyAozJcVX8463356;     LAyAozJcVX8463356 = LAyAozJcVX81310910;     LAyAozJcVX81310910 = LAyAozJcVX8257683;     LAyAozJcVX8257683 = LAyAozJcVX11612945;     LAyAozJcVX11612945 = LAyAozJcVX29076247;     LAyAozJcVX29076247 = LAyAozJcVX7249114;     LAyAozJcVX7249114 = LAyAozJcVX32942438;     LAyAozJcVX32942438 = LAyAozJcVX2599910;     LAyAozJcVX2599910 = LAyAozJcVX35810347;     LAyAozJcVX35810347 = LAyAozJcVX24692477;     LAyAozJcVX24692477 = LAyAozJcVX7578410;     LAyAozJcVX7578410 = LAyAozJcVX14050533;     LAyAozJcVX14050533 = LAyAozJcVX42681154;     LAyAozJcVX42681154 = LAyAozJcVX92198789;     LAyAozJcVX92198789 = LAyAozJcVX38706057;     LAyAozJcVX38706057 = LAyAozJcVX68510721;     LAyAozJcVX68510721 = LAyAozJcVX57952841;     LAyAozJcVX57952841 = LAyAozJcVX9308797;     LAyAozJcVX9308797 = LAyAozJcVX10116775;     LAyAozJcVX10116775 = LAyAozJcVX20378990;     LAyAozJcVX20378990 = LAyAozJcVX96626526;     LAyAozJcVX96626526 = LAyAozJcVX5993330;     LAyAozJcVX5993330 = LAyAozJcVX31883487;     LAyAozJcVX31883487 = LAyAozJcVX46570672;     LAyAozJcVX46570672 = LAyAozJcVX61623001;     LAyAozJcVX61623001 = LAyAozJcVX15755845;     LAyAozJcVX15755845 = LAyAozJcVX96302097;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QbrRdgTmuV77931079() {     int EyJUlACXFD9088265 = -971436337;    int EyJUlACXFD92731589 = -526168447;    int EyJUlACXFD29383745 = -930146421;    int EyJUlACXFD8151500 = -176973750;    int EyJUlACXFD88096966 = -29914315;    int EyJUlACXFD32679998 = -523971634;    int EyJUlACXFD77893907 = -754306782;    int EyJUlACXFD33141709 = -369105727;    int EyJUlACXFD68947980 = -501427895;    int EyJUlACXFD22235538 = -273141194;    int EyJUlACXFD61652609 = -112912592;    int EyJUlACXFD47959952 = -756345850;    int EyJUlACXFD73167905 = -981533173;    int EyJUlACXFD75254133 = -261672779;    int EyJUlACXFD2870810 = -517578096;    int EyJUlACXFD43869766 = -638937893;    int EyJUlACXFD71281329 = -834279198;    int EyJUlACXFD49464347 = -115105090;    int EyJUlACXFD32220391 = -529228464;    int EyJUlACXFD37421098 = -72557040;    int EyJUlACXFD58070495 = -368109768;    int EyJUlACXFD26021923 = -275554895;    int EyJUlACXFD64836510 = -974632970;    int EyJUlACXFD34130056 = -378258209;    int EyJUlACXFD1515600 = -288926972;    int EyJUlACXFD7561367 = -967421249;    int EyJUlACXFD15280691 = -635933363;    int EyJUlACXFD3729158 = -548945950;    int EyJUlACXFD58337608 = -638895947;    int EyJUlACXFD84148535 = -602715869;    int EyJUlACXFD7612560 = -732715090;    int EyJUlACXFD92693215 = -300162572;    int EyJUlACXFD88556293 = -325965402;    int EyJUlACXFD45361198 = -758229790;    int EyJUlACXFD50899247 = -593895982;    int EyJUlACXFD33061443 = 29638140;    int EyJUlACXFD68080875 = -139523313;    int EyJUlACXFD72410131 = -72934801;    int EyJUlACXFD94497516 = -932135488;    int EyJUlACXFD8396029 = 85993984;    int EyJUlACXFD87734554 = -912283419;    int EyJUlACXFD4287984 = -404231170;    int EyJUlACXFD23599892 = -469576962;    int EyJUlACXFD59091808 = -346352056;    int EyJUlACXFD40555170 = -40925448;    int EyJUlACXFD62000356 = -86824151;    int EyJUlACXFD70882303 = -217367779;    int EyJUlACXFD30705212 = 85189315;    int EyJUlACXFD44629934 = -228840822;    int EyJUlACXFD63978499 = -360098172;    int EyJUlACXFD42146504 = 83547043;    int EyJUlACXFD77302577 = -460572198;    int EyJUlACXFD81178387 = 56033381;    int EyJUlACXFD95011280 = -338285834;    int EyJUlACXFD19860585 = -844336697;    int EyJUlACXFD83066341 = -595881443;    int EyJUlACXFD27895080 = -551535477;    int EyJUlACXFD95253688 = -451888212;    int EyJUlACXFD6635901 = -888046778;    int EyJUlACXFD80535600 = -62493067;    int EyJUlACXFD17399307 = -888038272;    int EyJUlACXFD74164750 = -105360832;    int EyJUlACXFD74804101 = -730209780;    int EyJUlACXFD84799444 = -898712027;    int EyJUlACXFD14622979 = -540426104;    int EyJUlACXFD68959394 = -812750021;    int EyJUlACXFD59403659 = -330380449;    int EyJUlACXFD27806708 = -123303383;    int EyJUlACXFD24354887 = -667776797;    int EyJUlACXFD69809366 = -447216236;    int EyJUlACXFD75788891 = -399414581;    int EyJUlACXFD98871197 = -661344398;    int EyJUlACXFD54966830 = -182969603;    int EyJUlACXFD23824363 = -515222449;    int EyJUlACXFD49686543 = -160273622;    int EyJUlACXFD53782511 = -963878599;    int EyJUlACXFD2422032 = -805977934;    int EyJUlACXFD5744703 = -528280914;    int EyJUlACXFD93574886 = -237332762;    int EyJUlACXFD39515244 = -102102821;    int EyJUlACXFD36679063 = -650053470;    int EyJUlACXFD84575479 = -621122679;    int EyJUlACXFD59099223 = -220105129;    int EyJUlACXFD94359108 = -178797775;    int EyJUlACXFD42002032 = -586262913;    int EyJUlACXFD30309982 = -172142893;    int EyJUlACXFD11514828 = -256195953;    int EyJUlACXFD93545013 = -987679569;    int EyJUlACXFD25500614 = -913893094;    int EyJUlACXFD67832905 = -998014539;    int EyJUlACXFD5166364 = -418826384;    int EyJUlACXFD72827186 = -687635101;    int EyJUlACXFD65774231 = -184888023;    int EyJUlACXFD13961917 = -769642422;    int EyJUlACXFD90996721 = -25967744;    int EyJUlACXFD13569805 = -706922587;    int EyJUlACXFD29483883 = -674021390;    int EyJUlACXFD38800447 = -570864936;    int EyJUlACXFD44468830 = -805925953;    int EyJUlACXFD71595776 = -971436337;     EyJUlACXFD9088265 = EyJUlACXFD92731589;     EyJUlACXFD92731589 = EyJUlACXFD29383745;     EyJUlACXFD29383745 = EyJUlACXFD8151500;     EyJUlACXFD8151500 = EyJUlACXFD88096966;     EyJUlACXFD88096966 = EyJUlACXFD32679998;     EyJUlACXFD32679998 = EyJUlACXFD77893907;     EyJUlACXFD77893907 = EyJUlACXFD33141709;     EyJUlACXFD33141709 = EyJUlACXFD68947980;     EyJUlACXFD68947980 = EyJUlACXFD22235538;     EyJUlACXFD22235538 = EyJUlACXFD61652609;     EyJUlACXFD61652609 = EyJUlACXFD47959952;     EyJUlACXFD47959952 = EyJUlACXFD73167905;     EyJUlACXFD73167905 = EyJUlACXFD75254133;     EyJUlACXFD75254133 = EyJUlACXFD2870810;     EyJUlACXFD2870810 = EyJUlACXFD43869766;     EyJUlACXFD43869766 = EyJUlACXFD71281329;     EyJUlACXFD71281329 = EyJUlACXFD49464347;     EyJUlACXFD49464347 = EyJUlACXFD32220391;     EyJUlACXFD32220391 = EyJUlACXFD37421098;     EyJUlACXFD37421098 = EyJUlACXFD58070495;     EyJUlACXFD58070495 = EyJUlACXFD26021923;     EyJUlACXFD26021923 = EyJUlACXFD64836510;     EyJUlACXFD64836510 = EyJUlACXFD34130056;     EyJUlACXFD34130056 = EyJUlACXFD1515600;     EyJUlACXFD1515600 = EyJUlACXFD7561367;     EyJUlACXFD7561367 = EyJUlACXFD15280691;     EyJUlACXFD15280691 = EyJUlACXFD3729158;     EyJUlACXFD3729158 = EyJUlACXFD58337608;     EyJUlACXFD58337608 = EyJUlACXFD84148535;     EyJUlACXFD84148535 = EyJUlACXFD7612560;     EyJUlACXFD7612560 = EyJUlACXFD92693215;     EyJUlACXFD92693215 = EyJUlACXFD88556293;     EyJUlACXFD88556293 = EyJUlACXFD45361198;     EyJUlACXFD45361198 = EyJUlACXFD50899247;     EyJUlACXFD50899247 = EyJUlACXFD33061443;     EyJUlACXFD33061443 = EyJUlACXFD68080875;     EyJUlACXFD68080875 = EyJUlACXFD72410131;     EyJUlACXFD72410131 = EyJUlACXFD94497516;     EyJUlACXFD94497516 = EyJUlACXFD8396029;     EyJUlACXFD8396029 = EyJUlACXFD87734554;     EyJUlACXFD87734554 = EyJUlACXFD4287984;     EyJUlACXFD4287984 = EyJUlACXFD23599892;     EyJUlACXFD23599892 = EyJUlACXFD59091808;     EyJUlACXFD59091808 = EyJUlACXFD40555170;     EyJUlACXFD40555170 = EyJUlACXFD62000356;     EyJUlACXFD62000356 = EyJUlACXFD70882303;     EyJUlACXFD70882303 = EyJUlACXFD30705212;     EyJUlACXFD30705212 = EyJUlACXFD44629934;     EyJUlACXFD44629934 = EyJUlACXFD63978499;     EyJUlACXFD63978499 = EyJUlACXFD42146504;     EyJUlACXFD42146504 = EyJUlACXFD77302577;     EyJUlACXFD77302577 = EyJUlACXFD81178387;     EyJUlACXFD81178387 = EyJUlACXFD95011280;     EyJUlACXFD95011280 = EyJUlACXFD19860585;     EyJUlACXFD19860585 = EyJUlACXFD83066341;     EyJUlACXFD83066341 = EyJUlACXFD27895080;     EyJUlACXFD27895080 = EyJUlACXFD95253688;     EyJUlACXFD95253688 = EyJUlACXFD6635901;     EyJUlACXFD6635901 = EyJUlACXFD80535600;     EyJUlACXFD80535600 = EyJUlACXFD17399307;     EyJUlACXFD17399307 = EyJUlACXFD74164750;     EyJUlACXFD74164750 = EyJUlACXFD74804101;     EyJUlACXFD74804101 = EyJUlACXFD84799444;     EyJUlACXFD84799444 = EyJUlACXFD14622979;     EyJUlACXFD14622979 = EyJUlACXFD68959394;     EyJUlACXFD68959394 = EyJUlACXFD59403659;     EyJUlACXFD59403659 = EyJUlACXFD27806708;     EyJUlACXFD27806708 = EyJUlACXFD24354887;     EyJUlACXFD24354887 = EyJUlACXFD69809366;     EyJUlACXFD69809366 = EyJUlACXFD75788891;     EyJUlACXFD75788891 = EyJUlACXFD98871197;     EyJUlACXFD98871197 = EyJUlACXFD54966830;     EyJUlACXFD54966830 = EyJUlACXFD23824363;     EyJUlACXFD23824363 = EyJUlACXFD49686543;     EyJUlACXFD49686543 = EyJUlACXFD53782511;     EyJUlACXFD53782511 = EyJUlACXFD2422032;     EyJUlACXFD2422032 = EyJUlACXFD5744703;     EyJUlACXFD5744703 = EyJUlACXFD93574886;     EyJUlACXFD93574886 = EyJUlACXFD39515244;     EyJUlACXFD39515244 = EyJUlACXFD36679063;     EyJUlACXFD36679063 = EyJUlACXFD84575479;     EyJUlACXFD84575479 = EyJUlACXFD59099223;     EyJUlACXFD59099223 = EyJUlACXFD94359108;     EyJUlACXFD94359108 = EyJUlACXFD42002032;     EyJUlACXFD42002032 = EyJUlACXFD30309982;     EyJUlACXFD30309982 = EyJUlACXFD11514828;     EyJUlACXFD11514828 = EyJUlACXFD93545013;     EyJUlACXFD93545013 = EyJUlACXFD25500614;     EyJUlACXFD25500614 = EyJUlACXFD67832905;     EyJUlACXFD67832905 = EyJUlACXFD5166364;     EyJUlACXFD5166364 = EyJUlACXFD72827186;     EyJUlACXFD72827186 = EyJUlACXFD65774231;     EyJUlACXFD65774231 = EyJUlACXFD13961917;     EyJUlACXFD13961917 = EyJUlACXFD90996721;     EyJUlACXFD90996721 = EyJUlACXFD13569805;     EyJUlACXFD13569805 = EyJUlACXFD29483883;     EyJUlACXFD29483883 = EyJUlACXFD38800447;     EyJUlACXFD38800447 = EyJUlACXFD44468830;     EyJUlACXFD44468830 = EyJUlACXFD71595776;     EyJUlACXFD71595776 = EyJUlACXFD9088265;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OWSJMncAHm48056755() {     int xTCwvaqzxP76535300 = -571480472;    int xTCwvaqzxP87927780 = -133472149;    int xTCwvaqzxP6555979 = -666761802;    int xTCwvaqzxP2447407 = -186737608;    int xTCwvaqzxP47219639 = -825810698;    int xTCwvaqzxP55359033 = -730612972;    int xTCwvaqzxP51655033 = -797335058;    int xTCwvaqzxP19821874 = -510014899;    int xTCwvaqzxP69387037 = -796393288;    int xTCwvaqzxP14446715 = -719223225;    int xTCwvaqzxP47430681 = -893483107;    int xTCwvaqzxP28383494 = -614624393;    int xTCwvaqzxP75275609 = -667197660;    int xTCwvaqzxP64000423 = -961038503;    int xTCwvaqzxP11229327 = -368972707;    int xTCwvaqzxP87497823 = -487193196;    int xTCwvaqzxP71012150 = -52038924;    int xTCwvaqzxP77618886 = -436808238;    int xTCwvaqzxP14173353 = -106526940;    int xTCwvaqzxP85636386 = -405142874;    int xTCwvaqzxP92749515 = -617042810;    int xTCwvaqzxP28026065 = -101865468;    int xTCwvaqzxP3188241 = 66577556;    int xTCwvaqzxP36415446 = -556723515;    int xTCwvaqzxP74719785 = -445588495;    int xTCwvaqzxP13546882 = -753355304;    int xTCwvaqzxP58464570 = -423684209;    int xTCwvaqzxP88927557 = -121828927;    int xTCwvaqzxP2173891 = -587287748;    int xTCwvaqzxP55245190 = -847928618;    int xTCwvaqzxP70341405 = -70911131;    int xTCwvaqzxP42279792 = -298557017;    int xTCwvaqzxP16805696 = -556393585;    int xTCwvaqzxP16810627 = -466423085;    int xTCwvaqzxP86105897 = -62477197;    int xTCwvaqzxP72783910 = 72845185;    int xTCwvaqzxP67608055 = -780205905;    int xTCwvaqzxP73173713 = -971221235;    int xTCwvaqzxP15728955 = -443253251;    int xTCwvaqzxP86774260 = -907299774;    int xTCwvaqzxP17269596 = -212363672;    int xTCwvaqzxP62967437 = -716282509;    int xTCwvaqzxP89200184 = -874385470;    int xTCwvaqzxP53482847 = -604239522;    int xTCwvaqzxP96529671 = -692953604;    int xTCwvaqzxP50534119 = -416887535;    int xTCwvaqzxP37240242 = -260659593;    int xTCwvaqzxP8188907 = -539054480;    int xTCwvaqzxP12378361 = -728409124;    int xTCwvaqzxP35725159 = -524129043;    int xTCwvaqzxP22328167 = -247997760;    int xTCwvaqzxP80314017 = 35322229;    int xTCwvaqzxP71943192 = 32168649;    int xTCwvaqzxP86876492 = -391704209;    int xTCwvaqzxP55307240 = -377790587;    int xTCwvaqzxP48509235 = -369615004;    int xTCwvaqzxP84739540 = -100049706;    int xTCwvaqzxP70140533 = -10038288;    int xTCwvaqzxP27727621 = -741149113;    int xTCwvaqzxP33672757 = 27544606;    int xTCwvaqzxP96894463 = -206928763;    int xTCwvaqzxP62727476 = -575506131;    int xTCwvaqzxP17647984 = -922727152;    int xTCwvaqzxP14141847 = -948464671;    int xTCwvaqzxP44105309 = -548312094;    int xTCwvaqzxP5150890 = -494926091;    int xTCwvaqzxP11577798 = 41769192;    int xTCwvaqzxP58464983 = -100774575;    int xTCwvaqzxP77894525 = -798561307;    int xTCwvaqzxP38445416 = -341817892;    int xTCwvaqzxP19889768 = -706987291;    int xTCwvaqzxP97838437 = -80817690;    int xTCwvaqzxP61889931 = -993554988;    int xTCwvaqzxP27399092 = -199227166;    int xTCwvaqzxP68366791 = -92779203;    int xTCwvaqzxP29782079 = -900760302;    int xTCwvaqzxP38825881 = -227479998;    int xTCwvaqzxP49705394 = -329182923;    int xTCwvaqzxP39885775 = -863769912;    int xTCwvaqzxP24185667 = 71299039;    int xTCwvaqzxP76306639 = -392695712;    int xTCwvaqzxP50275663 = -884629729;    int xTCwvaqzxP76549196 = -393419803;    int xTCwvaqzxP66448732 = 36841295;    int xTCwvaqzxP32917023 = -499930859;    int xTCwvaqzxP90027388 = -6233361;    int xTCwvaqzxP70336599 = -230725666;    int xTCwvaqzxP29929204 = -64689376;    int xTCwvaqzxP61503387 = 11367501;    int xTCwvaqzxP37596663 = -692862193;    int xTCwvaqzxP88044370 = -827105110;    int xTCwvaqzxP97467522 = -670167618;    int xTCwvaqzxP45446093 = -130072122;    int xTCwvaqzxP82056197 = -370797857;    int xTCwvaqzxP89879796 = -600371012;    int xTCwvaqzxP54542120 = -636857541;    int xTCwvaqzxP45319454 = -793555357;    int xTCwvaqzxP75058337 = -925920800;    int xTCwvaqzxP9377538 = 44072572;    int xTCwvaqzxP91378782 = -571480472;     xTCwvaqzxP76535300 = xTCwvaqzxP87927780;     xTCwvaqzxP87927780 = xTCwvaqzxP6555979;     xTCwvaqzxP6555979 = xTCwvaqzxP2447407;     xTCwvaqzxP2447407 = xTCwvaqzxP47219639;     xTCwvaqzxP47219639 = xTCwvaqzxP55359033;     xTCwvaqzxP55359033 = xTCwvaqzxP51655033;     xTCwvaqzxP51655033 = xTCwvaqzxP19821874;     xTCwvaqzxP19821874 = xTCwvaqzxP69387037;     xTCwvaqzxP69387037 = xTCwvaqzxP14446715;     xTCwvaqzxP14446715 = xTCwvaqzxP47430681;     xTCwvaqzxP47430681 = xTCwvaqzxP28383494;     xTCwvaqzxP28383494 = xTCwvaqzxP75275609;     xTCwvaqzxP75275609 = xTCwvaqzxP64000423;     xTCwvaqzxP64000423 = xTCwvaqzxP11229327;     xTCwvaqzxP11229327 = xTCwvaqzxP87497823;     xTCwvaqzxP87497823 = xTCwvaqzxP71012150;     xTCwvaqzxP71012150 = xTCwvaqzxP77618886;     xTCwvaqzxP77618886 = xTCwvaqzxP14173353;     xTCwvaqzxP14173353 = xTCwvaqzxP85636386;     xTCwvaqzxP85636386 = xTCwvaqzxP92749515;     xTCwvaqzxP92749515 = xTCwvaqzxP28026065;     xTCwvaqzxP28026065 = xTCwvaqzxP3188241;     xTCwvaqzxP3188241 = xTCwvaqzxP36415446;     xTCwvaqzxP36415446 = xTCwvaqzxP74719785;     xTCwvaqzxP74719785 = xTCwvaqzxP13546882;     xTCwvaqzxP13546882 = xTCwvaqzxP58464570;     xTCwvaqzxP58464570 = xTCwvaqzxP88927557;     xTCwvaqzxP88927557 = xTCwvaqzxP2173891;     xTCwvaqzxP2173891 = xTCwvaqzxP55245190;     xTCwvaqzxP55245190 = xTCwvaqzxP70341405;     xTCwvaqzxP70341405 = xTCwvaqzxP42279792;     xTCwvaqzxP42279792 = xTCwvaqzxP16805696;     xTCwvaqzxP16805696 = xTCwvaqzxP16810627;     xTCwvaqzxP16810627 = xTCwvaqzxP86105897;     xTCwvaqzxP86105897 = xTCwvaqzxP72783910;     xTCwvaqzxP72783910 = xTCwvaqzxP67608055;     xTCwvaqzxP67608055 = xTCwvaqzxP73173713;     xTCwvaqzxP73173713 = xTCwvaqzxP15728955;     xTCwvaqzxP15728955 = xTCwvaqzxP86774260;     xTCwvaqzxP86774260 = xTCwvaqzxP17269596;     xTCwvaqzxP17269596 = xTCwvaqzxP62967437;     xTCwvaqzxP62967437 = xTCwvaqzxP89200184;     xTCwvaqzxP89200184 = xTCwvaqzxP53482847;     xTCwvaqzxP53482847 = xTCwvaqzxP96529671;     xTCwvaqzxP96529671 = xTCwvaqzxP50534119;     xTCwvaqzxP50534119 = xTCwvaqzxP37240242;     xTCwvaqzxP37240242 = xTCwvaqzxP8188907;     xTCwvaqzxP8188907 = xTCwvaqzxP12378361;     xTCwvaqzxP12378361 = xTCwvaqzxP35725159;     xTCwvaqzxP35725159 = xTCwvaqzxP22328167;     xTCwvaqzxP22328167 = xTCwvaqzxP80314017;     xTCwvaqzxP80314017 = xTCwvaqzxP71943192;     xTCwvaqzxP71943192 = xTCwvaqzxP86876492;     xTCwvaqzxP86876492 = xTCwvaqzxP55307240;     xTCwvaqzxP55307240 = xTCwvaqzxP48509235;     xTCwvaqzxP48509235 = xTCwvaqzxP84739540;     xTCwvaqzxP84739540 = xTCwvaqzxP70140533;     xTCwvaqzxP70140533 = xTCwvaqzxP27727621;     xTCwvaqzxP27727621 = xTCwvaqzxP33672757;     xTCwvaqzxP33672757 = xTCwvaqzxP96894463;     xTCwvaqzxP96894463 = xTCwvaqzxP62727476;     xTCwvaqzxP62727476 = xTCwvaqzxP17647984;     xTCwvaqzxP17647984 = xTCwvaqzxP14141847;     xTCwvaqzxP14141847 = xTCwvaqzxP44105309;     xTCwvaqzxP44105309 = xTCwvaqzxP5150890;     xTCwvaqzxP5150890 = xTCwvaqzxP11577798;     xTCwvaqzxP11577798 = xTCwvaqzxP58464983;     xTCwvaqzxP58464983 = xTCwvaqzxP77894525;     xTCwvaqzxP77894525 = xTCwvaqzxP38445416;     xTCwvaqzxP38445416 = xTCwvaqzxP19889768;     xTCwvaqzxP19889768 = xTCwvaqzxP97838437;     xTCwvaqzxP97838437 = xTCwvaqzxP61889931;     xTCwvaqzxP61889931 = xTCwvaqzxP27399092;     xTCwvaqzxP27399092 = xTCwvaqzxP68366791;     xTCwvaqzxP68366791 = xTCwvaqzxP29782079;     xTCwvaqzxP29782079 = xTCwvaqzxP38825881;     xTCwvaqzxP38825881 = xTCwvaqzxP49705394;     xTCwvaqzxP49705394 = xTCwvaqzxP39885775;     xTCwvaqzxP39885775 = xTCwvaqzxP24185667;     xTCwvaqzxP24185667 = xTCwvaqzxP76306639;     xTCwvaqzxP76306639 = xTCwvaqzxP50275663;     xTCwvaqzxP50275663 = xTCwvaqzxP76549196;     xTCwvaqzxP76549196 = xTCwvaqzxP66448732;     xTCwvaqzxP66448732 = xTCwvaqzxP32917023;     xTCwvaqzxP32917023 = xTCwvaqzxP90027388;     xTCwvaqzxP90027388 = xTCwvaqzxP70336599;     xTCwvaqzxP70336599 = xTCwvaqzxP29929204;     xTCwvaqzxP29929204 = xTCwvaqzxP61503387;     xTCwvaqzxP61503387 = xTCwvaqzxP37596663;     xTCwvaqzxP37596663 = xTCwvaqzxP88044370;     xTCwvaqzxP88044370 = xTCwvaqzxP97467522;     xTCwvaqzxP97467522 = xTCwvaqzxP45446093;     xTCwvaqzxP45446093 = xTCwvaqzxP82056197;     xTCwvaqzxP82056197 = xTCwvaqzxP89879796;     xTCwvaqzxP89879796 = xTCwvaqzxP54542120;     xTCwvaqzxP54542120 = xTCwvaqzxP45319454;     xTCwvaqzxP45319454 = xTCwvaqzxP75058337;     xTCwvaqzxP75058337 = xTCwvaqzxP9377538;     xTCwvaqzxP9377538 = xTCwvaqzxP91378782;     xTCwvaqzxP91378782 = xTCwvaqzxP76535300;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MdJNVAJvGc66270097() {     int vvRpkLGrUd59979606 = -352146152;    int vvRpkLGrUd4799870 = -132854337;    int vvRpkLGrUd2964486 = -460287030;    int vvRpkLGrUd65239657 = -380537444;    int vvRpkLGrUd53340963 = -632678189;    int vvRpkLGrUd53863434 = -509684625;    int vvRpkLGrUd22694351 = -537246371;    int vvRpkLGrUd34169317 = -497172928;    int vvRpkLGrUd27916738 = -37852639;    int vvRpkLGrUd46992366 = -87599337;    int vvRpkLGrUd96846618 = 16401020;    int vvRpkLGrUd91588892 = -602242639;    int vvRpkLGrUd93393385 = -458858901;    int vvRpkLGrUd43627315 = -749328860;    int vvRpkLGrUd72083790 = -462995689;    int vvRpkLGrUd73886984 = -275272521;    int vvRpkLGrUd64764035 = -905383951;    int vvRpkLGrUd58470332 = -684113114;    int vvRpkLGrUd76613494 = -105201811;    int vvRpkLGrUd75003829 = -934728867;    int vvRpkLGrUd48389903 = -206571841;    int vvRpkLGrUd40173384 = -93017238;    int vvRpkLGrUd7013949 = 26688359;    int vvRpkLGrUd71122004 = -628548911;    int vvRpkLGrUd16295744 = 94758746;    int vvRpkLGrUd8889031 = -633865020;    int vvRpkLGrUd7919310 = -51768552;    int vvRpkLGrUd28659183 = -191783811;    int vvRpkLGrUd38254985 = -449792890;    int vvRpkLGrUd71078531 = -260778469;    int vvRpkLGrUd88436719 = -808668332;    int vvRpkLGrUd5386219 = -213752097;    int vvRpkLGrUd60344583 = -177265762;    int vvRpkLGrUd58969137 = -58025255;    int vvRpkLGrUd64832628 = -260141932;    int vvRpkLGrUd85173094 = -815178721;    int vvRpkLGrUd36222234 = -659055085;    int vvRpkLGrUd13752546 = -679691755;    int vvRpkLGrUd64724033 = -628151048;    int vvRpkLGrUd36269395 = -659893297;    int vvRpkLGrUd50918520 = -883912645;    int vvRpkLGrUd13455224 = -190190830;    int vvRpkLGrUd12767254 = -325442713;    int vvRpkLGrUd45387787 = -360422293;    int vvRpkLGrUd88474168 = -421110557;    int vvRpkLGrUd90140114 = -436498459;    int vvRpkLGrUd13255673 = -392963823;    int vvRpkLGrUd27859502 = -537776199;    int vvRpkLGrUd43225473 = -214011206;    int vvRpkLGrUd71057642 = -731149498;    int vvRpkLGrUd95585615 = -468899182;    int vvRpkLGrUd35814812 = -22937082;    int vvRpkLGrUd86055651 = -179578667;    int vvRpkLGrUd11553251 = -110117597;    int vvRpkLGrUd17880194 = -726276710;    int vvRpkLGrUd19806223 = -159128915;    int vvRpkLGrUd97785921 = -59542696;    int vvRpkLGrUd31842482 = -831738119;    int vvRpkLGrUd48943914 = -375296190;    int vvRpkLGrUd44451932 = -998813170;    int vvRpkLGrUd45944124 = -357916074;    int vvRpkLGrUd94035168 = -245462561;    int vvRpkLGrUd95914331 = 52619961;    int vvRpkLGrUd56838206 = -777074170;    int vvRpkLGrUd58555647 = -278931005;    int vvRpkLGrUd91460400 = -769846884;    int vvRpkLGrUd31244310 = -324976877;    int vvRpkLGrUd34424248 = -300833646;    int vvRpkLGrUd78794686 = -389186929;    int vvRpkLGrUd86910695 = -647816968;    int vvRpkLGrUd37664751 = -616217437;    int vvRpkLGrUd51011490 = -125692197;    int vvRpkLGrUd93746299 = 44037934;    int vvRpkLGrUd40344099 = -445308515;    int vvRpkLGrUd24085309 = 49183778;    int vvRpkLGrUd34934679 = 83618989;    int vvRpkLGrUd27406130 = -767574525;    int vvRpkLGrUd61626161 = -612889349;    int vvRpkLGrUd82647836 = -107438354;    int vvRpkLGrUd26155629 = -468742796;    int vvRpkLGrUd95633358 = -140901198;    int vvRpkLGrUd80059807 = -513992353;    int vvRpkLGrUd85433709 = -977772605;    int vvRpkLGrUd67197342 = -718643392;    int vvRpkLGrUd75492916 = -791879288;    int vvRpkLGrUd52621908 = -685731251;    int vvRpkLGrUd19330568 = 65826570;    int vvRpkLGrUd48791332 = 32851834;    int vvRpkLGrUd41088943 = -331748546;    int vvRpkLGrUd45026406 = -1013018;    int vvRpkLGrUd87387173 = -655636025;    int vvRpkLGrUd4379753 = -827316966;    int vvRpkLGrUd64808631 = -204395565;    int vvRpkLGrUd20272101 = -629337879;    int vvRpkLGrUd90325271 = -201977223;    int vvRpkLGrUd56883352 = -538450085;    int vvRpkLGrUd17540892 = -142810791;    int vvRpkLGrUd55929047 = -548368544;    int vvRpkLGrUd86832139 = 18508712;    int vvRpkLGrUd97013768 = -352146152;     vvRpkLGrUd59979606 = vvRpkLGrUd4799870;     vvRpkLGrUd4799870 = vvRpkLGrUd2964486;     vvRpkLGrUd2964486 = vvRpkLGrUd65239657;     vvRpkLGrUd65239657 = vvRpkLGrUd53340963;     vvRpkLGrUd53340963 = vvRpkLGrUd53863434;     vvRpkLGrUd53863434 = vvRpkLGrUd22694351;     vvRpkLGrUd22694351 = vvRpkLGrUd34169317;     vvRpkLGrUd34169317 = vvRpkLGrUd27916738;     vvRpkLGrUd27916738 = vvRpkLGrUd46992366;     vvRpkLGrUd46992366 = vvRpkLGrUd96846618;     vvRpkLGrUd96846618 = vvRpkLGrUd91588892;     vvRpkLGrUd91588892 = vvRpkLGrUd93393385;     vvRpkLGrUd93393385 = vvRpkLGrUd43627315;     vvRpkLGrUd43627315 = vvRpkLGrUd72083790;     vvRpkLGrUd72083790 = vvRpkLGrUd73886984;     vvRpkLGrUd73886984 = vvRpkLGrUd64764035;     vvRpkLGrUd64764035 = vvRpkLGrUd58470332;     vvRpkLGrUd58470332 = vvRpkLGrUd76613494;     vvRpkLGrUd76613494 = vvRpkLGrUd75003829;     vvRpkLGrUd75003829 = vvRpkLGrUd48389903;     vvRpkLGrUd48389903 = vvRpkLGrUd40173384;     vvRpkLGrUd40173384 = vvRpkLGrUd7013949;     vvRpkLGrUd7013949 = vvRpkLGrUd71122004;     vvRpkLGrUd71122004 = vvRpkLGrUd16295744;     vvRpkLGrUd16295744 = vvRpkLGrUd8889031;     vvRpkLGrUd8889031 = vvRpkLGrUd7919310;     vvRpkLGrUd7919310 = vvRpkLGrUd28659183;     vvRpkLGrUd28659183 = vvRpkLGrUd38254985;     vvRpkLGrUd38254985 = vvRpkLGrUd71078531;     vvRpkLGrUd71078531 = vvRpkLGrUd88436719;     vvRpkLGrUd88436719 = vvRpkLGrUd5386219;     vvRpkLGrUd5386219 = vvRpkLGrUd60344583;     vvRpkLGrUd60344583 = vvRpkLGrUd58969137;     vvRpkLGrUd58969137 = vvRpkLGrUd64832628;     vvRpkLGrUd64832628 = vvRpkLGrUd85173094;     vvRpkLGrUd85173094 = vvRpkLGrUd36222234;     vvRpkLGrUd36222234 = vvRpkLGrUd13752546;     vvRpkLGrUd13752546 = vvRpkLGrUd64724033;     vvRpkLGrUd64724033 = vvRpkLGrUd36269395;     vvRpkLGrUd36269395 = vvRpkLGrUd50918520;     vvRpkLGrUd50918520 = vvRpkLGrUd13455224;     vvRpkLGrUd13455224 = vvRpkLGrUd12767254;     vvRpkLGrUd12767254 = vvRpkLGrUd45387787;     vvRpkLGrUd45387787 = vvRpkLGrUd88474168;     vvRpkLGrUd88474168 = vvRpkLGrUd90140114;     vvRpkLGrUd90140114 = vvRpkLGrUd13255673;     vvRpkLGrUd13255673 = vvRpkLGrUd27859502;     vvRpkLGrUd27859502 = vvRpkLGrUd43225473;     vvRpkLGrUd43225473 = vvRpkLGrUd71057642;     vvRpkLGrUd71057642 = vvRpkLGrUd95585615;     vvRpkLGrUd95585615 = vvRpkLGrUd35814812;     vvRpkLGrUd35814812 = vvRpkLGrUd86055651;     vvRpkLGrUd86055651 = vvRpkLGrUd11553251;     vvRpkLGrUd11553251 = vvRpkLGrUd17880194;     vvRpkLGrUd17880194 = vvRpkLGrUd19806223;     vvRpkLGrUd19806223 = vvRpkLGrUd97785921;     vvRpkLGrUd97785921 = vvRpkLGrUd31842482;     vvRpkLGrUd31842482 = vvRpkLGrUd48943914;     vvRpkLGrUd48943914 = vvRpkLGrUd44451932;     vvRpkLGrUd44451932 = vvRpkLGrUd45944124;     vvRpkLGrUd45944124 = vvRpkLGrUd94035168;     vvRpkLGrUd94035168 = vvRpkLGrUd95914331;     vvRpkLGrUd95914331 = vvRpkLGrUd56838206;     vvRpkLGrUd56838206 = vvRpkLGrUd58555647;     vvRpkLGrUd58555647 = vvRpkLGrUd91460400;     vvRpkLGrUd91460400 = vvRpkLGrUd31244310;     vvRpkLGrUd31244310 = vvRpkLGrUd34424248;     vvRpkLGrUd34424248 = vvRpkLGrUd78794686;     vvRpkLGrUd78794686 = vvRpkLGrUd86910695;     vvRpkLGrUd86910695 = vvRpkLGrUd37664751;     vvRpkLGrUd37664751 = vvRpkLGrUd51011490;     vvRpkLGrUd51011490 = vvRpkLGrUd93746299;     vvRpkLGrUd93746299 = vvRpkLGrUd40344099;     vvRpkLGrUd40344099 = vvRpkLGrUd24085309;     vvRpkLGrUd24085309 = vvRpkLGrUd34934679;     vvRpkLGrUd34934679 = vvRpkLGrUd27406130;     vvRpkLGrUd27406130 = vvRpkLGrUd61626161;     vvRpkLGrUd61626161 = vvRpkLGrUd82647836;     vvRpkLGrUd82647836 = vvRpkLGrUd26155629;     vvRpkLGrUd26155629 = vvRpkLGrUd95633358;     vvRpkLGrUd95633358 = vvRpkLGrUd80059807;     vvRpkLGrUd80059807 = vvRpkLGrUd85433709;     vvRpkLGrUd85433709 = vvRpkLGrUd67197342;     vvRpkLGrUd67197342 = vvRpkLGrUd75492916;     vvRpkLGrUd75492916 = vvRpkLGrUd52621908;     vvRpkLGrUd52621908 = vvRpkLGrUd19330568;     vvRpkLGrUd19330568 = vvRpkLGrUd48791332;     vvRpkLGrUd48791332 = vvRpkLGrUd41088943;     vvRpkLGrUd41088943 = vvRpkLGrUd45026406;     vvRpkLGrUd45026406 = vvRpkLGrUd87387173;     vvRpkLGrUd87387173 = vvRpkLGrUd4379753;     vvRpkLGrUd4379753 = vvRpkLGrUd64808631;     vvRpkLGrUd64808631 = vvRpkLGrUd20272101;     vvRpkLGrUd20272101 = vvRpkLGrUd90325271;     vvRpkLGrUd90325271 = vvRpkLGrUd56883352;     vvRpkLGrUd56883352 = vvRpkLGrUd17540892;     vvRpkLGrUd17540892 = vvRpkLGrUd55929047;     vvRpkLGrUd55929047 = vvRpkLGrUd86832139;     vvRpkLGrUd86832139 = vvRpkLGrUd97013768;     vvRpkLGrUd97013768 = vvRpkLGrUd59979606;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GcuCSpehWr36725970() {     int BUfGIGjTJm72765773 = -487186844;    int BUfGIGjTJm91570758 = -303188351;    int BUfGIGjTJm20717286 = -868646412;    int BUfGIGjTJm73180483 = -204739721;    int BUfGIGjTJm46852067 = -24494654;    int BUfGIGjTJm97173504 = -286607938;    int BUfGIGjTJm43902109 = 17081559;    int BUfGIGjTJm60888429 = -941691185;    int BUfGIGjTJm23321548 = 389269;    int BUfGIGjTJm53211071 = -991686970;    int BUfGIGjTJm24334001 = -98284995;    int BUfGIGjTJm23539398 = -9575455;    int BUfGIGjTJm63536688 = -362641558;    int BUfGIGjTJm93251393 = 18255941;    int BUfGIGjTJm95390343 = -369981520;    int BUfGIGjTJm2312054 = -826163911;    int BUfGIGjTJm20515853 = -569158418;    int BUfGIGjTJm42028818 = -239323416;    int BUfGIGjTJm77774124 = -14671007;    int BUfGIGjTJm77658325 = -433973006;    int BUfGIGjTJm28563960 = -697888105;    int BUfGIGjTJm69221202 = -262875586;    int BUfGIGjTJm45774244 = 95684464;    int BUfGIGjTJm15629134 = -988893924;    int BUfGIGjTJm22190003 = -115683179;    int BUfGIGjTJm90207676 = -324296219;    int BUfGIGjTJm19334846 = -685474832;    int BUfGIGjTJm96012107 = -365581915;    int BUfGIGjTJm54872038 = -664010130;    int BUfGIGjTJm23829647 = -990664624;    int BUfGIGjTJm32872715 = 14914918;    int BUfGIGjTJm8705043 = -192471775;    int BUfGIGjTJm6390532 = -53120547;    int BUfGIGjTJm36045510 = 37220526;    int BUfGIGjTJm47893160 = -870173813;    int BUfGIGjTJm80397209 = -603741825;    int BUfGIGjTJm72986294 = -448964436;    int BUfGIGjTJm71456568 = -599311846;    int BUfGIGjTJm70499419 = -470001625;    int BUfGIGjTJm62534125 = -504310142;    int BUfGIGjTJm12349829 = -159386638;    int BUfGIGjTJm21157679 = -88502164;    int BUfGIGjTJm47650724 = -348876158;    int BUfGIGjTJm90016324 = -701594535;    int BUfGIGjTJm68482659 = -313880518;    int BUfGIGjTJm63768244 = -509816898;    int BUfGIGjTJm90837691 = -890478875;    int BUfGIGjTJm57299468 = -693128978;    int BUfGIGjTJm87289522 = -377613181;    int BUfGIGjTJm18008064 = -242185961;    int BUfGIGjTJm13913109 = -721783490;    int BUfGIGjTJm63991358 = -494122420;    int BUfGIGjTJm20540802 = -355581950;    int BUfGIGjTJm56252976 = -799569339;    int BUfGIGjTJm8162011 = -720721196;    int BUfGIGjTJm3544571 = -124311259;    int BUfGIGjTJm45796514 = -298872815;    int BUfGIGjTJm5088152 = -879752489;    int BUfGIGjTJm50990481 = 10943458;    int BUfGIGjTJm56644391 = -700198435;    int BUfGIGjTJm77838658 = -601133107;    int BUfGIGjTJm47890002 = -617336527;    int BUfGIGjTJm6016392 = -177681056;    int BUfGIGjTJm99491901 = -8946108;    int BUfGIGjTJm20338356 = -906601888;    int BUfGIGjTJm15628959 = -905813221;    int BUfGIGjTJm17148866 = -956454909;    int BUfGIGjTJm27491178 = -299862085;    int BUfGIGjTJm45358233 = -111570247;    int BUfGIGjTJm14993134 = -766239695;    int BUfGIGjTJm29325759 = -277199475;    int BUfGIGjTJm49059285 = -969846572;    int BUfGIGjTJm71529398 = -769321791;    int BUfGIGjTJm15240000 = -510360865;    int BUfGIGjTJm65308497 = -174586368;    int BUfGIGjTJm7406281 = -509385941;    int BUfGIGjTJm21570479 = -913999428;    int BUfGIGjTJm55757919 = -202721001;    int BUfGIGjTJm47146475 = -575013406;    int BUfGIGjTJm58421759 = -605866281;    int BUfGIGjTJm99369984 = -433817344;    int BUfGIGjTJm62035377 = -992345854;    int BUfGIGjTJm8722585 = -987968735;    int BUfGIGjTJm36863975 = -321824169;    int BUfGIGjTJm9916539 = -168881134;    int BUfGIGjTJm68881356 = -490962663;    int BUfGIGjTJm88164241 = -836889826;    int BUfGIGjTJm50137556 = -253551208;    int BUfGIGjTJm27883500 = -242058278;    int BUfGIGjTJm44348590 = -645862554;    int BUfGIGjTJm34600695 = -204869010;    int BUfGIGjTJm67898142 = -569211947;    int BUfGIGjTJm20466088 = -510255304;    int BUfGIGjTJm13855028 = -769803190;    int BUfGIGjTJm84695466 = -903177036;    int BUfGIGjTJm64459826 = -542050112;    int BUfGIGjTJm15141288 = -910821109;    int BUfGIGjTJm48158822 = -239930051;    int BUfGIGjTJm69677969 = -794992648;    int BUfGIGjTJm52853700 = -487186844;     BUfGIGjTJm72765773 = BUfGIGjTJm91570758;     BUfGIGjTJm91570758 = BUfGIGjTJm20717286;     BUfGIGjTJm20717286 = BUfGIGjTJm73180483;     BUfGIGjTJm73180483 = BUfGIGjTJm46852067;     BUfGIGjTJm46852067 = BUfGIGjTJm97173504;     BUfGIGjTJm97173504 = BUfGIGjTJm43902109;     BUfGIGjTJm43902109 = BUfGIGjTJm60888429;     BUfGIGjTJm60888429 = BUfGIGjTJm23321548;     BUfGIGjTJm23321548 = BUfGIGjTJm53211071;     BUfGIGjTJm53211071 = BUfGIGjTJm24334001;     BUfGIGjTJm24334001 = BUfGIGjTJm23539398;     BUfGIGjTJm23539398 = BUfGIGjTJm63536688;     BUfGIGjTJm63536688 = BUfGIGjTJm93251393;     BUfGIGjTJm93251393 = BUfGIGjTJm95390343;     BUfGIGjTJm95390343 = BUfGIGjTJm2312054;     BUfGIGjTJm2312054 = BUfGIGjTJm20515853;     BUfGIGjTJm20515853 = BUfGIGjTJm42028818;     BUfGIGjTJm42028818 = BUfGIGjTJm77774124;     BUfGIGjTJm77774124 = BUfGIGjTJm77658325;     BUfGIGjTJm77658325 = BUfGIGjTJm28563960;     BUfGIGjTJm28563960 = BUfGIGjTJm69221202;     BUfGIGjTJm69221202 = BUfGIGjTJm45774244;     BUfGIGjTJm45774244 = BUfGIGjTJm15629134;     BUfGIGjTJm15629134 = BUfGIGjTJm22190003;     BUfGIGjTJm22190003 = BUfGIGjTJm90207676;     BUfGIGjTJm90207676 = BUfGIGjTJm19334846;     BUfGIGjTJm19334846 = BUfGIGjTJm96012107;     BUfGIGjTJm96012107 = BUfGIGjTJm54872038;     BUfGIGjTJm54872038 = BUfGIGjTJm23829647;     BUfGIGjTJm23829647 = BUfGIGjTJm32872715;     BUfGIGjTJm32872715 = BUfGIGjTJm8705043;     BUfGIGjTJm8705043 = BUfGIGjTJm6390532;     BUfGIGjTJm6390532 = BUfGIGjTJm36045510;     BUfGIGjTJm36045510 = BUfGIGjTJm47893160;     BUfGIGjTJm47893160 = BUfGIGjTJm80397209;     BUfGIGjTJm80397209 = BUfGIGjTJm72986294;     BUfGIGjTJm72986294 = BUfGIGjTJm71456568;     BUfGIGjTJm71456568 = BUfGIGjTJm70499419;     BUfGIGjTJm70499419 = BUfGIGjTJm62534125;     BUfGIGjTJm62534125 = BUfGIGjTJm12349829;     BUfGIGjTJm12349829 = BUfGIGjTJm21157679;     BUfGIGjTJm21157679 = BUfGIGjTJm47650724;     BUfGIGjTJm47650724 = BUfGIGjTJm90016324;     BUfGIGjTJm90016324 = BUfGIGjTJm68482659;     BUfGIGjTJm68482659 = BUfGIGjTJm63768244;     BUfGIGjTJm63768244 = BUfGIGjTJm90837691;     BUfGIGjTJm90837691 = BUfGIGjTJm57299468;     BUfGIGjTJm57299468 = BUfGIGjTJm87289522;     BUfGIGjTJm87289522 = BUfGIGjTJm18008064;     BUfGIGjTJm18008064 = BUfGIGjTJm13913109;     BUfGIGjTJm13913109 = BUfGIGjTJm63991358;     BUfGIGjTJm63991358 = BUfGIGjTJm20540802;     BUfGIGjTJm20540802 = BUfGIGjTJm56252976;     BUfGIGjTJm56252976 = BUfGIGjTJm8162011;     BUfGIGjTJm8162011 = BUfGIGjTJm3544571;     BUfGIGjTJm3544571 = BUfGIGjTJm45796514;     BUfGIGjTJm45796514 = BUfGIGjTJm5088152;     BUfGIGjTJm5088152 = BUfGIGjTJm50990481;     BUfGIGjTJm50990481 = BUfGIGjTJm56644391;     BUfGIGjTJm56644391 = BUfGIGjTJm77838658;     BUfGIGjTJm77838658 = BUfGIGjTJm47890002;     BUfGIGjTJm47890002 = BUfGIGjTJm6016392;     BUfGIGjTJm6016392 = BUfGIGjTJm99491901;     BUfGIGjTJm99491901 = BUfGIGjTJm20338356;     BUfGIGjTJm20338356 = BUfGIGjTJm15628959;     BUfGIGjTJm15628959 = BUfGIGjTJm17148866;     BUfGIGjTJm17148866 = BUfGIGjTJm27491178;     BUfGIGjTJm27491178 = BUfGIGjTJm45358233;     BUfGIGjTJm45358233 = BUfGIGjTJm14993134;     BUfGIGjTJm14993134 = BUfGIGjTJm29325759;     BUfGIGjTJm29325759 = BUfGIGjTJm49059285;     BUfGIGjTJm49059285 = BUfGIGjTJm71529398;     BUfGIGjTJm71529398 = BUfGIGjTJm15240000;     BUfGIGjTJm15240000 = BUfGIGjTJm65308497;     BUfGIGjTJm65308497 = BUfGIGjTJm7406281;     BUfGIGjTJm7406281 = BUfGIGjTJm21570479;     BUfGIGjTJm21570479 = BUfGIGjTJm55757919;     BUfGIGjTJm55757919 = BUfGIGjTJm47146475;     BUfGIGjTJm47146475 = BUfGIGjTJm58421759;     BUfGIGjTJm58421759 = BUfGIGjTJm99369984;     BUfGIGjTJm99369984 = BUfGIGjTJm62035377;     BUfGIGjTJm62035377 = BUfGIGjTJm8722585;     BUfGIGjTJm8722585 = BUfGIGjTJm36863975;     BUfGIGjTJm36863975 = BUfGIGjTJm9916539;     BUfGIGjTJm9916539 = BUfGIGjTJm68881356;     BUfGIGjTJm68881356 = BUfGIGjTJm88164241;     BUfGIGjTJm88164241 = BUfGIGjTJm50137556;     BUfGIGjTJm50137556 = BUfGIGjTJm27883500;     BUfGIGjTJm27883500 = BUfGIGjTJm44348590;     BUfGIGjTJm44348590 = BUfGIGjTJm34600695;     BUfGIGjTJm34600695 = BUfGIGjTJm67898142;     BUfGIGjTJm67898142 = BUfGIGjTJm20466088;     BUfGIGjTJm20466088 = BUfGIGjTJm13855028;     BUfGIGjTJm13855028 = BUfGIGjTJm84695466;     BUfGIGjTJm84695466 = BUfGIGjTJm64459826;     BUfGIGjTJm64459826 = BUfGIGjTJm15141288;     BUfGIGjTJm15141288 = BUfGIGjTJm48158822;     BUfGIGjTJm48158822 = BUfGIGjTJm69677969;     BUfGIGjTJm69677969 = BUfGIGjTJm52853700;     BUfGIGjTJm52853700 = BUfGIGjTJm72765773;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CWhcjMJXAG54939311() {     int uTtOZyHoBY56210078 = -267852525;    int uTtOZyHoBY8442848 = -302570538;    int uTtOZyHoBY17125793 = -662171640;    int uTtOZyHoBY35972735 = -398539556;    int uTtOZyHoBY52973391 = -931362145;    int uTtOZyHoBY95677905 = -65679592;    int uTtOZyHoBY14941427 = -822829755;    int uTtOZyHoBY75235872 = -928849215;    int uTtOZyHoBY81851248 = -341070082;    int uTtOZyHoBY85756723 = -360063082;    int uTtOZyHoBY73749939 = -288400868;    int uTtOZyHoBY86744797 = 2806299;    int uTtOZyHoBY81654464 = -154302799;    int uTtOZyHoBY72878285 = -870034415;    int uTtOZyHoBY56244806 = -464004502;    int uTtOZyHoBY88701215 = -614243236;    int uTtOZyHoBY14267738 = -322503445;    int uTtOZyHoBY22880264 = -486628293;    int uTtOZyHoBY40214267 = -13345877;    int uTtOZyHoBY67025768 = -963558998;    int uTtOZyHoBY84204346 = -287417136;    int uTtOZyHoBY81368520 = -254027356;    int uTtOZyHoBY49599951 = 55795267;    int uTtOZyHoBY50335692 = 39280680;    int uTtOZyHoBY63765960 = -675335938;    int uTtOZyHoBY85549825 = -204805935;    int uTtOZyHoBY68789586 = -313559174;    int uTtOZyHoBY35743733 = -435536799;    int uTtOZyHoBY90953132 = -526515272;    int uTtOZyHoBY39662988 = -403514476;    int uTtOZyHoBY50968029 = -722842283;    int uTtOZyHoBY71811469 = -107666855;    int uTtOZyHoBY49929419 = -773992724;    int uTtOZyHoBY78204020 = -654381644;    int uTtOZyHoBY26619891 = 32161452;    int uTtOZyHoBY92786393 = -391765731;    int uTtOZyHoBY41600473 = -327813616;    int uTtOZyHoBY12035401 = -307782366;    int uTtOZyHoBY19494497 = -654899423;    int uTtOZyHoBY12029260 = -256903665;    int uTtOZyHoBY45998753 = -830935612;    int uTtOZyHoBY71645465 = -662410486;    int uTtOZyHoBY71217793 = -899933401;    int uTtOZyHoBY81921265 = -457777306;    int uTtOZyHoBY60427155 = -42037471;    int uTtOZyHoBY3374240 = -529427822;    int uTtOZyHoBY66853122 = 77216895;    int uTtOZyHoBY76970064 = -691850697;    int uTtOZyHoBY18136635 = -963215263;    int uTtOZyHoBY53340547 = -449206417;    int uTtOZyHoBY87170556 = -942684913;    int uTtOZyHoBY19492153 = -552381732;    int uTtOZyHoBY34653260 = -567329266;    int uTtOZyHoBY80929735 = -517982726;    int uTtOZyHoBY70734965 = 30792681;    int uTtOZyHoBY74841558 = 86174831;    int uTtOZyHoBY58842896 = -258365806;    int uTtOZyHoBY66790100 = -601452321;    int uTtOZyHoBY72206774 = -723203619;    int uTtOZyHoBY67423566 = -626556211;    int uTtOZyHoBY26888319 = -752120418;    int uTtOZyHoBY79197694 = -287292956;    int uTtOZyHoBY84282740 = -302333943;    int uTtOZyHoBY42188261 = -937555607;    int uTtOZyHoBY34788694 = -637220799;    int uTtOZyHoBY1938470 = -80734014;    int uTtOZyHoBY36815379 = -223200977;    int uTtOZyHoBY3450444 = -499921156;    int uTtOZyHoBY46258395 = -802195868;    int uTtOZyHoBY63458413 = 27761229;    int uTtOZyHoBY47100742 = -186429621;    int uTtOZyHoBY2232338 = 85278921;    int uTtOZyHoBY3385767 = -831728870;    int uTtOZyHoBY28185007 = -756442213;    int uTtOZyHoBY21027016 = -32623387;    int uTtOZyHoBY12558882 = -625006651;    int uTtOZyHoBY10150728 = -354093956;    int uTtOZyHoBY67678686 = -486427427;    int uTtOZyHoBY89908536 = -918681849;    int uTtOZyHoBY60391721 = -45908116;    int uTtOZyHoBY18696703 = -182022831;    int uTtOZyHoBY91819522 = -621708478;    int uTtOZyHoBY17607098 = -472321536;    int uTtOZyHoBY37612585 = 22691144;    int uTtOZyHoBY52492431 = -460829564;    int uTtOZyHoBY31475876 = -70460552;    int uTtOZyHoBY37158209 = -540337589;    int uTtOZyHoBY68999683 = -156009998;    int uTtOZyHoBY7469056 = -585174325;    int uTtOZyHoBY51778333 = 45986621;    int uTtOZyHoBY33943498 = -33399926;    int uTtOZyHoBY74810372 = -726361295;    int uTtOZyHoBY39828626 = -584578747;    int uTtOZyHoBY52070931 = 71656787;    int uTtOZyHoBY85140940 = -504783247;    int uTtOZyHoBY66801058 = -443642656;    int uTtOZyHoBY87362725 = -260076543;    int uTtOZyHoBY29029533 = -962377794;    int uTtOZyHoBY47132571 = -820556507;    int uTtOZyHoBY58488686 = -267852525;     uTtOZyHoBY56210078 = uTtOZyHoBY8442848;     uTtOZyHoBY8442848 = uTtOZyHoBY17125793;     uTtOZyHoBY17125793 = uTtOZyHoBY35972735;     uTtOZyHoBY35972735 = uTtOZyHoBY52973391;     uTtOZyHoBY52973391 = uTtOZyHoBY95677905;     uTtOZyHoBY95677905 = uTtOZyHoBY14941427;     uTtOZyHoBY14941427 = uTtOZyHoBY75235872;     uTtOZyHoBY75235872 = uTtOZyHoBY81851248;     uTtOZyHoBY81851248 = uTtOZyHoBY85756723;     uTtOZyHoBY85756723 = uTtOZyHoBY73749939;     uTtOZyHoBY73749939 = uTtOZyHoBY86744797;     uTtOZyHoBY86744797 = uTtOZyHoBY81654464;     uTtOZyHoBY81654464 = uTtOZyHoBY72878285;     uTtOZyHoBY72878285 = uTtOZyHoBY56244806;     uTtOZyHoBY56244806 = uTtOZyHoBY88701215;     uTtOZyHoBY88701215 = uTtOZyHoBY14267738;     uTtOZyHoBY14267738 = uTtOZyHoBY22880264;     uTtOZyHoBY22880264 = uTtOZyHoBY40214267;     uTtOZyHoBY40214267 = uTtOZyHoBY67025768;     uTtOZyHoBY67025768 = uTtOZyHoBY84204346;     uTtOZyHoBY84204346 = uTtOZyHoBY81368520;     uTtOZyHoBY81368520 = uTtOZyHoBY49599951;     uTtOZyHoBY49599951 = uTtOZyHoBY50335692;     uTtOZyHoBY50335692 = uTtOZyHoBY63765960;     uTtOZyHoBY63765960 = uTtOZyHoBY85549825;     uTtOZyHoBY85549825 = uTtOZyHoBY68789586;     uTtOZyHoBY68789586 = uTtOZyHoBY35743733;     uTtOZyHoBY35743733 = uTtOZyHoBY90953132;     uTtOZyHoBY90953132 = uTtOZyHoBY39662988;     uTtOZyHoBY39662988 = uTtOZyHoBY50968029;     uTtOZyHoBY50968029 = uTtOZyHoBY71811469;     uTtOZyHoBY71811469 = uTtOZyHoBY49929419;     uTtOZyHoBY49929419 = uTtOZyHoBY78204020;     uTtOZyHoBY78204020 = uTtOZyHoBY26619891;     uTtOZyHoBY26619891 = uTtOZyHoBY92786393;     uTtOZyHoBY92786393 = uTtOZyHoBY41600473;     uTtOZyHoBY41600473 = uTtOZyHoBY12035401;     uTtOZyHoBY12035401 = uTtOZyHoBY19494497;     uTtOZyHoBY19494497 = uTtOZyHoBY12029260;     uTtOZyHoBY12029260 = uTtOZyHoBY45998753;     uTtOZyHoBY45998753 = uTtOZyHoBY71645465;     uTtOZyHoBY71645465 = uTtOZyHoBY71217793;     uTtOZyHoBY71217793 = uTtOZyHoBY81921265;     uTtOZyHoBY81921265 = uTtOZyHoBY60427155;     uTtOZyHoBY60427155 = uTtOZyHoBY3374240;     uTtOZyHoBY3374240 = uTtOZyHoBY66853122;     uTtOZyHoBY66853122 = uTtOZyHoBY76970064;     uTtOZyHoBY76970064 = uTtOZyHoBY18136635;     uTtOZyHoBY18136635 = uTtOZyHoBY53340547;     uTtOZyHoBY53340547 = uTtOZyHoBY87170556;     uTtOZyHoBY87170556 = uTtOZyHoBY19492153;     uTtOZyHoBY19492153 = uTtOZyHoBY34653260;     uTtOZyHoBY34653260 = uTtOZyHoBY80929735;     uTtOZyHoBY80929735 = uTtOZyHoBY70734965;     uTtOZyHoBY70734965 = uTtOZyHoBY74841558;     uTtOZyHoBY74841558 = uTtOZyHoBY58842896;     uTtOZyHoBY58842896 = uTtOZyHoBY66790100;     uTtOZyHoBY66790100 = uTtOZyHoBY72206774;     uTtOZyHoBY72206774 = uTtOZyHoBY67423566;     uTtOZyHoBY67423566 = uTtOZyHoBY26888319;     uTtOZyHoBY26888319 = uTtOZyHoBY79197694;     uTtOZyHoBY79197694 = uTtOZyHoBY84282740;     uTtOZyHoBY84282740 = uTtOZyHoBY42188261;     uTtOZyHoBY42188261 = uTtOZyHoBY34788694;     uTtOZyHoBY34788694 = uTtOZyHoBY1938470;     uTtOZyHoBY1938470 = uTtOZyHoBY36815379;     uTtOZyHoBY36815379 = uTtOZyHoBY3450444;     uTtOZyHoBY3450444 = uTtOZyHoBY46258395;     uTtOZyHoBY46258395 = uTtOZyHoBY63458413;     uTtOZyHoBY63458413 = uTtOZyHoBY47100742;     uTtOZyHoBY47100742 = uTtOZyHoBY2232338;     uTtOZyHoBY2232338 = uTtOZyHoBY3385767;     uTtOZyHoBY3385767 = uTtOZyHoBY28185007;     uTtOZyHoBY28185007 = uTtOZyHoBY21027016;     uTtOZyHoBY21027016 = uTtOZyHoBY12558882;     uTtOZyHoBY12558882 = uTtOZyHoBY10150728;     uTtOZyHoBY10150728 = uTtOZyHoBY67678686;     uTtOZyHoBY67678686 = uTtOZyHoBY89908536;     uTtOZyHoBY89908536 = uTtOZyHoBY60391721;     uTtOZyHoBY60391721 = uTtOZyHoBY18696703;     uTtOZyHoBY18696703 = uTtOZyHoBY91819522;     uTtOZyHoBY91819522 = uTtOZyHoBY17607098;     uTtOZyHoBY17607098 = uTtOZyHoBY37612585;     uTtOZyHoBY37612585 = uTtOZyHoBY52492431;     uTtOZyHoBY52492431 = uTtOZyHoBY31475876;     uTtOZyHoBY31475876 = uTtOZyHoBY37158209;     uTtOZyHoBY37158209 = uTtOZyHoBY68999683;     uTtOZyHoBY68999683 = uTtOZyHoBY7469056;     uTtOZyHoBY7469056 = uTtOZyHoBY51778333;     uTtOZyHoBY51778333 = uTtOZyHoBY33943498;     uTtOZyHoBY33943498 = uTtOZyHoBY74810372;     uTtOZyHoBY74810372 = uTtOZyHoBY39828626;     uTtOZyHoBY39828626 = uTtOZyHoBY52070931;     uTtOZyHoBY52070931 = uTtOZyHoBY85140940;     uTtOZyHoBY85140940 = uTtOZyHoBY66801058;     uTtOZyHoBY66801058 = uTtOZyHoBY87362725;     uTtOZyHoBY87362725 = uTtOZyHoBY29029533;     uTtOZyHoBY29029533 = uTtOZyHoBY47132571;     uTtOZyHoBY47132571 = uTtOZyHoBY58488686;     uTtOZyHoBY58488686 = uTtOZyHoBY56210078;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CcquLFgGHW25395184() {     int IbEHOYygWs68996246 = -402893216;    int IbEHOYygWs95213735 = -472904552;    int IbEHOYygWs34878593 = 29468979;    int IbEHOYygWs43913560 = -222741833;    int IbEHOYygWs46484496 = -323178610;    int IbEHOYygWs38987976 = -942602905;    int IbEHOYygWs36149185 = -268501825;    int IbEHOYygWs1954985 = -273367471;    int IbEHOYygWs77256059 = -302828174;    int IbEHOYygWs91975428 = -164150715;    int IbEHOYygWs1237322 = -403086883;    int IbEHOYygWs18695302 = -504526517;    int IbEHOYygWs51797767 = -58085456;    int IbEHOYygWs22502364 = -102449614;    int IbEHOYygWs79551359 = -370990332;    int IbEHOYygWs17126284 = -65134625;    int IbEHOYygWs70019555 = 13722088;    int IbEHOYygWs6438749 = -41838595;    int IbEHOYygWs41374897 = 77184927;    int IbEHOYygWs69680264 = -462803137;    int IbEHOYygWs64378403 = -778733400;    int IbEHOYygWs10416340 = -423885704;    int IbEHOYygWs88360247 = -975208628;    int IbEHOYygWs94842821 = -321064332;    int IbEHOYygWs69660220 = -885777862;    int IbEHOYygWs66868470 = -995237134;    int IbEHOYygWs80205122 = -947265454;    int IbEHOYygWs3096659 = -609334904;    int IbEHOYygWs7570186 = -740732512;    int IbEHOYygWs92414103 = -33400630;    int IbEHOYygWs95404024 = -999259033;    int IbEHOYygWs75130293 = -86386533;    int IbEHOYygWs95975367 = -649847509;    int IbEHOYygWs55280393 = -559135862;    int IbEHOYygWs9680423 = -577870428;    int IbEHOYygWs88010508 = -180328835;    int IbEHOYygWs78364533 = -117722966;    int IbEHOYygWs69739423 = -227402458;    int IbEHOYygWs25269883 = -496750000;    int IbEHOYygWs38293990 = -101320510;    int IbEHOYygWs7430061 = -106409605;    int IbEHOYygWs79347921 = -560721820;    int IbEHOYygWs6101263 = -923366846;    int IbEHOYygWs26549803 = -798949549;    int IbEHOYygWs40435646 = 65192569;    int IbEHOYygWs77002369 = -602746261;    int IbEHOYygWs44435142 = -420298157;    int IbEHOYygWs6410031 = -847203477;    int IbEHOYygWs62200684 = -26817238;    int IbEHOYygWs290968 = 39757121;    int IbEHOYygWs5498051 = -95569221;    int IbEHOYygWs47668700 = 76432930;    int IbEHOYygWs69138410 = -743332549;    int IbEHOYygWs25629461 = -107434468;    int IbEHOYygWs61016781 = 36348194;    int IbEHOYygWs58579906 = -979007513;    int IbEHOYygWs6853489 = -497695924;    int IbEHOYygWs40035771 = -649466690;    int IbEHOYygWs74253340 = -336963972;    int IbEHOYygWs79616025 = -327941477;    int IbEHOYygWs58782853 = -995337451;    int IbEHOYygWs33052527 = -659166922;    int IbEHOYygWs94384799 = -532634960;    int IbEHOYygWs84841955 = -169427545;    int IbEHOYygWs96571403 = -164891682;    int IbEHOYygWs26107028 = -216700351;    int IbEHOYygWs22719935 = -854679009;    int IbEHOYygWs96517373 = -498949594;    int IbEHOYygWs12821942 = -524579186;    int IbEHOYygWs91540851 = -90661498;    int IbEHOYygWs38761751 = -947411660;    int IbEHOYygWs280133 = -758875454;    int IbEHOYygWs81168866 = -545088595;    int IbEHOYygWs3080908 = -821494563;    int IbEHOYygWs62250203 = -256393533;    int IbEHOYygWs85030482 = -118011581;    int IbEHOYygWs4315077 = -500518859;    int IbEHOYygWs61810444 = -76259079;    int IbEHOYygWs54407176 = -286256901;    int IbEHOYygWs92657850 = -183031601;    int IbEHOYygWs22433329 = -474938977;    int IbEHOYygWs73795092 = -61978;    int IbEHOYygWs40895974 = -482517666;    int IbEHOYygWs7279218 = -680489633;    int IbEHOYygWs86916053 = -937831410;    int IbEHOYygWs47735325 = -975691964;    int IbEHOYygWs5991883 = -343053985;    int IbEHOYygWs70345907 = -442413041;    int IbEHOYygWs94263612 = -495484057;    int IbEHOYygWs51100517 = -598862916;    int IbEHOYygWs81157020 = -682632911;    int IbEHOYygWs38328763 = -468256277;    int IbEHOYygWs95486082 = -890438486;    int IbEHOYygWs45653857 = -68808524;    int IbEHOYygWs79511136 = -105983060;    int IbEHOYygWs74377533 = -447242684;    int IbEHOYygWs84963121 = 71913139;    int IbEHOYygWs21259308 = -653939301;    int IbEHOYygWs29978400 = -534057867;    int IbEHOYygWs14328618 = -402893216;     IbEHOYygWs68996246 = IbEHOYygWs95213735;     IbEHOYygWs95213735 = IbEHOYygWs34878593;     IbEHOYygWs34878593 = IbEHOYygWs43913560;     IbEHOYygWs43913560 = IbEHOYygWs46484496;     IbEHOYygWs46484496 = IbEHOYygWs38987976;     IbEHOYygWs38987976 = IbEHOYygWs36149185;     IbEHOYygWs36149185 = IbEHOYygWs1954985;     IbEHOYygWs1954985 = IbEHOYygWs77256059;     IbEHOYygWs77256059 = IbEHOYygWs91975428;     IbEHOYygWs91975428 = IbEHOYygWs1237322;     IbEHOYygWs1237322 = IbEHOYygWs18695302;     IbEHOYygWs18695302 = IbEHOYygWs51797767;     IbEHOYygWs51797767 = IbEHOYygWs22502364;     IbEHOYygWs22502364 = IbEHOYygWs79551359;     IbEHOYygWs79551359 = IbEHOYygWs17126284;     IbEHOYygWs17126284 = IbEHOYygWs70019555;     IbEHOYygWs70019555 = IbEHOYygWs6438749;     IbEHOYygWs6438749 = IbEHOYygWs41374897;     IbEHOYygWs41374897 = IbEHOYygWs69680264;     IbEHOYygWs69680264 = IbEHOYygWs64378403;     IbEHOYygWs64378403 = IbEHOYygWs10416340;     IbEHOYygWs10416340 = IbEHOYygWs88360247;     IbEHOYygWs88360247 = IbEHOYygWs94842821;     IbEHOYygWs94842821 = IbEHOYygWs69660220;     IbEHOYygWs69660220 = IbEHOYygWs66868470;     IbEHOYygWs66868470 = IbEHOYygWs80205122;     IbEHOYygWs80205122 = IbEHOYygWs3096659;     IbEHOYygWs3096659 = IbEHOYygWs7570186;     IbEHOYygWs7570186 = IbEHOYygWs92414103;     IbEHOYygWs92414103 = IbEHOYygWs95404024;     IbEHOYygWs95404024 = IbEHOYygWs75130293;     IbEHOYygWs75130293 = IbEHOYygWs95975367;     IbEHOYygWs95975367 = IbEHOYygWs55280393;     IbEHOYygWs55280393 = IbEHOYygWs9680423;     IbEHOYygWs9680423 = IbEHOYygWs88010508;     IbEHOYygWs88010508 = IbEHOYygWs78364533;     IbEHOYygWs78364533 = IbEHOYygWs69739423;     IbEHOYygWs69739423 = IbEHOYygWs25269883;     IbEHOYygWs25269883 = IbEHOYygWs38293990;     IbEHOYygWs38293990 = IbEHOYygWs7430061;     IbEHOYygWs7430061 = IbEHOYygWs79347921;     IbEHOYygWs79347921 = IbEHOYygWs6101263;     IbEHOYygWs6101263 = IbEHOYygWs26549803;     IbEHOYygWs26549803 = IbEHOYygWs40435646;     IbEHOYygWs40435646 = IbEHOYygWs77002369;     IbEHOYygWs77002369 = IbEHOYygWs44435142;     IbEHOYygWs44435142 = IbEHOYygWs6410031;     IbEHOYygWs6410031 = IbEHOYygWs62200684;     IbEHOYygWs62200684 = IbEHOYygWs290968;     IbEHOYygWs290968 = IbEHOYygWs5498051;     IbEHOYygWs5498051 = IbEHOYygWs47668700;     IbEHOYygWs47668700 = IbEHOYygWs69138410;     IbEHOYygWs69138410 = IbEHOYygWs25629461;     IbEHOYygWs25629461 = IbEHOYygWs61016781;     IbEHOYygWs61016781 = IbEHOYygWs58579906;     IbEHOYygWs58579906 = IbEHOYygWs6853489;     IbEHOYygWs6853489 = IbEHOYygWs40035771;     IbEHOYygWs40035771 = IbEHOYygWs74253340;     IbEHOYygWs74253340 = IbEHOYygWs79616025;     IbEHOYygWs79616025 = IbEHOYygWs58782853;     IbEHOYygWs58782853 = IbEHOYygWs33052527;     IbEHOYygWs33052527 = IbEHOYygWs94384799;     IbEHOYygWs94384799 = IbEHOYygWs84841955;     IbEHOYygWs84841955 = IbEHOYygWs96571403;     IbEHOYygWs96571403 = IbEHOYygWs26107028;     IbEHOYygWs26107028 = IbEHOYygWs22719935;     IbEHOYygWs22719935 = IbEHOYygWs96517373;     IbEHOYygWs96517373 = IbEHOYygWs12821942;     IbEHOYygWs12821942 = IbEHOYygWs91540851;     IbEHOYygWs91540851 = IbEHOYygWs38761751;     IbEHOYygWs38761751 = IbEHOYygWs280133;     IbEHOYygWs280133 = IbEHOYygWs81168866;     IbEHOYygWs81168866 = IbEHOYygWs3080908;     IbEHOYygWs3080908 = IbEHOYygWs62250203;     IbEHOYygWs62250203 = IbEHOYygWs85030482;     IbEHOYygWs85030482 = IbEHOYygWs4315077;     IbEHOYygWs4315077 = IbEHOYygWs61810444;     IbEHOYygWs61810444 = IbEHOYygWs54407176;     IbEHOYygWs54407176 = IbEHOYygWs92657850;     IbEHOYygWs92657850 = IbEHOYygWs22433329;     IbEHOYygWs22433329 = IbEHOYygWs73795092;     IbEHOYygWs73795092 = IbEHOYygWs40895974;     IbEHOYygWs40895974 = IbEHOYygWs7279218;     IbEHOYygWs7279218 = IbEHOYygWs86916053;     IbEHOYygWs86916053 = IbEHOYygWs47735325;     IbEHOYygWs47735325 = IbEHOYygWs5991883;     IbEHOYygWs5991883 = IbEHOYygWs70345907;     IbEHOYygWs70345907 = IbEHOYygWs94263612;     IbEHOYygWs94263612 = IbEHOYygWs51100517;     IbEHOYygWs51100517 = IbEHOYygWs81157020;     IbEHOYygWs81157020 = IbEHOYygWs38328763;     IbEHOYygWs38328763 = IbEHOYygWs95486082;     IbEHOYygWs95486082 = IbEHOYygWs45653857;     IbEHOYygWs45653857 = IbEHOYygWs79511136;     IbEHOYygWs79511136 = IbEHOYygWs74377533;     IbEHOYygWs74377533 = IbEHOYygWs84963121;     IbEHOYygWs84963121 = IbEHOYygWs21259308;     IbEHOYygWs21259308 = IbEHOYygWs29978400;     IbEHOYygWs29978400 = IbEHOYygWs14328618;     IbEHOYygWs14328618 = IbEHOYygWs68996246;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mQjhaKPCXi25837287() {     int XvKdzWWdPK4176002 = -299813730;    int XvKdzWWdPK27759808 = -549186495;    int XvKdzWWdPK80087459 = -661415634;    int XvKdzWWdPK81781213 = -232810812;    int XvKdzWWdPK29329753 = -937696755;    int XvKdzWWdPK62375731 = -880701784;    int XvKdzWWdPK80965346 = -931624735;    int XvKdzWWdPK35093906 = -384305055;    int XvKdzWWdPK87083836 = -435136236;    int XvKdzWWdPK93318203 = -74172810;    int XvKdzWWdPK45945958 = -554925228;    int XvKdzWWdPK92257079 = -427126264;    int XvKdzWWdPK57096336 = -558926958;    int XvKdzWWdPK60896975 = -617420518;    int XvKdzWWdPK94421080 = 57258976;    int XvKdzWWdPK15242718 = -664897906;    int XvKdzWWdPK19741966 = -107717628;    int XvKdzWWdPK72973117 = -751719966;    int XvKdzWWdPK63388889 = -449404127;    int XvKdzWWdPK78777281 = -702657279;    int XvKdzWWdPK65766143 = -451070599;    int XvKdzWWdPK24983112 = -588518482;    int XvKdzWWdPK93535469 = -623335273;    int XvKdzWWdPK22199630 = -264481679;    int XvKdzWWdPK82652036 = -291085058;    int XvKdzWWdPK19916033 = -121356628;    int XvKdzWWdPK68488497 = 62241485;    int XvKdzWWdPK40957508 = 37379526;    int XvKdzWWdPK18401352 = -653136557;    int XvKdzWWdPK78232528 = 91848722;    int XvKdzWWdPK50718147 = -969898701;    int XvKdzWWdPK51266450 = -325355805;    int XvKdzWWdPK37607564 = -853101572;    int XvKdzWWdPK91462616 = -911335198;    int XvKdzWWdPK86612281 = -992344806;    int XvKdzWWdPK82099303 = -204521570;    int XvKdzWWdPK96626938 = -640926890;    int XvKdzWWdPK11151868 = -19385342;    int XvKdzWWdPK44039804 = -26965193;    int XvKdzWWdPK12871541 = -472529699;    int XvKdzWWdPK9763072 = -897117366;    int XvKdzWWdPK89861107 = -23149763;    int XvKdzWWdPK86251564 = -275200620;    int XvKdzWWdPK11390562 = -480520998;    int XvKdzWWdPK4409351 = -263461467;    int XvKdzWWdPK18302813 = 53750874;    int XvKdzWWdPK6616767 = 85057160;    int XvKdzWWdPK5065091 = -150329891;    int XvKdzWWdPK82066249 = -576372050;    int XvKdzWWdPK24279711 = -26274715;    int XvKdzWWdPK19435391 = -24974799;    int XvKdzWWdPK35149247 = -443425941;    int XvKdzWWdPK6489616 = -699193053;    int XvKdzWWdPK20365460 = -540647168;    int XvKdzWWdPK60071144 = -341901130;    int XvKdzWWdPK79192890 = -711295248;    int XvKdzWWdPK34224338 = -925851223;    int XvKdzWWdPK57887829 = -296933955;    int XvKdzWWdPK99129177 = -941725754;    int XvKdzWWdPK9413720 = -716340127;    int XvKdzWWdPK93887233 = -842943270;    int XvKdzWWdPK40007838 = -869004262;    int XvKdzWWdPK16692554 = -731168499;    int XvKdzWWdPK8851309 = -426984959;    int XvKdzWWdPK42600057 = -104274109;    int XvKdzWWdPK94679507 = -129569423;    int XvKdzWWdPK54649515 = -574024692;    int XvKdzWWdPK65633720 = -647591761;    int XvKdzWWdPK74284693 = -625075712;    int XvKdzWWdPK12321778 = -738219455;    int XvKdzWWdPK18615780 = 76028983;    int XvKdzWWdPK8590098 = 11667713;    int XvKdzWWdPK28933314 = -624754774;    int XvKdzWWdPK50517348 = -976874428;    int XvKdzWWdPK69014209 = -805539913;    int XvKdzWWdPK75905035 = -327920837;    int XvKdzWWdPK38731547 = -213317862;    int XvKdzWWdPK82144907 = -42814275;    int XvKdzWWdPK17790280 = 98979788;    int XvKdzWWdPK64349224 = -244835933;    int XvKdzWWdPK13299267 = -106413789;    int XvKdzWWdPK63423407 = -787428624;    int XvKdzWWdPK58891259 = -386248424;    int XvKdzWWdPK94121641 = -526861842;    int XvKdzWWdPK58797138 = -883176479;    int XvKdzWWdPK15568900 = -426472760;    int XvKdzWWdPK44776835 = -626162752;    int XvKdzWWdPK17242104 = -212454405;    int XvKdzWWdPK31391472 = -469434069;    int XvKdzWWdPK7419392 = -181049559;    int XvKdzWWdPK47874965 = -278670347;    int XvKdzWWdPK38739109 = -243992935;    int XvKdzWWdPK12022691 = -77659588;    int XvKdzWWdPK34626084 = -310625067;    int XvKdzWWdPK18984308 = -629586429;    int XvKdzWWdPK69755234 = 71886895;    int XvKdzWWdPK73168554 = -291981265;    int XvKdzWWdPK77400256 = -848215662;    int XvKdzWWdPK68790505 = -276246889;    int XvKdzWWdPK9729843 = -299813730;     XvKdzWWdPK4176002 = XvKdzWWdPK27759808;     XvKdzWWdPK27759808 = XvKdzWWdPK80087459;     XvKdzWWdPK80087459 = XvKdzWWdPK81781213;     XvKdzWWdPK81781213 = XvKdzWWdPK29329753;     XvKdzWWdPK29329753 = XvKdzWWdPK62375731;     XvKdzWWdPK62375731 = XvKdzWWdPK80965346;     XvKdzWWdPK80965346 = XvKdzWWdPK35093906;     XvKdzWWdPK35093906 = XvKdzWWdPK87083836;     XvKdzWWdPK87083836 = XvKdzWWdPK93318203;     XvKdzWWdPK93318203 = XvKdzWWdPK45945958;     XvKdzWWdPK45945958 = XvKdzWWdPK92257079;     XvKdzWWdPK92257079 = XvKdzWWdPK57096336;     XvKdzWWdPK57096336 = XvKdzWWdPK60896975;     XvKdzWWdPK60896975 = XvKdzWWdPK94421080;     XvKdzWWdPK94421080 = XvKdzWWdPK15242718;     XvKdzWWdPK15242718 = XvKdzWWdPK19741966;     XvKdzWWdPK19741966 = XvKdzWWdPK72973117;     XvKdzWWdPK72973117 = XvKdzWWdPK63388889;     XvKdzWWdPK63388889 = XvKdzWWdPK78777281;     XvKdzWWdPK78777281 = XvKdzWWdPK65766143;     XvKdzWWdPK65766143 = XvKdzWWdPK24983112;     XvKdzWWdPK24983112 = XvKdzWWdPK93535469;     XvKdzWWdPK93535469 = XvKdzWWdPK22199630;     XvKdzWWdPK22199630 = XvKdzWWdPK82652036;     XvKdzWWdPK82652036 = XvKdzWWdPK19916033;     XvKdzWWdPK19916033 = XvKdzWWdPK68488497;     XvKdzWWdPK68488497 = XvKdzWWdPK40957508;     XvKdzWWdPK40957508 = XvKdzWWdPK18401352;     XvKdzWWdPK18401352 = XvKdzWWdPK78232528;     XvKdzWWdPK78232528 = XvKdzWWdPK50718147;     XvKdzWWdPK50718147 = XvKdzWWdPK51266450;     XvKdzWWdPK51266450 = XvKdzWWdPK37607564;     XvKdzWWdPK37607564 = XvKdzWWdPK91462616;     XvKdzWWdPK91462616 = XvKdzWWdPK86612281;     XvKdzWWdPK86612281 = XvKdzWWdPK82099303;     XvKdzWWdPK82099303 = XvKdzWWdPK96626938;     XvKdzWWdPK96626938 = XvKdzWWdPK11151868;     XvKdzWWdPK11151868 = XvKdzWWdPK44039804;     XvKdzWWdPK44039804 = XvKdzWWdPK12871541;     XvKdzWWdPK12871541 = XvKdzWWdPK9763072;     XvKdzWWdPK9763072 = XvKdzWWdPK89861107;     XvKdzWWdPK89861107 = XvKdzWWdPK86251564;     XvKdzWWdPK86251564 = XvKdzWWdPK11390562;     XvKdzWWdPK11390562 = XvKdzWWdPK4409351;     XvKdzWWdPK4409351 = XvKdzWWdPK18302813;     XvKdzWWdPK18302813 = XvKdzWWdPK6616767;     XvKdzWWdPK6616767 = XvKdzWWdPK5065091;     XvKdzWWdPK5065091 = XvKdzWWdPK82066249;     XvKdzWWdPK82066249 = XvKdzWWdPK24279711;     XvKdzWWdPK24279711 = XvKdzWWdPK19435391;     XvKdzWWdPK19435391 = XvKdzWWdPK35149247;     XvKdzWWdPK35149247 = XvKdzWWdPK6489616;     XvKdzWWdPK6489616 = XvKdzWWdPK20365460;     XvKdzWWdPK20365460 = XvKdzWWdPK60071144;     XvKdzWWdPK60071144 = XvKdzWWdPK79192890;     XvKdzWWdPK79192890 = XvKdzWWdPK34224338;     XvKdzWWdPK34224338 = XvKdzWWdPK57887829;     XvKdzWWdPK57887829 = XvKdzWWdPK99129177;     XvKdzWWdPK99129177 = XvKdzWWdPK9413720;     XvKdzWWdPK9413720 = XvKdzWWdPK93887233;     XvKdzWWdPK93887233 = XvKdzWWdPK40007838;     XvKdzWWdPK40007838 = XvKdzWWdPK16692554;     XvKdzWWdPK16692554 = XvKdzWWdPK8851309;     XvKdzWWdPK8851309 = XvKdzWWdPK42600057;     XvKdzWWdPK42600057 = XvKdzWWdPK94679507;     XvKdzWWdPK94679507 = XvKdzWWdPK54649515;     XvKdzWWdPK54649515 = XvKdzWWdPK65633720;     XvKdzWWdPK65633720 = XvKdzWWdPK74284693;     XvKdzWWdPK74284693 = XvKdzWWdPK12321778;     XvKdzWWdPK12321778 = XvKdzWWdPK18615780;     XvKdzWWdPK18615780 = XvKdzWWdPK8590098;     XvKdzWWdPK8590098 = XvKdzWWdPK28933314;     XvKdzWWdPK28933314 = XvKdzWWdPK50517348;     XvKdzWWdPK50517348 = XvKdzWWdPK69014209;     XvKdzWWdPK69014209 = XvKdzWWdPK75905035;     XvKdzWWdPK75905035 = XvKdzWWdPK38731547;     XvKdzWWdPK38731547 = XvKdzWWdPK82144907;     XvKdzWWdPK82144907 = XvKdzWWdPK17790280;     XvKdzWWdPK17790280 = XvKdzWWdPK64349224;     XvKdzWWdPK64349224 = XvKdzWWdPK13299267;     XvKdzWWdPK13299267 = XvKdzWWdPK63423407;     XvKdzWWdPK63423407 = XvKdzWWdPK58891259;     XvKdzWWdPK58891259 = XvKdzWWdPK94121641;     XvKdzWWdPK94121641 = XvKdzWWdPK58797138;     XvKdzWWdPK58797138 = XvKdzWWdPK15568900;     XvKdzWWdPK15568900 = XvKdzWWdPK44776835;     XvKdzWWdPK44776835 = XvKdzWWdPK17242104;     XvKdzWWdPK17242104 = XvKdzWWdPK31391472;     XvKdzWWdPK31391472 = XvKdzWWdPK7419392;     XvKdzWWdPK7419392 = XvKdzWWdPK47874965;     XvKdzWWdPK47874965 = XvKdzWWdPK38739109;     XvKdzWWdPK38739109 = XvKdzWWdPK12022691;     XvKdzWWdPK12022691 = XvKdzWWdPK34626084;     XvKdzWWdPK34626084 = XvKdzWWdPK18984308;     XvKdzWWdPK18984308 = XvKdzWWdPK69755234;     XvKdzWWdPK69755234 = XvKdzWWdPK73168554;     XvKdzWWdPK73168554 = XvKdzWWdPK77400256;     XvKdzWWdPK77400256 = XvKdzWWdPK68790505;     XvKdzWWdPK68790505 = XvKdzWWdPK9729843;     XvKdzWWdPK9729843 = XvKdzWWdPK4176002;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yjfjsOyPOM44050629() {     int IdvsHcncBF87620307 = -80479411;    int IdvsHcncBF44631896 = -548568683;    int IdvsHcncBF76495966 = -454940861;    int IdvsHcncBF44573465 = -426610647;    int IdvsHcncBF35451076 = -744564246;    int IdvsHcncBF60880132 = -659773438;    int IdvsHcncBF52004664 = -671536048;    int IdvsHcncBF49441348 = -371463085;    int IdvsHcncBF45613537 = -776595587;    int IdvsHcncBF25863856 = -542548922;    int IdvsHcncBF95361895 = -745041100;    int IdvsHcncBF55462478 = -414744510;    int IdvsHcncBF75214112 = -350588199;    int IdvsHcncBF40523867 = -405710874;    int IdvsHcncBF55275544 = -36764007;    int IdvsHcncBF1631880 = -452977231;    int IdvsHcncBF13493851 = -961062655;    int IdvsHcncBF53824564 = -999024842;    int IdvsHcncBF25829031 = -448078997;    int IdvsHcncBF68144723 = -132243271;    int IdvsHcncBF21406531 = -40599630;    int IdvsHcncBF37130430 = -579670252;    int IdvsHcncBF97361176 = -663224470;    int IdvsHcncBF56906188 = -336307075;    int IdvsHcncBF24227995 = -850737817;    int IdvsHcncBF15258182 = -1866344;    int IdvsHcncBF17943238 = -665842857;    int IdvsHcncBF80689133 = -32575358;    int IdvsHcncBF54482446 = -515641699;    int IdvsHcncBF94065869 = -421001129;    int IdvsHcncBF68813461 = -607655902;    int IdvsHcncBF14372878 = -240550885;    int IdvsHcncBF81146451 = -473973749;    int IdvsHcncBF33621127 = -502937368;    int IdvsHcncBF65339012 = -90009541;    int IdvsHcncBF94488487 = 7454525;    int IdvsHcncBF65241117 = -519776070;    int IdvsHcncBF51730699 = -827855862;    int IdvsHcncBF93034881 = -211862991;    int IdvsHcncBF62366676 = -225123222;    int IdvsHcncBF43411996 = -468666339;    int IdvsHcncBF40348894 = -597058085;    int IdvsHcncBF9818635 = -826257863;    int IdvsHcncBF3295502 = -236703769;    int IdvsHcncBF96353847 = 8381579;    int IdvsHcncBF57908808 = 34139951;    int IdvsHcncBF82632196 = -47247070;    int IdvsHcncBF24735686 = -149051609;    int IdvsHcncBF12913362 = -61974132;    int IdvsHcncBF59612195 = -233295170;    int IdvsHcncBF92692838 = -245876221;    int IdvsHcncBF90650041 = -501685253;    int IdvsHcncBF20602075 = -910940370;    int IdvsHcncBF45042219 = -259060555;    int IdvsHcncBF22644099 = -690387253;    int IdvsHcncBF50489877 = -500809159;    int IdvsHcncBF47270720 = -885344213;    int IdvsHcncBF19589778 = -18633787;    int IdvsHcncBF20345471 = -575872831;    int IdvsHcncBF20192895 = -642697902;    int IdvsHcncBF42936895 = -993930581;    int IdvsHcncBF71315530 = -538960691;    int IdvsHcncBF94958902 = -855821386;    int IdvsHcncBF51547668 = -255594458;    int IdvsHcncBF57050395 = -934893021;    int IdvsHcncBF80989018 = -404490216;    int IdvsHcncBF74316027 = -940770761;    int IdvsHcncBF41592986 = -847650832;    int IdvsHcncBF75184855 = -215701333;    int IdvsHcncBF60787057 = 55781468;    int IdvsHcncBF36390763 = -933201162;    int IdvsHcncBF61763151 = -33206794;    int IdvsHcncBF60789682 = -687161852;    int IdvsHcncBF63462355 = -122955776;    int IdvsHcncBF24732727 = -663576933;    int IdvsHcncBF81057636 = -443541546;    int IdvsHcncBF27311796 = -753412390;    int IdvsHcncBF94065675 = -326520702;    int IdvsHcncBF60552341 = -244688655;    int IdvsHcncBF66319186 = -784877768;    int IdvsHcncBF32625986 = -954619275;    int IdvsHcncBF93207551 = -416791248;    int IdvsHcncBF67775772 = -970601226;    int IdvsHcncBF94870251 = -182346529;    int IdvsHcncBF1373032 = -75124908;    int IdvsHcncBF78163419 = -5970649;    int IdvsHcncBF93770802 = -329610515;    int IdvsHcncBF36104232 = -114913195;    int IdvsHcncBF10977029 = -812550116;    int IdvsHcncBF14849135 = -589200383;    int IdvsHcncBF47217767 = -107201263;    int IdvsHcncBF45651339 = -401142283;    int IdvsHcncBF31385229 = -151983031;    int IdvsHcncBF72841987 = -569165089;    int IdvsHcncBF19429782 = -231192641;    int IdvsHcncBF72096466 = -929705649;    int IdvsHcncBF45389991 = -741236699;    int IdvsHcncBF58270967 = -470663405;    int IdvsHcncBF46245107 = -301810748;    int IdvsHcncBF15364829 = -80479411;     IdvsHcncBF87620307 = IdvsHcncBF44631896;     IdvsHcncBF44631896 = IdvsHcncBF76495966;     IdvsHcncBF76495966 = IdvsHcncBF44573465;     IdvsHcncBF44573465 = IdvsHcncBF35451076;     IdvsHcncBF35451076 = IdvsHcncBF60880132;     IdvsHcncBF60880132 = IdvsHcncBF52004664;     IdvsHcncBF52004664 = IdvsHcncBF49441348;     IdvsHcncBF49441348 = IdvsHcncBF45613537;     IdvsHcncBF45613537 = IdvsHcncBF25863856;     IdvsHcncBF25863856 = IdvsHcncBF95361895;     IdvsHcncBF95361895 = IdvsHcncBF55462478;     IdvsHcncBF55462478 = IdvsHcncBF75214112;     IdvsHcncBF75214112 = IdvsHcncBF40523867;     IdvsHcncBF40523867 = IdvsHcncBF55275544;     IdvsHcncBF55275544 = IdvsHcncBF1631880;     IdvsHcncBF1631880 = IdvsHcncBF13493851;     IdvsHcncBF13493851 = IdvsHcncBF53824564;     IdvsHcncBF53824564 = IdvsHcncBF25829031;     IdvsHcncBF25829031 = IdvsHcncBF68144723;     IdvsHcncBF68144723 = IdvsHcncBF21406531;     IdvsHcncBF21406531 = IdvsHcncBF37130430;     IdvsHcncBF37130430 = IdvsHcncBF97361176;     IdvsHcncBF97361176 = IdvsHcncBF56906188;     IdvsHcncBF56906188 = IdvsHcncBF24227995;     IdvsHcncBF24227995 = IdvsHcncBF15258182;     IdvsHcncBF15258182 = IdvsHcncBF17943238;     IdvsHcncBF17943238 = IdvsHcncBF80689133;     IdvsHcncBF80689133 = IdvsHcncBF54482446;     IdvsHcncBF54482446 = IdvsHcncBF94065869;     IdvsHcncBF94065869 = IdvsHcncBF68813461;     IdvsHcncBF68813461 = IdvsHcncBF14372878;     IdvsHcncBF14372878 = IdvsHcncBF81146451;     IdvsHcncBF81146451 = IdvsHcncBF33621127;     IdvsHcncBF33621127 = IdvsHcncBF65339012;     IdvsHcncBF65339012 = IdvsHcncBF94488487;     IdvsHcncBF94488487 = IdvsHcncBF65241117;     IdvsHcncBF65241117 = IdvsHcncBF51730699;     IdvsHcncBF51730699 = IdvsHcncBF93034881;     IdvsHcncBF93034881 = IdvsHcncBF62366676;     IdvsHcncBF62366676 = IdvsHcncBF43411996;     IdvsHcncBF43411996 = IdvsHcncBF40348894;     IdvsHcncBF40348894 = IdvsHcncBF9818635;     IdvsHcncBF9818635 = IdvsHcncBF3295502;     IdvsHcncBF3295502 = IdvsHcncBF96353847;     IdvsHcncBF96353847 = IdvsHcncBF57908808;     IdvsHcncBF57908808 = IdvsHcncBF82632196;     IdvsHcncBF82632196 = IdvsHcncBF24735686;     IdvsHcncBF24735686 = IdvsHcncBF12913362;     IdvsHcncBF12913362 = IdvsHcncBF59612195;     IdvsHcncBF59612195 = IdvsHcncBF92692838;     IdvsHcncBF92692838 = IdvsHcncBF90650041;     IdvsHcncBF90650041 = IdvsHcncBF20602075;     IdvsHcncBF20602075 = IdvsHcncBF45042219;     IdvsHcncBF45042219 = IdvsHcncBF22644099;     IdvsHcncBF22644099 = IdvsHcncBF50489877;     IdvsHcncBF50489877 = IdvsHcncBF47270720;     IdvsHcncBF47270720 = IdvsHcncBF19589778;     IdvsHcncBF19589778 = IdvsHcncBF20345471;     IdvsHcncBF20345471 = IdvsHcncBF20192895;     IdvsHcncBF20192895 = IdvsHcncBF42936895;     IdvsHcncBF42936895 = IdvsHcncBF71315530;     IdvsHcncBF71315530 = IdvsHcncBF94958902;     IdvsHcncBF94958902 = IdvsHcncBF51547668;     IdvsHcncBF51547668 = IdvsHcncBF57050395;     IdvsHcncBF57050395 = IdvsHcncBF80989018;     IdvsHcncBF80989018 = IdvsHcncBF74316027;     IdvsHcncBF74316027 = IdvsHcncBF41592986;     IdvsHcncBF41592986 = IdvsHcncBF75184855;     IdvsHcncBF75184855 = IdvsHcncBF60787057;     IdvsHcncBF60787057 = IdvsHcncBF36390763;     IdvsHcncBF36390763 = IdvsHcncBF61763151;     IdvsHcncBF61763151 = IdvsHcncBF60789682;     IdvsHcncBF60789682 = IdvsHcncBF63462355;     IdvsHcncBF63462355 = IdvsHcncBF24732727;     IdvsHcncBF24732727 = IdvsHcncBF81057636;     IdvsHcncBF81057636 = IdvsHcncBF27311796;     IdvsHcncBF27311796 = IdvsHcncBF94065675;     IdvsHcncBF94065675 = IdvsHcncBF60552341;     IdvsHcncBF60552341 = IdvsHcncBF66319186;     IdvsHcncBF66319186 = IdvsHcncBF32625986;     IdvsHcncBF32625986 = IdvsHcncBF93207551;     IdvsHcncBF93207551 = IdvsHcncBF67775772;     IdvsHcncBF67775772 = IdvsHcncBF94870251;     IdvsHcncBF94870251 = IdvsHcncBF1373032;     IdvsHcncBF1373032 = IdvsHcncBF78163419;     IdvsHcncBF78163419 = IdvsHcncBF93770802;     IdvsHcncBF93770802 = IdvsHcncBF36104232;     IdvsHcncBF36104232 = IdvsHcncBF10977029;     IdvsHcncBF10977029 = IdvsHcncBF14849135;     IdvsHcncBF14849135 = IdvsHcncBF47217767;     IdvsHcncBF47217767 = IdvsHcncBF45651339;     IdvsHcncBF45651339 = IdvsHcncBF31385229;     IdvsHcncBF31385229 = IdvsHcncBF72841987;     IdvsHcncBF72841987 = IdvsHcncBF19429782;     IdvsHcncBF19429782 = IdvsHcncBF72096466;     IdvsHcncBF72096466 = IdvsHcncBF45389991;     IdvsHcncBF45389991 = IdvsHcncBF58270967;     IdvsHcncBF58270967 = IdvsHcncBF46245107;     IdvsHcncBF46245107 = IdvsHcncBF15364829;     IdvsHcncBF15364829 = IdvsHcncBF87620307;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QfqUxnaRQT14506502() {     int vbkxCaQOaL406475 = -215520102;    int vbkxCaQOaL31402785 = -718902696;    int vbkxCaQOaL94248765 = -863300243;    int vbkxCaQOaL52514290 = -250812924;    int vbkxCaQOaL28962181 = -136380711;    int vbkxCaQOaL4190203 = -436696751;    int vbkxCaQOaL73212422 = -117208119;    int vbkxCaQOaL76160461 = -815981341;    int vbkxCaQOaL41018348 = -738353679;    int vbkxCaQOaL32082561 = -346636555;    int vbkxCaQOaL22849278 = -859727116;    int vbkxCaQOaL87412983 = -922077326;    int vbkxCaQOaL45357415 = -254370856;    int vbkxCaQOaL90147945 = -738126073;    int vbkxCaQOaL78582097 = 56250163;    int vbkxCaQOaL30056948 = 96131379;    int vbkxCaQOaL69245668 = -624837122;    int vbkxCaQOaL37383049 = -554235145;    int vbkxCaQOaL26989661 = -357548193;    int vbkxCaQOaL70799219 = -731487410;    int vbkxCaQOaL1580588 = -531915894;    int vbkxCaQOaL66178249 = -749528600;    int vbkxCaQOaL36121472 = -594228365;    int vbkxCaQOaL1413318 = -696652087;    int vbkxCaQOaL30122254 = 38820259;    int vbkxCaQOaL96576827 = -792297543;    int vbkxCaQOaL29358774 = -199549137;    int vbkxCaQOaL48042059 = -206373462;    int vbkxCaQOaL71099499 = -729858939;    int vbkxCaQOaL46816985 = -50887284;    int vbkxCaQOaL13249457 = -884072652;    int vbkxCaQOaL17691701 = -219270563;    int vbkxCaQOaL27192400 = -349828534;    int vbkxCaQOaL10697500 = -407691586;    int vbkxCaQOaL48399544 = -700041422;    int vbkxCaQOaL89712602 = -881108580;    int vbkxCaQOaL2005178 = -309685420;    int vbkxCaQOaL9434723 = -747475954;    int vbkxCaQOaL98810267 = -53713568;    int vbkxCaQOaL88631406 = -69540067;    int vbkxCaQOaL4843305 = -844140333;    int vbkxCaQOaL48051349 = -495369419;    int vbkxCaQOaL44702104 = -849691308;    int vbkxCaQOaL47924039 = -577876011;    int vbkxCaQOaL76362337 = -984388381;    int vbkxCaQOaL31536938 = -39178489;    int vbkxCaQOaL60214216 = -544762122;    int vbkxCaQOaL54175652 = -304404389;    int vbkxCaQOaL56977411 = -225576107;    int vbkxCaQOaL6562616 = -844331633;    int vbkxCaQOaL11020332 = -498760530;    int vbkxCaQOaL18826589 = -972870591;    int vbkxCaQOaL55087224 = 13056348;    int vbkxCaQOaL89741944 = -948512297;    int vbkxCaQOaL12925915 = -684831739;    int vbkxCaQOaL34228225 = -465991503;    int vbkxCaQOaL95281312 = -24674332;    int vbkxCaQOaL92835447 = -66648156;    int vbkxCaQOaL22392037 = -189633184;    int vbkxCaQOaL32385354 = -344083168;    int vbkxCaQOaL74831429 = -137147614;    int vbkxCaQOaL25170364 = -910834657;    int vbkxCaQOaL5060962 = 13877597;    int vbkxCaQOaL94201362 = -587466396;    int vbkxCaQOaL18833104 = -462563904;    int vbkxCaQOaL5157577 = -540456553;    int vbkxCaQOaL60220583 = -472248793;    int vbkxCaQOaL34659916 = -846679271;    int vbkxCaQOaL41748402 = 61915349;    int vbkxCaQOaL88869495 = -62641258;    int vbkxCaQOaL28051771 = -594183201;    int vbkxCaQOaL59810946 = -877361169;    int vbkxCaQOaL38572781 = -400521577;    int vbkxCaQOaL38358255 = -188008126;    int vbkxCaQOaL65955915 = -887347078;    int vbkxCaQOaL53529238 = 63453524;    int vbkxCaQOaL21476145 = -899837293;    int vbkxCaQOaL88197432 = 83647646;    int vbkxCaQOaL25050980 = -712263707;    int vbkxCaQOaL98585316 = -922001252;    int vbkxCaQOaL36362612 = -147535422;    int vbkxCaQOaL75183121 = -895144749;    int vbkxCaQOaL91064647 = -980797356;    int vbkxCaQOaL64536884 = -885527306;    int vbkxCaQOaL35796653 = -552126755;    int vbkxCaQOaL94422868 = -911202061;    int vbkxCaQOaL62604476 = -132326911;    int vbkxCaQOaL37450456 = -401316237;    int vbkxCaQOaL97771585 = -722859848;    int vbkxCaQOaL14171319 = -134049920;    int vbkxCaQOaL94431289 = -756434248;    int vbkxCaQOaL9169730 = -143037264;    int vbkxCaQOaL87042685 = -457842771;    int vbkxCaQOaL66424914 = -709630400;    int vbkxCaQOaL13799977 = -932392454;    int vbkxCaQOaL79672941 = -933305676;    int vbkxCaQOaL42990388 = -409247016;    int vbkxCaQOaL50500741 = -162224913;    int vbkxCaQOaL29090936 = -15312108;    int vbkxCaQOaL71204760 = -215520102;     vbkxCaQOaL406475 = vbkxCaQOaL31402785;     vbkxCaQOaL31402785 = vbkxCaQOaL94248765;     vbkxCaQOaL94248765 = vbkxCaQOaL52514290;     vbkxCaQOaL52514290 = vbkxCaQOaL28962181;     vbkxCaQOaL28962181 = vbkxCaQOaL4190203;     vbkxCaQOaL4190203 = vbkxCaQOaL73212422;     vbkxCaQOaL73212422 = vbkxCaQOaL76160461;     vbkxCaQOaL76160461 = vbkxCaQOaL41018348;     vbkxCaQOaL41018348 = vbkxCaQOaL32082561;     vbkxCaQOaL32082561 = vbkxCaQOaL22849278;     vbkxCaQOaL22849278 = vbkxCaQOaL87412983;     vbkxCaQOaL87412983 = vbkxCaQOaL45357415;     vbkxCaQOaL45357415 = vbkxCaQOaL90147945;     vbkxCaQOaL90147945 = vbkxCaQOaL78582097;     vbkxCaQOaL78582097 = vbkxCaQOaL30056948;     vbkxCaQOaL30056948 = vbkxCaQOaL69245668;     vbkxCaQOaL69245668 = vbkxCaQOaL37383049;     vbkxCaQOaL37383049 = vbkxCaQOaL26989661;     vbkxCaQOaL26989661 = vbkxCaQOaL70799219;     vbkxCaQOaL70799219 = vbkxCaQOaL1580588;     vbkxCaQOaL1580588 = vbkxCaQOaL66178249;     vbkxCaQOaL66178249 = vbkxCaQOaL36121472;     vbkxCaQOaL36121472 = vbkxCaQOaL1413318;     vbkxCaQOaL1413318 = vbkxCaQOaL30122254;     vbkxCaQOaL30122254 = vbkxCaQOaL96576827;     vbkxCaQOaL96576827 = vbkxCaQOaL29358774;     vbkxCaQOaL29358774 = vbkxCaQOaL48042059;     vbkxCaQOaL48042059 = vbkxCaQOaL71099499;     vbkxCaQOaL71099499 = vbkxCaQOaL46816985;     vbkxCaQOaL46816985 = vbkxCaQOaL13249457;     vbkxCaQOaL13249457 = vbkxCaQOaL17691701;     vbkxCaQOaL17691701 = vbkxCaQOaL27192400;     vbkxCaQOaL27192400 = vbkxCaQOaL10697500;     vbkxCaQOaL10697500 = vbkxCaQOaL48399544;     vbkxCaQOaL48399544 = vbkxCaQOaL89712602;     vbkxCaQOaL89712602 = vbkxCaQOaL2005178;     vbkxCaQOaL2005178 = vbkxCaQOaL9434723;     vbkxCaQOaL9434723 = vbkxCaQOaL98810267;     vbkxCaQOaL98810267 = vbkxCaQOaL88631406;     vbkxCaQOaL88631406 = vbkxCaQOaL4843305;     vbkxCaQOaL4843305 = vbkxCaQOaL48051349;     vbkxCaQOaL48051349 = vbkxCaQOaL44702104;     vbkxCaQOaL44702104 = vbkxCaQOaL47924039;     vbkxCaQOaL47924039 = vbkxCaQOaL76362337;     vbkxCaQOaL76362337 = vbkxCaQOaL31536938;     vbkxCaQOaL31536938 = vbkxCaQOaL60214216;     vbkxCaQOaL60214216 = vbkxCaQOaL54175652;     vbkxCaQOaL54175652 = vbkxCaQOaL56977411;     vbkxCaQOaL56977411 = vbkxCaQOaL6562616;     vbkxCaQOaL6562616 = vbkxCaQOaL11020332;     vbkxCaQOaL11020332 = vbkxCaQOaL18826589;     vbkxCaQOaL18826589 = vbkxCaQOaL55087224;     vbkxCaQOaL55087224 = vbkxCaQOaL89741944;     vbkxCaQOaL89741944 = vbkxCaQOaL12925915;     vbkxCaQOaL12925915 = vbkxCaQOaL34228225;     vbkxCaQOaL34228225 = vbkxCaQOaL95281312;     vbkxCaQOaL95281312 = vbkxCaQOaL92835447;     vbkxCaQOaL92835447 = vbkxCaQOaL22392037;     vbkxCaQOaL22392037 = vbkxCaQOaL32385354;     vbkxCaQOaL32385354 = vbkxCaQOaL74831429;     vbkxCaQOaL74831429 = vbkxCaQOaL25170364;     vbkxCaQOaL25170364 = vbkxCaQOaL5060962;     vbkxCaQOaL5060962 = vbkxCaQOaL94201362;     vbkxCaQOaL94201362 = vbkxCaQOaL18833104;     vbkxCaQOaL18833104 = vbkxCaQOaL5157577;     vbkxCaQOaL5157577 = vbkxCaQOaL60220583;     vbkxCaQOaL60220583 = vbkxCaQOaL34659916;     vbkxCaQOaL34659916 = vbkxCaQOaL41748402;     vbkxCaQOaL41748402 = vbkxCaQOaL88869495;     vbkxCaQOaL88869495 = vbkxCaQOaL28051771;     vbkxCaQOaL28051771 = vbkxCaQOaL59810946;     vbkxCaQOaL59810946 = vbkxCaQOaL38572781;     vbkxCaQOaL38572781 = vbkxCaQOaL38358255;     vbkxCaQOaL38358255 = vbkxCaQOaL65955915;     vbkxCaQOaL65955915 = vbkxCaQOaL53529238;     vbkxCaQOaL53529238 = vbkxCaQOaL21476145;     vbkxCaQOaL21476145 = vbkxCaQOaL88197432;     vbkxCaQOaL88197432 = vbkxCaQOaL25050980;     vbkxCaQOaL25050980 = vbkxCaQOaL98585316;     vbkxCaQOaL98585316 = vbkxCaQOaL36362612;     vbkxCaQOaL36362612 = vbkxCaQOaL75183121;     vbkxCaQOaL75183121 = vbkxCaQOaL91064647;     vbkxCaQOaL91064647 = vbkxCaQOaL64536884;     vbkxCaQOaL64536884 = vbkxCaQOaL35796653;     vbkxCaQOaL35796653 = vbkxCaQOaL94422868;     vbkxCaQOaL94422868 = vbkxCaQOaL62604476;     vbkxCaQOaL62604476 = vbkxCaQOaL37450456;     vbkxCaQOaL37450456 = vbkxCaQOaL97771585;     vbkxCaQOaL97771585 = vbkxCaQOaL14171319;     vbkxCaQOaL14171319 = vbkxCaQOaL94431289;     vbkxCaQOaL94431289 = vbkxCaQOaL9169730;     vbkxCaQOaL9169730 = vbkxCaQOaL87042685;     vbkxCaQOaL87042685 = vbkxCaQOaL66424914;     vbkxCaQOaL66424914 = vbkxCaQOaL13799977;     vbkxCaQOaL13799977 = vbkxCaQOaL79672941;     vbkxCaQOaL79672941 = vbkxCaQOaL42990388;     vbkxCaQOaL42990388 = vbkxCaQOaL50500741;     vbkxCaQOaL50500741 = vbkxCaQOaL29090936;     vbkxCaQOaL29090936 = vbkxCaQOaL71204760;     vbkxCaQOaL71204760 = vbkxCaQOaL406475;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OHDBtCSfdW32719843() {     int IUJOMIyxTY83850779 = 3814217;    int IUJOMIyxTY48274874 = -718284884;    int IUJOMIyxTY90657272 = -656825471;    int IUJOMIyxTY15306542 = -444612760;    int IUJOMIyxTY35083505 = 56751798;    int IUJOMIyxTY2694604 = -215768404;    int IUJOMIyxTY44251740 = -957119432;    int IUJOMIyxTY90507903 = -803139371;    int IUJOMIyxTY99548048 = 20186970;    int IUJOMIyxTY64628213 = -815012667;    int IUJOMIyxTY72265215 = 50157012;    int IUJOMIyxTY50618382 = -909695572;    int IUJOMIyxTY63475191 = -46032097;    int IUJOMIyxTY69774837 = -526416430;    int IUJOMIyxTY39436561 = -37772820;    int IUJOMIyxTY16446110 = -791947946;    int IUJOMIyxTY62997553 = -378182149;    int IUJOMIyxTY18234496 = -801540021;    int IUJOMIyxTY89429803 = -356223063;    int IUJOMIyxTY60166662 = -161073403;    int IUJOMIyxTY57220974 = -121444925;    int IUJOMIyxTY78325567 = -740680371;    int IUJOMIyxTY39947180 = -634117562;    int IUJOMIyxTY36119876 = -768477483;    int IUJOMIyxTY71698212 = -520832500;    int IUJOMIyxTY91918976 = -672807259;    int IUJOMIyxTY78813514 = -927633480;    int IUJOMIyxTY87773684 = -276328346;    int IUJOMIyxTY7180594 = -592364081;    int IUJOMIyxTY62650326 = -563737135;    int IUJOMIyxTY31344771 = -521829853;    int IUJOMIyxTY80798128 = -134465643;    int IUJOMIyxTY70731287 = 29299289;    int IUJOMIyxTY52856010 = 706244;    int IUJOMIyxTY27126274 = -897706157;    int IUJOMIyxTY2101787 = -669132486;    int IUJOMIyxTY70619356 = -188534600;    int IUJOMIyxTY50013554 = -455946474;    int IUJOMIyxTY47805346 = -238611365;    int IUJOMIyxTY38126541 = -922133589;    int IUJOMIyxTY38492229 = -415689306;    int IUJOMIyxTY98539135 = 30722260;    int IUJOMIyxTY68269174 = -300748551;    int IUJOMIyxTY39828980 = -334058782;    int IUJOMIyxTY68306834 = -712545334;    int IUJOMIyxTY71142933 = -58789413;    int IUJOMIyxTY36229646 = -677066351;    int IUJOMIyxTY73846247 = -303126107;    int IUJOMIyxTY87824523 = -811178189;    int IUJOMIyxTY41895099 = 48647912;    int IUJOMIyxTY84277780 = -719661952;    int IUJOMIyxTY74327383 = 68870097;    int IUJOMIyxTY69199683 = -198690969;    int IUJOMIyxTY14418704 = -666925684;    int IUJOMIyxTY75498869 = 66682138;    int IUJOMIyxTY5525213 = -255505413;    int IUJOMIyxTY8327694 = 15832678;    int IUJOMIyxTY54537396 = -888347988;    int IUJOMIyxTY43608330 = -923780261;    int IUJOMIyxTY43164528 = -270440944;    int IUJOMIyxTY23881090 = -288134925;    int IUJOMIyxTY56478056 = -580791086;    int IUJOMIyxTY83327310 = -110775290;    int IUJOMIyxTY36897722 = -416075895;    int IUJOMIyxTY33283442 = -193182815;    int IUJOMIyxTY91467087 = -815377346;    int IUJOMIyxTY79887095 = -838994862;    int IUJOMIyxTY10619181 = 53261658;    int IUJOMIyxTY42648563 = -628710273;    int IUJOMIyxTY37334774 = -368640335;    int IUJOMIyxTY45826754 = -503413346;    int IUJOMIyxTY12983999 = -922235676;    int IUJOMIyxTY70429149 = -462928656;    int IUJOMIyxTY51303262 = -434089474;    int IUJOMIyxTY21674433 = -745384098;    int IUJOMIyxTY58681839 = -52167185;    int IUJOMIyxTY10056394 = -339931821;    int IUJOMIyxTY118201 = -200058780;    int IUJOMIyxTY67813041 = 44067850;    int IUJOMIyxTY555279 = -362043088;    int IUJOMIyxTY55689330 = -995740908;    int IUJOMIyxTY4967267 = -524507373;    int IUJOMIyxTY99949160 = -465150157;    int IUJOMIyxTY65285494 = -541011993;    int IUJOMIyxTY78372546 = -844075184;    int IUJOMIyxTY57017387 = -490699951;    int IUJOMIyxTY11598445 = -935774675;    int IUJOMIyxTY56312584 = -303775027;    int IUJOMIyxTY77357141 = 34024105;    int IUJOMIyxTY21601062 = -542200745;    int IUJOMIyxTY93774092 = -584965164;    int IUJOMIyxTY16081960 = -300186612;    int IUJOMIyxTY6405224 = -532166214;    int IUJOMIyxTY4640818 = -968170422;    int IUJOMIyxTY14245452 = -533998665;    int IUJOMIyxTY82014172 = -834898220;    int IUJOMIyxTY15211826 = -858502451;    int IUJOMIyxTY31371452 = -884672656;    int IUJOMIyxTY6545538 = -40875968;    int IUJOMIyxTY76839746 = 3814217;     IUJOMIyxTY83850779 = IUJOMIyxTY48274874;     IUJOMIyxTY48274874 = IUJOMIyxTY90657272;     IUJOMIyxTY90657272 = IUJOMIyxTY15306542;     IUJOMIyxTY15306542 = IUJOMIyxTY35083505;     IUJOMIyxTY35083505 = IUJOMIyxTY2694604;     IUJOMIyxTY2694604 = IUJOMIyxTY44251740;     IUJOMIyxTY44251740 = IUJOMIyxTY90507903;     IUJOMIyxTY90507903 = IUJOMIyxTY99548048;     IUJOMIyxTY99548048 = IUJOMIyxTY64628213;     IUJOMIyxTY64628213 = IUJOMIyxTY72265215;     IUJOMIyxTY72265215 = IUJOMIyxTY50618382;     IUJOMIyxTY50618382 = IUJOMIyxTY63475191;     IUJOMIyxTY63475191 = IUJOMIyxTY69774837;     IUJOMIyxTY69774837 = IUJOMIyxTY39436561;     IUJOMIyxTY39436561 = IUJOMIyxTY16446110;     IUJOMIyxTY16446110 = IUJOMIyxTY62997553;     IUJOMIyxTY62997553 = IUJOMIyxTY18234496;     IUJOMIyxTY18234496 = IUJOMIyxTY89429803;     IUJOMIyxTY89429803 = IUJOMIyxTY60166662;     IUJOMIyxTY60166662 = IUJOMIyxTY57220974;     IUJOMIyxTY57220974 = IUJOMIyxTY78325567;     IUJOMIyxTY78325567 = IUJOMIyxTY39947180;     IUJOMIyxTY39947180 = IUJOMIyxTY36119876;     IUJOMIyxTY36119876 = IUJOMIyxTY71698212;     IUJOMIyxTY71698212 = IUJOMIyxTY91918976;     IUJOMIyxTY91918976 = IUJOMIyxTY78813514;     IUJOMIyxTY78813514 = IUJOMIyxTY87773684;     IUJOMIyxTY87773684 = IUJOMIyxTY7180594;     IUJOMIyxTY7180594 = IUJOMIyxTY62650326;     IUJOMIyxTY62650326 = IUJOMIyxTY31344771;     IUJOMIyxTY31344771 = IUJOMIyxTY80798128;     IUJOMIyxTY80798128 = IUJOMIyxTY70731287;     IUJOMIyxTY70731287 = IUJOMIyxTY52856010;     IUJOMIyxTY52856010 = IUJOMIyxTY27126274;     IUJOMIyxTY27126274 = IUJOMIyxTY2101787;     IUJOMIyxTY2101787 = IUJOMIyxTY70619356;     IUJOMIyxTY70619356 = IUJOMIyxTY50013554;     IUJOMIyxTY50013554 = IUJOMIyxTY47805346;     IUJOMIyxTY47805346 = IUJOMIyxTY38126541;     IUJOMIyxTY38126541 = IUJOMIyxTY38492229;     IUJOMIyxTY38492229 = IUJOMIyxTY98539135;     IUJOMIyxTY98539135 = IUJOMIyxTY68269174;     IUJOMIyxTY68269174 = IUJOMIyxTY39828980;     IUJOMIyxTY39828980 = IUJOMIyxTY68306834;     IUJOMIyxTY68306834 = IUJOMIyxTY71142933;     IUJOMIyxTY71142933 = IUJOMIyxTY36229646;     IUJOMIyxTY36229646 = IUJOMIyxTY73846247;     IUJOMIyxTY73846247 = IUJOMIyxTY87824523;     IUJOMIyxTY87824523 = IUJOMIyxTY41895099;     IUJOMIyxTY41895099 = IUJOMIyxTY84277780;     IUJOMIyxTY84277780 = IUJOMIyxTY74327383;     IUJOMIyxTY74327383 = IUJOMIyxTY69199683;     IUJOMIyxTY69199683 = IUJOMIyxTY14418704;     IUJOMIyxTY14418704 = IUJOMIyxTY75498869;     IUJOMIyxTY75498869 = IUJOMIyxTY5525213;     IUJOMIyxTY5525213 = IUJOMIyxTY8327694;     IUJOMIyxTY8327694 = IUJOMIyxTY54537396;     IUJOMIyxTY54537396 = IUJOMIyxTY43608330;     IUJOMIyxTY43608330 = IUJOMIyxTY43164528;     IUJOMIyxTY43164528 = IUJOMIyxTY23881090;     IUJOMIyxTY23881090 = IUJOMIyxTY56478056;     IUJOMIyxTY56478056 = IUJOMIyxTY83327310;     IUJOMIyxTY83327310 = IUJOMIyxTY36897722;     IUJOMIyxTY36897722 = IUJOMIyxTY33283442;     IUJOMIyxTY33283442 = IUJOMIyxTY91467087;     IUJOMIyxTY91467087 = IUJOMIyxTY79887095;     IUJOMIyxTY79887095 = IUJOMIyxTY10619181;     IUJOMIyxTY10619181 = IUJOMIyxTY42648563;     IUJOMIyxTY42648563 = IUJOMIyxTY37334774;     IUJOMIyxTY37334774 = IUJOMIyxTY45826754;     IUJOMIyxTY45826754 = IUJOMIyxTY12983999;     IUJOMIyxTY12983999 = IUJOMIyxTY70429149;     IUJOMIyxTY70429149 = IUJOMIyxTY51303262;     IUJOMIyxTY51303262 = IUJOMIyxTY21674433;     IUJOMIyxTY21674433 = IUJOMIyxTY58681839;     IUJOMIyxTY58681839 = IUJOMIyxTY10056394;     IUJOMIyxTY10056394 = IUJOMIyxTY118201;     IUJOMIyxTY118201 = IUJOMIyxTY67813041;     IUJOMIyxTY67813041 = IUJOMIyxTY555279;     IUJOMIyxTY555279 = IUJOMIyxTY55689330;     IUJOMIyxTY55689330 = IUJOMIyxTY4967267;     IUJOMIyxTY4967267 = IUJOMIyxTY99949160;     IUJOMIyxTY99949160 = IUJOMIyxTY65285494;     IUJOMIyxTY65285494 = IUJOMIyxTY78372546;     IUJOMIyxTY78372546 = IUJOMIyxTY57017387;     IUJOMIyxTY57017387 = IUJOMIyxTY11598445;     IUJOMIyxTY11598445 = IUJOMIyxTY56312584;     IUJOMIyxTY56312584 = IUJOMIyxTY77357141;     IUJOMIyxTY77357141 = IUJOMIyxTY21601062;     IUJOMIyxTY21601062 = IUJOMIyxTY93774092;     IUJOMIyxTY93774092 = IUJOMIyxTY16081960;     IUJOMIyxTY16081960 = IUJOMIyxTY6405224;     IUJOMIyxTY6405224 = IUJOMIyxTY4640818;     IUJOMIyxTY4640818 = IUJOMIyxTY14245452;     IUJOMIyxTY14245452 = IUJOMIyxTY82014172;     IUJOMIyxTY82014172 = IUJOMIyxTY15211826;     IUJOMIyxTY15211826 = IUJOMIyxTY31371452;     IUJOMIyxTY31371452 = IUJOMIyxTY6545538;     IUJOMIyxTY6545538 = IUJOMIyxTY76839746;     IUJOMIyxTY76839746 = IUJOMIyxTY83850779;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IkhPmZjQYl3175717() {     int WeDCMgmfJf96636946 = -131226475;    int WeDCMgmfJf35045762 = -888618897;    int WeDCMgmfJf8410073 = 34815147;    int WeDCMgmfJf23247368 = -268815037;    int WeDCMgmfJf28594609 = -435064667;    int WeDCMgmfJf46004674 = 7308283;    int WeDCMgmfJf65459498 = -402791502;    int WeDCMgmfJf17227017 = -147657627;    int WeDCMgmfJf94952858 = 58428878;    int WeDCMgmfJf70846917 = -619100300;    int WeDCMgmfJf99752597 = -64529004;    int WeDCMgmfJf82568887 = -317028388;    int WeDCMgmfJf33618494 = 50185246;    int WeDCMgmfJf19398916 = -858831628;    int WeDCMgmfJf62743113 = 55241350;    int WeDCMgmfJf44871179 = -242839335;    int WeDCMgmfJf18749371 = -41956616;    int WeDCMgmfJf1792981 = -356750323;    int WeDCMgmfJf90590433 = -265692259;    int WeDCMgmfJf62821158 = -760317542;    int WeDCMgmfJf37395031 = -612761189;    int WeDCMgmfJf7373387 = -910538719;    int WeDCMgmfJf78707475 = -565121457;    int WeDCMgmfJf80627005 = -28822496;    int WeDCMgmfJf77592471 = -731274424;    int WeDCMgmfJf73237621 = -363238458;    int WeDCMgmfJf90229050 = -461339760;    int WeDCMgmfJf55126609 = -450126451;    int WeDCMgmfJf23797647 = -806581321;    int WeDCMgmfJf15401442 = -193623290;    int WeDCMgmfJf75780766 = -798246603;    int WeDCMgmfJf84116951 = -113185322;    int WeDCMgmfJf16777236 = -946555496;    int WeDCMgmfJf29932383 = 95952026;    int WeDCMgmfJf10186807 = -407738037;    int WeDCMgmfJf97325901 = -457695590;    int WeDCMgmfJf7383417 = 21556050;    int WeDCMgmfJf7717577 = -375566566;    int WeDCMgmfJf53580732 = -80461943;    int WeDCMgmfJf64391271 = -766550435;    int WeDCMgmfJf99923537 = -791163299;    int WeDCMgmfJf6241592 = -967589075;    int WeDCMgmfJf3152644 = -324181995;    int WeDCMgmfJf84457517 = -675231025;    int WeDCMgmfJf48315325 = -605315294;    int WeDCMgmfJf44771063 = -132107852;    int WeDCMgmfJf13811666 = -74581404;    int WeDCMgmfJf3286215 = -458478887;    int WeDCMgmfJf31888573 = -974780164;    int WeDCMgmfJf88845520 = -562388551;    int WeDCMgmfJf2605274 = -972546260;    int WeDCMgmfJf2503931 = -402315241;    int WeDCMgmfJf3684834 = -374694251;    int WeDCMgmfJf59118428 = -256377426;    int WeDCMgmfJf65780685 = 72237652;    int WeDCMgmfJf89263560 = -220687757;    int WeDCMgmfJf56338286 = -223497441;    int WeDCMgmfJf27783067 = -936362357;    int WeDCMgmfJf45654897 = -537540613;    int WeDCMgmfJf55356988 = 28173791;    int WeDCMgmfJf55775624 = -531351958;    int WeDCMgmfJf10332890 = -952665052;    int WeDCMgmfJf93429369 = -341076307;    int WeDCMgmfJf79551417 = -747947833;    int WeDCMgmfJf95066150 = -820853698;    int WeDCMgmfJf15635647 = -951343683;    int WeDCMgmfJf65791651 = -370472893;    int WeDCMgmfJf3686111 = 54233220;    int WeDCMgmfJf9212110 = -351093591;    int WeDCMgmfJf65417212 = -487063061;    int WeDCMgmfJf37487762 = -164395385;    int WeDCMgmfJf11031794 = -666390051;    int WeDCMgmfJf48212249 = -176288381;    int WeDCMgmfJf26199163 = -499141824;    int WeDCMgmfJf62897621 = -969154243;    int WeDCMgmfJf31153440 = -645172115;    int WeDCMgmfJf4220743 = -486356724;    int WeDCMgmfJf94249958 = -889890432;    int WeDCMgmfJf32311681 = -423507202;    int WeDCMgmfJf32821408 = -499166572;    int WeDCMgmfJf59425956 = -188657055;    int WeDCMgmfJf86942836 = 97139126;    int WeDCMgmfJf23238037 = -475346287;    int WeDCMgmfJf34952127 = -144192771;    int WeDCMgmfJf12796168 = -221077030;    int WeDCMgmfJf73276836 = -295931363;    int WeDCMgmfJf80432118 = -738491071;    int WeDCMgmfJf57658808 = -590178070;    int WeDCMgmfJf64151698 = -976285627;    int WeDCMgmfJf20923246 = -87050281;    int WeDCMgmfJf40987615 = -134198149;    int WeDCMgmfJf79600349 = -42081594;    int WeDCMgmfJf62062680 = -838025953;    int WeDCMgmfJf98223744 = -8635734;    int WeDCMgmfJf8615647 = -135198478;    int WeDCMgmfJf89590647 = -838498247;    int WeDCMgmfJf12812222 = -526512768;    int WeDCMgmfJf23601227 = -576234163;    int WeDCMgmfJf89391366 = -854377328;    int WeDCMgmfJf32679679 = -131226475;     WeDCMgmfJf96636946 = WeDCMgmfJf35045762;     WeDCMgmfJf35045762 = WeDCMgmfJf8410073;     WeDCMgmfJf8410073 = WeDCMgmfJf23247368;     WeDCMgmfJf23247368 = WeDCMgmfJf28594609;     WeDCMgmfJf28594609 = WeDCMgmfJf46004674;     WeDCMgmfJf46004674 = WeDCMgmfJf65459498;     WeDCMgmfJf65459498 = WeDCMgmfJf17227017;     WeDCMgmfJf17227017 = WeDCMgmfJf94952858;     WeDCMgmfJf94952858 = WeDCMgmfJf70846917;     WeDCMgmfJf70846917 = WeDCMgmfJf99752597;     WeDCMgmfJf99752597 = WeDCMgmfJf82568887;     WeDCMgmfJf82568887 = WeDCMgmfJf33618494;     WeDCMgmfJf33618494 = WeDCMgmfJf19398916;     WeDCMgmfJf19398916 = WeDCMgmfJf62743113;     WeDCMgmfJf62743113 = WeDCMgmfJf44871179;     WeDCMgmfJf44871179 = WeDCMgmfJf18749371;     WeDCMgmfJf18749371 = WeDCMgmfJf1792981;     WeDCMgmfJf1792981 = WeDCMgmfJf90590433;     WeDCMgmfJf90590433 = WeDCMgmfJf62821158;     WeDCMgmfJf62821158 = WeDCMgmfJf37395031;     WeDCMgmfJf37395031 = WeDCMgmfJf7373387;     WeDCMgmfJf7373387 = WeDCMgmfJf78707475;     WeDCMgmfJf78707475 = WeDCMgmfJf80627005;     WeDCMgmfJf80627005 = WeDCMgmfJf77592471;     WeDCMgmfJf77592471 = WeDCMgmfJf73237621;     WeDCMgmfJf73237621 = WeDCMgmfJf90229050;     WeDCMgmfJf90229050 = WeDCMgmfJf55126609;     WeDCMgmfJf55126609 = WeDCMgmfJf23797647;     WeDCMgmfJf23797647 = WeDCMgmfJf15401442;     WeDCMgmfJf15401442 = WeDCMgmfJf75780766;     WeDCMgmfJf75780766 = WeDCMgmfJf84116951;     WeDCMgmfJf84116951 = WeDCMgmfJf16777236;     WeDCMgmfJf16777236 = WeDCMgmfJf29932383;     WeDCMgmfJf29932383 = WeDCMgmfJf10186807;     WeDCMgmfJf10186807 = WeDCMgmfJf97325901;     WeDCMgmfJf97325901 = WeDCMgmfJf7383417;     WeDCMgmfJf7383417 = WeDCMgmfJf7717577;     WeDCMgmfJf7717577 = WeDCMgmfJf53580732;     WeDCMgmfJf53580732 = WeDCMgmfJf64391271;     WeDCMgmfJf64391271 = WeDCMgmfJf99923537;     WeDCMgmfJf99923537 = WeDCMgmfJf6241592;     WeDCMgmfJf6241592 = WeDCMgmfJf3152644;     WeDCMgmfJf3152644 = WeDCMgmfJf84457517;     WeDCMgmfJf84457517 = WeDCMgmfJf48315325;     WeDCMgmfJf48315325 = WeDCMgmfJf44771063;     WeDCMgmfJf44771063 = WeDCMgmfJf13811666;     WeDCMgmfJf13811666 = WeDCMgmfJf3286215;     WeDCMgmfJf3286215 = WeDCMgmfJf31888573;     WeDCMgmfJf31888573 = WeDCMgmfJf88845520;     WeDCMgmfJf88845520 = WeDCMgmfJf2605274;     WeDCMgmfJf2605274 = WeDCMgmfJf2503931;     WeDCMgmfJf2503931 = WeDCMgmfJf3684834;     WeDCMgmfJf3684834 = WeDCMgmfJf59118428;     WeDCMgmfJf59118428 = WeDCMgmfJf65780685;     WeDCMgmfJf65780685 = WeDCMgmfJf89263560;     WeDCMgmfJf89263560 = WeDCMgmfJf56338286;     WeDCMgmfJf56338286 = WeDCMgmfJf27783067;     WeDCMgmfJf27783067 = WeDCMgmfJf45654897;     WeDCMgmfJf45654897 = WeDCMgmfJf55356988;     WeDCMgmfJf55356988 = WeDCMgmfJf55775624;     WeDCMgmfJf55775624 = WeDCMgmfJf10332890;     WeDCMgmfJf10332890 = WeDCMgmfJf93429369;     WeDCMgmfJf93429369 = WeDCMgmfJf79551417;     WeDCMgmfJf79551417 = WeDCMgmfJf95066150;     WeDCMgmfJf95066150 = WeDCMgmfJf15635647;     WeDCMgmfJf15635647 = WeDCMgmfJf65791651;     WeDCMgmfJf65791651 = WeDCMgmfJf3686111;     WeDCMgmfJf3686111 = WeDCMgmfJf9212110;     WeDCMgmfJf9212110 = WeDCMgmfJf65417212;     WeDCMgmfJf65417212 = WeDCMgmfJf37487762;     WeDCMgmfJf37487762 = WeDCMgmfJf11031794;     WeDCMgmfJf11031794 = WeDCMgmfJf48212249;     WeDCMgmfJf48212249 = WeDCMgmfJf26199163;     WeDCMgmfJf26199163 = WeDCMgmfJf62897621;     WeDCMgmfJf62897621 = WeDCMgmfJf31153440;     WeDCMgmfJf31153440 = WeDCMgmfJf4220743;     WeDCMgmfJf4220743 = WeDCMgmfJf94249958;     WeDCMgmfJf94249958 = WeDCMgmfJf32311681;     WeDCMgmfJf32311681 = WeDCMgmfJf32821408;     WeDCMgmfJf32821408 = WeDCMgmfJf59425956;     WeDCMgmfJf59425956 = WeDCMgmfJf86942836;     WeDCMgmfJf86942836 = WeDCMgmfJf23238037;     WeDCMgmfJf23238037 = WeDCMgmfJf34952127;     WeDCMgmfJf34952127 = WeDCMgmfJf12796168;     WeDCMgmfJf12796168 = WeDCMgmfJf73276836;     WeDCMgmfJf73276836 = WeDCMgmfJf80432118;     WeDCMgmfJf80432118 = WeDCMgmfJf57658808;     WeDCMgmfJf57658808 = WeDCMgmfJf64151698;     WeDCMgmfJf64151698 = WeDCMgmfJf20923246;     WeDCMgmfJf20923246 = WeDCMgmfJf40987615;     WeDCMgmfJf40987615 = WeDCMgmfJf79600349;     WeDCMgmfJf79600349 = WeDCMgmfJf62062680;     WeDCMgmfJf62062680 = WeDCMgmfJf98223744;     WeDCMgmfJf98223744 = WeDCMgmfJf8615647;     WeDCMgmfJf8615647 = WeDCMgmfJf89590647;     WeDCMgmfJf89590647 = WeDCMgmfJf12812222;     WeDCMgmfJf12812222 = WeDCMgmfJf23601227;     WeDCMgmfJf23601227 = WeDCMgmfJf89391366;     WeDCMgmfJf89391366 = WeDCMgmfJf32679679;     WeDCMgmfJf32679679 = WeDCMgmfJf96636946;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gynDIDQQrZ21389058() {     int yCxEIIREvm80081252 = 88107845;    int yCxEIIREvm51917851 = -888001085;    int yCxEIIREvm4818580 = -858710080;    int yCxEIIREvm86039619 = -462614873;    int yCxEIIREvm34715933 = -241932158;    int yCxEIIREvm44509075 = -871763370;    int yCxEIIREvm36498816 = -142702816;    int yCxEIIREvm31574459 = -134815657;    int yCxEIIREvm53482560 = -283030473;    int yCxEIIREvm3392570 = 12523588;    int yCxEIIREvm49168536 = -254644876;    int yCxEIIREvm45774287 = -304646634;    int yCxEIIREvm51736270 = -841475995;    int yCxEIIREvm99025808 = -647121985;    int yCxEIIREvm23597577 = -38781632;    int yCxEIIREvm31260340 = -30918660;    int yCxEIIREvm12501256 = -895301643;    int yCxEIIREvm82644427 = -604055200;    int yCxEIIREvm53030576 = -264367130;    int yCxEIIREvm52188601 = -189903534;    int yCxEIIREvm93035418 = -202290220;    int yCxEIIREvm19520705 = -901690489;    int yCxEIIREvm82533183 = -605010654;    int yCxEIIREvm15333564 = -100647892;    int yCxEIIREvm19168429 = -190927183;    int yCxEIIREvm68579770 = -243748174;    int yCxEIIREvm39683791 = -89424102;    int yCxEIIREvm94858234 = -520081335;    int yCxEIIREvm59878740 = -669086463;    int yCxEIIREvm31234783 = -706473141;    int yCxEIIREvm93876080 = -436003804;    int yCxEIIREvm47223378 = -28380401;    int yCxEIIREvm60316123 = -567427673;    int yCxEIIREvm72090893 = -595650144;    int yCxEIIREvm88913536 = -605402773;    int yCxEIIREvm9715086 = -245719496;    int yCxEIIREvm75997595 = -957293130;    int yCxEIIREvm48296409 = -84037086;    int yCxEIIREvm2575810 = -265359740;    int yCxEIIREvm13886406 = -519143957;    int yCxEIIREvm33572462 = -362712272;    int yCxEIIREvm56729377 = -441497396;    int yCxEIIREvm26719714 = -875239238;    int yCxEIIREvm76362457 = -431413796;    int yCxEIIREvm40259822 = -333472248;    int yCxEIIREvm84377058 = -151718776;    int yCxEIIREvm89827096 = -206885633;    int yCxEIIREvm22956810 = -457200605;    int yCxEIIREvm62735685 = -460382247;    int yCxEIIREvm24178004 = -769409006;    int yCxEIIREvm75862721 = -93447683;    int yCxEIIREvm58004725 = -460574552;    int yCxEIIREvm17797293 = -586441568;    int yCxEIIREvm83795187 = 25209186;    int yCxEIIREvm28353640 = -276248471;    int yCxEIIREvm60560548 = -10201667;    int yCxEIIREvm69384668 = -182990432;    int yCxEIIREvm89485015 = -658062189;    int yCxEIIREvm66871190 = -171687690;    int yCxEIIREvm66136162 = -998183985;    int yCxEIIREvm4825285 = -682339269;    int yCxEIIREvm41640582 = -622621482;    int yCxEIIREvm71695718 = -465729194;    int yCxEIIREvm22247777 = -576557332;    int yCxEIIREvm9516489 = -551472609;    int yCxEIIREvm1945158 = -126264476;    int yCxEIIREvm85458163 = -737218962;    int yCxEIIREvm79645376 = -145825851;    int yCxEIIREvm10112272 = 58280787;    int yCxEIIREvm13882492 = -793062137;    int yCxEIIREvm55262745 = -73625531;    int yCxEIIREvm64204846 = -711264558;    int yCxEIIREvm80068617 = -238695460;    int yCxEIIREvm39144170 = -745223173;    int yCxEIIREvm18616140 = -827191263;    int yCxEIIREvm36306041 = -760792825;    int yCxEIIREvm92800991 = 73548749;    int yCxEIIREvm6170726 = -73596858;    int yCxEIIREvm75073742 = -767175645;    int yCxEIIREvm34791370 = 60791592;    int yCxEIIREvm78752674 = 63137459;    int yCxEIIREvm16726981 = -632223498;    int yCxEIIREvm32122550 = 40300911;    int yCxEIIREvm35700737 = -899677458;    int yCxEIIREvm55372061 = -513025459;    int yCxEIIREvm35871356 = -975429252;    int yCxEIIREvm29426086 = -441938834;    int yCxEIIREvm76520935 = -492636860;    int yCxEIIREvm43737254 = -219401674;    int yCxEIIREvm28352989 = -495201106;    int yCxEIIREvm40330417 = 37270936;    int yCxEIIREvm86512579 = -199230942;    int yCxEIIREvm81425219 = -912349396;    int yCxEIIREvm36439648 = -267175756;    int yCxEIIREvm9061122 = -836804689;    int yCxEIIREvm91931879 = -740090791;    int yCxEIIREvm85033659 = -975768203;    int yCxEIIREvm4471937 = -198681907;    int yCxEIIREvm66845968 = -879941188;    int yCxEIIREvm38314665 = 88107845;     yCxEIIREvm80081252 = yCxEIIREvm51917851;     yCxEIIREvm51917851 = yCxEIIREvm4818580;     yCxEIIREvm4818580 = yCxEIIREvm86039619;     yCxEIIREvm86039619 = yCxEIIREvm34715933;     yCxEIIREvm34715933 = yCxEIIREvm44509075;     yCxEIIREvm44509075 = yCxEIIREvm36498816;     yCxEIIREvm36498816 = yCxEIIREvm31574459;     yCxEIIREvm31574459 = yCxEIIREvm53482560;     yCxEIIREvm53482560 = yCxEIIREvm3392570;     yCxEIIREvm3392570 = yCxEIIREvm49168536;     yCxEIIREvm49168536 = yCxEIIREvm45774287;     yCxEIIREvm45774287 = yCxEIIREvm51736270;     yCxEIIREvm51736270 = yCxEIIREvm99025808;     yCxEIIREvm99025808 = yCxEIIREvm23597577;     yCxEIIREvm23597577 = yCxEIIREvm31260340;     yCxEIIREvm31260340 = yCxEIIREvm12501256;     yCxEIIREvm12501256 = yCxEIIREvm82644427;     yCxEIIREvm82644427 = yCxEIIREvm53030576;     yCxEIIREvm53030576 = yCxEIIREvm52188601;     yCxEIIREvm52188601 = yCxEIIREvm93035418;     yCxEIIREvm93035418 = yCxEIIREvm19520705;     yCxEIIREvm19520705 = yCxEIIREvm82533183;     yCxEIIREvm82533183 = yCxEIIREvm15333564;     yCxEIIREvm15333564 = yCxEIIREvm19168429;     yCxEIIREvm19168429 = yCxEIIREvm68579770;     yCxEIIREvm68579770 = yCxEIIREvm39683791;     yCxEIIREvm39683791 = yCxEIIREvm94858234;     yCxEIIREvm94858234 = yCxEIIREvm59878740;     yCxEIIREvm59878740 = yCxEIIREvm31234783;     yCxEIIREvm31234783 = yCxEIIREvm93876080;     yCxEIIREvm93876080 = yCxEIIREvm47223378;     yCxEIIREvm47223378 = yCxEIIREvm60316123;     yCxEIIREvm60316123 = yCxEIIREvm72090893;     yCxEIIREvm72090893 = yCxEIIREvm88913536;     yCxEIIREvm88913536 = yCxEIIREvm9715086;     yCxEIIREvm9715086 = yCxEIIREvm75997595;     yCxEIIREvm75997595 = yCxEIIREvm48296409;     yCxEIIREvm48296409 = yCxEIIREvm2575810;     yCxEIIREvm2575810 = yCxEIIREvm13886406;     yCxEIIREvm13886406 = yCxEIIREvm33572462;     yCxEIIREvm33572462 = yCxEIIREvm56729377;     yCxEIIREvm56729377 = yCxEIIREvm26719714;     yCxEIIREvm26719714 = yCxEIIREvm76362457;     yCxEIIREvm76362457 = yCxEIIREvm40259822;     yCxEIIREvm40259822 = yCxEIIREvm84377058;     yCxEIIREvm84377058 = yCxEIIREvm89827096;     yCxEIIREvm89827096 = yCxEIIREvm22956810;     yCxEIIREvm22956810 = yCxEIIREvm62735685;     yCxEIIREvm62735685 = yCxEIIREvm24178004;     yCxEIIREvm24178004 = yCxEIIREvm75862721;     yCxEIIREvm75862721 = yCxEIIREvm58004725;     yCxEIIREvm58004725 = yCxEIIREvm17797293;     yCxEIIREvm17797293 = yCxEIIREvm83795187;     yCxEIIREvm83795187 = yCxEIIREvm28353640;     yCxEIIREvm28353640 = yCxEIIREvm60560548;     yCxEIIREvm60560548 = yCxEIIREvm69384668;     yCxEIIREvm69384668 = yCxEIIREvm89485015;     yCxEIIREvm89485015 = yCxEIIREvm66871190;     yCxEIIREvm66871190 = yCxEIIREvm66136162;     yCxEIIREvm66136162 = yCxEIIREvm4825285;     yCxEIIREvm4825285 = yCxEIIREvm41640582;     yCxEIIREvm41640582 = yCxEIIREvm71695718;     yCxEIIREvm71695718 = yCxEIIREvm22247777;     yCxEIIREvm22247777 = yCxEIIREvm9516489;     yCxEIIREvm9516489 = yCxEIIREvm1945158;     yCxEIIREvm1945158 = yCxEIIREvm85458163;     yCxEIIREvm85458163 = yCxEIIREvm79645376;     yCxEIIREvm79645376 = yCxEIIREvm10112272;     yCxEIIREvm10112272 = yCxEIIREvm13882492;     yCxEIIREvm13882492 = yCxEIIREvm55262745;     yCxEIIREvm55262745 = yCxEIIREvm64204846;     yCxEIIREvm64204846 = yCxEIIREvm80068617;     yCxEIIREvm80068617 = yCxEIIREvm39144170;     yCxEIIREvm39144170 = yCxEIIREvm18616140;     yCxEIIREvm18616140 = yCxEIIREvm36306041;     yCxEIIREvm36306041 = yCxEIIREvm92800991;     yCxEIIREvm92800991 = yCxEIIREvm6170726;     yCxEIIREvm6170726 = yCxEIIREvm75073742;     yCxEIIREvm75073742 = yCxEIIREvm34791370;     yCxEIIREvm34791370 = yCxEIIREvm78752674;     yCxEIIREvm78752674 = yCxEIIREvm16726981;     yCxEIIREvm16726981 = yCxEIIREvm32122550;     yCxEIIREvm32122550 = yCxEIIREvm35700737;     yCxEIIREvm35700737 = yCxEIIREvm55372061;     yCxEIIREvm55372061 = yCxEIIREvm35871356;     yCxEIIREvm35871356 = yCxEIIREvm29426086;     yCxEIIREvm29426086 = yCxEIIREvm76520935;     yCxEIIREvm76520935 = yCxEIIREvm43737254;     yCxEIIREvm43737254 = yCxEIIREvm28352989;     yCxEIIREvm28352989 = yCxEIIREvm40330417;     yCxEIIREvm40330417 = yCxEIIREvm86512579;     yCxEIIREvm86512579 = yCxEIIREvm81425219;     yCxEIIREvm81425219 = yCxEIIREvm36439648;     yCxEIIREvm36439648 = yCxEIIREvm9061122;     yCxEIIREvm9061122 = yCxEIIREvm91931879;     yCxEIIREvm91931879 = yCxEIIREvm85033659;     yCxEIIREvm85033659 = yCxEIIREvm4471937;     yCxEIIREvm4471937 = yCxEIIREvm66845968;     yCxEIIREvm66845968 = yCxEIIREvm38314665;     yCxEIIREvm38314665 = yCxEIIREvm80081252;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FTduYCOoDY91844930() {     int iVShkZOwWu92867419 = -46932847;    int iVShkZOwWu38688739 = 41664901;    int iVShkZOwWu22571380 = -167069462;    int iVShkZOwWu93980444 = -286817150;    int iVShkZOwWu28227038 = -733748623;    int iVShkZOwWu87819145 = -648686684;    int iVShkZOwWu57706574 = -688374886;    int iVShkZOwWu58293572 = -579333914;    int iVShkZOwWu48887370 = -244788565;    int iVShkZOwWu9611275 = -891564045;    int iVShkZOwWu76655918 = -369330892;    int iVShkZOwWu77724791 = -811979450;    int iVShkZOwWu21879573 = -745258653;    int iVShkZOwWu48649887 = -979537183;    int iVShkZOwWu46904130 = 54232537;    int iVShkZOwWu59685409 = -581810050;    int iVShkZOwWu68253073 = -559076110;    int iVShkZOwWu66202912 = -159265502;    int iVShkZOwWu54191206 = -173836325;    int iVShkZOwWu54843097 = -789147673;    int iVShkZOwWu73209475 = -693606485;    int iVShkZOwWu48568524 = 28451163;    int iVShkZOwWu21293479 = -536014549;    int iVShkZOwWu59840693 = -460992904;    int iVShkZOwWu25062688 = -401369107;    int iVShkZOwWu49898416 = 65820627;    int iVShkZOwWu51099327 = -723130383;    int iVShkZOwWu62211159 = -693879439;    int iVShkZOwWu76495793 = -883303704;    int iVShkZOwWu83985898 = -336359296;    int iVShkZOwWu38312077 = -712420554;    int iVShkZOwWu50542202 = -7100080;    int iVShkZOwWu6362072 = -443282457;    int iVShkZOwWu49167266 = -500404363;    int iVShkZOwWu71974069 = -115434653;    int iVShkZOwWu4939200 = -34282600;    int iVShkZOwWu12761656 = -747202481;    int iVShkZOwWu6000432 = -3657177;    int iVShkZOwWu8351196 = -107210317;    int iVShkZOwWu40151136 = -363560803;    int iVShkZOwWu95003769 = -738186266;    int iVShkZOwWu64431833 = -339808730;    int iVShkZOwWu61603183 = -898672683;    int iVShkZOwWu20990996 = -772586039;    int iVShkZOwWu20268312 = -226242208;    int iVShkZOwWu58005188 = -225037215;    int iVShkZOwWu67409115 = -704400686;    int iVShkZOwWu52396776 = -612553385;    int iVShkZOwWu6799735 = -623984221;    int iVShkZOwWu71128424 = -280445469;    int iVShkZOwWu94190215 = -346331991;    int iVShkZOwWu86181272 = -931759890;    int iVShkZOwWu52282442 = -762444850;    int iVShkZOwWu28494913 = -664242556;    int iVShkZOwWu18635456 = -270692958;    int iVShkZOwWu44298896 = 24615989;    int iVShkZOwWu17395261 = -422320551;    int iVShkZOwWu62730686 = -706076559;    int iVShkZOwWu68917756 = -885448043;    int iVShkZOwWu78328622 = -699569251;    int iVShkZOwWu36719819 = -925556302;    int iVShkZOwWu95495415 = -994495447;    int iVShkZOwWu81797778 = -696030211;    int iVShkZOwWu64901472 = -908429270;    int iVShkZOwWu71299198 = -79143492;    int iVShkZOwWu26113716 = -262230812;    int iVShkZOwWu71362719 = -268696994;    int iVShkZOwWu72712306 = -144854290;    int iVShkZOwWu76675818 = -764102531;    int iVShkZOwWu41964930 = -911484864;    int iVShkZOwWu46923754 = -834607569;    int iVShkZOwWu62252641 = -455418933;    int iVShkZOwWu57851716 = 47944815;    int iVShkZOwWu14040071 = -810275522;    int iVShkZOwWu59839328 = 49038592;    int iVShkZOwWu8777642 = -253797755;    int iVShkZOwWu86965340 = -72876154;    int iVShkZOwWu302484 = -763428510;    int iVShkZOwWu39572382 = -134750697;    int iVShkZOwWu67057500 = -76331892;    int iVShkZOwWu82489300 = -229778687;    int iVShkZOwWu98702550 = -10576998;    int iVShkZOwWu55411425 = 30104781;    int iVShkZOwWu5367370 = -502858235;    int iVShkZOwWu89795683 = -990027306;    int iVShkZOwWu52130804 = -780660664;    int iVShkZOwWu98259759 = -244655230;    int iVShkZOwWu77867159 = -779039902;    int iVShkZOwWu30531811 = -129711405;    int iVShkZOwWu27675173 = -40050643;    int iVShkZOwWu87543939 = -611962050;    int iVShkZOwWu50030969 = 58874077;    int iVShkZOwWu37082676 = -118209135;    int iVShkZOwWu30022574 = -407641067;    int iVShkZOwWu3431317 = -438004502;    int iVShkZOwWu99508354 = -743690819;    int iVShkZOwWu82634055 = -643778520;    int iVShkZOwWu96701711 = -990243414;    int iVShkZOwWu49691797 = -593442548;    int iVShkZOwWu94154596 = -46932847;     iVShkZOwWu92867419 = iVShkZOwWu38688739;     iVShkZOwWu38688739 = iVShkZOwWu22571380;     iVShkZOwWu22571380 = iVShkZOwWu93980444;     iVShkZOwWu93980444 = iVShkZOwWu28227038;     iVShkZOwWu28227038 = iVShkZOwWu87819145;     iVShkZOwWu87819145 = iVShkZOwWu57706574;     iVShkZOwWu57706574 = iVShkZOwWu58293572;     iVShkZOwWu58293572 = iVShkZOwWu48887370;     iVShkZOwWu48887370 = iVShkZOwWu9611275;     iVShkZOwWu9611275 = iVShkZOwWu76655918;     iVShkZOwWu76655918 = iVShkZOwWu77724791;     iVShkZOwWu77724791 = iVShkZOwWu21879573;     iVShkZOwWu21879573 = iVShkZOwWu48649887;     iVShkZOwWu48649887 = iVShkZOwWu46904130;     iVShkZOwWu46904130 = iVShkZOwWu59685409;     iVShkZOwWu59685409 = iVShkZOwWu68253073;     iVShkZOwWu68253073 = iVShkZOwWu66202912;     iVShkZOwWu66202912 = iVShkZOwWu54191206;     iVShkZOwWu54191206 = iVShkZOwWu54843097;     iVShkZOwWu54843097 = iVShkZOwWu73209475;     iVShkZOwWu73209475 = iVShkZOwWu48568524;     iVShkZOwWu48568524 = iVShkZOwWu21293479;     iVShkZOwWu21293479 = iVShkZOwWu59840693;     iVShkZOwWu59840693 = iVShkZOwWu25062688;     iVShkZOwWu25062688 = iVShkZOwWu49898416;     iVShkZOwWu49898416 = iVShkZOwWu51099327;     iVShkZOwWu51099327 = iVShkZOwWu62211159;     iVShkZOwWu62211159 = iVShkZOwWu76495793;     iVShkZOwWu76495793 = iVShkZOwWu83985898;     iVShkZOwWu83985898 = iVShkZOwWu38312077;     iVShkZOwWu38312077 = iVShkZOwWu50542202;     iVShkZOwWu50542202 = iVShkZOwWu6362072;     iVShkZOwWu6362072 = iVShkZOwWu49167266;     iVShkZOwWu49167266 = iVShkZOwWu71974069;     iVShkZOwWu71974069 = iVShkZOwWu4939200;     iVShkZOwWu4939200 = iVShkZOwWu12761656;     iVShkZOwWu12761656 = iVShkZOwWu6000432;     iVShkZOwWu6000432 = iVShkZOwWu8351196;     iVShkZOwWu8351196 = iVShkZOwWu40151136;     iVShkZOwWu40151136 = iVShkZOwWu95003769;     iVShkZOwWu95003769 = iVShkZOwWu64431833;     iVShkZOwWu64431833 = iVShkZOwWu61603183;     iVShkZOwWu61603183 = iVShkZOwWu20990996;     iVShkZOwWu20990996 = iVShkZOwWu20268312;     iVShkZOwWu20268312 = iVShkZOwWu58005188;     iVShkZOwWu58005188 = iVShkZOwWu67409115;     iVShkZOwWu67409115 = iVShkZOwWu52396776;     iVShkZOwWu52396776 = iVShkZOwWu6799735;     iVShkZOwWu6799735 = iVShkZOwWu71128424;     iVShkZOwWu71128424 = iVShkZOwWu94190215;     iVShkZOwWu94190215 = iVShkZOwWu86181272;     iVShkZOwWu86181272 = iVShkZOwWu52282442;     iVShkZOwWu52282442 = iVShkZOwWu28494913;     iVShkZOwWu28494913 = iVShkZOwWu18635456;     iVShkZOwWu18635456 = iVShkZOwWu44298896;     iVShkZOwWu44298896 = iVShkZOwWu17395261;     iVShkZOwWu17395261 = iVShkZOwWu62730686;     iVShkZOwWu62730686 = iVShkZOwWu68917756;     iVShkZOwWu68917756 = iVShkZOwWu78328622;     iVShkZOwWu78328622 = iVShkZOwWu36719819;     iVShkZOwWu36719819 = iVShkZOwWu95495415;     iVShkZOwWu95495415 = iVShkZOwWu81797778;     iVShkZOwWu81797778 = iVShkZOwWu64901472;     iVShkZOwWu64901472 = iVShkZOwWu71299198;     iVShkZOwWu71299198 = iVShkZOwWu26113716;     iVShkZOwWu26113716 = iVShkZOwWu71362719;     iVShkZOwWu71362719 = iVShkZOwWu72712306;     iVShkZOwWu72712306 = iVShkZOwWu76675818;     iVShkZOwWu76675818 = iVShkZOwWu41964930;     iVShkZOwWu41964930 = iVShkZOwWu46923754;     iVShkZOwWu46923754 = iVShkZOwWu62252641;     iVShkZOwWu62252641 = iVShkZOwWu57851716;     iVShkZOwWu57851716 = iVShkZOwWu14040071;     iVShkZOwWu14040071 = iVShkZOwWu59839328;     iVShkZOwWu59839328 = iVShkZOwWu8777642;     iVShkZOwWu8777642 = iVShkZOwWu86965340;     iVShkZOwWu86965340 = iVShkZOwWu302484;     iVShkZOwWu302484 = iVShkZOwWu39572382;     iVShkZOwWu39572382 = iVShkZOwWu67057500;     iVShkZOwWu67057500 = iVShkZOwWu82489300;     iVShkZOwWu82489300 = iVShkZOwWu98702550;     iVShkZOwWu98702550 = iVShkZOwWu55411425;     iVShkZOwWu55411425 = iVShkZOwWu5367370;     iVShkZOwWu5367370 = iVShkZOwWu89795683;     iVShkZOwWu89795683 = iVShkZOwWu52130804;     iVShkZOwWu52130804 = iVShkZOwWu98259759;     iVShkZOwWu98259759 = iVShkZOwWu77867159;     iVShkZOwWu77867159 = iVShkZOwWu30531811;     iVShkZOwWu30531811 = iVShkZOwWu27675173;     iVShkZOwWu27675173 = iVShkZOwWu87543939;     iVShkZOwWu87543939 = iVShkZOwWu50030969;     iVShkZOwWu50030969 = iVShkZOwWu37082676;     iVShkZOwWu37082676 = iVShkZOwWu30022574;     iVShkZOwWu30022574 = iVShkZOwWu3431317;     iVShkZOwWu3431317 = iVShkZOwWu99508354;     iVShkZOwWu99508354 = iVShkZOwWu82634055;     iVShkZOwWu82634055 = iVShkZOwWu96701711;     iVShkZOwWu96701711 = iVShkZOwWu49691797;     iVShkZOwWu49691797 = iVShkZOwWu94154596;     iVShkZOwWu94154596 = iVShkZOwWu92867419;}
// Junk Finished
