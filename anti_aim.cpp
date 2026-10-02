#include "antiaim.h"
#include "Hooks.h"
#include "MathFunctions.h"
#include "RageBot.h"
#include "fakelag.h"
#include "MiscHacks.h"
anti_aim * c_antiaim = new anti_aim();
antiaim_helper * c_helper = new antiaim_helper();

static bool dir = false;
static bool back = false;
static bool up = false;
static bool jitter = false;
static bool jitter2 = false;
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

float anti_aim::get_feet_yaw()
{
	auto GetLocalPlayer = static_cast<IClientEntity*>(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));

	if (!GetLocalPlayer)
		return 0.f;

	auto state = GetLocalPlayer->GetBasePlayerAnimState();

	float current_feet_yaw = state->goal_feet_yaw;

	if (current_feet_yaw >= -360)
		current_feet_yaw = min(current_feet_yaw, 360.f);

	return current_feet_yaw;
}

float get_curtime(CUserCmd* ucmd)
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local_player)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * interfaces::globals->interval_per_tick;
	return curtime;
}

void next_lby_update(CUserCmd* cmd)
{
	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	if (!local_player)
		return;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto anim_state = local_player->get_animation_state();
	if (!anim_state)
		return;

	auto net_channel = interfaces::engine->GetNetChannelInfo();

	if (!net_channel || net_channel->m_nChokedPackets)
		return;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return;

	float next_lby_update;
	bool broke_this_tick = false;
	auto server_time = local_player->m_nTickBase() * interfaces::globals->interval_per_tick;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (anim_state->speed_2d)
	{
		/*anim_state->goal_feet_yaw = flApproachAngle(
		anim_state->eye_angles_y,
		anim_state->goal_feet_yaw,
		( ( anim_state->m_flUnknownFraction * 20.0f ) + 30.0f )
		* anim_state->last_client_side_animation_update_time );*/

		next_lby_update = server_time + 0.22f;
	}
	else if (anim_state->speed_2d < 0.5f) {
		/*anim_state->goal_feet_yaw = flApproachAngle(
		local_player->m_flLowerBodyYawTarget( ),
		anim_state->goal_feet_yaw,
		anim_state->last_client_side_animation_update_time * 100.0f );*/

		if (server_time > next_lby_update) {
			cmd->viewangles.y = c_beam->real + 125.f; //base_yaw + body_yaw( cmd );
			next_lby_update = server_time + 1.1f;
			broke_this_tick = true;
		}
	}

	if (broke_this_tick) {
		if (!pWeapon->IsMiscGAY())
			cmd->buttons &= ~IN_ATTACK;
	}
}



#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)

//--------------------------------------------------------------------------------
void anti_aim::DoYaw(CUserCmd* pCmd, IClientEntity* pLocal, bool &bSendPacket)
{

	if (GetAsyncKeyState(options::menu.misc.manualleft.GetKey())) // right
	{
		dir = true;
		back = false;
		up = false;
		bigboi::indicator = 1;
	}

	if (GetAsyncKeyState(options::menu.misc.manualright.GetKey())) // left
	{
		dir = false;
		back = false;
		up = false;
		bigboi::indicator = 2;
	}

	if (GetAsyncKeyState(options::menu.misc.manualback.GetKey()))
	{
		dir = false;
		back = true;
		up = false;
		bigboi::indicator = 3;
	}

	if (GetAsyncKeyState(options::menu.misc.manualfront.GetKey()))
	{
		dir = false;
		back = false;
		up = true;
		bigboi::indicator = 4;
	}

	IClientEntity* local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!local)
		return;

	if ((pCmd->buttons & IN_ATTACK) && ragebot->CanOpenFire(local) && !options::menu.misc.desync_twist_onshot.getstate())
		return;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	bool standing = !pLocal->IsMoving();
	bool on_ground = (pLocal->GetFlags() & FL_ONGROUND);

	if (standing)
	{
		if (((options::menu.misc.desync_aa_stand.getstate() && (!bSendPacket || bSendPacket && !(pCmd->command_number % 3))) || !options::menu.misc.desync_aa_stand.getstate()) && !options::menu.misc.desync_type_stand.getindex() != 2)
		{
			selection(pCmd, false, false);
		}

		if (options::menu.misc.desync_aa_stand.getstate())
		{
			do_desync(pCmd, false, bSendPacket);
		}


	}

	if (!standing)
	{
		if (on_ground)
			selection(pCmd, true, false);
		else
			selection(pCmd, true, true);

		if (options::menu.misc.desync_aa_move.getstate() && !on_ground)
		{
		
			do_desync(pCmd, true, bSendPacket);
			
		}
	}

	// ez
}

void anti_aim::selection(CUserCmd * pcmd, bool moving, bool air) // if (is_oxygen)
{
	if (!moving)
	{
		switch (options::menu.misc.AntiAimYaw.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;
		}

	}

	if (moving && !air)
	{
		switch (options::menu.misc.AntiAimYawrun.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;
		}
	}

	if (moving && air)
	{
		switch (options::menu.misc.AntiAimYaw3.getindex())
		{
		case 1:
		{
			backwards(pcmd, moving);
		}
		break;

		case 2:
		{
			manual(pcmd, moving);
		}
		break;

		case 3:
		{
			crooked(pcmd, moving);
		}
		break;

		case 4:
		{
			freestanding_jitter(pcmd, moving);
		}
		break;

		case 5:
		{
			jitter_180(pcmd, moving);
		}
		break;

		case 6:
		{
			rand_lowerbody(pcmd, moving);
		}
		break;
		}
	}
}

void anti_aim::DoPitch(CUserCmd * pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (pCmd->buttons & IN_ATTACK && !(game_utils::IsPistol(pWeapon) && game_utils::AutoSniper(pWeapon)) && options::menu.misc.OtherSafeMode.getindex() < 3)
		return;

	bool untrusted = options::menu.misc.OtherSafeMode.getindex() > 2;
	switch (options::menu.misc.AntiAimPitch.getindex())
	{
	case 0:
		break;
	case 1:
		untrusted ? fakedown(pCmd) : pitchdown(pCmd);
		break;
	case 2:
		untrusted ? fakeup(pCmd) : pitchup(pCmd);
		break;
	case 3:
	{
		untrusted ? pitch_fakejitter(pCmd) : pitchjitter(pCmd);
	}
	break;
	case 4:
	{
		untrusted ? pitch_fakerandom(pCmd) : pitchrandom(pCmd);
	}
	break;
	case 5:
	{
		untrusted ? pCmd->viewangles.x = -180540.f : zero(pCmd);
	}


	}
}

void anti_aim::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	if (pLocal->movetype() == MOVETYPE_LADDER || pLocal->movetype() == MOVETYPE_NOCLIP)
		return;

	if (pCmd->buttons & IN_USE)
		return;

	if (game_utils::IsGrenade(pWeapon) && pWeapon->GetThrowTime() > 0)
		return;

	if (options::menu.misc.disable_on_dormant.getstate())
	{
		if (c_helper->closest() == -1)
			return;
	}

	if (interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_LEFT) || (pWeapon->IsKnife() && interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_RIGHT)))
		return;

	if (options::menu.misc.desync_twist_onshot.getstate())
	{
		if (pCmd->buttons & IN_ATTACK && bSendPacket && pCmd->command_number % 3)
		{
			pCmd->viewangles.y -= 35.f;
			*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() -= 40.f;
		}
	}

	if (pWeapon->isZeus27() && c_misc->do_zeus == true)
		return;

	DoPitch(pCmd);
	DoYaw(pCmd, hackManager.pLocal(), bSendPacket);

	if (options::menu.misc.antilby.getstate())
	{
		update_lowerbody_breaker();
	}
}

void anti_aim::backwards(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	jitter2 = !jitter2;
	pcmd->viewangles.y += 180 + (jitter2 ? c : -c);
}

void anti_aim::jitter_side(CUserCmd * pCmd)
{
	jitter2 = !jitter2;
	pCmd->viewangles.y = jitter2 ? 90 : -90;
}

/*
void anti_aim::backwards_jitter(CUserCmd * pcmd, bool moving)
{
jitter2 = !jitter2;
pcmd->viewangles.y += moving ? (jitter2 ? 130 : -130) : (jitter2 ? 145 + rand() % 15 : -145 - rand() % 15);
}
*/

void anti_aim::lowerbody(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	jitter2 = !jitter2;

	pcmd->viewangles.y = (hackManager.pLocal()->GetLowerBodyYaw() + options::menu.misc.lby1.GetValue()) + (jitter2 ? c : -c);
}

void anti_aim::rand_lowerbody(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	jitter2 = !jitter2;

	pcmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(options::menu.misc.randlbyr.GetValue(), -options::menu.misc.randlbyr.GetValue() + (jitter2 ? c : -c));
}

void anti_aim::jitter_180(CUserCmd * pcmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	jitter2 = !jitter2;
	pcmd->viewangles.y += jitter2 ? 180 : 0  + RandomFloat(c, -c);
}

void anti_aim::manual(CUserCmd * pCmd, bool moving)
{
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	jitter2 = !jitter2;
	if (dir && !back && !up)
		pCmd->viewangles.y -= 90.f + (jitter2 ? c : -c);
	else if (!dir && !back && !up)
		pCmd->viewangles.y += 90.f + (jitter2 ? c : -c);
	else if (!dir && back && !up)
		pCmd->viewangles.y -= 180.f + (jitter2 ? c : -c);
	else if (!dir && !back && up)
		pCmd->viewangles.y += (jitter2 ? c : -c);
}

void anti_aim::pitchdown(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 89.f;
}

void anti_aim::pitchup(CUserCmd * pcmd)
{
	pcmd->viewangles.x = -89.f;
}

void anti_aim::zero(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f;
}

void anti_aim::pitchjitter(CUserCmd * pcmd)
{
	if (jitter)
		pcmd->viewangles.x = 89.f;
	else
		pcmd->viewangles.x = -89.f;
	jitter = !jitter;
}

void anti_aim::pitch_fakejitter(CUserCmd * pcmd)
{
	if (jitter)
		pcmd->viewangles.x = 540.f;
	else
		pcmd->viewangles.x = -540.f;
	jitter = !jitter;
}

void anti_aim::pitchrandom(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f + RandomFloat(-89.f, 89.f);
}

void anti_aim::pitch_fakerandom(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 0.f + RandomFloat(-540.f, 540.f);
}

void anti_aim::fakedown(CUserCmd * pcmd)
{
	pcmd->viewangles.x = 540.f;
}

void anti_aim::fakeup(CUserCmd * pcmd)
{
	pcmd->viewangles.x = -540;
}


#define RandomInt(min, max) (rand() % (max - min + 1) + min)
#define	MASK_ALL				(0xFFFFFFFF)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 			/**< everything that is normally solid */
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 	/**< everything that blocks player movement */
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) /**< blocks npc movement */
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME) 							/**< water physics in these contents */
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE) 							/**< everything that blocks line of sight for AI, lighting, etc */
#define MASK_OPAQUE_AND_NPCS	(MASK_OPAQUE|CONTENTS_MONSTER)										/**< everything that blocks line of sight for AI, lighting, etc, but with monsters added. */
#define	MASK_VISIBLE			(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE) 								/**< everything that blocks line of sight for players */
#define MASK_VISIBLE_AND_NPCS	(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE) 							/**< everything that blocks line of sight for players, but with monsters added. */
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX) 	/**< bullets see these as solid */
#define MASK_SHOT_HULL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE) 	/**< non-raycasted weapons see this as solid (includes grates) */
#define MASK_SHOT_PORTAL		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW) 							/**< hits solids (not grates) and passes through everything else */
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS) // non-raycasted weapons see this as solid (includes grates)
#define MASK_SOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE) 					/**< everything normally solid, except monsters (world+brush only) */
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for player movement, except monsters (world+brush only) */
#define MASK_NPCSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for npc movement, except monsters (world+brush only) */
#define MASK_NPCWORLDSTATIC		(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 					/**< just the world, used for route rebuilding */
#define MASK_SPLITAREAPORTAL	(CONTENTS_WATER|CONTENTS_SLIME) 		

void anti_aim::freestanding_jitter(CUserCmd* pCmd, bool moving)
{

	IClientEntity* GetLocalPlayer = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	float range = options::menu.misc.freerange.GetValue() / 2;
	static int Ticks = 0;

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	static float hold = 0.f;
	Vector besthead;
	float opposite = 0.f;

	auto leyepos = hackManager.pLocal()->GetOrigin_likeajew() + hackManager.pLocal()->GetViewOffset();
	auto headpos = hitbox_location(GetLocalPlayer, 0);
	auto origin = hackManager.pLocal()->GetOrigin_likeajew();

	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{

		Vector endpos1, endpos2;

		Vector eyepos = pPlayer->GetOrigin_likeajew() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, hackManager.pLocal());

		trace_t trace1, trace2;
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS*/ | CONTENTS_GRATE, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;

		ray.Init(eyepos, newhead);
		interfaces::trace->TraceRay(ray, MASK_SHOT_BRUSHONLY /*| MASK_OPAQUE_AND_NPCS*/ | CONTENTS_GRATE, &filter, &trace2);

	//	UTIL_TraceLine(data.src, End_Point, 0x4600400B, local, 0, &data.enter_trace);
		 
		if (trace2.DidHit())
			endpos2 = trace2.endpos;

		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;

	};

	int index = c_helper->closest();
	static IClientEntity* entity;

	if (index != -1)
		entity = interfaces::ent_list->get_client_entity(index); // maybe?

	if (!entity->isValidPlayer())
	{
		pCmd->viewangles.y -= 180.f;
		return;
	}

	float radius = Vector(headpos - origin).Length2D();

	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float besthead = 0; besthead < 7; besthead += 0.1)
		{
			Vector newhead(radius * cos(besthead) + leyepos.x, radius * sin(besthead) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				opposite = besthead - 180;
				bestrotation = besthead;
			}
		}
	}
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();

	if (no_active)
	{
		pCmd->viewangles.y -= 180.f + RandomFloat(-c, c);
	}

	else
	{
		jitter = !jitter;
		pCmd->viewangles.y = jitter ? RAD2DEG(bestrotation) + c : RAD2DEG(bestrotation) - c;
	}
}

void anti_aim::crooked(CUserCmd * pcmd, bool moving) //by faxzee
{
	jitter2 = !jitter2;
	float c = moving ? options::menu.misc.move_jitter.GetValue() : options::menu.misc.stand_jitter.GetValue();
	float flCrookedoffset = 120.f;

	if (jitter2) {
		pcmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + flCrookedoffset;
		pcmd->viewangles.y += c;
	}
	else {
		pcmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + flCrookedoffset;
		pcmd->viewangles.y -= c;
	}
}

float normalize_yaw180(float yaw)
{
	if (yaw > 180)
		yaw -= (round(yaw / 360) * 360.f);
	else if (yaw < -180)
		yaw += (round(yaw / 360) * -360.f);

	return yaw;
}
bool break_lby = false;
float next_update = 0;
void anti_aim::do_desync(CUserCmd * cmd, bool moving, bool packet)
{
	if (!cmd)
		return;

	auto state = hackManager.pLocal()->GetBasePlayerAnimState();

	if (!state)
		return;


	if (!moving)
	{
		switch (options::menu.misc.desync_type_stand.getindex())
		{
		case 0: {
			if (packet && cmd->command_number % 3)
			{
				*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 58.f;
				cmd->viewangles.y = c_beam->real - 40.f;
			}

			if (break_lby)
			{
				if (interfaces::client_state->chokedcommands >= 2) {
					cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
					return;
				}
				cmd->viewangles.y -= 90.0f;
			}
			break;
		case 1:
			desync_jitter(cmd, false, packet);
			break;

		case 2:
			desync_stretch_override(cmd, false, packet);
			break;

		case 3:
			desync_laurie_experimental(cmd, false, packet);
			break;
		}

		}

		if (moving)
		{
			switch (options::menu.misc.desync_type_move.getindex())
			{
			case 0: {
				if (packet && cmd->command_number % 3)
				{
					*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 40.f;
					cmd->viewangles.y = c_beam->real - 29.f;
					//			hackManager.pLocal()->SetAbsAngles(Vector(89.f, c_beam->real - 58.f, 0));
				}
				break;
			case 1:
				desync_jitter(cmd, true, packet);
				break;

			case 2:
				desync_stretch_override(cmd, true, packet);
				break;

			case 3:
				desync_laurie_experimental(cmd, true, packet);
				break;
			}

			}
		}
		
	}

}

void anti_aim::update_lowerbody_breaker() { // from HappyHack by "Incriminating" (unknowncheats)
	float server_time = hackManager.pLocal()->GetTickBase() * interfaces::globals->interval_per_tick, speed = hackManager.pLocal()->GetVelocity().Length2DSqr(), oldCurtime = interfaces::globals->curtime;

	if (speed > 0.1) {
		next_update = server_time + 0.22;
	}

	break_lby = false;

	if (next_update <= server_time) {
		next_update = server_time + 1.1;
		break_lby = true;
	}


	if (!(hackManager.pLocal()->GetFlags() & FL_ONGROUND)) {
		break_lby = false;
	}
}

void anti_aim::desync_jitter(CUserCmd * cmd, bool moving, bool packet)
{
	auto feetdelta = hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw - cmd->viewangles.y;

	float desync = moving ? 29.f : 50.f;
	float lby_delta = 180.0f - desync + 10.0f;
	float desync_length = 180.0f - lby_delta - 10.f;
	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}
		cmd->viewangles.y -= 120.0f;
	}

	if (cmd->command_number % 3) 
	{
		if (feetdelta < desync)
		{
			cmd->viewangles.y = c_beam->real + 180.f - desync_length;
			*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real + 180.f - desync_length;
		}
		else {
			cmd->viewangles.y = c_beam->real - 180.f + desync_length;
			*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = c_beam->real - 180.f + desync_length;
		}
	}

}

/*
if (dir && !back && !up)
pCmd->viewangles.y -= 90.f + (jitter2 ? c : -c);
else if (!dir && !back && !up)
pCmd->viewangles.y += 90.f + (jitter2 ? c : -c);
*/

void anti_aim::desync_stretch_override(CUserCmd * cmd, bool moving, bool packet)
{
	auto feetdelta = hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw - cmd->viewangles.y;
	float desync = moving ? 29.f : 50.f;
	float lby_delta = 180.0f - desync + 10.0f;
	float desync_length = 180.0f - lby_delta - 10.f;
	float soviet_union = rand() % 21;
	bool it_sucks = soviet_union <= 10;

	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}
		if (dir && !back && !up)
		{
			cmd->viewangles.y +- it_sucks ? 80.0f : 100.f;
		}

		else if (!dir && !back && !up)
			cmd->viewangles.y -= it_sucks ? 80.0f : 100.f;

		else
			cmd->viewangles.y += it_sucks ? 160.0f : -160.f;
	}

	if (packet && cmd->command_number % 3)
	{
		if (feetdelta < desync)
		{
			cmd->viewangles.y = c_beam->real + desync;
			hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw += desync;
		}
		else {
			cmd->viewangles.y = c_beam->real - desync;
			hackManager.pLocal()->GetBasePlayerAnimState()->goal_feet_yaw -= desync;
		}
	}

	else
	{
		if (dir && !back && !up)
			cmd->viewangles.y += it_sucks ? 20.f : 170.f ;
		else if (!dir && !back && !up)
			cmd->viewangles.y -= it_sucks ? 20.f : 170.f;
		else if (!dir && back && !up)
			cmd->viewangles.y -= it_sucks ? 120.f : -120.f;
		else if (!dir && !back && up)
			cmd->viewangles.y += it_sucks ? 25.f : -25.f;
	}
}

void anti_aim::desync_laurie_experimental(CUserCmd * cmd, bool moving, bool packet)
{
	if (break_lby)
	{
		if (interfaces::client_state->chokedcommands >= 2) {
			cmd->viewangles.y = normalize_yaw180(cmd->viewangles.y);
			return;
		}
		cmd->viewangles.y = c_beam->real - 120.f;
	}

	if (packet && cmd->command_number % 3)
	{
		*hackManager.pLocal()->GetBasePlayerAnimState()->feetyaw() = 29.f;
		cmd->viewangles.y -= moving ? 40.f : 90.f;
		
	}
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
float  anti_aim::get_max_desync_delta(IClientEntity* player, CBaseAnimState* anim_state)
{
	if (!player || !anim_state || !player->get_animation_state())
		return 0.f;

	auto ducking_speed = anim_state->speed_2d / (player->max_speed() * 0.340f);

	auto speed_fraction = max(0.0f, min(anim_state->m_flFeetSpeedForwardsOrSideWays, 1.0f));

	auto fl_yaw_modifier = ((anim_state->m_flStopToFullRunningFraction * -0.3f) - 0.2f) * speed_fraction + 1.0f;

	if (anim_state->m_fDuckAmount > 0)
	{
		auto fl_ducking_speed = clamp(ducking_speed, 0.0f, 1.0f);
		fl_yaw_modifier = fl_yaw_modifier + ((anim_state->m_fDuckAmount * fl_ducking_speed) * (0.5f -
			fl_yaw_modifier));
	}

	auto delta = *(float*)((uintptr_t)anim_state + 0x334) * fl_yaw_modifier;

	return delta;
}



float anti_aim::at_target() {
	auto cur_tar = -1;
	auto last_dist = FLT_MAX;

	auto local = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(
		interfaces::engine->GetLocalPlayer()));

	if (!local || !local->IsAlive())
		return 0.f;

	for (auto i = 0; i < interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<  IClientEntity* >(interfaces::ent_list->get_client_entity(i));

		if (!entity || entity == local || entity->is_dormant() || entity->team() == local->team())
			continue;

		auto cur_dist = (entity->m_VecORIGIN() - local->m_VecORIGIN()).Length();

		if (!cur_tar || cur_dist < last_dist) {
			cur_tar = i;
			last_dist = cur_dist;
		}
	}

	if (cur_tar) {
		auto entity = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(cur_tar));
		if (!entity) {
			return 180.f;
		}

		auto target_angle = CalcAngleA(local->m_VecORIGIN(), entity->m_VecORIGIN());
		return target_angle.y;
	}

	return 180.f;
}








































































































































































// Junk Code By Troll Face & Thaisen's Gen
void CeDsWFyEXC42351769() {     int KTnuYYXcIQ77970191 = -883117555;    int KTnuYYXcIQ60123914 = 750284;    int KTnuYYXcIQ59564757 = -197902435;    int KTnuYYXcIQ63746750 = -318903092;    int KTnuYYXcIQ61379089 = -971082272;    int KTnuYYXcIQ10702024 = -855261181;    int KTnuYYXcIQ69577243 = -678130379;    int KTnuYYXcIQ49500771 = 48566221;    int KTnuYYXcIQ31395191 = -994633596;    int KTnuYYXcIQ2409315 = -571706515;    int KTnuYYXcIQ92872540 = -487497601;    int KTnuYYXcIQ77665288 = -809359341;    int KTnuYYXcIQ48838505 = -793101826;    int KTnuYYXcIQ14666365 = -597082722;    int KTnuYYXcIQ56820649 = -851067211;    int KTnuYYXcIQ67234874 = -270660923;    int KTnuYYXcIQ66463223 = -962025683;    int KTnuYYXcIQ5744805 = -372111994;    int KTnuYYXcIQ84285425 = -848505178;    int KTnuYYXcIQ76894818 = -966530789;    int KTnuYYXcIQ73228586 = -78932017;    int KTnuYYXcIQ2522236 = 48068252;    int KTnuYYXcIQ8668650 = -427203089;    int KTnuYYXcIQ6695481 = -808236666;    int KTnuYYXcIQ79194321 = -978815390;    int KTnuYYXcIQ2355464 = -816406295;    int KTnuYYXcIQ97821075 = -497841335;    int KTnuYYXcIQ90844281 = -550460019;    int KTnuYYXcIQ5288448 = -19319649;    int KTnuYYXcIQ97280900 = -294122991;    int KTnuYYXcIQ86210879 = -655055823;    int KTnuYYXcIQ42370954 = -17637161;    int KTnuYYXcIQ57020229 = -166437859;    int KTnuYYXcIQ82944621 = -868805078;    int KTnuYYXcIQ36340645 = -589723011;    int KTnuYYXcIQ3175020 = -400423195;    int KTnuYYXcIQ51706907 = -189746219;    int KTnuYYXcIQ2682433 = -853008644;    int KTnuYYXcIQ61950579 = -70470172;    int KTnuYYXcIQ4006808 = -920976444;    int KTnuYYXcIQ45728572 = -77156048;    int KTnuYYXcIQ43041174 = -214116754;    int KTnuYYXcIQ73665406 = -726339003;    int KTnuYYXcIQ74544354 = -176257669;    int KTnuYYXcIQ72635128 = -842682820;    int KTnuYYXcIQ31270737 = -621723350;    int KTnuYYXcIQ56871183 = -119684247;    int KTnuYYXcIQ51243680 = -103487240;    int KTnuYYXcIQ15563530 = -16736299;    int KTnuYYXcIQ18156851 = -726954626;    int KTnuYYXcIQ76938866 = -301022613;    int KTnuYYXcIQ73055099 = -540770655;    int KTnuYYXcIQ75602820 = -716432549;    int KTnuYYXcIQ31654102 = -391664103;    int KTnuYYXcIQ69123183 = -577599030;    int KTnuYYXcIQ75447956 = -831185807;    int KTnuYYXcIQ51455265 = -572046627;    int KTnuYYXcIQ52869277 = -389665770;    int KTnuYYXcIQ84552428 = -340087703;    int KTnuYYXcIQ59023625 = -54675978;    int KTnuYYXcIQ12880948 = -257419846;    int KTnuYYXcIQ78732962 = -27670360;    int KTnuYYXcIQ44212324 = -932114131;    int KTnuYYXcIQ34114291 = -600510606;    int KTnuYYXcIQ16198435 = -916650693;    int KTnuYYXcIQ50501586 = -369860440;    int KTnuYYXcIQ20645060 = -542921482;    int KTnuYYXcIQ65893884 = -924296748;    int KTnuYYXcIQ78325719 = 92640288;    int KTnuYYXcIQ53645629 = -350644016;    int KTnuYYXcIQ15527967 = 19085295;    int KTnuYYXcIQ63780791 = -9017040;    int KTnuYYXcIQ43794225 = -201641823;    int KTnuYYXcIQ80278618 = -927528735;    int KTnuYYXcIQ31166247 = -789374742;    int KTnuYYXcIQ30187412 = -864815263;    int KTnuYYXcIQ28856829 = -225592746;    int KTnuYYXcIQ34124296 = -150945421;    int KTnuYYXcIQ34060353 = -965553846;    int KTnuYYXcIQ47923585 = -257092040;    int KTnuYYXcIQ45484281 = -596722048;    int KTnuYYXcIQ46577395 = -294354096;    int KTnuYYXcIQ75280751 = -433723721;    int KTnuYYXcIQ87131596 = -292365023;    int KTnuYYXcIQ20342035 = -993100379;    int KTnuYYXcIQ13155780 = -14285168;    int KTnuYYXcIQ66768133 = -301204613;    int KTnuYYXcIQ25366128 = -774773757;    int KTnuYYXcIQ13821438 = -191206049;    int KTnuYYXcIQ60892689 = -758537204;    int KTnuYYXcIQ51719755 = -828376568;    int KTnuYYXcIQ98837629 = -800080449;    int KTnuYYXcIQ18130004 = -412920941;    int KTnuYYXcIQ2926954 = 84205805;    int KTnuYYXcIQ91125859 = -563556598;    int KTnuYYXcIQ66995609 = 50514312;    int KTnuYYXcIQ98828850 = -282002624;    int KTnuYYXcIQ39551116 = -25828397;    int KTnuYYXcIQ58345919 = -259606976;    int KTnuYYXcIQ22133542 = -883117555;     KTnuYYXcIQ77970191 = KTnuYYXcIQ60123914;     KTnuYYXcIQ60123914 = KTnuYYXcIQ59564757;     KTnuYYXcIQ59564757 = KTnuYYXcIQ63746750;     KTnuYYXcIQ63746750 = KTnuYYXcIQ61379089;     KTnuYYXcIQ61379089 = KTnuYYXcIQ10702024;     KTnuYYXcIQ10702024 = KTnuYYXcIQ69577243;     KTnuYYXcIQ69577243 = KTnuYYXcIQ49500771;     KTnuYYXcIQ49500771 = KTnuYYXcIQ31395191;     KTnuYYXcIQ31395191 = KTnuYYXcIQ2409315;     KTnuYYXcIQ2409315 = KTnuYYXcIQ92872540;     KTnuYYXcIQ92872540 = KTnuYYXcIQ77665288;     KTnuYYXcIQ77665288 = KTnuYYXcIQ48838505;     KTnuYYXcIQ48838505 = KTnuYYXcIQ14666365;     KTnuYYXcIQ14666365 = KTnuYYXcIQ56820649;     KTnuYYXcIQ56820649 = KTnuYYXcIQ67234874;     KTnuYYXcIQ67234874 = KTnuYYXcIQ66463223;     KTnuYYXcIQ66463223 = KTnuYYXcIQ5744805;     KTnuYYXcIQ5744805 = KTnuYYXcIQ84285425;     KTnuYYXcIQ84285425 = KTnuYYXcIQ76894818;     KTnuYYXcIQ76894818 = KTnuYYXcIQ73228586;     KTnuYYXcIQ73228586 = KTnuYYXcIQ2522236;     KTnuYYXcIQ2522236 = KTnuYYXcIQ8668650;     KTnuYYXcIQ8668650 = KTnuYYXcIQ6695481;     KTnuYYXcIQ6695481 = KTnuYYXcIQ79194321;     KTnuYYXcIQ79194321 = KTnuYYXcIQ2355464;     KTnuYYXcIQ2355464 = KTnuYYXcIQ97821075;     KTnuYYXcIQ97821075 = KTnuYYXcIQ90844281;     KTnuYYXcIQ90844281 = KTnuYYXcIQ5288448;     KTnuYYXcIQ5288448 = KTnuYYXcIQ97280900;     KTnuYYXcIQ97280900 = KTnuYYXcIQ86210879;     KTnuYYXcIQ86210879 = KTnuYYXcIQ42370954;     KTnuYYXcIQ42370954 = KTnuYYXcIQ57020229;     KTnuYYXcIQ57020229 = KTnuYYXcIQ82944621;     KTnuYYXcIQ82944621 = KTnuYYXcIQ36340645;     KTnuYYXcIQ36340645 = KTnuYYXcIQ3175020;     KTnuYYXcIQ3175020 = KTnuYYXcIQ51706907;     KTnuYYXcIQ51706907 = KTnuYYXcIQ2682433;     KTnuYYXcIQ2682433 = KTnuYYXcIQ61950579;     KTnuYYXcIQ61950579 = KTnuYYXcIQ4006808;     KTnuYYXcIQ4006808 = KTnuYYXcIQ45728572;     KTnuYYXcIQ45728572 = KTnuYYXcIQ43041174;     KTnuYYXcIQ43041174 = KTnuYYXcIQ73665406;     KTnuYYXcIQ73665406 = KTnuYYXcIQ74544354;     KTnuYYXcIQ74544354 = KTnuYYXcIQ72635128;     KTnuYYXcIQ72635128 = KTnuYYXcIQ31270737;     KTnuYYXcIQ31270737 = KTnuYYXcIQ56871183;     KTnuYYXcIQ56871183 = KTnuYYXcIQ51243680;     KTnuYYXcIQ51243680 = KTnuYYXcIQ15563530;     KTnuYYXcIQ15563530 = KTnuYYXcIQ18156851;     KTnuYYXcIQ18156851 = KTnuYYXcIQ76938866;     KTnuYYXcIQ76938866 = KTnuYYXcIQ73055099;     KTnuYYXcIQ73055099 = KTnuYYXcIQ75602820;     KTnuYYXcIQ75602820 = KTnuYYXcIQ31654102;     KTnuYYXcIQ31654102 = KTnuYYXcIQ69123183;     KTnuYYXcIQ69123183 = KTnuYYXcIQ75447956;     KTnuYYXcIQ75447956 = KTnuYYXcIQ51455265;     KTnuYYXcIQ51455265 = KTnuYYXcIQ52869277;     KTnuYYXcIQ52869277 = KTnuYYXcIQ84552428;     KTnuYYXcIQ84552428 = KTnuYYXcIQ59023625;     KTnuYYXcIQ59023625 = KTnuYYXcIQ12880948;     KTnuYYXcIQ12880948 = KTnuYYXcIQ78732962;     KTnuYYXcIQ78732962 = KTnuYYXcIQ44212324;     KTnuYYXcIQ44212324 = KTnuYYXcIQ34114291;     KTnuYYXcIQ34114291 = KTnuYYXcIQ16198435;     KTnuYYXcIQ16198435 = KTnuYYXcIQ50501586;     KTnuYYXcIQ50501586 = KTnuYYXcIQ20645060;     KTnuYYXcIQ20645060 = KTnuYYXcIQ65893884;     KTnuYYXcIQ65893884 = KTnuYYXcIQ78325719;     KTnuYYXcIQ78325719 = KTnuYYXcIQ53645629;     KTnuYYXcIQ53645629 = KTnuYYXcIQ15527967;     KTnuYYXcIQ15527967 = KTnuYYXcIQ63780791;     KTnuYYXcIQ63780791 = KTnuYYXcIQ43794225;     KTnuYYXcIQ43794225 = KTnuYYXcIQ80278618;     KTnuYYXcIQ80278618 = KTnuYYXcIQ31166247;     KTnuYYXcIQ31166247 = KTnuYYXcIQ30187412;     KTnuYYXcIQ30187412 = KTnuYYXcIQ28856829;     KTnuYYXcIQ28856829 = KTnuYYXcIQ34124296;     KTnuYYXcIQ34124296 = KTnuYYXcIQ34060353;     KTnuYYXcIQ34060353 = KTnuYYXcIQ47923585;     KTnuYYXcIQ47923585 = KTnuYYXcIQ45484281;     KTnuYYXcIQ45484281 = KTnuYYXcIQ46577395;     KTnuYYXcIQ46577395 = KTnuYYXcIQ75280751;     KTnuYYXcIQ75280751 = KTnuYYXcIQ87131596;     KTnuYYXcIQ87131596 = KTnuYYXcIQ20342035;     KTnuYYXcIQ20342035 = KTnuYYXcIQ13155780;     KTnuYYXcIQ13155780 = KTnuYYXcIQ66768133;     KTnuYYXcIQ66768133 = KTnuYYXcIQ25366128;     KTnuYYXcIQ25366128 = KTnuYYXcIQ13821438;     KTnuYYXcIQ13821438 = KTnuYYXcIQ60892689;     KTnuYYXcIQ60892689 = KTnuYYXcIQ51719755;     KTnuYYXcIQ51719755 = KTnuYYXcIQ98837629;     KTnuYYXcIQ98837629 = KTnuYYXcIQ18130004;     KTnuYYXcIQ18130004 = KTnuYYXcIQ2926954;     KTnuYYXcIQ2926954 = KTnuYYXcIQ91125859;     KTnuYYXcIQ91125859 = KTnuYYXcIQ66995609;     KTnuYYXcIQ66995609 = KTnuYYXcIQ98828850;     KTnuYYXcIQ98828850 = KTnuYYXcIQ39551116;     KTnuYYXcIQ39551116 = KTnuYYXcIQ58345919;     KTnuYYXcIQ58345919 = KTnuYYXcIQ22133542;     KTnuYYXcIQ22133542 = KTnuYYXcIQ77970191;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JuJGkYqkAr12807642() {     int PjVOZoyySK90756358 = 81841753;    int PjVOZoyySK46894802 = -169583729;    int PjVOZoyySK77317557 = -606261816;    int PjVOZoyySK71687575 = -143105369;    int PjVOZoyySK54890194 = -362898737;    int PjVOZoyySK54012094 = -632184494;    int PjVOZoyySK90785001 = -123802449;    int PjVOZoyySK76219884 = -395952036;    int PjVOZoyySK26800002 = -956391688;    int PjVOZoyySK8628019 = -375794148;    int PjVOZoyySK20359923 = -602183616;    int PjVOZoyySK9615794 = -216692157;    int PjVOZoyySK18981809 = -696884483;    int PjVOZoyySK64290443 = -929497920;    int PjVOZoyySK80127202 = -758053041;    int PjVOZoyySK95659942 = -821552312;    int PjVOZoyySK22215041 = -625800150;    int PjVOZoyySK89303289 = 72677704;    int PjVOZoyySK85446056 = -757974373;    int PjVOZoyySK79549314 = -465774928;    int PjVOZoyySK53402643 = -570248281;    int PjVOZoyySK31570055 = -121790096;    int PjVOZoyySK47428945 = -358206984;    int PjVOZoyySK51202610 = -68581678;    int PjVOZoyySK85088580 = -89257314;    int PjVOZoyySK83674109 = -506837494;    int PjVOZoyySK9236612 = -31547615;    int PjVOZoyySK58197206 = -724258124;    int PjVOZoyySK21905501 = -233536889;    int PjVOZoyySK50032016 = 75990855;    int PjVOZoyySK30646875 = -931472573;    int PjVOZoyySK45689778 = 3643160;    int PjVOZoyySK3066179 = -42292644;    int PjVOZoyySK60020994 = -773559297;    int PjVOZoyySK19401177 = -99754891;    int PjVOZoyySK98399134 = -188986299;    int PjVOZoyySK88470967 = 20344431;    int PjVOZoyySK60386455 = -772628735;    int PjVOZoyySK67725965 = 87679251;    int PjVOZoyySK30271537 = -765393289;    int PjVOZoyySK7159880 = -452630041;    int PjVOZoyySK50743630 = -112428089;    int PjVOZoyySK8548877 = -749772448;    int PjVOZoyySK19172892 = -517429912;    int PjVOZoyySK52643618 = -735452780;    int PjVOZoyySK4898866 = -695041790;    int PjVOZoyySK34453203 = -617199300;    int PjVOZoyySK80683646 = -258840019;    int PjVOZoyySK59627579 = -180338273;    int PjVOZoyySK65107272 = -237991089;    int PjVOZoyySK95266359 = -553906921;    int PjVOZoyySK1231647 = 88044007;    int PjVOZoyySK10087971 = -892435831;    int PjVOZoyySK76353827 = 18884155;    int PjVOZoyySK59404999 = -572043517;    int PjVOZoyySK59186304 = -796368151;    int PjVOZoyySK99465857 = -811376746;    int PjVOZoyySK26114948 = -437680139;    int PjVOZoyySK86598994 = 46151945;    int PjVOZoyySK71216084 = -856061243;    int PjVOZoyySK44775482 = -500636879;    int PjVOZoyySK32587795 = -399544326;    int PjVOZoyySK54314384 = -62415148;    int PjVOZoyySK76767986 = -932382543;    int PjVOZoyySK77981144 = -444321576;    int PjVOZoyySK74670145 = -505826777;    int PjVOZoyySK6549616 = -74399514;    int PjVOZoyySK58960814 = -923325187;    int PjVOZoyySK44889266 = -729743030;    int PjVOZoyySK81728067 = -469066743;    int PjVOZoyySK7188975 = -741896743;    int PjVOZoyySK61828586 = -853171416;    int PjVOZoyySK21577324 = 84998452;    int PjVOZoyySK55174519 = -992581085;    int PjVOZoyySK72389435 = 86855112;    int PjVOZoyySK2659013 = -357820193;    int PjVOZoyySK23021178 = -372017649;    int PjVOZoyySK28256053 = -840777072;    int PjVOZoyySK98558991 = -333128899;    int PjVOZoyySK80189715 = -394215525;    int PjVOZoyySK49220907 = -889638195;    int PjVOZoyySK28552965 = -772707597;    int PjVOZoyySK98569627 = -443919851;    int PjVOZoyySK56798229 = -995545800;    int PjVOZoyySK54765656 = -370102225;    int PjVOZoyySK29415228 = -919516580;    int PjVOZoyySK35601807 = -103921009;    int PjVOZoyySK26712352 = 38823201;    int PjVOZoyySK615995 = -101515781;    int PjVOZoyySK60214873 = -303386741;    int PjVOZoyySK98933277 = -377609553;    int PjVOZoyySK62356020 = -541975431;    int PjVOZoyySK73787460 = -718780681;    int PjVOZoyySK96509880 = -56259506;    int PjVOZoyySK85496054 = -164756411;    int PjVOZoyySK74572084 = 46914284;    int PjVOZoyySK96429246 = 49987059;    int PjVOZoyySK31780891 = -817389905;    int PjVOZoyySK41191748 = 26891664;    int PjVOZoyySK77973473 = 81841753;     PjVOZoyySK90756358 = PjVOZoyySK46894802;     PjVOZoyySK46894802 = PjVOZoyySK77317557;     PjVOZoyySK77317557 = PjVOZoyySK71687575;     PjVOZoyySK71687575 = PjVOZoyySK54890194;     PjVOZoyySK54890194 = PjVOZoyySK54012094;     PjVOZoyySK54012094 = PjVOZoyySK90785001;     PjVOZoyySK90785001 = PjVOZoyySK76219884;     PjVOZoyySK76219884 = PjVOZoyySK26800002;     PjVOZoyySK26800002 = PjVOZoyySK8628019;     PjVOZoyySK8628019 = PjVOZoyySK20359923;     PjVOZoyySK20359923 = PjVOZoyySK9615794;     PjVOZoyySK9615794 = PjVOZoyySK18981809;     PjVOZoyySK18981809 = PjVOZoyySK64290443;     PjVOZoyySK64290443 = PjVOZoyySK80127202;     PjVOZoyySK80127202 = PjVOZoyySK95659942;     PjVOZoyySK95659942 = PjVOZoyySK22215041;     PjVOZoyySK22215041 = PjVOZoyySK89303289;     PjVOZoyySK89303289 = PjVOZoyySK85446056;     PjVOZoyySK85446056 = PjVOZoyySK79549314;     PjVOZoyySK79549314 = PjVOZoyySK53402643;     PjVOZoyySK53402643 = PjVOZoyySK31570055;     PjVOZoyySK31570055 = PjVOZoyySK47428945;     PjVOZoyySK47428945 = PjVOZoyySK51202610;     PjVOZoyySK51202610 = PjVOZoyySK85088580;     PjVOZoyySK85088580 = PjVOZoyySK83674109;     PjVOZoyySK83674109 = PjVOZoyySK9236612;     PjVOZoyySK9236612 = PjVOZoyySK58197206;     PjVOZoyySK58197206 = PjVOZoyySK21905501;     PjVOZoyySK21905501 = PjVOZoyySK50032016;     PjVOZoyySK50032016 = PjVOZoyySK30646875;     PjVOZoyySK30646875 = PjVOZoyySK45689778;     PjVOZoyySK45689778 = PjVOZoyySK3066179;     PjVOZoyySK3066179 = PjVOZoyySK60020994;     PjVOZoyySK60020994 = PjVOZoyySK19401177;     PjVOZoyySK19401177 = PjVOZoyySK98399134;     PjVOZoyySK98399134 = PjVOZoyySK88470967;     PjVOZoyySK88470967 = PjVOZoyySK60386455;     PjVOZoyySK60386455 = PjVOZoyySK67725965;     PjVOZoyySK67725965 = PjVOZoyySK30271537;     PjVOZoyySK30271537 = PjVOZoyySK7159880;     PjVOZoyySK7159880 = PjVOZoyySK50743630;     PjVOZoyySK50743630 = PjVOZoyySK8548877;     PjVOZoyySK8548877 = PjVOZoyySK19172892;     PjVOZoyySK19172892 = PjVOZoyySK52643618;     PjVOZoyySK52643618 = PjVOZoyySK4898866;     PjVOZoyySK4898866 = PjVOZoyySK34453203;     PjVOZoyySK34453203 = PjVOZoyySK80683646;     PjVOZoyySK80683646 = PjVOZoyySK59627579;     PjVOZoyySK59627579 = PjVOZoyySK65107272;     PjVOZoyySK65107272 = PjVOZoyySK95266359;     PjVOZoyySK95266359 = PjVOZoyySK1231647;     PjVOZoyySK1231647 = PjVOZoyySK10087971;     PjVOZoyySK10087971 = PjVOZoyySK76353827;     PjVOZoyySK76353827 = PjVOZoyySK59404999;     PjVOZoyySK59404999 = PjVOZoyySK59186304;     PjVOZoyySK59186304 = PjVOZoyySK99465857;     PjVOZoyySK99465857 = PjVOZoyySK26114948;     PjVOZoyySK26114948 = PjVOZoyySK86598994;     PjVOZoyySK86598994 = PjVOZoyySK71216084;     PjVOZoyySK71216084 = PjVOZoyySK44775482;     PjVOZoyySK44775482 = PjVOZoyySK32587795;     PjVOZoyySK32587795 = PjVOZoyySK54314384;     PjVOZoyySK54314384 = PjVOZoyySK76767986;     PjVOZoyySK76767986 = PjVOZoyySK77981144;     PjVOZoyySK77981144 = PjVOZoyySK74670145;     PjVOZoyySK74670145 = PjVOZoyySK6549616;     PjVOZoyySK6549616 = PjVOZoyySK58960814;     PjVOZoyySK58960814 = PjVOZoyySK44889266;     PjVOZoyySK44889266 = PjVOZoyySK81728067;     PjVOZoyySK81728067 = PjVOZoyySK7188975;     PjVOZoyySK7188975 = PjVOZoyySK61828586;     PjVOZoyySK61828586 = PjVOZoyySK21577324;     PjVOZoyySK21577324 = PjVOZoyySK55174519;     PjVOZoyySK55174519 = PjVOZoyySK72389435;     PjVOZoyySK72389435 = PjVOZoyySK2659013;     PjVOZoyySK2659013 = PjVOZoyySK23021178;     PjVOZoyySK23021178 = PjVOZoyySK28256053;     PjVOZoyySK28256053 = PjVOZoyySK98558991;     PjVOZoyySK98558991 = PjVOZoyySK80189715;     PjVOZoyySK80189715 = PjVOZoyySK49220907;     PjVOZoyySK49220907 = PjVOZoyySK28552965;     PjVOZoyySK28552965 = PjVOZoyySK98569627;     PjVOZoyySK98569627 = PjVOZoyySK56798229;     PjVOZoyySK56798229 = PjVOZoyySK54765656;     PjVOZoyySK54765656 = PjVOZoyySK29415228;     PjVOZoyySK29415228 = PjVOZoyySK35601807;     PjVOZoyySK35601807 = PjVOZoyySK26712352;     PjVOZoyySK26712352 = PjVOZoyySK615995;     PjVOZoyySK615995 = PjVOZoyySK60214873;     PjVOZoyySK60214873 = PjVOZoyySK98933277;     PjVOZoyySK98933277 = PjVOZoyySK62356020;     PjVOZoyySK62356020 = PjVOZoyySK73787460;     PjVOZoyySK73787460 = PjVOZoyySK96509880;     PjVOZoyySK96509880 = PjVOZoyySK85496054;     PjVOZoyySK85496054 = PjVOZoyySK74572084;     PjVOZoyySK74572084 = PjVOZoyySK96429246;     PjVOZoyySK96429246 = PjVOZoyySK31780891;     PjVOZoyySK31780891 = PjVOZoyySK41191748;     PjVOZoyySK41191748 = PjVOZoyySK77973473;     PjVOZoyySK77973473 = PjVOZoyySK90756358;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LwOihCorAm31020984() {     int OMPnagILyt74200663 = -798823927;    int OMPnagILyt63766891 = -168965917;    int OMPnagILyt73726064 = -399787044;    int OMPnagILyt34479827 = -336905205;    int OMPnagILyt61011518 = -169766228;    int OMPnagILyt52516495 = -411256147;    int OMPnagILyt61824319 = -963713763;    int OMPnagILyt90567326 = -383110065;    int OMPnagILyt85329702 = -197851039;    int OMPnagILyt41173671 = -844170260;    int OMPnagILyt69775860 = -792299489;    int OMPnagILyt72821193 = -204310403;    int OMPnagILyt37099584 = -488545724;    int OMPnagILyt43917335 = -717788277;    int OMPnagILyt40981666 = -852076023;    int OMPnagILyt82049104 = -609631637;    int OMPnagILyt15966926 = -379145177;    int OMPnagILyt70154735 = -174627173;    int OMPnagILyt47886198 = -756649244;    int OMPnagILyt68916757 = -995360921;    int OMPnagILyt9043030 = -159777312;    int OMPnagILyt43717373 = -112941867;    int OMPnagILyt51254653 = -398096181;    int OMPnagILyt85909168 = -140407074;    int OMPnagILyt26664539 = -648910073;    int OMPnagILyt79016258 = -387347210;    int OMPnagILyt58691352 = -759631958;    int OMPnagILyt97928831 = -794213008;    int OMPnagILyt57986594 = -96042031;    int OMPnagILyt65865357 = -436858997;    int OMPnagILyt48742189 = -569229773;    int OMPnagILyt8796205 = 88448080;    int OMPnagILyt46605065 = -763164821;    int OMPnagILyt2179505 = -365161467;    int OMPnagILyt98127907 = -297419627;    int OMPnagILyt10788319 = 22989795;    int OMPnagILyt57085146 = -958504749;    int OMPnagILyt965287 = -481099255;    int OMPnagILyt16721044 = -97218547;    int OMPnagILyt79766672 = -517986812;    int OMPnagILyt40808804 = -24179014;    int OMPnagILyt1231417 = -686336410;    int OMPnagILyt32115946 = -200829690;    int OMPnagILyt11077832 = -273612683;    int OMPnagILyt44588115 = -463609733;    int OMPnagILyt44504862 = -714652714;    int OMPnagILyt10468633 = -749503529;    int OMPnagILyt354242 = -257561738;    int OMPnagILyt90474691 = -765940356;    int OMPnagILyt439756 = -445011544;    int OMPnagILyt68523807 = -774808343;    int OMPnagILyt56732441 = 29784696;    int OMPnagILyt24200430 = -4183148;    int OMPnagILyt1030586 = -799529232;    int OMPnagILyt21977954 = -920529639;    int OMPnagILyt30483291 = -585882061;    int OMPnagILyt12512239 = -770869737;    int OMPnagILyt87816896 = -159379971;    int OMPnagILyt7815289 = -687995132;    int OMPnagILyt81995259 = -782419019;    int OMPnagILyt93825142 = -651624190;    int OMPnagILyt63895488 = -69500755;    int OMPnagILyt32580732 = -187068035;    int OMPnagILyt19464346 = -760992043;    int OMPnagILyt92431482 = -174940487;    int OMPnagILyt60979656 = -780747569;    int OMPnagILyt26216128 = -441145583;    int OMPnagILyt34920080 = -23384258;    int OMPnagILyt45789428 = -320368651;    int OMPnagILyt30193347 = -775065819;    int OMPnagILyt24963958 = -651126889;    int OMPnagILyt15001639 = -898045923;    int OMPnagILyt53433692 = 22591373;    int OMPnagILyt68119526 = -138662433;    int OMPnagILyt28107953 = -871181907;    int OMPnagILyt7811614 = -473440902;    int OMPnagILyt11601427 = -912112177;    int OMPnagILyt40176821 = -24483499;    int OMPnagILyt41321053 = -676797341;    int OMPnagILyt82159677 = -934257360;    int OMPnagILyt68547625 = -637843681;    int OMPnagILyt58337110 = -402070221;    int OMPnagILyt7454141 = 71727348;    int OMPnagILyt57546839 = -651030487;    int OMPnagILyt97341549 = -662050654;    int OMPnagILyt92009747 = -499014470;    int OMPnagILyt84595775 = -907368773;    int OMPnagILyt45574479 = -963635589;    int OMPnagILyt80201550 = -444631828;    int OMPnagILyt67644616 = -711537566;    int OMPnagILyt98276080 = -206140469;    int OMPnagILyt69268250 = -699124779;    int OMPnagILyt93149998 = -793104124;    int OMPnagILyt34725784 = -314799528;    int OMPnagILyt85941529 = -866362622;    int OMPnagILyt76913316 = -954678260;    int OMPnagILyt68650684 = -399268376;    int OMPnagILyt12651601 = -439837648;    int OMPnagILyt18646350 = 1327804;    int OMPnagILyt83608459 = -798823927;     OMPnagILyt74200663 = OMPnagILyt63766891;     OMPnagILyt63766891 = OMPnagILyt73726064;     OMPnagILyt73726064 = OMPnagILyt34479827;     OMPnagILyt34479827 = OMPnagILyt61011518;     OMPnagILyt61011518 = OMPnagILyt52516495;     OMPnagILyt52516495 = OMPnagILyt61824319;     OMPnagILyt61824319 = OMPnagILyt90567326;     OMPnagILyt90567326 = OMPnagILyt85329702;     OMPnagILyt85329702 = OMPnagILyt41173671;     OMPnagILyt41173671 = OMPnagILyt69775860;     OMPnagILyt69775860 = OMPnagILyt72821193;     OMPnagILyt72821193 = OMPnagILyt37099584;     OMPnagILyt37099584 = OMPnagILyt43917335;     OMPnagILyt43917335 = OMPnagILyt40981666;     OMPnagILyt40981666 = OMPnagILyt82049104;     OMPnagILyt82049104 = OMPnagILyt15966926;     OMPnagILyt15966926 = OMPnagILyt70154735;     OMPnagILyt70154735 = OMPnagILyt47886198;     OMPnagILyt47886198 = OMPnagILyt68916757;     OMPnagILyt68916757 = OMPnagILyt9043030;     OMPnagILyt9043030 = OMPnagILyt43717373;     OMPnagILyt43717373 = OMPnagILyt51254653;     OMPnagILyt51254653 = OMPnagILyt85909168;     OMPnagILyt85909168 = OMPnagILyt26664539;     OMPnagILyt26664539 = OMPnagILyt79016258;     OMPnagILyt79016258 = OMPnagILyt58691352;     OMPnagILyt58691352 = OMPnagILyt97928831;     OMPnagILyt97928831 = OMPnagILyt57986594;     OMPnagILyt57986594 = OMPnagILyt65865357;     OMPnagILyt65865357 = OMPnagILyt48742189;     OMPnagILyt48742189 = OMPnagILyt8796205;     OMPnagILyt8796205 = OMPnagILyt46605065;     OMPnagILyt46605065 = OMPnagILyt2179505;     OMPnagILyt2179505 = OMPnagILyt98127907;     OMPnagILyt98127907 = OMPnagILyt10788319;     OMPnagILyt10788319 = OMPnagILyt57085146;     OMPnagILyt57085146 = OMPnagILyt965287;     OMPnagILyt965287 = OMPnagILyt16721044;     OMPnagILyt16721044 = OMPnagILyt79766672;     OMPnagILyt79766672 = OMPnagILyt40808804;     OMPnagILyt40808804 = OMPnagILyt1231417;     OMPnagILyt1231417 = OMPnagILyt32115946;     OMPnagILyt32115946 = OMPnagILyt11077832;     OMPnagILyt11077832 = OMPnagILyt44588115;     OMPnagILyt44588115 = OMPnagILyt44504862;     OMPnagILyt44504862 = OMPnagILyt10468633;     OMPnagILyt10468633 = OMPnagILyt354242;     OMPnagILyt354242 = OMPnagILyt90474691;     OMPnagILyt90474691 = OMPnagILyt439756;     OMPnagILyt439756 = OMPnagILyt68523807;     OMPnagILyt68523807 = OMPnagILyt56732441;     OMPnagILyt56732441 = OMPnagILyt24200430;     OMPnagILyt24200430 = OMPnagILyt1030586;     OMPnagILyt1030586 = OMPnagILyt21977954;     OMPnagILyt21977954 = OMPnagILyt30483291;     OMPnagILyt30483291 = OMPnagILyt12512239;     OMPnagILyt12512239 = OMPnagILyt87816896;     OMPnagILyt87816896 = OMPnagILyt7815289;     OMPnagILyt7815289 = OMPnagILyt81995259;     OMPnagILyt81995259 = OMPnagILyt93825142;     OMPnagILyt93825142 = OMPnagILyt63895488;     OMPnagILyt63895488 = OMPnagILyt32580732;     OMPnagILyt32580732 = OMPnagILyt19464346;     OMPnagILyt19464346 = OMPnagILyt92431482;     OMPnagILyt92431482 = OMPnagILyt60979656;     OMPnagILyt60979656 = OMPnagILyt26216128;     OMPnagILyt26216128 = OMPnagILyt34920080;     OMPnagILyt34920080 = OMPnagILyt45789428;     OMPnagILyt45789428 = OMPnagILyt30193347;     OMPnagILyt30193347 = OMPnagILyt24963958;     OMPnagILyt24963958 = OMPnagILyt15001639;     OMPnagILyt15001639 = OMPnagILyt53433692;     OMPnagILyt53433692 = OMPnagILyt68119526;     OMPnagILyt68119526 = OMPnagILyt28107953;     OMPnagILyt28107953 = OMPnagILyt7811614;     OMPnagILyt7811614 = OMPnagILyt11601427;     OMPnagILyt11601427 = OMPnagILyt40176821;     OMPnagILyt40176821 = OMPnagILyt41321053;     OMPnagILyt41321053 = OMPnagILyt82159677;     OMPnagILyt82159677 = OMPnagILyt68547625;     OMPnagILyt68547625 = OMPnagILyt58337110;     OMPnagILyt58337110 = OMPnagILyt7454141;     OMPnagILyt7454141 = OMPnagILyt57546839;     OMPnagILyt57546839 = OMPnagILyt97341549;     OMPnagILyt97341549 = OMPnagILyt92009747;     OMPnagILyt92009747 = OMPnagILyt84595775;     OMPnagILyt84595775 = OMPnagILyt45574479;     OMPnagILyt45574479 = OMPnagILyt80201550;     OMPnagILyt80201550 = OMPnagILyt67644616;     OMPnagILyt67644616 = OMPnagILyt98276080;     OMPnagILyt98276080 = OMPnagILyt69268250;     OMPnagILyt69268250 = OMPnagILyt93149998;     OMPnagILyt93149998 = OMPnagILyt34725784;     OMPnagILyt34725784 = OMPnagILyt85941529;     OMPnagILyt85941529 = OMPnagILyt76913316;     OMPnagILyt76913316 = OMPnagILyt68650684;     OMPnagILyt68650684 = OMPnagILyt12651601;     OMPnagILyt12651601 = OMPnagILyt18646350;     OMPnagILyt18646350 = OMPnagILyt83608459;     OMPnagILyt83608459 = OMPnagILyt74200663;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jWUfIRMtFi1476857() {     int eflXxhHABl86986830 = -933864619;    int eflXxhHABl50537779 = -339299931;    int eflXxhHABl91478864 = -808146426;    int eflXxhHABl42420652 = -161107481;    int eflXxhHABl54522622 = -661582693;    int eflXxhHABl95826565 = -188179460;    int eflXxhHABl83032077 = -409385833;    int eflXxhHABl17286440 = -827628322;    int eflXxhHABl80734512 = -159609132;    int eflXxhHABl47392376 = -648257893;    int eflXxhHABl97263242 = -906985504;    int eflXxhHABl4771698 = -711643219;    int eflXxhHABl7242887 = -392328381;    int eflXxhHABl93541413 = 49796524;    int eflXxhHABl64288218 = -759061854;    int eflXxhHABl10474173 = -60523026;    int eflXxhHABl71718743 = -42919644;    int eflXxhHABl53713221 = -829837475;    int eflXxhHABl49046829 = -666118440;    int eflXxhHABl71571253 = -494605060;    int eflXxhHABl89217086 = -651093576;    int eflXxhHABl72765191 = -282800215;    int eflXxhHABl90014948 = -329100076;    int eflXxhHABl30416298 = -500752086;    int eflXxhHABl32558798 = -859351997;    int eflXxhHABl60334903 = -77778409;    int eflXxhHABl70106888 = -293338238;    int eflXxhHABl65281757 = -968011113;    int eflXxhHABl74603647 = -310259271;    int eflXxhHABl18616473 = -66745152;    int eflXxhHABl93178184 = -845646523;    int eflXxhHABl12115029 = -990271598;    int eflXxhHABl92651014 = -639019605;    int eflXxhHABl79255877 = -269915685;    int eflXxhHABl81188439 = -907451507;    int eflXxhHABl6012434 = -865573309;    int eflXxhHABl93849206 = -748414099;    int eflXxhHABl58669310 = -400719347;    int eflXxhHABl22496430 = 60930876;    int eflXxhHABl6031402 = -362403657;    int eflXxhHABl2240113 = -399653008;    int eflXxhHABl8933873 = -584647744;    int eflXxhHABl66999416 = -224263135;    int eflXxhHABl55706370 = -614784926;    int eflXxhHABl24596606 = -356379693;    int eflXxhHABl18132991 = -787971153;    int eflXxhHABl88050652 = -147018582;    int eflXxhHABl29794209 = -412914517;    int eflXxhHABl34538741 = -929542331;    int eflXxhHABl47390176 = 43951993;    int eflXxhHABl86851301 = 72307348;    int eflXxhHABl84908988 = -441400642;    int eflXxhHABl58685579 = -180186430;    int eflXxhHABl45730311 = -388980974;    int eflXxhHABl12259770 = -914974126;    int eflXxhHABl14221639 = -551064405;    int eflXxhHABl60522831 = 89800145;    int eflXxhHABl61062567 = -207394340;    int eflXxhHABl9861855 = -301755485;    int eflXxhHABl94187718 = -483804285;    int eflXxhHABl25719677 = -894841223;    int eflXxhHABl17750321 = -441374721;    int eflXxhHABl42682792 = -417369052;    int eflXxhHABl62118040 = 7136019;    int eflXxhHABl54214191 = -802611370;    int eflXxhHABl85148214 = -916713906;    int eflXxhHABl12120684 = 27376386;    int eflXxhHABl27987010 = -22412696;    int eflXxhHABl12352975 = -42751969;    int eflXxhHABl58275785 = -893488546;    int eflXxhHABl16624967 = -312108927;    int eflXxhHABl13049434 = -642200298;    int eflXxhHABl31216791 = -790768352;    int eflXxhHABl43015427 = -203714783;    int eflXxhHABl69331141 = 5047947;    int eflXxhHABl80283214 = 33554168;    int eflXxhHABl5765776 = 41462920;    int eflXxhHABl34308579 = -714315151;    int eflXxhHABl5819693 = -44372393;    int eflXxhHABl14425807 = 28619155;    int eflXxhHABl72284251 = -930759828;    int eflXxhHABl40312680 = -880423721;    int eflXxhHABl30743016 = 61531218;    int eflXxhHABl27213472 = -254211264;    int eflXxhHABl31765171 = -39052500;    int eflXxhHABl8269197 = -304245882;    int eflXxhHABl53429449 = -710085168;    int eflXxhHABl46920703 = -150038632;    int eflXxhHABl66996107 = -354941560;    int eflXxhHABl66966800 = -256387102;    int eflXxhHABl45489603 = -855373454;    int eflXxhHABl32786640 = -441019760;    int eflXxhHABl48807455 = 1036137;    int eflXxhHABl28308711 = -455264840;    int eflXxhHABl80311724 = -467562435;    int eflXxhHABl84489791 = -958278287;    int eflXxhHABl66251080 = -67278693;    int eflXxhHABl4881376 = -131399155;    int eflXxhHABl1492179 = -812173556;    int eflXxhHABl39448391 = -933864619;     eflXxhHABl86986830 = eflXxhHABl50537779;     eflXxhHABl50537779 = eflXxhHABl91478864;     eflXxhHABl91478864 = eflXxhHABl42420652;     eflXxhHABl42420652 = eflXxhHABl54522622;     eflXxhHABl54522622 = eflXxhHABl95826565;     eflXxhHABl95826565 = eflXxhHABl83032077;     eflXxhHABl83032077 = eflXxhHABl17286440;     eflXxhHABl17286440 = eflXxhHABl80734512;     eflXxhHABl80734512 = eflXxhHABl47392376;     eflXxhHABl47392376 = eflXxhHABl97263242;     eflXxhHABl97263242 = eflXxhHABl4771698;     eflXxhHABl4771698 = eflXxhHABl7242887;     eflXxhHABl7242887 = eflXxhHABl93541413;     eflXxhHABl93541413 = eflXxhHABl64288218;     eflXxhHABl64288218 = eflXxhHABl10474173;     eflXxhHABl10474173 = eflXxhHABl71718743;     eflXxhHABl71718743 = eflXxhHABl53713221;     eflXxhHABl53713221 = eflXxhHABl49046829;     eflXxhHABl49046829 = eflXxhHABl71571253;     eflXxhHABl71571253 = eflXxhHABl89217086;     eflXxhHABl89217086 = eflXxhHABl72765191;     eflXxhHABl72765191 = eflXxhHABl90014948;     eflXxhHABl90014948 = eflXxhHABl30416298;     eflXxhHABl30416298 = eflXxhHABl32558798;     eflXxhHABl32558798 = eflXxhHABl60334903;     eflXxhHABl60334903 = eflXxhHABl70106888;     eflXxhHABl70106888 = eflXxhHABl65281757;     eflXxhHABl65281757 = eflXxhHABl74603647;     eflXxhHABl74603647 = eflXxhHABl18616473;     eflXxhHABl18616473 = eflXxhHABl93178184;     eflXxhHABl93178184 = eflXxhHABl12115029;     eflXxhHABl12115029 = eflXxhHABl92651014;     eflXxhHABl92651014 = eflXxhHABl79255877;     eflXxhHABl79255877 = eflXxhHABl81188439;     eflXxhHABl81188439 = eflXxhHABl6012434;     eflXxhHABl6012434 = eflXxhHABl93849206;     eflXxhHABl93849206 = eflXxhHABl58669310;     eflXxhHABl58669310 = eflXxhHABl22496430;     eflXxhHABl22496430 = eflXxhHABl6031402;     eflXxhHABl6031402 = eflXxhHABl2240113;     eflXxhHABl2240113 = eflXxhHABl8933873;     eflXxhHABl8933873 = eflXxhHABl66999416;     eflXxhHABl66999416 = eflXxhHABl55706370;     eflXxhHABl55706370 = eflXxhHABl24596606;     eflXxhHABl24596606 = eflXxhHABl18132991;     eflXxhHABl18132991 = eflXxhHABl88050652;     eflXxhHABl88050652 = eflXxhHABl29794209;     eflXxhHABl29794209 = eflXxhHABl34538741;     eflXxhHABl34538741 = eflXxhHABl47390176;     eflXxhHABl47390176 = eflXxhHABl86851301;     eflXxhHABl86851301 = eflXxhHABl84908988;     eflXxhHABl84908988 = eflXxhHABl58685579;     eflXxhHABl58685579 = eflXxhHABl45730311;     eflXxhHABl45730311 = eflXxhHABl12259770;     eflXxhHABl12259770 = eflXxhHABl14221639;     eflXxhHABl14221639 = eflXxhHABl60522831;     eflXxhHABl60522831 = eflXxhHABl61062567;     eflXxhHABl61062567 = eflXxhHABl9861855;     eflXxhHABl9861855 = eflXxhHABl94187718;     eflXxhHABl94187718 = eflXxhHABl25719677;     eflXxhHABl25719677 = eflXxhHABl17750321;     eflXxhHABl17750321 = eflXxhHABl42682792;     eflXxhHABl42682792 = eflXxhHABl62118040;     eflXxhHABl62118040 = eflXxhHABl54214191;     eflXxhHABl54214191 = eflXxhHABl85148214;     eflXxhHABl85148214 = eflXxhHABl12120684;     eflXxhHABl12120684 = eflXxhHABl27987010;     eflXxhHABl27987010 = eflXxhHABl12352975;     eflXxhHABl12352975 = eflXxhHABl58275785;     eflXxhHABl58275785 = eflXxhHABl16624967;     eflXxhHABl16624967 = eflXxhHABl13049434;     eflXxhHABl13049434 = eflXxhHABl31216791;     eflXxhHABl31216791 = eflXxhHABl43015427;     eflXxhHABl43015427 = eflXxhHABl69331141;     eflXxhHABl69331141 = eflXxhHABl80283214;     eflXxhHABl80283214 = eflXxhHABl5765776;     eflXxhHABl5765776 = eflXxhHABl34308579;     eflXxhHABl34308579 = eflXxhHABl5819693;     eflXxhHABl5819693 = eflXxhHABl14425807;     eflXxhHABl14425807 = eflXxhHABl72284251;     eflXxhHABl72284251 = eflXxhHABl40312680;     eflXxhHABl40312680 = eflXxhHABl30743016;     eflXxhHABl30743016 = eflXxhHABl27213472;     eflXxhHABl27213472 = eflXxhHABl31765171;     eflXxhHABl31765171 = eflXxhHABl8269197;     eflXxhHABl8269197 = eflXxhHABl53429449;     eflXxhHABl53429449 = eflXxhHABl46920703;     eflXxhHABl46920703 = eflXxhHABl66996107;     eflXxhHABl66996107 = eflXxhHABl66966800;     eflXxhHABl66966800 = eflXxhHABl45489603;     eflXxhHABl45489603 = eflXxhHABl32786640;     eflXxhHABl32786640 = eflXxhHABl48807455;     eflXxhHABl48807455 = eflXxhHABl28308711;     eflXxhHABl28308711 = eflXxhHABl80311724;     eflXxhHABl80311724 = eflXxhHABl84489791;     eflXxhHABl84489791 = eflXxhHABl66251080;     eflXxhHABl66251080 = eflXxhHABl4881376;     eflXxhHABl4881376 = eflXxhHABl1492179;     eflXxhHABl1492179 = eflXxhHABl39448391;     eflXxhHABl39448391 = eflXxhHABl86986830;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xXAdBkCnhT19690198() {     int RjHyXiGRCI70431136 = -714530300;    int RjHyXiGRCI67409868 = -338682118;    int RjHyXiGRCI87887371 = -601671654;    int RjHyXiGRCI5212904 = -354907317;    int RjHyXiGRCI60643946 = -468450184;    int RjHyXiGRCI94330966 = 32748887;    int RjHyXiGRCI54071395 = -149297147;    int RjHyXiGRCI31633882 = -814786352;    int RjHyXiGRCI39264213 = -501068483;    int RjHyXiGRCI79938027 = -16634005;    int RjHyXiGRCI46679180 = 2898623;    int RjHyXiGRCI67977097 = -699261466;    int RjHyXiGRCI25360663 = -183989622;    int RjHyXiGRCI73168306 = -838493833;    int RjHyXiGRCI25142682 = -853084836;    int RjHyXiGRCI96863334 = -948602352;    int RjHyXiGRCI65470628 = -896264671;    int RjHyXiGRCI34564667 = 22857649;    int RjHyXiGRCI11486971 = -664793310;    int RjHyXiGRCI60938696 = 75808948;    int RjHyXiGRCI44857474 = -240622607;    int RjHyXiGRCI84912510 = -273951985;    int RjHyXiGRCI93840655 = -368989273;    int RjHyXiGRCI65122856 = -572577482;    int RjHyXiGRCI74134756 = -319004756;    int RjHyXiGRCI55677052 = 41711876;    int RjHyXiGRCI19561629 = 78577420;    int RjHyXiGRCI5013383 = 62034004;    int RjHyXiGRCI10684742 = -172764413;    int RjHyXiGRCI34449814 = -579595003;    int RjHyXiGRCI11273499 = -483403724;    int RjHyXiGRCI75221455 = -905466678;    int RjHyXiGRCI36189901 = -259891783;    int RjHyXiGRCI21414388 = -961517855;    int RjHyXiGRCI59915170 = -5116242;    int RjHyXiGRCI18401618 = -653597215;    int RjHyXiGRCI62463385 = -627263279;    int RjHyXiGRCI99248141 = -109189867;    int RjHyXiGRCI71491507 = -123966921;    int RjHyXiGRCI55526537 = -114997180;    int RjHyXiGRCI35889037 = 28798019;    int RjHyXiGRCI59421658 = -58556066;    int RjHyXiGRCI90566485 = -775320378;    int RjHyXiGRCI47611310 = -370967697;    int RjHyXiGRCI16541102 = -84536647;    int RjHyXiGRCI57738987 = -807582077;    int RjHyXiGRCI64066082 = -279322811;    int RjHyXiGRCI49464804 = -411636236;    int RjHyXiGRCI65385853 = -415144413;    int RjHyXiGRCI82722660 = -163068463;    int RjHyXiGRCI60108749 = -148594074;    int RjHyXiGRCI40409783 = -499659954;    int RjHyXiGRCI72798038 = -391933747;    int RjHyXiGRCI70407070 = -107394362;    int RjHyXiGRCI74832724 = -163460249;    int RjHyXiGRCI85518626 = -340578316;    int RjHyXiGRCI73569212 = -969692846;    int RjHyXiGRCI22764516 = 70905828;    int RjHyXiGRCI31078148 = 64097438;    int RjHyXiGRCI4966894 = -410162060;    int RjHyXiGRCI74769338 = 54171466;    int RjHyXiGRCI49058013 = -111331151;    int RjHyXiGRCI20949141 = -542021939;    int RjHyXiGRCI4814400 = -921473480;    int RjHyXiGRCI68664529 = -533230282;    int RjHyXiGRCI71457725 = -91634699;    int RjHyXiGRCI31787196 = -339369683;    int RjHyXiGRCI3946276 = -222471767;    int RjHyXiGRCI13253136 = -733377591;    int RjHyXiGRCI6741064 = -99487622;    int RjHyXiGRCI34399950 = -221339073;    int RjHyXiGRCI66222486 = -687074805;    int RjHyXiGRCI63073159 = -853175430;    int RjHyXiGRCI55960434 = -449796131;    int RjHyXiGRCI25049660 = -952989072;    int RjHyXiGRCI85435815 = -82066542;    int RjHyXiGRCI94346024 = -498631607;    int RjHyXiGRCI46229346 = -998021577;    int RjHyXiGRCI48581754 = -388040836;    int RjHyXiGRCI16395769 = -511422680;    int RjHyXiGRCI91610969 = -678965314;    int RjHyXiGRCI70096824 = -509786345;    int RjHyXiGRCI39627529 = -522821584;    int RjHyXiGRCI27962082 = 90304049;    int RjHyXiGRCI74341064 = -331000930;    int RjHyXiGRCI70863716 = -983743771;    int RjHyXiGRCI2423417 = -413532932;    int RjHyXiGRCI65782831 = -52497422;    int RjHyXiGRCI46581664 = -698057607;    int RjHyXiGRCI74396543 = -664537927;    int RjHyXiGRCI44832405 = -683904370;    int RjHyXiGRCI39698870 = -598169108;    int RjHyXiGRCI68169994 = -73287306;    int RjHyXiGRCI66524614 = -713804862;    int RjHyXiGRCI80757198 = -69168647;    int RjHyXiGRCI86831023 = -859870831;    int RjHyXiGRCI38472518 = -516534128;    int RjHyXiGRCI85752085 = -853846899;    int RjHyXiGRCI78946780 = -837737416;    int RjHyXiGRCI45083377 = -714530300;     RjHyXiGRCI70431136 = RjHyXiGRCI67409868;     RjHyXiGRCI67409868 = RjHyXiGRCI87887371;     RjHyXiGRCI87887371 = RjHyXiGRCI5212904;     RjHyXiGRCI5212904 = RjHyXiGRCI60643946;     RjHyXiGRCI60643946 = RjHyXiGRCI94330966;     RjHyXiGRCI94330966 = RjHyXiGRCI54071395;     RjHyXiGRCI54071395 = RjHyXiGRCI31633882;     RjHyXiGRCI31633882 = RjHyXiGRCI39264213;     RjHyXiGRCI39264213 = RjHyXiGRCI79938027;     RjHyXiGRCI79938027 = RjHyXiGRCI46679180;     RjHyXiGRCI46679180 = RjHyXiGRCI67977097;     RjHyXiGRCI67977097 = RjHyXiGRCI25360663;     RjHyXiGRCI25360663 = RjHyXiGRCI73168306;     RjHyXiGRCI73168306 = RjHyXiGRCI25142682;     RjHyXiGRCI25142682 = RjHyXiGRCI96863334;     RjHyXiGRCI96863334 = RjHyXiGRCI65470628;     RjHyXiGRCI65470628 = RjHyXiGRCI34564667;     RjHyXiGRCI34564667 = RjHyXiGRCI11486971;     RjHyXiGRCI11486971 = RjHyXiGRCI60938696;     RjHyXiGRCI60938696 = RjHyXiGRCI44857474;     RjHyXiGRCI44857474 = RjHyXiGRCI84912510;     RjHyXiGRCI84912510 = RjHyXiGRCI93840655;     RjHyXiGRCI93840655 = RjHyXiGRCI65122856;     RjHyXiGRCI65122856 = RjHyXiGRCI74134756;     RjHyXiGRCI74134756 = RjHyXiGRCI55677052;     RjHyXiGRCI55677052 = RjHyXiGRCI19561629;     RjHyXiGRCI19561629 = RjHyXiGRCI5013383;     RjHyXiGRCI5013383 = RjHyXiGRCI10684742;     RjHyXiGRCI10684742 = RjHyXiGRCI34449814;     RjHyXiGRCI34449814 = RjHyXiGRCI11273499;     RjHyXiGRCI11273499 = RjHyXiGRCI75221455;     RjHyXiGRCI75221455 = RjHyXiGRCI36189901;     RjHyXiGRCI36189901 = RjHyXiGRCI21414388;     RjHyXiGRCI21414388 = RjHyXiGRCI59915170;     RjHyXiGRCI59915170 = RjHyXiGRCI18401618;     RjHyXiGRCI18401618 = RjHyXiGRCI62463385;     RjHyXiGRCI62463385 = RjHyXiGRCI99248141;     RjHyXiGRCI99248141 = RjHyXiGRCI71491507;     RjHyXiGRCI71491507 = RjHyXiGRCI55526537;     RjHyXiGRCI55526537 = RjHyXiGRCI35889037;     RjHyXiGRCI35889037 = RjHyXiGRCI59421658;     RjHyXiGRCI59421658 = RjHyXiGRCI90566485;     RjHyXiGRCI90566485 = RjHyXiGRCI47611310;     RjHyXiGRCI47611310 = RjHyXiGRCI16541102;     RjHyXiGRCI16541102 = RjHyXiGRCI57738987;     RjHyXiGRCI57738987 = RjHyXiGRCI64066082;     RjHyXiGRCI64066082 = RjHyXiGRCI49464804;     RjHyXiGRCI49464804 = RjHyXiGRCI65385853;     RjHyXiGRCI65385853 = RjHyXiGRCI82722660;     RjHyXiGRCI82722660 = RjHyXiGRCI60108749;     RjHyXiGRCI60108749 = RjHyXiGRCI40409783;     RjHyXiGRCI40409783 = RjHyXiGRCI72798038;     RjHyXiGRCI72798038 = RjHyXiGRCI70407070;     RjHyXiGRCI70407070 = RjHyXiGRCI74832724;     RjHyXiGRCI74832724 = RjHyXiGRCI85518626;     RjHyXiGRCI85518626 = RjHyXiGRCI73569212;     RjHyXiGRCI73569212 = RjHyXiGRCI22764516;     RjHyXiGRCI22764516 = RjHyXiGRCI31078148;     RjHyXiGRCI31078148 = RjHyXiGRCI4966894;     RjHyXiGRCI4966894 = RjHyXiGRCI74769338;     RjHyXiGRCI74769338 = RjHyXiGRCI49058013;     RjHyXiGRCI49058013 = RjHyXiGRCI20949141;     RjHyXiGRCI20949141 = RjHyXiGRCI4814400;     RjHyXiGRCI4814400 = RjHyXiGRCI68664529;     RjHyXiGRCI68664529 = RjHyXiGRCI71457725;     RjHyXiGRCI71457725 = RjHyXiGRCI31787196;     RjHyXiGRCI31787196 = RjHyXiGRCI3946276;     RjHyXiGRCI3946276 = RjHyXiGRCI13253136;     RjHyXiGRCI13253136 = RjHyXiGRCI6741064;     RjHyXiGRCI6741064 = RjHyXiGRCI34399950;     RjHyXiGRCI34399950 = RjHyXiGRCI66222486;     RjHyXiGRCI66222486 = RjHyXiGRCI63073159;     RjHyXiGRCI63073159 = RjHyXiGRCI55960434;     RjHyXiGRCI55960434 = RjHyXiGRCI25049660;     RjHyXiGRCI25049660 = RjHyXiGRCI85435815;     RjHyXiGRCI85435815 = RjHyXiGRCI94346024;     RjHyXiGRCI94346024 = RjHyXiGRCI46229346;     RjHyXiGRCI46229346 = RjHyXiGRCI48581754;     RjHyXiGRCI48581754 = RjHyXiGRCI16395769;     RjHyXiGRCI16395769 = RjHyXiGRCI91610969;     RjHyXiGRCI91610969 = RjHyXiGRCI70096824;     RjHyXiGRCI70096824 = RjHyXiGRCI39627529;     RjHyXiGRCI39627529 = RjHyXiGRCI27962082;     RjHyXiGRCI27962082 = RjHyXiGRCI74341064;     RjHyXiGRCI74341064 = RjHyXiGRCI70863716;     RjHyXiGRCI70863716 = RjHyXiGRCI2423417;     RjHyXiGRCI2423417 = RjHyXiGRCI65782831;     RjHyXiGRCI65782831 = RjHyXiGRCI46581664;     RjHyXiGRCI46581664 = RjHyXiGRCI74396543;     RjHyXiGRCI74396543 = RjHyXiGRCI44832405;     RjHyXiGRCI44832405 = RjHyXiGRCI39698870;     RjHyXiGRCI39698870 = RjHyXiGRCI68169994;     RjHyXiGRCI68169994 = RjHyXiGRCI66524614;     RjHyXiGRCI66524614 = RjHyXiGRCI80757198;     RjHyXiGRCI80757198 = RjHyXiGRCI86831023;     RjHyXiGRCI86831023 = RjHyXiGRCI38472518;     RjHyXiGRCI38472518 = RjHyXiGRCI85752085;     RjHyXiGRCI85752085 = RjHyXiGRCI78946780;     RjHyXiGRCI78946780 = RjHyXiGRCI45083377;     RjHyXiGRCI45083377 = RjHyXiGRCI70431136;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WumhOBQTiD90146071() {     int HrRDTcfrsb83217303 = -849570992;    int HrRDTcfrsb54180757 = -509016132;    int HrRDTcfrsb5640171 = 89968964;    int HrRDTcfrsb13153730 = -179109594;    int HrRDTcfrsb54155051 = -960266649;    int HrRDTcfrsb37641037 = -844174427;    int HrRDTcfrsb75279153 = -694969217;    int HrRDTcfrsb58352995 = -159304608;    int HrRDTcfrsb34669024 = -462826575;    int HrRDTcfrsb86156732 = -920721638;    int HrRDTcfrsb74166562 = -111787392;    int HrRDTcfrsb99927602 = -106594281;    int HrRDTcfrsb95503965 = -87772279;    int HrRDTcfrsb22792385 = -70909031;    int HrRDTcfrsb48449235 = -760070667;    int HrRDTcfrsb25288404 = -399493741;    int HrRDTcfrsb21222446 = -560039138;    int HrRDTcfrsb18123153 = -632352654;    int HrRDTcfrsb12647601 = -574262506;    int HrRDTcfrsb63593192 = -523435191;    int HrRDTcfrsb25031531 = -731938871;    int HrRDTcfrsb13960329 = -443810333;    int HrRDTcfrsb32600951 = -299993168;    int HrRDTcfrsb9629986 = -932922495;    int HrRDTcfrsb80029015 = -529446680;    int HrRDTcfrsb36995698 = -748719323;    int HrRDTcfrsb30977165 = -555128861;    int HrRDTcfrsb72366307 = -111764101;    int HrRDTcfrsb27301795 = -386981653;    int HrRDTcfrsb87200928 = -209481158;    int HrRDTcfrsb55709494 = -759820474;    int HrRDTcfrsb78540278 = -884186356;    int HrRDTcfrsb82235850 = -135746567;    int HrRDTcfrsb98490760 = -866272073;    int HrRDTcfrsb42975702 = -615148123;    int HrRDTcfrsb13625733 = -442160319;    int HrRDTcfrsb99227445 = -417172630;    int HrRDTcfrsb56952165 = -28809958;    int HrRDTcfrsb77266893 = 34182501;    int HrRDTcfrsb81791266 = 40585975;    int HrRDTcfrsb97320344 = -346675974;    int HrRDTcfrsb67124114 = 43132600;    int HrRDTcfrsb25449956 = -798753823;    int HrRDTcfrsb92239847 = -712139939;    int HrRDTcfrsb96549592 = 22693393;    int HrRDTcfrsb31367117 = -880900516;    int HrRDTcfrsb41648102 = -776837864;    int HrRDTcfrsb78904770 = -566989015;    int HrRDTcfrsb9449903 = -578746388;    int HrRDTcfrsb29673081 = -774104925;    int HrRDTcfrsb78436242 = -401478382;    int HrRDTcfrsb68586330 = -970845292;    int HrRDTcfrsb7283189 = -567937029;    int HrRDTcfrsb15106796 = -796846104;    int HrRDTcfrsb65114540 = -157904735;    int HrRDTcfrsb69256974 = -305760659;    int HrRDTcfrsb21579806 = -109022965;    int HrRDTcfrsb96010185 = 22891459;    int HrRDTcfrsb33124714 = -649662914;    int HrRDTcfrsb17159353 = -111547326;    int HrRDTcfrsb6663873 = -189045567;    int HrRDTcfrsb2912847 = -483205116;    int HrRDTcfrsb31051200 = -772322955;    int HrRDTcfrsb47468095 = -153345418;    int HrRDTcfrsb30447238 = -60901164;    int HrRDTcfrsb95626283 = -227601036;    int HrRDTcfrsb17691752 = -970847715;    int HrRDTcfrsb97013205 = -221500206;    int HrRDTcfrsb79816682 = -455760909;    int HrRDTcfrsb34823502 = -217910349;    int HrRDTcfrsb26060958 = -982321112;    int HrRDTcfrsb64270281 = -431229180;    int HrRDTcfrsb40856259 = -566535156;    int HrRDTcfrsb30856335 = -514848481;    int HrRDTcfrsb66272847 = -76759218;    int HrRDTcfrsb57907416 = -675071471;    int HrRDTcfrsb88510373 = -645056510;    int HrRDTcfrsb40361104 = -587853229;    int HrRDTcfrsb13080393 = -855615888;    int HrRDTcfrsb48661899 = -648546165;    int HrRDTcfrsb95347595 = -971881460;    int HrRDTcfrsb52072394 = -988139846;    int HrRDTcfrsb62916404 = -533017714;    int HrRDTcfrsb97628713 = -612876728;    int HrRDTcfrsb8764687 = -808002776;    int HrRDTcfrsb87123164 = -788975183;    int HrRDTcfrsb71257090 = -216249328;    int HrRDTcfrsb67129055 = -338900464;    int HrRDTcfrsb33376220 = -608367338;    int HrRDTcfrsb73718727 = -209387464;    int HrRDTcfrsb92045927 = -233137355;    int HrRDTcfrsb3217260 = -340064089;    int HrRDTcfrsb23827451 = -379147045;    int HrRDTcfrsb60107540 = -854270173;    int HrRDTcfrsb75127394 = -770368459;    int HrRDTcfrsb94407498 = -863470858;    int HrRDTcfrsb36072914 = -184544445;    int HrRDTcfrsb77981860 = -545408406;    int HrRDTcfrsb61792610 = -551238776;    int HrRDTcfrsb923309 = -849570992;     HrRDTcfrsb83217303 = HrRDTcfrsb54180757;     HrRDTcfrsb54180757 = HrRDTcfrsb5640171;     HrRDTcfrsb5640171 = HrRDTcfrsb13153730;     HrRDTcfrsb13153730 = HrRDTcfrsb54155051;     HrRDTcfrsb54155051 = HrRDTcfrsb37641037;     HrRDTcfrsb37641037 = HrRDTcfrsb75279153;     HrRDTcfrsb75279153 = HrRDTcfrsb58352995;     HrRDTcfrsb58352995 = HrRDTcfrsb34669024;     HrRDTcfrsb34669024 = HrRDTcfrsb86156732;     HrRDTcfrsb86156732 = HrRDTcfrsb74166562;     HrRDTcfrsb74166562 = HrRDTcfrsb99927602;     HrRDTcfrsb99927602 = HrRDTcfrsb95503965;     HrRDTcfrsb95503965 = HrRDTcfrsb22792385;     HrRDTcfrsb22792385 = HrRDTcfrsb48449235;     HrRDTcfrsb48449235 = HrRDTcfrsb25288404;     HrRDTcfrsb25288404 = HrRDTcfrsb21222446;     HrRDTcfrsb21222446 = HrRDTcfrsb18123153;     HrRDTcfrsb18123153 = HrRDTcfrsb12647601;     HrRDTcfrsb12647601 = HrRDTcfrsb63593192;     HrRDTcfrsb63593192 = HrRDTcfrsb25031531;     HrRDTcfrsb25031531 = HrRDTcfrsb13960329;     HrRDTcfrsb13960329 = HrRDTcfrsb32600951;     HrRDTcfrsb32600951 = HrRDTcfrsb9629986;     HrRDTcfrsb9629986 = HrRDTcfrsb80029015;     HrRDTcfrsb80029015 = HrRDTcfrsb36995698;     HrRDTcfrsb36995698 = HrRDTcfrsb30977165;     HrRDTcfrsb30977165 = HrRDTcfrsb72366307;     HrRDTcfrsb72366307 = HrRDTcfrsb27301795;     HrRDTcfrsb27301795 = HrRDTcfrsb87200928;     HrRDTcfrsb87200928 = HrRDTcfrsb55709494;     HrRDTcfrsb55709494 = HrRDTcfrsb78540278;     HrRDTcfrsb78540278 = HrRDTcfrsb82235850;     HrRDTcfrsb82235850 = HrRDTcfrsb98490760;     HrRDTcfrsb98490760 = HrRDTcfrsb42975702;     HrRDTcfrsb42975702 = HrRDTcfrsb13625733;     HrRDTcfrsb13625733 = HrRDTcfrsb99227445;     HrRDTcfrsb99227445 = HrRDTcfrsb56952165;     HrRDTcfrsb56952165 = HrRDTcfrsb77266893;     HrRDTcfrsb77266893 = HrRDTcfrsb81791266;     HrRDTcfrsb81791266 = HrRDTcfrsb97320344;     HrRDTcfrsb97320344 = HrRDTcfrsb67124114;     HrRDTcfrsb67124114 = HrRDTcfrsb25449956;     HrRDTcfrsb25449956 = HrRDTcfrsb92239847;     HrRDTcfrsb92239847 = HrRDTcfrsb96549592;     HrRDTcfrsb96549592 = HrRDTcfrsb31367117;     HrRDTcfrsb31367117 = HrRDTcfrsb41648102;     HrRDTcfrsb41648102 = HrRDTcfrsb78904770;     HrRDTcfrsb78904770 = HrRDTcfrsb9449903;     HrRDTcfrsb9449903 = HrRDTcfrsb29673081;     HrRDTcfrsb29673081 = HrRDTcfrsb78436242;     HrRDTcfrsb78436242 = HrRDTcfrsb68586330;     HrRDTcfrsb68586330 = HrRDTcfrsb7283189;     HrRDTcfrsb7283189 = HrRDTcfrsb15106796;     HrRDTcfrsb15106796 = HrRDTcfrsb65114540;     HrRDTcfrsb65114540 = HrRDTcfrsb69256974;     HrRDTcfrsb69256974 = HrRDTcfrsb21579806;     HrRDTcfrsb21579806 = HrRDTcfrsb96010185;     HrRDTcfrsb96010185 = HrRDTcfrsb33124714;     HrRDTcfrsb33124714 = HrRDTcfrsb17159353;     HrRDTcfrsb17159353 = HrRDTcfrsb6663873;     HrRDTcfrsb6663873 = HrRDTcfrsb2912847;     HrRDTcfrsb2912847 = HrRDTcfrsb31051200;     HrRDTcfrsb31051200 = HrRDTcfrsb47468095;     HrRDTcfrsb47468095 = HrRDTcfrsb30447238;     HrRDTcfrsb30447238 = HrRDTcfrsb95626283;     HrRDTcfrsb95626283 = HrRDTcfrsb17691752;     HrRDTcfrsb17691752 = HrRDTcfrsb97013205;     HrRDTcfrsb97013205 = HrRDTcfrsb79816682;     HrRDTcfrsb79816682 = HrRDTcfrsb34823502;     HrRDTcfrsb34823502 = HrRDTcfrsb26060958;     HrRDTcfrsb26060958 = HrRDTcfrsb64270281;     HrRDTcfrsb64270281 = HrRDTcfrsb40856259;     HrRDTcfrsb40856259 = HrRDTcfrsb30856335;     HrRDTcfrsb30856335 = HrRDTcfrsb66272847;     HrRDTcfrsb66272847 = HrRDTcfrsb57907416;     HrRDTcfrsb57907416 = HrRDTcfrsb88510373;     HrRDTcfrsb88510373 = HrRDTcfrsb40361104;     HrRDTcfrsb40361104 = HrRDTcfrsb13080393;     HrRDTcfrsb13080393 = HrRDTcfrsb48661899;     HrRDTcfrsb48661899 = HrRDTcfrsb95347595;     HrRDTcfrsb95347595 = HrRDTcfrsb52072394;     HrRDTcfrsb52072394 = HrRDTcfrsb62916404;     HrRDTcfrsb62916404 = HrRDTcfrsb97628713;     HrRDTcfrsb97628713 = HrRDTcfrsb8764687;     HrRDTcfrsb8764687 = HrRDTcfrsb87123164;     HrRDTcfrsb87123164 = HrRDTcfrsb71257090;     HrRDTcfrsb71257090 = HrRDTcfrsb67129055;     HrRDTcfrsb67129055 = HrRDTcfrsb33376220;     HrRDTcfrsb33376220 = HrRDTcfrsb73718727;     HrRDTcfrsb73718727 = HrRDTcfrsb92045927;     HrRDTcfrsb92045927 = HrRDTcfrsb3217260;     HrRDTcfrsb3217260 = HrRDTcfrsb23827451;     HrRDTcfrsb23827451 = HrRDTcfrsb60107540;     HrRDTcfrsb60107540 = HrRDTcfrsb75127394;     HrRDTcfrsb75127394 = HrRDTcfrsb94407498;     HrRDTcfrsb94407498 = HrRDTcfrsb36072914;     HrRDTcfrsb36072914 = HrRDTcfrsb77981860;     HrRDTcfrsb77981860 = HrRDTcfrsb61792610;     HrRDTcfrsb61792610 = HrRDTcfrsb923309;     HrRDTcfrsb923309 = HrRDTcfrsb83217303;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void unqgmLFVlP60271747() {     int MERWTBezeS50664340 = -449615126;    int MERWTBezeS49376948 = -116319834;    int MERWTBezeS82812405 = -746646417;    int MERWTBezeS7449636 = -188873452;    int MERWTBezeS13277724 = -656163032;    int MERWTBezeS60320072 = 49184235;    int MERWTBezeS49040280 = -737997493;    int MERWTBezeS45033160 = -300213780;    int MERWTBezeS35108081 = -757791968;    int MERWTBezeS78367909 = -266803669;    int MERWTBezeS59944634 = -892357908;    int MERWTBezeS80351143 = 35127176;    int MERWTBezeS97611669 = -873436766;    int MERWTBezeS11538674 = -770274756;    int MERWTBezeS56807752 = -611465278;    int MERWTBezeS68916460 = -247749044;    int MERWTBezeS20953268 = -877798864;    int MERWTBezeS46277691 = -954055801;    int MERWTBezeS94600562 = -151560982;    int MERWTBezeS11808481 = -856021025;    int MERWTBezeS59710551 = -980871913;    int MERWTBezeS15964472 = -270120905;    int MERWTBezeS70952682 = -358782641;    int MERWTBezeS11915376 = -11387801;    int MERWTBezeS53233201 = -686108204;    int MERWTBezeS42981213 = -534653379;    int MERWTBezeS74161043 = -342879707;    int MERWTBezeS57564707 = -784647078;    int MERWTBezeS71138078 = -335373454;    int MERWTBezeS58297583 = -454693907;    int MERWTBezeS18438341 = -98016516;    int MERWTBezeS28126856 = -882580802;    int MERWTBezeS10485253 = -366174750;    int MERWTBezeS69940189 = -574465369;    int MERWTBezeS78182352 = -83729338;    int MERWTBezeS53348200 = -398953274;    int MERWTBezeS98754626 = 42144777;    int MERWTBezeS57715747 = -927096392;    int MERWTBezeS98498331 = -576935261;    int MERWTBezeS60169498 = -952707784;    int MERWTBezeS26855386 = -746756227;    int MERWTBezeS25803568 = -268918739;    int MERWTBezeS91050248 = -103562332;    int MERWTBezeS86630886 = -970027405;    int MERWTBezeS52524094 = -629334763;    int MERWTBezeS19900880 = -110963900;    int MERWTBezeS8006042 = -820129678;    int MERWTBezeS56388465 = -91232811;    int MERWTBezeS77198329 = 21685310;    int MERWTBezeS1419741 = -938135796;    int MERWTBezeS58617906 = -733023185;    int MERWTBezeS71597769 = -474950865;    int MERWTBezeS98047993 = -591801761;    int MERWTBezeS6972008 = -850264479;    int MERWTBezeS561196 = -791358625;    int MERWTBezeS34699868 = -79494221;    int MERWTBezeS78424266 = -757537193;    int MERWTBezeS70897030 = -635258617;    int MERWTBezeS54216435 = -502765249;    int MERWTBezeS70296510 = -21509653;    int MERWTBezeS86159029 = -607936058;    int MERWTBezeS91475572 = -953350416;    int MERWTBezeS73895082 = -964840327;    int MERWTBezeS76810497 = -203098061;    int MERWTBezeS59929569 = -68787154;    int MERWTBezeS31817779 = 90222894;    int MERWTBezeS69865890 = -598698074;    int MERWTBezeS27671481 = -198971398;    int MERWTBezeS33356321 = -586545418;    int MERWTBezeS3459553 = -112512004;    int MERWTBezeS70161834 = -189893822;    int MERWTBezeS63237521 = -950702472;    int MERWTBezeS47779360 = -277120541;    int MERWTBezeS34431064 = -198853198;    int MERWTBezeS84953095 = -9264799;    int MERWTBezeS33906984 = -611953174;    int MERWTBezeS24914224 = -66558574;    int MERWTBezeS84321795 = -388755237;    int MERWTBezeS59391281 = -382053038;    int MERWTBezeS33332322 = -475144304;    int MERWTBezeS34975172 = -714523702;    int MERWTBezeS17772579 = -151646897;    int MERWTBezeS80366378 = -706332389;    int MERWTBezeS69718337 = -397237658;    int MERWTBezeS99679677 = -721670722;    int MERWTBezeS46840571 = -623065651;    int MERWTBezeS30078862 = -190779041;    int MERWTBezeS3513246 = -515910271;    int MERWTBezeS69378993 = -783106744;    int MERWTBezeS43482485 = 95764883;    int MERWTBezeS74923934 = -641416081;    int MERWTBezeS27857596 = -322596606;    int MERWTBezeS3499313 = -324331144;    int MERWTBezeS28201821 = -455425608;    int MERWTBezeS74010469 = -244771726;    int MERWTBezeS35379814 = -793405812;    int MERWTBezeS51908485 = -304078412;    int MERWTBezeS14239751 = -900464271;    int MERWTBezeS26701318 = -801240251;    int MERWTBezeS20706315 = -449615126;     MERWTBezeS50664340 = MERWTBezeS49376948;     MERWTBezeS49376948 = MERWTBezeS82812405;     MERWTBezeS82812405 = MERWTBezeS7449636;     MERWTBezeS7449636 = MERWTBezeS13277724;     MERWTBezeS13277724 = MERWTBezeS60320072;     MERWTBezeS60320072 = MERWTBezeS49040280;     MERWTBezeS49040280 = MERWTBezeS45033160;     MERWTBezeS45033160 = MERWTBezeS35108081;     MERWTBezeS35108081 = MERWTBezeS78367909;     MERWTBezeS78367909 = MERWTBezeS59944634;     MERWTBezeS59944634 = MERWTBezeS80351143;     MERWTBezeS80351143 = MERWTBezeS97611669;     MERWTBezeS97611669 = MERWTBezeS11538674;     MERWTBezeS11538674 = MERWTBezeS56807752;     MERWTBezeS56807752 = MERWTBezeS68916460;     MERWTBezeS68916460 = MERWTBezeS20953268;     MERWTBezeS20953268 = MERWTBezeS46277691;     MERWTBezeS46277691 = MERWTBezeS94600562;     MERWTBezeS94600562 = MERWTBezeS11808481;     MERWTBezeS11808481 = MERWTBezeS59710551;     MERWTBezeS59710551 = MERWTBezeS15964472;     MERWTBezeS15964472 = MERWTBezeS70952682;     MERWTBezeS70952682 = MERWTBezeS11915376;     MERWTBezeS11915376 = MERWTBezeS53233201;     MERWTBezeS53233201 = MERWTBezeS42981213;     MERWTBezeS42981213 = MERWTBezeS74161043;     MERWTBezeS74161043 = MERWTBezeS57564707;     MERWTBezeS57564707 = MERWTBezeS71138078;     MERWTBezeS71138078 = MERWTBezeS58297583;     MERWTBezeS58297583 = MERWTBezeS18438341;     MERWTBezeS18438341 = MERWTBezeS28126856;     MERWTBezeS28126856 = MERWTBezeS10485253;     MERWTBezeS10485253 = MERWTBezeS69940189;     MERWTBezeS69940189 = MERWTBezeS78182352;     MERWTBezeS78182352 = MERWTBezeS53348200;     MERWTBezeS53348200 = MERWTBezeS98754626;     MERWTBezeS98754626 = MERWTBezeS57715747;     MERWTBezeS57715747 = MERWTBezeS98498331;     MERWTBezeS98498331 = MERWTBezeS60169498;     MERWTBezeS60169498 = MERWTBezeS26855386;     MERWTBezeS26855386 = MERWTBezeS25803568;     MERWTBezeS25803568 = MERWTBezeS91050248;     MERWTBezeS91050248 = MERWTBezeS86630886;     MERWTBezeS86630886 = MERWTBezeS52524094;     MERWTBezeS52524094 = MERWTBezeS19900880;     MERWTBezeS19900880 = MERWTBezeS8006042;     MERWTBezeS8006042 = MERWTBezeS56388465;     MERWTBezeS56388465 = MERWTBezeS77198329;     MERWTBezeS77198329 = MERWTBezeS1419741;     MERWTBezeS1419741 = MERWTBezeS58617906;     MERWTBezeS58617906 = MERWTBezeS71597769;     MERWTBezeS71597769 = MERWTBezeS98047993;     MERWTBezeS98047993 = MERWTBezeS6972008;     MERWTBezeS6972008 = MERWTBezeS561196;     MERWTBezeS561196 = MERWTBezeS34699868;     MERWTBezeS34699868 = MERWTBezeS78424266;     MERWTBezeS78424266 = MERWTBezeS70897030;     MERWTBezeS70897030 = MERWTBezeS54216435;     MERWTBezeS54216435 = MERWTBezeS70296510;     MERWTBezeS70296510 = MERWTBezeS86159029;     MERWTBezeS86159029 = MERWTBezeS91475572;     MERWTBezeS91475572 = MERWTBezeS73895082;     MERWTBezeS73895082 = MERWTBezeS76810497;     MERWTBezeS76810497 = MERWTBezeS59929569;     MERWTBezeS59929569 = MERWTBezeS31817779;     MERWTBezeS31817779 = MERWTBezeS69865890;     MERWTBezeS69865890 = MERWTBezeS27671481;     MERWTBezeS27671481 = MERWTBezeS33356321;     MERWTBezeS33356321 = MERWTBezeS3459553;     MERWTBezeS3459553 = MERWTBezeS70161834;     MERWTBezeS70161834 = MERWTBezeS63237521;     MERWTBezeS63237521 = MERWTBezeS47779360;     MERWTBezeS47779360 = MERWTBezeS34431064;     MERWTBezeS34431064 = MERWTBezeS84953095;     MERWTBezeS84953095 = MERWTBezeS33906984;     MERWTBezeS33906984 = MERWTBezeS24914224;     MERWTBezeS24914224 = MERWTBezeS84321795;     MERWTBezeS84321795 = MERWTBezeS59391281;     MERWTBezeS59391281 = MERWTBezeS33332322;     MERWTBezeS33332322 = MERWTBezeS34975172;     MERWTBezeS34975172 = MERWTBezeS17772579;     MERWTBezeS17772579 = MERWTBezeS80366378;     MERWTBezeS80366378 = MERWTBezeS69718337;     MERWTBezeS69718337 = MERWTBezeS99679677;     MERWTBezeS99679677 = MERWTBezeS46840571;     MERWTBezeS46840571 = MERWTBezeS30078862;     MERWTBezeS30078862 = MERWTBezeS3513246;     MERWTBezeS3513246 = MERWTBezeS69378993;     MERWTBezeS69378993 = MERWTBezeS43482485;     MERWTBezeS43482485 = MERWTBezeS74923934;     MERWTBezeS74923934 = MERWTBezeS27857596;     MERWTBezeS27857596 = MERWTBezeS3499313;     MERWTBezeS3499313 = MERWTBezeS28201821;     MERWTBezeS28201821 = MERWTBezeS74010469;     MERWTBezeS74010469 = MERWTBezeS35379814;     MERWTBezeS35379814 = MERWTBezeS51908485;     MERWTBezeS51908485 = MERWTBezeS14239751;     MERWTBezeS14239751 = MERWTBezeS26701318;     MERWTBezeS26701318 = MERWTBezeS20706315;     MERWTBezeS20706315 = MERWTBezeS50664340;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NUtUNrORvi78485088() {     int hLFtbAtaBm34108645 = -230280807;    int hLFtbAtaBm66249037 = -115702022;    int hLFtbAtaBm79220912 = -540171645;    int hLFtbAtaBm70241887 = -382673288;    int hLFtbAtaBm19399048 = -463030523;    int hLFtbAtaBm58824473 = -829887418;    int hLFtbAtaBm20079598 = -477908806;    int hLFtbAtaBm59380603 = -287371810;    int hLFtbAtaBm93637781 = 748681;    int hLFtbAtaBm10913562 = -735179782;    int hLFtbAtaBm9360573 = 17526220;    int hLFtbAtaBm43556543 = 47508930;    int hLFtbAtaBm15729446 = -665098007;    int hLFtbAtaBm91165565 = -558565113;    int hLFtbAtaBm17662216 = -705488260;    int hLFtbAtaBm55305622 = -35828369;    int hLFtbAtaBm14705153 = -631143891;    int hLFtbAtaBm27129138 = -101360678;    int hLFtbAtaBm57040705 = -150235853;    int hLFtbAtaBm1175924 = -285607018;    int hLFtbAtaBm15350939 = -570400944;    int hLFtbAtaBm28111790 = -261272676;    int hLFtbAtaBm74778389 = -398671839;    int hLFtbAtaBm46621933 = -83213197;    int hLFtbAtaBm94809158 = -145760963;    int hLFtbAtaBm38323362 = -415163095;    int hLFtbAtaBm23615784 = 29035951;    int hLFtbAtaBm97296332 = -854601962;    int hLFtbAtaBm7219172 = -197878596;    int hLFtbAtaBm74130924 = -967543758;    int hLFtbAtaBm36533654 = -835773716;    int hLFtbAtaBm91233282 = -797775881;    int hLFtbAtaBm54024139 = 12953073;    int hLFtbAtaBm12098700 = -166067539;    int hLFtbAtaBm56909083 = -281394073;    int hLFtbAtaBm65737384 = -186977180;    int hLFtbAtaBm67368804 = -936704402;    int hLFtbAtaBm98294578 = -635566912;    int hLFtbAtaBm47493410 = -761833059;    int hLFtbAtaBm9664634 = -705301306;    int hLFtbAtaBm60504310 = -318305201;    int hLFtbAtaBm76291353 = -842827060;    int hLFtbAtaBm14617318 = -654619574;    int hLFtbAtaBm78535826 = -726210176;    int hLFtbAtaBm44468591 = -357491717;    int hLFtbAtaBm59506875 = -130574824;    int hLFtbAtaBm84021471 = -952433907;    int hLFtbAtaBm76059060 = -89954529;    int hLFtbAtaBm8045442 = -563916772;    int hLFtbAtaBm36752225 = -45156251;    int hLFtbAtaBm31875354 = -953924608;    int hLFtbAtaBm27098564 = -533210176;    int hLFtbAtaBm12160453 = -803549077;    int hLFtbAtaBm31648766 = -568677866;    int hLFtbAtaBm63134150 = -39844748;    int hLFtbAtaBm5996856 = -969008131;    int hLFtbAtaBm91470647 = -717030184;    int hLFtbAtaBm32598979 = -356958448;    int hLFtbAtaBm75432728 = -136912326;    int hLFtbAtaBm81075685 = 52132571;    int hLFtbAtaBm35208690 = -758923369;    int hLFtbAtaBm22783265 = -623306845;    int hLFtbAtaBm52161431 = 10506786;    int hLFtbAtaBm19506857 = -31707561;    int hLFtbAtaBm74379907 = -899406066;    int hLFtbAtaBm18127290 = -184697899;    int hLFtbAtaBm89532403 = -965444143;    int hLFtbAtaBm3630746 = -399030469;    int hLFtbAtaBm34256483 = -177171040;    int hLFtbAtaBm51924831 = -418511081;    int hLFtbAtaBm87936817 = -99123967;    int hLFtbAtaBm16410574 = -995576979;    int hLFtbAtaBm79635728 = -339527619;    int hLFtbAtaBm47376071 = -444934547;    int hLFtbAtaBm40671613 = -967301818;    int hLFtbAtaBm39059585 = -727573884;    int hLFtbAtaBm13494472 = -606653102;    int hLFtbAtaBm96242562 = -672461664;    int hLFtbAtaBm2153343 = -725721481;    int hLFtbAtaBm35302284 = 84813861;    int hLFtbAtaBm54301891 = -462729188;    int hLFtbAtaBm47556723 = -881009520;    int hLFtbAtaBm89250891 = -190685190;    int hLFtbAtaBm70466947 = -52722345;    int hLFtbAtaBm42255571 = 86380849;    int hLFtbAtaBm9435091 = -202563541;    int hLFtbAtaBm79072830 = -994226805;    int hLFtbAtaBm22375374 = -418369061;    int hLFtbAtaBm48964549 = -26222791;    int hLFtbAtaBm50912228 = -312385942;    int hLFtbAtaBm74266737 = -469946996;    int hLFtbAtaBm34769826 = -479745954;    int hLFtbAtaBm22861851 = -398654587;    int hLFtbAtaBm66417724 = -713965631;    int hLFtbAtaBm74455943 = -946377938;    int hLFtbAtaBm37721046 = -694998356;    int hLFtbAtaBm24129923 = -753333847;    int hLFtbAtaBm95110461 = -522912014;    int hLFtbAtaBm4155920 = -826804110;    int hLFtbAtaBm26341301 = -230280807;     hLFtbAtaBm34108645 = hLFtbAtaBm66249037;     hLFtbAtaBm66249037 = hLFtbAtaBm79220912;     hLFtbAtaBm79220912 = hLFtbAtaBm70241887;     hLFtbAtaBm70241887 = hLFtbAtaBm19399048;     hLFtbAtaBm19399048 = hLFtbAtaBm58824473;     hLFtbAtaBm58824473 = hLFtbAtaBm20079598;     hLFtbAtaBm20079598 = hLFtbAtaBm59380603;     hLFtbAtaBm59380603 = hLFtbAtaBm93637781;     hLFtbAtaBm93637781 = hLFtbAtaBm10913562;     hLFtbAtaBm10913562 = hLFtbAtaBm9360573;     hLFtbAtaBm9360573 = hLFtbAtaBm43556543;     hLFtbAtaBm43556543 = hLFtbAtaBm15729446;     hLFtbAtaBm15729446 = hLFtbAtaBm91165565;     hLFtbAtaBm91165565 = hLFtbAtaBm17662216;     hLFtbAtaBm17662216 = hLFtbAtaBm55305622;     hLFtbAtaBm55305622 = hLFtbAtaBm14705153;     hLFtbAtaBm14705153 = hLFtbAtaBm27129138;     hLFtbAtaBm27129138 = hLFtbAtaBm57040705;     hLFtbAtaBm57040705 = hLFtbAtaBm1175924;     hLFtbAtaBm1175924 = hLFtbAtaBm15350939;     hLFtbAtaBm15350939 = hLFtbAtaBm28111790;     hLFtbAtaBm28111790 = hLFtbAtaBm74778389;     hLFtbAtaBm74778389 = hLFtbAtaBm46621933;     hLFtbAtaBm46621933 = hLFtbAtaBm94809158;     hLFtbAtaBm94809158 = hLFtbAtaBm38323362;     hLFtbAtaBm38323362 = hLFtbAtaBm23615784;     hLFtbAtaBm23615784 = hLFtbAtaBm97296332;     hLFtbAtaBm97296332 = hLFtbAtaBm7219172;     hLFtbAtaBm7219172 = hLFtbAtaBm74130924;     hLFtbAtaBm74130924 = hLFtbAtaBm36533654;     hLFtbAtaBm36533654 = hLFtbAtaBm91233282;     hLFtbAtaBm91233282 = hLFtbAtaBm54024139;     hLFtbAtaBm54024139 = hLFtbAtaBm12098700;     hLFtbAtaBm12098700 = hLFtbAtaBm56909083;     hLFtbAtaBm56909083 = hLFtbAtaBm65737384;     hLFtbAtaBm65737384 = hLFtbAtaBm67368804;     hLFtbAtaBm67368804 = hLFtbAtaBm98294578;     hLFtbAtaBm98294578 = hLFtbAtaBm47493410;     hLFtbAtaBm47493410 = hLFtbAtaBm9664634;     hLFtbAtaBm9664634 = hLFtbAtaBm60504310;     hLFtbAtaBm60504310 = hLFtbAtaBm76291353;     hLFtbAtaBm76291353 = hLFtbAtaBm14617318;     hLFtbAtaBm14617318 = hLFtbAtaBm78535826;     hLFtbAtaBm78535826 = hLFtbAtaBm44468591;     hLFtbAtaBm44468591 = hLFtbAtaBm59506875;     hLFtbAtaBm59506875 = hLFtbAtaBm84021471;     hLFtbAtaBm84021471 = hLFtbAtaBm76059060;     hLFtbAtaBm76059060 = hLFtbAtaBm8045442;     hLFtbAtaBm8045442 = hLFtbAtaBm36752225;     hLFtbAtaBm36752225 = hLFtbAtaBm31875354;     hLFtbAtaBm31875354 = hLFtbAtaBm27098564;     hLFtbAtaBm27098564 = hLFtbAtaBm12160453;     hLFtbAtaBm12160453 = hLFtbAtaBm31648766;     hLFtbAtaBm31648766 = hLFtbAtaBm63134150;     hLFtbAtaBm63134150 = hLFtbAtaBm5996856;     hLFtbAtaBm5996856 = hLFtbAtaBm91470647;     hLFtbAtaBm91470647 = hLFtbAtaBm32598979;     hLFtbAtaBm32598979 = hLFtbAtaBm75432728;     hLFtbAtaBm75432728 = hLFtbAtaBm81075685;     hLFtbAtaBm81075685 = hLFtbAtaBm35208690;     hLFtbAtaBm35208690 = hLFtbAtaBm22783265;     hLFtbAtaBm22783265 = hLFtbAtaBm52161431;     hLFtbAtaBm52161431 = hLFtbAtaBm19506857;     hLFtbAtaBm19506857 = hLFtbAtaBm74379907;     hLFtbAtaBm74379907 = hLFtbAtaBm18127290;     hLFtbAtaBm18127290 = hLFtbAtaBm89532403;     hLFtbAtaBm89532403 = hLFtbAtaBm3630746;     hLFtbAtaBm3630746 = hLFtbAtaBm34256483;     hLFtbAtaBm34256483 = hLFtbAtaBm51924831;     hLFtbAtaBm51924831 = hLFtbAtaBm87936817;     hLFtbAtaBm87936817 = hLFtbAtaBm16410574;     hLFtbAtaBm16410574 = hLFtbAtaBm79635728;     hLFtbAtaBm79635728 = hLFtbAtaBm47376071;     hLFtbAtaBm47376071 = hLFtbAtaBm40671613;     hLFtbAtaBm40671613 = hLFtbAtaBm39059585;     hLFtbAtaBm39059585 = hLFtbAtaBm13494472;     hLFtbAtaBm13494472 = hLFtbAtaBm96242562;     hLFtbAtaBm96242562 = hLFtbAtaBm2153343;     hLFtbAtaBm2153343 = hLFtbAtaBm35302284;     hLFtbAtaBm35302284 = hLFtbAtaBm54301891;     hLFtbAtaBm54301891 = hLFtbAtaBm47556723;     hLFtbAtaBm47556723 = hLFtbAtaBm89250891;     hLFtbAtaBm89250891 = hLFtbAtaBm70466947;     hLFtbAtaBm70466947 = hLFtbAtaBm42255571;     hLFtbAtaBm42255571 = hLFtbAtaBm9435091;     hLFtbAtaBm9435091 = hLFtbAtaBm79072830;     hLFtbAtaBm79072830 = hLFtbAtaBm22375374;     hLFtbAtaBm22375374 = hLFtbAtaBm48964549;     hLFtbAtaBm48964549 = hLFtbAtaBm50912228;     hLFtbAtaBm50912228 = hLFtbAtaBm74266737;     hLFtbAtaBm74266737 = hLFtbAtaBm34769826;     hLFtbAtaBm34769826 = hLFtbAtaBm22861851;     hLFtbAtaBm22861851 = hLFtbAtaBm66417724;     hLFtbAtaBm66417724 = hLFtbAtaBm74455943;     hLFtbAtaBm74455943 = hLFtbAtaBm37721046;     hLFtbAtaBm37721046 = hLFtbAtaBm24129923;     hLFtbAtaBm24129923 = hLFtbAtaBm95110461;     hLFtbAtaBm95110461 = hLFtbAtaBm4155920;     hLFtbAtaBm4155920 = hLFtbAtaBm26341301;     hLFtbAtaBm26341301 = hLFtbAtaBm34108645;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WhpLinKDHQ48940961() {     int FheegWWEAQ46894812 = -365321498;    int FheegWWEAQ53019925 = -286036036;    int FheegWWEAQ96973711 = -948531027;    int FheegWWEAQ78182712 = -206875564;    int FheegWWEAQ12910153 = -954846988;    int FheegWWEAQ2134544 = -606810731;    int FheegWWEAQ41287356 = 76419123;    int FheegWWEAQ86099715 = -731890067;    int FheegWWEAQ89042591 = 38990589;    int FheegWWEAQ17132266 = -539267414;    int FheegWWEAQ36847955 = -97159796;    int FheegWWEAQ75507047 = -459823886;    int FheegWWEAQ85872748 = -568880664;    int FheegWWEAQ40789644 = -890980311;    int FheegWWEAQ40968769 = -612474091;    int FheegWWEAQ83730691 = -586719758;    int FheegWWEAQ70456970 = -294918358;    int FheegWWEAQ10687623 = -756570980;    int FheegWWEAQ58201335 = -59705048;    int FheegWWEAQ3830420 = -884851157;    int FheegWWEAQ95524995 = 38282792;    int FheegWWEAQ57159609 = -431131024;    int FheegWWEAQ13538685 = -329675733;    int FheegWWEAQ91129063 = -443558209;    int FheegWWEAQ703419 = -356202887;    int FheegWWEAQ19642008 = -105594294;    int FheegWWEAQ35031320 = -604670329;    int FheegWWEAQ64649258 = 71599934;    int FheegWWEAQ23836225 = -412095836;    int FheegWWEAQ26882040 = -597429913;    int FheegWWEAQ80969650 = -12190466;    int FheegWWEAQ94552106 = -776495560;    int FheegWWEAQ70089 = -962901712;    int FheegWWEAQ89175072 = -70821757;    int FheegWWEAQ39969615 = -891425953;    int FheegWWEAQ60961499 = 24459716;    int FheegWWEAQ4132866 = -726613753;    int FheegWWEAQ55998602 = -555187004;    int FheegWWEAQ53268796 = -603683636;    int FheegWWEAQ35929363 = -549718152;    int FheegWWEAQ21935619 = -693779194;    int FheegWWEAQ83993809 = -741138395;    int FheegWWEAQ49500788 = -678053019;    int FheegWWEAQ23164365 = 32617582;    int FheegWWEAQ24477081 = -250261677;    int FheegWWEAQ33135005 = -203893263;    int FheegWWEAQ61603491 = -349948959;    int FheegWWEAQ5499027 = -245307309;    int FheegWWEAQ52109491 = -727518747;    int FheegWWEAQ83702645 = -656192714;    int FheegWWEAQ50202847 = -106808916;    int FheegWWEAQ55275111 = 95604486;    int FheegWWEAQ46645602 = -979552360;    int FheegWWEAQ76348491 = -158129608;    int FheegWWEAQ53415966 = -34289235;    int FheegWWEAQ89735203 = -934190475;    int FheegWWEAQ39481240 = -956360303;    int FheegWWEAQ5844649 = -404972818;    int FheegWWEAQ77479294 = -850672678;    int FheegWWEAQ93268144 = -749252695;    int FheegWWEAQ67103224 = 97859598;    int FheegWWEAQ76638098 = -995180811;    int FheegWWEAQ62263491 = -219794231;    int FheegWWEAQ62160552 = -363579498;    int FheegWWEAQ36162616 = -427076948;    int FheegWWEAQ42295848 = -320664236;    int FheegWWEAQ75436959 = -496922175;    int FheegWWEAQ96697675 = -398058908;    int FheegWWEAQ820030 = -999554358;    int FheegWWEAQ80007269 = -536933807;    int FheegWWEAQ79597826 = -860106006;    int FheegWWEAQ14458369 = -739731355;    int FheegWWEAQ57418827 = -52887344;    int FheegWWEAQ22271972 = -509986897;    int FheegWWEAQ81894801 = -91071964;    int FheegWWEAQ11531186 = -220578814;    int FheegWWEAQ7658822 = -753078005;    int FheegWWEAQ90374320 = -262293315;    int FheegWWEAQ66651982 = -93296533;    int FheegWWEAQ67568413 = -52309624;    int FheegWWEAQ58038516 = -755645335;    int FheegWWEAQ29532293 = -259363021;    int FheegWWEAQ12539767 = -200881320;    int FheegWWEAQ40133580 = -755903122;    int FheegWWEAQ76679192 = -390620997;    int FheegWWEAQ25694539 = -7794953;    int FheegWWEAQ47906504 = -796943201;    int FheegWWEAQ23721598 = -704772104;    int FheegWWEAQ35759106 = 63467477;    int FheegWWEAQ50234412 = -957235479;    int FheegWWEAQ21480260 = -19179982;    int FheegWWEAQ98288216 = -221640936;    int FheegWWEAQ78519307 = -704514326;    int FheegWWEAQ60000651 = -854430942;    int FheegWWEAQ68826139 = -547577751;    int FheegWWEAQ45297521 = -698598384;    int FheegWWEAQ21730319 = -421344164;    int FheegWWEAQ87340235 = -214473521;    int FheegWWEAQ87001748 = -540305470;    int FheegWWEAQ82181232 = -365321498;     FheegWWEAQ46894812 = FheegWWEAQ53019925;     FheegWWEAQ53019925 = FheegWWEAQ96973711;     FheegWWEAQ96973711 = FheegWWEAQ78182712;     FheegWWEAQ78182712 = FheegWWEAQ12910153;     FheegWWEAQ12910153 = FheegWWEAQ2134544;     FheegWWEAQ2134544 = FheegWWEAQ41287356;     FheegWWEAQ41287356 = FheegWWEAQ86099715;     FheegWWEAQ86099715 = FheegWWEAQ89042591;     FheegWWEAQ89042591 = FheegWWEAQ17132266;     FheegWWEAQ17132266 = FheegWWEAQ36847955;     FheegWWEAQ36847955 = FheegWWEAQ75507047;     FheegWWEAQ75507047 = FheegWWEAQ85872748;     FheegWWEAQ85872748 = FheegWWEAQ40789644;     FheegWWEAQ40789644 = FheegWWEAQ40968769;     FheegWWEAQ40968769 = FheegWWEAQ83730691;     FheegWWEAQ83730691 = FheegWWEAQ70456970;     FheegWWEAQ70456970 = FheegWWEAQ10687623;     FheegWWEAQ10687623 = FheegWWEAQ58201335;     FheegWWEAQ58201335 = FheegWWEAQ3830420;     FheegWWEAQ3830420 = FheegWWEAQ95524995;     FheegWWEAQ95524995 = FheegWWEAQ57159609;     FheegWWEAQ57159609 = FheegWWEAQ13538685;     FheegWWEAQ13538685 = FheegWWEAQ91129063;     FheegWWEAQ91129063 = FheegWWEAQ703419;     FheegWWEAQ703419 = FheegWWEAQ19642008;     FheegWWEAQ19642008 = FheegWWEAQ35031320;     FheegWWEAQ35031320 = FheegWWEAQ64649258;     FheegWWEAQ64649258 = FheegWWEAQ23836225;     FheegWWEAQ23836225 = FheegWWEAQ26882040;     FheegWWEAQ26882040 = FheegWWEAQ80969650;     FheegWWEAQ80969650 = FheegWWEAQ94552106;     FheegWWEAQ94552106 = FheegWWEAQ70089;     FheegWWEAQ70089 = FheegWWEAQ89175072;     FheegWWEAQ89175072 = FheegWWEAQ39969615;     FheegWWEAQ39969615 = FheegWWEAQ60961499;     FheegWWEAQ60961499 = FheegWWEAQ4132866;     FheegWWEAQ4132866 = FheegWWEAQ55998602;     FheegWWEAQ55998602 = FheegWWEAQ53268796;     FheegWWEAQ53268796 = FheegWWEAQ35929363;     FheegWWEAQ35929363 = FheegWWEAQ21935619;     FheegWWEAQ21935619 = FheegWWEAQ83993809;     FheegWWEAQ83993809 = FheegWWEAQ49500788;     FheegWWEAQ49500788 = FheegWWEAQ23164365;     FheegWWEAQ23164365 = FheegWWEAQ24477081;     FheegWWEAQ24477081 = FheegWWEAQ33135005;     FheegWWEAQ33135005 = FheegWWEAQ61603491;     FheegWWEAQ61603491 = FheegWWEAQ5499027;     FheegWWEAQ5499027 = FheegWWEAQ52109491;     FheegWWEAQ52109491 = FheegWWEAQ83702645;     FheegWWEAQ83702645 = FheegWWEAQ50202847;     FheegWWEAQ50202847 = FheegWWEAQ55275111;     FheegWWEAQ55275111 = FheegWWEAQ46645602;     FheegWWEAQ46645602 = FheegWWEAQ76348491;     FheegWWEAQ76348491 = FheegWWEAQ53415966;     FheegWWEAQ53415966 = FheegWWEAQ89735203;     FheegWWEAQ89735203 = FheegWWEAQ39481240;     FheegWWEAQ39481240 = FheegWWEAQ5844649;     FheegWWEAQ5844649 = FheegWWEAQ77479294;     FheegWWEAQ77479294 = FheegWWEAQ93268144;     FheegWWEAQ93268144 = FheegWWEAQ67103224;     FheegWWEAQ67103224 = FheegWWEAQ76638098;     FheegWWEAQ76638098 = FheegWWEAQ62263491;     FheegWWEAQ62263491 = FheegWWEAQ62160552;     FheegWWEAQ62160552 = FheegWWEAQ36162616;     FheegWWEAQ36162616 = FheegWWEAQ42295848;     FheegWWEAQ42295848 = FheegWWEAQ75436959;     FheegWWEAQ75436959 = FheegWWEAQ96697675;     FheegWWEAQ96697675 = FheegWWEAQ820030;     FheegWWEAQ820030 = FheegWWEAQ80007269;     FheegWWEAQ80007269 = FheegWWEAQ79597826;     FheegWWEAQ79597826 = FheegWWEAQ14458369;     FheegWWEAQ14458369 = FheegWWEAQ57418827;     FheegWWEAQ57418827 = FheegWWEAQ22271972;     FheegWWEAQ22271972 = FheegWWEAQ81894801;     FheegWWEAQ81894801 = FheegWWEAQ11531186;     FheegWWEAQ11531186 = FheegWWEAQ7658822;     FheegWWEAQ7658822 = FheegWWEAQ90374320;     FheegWWEAQ90374320 = FheegWWEAQ66651982;     FheegWWEAQ66651982 = FheegWWEAQ67568413;     FheegWWEAQ67568413 = FheegWWEAQ58038516;     FheegWWEAQ58038516 = FheegWWEAQ29532293;     FheegWWEAQ29532293 = FheegWWEAQ12539767;     FheegWWEAQ12539767 = FheegWWEAQ40133580;     FheegWWEAQ40133580 = FheegWWEAQ76679192;     FheegWWEAQ76679192 = FheegWWEAQ25694539;     FheegWWEAQ25694539 = FheegWWEAQ47906504;     FheegWWEAQ47906504 = FheegWWEAQ23721598;     FheegWWEAQ23721598 = FheegWWEAQ35759106;     FheegWWEAQ35759106 = FheegWWEAQ50234412;     FheegWWEAQ50234412 = FheegWWEAQ21480260;     FheegWWEAQ21480260 = FheegWWEAQ98288216;     FheegWWEAQ98288216 = FheegWWEAQ78519307;     FheegWWEAQ78519307 = FheegWWEAQ60000651;     FheegWWEAQ60000651 = FheegWWEAQ68826139;     FheegWWEAQ68826139 = FheegWWEAQ45297521;     FheegWWEAQ45297521 = FheegWWEAQ21730319;     FheegWWEAQ21730319 = FheegWWEAQ87340235;     FheegWWEAQ87340235 = FheegWWEAQ87001748;     FheegWWEAQ87001748 = FheegWWEAQ82181232;     FheegWWEAQ82181232 = FheegWWEAQ46894812;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LoRBmngvhW67154303() {     int yzrVaWePxQ30339118 = -145987179;    int yzrVaWePxQ69892014 = -285418223;    int yzrVaWePxQ93382218 = -742056254;    int yzrVaWePxQ40974964 = -400675400;    int yzrVaWePxQ19031476 = -761714479;    int yzrVaWePxQ638945 = -385882384;    int yzrVaWePxQ12326674 = -763492190;    int yzrVaWePxQ447159 = -719048096;    int yzrVaWePxQ47572292 = -302468762;    int yzrVaWePxQ49677918 = 92356473;    int yzrVaWePxQ86263892 = -287275668;    int yzrVaWePxQ38712447 = -447442132;    int yzrVaWePxQ3990525 = -360541905;    int yzrVaWePxQ20416537 = -679270668;    int yzrVaWePxQ1823233 = -706497073;    int yzrVaWePxQ70119852 = -374799083;    int yzrVaWePxQ64208855 = -48263385;    int yzrVaWePxQ91539069 = 96124144;    int yzrVaWePxQ20641477 = -58379919;    int yzrVaWePxQ93197862 = -314437150;    int yzrVaWePxQ51165382 = -651246239;    int yzrVaWePxQ69306927 = -422282794;    int yzrVaWePxQ17364393 = -369564931;    int yzrVaWePxQ25835621 = -515383605;    int yzrVaWePxQ42279376 = -915855646;    int yzrVaWePxQ14984157 = 13895991;    int yzrVaWePxQ84486059 = -232754672;    int yzrVaWePxQ4380884 = 1645050;    int yzrVaWePxQ59917319 = -274600978;    int yzrVaWePxQ42715381 = -10279764;    int yzrVaWePxQ99064963 = -749947667;    int yzrVaWePxQ57658533 = -691690640;    int yzrVaWePxQ43608976 = -583773889;    int yzrVaWePxQ31333583 = -762423927;    int yzrVaWePxQ18696346 = 10909311;    int yzrVaWePxQ73350683 = -863564190;    int yzrVaWePxQ72747043 = -605462933;    int yzrVaWePxQ96577433 = -263657524;    int yzrVaWePxQ2263875 = -788581434;    int yzrVaWePxQ85424498 = -302311674;    int yzrVaWePxQ55584543 = -265328167;    int yzrVaWePxQ34481596 = -215046716;    int yzrVaWePxQ73067857 = -129110262;    int yzrVaWePxQ15069305 = -823565189;    int yzrVaWePxQ16421578 = 21581370;    int yzrVaWePxQ72741000 = -223504187;    int yzrVaWePxQ37618921 = -482253189;    int yzrVaWePxQ25169622 = -244029027;    int yzrVaWePxQ82956603 = -213120830;    int yzrVaWePxQ19035129 = -863213170;    int yzrVaWePxQ23460296 = -327710338;    int yzrVaWePxQ10775906 = 37345174;    int yzrVaWePxQ60758061 = -91299676;    int yzrVaWePxQ1025251 = -976542996;    int yzrVaWePxQ15988921 = -382775358;    int yzrVaWePxQ61032191 = -723704386;    int yzrVaWePxQ52527621 = -915853293;    int yzrVaWePxQ67546597 = -126672650;    int yzrVaWePxQ98695587 = -484819755;    int yzrVaWePxQ4047320 = -675610470;    int yzrVaWePxQ16152885 = -53127713;    int yzrVaWePxQ7945791 = -665137240;    int yzrVaWePxQ40529839 = -344447118;    int yzrVaWePxQ4856912 = -192188998;    int yzrVaWePxQ50612954 = -157695860;    int yzrVaWePxQ28605359 = -595585029;    int yzrVaWePxQ95103471 = -863668244;    int yzrVaWePxQ72656941 = -598117979;    int yzrVaWePxQ1720191 = -590179980;    int yzrVaWePxQ28472549 = -842932883;    int yzrVaWePxQ97372808 = -769336151;    int yzrVaWePxQ67631421 = -784605862;    int yzrVaWePxQ89275195 = -115294423;    int yzrVaWePxQ35216979 = -756068245;    int yzrVaWePxQ37613320 = 50891017;    int yzrVaWePxQ16683787 = -336199523;    int yzrVaWePxQ96239069 = -193172532;    int yzrVaWePxQ2295088 = -545999742;    int yzrVaWePxQ9414044 = -436964976;    int yzrVaWePxQ69538376 = -592351459;    int yzrVaWePxQ77365235 = -503850821;    int yzrVaWePxQ59316438 = -988725645;    int yzrVaWePxQ21424280 = -785234121;    int yzrVaWePxQ40882190 = -411387809;    int yzrVaWePxQ19255086 = -682569427;    int yzrVaWePxQ88289058 = -687292842;    int yzrVaWePxQ96900471 = -500390964;    int yzrVaWePxQ42583725 = -607230894;    int yzrVaWePxQ15344663 = -279648570;    int yzrVaWePxQ57664155 = -265386303;    int yzrVaWePxQ20823062 = -947710897;    int yzrVaWePxQ5200447 = -378790284;    int yzrVaWePxQ97881846 = -778837769;    int yzrVaWePxQ98216554 = -12970964;    int yzrVaWePxQ69271613 = -149183962;    int yzrVaWePxQ47638753 = -600190927;    int yzrVaWePxQ93951756 = -870599599;    int yzrVaWePxQ68210946 = -936921265;    int yzrVaWePxQ64456350 = -565869330;    int yzrVaWePxQ87816218 = -145987179;     yzrVaWePxQ30339118 = yzrVaWePxQ69892014;     yzrVaWePxQ69892014 = yzrVaWePxQ93382218;     yzrVaWePxQ93382218 = yzrVaWePxQ40974964;     yzrVaWePxQ40974964 = yzrVaWePxQ19031476;     yzrVaWePxQ19031476 = yzrVaWePxQ638945;     yzrVaWePxQ638945 = yzrVaWePxQ12326674;     yzrVaWePxQ12326674 = yzrVaWePxQ447159;     yzrVaWePxQ447159 = yzrVaWePxQ47572292;     yzrVaWePxQ47572292 = yzrVaWePxQ49677918;     yzrVaWePxQ49677918 = yzrVaWePxQ86263892;     yzrVaWePxQ86263892 = yzrVaWePxQ38712447;     yzrVaWePxQ38712447 = yzrVaWePxQ3990525;     yzrVaWePxQ3990525 = yzrVaWePxQ20416537;     yzrVaWePxQ20416537 = yzrVaWePxQ1823233;     yzrVaWePxQ1823233 = yzrVaWePxQ70119852;     yzrVaWePxQ70119852 = yzrVaWePxQ64208855;     yzrVaWePxQ64208855 = yzrVaWePxQ91539069;     yzrVaWePxQ91539069 = yzrVaWePxQ20641477;     yzrVaWePxQ20641477 = yzrVaWePxQ93197862;     yzrVaWePxQ93197862 = yzrVaWePxQ51165382;     yzrVaWePxQ51165382 = yzrVaWePxQ69306927;     yzrVaWePxQ69306927 = yzrVaWePxQ17364393;     yzrVaWePxQ17364393 = yzrVaWePxQ25835621;     yzrVaWePxQ25835621 = yzrVaWePxQ42279376;     yzrVaWePxQ42279376 = yzrVaWePxQ14984157;     yzrVaWePxQ14984157 = yzrVaWePxQ84486059;     yzrVaWePxQ84486059 = yzrVaWePxQ4380884;     yzrVaWePxQ4380884 = yzrVaWePxQ59917319;     yzrVaWePxQ59917319 = yzrVaWePxQ42715381;     yzrVaWePxQ42715381 = yzrVaWePxQ99064963;     yzrVaWePxQ99064963 = yzrVaWePxQ57658533;     yzrVaWePxQ57658533 = yzrVaWePxQ43608976;     yzrVaWePxQ43608976 = yzrVaWePxQ31333583;     yzrVaWePxQ31333583 = yzrVaWePxQ18696346;     yzrVaWePxQ18696346 = yzrVaWePxQ73350683;     yzrVaWePxQ73350683 = yzrVaWePxQ72747043;     yzrVaWePxQ72747043 = yzrVaWePxQ96577433;     yzrVaWePxQ96577433 = yzrVaWePxQ2263875;     yzrVaWePxQ2263875 = yzrVaWePxQ85424498;     yzrVaWePxQ85424498 = yzrVaWePxQ55584543;     yzrVaWePxQ55584543 = yzrVaWePxQ34481596;     yzrVaWePxQ34481596 = yzrVaWePxQ73067857;     yzrVaWePxQ73067857 = yzrVaWePxQ15069305;     yzrVaWePxQ15069305 = yzrVaWePxQ16421578;     yzrVaWePxQ16421578 = yzrVaWePxQ72741000;     yzrVaWePxQ72741000 = yzrVaWePxQ37618921;     yzrVaWePxQ37618921 = yzrVaWePxQ25169622;     yzrVaWePxQ25169622 = yzrVaWePxQ82956603;     yzrVaWePxQ82956603 = yzrVaWePxQ19035129;     yzrVaWePxQ19035129 = yzrVaWePxQ23460296;     yzrVaWePxQ23460296 = yzrVaWePxQ10775906;     yzrVaWePxQ10775906 = yzrVaWePxQ60758061;     yzrVaWePxQ60758061 = yzrVaWePxQ1025251;     yzrVaWePxQ1025251 = yzrVaWePxQ15988921;     yzrVaWePxQ15988921 = yzrVaWePxQ61032191;     yzrVaWePxQ61032191 = yzrVaWePxQ52527621;     yzrVaWePxQ52527621 = yzrVaWePxQ67546597;     yzrVaWePxQ67546597 = yzrVaWePxQ98695587;     yzrVaWePxQ98695587 = yzrVaWePxQ4047320;     yzrVaWePxQ4047320 = yzrVaWePxQ16152885;     yzrVaWePxQ16152885 = yzrVaWePxQ7945791;     yzrVaWePxQ7945791 = yzrVaWePxQ40529839;     yzrVaWePxQ40529839 = yzrVaWePxQ4856912;     yzrVaWePxQ4856912 = yzrVaWePxQ50612954;     yzrVaWePxQ50612954 = yzrVaWePxQ28605359;     yzrVaWePxQ28605359 = yzrVaWePxQ95103471;     yzrVaWePxQ95103471 = yzrVaWePxQ72656941;     yzrVaWePxQ72656941 = yzrVaWePxQ1720191;     yzrVaWePxQ1720191 = yzrVaWePxQ28472549;     yzrVaWePxQ28472549 = yzrVaWePxQ97372808;     yzrVaWePxQ97372808 = yzrVaWePxQ67631421;     yzrVaWePxQ67631421 = yzrVaWePxQ89275195;     yzrVaWePxQ89275195 = yzrVaWePxQ35216979;     yzrVaWePxQ35216979 = yzrVaWePxQ37613320;     yzrVaWePxQ37613320 = yzrVaWePxQ16683787;     yzrVaWePxQ16683787 = yzrVaWePxQ96239069;     yzrVaWePxQ96239069 = yzrVaWePxQ2295088;     yzrVaWePxQ2295088 = yzrVaWePxQ9414044;     yzrVaWePxQ9414044 = yzrVaWePxQ69538376;     yzrVaWePxQ69538376 = yzrVaWePxQ77365235;     yzrVaWePxQ77365235 = yzrVaWePxQ59316438;     yzrVaWePxQ59316438 = yzrVaWePxQ21424280;     yzrVaWePxQ21424280 = yzrVaWePxQ40882190;     yzrVaWePxQ40882190 = yzrVaWePxQ19255086;     yzrVaWePxQ19255086 = yzrVaWePxQ88289058;     yzrVaWePxQ88289058 = yzrVaWePxQ96900471;     yzrVaWePxQ96900471 = yzrVaWePxQ42583725;     yzrVaWePxQ42583725 = yzrVaWePxQ15344663;     yzrVaWePxQ15344663 = yzrVaWePxQ57664155;     yzrVaWePxQ57664155 = yzrVaWePxQ20823062;     yzrVaWePxQ20823062 = yzrVaWePxQ5200447;     yzrVaWePxQ5200447 = yzrVaWePxQ97881846;     yzrVaWePxQ97881846 = yzrVaWePxQ98216554;     yzrVaWePxQ98216554 = yzrVaWePxQ69271613;     yzrVaWePxQ69271613 = yzrVaWePxQ47638753;     yzrVaWePxQ47638753 = yzrVaWePxQ93951756;     yzrVaWePxQ93951756 = yzrVaWePxQ68210946;     yzrVaWePxQ68210946 = yzrVaWePxQ64456350;     yzrVaWePxQ64456350 = yzrVaWePxQ87816218;     yzrVaWePxQ87816218 = yzrVaWePxQ30339118;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OzIpQXrbmQ37610176() {     int ysYrtCrojt43125285 = -281027871;    int ysYrtCrojt56662902 = -455752237;    int ysYrtCrojt11135019 = -50415636;    int ysYrtCrojt48915790 = -224877677;    int ysYrtCrojt12542581 = -153530944;    int ysYrtCrojt43949015 = -162805698;    int ysYrtCrojt33534432 = -209164261;    int ysYrtCrojt27166271 = -63566353;    int ysYrtCrojt42977103 = -264226854;    int ysYrtCrojt55896623 = -811731159;    int ysYrtCrojt13751275 = -401961684;    int ysYrtCrojt70662952 = -954774948;    int ysYrtCrojt74133827 = -264324562;    int ysYrtCrojt70040615 = 88314134;    int ysYrtCrojt25129785 = -613482903;    int ysYrtCrojt98544921 = -925690473;    int ysYrtCrojt19960673 = -812037851;    int ysYrtCrojt75097554 = -559086159;    int ysYrtCrojt21802108 = 32150886;    int ysYrtCrojt95852358 = -913681289;    int ysYrtCrojt31339439 = -42562503;    int ysYrtCrojt98354746 = -592141142;    int ysYrtCrojt56124688 = -300568825;    int ysYrtCrojt70342751 = -875728618;    int ysYrtCrojt48173635 = -26297570;    int ysYrtCrojt96302801 = -776535208;    int ysYrtCrojt95901596 = -866460952;    int ysYrtCrojt71733808 = -172153055;    int ysYrtCrojt76534372 = -488818219;    int ysYrtCrojt95466496 = -740165919;    int ysYrtCrojt43500960 = 73635583;    int ysYrtCrojt60977357 = -670410318;    int ysYrtCrojt89654924 = -459628673;    int ysYrtCrojt8409956 = -667178145;    int ysYrtCrojt1756878 = -599122569;    int ysYrtCrojt68574798 = -652127294;    int ysYrtCrojt9511104 = -395372283;    int ysYrtCrojt54281457 = -183277615;    int ysYrtCrojt8039260 = -630432011;    int ysYrtCrojt11689228 = -146728520;    int ysYrtCrojt17015851 = -640802160;    int ysYrtCrojt42184052 = -113358050;    int ysYrtCrojt7951327 = -152543707;    int ysYrtCrojt59697842 = -64737432;    int ysYrtCrojt96430068 = -971188590;    int ysYrtCrojt46369130 = -296822627;    int ysYrtCrojt15200941 = -979768241;    int ysYrtCrojt54609589 = -399381807;    int ysYrtCrojt27020653 = -376722804;    int ysYrtCrojt65985549 = -374249632;    int ysYrtCrojt41787789 = -580594647;    int ysYrtCrojt38952453 = -433840164;    int ysYrtCrojt95243211 = -267302959;    int ysYrtCrojt45724976 = -565994738;    int ysYrtCrojt6270737 = -377219844;    int ysYrtCrojt44770539 = -688886729;    int ysYrtCrojt538215 = -55183412;    int ysYrtCrojt40792268 = -174687019;    int ysYrtCrojt742155 = -98580108;    int ysYrtCrojt16239779 = -376995736;    int ysYrtCrojt48047419 = -296344746;    int ysYrtCrojt61800623 = 62988794;    int ysYrtCrojt50631899 = -574748135;    int ysYrtCrojt47510606 = -524060935;    int ysYrtCrojt12395663 = -785366743;    int ysYrtCrojt52773918 = -731551366;    int ysYrtCrojt81008027 = -395146275;    int ysYrtCrojt65723871 = -597146418;    int ysYrtCrojt68283737 = -312563298;    int ysYrtCrojt56554987 = -961355610;    int ysYrtCrojt89033817 = -430318190;    int ysYrtCrojt65679216 = -528760237;    int ysYrtCrojt67058294 = -928654148;    int ysYrtCrojt10112880 = -821120595;    int ysYrtCrojt78836507 = -172879129;    int ysYrtCrojt89155387 = -929204453;    int ysYrtCrojt90403419 = -339597435;    int ysYrtCrojt96426845 = -135831394;    int ysYrtCrojt73912682 = -904540028;    int ysYrtCrojt1804506 = -729474944;    int ysYrtCrojt81101861 = -796766968;    int ysYrtCrojt41292008 = -367079146;    int ysYrtCrojt44713155 = -795430251;    int ysYrtCrojt10548823 = -14568587;    int ysYrtCrojt53678708 = -59571273;    int ysYrtCrojt4548508 = -492524254;    int ysYrtCrojt65734145 = -303107360;    int ysYrtCrojt43929949 = -893633936;    int ysYrtCrojt2139220 = -189958302;    int ysYrtCrojt56986339 = -910235840;    int ysYrtCrojt68036584 = -496943882;    int ysYrtCrojt68718836 = -120685265;    int ysYrtCrojt53539303 = 15302492;    int ysYrtCrojt91799481 = -153436275;    int ysYrtCrojt63641809 = -850383775;    int ysYrtCrojt55215227 = -603790955;    int ysYrtCrojt91552152 = -538609916;    int ysYrtCrojt60440721 = -628482772;    int ysYrtCrojt47302180 = -279370690;    int ysYrtCrojt43656150 = -281027871;     ysYrtCrojt43125285 = ysYrtCrojt56662902;     ysYrtCrojt56662902 = ysYrtCrojt11135019;     ysYrtCrojt11135019 = ysYrtCrojt48915790;     ysYrtCrojt48915790 = ysYrtCrojt12542581;     ysYrtCrojt12542581 = ysYrtCrojt43949015;     ysYrtCrojt43949015 = ysYrtCrojt33534432;     ysYrtCrojt33534432 = ysYrtCrojt27166271;     ysYrtCrojt27166271 = ysYrtCrojt42977103;     ysYrtCrojt42977103 = ysYrtCrojt55896623;     ysYrtCrojt55896623 = ysYrtCrojt13751275;     ysYrtCrojt13751275 = ysYrtCrojt70662952;     ysYrtCrojt70662952 = ysYrtCrojt74133827;     ysYrtCrojt74133827 = ysYrtCrojt70040615;     ysYrtCrojt70040615 = ysYrtCrojt25129785;     ysYrtCrojt25129785 = ysYrtCrojt98544921;     ysYrtCrojt98544921 = ysYrtCrojt19960673;     ysYrtCrojt19960673 = ysYrtCrojt75097554;     ysYrtCrojt75097554 = ysYrtCrojt21802108;     ysYrtCrojt21802108 = ysYrtCrojt95852358;     ysYrtCrojt95852358 = ysYrtCrojt31339439;     ysYrtCrojt31339439 = ysYrtCrojt98354746;     ysYrtCrojt98354746 = ysYrtCrojt56124688;     ysYrtCrojt56124688 = ysYrtCrojt70342751;     ysYrtCrojt70342751 = ysYrtCrojt48173635;     ysYrtCrojt48173635 = ysYrtCrojt96302801;     ysYrtCrojt96302801 = ysYrtCrojt95901596;     ysYrtCrojt95901596 = ysYrtCrojt71733808;     ysYrtCrojt71733808 = ysYrtCrojt76534372;     ysYrtCrojt76534372 = ysYrtCrojt95466496;     ysYrtCrojt95466496 = ysYrtCrojt43500960;     ysYrtCrojt43500960 = ysYrtCrojt60977357;     ysYrtCrojt60977357 = ysYrtCrojt89654924;     ysYrtCrojt89654924 = ysYrtCrojt8409956;     ysYrtCrojt8409956 = ysYrtCrojt1756878;     ysYrtCrojt1756878 = ysYrtCrojt68574798;     ysYrtCrojt68574798 = ysYrtCrojt9511104;     ysYrtCrojt9511104 = ysYrtCrojt54281457;     ysYrtCrojt54281457 = ysYrtCrojt8039260;     ysYrtCrojt8039260 = ysYrtCrojt11689228;     ysYrtCrojt11689228 = ysYrtCrojt17015851;     ysYrtCrojt17015851 = ysYrtCrojt42184052;     ysYrtCrojt42184052 = ysYrtCrojt7951327;     ysYrtCrojt7951327 = ysYrtCrojt59697842;     ysYrtCrojt59697842 = ysYrtCrojt96430068;     ysYrtCrojt96430068 = ysYrtCrojt46369130;     ysYrtCrojt46369130 = ysYrtCrojt15200941;     ysYrtCrojt15200941 = ysYrtCrojt54609589;     ysYrtCrojt54609589 = ysYrtCrojt27020653;     ysYrtCrojt27020653 = ysYrtCrojt65985549;     ysYrtCrojt65985549 = ysYrtCrojt41787789;     ysYrtCrojt41787789 = ysYrtCrojt38952453;     ysYrtCrojt38952453 = ysYrtCrojt95243211;     ysYrtCrojt95243211 = ysYrtCrojt45724976;     ysYrtCrojt45724976 = ysYrtCrojt6270737;     ysYrtCrojt6270737 = ysYrtCrojt44770539;     ysYrtCrojt44770539 = ysYrtCrojt538215;     ysYrtCrojt538215 = ysYrtCrojt40792268;     ysYrtCrojt40792268 = ysYrtCrojt742155;     ysYrtCrojt742155 = ysYrtCrojt16239779;     ysYrtCrojt16239779 = ysYrtCrojt48047419;     ysYrtCrojt48047419 = ysYrtCrojt61800623;     ysYrtCrojt61800623 = ysYrtCrojt50631899;     ysYrtCrojt50631899 = ysYrtCrojt47510606;     ysYrtCrojt47510606 = ysYrtCrojt12395663;     ysYrtCrojt12395663 = ysYrtCrojt52773918;     ysYrtCrojt52773918 = ysYrtCrojt81008027;     ysYrtCrojt81008027 = ysYrtCrojt65723871;     ysYrtCrojt65723871 = ysYrtCrojt68283737;     ysYrtCrojt68283737 = ysYrtCrojt56554987;     ysYrtCrojt56554987 = ysYrtCrojt89033817;     ysYrtCrojt89033817 = ysYrtCrojt65679216;     ysYrtCrojt65679216 = ysYrtCrojt67058294;     ysYrtCrojt67058294 = ysYrtCrojt10112880;     ysYrtCrojt10112880 = ysYrtCrojt78836507;     ysYrtCrojt78836507 = ysYrtCrojt89155387;     ysYrtCrojt89155387 = ysYrtCrojt90403419;     ysYrtCrojt90403419 = ysYrtCrojt96426845;     ysYrtCrojt96426845 = ysYrtCrojt73912682;     ysYrtCrojt73912682 = ysYrtCrojt1804506;     ysYrtCrojt1804506 = ysYrtCrojt81101861;     ysYrtCrojt81101861 = ysYrtCrojt41292008;     ysYrtCrojt41292008 = ysYrtCrojt44713155;     ysYrtCrojt44713155 = ysYrtCrojt10548823;     ysYrtCrojt10548823 = ysYrtCrojt53678708;     ysYrtCrojt53678708 = ysYrtCrojt4548508;     ysYrtCrojt4548508 = ysYrtCrojt65734145;     ysYrtCrojt65734145 = ysYrtCrojt43929949;     ysYrtCrojt43929949 = ysYrtCrojt2139220;     ysYrtCrojt2139220 = ysYrtCrojt56986339;     ysYrtCrojt56986339 = ysYrtCrojt68036584;     ysYrtCrojt68036584 = ysYrtCrojt68718836;     ysYrtCrojt68718836 = ysYrtCrojt53539303;     ysYrtCrojt53539303 = ysYrtCrojt91799481;     ysYrtCrojt91799481 = ysYrtCrojt63641809;     ysYrtCrojt63641809 = ysYrtCrojt55215227;     ysYrtCrojt55215227 = ysYrtCrojt91552152;     ysYrtCrojt91552152 = ysYrtCrojt60440721;     ysYrtCrojt60440721 = ysYrtCrojt47302180;     ysYrtCrojt47302180 = ysYrtCrojt43656150;     ysYrtCrojt43656150 = ysYrtCrojt43125285;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ICgXIwDmwh38052279() {     int uJpvewOcke78305040 = -177948384;    int uJpvewOcke89208974 = -532034180;    int uJpvewOcke56343885 = -741300248;    int uJpvewOcke86783442 = -234946655;    int uJpvewOcke95387837 = -768049089;    int uJpvewOcke67336770 = -100904577;    int uJpvewOcke78350592 = -872287170;    int uJpvewOcke60305192 = -174503937;    int uJpvewOcke52804880 = -396534915;    int uJpvewOcke57239399 = -721753254;    int uJpvewOcke58459911 = -553800028;    int uJpvewOcke44224729 = -877374695;    int uJpvewOcke79432396 = -765166065;    int uJpvewOcke8435226 = -426656770;    int uJpvewOcke39999506 = -185233595;    int uJpvewOcke96661355 = -425453754;    int uJpvewOcke69683082 = -933477568;    int uJpvewOcke41631923 = -168967530;    int uJpvewOcke43816099 = -494438168;    int uJpvewOcke4949376 = -53535430;    int uJpvewOcke32727179 = -814899702;    int uJpvewOcke12921518 = -756773920;    int uJpvewOcke61299910 = 51304530;    int uJpvewOcke97699559 = -819145965;    int uJpvewOcke61165452 = -531604766;    int uJpvewOcke49350365 = 97345297;    int uJpvewOcke84184971 = -956954012;    int uJpvewOcke9594659 = -625438625;    int uJpvewOcke87365539 = -401222263;    int uJpvewOcke81284921 = -614916567;    int uJpvewOcke98815082 = -997004085;    int uJpvewOcke37113514 = -909379590;    int uJpvewOcke31287121 = -662882737;    int uJpvewOcke44592179 = 80622519;    int uJpvewOcke78688736 = 86403053;    int uJpvewOcke62663592 = -676320028;    int uJpvewOcke27773509 = -918576207;    int uJpvewOcke95693900 = 24739500;    int uJpvewOcke26809181 = -160647204;    int uJpvewOcke86266779 = -517937709;    int uJpvewOcke19348863 = -331509921;    int uJpvewOcke52697238 = -675785993;    int uJpvewOcke88101628 = -604377481;    int uJpvewOcke44538601 = -846308881;    int uJpvewOcke60403773 = -199842627;    int uJpvewOcke87669572 = -740325491;    int uJpvewOcke77382565 = -474412924;    int uJpvewOcke53264649 = -802508221;    int uJpvewOcke46886218 = -926277616;    int uJpvewOcke89974292 = -440281468;    int uJpvewOcke55725129 = -510000225;    int uJpvewOcke26433000 = -953699035;    int uJpvewOcke32594417 = -223163463;    int uJpvewOcke40460975 = -999207437;    int uJpvewOcke5325100 = -755469168;    int uJpvewOcke65383523 = -421174465;    int uJpvewOcke27909064 = -483338710;    int uJpvewOcke58644326 = -922154284;    int uJpvewOcke25617991 = -703341890;    int uJpvewOcke46037473 = -765394386;    int uJpvewOcke83151799 = -143950565;    int uJpvewOcke68755934 = -146848546;    int uJpvewOcke72939652 = -773281674;    int uJpvewOcke71519959 = -781618349;    int uJpvewOcke58424316 = -724749170;    int uJpvewOcke21346398 = -644420439;    int uJpvewOcke12937608 = -114491958;    int uJpvewOcke34840218 = -745788584;    int uJpvewOcke29746489 = -413059823;    int uJpvewOcke77335913 = -508913568;    int uJpvewOcke68887846 = -506877547;    int uJpvewOcke73989181 = -858217069;    int uJpvewOcke14822743 = 91679673;    int uJpvewOcke57549320 = -976500460;    int uJpvewOcke85600513 = -722025509;    int uJpvewOcke80029941 = -39113709;    int uJpvewOcke24819889 = -52396439;    int uJpvewOcke16761309 = -102386590;    int uJpvewOcke37295787 = -519303339;    int uJpvewOcke73495879 = -791279276;    int uJpvewOcke71967799 = -428241779;    int uJpvewOcke30920323 = -54445792;    int uJpvewOcke62708440 = -699161010;    int uJpvewOcke97391246 = -960940796;    int uJpvewOcke25559793 = -4916342;    int uJpvewOcke72382082 = 56694950;    int uJpvewOcke4519098 = -586216127;    int uJpvewOcke90826145 = -663675300;    int uJpvewOcke39267079 = -163908314;    int uJpvewOcke13305214 = -492422483;    int uJpvewOcke34754529 = -92981318;    int uJpvewOcke69129183 = -996421923;    int uJpvewOcke70075910 = -271918610;    int uJpvewOcke80771708 = -395252818;    int uJpvewOcke3114980 = -273987144;    int uJpvewOcke50592928 = -84661376;    int uJpvewOcke79757585 = -902504320;    int uJpvewOcke16581670 = -822759132;    int uJpvewOcke86114285 = -21559711;    int uJpvewOcke39057376 = -177948384;     uJpvewOcke78305040 = uJpvewOcke89208974;     uJpvewOcke89208974 = uJpvewOcke56343885;     uJpvewOcke56343885 = uJpvewOcke86783442;     uJpvewOcke86783442 = uJpvewOcke95387837;     uJpvewOcke95387837 = uJpvewOcke67336770;     uJpvewOcke67336770 = uJpvewOcke78350592;     uJpvewOcke78350592 = uJpvewOcke60305192;     uJpvewOcke60305192 = uJpvewOcke52804880;     uJpvewOcke52804880 = uJpvewOcke57239399;     uJpvewOcke57239399 = uJpvewOcke58459911;     uJpvewOcke58459911 = uJpvewOcke44224729;     uJpvewOcke44224729 = uJpvewOcke79432396;     uJpvewOcke79432396 = uJpvewOcke8435226;     uJpvewOcke8435226 = uJpvewOcke39999506;     uJpvewOcke39999506 = uJpvewOcke96661355;     uJpvewOcke96661355 = uJpvewOcke69683082;     uJpvewOcke69683082 = uJpvewOcke41631923;     uJpvewOcke41631923 = uJpvewOcke43816099;     uJpvewOcke43816099 = uJpvewOcke4949376;     uJpvewOcke4949376 = uJpvewOcke32727179;     uJpvewOcke32727179 = uJpvewOcke12921518;     uJpvewOcke12921518 = uJpvewOcke61299910;     uJpvewOcke61299910 = uJpvewOcke97699559;     uJpvewOcke97699559 = uJpvewOcke61165452;     uJpvewOcke61165452 = uJpvewOcke49350365;     uJpvewOcke49350365 = uJpvewOcke84184971;     uJpvewOcke84184971 = uJpvewOcke9594659;     uJpvewOcke9594659 = uJpvewOcke87365539;     uJpvewOcke87365539 = uJpvewOcke81284921;     uJpvewOcke81284921 = uJpvewOcke98815082;     uJpvewOcke98815082 = uJpvewOcke37113514;     uJpvewOcke37113514 = uJpvewOcke31287121;     uJpvewOcke31287121 = uJpvewOcke44592179;     uJpvewOcke44592179 = uJpvewOcke78688736;     uJpvewOcke78688736 = uJpvewOcke62663592;     uJpvewOcke62663592 = uJpvewOcke27773509;     uJpvewOcke27773509 = uJpvewOcke95693900;     uJpvewOcke95693900 = uJpvewOcke26809181;     uJpvewOcke26809181 = uJpvewOcke86266779;     uJpvewOcke86266779 = uJpvewOcke19348863;     uJpvewOcke19348863 = uJpvewOcke52697238;     uJpvewOcke52697238 = uJpvewOcke88101628;     uJpvewOcke88101628 = uJpvewOcke44538601;     uJpvewOcke44538601 = uJpvewOcke60403773;     uJpvewOcke60403773 = uJpvewOcke87669572;     uJpvewOcke87669572 = uJpvewOcke77382565;     uJpvewOcke77382565 = uJpvewOcke53264649;     uJpvewOcke53264649 = uJpvewOcke46886218;     uJpvewOcke46886218 = uJpvewOcke89974292;     uJpvewOcke89974292 = uJpvewOcke55725129;     uJpvewOcke55725129 = uJpvewOcke26433000;     uJpvewOcke26433000 = uJpvewOcke32594417;     uJpvewOcke32594417 = uJpvewOcke40460975;     uJpvewOcke40460975 = uJpvewOcke5325100;     uJpvewOcke5325100 = uJpvewOcke65383523;     uJpvewOcke65383523 = uJpvewOcke27909064;     uJpvewOcke27909064 = uJpvewOcke58644326;     uJpvewOcke58644326 = uJpvewOcke25617991;     uJpvewOcke25617991 = uJpvewOcke46037473;     uJpvewOcke46037473 = uJpvewOcke83151799;     uJpvewOcke83151799 = uJpvewOcke68755934;     uJpvewOcke68755934 = uJpvewOcke72939652;     uJpvewOcke72939652 = uJpvewOcke71519959;     uJpvewOcke71519959 = uJpvewOcke58424316;     uJpvewOcke58424316 = uJpvewOcke21346398;     uJpvewOcke21346398 = uJpvewOcke12937608;     uJpvewOcke12937608 = uJpvewOcke34840218;     uJpvewOcke34840218 = uJpvewOcke29746489;     uJpvewOcke29746489 = uJpvewOcke77335913;     uJpvewOcke77335913 = uJpvewOcke68887846;     uJpvewOcke68887846 = uJpvewOcke73989181;     uJpvewOcke73989181 = uJpvewOcke14822743;     uJpvewOcke14822743 = uJpvewOcke57549320;     uJpvewOcke57549320 = uJpvewOcke85600513;     uJpvewOcke85600513 = uJpvewOcke80029941;     uJpvewOcke80029941 = uJpvewOcke24819889;     uJpvewOcke24819889 = uJpvewOcke16761309;     uJpvewOcke16761309 = uJpvewOcke37295787;     uJpvewOcke37295787 = uJpvewOcke73495879;     uJpvewOcke73495879 = uJpvewOcke71967799;     uJpvewOcke71967799 = uJpvewOcke30920323;     uJpvewOcke30920323 = uJpvewOcke62708440;     uJpvewOcke62708440 = uJpvewOcke97391246;     uJpvewOcke97391246 = uJpvewOcke25559793;     uJpvewOcke25559793 = uJpvewOcke72382082;     uJpvewOcke72382082 = uJpvewOcke4519098;     uJpvewOcke4519098 = uJpvewOcke90826145;     uJpvewOcke90826145 = uJpvewOcke39267079;     uJpvewOcke39267079 = uJpvewOcke13305214;     uJpvewOcke13305214 = uJpvewOcke34754529;     uJpvewOcke34754529 = uJpvewOcke69129183;     uJpvewOcke69129183 = uJpvewOcke70075910;     uJpvewOcke70075910 = uJpvewOcke80771708;     uJpvewOcke80771708 = uJpvewOcke3114980;     uJpvewOcke3114980 = uJpvewOcke50592928;     uJpvewOcke50592928 = uJpvewOcke79757585;     uJpvewOcke79757585 = uJpvewOcke16581670;     uJpvewOcke16581670 = uJpvewOcke86114285;     uJpvewOcke86114285 = uJpvewOcke39057376;     uJpvewOcke39057376 = uJpvewOcke78305040;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zOzURzdjLt56265620() {     int SMzArnJMlw61749346 = 41385935;    int SMzArnJMlw6081064 = -531416368;    int SMzArnJMlw52752392 = -534825476;    int SMzArnJMlw49575694 = -428746491;    int SMzArnJMlw1509161 = -574916580;    int SMzArnJMlw65841171 = -979976230;    int SMzArnJMlw49389911 = -612198484;    int SMzArnJMlw74652634 = -161661966;    int SMzArnJMlw11334581 = -737994266;    int SMzArnJMlw89785050 = -90129366;    int SMzArnJMlw7875849 = -743915901;    int SMzArnJMlw7430128 = -864992941;    int SMzArnJMlw97550172 = -556827306;    int SMzArnJMlw88062117 = -214947127;    int SMzArnJMlw853970 = -279256578;    int SMzArnJMlw83050517 = -213533079;    int SMzArnJMlw63434967 = -686822595;    int SMzArnJMlw22483370 = -416272406;    int SMzArnJMlw6256242 = -493113039;    int SMzArnJMlw94316817 = -583121422;    int SMzArnJMlw88367566 = -404428733;    int SMzArnJMlw25068836 = -747925690;    int SMzArnJMlw65125618 = 11415333;    int SMzArnJMlw32406118 = -890971361;    int SMzArnJMlw2741410 = 8742475;    int SMzArnJMlw44692514 = -883164419;    int SMzArnJMlw33639711 = -585038355;    int SMzArnJMlw49326284 = -695393509;    int SMzArnJMlw23446633 = -263727405;    int SMzArnJMlw97118262 = -27766418;    int SMzArnJMlw16910396 = -634761286;    int SMzArnJMlw219941 = -824574670;    int SMzArnJMlw74826008 = -283754914;    int SMzArnJMlw86750689 = -610979651;    int SMzArnJMlw57415467 = -111261682;    int SMzArnJMlw75052777 = -464343934;    int SMzArnJMlw96387687 = -797425387;    int SMzArnJMlw36272733 = -783731020;    int SMzArnJMlw75804259 = -345545001;    int SMzArnJMlw35761915 = -270531231;    int SMzArnJMlw52997787 = 96941105;    int SMzArnJMlw3185024 = -149694315;    int SMzArnJMlw11668699 = -55434724;    int SMzArnJMlw36443541 = -602491652;    int SMzArnJMlw52348270 = 72000420;    int SMzArnJMlw27275569 = -759936414;    int SMzArnJMlw53397995 = -606717154;    int SMzArnJMlw72935244 = -801229940;    int SMzArnJMlw77733330 = -411879698;    int SMzArnJMlw25306777 = -647301923;    int SMzArnJMlw28982578 = -730901647;    int SMzArnJMlw81933794 = 88041653;    int SMzArnJMlw46706875 = -434910780;    int SMzArnJMlw65137734 = -717620825;    int SMzArnJMlw67898054 = -3955291;    int SMzArnJMlw36680510 = -210688375;    int SMzArnJMlw40955446 = -442831701;    int SMzArnJMlw20346275 = -643854116;    int SMzArnJMlw46834284 = -337488967;    int SMzArnJMlw56816647 = -691752161;    int SMzArnJMlw32201460 = -294937876;    int SMzArnJMlw63627 = -916804975;    int SMzArnJMlw51206001 = -897934561;    int SMzArnJMlw14216319 = -610227849;    int SMzArnJMlw72874654 = -455368081;    int SMzArnJMlw7655908 = -919341231;    int SMzArnJMlw32604120 = -481238027;    int SMzArnJMlw10799484 = -945847655;    int SMzArnJMlw30646651 = -3685445;    int SMzArnJMlw25801193 = -814912644;    int SMzArnJMlw86662829 = -416107693;    int SMzArnJMlw27162235 = -903091576;    int SMzArnJMlw46679111 = 29272595;    int SMzArnJMlw70494327 = -122581808;    int SMzArnJMlw41319031 = -580062528;    int SMzArnJMlw85182542 = -154734419;    int SMzArnJMlw13400138 = -592490967;    int SMzArnJMlw28682077 = -386093016;    int SMzArnJMlw80057848 = -862971781;    int SMzArnJMlw75465841 = -231321111;    int SMzArnJMlw91294518 = -176447265;    int SMzArnJMlw60704467 = -783808416;    int SMzArnJMlw71592953 = -183513811;    int SMzArnJMlw98139856 = -616425483;    int SMzArnJMlw68135685 = -296864772;    int SMzArnJMlw34976602 = -622802939;    int SMzArnJMlw53513065 = -289663890;    int SMzArnJMlw9688274 = -566134090;    int SMzArnJMlw18852635 = -507024361;    int SMzArnJMlw20734957 = -900573307;    int SMzArnJMlw34097332 = 78487766;    int SMzArnJMlw76041413 = -53571271;    int SMzArnJMlw89438448 = -346242053;    int SMzArnJMlw18987612 = -653792840;    int SMzArnJMlw3560455 = -975593356;    int SMzArnJMlw52934160 = 13746080;    int SMzArnJMlw51979023 = -251759754;    int SMzArnJMlw97452380 = -445206876;    int SMzArnJMlw63568887 = -47123571;    int SMzArnJMlw44692362 = 41385935;     SMzArnJMlw61749346 = SMzArnJMlw6081064;     SMzArnJMlw6081064 = SMzArnJMlw52752392;     SMzArnJMlw52752392 = SMzArnJMlw49575694;     SMzArnJMlw49575694 = SMzArnJMlw1509161;     SMzArnJMlw1509161 = SMzArnJMlw65841171;     SMzArnJMlw65841171 = SMzArnJMlw49389911;     SMzArnJMlw49389911 = SMzArnJMlw74652634;     SMzArnJMlw74652634 = SMzArnJMlw11334581;     SMzArnJMlw11334581 = SMzArnJMlw89785050;     SMzArnJMlw89785050 = SMzArnJMlw7875849;     SMzArnJMlw7875849 = SMzArnJMlw7430128;     SMzArnJMlw7430128 = SMzArnJMlw97550172;     SMzArnJMlw97550172 = SMzArnJMlw88062117;     SMzArnJMlw88062117 = SMzArnJMlw853970;     SMzArnJMlw853970 = SMzArnJMlw83050517;     SMzArnJMlw83050517 = SMzArnJMlw63434967;     SMzArnJMlw63434967 = SMzArnJMlw22483370;     SMzArnJMlw22483370 = SMzArnJMlw6256242;     SMzArnJMlw6256242 = SMzArnJMlw94316817;     SMzArnJMlw94316817 = SMzArnJMlw88367566;     SMzArnJMlw88367566 = SMzArnJMlw25068836;     SMzArnJMlw25068836 = SMzArnJMlw65125618;     SMzArnJMlw65125618 = SMzArnJMlw32406118;     SMzArnJMlw32406118 = SMzArnJMlw2741410;     SMzArnJMlw2741410 = SMzArnJMlw44692514;     SMzArnJMlw44692514 = SMzArnJMlw33639711;     SMzArnJMlw33639711 = SMzArnJMlw49326284;     SMzArnJMlw49326284 = SMzArnJMlw23446633;     SMzArnJMlw23446633 = SMzArnJMlw97118262;     SMzArnJMlw97118262 = SMzArnJMlw16910396;     SMzArnJMlw16910396 = SMzArnJMlw219941;     SMzArnJMlw219941 = SMzArnJMlw74826008;     SMzArnJMlw74826008 = SMzArnJMlw86750689;     SMzArnJMlw86750689 = SMzArnJMlw57415467;     SMzArnJMlw57415467 = SMzArnJMlw75052777;     SMzArnJMlw75052777 = SMzArnJMlw96387687;     SMzArnJMlw96387687 = SMzArnJMlw36272733;     SMzArnJMlw36272733 = SMzArnJMlw75804259;     SMzArnJMlw75804259 = SMzArnJMlw35761915;     SMzArnJMlw35761915 = SMzArnJMlw52997787;     SMzArnJMlw52997787 = SMzArnJMlw3185024;     SMzArnJMlw3185024 = SMzArnJMlw11668699;     SMzArnJMlw11668699 = SMzArnJMlw36443541;     SMzArnJMlw36443541 = SMzArnJMlw52348270;     SMzArnJMlw52348270 = SMzArnJMlw27275569;     SMzArnJMlw27275569 = SMzArnJMlw53397995;     SMzArnJMlw53397995 = SMzArnJMlw72935244;     SMzArnJMlw72935244 = SMzArnJMlw77733330;     SMzArnJMlw77733330 = SMzArnJMlw25306777;     SMzArnJMlw25306777 = SMzArnJMlw28982578;     SMzArnJMlw28982578 = SMzArnJMlw81933794;     SMzArnJMlw81933794 = SMzArnJMlw46706875;     SMzArnJMlw46706875 = SMzArnJMlw65137734;     SMzArnJMlw65137734 = SMzArnJMlw67898054;     SMzArnJMlw67898054 = SMzArnJMlw36680510;     SMzArnJMlw36680510 = SMzArnJMlw40955446;     SMzArnJMlw40955446 = SMzArnJMlw20346275;     SMzArnJMlw20346275 = SMzArnJMlw46834284;     SMzArnJMlw46834284 = SMzArnJMlw56816647;     SMzArnJMlw56816647 = SMzArnJMlw32201460;     SMzArnJMlw32201460 = SMzArnJMlw63627;     SMzArnJMlw63627 = SMzArnJMlw51206001;     SMzArnJMlw51206001 = SMzArnJMlw14216319;     SMzArnJMlw14216319 = SMzArnJMlw72874654;     SMzArnJMlw72874654 = SMzArnJMlw7655908;     SMzArnJMlw7655908 = SMzArnJMlw32604120;     SMzArnJMlw32604120 = SMzArnJMlw10799484;     SMzArnJMlw10799484 = SMzArnJMlw30646651;     SMzArnJMlw30646651 = SMzArnJMlw25801193;     SMzArnJMlw25801193 = SMzArnJMlw86662829;     SMzArnJMlw86662829 = SMzArnJMlw27162235;     SMzArnJMlw27162235 = SMzArnJMlw46679111;     SMzArnJMlw46679111 = SMzArnJMlw70494327;     SMzArnJMlw70494327 = SMzArnJMlw41319031;     SMzArnJMlw41319031 = SMzArnJMlw85182542;     SMzArnJMlw85182542 = SMzArnJMlw13400138;     SMzArnJMlw13400138 = SMzArnJMlw28682077;     SMzArnJMlw28682077 = SMzArnJMlw80057848;     SMzArnJMlw80057848 = SMzArnJMlw75465841;     SMzArnJMlw75465841 = SMzArnJMlw91294518;     SMzArnJMlw91294518 = SMzArnJMlw60704467;     SMzArnJMlw60704467 = SMzArnJMlw71592953;     SMzArnJMlw71592953 = SMzArnJMlw98139856;     SMzArnJMlw98139856 = SMzArnJMlw68135685;     SMzArnJMlw68135685 = SMzArnJMlw34976602;     SMzArnJMlw34976602 = SMzArnJMlw53513065;     SMzArnJMlw53513065 = SMzArnJMlw9688274;     SMzArnJMlw9688274 = SMzArnJMlw18852635;     SMzArnJMlw18852635 = SMzArnJMlw20734957;     SMzArnJMlw20734957 = SMzArnJMlw34097332;     SMzArnJMlw34097332 = SMzArnJMlw76041413;     SMzArnJMlw76041413 = SMzArnJMlw89438448;     SMzArnJMlw89438448 = SMzArnJMlw18987612;     SMzArnJMlw18987612 = SMzArnJMlw3560455;     SMzArnJMlw3560455 = SMzArnJMlw52934160;     SMzArnJMlw52934160 = SMzArnJMlw51979023;     SMzArnJMlw51979023 = SMzArnJMlw97452380;     SMzArnJMlw97452380 = SMzArnJMlw63568887;     SMzArnJMlw63568887 = SMzArnJMlw44692362;     SMzArnJMlw44692362 = SMzArnJMlw61749346;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jlXioxpVMY26721493() {     int GntbLmAhIN74535513 = -93654757;    int GntbLmAhIN92851951 = -701750381;    int GntbLmAhIN70505192 = -943184858;    int GntbLmAhIN57516520 = -252948768;    int GntbLmAhIN95020265 = 33266955;    int GntbLmAhIN9151242 = -756899544;    int GntbLmAhIN70597668 = -57870554;    int GntbLmAhIN1371748 = -606180223;    int GntbLmAhIN6739392 = -699752359;    int GntbLmAhIN96003755 = -994216999;    int GntbLmAhIN35363231 = -858601916;    int GntbLmAhIN39380633 = -272325757;    int GntbLmAhIN67693475 = -460609963;    int GntbLmAhIN37686196 = -547362325;    int GntbLmAhIN24160523 = -186242408;    int GntbLmAhIN11475586 = -764424468;    int GntbLmAhIN19186785 = -350597062;    int GntbLmAhIN6041855 = 28517292;    int GntbLmAhIN7416872 = -402582234;    int GntbLmAhIN96971314 = -82365561;    int GntbLmAhIN68541623 = -895744997;    int GntbLmAhIN54116655 = -917784038;    int GntbLmAhIN3885914 = 80411438;    int GntbLmAhIN76913247 = -151316373;    int GntbLmAhIN8635670 = -201699449;    int GntbLmAhIN26011159 = -573595618;    int GntbLmAhIN45055247 = -118744635;    int GntbLmAhIN16679209 = -869191613;    int GntbLmAhIN40063686 = -477944645;    int GntbLmAhIN49869378 = -757652573;    int GntbLmAhIN61346392 = -911178036;    int GntbLmAhIN3538765 = -803294348;    int GntbLmAhIN20871957 = -159609699;    int GntbLmAhIN63827062 = -515733870;    int GntbLmAhIN40475999 = -721293563;    int GntbLmAhIN70276891 = -252907038;    int GntbLmAhIN33151748 = -587334737;    int GntbLmAhIN93976755 = -703351111;    int GntbLmAhIN81579645 = -187395578;    int GntbLmAhIN62026644 = -114948077;    int GntbLmAhIN14429095 = -278532888;    int GntbLmAhIN10887480 = -48005649;    int GntbLmAhIN46552168 = -78868169;    int GntbLmAhIN81072079 = -943663894;    int GntbLmAhIN32356760 = -920769540;    int GntbLmAhIN903699 = -833254854;    int GntbLmAhIN30980015 = -4232206;    int GntbLmAhIN2375211 = -956582719;    int GntbLmAhIN21797380 = -575481673;    int GntbLmAhIN72257197 = -158338386;    int GntbLmAhIN47310071 = -983785955;    int GntbLmAhIN10110342 = -383143685;    int GntbLmAhIN81192025 = -610914063;    int GntbLmAhIN9837460 = -307072567;    int GntbLmAhIN58179870 = 1600223;    int GntbLmAhIN20418859 = -175870719;    int GntbLmAhIN88966038 = -682161820;    int GntbLmAhIN93591945 = -691868485;    int GntbLmAhIN48880851 = 48750680;    int GntbLmAhIN69009106 = -393137427;    int GntbLmAhIN64095994 = -538154909;    int GntbLmAhIN53918460 = -188678941;    int GntbLmAhIN61308061 = -28235578;    int GntbLmAhIN56870013 = -942099786;    int GntbLmAhIN34657364 = 16961036;    int GntbLmAhIN31824467 = 44692432;    int GntbLmAhIN18508676 = -12716059;    int GntbLmAhIN3866414 = -944876094;    int GntbLmAhIN97210197 = -826068763;    int GntbLmAhIN53883631 = -933335370;    int GntbLmAhIN78323837 = -77089731;    int GntbLmAhIN25210030 = -647245952;    int GntbLmAhIN24462210 = -784087130;    int GntbLmAhIN45390228 = -187634158;    int GntbLmAhIN82542219 = -803832674;    int GntbLmAhIN57654143 = -747739348;    int GntbLmAhIN7564487 = -738915870;    int GntbLmAhIN22813834 = 24075332;    int GntbLmAhIN44556487 = -230546833;    int GntbLmAhIN7731971 = -368444596;    int GntbLmAhIN95031143 = -469363412;    int GntbLmAhIN42680037 = -162161916;    int GntbLmAhIN94881829 = -193709941;    int GntbLmAhIN67806489 = -219606260;    int GntbLmAhIN2559308 = -773866618;    int GntbLmAhIN51236051 = -428034351;    int GntbLmAhIN22346739 = -92380286;    int GntbLmAhIN11034498 = -852537133;    int GntbLmAhIN5647192 = -417334093;    int GntbLmAhIN20057141 = -445422844;    int GntbLmAhIN81310853 = -570745219;    int GntbLmAhIN39559803 = -895466253;    int GntbLmAhIN45095905 = -652101792;    int GntbLmAhIN12570539 = -794258152;    int GntbLmAhIN97930649 = -576793168;    int GntbLmAhIN60510635 = 10146053;    int GntbLmAhIN49579419 = 80229928;    int GntbLmAhIN89682155 = -136768383;    int GntbLmAhIN46414716 = -860624931;    int GntbLmAhIN532294 = -93654757;     GntbLmAhIN74535513 = GntbLmAhIN92851951;     GntbLmAhIN92851951 = GntbLmAhIN70505192;     GntbLmAhIN70505192 = GntbLmAhIN57516520;     GntbLmAhIN57516520 = GntbLmAhIN95020265;     GntbLmAhIN95020265 = GntbLmAhIN9151242;     GntbLmAhIN9151242 = GntbLmAhIN70597668;     GntbLmAhIN70597668 = GntbLmAhIN1371748;     GntbLmAhIN1371748 = GntbLmAhIN6739392;     GntbLmAhIN6739392 = GntbLmAhIN96003755;     GntbLmAhIN96003755 = GntbLmAhIN35363231;     GntbLmAhIN35363231 = GntbLmAhIN39380633;     GntbLmAhIN39380633 = GntbLmAhIN67693475;     GntbLmAhIN67693475 = GntbLmAhIN37686196;     GntbLmAhIN37686196 = GntbLmAhIN24160523;     GntbLmAhIN24160523 = GntbLmAhIN11475586;     GntbLmAhIN11475586 = GntbLmAhIN19186785;     GntbLmAhIN19186785 = GntbLmAhIN6041855;     GntbLmAhIN6041855 = GntbLmAhIN7416872;     GntbLmAhIN7416872 = GntbLmAhIN96971314;     GntbLmAhIN96971314 = GntbLmAhIN68541623;     GntbLmAhIN68541623 = GntbLmAhIN54116655;     GntbLmAhIN54116655 = GntbLmAhIN3885914;     GntbLmAhIN3885914 = GntbLmAhIN76913247;     GntbLmAhIN76913247 = GntbLmAhIN8635670;     GntbLmAhIN8635670 = GntbLmAhIN26011159;     GntbLmAhIN26011159 = GntbLmAhIN45055247;     GntbLmAhIN45055247 = GntbLmAhIN16679209;     GntbLmAhIN16679209 = GntbLmAhIN40063686;     GntbLmAhIN40063686 = GntbLmAhIN49869378;     GntbLmAhIN49869378 = GntbLmAhIN61346392;     GntbLmAhIN61346392 = GntbLmAhIN3538765;     GntbLmAhIN3538765 = GntbLmAhIN20871957;     GntbLmAhIN20871957 = GntbLmAhIN63827062;     GntbLmAhIN63827062 = GntbLmAhIN40475999;     GntbLmAhIN40475999 = GntbLmAhIN70276891;     GntbLmAhIN70276891 = GntbLmAhIN33151748;     GntbLmAhIN33151748 = GntbLmAhIN93976755;     GntbLmAhIN93976755 = GntbLmAhIN81579645;     GntbLmAhIN81579645 = GntbLmAhIN62026644;     GntbLmAhIN62026644 = GntbLmAhIN14429095;     GntbLmAhIN14429095 = GntbLmAhIN10887480;     GntbLmAhIN10887480 = GntbLmAhIN46552168;     GntbLmAhIN46552168 = GntbLmAhIN81072079;     GntbLmAhIN81072079 = GntbLmAhIN32356760;     GntbLmAhIN32356760 = GntbLmAhIN903699;     GntbLmAhIN903699 = GntbLmAhIN30980015;     GntbLmAhIN30980015 = GntbLmAhIN2375211;     GntbLmAhIN2375211 = GntbLmAhIN21797380;     GntbLmAhIN21797380 = GntbLmAhIN72257197;     GntbLmAhIN72257197 = GntbLmAhIN47310071;     GntbLmAhIN47310071 = GntbLmAhIN10110342;     GntbLmAhIN10110342 = GntbLmAhIN81192025;     GntbLmAhIN81192025 = GntbLmAhIN9837460;     GntbLmAhIN9837460 = GntbLmAhIN58179870;     GntbLmAhIN58179870 = GntbLmAhIN20418859;     GntbLmAhIN20418859 = GntbLmAhIN88966038;     GntbLmAhIN88966038 = GntbLmAhIN93591945;     GntbLmAhIN93591945 = GntbLmAhIN48880851;     GntbLmAhIN48880851 = GntbLmAhIN69009106;     GntbLmAhIN69009106 = GntbLmAhIN64095994;     GntbLmAhIN64095994 = GntbLmAhIN53918460;     GntbLmAhIN53918460 = GntbLmAhIN61308061;     GntbLmAhIN61308061 = GntbLmAhIN56870013;     GntbLmAhIN56870013 = GntbLmAhIN34657364;     GntbLmAhIN34657364 = GntbLmAhIN31824467;     GntbLmAhIN31824467 = GntbLmAhIN18508676;     GntbLmAhIN18508676 = GntbLmAhIN3866414;     GntbLmAhIN3866414 = GntbLmAhIN97210197;     GntbLmAhIN97210197 = GntbLmAhIN53883631;     GntbLmAhIN53883631 = GntbLmAhIN78323837;     GntbLmAhIN78323837 = GntbLmAhIN25210030;     GntbLmAhIN25210030 = GntbLmAhIN24462210;     GntbLmAhIN24462210 = GntbLmAhIN45390228;     GntbLmAhIN45390228 = GntbLmAhIN82542219;     GntbLmAhIN82542219 = GntbLmAhIN57654143;     GntbLmAhIN57654143 = GntbLmAhIN7564487;     GntbLmAhIN7564487 = GntbLmAhIN22813834;     GntbLmAhIN22813834 = GntbLmAhIN44556487;     GntbLmAhIN44556487 = GntbLmAhIN7731971;     GntbLmAhIN7731971 = GntbLmAhIN95031143;     GntbLmAhIN95031143 = GntbLmAhIN42680037;     GntbLmAhIN42680037 = GntbLmAhIN94881829;     GntbLmAhIN94881829 = GntbLmAhIN67806489;     GntbLmAhIN67806489 = GntbLmAhIN2559308;     GntbLmAhIN2559308 = GntbLmAhIN51236051;     GntbLmAhIN51236051 = GntbLmAhIN22346739;     GntbLmAhIN22346739 = GntbLmAhIN11034498;     GntbLmAhIN11034498 = GntbLmAhIN5647192;     GntbLmAhIN5647192 = GntbLmAhIN20057141;     GntbLmAhIN20057141 = GntbLmAhIN81310853;     GntbLmAhIN81310853 = GntbLmAhIN39559803;     GntbLmAhIN39559803 = GntbLmAhIN45095905;     GntbLmAhIN45095905 = GntbLmAhIN12570539;     GntbLmAhIN12570539 = GntbLmAhIN97930649;     GntbLmAhIN97930649 = GntbLmAhIN60510635;     GntbLmAhIN60510635 = GntbLmAhIN49579419;     GntbLmAhIN49579419 = GntbLmAhIN89682155;     GntbLmAhIN89682155 = GntbLmAhIN46414716;     GntbLmAhIN46414716 = GntbLmAhIN532294;     GntbLmAhIN532294 = GntbLmAhIN74535513;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dnCjGTDFNK44934835() {     int FGgXJcuhBC57979819 = -974320437;    int FGgXJcuhBC9724041 = -701132569;    int FGgXJcuhBC66913699 = -736710086;    int FGgXJcuhBC20308772 = -446748604;    int FGgXJcuhBC1141590 = -873600536;    int FGgXJcuhBC7655643 = -535971197;    int FGgXJcuhBC41636987 = -897781867;    int FGgXJcuhBC15719190 = -593338252;    int FGgXJcuhBC65269092 = 58788290;    int FGgXJcuhBC28549408 = -362593111;    int FGgXJcuhBC84779168 = 51282211;    int FGgXJcuhBC2586033 = -259944003;    int FGgXJcuhBC85811251 = -252271204;    int FGgXJcuhBC17313089 = -335652682;    int FGgXJcuhBC85014986 = -280265391;    int FGgXJcuhBC97864747 = -552503793;    int FGgXJcuhBC12938670 = -103942089;    int FGgXJcuhBC86893301 = -218787585;    int FGgXJcuhBC69857014 = -401257105;    int FGgXJcuhBC86338756 = -611951554;    int FGgXJcuhBC24182010 = -485274028;    int FGgXJcuhBC66263973 = -908935808;    int FGgXJcuhBC7711621 = 40522241;    int FGgXJcuhBC11619806 = -223141769;    int FGgXJcuhBC50211627 = -761352208;    int FGgXJcuhBC21353308 = -454105333;    int FGgXJcuhBC94509987 = -846828977;    int FGgXJcuhBC56410834 = -939146497;    int FGgXJcuhBC76144780 = -340449787;    int FGgXJcuhBC65702719 = -170502424;    int FGgXJcuhBC79441706 = -548935237;    int FGgXJcuhBC66645191 = -718489428;    int FGgXJcuhBC64410844 = -880481876;    int FGgXJcuhBC5985573 = -107336040;    int FGgXJcuhBC19202729 = -918958298;    int FGgXJcuhBC82666076 = -40930944;    int FGgXJcuhBC1765927 = -466183917;    int FGgXJcuhBC34555588 = -411821631;    int FGgXJcuhBC30574723 = -372293376;    int FGgXJcuhBC11521780 = -967541599;    int FGgXJcuhBC48078019 = -950081861;    int FGgXJcuhBC61375266 = -621913971;    int FGgXJcuhBC70119238 = -629925412;    int FGgXJcuhBC72977019 = -699846665;    int FGgXJcuhBC24301257 = -648926493;    int FGgXJcuhBC40509694 = -852865778;    int FGgXJcuhBC6995446 = -136536436;    int FGgXJcuhBC22045806 = -955304438;    int FGgXJcuhBC52644492 = -61083756;    int FGgXJcuhBC7589682 = -365358841;    int FGgXJcuhBC20567519 = -104687378;    int FGgXJcuhBC65611135 = -441402997;    int FGgXJcuhBC95304484 = -822661379;    int FGgXJcuhBC34514218 = -25485954;    int FGgXJcuhBC20752825 = -346885900;    int FGgXJcuhBC91715845 = 34615370;    int FGgXJcuhBC2012420 = -641654810;    int FGgXJcuhBC55293894 = -413568317;    int FGgXJcuhBC70097144 = -685396396;    int FGgXJcuhBC79788281 = -319495203;    int FGgXJcuhBC13145655 = -689142220;    int FGgXJcuhBC85226152 = -958635371;    int FGgXJcuhBC39574410 = -152888465;    int FGgXJcuhBC99566372 = -770709286;    int FGgXJcuhBC49107702 = -813657875;    int FGgXJcuhBC18133978 = -230228361;    int FGgXJcuhBC38175188 = -379462128;    int FGgXJcuhBC79825678 = -44935165;    int FGgXJcuhBC98110359 = -416694385;    int FGgXJcuhBC2348911 = -139334447;    int FGgXJcuhBC96098820 = 13680123;    int FGgXJcuhBC78383082 = -692120459;    int FGgXJcuhBC56318578 = -846494209;    int FGgXJcuhBC58335234 = -433715506;    int FGgXJcuhBC38260737 = -661869693;    int FGgXJcuhBC62806744 = -863360058;    int FGgXJcuhBC96144735 = -179010397;    int FGgXJcuhBC34734602 = -259631094;    int FGgXJcuhBC87318548 = -574215276;    int FGgXJcuhBC9701934 = -908486431;    int FGgXJcuhBC14357863 = -217568898;    int FGgXJcuhBC72464181 = -891524540;    int FGgXJcuhBC3766343 = -778062742;    int FGgXJcuhBC68555099 = -975090947;    int FGgXJcuhBC45135201 = 34184953;    int FGgXJcuhBC13830571 = -7532241;    int FGgXJcuhBC71340707 = -895828050;    int FGgXJcuhBC29896626 = -754995923;    int FGgXJcuhBC85232747 = -760450140;    int FGgXJcuhBC27486884 = -853573669;    int FGgXJcuhBC80653656 = -399276135;    int FGgXJcuhBC46472033 = 47384399;    int FGgXJcuhBC64458444 = -726425235;    int FGgXJcuhBC50786441 = 47201826;    int FGgXJcuhBC98376124 = -178399380;    int FGgXJcuhBC62851867 = -991446491;    int FGgXJcuhBC21800857 = -369025506;    int FGgXJcuhBC70552865 = -859216126;    int FGgXJcuhBC23869318 = -886188791;    int FGgXJcuhBC6167280 = -974320437;     FGgXJcuhBC57979819 = FGgXJcuhBC9724041;     FGgXJcuhBC9724041 = FGgXJcuhBC66913699;     FGgXJcuhBC66913699 = FGgXJcuhBC20308772;     FGgXJcuhBC20308772 = FGgXJcuhBC1141590;     FGgXJcuhBC1141590 = FGgXJcuhBC7655643;     FGgXJcuhBC7655643 = FGgXJcuhBC41636987;     FGgXJcuhBC41636987 = FGgXJcuhBC15719190;     FGgXJcuhBC15719190 = FGgXJcuhBC65269092;     FGgXJcuhBC65269092 = FGgXJcuhBC28549408;     FGgXJcuhBC28549408 = FGgXJcuhBC84779168;     FGgXJcuhBC84779168 = FGgXJcuhBC2586033;     FGgXJcuhBC2586033 = FGgXJcuhBC85811251;     FGgXJcuhBC85811251 = FGgXJcuhBC17313089;     FGgXJcuhBC17313089 = FGgXJcuhBC85014986;     FGgXJcuhBC85014986 = FGgXJcuhBC97864747;     FGgXJcuhBC97864747 = FGgXJcuhBC12938670;     FGgXJcuhBC12938670 = FGgXJcuhBC86893301;     FGgXJcuhBC86893301 = FGgXJcuhBC69857014;     FGgXJcuhBC69857014 = FGgXJcuhBC86338756;     FGgXJcuhBC86338756 = FGgXJcuhBC24182010;     FGgXJcuhBC24182010 = FGgXJcuhBC66263973;     FGgXJcuhBC66263973 = FGgXJcuhBC7711621;     FGgXJcuhBC7711621 = FGgXJcuhBC11619806;     FGgXJcuhBC11619806 = FGgXJcuhBC50211627;     FGgXJcuhBC50211627 = FGgXJcuhBC21353308;     FGgXJcuhBC21353308 = FGgXJcuhBC94509987;     FGgXJcuhBC94509987 = FGgXJcuhBC56410834;     FGgXJcuhBC56410834 = FGgXJcuhBC76144780;     FGgXJcuhBC76144780 = FGgXJcuhBC65702719;     FGgXJcuhBC65702719 = FGgXJcuhBC79441706;     FGgXJcuhBC79441706 = FGgXJcuhBC66645191;     FGgXJcuhBC66645191 = FGgXJcuhBC64410844;     FGgXJcuhBC64410844 = FGgXJcuhBC5985573;     FGgXJcuhBC5985573 = FGgXJcuhBC19202729;     FGgXJcuhBC19202729 = FGgXJcuhBC82666076;     FGgXJcuhBC82666076 = FGgXJcuhBC1765927;     FGgXJcuhBC1765927 = FGgXJcuhBC34555588;     FGgXJcuhBC34555588 = FGgXJcuhBC30574723;     FGgXJcuhBC30574723 = FGgXJcuhBC11521780;     FGgXJcuhBC11521780 = FGgXJcuhBC48078019;     FGgXJcuhBC48078019 = FGgXJcuhBC61375266;     FGgXJcuhBC61375266 = FGgXJcuhBC70119238;     FGgXJcuhBC70119238 = FGgXJcuhBC72977019;     FGgXJcuhBC72977019 = FGgXJcuhBC24301257;     FGgXJcuhBC24301257 = FGgXJcuhBC40509694;     FGgXJcuhBC40509694 = FGgXJcuhBC6995446;     FGgXJcuhBC6995446 = FGgXJcuhBC22045806;     FGgXJcuhBC22045806 = FGgXJcuhBC52644492;     FGgXJcuhBC52644492 = FGgXJcuhBC7589682;     FGgXJcuhBC7589682 = FGgXJcuhBC20567519;     FGgXJcuhBC20567519 = FGgXJcuhBC65611135;     FGgXJcuhBC65611135 = FGgXJcuhBC95304484;     FGgXJcuhBC95304484 = FGgXJcuhBC34514218;     FGgXJcuhBC34514218 = FGgXJcuhBC20752825;     FGgXJcuhBC20752825 = FGgXJcuhBC91715845;     FGgXJcuhBC91715845 = FGgXJcuhBC2012420;     FGgXJcuhBC2012420 = FGgXJcuhBC55293894;     FGgXJcuhBC55293894 = FGgXJcuhBC70097144;     FGgXJcuhBC70097144 = FGgXJcuhBC79788281;     FGgXJcuhBC79788281 = FGgXJcuhBC13145655;     FGgXJcuhBC13145655 = FGgXJcuhBC85226152;     FGgXJcuhBC85226152 = FGgXJcuhBC39574410;     FGgXJcuhBC39574410 = FGgXJcuhBC99566372;     FGgXJcuhBC99566372 = FGgXJcuhBC49107702;     FGgXJcuhBC49107702 = FGgXJcuhBC18133978;     FGgXJcuhBC18133978 = FGgXJcuhBC38175188;     FGgXJcuhBC38175188 = FGgXJcuhBC79825678;     FGgXJcuhBC79825678 = FGgXJcuhBC98110359;     FGgXJcuhBC98110359 = FGgXJcuhBC2348911;     FGgXJcuhBC2348911 = FGgXJcuhBC96098820;     FGgXJcuhBC96098820 = FGgXJcuhBC78383082;     FGgXJcuhBC78383082 = FGgXJcuhBC56318578;     FGgXJcuhBC56318578 = FGgXJcuhBC58335234;     FGgXJcuhBC58335234 = FGgXJcuhBC38260737;     FGgXJcuhBC38260737 = FGgXJcuhBC62806744;     FGgXJcuhBC62806744 = FGgXJcuhBC96144735;     FGgXJcuhBC96144735 = FGgXJcuhBC34734602;     FGgXJcuhBC34734602 = FGgXJcuhBC87318548;     FGgXJcuhBC87318548 = FGgXJcuhBC9701934;     FGgXJcuhBC9701934 = FGgXJcuhBC14357863;     FGgXJcuhBC14357863 = FGgXJcuhBC72464181;     FGgXJcuhBC72464181 = FGgXJcuhBC3766343;     FGgXJcuhBC3766343 = FGgXJcuhBC68555099;     FGgXJcuhBC68555099 = FGgXJcuhBC45135201;     FGgXJcuhBC45135201 = FGgXJcuhBC13830571;     FGgXJcuhBC13830571 = FGgXJcuhBC71340707;     FGgXJcuhBC71340707 = FGgXJcuhBC29896626;     FGgXJcuhBC29896626 = FGgXJcuhBC85232747;     FGgXJcuhBC85232747 = FGgXJcuhBC27486884;     FGgXJcuhBC27486884 = FGgXJcuhBC80653656;     FGgXJcuhBC80653656 = FGgXJcuhBC46472033;     FGgXJcuhBC46472033 = FGgXJcuhBC64458444;     FGgXJcuhBC64458444 = FGgXJcuhBC50786441;     FGgXJcuhBC50786441 = FGgXJcuhBC98376124;     FGgXJcuhBC98376124 = FGgXJcuhBC62851867;     FGgXJcuhBC62851867 = FGgXJcuhBC21800857;     FGgXJcuhBC21800857 = FGgXJcuhBC70552865;     FGgXJcuhBC70552865 = FGgXJcuhBC23869318;     FGgXJcuhBC23869318 = FGgXJcuhBC6167280;     FGgXJcuhBC6167280 = FGgXJcuhBC57979819;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VmvYAegFrK15390708() {     int zqBAEWWriR70765986 = -9361129;    int zqBAEWWriR96494928 = -871466582;    int zqBAEWWriR84666498 = -45069467;    int zqBAEWWriR28249597 = -270950881;    int zqBAEWWriR94652693 = -265417001;    int zqBAEWWriR50965713 = -312894510;    int zqBAEWWriR62844745 = -343453938;    int zqBAEWWriR42438303 = 62143491;    int zqBAEWWriR60673903 = 97030198;    int zqBAEWWriR34768112 = -166680744;    int zqBAEWWriR12266551 = -63403804;    int zqBAEWWriR34536537 = -767276819;    int zqBAEWWriR55954554 = -156053861;    int zqBAEWWriR66937167 = -668067880;    int zqBAEWWriR8321540 = -187251221;    int zqBAEWWriR26289816 = -3395183;    int zqBAEWWriR68690487 = -867716556;    int zqBAEWWriR70451786 = -873997887;    int zqBAEWWriR71017644 = -310726301;    int zqBAEWWriR88993252 = -111195693;    int zqBAEWWriR4356067 = -976590292;    int zqBAEWWriR95311792 = 21205844;    int zqBAEWWriR46471916 = -990481654;    int zqBAEWWriR56126935 = -583486781;    int zqBAEWWriR56105886 = -971794132;    int zqBAEWWriR2671954 = -144536533;    int zqBAEWWriR5925524 = -380535258;    int zqBAEWWriR23763759 = -12944602;    int zqBAEWWriR92761833 = -554667028;    int zqBAEWWriR18453835 = -900388579;    int zqBAEWWriR23877702 = -825351987;    int zqBAEWWriR69964015 = -697209107;    int zqBAEWWriR10456793 = -756336660;    int zqBAEWWriR83061945 = -12090258;    int zqBAEWWriR2263262 = -428990178;    int zqBAEWWriR77890190 = -929494049;    int zqBAEWWriR38529987 = -256093268;    int zqBAEWWriR92259610 = -331441723;    int zqBAEWWriR36350109 = -214143953;    int zqBAEWWriR37786509 = -811958445;    int zqBAEWWriR9509328 = -225555854;    int zqBAEWWriR69077722 = -520225305;    int zqBAEWWriR5002708 = -653358857;    int zqBAEWWriR17605557 = 58981092;    int zqBAEWWriR4309748 = -541696454;    int zqBAEWWriR14137824 = -926184217;    int zqBAEWWriR84577465 = -634051488;    int zqBAEWWriR51485772 = -10657217;    int zqBAEWWriR96708541 = -224685730;    int zqBAEWWriR54540102 = -976395304;    int zqBAEWWriR38895013 = -357571686;    int zqBAEWWriR93787682 = -912588335;    int zqBAEWWriR29789635 = -998664662;    int zqBAEWWriR79213943 = -714937696;    int zqBAEWWriR11034641 = -341330387;    int zqBAEWWriR75454193 = 69433027;    int zqBAEWWriR50023012 = -880984929;    int zqBAEWWriR28539564 = -461582686;    int zqBAEWWriR72143710 = -299156749;    int zqBAEWWriR91980740 = -20880469;    int zqBAEWWriR45040189 = -932359253;    int zqBAEWWriR39080986 = -230509336;    int zqBAEWWriR49676469 = -383189482;    int zqBAEWWriR42220068 = -2581224;    int zqBAEWWriR10890411 = -341328758;    int zqBAEWWriR42302536 = -366194698;    int zqBAEWWriR24079745 = 89059841;    int zqBAEWWriR72892608 = -43963604;    int zqBAEWWriR64673906 = -139077702;    int zqBAEWWriR30431349 = -257757173;    int zqBAEWWriR87759829 = -747301915;    int zqBAEWWriR76430877 = -436274834;    int zqBAEWWriR34101677 = -559853934;    int zqBAEWWriR33231135 = -498767856;    int zqBAEWWriR79483925 = -885639839;    int zqBAEWWriR35278345 = -356364988;    int zqBAEWWriR90309084 = -325435300;    int zqBAEWWriR28866359 = -949462746;    int zqBAEWWriR51817188 = 58209672;    int zqBAEWWriR41968063 = 54390085;    int zqBAEWWriR18094489 = -510485045;    int zqBAEWWriR54439751 = -269878041;    int zqBAEWWriR27055218 = -788258872;    int zqBAEWWriR38221732 = -578271724;    int zqBAEWWriR79558822 = -442816893;    int zqBAEWWriR30090019 = -912763653;    int zqBAEWWriR40174381 = -698544445;    int zqBAEWWriR31242850 = 58601035;    int zqBAEWWriR72027304 = -670759871;    int zqBAEWWriR26809068 = -398423205;    int zqBAEWWriR27867179 = 51490880;    int zqBAEWWriR9990423 = -794510582;    int zqBAEWWriR20115901 = 67715026;    int zqBAEWWriR44369368 = -93263485;    int zqBAEWWriR92746319 = -879599192;    int zqBAEWWriR70428342 = -995046519;    int zqBAEWWriR19401253 = -37035824;    int zqBAEWWriR62782640 = -550777634;    int zqBAEWWriR6715147 = -599690151;    int zqBAEWWriR62007211 = -9361129;     zqBAEWWriR70765986 = zqBAEWWriR96494928;     zqBAEWWriR96494928 = zqBAEWWriR84666498;     zqBAEWWriR84666498 = zqBAEWWriR28249597;     zqBAEWWriR28249597 = zqBAEWWriR94652693;     zqBAEWWriR94652693 = zqBAEWWriR50965713;     zqBAEWWriR50965713 = zqBAEWWriR62844745;     zqBAEWWriR62844745 = zqBAEWWriR42438303;     zqBAEWWriR42438303 = zqBAEWWriR60673903;     zqBAEWWriR60673903 = zqBAEWWriR34768112;     zqBAEWWriR34768112 = zqBAEWWriR12266551;     zqBAEWWriR12266551 = zqBAEWWriR34536537;     zqBAEWWriR34536537 = zqBAEWWriR55954554;     zqBAEWWriR55954554 = zqBAEWWriR66937167;     zqBAEWWriR66937167 = zqBAEWWriR8321540;     zqBAEWWriR8321540 = zqBAEWWriR26289816;     zqBAEWWriR26289816 = zqBAEWWriR68690487;     zqBAEWWriR68690487 = zqBAEWWriR70451786;     zqBAEWWriR70451786 = zqBAEWWriR71017644;     zqBAEWWriR71017644 = zqBAEWWriR88993252;     zqBAEWWriR88993252 = zqBAEWWriR4356067;     zqBAEWWriR4356067 = zqBAEWWriR95311792;     zqBAEWWriR95311792 = zqBAEWWriR46471916;     zqBAEWWriR46471916 = zqBAEWWriR56126935;     zqBAEWWriR56126935 = zqBAEWWriR56105886;     zqBAEWWriR56105886 = zqBAEWWriR2671954;     zqBAEWWriR2671954 = zqBAEWWriR5925524;     zqBAEWWriR5925524 = zqBAEWWriR23763759;     zqBAEWWriR23763759 = zqBAEWWriR92761833;     zqBAEWWriR92761833 = zqBAEWWriR18453835;     zqBAEWWriR18453835 = zqBAEWWriR23877702;     zqBAEWWriR23877702 = zqBAEWWriR69964015;     zqBAEWWriR69964015 = zqBAEWWriR10456793;     zqBAEWWriR10456793 = zqBAEWWriR83061945;     zqBAEWWriR83061945 = zqBAEWWriR2263262;     zqBAEWWriR2263262 = zqBAEWWriR77890190;     zqBAEWWriR77890190 = zqBAEWWriR38529987;     zqBAEWWriR38529987 = zqBAEWWriR92259610;     zqBAEWWriR92259610 = zqBAEWWriR36350109;     zqBAEWWriR36350109 = zqBAEWWriR37786509;     zqBAEWWriR37786509 = zqBAEWWriR9509328;     zqBAEWWriR9509328 = zqBAEWWriR69077722;     zqBAEWWriR69077722 = zqBAEWWriR5002708;     zqBAEWWriR5002708 = zqBAEWWriR17605557;     zqBAEWWriR17605557 = zqBAEWWriR4309748;     zqBAEWWriR4309748 = zqBAEWWriR14137824;     zqBAEWWriR14137824 = zqBAEWWriR84577465;     zqBAEWWriR84577465 = zqBAEWWriR51485772;     zqBAEWWriR51485772 = zqBAEWWriR96708541;     zqBAEWWriR96708541 = zqBAEWWriR54540102;     zqBAEWWriR54540102 = zqBAEWWriR38895013;     zqBAEWWriR38895013 = zqBAEWWriR93787682;     zqBAEWWriR93787682 = zqBAEWWriR29789635;     zqBAEWWriR29789635 = zqBAEWWriR79213943;     zqBAEWWriR79213943 = zqBAEWWriR11034641;     zqBAEWWriR11034641 = zqBAEWWriR75454193;     zqBAEWWriR75454193 = zqBAEWWriR50023012;     zqBAEWWriR50023012 = zqBAEWWriR28539564;     zqBAEWWriR28539564 = zqBAEWWriR72143710;     zqBAEWWriR72143710 = zqBAEWWriR91980740;     zqBAEWWriR91980740 = zqBAEWWriR45040189;     zqBAEWWriR45040189 = zqBAEWWriR39080986;     zqBAEWWriR39080986 = zqBAEWWriR49676469;     zqBAEWWriR49676469 = zqBAEWWriR42220068;     zqBAEWWriR42220068 = zqBAEWWriR10890411;     zqBAEWWriR10890411 = zqBAEWWriR42302536;     zqBAEWWriR42302536 = zqBAEWWriR24079745;     zqBAEWWriR24079745 = zqBAEWWriR72892608;     zqBAEWWriR72892608 = zqBAEWWriR64673906;     zqBAEWWriR64673906 = zqBAEWWriR30431349;     zqBAEWWriR30431349 = zqBAEWWriR87759829;     zqBAEWWriR87759829 = zqBAEWWriR76430877;     zqBAEWWriR76430877 = zqBAEWWriR34101677;     zqBAEWWriR34101677 = zqBAEWWriR33231135;     zqBAEWWriR33231135 = zqBAEWWriR79483925;     zqBAEWWriR79483925 = zqBAEWWriR35278345;     zqBAEWWriR35278345 = zqBAEWWriR90309084;     zqBAEWWriR90309084 = zqBAEWWriR28866359;     zqBAEWWriR28866359 = zqBAEWWriR51817188;     zqBAEWWriR51817188 = zqBAEWWriR41968063;     zqBAEWWriR41968063 = zqBAEWWriR18094489;     zqBAEWWriR18094489 = zqBAEWWriR54439751;     zqBAEWWriR54439751 = zqBAEWWriR27055218;     zqBAEWWriR27055218 = zqBAEWWriR38221732;     zqBAEWWriR38221732 = zqBAEWWriR79558822;     zqBAEWWriR79558822 = zqBAEWWriR30090019;     zqBAEWWriR30090019 = zqBAEWWriR40174381;     zqBAEWWriR40174381 = zqBAEWWriR31242850;     zqBAEWWriR31242850 = zqBAEWWriR72027304;     zqBAEWWriR72027304 = zqBAEWWriR26809068;     zqBAEWWriR26809068 = zqBAEWWriR27867179;     zqBAEWWriR27867179 = zqBAEWWriR9990423;     zqBAEWWriR9990423 = zqBAEWWriR20115901;     zqBAEWWriR20115901 = zqBAEWWriR44369368;     zqBAEWWriR44369368 = zqBAEWWriR92746319;     zqBAEWWriR92746319 = zqBAEWWriR70428342;     zqBAEWWriR70428342 = zqBAEWWriR19401253;     zqBAEWWriR19401253 = zqBAEWWriR62782640;     zqBAEWWriR62782640 = zqBAEWWriR6715147;     zqBAEWWriR6715147 = zqBAEWWriR62007211;     zqBAEWWriR62007211 = zqBAEWWriR70765986;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KmdsTetjXI33604049() {     int MgmOekwGrz54210292 = -890026810;    int MgmOekwGrz13367018 = -870848770;    int MgmOekwGrz81075005 = -938594695;    int MgmOekwGrz91041848 = -464750717;    int MgmOekwGrz774018 = -72284492;    int MgmOekwGrz49470114 = -91966163;    int MgmOekwGrz33884063 = -83365251;    int MgmOekwGrz56785745 = 74985462;    int MgmOekwGrz19203604 = -244429153;    int MgmOekwGrz67313764 = -635056856;    int MgmOekwGrz61682489 = -253519677;    int MgmOekwGrz97741936 = -754895066;    int MgmOekwGrz74072330 = 52284898;    int MgmOekwGrz46564059 = -456358237;    int MgmOekwGrz69176002 = -281274204;    int MgmOekwGrz12678978 = -891474508;    int MgmOekwGrz62442372 = -621061583;    int MgmOekwGrz51303233 = -21302763;    int MgmOekwGrz33457786 = -309401171;    int MgmOekwGrz78360695 = -640781686;    int MgmOekwGrz59996454 = -566119323;    int MgmOekwGrz7459111 = 30054073;    int MgmOekwGrz50297624 = 69629149;    int MgmOekwGrz90833493 = -655312178;    int MgmOekwGrz97681844 = -431446891;    int MgmOekwGrz98014102 = -25046248;    int MgmOekwGrz55380264 = -8619600;    int MgmOekwGrz63495384 = -82899486;    int MgmOekwGrz28842928 = -417172170;    int MgmOekwGrz34287176 = -313238430;    int MgmOekwGrz41973016 = -463109188;    int MgmOekwGrz33070442 = -612404186;    int MgmOekwGrz53995680 = -377208838;    int MgmOekwGrz25220456 = -703692428;    int MgmOekwGrz80989991 = -626654913;    int MgmOekwGrz90279375 = -717517954;    int MgmOekwGrz7144166 = -134942447;    int MgmOekwGrz32838443 = -39912243;    int MgmOekwGrz85345187 = -399041751;    int MgmOekwGrz87281644 = -564551967;    int MgmOekwGrz43158252 = -897104828;    int MgmOekwGrz19565508 = 5866374;    int MgmOekwGrz28569778 = -104416099;    int MgmOekwGrz9510498 = -797201679;    int MgmOekwGrz96254243 = -269853407;    int MgmOekwGrz53743819 = -945795141;    int MgmOekwGrz60592895 = -766355718;    int MgmOekwGrz71156368 = -9378936;    int MgmOekwGrz27555654 = -810287813;    int MgmOekwGrz89872585 = -83415759;    int MgmOekwGrz12152461 = -578473108;    int MgmOekwGrz49288477 = -970847646;    int MgmOekwGrz43902093 = -110411978;    int MgmOekwGrz3890703 = -433351083;    int MgmOekwGrz73607595 = -689816510;    int MgmOekwGrz46751181 = -820080884;    int MgmOekwGrz63069393 = -840477919;    int MgmOekwGrz90241512 = -183282518;    int MgmOekwGrz93360003 = 66696174;    int MgmOekwGrz2759916 = 52761756;    int MgmOekwGrz94089850 = 16653436;    int MgmOekwGrz70388678 = 99534234;    int MgmOekwGrz27942818 = -507842369;    int MgmOekwGrz84916427 = -931190723;    int MgmOekwGrz25340749 = -71947669;    int MgmOekwGrz28612047 = -641115491;    int MgmOekwGrz43746257 = -277686228;    int MgmOekwGrz48851874 = -244022675;    int MgmOekwGrz65574067 = -829703324;    int MgmOekwGrz78896627 = -563756250;    int MgmOekwGrz5534813 = -656532061;    int MgmOekwGrz29603930 = -481149341;    int MgmOekwGrz65958045 = -622261013;    int MgmOekwGrz46176142 = -744849205;    int MgmOekwGrz35202444 = -743676858;    int MgmOekwGrz40430946 = -471985697;    int MgmOekwGrz78889333 = -865529828;    int MgmOekwGrz40787127 = -133169173;    int MgmOekwGrz94579249 = -285458771;    int MgmOekwGrz43938025 = -485651751;    int MgmOekwGrz37421207 = -258690531;    int MgmOekwGrz84223896 = -999240665;    int MgmOekwGrz35939731 = -272611674;    int MgmOekwGrz38970342 = -233756411;    int MgmOekwGrz22134716 = -734765323;    int MgmOekwGrz92684538 = -492261542;    int MgmOekwGrz89168348 = -401992209;    int MgmOekwGrz50104977 = -943857755;    int MgmOekwGrz51612860 = 86124081;    int MgmOekwGrz34238811 = -806574030;    int MgmOekwGrz27209982 = -877040036;    int MgmOekwGrz16902653 = -951659930;    int MgmOekwGrz39478439 = -6608417;    int MgmOekwGrz82585271 = -351803507;    int MgmOekwGrz93191794 = -481205404;    int MgmOekwGrz72769573 = -896639062;    int MgmOekwGrz91622690 = -486291258;    int MgmOekwGrz43653350 = -173225377;    int MgmOekwGrz84169748 = -625254010;    int MgmOekwGrz67642197 = -890026810;     MgmOekwGrz54210292 = MgmOekwGrz13367018;     MgmOekwGrz13367018 = MgmOekwGrz81075005;     MgmOekwGrz81075005 = MgmOekwGrz91041848;     MgmOekwGrz91041848 = MgmOekwGrz774018;     MgmOekwGrz774018 = MgmOekwGrz49470114;     MgmOekwGrz49470114 = MgmOekwGrz33884063;     MgmOekwGrz33884063 = MgmOekwGrz56785745;     MgmOekwGrz56785745 = MgmOekwGrz19203604;     MgmOekwGrz19203604 = MgmOekwGrz67313764;     MgmOekwGrz67313764 = MgmOekwGrz61682489;     MgmOekwGrz61682489 = MgmOekwGrz97741936;     MgmOekwGrz97741936 = MgmOekwGrz74072330;     MgmOekwGrz74072330 = MgmOekwGrz46564059;     MgmOekwGrz46564059 = MgmOekwGrz69176002;     MgmOekwGrz69176002 = MgmOekwGrz12678978;     MgmOekwGrz12678978 = MgmOekwGrz62442372;     MgmOekwGrz62442372 = MgmOekwGrz51303233;     MgmOekwGrz51303233 = MgmOekwGrz33457786;     MgmOekwGrz33457786 = MgmOekwGrz78360695;     MgmOekwGrz78360695 = MgmOekwGrz59996454;     MgmOekwGrz59996454 = MgmOekwGrz7459111;     MgmOekwGrz7459111 = MgmOekwGrz50297624;     MgmOekwGrz50297624 = MgmOekwGrz90833493;     MgmOekwGrz90833493 = MgmOekwGrz97681844;     MgmOekwGrz97681844 = MgmOekwGrz98014102;     MgmOekwGrz98014102 = MgmOekwGrz55380264;     MgmOekwGrz55380264 = MgmOekwGrz63495384;     MgmOekwGrz63495384 = MgmOekwGrz28842928;     MgmOekwGrz28842928 = MgmOekwGrz34287176;     MgmOekwGrz34287176 = MgmOekwGrz41973016;     MgmOekwGrz41973016 = MgmOekwGrz33070442;     MgmOekwGrz33070442 = MgmOekwGrz53995680;     MgmOekwGrz53995680 = MgmOekwGrz25220456;     MgmOekwGrz25220456 = MgmOekwGrz80989991;     MgmOekwGrz80989991 = MgmOekwGrz90279375;     MgmOekwGrz90279375 = MgmOekwGrz7144166;     MgmOekwGrz7144166 = MgmOekwGrz32838443;     MgmOekwGrz32838443 = MgmOekwGrz85345187;     MgmOekwGrz85345187 = MgmOekwGrz87281644;     MgmOekwGrz87281644 = MgmOekwGrz43158252;     MgmOekwGrz43158252 = MgmOekwGrz19565508;     MgmOekwGrz19565508 = MgmOekwGrz28569778;     MgmOekwGrz28569778 = MgmOekwGrz9510498;     MgmOekwGrz9510498 = MgmOekwGrz96254243;     MgmOekwGrz96254243 = MgmOekwGrz53743819;     MgmOekwGrz53743819 = MgmOekwGrz60592895;     MgmOekwGrz60592895 = MgmOekwGrz71156368;     MgmOekwGrz71156368 = MgmOekwGrz27555654;     MgmOekwGrz27555654 = MgmOekwGrz89872585;     MgmOekwGrz89872585 = MgmOekwGrz12152461;     MgmOekwGrz12152461 = MgmOekwGrz49288477;     MgmOekwGrz49288477 = MgmOekwGrz43902093;     MgmOekwGrz43902093 = MgmOekwGrz3890703;     MgmOekwGrz3890703 = MgmOekwGrz73607595;     MgmOekwGrz73607595 = MgmOekwGrz46751181;     MgmOekwGrz46751181 = MgmOekwGrz63069393;     MgmOekwGrz63069393 = MgmOekwGrz90241512;     MgmOekwGrz90241512 = MgmOekwGrz93360003;     MgmOekwGrz93360003 = MgmOekwGrz2759916;     MgmOekwGrz2759916 = MgmOekwGrz94089850;     MgmOekwGrz94089850 = MgmOekwGrz70388678;     MgmOekwGrz70388678 = MgmOekwGrz27942818;     MgmOekwGrz27942818 = MgmOekwGrz84916427;     MgmOekwGrz84916427 = MgmOekwGrz25340749;     MgmOekwGrz25340749 = MgmOekwGrz28612047;     MgmOekwGrz28612047 = MgmOekwGrz43746257;     MgmOekwGrz43746257 = MgmOekwGrz48851874;     MgmOekwGrz48851874 = MgmOekwGrz65574067;     MgmOekwGrz65574067 = MgmOekwGrz78896627;     MgmOekwGrz78896627 = MgmOekwGrz5534813;     MgmOekwGrz5534813 = MgmOekwGrz29603930;     MgmOekwGrz29603930 = MgmOekwGrz65958045;     MgmOekwGrz65958045 = MgmOekwGrz46176142;     MgmOekwGrz46176142 = MgmOekwGrz35202444;     MgmOekwGrz35202444 = MgmOekwGrz40430946;     MgmOekwGrz40430946 = MgmOekwGrz78889333;     MgmOekwGrz78889333 = MgmOekwGrz40787127;     MgmOekwGrz40787127 = MgmOekwGrz94579249;     MgmOekwGrz94579249 = MgmOekwGrz43938025;     MgmOekwGrz43938025 = MgmOekwGrz37421207;     MgmOekwGrz37421207 = MgmOekwGrz84223896;     MgmOekwGrz84223896 = MgmOekwGrz35939731;     MgmOekwGrz35939731 = MgmOekwGrz38970342;     MgmOekwGrz38970342 = MgmOekwGrz22134716;     MgmOekwGrz22134716 = MgmOekwGrz92684538;     MgmOekwGrz92684538 = MgmOekwGrz89168348;     MgmOekwGrz89168348 = MgmOekwGrz50104977;     MgmOekwGrz50104977 = MgmOekwGrz51612860;     MgmOekwGrz51612860 = MgmOekwGrz34238811;     MgmOekwGrz34238811 = MgmOekwGrz27209982;     MgmOekwGrz27209982 = MgmOekwGrz16902653;     MgmOekwGrz16902653 = MgmOekwGrz39478439;     MgmOekwGrz39478439 = MgmOekwGrz82585271;     MgmOekwGrz82585271 = MgmOekwGrz93191794;     MgmOekwGrz93191794 = MgmOekwGrz72769573;     MgmOekwGrz72769573 = MgmOekwGrz91622690;     MgmOekwGrz91622690 = MgmOekwGrz43653350;     MgmOekwGrz43653350 = MgmOekwGrz84169748;     MgmOekwGrz84169748 = MgmOekwGrz67642197;     MgmOekwGrz67642197 = MgmOekwGrz54210292;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XgNeZiCKGZ4059923() {     int aEtoHxLoBi66996459 = 74932498;    int aEtoHxLoBi137906 = 58817216;    int aEtoHxLoBi98827805 = -246954077;    int aEtoHxLoBi98982673 = -288952993;    int aEtoHxLoBi94285122 = -564100957;    int aEtoHxLoBi92780184 = -968889477;    int aEtoHxLoBi55091821 = -629037322;    int aEtoHxLoBi83504858 = -369532795;    int aEtoHxLoBi14608414 = -206187245;    int aEtoHxLoBi73532469 = -439144489;    int aEtoHxLoBi89169871 = -368205692;    int aEtoHxLoBi29692442 = -162227881;    int aEtoHxLoBi44215633 = -951497759;    int aEtoHxLoBi96188137 = -788773435;    int aEtoHxLoBi92482555 = -188260034;    int aEtoHxLoBi41104046 = -342365897;    int aEtoHxLoBi18194190 = -284836050;    int aEtoHxLoBi34861718 = -676513065;    int aEtoHxLoBi34618417 = -218870367;    int aEtoHxLoBi81015191 = -140025825;    int aEtoHxLoBi40170511 = 42564413;    int aEtoHxLoBi36506930 = -139804275;    int aEtoHxLoBi89057919 = -961374746;    int aEtoHxLoBi35340623 = 84342810;    int aEtoHxLoBi3576104 = -641888815;    int aEtoHxLoBi79332747 = -815477447;    int aEtoHxLoBi66795800 = -642325880;    int aEtoHxLoBi30848310 = -256697590;    int aEtoHxLoBi45459981 = -631389410;    int aEtoHxLoBi87038291 = 56875415;    int aEtoHxLoBi86409011 = -739525938;    int aEtoHxLoBi36389266 = -591123865;    int aEtoHxLoBi41629 = -253063622;    int aEtoHxLoBi2296829 = -608446646;    int aEtoHxLoBi64050523 = -136686794;    int aEtoHxLoBi85503489 = -506081059;    int aEtoHxLoBi43908226 = 75148202;    int aEtoHxLoBi90542465 = 40467666;    int aEtoHxLoBi91120573 = -240892328;    int aEtoHxLoBi13546374 = -408968813;    int aEtoHxLoBi4589560 = -172578821;    int aEtoHxLoBi27267964 = -992444961;    int aEtoHxLoBi63453247 = -127849544;    int aEtoHxLoBi54139035 = -38373922;    int aEtoHxLoBi76262734 = -162623367;    int aEtoHxLoBi27371949 = 80886419;    int aEtoHxLoBi38174915 = -163870770;    int aEtoHxLoBi596335 = -164731715;    int aEtoHxLoBi71619703 = -973889787;    int aEtoHxLoBi36823006 = -694452222;    int aEtoHxLoBi30479954 = -831357417;    int aEtoHxLoBi77465024 = -342032984;    int aEtoHxLoBi78387243 = -286415261;    int aEtoHxLoBi48590428 = -22802825;    int aEtoHxLoBi63889411 = -684260996;    int aEtoHxLoBi30489529 = -785263228;    int aEtoHxLoBi11079987 = 20191962;    int aEtoHxLoBi63487183 = -231296888;    int aEtoHxLoBi95406570 = -647064178;    int aEtoHxLoBi14952375 = -748623510;    int aEtoHxLoBi25984385 = -226563597;    int aEtoHxLoBi24243511 = -272339732;    int aEtoHxLoBi38044878 = -738143386;    int aEtoHxLoBi27570123 = -163062661;    int aEtoHxLoBi87123457 = -699618552;    int aEtoHxLoBi52780605 = -777081828;    int aEtoHxLoBi29650813 = -909164260;    int aEtoHxLoBi41918804 = -243051113;    int aEtoHxLoBi32137614 = -552086642;    int aEtoHxLoBi6979066 = -682178976;    int aEtoHxLoBi97195820 = -317514100;    int aEtoHxLoBi27651725 = -225303716;    int aEtoHxLoBi43741145 = -335620738;    int aEtoHxLoBi21072043 = -809901554;    int aEtoHxLoBi76425632 = -967447004;    int aEtoHxLoBi12902547 = 35009373;    int aEtoHxLoBi73053682 = 88045269;    int aEtoHxLoBi34918885 = -823000824;    int aEtoHxLoBi59077888 = -753033823;    int aEtoHxLoBi76204155 = -622775235;    int aEtoHxLoBi41157833 = -551606678;    int aEtoHxLoBi66199466 = -377594166;    int aEtoHxLoBi59228607 = -282807804;    int aEtoHxLoBi8636975 = -936937188;    int aEtoHxLoBi56558337 = -111767169;    int aEtoHxLoBi8943988 = -297492954;    int aEtoHxLoBi58002022 = -204708605;    int aEtoHxLoBi51451201 = -130260797;    int aEtoHxLoBi38407417 = -924185650;    int aEtoHxLoBi33560995 = -351423567;    int aEtoHxLoBi74423503 = -426273021;    int aEtoHxLoBi80421043 = -693554911;    int aEtoHxLoBi95135895 = -312468157;    int aEtoHxLoBi76168198 = -492268819;    int aEtoHxLoBi87561989 = -82405217;    int aEtoHxLoBi80346048 = -900239090;    int aEtoHxLoBi89223086 = -154301576;    int aEtoHxLoBi35883125 = -964786884;    int aEtoHxLoBi67015577 = -338755370;    int aEtoHxLoBi23482129 = 74932498;     aEtoHxLoBi66996459 = aEtoHxLoBi137906;     aEtoHxLoBi137906 = aEtoHxLoBi98827805;     aEtoHxLoBi98827805 = aEtoHxLoBi98982673;     aEtoHxLoBi98982673 = aEtoHxLoBi94285122;     aEtoHxLoBi94285122 = aEtoHxLoBi92780184;     aEtoHxLoBi92780184 = aEtoHxLoBi55091821;     aEtoHxLoBi55091821 = aEtoHxLoBi83504858;     aEtoHxLoBi83504858 = aEtoHxLoBi14608414;     aEtoHxLoBi14608414 = aEtoHxLoBi73532469;     aEtoHxLoBi73532469 = aEtoHxLoBi89169871;     aEtoHxLoBi89169871 = aEtoHxLoBi29692442;     aEtoHxLoBi29692442 = aEtoHxLoBi44215633;     aEtoHxLoBi44215633 = aEtoHxLoBi96188137;     aEtoHxLoBi96188137 = aEtoHxLoBi92482555;     aEtoHxLoBi92482555 = aEtoHxLoBi41104046;     aEtoHxLoBi41104046 = aEtoHxLoBi18194190;     aEtoHxLoBi18194190 = aEtoHxLoBi34861718;     aEtoHxLoBi34861718 = aEtoHxLoBi34618417;     aEtoHxLoBi34618417 = aEtoHxLoBi81015191;     aEtoHxLoBi81015191 = aEtoHxLoBi40170511;     aEtoHxLoBi40170511 = aEtoHxLoBi36506930;     aEtoHxLoBi36506930 = aEtoHxLoBi89057919;     aEtoHxLoBi89057919 = aEtoHxLoBi35340623;     aEtoHxLoBi35340623 = aEtoHxLoBi3576104;     aEtoHxLoBi3576104 = aEtoHxLoBi79332747;     aEtoHxLoBi79332747 = aEtoHxLoBi66795800;     aEtoHxLoBi66795800 = aEtoHxLoBi30848310;     aEtoHxLoBi30848310 = aEtoHxLoBi45459981;     aEtoHxLoBi45459981 = aEtoHxLoBi87038291;     aEtoHxLoBi87038291 = aEtoHxLoBi86409011;     aEtoHxLoBi86409011 = aEtoHxLoBi36389266;     aEtoHxLoBi36389266 = aEtoHxLoBi41629;     aEtoHxLoBi41629 = aEtoHxLoBi2296829;     aEtoHxLoBi2296829 = aEtoHxLoBi64050523;     aEtoHxLoBi64050523 = aEtoHxLoBi85503489;     aEtoHxLoBi85503489 = aEtoHxLoBi43908226;     aEtoHxLoBi43908226 = aEtoHxLoBi90542465;     aEtoHxLoBi90542465 = aEtoHxLoBi91120573;     aEtoHxLoBi91120573 = aEtoHxLoBi13546374;     aEtoHxLoBi13546374 = aEtoHxLoBi4589560;     aEtoHxLoBi4589560 = aEtoHxLoBi27267964;     aEtoHxLoBi27267964 = aEtoHxLoBi63453247;     aEtoHxLoBi63453247 = aEtoHxLoBi54139035;     aEtoHxLoBi54139035 = aEtoHxLoBi76262734;     aEtoHxLoBi76262734 = aEtoHxLoBi27371949;     aEtoHxLoBi27371949 = aEtoHxLoBi38174915;     aEtoHxLoBi38174915 = aEtoHxLoBi596335;     aEtoHxLoBi596335 = aEtoHxLoBi71619703;     aEtoHxLoBi71619703 = aEtoHxLoBi36823006;     aEtoHxLoBi36823006 = aEtoHxLoBi30479954;     aEtoHxLoBi30479954 = aEtoHxLoBi77465024;     aEtoHxLoBi77465024 = aEtoHxLoBi78387243;     aEtoHxLoBi78387243 = aEtoHxLoBi48590428;     aEtoHxLoBi48590428 = aEtoHxLoBi63889411;     aEtoHxLoBi63889411 = aEtoHxLoBi30489529;     aEtoHxLoBi30489529 = aEtoHxLoBi11079987;     aEtoHxLoBi11079987 = aEtoHxLoBi63487183;     aEtoHxLoBi63487183 = aEtoHxLoBi95406570;     aEtoHxLoBi95406570 = aEtoHxLoBi14952375;     aEtoHxLoBi14952375 = aEtoHxLoBi25984385;     aEtoHxLoBi25984385 = aEtoHxLoBi24243511;     aEtoHxLoBi24243511 = aEtoHxLoBi38044878;     aEtoHxLoBi38044878 = aEtoHxLoBi27570123;     aEtoHxLoBi27570123 = aEtoHxLoBi87123457;     aEtoHxLoBi87123457 = aEtoHxLoBi52780605;     aEtoHxLoBi52780605 = aEtoHxLoBi29650813;     aEtoHxLoBi29650813 = aEtoHxLoBi41918804;     aEtoHxLoBi41918804 = aEtoHxLoBi32137614;     aEtoHxLoBi32137614 = aEtoHxLoBi6979066;     aEtoHxLoBi6979066 = aEtoHxLoBi97195820;     aEtoHxLoBi97195820 = aEtoHxLoBi27651725;     aEtoHxLoBi27651725 = aEtoHxLoBi43741145;     aEtoHxLoBi43741145 = aEtoHxLoBi21072043;     aEtoHxLoBi21072043 = aEtoHxLoBi76425632;     aEtoHxLoBi76425632 = aEtoHxLoBi12902547;     aEtoHxLoBi12902547 = aEtoHxLoBi73053682;     aEtoHxLoBi73053682 = aEtoHxLoBi34918885;     aEtoHxLoBi34918885 = aEtoHxLoBi59077888;     aEtoHxLoBi59077888 = aEtoHxLoBi76204155;     aEtoHxLoBi76204155 = aEtoHxLoBi41157833;     aEtoHxLoBi41157833 = aEtoHxLoBi66199466;     aEtoHxLoBi66199466 = aEtoHxLoBi59228607;     aEtoHxLoBi59228607 = aEtoHxLoBi8636975;     aEtoHxLoBi8636975 = aEtoHxLoBi56558337;     aEtoHxLoBi56558337 = aEtoHxLoBi8943988;     aEtoHxLoBi8943988 = aEtoHxLoBi58002022;     aEtoHxLoBi58002022 = aEtoHxLoBi51451201;     aEtoHxLoBi51451201 = aEtoHxLoBi38407417;     aEtoHxLoBi38407417 = aEtoHxLoBi33560995;     aEtoHxLoBi33560995 = aEtoHxLoBi74423503;     aEtoHxLoBi74423503 = aEtoHxLoBi80421043;     aEtoHxLoBi80421043 = aEtoHxLoBi95135895;     aEtoHxLoBi95135895 = aEtoHxLoBi76168198;     aEtoHxLoBi76168198 = aEtoHxLoBi87561989;     aEtoHxLoBi87561989 = aEtoHxLoBi80346048;     aEtoHxLoBi80346048 = aEtoHxLoBi89223086;     aEtoHxLoBi89223086 = aEtoHxLoBi35883125;     aEtoHxLoBi35883125 = aEtoHxLoBi67015577;     aEtoHxLoBi67015577 = aEtoHxLoBi23482129;     aEtoHxLoBi23482129 = aEtoHxLoBi66996459;}
// Junk Finished
