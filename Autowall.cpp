#include "Autowall.h"
#include "Menu.h"
#include "XorStr.hpp"
#include "RageBot.h"

inline bool CGameTrace::DidHitWorld() const
{
	return m_pEnt->GetIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}

bool autowall_2::did_hit_world(IClientEntity* ent) {
	return ent == interfaces::ent_list->get_client_entity(0);
}

bool autowall_2::did_hit_non_world_entity(IClientEntity* ent) {
	return ent != nullptr && !did_hit_world(ent);
}


backup_autowall * backup_awall = new backup_autowall;
autowall_2* new_autowall = new autowall_2;


#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1	
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

/*
void ScaleDamage_1(int hitgroup, IClientEntity* enemy, float weapon_armor_ratio, float& current_damage)
{
	int armor = enemy->ArmorValue();
	float ratio;

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage *= 4.f;
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	}

	if (armor > 0)
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			if (enemy->HasHelmet())
			{
				ratio = (weapon_armor_ratio * 0.5) * current_damage;
				if (((current_damage - ratio) * 0.5) > armor)
					ratio = current_damage - (armor * 2.0);
				current_damage = ratio;
			}
			break;
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			ratio = (weapon_armor_ratio * 0.5) * current_damage;
			if (((current_damage - ratio) * 0.5) > armor)
				ratio = current_damage - (armor * 2.0);
			current_damage = ratio;
			break;
		}
	}
}


void autowall_2::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces::trace->TraceRay(ray, mask, &filter, ptr);
}
*/
/*
void autowall_2::ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{

	IClientEntity *pLocal = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	static DWORD dwAddress = Utilities::Memory::FindPatternV2("client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}
*/
void autowall_2::ClipTraceToPlayers(Vector& absStart, Vector absEnd, unsigned int mask, CTraceFilter* filter,
	CGameTrace* tr, float minDistanceToRay)
{

	CGameTrace playerTrace;
	Ray_t ray;
	auto smallestFraction = tr->fraction;
	const auto maxRange = 60.0f;

	ray.Init2(absStart, absEnd);

	for (auto i = 1; i <= interfaces::globals->max_clients; i++) {
		auto player = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(i));

		if (!player || !player->IsAlive() || player->IsDormant())
			continue;

		if (filter && !filter->ShouldHitEntity(player, mask))
			continue;

		float a;
		Vector *c;

		auto range = Math_trash::distance_to_ray( player->world_space_center( ), absStart, absEnd, &a, c );

		if (range < minDistanceToRay || range > maxRange)
			continue;

		interfaces::trace_2->ClipRayToEntity(ray, mask | CONTENTS_HITBOX, player, &playerTrace);
		if (playerTrace.fraction < smallestFraction) {
			*tr = playerTrace;
			smallestFraction = playerTrace.fraction;
		}
	}
}

////////////////////////////////////// Legacy Functions //////////////////////////////////////
void autowall_2::GetBulletTypeParameters(float& maxRange, float& maxDistance, bool sv_penetration_type)
{
	if (sv_penetration_type)
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{

		IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
		C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

		if (pWeapon->GetWeaponID2() == WEAPON_AWP)
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}

		else if (pWeapon->GetWeaponID2() == WEAPON_SSG08 || pWeapon->GetWeaponID2() == WEAPON_SCAR20 || pWeapon->GetWeaponID2() == WEAPON_G3SG1)
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}

		else if (pWeapon->GetWeaponID2() == WEAPON_DEAGLE)
		{
			maxRange = 30.0;
			maxDistance = 3000.0;
		}

		else if (pWeapon->GetWeaponID2() == WEAPONTYPE_RIFLE)
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}

		else
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
	}
}

////////////////////////////////////// Misc Functions //////////////////////////////////////
/*
bool autowall_2::BreakableEntity(IClientEntity* entity)
{

	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
		return false;

	if (pClass == nullptr)
		return false;

//	if (pClass->m_ClassID = (int)CSGOClassID::CChicken)
//	return false;

	return pClass->m_ClassID == (int)CSGOClassID::CBreakableProp || pClass->m_ClassID == (int)CSGOClassID::CBreakableSurface;

}
void autowall_2::ScaleDamage(trace_t &enterTrace, CSWeaponInfo *weaponData, float& currentDamage)
{
	//Cred. to N0xius for reversing this.
	//TODO: _xAE^; look into reversing this yourself sometime

	bool hasHeavyArmor = false;
	int armorValue = ((IClientEntity*)enterTrace.m_pEnt)->ArmorValue();
	int hitGroup = enterTrace.hitgroup;

	//Fuck making a new function, lambda beste. ~ Does the person have armor on for the hitbox checked?
	auto IsArmored = [&enterTrace]()->bool
	{
		IClientEntity* targetEntity = (IClientEntity*)enterTrace.m_pEnt;
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return !!(IClientEntity*)targetEntity->HasHelmet(); //Fuck compiler errors - force-convert it to a bool via (!!)
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitGroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= 2.f; //Heavy Armor does 1/2 damage
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weaponData->armor_ratio / 2.f;

		//Damage gets modified for heavy armor users
		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = currentDamage * armorRatio;

		if (((currentDamage - (currentDamage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
			NewDamage = currentDamage - (armorValue / armorBonusRatio);

		currentDamage = NewDamage;
		damage_output = currentDamage;
	}
}
void tr_ln(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr) {
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces::trace->TraceRay(ray, mask, &filter, ptr);
}
////////////////////////////////////// Main Autowall Functions //////////////////////////////////////
bool autowall_2::TraceToExit(Vector& vecEnd, CGameTrace* pEnterTrace, Vector vecStart, Vector vecDir,
	CGameTrace* pExitTrace)
{
	auto dist = 0.f;

	while (dist <= 90.f) {
		dist += 4.f;
		vecEnd = vecStart + vecDir * dist;

		const auto point_contents = interfaces::trace->GetPointContents(
			vecEnd, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);
		if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX))
			continue;

		auto a1 = vecEnd - vecDir * 4.f;
		Ray_t ray;
		ray.Init(vecEnd, a1);

		interfaces::trace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, pExitTrace);

		if (pExitTrace->startpos != Vector(0, 0, 0) && pExitTrace->surface.flags & SURF_HITBOX) {
			CTraceFilterSkipEntity filter(pExitTrace->m_pEnt);
			Ray_t ray2;
			ray2.Init(vecEnd, vecStart);
			interfaces::trace->TraceRay(ray2, MASK_SHOT_HULL, &filter, pExitTrace);

			if ((pExitTrace->fraction < 1.f || pExitTrace->allsolid) && !pExitTrace->startsolid) {
				vecEnd = pExitTrace->endpos;
				return true;
			}

			continue;
		}

		if (!pExitTrace->DidHit() || pExitTrace->startsolid) {
			if (pEnterTrace->m_pEnt && (pEnterTrace->m_pEnt != nullptr && pEnterTrace->m_pEnt !=
				interfaces::ent_list->get_client_entity(0))) {
				*pExitTrace = *pEnterTrace;
				pExitTrace->endpos = vecStart + vecDir;
				return true;
			}

			continue;
		}

		if (!pExitTrace->DidHit() || pExitTrace->startsolid) {
			if (pEnterTrace->m_pEnt != nullptr && !pEnterTrace->m_pEnt->GetIndex() == 0 &&
				IsBreakableEntity(
					reinterpret_cast< IClientEntity* >(pEnterTrace->m_pEnt))) {
				*pExitTrace = *pEnterTrace;
				pExitTrace->endpos = vecStart + vecDir;
				return true;
			}

			continue;
		}

		if (pExitTrace->surface.flags >> 7 & SURF_LIGHT && !(pEnterTrace->surface.flags >> 7 & SURF_LIGHT))
			continue;

		if (pExitTrace->plane.normal.Dot(vecDir) <= 1.f) {
			vecEnd = vecEnd - vecDir * (pExitTrace->fraction * 4.f);
			return true;
		}
	}

	return false;
}

bool autowall_2::HandleBulletPenetration(CSWeaponInfo* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration)
{
	auto local = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(
		interfaces::engine->GetLocalPlayer()));
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr || !local || !weaponData || !local->IsAlive())
		throw std::invalid_argument("currentDamage is null!\n");

	Vector end;
	CGameTrace exitTrace;
	auto pEnemy = reinterpret_cast< IClientEntity* >(enterTrace.m_pEnt);
	if (!pEnemy || pEnemy->is_dormant() || !pEnemy->IsAlive()) return false;
	auto enterSurfaceData = interfaces::phys_props->GetSurfaceData(enterTrace.surface.surfaceProps);
	if (!enterSurfaceData) return false;
	int enterMaterial = enterSurfaceData->game.material;

	auto enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	auto enterDamageModifier = enterSurfaceData->game.flDamageModifier;
	float thickness, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (possibleHitsRemaining <= 0
		//Test for "DE_CACHE/DE_CACHE_TELA_03" as the entering surface and "CS_ITALY/CR_MISCWOOD2B" as the exiting surface.
		//Fixes a wall in de_cache which seems to be broken in some way. Although bullet penetration is not recorded to go through this wall
		//Decals can be seen of bullets within the glass behind of the enemy. Hacky method, but works.
		//You might want to have a check for this to only be activated on de_cache.
		|| (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial
			!= CHAR_TEX_GLASS)
		|| weaponData->penetration <= 0.f
		|| !TraceToExit(end, &enterTrace, enterTrace.endpos, direction, &exitTrace)
		&& !(interfaces::trace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL
			))
		return false;

	auto exitSurfaceData = interfaces::phys_props->
		GetSurfaceData(exitTrace.surface.surfaceProps);
	if (!exitSurfaceData) return false;
	int exitMaterial = exitSurfaceData->game.material;
	auto exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	auto exitDamageModifier = exitSurfaceData->game.flDamageModifier;

	//Are we using the newer penetration system?
	if (sv_penetration_type) {
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS) {
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf) {
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (local->team() == pEnemy->team() &&
			ff_damage_reduction_bullets == 0.f)) {
			if (ff_damage_bullet_penetration == 0.f)
				return false;

			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else {
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		//Do our materials line up?
		if (enterMaterial == exitMaterial) {
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
				combinedPenetrationModifier = 3.f;
			else if (exitMaterial == CHAR_TEX_PLASTIC)
				combinedPenetrationModifier = 2.f;
		}

		//Calculate thickness of the wall by getting the length of the range of the trace and squaring
		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
		auto modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		//This calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
		auto lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		//Did we loose too much damage?
		if (lostDamage > currentDamage)
			return false;

		//We can't use any of the damage that we've lost
		if (lostDamage > 0.f)
			currentDamage -= lostDamage;

		//Do we still have enough damage to deal?
		if (currentDamage < 1.f)
			return false;

		eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;

		return true;
	}
	else //Legacy penetration system
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
			finalDamageModifier = 0.99f; //Good meme :^)
		else {
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
			combinedPenetrationModifier += combinedPenetrationModifier;

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower) {
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.endpos;
			--possibleHitsRemaining;

			return true;
		}

		return false;
	}
}

bool autowall_2::FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage)
{
	if (!pWeapon)
		return false;

	IClientEntity* local = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	auto data = FireBulletData(local->GetEyePosition());

	data.filter = CTraceFilter();
	data.filter.pSkip = local;

	bool sv_penetration_type;

	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f;
	Vector eyePosition = local->GetEyePosition();

	static ConVar* penetrationSystem = interfaces::cvar->FindVar(("sv_penetration_type"));
	static ConVar* damageReductionBullets = interfaces::cvar->FindVar(("ff_damage_reduction_bullets"));
	static ConVar* damageBulletPenetration = interfaces::cvar->FindVar(("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetBool();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	damage_reduction = ff_damage_bullet_penetration;

	CSWeaponInfo* weaponData = pWeapon->GetCSWpnData();
	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = local;

	if (!weaponData)
		return false;

	maxRange = weaponData->range;

	GetBulletTypeParameters(penetrationPower, penetrationDistance, sv_penetration_type);

	if (sv_penetration_type)
		penetrationPower = weaponData->penetration;

	int possibleHitsRemaining = 4;

	currentDamage = weaponData->damage;

	while (possibleHitsRemaining > 0 && currentDamage >= 1.f)
	{
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, local, &enterTrace);
		ClipTraceToPlayers(eyePosition, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter,
			&enterTrace, 1.f); //  | CONTENTS_HITBOX

		surfacedata_t *enterSurfaceData = interfaces::phys_props->GetSurfaceData(enterTrace.surface.surfaceProps);

		if (enterSurfaceData == nullptr)
			return false;

		float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		int enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.f)
			break;

		currentDistance += enterTrace.fraction * maxRange;

		currentDamage *= pow(weaponData->range_modifier, (currentDistance / 500.f));

		if (currentDistance > penetrationDistance && weaponData->penetration > 0.f || enterSurfPenetrationModifier < 0.1f)
			break;

		auto canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);
		auto isPlayer = ((enterTrace.m_pEnt)->cs_player());

		bool isEnemy = (((IClientEntity*)local)->team() != ((IClientEntity*)enterTrace.m_pEnt)->team());
		auto onTeam = ((reinterpret_cast< IClientEntity* >(enterTrace.m_pEnt)->team() == 3 || (
			reinterpret_cast< IClientEntity* >(enterTrace.m_pEnt)->team() == 2)));

		if ((canDoDamage && isPlayer && isEnemy) && onTeam) {

			ScaleDamage(enterTrace, weaponData, currentDamage);
			damage_output = currentDamage;
			return true;
		}
		if (!HandleBulletPenetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration))
			break;
	}
	return false;
}

////////////////////////////////////// Usage Calls //////////////////////////////////////
bool autowall_2::CanHit(Vector &point)
{

	auto player = reinterpret_cast< IClientEntity* >(interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer()));
	if (!player || !player->IsAlive())
		return false;

	Vector angles, direction;
	auto tmp = point - player->GetEyePosition();
	float currentDamage = 0;

	VectorAngles(tmp, angles);
	AngleVectors(angles, &direction);
	direction.NormalizeInPlace();
	auto weapon = player->GetWeapon2();
	if (!weapon)
		return -1;
	if (FireBullet(weapon, direction, currentDamage))
	{
		if (damage_output >= options::menu.aimbot.AccuracyMinimumDamage.GetValue())
		{
			return true;
		}
	}

	return false; //That wall is just a bit too thick buddy
}

bool autowall_2::PenetrateWall(IClientEntity* pBaseEntity, Vector& vecPoint)
{
	float min_damage = options::menu.aimbot.AccuracyMinimumDamage.GetValue();
	if (pBaseEntity->GetHealth() <= min_damage)
		min_damage = pBaseEntity->GetHealth();

	if (CanHit(vecPoint) >= min_damage)
	{
		ragebot->can_autowall = true;
		return true;
	}
	else
		ragebot->can_autowall = false;

	return false;
}
*/


float backup_autowall::GetHitgroupDamageMult_2(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
		return 4.f;
	case HITGROUP_STOMACH:
		return 1.25f;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return 0.75f;
	default:
		return 1.f;
	}
}

void backup_autowall::ScaleDamage_2(int hitgroup, IClientEntity *enemy, float weapon_armor_ratio, float &current_damage)
{
	current_damage *= GetHitgroupDamageMult_2(hitgroup);
	int helmet = enemy->HasHelmet();
	int armor = enemy->ArmorValue();
	float ratio;
	if (armor > 0)
	{
		if (armor > 0)
		{
			switch (hitgroup)
			{
			case HITGROUP_HEAD:
				if (enemy->HasHelmet())
				{
					ratio = (weapon_armor_ratio * 0.5) * current_damage;
					if (((current_damage - ratio) * 0.5) > armor)
						ratio = current_damage - (armor * 2.0);
					current_damage = ratio;
				}
				break;
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				ratio = (weapon_armor_ratio * 0.5) * current_damage;
				if (((current_damage - ratio) * 0.5) > armor)
					ratio = current_damage - (armor * 2.0);
				current_damage = ratio;
				break;
			}
		}
	}
}


void angle_vectors_2(const Vector &angles, Vector& forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

bool backup_autowall::HandleBulletPenetration_2(CSWeaponInfo *wpn_data, FireBulletData &data)
{
	if (&data.current_damage == nullptr)
		throw std::invalid_argument(" current damage is nullptr ");

	surfacedata_t *enter_surface_data = interfaces::phys_props->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->range_modifier, (data.trace_length * 0.002));

	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;

	if (data.penetrate_count <= 0)
		return false;

	bool isSolidSurf = ((data.enter_trace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((data.enter_trace.surface.flags >> 7) & SURF_LIGHT);

	Vector dummy;
	trace_t trace_exit;

	if (!TraceToExit(dummy, data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit)) 
		return false;

	surfacedata_t *exit_surface_data = interfaces::phys_props->GetSurfaceData(trace_exit.surface.surfaceProps);

	int exit_material = exit_surface_data->game.material;

	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;

	if (((data.enter_trace.contents & CONTENTS_GRATE) != 0) || (enter_material == 89) || (enter_material == 71) || (enter_material == CHAR_TEX_GLASS))
	{ 
		combined_penetration_modifier = 3.0f; final_damage_modifier = 0.05f; 
	}
	else if (isSolidSurf || isLightSurf)
	{
		combined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else 
	{ 
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f; 
	}

	
	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.0f;
	}

	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->penetration) * 1.25f);
	float thickness = VectorLength(trace_exit.endpos - data.enter_trace.endpos);

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)return false;
	if (lost_damage >= 0.0f)data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f) return false;
	data.src = trace_exit.endpos;
	data.penetrate_count--;

	return true;
}

bool backup_autowall::SimulateFireBullet_2(IClientEntity *local, C_BaseCombatWeapon *weapon, FireBulletData &data)
{

	data.penetrate_count = 4; // Max Amount Of Penitration
	data.trace_length = 0.0f; // wow what a meme

	if (!weapon)
		return false;

	if (!&data)
		return false;

	auto *wpn_data = weapon->GetCSWpnData(); // Get Weapon Info

	if (!wpn_data)
		return false;

	data.current_damage = (float)wpn_data->damage;// Set Damage Memes
	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->range - data.trace_length;

		Vector End_Point = data.src + data.direction * data.trace_length_remaining;
		UTIL_TraceLine(data.src, End_Point, MASK_SHOT_HULL | CONTENTS_HITBOX, local, 0, &data.enter_trace);
		UTIL_ClipTraceToPlayers( data.src, End_Point * 40.f, MASK_SHOT_HULL | CONTENTS_HITBOX, &data.filter, &data.enter_trace );

		if (data.enter_trace.fraction == 1.0f)
			break;

		if ((data.enter_trace.hitgroup != HITGROUP_GENERIC) && (data.enter_trace.hitgroup <= 7) && (options::menu.misc.OtherSafeMode.getindex() != 2 ? local->team() != data.enter_trace.m_pEnt->team() : data.enter_trace.m_pEnt != local) )
		{
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->range, data.trace_length * 0.002);
			ScaleDamage_2(data.enter_trace.hitgroup, data.enter_trace.m_pEnt, wpn_data->armor_ratio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration_2(wpn_data, data))
			break;
	}

	return false;
}

bool backup_autowall::can_hit(const Vector &point, float *damage_given)
{
	auto *local = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());
	auto data = FireBulletData(local->GetOrigin() + local->GetViewOffset());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;

	Vector angles;
	CalcAngle(data.src, point, angles);
	angle_vectors_2(angles, data.direction);
	VectorNormalize(data.direction);

	if (SimulateFireBullet_2(local, (C_BaseCombatWeapon*)interfaces::ent_list->GetClientEntityFromHandle((HANDLE)local->GetActiveWeaponHandle()), data))
	{
		*damage_given = data.current_damage;

		if (data.current_damage >= options::menu.aimbot.AccuracyMinimumDamage.GetValue())
		{
			ragebot->can_autowall = true;
			return true;
		}
	}
	else
		ragebot->can_autowall = false;

	return false;
}







































































































































































// Junk Code By Troll Face & Thaisen's Gen
void oBgBaKbwPG72668196() {     int uWtYKfaejb45702911 = -79993934;    int uWtYKfaejb97473795 = -468227957;    int uWtYKfaejb27601390 = -52171665;    int uWtYKfaejb7318497 = -319208212;    int uWtYKfaejb85101673 = -789704034;    int uWtYKfaejb11410743 = -586718722;    int uWtYKfaejb40632279 = -198225013;    int uWtYKfaejb95959526 = 78537809;    int uWtYKfaejb40783912 = -831976265;    int uWtYKfaejb11540914 = -35646579;    int uWtYKfaejb51803105 = -958765429;    int uWtYKfaejb70803524 = -873680546;    int uWtYKfaejb52029371 = -508278841;    int uWtYKfaejb64314686 = -412687901;    int uWtYKfaejb63331853 = -571423292;    int uWtYKfaejb21723251 = 77831099;    int uWtYKfaejb16454812 = -765705675;    int uWtYKfaejb44124633 = -760290218;    int uWtYKfaejb24346456 = -697795755;    int uWtYKfaejb37776546 = -873799096;    int uWtYKfaejb39937305 = -602336174;    int uWtYKfaejb15084865 = -290253954;    int uWtYKfaejb75492141 = -16540260;    int uWtYKfaejb31766899 = -573188706;    int uWtYKfaejb18981952 = -227461062;    int uWtYKfaejb49417511 = -156591734;    int uWtYKfaejb42920572 = -800583549;    int uWtYKfaejb43506731 = -330862612;    int uWtYKfaejb72283331 = 16668108;    int uWtYKfaejb12002671 = 76339111;    int uWtYKfaejb78796155 = -187499449;    int uWtYKfaejb68920534 = -258211988;    int uWtYKfaejb70403023 = -139263740;    int uWtYKfaejb47677416 = -412811119;    int uWtYKfaejb78065852 = -435616174;    int uWtYKfaejb57541347 = -467822975;    int uWtYKfaejb70442131 = -72267550;    int uWtYKfaejb43331294 = -846705095;    int uWtYKfaejb59489062 = -89567602;    int uWtYKfaejb206127 = -298891874;    int uWtYKfaejb18526542 = -467783556;    int uWtYKfaejb94874907 = -464493359;    int uWtYKfaejb88215415 = -773364269;    int uWtYKfaejb64994074 = -699941652;    int uWtYKfaejb80634331 = -519308700;    int uWtYKfaejb84037416 = -735162831;    int uWtYKfaejb52694869 = -671037117;    int uWtYKfaejb72415045 = -982369858;    int uWtYKfaejb67680668 = -66722808;    int uWtYKfaejb70398934 = -628955591;    int uWtYKfaejb10694543 = -998883388;    int uWtYKfaejb57524207 = -456523954;    int uWtYKfaejb22189221 = -648428322;    int uWtYKfaejb34524890 = -771458427;    int uWtYKfaejb32730891 = -322394464;    int uWtYKfaejb30618046 = -789739981;    int uWtYKfaejb21981654 = -351687697;    int uWtYKfaejb95834491 = -478982959;    int uWtYKfaejb88336544 = 8252849;    int uWtYKfaejb35684162 = -533112300;    int uWtYKfaejb68490171 = -786135174;    int uWtYKfaejb97125547 = -867362401;    int uWtYKfaejb23676196 = -938130299;    int uWtYKfaejb28781241 = -808315376;    int uWtYKfaejb32744758 = -848147131;    int uWtYKfaejb82882570 = -600553442;    int uWtYKfaejb400502 = -634416806;    int uWtYKfaejb4351956 = 4532277;    int uWtYKfaejb86248833 = -977071728;    int uWtYKfaejb5790506 = -3600318;    int uWtYKfaejb51281119 = -849901352;    int uWtYKfaejb73123517 = -919000581;    int uWtYKfaejb84635571 = -570722616;    int uWtYKfaejb24140329 = -298903882;    int uWtYKfaejb19250005 = -306015541;    int uWtYKfaejb45062398 = -37842816;    int uWtYKfaejb26869449 = -516889686;    int uWtYKfaejb10498067 = -316598608;    int uWtYKfaejb51132568 = 46119993;    int uWtYKfaejb34944536 = -492298232;    int uWtYKfaejb96722642 = -485554618;    int uWtYKfaejb70505526 = -818213691;    int uWtYKfaejb75826063 = -164139805;    int uWtYKfaejb1884397 = -354376302;    int uWtYKfaejb1308128 = 75222498;    int uWtYKfaejb21271949 = -730975495;    int uWtYKfaejb46731314 = -609783667;    int uWtYKfaejb35878134 = -367805313;    int uWtYKfaejb14946525 = 9583345;    int uWtYKfaejb47447807 = -645876193;    int uWtYKfaejb35559693 = -16135278;    int uWtYKfaejb74607640 = -593284591;    int uWtYKfaejb54994749 = -754957945;    int uWtYKfaejb23804900 = -556455302;    int uWtYKfaejb31715956 = -512756700;    int uWtYKfaejb21400995 = -600421156;    int uWtYKfaejb71198712 = -526363060;    int uWtYKfaejb59434175 = -965048893;    int uWtYKfaejb32249316 = -851794522;    int uWtYKfaejb97751760 = -79993934;     uWtYKfaejb45702911 = uWtYKfaejb97473795;     uWtYKfaejb97473795 = uWtYKfaejb27601390;     uWtYKfaejb27601390 = uWtYKfaejb7318497;     uWtYKfaejb7318497 = uWtYKfaejb85101673;     uWtYKfaejb85101673 = uWtYKfaejb11410743;     uWtYKfaejb11410743 = uWtYKfaejb40632279;     uWtYKfaejb40632279 = uWtYKfaejb95959526;     uWtYKfaejb95959526 = uWtYKfaejb40783912;     uWtYKfaejb40783912 = uWtYKfaejb11540914;     uWtYKfaejb11540914 = uWtYKfaejb51803105;     uWtYKfaejb51803105 = uWtYKfaejb70803524;     uWtYKfaejb70803524 = uWtYKfaejb52029371;     uWtYKfaejb52029371 = uWtYKfaejb64314686;     uWtYKfaejb64314686 = uWtYKfaejb63331853;     uWtYKfaejb63331853 = uWtYKfaejb21723251;     uWtYKfaejb21723251 = uWtYKfaejb16454812;     uWtYKfaejb16454812 = uWtYKfaejb44124633;     uWtYKfaejb44124633 = uWtYKfaejb24346456;     uWtYKfaejb24346456 = uWtYKfaejb37776546;     uWtYKfaejb37776546 = uWtYKfaejb39937305;     uWtYKfaejb39937305 = uWtYKfaejb15084865;     uWtYKfaejb15084865 = uWtYKfaejb75492141;     uWtYKfaejb75492141 = uWtYKfaejb31766899;     uWtYKfaejb31766899 = uWtYKfaejb18981952;     uWtYKfaejb18981952 = uWtYKfaejb49417511;     uWtYKfaejb49417511 = uWtYKfaejb42920572;     uWtYKfaejb42920572 = uWtYKfaejb43506731;     uWtYKfaejb43506731 = uWtYKfaejb72283331;     uWtYKfaejb72283331 = uWtYKfaejb12002671;     uWtYKfaejb12002671 = uWtYKfaejb78796155;     uWtYKfaejb78796155 = uWtYKfaejb68920534;     uWtYKfaejb68920534 = uWtYKfaejb70403023;     uWtYKfaejb70403023 = uWtYKfaejb47677416;     uWtYKfaejb47677416 = uWtYKfaejb78065852;     uWtYKfaejb78065852 = uWtYKfaejb57541347;     uWtYKfaejb57541347 = uWtYKfaejb70442131;     uWtYKfaejb70442131 = uWtYKfaejb43331294;     uWtYKfaejb43331294 = uWtYKfaejb59489062;     uWtYKfaejb59489062 = uWtYKfaejb206127;     uWtYKfaejb206127 = uWtYKfaejb18526542;     uWtYKfaejb18526542 = uWtYKfaejb94874907;     uWtYKfaejb94874907 = uWtYKfaejb88215415;     uWtYKfaejb88215415 = uWtYKfaejb64994074;     uWtYKfaejb64994074 = uWtYKfaejb80634331;     uWtYKfaejb80634331 = uWtYKfaejb84037416;     uWtYKfaejb84037416 = uWtYKfaejb52694869;     uWtYKfaejb52694869 = uWtYKfaejb72415045;     uWtYKfaejb72415045 = uWtYKfaejb67680668;     uWtYKfaejb67680668 = uWtYKfaejb70398934;     uWtYKfaejb70398934 = uWtYKfaejb10694543;     uWtYKfaejb10694543 = uWtYKfaejb57524207;     uWtYKfaejb57524207 = uWtYKfaejb22189221;     uWtYKfaejb22189221 = uWtYKfaejb34524890;     uWtYKfaejb34524890 = uWtYKfaejb32730891;     uWtYKfaejb32730891 = uWtYKfaejb30618046;     uWtYKfaejb30618046 = uWtYKfaejb21981654;     uWtYKfaejb21981654 = uWtYKfaejb95834491;     uWtYKfaejb95834491 = uWtYKfaejb88336544;     uWtYKfaejb88336544 = uWtYKfaejb35684162;     uWtYKfaejb35684162 = uWtYKfaejb68490171;     uWtYKfaejb68490171 = uWtYKfaejb97125547;     uWtYKfaejb97125547 = uWtYKfaejb23676196;     uWtYKfaejb23676196 = uWtYKfaejb28781241;     uWtYKfaejb28781241 = uWtYKfaejb32744758;     uWtYKfaejb32744758 = uWtYKfaejb82882570;     uWtYKfaejb82882570 = uWtYKfaejb400502;     uWtYKfaejb400502 = uWtYKfaejb4351956;     uWtYKfaejb4351956 = uWtYKfaejb86248833;     uWtYKfaejb86248833 = uWtYKfaejb5790506;     uWtYKfaejb5790506 = uWtYKfaejb51281119;     uWtYKfaejb51281119 = uWtYKfaejb73123517;     uWtYKfaejb73123517 = uWtYKfaejb84635571;     uWtYKfaejb84635571 = uWtYKfaejb24140329;     uWtYKfaejb24140329 = uWtYKfaejb19250005;     uWtYKfaejb19250005 = uWtYKfaejb45062398;     uWtYKfaejb45062398 = uWtYKfaejb26869449;     uWtYKfaejb26869449 = uWtYKfaejb10498067;     uWtYKfaejb10498067 = uWtYKfaejb51132568;     uWtYKfaejb51132568 = uWtYKfaejb34944536;     uWtYKfaejb34944536 = uWtYKfaejb96722642;     uWtYKfaejb96722642 = uWtYKfaejb70505526;     uWtYKfaejb70505526 = uWtYKfaejb75826063;     uWtYKfaejb75826063 = uWtYKfaejb1884397;     uWtYKfaejb1884397 = uWtYKfaejb1308128;     uWtYKfaejb1308128 = uWtYKfaejb21271949;     uWtYKfaejb21271949 = uWtYKfaejb46731314;     uWtYKfaejb46731314 = uWtYKfaejb35878134;     uWtYKfaejb35878134 = uWtYKfaejb14946525;     uWtYKfaejb14946525 = uWtYKfaejb47447807;     uWtYKfaejb47447807 = uWtYKfaejb35559693;     uWtYKfaejb35559693 = uWtYKfaejb74607640;     uWtYKfaejb74607640 = uWtYKfaejb54994749;     uWtYKfaejb54994749 = uWtYKfaejb23804900;     uWtYKfaejb23804900 = uWtYKfaejb31715956;     uWtYKfaejb31715956 = uWtYKfaejb21400995;     uWtYKfaejb21400995 = uWtYKfaejb71198712;     uWtYKfaejb71198712 = uWtYKfaejb59434175;     uWtYKfaejb59434175 = uWtYKfaejb32249316;     uWtYKfaejb32249316 = uWtYKfaejb97751760;     uWtYKfaejb97751760 = uWtYKfaejb45702911;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NJvyOepSyK43124070() {     int VZLqgPNjAm58489078 = -215034626;    int VZLqgPNjAm84244683 = -638561970;    int VZLqgPNjAm45354190 = -460531047;    int VZLqgPNjAm15259323 = -143410489;    int VZLqgPNjAm78612777 = -181520499;    int VZLqgPNjAm54720814 = -363642036;    int VZLqgPNjAm61840037 = -743897083;    int VZLqgPNjAm22678639 = -365980448;    int VZLqgPNjAm36188722 = -793734357;    int VZLqgPNjAm17759618 = -939734212;    int VZLqgPNjAm79290487 = 26548555;    int VZLqgPNjAm2754030 = -281013361;    int VZLqgPNjAm22172674 = -412061498;    int VZLqgPNjAm13938765 = -745103099;    int VZLqgPNjAm86638405 = -478409123;    int VZLqgPNjAm50148319 = -473060290;    int VZLqgPNjAm72206629 = -429480142;    int VZLqgPNjAm27683119 = -315500520;    int VZLqgPNjAm25507086 = -607264951;    int VZLqgPNjAm40431043 = -373043235;    int VZLqgPNjAm20111362 = 6347562;    int VZLqgPNjAm44132684 = -460112302;    int VZLqgPNjAm14252437 = 52455845;    int VZLqgPNjAm76274028 = -933533719;    int VZLqgPNjAm24876212 = -437902987;    int VZLqgPNjAm30736157 = -947022933;    int VZLqgPNjAm54336108 = -334289829;    int VZLqgPNjAm10859657 = -504660717;    int VZLqgPNjAm88900384 = -197549133;    int VZLqgPNjAm64753786 = -653547044;    int VZLqgPNjAm23232152 = -463916199;    int VZLqgPNjAm72239358 = -236931666;    int VZLqgPNjAm16448972 = -15118524;    int VZLqgPNjAm24753789 = -317565337;    int VZLqgPNjAm61126385 = 54351946;    int VZLqgPNjAm52765462 = -256386079;    int VZLqgPNjAm7206192 = -962176900;    int VZLqgPNjAm1035317 = -766325186;    int VZLqgPNjAm65264448 = 68581821;    int VZLqgPNjAm26470857 = -143308719;    int VZLqgPNjAm79957849 = -843257549;    int VZLqgPNjAm2577363 = -362804693;    int VZLqgPNjAm23098886 = -796797714;    int VZLqgPNjAm9622612 = 58886105;    int VZLqgPNjAm60642821 = -412078660;    int VZLqgPNjAm57665546 = -808481271;    int VZLqgPNjAm30276888 = -68552169;    int VZLqgPNjAm1855012 = -37722638;    int VZLqgPNjAm11744718 = -230324783;    int VZLqgPNjAm17349355 = -139992054;    int VZLqgPNjAm29022037 = -151767696;    int VZLqgPNjAm85700754 = -927709292;    int VZLqgPNjAm56674371 = -824431604;    int VZLqgPNjAm79224615 = -360910169;    int VZLqgPNjAm23012708 = -316838951;    int VZLqgPNjAm14356395 = -754922325;    int VZLqgPNjAm69992246 = -591017816;    int VZLqgPNjAm69080161 = -526997329;    int VZLqgPNjAm90383111 = -705507503;    int VZLqgPNjAm47876621 = -234497566;    int VZLqgPNjAm384706 = 70647793;    int VZLqgPNjAm50980380 = -139236366;    int VZLqgPNjAm33778255 = -68431316;    int VZLqgPNjAm71434936 = -40187314;    int VZLqgPNjAm94527466 = -375818013;    int VZLqgPNjAm7051130 = -736519779;    int VZLqgPNjAm86305057 = -165894838;    int VZLqgPNjAm97418885 = 5503839;    int VZLqgPNjAm52812380 = -699455045;    int VZLqgPNjAm33872944 = -122023045;    int VZLqgPNjAm42942127 = -510883390;    int VZLqgPNjAm71171312 = -663154956;    int VZLqgPNjAm62418671 = -284082341;    int VZLqgPNjAm99036229 = -363956232;    int VZLqgPNjAm60473193 = -529785687;    int VZLqgPNjAm17534000 = -630847746;    int VZLqgPNjAm21033799 = -663314589;    int VZLqgPNjAm4629825 = 93569740;    int VZLqgPNjAm15631207 = -421455059;    int VZLqgPNjAm67210665 = -629421717;    int VZLqgPNjAm459269 = -778470765;    int VZLqgPNjAm52481096 = -196567192;    int VZLqgPNjAm99114938 = -174335935;    int VZLqgPNjAm71551029 = 42442921;    int VZLqgPNjAm35731750 = -401779348;    int VZLqgPNjAm37531397 = -536206907;    int VZLqgPNjAm15564988 = -412500063;    int VZLqgPNjAm37224357 = -654208356;    int VZLqgPNjAm1741082 = 99273613;    int VZLqgPNjAm46769991 = -190725730;    int VZLqgPNjAm82773215 = -665368264;    int VZLqgPNjAm38126030 = -335179572;    int VZLqgPNjAm10652206 = 39182316;    int VZLqgPNjAm17387827 = -696920613;    int VZLqgPNjAm26086151 = -113956513;    int VZLqgPNjAm28977470 = -604021183;    int VZLqgPNjAm68799108 = -194373378;    int VZLqgPNjAm51663950 = -656610400;    int VZLqgPNjAm15095145 = -565295882;    int VZLqgPNjAm53591692 = -215034626;     VZLqgPNjAm58489078 = VZLqgPNjAm84244683;     VZLqgPNjAm84244683 = VZLqgPNjAm45354190;     VZLqgPNjAm45354190 = VZLqgPNjAm15259323;     VZLqgPNjAm15259323 = VZLqgPNjAm78612777;     VZLqgPNjAm78612777 = VZLqgPNjAm54720814;     VZLqgPNjAm54720814 = VZLqgPNjAm61840037;     VZLqgPNjAm61840037 = VZLqgPNjAm22678639;     VZLqgPNjAm22678639 = VZLqgPNjAm36188722;     VZLqgPNjAm36188722 = VZLqgPNjAm17759618;     VZLqgPNjAm17759618 = VZLqgPNjAm79290487;     VZLqgPNjAm79290487 = VZLqgPNjAm2754030;     VZLqgPNjAm2754030 = VZLqgPNjAm22172674;     VZLqgPNjAm22172674 = VZLqgPNjAm13938765;     VZLqgPNjAm13938765 = VZLqgPNjAm86638405;     VZLqgPNjAm86638405 = VZLqgPNjAm50148319;     VZLqgPNjAm50148319 = VZLqgPNjAm72206629;     VZLqgPNjAm72206629 = VZLqgPNjAm27683119;     VZLqgPNjAm27683119 = VZLqgPNjAm25507086;     VZLqgPNjAm25507086 = VZLqgPNjAm40431043;     VZLqgPNjAm40431043 = VZLqgPNjAm20111362;     VZLqgPNjAm20111362 = VZLqgPNjAm44132684;     VZLqgPNjAm44132684 = VZLqgPNjAm14252437;     VZLqgPNjAm14252437 = VZLqgPNjAm76274028;     VZLqgPNjAm76274028 = VZLqgPNjAm24876212;     VZLqgPNjAm24876212 = VZLqgPNjAm30736157;     VZLqgPNjAm30736157 = VZLqgPNjAm54336108;     VZLqgPNjAm54336108 = VZLqgPNjAm10859657;     VZLqgPNjAm10859657 = VZLqgPNjAm88900384;     VZLqgPNjAm88900384 = VZLqgPNjAm64753786;     VZLqgPNjAm64753786 = VZLqgPNjAm23232152;     VZLqgPNjAm23232152 = VZLqgPNjAm72239358;     VZLqgPNjAm72239358 = VZLqgPNjAm16448972;     VZLqgPNjAm16448972 = VZLqgPNjAm24753789;     VZLqgPNjAm24753789 = VZLqgPNjAm61126385;     VZLqgPNjAm61126385 = VZLqgPNjAm52765462;     VZLqgPNjAm52765462 = VZLqgPNjAm7206192;     VZLqgPNjAm7206192 = VZLqgPNjAm1035317;     VZLqgPNjAm1035317 = VZLqgPNjAm65264448;     VZLqgPNjAm65264448 = VZLqgPNjAm26470857;     VZLqgPNjAm26470857 = VZLqgPNjAm79957849;     VZLqgPNjAm79957849 = VZLqgPNjAm2577363;     VZLqgPNjAm2577363 = VZLqgPNjAm23098886;     VZLqgPNjAm23098886 = VZLqgPNjAm9622612;     VZLqgPNjAm9622612 = VZLqgPNjAm60642821;     VZLqgPNjAm60642821 = VZLqgPNjAm57665546;     VZLqgPNjAm57665546 = VZLqgPNjAm30276888;     VZLqgPNjAm30276888 = VZLqgPNjAm1855012;     VZLqgPNjAm1855012 = VZLqgPNjAm11744718;     VZLqgPNjAm11744718 = VZLqgPNjAm17349355;     VZLqgPNjAm17349355 = VZLqgPNjAm29022037;     VZLqgPNjAm29022037 = VZLqgPNjAm85700754;     VZLqgPNjAm85700754 = VZLqgPNjAm56674371;     VZLqgPNjAm56674371 = VZLqgPNjAm79224615;     VZLqgPNjAm79224615 = VZLqgPNjAm23012708;     VZLqgPNjAm23012708 = VZLqgPNjAm14356395;     VZLqgPNjAm14356395 = VZLqgPNjAm69992246;     VZLqgPNjAm69992246 = VZLqgPNjAm69080161;     VZLqgPNjAm69080161 = VZLqgPNjAm90383111;     VZLqgPNjAm90383111 = VZLqgPNjAm47876621;     VZLqgPNjAm47876621 = VZLqgPNjAm384706;     VZLqgPNjAm384706 = VZLqgPNjAm50980380;     VZLqgPNjAm50980380 = VZLqgPNjAm33778255;     VZLqgPNjAm33778255 = VZLqgPNjAm71434936;     VZLqgPNjAm71434936 = VZLqgPNjAm94527466;     VZLqgPNjAm94527466 = VZLqgPNjAm7051130;     VZLqgPNjAm7051130 = VZLqgPNjAm86305057;     VZLqgPNjAm86305057 = VZLqgPNjAm97418885;     VZLqgPNjAm97418885 = VZLqgPNjAm52812380;     VZLqgPNjAm52812380 = VZLqgPNjAm33872944;     VZLqgPNjAm33872944 = VZLqgPNjAm42942127;     VZLqgPNjAm42942127 = VZLqgPNjAm71171312;     VZLqgPNjAm71171312 = VZLqgPNjAm62418671;     VZLqgPNjAm62418671 = VZLqgPNjAm99036229;     VZLqgPNjAm99036229 = VZLqgPNjAm60473193;     VZLqgPNjAm60473193 = VZLqgPNjAm17534000;     VZLqgPNjAm17534000 = VZLqgPNjAm21033799;     VZLqgPNjAm21033799 = VZLqgPNjAm4629825;     VZLqgPNjAm4629825 = VZLqgPNjAm15631207;     VZLqgPNjAm15631207 = VZLqgPNjAm67210665;     VZLqgPNjAm67210665 = VZLqgPNjAm459269;     VZLqgPNjAm459269 = VZLqgPNjAm52481096;     VZLqgPNjAm52481096 = VZLqgPNjAm99114938;     VZLqgPNjAm99114938 = VZLqgPNjAm71551029;     VZLqgPNjAm71551029 = VZLqgPNjAm35731750;     VZLqgPNjAm35731750 = VZLqgPNjAm37531397;     VZLqgPNjAm37531397 = VZLqgPNjAm15564988;     VZLqgPNjAm15564988 = VZLqgPNjAm37224357;     VZLqgPNjAm37224357 = VZLqgPNjAm1741082;     VZLqgPNjAm1741082 = VZLqgPNjAm46769991;     VZLqgPNjAm46769991 = VZLqgPNjAm82773215;     VZLqgPNjAm82773215 = VZLqgPNjAm38126030;     VZLqgPNjAm38126030 = VZLqgPNjAm10652206;     VZLqgPNjAm10652206 = VZLqgPNjAm17387827;     VZLqgPNjAm17387827 = VZLqgPNjAm26086151;     VZLqgPNjAm26086151 = VZLqgPNjAm28977470;     VZLqgPNjAm28977470 = VZLqgPNjAm68799108;     VZLqgPNjAm68799108 = VZLqgPNjAm51663950;     VZLqgPNjAm51663950 = VZLqgPNjAm15095145;     VZLqgPNjAm15095145 = VZLqgPNjAm53591692;     VZLqgPNjAm53591692 = VZLqgPNjAm58489078;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vwojFhpkfs61337411() {     int NsLbaxzVzn41933384 = 4299693;    int NsLbaxzVzn1116773 = -637944158;    int NsLbaxzVzn41762697 = -254056275;    int NsLbaxzVzn78051574 = -337210325;    int NsLbaxzVzn84734101 = 11612010;    int NsLbaxzVzn53225214 = -142713689;    int NsLbaxzVzn32879355 = -483808396;    int NsLbaxzVzn37026082 = -353138477;    int NsLbaxzVzn94718422 = -35193708;    int NsLbaxzVzn50305270 = -308110324;    int NsLbaxzVzn28706425 = -163567317;    int NsLbaxzVzn65959428 = -268631608;    int NsLbaxzVzn40290450 = -203722739;    int NsLbaxzVzn93565656 = -533393456;    int NsLbaxzVzn47492869 = -572432105;    int NsLbaxzVzn36537481 = -261139615;    int NsLbaxzVzn65958514 = -182825169;    int NsLbaxzVzn8534565 = -562805396;    int NsLbaxzVzn87947228 = -605939822;    int NsLbaxzVzn29798485 = -902629228;    int NsLbaxzVzn75751749 = -683181469;    int NsLbaxzVzn56280002 = -451264072;    int NsLbaxzVzn18078145 = 12566648;    int NsLbaxzVzn10980587 = 94640885;    int NsLbaxzVzn66452169 = -997555746;    int NsLbaxzVzn26078306 = -827532649;    int NsLbaxzVzn3790848 = 37625828;    int NsLbaxzVzn50591282 = -574615601;    int NsLbaxzVzn24981478 = -60054275;    int NsLbaxzVzn80587127 = -66396895;    int NsLbaxzVzn41327465 = -101673400;    int NsLbaxzVzn35345785 = -152126746;    int NsLbaxzVzn59987859 = -735990702;    int NsLbaxzVzn66912299 = 90832493;    int NsLbaxzVzn39853115 = -143312790;    int NsLbaxzVzn65154646 = -44409985;    int NsLbaxzVzn75820370 = -841026080;    int NsLbaxzVzn41614149 = -474795706;    int NsLbaxzVzn14259526 = -116315977;    int NsLbaxzVzn75965991 = -995902242;    int NsLbaxzVzn13606774 = -414806522;    int NsLbaxzVzn53065149 = -936713014;    int NsLbaxzVzn46665955 = -247854956;    int NsLbaxzVzn1527552 = -797296666;    int NsLbaxzVzn52587318 = -140235613;    int NsLbaxzVzn97271541 = -828092194;    int NsLbaxzVzn6292319 = -200856398;    int NsLbaxzVzn21525608 = -36444356;    int NsLbaxzVzn42591830 = -815926865;    int NsLbaxzVzn52681839 = -347012509;    int NsLbaxzVzn2279485 = -372669118;    int NsLbaxzVzn41201549 = -985968604;    int NsLbaxzVzn70786829 = 63821079;    int NsLbaxzVzn3901374 = -79323557;    int NsLbaxzVzn85585661 = -665325074;    int NsLbaxzVzn85653381 = -544436235;    int NsLbaxzVzn83038628 = -550510806;    int NsLbaxzVzn30782110 = -248697161;    int NsLbaxzVzn11599405 = -339654580;    int NsLbaxzVzn58655796 = -160855341;    int NsLbaxzVzn49434366 = -80339518;    int NsLbaxzVzn82288073 = -909192796;    int NsLbaxzVzn12044604 = -193084203;    int NsLbaxzVzn14131296 = -968796813;    int NsLbaxzVzn8977805 = -106436925;    int NsLbaxzVzn93360640 = 88559428;    int NsLbaxzVzn5971570 = -532640907;    int NsLbaxzVzn73378151 = -194555232;    int NsLbaxzVzn53712541 = -290080667;    int NsLbaxzVzn82338223 = -428022121;    int NsLbaxzVzn60717110 = -420113536;    int NsLbaxzVzn24344365 = -708029463;    int NsLbaxzVzn94275039 = -346489420;    int NsLbaxzVzn11981237 = -610037580;    int NsLbaxzVzn16191711 = -387822706;    int NsLbaxzVzn22686600 = -746468455;    int NsLbaxzVzn9614047 = -103409116;    int NsLbaxzVzn16550593 = -190136687;    int NsLbaxzVzn58393268 = -765123502;    int NsLbaxzVzn69180628 = -69463552;    int NsLbaxzVzn19785987 = -526676251;    int NsLbaxzVzn82265240 = -925929816;    int NsLbaxzVzn7999452 = -758688736;    int NsLbaxzVzn72299639 = -713041766;    int NsLbaxzVzn78307642 = -693727777;    int NsLbaxzVzn125917 = -115704797;    int NsLbaxzVzn64558955 = -115947826;    int NsLbaxzVzn56086485 = -556667146;    int NsLbaxzVzn81326637 = -243842434;    int NsLbaxzVzn54199734 = -598876555;    int NsLbaxzVzn82116018 = -493899179;    int NsLbaxzVzn45038260 = -492328920;    int NsLbaxzVzn30014745 = -35141127;    int NsLbaxzVzn55603730 = -955460636;    int NsLbaxzVzn26531625 = -815562724;    int NsLbaxzVzn31318701 = -505613727;    int NsLbaxzVzn41020546 = -643628812;    int NsLbaxzVzn32534660 = -279058144;    int NsLbaxzVzn92549747 = -590859742;    int NsLbaxzVzn59226678 = 4299693;     NsLbaxzVzn41933384 = NsLbaxzVzn1116773;     NsLbaxzVzn1116773 = NsLbaxzVzn41762697;     NsLbaxzVzn41762697 = NsLbaxzVzn78051574;     NsLbaxzVzn78051574 = NsLbaxzVzn84734101;     NsLbaxzVzn84734101 = NsLbaxzVzn53225214;     NsLbaxzVzn53225214 = NsLbaxzVzn32879355;     NsLbaxzVzn32879355 = NsLbaxzVzn37026082;     NsLbaxzVzn37026082 = NsLbaxzVzn94718422;     NsLbaxzVzn94718422 = NsLbaxzVzn50305270;     NsLbaxzVzn50305270 = NsLbaxzVzn28706425;     NsLbaxzVzn28706425 = NsLbaxzVzn65959428;     NsLbaxzVzn65959428 = NsLbaxzVzn40290450;     NsLbaxzVzn40290450 = NsLbaxzVzn93565656;     NsLbaxzVzn93565656 = NsLbaxzVzn47492869;     NsLbaxzVzn47492869 = NsLbaxzVzn36537481;     NsLbaxzVzn36537481 = NsLbaxzVzn65958514;     NsLbaxzVzn65958514 = NsLbaxzVzn8534565;     NsLbaxzVzn8534565 = NsLbaxzVzn87947228;     NsLbaxzVzn87947228 = NsLbaxzVzn29798485;     NsLbaxzVzn29798485 = NsLbaxzVzn75751749;     NsLbaxzVzn75751749 = NsLbaxzVzn56280002;     NsLbaxzVzn56280002 = NsLbaxzVzn18078145;     NsLbaxzVzn18078145 = NsLbaxzVzn10980587;     NsLbaxzVzn10980587 = NsLbaxzVzn66452169;     NsLbaxzVzn66452169 = NsLbaxzVzn26078306;     NsLbaxzVzn26078306 = NsLbaxzVzn3790848;     NsLbaxzVzn3790848 = NsLbaxzVzn50591282;     NsLbaxzVzn50591282 = NsLbaxzVzn24981478;     NsLbaxzVzn24981478 = NsLbaxzVzn80587127;     NsLbaxzVzn80587127 = NsLbaxzVzn41327465;     NsLbaxzVzn41327465 = NsLbaxzVzn35345785;     NsLbaxzVzn35345785 = NsLbaxzVzn59987859;     NsLbaxzVzn59987859 = NsLbaxzVzn66912299;     NsLbaxzVzn66912299 = NsLbaxzVzn39853115;     NsLbaxzVzn39853115 = NsLbaxzVzn65154646;     NsLbaxzVzn65154646 = NsLbaxzVzn75820370;     NsLbaxzVzn75820370 = NsLbaxzVzn41614149;     NsLbaxzVzn41614149 = NsLbaxzVzn14259526;     NsLbaxzVzn14259526 = NsLbaxzVzn75965991;     NsLbaxzVzn75965991 = NsLbaxzVzn13606774;     NsLbaxzVzn13606774 = NsLbaxzVzn53065149;     NsLbaxzVzn53065149 = NsLbaxzVzn46665955;     NsLbaxzVzn46665955 = NsLbaxzVzn1527552;     NsLbaxzVzn1527552 = NsLbaxzVzn52587318;     NsLbaxzVzn52587318 = NsLbaxzVzn97271541;     NsLbaxzVzn97271541 = NsLbaxzVzn6292319;     NsLbaxzVzn6292319 = NsLbaxzVzn21525608;     NsLbaxzVzn21525608 = NsLbaxzVzn42591830;     NsLbaxzVzn42591830 = NsLbaxzVzn52681839;     NsLbaxzVzn52681839 = NsLbaxzVzn2279485;     NsLbaxzVzn2279485 = NsLbaxzVzn41201549;     NsLbaxzVzn41201549 = NsLbaxzVzn70786829;     NsLbaxzVzn70786829 = NsLbaxzVzn3901374;     NsLbaxzVzn3901374 = NsLbaxzVzn85585661;     NsLbaxzVzn85585661 = NsLbaxzVzn85653381;     NsLbaxzVzn85653381 = NsLbaxzVzn83038628;     NsLbaxzVzn83038628 = NsLbaxzVzn30782110;     NsLbaxzVzn30782110 = NsLbaxzVzn11599405;     NsLbaxzVzn11599405 = NsLbaxzVzn58655796;     NsLbaxzVzn58655796 = NsLbaxzVzn49434366;     NsLbaxzVzn49434366 = NsLbaxzVzn82288073;     NsLbaxzVzn82288073 = NsLbaxzVzn12044604;     NsLbaxzVzn12044604 = NsLbaxzVzn14131296;     NsLbaxzVzn14131296 = NsLbaxzVzn8977805;     NsLbaxzVzn8977805 = NsLbaxzVzn93360640;     NsLbaxzVzn93360640 = NsLbaxzVzn5971570;     NsLbaxzVzn5971570 = NsLbaxzVzn73378151;     NsLbaxzVzn73378151 = NsLbaxzVzn53712541;     NsLbaxzVzn53712541 = NsLbaxzVzn82338223;     NsLbaxzVzn82338223 = NsLbaxzVzn60717110;     NsLbaxzVzn60717110 = NsLbaxzVzn24344365;     NsLbaxzVzn24344365 = NsLbaxzVzn94275039;     NsLbaxzVzn94275039 = NsLbaxzVzn11981237;     NsLbaxzVzn11981237 = NsLbaxzVzn16191711;     NsLbaxzVzn16191711 = NsLbaxzVzn22686600;     NsLbaxzVzn22686600 = NsLbaxzVzn9614047;     NsLbaxzVzn9614047 = NsLbaxzVzn16550593;     NsLbaxzVzn16550593 = NsLbaxzVzn58393268;     NsLbaxzVzn58393268 = NsLbaxzVzn69180628;     NsLbaxzVzn69180628 = NsLbaxzVzn19785987;     NsLbaxzVzn19785987 = NsLbaxzVzn82265240;     NsLbaxzVzn82265240 = NsLbaxzVzn7999452;     NsLbaxzVzn7999452 = NsLbaxzVzn72299639;     NsLbaxzVzn72299639 = NsLbaxzVzn78307642;     NsLbaxzVzn78307642 = NsLbaxzVzn125917;     NsLbaxzVzn125917 = NsLbaxzVzn64558955;     NsLbaxzVzn64558955 = NsLbaxzVzn56086485;     NsLbaxzVzn56086485 = NsLbaxzVzn81326637;     NsLbaxzVzn81326637 = NsLbaxzVzn54199734;     NsLbaxzVzn54199734 = NsLbaxzVzn82116018;     NsLbaxzVzn82116018 = NsLbaxzVzn45038260;     NsLbaxzVzn45038260 = NsLbaxzVzn30014745;     NsLbaxzVzn30014745 = NsLbaxzVzn55603730;     NsLbaxzVzn55603730 = NsLbaxzVzn26531625;     NsLbaxzVzn26531625 = NsLbaxzVzn31318701;     NsLbaxzVzn31318701 = NsLbaxzVzn41020546;     NsLbaxzVzn41020546 = NsLbaxzVzn32534660;     NsLbaxzVzn32534660 = NsLbaxzVzn92549747;     NsLbaxzVzn92549747 = NsLbaxzVzn59226678;     NsLbaxzVzn59226678 = NsLbaxzVzn41933384;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZMqMaCtEiy31793284() {     int dPuUolETDj54719551 = -130740998;    int dPuUolETDj87887660 = -808278171;    int dPuUolETDj59515496 = -662415657;    int dPuUolETDj85992399 = -161412602;    int dPuUolETDj78245206 = -480204455;    int dPuUolETDj96535285 = 80362998;    int dPuUolETDj54087113 = 70519533;    int dPuUolETDj63745194 = -797656734;    int dPuUolETDj90123233 = 3048200;    int dPuUolETDj56523975 = -112197957;    int dPuUolETDj56193807 = -278253333;    int dPuUolETDj97909933 = -775964424;    int dPuUolETDj10433753 = -107505396;    int dPuUolETDj43189735 = -865808655;    int dPuUolETDj70799422 = -479417936;    int dPuUolETDj64962550 = -812031005;    int dPuUolETDj21710332 = -946599636;    int dPuUolETDj92093050 = -118015698;    int dPuUolETDj89107858 = -515409017;    int dPuUolETDj32452981 = -401873367;    int dPuUolETDj55925806 = -74497734;    int dPuUolETDj85327821 = -621122420;    int dPuUolETDj56838440 = 81562753;    int dPuUolETDj55487716 = -265704127;    int dPuUolETDj72346428 = -107997670;    int dPuUolETDj7396951 = -517963848;    int dPuUolETDj15206385 = -596080452;    int dPuUolETDj17944207 = -748413706;    int dPuUolETDj41598531 = -274271515;    int dPuUolETDj33338243 = -796283050;    int dPuUolETDj85763461 = -378090150;    int dPuUolETDj38664609 = -130846425;    int dPuUolETDj6033809 = -611845486;    int dPuUolETDj43988672 = -913921726;    int dPuUolETDj22913648 = -753344670;    int dPuUolETDj60378761 = -932973089;    int dPuUolETDj12584431 = -630935430;    int dPuUolETDj99318171 = -394415798;    int dPuUolETDj20034912 = 41833446;    int dPuUolETDj2230722 = -840319087;    int dPuUolETDj75038082 = -790280516;    int dPuUolETDj60767605 = -835024349;    int dPuUolETDj81549425 = -271288401;    int dPuUolETDj46156090 = -38468909;    int dPuUolETDj32595809 = -33005573;    int dPuUolETDj70899671 = -901410634;    int dPuUolETDj83874338 = -698371451;    int dPuUolETDj50965574 = -191797136;    int dPuUolETDj86655879 = -979528840;    int dPuUolETDj99632259 = -958048972;    int dPuUolETDj20606978 = -625553427;    int dPuUolETDj69378096 = -357153942;    int dPuUolETDj5271980 = -112182203;    int dPuUolETDj48601099 = -768775299;    int dPuUolETDj75867478 = -659769560;    int dPuUolETDj69391729 = -509618579;    int dPuUolETDj31049221 = -789840925;    int dPuUolETDj4027781 = -296711530;    int dPuUolETDj13645971 = 46585067;    int dPuUolETDj70848255 = -962240607;    int dPuUolETDj81328901 = -323556551;    int dPuUolETDj36142906 = -181066762;    int dPuUolETDj22146663 = -423385219;    int dPuUolETDj56784990 = -200668751;    int dPuUolETDj70760514 = -734107807;    int dPuUolETDj17529199 = -47406909;    int dPuUolETDj91876125 = -64118938;    int dPuUolETDj66445081 = -193583671;    int dPuUolETDj20276088 = -12463985;    int dPuUolETDj10420662 = -546444847;    int dPuUolETDj52378119 = -81095575;    int dPuUolETDj22392160 = -452183838;    int dPuUolETDj72058138 = -59849145;    int dPuUolETDj86877137 = -675089930;    int dPuUolETDj57414899 = -611592852;    int dPuUolETDj95158201 = -239473385;    int dPuUolETDj3778397 = -249834019;    int dPuUolETDj10682350 = -879968338;    int dPuUolETDj22891908 = -132698554;    int dPuUolETDj1446758 = -206587036;    int dPuUolETDj23522613 = -819592398;    int dPuUolETDj64240810 = -304283317;    int dPuUolETDj31288328 = -768884866;    int dPuUolETDj41966272 = -316222543;    int dPuUolETDj12731265 = -70729624;    int dPuUolETDj16385366 = 79063791;    int dPuUolETDj33392629 = 81335778;    int dPuUolETDj57432709 = -843070188;    int dPuUolETDj68121194 = -154152166;    int dPuUolETDj53521918 = -143726091;    int dPuUolETDj29329541 = -43132164;    int dPuUolETDj8556651 = -234223901;    int dPuUolETDj85672201 = -341000866;    int dPuUolETDj49186657 = 4074053;    int dPuUolETDj20901821 = -416762537;    int dPuUolETDj38895176 = -509213754;    int dPuUolETDj38620942 = -311639130;    int dPuUolETDj24764435 = 29380349;    int dPuUolETDj75395576 = -304361102;    int dPuUolETDj15066610 = -130740998;     dPuUolETDj54719551 = dPuUolETDj87887660;     dPuUolETDj87887660 = dPuUolETDj59515496;     dPuUolETDj59515496 = dPuUolETDj85992399;     dPuUolETDj85992399 = dPuUolETDj78245206;     dPuUolETDj78245206 = dPuUolETDj96535285;     dPuUolETDj96535285 = dPuUolETDj54087113;     dPuUolETDj54087113 = dPuUolETDj63745194;     dPuUolETDj63745194 = dPuUolETDj90123233;     dPuUolETDj90123233 = dPuUolETDj56523975;     dPuUolETDj56523975 = dPuUolETDj56193807;     dPuUolETDj56193807 = dPuUolETDj97909933;     dPuUolETDj97909933 = dPuUolETDj10433753;     dPuUolETDj10433753 = dPuUolETDj43189735;     dPuUolETDj43189735 = dPuUolETDj70799422;     dPuUolETDj70799422 = dPuUolETDj64962550;     dPuUolETDj64962550 = dPuUolETDj21710332;     dPuUolETDj21710332 = dPuUolETDj92093050;     dPuUolETDj92093050 = dPuUolETDj89107858;     dPuUolETDj89107858 = dPuUolETDj32452981;     dPuUolETDj32452981 = dPuUolETDj55925806;     dPuUolETDj55925806 = dPuUolETDj85327821;     dPuUolETDj85327821 = dPuUolETDj56838440;     dPuUolETDj56838440 = dPuUolETDj55487716;     dPuUolETDj55487716 = dPuUolETDj72346428;     dPuUolETDj72346428 = dPuUolETDj7396951;     dPuUolETDj7396951 = dPuUolETDj15206385;     dPuUolETDj15206385 = dPuUolETDj17944207;     dPuUolETDj17944207 = dPuUolETDj41598531;     dPuUolETDj41598531 = dPuUolETDj33338243;     dPuUolETDj33338243 = dPuUolETDj85763461;     dPuUolETDj85763461 = dPuUolETDj38664609;     dPuUolETDj38664609 = dPuUolETDj6033809;     dPuUolETDj6033809 = dPuUolETDj43988672;     dPuUolETDj43988672 = dPuUolETDj22913648;     dPuUolETDj22913648 = dPuUolETDj60378761;     dPuUolETDj60378761 = dPuUolETDj12584431;     dPuUolETDj12584431 = dPuUolETDj99318171;     dPuUolETDj99318171 = dPuUolETDj20034912;     dPuUolETDj20034912 = dPuUolETDj2230722;     dPuUolETDj2230722 = dPuUolETDj75038082;     dPuUolETDj75038082 = dPuUolETDj60767605;     dPuUolETDj60767605 = dPuUolETDj81549425;     dPuUolETDj81549425 = dPuUolETDj46156090;     dPuUolETDj46156090 = dPuUolETDj32595809;     dPuUolETDj32595809 = dPuUolETDj70899671;     dPuUolETDj70899671 = dPuUolETDj83874338;     dPuUolETDj83874338 = dPuUolETDj50965574;     dPuUolETDj50965574 = dPuUolETDj86655879;     dPuUolETDj86655879 = dPuUolETDj99632259;     dPuUolETDj99632259 = dPuUolETDj20606978;     dPuUolETDj20606978 = dPuUolETDj69378096;     dPuUolETDj69378096 = dPuUolETDj5271980;     dPuUolETDj5271980 = dPuUolETDj48601099;     dPuUolETDj48601099 = dPuUolETDj75867478;     dPuUolETDj75867478 = dPuUolETDj69391729;     dPuUolETDj69391729 = dPuUolETDj31049221;     dPuUolETDj31049221 = dPuUolETDj4027781;     dPuUolETDj4027781 = dPuUolETDj13645971;     dPuUolETDj13645971 = dPuUolETDj70848255;     dPuUolETDj70848255 = dPuUolETDj81328901;     dPuUolETDj81328901 = dPuUolETDj36142906;     dPuUolETDj36142906 = dPuUolETDj22146663;     dPuUolETDj22146663 = dPuUolETDj56784990;     dPuUolETDj56784990 = dPuUolETDj70760514;     dPuUolETDj70760514 = dPuUolETDj17529199;     dPuUolETDj17529199 = dPuUolETDj91876125;     dPuUolETDj91876125 = dPuUolETDj66445081;     dPuUolETDj66445081 = dPuUolETDj20276088;     dPuUolETDj20276088 = dPuUolETDj10420662;     dPuUolETDj10420662 = dPuUolETDj52378119;     dPuUolETDj52378119 = dPuUolETDj22392160;     dPuUolETDj22392160 = dPuUolETDj72058138;     dPuUolETDj72058138 = dPuUolETDj86877137;     dPuUolETDj86877137 = dPuUolETDj57414899;     dPuUolETDj57414899 = dPuUolETDj95158201;     dPuUolETDj95158201 = dPuUolETDj3778397;     dPuUolETDj3778397 = dPuUolETDj10682350;     dPuUolETDj10682350 = dPuUolETDj22891908;     dPuUolETDj22891908 = dPuUolETDj1446758;     dPuUolETDj1446758 = dPuUolETDj23522613;     dPuUolETDj23522613 = dPuUolETDj64240810;     dPuUolETDj64240810 = dPuUolETDj31288328;     dPuUolETDj31288328 = dPuUolETDj41966272;     dPuUolETDj41966272 = dPuUolETDj12731265;     dPuUolETDj12731265 = dPuUolETDj16385366;     dPuUolETDj16385366 = dPuUolETDj33392629;     dPuUolETDj33392629 = dPuUolETDj57432709;     dPuUolETDj57432709 = dPuUolETDj68121194;     dPuUolETDj68121194 = dPuUolETDj53521918;     dPuUolETDj53521918 = dPuUolETDj29329541;     dPuUolETDj29329541 = dPuUolETDj8556651;     dPuUolETDj8556651 = dPuUolETDj85672201;     dPuUolETDj85672201 = dPuUolETDj49186657;     dPuUolETDj49186657 = dPuUolETDj20901821;     dPuUolETDj20901821 = dPuUolETDj38895176;     dPuUolETDj38895176 = dPuUolETDj38620942;     dPuUolETDj38620942 = dPuUolETDj24764435;     dPuUolETDj24764435 = dPuUolETDj75395576;     dPuUolETDj75395576 = dPuUolETDj15066610;     dPuUolETDj15066610 = dPuUolETDj54719551;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FMighPyISm50006625() {     int kmUYrhXicy38163856 = 88593321;    int kmUYrhXicy4759750 = -807660359;    int kmUYrhXicy55924003 = -455940885;    int kmUYrhXicy48784651 = -355212438;    int kmUYrhXicy84366529 = -287071946;    int kmUYrhXicy95039686 = -798708655;    int kmUYrhXicy25126431 = -769391780;    int kmUYrhXicy78092637 = -784814763;    int kmUYrhXicy48652934 = -338411151;    int kmUYrhXicy89069627 = -580574069;    int kmUYrhXicy5609746 = -468369205;    int kmUYrhXicy61115333 = -763582670;    int kmUYrhXicy28551529 = -999166637;    int kmUYrhXicy22816628 = -654099011;    int kmUYrhXicy31653886 = -573440918;    int kmUYrhXicy51351711 = -600110330;    int kmUYrhXicy15462217 = -699944663;    int kmUYrhXicy72944496 = -365320575;    int kmUYrhXicy51548001 = -514083888;    int kmUYrhXicy21820424 = -931459360;    int kmUYrhXicy11566194 = -764026764;    int kmUYrhXicy97475139 = -612274190;    int kmUYrhXicy60664147 = 41673556;    int kmUYrhXicy90194274 = -337529523;    int kmUYrhXicy13922387 = -667650429;    int kmUYrhXicy2739100 = -398473564;    int kmUYrhXicy64661124 = -224164794;    int kmUYrhXicy57675832 = -818368589;    int kmUYrhXicy77679625 = -136776657;    int kmUYrhXicy49171584 = -209132901;    int kmUYrhXicy3858775 = -15847351;    int kmUYrhXicy1771036 = -46041504;    int kmUYrhXicy49572695 = -232717663;    int kmUYrhXicy86147182 = -505523896;    int kmUYrhXicy1640378 = -951009405;    int kmUYrhXicy72767945 = -720996995;    int kmUYrhXicy81198609 = -509784610;    int kmUYrhXicy39897004 = -102886318;    int kmUYrhXicy69029990 = -143064352;    int kmUYrhXicy51725856 = -592912610;    int kmUYrhXicy8687007 = -361829489;    int kmUYrhXicy11255392 = -308932670;    int kmUYrhXicy5116495 = -822345644;    int kmUYrhXicy38061030 = -894651680;    int kmUYrhXicy24540305 = -861162527;    int kmUYrhXicy10505667 = -921021558;    int kmUYrhXicy59889768 = -830675680;    int kmUYrhXicy70636169 = -190518854;    int kmUYrhXicy17502992 = -465130923;    int kmUYrhXicy34964743 = -65069427;    int kmUYrhXicy93864426 = -846454849;    int kmUYrhXicy24878890 = -415413253;    int kmUYrhXicy19384439 = -323929520;    int kmUYrhXicy73277858 = -487188686;    int kmUYrhXicy38440432 = 91744317;    int kmUYrhXicy40688717 = -299132489;    int kmUYrhXicy44095602 = -749333915;    int kmUYrhXicy65729729 = -18411362;    int kmUYrhXicy34862264 = -687562009;    int kmUYrhXicy81627430 = -888598383;    int kmUYrhXicy30378562 = -474543861;    int kmUYrhXicy67450598 = -951023191;    int kmUYrhXicy413012 = -548038107;    int kmUYrhXicy99481349 = -29278250;    int kmUYrhXicy85210852 = -464726719;    int kmUYrhXicy3838710 = -322327701;    int kmUYrhXicy11542638 = -430865007;    int kmUYrhXicy42404346 = -393642742;    int kmUYrhXicy21176250 = -703089607;    int kmUYrhXicy58885940 = -852443924;    int kmUYrhXicy70153102 = 9674280;    int kmUYrhXicy75565213 = -497058345;    int kmUYrhXicy3914507 = -122256224;    int kmUYrhXicy99822144 = -921171279;    int kmUYrhXicy13133417 = -469629871;    int kmUYrhXicy310803 = -355094095;    int kmUYrhXicy92358644 = -789928547;    int kmUYrhXicy22603118 = -63674765;    int kmUYrhXicy65653969 = -476366997;    int kmUYrhXicy3416720 = -746628872;    int kmUYrhXicy42849332 = -567797884;    int kmUYrhXicy94024955 = 66354059;    int kmUYrhXicy40172841 = -253237667;    int kmUYrhXicy42714882 = 28292770;    int kmUYrhXicy55307158 = -362678053;    int kmUYrhXicy78979884 = -600434098;    int kmUYrhXicy82386597 = -722111985;    int kmUYrhXicy76294837 = -745528978;    int kmUYrhXicy47706750 = -497268213;    int kmUYrhXicy60951661 = -551876916;    int kmUYrhXicy28672343 = -971663080;    int kmUYrhXicy15468881 = -391373249;    int kmUYrhXicy5034740 = -415324309;    int kmUYrhXicy87402560 = -254465969;    int kmUYrhXicy21347295 = -18368749;    int kmUYrhXicy41236408 = -410806298;    int kmUYrhXicy10842380 = -760894564;    int kmUYrhXicy5635145 = -693067394;    int kmUYrhXicy52850178 = -329924962;    int kmUYrhXicy20701596 = 88593321;     kmUYrhXicy38163856 = kmUYrhXicy4759750;     kmUYrhXicy4759750 = kmUYrhXicy55924003;     kmUYrhXicy55924003 = kmUYrhXicy48784651;     kmUYrhXicy48784651 = kmUYrhXicy84366529;     kmUYrhXicy84366529 = kmUYrhXicy95039686;     kmUYrhXicy95039686 = kmUYrhXicy25126431;     kmUYrhXicy25126431 = kmUYrhXicy78092637;     kmUYrhXicy78092637 = kmUYrhXicy48652934;     kmUYrhXicy48652934 = kmUYrhXicy89069627;     kmUYrhXicy89069627 = kmUYrhXicy5609746;     kmUYrhXicy5609746 = kmUYrhXicy61115333;     kmUYrhXicy61115333 = kmUYrhXicy28551529;     kmUYrhXicy28551529 = kmUYrhXicy22816628;     kmUYrhXicy22816628 = kmUYrhXicy31653886;     kmUYrhXicy31653886 = kmUYrhXicy51351711;     kmUYrhXicy51351711 = kmUYrhXicy15462217;     kmUYrhXicy15462217 = kmUYrhXicy72944496;     kmUYrhXicy72944496 = kmUYrhXicy51548001;     kmUYrhXicy51548001 = kmUYrhXicy21820424;     kmUYrhXicy21820424 = kmUYrhXicy11566194;     kmUYrhXicy11566194 = kmUYrhXicy97475139;     kmUYrhXicy97475139 = kmUYrhXicy60664147;     kmUYrhXicy60664147 = kmUYrhXicy90194274;     kmUYrhXicy90194274 = kmUYrhXicy13922387;     kmUYrhXicy13922387 = kmUYrhXicy2739100;     kmUYrhXicy2739100 = kmUYrhXicy64661124;     kmUYrhXicy64661124 = kmUYrhXicy57675832;     kmUYrhXicy57675832 = kmUYrhXicy77679625;     kmUYrhXicy77679625 = kmUYrhXicy49171584;     kmUYrhXicy49171584 = kmUYrhXicy3858775;     kmUYrhXicy3858775 = kmUYrhXicy1771036;     kmUYrhXicy1771036 = kmUYrhXicy49572695;     kmUYrhXicy49572695 = kmUYrhXicy86147182;     kmUYrhXicy86147182 = kmUYrhXicy1640378;     kmUYrhXicy1640378 = kmUYrhXicy72767945;     kmUYrhXicy72767945 = kmUYrhXicy81198609;     kmUYrhXicy81198609 = kmUYrhXicy39897004;     kmUYrhXicy39897004 = kmUYrhXicy69029990;     kmUYrhXicy69029990 = kmUYrhXicy51725856;     kmUYrhXicy51725856 = kmUYrhXicy8687007;     kmUYrhXicy8687007 = kmUYrhXicy11255392;     kmUYrhXicy11255392 = kmUYrhXicy5116495;     kmUYrhXicy5116495 = kmUYrhXicy38061030;     kmUYrhXicy38061030 = kmUYrhXicy24540305;     kmUYrhXicy24540305 = kmUYrhXicy10505667;     kmUYrhXicy10505667 = kmUYrhXicy59889768;     kmUYrhXicy59889768 = kmUYrhXicy70636169;     kmUYrhXicy70636169 = kmUYrhXicy17502992;     kmUYrhXicy17502992 = kmUYrhXicy34964743;     kmUYrhXicy34964743 = kmUYrhXicy93864426;     kmUYrhXicy93864426 = kmUYrhXicy24878890;     kmUYrhXicy24878890 = kmUYrhXicy19384439;     kmUYrhXicy19384439 = kmUYrhXicy73277858;     kmUYrhXicy73277858 = kmUYrhXicy38440432;     kmUYrhXicy38440432 = kmUYrhXicy40688717;     kmUYrhXicy40688717 = kmUYrhXicy44095602;     kmUYrhXicy44095602 = kmUYrhXicy65729729;     kmUYrhXicy65729729 = kmUYrhXicy34862264;     kmUYrhXicy34862264 = kmUYrhXicy81627430;     kmUYrhXicy81627430 = kmUYrhXicy30378562;     kmUYrhXicy30378562 = kmUYrhXicy67450598;     kmUYrhXicy67450598 = kmUYrhXicy413012;     kmUYrhXicy413012 = kmUYrhXicy99481349;     kmUYrhXicy99481349 = kmUYrhXicy85210852;     kmUYrhXicy85210852 = kmUYrhXicy3838710;     kmUYrhXicy3838710 = kmUYrhXicy11542638;     kmUYrhXicy11542638 = kmUYrhXicy42404346;     kmUYrhXicy42404346 = kmUYrhXicy21176250;     kmUYrhXicy21176250 = kmUYrhXicy58885940;     kmUYrhXicy58885940 = kmUYrhXicy70153102;     kmUYrhXicy70153102 = kmUYrhXicy75565213;     kmUYrhXicy75565213 = kmUYrhXicy3914507;     kmUYrhXicy3914507 = kmUYrhXicy99822144;     kmUYrhXicy99822144 = kmUYrhXicy13133417;     kmUYrhXicy13133417 = kmUYrhXicy310803;     kmUYrhXicy310803 = kmUYrhXicy92358644;     kmUYrhXicy92358644 = kmUYrhXicy22603118;     kmUYrhXicy22603118 = kmUYrhXicy65653969;     kmUYrhXicy65653969 = kmUYrhXicy3416720;     kmUYrhXicy3416720 = kmUYrhXicy42849332;     kmUYrhXicy42849332 = kmUYrhXicy94024955;     kmUYrhXicy94024955 = kmUYrhXicy40172841;     kmUYrhXicy40172841 = kmUYrhXicy42714882;     kmUYrhXicy42714882 = kmUYrhXicy55307158;     kmUYrhXicy55307158 = kmUYrhXicy78979884;     kmUYrhXicy78979884 = kmUYrhXicy82386597;     kmUYrhXicy82386597 = kmUYrhXicy76294837;     kmUYrhXicy76294837 = kmUYrhXicy47706750;     kmUYrhXicy47706750 = kmUYrhXicy60951661;     kmUYrhXicy60951661 = kmUYrhXicy28672343;     kmUYrhXicy28672343 = kmUYrhXicy15468881;     kmUYrhXicy15468881 = kmUYrhXicy5034740;     kmUYrhXicy5034740 = kmUYrhXicy87402560;     kmUYrhXicy87402560 = kmUYrhXicy21347295;     kmUYrhXicy21347295 = kmUYrhXicy41236408;     kmUYrhXicy41236408 = kmUYrhXicy10842380;     kmUYrhXicy10842380 = kmUYrhXicy5635145;     kmUYrhXicy5635145 = kmUYrhXicy52850178;     kmUYrhXicy52850178 = kmUYrhXicy20701596;     kmUYrhXicy20701596 = kmUYrhXicy38163856;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZtPRPrjggq20462499() {     int iVFBmmGlpz50950023 = -46447371;    int iVFBmmGlpz91530637 = -977994373;    int iVFBmmGlpz73676803 = -864300266;    int iVFBmmGlpz56725476 = -179414715;    int iVFBmmGlpz77877634 = -778888411;    int iVFBmmGlpz38349757 = -575631969;    int iVFBmmGlpz46334189 = -215063851;    int iVFBmmGlpz4811750 = -129333020;    int iVFBmmGlpz44057744 = -300169243;    int iVFBmmGlpz95288331 = -384661702;    int iVFBmmGlpz33097127 = -583055221;    int iVFBmmGlpz93065837 = -170915486;    int iVFBmmGlpz98694831 = -902949294;    int iVFBmmGlpz72440706 = -986514210;    int iVFBmmGlpz54960439 = -480426749;    int iVFBmmGlpz79776780 = -51001719;    int iVFBmmGlpz71214034 = -363719130;    int iVFBmmGlpz56502982 = 79469123;    int iVFBmmGlpz52708631 = -423553083;    int iVFBmmGlpz24474920 = -430703499;    int iVFBmmGlpz91740250 = -155343029;    int iVFBmmGlpz26522959 = -782132538;    int iVFBmmGlpz99424442 = -989330339;    int iVFBmmGlpz34701404 = -697874536;    int iVFBmmGlpz19816646 = -878092353;    int iVFBmmGlpz84057745 = -88904763;    int iVFBmmGlpz76076660 = -857871075;    int iVFBmmGlpz25028757 = -992166694;    int iVFBmmGlpz94296678 = -350993897;    int iVFBmmGlpz1922700 = -939019056;    int iVFBmmGlpz48294771 = -292264101;    int iVFBmmGlpz5089860 = -24761183;    int iVFBmmGlpz95618644 = -108572448;    int iVFBmmGlpz63223555 = -410278114;    int iVFBmmGlpz84700909 = -461041286;    int iVFBmmGlpz67992060 = -509560099;    int iVFBmmGlpz17962670 = -299693961;    int iVFBmmGlpz97601026 = -22506410;    int iVFBmmGlpz74805376 = 15085071;    int iVFBmmGlpz77990586 = -437329455;    int iVFBmmGlpz70118315 = -737303482;    int iVFBmmGlpz18957847 = -207244004;    int iVFBmmGlpz39999965 = -845779089;    int iVFBmmGlpz82689567 = -135823923;    int iVFBmmGlpz4548796 = -753932487;    int iVFBmmGlpz84133796 = -994339997;    int iVFBmmGlpz37471788 = -228190733;    int iVFBmmGlpz76136 = -345871634;    int iVFBmmGlpz61567041 = -628732897;    int iVFBmmGlpz81915163 = -676105890;    int iVFBmmGlpz12191920 = 660843;    int iVFBmmGlpz53055437 = -886598591;    int iVFBmmGlpz53869589 = -499932802;    int iVFBmmGlpz17977584 = -76640428;    int iVFBmmGlpz28722249 = 97299831;    int iVFBmmGlpz24427065 = -264314833;    int iVFBmmGlpz92106194 = -988664034;    int iVFBmmGlpz38975400 = -66425731;    int iVFBmmGlpz36908831 = -301322362;    int iVFBmmGlpz93819889 = -589983649;    int iVFBmmGlpz62273096 = -717760895;    int iVFBmmGlpz21305432 = -222897157;    int iVFBmmGlpz10515072 = -778339123;    int iVFBmmGlpz42135045 = -361150188;    int iVFBmmGlpz46993561 = 7602398;    int iVFBmmGlpz28007268 = -458294038;    int iVFBmmGlpz97447193 = 37656962;    int iVFBmmGlpz35471276 = -392671181;    int iVFBmmGlpz87739796 = -425472925;    int iVFBmmGlpz86968378 = -970866650;    int iVFBmmGlpz61814110 = -751307759;    int iVFBmmGlpz73613007 = -241212721;    int iVFBmmGlpz81697605 = -935615949;    int iVFBmmGlpz74718045 = -986223628;    int iVFBmmGlpz54356605 = -693400017;    int iVFBmmGlpz72782403 = -948099025;    int iVFBmmGlpz86522994 = -936353450;    int iVFBmmGlpz16734876 = -753506417;    int iVFBmmGlpz30152608 = -943942049;    int iVFBmmGlpz35682850 = -883752356;    int iVFBmmGlpz46585957 = -860714030;    int iVFBmmGlpz76000525 = -411999441;    int iVFBmmGlpz63461716 = -263433797;    int iVFBmmGlpz12381515 = -674888008;    int iVFBmmGlpz89730779 = -839679899;    int iVFBmmGlpz95239333 = -405665510;    int iVFBmmGlpz51220271 = -524828381;    int iVFBmmGlpz77641061 = 68067980;    int iVFBmmGlpz34501307 = -407577945;    int iVFBmmGlpz60273845 = -96726453;    int iVFBmmGlpz75885865 = -520896065;    int iVFBmmGlpz78987270 = -133268230;    int iVFBmmGlpz60692196 = -721184048;    int iVFBmmGlpz80985487 = -394931281;    int iVFBmmGlpz15717491 = -719568561;    int iVFBmmGlpz48812883 = -414406326;    int iVFBmmGlpz8442776 = -428904882;    int iVFBmmGlpz97864919 = -384628902;    int iVFBmmGlpz35696007 = -43426322;    int iVFBmmGlpz76541527 = -46447371;     iVFBmmGlpz50950023 = iVFBmmGlpz91530637;     iVFBmmGlpz91530637 = iVFBmmGlpz73676803;     iVFBmmGlpz73676803 = iVFBmmGlpz56725476;     iVFBmmGlpz56725476 = iVFBmmGlpz77877634;     iVFBmmGlpz77877634 = iVFBmmGlpz38349757;     iVFBmmGlpz38349757 = iVFBmmGlpz46334189;     iVFBmmGlpz46334189 = iVFBmmGlpz4811750;     iVFBmmGlpz4811750 = iVFBmmGlpz44057744;     iVFBmmGlpz44057744 = iVFBmmGlpz95288331;     iVFBmmGlpz95288331 = iVFBmmGlpz33097127;     iVFBmmGlpz33097127 = iVFBmmGlpz93065837;     iVFBmmGlpz93065837 = iVFBmmGlpz98694831;     iVFBmmGlpz98694831 = iVFBmmGlpz72440706;     iVFBmmGlpz72440706 = iVFBmmGlpz54960439;     iVFBmmGlpz54960439 = iVFBmmGlpz79776780;     iVFBmmGlpz79776780 = iVFBmmGlpz71214034;     iVFBmmGlpz71214034 = iVFBmmGlpz56502982;     iVFBmmGlpz56502982 = iVFBmmGlpz52708631;     iVFBmmGlpz52708631 = iVFBmmGlpz24474920;     iVFBmmGlpz24474920 = iVFBmmGlpz91740250;     iVFBmmGlpz91740250 = iVFBmmGlpz26522959;     iVFBmmGlpz26522959 = iVFBmmGlpz99424442;     iVFBmmGlpz99424442 = iVFBmmGlpz34701404;     iVFBmmGlpz34701404 = iVFBmmGlpz19816646;     iVFBmmGlpz19816646 = iVFBmmGlpz84057745;     iVFBmmGlpz84057745 = iVFBmmGlpz76076660;     iVFBmmGlpz76076660 = iVFBmmGlpz25028757;     iVFBmmGlpz25028757 = iVFBmmGlpz94296678;     iVFBmmGlpz94296678 = iVFBmmGlpz1922700;     iVFBmmGlpz1922700 = iVFBmmGlpz48294771;     iVFBmmGlpz48294771 = iVFBmmGlpz5089860;     iVFBmmGlpz5089860 = iVFBmmGlpz95618644;     iVFBmmGlpz95618644 = iVFBmmGlpz63223555;     iVFBmmGlpz63223555 = iVFBmmGlpz84700909;     iVFBmmGlpz84700909 = iVFBmmGlpz67992060;     iVFBmmGlpz67992060 = iVFBmmGlpz17962670;     iVFBmmGlpz17962670 = iVFBmmGlpz97601026;     iVFBmmGlpz97601026 = iVFBmmGlpz74805376;     iVFBmmGlpz74805376 = iVFBmmGlpz77990586;     iVFBmmGlpz77990586 = iVFBmmGlpz70118315;     iVFBmmGlpz70118315 = iVFBmmGlpz18957847;     iVFBmmGlpz18957847 = iVFBmmGlpz39999965;     iVFBmmGlpz39999965 = iVFBmmGlpz82689567;     iVFBmmGlpz82689567 = iVFBmmGlpz4548796;     iVFBmmGlpz4548796 = iVFBmmGlpz84133796;     iVFBmmGlpz84133796 = iVFBmmGlpz37471788;     iVFBmmGlpz37471788 = iVFBmmGlpz76136;     iVFBmmGlpz76136 = iVFBmmGlpz61567041;     iVFBmmGlpz61567041 = iVFBmmGlpz81915163;     iVFBmmGlpz81915163 = iVFBmmGlpz12191920;     iVFBmmGlpz12191920 = iVFBmmGlpz53055437;     iVFBmmGlpz53055437 = iVFBmmGlpz53869589;     iVFBmmGlpz53869589 = iVFBmmGlpz17977584;     iVFBmmGlpz17977584 = iVFBmmGlpz28722249;     iVFBmmGlpz28722249 = iVFBmmGlpz24427065;     iVFBmmGlpz24427065 = iVFBmmGlpz92106194;     iVFBmmGlpz92106194 = iVFBmmGlpz38975400;     iVFBmmGlpz38975400 = iVFBmmGlpz36908831;     iVFBmmGlpz36908831 = iVFBmmGlpz93819889;     iVFBmmGlpz93819889 = iVFBmmGlpz62273096;     iVFBmmGlpz62273096 = iVFBmmGlpz21305432;     iVFBmmGlpz21305432 = iVFBmmGlpz10515072;     iVFBmmGlpz10515072 = iVFBmmGlpz42135045;     iVFBmmGlpz42135045 = iVFBmmGlpz46993561;     iVFBmmGlpz46993561 = iVFBmmGlpz28007268;     iVFBmmGlpz28007268 = iVFBmmGlpz97447193;     iVFBmmGlpz97447193 = iVFBmmGlpz35471276;     iVFBmmGlpz35471276 = iVFBmmGlpz87739796;     iVFBmmGlpz87739796 = iVFBmmGlpz86968378;     iVFBmmGlpz86968378 = iVFBmmGlpz61814110;     iVFBmmGlpz61814110 = iVFBmmGlpz73613007;     iVFBmmGlpz73613007 = iVFBmmGlpz81697605;     iVFBmmGlpz81697605 = iVFBmmGlpz74718045;     iVFBmmGlpz74718045 = iVFBmmGlpz54356605;     iVFBmmGlpz54356605 = iVFBmmGlpz72782403;     iVFBmmGlpz72782403 = iVFBmmGlpz86522994;     iVFBmmGlpz86522994 = iVFBmmGlpz16734876;     iVFBmmGlpz16734876 = iVFBmmGlpz30152608;     iVFBmmGlpz30152608 = iVFBmmGlpz35682850;     iVFBmmGlpz35682850 = iVFBmmGlpz46585957;     iVFBmmGlpz46585957 = iVFBmmGlpz76000525;     iVFBmmGlpz76000525 = iVFBmmGlpz63461716;     iVFBmmGlpz63461716 = iVFBmmGlpz12381515;     iVFBmmGlpz12381515 = iVFBmmGlpz89730779;     iVFBmmGlpz89730779 = iVFBmmGlpz95239333;     iVFBmmGlpz95239333 = iVFBmmGlpz51220271;     iVFBmmGlpz51220271 = iVFBmmGlpz77641061;     iVFBmmGlpz77641061 = iVFBmmGlpz34501307;     iVFBmmGlpz34501307 = iVFBmmGlpz60273845;     iVFBmmGlpz60273845 = iVFBmmGlpz75885865;     iVFBmmGlpz75885865 = iVFBmmGlpz78987270;     iVFBmmGlpz78987270 = iVFBmmGlpz60692196;     iVFBmmGlpz60692196 = iVFBmmGlpz80985487;     iVFBmmGlpz80985487 = iVFBmmGlpz15717491;     iVFBmmGlpz15717491 = iVFBmmGlpz48812883;     iVFBmmGlpz48812883 = iVFBmmGlpz8442776;     iVFBmmGlpz8442776 = iVFBmmGlpz97864919;     iVFBmmGlpz97864919 = iVFBmmGlpz35696007;     iVFBmmGlpz35696007 = iVFBmmGlpz76541527;     iVFBmmGlpz76541527 = iVFBmmGlpz50950023;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ilabqodpum90588174() {     int DRrFKnTwho18397060 = -746491505;    int DRrFKnTwho86726828 = -585298075;    int DRrFKnTwho50849037 = -600915648;    int DRrFKnTwho51021383 = -189178572;    int DRrFKnTwho37000307 = -474784794;    int DRrFKnTwho61028792 = -782273306;    int DRrFKnTwho20095315 = -258092127;    int DRrFKnTwho91491915 = -270242192;    int DRrFKnTwho44496801 = -595134636;    int DRrFKnTwho87499508 = -830743733;    int DRrFKnTwho18875200 = -263625736;    int DRrFKnTwho73489379 = -29194028;    int DRrFKnTwho802536 = -588613781;    int DRrFKnTwho61186995 = -585879935;    int DRrFKnTwho63318956 = -331821359;    int DRrFKnTwho23404838 = -999257022;    int DRrFKnTwho70944856 = -681478855;    int DRrFKnTwho84657520 = -242234025;    int DRrFKnTwho34661593 = -851560;    int DRrFKnTwho72690209 = -763289333;    int DRrFKnTwho26419271 = -404276070;    int DRrFKnTwho28527101 = -608443111;    int DRrFKnTwho37776174 = 51880188;    int DRrFKnTwho36986794 = -876339842;    int DRrFKnTwho93020831 = 65246124;    int DRrFKnTwho90043260 = -974838818;    int DRrFKnTwho19260540 = -645621921;    int DRrFKnTwho10227158 = -565049671;    int DRrFKnTwho38132962 = -299385698;    int DRrFKnTwho73019354 = -84231805;    int DRrFKnTwho11023617 = -730460142;    int DRrFKnTwho54676436 = -23155628;    int DRrFKnTwho23868047 = -339000631;    int DRrFKnTwho34672984 = -118471409;    int DRrFKnTwho19907561 = 70377499;    int DRrFKnTwho7714528 = -466353054;    int DRrFKnTwho17489851 = -940376554;    int DRrFKnTwho98364608 = -920792843;    int DRrFKnTwho96036814 = -596032691;    int DRrFKnTwho56368818 = -330623214;    int DRrFKnTwho99653356 = -37383735;    int DRrFKnTwho77637300 = -519295343;    int DRrFKnTwho5600258 = -150587597;    int DRrFKnTwho77080606 = -393711388;    int DRrFKnTwho60523297 = -305960643;    int DRrFKnTwho72667559 = -224403381;    int DRrFKnTwho3829727 = -271482547;    int DRrFKnTwho77559830 = -970115429;    int DRrFKnTwho29315468 = -28301199;    int DRrFKnTwho53661824 = -840136761;    int DRrFKnTwho92373582 = -330883960;    int DRrFKnTwho56066877 = -390704164;    int DRrFKnTwho44634394 = -523797534;    int DRrFKnTwho9842795 = -130058803;    int DRrFKnTwho64168903 = -536154059;    int DRrFKnTwho89869958 = -38048395;    int DRrFKnTwho48950655 = -537178263;    int DRrFKnTwho13862244 = -724575806;    int DRrFKnTwho58000551 = -154424697;    int DRrFKnTwho46957047 = -499945976;    int DRrFKnTwho41768253 = -36651386;    int DRrFKnTwho9868158 = -693042456;    int DRrFKnTwho53358954 = -970856495;    int DRrFKnTwho71477447 = -410902832;    int DRrFKnTwho76475891 = -283592;    int DRrFKnTwho64198763 = -140470109;    int DRrFKnTwho49621332 = -690193398;    int DRrFKnTwho66129552 = -370142373;    int DRrFKnTwho41279435 = -556257434;    int DRrFKnTwho55604429 = -865468306;    int DRrFKnTwho5914987 = 41119531;    int DRrFKnTwho72580247 = -760686013;    int DRrFKnTwho88620706 = -646201334;    int DRrFKnTwho78292774 = -670228346;    int DRrFKnTwho73036853 = -625905598;    int DRrFKnTwho48781970 = -884980727;    int DRrFKnTwho22926844 = -357855514;    int DRrFKnTwho60695567 = -554408425;    int DRrFKnTwho76463496 = -470379199;    int DRrFKnTwho20353273 = -710350496;    int DRrFKnTwho86213533 = -603356272;    int DRrFKnTwho41700709 = -675506492;    int DRrFKnTwho80911689 = -436748472;    int DRrFKnTwho84471137 = -459248937;    int DRrFKnTwho80645771 = -753347845;    int DRrFKnTwho54956740 = -239755979;    int DRrFKnTwho10042043 = -499358095;    int DRrFKnTwho14025252 = -108941828;    int DRrFKnTwho70504080 = -582317350;    int DRrFKnTwho30037602 = -891574106;    int DRrFKnTwho58763872 = -929174791;    int DRrFKnTwho3627607 = -115800748;    int DRrFKnTwho40364058 = -666368147;    int DRrFKnTwho49079768 = 3913284;    int DRrFKnTwho14600566 = -193971828;    int DRrFKnTwho89785198 = -344341280;    int DRrFKnTwho24278347 = -548438849;    int DRrFKnTwho34122810 = -739684766;    int DRrFKnTwho604716 = -293427797;    int DRrFKnTwho96324533 = -746491505;     DRrFKnTwho18397060 = DRrFKnTwho86726828;     DRrFKnTwho86726828 = DRrFKnTwho50849037;     DRrFKnTwho50849037 = DRrFKnTwho51021383;     DRrFKnTwho51021383 = DRrFKnTwho37000307;     DRrFKnTwho37000307 = DRrFKnTwho61028792;     DRrFKnTwho61028792 = DRrFKnTwho20095315;     DRrFKnTwho20095315 = DRrFKnTwho91491915;     DRrFKnTwho91491915 = DRrFKnTwho44496801;     DRrFKnTwho44496801 = DRrFKnTwho87499508;     DRrFKnTwho87499508 = DRrFKnTwho18875200;     DRrFKnTwho18875200 = DRrFKnTwho73489379;     DRrFKnTwho73489379 = DRrFKnTwho802536;     DRrFKnTwho802536 = DRrFKnTwho61186995;     DRrFKnTwho61186995 = DRrFKnTwho63318956;     DRrFKnTwho63318956 = DRrFKnTwho23404838;     DRrFKnTwho23404838 = DRrFKnTwho70944856;     DRrFKnTwho70944856 = DRrFKnTwho84657520;     DRrFKnTwho84657520 = DRrFKnTwho34661593;     DRrFKnTwho34661593 = DRrFKnTwho72690209;     DRrFKnTwho72690209 = DRrFKnTwho26419271;     DRrFKnTwho26419271 = DRrFKnTwho28527101;     DRrFKnTwho28527101 = DRrFKnTwho37776174;     DRrFKnTwho37776174 = DRrFKnTwho36986794;     DRrFKnTwho36986794 = DRrFKnTwho93020831;     DRrFKnTwho93020831 = DRrFKnTwho90043260;     DRrFKnTwho90043260 = DRrFKnTwho19260540;     DRrFKnTwho19260540 = DRrFKnTwho10227158;     DRrFKnTwho10227158 = DRrFKnTwho38132962;     DRrFKnTwho38132962 = DRrFKnTwho73019354;     DRrFKnTwho73019354 = DRrFKnTwho11023617;     DRrFKnTwho11023617 = DRrFKnTwho54676436;     DRrFKnTwho54676436 = DRrFKnTwho23868047;     DRrFKnTwho23868047 = DRrFKnTwho34672984;     DRrFKnTwho34672984 = DRrFKnTwho19907561;     DRrFKnTwho19907561 = DRrFKnTwho7714528;     DRrFKnTwho7714528 = DRrFKnTwho17489851;     DRrFKnTwho17489851 = DRrFKnTwho98364608;     DRrFKnTwho98364608 = DRrFKnTwho96036814;     DRrFKnTwho96036814 = DRrFKnTwho56368818;     DRrFKnTwho56368818 = DRrFKnTwho99653356;     DRrFKnTwho99653356 = DRrFKnTwho77637300;     DRrFKnTwho77637300 = DRrFKnTwho5600258;     DRrFKnTwho5600258 = DRrFKnTwho77080606;     DRrFKnTwho77080606 = DRrFKnTwho60523297;     DRrFKnTwho60523297 = DRrFKnTwho72667559;     DRrFKnTwho72667559 = DRrFKnTwho3829727;     DRrFKnTwho3829727 = DRrFKnTwho77559830;     DRrFKnTwho77559830 = DRrFKnTwho29315468;     DRrFKnTwho29315468 = DRrFKnTwho53661824;     DRrFKnTwho53661824 = DRrFKnTwho92373582;     DRrFKnTwho92373582 = DRrFKnTwho56066877;     DRrFKnTwho56066877 = DRrFKnTwho44634394;     DRrFKnTwho44634394 = DRrFKnTwho9842795;     DRrFKnTwho9842795 = DRrFKnTwho64168903;     DRrFKnTwho64168903 = DRrFKnTwho89869958;     DRrFKnTwho89869958 = DRrFKnTwho48950655;     DRrFKnTwho48950655 = DRrFKnTwho13862244;     DRrFKnTwho13862244 = DRrFKnTwho58000551;     DRrFKnTwho58000551 = DRrFKnTwho46957047;     DRrFKnTwho46957047 = DRrFKnTwho41768253;     DRrFKnTwho41768253 = DRrFKnTwho9868158;     DRrFKnTwho9868158 = DRrFKnTwho53358954;     DRrFKnTwho53358954 = DRrFKnTwho71477447;     DRrFKnTwho71477447 = DRrFKnTwho76475891;     DRrFKnTwho76475891 = DRrFKnTwho64198763;     DRrFKnTwho64198763 = DRrFKnTwho49621332;     DRrFKnTwho49621332 = DRrFKnTwho66129552;     DRrFKnTwho66129552 = DRrFKnTwho41279435;     DRrFKnTwho41279435 = DRrFKnTwho55604429;     DRrFKnTwho55604429 = DRrFKnTwho5914987;     DRrFKnTwho5914987 = DRrFKnTwho72580247;     DRrFKnTwho72580247 = DRrFKnTwho88620706;     DRrFKnTwho88620706 = DRrFKnTwho78292774;     DRrFKnTwho78292774 = DRrFKnTwho73036853;     DRrFKnTwho73036853 = DRrFKnTwho48781970;     DRrFKnTwho48781970 = DRrFKnTwho22926844;     DRrFKnTwho22926844 = DRrFKnTwho60695567;     DRrFKnTwho60695567 = DRrFKnTwho76463496;     DRrFKnTwho76463496 = DRrFKnTwho20353273;     DRrFKnTwho20353273 = DRrFKnTwho86213533;     DRrFKnTwho86213533 = DRrFKnTwho41700709;     DRrFKnTwho41700709 = DRrFKnTwho80911689;     DRrFKnTwho80911689 = DRrFKnTwho84471137;     DRrFKnTwho84471137 = DRrFKnTwho80645771;     DRrFKnTwho80645771 = DRrFKnTwho54956740;     DRrFKnTwho54956740 = DRrFKnTwho10042043;     DRrFKnTwho10042043 = DRrFKnTwho14025252;     DRrFKnTwho14025252 = DRrFKnTwho70504080;     DRrFKnTwho70504080 = DRrFKnTwho30037602;     DRrFKnTwho30037602 = DRrFKnTwho58763872;     DRrFKnTwho58763872 = DRrFKnTwho3627607;     DRrFKnTwho3627607 = DRrFKnTwho40364058;     DRrFKnTwho40364058 = DRrFKnTwho49079768;     DRrFKnTwho49079768 = DRrFKnTwho14600566;     DRrFKnTwho14600566 = DRrFKnTwho89785198;     DRrFKnTwho89785198 = DRrFKnTwho24278347;     DRrFKnTwho24278347 = DRrFKnTwho34122810;     DRrFKnTwho34122810 = DRrFKnTwho604716;     DRrFKnTwho604716 = DRrFKnTwho96324533;     DRrFKnTwho96324533 = DRrFKnTwho18397060;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lBAQxQqmrB8801516() {     int TVVdmnDBQk1841365 = -527157186;    int TVVdmnDBQk3598918 = -584680263;    int TVVdmnDBQk47257544 = -394440876;    int TVVdmnDBQk13813634 = -382978408;    int TVVdmnDBQk43121631 = -281652285;    int TVVdmnDBQk59533193 = -561344960;    int TVVdmnDBQk91134632 = 1996560;    int TVVdmnDBQk5839359 = -257400222;    int TVVdmnDBQk3026502 = -936593987;    int TVVdmnDBQk20045161 = -199119845;    int TVVdmnDBQk68291137 = -453741609;    int TVVdmnDBQk36694778 = -16812274;    int TVVdmnDBQk18920311 = -380275022;    int TVVdmnDBQk40813887 = -374170291;    int TVVdmnDBQk24173420 = -425844341;    int TVVdmnDBQk9793999 = -787336347;    int TVVdmnDBQk64696741 = -434823882;    int TVVdmnDBQk65508967 = -489538901;    int TVVdmnDBQk97101734 = 473570;    int TVVdmnDBQk62057651 = -192875325;    int TVVdmnDBQk82059657 = 6194899;    int TVVdmnDBQk40674419 = -599594881;    int TVVdmnDBQk41601881 = 11990990;    int TVVdmnDBQk71693352 = -948165238;    int TVVdmnDBQk34596790 = -494406635;    int TVVdmnDBQk85385409 = -855348534;    int TVVdmnDBQk68715279 = -273706263;    int TVVdmnDBQk49958783 = -635004555;    int TVVdmnDBQk74214055 = -161890840;    int TVVdmnDBQk88852695 = -597081657;    int TVVdmnDBQk29118931 = -368217343;    int TVVdmnDBQk17782863 = 61649292;    int TVVdmnDBQk67406933 = 40127192;    int TVVdmnDBQk76831494 = -810073579;    int TVVdmnDBQk98634290 = -127287236;    int TVVdmnDBQk20103712 = -254376960;    int TVVdmnDBQk86104029 = -819225733;    int TVVdmnDBQk38943441 = -629263363;    int TVVdmnDBQk45031892 = -780930489;    int TVVdmnDBQk5863953 = -83216736;    int TVVdmnDBQk33302281 = -708932708;    int TVVdmnDBQk28125087 = 6796335;    int TVVdmnDBQk29167327 = -701644840;    int TVVdmnDBQk68985546 = -149894159;    int TVVdmnDBQk52467794 = -34117596;    int TVVdmnDBQk12273556 = -244014304;    int TVVdmnDBQk79845157 = -403786776;    int TVVdmnDBQk97230425 = -968837148;    int TVVdmnDBQk60162580 = -613903282;    int TVVdmnDBQk88994307 = 52842784;    int TVVdmnDBQk65631031 = -551785383;    int TVVdmnDBQk11567672 = -448963475;    int TVVdmnDBQk58746852 = -735544850;    int TVVdmnDBQk34519554 = -948472191;    int TVVdmnDBQk26741858 = -884640182;    int TVVdmnDBQk61166946 = -927562305;    int TVVdmnDBQk61997037 = -496671253;    int TVVdmnDBQk75564192 = -446275638;    int TVVdmnDBQk79216844 = -888571774;    int TVVdmnDBQk57736222 = -426303751;    int TVVdmnDBQk90817913 = -187638697;    int TVVdmnDBQk41175850 = -362998886;    int TVVdmnDBQk31625303 = 4490618;    int TVVdmnDBQk14173807 = -239512331;    int TVVdmnDBQk90926229 = -830902503;    int TVVdmnDBQk50508274 = -415390902;    int TVVdmnDBQk69287845 = 43060533;    int TVVdmnDBQk42088817 = -570201444;    int TVVdmnDBQk42179596 = -146883056;    int TVVdmnDBQk4069708 = -71467382;    int TVVdmnDBQk23689970 = -968110614;    int TVVdmnDBQk25753300 = -805560520;    int TVVdmnDBQk20477075 = -708608413;    int TVVdmnDBQk91237781 = -916309694;    int TVVdmnDBQk28755371 = -483942617;    int TVVdmnDBQk53934571 = 99398563;    int TVVdmnDBQk11507092 = -897950041;    int TVVdmnDBQk72616334 = -838114851;    int TVVdmnDBQk19225558 = -814047642;    int TVVdmnDBQk22323235 = -150392331;    int TVVdmnDBQk5540253 = -351561758;    int TVVdmnDBQk71484854 = -304869116;    int TVVdmnDBQk89796202 = 78898727;    int TVVdmnDBQk85219748 = -114733624;    int TVVdmnDBQk23221664 = 54703725;    int TVVdmnDBQk17551260 = -919253868;    int TVVdmnDBQk59036010 = -202805858;    int TVVdmnDBQk32887380 = -11400618;    int TVVdmnDBQk50089636 = -925433397;    int TVVdmnDBQk37467345 = -199724931;    int TVVdmnDBQk58106675 = -757705707;    int TVVdmnDBQk10539837 = -272950096;    int TVVdmnDBQk59726596 = -740691590;    int TVVdmnDBQk87295670 = -254626738;    int TVVdmnDBQk15046040 = -895578040;    int TVVdmnDBQk92126430 = -245933823;    int TVVdmnDBQk96499784 = -997694283;    int TVVdmnDBQk14993520 = -362132510;    int TVVdmnDBQk78059317 = -318991656;    int TVVdmnDBQk1959520 = -527157186;     TVVdmnDBQk1841365 = TVVdmnDBQk3598918;     TVVdmnDBQk3598918 = TVVdmnDBQk47257544;     TVVdmnDBQk47257544 = TVVdmnDBQk13813634;     TVVdmnDBQk13813634 = TVVdmnDBQk43121631;     TVVdmnDBQk43121631 = TVVdmnDBQk59533193;     TVVdmnDBQk59533193 = TVVdmnDBQk91134632;     TVVdmnDBQk91134632 = TVVdmnDBQk5839359;     TVVdmnDBQk5839359 = TVVdmnDBQk3026502;     TVVdmnDBQk3026502 = TVVdmnDBQk20045161;     TVVdmnDBQk20045161 = TVVdmnDBQk68291137;     TVVdmnDBQk68291137 = TVVdmnDBQk36694778;     TVVdmnDBQk36694778 = TVVdmnDBQk18920311;     TVVdmnDBQk18920311 = TVVdmnDBQk40813887;     TVVdmnDBQk40813887 = TVVdmnDBQk24173420;     TVVdmnDBQk24173420 = TVVdmnDBQk9793999;     TVVdmnDBQk9793999 = TVVdmnDBQk64696741;     TVVdmnDBQk64696741 = TVVdmnDBQk65508967;     TVVdmnDBQk65508967 = TVVdmnDBQk97101734;     TVVdmnDBQk97101734 = TVVdmnDBQk62057651;     TVVdmnDBQk62057651 = TVVdmnDBQk82059657;     TVVdmnDBQk82059657 = TVVdmnDBQk40674419;     TVVdmnDBQk40674419 = TVVdmnDBQk41601881;     TVVdmnDBQk41601881 = TVVdmnDBQk71693352;     TVVdmnDBQk71693352 = TVVdmnDBQk34596790;     TVVdmnDBQk34596790 = TVVdmnDBQk85385409;     TVVdmnDBQk85385409 = TVVdmnDBQk68715279;     TVVdmnDBQk68715279 = TVVdmnDBQk49958783;     TVVdmnDBQk49958783 = TVVdmnDBQk74214055;     TVVdmnDBQk74214055 = TVVdmnDBQk88852695;     TVVdmnDBQk88852695 = TVVdmnDBQk29118931;     TVVdmnDBQk29118931 = TVVdmnDBQk17782863;     TVVdmnDBQk17782863 = TVVdmnDBQk67406933;     TVVdmnDBQk67406933 = TVVdmnDBQk76831494;     TVVdmnDBQk76831494 = TVVdmnDBQk98634290;     TVVdmnDBQk98634290 = TVVdmnDBQk20103712;     TVVdmnDBQk20103712 = TVVdmnDBQk86104029;     TVVdmnDBQk86104029 = TVVdmnDBQk38943441;     TVVdmnDBQk38943441 = TVVdmnDBQk45031892;     TVVdmnDBQk45031892 = TVVdmnDBQk5863953;     TVVdmnDBQk5863953 = TVVdmnDBQk33302281;     TVVdmnDBQk33302281 = TVVdmnDBQk28125087;     TVVdmnDBQk28125087 = TVVdmnDBQk29167327;     TVVdmnDBQk29167327 = TVVdmnDBQk68985546;     TVVdmnDBQk68985546 = TVVdmnDBQk52467794;     TVVdmnDBQk52467794 = TVVdmnDBQk12273556;     TVVdmnDBQk12273556 = TVVdmnDBQk79845157;     TVVdmnDBQk79845157 = TVVdmnDBQk97230425;     TVVdmnDBQk97230425 = TVVdmnDBQk60162580;     TVVdmnDBQk60162580 = TVVdmnDBQk88994307;     TVVdmnDBQk88994307 = TVVdmnDBQk65631031;     TVVdmnDBQk65631031 = TVVdmnDBQk11567672;     TVVdmnDBQk11567672 = TVVdmnDBQk58746852;     TVVdmnDBQk58746852 = TVVdmnDBQk34519554;     TVVdmnDBQk34519554 = TVVdmnDBQk26741858;     TVVdmnDBQk26741858 = TVVdmnDBQk61166946;     TVVdmnDBQk61166946 = TVVdmnDBQk61997037;     TVVdmnDBQk61997037 = TVVdmnDBQk75564192;     TVVdmnDBQk75564192 = TVVdmnDBQk79216844;     TVVdmnDBQk79216844 = TVVdmnDBQk57736222;     TVVdmnDBQk57736222 = TVVdmnDBQk90817913;     TVVdmnDBQk90817913 = TVVdmnDBQk41175850;     TVVdmnDBQk41175850 = TVVdmnDBQk31625303;     TVVdmnDBQk31625303 = TVVdmnDBQk14173807;     TVVdmnDBQk14173807 = TVVdmnDBQk90926229;     TVVdmnDBQk90926229 = TVVdmnDBQk50508274;     TVVdmnDBQk50508274 = TVVdmnDBQk69287845;     TVVdmnDBQk69287845 = TVVdmnDBQk42088817;     TVVdmnDBQk42088817 = TVVdmnDBQk42179596;     TVVdmnDBQk42179596 = TVVdmnDBQk4069708;     TVVdmnDBQk4069708 = TVVdmnDBQk23689970;     TVVdmnDBQk23689970 = TVVdmnDBQk25753300;     TVVdmnDBQk25753300 = TVVdmnDBQk20477075;     TVVdmnDBQk20477075 = TVVdmnDBQk91237781;     TVVdmnDBQk91237781 = TVVdmnDBQk28755371;     TVVdmnDBQk28755371 = TVVdmnDBQk53934571;     TVVdmnDBQk53934571 = TVVdmnDBQk11507092;     TVVdmnDBQk11507092 = TVVdmnDBQk72616334;     TVVdmnDBQk72616334 = TVVdmnDBQk19225558;     TVVdmnDBQk19225558 = TVVdmnDBQk22323235;     TVVdmnDBQk22323235 = TVVdmnDBQk5540253;     TVVdmnDBQk5540253 = TVVdmnDBQk71484854;     TVVdmnDBQk71484854 = TVVdmnDBQk89796202;     TVVdmnDBQk89796202 = TVVdmnDBQk85219748;     TVVdmnDBQk85219748 = TVVdmnDBQk23221664;     TVVdmnDBQk23221664 = TVVdmnDBQk17551260;     TVVdmnDBQk17551260 = TVVdmnDBQk59036010;     TVVdmnDBQk59036010 = TVVdmnDBQk32887380;     TVVdmnDBQk32887380 = TVVdmnDBQk50089636;     TVVdmnDBQk50089636 = TVVdmnDBQk37467345;     TVVdmnDBQk37467345 = TVVdmnDBQk58106675;     TVVdmnDBQk58106675 = TVVdmnDBQk10539837;     TVVdmnDBQk10539837 = TVVdmnDBQk59726596;     TVVdmnDBQk59726596 = TVVdmnDBQk87295670;     TVVdmnDBQk87295670 = TVVdmnDBQk15046040;     TVVdmnDBQk15046040 = TVVdmnDBQk92126430;     TVVdmnDBQk92126430 = TVVdmnDBQk96499784;     TVVdmnDBQk96499784 = TVVdmnDBQk14993520;     TVVdmnDBQk14993520 = TVVdmnDBQk78059317;     TVVdmnDBQk78059317 = TVVdmnDBQk1959520;     TVVdmnDBQk1959520 = TVVdmnDBQk1841365;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aLnyCBSglE79257388() {     int slubaeFVqa14627532 = -662197878;    int slubaeFVqa90369805 = -755014276;    int slubaeFVqa65010344 = -802800257;    int slubaeFVqa21754460 = -207180685;    int slubaeFVqa36632736 = -773468750;    int slubaeFVqa2843264 = -338268273;    int slubaeFVqa12342391 = -543675510;    int slubaeFVqa32558471 = -701918478;    int slubaeFVqa98431312 = -898352079;    int slubaeFVqa26263865 = -3207478;    int slubaeFVqa95778519 = -568427624;    int slubaeFVqa68645283 = -524145090;    int slubaeFVqa89063614 = -284057680;    int slubaeFVqa90437965 = -706585490;    int slubaeFVqa47479972 = -332830172;    int slubaeFVqa38219068 = -238227736;    int slubaeFVqa20448559 = -98598349;    int slubaeFVqa49067452 = -44749203;    int slubaeFVqa98262365 = 91004374;    int slubaeFVqa64712147 = -792119464;    int slubaeFVqa62233714 = -485121365;    int slubaeFVqa69722238 = -769453229;    int slubaeFVqa80362176 = 80987096;    int slubaeFVqa16200482 = -208510250;    int slubaeFVqa40491049 = -704848559;    int slubaeFVqa66704055 = -545779733;    int slubaeFVqa80130815 = -907412543;    int slubaeFVqa17311708 = -808802659;    int slubaeFVqa90831108 = -376108080;    int slubaeFVqa41603810 = -226967811;    int slubaeFVqa73554926 = -644634093;    int slubaeFVqa21101687 = 82929614;    int slubaeFVqa13452883 = -935727592;    int slubaeFVqa53907867 = -714827798;    int slubaeFVqa81694823 = -737319116;    int slubaeFVqa15327827 = -42940064;    int slubaeFVqa22868090 = -609135084;    int slubaeFVqa96647463 = -548883455;    int slubaeFVqa50807278 = -622781066;    int slubaeFVqa32128683 = 72366418;    int slubaeFVqa94733588 = 15593298;    int slubaeFVqa35827543 = -991514999;    int slubaeFVqa64050797 = -725078285;    int slubaeFVqa13614085 = -491066402;    int slubaeFVqa32476284 = 73112443;    int slubaeFVqa85901684 = -317332744;    int slubaeFVqa57427176 = -901301829;    int slubaeFVqa26670392 = -24189927;    int slubaeFVqa4226630 = -777505256;    int slubaeFVqa35944728 = -558193679;    int slubaeFVqa83958524 = -804669691;    int slubaeFVqa39744219 = -920148813;    int slubaeFVqa93232002 = -911548133;    int slubaeFVqa79219279 = -537923933;    int slubaeFVqa17023674 = -879084669;    int slubaeFVqa44905294 = -892744649;    int slubaeFVqa10007630 = -736001372;    int slubaeFVqa48809863 = -494290008;    int slubaeFVqa81263410 = -502332126;    int slubaeFVqa69928681 = -127689017;    int slubaeFVqa22712448 = -430855730;    int slubaeFVqa95030683 = -734872851;    int slubaeFVqa41727362 = -225810399;    int slubaeFVqa56827502 = -571384269;    int slubaeFVqa52708939 = -358573386;    int slubaeFVqa74676832 = -551357238;    int slubaeFVqa55192401 = -588417498;    int slubaeFVqa35155747 = -569229883;    int slubaeFVqa8743143 = -969266374;    int slubaeFVqa32152146 = -189890109;    int slubaeFVqa15350979 = -629092653;    int slubaeFVqa23801095 = -549714895;    int slubaeFVqa98260173 = -421968138;    int slubaeFVqa66133682 = -981362044;    int slubaeFVqa69978559 = -707712763;    int slubaeFVqa26406172 = -493606367;    int slubaeFVqa5671442 = 55625056;    int slubaeFVqa66748092 = -427946503;    int slubaeFVqa83724197 = -181622694;    int slubaeFVqa54589364 = -287515816;    int slubaeFVqa9276879 = -644477905;    int slubaeFVqa53460424 = -783222616;    int slubaeFVqa13085079 = 68702597;    int slubaeFVqa54886380 = -817914402;    int slubaeFVqa57645286 = -422298121;    int slubaeFVqa33810708 = -724485280;    int slubaeFVqa27869684 = -5522254;    int slubaeFVqa34233603 = -297803660;    int slubaeFVqa36884193 = -835743129;    int slubaeFVqa36789529 = -844574468;    int slubaeFVqa5320197 = -306938692;    int slubaeFVqa74058226 = -14845077;    int slubaeFVqa15384053 = 53448671;    int slubaeFVqa80878597 = -395092049;    int slubaeFVqa9416235 = -496777853;    int slubaeFVqa99702905 = -249533851;    int slubaeFVqa94100180 = -665704601;    int slubaeFVqa7223295 = -53694017;    int slubaeFVqa60905146 = -32493016;    int slubaeFVqa57799451 = -662197878;     slubaeFVqa14627532 = slubaeFVqa90369805;     slubaeFVqa90369805 = slubaeFVqa65010344;     slubaeFVqa65010344 = slubaeFVqa21754460;     slubaeFVqa21754460 = slubaeFVqa36632736;     slubaeFVqa36632736 = slubaeFVqa2843264;     slubaeFVqa2843264 = slubaeFVqa12342391;     slubaeFVqa12342391 = slubaeFVqa32558471;     slubaeFVqa32558471 = slubaeFVqa98431312;     slubaeFVqa98431312 = slubaeFVqa26263865;     slubaeFVqa26263865 = slubaeFVqa95778519;     slubaeFVqa95778519 = slubaeFVqa68645283;     slubaeFVqa68645283 = slubaeFVqa89063614;     slubaeFVqa89063614 = slubaeFVqa90437965;     slubaeFVqa90437965 = slubaeFVqa47479972;     slubaeFVqa47479972 = slubaeFVqa38219068;     slubaeFVqa38219068 = slubaeFVqa20448559;     slubaeFVqa20448559 = slubaeFVqa49067452;     slubaeFVqa49067452 = slubaeFVqa98262365;     slubaeFVqa98262365 = slubaeFVqa64712147;     slubaeFVqa64712147 = slubaeFVqa62233714;     slubaeFVqa62233714 = slubaeFVqa69722238;     slubaeFVqa69722238 = slubaeFVqa80362176;     slubaeFVqa80362176 = slubaeFVqa16200482;     slubaeFVqa16200482 = slubaeFVqa40491049;     slubaeFVqa40491049 = slubaeFVqa66704055;     slubaeFVqa66704055 = slubaeFVqa80130815;     slubaeFVqa80130815 = slubaeFVqa17311708;     slubaeFVqa17311708 = slubaeFVqa90831108;     slubaeFVqa90831108 = slubaeFVqa41603810;     slubaeFVqa41603810 = slubaeFVqa73554926;     slubaeFVqa73554926 = slubaeFVqa21101687;     slubaeFVqa21101687 = slubaeFVqa13452883;     slubaeFVqa13452883 = slubaeFVqa53907867;     slubaeFVqa53907867 = slubaeFVqa81694823;     slubaeFVqa81694823 = slubaeFVqa15327827;     slubaeFVqa15327827 = slubaeFVqa22868090;     slubaeFVqa22868090 = slubaeFVqa96647463;     slubaeFVqa96647463 = slubaeFVqa50807278;     slubaeFVqa50807278 = slubaeFVqa32128683;     slubaeFVqa32128683 = slubaeFVqa94733588;     slubaeFVqa94733588 = slubaeFVqa35827543;     slubaeFVqa35827543 = slubaeFVqa64050797;     slubaeFVqa64050797 = slubaeFVqa13614085;     slubaeFVqa13614085 = slubaeFVqa32476284;     slubaeFVqa32476284 = slubaeFVqa85901684;     slubaeFVqa85901684 = slubaeFVqa57427176;     slubaeFVqa57427176 = slubaeFVqa26670392;     slubaeFVqa26670392 = slubaeFVqa4226630;     slubaeFVqa4226630 = slubaeFVqa35944728;     slubaeFVqa35944728 = slubaeFVqa83958524;     slubaeFVqa83958524 = slubaeFVqa39744219;     slubaeFVqa39744219 = slubaeFVqa93232002;     slubaeFVqa93232002 = slubaeFVqa79219279;     slubaeFVqa79219279 = slubaeFVqa17023674;     slubaeFVqa17023674 = slubaeFVqa44905294;     slubaeFVqa44905294 = slubaeFVqa10007630;     slubaeFVqa10007630 = slubaeFVqa48809863;     slubaeFVqa48809863 = slubaeFVqa81263410;     slubaeFVqa81263410 = slubaeFVqa69928681;     slubaeFVqa69928681 = slubaeFVqa22712448;     slubaeFVqa22712448 = slubaeFVqa95030683;     slubaeFVqa95030683 = slubaeFVqa41727362;     slubaeFVqa41727362 = slubaeFVqa56827502;     slubaeFVqa56827502 = slubaeFVqa52708939;     slubaeFVqa52708939 = slubaeFVqa74676832;     slubaeFVqa74676832 = slubaeFVqa55192401;     slubaeFVqa55192401 = slubaeFVqa35155747;     slubaeFVqa35155747 = slubaeFVqa8743143;     slubaeFVqa8743143 = slubaeFVqa32152146;     slubaeFVqa32152146 = slubaeFVqa15350979;     slubaeFVqa15350979 = slubaeFVqa23801095;     slubaeFVqa23801095 = slubaeFVqa98260173;     slubaeFVqa98260173 = slubaeFVqa66133682;     slubaeFVqa66133682 = slubaeFVqa69978559;     slubaeFVqa69978559 = slubaeFVqa26406172;     slubaeFVqa26406172 = slubaeFVqa5671442;     slubaeFVqa5671442 = slubaeFVqa66748092;     slubaeFVqa66748092 = slubaeFVqa83724197;     slubaeFVqa83724197 = slubaeFVqa54589364;     slubaeFVqa54589364 = slubaeFVqa9276879;     slubaeFVqa9276879 = slubaeFVqa53460424;     slubaeFVqa53460424 = slubaeFVqa13085079;     slubaeFVqa13085079 = slubaeFVqa54886380;     slubaeFVqa54886380 = slubaeFVqa57645286;     slubaeFVqa57645286 = slubaeFVqa33810708;     slubaeFVqa33810708 = slubaeFVqa27869684;     slubaeFVqa27869684 = slubaeFVqa34233603;     slubaeFVqa34233603 = slubaeFVqa36884193;     slubaeFVqa36884193 = slubaeFVqa36789529;     slubaeFVqa36789529 = slubaeFVqa5320197;     slubaeFVqa5320197 = slubaeFVqa74058226;     slubaeFVqa74058226 = slubaeFVqa15384053;     slubaeFVqa15384053 = slubaeFVqa80878597;     slubaeFVqa80878597 = slubaeFVqa9416235;     slubaeFVqa9416235 = slubaeFVqa99702905;     slubaeFVqa99702905 = slubaeFVqa94100180;     slubaeFVqa94100180 = slubaeFVqa7223295;     slubaeFVqa7223295 = slubaeFVqa60905146;     slubaeFVqa60905146 = slubaeFVqa57799451;     slubaeFVqa57799451 = slubaeFVqa14627532;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TuXuQYqoGZ97470730() {     int enCovtdltG98071837 = -442863558;    int enCovtdltG7241895 = -754396464;    int enCovtdltG61418851 = -596325485;    int enCovtdltG84546711 = -400980521;    int enCovtdltG42754060 = -580336241;    int enCovtdltG1347665 = -117339926;    int enCovtdltG83381708 = -283586824;    int enCovtdltG46905913 = -689076508;    int enCovtdltG56961013 = -139811430;    int enCovtdltG58809517 = -471583590;    int enCovtdltG45194457 = -758543497;    int enCovtdltG31850683 = -511763337;    int enCovtdltG7181390 = -75718921;    int enCovtdltG70064858 = -494875847;    int enCovtdltG8334436 = -426853154;    int enCovtdltG24608230 = -26307062;    int enCovtdltG14200444 = -951943376;    int enCovtdltG29918899 = -292054080;    int enCovtdltG60702507 = 92329504;    int enCovtdltG54079590 = -221705457;    int enCovtdltG17874102 = -74650396;    int enCovtdltG81869556 = -760604999;    int enCovtdltG84187884 = 41097898;    int enCovtdltG50907040 = -280335646;    int enCovtdltG82067007 = -164501318;    int enCovtdltG62046204 = -426289449;    int enCovtdltG29585556 = -535496886;    int enCovtdltG57043333 = -878757543;    int enCovtdltG26912203 = -238613222;    int enCovtdltG57437152 = -739817663;    int enCovtdltG91650240 = -282391294;    int enCovtdltG84208113 = -932265466;    int enCovtdltG56991769 = -556599770;    int enCovtdltG96066377 = -306429968;    int enCovtdltG60421553 = -934983851;    int enCovtdltG27717011 = -930963970;    int enCovtdltG91482268 = -487984264;    int enCovtdltG37226296 = -257353975;    int enCovtdltG99802356 = -807678864;    int enCovtdltG81623818 = -780227104;    int enCovtdltG28382513 = -655955675;    int enCovtdltG86315328 = -465423320;    int enCovtdltG87617866 = -176135528;    int enCovtdltG5519025 = -247249173;    int enCovtdltG24420781 = -755044510;    int enCovtdltG25507681 = -336943668;    int enCovtdltG33442607 = 66393942;    int enCovtdltG46340988 = -22911646;    int enCovtdltG35073742 = -263107339;    int enCovtdltG71277212 = -765214134;    int enCovtdltG57215972 = 74428887;    int enCovtdltG95245012 = -978408125;    int enCovtdltG7344462 = -23295449;    int enCovtdltG3896039 = -256337320;    int enCovtdltG79596628 = -127570792;    int enCovtdltG16202282 = -682258559;    int enCovtdltG23054011 = -695494363;    int enCovtdltG10511812 = -215989839;    int enCovtdltG2479705 = -136479203;    int enCovtdltG80707855 = -54046793;    int enCovtdltG71762108 = -581843041;    int enCovtdltG26338376 = -404829281;    int enCovtdltG19993711 = -350463286;    int enCovtdltG99523861 = -399993768;    int enCovtdltG67159277 = -89192297;    int enCovtdltG60986343 = -826278031;    int enCovtdltG74858913 = -955163567;    int enCovtdltG11115013 = -769288954;    int enCovtdltG9643305 = -559891996;    int enCovtdltG80617425 = -495889185;    int enCovtdltG33125961 = -538322798;    int enCovtdltG76974147 = -594589402;    int enCovtdltG30116542 = -484375216;    int enCovtdltG79078689 = -127443393;    int enCovtdltG25697077 = -565749782;    int enCovtdltG31558773 = -609227076;    int enCovtdltG94251690 = -484469472;    int enCovtdltG78668859 = -711652930;    int enCovtdltG26486259 = -525291137;    int enCovtdltG56559326 = -827557651;    int enCovtdltG28603597 = -392683391;    int enCovtdltG83244568 = -412585240;    int enCovtdltG21969592 = -515650205;    int enCovtdltG55634991 = -473399089;    int enCovtdltG221180 = -714246550;    int enCovtdltG96405227 = -303983169;    int enCovtdltG76863652 = -808970017;    int enCovtdltG53095731 = -200262450;    int enCovtdltG16469749 = -78859176;    int enCovtdltG44219272 = -152725293;    int enCovtdltG4663000 = -135469607;    int enCovtdltG80970456 = -171994425;    int enCovtdltG34746592 = -20874772;    int enCovtdltG19094501 = -653632072;    int enCovtdltG9861710 = -98384064;    int enCovtdltG2044138 = -151126395;    int enCovtdltG66321618 = -14960035;    int enCovtdltG88094005 = -776141760;    int enCovtdltG38359748 = -58056876;    int enCovtdltG63434437 = -442863558;     enCovtdltG98071837 = enCovtdltG7241895;     enCovtdltG7241895 = enCovtdltG61418851;     enCovtdltG61418851 = enCovtdltG84546711;     enCovtdltG84546711 = enCovtdltG42754060;     enCovtdltG42754060 = enCovtdltG1347665;     enCovtdltG1347665 = enCovtdltG83381708;     enCovtdltG83381708 = enCovtdltG46905913;     enCovtdltG46905913 = enCovtdltG56961013;     enCovtdltG56961013 = enCovtdltG58809517;     enCovtdltG58809517 = enCovtdltG45194457;     enCovtdltG45194457 = enCovtdltG31850683;     enCovtdltG31850683 = enCovtdltG7181390;     enCovtdltG7181390 = enCovtdltG70064858;     enCovtdltG70064858 = enCovtdltG8334436;     enCovtdltG8334436 = enCovtdltG24608230;     enCovtdltG24608230 = enCovtdltG14200444;     enCovtdltG14200444 = enCovtdltG29918899;     enCovtdltG29918899 = enCovtdltG60702507;     enCovtdltG60702507 = enCovtdltG54079590;     enCovtdltG54079590 = enCovtdltG17874102;     enCovtdltG17874102 = enCovtdltG81869556;     enCovtdltG81869556 = enCovtdltG84187884;     enCovtdltG84187884 = enCovtdltG50907040;     enCovtdltG50907040 = enCovtdltG82067007;     enCovtdltG82067007 = enCovtdltG62046204;     enCovtdltG62046204 = enCovtdltG29585556;     enCovtdltG29585556 = enCovtdltG57043333;     enCovtdltG57043333 = enCovtdltG26912203;     enCovtdltG26912203 = enCovtdltG57437152;     enCovtdltG57437152 = enCovtdltG91650240;     enCovtdltG91650240 = enCovtdltG84208113;     enCovtdltG84208113 = enCovtdltG56991769;     enCovtdltG56991769 = enCovtdltG96066377;     enCovtdltG96066377 = enCovtdltG60421553;     enCovtdltG60421553 = enCovtdltG27717011;     enCovtdltG27717011 = enCovtdltG91482268;     enCovtdltG91482268 = enCovtdltG37226296;     enCovtdltG37226296 = enCovtdltG99802356;     enCovtdltG99802356 = enCovtdltG81623818;     enCovtdltG81623818 = enCovtdltG28382513;     enCovtdltG28382513 = enCovtdltG86315328;     enCovtdltG86315328 = enCovtdltG87617866;     enCovtdltG87617866 = enCovtdltG5519025;     enCovtdltG5519025 = enCovtdltG24420781;     enCovtdltG24420781 = enCovtdltG25507681;     enCovtdltG25507681 = enCovtdltG33442607;     enCovtdltG33442607 = enCovtdltG46340988;     enCovtdltG46340988 = enCovtdltG35073742;     enCovtdltG35073742 = enCovtdltG71277212;     enCovtdltG71277212 = enCovtdltG57215972;     enCovtdltG57215972 = enCovtdltG95245012;     enCovtdltG95245012 = enCovtdltG7344462;     enCovtdltG7344462 = enCovtdltG3896039;     enCovtdltG3896039 = enCovtdltG79596628;     enCovtdltG79596628 = enCovtdltG16202282;     enCovtdltG16202282 = enCovtdltG23054011;     enCovtdltG23054011 = enCovtdltG10511812;     enCovtdltG10511812 = enCovtdltG2479705;     enCovtdltG2479705 = enCovtdltG80707855;     enCovtdltG80707855 = enCovtdltG71762108;     enCovtdltG71762108 = enCovtdltG26338376;     enCovtdltG26338376 = enCovtdltG19993711;     enCovtdltG19993711 = enCovtdltG99523861;     enCovtdltG99523861 = enCovtdltG67159277;     enCovtdltG67159277 = enCovtdltG60986343;     enCovtdltG60986343 = enCovtdltG74858913;     enCovtdltG74858913 = enCovtdltG11115013;     enCovtdltG11115013 = enCovtdltG9643305;     enCovtdltG9643305 = enCovtdltG80617425;     enCovtdltG80617425 = enCovtdltG33125961;     enCovtdltG33125961 = enCovtdltG76974147;     enCovtdltG76974147 = enCovtdltG30116542;     enCovtdltG30116542 = enCovtdltG79078689;     enCovtdltG79078689 = enCovtdltG25697077;     enCovtdltG25697077 = enCovtdltG31558773;     enCovtdltG31558773 = enCovtdltG94251690;     enCovtdltG94251690 = enCovtdltG78668859;     enCovtdltG78668859 = enCovtdltG26486259;     enCovtdltG26486259 = enCovtdltG56559326;     enCovtdltG56559326 = enCovtdltG28603597;     enCovtdltG28603597 = enCovtdltG83244568;     enCovtdltG83244568 = enCovtdltG21969592;     enCovtdltG21969592 = enCovtdltG55634991;     enCovtdltG55634991 = enCovtdltG221180;     enCovtdltG221180 = enCovtdltG96405227;     enCovtdltG96405227 = enCovtdltG76863652;     enCovtdltG76863652 = enCovtdltG53095731;     enCovtdltG53095731 = enCovtdltG16469749;     enCovtdltG16469749 = enCovtdltG44219272;     enCovtdltG44219272 = enCovtdltG4663000;     enCovtdltG4663000 = enCovtdltG80970456;     enCovtdltG80970456 = enCovtdltG34746592;     enCovtdltG34746592 = enCovtdltG19094501;     enCovtdltG19094501 = enCovtdltG9861710;     enCovtdltG9861710 = enCovtdltG2044138;     enCovtdltG2044138 = enCovtdltG66321618;     enCovtdltG66321618 = enCovtdltG88094005;     enCovtdltG88094005 = enCovtdltG38359748;     enCovtdltG38359748 = enCovtdltG63434437;     enCovtdltG63434437 = enCovtdltG98071837;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NqPdhdBLNv67926603() {     int djmjjPYBPz10858005 = -577904250;    int djmjjPYBPz94012783 = -924730477;    int djmjjPYBPz79171651 = 95315133;    int djmjjPYBPz92487536 = -225182798;    int djmjjPYBPz36265164 = 27847294;    int djmjjPYBPz44657735 = -994263239;    int djmjjPYBPz4589467 = -829258894;    int djmjjPYBPz73625026 = -33594764;    int djmjjPYBPz52365823 = -101569523;    int djmjjPYBPz65028222 = -275671223;    int djmjjPYBPz72681839 = -873229512;    int djmjjPYBPz63801187 = 80903847;    int djmjjPYBPz77324692 = 20498422;    int djmjjPYBPz19688937 = -827291045;    int djmjjPYBPz31640989 = -333838985;    int djmjjPYBPz53033298 = -577198451;    int djmjjPYBPz69952261 = -615717843;    int djmjjPYBPz13477384 = -947264382;    int djmjjPYBPz61863138 = -917139692;    int djmjjPYBPz56734086 = -820949596;    int djmjjPYBPz98048158 = -565966660;    int djmjjPYBPz10917376 = -930463347;    int djmjjPYBPz22948180 = -989905996;    int djmjjPYBPz95414169 = -640680658;    int djmjjPYBPz87961266 = -374943243;    int djmjjPYBPz43364849 = -116720648;    int djmjjPYBPz41001092 = -69203166;    int djmjjPYBPz24396258 = 47444352;    int djmjjPYBPz43529256 = -452830462;    int djmjjPYBPz10188267 = -369703817;    int djmjjPYBPz36086236 = -558808044;    int djmjjPYBPz87526937 = -910985145;    int djmjjPYBPz3037719 = -432454554;    int djmjjPYBPz73142750 = -211184186;    int djmjjPYBPz43482085 = -445015732;    int djmjjPYBPz22941126 = -719527074;    int djmjjPYBPz28246329 = -277893614;    int djmjjPYBPz94930318 = -176974066;    int djmjjPYBPz5577743 = -649529441;    int djmjjPYBPz7888548 = -624643950;    int djmjjPYBPz89813821 = 68570332;    int djmjjPYBPz94017784 = -363734655;    int djmjjPYBPz22501336 = -199568973;    int djmjjPYBPz50147562 = -588421415;    int djmjjPYBPz4429272 = -647814470;    int djmjjPYBPz99135809 = -410262107;    int djmjjPYBPz11024627 = -431121110;    int djmjjPYBPz75780954 = -178264425;    int djmjjPYBPz79137791 = -426709314;    int djmjjPYBPz18227633 = -276250597;    int djmjjPYBPz75543466 = -178455422;    int djmjjPYBPz23421560 = -349593463;    int djmjjPYBPz41829612 = -199298732;    int djmjjPYBPz48595763 = -945789062;    int djmjjPYBPz69878444 = -122015278;    int djmjjPYBPz99940629 = -647440903;    int djmjjPYBPz71064603 = -934824482;    int djmjjPYBPz83757481 = -264004209;    int djmjjPYBPz4526271 = -850239556;    int djmjjPYBPz92900315 = -855432059;    int djmjjPYBPz3656643 = -825060074;    int djmjjPYBPz80193208 = -776703247;    int djmjjPYBPz30095770 = -580764302;    int djmjjPYBPz42177556 = -731865706;    int djmjjPYBPz28941986 = -716863180;    int djmjjPYBPz85154902 = -962244368;    int djmjjPYBPz60763469 = -486641599;    int djmjjPYBPz4181943 = -768317392;    int djmjjPYBPz76206851 = -282275314;    int djmjjPYBPz8699864 = -614311912;    int djmjjPYBPz24786970 = -199304837;    int djmjjPYBPz75021942 = -338743777;    int djmjjPYBPz7899642 = -197734941;    int djmjjPYBPz53974590 = -192495742;    int djmjjPYBPz66920265 = -789519928;    int djmjjPYBPz4030374 = -102232006;    int djmjjPYBPz88416039 = -630894375;    int djmjjPYBPz72800617 = -301484581;    int djmjjPYBPz90984898 = -992866189;    int djmjjPYBPz88825456 = -964681136;    int djmjjPYBPz32340223 = -685599538;    int djmjjPYBPz65220138 = -890938741;    int djmjjPYBPz45258467 = -525846335;    int djmjjPYBPz25301623 = -76579866;    int djmjjPYBPz34644801 = -91248396;    int djmjjPYBPz12664677 = -109214581;    int djmjjPYBPz45697326 = -611686413;    int djmjjPYBPz54441955 = -486665493;    int djmjjPYBPz3264306 = 10831092;    int djmjjPYBPz43541456 = -797574829;    int djmjjPYBPz51876522 = -784702593;    int djmjjPYBPz44488847 = 86110594;    int djmjjPYBPz90404048 = -326734511;    int djmjjPYBPz12677428 = -794097383;    int djmjjPYBPz4231905 = -799583877;    int djmjjPYBPz9620613 = -154726422;    int djmjjPYBPz63922014 = -782970353;    int djmjjPYBPz80323779 = -467703268;    int djmjjPYBPz21205577 = -871558236;    int djmjjPYBPz19274370 = -577904250;     djmjjPYBPz10858005 = djmjjPYBPz94012783;     djmjjPYBPz94012783 = djmjjPYBPz79171651;     djmjjPYBPz79171651 = djmjjPYBPz92487536;     djmjjPYBPz92487536 = djmjjPYBPz36265164;     djmjjPYBPz36265164 = djmjjPYBPz44657735;     djmjjPYBPz44657735 = djmjjPYBPz4589467;     djmjjPYBPz4589467 = djmjjPYBPz73625026;     djmjjPYBPz73625026 = djmjjPYBPz52365823;     djmjjPYBPz52365823 = djmjjPYBPz65028222;     djmjjPYBPz65028222 = djmjjPYBPz72681839;     djmjjPYBPz72681839 = djmjjPYBPz63801187;     djmjjPYBPz63801187 = djmjjPYBPz77324692;     djmjjPYBPz77324692 = djmjjPYBPz19688937;     djmjjPYBPz19688937 = djmjjPYBPz31640989;     djmjjPYBPz31640989 = djmjjPYBPz53033298;     djmjjPYBPz53033298 = djmjjPYBPz69952261;     djmjjPYBPz69952261 = djmjjPYBPz13477384;     djmjjPYBPz13477384 = djmjjPYBPz61863138;     djmjjPYBPz61863138 = djmjjPYBPz56734086;     djmjjPYBPz56734086 = djmjjPYBPz98048158;     djmjjPYBPz98048158 = djmjjPYBPz10917376;     djmjjPYBPz10917376 = djmjjPYBPz22948180;     djmjjPYBPz22948180 = djmjjPYBPz95414169;     djmjjPYBPz95414169 = djmjjPYBPz87961266;     djmjjPYBPz87961266 = djmjjPYBPz43364849;     djmjjPYBPz43364849 = djmjjPYBPz41001092;     djmjjPYBPz41001092 = djmjjPYBPz24396258;     djmjjPYBPz24396258 = djmjjPYBPz43529256;     djmjjPYBPz43529256 = djmjjPYBPz10188267;     djmjjPYBPz10188267 = djmjjPYBPz36086236;     djmjjPYBPz36086236 = djmjjPYBPz87526937;     djmjjPYBPz87526937 = djmjjPYBPz3037719;     djmjjPYBPz3037719 = djmjjPYBPz73142750;     djmjjPYBPz73142750 = djmjjPYBPz43482085;     djmjjPYBPz43482085 = djmjjPYBPz22941126;     djmjjPYBPz22941126 = djmjjPYBPz28246329;     djmjjPYBPz28246329 = djmjjPYBPz94930318;     djmjjPYBPz94930318 = djmjjPYBPz5577743;     djmjjPYBPz5577743 = djmjjPYBPz7888548;     djmjjPYBPz7888548 = djmjjPYBPz89813821;     djmjjPYBPz89813821 = djmjjPYBPz94017784;     djmjjPYBPz94017784 = djmjjPYBPz22501336;     djmjjPYBPz22501336 = djmjjPYBPz50147562;     djmjjPYBPz50147562 = djmjjPYBPz4429272;     djmjjPYBPz4429272 = djmjjPYBPz99135809;     djmjjPYBPz99135809 = djmjjPYBPz11024627;     djmjjPYBPz11024627 = djmjjPYBPz75780954;     djmjjPYBPz75780954 = djmjjPYBPz79137791;     djmjjPYBPz79137791 = djmjjPYBPz18227633;     djmjjPYBPz18227633 = djmjjPYBPz75543466;     djmjjPYBPz75543466 = djmjjPYBPz23421560;     djmjjPYBPz23421560 = djmjjPYBPz41829612;     djmjjPYBPz41829612 = djmjjPYBPz48595763;     djmjjPYBPz48595763 = djmjjPYBPz69878444;     djmjjPYBPz69878444 = djmjjPYBPz99940629;     djmjjPYBPz99940629 = djmjjPYBPz71064603;     djmjjPYBPz71064603 = djmjjPYBPz83757481;     djmjjPYBPz83757481 = djmjjPYBPz4526271;     djmjjPYBPz4526271 = djmjjPYBPz92900315;     djmjjPYBPz92900315 = djmjjPYBPz3656643;     djmjjPYBPz3656643 = djmjjPYBPz80193208;     djmjjPYBPz80193208 = djmjjPYBPz30095770;     djmjjPYBPz30095770 = djmjjPYBPz42177556;     djmjjPYBPz42177556 = djmjjPYBPz28941986;     djmjjPYBPz28941986 = djmjjPYBPz85154902;     djmjjPYBPz85154902 = djmjjPYBPz60763469;     djmjjPYBPz60763469 = djmjjPYBPz4181943;     djmjjPYBPz4181943 = djmjjPYBPz76206851;     djmjjPYBPz76206851 = djmjjPYBPz8699864;     djmjjPYBPz8699864 = djmjjPYBPz24786970;     djmjjPYBPz24786970 = djmjjPYBPz75021942;     djmjjPYBPz75021942 = djmjjPYBPz7899642;     djmjjPYBPz7899642 = djmjjPYBPz53974590;     djmjjPYBPz53974590 = djmjjPYBPz66920265;     djmjjPYBPz66920265 = djmjjPYBPz4030374;     djmjjPYBPz4030374 = djmjjPYBPz88416039;     djmjjPYBPz88416039 = djmjjPYBPz72800617;     djmjjPYBPz72800617 = djmjjPYBPz90984898;     djmjjPYBPz90984898 = djmjjPYBPz88825456;     djmjjPYBPz88825456 = djmjjPYBPz32340223;     djmjjPYBPz32340223 = djmjjPYBPz65220138;     djmjjPYBPz65220138 = djmjjPYBPz45258467;     djmjjPYBPz45258467 = djmjjPYBPz25301623;     djmjjPYBPz25301623 = djmjjPYBPz34644801;     djmjjPYBPz34644801 = djmjjPYBPz12664677;     djmjjPYBPz12664677 = djmjjPYBPz45697326;     djmjjPYBPz45697326 = djmjjPYBPz54441955;     djmjjPYBPz54441955 = djmjjPYBPz3264306;     djmjjPYBPz3264306 = djmjjPYBPz43541456;     djmjjPYBPz43541456 = djmjjPYBPz51876522;     djmjjPYBPz51876522 = djmjjPYBPz44488847;     djmjjPYBPz44488847 = djmjjPYBPz90404048;     djmjjPYBPz90404048 = djmjjPYBPz12677428;     djmjjPYBPz12677428 = djmjjPYBPz4231905;     djmjjPYBPz4231905 = djmjjPYBPz9620613;     djmjjPYBPz9620613 = djmjjPYBPz63922014;     djmjjPYBPz63922014 = djmjjPYBPz80323779;     djmjjPYBPz80323779 = djmjjPYBPz21205577;     djmjjPYBPz21205577 = djmjjPYBPz19274370;     djmjjPYBPz19274370 = djmjjPYBPz10858005;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jtDslnArhf68368706() {     int doyikFSEhu46037761 = -474824763;    int doyikFSEhu26558855 = 98987579;    int doyikFSEhu24380518 = -595569479;    int doyikFSEhu30355190 = -235251776;    int doyikFSEhu19110421 = -586670851;    int doyikFSEhu68045490 = -932362119;    int doyikFSEhu49405628 = -392381804;    int doyikFSEhu6763947 = -144532348;    int doyikFSEhu62193601 = -233877584;    int doyikFSEhu66370998 = -185693318;    int doyikFSEhu17390476 = 74932143;    int doyikFSEhu37362965 = -941695899;    int doyikFSEhu82623262 = -480343080;    int doyikFSEhu58083547 = -242261949;    int doyikFSEhu46510710 = 94410323;    int doyikFSEhu51149732 = -76961732;    int doyikFSEhu19674671 = -737157560;    int doyikFSEhu80011752 = -557145753;    int doyikFSEhu83877129 = -343728746;    int doyikFSEhu65831103 = 39196263;    int doyikFSEhu99435898 = -238303859;    int doyikFSEhu25484148 = 4903875;    int doyikFSEhu28123402 = -638032641;    int doyikFSEhu22770978 = -584098006;    int doyikFSEhu953083 = -880250438;    int doyikFSEhu96412411 = -342840142;    int doyikFSEhu29284467 = -159696226;    int doyikFSEhu62257108 = -405841218;    int doyikFSEhu54360423 = -365234507;    int doyikFSEhu96006692 = -244454465;    int doyikFSEhu91400358 = -529447711;    int doyikFSEhu63663094 = -49954416;    int doyikFSEhu44669915 = -635708618;    int doyikFSEhu9324974 = -563383522;    int doyikFSEhu20413944 = -859490110;    int doyikFSEhu17029920 = -743719808;    int doyikFSEhu46508733 = -801097538;    int doyikFSEhu36342763 = 31043049;    int doyikFSEhu24347663 = -179744634;    int doyikFSEhu82466099 = -995853139;    int doyikFSEhu92146832 = -722137429;    int doyikFSEhu4530971 = -926162598;    int doyikFSEhu2651638 = -651402747;    int doyikFSEhu34988321 = -269992864;    int doyikFSEhu68402976 = -976468507;    int doyikFSEhu40436253 = -853764971;    int doyikFSEhu73206251 = 74234206;    int doyikFSEhu74436014 = -581390840;    int doyikFSEhu99003356 = -976264125;    int doyikFSEhu42216376 = -342282433;    int doyikFSEhu89480806 = -107861000;    int doyikFSEhu10902107 = -869452335;    int doyikFSEhu79180816 = -155159236;    int doyikFSEhu43331763 = -279001761;    int doyikFSEhu68932808 = -500264602;    int doyikFSEhu20553614 = -379728639;    int doyikFSEhu98435453 = -262979780;    int doyikFSEhu1609540 = 88528526;    int doyikFSEhu29402107 = -355001338;    int doyikFSEhu22698009 = -143830709;    int doyikFSEhu38761023 = -672665893;    int doyikFSEhu87148519 = -986540586;    int doyikFSEhu52403524 = -779297842;    int doyikFSEhu66186909 = -989423120;    int doyikFSEhu74970639 = -656245607;    int doyikFSEhu53727381 = -875113441;    int doyikFSEhu92693049 = -205987282;    int doyikFSEhu73298289 = -916959559;    int doyikFSEhu37669603 = -382771839;    int doyikFSEhu29480790 = -161869869;    int doyikFSEhu4640999 = -275864194;    int doyikFSEhu83331908 = -668200610;    int doyikFSEhu55664089 = -277401120;    int doyikFSEhu1411031 = -347875607;    int doyikFSEhu73684270 = -238666308;    int doyikFSEhu94904927 = -312141262;    int doyikFSEhu22832510 = -343693378;    int doyikFSEhu93135080 = -268039778;    int doyikFSEhu54368002 = -607629499;    int doyikFSEhu60516830 = 73514533;    int doyikFSEhu23206161 = -317074349;    int doyikFSEhu54848453 = -578305387;    int doyikFSEhu63253752 = -429577093;    int doyikFSEhu12144047 = 77047925;    int doyikFSEhu6525886 = -36593466;    int doyikFSEhu80498251 = -659995377;    int doyikFSEhu84482277 = -894795180;    int doyikFSEhu1338152 = -256706857;    int doyikFSEhu40392166 = 36881080;    int doyikFSEhu99860330 = -379761472;    int doyikFSEhu18594467 = -380740029;    int doyikFSEhu44899193 = -789626065;    int doyikFSEhu6940656 = -613955613;    int doyikFSEhu1649655 = 64086075;    int doyikFSEhu43705076 = -223187246;    int doyikFSEhu4998313 = -735596843;    int doyikFSEhu52127447 = -46864756;    int doyikFSEhu36464729 = -661979628;    int doyikFSEhu60017682 = -613747257;    int doyikFSEhu14675595 = -474824763;     doyikFSEhu46037761 = doyikFSEhu26558855;     doyikFSEhu26558855 = doyikFSEhu24380518;     doyikFSEhu24380518 = doyikFSEhu30355190;     doyikFSEhu30355190 = doyikFSEhu19110421;     doyikFSEhu19110421 = doyikFSEhu68045490;     doyikFSEhu68045490 = doyikFSEhu49405628;     doyikFSEhu49405628 = doyikFSEhu6763947;     doyikFSEhu6763947 = doyikFSEhu62193601;     doyikFSEhu62193601 = doyikFSEhu66370998;     doyikFSEhu66370998 = doyikFSEhu17390476;     doyikFSEhu17390476 = doyikFSEhu37362965;     doyikFSEhu37362965 = doyikFSEhu82623262;     doyikFSEhu82623262 = doyikFSEhu58083547;     doyikFSEhu58083547 = doyikFSEhu46510710;     doyikFSEhu46510710 = doyikFSEhu51149732;     doyikFSEhu51149732 = doyikFSEhu19674671;     doyikFSEhu19674671 = doyikFSEhu80011752;     doyikFSEhu80011752 = doyikFSEhu83877129;     doyikFSEhu83877129 = doyikFSEhu65831103;     doyikFSEhu65831103 = doyikFSEhu99435898;     doyikFSEhu99435898 = doyikFSEhu25484148;     doyikFSEhu25484148 = doyikFSEhu28123402;     doyikFSEhu28123402 = doyikFSEhu22770978;     doyikFSEhu22770978 = doyikFSEhu953083;     doyikFSEhu953083 = doyikFSEhu96412411;     doyikFSEhu96412411 = doyikFSEhu29284467;     doyikFSEhu29284467 = doyikFSEhu62257108;     doyikFSEhu62257108 = doyikFSEhu54360423;     doyikFSEhu54360423 = doyikFSEhu96006692;     doyikFSEhu96006692 = doyikFSEhu91400358;     doyikFSEhu91400358 = doyikFSEhu63663094;     doyikFSEhu63663094 = doyikFSEhu44669915;     doyikFSEhu44669915 = doyikFSEhu9324974;     doyikFSEhu9324974 = doyikFSEhu20413944;     doyikFSEhu20413944 = doyikFSEhu17029920;     doyikFSEhu17029920 = doyikFSEhu46508733;     doyikFSEhu46508733 = doyikFSEhu36342763;     doyikFSEhu36342763 = doyikFSEhu24347663;     doyikFSEhu24347663 = doyikFSEhu82466099;     doyikFSEhu82466099 = doyikFSEhu92146832;     doyikFSEhu92146832 = doyikFSEhu4530971;     doyikFSEhu4530971 = doyikFSEhu2651638;     doyikFSEhu2651638 = doyikFSEhu34988321;     doyikFSEhu34988321 = doyikFSEhu68402976;     doyikFSEhu68402976 = doyikFSEhu40436253;     doyikFSEhu40436253 = doyikFSEhu73206251;     doyikFSEhu73206251 = doyikFSEhu74436014;     doyikFSEhu74436014 = doyikFSEhu99003356;     doyikFSEhu99003356 = doyikFSEhu42216376;     doyikFSEhu42216376 = doyikFSEhu89480806;     doyikFSEhu89480806 = doyikFSEhu10902107;     doyikFSEhu10902107 = doyikFSEhu79180816;     doyikFSEhu79180816 = doyikFSEhu43331763;     doyikFSEhu43331763 = doyikFSEhu68932808;     doyikFSEhu68932808 = doyikFSEhu20553614;     doyikFSEhu20553614 = doyikFSEhu98435453;     doyikFSEhu98435453 = doyikFSEhu1609540;     doyikFSEhu1609540 = doyikFSEhu29402107;     doyikFSEhu29402107 = doyikFSEhu22698009;     doyikFSEhu22698009 = doyikFSEhu38761023;     doyikFSEhu38761023 = doyikFSEhu87148519;     doyikFSEhu87148519 = doyikFSEhu52403524;     doyikFSEhu52403524 = doyikFSEhu66186909;     doyikFSEhu66186909 = doyikFSEhu74970639;     doyikFSEhu74970639 = doyikFSEhu53727381;     doyikFSEhu53727381 = doyikFSEhu92693049;     doyikFSEhu92693049 = doyikFSEhu73298289;     doyikFSEhu73298289 = doyikFSEhu37669603;     doyikFSEhu37669603 = doyikFSEhu29480790;     doyikFSEhu29480790 = doyikFSEhu4640999;     doyikFSEhu4640999 = doyikFSEhu83331908;     doyikFSEhu83331908 = doyikFSEhu55664089;     doyikFSEhu55664089 = doyikFSEhu1411031;     doyikFSEhu1411031 = doyikFSEhu73684270;     doyikFSEhu73684270 = doyikFSEhu94904927;     doyikFSEhu94904927 = doyikFSEhu22832510;     doyikFSEhu22832510 = doyikFSEhu93135080;     doyikFSEhu93135080 = doyikFSEhu54368002;     doyikFSEhu54368002 = doyikFSEhu60516830;     doyikFSEhu60516830 = doyikFSEhu23206161;     doyikFSEhu23206161 = doyikFSEhu54848453;     doyikFSEhu54848453 = doyikFSEhu63253752;     doyikFSEhu63253752 = doyikFSEhu12144047;     doyikFSEhu12144047 = doyikFSEhu6525886;     doyikFSEhu6525886 = doyikFSEhu80498251;     doyikFSEhu80498251 = doyikFSEhu84482277;     doyikFSEhu84482277 = doyikFSEhu1338152;     doyikFSEhu1338152 = doyikFSEhu40392166;     doyikFSEhu40392166 = doyikFSEhu99860330;     doyikFSEhu99860330 = doyikFSEhu18594467;     doyikFSEhu18594467 = doyikFSEhu44899193;     doyikFSEhu44899193 = doyikFSEhu6940656;     doyikFSEhu6940656 = doyikFSEhu1649655;     doyikFSEhu1649655 = doyikFSEhu43705076;     doyikFSEhu43705076 = doyikFSEhu4998313;     doyikFSEhu4998313 = doyikFSEhu52127447;     doyikFSEhu52127447 = doyikFSEhu36464729;     doyikFSEhu36464729 = doyikFSEhu60017682;     doyikFSEhu60017682 = doyikFSEhu14675595;     doyikFSEhu14675595 = doyikFSEhu46037761;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FaoezeLORf86582047() {     int kBUublZGVo29482066 = -255490444;    int kBUublZGVo43430944 = 99605392;    int kBUublZGVo20789025 = -389094707;    int kBUublZGVo93147441 = -429051612;    int kBUublZGVo25231745 = -393538342;    int kBUublZGVo66549891 = -711433772;    int kBUublZGVo20444946 = -132293117;    int kBUublZGVo21111390 = -131690378;    int kBUublZGVo20723302 = -575336935;    int kBUublZGVo98916649 = -654069430;    int kBUublZGVo66806413 = -115183729;    int kBUublZGVo568364 = -929314146;    int kBUublZGVo741039 = -272004321;    int kBUublZGVo37710439 = -30552306;    int kBUublZGVo7365174 = 387341;    int kBUublZGVo37538894 = -965041057;    int kBUublZGVo13426556 = -490502587;    int kBUublZGVo60863199 = -804450629;    int kBUublZGVo46317271 = -342403616;    int kBUublZGVo55198545 = -490389730;    int kBUublZGVo55076285 = -927832890;    int kBUublZGVo37631466 = 13752105;    int kBUublZGVo31949110 = -677921838;    int kBUublZGVo57477536 = -655923402;    int kBUublZGVo42529041 = -339903197;    int kBUublZGVo91754560 = -223349858;    int kBUublZGVo78739207 = -887780569;    int kBUublZGVo1988734 = -475796102;    int kBUublZGVo90441517 = -227739649;    int kBUublZGVo11840034 = -757304316;    int kBUublZGVo9495673 = -167204912;    int kBUublZGVo26769522 = 34850504;    int kBUublZGVo88208801 = -256580795;    int kBUublZGVo51483484 = -154985692;    int kBUublZGVo99140674 = 42845155;    int kBUublZGVo29419104 = -531743714;    int kBUublZGVo15122912 = -679946718;    int kBUublZGVo76921595 = -777427471;    int kBUublZGVo73342741 = -364642431;    int kBUublZGVo31961234 = -748446661;    int kBUublZGVo25795757 = -293686402;    int kBUublZGVo55018757 = -400070919;    int kBUublZGVo26218708 = -102459990;    int kBUublZGVo26893262 = -26175635;    int kBUublZGVo60347473 = -704625460;    int kBUublZGVo80042249 = -873375895;    int kBUublZGVo49221681 = -58070023;    int kBUublZGVo94106609 = -580112558;    int kBUublZGVo29850469 = -461866208;    int kBUublZGVo77548860 = -549302888;    int kBUublZGVo62738254 = -328762422;    int kBUublZGVo66402901 = -927711646;    int kBUublZGVo93293275 = -366906553;    int kBUublZGVo68008522 = 2584851;    int kBUublZGVo31505762 = -848750725;    int kBUublZGVo91850600 = -169242549;    int kBUublZGVo11481835 = -222472770;    int kBUublZGVo63311488 = -733171306;    int kBUublZGVo50618401 = 10851585;    int kBUublZGVo33477184 = -70188484;    int kBUublZGVo87810683 = -823653204;    int kBUublZGVo18456212 = -656497016;    int kBUublZGVo30669873 = -903950729;    int kBUublZGVo8883269 = -818032619;    int kBUublZGVo89420977 = -386864518;    int kBUublZGVo40036892 = -50034234;    int kBUublZGVo12359562 = -572733351;    int kBUublZGVo49257554 = -17018630;    int kBUublZGVo38569765 = 26602539;    int kBUublZGVo77946069 = -467868946;    int kBUublZGVo22415982 = -185094340;    int kBUublZGVo36504961 = -713075117;    int kBUublZGVo87520457 = -339808199;    int kBUublZGVo14356037 = -593956956;    int kBUublZGVo29402789 = -96703328;    int kBUublZGVo57529 = -427761972;    int kBUublZGVo11412758 = -883787906;    int kBUublZGVo5055848 = -551746204;    int kBUublZGVo97130063 = -951297942;    int kBUublZGVo62486792 = -466527303;    int kBUublZGVo42532880 = -65279835;    int kBUublZGVo84632598 = -207668011;    int kBUublZGVo72138265 = 86070106;    int kBUublZGVo12892657 = -678436762;    int kBUublZGVo49101779 = -328541895;    int kBUublZGVo43092771 = -239493266;    int kBUublZGVo33476246 = -598242944;    int kBUublZGVo20200280 = -159165647;    int kBUublZGVo19977722 = -306234967;    int kBUublZGVo7290074 = -787912297;    int kBUublZGVo17937269 = -209270944;    int kBUublZGVo51811423 = -946775413;    int kBUublZGVo26303195 = -688279056;    int kBUublZGVo39865558 = -194453948;    int kBUublZGVo44150551 = -924793458;    int kBUublZGVo7339545 = -637189387;    int kBUublZGVo24348884 = -496120191;    int kBUublZGVo17335440 = -284427372;    int kBUublZGVo37472284 = -639311117;    int kBUublZGVo20310581 = -255490444;     kBUublZGVo29482066 = kBUublZGVo43430944;     kBUublZGVo43430944 = kBUublZGVo20789025;     kBUublZGVo20789025 = kBUublZGVo93147441;     kBUublZGVo93147441 = kBUublZGVo25231745;     kBUublZGVo25231745 = kBUublZGVo66549891;     kBUublZGVo66549891 = kBUublZGVo20444946;     kBUublZGVo20444946 = kBUublZGVo21111390;     kBUublZGVo21111390 = kBUublZGVo20723302;     kBUublZGVo20723302 = kBUublZGVo98916649;     kBUublZGVo98916649 = kBUublZGVo66806413;     kBUublZGVo66806413 = kBUublZGVo568364;     kBUublZGVo568364 = kBUublZGVo741039;     kBUublZGVo741039 = kBUublZGVo37710439;     kBUublZGVo37710439 = kBUublZGVo7365174;     kBUublZGVo7365174 = kBUublZGVo37538894;     kBUublZGVo37538894 = kBUublZGVo13426556;     kBUublZGVo13426556 = kBUublZGVo60863199;     kBUublZGVo60863199 = kBUublZGVo46317271;     kBUublZGVo46317271 = kBUublZGVo55198545;     kBUublZGVo55198545 = kBUublZGVo55076285;     kBUublZGVo55076285 = kBUublZGVo37631466;     kBUublZGVo37631466 = kBUublZGVo31949110;     kBUublZGVo31949110 = kBUublZGVo57477536;     kBUublZGVo57477536 = kBUublZGVo42529041;     kBUublZGVo42529041 = kBUublZGVo91754560;     kBUublZGVo91754560 = kBUublZGVo78739207;     kBUublZGVo78739207 = kBUublZGVo1988734;     kBUublZGVo1988734 = kBUublZGVo90441517;     kBUublZGVo90441517 = kBUublZGVo11840034;     kBUublZGVo11840034 = kBUublZGVo9495673;     kBUublZGVo9495673 = kBUublZGVo26769522;     kBUublZGVo26769522 = kBUublZGVo88208801;     kBUublZGVo88208801 = kBUublZGVo51483484;     kBUublZGVo51483484 = kBUublZGVo99140674;     kBUublZGVo99140674 = kBUublZGVo29419104;     kBUublZGVo29419104 = kBUublZGVo15122912;     kBUublZGVo15122912 = kBUublZGVo76921595;     kBUublZGVo76921595 = kBUublZGVo73342741;     kBUublZGVo73342741 = kBUublZGVo31961234;     kBUublZGVo31961234 = kBUublZGVo25795757;     kBUublZGVo25795757 = kBUublZGVo55018757;     kBUublZGVo55018757 = kBUublZGVo26218708;     kBUublZGVo26218708 = kBUublZGVo26893262;     kBUublZGVo26893262 = kBUublZGVo60347473;     kBUublZGVo60347473 = kBUublZGVo80042249;     kBUublZGVo80042249 = kBUublZGVo49221681;     kBUublZGVo49221681 = kBUublZGVo94106609;     kBUublZGVo94106609 = kBUublZGVo29850469;     kBUublZGVo29850469 = kBUublZGVo77548860;     kBUublZGVo77548860 = kBUublZGVo62738254;     kBUublZGVo62738254 = kBUublZGVo66402901;     kBUublZGVo66402901 = kBUublZGVo93293275;     kBUublZGVo93293275 = kBUublZGVo68008522;     kBUublZGVo68008522 = kBUublZGVo31505762;     kBUublZGVo31505762 = kBUublZGVo91850600;     kBUublZGVo91850600 = kBUublZGVo11481835;     kBUublZGVo11481835 = kBUublZGVo63311488;     kBUublZGVo63311488 = kBUublZGVo50618401;     kBUublZGVo50618401 = kBUublZGVo33477184;     kBUublZGVo33477184 = kBUublZGVo87810683;     kBUublZGVo87810683 = kBUublZGVo18456212;     kBUublZGVo18456212 = kBUublZGVo30669873;     kBUublZGVo30669873 = kBUublZGVo8883269;     kBUublZGVo8883269 = kBUublZGVo89420977;     kBUublZGVo89420977 = kBUublZGVo40036892;     kBUublZGVo40036892 = kBUublZGVo12359562;     kBUublZGVo12359562 = kBUublZGVo49257554;     kBUublZGVo49257554 = kBUublZGVo38569765;     kBUublZGVo38569765 = kBUublZGVo77946069;     kBUublZGVo77946069 = kBUublZGVo22415982;     kBUublZGVo22415982 = kBUublZGVo36504961;     kBUublZGVo36504961 = kBUublZGVo87520457;     kBUublZGVo87520457 = kBUublZGVo14356037;     kBUublZGVo14356037 = kBUublZGVo29402789;     kBUublZGVo29402789 = kBUublZGVo57529;     kBUublZGVo57529 = kBUublZGVo11412758;     kBUublZGVo11412758 = kBUublZGVo5055848;     kBUublZGVo5055848 = kBUublZGVo97130063;     kBUublZGVo97130063 = kBUublZGVo62486792;     kBUublZGVo62486792 = kBUublZGVo42532880;     kBUublZGVo42532880 = kBUublZGVo84632598;     kBUublZGVo84632598 = kBUublZGVo72138265;     kBUublZGVo72138265 = kBUublZGVo12892657;     kBUublZGVo12892657 = kBUublZGVo49101779;     kBUublZGVo49101779 = kBUublZGVo43092771;     kBUublZGVo43092771 = kBUublZGVo33476246;     kBUublZGVo33476246 = kBUublZGVo20200280;     kBUublZGVo20200280 = kBUublZGVo19977722;     kBUublZGVo19977722 = kBUublZGVo7290074;     kBUublZGVo7290074 = kBUublZGVo17937269;     kBUublZGVo17937269 = kBUublZGVo51811423;     kBUublZGVo51811423 = kBUublZGVo26303195;     kBUublZGVo26303195 = kBUublZGVo39865558;     kBUublZGVo39865558 = kBUublZGVo44150551;     kBUublZGVo44150551 = kBUublZGVo7339545;     kBUublZGVo7339545 = kBUublZGVo24348884;     kBUublZGVo24348884 = kBUublZGVo17335440;     kBUublZGVo17335440 = kBUublZGVo37472284;     kBUublZGVo37472284 = kBUublZGVo20310581;     kBUublZGVo20310581 = kBUublZGVo29482066;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SaHIPxdubu57037920() {     int EfXLYXDUPr42268233 = -390531136;    int EfXLYXDUPr30201832 = -70728622;    int EfXLYXDUPr38541824 = -797454088;    int EfXLYXDUPr1088267 = -253253889;    int EfXLYXDUPr18742849 = -885354807;    int EfXLYXDUPr9859962 = -488357085;    int EfXLYXDUPr41652704 = -677965188;    int EfXLYXDUPr47830502 = -576208634;    int EfXLYXDUPr16128112 = -537095027;    int EfXLYXDUPr5135355 = -458157063;    int EfXLYXDUPr94293795 = -229869745;    int EfXLYXDUPr32518869 = -336646961;    int EfXLYXDUPr70884341 = -175786978;    int EfXLYXDUPr87334517 = -362967504;    int EfXLYXDUPr30671727 = 93401510;    int EfXLYXDUPr65963962 = -415932446;    int EfXLYXDUPr69178373 = -154277054;    int EfXLYXDUPr44421684 = -359660932;    int EfXLYXDUPr47477902 = -251872812;    int EfXLYXDUPr57853042 = 10366131;    int EfXLYXDUPr35250342 = -319149155;    int EfXLYXDUPr66679284 = -156106243;    int EfXLYXDUPr70709405 = -608925733;    int EfXLYXDUPr1984666 = 83731586;    int EfXLYXDUPr48423300 = -550345122;    int EfXLYXDUPr73073206 = 86218943;    int EfXLYXDUPr90154743 = -421486849;    int EfXLYXDUPr69341659 = -649594206;    int EfXLYXDUPr7058571 = -441956889;    int EfXLYXDUPr64591148 = -387190471;    int EfXLYXDUPr53931668 = -443621662;    int EfXLYXDUPr30088345 = 56130825;    int EfXLYXDUPr34254751 = -132435579;    int EfXLYXDUPr28559857 = -59739910;    int EfXLYXDUPr82201206 = -567186726;    int EfXLYXDUPr24643219 = -320306818;    int EfXLYXDUPr51886972 = -469856068;    int EfXLYXDUPr34625618 = -697047562;    int EfXLYXDUPr79118127 = -206493008;    int EfXLYXDUPr58225964 = -592863507;    int EfXLYXDUPr87227065 = -669160396;    int EfXLYXDUPr62721213 = -298382253;    int EfXLYXDUPr61102177 = -125893435;    int EfXLYXDUPr71521799 = -367347878;    int EfXLYXDUPr40355963 = -597395420;    int EfXLYXDUPr53670378 = -946694335;    int EfXLYXDUPr26803701 = -555585075;    int EfXLYXDUPr23546576 = -735465338;    int EfXLYXDUPr73914518 = -625468182;    int EfXLYXDUPr24499281 = -60339351;    int EfXLYXDUPr81065748 = -581646730;    int EfXLYXDUPr94579448 = -298896984;    int EfXLYXDUPr27778426 = -542909835;    int EfXLYXDUPr12708248 = -686866891;    int EfXLYXDUPr21787579 = -843195211;    int EfXLYXDUPr75588948 = -134424893;    int EfXLYXDUPr59492427 = -461802889;    int EfXLYXDUPr36557159 = -781185675;    int EfXLYXDUPr52664967 = -702908768;    int EfXLYXDUPr45669643 = -871573750;    int EfXLYXDUPr19705218 = 33129763;    int EfXLYXDUPr72311045 = 71629018;    int EfXLYXDUPr40771932 = -34251746;    int EfXLYXDUPr51536963 = -49904557;    int EfXLYXDUPr51203686 = 85464599;    int EfXLYXDUPr64205451 = -186000570;    int EfXLYXDUPr98264117 = -104211383;    int EfXLYXDUPr42324484 = -16047069;    int EfXLYXDUPr5133312 = -795780779;    int EfXLYXDUPr6028508 = -586291672;    int EfXLYXDUPr14076990 = -946076378;    int EfXLYXDUPr34552756 = -457229492;    int EfXLYXDUPr65303556 = -53167924;    int EfXLYXDUPr89251937 = -659009305;    int EfXLYXDUPr70625977 = -320473474;    int EfXLYXDUPr72529129 = 79233098;    int EfXLYXDUPr5577108 = 69787191;    int EfXLYXDUPr99187605 = -141577856;    int EfXLYXDUPr61628702 = -318872994;    int EfXLYXDUPr94752921 = -603650787;    int EfXLYXDUPr46269506 = -358195982;    int EfXLYXDUPr66608167 = -686021512;    int EfXLYXDUPr95427140 = 75873976;    int EfXLYXDUPr82559289 = -281617539;    int EfXLYXDUPr83525400 = -805543741;    int EfXLYXDUPr59352220 = -44724678;    int EfXLYXDUPr2309920 = -400959340;    int EfXLYXDUPr21546504 = -445568689;    int EfXLYXDUPr6772279 = -216544699;    int EfXLYXDUPr6612258 = -332761833;    int EfXLYXDUPr65150791 = -858503929;    int EfXLYXDUPr15329814 = -688670394;    int EfXLYXDUPr81960651 = -994138795;    int EfXLYXDUPr33448485 = -334919259;    int EfXLYXDUPr38520746 = -525993270;    int EfXLYXDUPr14916020 = -640789415;    int EfXLYXDUPr21949281 = -164130508;    int EfXLYXDUPr9565214 = 24011121;    int EfXLYXDUPr20318113 = -352812477;    int EfXLYXDUPr76150512 = -390531136;     EfXLYXDUPr42268233 = EfXLYXDUPr30201832;     EfXLYXDUPr30201832 = EfXLYXDUPr38541824;     EfXLYXDUPr38541824 = EfXLYXDUPr1088267;     EfXLYXDUPr1088267 = EfXLYXDUPr18742849;     EfXLYXDUPr18742849 = EfXLYXDUPr9859962;     EfXLYXDUPr9859962 = EfXLYXDUPr41652704;     EfXLYXDUPr41652704 = EfXLYXDUPr47830502;     EfXLYXDUPr47830502 = EfXLYXDUPr16128112;     EfXLYXDUPr16128112 = EfXLYXDUPr5135355;     EfXLYXDUPr5135355 = EfXLYXDUPr94293795;     EfXLYXDUPr94293795 = EfXLYXDUPr32518869;     EfXLYXDUPr32518869 = EfXLYXDUPr70884341;     EfXLYXDUPr70884341 = EfXLYXDUPr87334517;     EfXLYXDUPr87334517 = EfXLYXDUPr30671727;     EfXLYXDUPr30671727 = EfXLYXDUPr65963962;     EfXLYXDUPr65963962 = EfXLYXDUPr69178373;     EfXLYXDUPr69178373 = EfXLYXDUPr44421684;     EfXLYXDUPr44421684 = EfXLYXDUPr47477902;     EfXLYXDUPr47477902 = EfXLYXDUPr57853042;     EfXLYXDUPr57853042 = EfXLYXDUPr35250342;     EfXLYXDUPr35250342 = EfXLYXDUPr66679284;     EfXLYXDUPr66679284 = EfXLYXDUPr70709405;     EfXLYXDUPr70709405 = EfXLYXDUPr1984666;     EfXLYXDUPr1984666 = EfXLYXDUPr48423300;     EfXLYXDUPr48423300 = EfXLYXDUPr73073206;     EfXLYXDUPr73073206 = EfXLYXDUPr90154743;     EfXLYXDUPr90154743 = EfXLYXDUPr69341659;     EfXLYXDUPr69341659 = EfXLYXDUPr7058571;     EfXLYXDUPr7058571 = EfXLYXDUPr64591148;     EfXLYXDUPr64591148 = EfXLYXDUPr53931668;     EfXLYXDUPr53931668 = EfXLYXDUPr30088345;     EfXLYXDUPr30088345 = EfXLYXDUPr34254751;     EfXLYXDUPr34254751 = EfXLYXDUPr28559857;     EfXLYXDUPr28559857 = EfXLYXDUPr82201206;     EfXLYXDUPr82201206 = EfXLYXDUPr24643219;     EfXLYXDUPr24643219 = EfXLYXDUPr51886972;     EfXLYXDUPr51886972 = EfXLYXDUPr34625618;     EfXLYXDUPr34625618 = EfXLYXDUPr79118127;     EfXLYXDUPr79118127 = EfXLYXDUPr58225964;     EfXLYXDUPr58225964 = EfXLYXDUPr87227065;     EfXLYXDUPr87227065 = EfXLYXDUPr62721213;     EfXLYXDUPr62721213 = EfXLYXDUPr61102177;     EfXLYXDUPr61102177 = EfXLYXDUPr71521799;     EfXLYXDUPr71521799 = EfXLYXDUPr40355963;     EfXLYXDUPr40355963 = EfXLYXDUPr53670378;     EfXLYXDUPr53670378 = EfXLYXDUPr26803701;     EfXLYXDUPr26803701 = EfXLYXDUPr23546576;     EfXLYXDUPr23546576 = EfXLYXDUPr73914518;     EfXLYXDUPr73914518 = EfXLYXDUPr24499281;     EfXLYXDUPr24499281 = EfXLYXDUPr81065748;     EfXLYXDUPr81065748 = EfXLYXDUPr94579448;     EfXLYXDUPr94579448 = EfXLYXDUPr27778426;     EfXLYXDUPr27778426 = EfXLYXDUPr12708248;     EfXLYXDUPr12708248 = EfXLYXDUPr21787579;     EfXLYXDUPr21787579 = EfXLYXDUPr75588948;     EfXLYXDUPr75588948 = EfXLYXDUPr59492427;     EfXLYXDUPr59492427 = EfXLYXDUPr36557159;     EfXLYXDUPr36557159 = EfXLYXDUPr52664967;     EfXLYXDUPr52664967 = EfXLYXDUPr45669643;     EfXLYXDUPr45669643 = EfXLYXDUPr19705218;     EfXLYXDUPr19705218 = EfXLYXDUPr72311045;     EfXLYXDUPr72311045 = EfXLYXDUPr40771932;     EfXLYXDUPr40771932 = EfXLYXDUPr51536963;     EfXLYXDUPr51536963 = EfXLYXDUPr51203686;     EfXLYXDUPr51203686 = EfXLYXDUPr64205451;     EfXLYXDUPr64205451 = EfXLYXDUPr98264117;     EfXLYXDUPr98264117 = EfXLYXDUPr42324484;     EfXLYXDUPr42324484 = EfXLYXDUPr5133312;     EfXLYXDUPr5133312 = EfXLYXDUPr6028508;     EfXLYXDUPr6028508 = EfXLYXDUPr14076990;     EfXLYXDUPr14076990 = EfXLYXDUPr34552756;     EfXLYXDUPr34552756 = EfXLYXDUPr65303556;     EfXLYXDUPr65303556 = EfXLYXDUPr89251937;     EfXLYXDUPr89251937 = EfXLYXDUPr70625977;     EfXLYXDUPr70625977 = EfXLYXDUPr72529129;     EfXLYXDUPr72529129 = EfXLYXDUPr5577108;     EfXLYXDUPr5577108 = EfXLYXDUPr99187605;     EfXLYXDUPr99187605 = EfXLYXDUPr61628702;     EfXLYXDUPr61628702 = EfXLYXDUPr94752921;     EfXLYXDUPr94752921 = EfXLYXDUPr46269506;     EfXLYXDUPr46269506 = EfXLYXDUPr66608167;     EfXLYXDUPr66608167 = EfXLYXDUPr95427140;     EfXLYXDUPr95427140 = EfXLYXDUPr82559289;     EfXLYXDUPr82559289 = EfXLYXDUPr83525400;     EfXLYXDUPr83525400 = EfXLYXDUPr59352220;     EfXLYXDUPr59352220 = EfXLYXDUPr2309920;     EfXLYXDUPr2309920 = EfXLYXDUPr21546504;     EfXLYXDUPr21546504 = EfXLYXDUPr6772279;     EfXLYXDUPr6772279 = EfXLYXDUPr6612258;     EfXLYXDUPr6612258 = EfXLYXDUPr65150791;     EfXLYXDUPr65150791 = EfXLYXDUPr15329814;     EfXLYXDUPr15329814 = EfXLYXDUPr81960651;     EfXLYXDUPr81960651 = EfXLYXDUPr33448485;     EfXLYXDUPr33448485 = EfXLYXDUPr38520746;     EfXLYXDUPr38520746 = EfXLYXDUPr14916020;     EfXLYXDUPr14916020 = EfXLYXDUPr21949281;     EfXLYXDUPr21949281 = EfXLYXDUPr9565214;     EfXLYXDUPr9565214 = EfXLYXDUPr20318113;     EfXLYXDUPr20318113 = EfXLYXDUPr76150512;     EfXLYXDUPr76150512 = EfXLYXDUPr42268233;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sDoiuWNxZv75251262() {     int kGfrrKoPyZ25712539 = -171196817;    int kGfrrKoPyZ47073921 = -70110809;    int kGfrrKoPyZ34950331 = -590979316;    int kGfrrKoPyZ63880518 = -447053724;    int kGfrrKoPyZ24864173 = -692222298;    int kGfrrKoPyZ8364363 = -267428738;    int kGfrrKoPyZ12692022 = -417876501;    int kGfrrKoPyZ62177945 = -563366664;    int kGfrrKoPyZ74657812 = -878554378;    int kGfrrKoPyZ37681007 = -926533175;    int kGfrrKoPyZ43709734 = -419985617;    int kGfrrKoPyZ95724267 = -324265208;    int kGfrrKoPyZ89002117 = 32551781;    int kGfrrKoPyZ66961410 = -151257861;    int kGfrrKoPyZ91526189 = -621472;    int kGfrrKoPyZ52353124 = -204011772;    int kGfrrKoPyZ62930258 = 92377919;    int kGfrrKoPyZ25273131 = -606965808;    int kGfrrKoPyZ9918044 = -250547683;    int kGfrrKoPyZ47220484 = -519219861;    int kGfrrKoPyZ90890729 = 91321814;    int kGfrrKoPyZ78826602 = -147258014;    int kGfrrKoPyZ74535112 = -648814930;    int kGfrrKoPyZ36691224 = 11906190;    int kGfrrKoPyZ89999258 = -9997881;    int kGfrrKoPyZ68415355 = -894290773;    int kGfrrKoPyZ39609484 = -49571191;    int kGfrrKoPyZ9073284 = -719549090;    int kGfrrKoPyZ43139664 = -304462031;    int kGfrrKoPyZ80424490 = -900040323;    int kGfrrKoPyZ72026982 = -81378863;    int kGfrrKoPyZ93194772 = -959064254;    int kGfrrKoPyZ77793637 = -853307757;    int kGfrrKoPyZ70718367 = -751342080;    int kGfrrKoPyZ60927937 = -764851461;    int kGfrrKoPyZ37032403 = -108330724;    int kGfrrKoPyZ20501151 = -348705248;    int kGfrrKoPyZ75204449 = -405518082;    int kGfrrKoPyZ28113206 = -391390806;    int kGfrrKoPyZ7721099 = -345457029;    int kGfrrKoPyZ20875990 = -240709369;    int kGfrrKoPyZ13208999 = -872290575;    int kGfrrKoPyZ84669247 = -676950678;    int kGfrrKoPyZ63426739 = -123530649;    int kGfrrKoPyZ32300460 = -325552373;    int kGfrrKoPyZ93276374 = -966305258;    int kGfrrKoPyZ2819131 = -687889305;    int kGfrrKoPyZ43217171 = -734187056;    int kGfrrKoPyZ4761631 = -111070265;    int kGfrrKoPyZ59831764 = -267359806;    int kGfrrKoPyZ54323196 = -802548153;    int kGfrrKoPyZ50080243 = -357156296;    int kGfrrKoPyZ41890885 = -754657152;    int kGfrrKoPyZ37385006 = -405280278;    int kGfrrKoPyZ84360532 = -91681334;    int kGfrrKoPyZ46885936 = 76061197;    int kGfrrKoPyZ72538809 = -421295880;    int kGfrrKoPyZ98259107 = -502885507;    int kGfrrKoPyZ73881260 = -337055844;    int kGfrrKoPyZ56448818 = -797931525;    int kGfrrKoPyZ68754878 = -117857548;    int kGfrrKoPyZ3618738 = -698327411;    int kGfrrKoPyZ19038281 = -158904633;    int kGfrrKoPyZ94233322 = -978514056;    int kGfrrKoPyZ65654024 = -745154312;    int kGfrrKoPyZ50514962 = -460921363;    int kGfrrKoPyZ17930631 = -470957452;    int kGfrrKoPyZ18283750 = -216106140;    int kGfrrKoPyZ6033473 = -386406401;    int kGfrrKoPyZ54493787 = -892290748;    int kGfrrKoPyZ31851973 = -855306524;    int kGfrrKoPyZ87725808 = -502103999;    int kGfrrKoPyZ97159924 = -115575002;    int kGfrrKoPyZ2196945 = -905090654;    int kGfrrKoPyZ26344495 = -178510493;    int kGfrrKoPyZ77681730 = -36387611;    int kGfrrKoPyZ94157355 = -470307337;    int kGfrrKoPyZ11108374 = -425284282;    int kGfrrKoPyZ4390764 = -662541437;    int kGfrrKoPyZ96722883 = -43692623;    int kGfrrKoPyZ65596224 = -106401468;    int kGfrrKoPyZ96392312 = -315384136;    int kGfrrKoPyZ4311654 = -508478826;    int kGfrrKoPyZ83307899 = 62897774;    int kGfrrKoPyZ26101294 = 2507830;    int kGfrrKoPyZ21946739 = -724222568;    int kGfrrKoPyZ51303887 = -104407103;    int kGfrrKoPyZ40408631 = -348027479;    int kGfrrKoPyZ86357834 = -559660746;    int kGfrrKoPyZ14042001 = -740912658;    int kGfrrKoPyZ64493594 = -687034845;    int kGfrrKoPyZ22242044 = -845819742;    int kGfrrKoPyZ1323190 = 31537762;    int kGfrrKoPyZ71664387 = -593459281;    int kGfrrKoPyZ38966220 = -127599482;    int kGfrrKoPyZ17257252 = -542381958;    int kGfrrKoPyZ94170717 = -613385942;    int kGfrrKoPyZ90435924 = -698436622;    int kGfrrKoPyZ97772714 = -378376337;    int kGfrrKoPyZ81785498 = -171196817;     kGfrrKoPyZ25712539 = kGfrrKoPyZ47073921;     kGfrrKoPyZ47073921 = kGfrrKoPyZ34950331;     kGfrrKoPyZ34950331 = kGfrrKoPyZ63880518;     kGfrrKoPyZ63880518 = kGfrrKoPyZ24864173;     kGfrrKoPyZ24864173 = kGfrrKoPyZ8364363;     kGfrrKoPyZ8364363 = kGfrrKoPyZ12692022;     kGfrrKoPyZ12692022 = kGfrrKoPyZ62177945;     kGfrrKoPyZ62177945 = kGfrrKoPyZ74657812;     kGfrrKoPyZ74657812 = kGfrrKoPyZ37681007;     kGfrrKoPyZ37681007 = kGfrrKoPyZ43709734;     kGfrrKoPyZ43709734 = kGfrrKoPyZ95724267;     kGfrrKoPyZ95724267 = kGfrrKoPyZ89002117;     kGfrrKoPyZ89002117 = kGfrrKoPyZ66961410;     kGfrrKoPyZ66961410 = kGfrrKoPyZ91526189;     kGfrrKoPyZ91526189 = kGfrrKoPyZ52353124;     kGfrrKoPyZ52353124 = kGfrrKoPyZ62930258;     kGfrrKoPyZ62930258 = kGfrrKoPyZ25273131;     kGfrrKoPyZ25273131 = kGfrrKoPyZ9918044;     kGfrrKoPyZ9918044 = kGfrrKoPyZ47220484;     kGfrrKoPyZ47220484 = kGfrrKoPyZ90890729;     kGfrrKoPyZ90890729 = kGfrrKoPyZ78826602;     kGfrrKoPyZ78826602 = kGfrrKoPyZ74535112;     kGfrrKoPyZ74535112 = kGfrrKoPyZ36691224;     kGfrrKoPyZ36691224 = kGfrrKoPyZ89999258;     kGfrrKoPyZ89999258 = kGfrrKoPyZ68415355;     kGfrrKoPyZ68415355 = kGfrrKoPyZ39609484;     kGfrrKoPyZ39609484 = kGfrrKoPyZ9073284;     kGfrrKoPyZ9073284 = kGfrrKoPyZ43139664;     kGfrrKoPyZ43139664 = kGfrrKoPyZ80424490;     kGfrrKoPyZ80424490 = kGfrrKoPyZ72026982;     kGfrrKoPyZ72026982 = kGfrrKoPyZ93194772;     kGfrrKoPyZ93194772 = kGfrrKoPyZ77793637;     kGfrrKoPyZ77793637 = kGfrrKoPyZ70718367;     kGfrrKoPyZ70718367 = kGfrrKoPyZ60927937;     kGfrrKoPyZ60927937 = kGfrrKoPyZ37032403;     kGfrrKoPyZ37032403 = kGfrrKoPyZ20501151;     kGfrrKoPyZ20501151 = kGfrrKoPyZ75204449;     kGfrrKoPyZ75204449 = kGfrrKoPyZ28113206;     kGfrrKoPyZ28113206 = kGfrrKoPyZ7721099;     kGfrrKoPyZ7721099 = kGfrrKoPyZ20875990;     kGfrrKoPyZ20875990 = kGfrrKoPyZ13208999;     kGfrrKoPyZ13208999 = kGfrrKoPyZ84669247;     kGfrrKoPyZ84669247 = kGfrrKoPyZ63426739;     kGfrrKoPyZ63426739 = kGfrrKoPyZ32300460;     kGfrrKoPyZ32300460 = kGfrrKoPyZ93276374;     kGfrrKoPyZ93276374 = kGfrrKoPyZ2819131;     kGfrrKoPyZ2819131 = kGfrrKoPyZ43217171;     kGfrrKoPyZ43217171 = kGfrrKoPyZ4761631;     kGfrrKoPyZ4761631 = kGfrrKoPyZ59831764;     kGfrrKoPyZ59831764 = kGfrrKoPyZ54323196;     kGfrrKoPyZ54323196 = kGfrrKoPyZ50080243;     kGfrrKoPyZ50080243 = kGfrrKoPyZ41890885;     kGfrrKoPyZ41890885 = kGfrrKoPyZ37385006;     kGfrrKoPyZ37385006 = kGfrrKoPyZ84360532;     kGfrrKoPyZ84360532 = kGfrrKoPyZ46885936;     kGfrrKoPyZ46885936 = kGfrrKoPyZ72538809;     kGfrrKoPyZ72538809 = kGfrrKoPyZ98259107;     kGfrrKoPyZ98259107 = kGfrrKoPyZ73881260;     kGfrrKoPyZ73881260 = kGfrrKoPyZ56448818;     kGfrrKoPyZ56448818 = kGfrrKoPyZ68754878;     kGfrrKoPyZ68754878 = kGfrrKoPyZ3618738;     kGfrrKoPyZ3618738 = kGfrrKoPyZ19038281;     kGfrrKoPyZ19038281 = kGfrrKoPyZ94233322;     kGfrrKoPyZ94233322 = kGfrrKoPyZ65654024;     kGfrrKoPyZ65654024 = kGfrrKoPyZ50514962;     kGfrrKoPyZ50514962 = kGfrrKoPyZ17930631;     kGfrrKoPyZ17930631 = kGfrrKoPyZ18283750;     kGfrrKoPyZ18283750 = kGfrrKoPyZ6033473;     kGfrrKoPyZ6033473 = kGfrrKoPyZ54493787;     kGfrrKoPyZ54493787 = kGfrrKoPyZ31851973;     kGfrrKoPyZ31851973 = kGfrrKoPyZ87725808;     kGfrrKoPyZ87725808 = kGfrrKoPyZ97159924;     kGfrrKoPyZ97159924 = kGfrrKoPyZ2196945;     kGfrrKoPyZ2196945 = kGfrrKoPyZ26344495;     kGfrrKoPyZ26344495 = kGfrrKoPyZ77681730;     kGfrrKoPyZ77681730 = kGfrrKoPyZ94157355;     kGfrrKoPyZ94157355 = kGfrrKoPyZ11108374;     kGfrrKoPyZ11108374 = kGfrrKoPyZ4390764;     kGfrrKoPyZ4390764 = kGfrrKoPyZ96722883;     kGfrrKoPyZ96722883 = kGfrrKoPyZ65596224;     kGfrrKoPyZ65596224 = kGfrrKoPyZ96392312;     kGfrrKoPyZ96392312 = kGfrrKoPyZ4311654;     kGfrrKoPyZ4311654 = kGfrrKoPyZ83307899;     kGfrrKoPyZ83307899 = kGfrrKoPyZ26101294;     kGfrrKoPyZ26101294 = kGfrrKoPyZ21946739;     kGfrrKoPyZ21946739 = kGfrrKoPyZ51303887;     kGfrrKoPyZ51303887 = kGfrrKoPyZ40408631;     kGfrrKoPyZ40408631 = kGfrrKoPyZ86357834;     kGfrrKoPyZ86357834 = kGfrrKoPyZ14042001;     kGfrrKoPyZ14042001 = kGfrrKoPyZ64493594;     kGfrrKoPyZ64493594 = kGfrrKoPyZ22242044;     kGfrrKoPyZ22242044 = kGfrrKoPyZ1323190;     kGfrrKoPyZ1323190 = kGfrrKoPyZ71664387;     kGfrrKoPyZ71664387 = kGfrrKoPyZ38966220;     kGfrrKoPyZ38966220 = kGfrrKoPyZ17257252;     kGfrrKoPyZ17257252 = kGfrrKoPyZ94170717;     kGfrrKoPyZ94170717 = kGfrrKoPyZ90435924;     kGfrrKoPyZ90435924 = kGfrrKoPyZ97772714;     kGfrrKoPyZ97772714 = kGfrrKoPyZ81785498;     kGfrrKoPyZ81785498 = kGfrrKoPyZ25712539;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void luAsfGSosP45707135() {     int TBzlWfvEjP38498706 = -306237508;    int TBzlWfvEjP33844810 = -240444823;    int TBzlWfvEjP52703131 = -999338698;    int TBzlWfvEjP71821344 = -271256001;    int TBzlWfvEjP18375278 = -84038763;    int TBzlWfvEjP51674433 = -44352052;    int TBzlWfvEjP33899780 = -963548571;    int TBzlWfvEjP88897057 = 92115080;    int TBzlWfvEjP70062623 = -840312470;    int TBzlWfvEjP43899712 = -730620808;    int TBzlWfvEjP71197116 = -534671633;    int TBzlWfvEjP27674773 = -831598024;    int TBzlWfvEjP59145420 = -971230876;    int TBzlWfvEjP16585489 = -483673059;    int TBzlWfvEjP14832743 = 92392697;    int TBzlWfvEjP80778192 = -754903161;    int TBzlWfvEjP18682076 = -671396548;    int TBzlWfvEjP8831616 = -162176110;    int TBzlWfvEjP11078675 = -160016878;    int TBzlWfvEjP49874981 = -18464000;    int TBzlWfvEjP71064786 = -399994450;    int TBzlWfvEjP7874422 = -317116362;    int TBzlWfvEjP13295408 = -579818825;    int TBzlWfvEjP81198353 = -348438822;    int TBzlWfvEjP95893517 = -220439805;    int TBzlWfvEjP49734000 = -584721972;    int TBzlWfvEjP51025020 = -683277471;    int TBzlWfvEjP76426209 = -893347195;    int TBzlWfvEjP59756717 = -518679271;    int TBzlWfvEjP33175605 = -529926477;    int TBzlWfvEjP16462979 = -357795613;    int TBzlWfvEjP96513595 = -937783933;    int TBzlWfvEjP23839587 = -729162541;    int TBzlWfvEjP47794740 = -656096298;    int TBzlWfvEjP43988469 = -274883341;    int TBzlWfvEjP32256518 = -996893828;    int TBzlWfvEjP57265211 = -138614599;    int TBzlWfvEjP32908473 = -325138174;    int TBzlWfvEjP33888592 = -233241383;    int TBzlWfvEjP33985829 = -189873875;    int TBzlWfvEjP82307297 = -616183362;    int TBzlWfvEjP20911455 = -770601909;    int TBzlWfvEjP19552717 = -700384123;    int TBzlWfvEjP8055277 = -464702891;    int TBzlWfvEjP12308951 = -218322334;    int TBzlWfvEjP66904503 = 60376302;    int TBzlWfvEjP80401150 = -85404357;    int TBzlWfvEjP72657138 = -889539836;    int TBzlWfvEjP48825679 = -274672240;    int TBzlWfvEjP6782185 = -878396269;    int TBzlWfvEjP72650689 = 44567539;    int TBzlWfvEjP78256790 = -828341634;    int TBzlWfvEjP76376034 = -930660434;    int TBzlWfvEjP82084731 = 5267980;    int TBzlWfvEjP74642349 = -86125821;    int TBzlWfvEjP30624284 = -989121147;    int TBzlWfvEjP20549402 = -660625999;    int TBzlWfvEjP71504778 = -550899876;    int TBzlWfvEjP75927826 = 49183803;    int TBzlWfvEjP68641277 = -499316791;    int TBzlWfvEjP649414 = -361074581;    int TBzlWfvEjP57473571 = 29798623;    int TBzlWfvEjP29140341 = -389205650;    int TBzlWfvEjP36887018 = -210385994;    int TBzlWfvEjP27436734 = -272825195;    int TBzlWfvEjP74683520 = -596887700;    int TBzlWfvEjP3835187 = -2435483;    int TBzlWfvEjP11350680 = -215134578;    int TBzlWfvEjP72597019 = -108789718;    int TBzlWfvEjP82576225 = 89286525;    int TBzlWfvEjP23512982 = -516288563;    int TBzlWfvEjP85773603 = -246258374;    int TBzlWfvEjP74943024 = -928934727;    int TBzlWfvEjP77092845 = -970143004;    int TBzlWfvEjP67567683 = -402280639;    int TBzlWfvEjP50153331 = -629392541;    int TBzlWfvEjP88321705 = -616732240;    int TBzlWfvEjP5240131 = -15115934;    int TBzlWfvEjP68889403 = -30116489;    int TBzlWfvEjP28989014 = -180816107;    int TBzlWfvEjP69332850 = -399317615;    int TBzlWfvEjP78367882 = -793737636;    int TBzlWfvEjP27600530 = -518674956;    int TBzlWfvEjP52974532 = -640283003;    int TBzlWfvEjP60524916 = -474494017;    int TBzlWfvEjP38206188 = -529453980;    int TBzlWfvEjP20137561 = 92876501;    int TBzlWfvEjP41754855 = -634430522;    int TBzlWfvEjP73152391 = -469970478;    int TBzlWfvEjP13364185 = -285762195;    int TBzlWfvEjP11707117 = -236267830;    int TBzlWfvEjP85760433 = -587714723;    int TBzlWfvEjP56980646 = -274321977;    int TBzlWfvEjP65247314 = -733924592;    int TBzlWfvEjP33336416 = -828799295;    int TBzlWfvEjP24833727 = -545981986;    int TBzlWfvEjP91771114 = -281396260;    int TBzlWfvEjP82665699 = -389998129;    int TBzlWfvEjP80618543 = -91877697;    int TBzlWfvEjP37625430 = -306237508;     TBzlWfvEjP38498706 = TBzlWfvEjP33844810;     TBzlWfvEjP33844810 = TBzlWfvEjP52703131;     TBzlWfvEjP52703131 = TBzlWfvEjP71821344;     TBzlWfvEjP71821344 = TBzlWfvEjP18375278;     TBzlWfvEjP18375278 = TBzlWfvEjP51674433;     TBzlWfvEjP51674433 = TBzlWfvEjP33899780;     TBzlWfvEjP33899780 = TBzlWfvEjP88897057;     TBzlWfvEjP88897057 = TBzlWfvEjP70062623;     TBzlWfvEjP70062623 = TBzlWfvEjP43899712;     TBzlWfvEjP43899712 = TBzlWfvEjP71197116;     TBzlWfvEjP71197116 = TBzlWfvEjP27674773;     TBzlWfvEjP27674773 = TBzlWfvEjP59145420;     TBzlWfvEjP59145420 = TBzlWfvEjP16585489;     TBzlWfvEjP16585489 = TBzlWfvEjP14832743;     TBzlWfvEjP14832743 = TBzlWfvEjP80778192;     TBzlWfvEjP80778192 = TBzlWfvEjP18682076;     TBzlWfvEjP18682076 = TBzlWfvEjP8831616;     TBzlWfvEjP8831616 = TBzlWfvEjP11078675;     TBzlWfvEjP11078675 = TBzlWfvEjP49874981;     TBzlWfvEjP49874981 = TBzlWfvEjP71064786;     TBzlWfvEjP71064786 = TBzlWfvEjP7874422;     TBzlWfvEjP7874422 = TBzlWfvEjP13295408;     TBzlWfvEjP13295408 = TBzlWfvEjP81198353;     TBzlWfvEjP81198353 = TBzlWfvEjP95893517;     TBzlWfvEjP95893517 = TBzlWfvEjP49734000;     TBzlWfvEjP49734000 = TBzlWfvEjP51025020;     TBzlWfvEjP51025020 = TBzlWfvEjP76426209;     TBzlWfvEjP76426209 = TBzlWfvEjP59756717;     TBzlWfvEjP59756717 = TBzlWfvEjP33175605;     TBzlWfvEjP33175605 = TBzlWfvEjP16462979;     TBzlWfvEjP16462979 = TBzlWfvEjP96513595;     TBzlWfvEjP96513595 = TBzlWfvEjP23839587;     TBzlWfvEjP23839587 = TBzlWfvEjP47794740;     TBzlWfvEjP47794740 = TBzlWfvEjP43988469;     TBzlWfvEjP43988469 = TBzlWfvEjP32256518;     TBzlWfvEjP32256518 = TBzlWfvEjP57265211;     TBzlWfvEjP57265211 = TBzlWfvEjP32908473;     TBzlWfvEjP32908473 = TBzlWfvEjP33888592;     TBzlWfvEjP33888592 = TBzlWfvEjP33985829;     TBzlWfvEjP33985829 = TBzlWfvEjP82307297;     TBzlWfvEjP82307297 = TBzlWfvEjP20911455;     TBzlWfvEjP20911455 = TBzlWfvEjP19552717;     TBzlWfvEjP19552717 = TBzlWfvEjP8055277;     TBzlWfvEjP8055277 = TBzlWfvEjP12308951;     TBzlWfvEjP12308951 = TBzlWfvEjP66904503;     TBzlWfvEjP66904503 = TBzlWfvEjP80401150;     TBzlWfvEjP80401150 = TBzlWfvEjP72657138;     TBzlWfvEjP72657138 = TBzlWfvEjP48825679;     TBzlWfvEjP48825679 = TBzlWfvEjP6782185;     TBzlWfvEjP6782185 = TBzlWfvEjP72650689;     TBzlWfvEjP72650689 = TBzlWfvEjP78256790;     TBzlWfvEjP78256790 = TBzlWfvEjP76376034;     TBzlWfvEjP76376034 = TBzlWfvEjP82084731;     TBzlWfvEjP82084731 = TBzlWfvEjP74642349;     TBzlWfvEjP74642349 = TBzlWfvEjP30624284;     TBzlWfvEjP30624284 = TBzlWfvEjP20549402;     TBzlWfvEjP20549402 = TBzlWfvEjP71504778;     TBzlWfvEjP71504778 = TBzlWfvEjP75927826;     TBzlWfvEjP75927826 = TBzlWfvEjP68641277;     TBzlWfvEjP68641277 = TBzlWfvEjP649414;     TBzlWfvEjP649414 = TBzlWfvEjP57473571;     TBzlWfvEjP57473571 = TBzlWfvEjP29140341;     TBzlWfvEjP29140341 = TBzlWfvEjP36887018;     TBzlWfvEjP36887018 = TBzlWfvEjP27436734;     TBzlWfvEjP27436734 = TBzlWfvEjP74683520;     TBzlWfvEjP74683520 = TBzlWfvEjP3835187;     TBzlWfvEjP3835187 = TBzlWfvEjP11350680;     TBzlWfvEjP11350680 = TBzlWfvEjP72597019;     TBzlWfvEjP72597019 = TBzlWfvEjP82576225;     TBzlWfvEjP82576225 = TBzlWfvEjP23512982;     TBzlWfvEjP23512982 = TBzlWfvEjP85773603;     TBzlWfvEjP85773603 = TBzlWfvEjP74943024;     TBzlWfvEjP74943024 = TBzlWfvEjP77092845;     TBzlWfvEjP77092845 = TBzlWfvEjP67567683;     TBzlWfvEjP67567683 = TBzlWfvEjP50153331;     TBzlWfvEjP50153331 = TBzlWfvEjP88321705;     TBzlWfvEjP88321705 = TBzlWfvEjP5240131;     TBzlWfvEjP5240131 = TBzlWfvEjP68889403;     TBzlWfvEjP68889403 = TBzlWfvEjP28989014;     TBzlWfvEjP28989014 = TBzlWfvEjP69332850;     TBzlWfvEjP69332850 = TBzlWfvEjP78367882;     TBzlWfvEjP78367882 = TBzlWfvEjP27600530;     TBzlWfvEjP27600530 = TBzlWfvEjP52974532;     TBzlWfvEjP52974532 = TBzlWfvEjP60524916;     TBzlWfvEjP60524916 = TBzlWfvEjP38206188;     TBzlWfvEjP38206188 = TBzlWfvEjP20137561;     TBzlWfvEjP20137561 = TBzlWfvEjP41754855;     TBzlWfvEjP41754855 = TBzlWfvEjP73152391;     TBzlWfvEjP73152391 = TBzlWfvEjP13364185;     TBzlWfvEjP13364185 = TBzlWfvEjP11707117;     TBzlWfvEjP11707117 = TBzlWfvEjP85760433;     TBzlWfvEjP85760433 = TBzlWfvEjP56980646;     TBzlWfvEjP56980646 = TBzlWfvEjP65247314;     TBzlWfvEjP65247314 = TBzlWfvEjP33336416;     TBzlWfvEjP33336416 = TBzlWfvEjP24833727;     TBzlWfvEjP24833727 = TBzlWfvEjP91771114;     TBzlWfvEjP91771114 = TBzlWfvEjP82665699;     TBzlWfvEjP82665699 = TBzlWfvEjP80618543;     TBzlWfvEjP80618543 = TBzlWfvEjP37625430;     TBzlWfvEjP37625430 = TBzlWfvEjP38498706;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vaXYBodICB63920476() {     int OTONmYyQwe21943012 = -86903189;    int OTONmYyQwe50716899 = -239827011;    int OTONmYyQwe49111638 = -792863926;    int OTONmYyQwe34613596 = -465055837;    int OTONmYyQwe24496602 = -990906254;    int OTONmYyQwe50178834 = -923423705;    int OTONmYyQwe4939098 = -703459885;    int OTONmYyQwe3244501 = -995042950;    int OTONmYyQwe28592324 = -81771821;    int OTONmYyQwe76445363 = -98996920;    int OTONmYyQwe20613054 = -724787505;    int OTONmYyQwe90880172 = -819216270;    int OTONmYyQwe77263195 = -762892117;    int OTONmYyQwe96212380 = -271963416;    int OTONmYyQwe75687206 = -1630285;    int OTONmYyQwe67167354 = -542982486;    int OTONmYyQwe12433961 = -424741575;    int OTONmYyQwe89683061 = -409480987;    int OTONmYyQwe73518816 = -158691749;    int OTONmYyQwe39242423 = -548049993;    int OTONmYyQwe26705173 = 10476519;    int OTONmYyQwe20021740 = -308268132;    int OTONmYyQwe17121116 = -619708022;    int OTONmYyQwe15904912 = -420264218;    int OTONmYyQwe37469476 = -780092564;    int OTONmYyQwe45076149 = -465231687;    int OTONmYyQwe479761 = -311361814;    int OTONmYyQwe16157835 = -963302079;    int OTONmYyQwe95837811 = -381184414;    int OTONmYyQwe49008947 = 57223671;    int OTONmYyQwe34558292 = 4447186;    int OTONmYyQwe59620023 = -852979013;    int OTONmYyQwe67378473 = -350034719;    int OTONmYyQwe89953250 = -247698468;    int OTONmYyQwe22715200 = -472548076;    int OTONmYyQwe44645702 = -784917734;    int OTONmYyQwe25879390 = -17463779;    int OTONmYyQwe73487304 = -33608694;    int OTONmYyQwe82883669 = -418139181;    int OTONmYyQwe83480963 = 57532603;    int OTONmYyQwe15956222 = -187732336;    int OTONmYyQwe71399241 = -244510231;    int OTONmYyQwe43119787 = -151441365;    int OTONmYyQwe99960217 = -220885662;    int OTONmYyQwe4253447 = 53520713;    int OTONmYyQwe6510500 = 40765378;    int OTONmYyQwe56416581 = -217708587;    int OTONmYyQwe92327733 = -888261554;    int OTONmYyQwe79672791 = -860274322;    int OTONmYyQwe42114669 = 14583276;    int OTONmYyQwe45908138 = -176333883;    int OTONmYyQwe33757585 = -886600945;    int OTONmYyQwe90488493 = -42407751;    int OTONmYyQwe6761491 = -813145408;    int OTONmYyQwe37215303 = -434611944;    int OTONmYyQwe1921272 = -778635058;    int OTONmYyQwe33595783 = -620118989;    int OTONmYyQwe33206727 = -272599708;    int OTONmYyQwe97144120 = -684963274;    int OTONmYyQwe79420452 = -425674567;    int OTONmYyQwe49699074 = -512061891;    int OTONmYyQwe88781263 = -740157806;    int OTONmYyQwe7406690 = -513858537;    int OTONmYyQwe79583377 = -38995493;    int OTONmYyQwe41887072 = -3444106;    int OTONmYyQwe60993031 = -871808493;    int OTONmYyQwe23501699 = -369181552;    int OTONmYyQwe87309945 = -415193649;    int OTONmYyQwe73497181 = -799415340;    int OTONmYyQwe31041504 = -216712551;    int OTONmYyQwe41287965 = -425518708;    int OTONmYyQwe38946656 = -291132881;    int OTONmYyQwe6799393 = -991341806;    int OTONmYyQwe90037852 = -116224352;    int OTONmYyQwe23286202 = -260317658;    int OTONmYyQwe55305932 = -745013251;    int OTONmYyQwe76901953 = -56826767;    int OTONmYyQwe17160899 = -298822360;    int OTONmYyQwe11651465 = -373784932;    int OTONmYyQwe30958976 = -720857943;    int OTONmYyQwe88659568 = -147523101;    int OTONmYyQwe8152027 = -423100260;    int OTONmYyQwe36485043 = -3027757;    int OTONmYyQwe53723142 = -295767690;    int OTONmYyQwe3100809 = -766442446;    int OTONmYyQwe800708 = -108951869;    int OTONmYyQwe69131529 = -710571262;    int OTONmYyQwe60616983 = -536889312;    int OTONmYyQwe52737947 = -813086525;    int OTONmYyQwe20793928 = -693913019;    int OTONmYyQwe11049919 = -64798746;    int OTONmYyQwe92672663 = -744864071;    int OTONmYyQwe76343184 = -348645421;    int OTONmYyQwe3463218 = -992464615;    int OTONmYyQwe33781890 = -430405506;    int OTONmYyQwe27174959 = -447574530;    int OTONmYyQwe63992552 = -730651694;    int OTONmYyQwe63536409 = -12445873;    int OTONmYyQwe58073145 = -117441557;    int OTONmYyQwe43260416 = -86903189;     OTONmYyQwe21943012 = OTONmYyQwe50716899;     OTONmYyQwe50716899 = OTONmYyQwe49111638;     OTONmYyQwe49111638 = OTONmYyQwe34613596;     OTONmYyQwe34613596 = OTONmYyQwe24496602;     OTONmYyQwe24496602 = OTONmYyQwe50178834;     OTONmYyQwe50178834 = OTONmYyQwe4939098;     OTONmYyQwe4939098 = OTONmYyQwe3244501;     OTONmYyQwe3244501 = OTONmYyQwe28592324;     OTONmYyQwe28592324 = OTONmYyQwe76445363;     OTONmYyQwe76445363 = OTONmYyQwe20613054;     OTONmYyQwe20613054 = OTONmYyQwe90880172;     OTONmYyQwe90880172 = OTONmYyQwe77263195;     OTONmYyQwe77263195 = OTONmYyQwe96212380;     OTONmYyQwe96212380 = OTONmYyQwe75687206;     OTONmYyQwe75687206 = OTONmYyQwe67167354;     OTONmYyQwe67167354 = OTONmYyQwe12433961;     OTONmYyQwe12433961 = OTONmYyQwe89683061;     OTONmYyQwe89683061 = OTONmYyQwe73518816;     OTONmYyQwe73518816 = OTONmYyQwe39242423;     OTONmYyQwe39242423 = OTONmYyQwe26705173;     OTONmYyQwe26705173 = OTONmYyQwe20021740;     OTONmYyQwe20021740 = OTONmYyQwe17121116;     OTONmYyQwe17121116 = OTONmYyQwe15904912;     OTONmYyQwe15904912 = OTONmYyQwe37469476;     OTONmYyQwe37469476 = OTONmYyQwe45076149;     OTONmYyQwe45076149 = OTONmYyQwe479761;     OTONmYyQwe479761 = OTONmYyQwe16157835;     OTONmYyQwe16157835 = OTONmYyQwe95837811;     OTONmYyQwe95837811 = OTONmYyQwe49008947;     OTONmYyQwe49008947 = OTONmYyQwe34558292;     OTONmYyQwe34558292 = OTONmYyQwe59620023;     OTONmYyQwe59620023 = OTONmYyQwe67378473;     OTONmYyQwe67378473 = OTONmYyQwe89953250;     OTONmYyQwe89953250 = OTONmYyQwe22715200;     OTONmYyQwe22715200 = OTONmYyQwe44645702;     OTONmYyQwe44645702 = OTONmYyQwe25879390;     OTONmYyQwe25879390 = OTONmYyQwe73487304;     OTONmYyQwe73487304 = OTONmYyQwe82883669;     OTONmYyQwe82883669 = OTONmYyQwe83480963;     OTONmYyQwe83480963 = OTONmYyQwe15956222;     OTONmYyQwe15956222 = OTONmYyQwe71399241;     OTONmYyQwe71399241 = OTONmYyQwe43119787;     OTONmYyQwe43119787 = OTONmYyQwe99960217;     OTONmYyQwe99960217 = OTONmYyQwe4253447;     OTONmYyQwe4253447 = OTONmYyQwe6510500;     OTONmYyQwe6510500 = OTONmYyQwe56416581;     OTONmYyQwe56416581 = OTONmYyQwe92327733;     OTONmYyQwe92327733 = OTONmYyQwe79672791;     OTONmYyQwe79672791 = OTONmYyQwe42114669;     OTONmYyQwe42114669 = OTONmYyQwe45908138;     OTONmYyQwe45908138 = OTONmYyQwe33757585;     OTONmYyQwe33757585 = OTONmYyQwe90488493;     OTONmYyQwe90488493 = OTONmYyQwe6761491;     OTONmYyQwe6761491 = OTONmYyQwe37215303;     OTONmYyQwe37215303 = OTONmYyQwe1921272;     OTONmYyQwe1921272 = OTONmYyQwe33595783;     OTONmYyQwe33595783 = OTONmYyQwe33206727;     OTONmYyQwe33206727 = OTONmYyQwe97144120;     OTONmYyQwe97144120 = OTONmYyQwe79420452;     OTONmYyQwe79420452 = OTONmYyQwe49699074;     OTONmYyQwe49699074 = OTONmYyQwe88781263;     OTONmYyQwe88781263 = OTONmYyQwe7406690;     OTONmYyQwe7406690 = OTONmYyQwe79583377;     OTONmYyQwe79583377 = OTONmYyQwe41887072;     OTONmYyQwe41887072 = OTONmYyQwe60993031;     OTONmYyQwe60993031 = OTONmYyQwe23501699;     OTONmYyQwe23501699 = OTONmYyQwe87309945;     OTONmYyQwe87309945 = OTONmYyQwe73497181;     OTONmYyQwe73497181 = OTONmYyQwe31041504;     OTONmYyQwe31041504 = OTONmYyQwe41287965;     OTONmYyQwe41287965 = OTONmYyQwe38946656;     OTONmYyQwe38946656 = OTONmYyQwe6799393;     OTONmYyQwe6799393 = OTONmYyQwe90037852;     OTONmYyQwe90037852 = OTONmYyQwe23286202;     OTONmYyQwe23286202 = OTONmYyQwe55305932;     OTONmYyQwe55305932 = OTONmYyQwe76901953;     OTONmYyQwe76901953 = OTONmYyQwe17160899;     OTONmYyQwe17160899 = OTONmYyQwe11651465;     OTONmYyQwe11651465 = OTONmYyQwe30958976;     OTONmYyQwe30958976 = OTONmYyQwe88659568;     OTONmYyQwe88659568 = OTONmYyQwe8152027;     OTONmYyQwe8152027 = OTONmYyQwe36485043;     OTONmYyQwe36485043 = OTONmYyQwe53723142;     OTONmYyQwe53723142 = OTONmYyQwe3100809;     OTONmYyQwe3100809 = OTONmYyQwe800708;     OTONmYyQwe800708 = OTONmYyQwe69131529;     OTONmYyQwe69131529 = OTONmYyQwe60616983;     OTONmYyQwe60616983 = OTONmYyQwe52737947;     OTONmYyQwe52737947 = OTONmYyQwe20793928;     OTONmYyQwe20793928 = OTONmYyQwe11049919;     OTONmYyQwe11049919 = OTONmYyQwe92672663;     OTONmYyQwe92672663 = OTONmYyQwe76343184;     OTONmYyQwe76343184 = OTONmYyQwe3463218;     OTONmYyQwe3463218 = OTONmYyQwe33781890;     OTONmYyQwe33781890 = OTONmYyQwe27174959;     OTONmYyQwe27174959 = OTONmYyQwe63992552;     OTONmYyQwe63992552 = OTONmYyQwe63536409;     OTONmYyQwe63536409 = OTONmYyQwe58073145;     OTONmYyQwe58073145 = OTONmYyQwe43260416;     OTONmYyQwe43260416 = OTONmYyQwe21943012;}
// Junk Finished
