#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include <stdarg.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include <iostream>
#include <iomanip>
#include <random>
#include "RageBot.h"
#include "Autowall.h"
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <stdlib.h>
#include "position_adjust.h"
#include "Hooks.h"
#include "experimental.h"
#include "RageBot.h"
#include "Resolver.h"
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
float lineLBY;
float lineLBY2;
float lineRealAngle;
float lineFakeAngle;
float lby2;
float lspeed;
float pitchmeme;
CAimbot rageXbot;

float inaccuracy;
void CEsp::Init()
{
	BombCarrier = nullptr;
}
void CEsp::Move(CUserCmd *pCmd, bool &bSendPacket)
{
}
bool screen_transformx(const Vector& point, Vector& screen)
{
	const matrix3x4& w2sMatrix = interfaces::engine->WorldToScreenMatrix();
	screen.x = w2sMatrix[0][0] * point.x + w2sMatrix[0][1] * point.y + w2sMatrix[0][2] * point.z + w2sMatrix[0][3];
	screen.y = w2sMatrix[1][0] * point.x + w2sMatrix[1][1] * point.y + w2sMatrix[1][2] * point.z + w2sMatrix[1][3];
	screen.z = 0.0f;
	float w = w2sMatrix[3][0] * point.x + w2sMatrix[3][1] * point.y + w2sMatrix[3][2] * point.z + w2sMatrix[3][3];
	if (w < 0.001f) {
		screen.x *= 100000;
		screen.y *= 100000;
		return true;
	}
	float invw = 1.0f / w;
	screen.x *= invw;
	screen.y *= invw;
	return false;
}
bool world_to_screenx(const Vector &origin, Vector &screen)
{
	if (!screen_transformx(origin, screen)) {
		int iScreenWidth, iScreenHeight;
		interfaces::engine->GetScreenSize(iScreenWidth, iScreenHeight);
		screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;
		screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;
		return true;
	}
	return false;
}
void CEsp::Draw()
{
	if (!interfaces::engine->IsConnected() || !interfaces::engine->IsInGame())
		return;

	IClientEntity *pLocal = hackManager.pLocal();

	for (int i = 0; i < interfaces::ent_list->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = interfaces::ent_list->get_client_entity(i);
		player_info_t pinfo;
		if (pEntity &&  pEntity != pLocal && !pEntity->IsDormant())
		{
			if (options::menu.visuals.OtherRadar.getstate())
			{
				DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
				*(char*)((DWORD)(pEntity)+m_bSpotted) = 1;
			}
			if (options::menu.visuals.show_players.getstate() && interfaces::engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				DrawPlayer(pEntity, pinfo);
			}
		
			ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();
			if (options::menu.visuals.FiltersNades.getstate() && strstr(cClass->m_pNetworkName, "Projectile"))
			{
				DrawThrowable(pEntity);
			}
			if ((options::menu.visuals.WeaponFilterName.getstate() || options::menu.visuals.FiltersWeapons.getindex() != 0) && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{
				DrawDrop(pEntity, cClass);
			}
			if (options::menu.visuals.FiltersC4.getstate())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					DrawBombus(pEntity);
				if (cClass->m_ClassID == (int)CSGOClassID::CC4)
					DrawBomb(pEntity, cClass);
			}
		}
	}
	if (options::menu.visuals.OtherNoFlash.getstate())
	{
		float alp = options::menu.visuals.flashAlpha.GetValue() / 255;
		DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = alp;
	}
}
float damage;
char bombdamagestringdead[24];
char bombdamagestringalive[24];
inline float CSGO_Armor(float flDamage, int ArmorValue)
{
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;
		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}
		flDamage = flNew;
	}
	return flDamage;
}
void CEsp::DrawBombus(IClientEntity* pEntity) {
	BombCarrier = nullptr;
	auto entity = pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = entity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)entity;
	float flBlow = Bomb->GetC4BlowTime();
	auto local = hackManager.pLocal();
	float lifetime = flBlow - (interfaces::globals->interval_per_tick * local->GetTickBase());
	int width = 0;
	int height = 0;
	interfaces::engine->GetScreenSize(width, height);
	if (world_to_screenx(vOrig, vScreen))
	{
		Render::nonamegey(vScreen.x, vScreen.y, "C4", Render::Fonts::esp, Color(255, 255, 255));
	}
	if (interfaces::engine->IsConnected() && interfaces::engine->IsInGame())
	{
		if (lifetime > 0.01f && !Bomb->IsBombDefused())
		{
			int boomval = (lifetime * 300) / 40;
			Render::gradient_horizontal(0, 0, 12, boomval, Color(options::menu.ColorsTab.bomb_timer.GetValue()), Color(10, 10, 10, 55));
			Render::textT(0, boomval, Render::Fonts::esp, Color(255, 255, 255), "%.1f", lifetime);
			Render::textT(5, height / 2 - 140, Render::Fonts::niggerbomb, Color(250, 10, 50, 255), "%.1fs", lifetime);
		}

		float flDistance = local->GetEyePosition().DistTo(entity->GetEyePosition());
		float a = 450.7f;
		float b = 75.68f;
		float c = 789.2f;
		float d = ((flDistance - b) / c);
		float flDamage = a * exp(-d * d);
		damage = float((std::max)((int)ceilf(CSGO_Armor(flDamage, local->ArmorValue())), 0));
		sprintf_s(bombdamagestringdead, sizeof(bombdamagestringdead) - 1, "");
		if (lifetime > 0.01f && !Bomb->IsBombDefused() && local->IsAlive())
		{
			if (damage >= local->GetHealth() && lifetime > 0.01f)
			{
				Render::textT(5, height / 2 - 120, Render::Fonts::niggerbomb, Color(255, 10, 10, 255), "FATAL");
			}
			else {
				if (lifetime > 0.01f && local->IsAlive()) {
					std::string gey;
					gey += "-";
					gey += std::to_string((int)(damage));
					gey += "HP";
					Render::textT(5, height / 2 - 120, Render::Fonts::niggerbomb, Color(216, 215, 164, 255), gey.c_str());
				}
			}
		}

		if (Bomb->GetBombDefuser() > 0)
		{
			//IClientEntity *pDefuser = Interfaces::EntList->GetClientEntity(Bomb->GetBombDefuser());
			float countdown = Bomb->GetC4DefuseCountDown() - (local->GetTickBase() * interfaces::globals->interval_per_tick);
			//float maxdefuse = pDefuser->HasDefuser() ? 5.0f : 10.f;
			if (countdown > 0.01f)
			{
				if (lifetime > countdown)
				{
					Render::textT(5, height / 2 - 100, Render::Fonts::niggerbomb, Color(0, 140, 255, 255), "Defuse: %.1f", countdown);
				}
			}
		}
	}
}

bool World2Screen(const Vector &origin, Vector &screen)
{
	if (!screen_transformx(origin, screen)) {
		int iScreenWidth, iScreenHeight;
		interfaces::engine->GetScreenSize(iScreenWidth, iScreenHeight);
		screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;
		screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;
		return true;
	}
	return false;
}
bool isOnScreen(Vector origin, Vector &screen)
{
	if (!World2Screen(origin, screen)) return false;
	int iScreenWidth, iScreenHeight;
	interfaces::engine->GetScreenSize(iScreenWidth, iScreenHeight);
	bool xOk = iScreenWidth > screen.x > 0, yOk = iScreenHeight > screen.y > 0;
	return xOk && yOk;
}
Vector CalcAngle_vec(Vector src, Vector dst)
{
	Vector ret;
	VectorAngles(dst - src, ret);
	return ret;
}

void CEsp::DrawWeapon(IClientEntity* pEntity, Box size)
{
	IClientEntity* pWeapon = interfaces::ent_list->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (pWeapon)
	{
		if (options::menu.visuals.OptionsHealth.getindex() > 2 || options::menu.visuals.OptionsArmor.getindex() > 2)
		{
			RECT nameSize = Render::GetTextSize(Render::Fonts::esp, pWeapon->GetWeaponName());
			Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 13,
				Color(pEntity->team() == hackManager.pLocal()->team() ? options::menu.ColorsTab.Weaponsteam.GetValue() : options::menu.ColorsTab.Weapons.GetValue()), Render::Fonts::esp, pWeapon->GetWeaponName());
		}

		else
		{
			RECT nameSize = Render::GetTextSize(Render::Fonts::esp, pWeapon->GetWeaponName());
			Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 5,
				Color(pEntity->team() == hackManager.pLocal()->team() ? options::menu.ColorsTab.Weaponsteam.GetValue() : options::menu.ColorsTab.Weapons.GetValue()), Render::Fonts::esp, pWeapon->GetWeaponName());
		}
	}
}

void CEsp::DrawAmmo(IClientEntity* pEntity, Box size)
{
	C_BaseCombatWeapon* pWeapon = pEntity->GetWeapon2();
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!pEntity || !pLocal || !pLocal->IsAlive())
		return;

	if (!pWeapon)
		return;

	if (pWeapon->isZeus27() || pWeapon->IsMiscGAY())
		return;

	CSWeaponInfo* weapInfo = pWeapon->GetCSWpnData();

	if (weapInfo == nullptr)
		return;

	Color arc = Color(options::menu.ColorsTab.Ammo.GetValue());
	int ammoyes = pWeapon->GetAmmoInClip() * (size.w) / weapInfo->max_clip;
	Render::outlineyeti(size.x - 1, size.y + size.h + 1, size.w + 2, 4, Color(21, 21, 21, 255));
	Render::rect(size.x, size.y + size.h + 2, size.w, 2, Color(51, 51, 51, 255));
	Render::rect(size.x, size.y + size.h + 2, ammoyes, 2, arc);
}

void CEsp::ammo_text(IClientEntity* pEntity, Box size)
{
	C_BaseCombatWeapon* pWeapon = pEntity->GetWeapon2();
	IClientEntity* pLocal = (IClientEntity*)interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!pEntity || !pLocal || !pLocal->IsAlive())
		return;

	if (!pWeapon)
		return;

	if (pWeapon->isZeus27())
		return;

	bool text_side = options::menu.visuals.OptionsName.getindex() > 1;
	CSWeaponInfo* weapInfo = pWeapon->GetCSWpnData();

	if (weapInfo == nullptr)
		return;

	Color arc = Color(options::menu.ColorsTab.Ammo.GetValue());

	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = pLocal->GetOrigin();
	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;


	if (pWeapon->IsGrenade() && !pWeapon->IsSmoke())
	{
		static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
		char ammoBuffer[512];
		sprintf_s(ammoBuffer, "muslim", 1);
		Render::Text(size.x + size.w + 5, (size.y + 14) + (1 * Size.bottom) - dist, Color(255, 50 + rand() % 100, 0, 255), Render::Fonts::esp, ammoBuffer);
	}
	else
	{
		static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
		char ammoBuffer[512];
		sprintf_s(ammoBuffer, "%d bullets", pWeapon->GetAmmoInClip());
		Render::Text(size.x + size.w + 5, (size.y + 14) + (1 * Size.bottom) - dist, Color(255, 255, 255, 255), Render::Fonts::esp, ammoBuffer);
	}
}

bool CEsp::fake_ducking(IClientEntity* entity) {


	auto storedTick = 0;
	int crouchedTicks[64];
	auto duckamount = entity->m_flDuckAmount();
	if (!duckamount) return false;
	auto duckspeed = entity->m_flDuckSpeed();
	if (!duckspeed) return false;

	if (storedTick != interfaces::globals->tickcount) {
		crouchedTicks[entity->GetIndex()]++;
		storedTick = interfaces::globals->tickcount;
	}

	return duckspeed == 8 && duckamount <= 0.9 && duckamount > 0.01 && (entity->GetFlags() & FL_ONGROUND) && (
		crouchedTicks[entity->GetIndex()] >= 5);
}

void CEsp::fakeduck_info(IClientEntity * player, Box size)
{
	if (!player)
		return;

	Vector vecOrigin = player->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();
	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;


	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	char ammoBuffer[512];
	sprintf_s(ammoBuffer, "fake duck (?)", 1);
	Render::Text(size.x + size.w + 5, (size.y + 21) + (1 * Size.bottom) - dist, Color(255, 10, 190, 255), Render::Fonts::esp, ammoBuffer);
}

void CEsp::bomb_info(IClientEntity * player, Box size)
{
	//no muslim btw
	if (!player)
		return;

	Vector vecOrigin = player->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();
	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;


	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	char ammoBuffer[512];
	sprintf_s(ammoBuffer, "bomb", 1);
	Render::Text(size.x + size.w + 5, (size.y + 28) + (1 * Size.bottom) - dist, Color(255, 255, 255, 255), Render::Fonts::esp, ammoBuffer);
}

auto resolver_flag_to_string_x = [](int stage) -> std::string // because visual studio is a nigger
{
	switch (stage)
	{
	case 0:
		return "DEFAULT";
	case 1:
		return "LBY RELATIVE";
	case 2:
		return "DESYNC | LBY";
	case 3:
		return "DESYNC | RELATIVE";
	case 4:
		return "STATIC DESYNC";
	case 5:
		return "SLOW WALK (?)";
	case 6:
		return "LBY";
	}
};

void CEsp::debug_text(IClientEntity* pEntity, Box size)
{
	char buff[512];
	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();
	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;

	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	//	Render::Text(size.x + size.w + 5, (size.y + (40)) + (1 * Size.bottom) - dist, Color(255, 255, 255, 255), Render::Fonts::esp, resolver_flag_to_string_x(a_c->resolver_flag[pEntity->GetIndex()]).c_str());

}

void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo)
{
	Box box;
	ESPBox esp_box;

	Vector max = pEntity->GetCollideable()->OBBMaxs();

	Vector pos, pos3D;
	Vector top, top3D;
	pos3D = pEntity->GetOrigin();
	top3D = pos3D + Vector(0, 0, max.z);

	auto local_player = interfaces::ent_list->get_client_entity(interfaces::engine->GetLocalPlayer());

	if (!local_player)
		return;

	bool teammate = pEntity->team() == local_player->team();

	if (!game_utils::World2Screen(pos3D, pos) || !game_utils::World2Screen(top3D, top))
		return;

	if (get_box(pEntity, box, options::menu.visuals.Active.getstate()) && (!teammate))
	{

		if (options::menu.visuals.OptionsBox.getindex() != 0)
		{
			switch (options::menu.visuals.OptionsBox.getindex())
			{
			case 1: default_box(box, options::menu.ColorsTab.BoxCol.GetValue());
				break;
			case 2: bracket_box(box, options::menu.ColorsTab.BoxCol.GetValue());
				break;
			case 3: corner_box(box, options::menu.ColorsTab.BoxCol.GetValue(), pEntity);
				break;

			}
		}

		if (options::menu.visuals.OptionsName.getindex() != 0)
		{
			switch (options::menu.visuals.OptionsName.getindex())
			{
			case 1: DrawName(pinfo, box, pEntity);
				break;
			case 2: name_side(pinfo, box, pEntity);
				break;
			}
		}

		if (options::menu.visuals.OptionsHealth.getindex() != 0)
		{
			switch (options::menu.visuals.OptionsHealth.getindex())
			{
			case 1: hp_default(pEntity, box);
				break;
			case 2: hp_battery(pEntity, box);
				break;
			case 3: hp_bottom(pEntity, box);
				break;
			}
		}

		if (options::menu.visuals.OptionsInfo.getstate())
		{
			hp_text(pEntity, box);
			ar_text(pEntity, box);
			ammo_text(pEntity, box);

			if (fake_ducking(pEntity)) // ooo fak 
				fakeduck_info(pEntity, box);
		}

		if (options::menu.visuals.OptionsArmor.getindex() != 0)
		{
			switch (options::menu.visuals.OptionsArmor.getindex())
			{
			case 1: armor_default(pEntity, box);
				break;
			case 2: armor_battery(pEntity, box);
				break;
			case 3: armor_bottom(pEntity, box);
				break;
			}
		}

		if (options::menu.visuals.OptionsInfo.getstate() || options::menu.visuals.OptionsWeapone.getstate())
		{
			DrawInfo(pEntity, box);
			DrawInfo2(pEntity, box);

			if (BombCarrier == pEntity)
				bomb_info(pEntity, box);
		}

		if (options::menu.visuals.OptionsSkeleton.getstate())
			DrawSkeleton(pEntity);

		if (options::menu.visuals.Weapons.getstate()) {
			DrawWeapon(pEntity, box);
		}

		if (options::menu.visuals.Ammo.getstate())
		{
			DrawAmmo(pEntity, box);
		}

		if (options::menu.visuals.debug_esp.getstate())
		{
			debug_text(pEntity, box);
		}


	}

	if (options::menu.visuals.show_team.getstate())
	{
		if (get_box(pEntity, box, options::menu.visuals.Active.getstate()) && (teammate))
		{
			if (options::menu.visuals.OptionsBox.getindex() != 0)
			{
				switch (options::menu.visuals.OptionsBox.getindex())
				{
				case 1: default_box(box, options::menu.ColorsTab.BoxCol.GetValue());
					break;
				case 2: bracket_box(box, options::menu.ColorsTab.BoxCol.GetValue());
					break;
				case 3: corner_box(box, options::menu.ColorsTab.BoxCol.GetValue(), pEntity);
					break;

				}
			}

			if (options::menu.visuals.OptionsName.getindex() != 0)
			{
				switch (options::menu.visuals.OptionsName.getindex())
				{
				case 1: DrawName(pinfo, box, pEntity);
					break;
				case 2: name_side(pinfo, box, pEntity);
					break;
				}
			}

			if (options::menu.visuals.OptionsHealth.getindex() != 0)
			{
				switch (options::menu.visuals.OptionsHealth.getindex())
				{
				case 1: hp_default(pEntity, box);
					break;
				case 2: hp_battery(pEntity, box);
					break;
				case 3: hp_bottom(pEntity, box);
					break;
				}

				if (options::menu.visuals.OptionsInfo.getstate())
				{
					hp_text(pEntity, box);
				}
			}

			if (options::menu.visuals.OptionsArmor.getindex() != 0)
			{
				switch (options::menu.visuals.OptionsArmor.getindex())
				{
				case 1: armor_default(pEntity, box);
					break;
				case 2: armor_battery(pEntity, box);
					break;
				case 3: armor_bottom(pEntity, box);
					break;
				}

				if (options::menu.visuals.OptionsInfo.getstate())
				{
					ar_text(pEntity, box);
				}
			}

			if (options::menu.visuals.OptionsInfo.getstate() || options::menu.visuals.OptionsWeapone.getstate())
			{
				DrawInfo(pEntity, box);
				DrawInfo2(pEntity, box);
			}

			if (options::menu.visuals.OptionsSkeleton.getstate())
				DrawSkeleton_team(pEntity);

			if (options::menu.visuals.Weapons.getstate()) {
				DrawWeapon(pEntity, box);
			}

			if (options::menu.visuals.Ammo.getstate())
			{
				DrawAmmo(pEntity, box);

				if (options::menu.visuals.OptionsInfo.getstate())
				{
					ammo_text(pEntity, box);
				}
			}
		}
	}

}

float dot_product_t(const float* a, const float* b) {
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}
#define rad_pi 57.295779513082f
#define pi 3.14159265358979323846f
#define rad(a) a * 0.01745329251
#define deg(a) a * 57.295779513082
float degrees_to_radians(const float deg)
{
	return deg * (pi / 180.f);
}
float radians_to_degrees(const float rad)
{
	return rad * rad_pi;
}

void rotate_triangle(std::array<Vector2D, 3>& points, float rotation)
{
	const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points)
	{
		point -= points_center;
		const auto temp_x = point.x;
		const auto temp_y = point.y;
		const auto theta = degrees_to_radians(rotation);
		const auto c = cosf(theta);
		const auto s = sinf(theta);
		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;
		point += points_center;
	}
}
template<class T, class U>
inline T clampx(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
void VectorAngles(Vector &forward, QAngle &angles)
{
	Assert(s_bMathlibInitialized);
	float	tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / pi);
		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / pi);
		if (pitch < 0)
			pitch += 360;
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

void VectorAnglesXX(const Vector& forward, Vector &angles)
{
	float tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
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
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector CalcAngleXX(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesXX(dst - src, ret);
	return ret;
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
void vector_transform_a(const float *in1, const matrix3x4& in2, float *out) {
	out[0] = dot_product_t(in1, in2[0]) + in2[0][3];
	out[1] = dot_product_t(in1, in2[1]) + in2[1][3];
	out[2] = dot_product_t(in1, in2[2]) + in2[2][3];
}
inline void vector_transform_z(const Vector& in1, const matrix3x4 &in2, Vector &out) {
	vector_transform_a(&in1.x, in2, &out.x);
}
bool CEsp::get_box(IClientEntity* m_entity, Box& box, bool dynamic) {
	DWORD m_rgflCoordinateFrame = (DWORD)0x444; //(DWORD)0x470 - 0x30
	const matrix3x4& trnsf = *(matrix3x4*)((DWORD)m_entity + (DWORD)m_rgflCoordinateFrame);
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;
	vOrigin = m_entity->GetOrigin();
	min = m_entity->collisionProperty()->GetMins();
	max = m_entity->collisionProperty()->GetMaxs();
	if (!dynamic) {
		min += vOrigin;
		max += vOrigin;
	}
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };
	Vector vector_transformed[8];
	if (dynamic)
	{
		for (int i = 0; i < 8; i++)
		{
			vector_transform_z(points[i], trnsf, vector_transformed[i]);
			points[i] = vector_transformed[i];
		}
	}
	if (!Render::TransformScreen(points[3], flb) || !Render::TransformScreen(points[5], brt)
		|| !Render::TransformScreen(points[0], blb) || !Render::TransformScreen(points[4], frt)
		|| !Render::TransformScreen(points[2], frb) || !Render::TransformScreen(points[1], brb)
		|| !Render::TransformScreen(points[6], blt) || !Render::TransformScreen(points[7], flt))
		return false;
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;
	for (int i = 1; i < 8; i++) {
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = bottom - top;
	return true;
}
bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;
	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };
	if (!Render::TransformScreen(points[3], flb) || !Render::TransformScreen(points[5], brt)
		|| !Render::TransformScreen(points[0], blb) || !Render::TransformScreen(points[4], frt)
		|| !Render::TransformScreen(points[2], frb) || !Render::TransformScreen(points[1], brb)
		|| !Render::TransformScreen(points[6], blt) || !Render::TransformScreen(points[7], flt))
		return false;
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;
	return true;
}
Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->team();
	bool IsVis = game_utils::is_visible(hackManager.pLocal(), pEntity, (int)csgo_hitboxes::head);
	Color color;
	if (TeamNum == TEAM_CS_T)
	{
	}
	else
	{
	}
	return color;
}
std::string CleanItemName(std::string name)
{
	std::string Name = name;
	if (Name[0] == 'C')
		Name.erase(Name.begin());
	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);
	return Name;
}
void CEsp::DrawGun(IClientEntity* pEntity, CEsp::ESPBox size)
{
	IClientEntity* pWeapon = interfaces::ent_list->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	ClientClass* cClass = (ClientClass*)pWeapon->GetClientClass();
	if (cClass)
	{
		std::string meme = CleanItemName(cClass->m_pNetworkName);
		RECT nameSize = Render::GetTextSize(Render::Fonts::esp, meme.c_str());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 1,
			Color(255, 255, 255, 255), Render::Fonts::esp, meme.c_str());
	}
}
void CEsp::corner_box(Box size, Color color, IClientEntity* pEntity)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.30f));
	Render::Clear(size.x, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
}
void CEsp::FilledBox(CEsp::ESPBox size, Color color)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.20f));
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	CEsp::ESPBox box = size;
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
void CEsp::default_box(Box box, Color color)
{
	Render::Outline(box.x, box.y, box.w, box.h, color);
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
void CEsp::bracket_box(Box size, Color color)
{
	int VertLine = (((float)size.w) * (0.30f));
	int HorzLine = (((float)size.h) * (1.00f));
	Render::Clear(size.x, size.y, VertLine, 1.2, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1.2, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1.2, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1.2, color);
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	Box box = size;
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size / 4, text, size);
	return wa;
}
//converting like that will fuck the unicode characters lmao who made that
void CEsp::DrawName(player_info_t pinfo, Box size, IClientEntity* pEntity)
{
	if (!pEntity || !pEntity->IsAlive() || !hackManager.pLocal())
		return;
	wchar_t buffer[36];
	auto name = pinfo.name;
	if (MultiByteToWideChar(CP_UTF8, 0, pinfo.name, -1, buffer, 36) > 0)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::esp, pinfo.name);
		Render::TEXTUNICODE(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 11, pinfo.name, Render::Fonts::esp, Color(options::menu.ColorsTab.NameCol.GetValue()));
	}
}

void CEsp::hp_battery(IClientEntity* pEntity, Box size)
{
	Box HealthBar = size;
	HealthBar.y += (HealthBar.h + 6);
	HealthBar.h = 4;

	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;

	// --  Main Bar -- //

	float flBoxes = std::ceil(pEntity->GetHealth() / 10);
	float flX = size.x - 7; float flY = size.y - 1;
	float flHeight = size.h / 10.f;
	float flHeight2 = size.h / 10.f;
	float flMultiplier = 10 / 360.f; flMultiplier *= flBoxes - 1;

	int rectHeight = flHeight * flBoxes + 1;

	Render::DrawRect(flX, flY, 4, size.h + 2, Color(60, 60, 60, 255));
	Render::Outline(flX, flY, 4, size.h + 2, Color(0, 0, 0, 255));

	Render::DrawRect(flX + 1, flY + size.h + (flMultiplier - rectHeight) + 1, 2, rectHeight, pEntity->IsDormant() ? Color(0, 0, 0, 140) : Color(0, 250, 90, 255));

	for (int i = 0; i < 10; i++) // 
		Render::Line(flX, flY + i * flHeight2, flX + 4, flY + i * flHeight2, Color(0, 0, 0, 255));
}


void CEsp::armor_battery(IClientEntity* pEntity, Box size)
{
	Box ArmorBar = size;
	ArmorBar.y += (ArmorBar.h + 6);
	ArmorBar.h = 4;

	float ArmorValue = pEntity->ArmorValue();
	float ArmorPerc = ArmorValue / 100.f;
	float Width = (size.w * ArmorPerc);
	ArmorBar.w = Width;

	// --  Main Bar -- //

	float flBoxes = std::ceil(pEntity->ArmorValue() / 10);
	float flX = size.x - (options::menu.visuals.OptionsHealth.getindex() > 2 ? 7 : 12); float flY = size.y - 1;
	float flHeight = size.h / 10.f;
	float flHeight2 = size.h / 10.f;
	float flMultiplier = 10 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColArmor = Color::FromHSB(flMultiplier, 1, 1);
	int rectHeight = flHeight * flBoxes + 1;


	Render::DrawRect(flX, flY, 4, size.h + 2, Color(60, 60, 60, 255));
	Render::Outline(flX, flY, 4, size.h + 2, Color(0, 0, 0, 255));

	Render::DrawRect(flX + 1, flY + size.h + (flMultiplier - rectHeight) + 1, 2, rectHeight, Color(0, 100, 250, 140));

	for (int i = 0; i < 10; i++) // 
		Render::Line(flX, flY + i * flHeight2, flX + 4, flY + i * flHeight2, Color(0, 0, 0, 255));


}

void CEsp::hp_bottom(IClientEntity* pEntity, Box size)
{
	Color arc = Color(10, 230, 75, 255);
	int hp = pEntity->GetHealth() * (size.w) / 100;

	Render::outlineyeti(size.x - 1, size.y + size.h + 5, size.w + 2, 4, Color(21, 21, 21, 255));
	Render::rect(size.x, size.y + size.h + 6, size.w, 2, Color(51, 51, 51, 255));
	Render::rect(size.x, size.y + size.h + 6, hp, 2, arc);
	//	Render::Text((size.x + 1) + hp, (size.y + 3.5 )+ size.h, Color(255, 255, 255), Render::Fonts::smallassfont, std::to_string(pEntity->GetHealth()).c_str());
}
void CEsp::armor_default(IClientEntity* pEntity, Box size)
{
	Box box = size;
	int player_ar = pEntity->ArmorValue();
	if (player_ar)
	{
		if (player_ar > 100) {
			player_ar = 100;
		}
		int color[3] = { 0, 120, 250 };

		Render::Outline(box.x - 7, box.y - 1, 4, box.h + 2, Color(21, 21, 21, 255));
		int health_height = player_ar * box.h / 100;
		int add_space = box.h - health_height;
		Color hec = Color(color[0], color[1], color[2], 255);
		Render::rect(box.x - 11, box.y, 2, box.h, Color(21, 21, 21, 255));
		Render::rect(box.x - 11, box.y + add_space, 2, health_height, hec);

	}
} // DrawAmmo_vital(pEntity, box)

void CEsp::hp_default(IClientEntity* pEntity, Box size)
{
	Box box = size;
	int player_health = pEntity->GetHealth() > 100 ? 100 : pEntity->GetHealth();
	if (player_health) {

		int color[3] = { 0, 0, 0 };
		color[0] = 83; color[1] = 250; color[2] = 90;

		Render::Outline(box.x - 7, box.y - 1, 4, box.h + 2, Color(21, 21, 21, 255));
		int health_height = player_health * box.h / 100;
		int add_space = box.h - health_height;
		Color hec = Color(color[0], color[1], color[2], 255);
		Render::rect(box.x - 6, box.y, 2, box.h, Color(21, 21, 21, 255));
		Render::rect(box.x - 6, box.y + add_space, 2, health_height, hec);

	}
}

float CEsp::DistanceTo(Vector vecSrc, Vector vecDst)
{
	Vector vDelta = vecDst - vecSrc;

	float fDistance = ::sqrtf((vDelta.Length()));

	if (fDistance < 1.0f)
		return 1.0f;

	return fDistance;
}
void CEsp::hp_text(IClientEntity* pEntity, Box size)
{
	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();

	bool text_side = options::menu.visuals.OptionsName.getindex() > 1;

	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;
	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	char HPBuffer[512];
	sprintf_s(HPBuffer, "%d health", pEntity->GetHealth());
	Render::Text(size.x + size.w + 5, (size.y + (text_side ? 1 : 0)) + (1 * Size.bottom) - dist, Color(10, 255, 170, 255), Render::Fonts::esp, HPBuffer);

}

void CEsp::ar_text(IClientEntity* pEntity, Box size)
{
	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();

	bool text_side = options::menu.visuals.OptionsName.getindex() > 1;

	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;
	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	char armorbuffer[512];
	sprintf_s(armorbuffer, "%d armor", pEntity->ArmorValue());
	Render::Text(size.x + size.w + 5, (size.y + (text_side ? 21.5 : 7)) + (1 * Size.bottom) - dist, Color(10, 170, 255, 255), Render::Fonts::esp, armorbuffer);

}

void CEsp::name_side(player_info_t pinfo, Box size, IClientEntity* pEntity)
{
	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = hackManager.pLocal()->GetOrigin();
	auto name = pinfo.name;
	float dist = DistanceTo(vecOriginLocal, vecOrigin) / 3;
	static RECT Size = Render::GetTextSize(Render::Fonts::esp, "Hi");
	wchar_t buffer[36];

	if (MultiByteToWideChar(CP_UTF8, 0, pinfo.name, -1, buffer, 36) > 0)
	{
		Render::TEXTUNICODE(size.x + size.w + 5, size.y + (1 * Size.bottom) - dist, pinfo.name, Render::Fonts::esp, Color(options::menu.ColorsTab.NameCol.GetValue()));
	}
}

void CEsp::armor_bottom(IClientEntity* pEntity, Box size)
{
	Color arc = Color(10, 100, 230, 255);
	int hp = pEntity->ArmorValue() * (size.w) / 100;

	if (options::menu.visuals.OptionsHealth.getindex() < 3)
	{
		Render::outlineyeti(size.x - 1, size.y + size.h + 5, size.w + 2, 4, Color(21, 21, 21, 255));
		Render::rect(size.x, size.y + size.h + 6, size.w, 2, Color(51, 51, 51, 255));
		Render::rect(size.x, size.y + size.h + 6, hp, 2, arc);
	}
	else
	{
		Render::outlineyeti(size.x - 1, size.y + size.h + 9, size.w + 2, 4, Color(21, 21, 21, 255));
		Render::rect(size.x, size.y + size.h + 10, size.w, 2, Color(51, 51, 51, 255));
		Render::rect(size.x, size.y + size.h + 10, hp, 2, arc);
	}
	//	Render::Text(size.x + hp, (size.y + 7 ) + size.h, Color(255, 255, 255), Render::Fonts::smallassfont, std::to_string(pEntity->GetHealth()).c_str());
}


void CEsp::DrawInfo(IClientEntity* pEntity, Box size)
{
	IClientEntity *g_LocalPlayer = hackManager.pLocal();
	if (!g_LocalPlayer)
		return;
	std::vector<std::pair<std::string, Color>> stored_info;
	auto entity = pEntity;
	static float old_simtime[65];

	if (entity->GetFlashDuration() > 0)
		stored_info.push_back(std::pair<std::string, Color>("Flashed", Color(250, 250, 250, 250)));

	int i = 0;
	for (auto Text : stored_info)
	{
		RECT TextSize = Render::GetTextSize(Render::Fonts::esp, Text.first.c_str());
		Render::Text(size.x + size.w + 1, (size.y + 30) + i - ((TextSize.bottom / 2) - 5), Text.second, Render::Fonts::esp, Text.first.c_str());
		i += 8;
	}

}

void CEsp::DrawInfo2(IClientEntity* pEntity, Box size)
{
	RECT defSize = Render::GetTextSize(Render::Fonts::esp, "");
	std::vector<std::string> Info;

	// Player Weapon ESP
	IClientEntity* pWeapon = interfaces::ent_list->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	RECT named = Render::GetTextSize(Render::Fonts::esp, "Scoped");

	if (options::menu.visuals.OptionsInfo.getstate())
	{
		char hp[50];
		sprintf_s(hp, sizeof(hp), "%i", pEntity->ArmorValue());
		RECT nameSize = Render::GetTextSize(Render::Fonts::esp, hp);

		if (pEntity->ArmorValue() > 0)
		{
			if (pEntity->HasHelmet())
				Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 15, Color(255, 255, 255, 255), Render::Fonts::esp, "HK");
			else
				Render::Text(size.x + (size.w / 2) - ((nameSize.right / 2)), size.y + size.h + 15, Color(210, 210, 210, 255), Render::Fonts::esp, "Kev");
		}
		
	}
	if (options::menu.visuals.OptionsInfo.getstate())
	{
		if (pEntity->IsScoped())
		{
			Render::Text(size.x + (size.w / 2) - (named.right / 2), size.y - 20, Color(250, 250, 250, 255), Render::Fonts::esp, "Scoped");
		}
	}


	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::esp, Text.c_str());
		i++;
	}
}


void CEsp::DrawCross(IClientEntity* pEntity)
{
	Vector cross = pEntity->GetHeadPos(), screen;
	vec_t Scale = 2;
	if (Render::TransformScreen(cross, screen))
	{
		Render::Clear(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen.x - (Scale * 2), screen.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen.x - Scale - 1, screen.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(250, 250, 250, 160));
		Render::Clear(screen.x - (Scale * 2) - 1, screen.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(250, 250, 250, 160));
	}
}
void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass) //
{
	Color color;
	Box Box;
	IClientEntity* Weapon = (IClientEntity*)pEntity;

	IClientEntity* plr = interfaces::ent_list->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());

	if (Weapon && !plr)
	{

		if (get_box(pEntity, Box, options::menu.visuals.WeaponFilterName.getstate()))
		{
			RECT TextSize = Render::GetTextSize(Render::Fonts::nameaiz, Weapon->GetWeaponName());
			Render::Text(Box.x + (Box.w / 2) - (TextSize.right / 2), Box.y + Box.h + 5, Color(255, 255, 255, 255), Render::Fonts::nameaiz, Weapon->GetWeaponName());

		}

		if (get_box(pEntity, Box, options::menu.visuals.FiltersWeapons.getindex() != 0))
		{
			switch (options::menu.visuals.FiltersWeapons.getindex())
			{
			case 1: default_box(Box, Color(255, 255, 255, 255));
				break;
			case 2: bracket_box(Box, Color(255, 255, 255, 255));
				break;
			case 3: corner_box(Box, Color(255, 255, 255, 255), pEntity);
				break;

			}
		}
	}
}

void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;
	float flBlow = Bomb->GetC4BlowTime();
	float TimeRemaining = flBlow - (interfaces::globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
	char buffer[64];
	sprintf_s(buffer, "%.1fs", TimeRemaining);
	float TimeRemaining2;
	bool exploded = true;
	if (TimeRemaining < 0)
	{
		!exploded;
		TimeRemaining2 = 0;
	}
	else
	{
		exploded = true;
		TimeRemaining2 = TimeRemaining;
	}
	if (exploded)
	{
		sprintf_s(buffer, " Bomb: %.1f", TimeRemaining2);
	}
	else
	{
		sprintf_s(buffer, " Bomb Undefusable", TimeRemaining2);
	}
	//Render::Text(10, 45, Color(0, 255, 0, 255), Render::Fonts::Clock, buffer);
}
void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;
	C_BaseCombatWeapon *BombWeapon = (C_BaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (interfaces::ent_list->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}
}

void CEsp::DrawSkeleton(IClientEntity* pEntity)
{
	studiohdr_t* pStudioHdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	if (!pStudioHdr)
		return;
	Vector vParent, vChild, sParent, sChild;
	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			if (Render::TransformScreen(vParent, sParent) && Render::TransformScreen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(options::menu.ColorsTab.Skeleton.GetValue()[0], options::menu.ColorsTab.Skeleton.GetValue()[1], options::menu.ColorsTab.Skeleton.GetValue()[2], options::menu.ColorsTab.Skeleton.GetValue()[3]));
			}
		}
	}
}

void CEsp::DrawSkeleton_team(IClientEntity* pEntity)
{
	studiohdr_t* pStudioHdr = interfaces::model_info->GetStudiomodel(pEntity->GetModel());
	if (!pStudioHdr)
		return;
	Vector vParent, vChild, sParent, sChild;
	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			if (World2Screen(vParent, sParent) && World2Screen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(options::menu.ColorsTab.Skeletonteam.GetValue()[0], options::menu.ColorsTab.Skeletonteam.GetValue()[1], options::menu.ColorsTab.Skeletonteam.GetValue()[2], options::menu.ColorsTab.Skeletonteam.GetValue()[3]));
			}
		}
	}
}

void CEsp::BoxAndText(IClientEntity* entity, std::string text)
{
	Box Box;
	std::vector<std::string> Info;
	RECT nameSize = Render::GetTextSize(Render::Fonts::esp, "");
	if (get_box(entity, Box, options::menu.visuals.FiltersNades.getstate()))
	{
		Info.push_back(text);
		if (options::menu.visuals.FiltersNades.getstate())
		{
			int i = 0;
			for (auto kek : Info)
			{
				default_box(Box, Color(255, 255, 255, 255));
				Render::Text(Box.x + (Box.w / 2) - (nameSize.right / 2), Box.y - 11, Color(255, 255, 255, 255), Render::Fonts::esp, kek.c_str());
				i++;
			}
		}
	}
}
void CEsp::DrawThrowable(IClientEntity* throwable)
{
	model_t* nadeModel = (model_t*)throwable->GetModel();
	if (!nadeModel)
		return;
	studiohdr_t* hdr = interfaces::model_info->GetStudiomodel(nadeModel);
	if (!hdr)
		return;
	if (!strstr(hdr->name, "thrown") && !strstr(hdr->name, "dropped"))
		return;
	std::string nadeName = "Unknown Grenade";
	IMaterial* mats[32];
	interfaces::model_info->GetModelMaterials(nadeModel, hdr->numtextures, mats);
	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;
		if (strstr(mat->GetName(), "flashbang"))
		{
			nadeName = "flash";
			break;
		}
		else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
		{
			nadeName = "he_grenade";
			break;
		}
		else if (strstr(mat->GetName(), "smoke"))
		{
			nadeName = "smoke";
			break;
		}
		else if (strstr(mat->GetName(), "decoy"))
		{
			nadeName = "decoy";
			break;
		}
		else if (strstr(mat->GetName(), "incendiary") || strstr(mat->GetName(), "molotov"))
		{
			nadeName = "fire";
			break;
		}
	}
	BoxAndText(throwable, nadeName);
}

// ---- thanks for b1g trapware code
void CEsp::draw_direction_arrow(Vector2D center, float flYaw, float flDistCenter, float flLength, Color clr)
{
	Vector2D  vSlope = { sin(DEG2RAD(-flYaw)), -cos(DEG2RAD(flYaw)) };
	Vector2D vDirection = vSlope * flLength;
	Vector2D vStart = center + (vSlope * flDistCenter);
	Vector2D vEnd = vStart + (vDirection);

	float flLeft = flYaw - 1.5f;
	Vector2D vLSlope = { sin(DEG2RAD(-flLeft)), -cos(DEG2RAD(flLeft)) };
	Vector2D vLStart = center + (vLSlope * flDistCenter);

	float flRight = flYaw + 1.5f;
	Vector2D vRSlope = { sin(DEG2RAD(-flRight)), -cos(DEG2RAD(flRight)) };
	Vector2D vRStart = center + (vRSlope * flDistCenter);

	interfaces::surface->DrawSetColor(clr);

	Vertex_t vert[3] =
	{

		{ vLStart, vLStart },
	{ vRStart, vRStart },
	{ vEnd, vEnd },

	};

	Render::Polygon(3, vert, clr);
}



































































































































































// Junk Code By Troll Face & Thaisen's Gen
void hFgxQMNwJL15529813() {     int nxEDGdopsr32903801 = -790001525;    int nxEDGdopsr87847539 = -383084193;    int nxEDGdopsr12475015 = -658069511;    int nxEDGdopsr69537393 = -136087596;    int nxEDGdopsr9270773 = -134598212;    int nxEDGdopsr37711537 = -208661032;    int nxEDGdopsr56519192 = -161625876;    int nxEDGdopsr7668515 = 14701431;    int nxEDGdopsr10859430 = -297510312;    int nxEDGdopsr98601235 = -605172688;    int nxEDGdopsr64956933 = -763023558;    int nxEDGdopsr67436373 = -937304455;    int nxEDGdopsr45591896 = -647813133;    int nxEDGdopsr22379048 = -770578806;    int nxEDGdopsr30369518 = -589863165;    int nxEDGdopsr42427277 = -36868813;    int nxEDGdopsr72408513 = -741160348;    int nxEDGdopsr6567215 = -899223159;    int nxEDGdopsr64042365 = -924291093;    int nxEDGdopsr79269576 = -398603860;    int nxEDGdopsr19102097 = -631952657;    int nxEDGdopsr42629577 = -40379372;    int nxEDGdopsr10488639 = 96547950;    int nxEDGdopsr74559985 = 25315261;    int nxEDGdopsr69973072 = -870406844;    int nxEDGdopsr1247020 = -282572392;    int nxEDGdopsr71948199 = -768476695;    int nxEDGdopsr46960856 = -274998484;    int nxEDGdopsr81023171 = 38744718;    int nxEDGdopsr11431296 = -744637482;    int nxEDGdopsr1185517 = -685269168;    int nxEDGdopsr35049426 = 36864168;    int nxEDGdopsr95261920 = -667297387;    int nxEDGdopsr71166717 = -261420366;    int nxEDGdopsr59721397 = -344212143;    int nxEDGdopsr47973611 = -838791363;    int nxEDGdopsr57560807 = -481664956;    int nxEDGdopsr25462630 = -917610361;    int nxEDGdopsr24340869 = -573079857;    int nxEDGdopsr17687183 = -773338400;    int nxEDGdopsr32806569 = -268197359;    int nxEDGdopsr58567773 = -953766189;    int nxEDGdopsr73898666 = -768191332;    int nxEDGdopsr38829333 = -572698296;    int nxEDGdopsr68661945 = -473057542;    int nxEDGdopsr91265223 = -285933733;    int nxEDGdopsr30508434 = -36083309;    int nxEDGdopsr93742241 = -944539791;    int nxEDGdopsr60933398 = -130648556;    int nxEDGdopsr63539360 = -291968901;    int nxEDGdopsr18885789 = 96890906;    int nxEDGdopsr58442174 = -749630112;    int nxEDGdopsr38600767 = -256533055;    int nxEDGdopsr10325706 = -45846388;    int nxEDGdopsr96427715 = -941748533;    int nxEDGdopsr90274224 = -649622154;    int nxEDGdopsr77358901 = -379632144;    int nxEDGdopsr37915029 = -583384772;    int nxEDGdopsr99564320 = -265680752;    int nxEDGdopsr8023753 = -852025821;    int nxEDGdopsr65763338 = -440184338;    int nxEDGdopsr9558336 = -886627392;    int nxEDGdopsr26645343 = 75956713;    int nxEDGdopsr99428134 = -552872831;    int nxEDGdopsr97415718 = -919903521;    int nxEDGdopsr29907508 = -699887726;    int nxEDGdopsr72174453 = -170007068;    int nxEDGdopsr74425179 = -286392768;    int nxEDGdopsr62657651 = -326366663;    int nxEDGdopsr82395906 = -751071803;    int nxEDGdopsr84866470 = -555203858;    int nxEDGdopsr46945883 = -823549987;    int nxEDGdopsr82226345 = -226143302;    int nxEDGdopsr46355182 = -50952694;    int nxEDGdopsr46463007 = -30406501;    int nxEDGdopsr60534324 = -678186469;    int nxEDGdopsr68730911 = -272188041;    int nxEDGdopsr71659306 = -330753754;    int nxEDGdopsr5898041 = -501627197;    int nxEDGdopsr78707848 = -484473112;    int nxEDGdopsr70738586 = -146489084;    int nxEDGdopsr78205957 = -823936904;    int nxEDGdopsr86027458 = -44349929;    int nxEDGdopsr17483812 = -669286382;    int nxEDGdopsr92545506 = -741528389;    int nxEDGdopsr42743342 = -935639056;    int nxEDGdopsr96448658 = -706602777;    int nxEDGdopsr84936214 = -521451000;    int nxEDGdopsr74739001 = -319671833;    int nxEDGdopsr69447173 = -694589990;    int nxEDGdopsr70614710 = -359159219;    int nxEDGdopsr19645778 = -898280184;    int nxEDGdopsr25898310 = -551929610;    int nxEDGdopsr16317117 = -721054037;    int nxEDGdopsr51923844 = -233154062;    int nxEDGdopsr23248233 = -381569968;    int nxEDGdopsr31922430 = -929722903;    int nxEDGdopsr74470532 = -115318502;    int nxEDGdopsr41413614 = -652794776;    int nxEDGdopsr38754437 = -790001525;     nxEDGdopsr32903801 = nxEDGdopsr87847539;     nxEDGdopsr87847539 = nxEDGdopsr12475015;     nxEDGdopsr12475015 = nxEDGdopsr69537393;     nxEDGdopsr69537393 = nxEDGdopsr9270773;     nxEDGdopsr9270773 = nxEDGdopsr37711537;     nxEDGdopsr37711537 = nxEDGdopsr56519192;     nxEDGdopsr56519192 = nxEDGdopsr7668515;     nxEDGdopsr7668515 = nxEDGdopsr10859430;     nxEDGdopsr10859430 = nxEDGdopsr98601235;     nxEDGdopsr98601235 = nxEDGdopsr64956933;     nxEDGdopsr64956933 = nxEDGdopsr67436373;     nxEDGdopsr67436373 = nxEDGdopsr45591896;     nxEDGdopsr45591896 = nxEDGdopsr22379048;     nxEDGdopsr22379048 = nxEDGdopsr30369518;     nxEDGdopsr30369518 = nxEDGdopsr42427277;     nxEDGdopsr42427277 = nxEDGdopsr72408513;     nxEDGdopsr72408513 = nxEDGdopsr6567215;     nxEDGdopsr6567215 = nxEDGdopsr64042365;     nxEDGdopsr64042365 = nxEDGdopsr79269576;     nxEDGdopsr79269576 = nxEDGdopsr19102097;     nxEDGdopsr19102097 = nxEDGdopsr42629577;     nxEDGdopsr42629577 = nxEDGdopsr10488639;     nxEDGdopsr10488639 = nxEDGdopsr74559985;     nxEDGdopsr74559985 = nxEDGdopsr69973072;     nxEDGdopsr69973072 = nxEDGdopsr1247020;     nxEDGdopsr1247020 = nxEDGdopsr71948199;     nxEDGdopsr71948199 = nxEDGdopsr46960856;     nxEDGdopsr46960856 = nxEDGdopsr81023171;     nxEDGdopsr81023171 = nxEDGdopsr11431296;     nxEDGdopsr11431296 = nxEDGdopsr1185517;     nxEDGdopsr1185517 = nxEDGdopsr35049426;     nxEDGdopsr35049426 = nxEDGdopsr95261920;     nxEDGdopsr95261920 = nxEDGdopsr71166717;     nxEDGdopsr71166717 = nxEDGdopsr59721397;     nxEDGdopsr59721397 = nxEDGdopsr47973611;     nxEDGdopsr47973611 = nxEDGdopsr57560807;     nxEDGdopsr57560807 = nxEDGdopsr25462630;     nxEDGdopsr25462630 = nxEDGdopsr24340869;     nxEDGdopsr24340869 = nxEDGdopsr17687183;     nxEDGdopsr17687183 = nxEDGdopsr32806569;     nxEDGdopsr32806569 = nxEDGdopsr58567773;     nxEDGdopsr58567773 = nxEDGdopsr73898666;     nxEDGdopsr73898666 = nxEDGdopsr38829333;     nxEDGdopsr38829333 = nxEDGdopsr68661945;     nxEDGdopsr68661945 = nxEDGdopsr91265223;     nxEDGdopsr91265223 = nxEDGdopsr30508434;     nxEDGdopsr30508434 = nxEDGdopsr93742241;     nxEDGdopsr93742241 = nxEDGdopsr60933398;     nxEDGdopsr60933398 = nxEDGdopsr63539360;     nxEDGdopsr63539360 = nxEDGdopsr18885789;     nxEDGdopsr18885789 = nxEDGdopsr58442174;     nxEDGdopsr58442174 = nxEDGdopsr38600767;     nxEDGdopsr38600767 = nxEDGdopsr10325706;     nxEDGdopsr10325706 = nxEDGdopsr96427715;     nxEDGdopsr96427715 = nxEDGdopsr90274224;     nxEDGdopsr90274224 = nxEDGdopsr77358901;     nxEDGdopsr77358901 = nxEDGdopsr37915029;     nxEDGdopsr37915029 = nxEDGdopsr99564320;     nxEDGdopsr99564320 = nxEDGdopsr8023753;     nxEDGdopsr8023753 = nxEDGdopsr65763338;     nxEDGdopsr65763338 = nxEDGdopsr9558336;     nxEDGdopsr9558336 = nxEDGdopsr26645343;     nxEDGdopsr26645343 = nxEDGdopsr99428134;     nxEDGdopsr99428134 = nxEDGdopsr97415718;     nxEDGdopsr97415718 = nxEDGdopsr29907508;     nxEDGdopsr29907508 = nxEDGdopsr72174453;     nxEDGdopsr72174453 = nxEDGdopsr74425179;     nxEDGdopsr74425179 = nxEDGdopsr62657651;     nxEDGdopsr62657651 = nxEDGdopsr82395906;     nxEDGdopsr82395906 = nxEDGdopsr84866470;     nxEDGdopsr84866470 = nxEDGdopsr46945883;     nxEDGdopsr46945883 = nxEDGdopsr82226345;     nxEDGdopsr82226345 = nxEDGdopsr46355182;     nxEDGdopsr46355182 = nxEDGdopsr46463007;     nxEDGdopsr46463007 = nxEDGdopsr60534324;     nxEDGdopsr60534324 = nxEDGdopsr68730911;     nxEDGdopsr68730911 = nxEDGdopsr71659306;     nxEDGdopsr71659306 = nxEDGdopsr5898041;     nxEDGdopsr5898041 = nxEDGdopsr78707848;     nxEDGdopsr78707848 = nxEDGdopsr70738586;     nxEDGdopsr70738586 = nxEDGdopsr78205957;     nxEDGdopsr78205957 = nxEDGdopsr86027458;     nxEDGdopsr86027458 = nxEDGdopsr17483812;     nxEDGdopsr17483812 = nxEDGdopsr92545506;     nxEDGdopsr92545506 = nxEDGdopsr42743342;     nxEDGdopsr42743342 = nxEDGdopsr96448658;     nxEDGdopsr96448658 = nxEDGdopsr84936214;     nxEDGdopsr84936214 = nxEDGdopsr74739001;     nxEDGdopsr74739001 = nxEDGdopsr69447173;     nxEDGdopsr69447173 = nxEDGdopsr70614710;     nxEDGdopsr70614710 = nxEDGdopsr19645778;     nxEDGdopsr19645778 = nxEDGdopsr25898310;     nxEDGdopsr25898310 = nxEDGdopsr16317117;     nxEDGdopsr16317117 = nxEDGdopsr51923844;     nxEDGdopsr51923844 = nxEDGdopsr23248233;     nxEDGdopsr23248233 = nxEDGdopsr31922430;     nxEDGdopsr31922430 = nxEDGdopsr74470532;     nxEDGdopsr74470532 = nxEDGdopsr41413614;     nxEDGdopsr41413614 = nxEDGdopsr38754437;     nxEDGdopsr38754437 = nxEDGdopsr32903801;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FJJowvoolJ33743154() {     int uRGlBjAUiL16348107 = -570667206;    int uRGlBjAUiL4719629 = -382466381;    int uRGlBjAUiL8883521 = -451594739;    int uRGlBjAUiL32329644 = -329887432;    int uRGlBjAUiL15392097 = 58534297;    int uRGlBjAUiL36215938 = 12267314;    int uRGlBjAUiL27558510 = 98462811;    int uRGlBjAUiL22015958 = 27543402;    int uRGlBjAUiL69389130 = -638969663;    int uRGlBjAUiL31146888 = 26451199;    int uRGlBjAUiL14372872 = -953139431;    int uRGlBjAUiL30641773 = -924922701;    int uRGlBjAUiL63709672 = -439474374;    int uRGlBjAUiL2005940 = -558869163;    int uRGlBjAUiL91223981 = -683886147;    int uRGlBjAUiL28816438 = -924948138;    int uRGlBjAUiL66160398 = -494505375;    int uRGlBjAUiL87418660 = -46528035;    int uRGlBjAUiL26482507 = -922965964;    int uRGlBjAUiL68637018 = -928189852;    int uRGlBjAUiL74742484 = -221481688;    int uRGlBjAUiL54776895 = -31531143;    int uRGlBjAUiL14314347 = 56658753;    int uRGlBjAUiL9266544 = -46510135;    int uRGlBjAUiL11549031 = -330059603;    int uRGlBjAUiL96589168 = -163082107;    int uRGlBjAUiL21402939 = -396561037;    int uRGlBjAUiL86692481 = -344953368;    int uRGlBjAUiL17104266 = -923760424;    int uRGlBjAUiL27264637 = -157487334;    int uRGlBjAUiL19280831 = -323026369;    int uRGlBjAUiL98155852 = -978330912;    int uRGlBjAUiL38800807 = -288169565;    int uRGlBjAUiL13325228 = -953022536;    int uRGlBjAUiL38448127 = -541876878;    int uRGlBjAUiL60362796 = -626815269;    int uRGlBjAUiL26174985 = -360514135;    int uRGlBjAUiL66041462 = -626080881;    int uRGlBjAUiL73335947 = -757977655;    int uRGlBjAUiL67182318 = -525931922;    int uRGlBjAUiL66455493 = -939746333;    int uRGlBjAUiL9055560 = -427674510;    int uRGlBjAUiL97465735 = -219248575;    int uRGlBjAUiL30734273 = -328881067;    int uRGlBjAUiL60606442 = -201214496;    int uRGlBjAUiL30871220 = -305544657;    int uRGlBjAUiL6523865 = -168387538;    int uRGlBjAUiL13412837 = -943261510;    int uRGlBjAUiL91780510 = -716250639;    int uRGlBjAUiL98871843 = -498989356;    int uRGlBjAUiL92143237 = -124010516;    int uRGlBjAUiL13942969 = -807889424;    int uRGlBjAUiL52713226 = -468280372;    int uRGlBjAUiL35002465 = -864259775;    int uRGlBjAUiL59000670 = -190234656;    int uRGlBjAUiL61571211 = -439136064;    int uRGlBjAUiL90405282 = -339125135;    int uRGlBjAUiL99616977 = -305084604;    int uRGlBjAUiL20780614 = -999827829;    int uRGlBjAUiL18802928 = -778383596;    int uRGlBjAUiL14812999 = -591171649;    int uRGlBjAUiL40866029 = -556583822;    int uRGlBjAUiL4911692 = -48696174;    int uRGlBjAUiL42124494 = -381482330;    int uRGlBjAUiL11866057 = -650522432;    int uRGlBjAUiL16217019 = -974808519;    int uRGlBjAUiL91840965 = -536753137;    int uRGlBjAUiL50384444 = -486451839;    int uRGlBjAUiL63557813 = 83007715;    int uRGlBjAUiL30861186 = 42929121;    int uRGlBjAUiL2641453 = -464434003;    int uRGlBjAUiL118936 = -868424494;    int uRGlBjAUiL14082714 = -288550381;    int uRGlBjAUiL59300189 = -297034042;    int uRGlBjAUiL2181526 = -988443520;    int uRGlBjAUiL65686925 = -793807178;    int uRGlBjAUiL57311159 = -812282568;    int uRGlBjAUiL83580073 = -614460180;    int uRGlBjAUiL48660102 = -845295640;    int uRGlBjAUiL80677810 = 75485053;    int uRGlBjAUiL90065304 = -994694570;    int uRGlBjAUiL7990103 = -453299528;    int uRGlBjAUiL94911971 = -628702730;    int uRGlBjAUiL18232422 = -324771069;    int uRGlBjAUiL35121400 = 66523182;    int uRGlBjAUiL5337862 = -515136945;    int uRGlBjAUiL45442627 = -410050541;    int uRGlBjAUiL3798343 = -423909790;    int uRGlBjAUiL54324558 = -662787880;    int uRGlBjAUiL76876916 = -2740815;    int uRGlBjAUiL69957513 = -187690135;    int uRGlBjAUiL26558008 = 44570468;    int uRGlBjAUiL45260848 = -626253053;    int uRGlBjAUiL54533020 = -979594059;    int uRGlBjAUiL52369319 = -934760274;    int uRGlBjAUiL25589465 = -283162511;    int uRGlBjAUiL4143868 = -278978337;    int uRGlBjAUiL55341242 = -837766245;    int uRGlBjAUiL18868216 = -678358636;    int uRGlBjAUiL44389423 = -570667206;     uRGlBjAUiL16348107 = uRGlBjAUiL4719629;     uRGlBjAUiL4719629 = uRGlBjAUiL8883521;     uRGlBjAUiL8883521 = uRGlBjAUiL32329644;     uRGlBjAUiL32329644 = uRGlBjAUiL15392097;     uRGlBjAUiL15392097 = uRGlBjAUiL36215938;     uRGlBjAUiL36215938 = uRGlBjAUiL27558510;     uRGlBjAUiL27558510 = uRGlBjAUiL22015958;     uRGlBjAUiL22015958 = uRGlBjAUiL69389130;     uRGlBjAUiL69389130 = uRGlBjAUiL31146888;     uRGlBjAUiL31146888 = uRGlBjAUiL14372872;     uRGlBjAUiL14372872 = uRGlBjAUiL30641773;     uRGlBjAUiL30641773 = uRGlBjAUiL63709672;     uRGlBjAUiL63709672 = uRGlBjAUiL2005940;     uRGlBjAUiL2005940 = uRGlBjAUiL91223981;     uRGlBjAUiL91223981 = uRGlBjAUiL28816438;     uRGlBjAUiL28816438 = uRGlBjAUiL66160398;     uRGlBjAUiL66160398 = uRGlBjAUiL87418660;     uRGlBjAUiL87418660 = uRGlBjAUiL26482507;     uRGlBjAUiL26482507 = uRGlBjAUiL68637018;     uRGlBjAUiL68637018 = uRGlBjAUiL74742484;     uRGlBjAUiL74742484 = uRGlBjAUiL54776895;     uRGlBjAUiL54776895 = uRGlBjAUiL14314347;     uRGlBjAUiL14314347 = uRGlBjAUiL9266544;     uRGlBjAUiL9266544 = uRGlBjAUiL11549031;     uRGlBjAUiL11549031 = uRGlBjAUiL96589168;     uRGlBjAUiL96589168 = uRGlBjAUiL21402939;     uRGlBjAUiL21402939 = uRGlBjAUiL86692481;     uRGlBjAUiL86692481 = uRGlBjAUiL17104266;     uRGlBjAUiL17104266 = uRGlBjAUiL27264637;     uRGlBjAUiL27264637 = uRGlBjAUiL19280831;     uRGlBjAUiL19280831 = uRGlBjAUiL98155852;     uRGlBjAUiL98155852 = uRGlBjAUiL38800807;     uRGlBjAUiL38800807 = uRGlBjAUiL13325228;     uRGlBjAUiL13325228 = uRGlBjAUiL38448127;     uRGlBjAUiL38448127 = uRGlBjAUiL60362796;     uRGlBjAUiL60362796 = uRGlBjAUiL26174985;     uRGlBjAUiL26174985 = uRGlBjAUiL66041462;     uRGlBjAUiL66041462 = uRGlBjAUiL73335947;     uRGlBjAUiL73335947 = uRGlBjAUiL67182318;     uRGlBjAUiL67182318 = uRGlBjAUiL66455493;     uRGlBjAUiL66455493 = uRGlBjAUiL9055560;     uRGlBjAUiL9055560 = uRGlBjAUiL97465735;     uRGlBjAUiL97465735 = uRGlBjAUiL30734273;     uRGlBjAUiL30734273 = uRGlBjAUiL60606442;     uRGlBjAUiL60606442 = uRGlBjAUiL30871220;     uRGlBjAUiL30871220 = uRGlBjAUiL6523865;     uRGlBjAUiL6523865 = uRGlBjAUiL13412837;     uRGlBjAUiL13412837 = uRGlBjAUiL91780510;     uRGlBjAUiL91780510 = uRGlBjAUiL98871843;     uRGlBjAUiL98871843 = uRGlBjAUiL92143237;     uRGlBjAUiL92143237 = uRGlBjAUiL13942969;     uRGlBjAUiL13942969 = uRGlBjAUiL52713226;     uRGlBjAUiL52713226 = uRGlBjAUiL35002465;     uRGlBjAUiL35002465 = uRGlBjAUiL59000670;     uRGlBjAUiL59000670 = uRGlBjAUiL61571211;     uRGlBjAUiL61571211 = uRGlBjAUiL90405282;     uRGlBjAUiL90405282 = uRGlBjAUiL99616977;     uRGlBjAUiL99616977 = uRGlBjAUiL20780614;     uRGlBjAUiL20780614 = uRGlBjAUiL18802928;     uRGlBjAUiL18802928 = uRGlBjAUiL14812999;     uRGlBjAUiL14812999 = uRGlBjAUiL40866029;     uRGlBjAUiL40866029 = uRGlBjAUiL4911692;     uRGlBjAUiL4911692 = uRGlBjAUiL42124494;     uRGlBjAUiL42124494 = uRGlBjAUiL11866057;     uRGlBjAUiL11866057 = uRGlBjAUiL16217019;     uRGlBjAUiL16217019 = uRGlBjAUiL91840965;     uRGlBjAUiL91840965 = uRGlBjAUiL50384444;     uRGlBjAUiL50384444 = uRGlBjAUiL63557813;     uRGlBjAUiL63557813 = uRGlBjAUiL30861186;     uRGlBjAUiL30861186 = uRGlBjAUiL2641453;     uRGlBjAUiL2641453 = uRGlBjAUiL118936;     uRGlBjAUiL118936 = uRGlBjAUiL14082714;     uRGlBjAUiL14082714 = uRGlBjAUiL59300189;     uRGlBjAUiL59300189 = uRGlBjAUiL2181526;     uRGlBjAUiL2181526 = uRGlBjAUiL65686925;     uRGlBjAUiL65686925 = uRGlBjAUiL57311159;     uRGlBjAUiL57311159 = uRGlBjAUiL83580073;     uRGlBjAUiL83580073 = uRGlBjAUiL48660102;     uRGlBjAUiL48660102 = uRGlBjAUiL80677810;     uRGlBjAUiL80677810 = uRGlBjAUiL90065304;     uRGlBjAUiL90065304 = uRGlBjAUiL7990103;     uRGlBjAUiL7990103 = uRGlBjAUiL94911971;     uRGlBjAUiL94911971 = uRGlBjAUiL18232422;     uRGlBjAUiL18232422 = uRGlBjAUiL35121400;     uRGlBjAUiL35121400 = uRGlBjAUiL5337862;     uRGlBjAUiL5337862 = uRGlBjAUiL45442627;     uRGlBjAUiL45442627 = uRGlBjAUiL3798343;     uRGlBjAUiL3798343 = uRGlBjAUiL54324558;     uRGlBjAUiL54324558 = uRGlBjAUiL76876916;     uRGlBjAUiL76876916 = uRGlBjAUiL69957513;     uRGlBjAUiL69957513 = uRGlBjAUiL26558008;     uRGlBjAUiL26558008 = uRGlBjAUiL45260848;     uRGlBjAUiL45260848 = uRGlBjAUiL54533020;     uRGlBjAUiL54533020 = uRGlBjAUiL52369319;     uRGlBjAUiL52369319 = uRGlBjAUiL25589465;     uRGlBjAUiL25589465 = uRGlBjAUiL4143868;     uRGlBjAUiL4143868 = uRGlBjAUiL55341242;     uRGlBjAUiL55341242 = uRGlBjAUiL18868216;     uRGlBjAUiL18868216 = uRGlBjAUiL44389423;     uRGlBjAUiL44389423 = uRGlBjAUiL16348107;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GVOIXLUKIZ4199028() {     int ddeXOXlVlb29134274 = -705707898;    int ddeXOXlVlb91490517 = -552800395;    int ddeXOXlVlb26636321 = -859954121;    int ddeXOXlVlb40270470 = -154089709;    int ddeXOXlVlb8903201 = -433282168;    int ddeXOXlVlb79526008 = -864655999;    int ddeXOXlVlb48766268 = -447209260;    int ddeXOXlVlb48735070 = -416974855;    int ddeXOXlVlb64793940 = -600727755;    int ddeXOXlVlb37365593 = -877636433;    int ddeXOXlVlb41860254 = 32174554;    int ddeXOXlVlb62592277 = -332255517;    int ddeXOXlVlb33852975 = -343257031;    int ddeXOXlVlb51630018 = -891284361;    int ddeXOXlVlb14530535 = -590871978;    int ddeXOXlVlb57241507 = -375839527;    int ddeXOXlVlb21912216 = -158279842;    int ddeXOXlVlb70977145 = -701738338;    int ddeXOXlVlb27643138 = -832435160;    int ddeXOXlVlb71291514 = -427433991;    int ddeXOXlVlb54916541 = -712797952;    int ddeXOXlVlb83824714 = -201389490;    int ddeXOXlVlb53074642 = -974345142;    int ddeXOXlVlb53773673 = -406855147;    int ddeXOXlVlb17443290 = -540501527;    int ddeXOXlVlb77907814 = -953513306;    int ddeXOXlVlb32818475 = 69732683;    int ddeXOXlVlb54045407 = -518751473;    int ddeXOXlVlb33721319 = -37977664;    int ddeXOXlVlb80015752 = -887373488;    int ddeXOXlVlb63716827 = -599443119;    int ddeXOXlVlb1474677 = -957050591;    int ddeXOXlVlb84846756 = -164024349;    int ddeXOXlVlb90401600 = -857776754;    int ddeXOXlVlb21508659 = -51908759;    int ddeXOXlVlb55586910 = -415378373;    int ddeXOXlVlb62939045 = -150423486;    int ddeXOXlVlb23745485 = -545700973;    int ddeXOXlVlb79111333 = -599828232;    int ddeXOXlVlb93447047 = -370348768;    int ddeXOXlVlb27886801 = -215220326;    int ddeXOXlVlb16758015 = -325985845;    int ddeXOXlVlb32349206 = -242682020;    int ddeXOXlVlb75362810 = -670053310;    int ddeXOXlVlb40614932 = -93984456;    int ddeXOXlVlb4499349 = -378863096;    int ddeXOXlVlb84105884 = -665902590;    int ddeXOXlVlb42852803 = 1385711;    int ddeXOXlVlb35844559 = -879852613;    int ddeXOXlVlb45822264 = -10025819;    int ddeXOXlVlb10470731 = -376894825;    int ddeXOXlVlb42119516 = -179074762;    int ddeXOXlVlb87198376 = -644283654;    int ddeXOXlVlb79702190 = -453711517;    int ddeXOXlVlb49282486 = -184679143;    int ddeXOXlVlb45309559 = -404318408;    int ddeXOXlVlb38415875 = -578455254;    int ddeXOXlVlb72862647 = -353098974;    int ddeXOXlVlb22827181 = -613588182;    int ddeXOXlVlb30995387 = -479768862;    int ddeXOXlVlb46707533 = -834388682;    int ddeXOXlVlb94720861 = -928457787;    int ddeXOXlVlb15013752 = -278997191;    int ddeXOXlVlb84778188 = -713354268;    int ddeXOXlVlb73648766 = -178193315;    int ddeXOXlVlb40385577 = -10774856;    int ddeXOXlVlb77745521 = -68231168;    int ddeXOXlVlb43451374 = -485480277;    int ddeXOXlVlb30121359 = -739375603;    int ddeXOXlVlb58943624 = -75493606;    int ddeXOXlVlb94302461 = -125416042;    int ddeXOXlVlb98166730 = -612578869;    int ddeXOXlVlb91865812 = -1910106;    int ddeXOXlVlb34196090 = -362086392;    int ddeXOXlVlb43404713 = -112213666;    int ddeXOXlVlb38158526 = -286812108;    int ddeXOXlVlb51475509 = -958707471;    int ddeXOXlVlb77711831 = -204291832;    int ddeXOXlVlb13158742 = -212870692;    int ddeXOXlVlb12943941 = -61638432;    int ddeXOXlVlb93801930 = -187610716;    int ddeXOXlVlb89965672 = -931653029;    int ddeXOXlVlb18200848 = -638898860;    int ddeXOXlVlb87899054 = 72048154;    int ddeXOXlVlb69545021 = -410478664;    int ddeXOXlVlb21597311 = -320368357;    int ddeXOXlVlb14276301 = -212766937;    int ddeXOXlVlb5144566 = -710312832;    int ddeXOXlVlb41119114 = -573097612;    int ddeXOXlVlb76199100 = -647590351;    int ddeXOXlVlb17171036 = -836923120;    int ddeXOXlVlb90076398 = -797324513;    int ddeXOXlVlb918305 = -932112792;    int ddeXOXlVlb48115946 = -20059371;    int ddeXOXlVlb46739514 = -535960086;    int ddeXOXlVlb33165940 = -286762539;    int ddeXOXlVlb1744264 = 53011346;    int ddeXOXlVlb47571017 = -529327753;    int ddeXOXlVlb1714045 = -391859996;    int ddeXOXlVlb229356 = -705707898;     ddeXOXlVlb29134274 = ddeXOXlVlb91490517;     ddeXOXlVlb91490517 = ddeXOXlVlb26636321;     ddeXOXlVlb26636321 = ddeXOXlVlb40270470;     ddeXOXlVlb40270470 = ddeXOXlVlb8903201;     ddeXOXlVlb8903201 = ddeXOXlVlb79526008;     ddeXOXlVlb79526008 = ddeXOXlVlb48766268;     ddeXOXlVlb48766268 = ddeXOXlVlb48735070;     ddeXOXlVlb48735070 = ddeXOXlVlb64793940;     ddeXOXlVlb64793940 = ddeXOXlVlb37365593;     ddeXOXlVlb37365593 = ddeXOXlVlb41860254;     ddeXOXlVlb41860254 = ddeXOXlVlb62592277;     ddeXOXlVlb62592277 = ddeXOXlVlb33852975;     ddeXOXlVlb33852975 = ddeXOXlVlb51630018;     ddeXOXlVlb51630018 = ddeXOXlVlb14530535;     ddeXOXlVlb14530535 = ddeXOXlVlb57241507;     ddeXOXlVlb57241507 = ddeXOXlVlb21912216;     ddeXOXlVlb21912216 = ddeXOXlVlb70977145;     ddeXOXlVlb70977145 = ddeXOXlVlb27643138;     ddeXOXlVlb27643138 = ddeXOXlVlb71291514;     ddeXOXlVlb71291514 = ddeXOXlVlb54916541;     ddeXOXlVlb54916541 = ddeXOXlVlb83824714;     ddeXOXlVlb83824714 = ddeXOXlVlb53074642;     ddeXOXlVlb53074642 = ddeXOXlVlb53773673;     ddeXOXlVlb53773673 = ddeXOXlVlb17443290;     ddeXOXlVlb17443290 = ddeXOXlVlb77907814;     ddeXOXlVlb77907814 = ddeXOXlVlb32818475;     ddeXOXlVlb32818475 = ddeXOXlVlb54045407;     ddeXOXlVlb54045407 = ddeXOXlVlb33721319;     ddeXOXlVlb33721319 = ddeXOXlVlb80015752;     ddeXOXlVlb80015752 = ddeXOXlVlb63716827;     ddeXOXlVlb63716827 = ddeXOXlVlb1474677;     ddeXOXlVlb1474677 = ddeXOXlVlb84846756;     ddeXOXlVlb84846756 = ddeXOXlVlb90401600;     ddeXOXlVlb90401600 = ddeXOXlVlb21508659;     ddeXOXlVlb21508659 = ddeXOXlVlb55586910;     ddeXOXlVlb55586910 = ddeXOXlVlb62939045;     ddeXOXlVlb62939045 = ddeXOXlVlb23745485;     ddeXOXlVlb23745485 = ddeXOXlVlb79111333;     ddeXOXlVlb79111333 = ddeXOXlVlb93447047;     ddeXOXlVlb93447047 = ddeXOXlVlb27886801;     ddeXOXlVlb27886801 = ddeXOXlVlb16758015;     ddeXOXlVlb16758015 = ddeXOXlVlb32349206;     ddeXOXlVlb32349206 = ddeXOXlVlb75362810;     ddeXOXlVlb75362810 = ddeXOXlVlb40614932;     ddeXOXlVlb40614932 = ddeXOXlVlb4499349;     ddeXOXlVlb4499349 = ddeXOXlVlb84105884;     ddeXOXlVlb84105884 = ddeXOXlVlb42852803;     ddeXOXlVlb42852803 = ddeXOXlVlb35844559;     ddeXOXlVlb35844559 = ddeXOXlVlb45822264;     ddeXOXlVlb45822264 = ddeXOXlVlb10470731;     ddeXOXlVlb10470731 = ddeXOXlVlb42119516;     ddeXOXlVlb42119516 = ddeXOXlVlb87198376;     ddeXOXlVlb87198376 = ddeXOXlVlb79702190;     ddeXOXlVlb79702190 = ddeXOXlVlb49282486;     ddeXOXlVlb49282486 = ddeXOXlVlb45309559;     ddeXOXlVlb45309559 = ddeXOXlVlb38415875;     ddeXOXlVlb38415875 = ddeXOXlVlb72862647;     ddeXOXlVlb72862647 = ddeXOXlVlb22827181;     ddeXOXlVlb22827181 = ddeXOXlVlb30995387;     ddeXOXlVlb30995387 = ddeXOXlVlb46707533;     ddeXOXlVlb46707533 = ddeXOXlVlb94720861;     ddeXOXlVlb94720861 = ddeXOXlVlb15013752;     ddeXOXlVlb15013752 = ddeXOXlVlb84778188;     ddeXOXlVlb84778188 = ddeXOXlVlb73648766;     ddeXOXlVlb73648766 = ddeXOXlVlb40385577;     ddeXOXlVlb40385577 = ddeXOXlVlb77745521;     ddeXOXlVlb77745521 = ddeXOXlVlb43451374;     ddeXOXlVlb43451374 = ddeXOXlVlb30121359;     ddeXOXlVlb30121359 = ddeXOXlVlb58943624;     ddeXOXlVlb58943624 = ddeXOXlVlb94302461;     ddeXOXlVlb94302461 = ddeXOXlVlb98166730;     ddeXOXlVlb98166730 = ddeXOXlVlb91865812;     ddeXOXlVlb91865812 = ddeXOXlVlb34196090;     ddeXOXlVlb34196090 = ddeXOXlVlb43404713;     ddeXOXlVlb43404713 = ddeXOXlVlb38158526;     ddeXOXlVlb38158526 = ddeXOXlVlb51475509;     ddeXOXlVlb51475509 = ddeXOXlVlb77711831;     ddeXOXlVlb77711831 = ddeXOXlVlb13158742;     ddeXOXlVlb13158742 = ddeXOXlVlb12943941;     ddeXOXlVlb12943941 = ddeXOXlVlb93801930;     ddeXOXlVlb93801930 = ddeXOXlVlb89965672;     ddeXOXlVlb89965672 = ddeXOXlVlb18200848;     ddeXOXlVlb18200848 = ddeXOXlVlb87899054;     ddeXOXlVlb87899054 = ddeXOXlVlb69545021;     ddeXOXlVlb69545021 = ddeXOXlVlb21597311;     ddeXOXlVlb21597311 = ddeXOXlVlb14276301;     ddeXOXlVlb14276301 = ddeXOXlVlb5144566;     ddeXOXlVlb5144566 = ddeXOXlVlb41119114;     ddeXOXlVlb41119114 = ddeXOXlVlb76199100;     ddeXOXlVlb76199100 = ddeXOXlVlb17171036;     ddeXOXlVlb17171036 = ddeXOXlVlb90076398;     ddeXOXlVlb90076398 = ddeXOXlVlb918305;     ddeXOXlVlb918305 = ddeXOXlVlb48115946;     ddeXOXlVlb48115946 = ddeXOXlVlb46739514;     ddeXOXlVlb46739514 = ddeXOXlVlb33165940;     ddeXOXlVlb33165940 = ddeXOXlVlb1744264;     ddeXOXlVlb1744264 = ddeXOXlVlb47571017;     ddeXOXlVlb47571017 = ddeXOXlVlb1714045;     ddeXOXlVlb1714045 = ddeXOXlVlb229356;     ddeXOXlVlb229356 = ddeXOXlVlb29134274;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PGhqyxCqCg22412369() {     int lybqKjBxZX12578580 = -486373578;    int lybqKjBxZX8362607 = -552182582;    int lybqKjBxZX23044828 = -653479348;    int lybqKjBxZX3062722 = -347889544;    int lybqKjBxZX15024525 = -240149659;    int lybqKjBxZX78030409 = -643727652;    int lybqKjBxZX19805586 = -187120573;    int lybqKjBxZX63082513 = -404132884;    int lybqKjBxZX23323641 = -942187106;    int lybqKjBxZX69911244 = -246012546;    int lybqKjBxZX91276191 = -157941319;    int lybqKjBxZX25797677 = -319873763;    int lybqKjBxZX51970751 = -134918272;    int lybqKjBxZX31256911 = -679574718;    int lybqKjBxZX75384997 = -684894960;    int lybqKjBxZX43630669 = -163918853;    int lybqKjBxZX15664101 = 88375131;    int lybqKjBxZX51828592 = -949043214;    int lybqKjBxZX90083279 = -831110030;    int lybqKjBxZX60658957 = -957019984;    int lybqKjBxZX10556928 = -302326983;    int lybqKjBxZX95972032 = -192541261;    int lybqKjBxZX56900349 = 85765661;    int lybqKjBxZX88480231 = -478680543;    int lybqKjBxZX59019247 = -154286;    int lybqKjBxZX73249963 = -834023022;    int lybqKjBxZX82273215 = -658351660;    int lybqKjBxZX93777032 = -588706357;    int lybqKjBxZX69802413 = 99517193;    int lybqKjBxZX95849093 = -300223340;    int lybqKjBxZX81812140 = -237200320;    int lybqKjBxZX64581103 = -872245671;    int lybqKjBxZX28385643 = -884896526;    int lybqKjBxZX32560111 = -449378924;    int lybqKjBxZX235390 = -249573494;    int lybqKjBxZX67976094 = -203402279;    int lybqKjBxZX31553224 = -29272666;    int lybqKjBxZX64324317 = -254171493;    int lybqKjBxZX28106412 = -784726030;    int lybqKjBxZX42942183 = -122942290;    int lybqKjBxZX61535725 = -886769299;    int lybqKjBxZX67245801 = -899894166;    int lybqKjBxZX55916275 = -793739263;    int lybqKjBxZX67267751 = -426236081;    int lybqKjBxZX32559429 = -922141409;    int lybqKjBxZX44105345 = -398474020;    int lybqKjBxZX60121314 = -798206820;    int lybqKjBxZX62523398 = 2663992;    int lybqKjBxZX66691671 = -365454696;    int lybqKjBxZX81154748 = -217046274;    int lybqKjBxZX83728178 = -597796247;    int lybqKjBxZX97620310 = -237334074;    int lybqKjBxZX1310835 = -856030971;    int lybqKjBxZX4378950 = -172124905;    int lybqKjBxZX11855441 = -533165266;    int lybqKjBxZX16606547 = -193832318;    int lybqKjBxZX51462257 = -537948244;    int lybqKjBxZX34564596 = -74798805;    int lybqKjBxZX44043474 = -247735259;    int lybqKjBxZX41774562 = -406126637;    int lybqKjBxZX95757194 = -985375993;    int lybqKjBxZX26028554 = -598414217;    int lybqKjBxZX93280100 = -403650078;    int lybqKjBxZX27474548 = -541963767;    int lybqKjBxZX88099104 = 91187774;    int lybqKjBxZX26695088 = -285695649;    int lybqKjBxZX97412033 = -434977237;    int lybqKjBxZX19410640 = -685539348;    int lybqKjBxZX31021521 = -330001225;    int lybqKjBxZX7408903 = -381492682;    int lybqKjBxZX12077445 = -34646187;    int lybqKjBxZX51339783 = -657453376;    int lybqKjBxZX23722181 = -64317185;    int lybqKjBxZX47141097 = -608167740;    int lybqKjBxZX99123231 = 29749315;    int lybqKjBxZX43311127 = -402432818;    int lybqKjBxZX40055757 = -398801999;    int lybqKjBxZX89632598 = -487998258;    int lybqKjBxZX55920803 = -556539135;    int lybqKjBxZX14913903 = -601680267;    int lybqKjBxZX13128649 = 64183797;    int lybqKjBxZX19749817 = -561015653;    int lybqKjBxZX27085361 = -123251661;    int lybqKjBxZX88647664 = -683436533;    int lybqKjBxZX12120915 = -702427093;    int lybqKjBxZX84191830 = -999866247;    int lybqKjBxZX63270268 = 83785300;    int lybqKjBxZX24006694 = -612771622;    int lybqKjBxZX20704671 = -916213659;    int lybqKjBxZX83628843 = 44258824;    int lybqKjBxZX16513838 = -665454036;    int lybqKjBxZX96988628 = -954473861;    int lybqKjBxZX20280844 = 93563765;    int lybqKjBxZX86331849 = -278599393;    int lybqKjBxZX47184989 = -137566298;    int lybqKjBxZX35507172 = -188355083;    int lybqKjBxZX73965701 = -396244089;    int lybqKjBxZX28441727 = -151775496;    int lybqKjBxZX79168646 = -417423855;    int lybqKjBxZX5864342 = -486373578;     lybqKjBxZX12578580 = lybqKjBxZX8362607;     lybqKjBxZX8362607 = lybqKjBxZX23044828;     lybqKjBxZX23044828 = lybqKjBxZX3062722;     lybqKjBxZX3062722 = lybqKjBxZX15024525;     lybqKjBxZX15024525 = lybqKjBxZX78030409;     lybqKjBxZX78030409 = lybqKjBxZX19805586;     lybqKjBxZX19805586 = lybqKjBxZX63082513;     lybqKjBxZX63082513 = lybqKjBxZX23323641;     lybqKjBxZX23323641 = lybqKjBxZX69911244;     lybqKjBxZX69911244 = lybqKjBxZX91276191;     lybqKjBxZX91276191 = lybqKjBxZX25797677;     lybqKjBxZX25797677 = lybqKjBxZX51970751;     lybqKjBxZX51970751 = lybqKjBxZX31256911;     lybqKjBxZX31256911 = lybqKjBxZX75384997;     lybqKjBxZX75384997 = lybqKjBxZX43630669;     lybqKjBxZX43630669 = lybqKjBxZX15664101;     lybqKjBxZX15664101 = lybqKjBxZX51828592;     lybqKjBxZX51828592 = lybqKjBxZX90083279;     lybqKjBxZX90083279 = lybqKjBxZX60658957;     lybqKjBxZX60658957 = lybqKjBxZX10556928;     lybqKjBxZX10556928 = lybqKjBxZX95972032;     lybqKjBxZX95972032 = lybqKjBxZX56900349;     lybqKjBxZX56900349 = lybqKjBxZX88480231;     lybqKjBxZX88480231 = lybqKjBxZX59019247;     lybqKjBxZX59019247 = lybqKjBxZX73249963;     lybqKjBxZX73249963 = lybqKjBxZX82273215;     lybqKjBxZX82273215 = lybqKjBxZX93777032;     lybqKjBxZX93777032 = lybqKjBxZX69802413;     lybqKjBxZX69802413 = lybqKjBxZX95849093;     lybqKjBxZX95849093 = lybqKjBxZX81812140;     lybqKjBxZX81812140 = lybqKjBxZX64581103;     lybqKjBxZX64581103 = lybqKjBxZX28385643;     lybqKjBxZX28385643 = lybqKjBxZX32560111;     lybqKjBxZX32560111 = lybqKjBxZX235390;     lybqKjBxZX235390 = lybqKjBxZX67976094;     lybqKjBxZX67976094 = lybqKjBxZX31553224;     lybqKjBxZX31553224 = lybqKjBxZX64324317;     lybqKjBxZX64324317 = lybqKjBxZX28106412;     lybqKjBxZX28106412 = lybqKjBxZX42942183;     lybqKjBxZX42942183 = lybqKjBxZX61535725;     lybqKjBxZX61535725 = lybqKjBxZX67245801;     lybqKjBxZX67245801 = lybqKjBxZX55916275;     lybqKjBxZX55916275 = lybqKjBxZX67267751;     lybqKjBxZX67267751 = lybqKjBxZX32559429;     lybqKjBxZX32559429 = lybqKjBxZX44105345;     lybqKjBxZX44105345 = lybqKjBxZX60121314;     lybqKjBxZX60121314 = lybqKjBxZX62523398;     lybqKjBxZX62523398 = lybqKjBxZX66691671;     lybqKjBxZX66691671 = lybqKjBxZX81154748;     lybqKjBxZX81154748 = lybqKjBxZX83728178;     lybqKjBxZX83728178 = lybqKjBxZX97620310;     lybqKjBxZX97620310 = lybqKjBxZX1310835;     lybqKjBxZX1310835 = lybqKjBxZX4378950;     lybqKjBxZX4378950 = lybqKjBxZX11855441;     lybqKjBxZX11855441 = lybqKjBxZX16606547;     lybqKjBxZX16606547 = lybqKjBxZX51462257;     lybqKjBxZX51462257 = lybqKjBxZX34564596;     lybqKjBxZX34564596 = lybqKjBxZX44043474;     lybqKjBxZX44043474 = lybqKjBxZX41774562;     lybqKjBxZX41774562 = lybqKjBxZX95757194;     lybqKjBxZX95757194 = lybqKjBxZX26028554;     lybqKjBxZX26028554 = lybqKjBxZX93280100;     lybqKjBxZX93280100 = lybqKjBxZX27474548;     lybqKjBxZX27474548 = lybqKjBxZX88099104;     lybqKjBxZX88099104 = lybqKjBxZX26695088;     lybqKjBxZX26695088 = lybqKjBxZX97412033;     lybqKjBxZX97412033 = lybqKjBxZX19410640;     lybqKjBxZX19410640 = lybqKjBxZX31021521;     lybqKjBxZX31021521 = lybqKjBxZX7408903;     lybqKjBxZX7408903 = lybqKjBxZX12077445;     lybqKjBxZX12077445 = lybqKjBxZX51339783;     lybqKjBxZX51339783 = lybqKjBxZX23722181;     lybqKjBxZX23722181 = lybqKjBxZX47141097;     lybqKjBxZX47141097 = lybqKjBxZX99123231;     lybqKjBxZX99123231 = lybqKjBxZX43311127;     lybqKjBxZX43311127 = lybqKjBxZX40055757;     lybqKjBxZX40055757 = lybqKjBxZX89632598;     lybqKjBxZX89632598 = lybqKjBxZX55920803;     lybqKjBxZX55920803 = lybqKjBxZX14913903;     lybqKjBxZX14913903 = lybqKjBxZX13128649;     lybqKjBxZX13128649 = lybqKjBxZX19749817;     lybqKjBxZX19749817 = lybqKjBxZX27085361;     lybqKjBxZX27085361 = lybqKjBxZX88647664;     lybqKjBxZX88647664 = lybqKjBxZX12120915;     lybqKjBxZX12120915 = lybqKjBxZX84191830;     lybqKjBxZX84191830 = lybqKjBxZX63270268;     lybqKjBxZX63270268 = lybqKjBxZX24006694;     lybqKjBxZX24006694 = lybqKjBxZX20704671;     lybqKjBxZX20704671 = lybqKjBxZX83628843;     lybqKjBxZX83628843 = lybqKjBxZX16513838;     lybqKjBxZX16513838 = lybqKjBxZX96988628;     lybqKjBxZX96988628 = lybqKjBxZX20280844;     lybqKjBxZX20280844 = lybqKjBxZX86331849;     lybqKjBxZX86331849 = lybqKjBxZX47184989;     lybqKjBxZX47184989 = lybqKjBxZX35507172;     lybqKjBxZX35507172 = lybqKjBxZX73965701;     lybqKjBxZX73965701 = lybqKjBxZX28441727;     lybqKjBxZX28441727 = lybqKjBxZX79168646;     lybqKjBxZX79168646 = lybqKjBxZX5864342;     lybqKjBxZX5864342 = lybqKjBxZX12578580;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YfnXgFLqlf92868241() {     int QJEsiXExKY25364747 = -621414270;    int QJEsiXExKY95133494 = -722516596;    int QJEsiXExKY40797628 = 38161270;    int QJEsiXExKY11003547 = -172091821;    int QJEsiXExKY8535630 = -731966124;    int QJEsiXExKY21340480 = -420650965;    int QJEsiXExKY41013344 = -732792644;    int QJEsiXExKY89801625 = -848651141;    int QJEsiXExKY18728452 = -903945199;    int QJEsiXExKY76129949 = -50100178;    int QJEsiXExKY18763574 = -272627334;    int QJEsiXExKY57748182 = -827206579;    int QJEsiXExKY22114054 = -38700929;    int QJEsiXExKY80880989 = 88010084;    int QJEsiXExKY98691550 = -591880791;    int QJEsiXExKY72055737 = -714810242;    int QJEsiXExKY71415918 = -675399335;    int QJEsiXExKY35387077 = -504253516;    int QJEsiXExKY91243910 = -740579226;    int QJEsiXExKY63313453 = -456264123;    int QJEsiXExKY90730984 = -793643248;    int QJEsiXExKY25019852 = -362399609;    int QJEsiXExKY95660645 = -945238234;    int QJEsiXExKY32987361 = -839025556;    int QJEsiXExKY64913507 = -210596211;    int QJEsiXExKY54568608 = -524454221;    int QJEsiXExKY93688751 = -192057940;    int QJEsiXExKY61129957 = -762504461;    int QJEsiXExKY86419466 = -114700047;    int QJEsiXExKY48600208 = 69890506;    int QJEsiXExKY26248137 = -513617070;    int QJEsiXExKY67899927 = -850965349;    int QJEsiXExKY74431592 = -760751311;    int QJEsiXExKY9636484 = -354133142;    int QJEsiXExKY83295921 = -859605374;    int QJEsiXExKY63200209 = 8034617;    int QJEsiXExKY68317284 = -919182016;    int QJEsiXExKY22028340 = -173791584;    int QJEsiXExKY33881798 = -626576607;    int QJEsiXExKY69206912 = 32640864;    int QJEsiXExKY22967034 = -162243292;    int QJEsiXExKY74948257 = -798205500;    int QJEsiXExKY90799745 = -817172707;    int QJEsiXExKY11896289 = -767408324;    int QJEsiXExKY12567920 = -814911369;    int QJEsiXExKY17733475 = -471792460;    int QJEsiXExKY37703334 = -195721872;    int QJEsiXExKY91963364 = -152688787;    int QJEsiXExKY10755721 = -529056671;    int QJEsiXExKY28105169 = -828082737;    int QJEsiXExKY2055673 = -850680555;    int QJEsiXExKY25796858 = -708519412;    int QJEsiXExKY35795985 = 67965747;    int QJEsiXExKY49078674 = -861576646;    int QJEsiXExKY2137257 = -527609752;    int QJEsiXExKY344895 = -159014662;    int QJEsiXExKY99472849 = -777278363;    int QJEsiXExKY7810267 = -122813175;    int QJEsiXExKY46090040 = -961495611;    int QJEsiXExKY53967021 = -107511903;    int QJEsiXExKY27651729 = -128593026;    int QJEsiXExKY79883387 = -970288183;    int QJEsiXExKY3382160 = -633951095;    int QJEsiXExKY70128243 = -873835705;    int QJEsiXExKY49881813 = -536483109;    int QJEsiXExKY50863647 = -421661986;    int QJEsiXExKY83316589 = 33544731;    int QJEsiXExKY12477570 = -684567787;    int QJEsiXExKY97585067 = -52384542;    int QJEsiXExKY35491341 = -499915408;    int QJEsiXExKY3738453 = -795628226;    int QJEsiXExKY49387578 = -401607752;    int QJEsiXExKY1505280 = -877676910;    int QJEsiXExKY22036998 = -673220090;    int QJEsiXExKY40346420 = -194020831;    int QJEsiXExKY15782728 = -995437748;    int QJEsiXExKY34220107 = -545226902;    int QJEsiXExKY83764356 = -77829910;    int QJEsiXExKY20419442 = 75885813;    int QJEsiXExKY47180032 = -738803752;    int QJEsiXExKY16865275 = -228732349;    int QJEsiXExKY1725387 = 60630847;    int QJEsiXExKY50374236 = -133447791;    int QJEsiXExKY58314297 = -286617310;    int QJEsiXExKY46544536 = -79428940;    int QJEsiXExKY451279 = -805097659;    int QJEsiXExKY32103942 = -818931096;    int QJEsiXExKY25352918 = -899174665;    int QJEsiXExKY7499228 = -826523391;    int QJEsiXExKY82951027 = -600590713;    int QJEsiXExKY63727360 = -214687021;    int QJEsiXExKY60507018 = -696368842;    int QJEsiXExKY75938300 = -212295974;    int QJEsiXExKY79914776 = -419064704;    int QJEsiXExKY41555184 = -838766111;    int QJEsiXExKY43083646 = -191955110;    int QJEsiXExKY71566097 = -64254406;    int QJEsiXExKY20671502 = -943337003;    int QJEsiXExKY62014475 = -130925215;    int QJEsiXExKY61704273 = -621414270;     QJEsiXExKY25364747 = QJEsiXExKY95133494;     QJEsiXExKY95133494 = QJEsiXExKY40797628;     QJEsiXExKY40797628 = QJEsiXExKY11003547;     QJEsiXExKY11003547 = QJEsiXExKY8535630;     QJEsiXExKY8535630 = QJEsiXExKY21340480;     QJEsiXExKY21340480 = QJEsiXExKY41013344;     QJEsiXExKY41013344 = QJEsiXExKY89801625;     QJEsiXExKY89801625 = QJEsiXExKY18728452;     QJEsiXExKY18728452 = QJEsiXExKY76129949;     QJEsiXExKY76129949 = QJEsiXExKY18763574;     QJEsiXExKY18763574 = QJEsiXExKY57748182;     QJEsiXExKY57748182 = QJEsiXExKY22114054;     QJEsiXExKY22114054 = QJEsiXExKY80880989;     QJEsiXExKY80880989 = QJEsiXExKY98691550;     QJEsiXExKY98691550 = QJEsiXExKY72055737;     QJEsiXExKY72055737 = QJEsiXExKY71415918;     QJEsiXExKY71415918 = QJEsiXExKY35387077;     QJEsiXExKY35387077 = QJEsiXExKY91243910;     QJEsiXExKY91243910 = QJEsiXExKY63313453;     QJEsiXExKY63313453 = QJEsiXExKY90730984;     QJEsiXExKY90730984 = QJEsiXExKY25019852;     QJEsiXExKY25019852 = QJEsiXExKY95660645;     QJEsiXExKY95660645 = QJEsiXExKY32987361;     QJEsiXExKY32987361 = QJEsiXExKY64913507;     QJEsiXExKY64913507 = QJEsiXExKY54568608;     QJEsiXExKY54568608 = QJEsiXExKY93688751;     QJEsiXExKY93688751 = QJEsiXExKY61129957;     QJEsiXExKY61129957 = QJEsiXExKY86419466;     QJEsiXExKY86419466 = QJEsiXExKY48600208;     QJEsiXExKY48600208 = QJEsiXExKY26248137;     QJEsiXExKY26248137 = QJEsiXExKY67899927;     QJEsiXExKY67899927 = QJEsiXExKY74431592;     QJEsiXExKY74431592 = QJEsiXExKY9636484;     QJEsiXExKY9636484 = QJEsiXExKY83295921;     QJEsiXExKY83295921 = QJEsiXExKY63200209;     QJEsiXExKY63200209 = QJEsiXExKY68317284;     QJEsiXExKY68317284 = QJEsiXExKY22028340;     QJEsiXExKY22028340 = QJEsiXExKY33881798;     QJEsiXExKY33881798 = QJEsiXExKY69206912;     QJEsiXExKY69206912 = QJEsiXExKY22967034;     QJEsiXExKY22967034 = QJEsiXExKY74948257;     QJEsiXExKY74948257 = QJEsiXExKY90799745;     QJEsiXExKY90799745 = QJEsiXExKY11896289;     QJEsiXExKY11896289 = QJEsiXExKY12567920;     QJEsiXExKY12567920 = QJEsiXExKY17733475;     QJEsiXExKY17733475 = QJEsiXExKY37703334;     QJEsiXExKY37703334 = QJEsiXExKY91963364;     QJEsiXExKY91963364 = QJEsiXExKY10755721;     QJEsiXExKY10755721 = QJEsiXExKY28105169;     QJEsiXExKY28105169 = QJEsiXExKY2055673;     QJEsiXExKY2055673 = QJEsiXExKY25796858;     QJEsiXExKY25796858 = QJEsiXExKY35795985;     QJEsiXExKY35795985 = QJEsiXExKY49078674;     QJEsiXExKY49078674 = QJEsiXExKY2137257;     QJEsiXExKY2137257 = QJEsiXExKY344895;     QJEsiXExKY344895 = QJEsiXExKY99472849;     QJEsiXExKY99472849 = QJEsiXExKY7810267;     QJEsiXExKY7810267 = QJEsiXExKY46090040;     QJEsiXExKY46090040 = QJEsiXExKY53967021;     QJEsiXExKY53967021 = QJEsiXExKY27651729;     QJEsiXExKY27651729 = QJEsiXExKY79883387;     QJEsiXExKY79883387 = QJEsiXExKY3382160;     QJEsiXExKY3382160 = QJEsiXExKY70128243;     QJEsiXExKY70128243 = QJEsiXExKY49881813;     QJEsiXExKY49881813 = QJEsiXExKY50863647;     QJEsiXExKY50863647 = QJEsiXExKY83316589;     QJEsiXExKY83316589 = QJEsiXExKY12477570;     QJEsiXExKY12477570 = QJEsiXExKY97585067;     QJEsiXExKY97585067 = QJEsiXExKY35491341;     QJEsiXExKY35491341 = QJEsiXExKY3738453;     QJEsiXExKY3738453 = QJEsiXExKY49387578;     QJEsiXExKY49387578 = QJEsiXExKY1505280;     QJEsiXExKY1505280 = QJEsiXExKY22036998;     QJEsiXExKY22036998 = QJEsiXExKY40346420;     QJEsiXExKY40346420 = QJEsiXExKY15782728;     QJEsiXExKY15782728 = QJEsiXExKY34220107;     QJEsiXExKY34220107 = QJEsiXExKY83764356;     QJEsiXExKY83764356 = QJEsiXExKY20419442;     QJEsiXExKY20419442 = QJEsiXExKY47180032;     QJEsiXExKY47180032 = QJEsiXExKY16865275;     QJEsiXExKY16865275 = QJEsiXExKY1725387;     QJEsiXExKY1725387 = QJEsiXExKY50374236;     QJEsiXExKY50374236 = QJEsiXExKY58314297;     QJEsiXExKY58314297 = QJEsiXExKY46544536;     QJEsiXExKY46544536 = QJEsiXExKY451279;     QJEsiXExKY451279 = QJEsiXExKY32103942;     QJEsiXExKY32103942 = QJEsiXExKY25352918;     QJEsiXExKY25352918 = QJEsiXExKY7499228;     QJEsiXExKY7499228 = QJEsiXExKY82951027;     QJEsiXExKY82951027 = QJEsiXExKY63727360;     QJEsiXExKY63727360 = QJEsiXExKY60507018;     QJEsiXExKY60507018 = QJEsiXExKY75938300;     QJEsiXExKY75938300 = QJEsiXExKY79914776;     QJEsiXExKY79914776 = QJEsiXExKY41555184;     QJEsiXExKY41555184 = QJEsiXExKY43083646;     QJEsiXExKY43083646 = QJEsiXExKY71566097;     QJEsiXExKY71566097 = QJEsiXExKY20671502;     QJEsiXExKY20671502 = QJEsiXExKY62014475;     QJEsiXExKY62014475 = QJEsiXExKY61704273;     QJEsiXExKY61704273 = QJEsiXExKY25364747;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aVmxuqeovP11081584() {     int XzMnZltGif8809052 = -402079951;    int XzMnZltGif12005584 = -721898784;    int XzMnZltGif37206135 = -855363958;    int XzMnZltGif73795798 = -365891657;    int XzMnZltGif14656954 = -538833615;    int XzMnZltGif19844881 = -199722618;    int XzMnZltGif12052662 = -472703957;    int XzMnZltGif4149069 = -835809170;    int XzMnZltGif77258152 = -145404550;    int XzMnZltGif8675602 = -518476291;    int XzMnZltGif68179511 = -462743207;    int XzMnZltGif20953581 = -814824826;    int XzMnZltGif40231830 = -930362170;    int XzMnZltGif60507881 = -800280273;    int XzMnZltGif59546014 = -685903773;    int XzMnZltGif58444899 = -502889567;    int XzMnZltGif65167803 = -428744362;    int XzMnZltGif16238524 = -751558393;    int XzMnZltGif53684052 = -739254096;    int XzMnZltGif52680896 = -985850116;    int XzMnZltGif46371372 = -383172279;    int XzMnZltGif37167170 = -353551379;    int XzMnZltGif99486352 = -985127431;    int XzMnZltGif67693919 = -910850952;    int XzMnZltGif6489465 = -770248970;    int XzMnZltGif49910757 = -404963937;    int XzMnZltGif43143492 = -920142282;    int XzMnZltGif861583 = -832459345;    int XzMnZltGif22500560 = 22794811;    int XzMnZltGif64433550 = -442959346;    int XzMnZltGif44343450 = -151374271;    int XzMnZltGif31006354 = -766160429;    int XzMnZltGif17970479 = -381623488;    int XzMnZltGif51794995 = 54264688;    int XzMnZltGif62022652 = 42729891;    int XzMnZltGif75589393 = -879989289;    int XzMnZltGif36931463 = -798031196;    int XzMnZltGif62607172 = -982262104;    int XzMnZltGif82876875 = -811474404;    int XzMnZltGif18702048 = -819952658;    int XzMnZltGif56615958 = -833792266;    int XzMnZltGif25436043 = -272113822;    int XzMnZltGif14366815 = -268229950;    int XzMnZltGif3801229 = -523591095;    int XzMnZltGif4512417 = -543068323;    int XzMnZltGif57339470 = -491403383;    int XzMnZltGif13718764 = -328026102;    int XzMnZltGif11633961 = -151410506;    int XzMnZltGif41602833 = -14658753;    int XzMnZltGif63437652 = 64896808;    int XzMnZltGif75313120 = 28418023;    int XzMnZltGif81297652 = -766778723;    int XzMnZltGif49908444 = -143781570;    int XzMnZltGif73755433 = -579990034;    int XzMnZltGif64710211 = -876095875;    int XzMnZltGif71641882 = 51471428;    int XzMnZltGif12519231 = -736771353;    int XzMnZltGif69512215 = -944513007;    int XzMnZltGif67306333 = -595642688;    int XzMnZltGif64746196 = -33869678;    int XzMnZltGif76701389 = -279580337;    int XzMnZltGif11191080 = -640244612;    int XzMnZltGif81648508 = -758603982;    int XzMnZltGif12824603 = -702445204;    int XzMnZltGif64332151 = -267102020;    int XzMnZltGif37173157 = -696582778;    int XzMnZltGif2983103 = -333201338;    int XzMnZltGif88436835 = -884626858;    int XzMnZltGif98485229 = -743010164;    int XzMnZltGif83956620 = -805914485;    int XzMnZltGif21513436 = -704858372;    int XzMnZltGif2560631 = -446482258;    int XzMnZltGif33361648 = -940083989;    int XzMnZltGif34982005 = -919301439;    int XzMnZltGif96064937 = -52057851;    int XzMnZltGif20935329 = -11058457;    int XzMnZltGif22800355 = 14678571;    int XzMnZltGif95685124 = -361536336;    int XzMnZltGif63181503 = -267782630;    int XzMnZltGif49149995 = -178845587;    int XzMnZltGif36191994 = 23062165;    int XzMnZltGif31509532 = -668731777;    int XzMnZltGif59258749 = -717800593;    int XzMnZltGif59062907 = 57898003;    int XzMnZltGif89120429 = -371377369;    int XzMnZltGif63045798 = -384595548;    int XzMnZltGif81097910 = -522378860;    int XzMnZltGif44215046 = -801633455;    int XzMnZltGif87084783 = -69639438;    int XzMnZltGif90380770 = 91258463;    int XzMnZltGif63070163 = -43217936;    int XzMnZltGif67419248 = -853518190;    int XzMnZltGif95300838 = -286619417;    int XzMnZltGif18130680 = -677604726;    int XzMnZltGif42000658 = -440372322;    int XzMnZltGif45424878 = -93547654;    int XzMnZltGif43787535 = -513509841;    int XzMnZltGif1542213 = -565784746;    int XzMnZltGif39469078 = -156489075;    int XzMnZltGif67339259 = -402079951;     XzMnZltGif8809052 = XzMnZltGif12005584;     XzMnZltGif12005584 = XzMnZltGif37206135;     XzMnZltGif37206135 = XzMnZltGif73795798;     XzMnZltGif73795798 = XzMnZltGif14656954;     XzMnZltGif14656954 = XzMnZltGif19844881;     XzMnZltGif19844881 = XzMnZltGif12052662;     XzMnZltGif12052662 = XzMnZltGif4149069;     XzMnZltGif4149069 = XzMnZltGif77258152;     XzMnZltGif77258152 = XzMnZltGif8675602;     XzMnZltGif8675602 = XzMnZltGif68179511;     XzMnZltGif68179511 = XzMnZltGif20953581;     XzMnZltGif20953581 = XzMnZltGif40231830;     XzMnZltGif40231830 = XzMnZltGif60507881;     XzMnZltGif60507881 = XzMnZltGif59546014;     XzMnZltGif59546014 = XzMnZltGif58444899;     XzMnZltGif58444899 = XzMnZltGif65167803;     XzMnZltGif65167803 = XzMnZltGif16238524;     XzMnZltGif16238524 = XzMnZltGif53684052;     XzMnZltGif53684052 = XzMnZltGif52680896;     XzMnZltGif52680896 = XzMnZltGif46371372;     XzMnZltGif46371372 = XzMnZltGif37167170;     XzMnZltGif37167170 = XzMnZltGif99486352;     XzMnZltGif99486352 = XzMnZltGif67693919;     XzMnZltGif67693919 = XzMnZltGif6489465;     XzMnZltGif6489465 = XzMnZltGif49910757;     XzMnZltGif49910757 = XzMnZltGif43143492;     XzMnZltGif43143492 = XzMnZltGif861583;     XzMnZltGif861583 = XzMnZltGif22500560;     XzMnZltGif22500560 = XzMnZltGif64433550;     XzMnZltGif64433550 = XzMnZltGif44343450;     XzMnZltGif44343450 = XzMnZltGif31006354;     XzMnZltGif31006354 = XzMnZltGif17970479;     XzMnZltGif17970479 = XzMnZltGif51794995;     XzMnZltGif51794995 = XzMnZltGif62022652;     XzMnZltGif62022652 = XzMnZltGif75589393;     XzMnZltGif75589393 = XzMnZltGif36931463;     XzMnZltGif36931463 = XzMnZltGif62607172;     XzMnZltGif62607172 = XzMnZltGif82876875;     XzMnZltGif82876875 = XzMnZltGif18702048;     XzMnZltGif18702048 = XzMnZltGif56615958;     XzMnZltGif56615958 = XzMnZltGif25436043;     XzMnZltGif25436043 = XzMnZltGif14366815;     XzMnZltGif14366815 = XzMnZltGif3801229;     XzMnZltGif3801229 = XzMnZltGif4512417;     XzMnZltGif4512417 = XzMnZltGif57339470;     XzMnZltGif57339470 = XzMnZltGif13718764;     XzMnZltGif13718764 = XzMnZltGif11633961;     XzMnZltGif11633961 = XzMnZltGif41602833;     XzMnZltGif41602833 = XzMnZltGif63437652;     XzMnZltGif63437652 = XzMnZltGif75313120;     XzMnZltGif75313120 = XzMnZltGif81297652;     XzMnZltGif81297652 = XzMnZltGif49908444;     XzMnZltGif49908444 = XzMnZltGif73755433;     XzMnZltGif73755433 = XzMnZltGif64710211;     XzMnZltGif64710211 = XzMnZltGif71641882;     XzMnZltGif71641882 = XzMnZltGif12519231;     XzMnZltGif12519231 = XzMnZltGif69512215;     XzMnZltGif69512215 = XzMnZltGif67306333;     XzMnZltGif67306333 = XzMnZltGif64746196;     XzMnZltGif64746196 = XzMnZltGif76701389;     XzMnZltGif76701389 = XzMnZltGif11191080;     XzMnZltGif11191080 = XzMnZltGif81648508;     XzMnZltGif81648508 = XzMnZltGif12824603;     XzMnZltGif12824603 = XzMnZltGif64332151;     XzMnZltGif64332151 = XzMnZltGif37173157;     XzMnZltGif37173157 = XzMnZltGif2983103;     XzMnZltGif2983103 = XzMnZltGif88436835;     XzMnZltGif88436835 = XzMnZltGif98485229;     XzMnZltGif98485229 = XzMnZltGif83956620;     XzMnZltGif83956620 = XzMnZltGif21513436;     XzMnZltGif21513436 = XzMnZltGif2560631;     XzMnZltGif2560631 = XzMnZltGif33361648;     XzMnZltGif33361648 = XzMnZltGif34982005;     XzMnZltGif34982005 = XzMnZltGif96064937;     XzMnZltGif96064937 = XzMnZltGif20935329;     XzMnZltGif20935329 = XzMnZltGif22800355;     XzMnZltGif22800355 = XzMnZltGif95685124;     XzMnZltGif95685124 = XzMnZltGif63181503;     XzMnZltGif63181503 = XzMnZltGif49149995;     XzMnZltGif49149995 = XzMnZltGif36191994;     XzMnZltGif36191994 = XzMnZltGif31509532;     XzMnZltGif31509532 = XzMnZltGif59258749;     XzMnZltGif59258749 = XzMnZltGif59062907;     XzMnZltGif59062907 = XzMnZltGif89120429;     XzMnZltGif89120429 = XzMnZltGif63045798;     XzMnZltGif63045798 = XzMnZltGif81097910;     XzMnZltGif81097910 = XzMnZltGif44215046;     XzMnZltGif44215046 = XzMnZltGif87084783;     XzMnZltGif87084783 = XzMnZltGif90380770;     XzMnZltGif90380770 = XzMnZltGif63070163;     XzMnZltGif63070163 = XzMnZltGif67419248;     XzMnZltGif67419248 = XzMnZltGif95300838;     XzMnZltGif95300838 = XzMnZltGif18130680;     XzMnZltGif18130680 = XzMnZltGif42000658;     XzMnZltGif42000658 = XzMnZltGif45424878;     XzMnZltGif45424878 = XzMnZltGif43787535;     XzMnZltGif43787535 = XzMnZltGif1542213;     XzMnZltGif1542213 = XzMnZltGif39469078;     XzMnZltGif39469078 = XzMnZltGif67339259;     XzMnZltGif67339259 = XzMnZltGif8809052;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZTOQoQMzhH81207259() {     int BEHvlSkaJr76256088 = -2124085;    int BEHvlSkaJr7201775 = -329202486;    int BEHvlSkaJr14378369 = -591979339;    int BEHvlSkaJr68091704 = -375655515;    int BEHvlSkaJr73779626 = -234729998;    int BEHvlSkaJr42523916 = -406363956;    int BEHvlSkaJr85813788 = -515732233;    int BEHvlSkaJr90829233 = -976718342;    int BEHvlSkaJr77697209 = -440369942;    int BEHvlSkaJr886778 = -964558322;    int BEHvlSkaJr53957583 = -143313722;    int BEHvlSkaJr1377123 = -673103368;    int BEHvlSkaJr42339533 = -616026657;    int BEHvlSkaJr49254170 = -399645998;    int BEHvlSkaJr67904531 = -537298384;    int BEHvlSkaJr2072957 = -351144870;    int BEHvlSkaJr64898624 = -746504088;    int BEHvlSkaJr44393063 = 26738460;    int BEHvlSkaJr35637014 = -316552573;    int BEHvlSkaJr896185 = -218435950;    int BEHvlSkaJr81050392 = -632105320;    int BEHvlSkaJr39171312 = -179861952;    int BEHvlSkaJr37838083 = 56083096;    int BEHvlSkaJr69979309 = 10683742;    int BEHvlSkaJr79693650 = -926910493;    int BEHvlSkaJr55896273 = -190897992;    int BEHvlSkaJr86327370 = -707893129;    int BEHvlSkaJr86059982 = -405342322;    int BEHvlSkaJr66336843 = 74403010;    int BEHvlSkaJr35530204 = -688172095;    int BEHvlSkaJr7072297 = -589570312;    int BEHvlSkaJr80592930 = -764554874;    int BEHvlSkaJr46219881 = -612051671;    int BEHvlSkaJr23244423 = -753928608;    int BEHvlSkaJr97229302 = -525851324;    int BEHvlSkaJr15311861 = -836782244;    int BEHvlSkaJr36458644 = -338713789;    int BEHvlSkaJr63370754 = -780548538;    int BEHvlSkaJr4108314 = -322592167;    int BEHvlSkaJr97080279 = -713246417;    int BEHvlSkaJr86150999 = -133872519;    int BEHvlSkaJr84115496 = -584165161;    int BEHvlSkaJr79967107 = -673038459;    int BEHvlSkaJr98192267 = -781478560;    int BEHvlSkaJr60486918 = -95096479;    int BEHvlSkaJr45873233 = -821466767;    int BEHvlSkaJr80076702 = -371317916;    int BEHvlSkaJr89117654 = -775654301;    int BEHvlSkaJr9351260 = -514227055;    int BEHvlSkaJr35184313 = -99134063;    int BEHvlSkaJr55494783 = -303126780;    int BEHvlSkaJr84309091 = -270884296;    int BEHvlSkaJr40673249 = -167646301;    int BEHvlSkaJr65620645 = -633408409;    int BEHvlSkaJr156867 = -409549765;    int BEHvlSkaJr37084776 = -822262134;    int BEHvlSkaJr69363691 = -285285582;    int BEHvlSkaJr44399059 = -502663082;    int BEHvlSkaJr88398054 = -448745023;    int BEHvlSkaJr17883354 = 56167994;    int BEHvlSkaJr56196546 = -698470828;    int BEHvlSkaJr99753805 = -10389911;    int BEHvlSkaJr24492391 = -951121353;    int BEHvlSkaJr42167005 = -752197848;    int BEHvlSkaJr93814481 = -274988010;    int BEHvlSkaJr73364652 = -378758849;    int BEHvlSkaJr55157241 = 38948303;    int BEHvlSkaJr19095111 = -862098050;    int BEHvlSkaJr52024867 = -873794674;    int BEHvlSkaJr52592670 = -700516140;    int BEHvlSkaJr65614312 = 87568918;    int BEHvlSkaJr1527871 = -965955551;    int BEHvlSkaJr40284749 = -650669374;    int BEHvlSkaJr38556735 = -603306156;    int BEHvlSkaJr14745186 = 15436568;    int BEHvlSkaJr96934895 = 52059840;    int BEHvlSkaJr59204205 = -506823493;    int BEHvlSkaJr39645816 = -162438345;    int BEHvlSkaJr9492392 = -894219779;    int BEHvlSkaJr33820418 = -5443727;    int BEHvlSkaJr75819570 = -819580077;    int BEHvlSkaJr97209715 = -932238828;    int BEHvlSkaJr76708722 = -891115267;    int BEHvlSkaJr31152531 = -826462927;    int BEHvlSkaJr80035420 = -285045315;    int BEHvlSkaJr22763205 = -218686017;    int BEHvlSkaJr39919682 = -496908573;    int BEHvlSkaJr80599236 = -978643262;    int BEHvlSkaJr23087557 = -244378843;    int BEHvlSkaJr60144527 = -703589191;    int BEHvlSkaJr45948170 = -451496662;    int BEHvlSkaJr92059584 = -836050708;    int BEHvlSkaJr74972700 = -231803516;    int BEHvlSkaJr86224960 = -278760162;    int BEHvlSkaJr40883734 = 85224410;    int BEHvlSkaJr86397194 = -23482608;    int BEHvlSkaJr59623106 = -633043808;    int BEHvlSkaJr37800103 = -920840611;    int BEHvlSkaJr4377786 = -406490550;    int BEHvlSkaJr87122265 = -2124085;     BEHvlSkaJr76256088 = BEHvlSkaJr7201775;     BEHvlSkaJr7201775 = BEHvlSkaJr14378369;     BEHvlSkaJr14378369 = BEHvlSkaJr68091704;     BEHvlSkaJr68091704 = BEHvlSkaJr73779626;     BEHvlSkaJr73779626 = BEHvlSkaJr42523916;     BEHvlSkaJr42523916 = BEHvlSkaJr85813788;     BEHvlSkaJr85813788 = BEHvlSkaJr90829233;     BEHvlSkaJr90829233 = BEHvlSkaJr77697209;     BEHvlSkaJr77697209 = BEHvlSkaJr886778;     BEHvlSkaJr886778 = BEHvlSkaJr53957583;     BEHvlSkaJr53957583 = BEHvlSkaJr1377123;     BEHvlSkaJr1377123 = BEHvlSkaJr42339533;     BEHvlSkaJr42339533 = BEHvlSkaJr49254170;     BEHvlSkaJr49254170 = BEHvlSkaJr67904531;     BEHvlSkaJr67904531 = BEHvlSkaJr2072957;     BEHvlSkaJr2072957 = BEHvlSkaJr64898624;     BEHvlSkaJr64898624 = BEHvlSkaJr44393063;     BEHvlSkaJr44393063 = BEHvlSkaJr35637014;     BEHvlSkaJr35637014 = BEHvlSkaJr896185;     BEHvlSkaJr896185 = BEHvlSkaJr81050392;     BEHvlSkaJr81050392 = BEHvlSkaJr39171312;     BEHvlSkaJr39171312 = BEHvlSkaJr37838083;     BEHvlSkaJr37838083 = BEHvlSkaJr69979309;     BEHvlSkaJr69979309 = BEHvlSkaJr79693650;     BEHvlSkaJr79693650 = BEHvlSkaJr55896273;     BEHvlSkaJr55896273 = BEHvlSkaJr86327370;     BEHvlSkaJr86327370 = BEHvlSkaJr86059982;     BEHvlSkaJr86059982 = BEHvlSkaJr66336843;     BEHvlSkaJr66336843 = BEHvlSkaJr35530204;     BEHvlSkaJr35530204 = BEHvlSkaJr7072297;     BEHvlSkaJr7072297 = BEHvlSkaJr80592930;     BEHvlSkaJr80592930 = BEHvlSkaJr46219881;     BEHvlSkaJr46219881 = BEHvlSkaJr23244423;     BEHvlSkaJr23244423 = BEHvlSkaJr97229302;     BEHvlSkaJr97229302 = BEHvlSkaJr15311861;     BEHvlSkaJr15311861 = BEHvlSkaJr36458644;     BEHvlSkaJr36458644 = BEHvlSkaJr63370754;     BEHvlSkaJr63370754 = BEHvlSkaJr4108314;     BEHvlSkaJr4108314 = BEHvlSkaJr97080279;     BEHvlSkaJr97080279 = BEHvlSkaJr86150999;     BEHvlSkaJr86150999 = BEHvlSkaJr84115496;     BEHvlSkaJr84115496 = BEHvlSkaJr79967107;     BEHvlSkaJr79967107 = BEHvlSkaJr98192267;     BEHvlSkaJr98192267 = BEHvlSkaJr60486918;     BEHvlSkaJr60486918 = BEHvlSkaJr45873233;     BEHvlSkaJr45873233 = BEHvlSkaJr80076702;     BEHvlSkaJr80076702 = BEHvlSkaJr89117654;     BEHvlSkaJr89117654 = BEHvlSkaJr9351260;     BEHvlSkaJr9351260 = BEHvlSkaJr35184313;     BEHvlSkaJr35184313 = BEHvlSkaJr55494783;     BEHvlSkaJr55494783 = BEHvlSkaJr84309091;     BEHvlSkaJr84309091 = BEHvlSkaJr40673249;     BEHvlSkaJr40673249 = BEHvlSkaJr65620645;     BEHvlSkaJr65620645 = BEHvlSkaJr156867;     BEHvlSkaJr156867 = BEHvlSkaJr37084776;     BEHvlSkaJr37084776 = BEHvlSkaJr69363691;     BEHvlSkaJr69363691 = BEHvlSkaJr44399059;     BEHvlSkaJr44399059 = BEHvlSkaJr88398054;     BEHvlSkaJr88398054 = BEHvlSkaJr17883354;     BEHvlSkaJr17883354 = BEHvlSkaJr56196546;     BEHvlSkaJr56196546 = BEHvlSkaJr99753805;     BEHvlSkaJr99753805 = BEHvlSkaJr24492391;     BEHvlSkaJr24492391 = BEHvlSkaJr42167005;     BEHvlSkaJr42167005 = BEHvlSkaJr93814481;     BEHvlSkaJr93814481 = BEHvlSkaJr73364652;     BEHvlSkaJr73364652 = BEHvlSkaJr55157241;     BEHvlSkaJr55157241 = BEHvlSkaJr19095111;     BEHvlSkaJr19095111 = BEHvlSkaJr52024867;     BEHvlSkaJr52024867 = BEHvlSkaJr52592670;     BEHvlSkaJr52592670 = BEHvlSkaJr65614312;     BEHvlSkaJr65614312 = BEHvlSkaJr1527871;     BEHvlSkaJr1527871 = BEHvlSkaJr40284749;     BEHvlSkaJr40284749 = BEHvlSkaJr38556735;     BEHvlSkaJr38556735 = BEHvlSkaJr14745186;     BEHvlSkaJr14745186 = BEHvlSkaJr96934895;     BEHvlSkaJr96934895 = BEHvlSkaJr59204205;     BEHvlSkaJr59204205 = BEHvlSkaJr39645816;     BEHvlSkaJr39645816 = BEHvlSkaJr9492392;     BEHvlSkaJr9492392 = BEHvlSkaJr33820418;     BEHvlSkaJr33820418 = BEHvlSkaJr75819570;     BEHvlSkaJr75819570 = BEHvlSkaJr97209715;     BEHvlSkaJr97209715 = BEHvlSkaJr76708722;     BEHvlSkaJr76708722 = BEHvlSkaJr31152531;     BEHvlSkaJr31152531 = BEHvlSkaJr80035420;     BEHvlSkaJr80035420 = BEHvlSkaJr22763205;     BEHvlSkaJr22763205 = BEHvlSkaJr39919682;     BEHvlSkaJr39919682 = BEHvlSkaJr80599236;     BEHvlSkaJr80599236 = BEHvlSkaJr23087557;     BEHvlSkaJr23087557 = BEHvlSkaJr60144527;     BEHvlSkaJr60144527 = BEHvlSkaJr45948170;     BEHvlSkaJr45948170 = BEHvlSkaJr92059584;     BEHvlSkaJr92059584 = BEHvlSkaJr74972700;     BEHvlSkaJr74972700 = BEHvlSkaJr86224960;     BEHvlSkaJr86224960 = BEHvlSkaJr40883734;     BEHvlSkaJr40883734 = BEHvlSkaJr86397194;     BEHvlSkaJr86397194 = BEHvlSkaJr59623106;     BEHvlSkaJr59623106 = BEHvlSkaJr37800103;     BEHvlSkaJr37800103 = BEHvlSkaJr4377786;     BEHvlSkaJr4377786 = BEHvlSkaJr87122265;     BEHvlSkaJr87122265 = BEHvlSkaJr76256088;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pVmdsgfmJS51663132() {     int DAVSXlTqjn89042255 = -137164777;    int DAVSXlTqjn93972662 = -499536499;    int DAVSXlTqjn32131169 = 99661279;    int DAVSXlTqjn76032530 = -199857792;    int DAVSXlTqjn67290731 = -726546463;    int DAVSXlTqjn85833987 = -183287270;    int DAVSXlTqjn7021547 = 38595697;    int DAVSXlTqjn17548347 = -321236599;    int DAVSXlTqjn73102019 = -402128035;    int DAVSXlTqjn7105483 = -768645954;    int DAVSXlTqjn81444965 = -257999738;    int DAVSXlTqjn33327627 = -80436184;    int DAVSXlTqjn12482837 = -519809314;    int DAVSXlTqjn98878248 = -732061196;    int DAVSXlTqjn91211084 = -444284214;    int DAVSXlTqjn30498025 = -902036259;    int DAVSXlTqjn20650442 = -410278555;    int DAVSXlTqjn27951548 = -628471843;    int DAVSXlTqjn36797644 = -226021768;    int DAVSXlTqjn3550681 = -817680089;    int DAVSXlTqjn61224449 = -23421584;    int DAVSXlTqjn68219131 = -349720300;    int DAVSXlTqjn76598378 = -974920799;    int DAVSXlTqjn14486439 = -349661270;    int DAVSXlTqjn85587909 = -37352417;    int DAVSXlTqjn37214918 = -981329191;    int DAVSXlTqjn97742906 = -241599409;    int DAVSXlTqjn53412908 = -579140427;    int DAVSXlTqjn82953896 = -139814230;    int DAVSXlTqjn88281319 = -318058250;    int DAVSXlTqjn51508292 = -865987062;    int DAVSXlTqjn83911754 = -743274552;    int DAVSXlTqjn92265830 = -487906455;    int DAVSXlTqjn320796 = -658682826;    int DAVSXlTqjn80289835 = -35883205;    int DAVSXlTqjn10535976 = -625345348;    int DAVSXlTqjn73222704 = -128623139;    int DAVSXlTqjn21074777 = -700168629;    int DAVSXlTqjn9883700 = -164442744;    int DAVSXlTqjn23345009 = -557663263;    int DAVSXlTqjn47582308 = -509346512;    int DAVSXlTqjn91817952 = -482476495;    int DAVSXlTqjn14850578 = -696471904;    int DAVSXlTqjn42820805 = -22650803;    int DAVSXlTqjn40495408 = 12133561;    int DAVSXlTqjn19501363 = -894785206;    int DAVSXlTqjn57658722 = -868832968;    int DAVSXlTqjn18557622 = -931007081;    int DAVSXlTqjn53415309 = -677829030;    int DAVSXlTqjn82134733 = -710170526;    int DAVSXlTqjn73822277 = -556011089;    int DAVSXlTqjn12485639 = -742069634;    int DAVSXlTqjn75158399 = -343649584;    int DAVSXlTqjn10320371 = -222860151;    int DAVSXlTqjn90438682 = -403994251;    int DAVSXlTqjn20823124 = -787444478;    int DAVSXlTqjn17374284 = -524615701;    int DAVSXlTqjn17644730 = -550677451;    int DAVSXlTqjn90444620 = -62505375;    int DAVSXlTqjn30075813 = -745217272;    int DAVSXlTqjn88091080 = -941687861;    int DAVSXlTqjn53608639 = -382263877;    int DAVSXlTqjn34594450 = -81422370;    int DAVSXlTqjn84820700 = 15930214;    int DAVSXlTqjn55597191 = -902658893;    int DAVSXlTqjn97533211 = -514725186;    int DAVSXlTqjn41061797 = -592529729;    int DAVSXlTqjn12162041 = -861126489;    int DAVSXlTqjn18588414 = -596177992;    int DAVSXlTqjn80675108 = -818938867;    int DAVSXlTqjn57275321 = -673413120;    int DAVSXlTqjn99575665 = -710109926;    int DAVSXlTqjn18067849 = -364029099;    int DAVSXlTqjn13452636 = -668358506;    int DAVSXlTqjn55968373 = -208333577;    int DAVSXlTqjn69406496 = -540945090;    int DAVSXlTqjn53368554 = -653248396;    int DAVSXlTqjn33777573 = -852269997;    int DAVSXlTqjn73991031 = -261794832;    int DAVSXlTqjn66086547 = -142567211;    int DAVSXlTqjn79556196 = -12496224;    int DAVSXlTqjn79185285 = -310592329;    int DAVSXlTqjn99997598 = -901311397;    int DAVSXlTqjn819164 = -429643704;    int DAVSXlTqjn14459043 = -762047161;    int DAVSXlTqjn39022653 = -23917429;    int DAVSXlTqjn8753356 = -299624969;    int DAVSXlTqjn81945460 = -165046305;    int DAVSXlTqjn9882113 = -154688575;    int DAVSXlTqjn59466711 = -248438728;    int DAVSXlTqjn93161691 = -729647;    int DAVSXlTqjn55577974 = -577945689;    int DAVSXlTqjn30630157 = -537663255;    int DAVSXlTqjn79807887 = -419225473;    int DAVSXlTqjn35253929 = -615975402;    int DAVSXlTqjn93973668 = -27082635;    int DAVSXlTqjn57223502 = -301054125;    int DAVSXlTqjn30029877 = -612402119;    int DAVSXlTqjn87223614 = -119991910;    int DAVSXlTqjn42962197 = -137164777;     DAVSXlTqjn89042255 = DAVSXlTqjn93972662;     DAVSXlTqjn93972662 = DAVSXlTqjn32131169;     DAVSXlTqjn32131169 = DAVSXlTqjn76032530;     DAVSXlTqjn76032530 = DAVSXlTqjn67290731;     DAVSXlTqjn67290731 = DAVSXlTqjn85833987;     DAVSXlTqjn85833987 = DAVSXlTqjn7021547;     DAVSXlTqjn7021547 = DAVSXlTqjn17548347;     DAVSXlTqjn17548347 = DAVSXlTqjn73102019;     DAVSXlTqjn73102019 = DAVSXlTqjn7105483;     DAVSXlTqjn7105483 = DAVSXlTqjn81444965;     DAVSXlTqjn81444965 = DAVSXlTqjn33327627;     DAVSXlTqjn33327627 = DAVSXlTqjn12482837;     DAVSXlTqjn12482837 = DAVSXlTqjn98878248;     DAVSXlTqjn98878248 = DAVSXlTqjn91211084;     DAVSXlTqjn91211084 = DAVSXlTqjn30498025;     DAVSXlTqjn30498025 = DAVSXlTqjn20650442;     DAVSXlTqjn20650442 = DAVSXlTqjn27951548;     DAVSXlTqjn27951548 = DAVSXlTqjn36797644;     DAVSXlTqjn36797644 = DAVSXlTqjn3550681;     DAVSXlTqjn3550681 = DAVSXlTqjn61224449;     DAVSXlTqjn61224449 = DAVSXlTqjn68219131;     DAVSXlTqjn68219131 = DAVSXlTqjn76598378;     DAVSXlTqjn76598378 = DAVSXlTqjn14486439;     DAVSXlTqjn14486439 = DAVSXlTqjn85587909;     DAVSXlTqjn85587909 = DAVSXlTqjn37214918;     DAVSXlTqjn37214918 = DAVSXlTqjn97742906;     DAVSXlTqjn97742906 = DAVSXlTqjn53412908;     DAVSXlTqjn53412908 = DAVSXlTqjn82953896;     DAVSXlTqjn82953896 = DAVSXlTqjn88281319;     DAVSXlTqjn88281319 = DAVSXlTqjn51508292;     DAVSXlTqjn51508292 = DAVSXlTqjn83911754;     DAVSXlTqjn83911754 = DAVSXlTqjn92265830;     DAVSXlTqjn92265830 = DAVSXlTqjn320796;     DAVSXlTqjn320796 = DAVSXlTqjn80289835;     DAVSXlTqjn80289835 = DAVSXlTqjn10535976;     DAVSXlTqjn10535976 = DAVSXlTqjn73222704;     DAVSXlTqjn73222704 = DAVSXlTqjn21074777;     DAVSXlTqjn21074777 = DAVSXlTqjn9883700;     DAVSXlTqjn9883700 = DAVSXlTqjn23345009;     DAVSXlTqjn23345009 = DAVSXlTqjn47582308;     DAVSXlTqjn47582308 = DAVSXlTqjn91817952;     DAVSXlTqjn91817952 = DAVSXlTqjn14850578;     DAVSXlTqjn14850578 = DAVSXlTqjn42820805;     DAVSXlTqjn42820805 = DAVSXlTqjn40495408;     DAVSXlTqjn40495408 = DAVSXlTqjn19501363;     DAVSXlTqjn19501363 = DAVSXlTqjn57658722;     DAVSXlTqjn57658722 = DAVSXlTqjn18557622;     DAVSXlTqjn18557622 = DAVSXlTqjn53415309;     DAVSXlTqjn53415309 = DAVSXlTqjn82134733;     DAVSXlTqjn82134733 = DAVSXlTqjn73822277;     DAVSXlTqjn73822277 = DAVSXlTqjn12485639;     DAVSXlTqjn12485639 = DAVSXlTqjn75158399;     DAVSXlTqjn75158399 = DAVSXlTqjn10320371;     DAVSXlTqjn10320371 = DAVSXlTqjn90438682;     DAVSXlTqjn90438682 = DAVSXlTqjn20823124;     DAVSXlTqjn20823124 = DAVSXlTqjn17374284;     DAVSXlTqjn17374284 = DAVSXlTqjn17644730;     DAVSXlTqjn17644730 = DAVSXlTqjn90444620;     DAVSXlTqjn90444620 = DAVSXlTqjn30075813;     DAVSXlTqjn30075813 = DAVSXlTqjn88091080;     DAVSXlTqjn88091080 = DAVSXlTqjn53608639;     DAVSXlTqjn53608639 = DAVSXlTqjn34594450;     DAVSXlTqjn34594450 = DAVSXlTqjn84820700;     DAVSXlTqjn84820700 = DAVSXlTqjn55597191;     DAVSXlTqjn55597191 = DAVSXlTqjn97533211;     DAVSXlTqjn97533211 = DAVSXlTqjn41061797;     DAVSXlTqjn41061797 = DAVSXlTqjn12162041;     DAVSXlTqjn12162041 = DAVSXlTqjn18588414;     DAVSXlTqjn18588414 = DAVSXlTqjn80675108;     DAVSXlTqjn80675108 = DAVSXlTqjn57275321;     DAVSXlTqjn57275321 = DAVSXlTqjn99575665;     DAVSXlTqjn99575665 = DAVSXlTqjn18067849;     DAVSXlTqjn18067849 = DAVSXlTqjn13452636;     DAVSXlTqjn13452636 = DAVSXlTqjn55968373;     DAVSXlTqjn55968373 = DAVSXlTqjn69406496;     DAVSXlTqjn69406496 = DAVSXlTqjn53368554;     DAVSXlTqjn53368554 = DAVSXlTqjn33777573;     DAVSXlTqjn33777573 = DAVSXlTqjn73991031;     DAVSXlTqjn73991031 = DAVSXlTqjn66086547;     DAVSXlTqjn66086547 = DAVSXlTqjn79556196;     DAVSXlTqjn79556196 = DAVSXlTqjn79185285;     DAVSXlTqjn79185285 = DAVSXlTqjn99997598;     DAVSXlTqjn99997598 = DAVSXlTqjn819164;     DAVSXlTqjn819164 = DAVSXlTqjn14459043;     DAVSXlTqjn14459043 = DAVSXlTqjn39022653;     DAVSXlTqjn39022653 = DAVSXlTqjn8753356;     DAVSXlTqjn8753356 = DAVSXlTqjn81945460;     DAVSXlTqjn81945460 = DAVSXlTqjn9882113;     DAVSXlTqjn9882113 = DAVSXlTqjn59466711;     DAVSXlTqjn59466711 = DAVSXlTqjn93161691;     DAVSXlTqjn93161691 = DAVSXlTqjn55577974;     DAVSXlTqjn55577974 = DAVSXlTqjn30630157;     DAVSXlTqjn30630157 = DAVSXlTqjn79807887;     DAVSXlTqjn79807887 = DAVSXlTqjn35253929;     DAVSXlTqjn35253929 = DAVSXlTqjn93973668;     DAVSXlTqjn93973668 = DAVSXlTqjn57223502;     DAVSXlTqjn57223502 = DAVSXlTqjn30029877;     DAVSXlTqjn30029877 = DAVSXlTqjn87223614;     DAVSXlTqjn87223614 = DAVSXlTqjn42962197;     DAVSXlTqjn42962197 = DAVSXlTqjn89042255;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PWYcyNRFKt69876473() {     int jXWpQXshdG72486560 = 82169542;    int jXWpQXshdG10844752 = -498918687;    int jXWpQXshdG28539676 = -793863949;    int jXWpQXshdG38824782 = -393657628;    int jXWpQXshdG73412055 = -533413954;    int jXWpQXshdG84338387 = 37641077;    int jXWpQXshdG78060864 = -801315617;    int jXWpQXshdG31895789 = -308394629;    int jXWpQXshdG31631720 = -743587386;    int jXWpQXshdG39651135 = -137022067;    int jXWpQXshdG30860903 = -448115610;    int jXWpQXshdG96533026 = -68054430;    int jXWpQXshdG30600612 = -311470555;    int jXWpQXshdG78505141 = -520351553;    int jXWpQXshdG52065548 = -538307196;    int jXWpQXshdG16887187 = -690115585;    int jXWpQXshdG14402327 = -163623582;    int jXWpQXshdG8802995 = -875776719;    int jXWpQXshdG99237786 = -224696639;    int jXWpQXshdG92918123 = -247266081;    int jXWpQXshdG16864837 = -712950615;    int jXWpQXshdG80366449 = -340872070;    int jXWpQXshdG80424086 = 85190004;    int jXWpQXshdG49192997 = -421486666;    int jXWpQXshdG27163868 = -597005176;    int jXWpQXshdG32557067 = -861838907;    int jXWpQXshdG47197647 = -969683751;    int jXWpQXshdG93144533 = -649095311;    int jXWpQXshdG19034991 = -2319372;    int jXWpQXshdG4114661 = -830908101;    int jXWpQXshdG69603606 = -503744263;    int jXWpQXshdG47018181 = -658469632;    int jXWpQXshdG35804718 = -108778633;    int jXWpQXshdG42479306 = -250284996;    int jXWpQXshdG59016565 = -233547940;    int jXWpQXshdG22925160 = -413369254;    int jXWpQXshdG41836883 = -7472319;    int jXWpQXshdG61653609 = -408639149;    int jXWpQXshdG58878778 = -349340542;    int jXWpQXshdG72840144 = -310256785;    int jXWpQXshdG81231232 = -80895485;    int jXWpQXshdG42305739 = 43615184;    int jXWpQXshdG38417647 = -147529146;    int jXWpQXshdG34725746 = -878833574;    int jXWpQXshdG32439905 = -816023393;    int jXWpQXshdG59107358 = -914396130;    int jXWpQXshdG33674153 = 98862802;    int jXWpQXshdG38228217 = -929728799;    int jXWpQXshdG84262421 = -163431112;    int jXWpQXshdG17467217 = -917190981;    int jXWpQXshdG47079725 = -776912511;    int jXWpQXshdG67986433 = -800328945;    int jXWpQXshdG89270857 = -555396900;    int jXWpQXshdG34997129 = 58726462;    int jXWpQXshdG53011637 = -752480374;    int jXWpQXshdG92120110 = -576958388;    int jXWpQXshdG30420666 = -484108691;    int jXWpQXshdG79346678 = -272377283;    int jXWpQXshdG11660914 = -796652452;    int jXWpQXshdG40854988 = -671575047;    int jXWpQXshdG37140741 = 7324828;    int jXWpQXshdG84916331 = -52220307;    int jXWpQXshdG12860799 = -206075257;    int jXWpQXshdG27517060 = -912679285;    int jXWpQXshdG70047529 = -633277804;    int jXWpQXshdG83842722 = -789645979;    int jXWpQXshdG60728309 = -959275798;    int jXWpQXshdG88121305 = 38814440;    int jXWpQXshdG19488576 = -186803613;    int jXWpQXshdG29140388 = -24937943;    int jXWpQXshdG75050304 = -582643266;    int jXWpQXshdG52748718 = -754984433;    int jXWpQXshdG49924217 = -426436178;    int jXWpQXshdG26397642 = -914439854;    int jXWpQXshdG11686892 = -66370597;    int jXWpQXshdG74559097 = -656565799;    int jXWpQXshdG41948803 = -93342924;    int jXWpQXshdG45698341 = -35976423;    int jXWpQXshdG16753093 = -605463274;    int jXWpQXshdG68056509 = -682609046;    int jXWpQXshdG98882914 = -860701710;    int jXWpQXshdG8969431 = 60045048;    int jXWpQXshdG8882112 = -385664199;    int jXWpQXshdG1567774 = -85128391;    int jXWpQXshdG57034936 = 46004410;    int jXWpQXshdG1617173 = -703415318;    int jXWpQXshdG57747323 = -3072732;    int jXWpQXshdG807589 = -67505095;    int jXWpQXshdG89467669 = -497804622;    int jXWpQXshdG66896454 = -656589552;    int jXWpQXshdG92504494 = -929260563;    int jXWpQXshdG62490204 = -735095037;    int jXWpQXshdG49992695 = -611986698;    int jXWpQXshdG18023791 = -677765495;    int jXWpQXshdG35699403 = -217581614;    int jXWpQXshdG96314900 = 71324821;    int jXWpQXshdG29444940 = -750309560;    int jXWpQXshdG10900588 = -234849862;    int jXWpQXshdG64678216 = -145555770;    int jXWpQXshdG48597183 = 82169542;     jXWpQXshdG72486560 = jXWpQXshdG10844752;     jXWpQXshdG10844752 = jXWpQXshdG28539676;     jXWpQXshdG28539676 = jXWpQXshdG38824782;     jXWpQXshdG38824782 = jXWpQXshdG73412055;     jXWpQXshdG73412055 = jXWpQXshdG84338387;     jXWpQXshdG84338387 = jXWpQXshdG78060864;     jXWpQXshdG78060864 = jXWpQXshdG31895789;     jXWpQXshdG31895789 = jXWpQXshdG31631720;     jXWpQXshdG31631720 = jXWpQXshdG39651135;     jXWpQXshdG39651135 = jXWpQXshdG30860903;     jXWpQXshdG30860903 = jXWpQXshdG96533026;     jXWpQXshdG96533026 = jXWpQXshdG30600612;     jXWpQXshdG30600612 = jXWpQXshdG78505141;     jXWpQXshdG78505141 = jXWpQXshdG52065548;     jXWpQXshdG52065548 = jXWpQXshdG16887187;     jXWpQXshdG16887187 = jXWpQXshdG14402327;     jXWpQXshdG14402327 = jXWpQXshdG8802995;     jXWpQXshdG8802995 = jXWpQXshdG99237786;     jXWpQXshdG99237786 = jXWpQXshdG92918123;     jXWpQXshdG92918123 = jXWpQXshdG16864837;     jXWpQXshdG16864837 = jXWpQXshdG80366449;     jXWpQXshdG80366449 = jXWpQXshdG80424086;     jXWpQXshdG80424086 = jXWpQXshdG49192997;     jXWpQXshdG49192997 = jXWpQXshdG27163868;     jXWpQXshdG27163868 = jXWpQXshdG32557067;     jXWpQXshdG32557067 = jXWpQXshdG47197647;     jXWpQXshdG47197647 = jXWpQXshdG93144533;     jXWpQXshdG93144533 = jXWpQXshdG19034991;     jXWpQXshdG19034991 = jXWpQXshdG4114661;     jXWpQXshdG4114661 = jXWpQXshdG69603606;     jXWpQXshdG69603606 = jXWpQXshdG47018181;     jXWpQXshdG47018181 = jXWpQXshdG35804718;     jXWpQXshdG35804718 = jXWpQXshdG42479306;     jXWpQXshdG42479306 = jXWpQXshdG59016565;     jXWpQXshdG59016565 = jXWpQXshdG22925160;     jXWpQXshdG22925160 = jXWpQXshdG41836883;     jXWpQXshdG41836883 = jXWpQXshdG61653609;     jXWpQXshdG61653609 = jXWpQXshdG58878778;     jXWpQXshdG58878778 = jXWpQXshdG72840144;     jXWpQXshdG72840144 = jXWpQXshdG81231232;     jXWpQXshdG81231232 = jXWpQXshdG42305739;     jXWpQXshdG42305739 = jXWpQXshdG38417647;     jXWpQXshdG38417647 = jXWpQXshdG34725746;     jXWpQXshdG34725746 = jXWpQXshdG32439905;     jXWpQXshdG32439905 = jXWpQXshdG59107358;     jXWpQXshdG59107358 = jXWpQXshdG33674153;     jXWpQXshdG33674153 = jXWpQXshdG38228217;     jXWpQXshdG38228217 = jXWpQXshdG84262421;     jXWpQXshdG84262421 = jXWpQXshdG17467217;     jXWpQXshdG17467217 = jXWpQXshdG47079725;     jXWpQXshdG47079725 = jXWpQXshdG67986433;     jXWpQXshdG67986433 = jXWpQXshdG89270857;     jXWpQXshdG89270857 = jXWpQXshdG34997129;     jXWpQXshdG34997129 = jXWpQXshdG53011637;     jXWpQXshdG53011637 = jXWpQXshdG92120110;     jXWpQXshdG92120110 = jXWpQXshdG30420666;     jXWpQXshdG30420666 = jXWpQXshdG79346678;     jXWpQXshdG79346678 = jXWpQXshdG11660914;     jXWpQXshdG11660914 = jXWpQXshdG40854988;     jXWpQXshdG40854988 = jXWpQXshdG37140741;     jXWpQXshdG37140741 = jXWpQXshdG84916331;     jXWpQXshdG84916331 = jXWpQXshdG12860799;     jXWpQXshdG12860799 = jXWpQXshdG27517060;     jXWpQXshdG27517060 = jXWpQXshdG70047529;     jXWpQXshdG70047529 = jXWpQXshdG83842722;     jXWpQXshdG83842722 = jXWpQXshdG60728309;     jXWpQXshdG60728309 = jXWpQXshdG88121305;     jXWpQXshdG88121305 = jXWpQXshdG19488576;     jXWpQXshdG19488576 = jXWpQXshdG29140388;     jXWpQXshdG29140388 = jXWpQXshdG75050304;     jXWpQXshdG75050304 = jXWpQXshdG52748718;     jXWpQXshdG52748718 = jXWpQXshdG49924217;     jXWpQXshdG49924217 = jXWpQXshdG26397642;     jXWpQXshdG26397642 = jXWpQXshdG11686892;     jXWpQXshdG11686892 = jXWpQXshdG74559097;     jXWpQXshdG74559097 = jXWpQXshdG41948803;     jXWpQXshdG41948803 = jXWpQXshdG45698341;     jXWpQXshdG45698341 = jXWpQXshdG16753093;     jXWpQXshdG16753093 = jXWpQXshdG68056509;     jXWpQXshdG68056509 = jXWpQXshdG98882914;     jXWpQXshdG98882914 = jXWpQXshdG8969431;     jXWpQXshdG8969431 = jXWpQXshdG8882112;     jXWpQXshdG8882112 = jXWpQXshdG1567774;     jXWpQXshdG1567774 = jXWpQXshdG57034936;     jXWpQXshdG57034936 = jXWpQXshdG1617173;     jXWpQXshdG1617173 = jXWpQXshdG57747323;     jXWpQXshdG57747323 = jXWpQXshdG807589;     jXWpQXshdG807589 = jXWpQXshdG89467669;     jXWpQXshdG89467669 = jXWpQXshdG66896454;     jXWpQXshdG66896454 = jXWpQXshdG92504494;     jXWpQXshdG92504494 = jXWpQXshdG62490204;     jXWpQXshdG62490204 = jXWpQXshdG49992695;     jXWpQXshdG49992695 = jXWpQXshdG18023791;     jXWpQXshdG18023791 = jXWpQXshdG35699403;     jXWpQXshdG35699403 = jXWpQXshdG96314900;     jXWpQXshdG96314900 = jXWpQXshdG29444940;     jXWpQXshdG29444940 = jXWpQXshdG10900588;     jXWpQXshdG10900588 = jXWpQXshdG64678216;     jXWpQXshdG64678216 = jXWpQXshdG48597183;     jXWpQXshdG48597183 = jXWpQXshdG72486560;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IcMmtktPAr40332346() {     int hvCXfGMGUX85272727 = -52871149;    int hvCXfGMGUX97615639 = -669252701;    int hvCXfGMGUX46292475 = -102223331;    int hvCXfGMGUX46765607 = -217859904;    int hvCXfGMGUX66923159 = 74769581;    int hvCXfGMGUX27648459 = -839282236;    int hvCXfGMGUX99268622 = -246987687;    int hvCXfGMGUX58614902 = -752912885;    int hvCXfGMGUX27036531 = -705345478;    int hvCXfGMGUX45869840 = 58890301;    int hvCXfGMGUX58348285 = -562801626;    int hvCXfGMGUX28483532 = -575387246;    int hvCXfGMGUX743915 = -215253212;    int hvCXfGMGUX28129220 = -852766751;    int hvCXfGMGUX75372101 = -445293027;    int hvCXfGMGUX45312255 = -141006974;    int hvCXfGMGUX70154144 = -927398049;    int hvCXfGMGUX92361479 = -430987021;    int hvCXfGMGUX398417 = -134165834;    int hvCXfGMGUX95572619 = -846510220;    int hvCXfGMGUX97038893 = -104266879;    int hvCXfGMGUX9414269 = -510730418;    int hvCXfGMGUX19184382 = -945813891;    int hvCXfGMGUX93700126 = -781831679;    int hvCXfGMGUX33058127 = -807447100;    int hvCXfGMGUX13875713 = -552270106;    int hvCXfGMGUX58613183 = -503390031;    int hvCXfGMGUX60497458 = -822893415;    int hvCXfGMGUX35652044 = -216536612;    int hvCXfGMGUX56865776 = -460794256;    int hvCXfGMGUX14039602 = -780161013;    int hvCXfGMGUX50337005 = -637189311;    int hvCXfGMGUX81850666 = 15366583;    int hvCXfGMGUX19555679 = -155039214;    int hvCXfGMGUX42077097 = -843579821;    int hvCXfGMGUX18149275 = -201932358;    int hvCXfGMGUX78600943 = -897381670;    int hvCXfGMGUX19357632 = -328259241;    int hvCXfGMGUX64654164 = -191191119;    int hvCXfGMGUX99104873 = -154673631;    int hvCXfGMGUX42662540 = -456369479;    int hvCXfGMGUX50008194 = -954696151;    int hvCXfGMGUX73301116 = -170962591;    int hvCXfGMGUX79354283 = -120005817;    int hvCXfGMGUX12448396 = -708793353;    int hvCXfGMGUX32735488 = -987714570;    int hvCXfGMGUX11256173 = -398652250;    int hvCXfGMGUX67668183 = 14918421;    int hvCXfGMGUX28326471 = -327033087;    int hvCXfGMGUX64417638 = -428227444;    int hvCXfGMGUX65407218 = 70203181;    int hvCXfGMGUX96162980 = -171514283;    int hvCXfGMGUX23756008 = -731400183;    int hvCXfGMGUX79696854 = -630725280;    int hvCXfGMGUX43293453 = -746924861;    int hvCXfGMGUX75858459 = -542140732;    int hvCXfGMGUX78431258 = -723438810;    int hvCXfGMGUX52592349 = -320391652;    int hvCXfGMGUX13707480 = -410412805;    int hvCXfGMGUX53047447 = -372960313;    int hvCXfGMGUX69035275 = -235892205;    int hvCXfGMGUX38771164 = -424094272;    int hvCXfGMGUX22962859 = -436376274;    int hvCXfGMGUX70170754 = -144551223;    int hvCXfGMGUX31830238 = -160948687;    int hvCXfGMGUX8011281 = -925612315;    int hvCXfGMGUX46632865 = -490753829;    int hvCXfGMGUX81188236 = 39786001;    int hvCXfGMGUX86052122 = 90813069;    int hvCXfGMGUX57222826 = -143360670;    int hvCXfGMGUX66711312 = -243625305;    int hvCXfGMGUX50796513 = -499138808;    int hvCXfGMGUX27707316 = -139795903;    int hvCXfGMGUX1293543 = -979492204;    int hvCXfGMGUX52910080 = -290140742;    int hvCXfGMGUX47030699 = -149570729;    int hvCXfGMGUX36113152 = -239767827;    int hvCXfGMGUX39830099 = -725808075;    int hvCXfGMGUX81251731 = 26961674;    int hvCXfGMGUX322640 = -819732531;    int hvCXfGMGUX2619541 = -53617857;    int hvCXfGMGUX90945000 = -418308453;    int hvCXfGMGUX32170987 = -395860329;    int hvCXfGMGUX71234406 = -788309168;    int hvCXfGMGUX91458557 = -430997437;    int hvCXfGMGUX17876622 = -508646730;    int hvCXfGMGUX26580997 = -905789128;    int hvCXfGMGUX2153812 = -353908137;    int hvCXfGMGUX76262226 = -408114354;    int hvCXfGMGUX66218638 = -201439089;    int hvCXfGMGUX39718017 = -478493548;    int hvCXfGMGUX26008595 = -476990018;    int hvCXfGMGUX5650152 = -917846437;    int hvCXfGMGUX11606717 = -818230806;    int hvCXfGMGUX30069599 = -918781426;    int hvCXfGMGUX3891376 = 67724793;    int hvCXfGMGUX27045336 = -418319877;    int hvCXfGMGUX3130363 = 73588631;    int hvCXfGMGUX47524046 = -959057130;    int hvCXfGMGUX4437115 = -52871149;     hvCXfGMGUX85272727 = hvCXfGMGUX97615639;     hvCXfGMGUX97615639 = hvCXfGMGUX46292475;     hvCXfGMGUX46292475 = hvCXfGMGUX46765607;     hvCXfGMGUX46765607 = hvCXfGMGUX66923159;     hvCXfGMGUX66923159 = hvCXfGMGUX27648459;     hvCXfGMGUX27648459 = hvCXfGMGUX99268622;     hvCXfGMGUX99268622 = hvCXfGMGUX58614902;     hvCXfGMGUX58614902 = hvCXfGMGUX27036531;     hvCXfGMGUX27036531 = hvCXfGMGUX45869840;     hvCXfGMGUX45869840 = hvCXfGMGUX58348285;     hvCXfGMGUX58348285 = hvCXfGMGUX28483532;     hvCXfGMGUX28483532 = hvCXfGMGUX743915;     hvCXfGMGUX743915 = hvCXfGMGUX28129220;     hvCXfGMGUX28129220 = hvCXfGMGUX75372101;     hvCXfGMGUX75372101 = hvCXfGMGUX45312255;     hvCXfGMGUX45312255 = hvCXfGMGUX70154144;     hvCXfGMGUX70154144 = hvCXfGMGUX92361479;     hvCXfGMGUX92361479 = hvCXfGMGUX398417;     hvCXfGMGUX398417 = hvCXfGMGUX95572619;     hvCXfGMGUX95572619 = hvCXfGMGUX97038893;     hvCXfGMGUX97038893 = hvCXfGMGUX9414269;     hvCXfGMGUX9414269 = hvCXfGMGUX19184382;     hvCXfGMGUX19184382 = hvCXfGMGUX93700126;     hvCXfGMGUX93700126 = hvCXfGMGUX33058127;     hvCXfGMGUX33058127 = hvCXfGMGUX13875713;     hvCXfGMGUX13875713 = hvCXfGMGUX58613183;     hvCXfGMGUX58613183 = hvCXfGMGUX60497458;     hvCXfGMGUX60497458 = hvCXfGMGUX35652044;     hvCXfGMGUX35652044 = hvCXfGMGUX56865776;     hvCXfGMGUX56865776 = hvCXfGMGUX14039602;     hvCXfGMGUX14039602 = hvCXfGMGUX50337005;     hvCXfGMGUX50337005 = hvCXfGMGUX81850666;     hvCXfGMGUX81850666 = hvCXfGMGUX19555679;     hvCXfGMGUX19555679 = hvCXfGMGUX42077097;     hvCXfGMGUX42077097 = hvCXfGMGUX18149275;     hvCXfGMGUX18149275 = hvCXfGMGUX78600943;     hvCXfGMGUX78600943 = hvCXfGMGUX19357632;     hvCXfGMGUX19357632 = hvCXfGMGUX64654164;     hvCXfGMGUX64654164 = hvCXfGMGUX99104873;     hvCXfGMGUX99104873 = hvCXfGMGUX42662540;     hvCXfGMGUX42662540 = hvCXfGMGUX50008194;     hvCXfGMGUX50008194 = hvCXfGMGUX73301116;     hvCXfGMGUX73301116 = hvCXfGMGUX79354283;     hvCXfGMGUX79354283 = hvCXfGMGUX12448396;     hvCXfGMGUX12448396 = hvCXfGMGUX32735488;     hvCXfGMGUX32735488 = hvCXfGMGUX11256173;     hvCXfGMGUX11256173 = hvCXfGMGUX67668183;     hvCXfGMGUX67668183 = hvCXfGMGUX28326471;     hvCXfGMGUX28326471 = hvCXfGMGUX64417638;     hvCXfGMGUX64417638 = hvCXfGMGUX65407218;     hvCXfGMGUX65407218 = hvCXfGMGUX96162980;     hvCXfGMGUX96162980 = hvCXfGMGUX23756008;     hvCXfGMGUX23756008 = hvCXfGMGUX79696854;     hvCXfGMGUX79696854 = hvCXfGMGUX43293453;     hvCXfGMGUX43293453 = hvCXfGMGUX75858459;     hvCXfGMGUX75858459 = hvCXfGMGUX78431258;     hvCXfGMGUX78431258 = hvCXfGMGUX52592349;     hvCXfGMGUX52592349 = hvCXfGMGUX13707480;     hvCXfGMGUX13707480 = hvCXfGMGUX53047447;     hvCXfGMGUX53047447 = hvCXfGMGUX69035275;     hvCXfGMGUX69035275 = hvCXfGMGUX38771164;     hvCXfGMGUX38771164 = hvCXfGMGUX22962859;     hvCXfGMGUX22962859 = hvCXfGMGUX70170754;     hvCXfGMGUX70170754 = hvCXfGMGUX31830238;     hvCXfGMGUX31830238 = hvCXfGMGUX8011281;     hvCXfGMGUX8011281 = hvCXfGMGUX46632865;     hvCXfGMGUX46632865 = hvCXfGMGUX81188236;     hvCXfGMGUX81188236 = hvCXfGMGUX86052122;     hvCXfGMGUX86052122 = hvCXfGMGUX57222826;     hvCXfGMGUX57222826 = hvCXfGMGUX66711312;     hvCXfGMGUX66711312 = hvCXfGMGUX50796513;     hvCXfGMGUX50796513 = hvCXfGMGUX27707316;     hvCXfGMGUX27707316 = hvCXfGMGUX1293543;     hvCXfGMGUX1293543 = hvCXfGMGUX52910080;     hvCXfGMGUX52910080 = hvCXfGMGUX47030699;     hvCXfGMGUX47030699 = hvCXfGMGUX36113152;     hvCXfGMGUX36113152 = hvCXfGMGUX39830099;     hvCXfGMGUX39830099 = hvCXfGMGUX81251731;     hvCXfGMGUX81251731 = hvCXfGMGUX322640;     hvCXfGMGUX322640 = hvCXfGMGUX2619541;     hvCXfGMGUX2619541 = hvCXfGMGUX90945000;     hvCXfGMGUX90945000 = hvCXfGMGUX32170987;     hvCXfGMGUX32170987 = hvCXfGMGUX71234406;     hvCXfGMGUX71234406 = hvCXfGMGUX91458557;     hvCXfGMGUX91458557 = hvCXfGMGUX17876622;     hvCXfGMGUX17876622 = hvCXfGMGUX26580997;     hvCXfGMGUX26580997 = hvCXfGMGUX2153812;     hvCXfGMGUX2153812 = hvCXfGMGUX76262226;     hvCXfGMGUX76262226 = hvCXfGMGUX66218638;     hvCXfGMGUX66218638 = hvCXfGMGUX39718017;     hvCXfGMGUX39718017 = hvCXfGMGUX26008595;     hvCXfGMGUX26008595 = hvCXfGMGUX5650152;     hvCXfGMGUX5650152 = hvCXfGMGUX11606717;     hvCXfGMGUX11606717 = hvCXfGMGUX30069599;     hvCXfGMGUX30069599 = hvCXfGMGUX3891376;     hvCXfGMGUX3891376 = hvCXfGMGUX27045336;     hvCXfGMGUX27045336 = hvCXfGMGUX3130363;     hvCXfGMGUX3130363 = hvCXfGMGUX47524046;     hvCXfGMGUX47524046 = hvCXfGMGUX4437115;     hvCXfGMGUX4437115 = hvCXfGMGUX85272727;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ToMIEziZOL58545688() {     int GOCokIYRST68717033 = -933536830;    int GOCokIYRST14487729 = -668634888;    int GOCokIYRST42700982 = -995748559;    int GOCokIYRST9557859 = -411659740;    int GOCokIYRST73044483 = -832097910;    int GOCokIYRST26152859 = -618353889;    int GOCokIYRST70307940 = 13100999;    int GOCokIYRST72962344 = -740070915;    int GOCokIYRST85566231 = 53195171;    int GOCokIYRST78415491 = -409485812;    int GOCokIYRST7764223 = -752917498;    int GOCokIYRST91688930 = -563005492;    int GOCokIYRST18861691 = -6914453;    int GOCokIYRST7756112 = -641057108;    int GOCokIYRST36226564 = -539316009;    int GOCokIYRST31701417 = 70913701;    int GOCokIYRST63906029 = -680743076;    int GOCokIYRST73212926 = -678291897;    int GOCokIYRST62838559 = -132840705;    int GOCokIYRST84940062 = -276096213;    int GOCokIYRST52679280 = -793795910;    int GOCokIYRST21561587 = -501882188;    int GOCokIYRST23010090 = -985703088;    int GOCokIYRST28406685 = -853657075;    int GOCokIYRST74634085 = -267099859;    int GOCokIYRST9217862 = -432779822;    int GOCokIYRST8067924 = -131474374;    int GOCokIYRST229084 = -892848299;    int GOCokIYRST71733137 = -79041754;    int GOCokIYRST72699117 = -973644107;    int GOCokIYRST32134916 = -417918214;    int GOCokIYRST13443432 = -552384391;    int GOCokIYRST25389554 = -705505595;    int GOCokIYRST61714190 = -846641384;    int GOCokIYRST20803828 = 58755444;    int GOCokIYRST30538459 = 10043736;    int GOCokIYRST47215122 = -776230850;    int GOCokIYRST59936464 = -36729761;    int GOCokIYRST13649242 = -376088917;    int GOCokIYRST48600009 = 92732847;    int GOCokIYRST76311464 = -27918452;    int GOCokIYRST495981 = -428604472;    int GOCokIYRST96868186 = -722019834;    int GOCokIYRST71259223 = -976188588;    int GOCokIYRST4392892 = -436950306;    int GOCokIYRST72341484 = 92674507;    int GOCokIYRST87271602 = -530956479;    int GOCokIYRST87338778 = 16196703;    int GOCokIYRST59173583 = -912635170;    int GOCokIYRST99750121 = -635247899;    int GOCokIYRST38664667 = -150698242;    int GOCokIYRST51663775 = -229773595;    int GOCokIYRST37868467 = -943147499;    int GOCokIYRST4373614 = -349138668;    int GOCokIYRST5866408 = 4589016;    int GOCokIYRST47155446 = -331654642;    int GOCokIYRST91477639 = -682931800;    int GOCokIYRST14294298 = -42091484;    int GOCokIYRST34923774 = -44559881;    int GOCokIYRST63826622 = -299318088;    int GOCokIYRST18084936 = -386879516;    int GOCokIYRST70078857 = -94050702;    int GOCokIYRST1229208 = -561029161;    int GOCokIYRST12867114 = 26839278;    int GOCokIYRST46280576 = -991567599;    int GOCokIYRST94320791 = -100533108;    int GOCokIYRST66299377 = -857499898;    int GOCokIYRST57147501 = -160273070;    int GOCokIYRST86952283 = -599812553;    int GOCokIYRST5688106 = -449359746;    int GOCokIYRST84486295 = -152855450;    int GOCokIYRST3969566 = -544013315;    int GOCokIYRST59563684 = -202202981;    int GOCokIYRST14238550 = -125573552;    int GOCokIYRST8628598 = -148177762;    int GOCokIYRST52183300 = -265191439;    int GOCokIYRST24693401 = -779862354;    int GOCokIYRST51750866 = 90485499;    int GOCokIYRST24013793 = -316706769;    int GOCokIYRST2292602 = -259774366;    int GOCokIYRST21946259 = -901823343;    int GOCokIYRST20729145 = -47671077;    int GOCokIYRST41055500 = -980213130;    int GOCokIYRST71983016 = -443793855;    int GOCokIYRST34034451 = -722945866;    int GOCokIYRST80471140 = -88144619;    int GOCokIYRST75574965 = -609236892;    int GOCokIYRST21015940 = -256366927;    int GOCokIYRST55847782 = -751230401;    int GOCokIYRST73648381 = -609589914;    int GOCokIYRST39060820 = -307024464;    int GOCokIYRST32920825 = -634139366;    int GOCokIYRST25012691 = -992169880;    int GOCokIYRST49822620 = 23229171;    int GOCokIYRST30515073 = -520387638;    int GOCokIYRST6232608 = -933867750;    int GOCokIYRST99266773 = -867575312;    int GOCokIYRST84001072 = -648859112;    int GOCokIYRST24978648 = -984620990;    int GOCokIYRST10072101 = -933536830;     GOCokIYRST68717033 = GOCokIYRST14487729;     GOCokIYRST14487729 = GOCokIYRST42700982;     GOCokIYRST42700982 = GOCokIYRST9557859;     GOCokIYRST9557859 = GOCokIYRST73044483;     GOCokIYRST73044483 = GOCokIYRST26152859;     GOCokIYRST26152859 = GOCokIYRST70307940;     GOCokIYRST70307940 = GOCokIYRST72962344;     GOCokIYRST72962344 = GOCokIYRST85566231;     GOCokIYRST85566231 = GOCokIYRST78415491;     GOCokIYRST78415491 = GOCokIYRST7764223;     GOCokIYRST7764223 = GOCokIYRST91688930;     GOCokIYRST91688930 = GOCokIYRST18861691;     GOCokIYRST18861691 = GOCokIYRST7756112;     GOCokIYRST7756112 = GOCokIYRST36226564;     GOCokIYRST36226564 = GOCokIYRST31701417;     GOCokIYRST31701417 = GOCokIYRST63906029;     GOCokIYRST63906029 = GOCokIYRST73212926;     GOCokIYRST73212926 = GOCokIYRST62838559;     GOCokIYRST62838559 = GOCokIYRST84940062;     GOCokIYRST84940062 = GOCokIYRST52679280;     GOCokIYRST52679280 = GOCokIYRST21561587;     GOCokIYRST21561587 = GOCokIYRST23010090;     GOCokIYRST23010090 = GOCokIYRST28406685;     GOCokIYRST28406685 = GOCokIYRST74634085;     GOCokIYRST74634085 = GOCokIYRST9217862;     GOCokIYRST9217862 = GOCokIYRST8067924;     GOCokIYRST8067924 = GOCokIYRST229084;     GOCokIYRST229084 = GOCokIYRST71733137;     GOCokIYRST71733137 = GOCokIYRST72699117;     GOCokIYRST72699117 = GOCokIYRST32134916;     GOCokIYRST32134916 = GOCokIYRST13443432;     GOCokIYRST13443432 = GOCokIYRST25389554;     GOCokIYRST25389554 = GOCokIYRST61714190;     GOCokIYRST61714190 = GOCokIYRST20803828;     GOCokIYRST20803828 = GOCokIYRST30538459;     GOCokIYRST30538459 = GOCokIYRST47215122;     GOCokIYRST47215122 = GOCokIYRST59936464;     GOCokIYRST59936464 = GOCokIYRST13649242;     GOCokIYRST13649242 = GOCokIYRST48600009;     GOCokIYRST48600009 = GOCokIYRST76311464;     GOCokIYRST76311464 = GOCokIYRST495981;     GOCokIYRST495981 = GOCokIYRST96868186;     GOCokIYRST96868186 = GOCokIYRST71259223;     GOCokIYRST71259223 = GOCokIYRST4392892;     GOCokIYRST4392892 = GOCokIYRST72341484;     GOCokIYRST72341484 = GOCokIYRST87271602;     GOCokIYRST87271602 = GOCokIYRST87338778;     GOCokIYRST87338778 = GOCokIYRST59173583;     GOCokIYRST59173583 = GOCokIYRST99750121;     GOCokIYRST99750121 = GOCokIYRST38664667;     GOCokIYRST38664667 = GOCokIYRST51663775;     GOCokIYRST51663775 = GOCokIYRST37868467;     GOCokIYRST37868467 = GOCokIYRST4373614;     GOCokIYRST4373614 = GOCokIYRST5866408;     GOCokIYRST5866408 = GOCokIYRST47155446;     GOCokIYRST47155446 = GOCokIYRST91477639;     GOCokIYRST91477639 = GOCokIYRST14294298;     GOCokIYRST14294298 = GOCokIYRST34923774;     GOCokIYRST34923774 = GOCokIYRST63826622;     GOCokIYRST63826622 = GOCokIYRST18084936;     GOCokIYRST18084936 = GOCokIYRST70078857;     GOCokIYRST70078857 = GOCokIYRST1229208;     GOCokIYRST1229208 = GOCokIYRST12867114;     GOCokIYRST12867114 = GOCokIYRST46280576;     GOCokIYRST46280576 = GOCokIYRST94320791;     GOCokIYRST94320791 = GOCokIYRST66299377;     GOCokIYRST66299377 = GOCokIYRST57147501;     GOCokIYRST57147501 = GOCokIYRST86952283;     GOCokIYRST86952283 = GOCokIYRST5688106;     GOCokIYRST5688106 = GOCokIYRST84486295;     GOCokIYRST84486295 = GOCokIYRST3969566;     GOCokIYRST3969566 = GOCokIYRST59563684;     GOCokIYRST59563684 = GOCokIYRST14238550;     GOCokIYRST14238550 = GOCokIYRST8628598;     GOCokIYRST8628598 = GOCokIYRST52183300;     GOCokIYRST52183300 = GOCokIYRST24693401;     GOCokIYRST24693401 = GOCokIYRST51750866;     GOCokIYRST51750866 = GOCokIYRST24013793;     GOCokIYRST24013793 = GOCokIYRST2292602;     GOCokIYRST2292602 = GOCokIYRST21946259;     GOCokIYRST21946259 = GOCokIYRST20729145;     GOCokIYRST20729145 = GOCokIYRST41055500;     GOCokIYRST41055500 = GOCokIYRST71983016;     GOCokIYRST71983016 = GOCokIYRST34034451;     GOCokIYRST34034451 = GOCokIYRST80471140;     GOCokIYRST80471140 = GOCokIYRST75574965;     GOCokIYRST75574965 = GOCokIYRST21015940;     GOCokIYRST21015940 = GOCokIYRST55847782;     GOCokIYRST55847782 = GOCokIYRST73648381;     GOCokIYRST73648381 = GOCokIYRST39060820;     GOCokIYRST39060820 = GOCokIYRST32920825;     GOCokIYRST32920825 = GOCokIYRST25012691;     GOCokIYRST25012691 = GOCokIYRST49822620;     GOCokIYRST49822620 = GOCokIYRST30515073;     GOCokIYRST30515073 = GOCokIYRST6232608;     GOCokIYRST6232608 = GOCokIYRST99266773;     GOCokIYRST99266773 = GOCokIYRST84001072;     GOCokIYRST84001072 = GOCokIYRST24978648;     GOCokIYRST24978648 = GOCokIYRST10072101;     GOCokIYRST10072101 = GOCokIYRST68717033;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WSAiAZoSTW58987791() {     int UOjuIrRiQq3896790 = -830457343;    int UOjuIrRiQq47033801 = -744916831;    int UOjuIrRiQq87909848 = -586633171;    int UOjuIrRiQq47425512 = -421728719;    int UOjuIrRiQq55889740 = -346616054;    int UOjuIrRiQq49540614 = -556452769;    int UOjuIrRiQq15124102 = -650021910;    int UOjuIrRiQq6101266 = -851008498;    int UOjuIrRiQq95394008 = -79112890;    int UOjuIrRiQq79758267 = -319507906;    int UOjuIrRiQq52472860 = -904755843;    int UOjuIrRiQq65250707 = -485605239;    int UOjuIrRiQq24160261 = -507755956;    int UOjuIrRiQq46150722 = -56028012;    int UOjuIrRiQq51096285 = -111066701;    int UOjuIrRiQq29817851 = -528849580;    int UOjuIrRiQq13628440 = -802182793;    int UOjuIrRiQq39747295 = -288173269;    int UOjuIrRiQq84852550 = -659429759;    int UOjuIrRiQq94037078 = -515950354;    int UOjuIrRiQq54067020 = -466133109;    int UOjuIrRiQq36128359 = -666514966;    int UOjuIrRiQq28185312 = -633829733;    int UOjuIrRiQq55763493 = -797074422;    int UOjuIrRiQq87625901 = -772407055;    int UOjuIrRiQq62265424 = -658899316;    int UOjuIrRiQq96351298 = -221967434;    int UOjuIrRiQq38089934 = -246133869;    int UOjuIrRiQq82564304 = 8554201;    int UOjuIrRiQq58517542 = -848394755;    int UOjuIrRiQq87449038 = -388557881;    int UOjuIrRiQq89579588 = -791353662;    int UOjuIrRiQq67021750 = -908759658;    int UOjuIrRiQq97896412 = -98840720;    int UOjuIrRiQq97735686 = -355718934;    int UOjuIrRiQq24627254 = -14148998;    int UOjuIrRiQq65477526 = -199434773;    int UOjuIrRiQq1348909 = -928712646;    int UOjuIrRiQq32419163 = 93695891;    int UOjuIrRiQq23177561 = -278476342;    int UOjuIrRiQq78644475 = -818626213;    int UOjuIrRiQq11009167 = -991032415;    int UOjuIrRiQq77018488 = -73853609;    int UOjuIrRiQq56099982 = -657760036;    int UOjuIrRiQq68366596 = -765604342;    int UOjuIrRiQq13641927 = -350828358;    int UOjuIrRiQq49453227 = -25601163;    int UOjuIrRiQq85993838 = -386929711;    int UOjuIrRiQq79039148 = -362189981;    int UOjuIrRiQq23738865 = -701279735;    int UOjuIrRiQq52602007 = -80103820;    int UOjuIrRiQq39144322 = -749632467;    int UOjuIrRiQq75219672 = -899008004;    int UOjuIrRiQq99109613 = -782351367;    int UOjuIrRiQq4920771 = -373660308;    int UOjuIrRiQq67768430 = -63942378;    int UOjuIrRiQq18848490 = -11087099;    int UOjuIrRiQq32146356 = -789558749;    int UOjuIrRiQq59799610 = -649321664;    int UOjuIrRiQq93624315 = -687716739;    int UOjuIrRiQq53189316 = -234485335;    int UOjuIrRiQq77034167 = -303888042;    int UOjuIrRiQq23536961 = -759562700;    int UOjuIrRiQq36876467 = -230718136;    int UOjuIrRiQq92309229 = -930950026;    int UOjuIrRiQq62893271 = -13402181;    int UOjuIrRiQq98228957 = -576845581;    int UOjuIrRiQq26263848 = -308915236;    int UOjuIrRiQq48415036 = -700309079;    int UOjuIrRiQq26469032 = 3082296;    int UOjuIrRiQq64340324 = -229414807;    int UOjuIrRiQq12279532 = -873470148;    int UOjuIrRiQq7328132 = -281869160;    int UOjuIrRiQq61674990 = -280953418;    int UOjuIrRiQq15392603 = -697324142;    int UOjuIrRiQq43057853 = -475100695;    int UOjuIrRiQq59109870 = -492661358;    int UOjuIrRiQq72085329 = -976069698;    int UOjuIrRiQq87396896 = 68529920;    int UOjuIrRiQq73983975 = -321578698;    int UOjuIrRiQq12812198 = -533298154;    int UOjuIrRiQq10357460 = -835037723;    int UOjuIrRiQq59050785 = -883943888;    int UOjuIrRiQq58825440 = -290166064;    int UOjuIrRiQq5915536 = -668290935;    int UOjuIrRiQq48304716 = -638925415;    int UOjuIrRiQq14359917 = -892345659;    int UOjuIrRiQq67912136 = -26408291;    int UOjuIrRiQq92975641 = -725180413;    int UOjuIrRiQq29967256 = -191776556;    int UOjuIrRiQq5778764 = 96938100;    int UOjuIrRiQq33331171 = -409876025;    int UOjuIrRiQq41549298 = -179390982;    int UOjuIrRiQq38794847 = -218587371;    int UOjuIrRiQq69988244 = 56008993;    int UOjuIrRiQq1610309 = -414738172;    int UOjuIrRiQq87472206 = -131469715;    int UOjuIrRiQq40142022 = -843135473;    int UOjuIrRiQq63790753 = -726810011;    int UOjuIrRiQq5473326 = -830457343;     UOjuIrRiQq3896790 = UOjuIrRiQq47033801;     UOjuIrRiQq47033801 = UOjuIrRiQq87909848;     UOjuIrRiQq87909848 = UOjuIrRiQq47425512;     UOjuIrRiQq47425512 = UOjuIrRiQq55889740;     UOjuIrRiQq55889740 = UOjuIrRiQq49540614;     UOjuIrRiQq49540614 = UOjuIrRiQq15124102;     UOjuIrRiQq15124102 = UOjuIrRiQq6101266;     UOjuIrRiQq6101266 = UOjuIrRiQq95394008;     UOjuIrRiQq95394008 = UOjuIrRiQq79758267;     UOjuIrRiQq79758267 = UOjuIrRiQq52472860;     UOjuIrRiQq52472860 = UOjuIrRiQq65250707;     UOjuIrRiQq65250707 = UOjuIrRiQq24160261;     UOjuIrRiQq24160261 = UOjuIrRiQq46150722;     UOjuIrRiQq46150722 = UOjuIrRiQq51096285;     UOjuIrRiQq51096285 = UOjuIrRiQq29817851;     UOjuIrRiQq29817851 = UOjuIrRiQq13628440;     UOjuIrRiQq13628440 = UOjuIrRiQq39747295;     UOjuIrRiQq39747295 = UOjuIrRiQq84852550;     UOjuIrRiQq84852550 = UOjuIrRiQq94037078;     UOjuIrRiQq94037078 = UOjuIrRiQq54067020;     UOjuIrRiQq54067020 = UOjuIrRiQq36128359;     UOjuIrRiQq36128359 = UOjuIrRiQq28185312;     UOjuIrRiQq28185312 = UOjuIrRiQq55763493;     UOjuIrRiQq55763493 = UOjuIrRiQq87625901;     UOjuIrRiQq87625901 = UOjuIrRiQq62265424;     UOjuIrRiQq62265424 = UOjuIrRiQq96351298;     UOjuIrRiQq96351298 = UOjuIrRiQq38089934;     UOjuIrRiQq38089934 = UOjuIrRiQq82564304;     UOjuIrRiQq82564304 = UOjuIrRiQq58517542;     UOjuIrRiQq58517542 = UOjuIrRiQq87449038;     UOjuIrRiQq87449038 = UOjuIrRiQq89579588;     UOjuIrRiQq89579588 = UOjuIrRiQq67021750;     UOjuIrRiQq67021750 = UOjuIrRiQq97896412;     UOjuIrRiQq97896412 = UOjuIrRiQq97735686;     UOjuIrRiQq97735686 = UOjuIrRiQq24627254;     UOjuIrRiQq24627254 = UOjuIrRiQq65477526;     UOjuIrRiQq65477526 = UOjuIrRiQq1348909;     UOjuIrRiQq1348909 = UOjuIrRiQq32419163;     UOjuIrRiQq32419163 = UOjuIrRiQq23177561;     UOjuIrRiQq23177561 = UOjuIrRiQq78644475;     UOjuIrRiQq78644475 = UOjuIrRiQq11009167;     UOjuIrRiQq11009167 = UOjuIrRiQq77018488;     UOjuIrRiQq77018488 = UOjuIrRiQq56099982;     UOjuIrRiQq56099982 = UOjuIrRiQq68366596;     UOjuIrRiQq68366596 = UOjuIrRiQq13641927;     UOjuIrRiQq13641927 = UOjuIrRiQq49453227;     UOjuIrRiQq49453227 = UOjuIrRiQq85993838;     UOjuIrRiQq85993838 = UOjuIrRiQq79039148;     UOjuIrRiQq79039148 = UOjuIrRiQq23738865;     UOjuIrRiQq23738865 = UOjuIrRiQq52602007;     UOjuIrRiQq52602007 = UOjuIrRiQq39144322;     UOjuIrRiQq39144322 = UOjuIrRiQq75219672;     UOjuIrRiQq75219672 = UOjuIrRiQq99109613;     UOjuIrRiQq99109613 = UOjuIrRiQq4920771;     UOjuIrRiQq4920771 = UOjuIrRiQq67768430;     UOjuIrRiQq67768430 = UOjuIrRiQq18848490;     UOjuIrRiQq18848490 = UOjuIrRiQq32146356;     UOjuIrRiQq32146356 = UOjuIrRiQq59799610;     UOjuIrRiQq59799610 = UOjuIrRiQq93624315;     UOjuIrRiQq93624315 = UOjuIrRiQq53189316;     UOjuIrRiQq53189316 = UOjuIrRiQq77034167;     UOjuIrRiQq77034167 = UOjuIrRiQq23536961;     UOjuIrRiQq23536961 = UOjuIrRiQq36876467;     UOjuIrRiQq36876467 = UOjuIrRiQq92309229;     UOjuIrRiQq92309229 = UOjuIrRiQq62893271;     UOjuIrRiQq62893271 = UOjuIrRiQq98228957;     UOjuIrRiQq98228957 = UOjuIrRiQq26263848;     UOjuIrRiQq26263848 = UOjuIrRiQq48415036;     UOjuIrRiQq48415036 = UOjuIrRiQq26469032;     UOjuIrRiQq26469032 = UOjuIrRiQq64340324;     UOjuIrRiQq64340324 = UOjuIrRiQq12279532;     UOjuIrRiQq12279532 = UOjuIrRiQq7328132;     UOjuIrRiQq7328132 = UOjuIrRiQq61674990;     UOjuIrRiQq61674990 = UOjuIrRiQq15392603;     UOjuIrRiQq15392603 = UOjuIrRiQq43057853;     UOjuIrRiQq43057853 = UOjuIrRiQq59109870;     UOjuIrRiQq59109870 = UOjuIrRiQq72085329;     UOjuIrRiQq72085329 = UOjuIrRiQq87396896;     UOjuIrRiQq87396896 = UOjuIrRiQq73983975;     UOjuIrRiQq73983975 = UOjuIrRiQq12812198;     UOjuIrRiQq12812198 = UOjuIrRiQq10357460;     UOjuIrRiQq10357460 = UOjuIrRiQq59050785;     UOjuIrRiQq59050785 = UOjuIrRiQq58825440;     UOjuIrRiQq58825440 = UOjuIrRiQq5915536;     UOjuIrRiQq5915536 = UOjuIrRiQq48304716;     UOjuIrRiQq48304716 = UOjuIrRiQq14359917;     UOjuIrRiQq14359917 = UOjuIrRiQq67912136;     UOjuIrRiQq67912136 = UOjuIrRiQq92975641;     UOjuIrRiQq92975641 = UOjuIrRiQq29967256;     UOjuIrRiQq29967256 = UOjuIrRiQq5778764;     UOjuIrRiQq5778764 = UOjuIrRiQq33331171;     UOjuIrRiQq33331171 = UOjuIrRiQq41549298;     UOjuIrRiQq41549298 = UOjuIrRiQq38794847;     UOjuIrRiQq38794847 = UOjuIrRiQq69988244;     UOjuIrRiQq69988244 = UOjuIrRiQq1610309;     UOjuIrRiQq1610309 = UOjuIrRiQq87472206;     UOjuIrRiQq87472206 = UOjuIrRiQq40142022;     UOjuIrRiQq40142022 = UOjuIrRiQq63790753;     UOjuIrRiQq63790753 = UOjuIrRiQq5473326;     UOjuIrRiQq5473326 = UOjuIrRiQq3896790;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UWBCDpgClW29443664() {     int HxxPVQfIvl16682957 = -965498035;    int HxxPVQfIvl33804689 = -915250845;    int HxxPVQfIvl5662649 = -994992552;    int HxxPVQfIvl55366337 = -245930995;    int HxxPVQfIvl49400844 = -838432519;    int HxxPVQfIvl92850685 = -333376082;    int HxxPVQfIvl36331860 = -95693981;    int HxxPVQfIvl32820378 = -195526755;    int HxxPVQfIvl90798819 = -40870983;    int HxxPVQfIvl85976972 = -123595539;    int HxxPVQfIvl79960242 = 80558142;    int HxxPVQfIvl97201212 = -992938055;    int HxxPVQfIvl94303563 = -411538613;    int HxxPVQfIvl95774800 = -388443210;    int HxxPVQfIvl74402838 = -18052532;    int HxxPVQfIvl58242919 = 20259031;    int HxxPVQfIvl69380257 = -465957260;    int HxxPVQfIvl23305780 = -943383571;    int HxxPVQfIvl86013180 = -568898954;    int HxxPVQfIvl96691575 = -15194493;    int HxxPVQfIvl34241077 = -957449374;    int HxxPVQfIvl65176178 = -836373314;    int HxxPVQfIvl66945607 = -564833628;    int HxxPVQfIvl270623 = -57419434;    int HxxPVQfIvl93520160 = -982848979;    int HxxPVQfIvl43584070 = -349330515;    int HxxPVQfIvl7766835 = -855673714;    int HxxPVQfIvl5442859 = -419931974;    int HxxPVQfIvl99181357 = -205663039;    int HxxPVQfIvl11268658 = -478280909;    int HxxPVQfIvl31885034 = -664974631;    int HxxPVQfIvl92898412 = -770073341;    int HxxPVQfIvl13067699 = -784614442;    int HxxPVQfIvl74972785 = -3594938;    int HxxPVQfIvl80796218 = -965750814;    int HxxPVQfIvl19851369 = -902712102;    int HxxPVQfIvl2241587 = 10655876;    int HxxPVQfIvl59052931 = -848332737;    int HxxPVQfIvl38194549 = -848154686;    int HxxPVQfIvl49442290 = -122893188;    int HxxPVQfIvl40075784 = -94100206;    int HxxPVQfIvl18711623 = -889343749;    int HxxPVQfIvl11901958 = -97287053;    int HxxPVQfIvl728520 = -998932279;    int HxxPVQfIvl48375087 = -658374303;    int HxxPVQfIvl87270056 = -424146797;    int HxxPVQfIvl27035247 = -523116215;    int HxxPVQfIvl15433805 = -542282491;    int HxxPVQfIvl23103198 = -525791956;    int HxxPVQfIvl70689285 = -212316198;    int HxxPVQfIvl70929500 = -332988128;    int HxxPVQfIvl67320869 = -120817805;    int HxxPVQfIvl9704822 = 24988713;    int HxxPVQfIvl43809338 = -371803109;    int HxxPVQfIvl95202586 = -368104794;    int HxxPVQfIvl51506778 = -29124722;    int HxxPVQfIvl66859082 = -250417218;    int HxxPVQfIvl5392026 = -837573119;    int HxxPVQfIvl61846176 = -263082017;    int HxxPVQfIvl5816775 = -389102004;    int HxxPVQfIvl85083850 = -477702368;    int HxxPVQfIvl30889001 = -675762007;    int HxxPVQfIvl33639021 = -989863717;    int HxxPVQfIvl79530161 = -562590074;    int HxxPVQfIvl54091938 = -458620909;    int HxxPVQfIvl87061829 = -149368518;    int HxxPVQfIvl84133513 = -108323613;    int HxxPVQfIvl19330778 = -307943675;    int HxxPVQfIvl14978583 = -422692397;    int HxxPVQfIvl54551470 = -115340430;    int HxxPVQfIvl56001333 = -990396846;    int HxxPVQfIvl10327326 = -617624523;    int HxxPVQfIvl85111231 = 4771115;    int HxxPVQfIvl36570891 = -346005767;    int HxxPVQfIvl56615791 = -921094288;    int HxxPVQfIvl15529454 = 31894375;    int HxxPVQfIvl53274220 = -639086261;    int HxxPVQfIvl66217087 = -565901349;    int HxxPVQfIvl51895536 = -399045132;    int HxxPVQfIvl6250105 = -458702183;    int HxxPVQfIvl16548823 = -826214301;    int HxxPVQfIvl92333029 = -213391224;    int HxxPVQfIvl82339660 = -894140019;    int HxxPVQfIvl28492072 = -993346841;    int HxxPVQfIvl40339157 = -45292782;    int HxxPVQfIvl64564165 = -444156827;    int HxxPVQfIvl83193590 = -695062055;    int HxxPVQfIvl69258360 = -312811334;    int HxxPVQfIvl79770198 = -635490145;    int HxxPVQfIvl29289440 = -836626093;    int HxxPVQfIvl52992286 = -552294885;    int HxxPVQfIvl96849561 = -151771006;    int HxxPVQfIvl97206754 = -485250721;    int HxxPVQfIvl32377774 = -359052682;    int HxxPVQfIvl64358439 = -645190820;    int HxxPVQfIvl9186783 = -418338199;    int HxxPVQfIvl85072602 = -899480033;    int HxxPVQfIvl32371797 = -534696980;    int HxxPVQfIvl46636582 = -440311371;    int HxxPVQfIvl61313257 = -965498035;     HxxPVQfIvl16682957 = HxxPVQfIvl33804689;     HxxPVQfIvl33804689 = HxxPVQfIvl5662649;     HxxPVQfIvl5662649 = HxxPVQfIvl55366337;     HxxPVQfIvl55366337 = HxxPVQfIvl49400844;     HxxPVQfIvl49400844 = HxxPVQfIvl92850685;     HxxPVQfIvl92850685 = HxxPVQfIvl36331860;     HxxPVQfIvl36331860 = HxxPVQfIvl32820378;     HxxPVQfIvl32820378 = HxxPVQfIvl90798819;     HxxPVQfIvl90798819 = HxxPVQfIvl85976972;     HxxPVQfIvl85976972 = HxxPVQfIvl79960242;     HxxPVQfIvl79960242 = HxxPVQfIvl97201212;     HxxPVQfIvl97201212 = HxxPVQfIvl94303563;     HxxPVQfIvl94303563 = HxxPVQfIvl95774800;     HxxPVQfIvl95774800 = HxxPVQfIvl74402838;     HxxPVQfIvl74402838 = HxxPVQfIvl58242919;     HxxPVQfIvl58242919 = HxxPVQfIvl69380257;     HxxPVQfIvl69380257 = HxxPVQfIvl23305780;     HxxPVQfIvl23305780 = HxxPVQfIvl86013180;     HxxPVQfIvl86013180 = HxxPVQfIvl96691575;     HxxPVQfIvl96691575 = HxxPVQfIvl34241077;     HxxPVQfIvl34241077 = HxxPVQfIvl65176178;     HxxPVQfIvl65176178 = HxxPVQfIvl66945607;     HxxPVQfIvl66945607 = HxxPVQfIvl270623;     HxxPVQfIvl270623 = HxxPVQfIvl93520160;     HxxPVQfIvl93520160 = HxxPVQfIvl43584070;     HxxPVQfIvl43584070 = HxxPVQfIvl7766835;     HxxPVQfIvl7766835 = HxxPVQfIvl5442859;     HxxPVQfIvl5442859 = HxxPVQfIvl99181357;     HxxPVQfIvl99181357 = HxxPVQfIvl11268658;     HxxPVQfIvl11268658 = HxxPVQfIvl31885034;     HxxPVQfIvl31885034 = HxxPVQfIvl92898412;     HxxPVQfIvl92898412 = HxxPVQfIvl13067699;     HxxPVQfIvl13067699 = HxxPVQfIvl74972785;     HxxPVQfIvl74972785 = HxxPVQfIvl80796218;     HxxPVQfIvl80796218 = HxxPVQfIvl19851369;     HxxPVQfIvl19851369 = HxxPVQfIvl2241587;     HxxPVQfIvl2241587 = HxxPVQfIvl59052931;     HxxPVQfIvl59052931 = HxxPVQfIvl38194549;     HxxPVQfIvl38194549 = HxxPVQfIvl49442290;     HxxPVQfIvl49442290 = HxxPVQfIvl40075784;     HxxPVQfIvl40075784 = HxxPVQfIvl18711623;     HxxPVQfIvl18711623 = HxxPVQfIvl11901958;     HxxPVQfIvl11901958 = HxxPVQfIvl728520;     HxxPVQfIvl728520 = HxxPVQfIvl48375087;     HxxPVQfIvl48375087 = HxxPVQfIvl87270056;     HxxPVQfIvl87270056 = HxxPVQfIvl27035247;     HxxPVQfIvl27035247 = HxxPVQfIvl15433805;     HxxPVQfIvl15433805 = HxxPVQfIvl23103198;     HxxPVQfIvl23103198 = HxxPVQfIvl70689285;     HxxPVQfIvl70689285 = HxxPVQfIvl70929500;     HxxPVQfIvl70929500 = HxxPVQfIvl67320869;     HxxPVQfIvl67320869 = HxxPVQfIvl9704822;     HxxPVQfIvl9704822 = HxxPVQfIvl43809338;     HxxPVQfIvl43809338 = HxxPVQfIvl95202586;     HxxPVQfIvl95202586 = HxxPVQfIvl51506778;     HxxPVQfIvl51506778 = HxxPVQfIvl66859082;     HxxPVQfIvl66859082 = HxxPVQfIvl5392026;     HxxPVQfIvl5392026 = HxxPVQfIvl61846176;     HxxPVQfIvl61846176 = HxxPVQfIvl5816775;     HxxPVQfIvl5816775 = HxxPVQfIvl85083850;     HxxPVQfIvl85083850 = HxxPVQfIvl30889001;     HxxPVQfIvl30889001 = HxxPVQfIvl33639021;     HxxPVQfIvl33639021 = HxxPVQfIvl79530161;     HxxPVQfIvl79530161 = HxxPVQfIvl54091938;     HxxPVQfIvl54091938 = HxxPVQfIvl87061829;     HxxPVQfIvl87061829 = HxxPVQfIvl84133513;     HxxPVQfIvl84133513 = HxxPVQfIvl19330778;     HxxPVQfIvl19330778 = HxxPVQfIvl14978583;     HxxPVQfIvl14978583 = HxxPVQfIvl54551470;     HxxPVQfIvl54551470 = HxxPVQfIvl56001333;     HxxPVQfIvl56001333 = HxxPVQfIvl10327326;     HxxPVQfIvl10327326 = HxxPVQfIvl85111231;     HxxPVQfIvl85111231 = HxxPVQfIvl36570891;     HxxPVQfIvl36570891 = HxxPVQfIvl56615791;     HxxPVQfIvl56615791 = HxxPVQfIvl15529454;     HxxPVQfIvl15529454 = HxxPVQfIvl53274220;     HxxPVQfIvl53274220 = HxxPVQfIvl66217087;     HxxPVQfIvl66217087 = HxxPVQfIvl51895536;     HxxPVQfIvl51895536 = HxxPVQfIvl6250105;     HxxPVQfIvl6250105 = HxxPVQfIvl16548823;     HxxPVQfIvl16548823 = HxxPVQfIvl92333029;     HxxPVQfIvl92333029 = HxxPVQfIvl82339660;     HxxPVQfIvl82339660 = HxxPVQfIvl28492072;     HxxPVQfIvl28492072 = HxxPVQfIvl40339157;     HxxPVQfIvl40339157 = HxxPVQfIvl64564165;     HxxPVQfIvl64564165 = HxxPVQfIvl83193590;     HxxPVQfIvl83193590 = HxxPVQfIvl69258360;     HxxPVQfIvl69258360 = HxxPVQfIvl79770198;     HxxPVQfIvl79770198 = HxxPVQfIvl29289440;     HxxPVQfIvl29289440 = HxxPVQfIvl52992286;     HxxPVQfIvl52992286 = HxxPVQfIvl96849561;     HxxPVQfIvl96849561 = HxxPVQfIvl97206754;     HxxPVQfIvl97206754 = HxxPVQfIvl32377774;     HxxPVQfIvl32377774 = HxxPVQfIvl64358439;     HxxPVQfIvl64358439 = HxxPVQfIvl9186783;     HxxPVQfIvl9186783 = HxxPVQfIvl85072602;     HxxPVQfIvl85072602 = HxxPVQfIvl32371797;     HxxPVQfIvl32371797 = HxxPVQfIvl46636582;     HxxPVQfIvl46636582 = HxxPVQfIvl61313257;     HxxPVQfIvl61313257 = HxxPVQfIvl16682957;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VDOVfkRyaE47657005() {     int wtaksIcGmr127262 = -746163716;    int wtaksIcGmr50676778 = -914633033;    int wtaksIcGmr2071156 = -788517780;    int wtaksIcGmr18158589 = -439730831;    int wtaksIcGmr55522168 = -645300010;    int wtaksIcGmr91355086 = -112447735;    int wtaksIcGmr7371178 = -935605294;    int wtaksIcGmr47167821 = -182684785;    int wtaksIcGmr49328520 = -382330333;    int wtaksIcGmr18522625 = -591971651;    int wtaksIcGmr29376180 = -109557731;    int wtaksIcGmr60406612 = -980556301;    int wtaksIcGmr12421339 = -203199854;    int wtaksIcGmr75401693 = -176733567;    int wtaksIcGmr35257302 = -112075514;    int wtaksIcGmr44632081 = -867820295;    int wtaksIcGmr63132142 = -219302287;    int wtaksIcGmr4157226 = -90688447;    int wtaksIcGmr48453323 = -567573825;    int wtaksIcGmr86059017 = -544780486;    int wtaksIcGmr89881464 = -546978404;    int wtaksIcGmr77323496 = -827525084;    int wtaksIcGmr70771314 = -604722825;    int wtaksIcGmr34977181 = -129244830;    int wtaksIcGmr35096119 = -442501738;    int wtaksIcGmr38926219 = -229840231;    int wtaksIcGmr57221575 = -483758057;    int wtaksIcGmr45174484 = -489886858;    int wtaksIcGmr35262452 = -68168181;    int wtaksIcGmr27101999 = -991130761;    int wtaksIcGmr49980348 = -302731832;    int wtaksIcGmr56004839 = -685268421;    int wtaksIcGmr56606586 = -405486620;    int wtaksIcGmr17131296 = -695197108;    int wtaksIcGmr59522949 = -63415549;    int wtaksIcGmr32240553 = -690736008;    int wtaksIcGmr70855765 = -968193304;    int wtaksIcGmr99631763 = -556803257;    int wtaksIcGmr87189626 = 66947516;    int wtaksIcGmr98937425 = -975486710;    int wtaksIcGmr73724708 = -765649179;    int wtaksIcGmr69199409 = -363252071;    int wtaksIcGmr35469028 = -648344296;    int wtaksIcGmr92633460 = -755115050;    int wtaksIcGmr40319584 = -386531256;    int wtaksIcGmr26876052 = -443757721;    int wtaksIcGmr3050677 = -655420444;    int wtaksIcGmr35104401 = -541004210;    int wtaksIcGmr53950310 = -11394038;    int wtaksIcGmr6021770 = -419336653;    int wtaksIcGmr44186949 = -553889550;    int wtaksIcGmr22821664 = -179077116;    int wtaksIcGmr23817281 = -186758603;    int wtaksIcGmr68486097 = -90216497;    int wtaksIcGmr57775541 = -716590917;    int wtaksIcGmr22803766 = -918638632;    int wtaksIcGmr79905463 = -209910208;    int wtaksIcGmr67093974 = -559272950;    int wtaksIcGmr83062470 = -997229093;    int wtaksIcGmr16595950 = -315459780;    int wtaksIcGmr34133511 = -628689679;    int wtaksIcGmr62196693 = -345718437;    int wtaksIcGmr11905369 = -14516604;    int wtaksIcGmr22226521 = -391199573;    int wtaksIcGmr68542276 = -189239820;    int wtaksIcGmr73371340 = -424289311;    int wtaksIcGmr3800027 = -475069682;    int wtaksIcGmr95290043 = -508002746;    int wtaksIcGmr15878744 = -13318018;    int wtaksIcGmr3016750 = -421339507;    int wtaksIcGmr73776315 = -899626991;    int wtaksIcGmr63500379 = -662499030;    int wtaksIcGmr16967600 = -57635964;    int wtaksIcGmr49515898 = -592087116;    int wtaksIcGmr12334310 = -779131307;    int wtaksIcGmr20682055 = -83726334;    int wtaksIcGmr41854469 = -79180789;    int wtaksIcGmr78137854 = -849607776;    int wtaksIcGmr94657597 = -742713575;    int wtaksIcGmr8220067 = -998744018;    int wtaksIcGmr35875542 = -574419787;    int wtaksIcGmr22117175 = -942753848;    int wtaksIcGmr91224173 = -378492820;    int wtaksIcGmr29240683 = -648831528;    int wtaksIcGmr82915050 = -337241211;    int wtaksIcGmr27158685 = -23654716;    int wtaksIcGmr32187559 = -398509818;    int wtaksIcGmr88120488 = -215270124;    int wtaksIcGmr59355754 = -978606192;    int wtaksIcGmr36719183 = -144776918;    int wtaksIcGmr52335089 = -380825801;    int wtaksIcGmr3761792 = -308920354;    int wtaksIcGmr16569293 = -559574164;    int wtaksIcGmr70593677 = -617592705;    int wtaksIcGmr64803914 = -246797032;    int wtaksIcGmr11528015 = -319930743;    int wtaksIcGmr57294040 = -248735467;    int wtaksIcGmr13242507 = -157144724;    int wtaksIcGmr24091184 = -465875230;    int wtaksIcGmr66948243 = -746163716;     wtaksIcGmr127262 = wtaksIcGmr50676778;     wtaksIcGmr50676778 = wtaksIcGmr2071156;     wtaksIcGmr2071156 = wtaksIcGmr18158589;     wtaksIcGmr18158589 = wtaksIcGmr55522168;     wtaksIcGmr55522168 = wtaksIcGmr91355086;     wtaksIcGmr91355086 = wtaksIcGmr7371178;     wtaksIcGmr7371178 = wtaksIcGmr47167821;     wtaksIcGmr47167821 = wtaksIcGmr49328520;     wtaksIcGmr49328520 = wtaksIcGmr18522625;     wtaksIcGmr18522625 = wtaksIcGmr29376180;     wtaksIcGmr29376180 = wtaksIcGmr60406612;     wtaksIcGmr60406612 = wtaksIcGmr12421339;     wtaksIcGmr12421339 = wtaksIcGmr75401693;     wtaksIcGmr75401693 = wtaksIcGmr35257302;     wtaksIcGmr35257302 = wtaksIcGmr44632081;     wtaksIcGmr44632081 = wtaksIcGmr63132142;     wtaksIcGmr63132142 = wtaksIcGmr4157226;     wtaksIcGmr4157226 = wtaksIcGmr48453323;     wtaksIcGmr48453323 = wtaksIcGmr86059017;     wtaksIcGmr86059017 = wtaksIcGmr89881464;     wtaksIcGmr89881464 = wtaksIcGmr77323496;     wtaksIcGmr77323496 = wtaksIcGmr70771314;     wtaksIcGmr70771314 = wtaksIcGmr34977181;     wtaksIcGmr34977181 = wtaksIcGmr35096119;     wtaksIcGmr35096119 = wtaksIcGmr38926219;     wtaksIcGmr38926219 = wtaksIcGmr57221575;     wtaksIcGmr57221575 = wtaksIcGmr45174484;     wtaksIcGmr45174484 = wtaksIcGmr35262452;     wtaksIcGmr35262452 = wtaksIcGmr27101999;     wtaksIcGmr27101999 = wtaksIcGmr49980348;     wtaksIcGmr49980348 = wtaksIcGmr56004839;     wtaksIcGmr56004839 = wtaksIcGmr56606586;     wtaksIcGmr56606586 = wtaksIcGmr17131296;     wtaksIcGmr17131296 = wtaksIcGmr59522949;     wtaksIcGmr59522949 = wtaksIcGmr32240553;     wtaksIcGmr32240553 = wtaksIcGmr70855765;     wtaksIcGmr70855765 = wtaksIcGmr99631763;     wtaksIcGmr99631763 = wtaksIcGmr87189626;     wtaksIcGmr87189626 = wtaksIcGmr98937425;     wtaksIcGmr98937425 = wtaksIcGmr73724708;     wtaksIcGmr73724708 = wtaksIcGmr69199409;     wtaksIcGmr69199409 = wtaksIcGmr35469028;     wtaksIcGmr35469028 = wtaksIcGmr92633460;     wtaksIcGmr92633460 = wtaksIcGmr40319584;     wtaksIcGmr40319584 = wtaksIcGmr26876052;     wtaksIcGmr26876052 = wtaksIcGmr3050677;     wtaksIcGmr3050677 = wtaksIcGmr35104401;     wtaksIcGmr35104401 = wtaksIcGmr53950310;     wtaksIcGmr53950310 = wtaksIcGmr6021770;     wtaksIcGmr6021770 = wtaksIcGmr44186949;     wtaksIcGmr44186949 = wtaksIcGmr22821664;     wtaksIcGmr22821664 = wtaksIcGmr23817281;     wtaksIcGmr23817281 = wtaksIcGmr68486097;     wtaksIcGmr68486097 = wtaksIcGmr57775541;     wtaksIcGmr57775541 = wtaksIcGmr22803766;     wtaksIcGmr22803766 = wtaksIcGmr79905463;     wtaksIcGmr79905463 = wtaksIcGmr67093974;     wtaksIcGmr67093974 = wtaksIcGmr83062470;     wtaksIcGmr83062470 = wtaksIcGmr16595950;     wtaksIcGmr16595950 = wtaksIcGmr34133511;     wtaksIcGmr34133511 = wtaksIcGmr62196693;     wtaksIcGmr62196693 = wtaksIcGmr11905369;     wtaksIcGmr11905369 = wtaksIcGmr22226521;     wtaksIcGmr22226521 = wtaksIcGmr68542276;     wtaksIcGmr68542276 = wtaksIcGmr73371340;     wtaksIcGmr73371340 = wtaksIcGmr3800027;     wtaksIcGmr3800027 = wtaksIcGmr95290043;     wtaksIcGmr95290043 = wtaksIcGmr15878744;     wtaksIcGmr15878744 = wtaksIcGmr3016750;     wtaksIcGmr3016750 = wtaksIcGmr73776315;     wtaksIcGmr73776315 = wtaksIcGmr63500379;     wtaksIcGmr63500379 = wtaksIcGmr16967600;     wtaksIcGmr16967600 = wtaksIcGmr49515898;     wtaksIcGmr49515898 = wtaksIcGmr12334310;     wtaksIcGmr12334310 = wtaksIcGmr20682055;     wtaksIcGmr20682055 = wtaksIcGmr41854469;     wtaksIcGmr41854469 = wtaksIcGmr78137854;     wtaksIcGmr78137854 = wtaksIcGmr94657597;     wtaksIcGmr94657597 = wtaksIcGmr8220067;     wtaksIcGmr8220067 = wtaksIcGmr35875542;     wtaksIcGmr35875542 = wtaksIcGmr22117175;     wtaksIcGmr22117175 = wtaksIcGmr91224173;     wtaksIcGmr91224173 = wtaksIcGmr29240683;     wtaksIcGmr29240683 = wtaksIcGmr82915050;     wtaksIcGmr82915050 = wtaksIcGmr27158685;     wtaksIcGmr27158685 = wtaksIcGmr32187559;     wtaksIcGmr32187559 = wtaksIcGmr88120488;     wtaksIcGmr88120488 = wtaksIcGmr59355754;     wtaksIcGmr59355754 = wtaksIcGmr36719183;     wtaksIcGmr36719183 = wtaksIcGmr52335089;     wtaksIcGmr52335089 = wtaksIcGmr3761792;     wtaksIcGmr3761792 = wtaksIcGmr16569293;     wtaksIcGmr16569293 = wtaksIcGmr70593677;     wtaksIcGmr70593677 = wtaksIcGmr64803914;     wtaksIcGmr64803914 = wtaksIcGmr11528015;     wtaksIcGmr11528015 = wtaksIcGmr57294040;     wtaksIcGmr57294040 = wtaksIcGmr13242507;     wtaksIcGmr13242507 = wtaksIcGmr24091184;     wtaksIcGmr24091184 = wtaksIcGmr66948243;     wtaksIcGmr66948243 = wtaksIcGmr127262;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hLmlqPILCQ18112879() {     int zCztKGQhhV12913429 = -881204408;    int zCztKGQhhV37447666 = 15032954;    int zCztKGQhhV19823956 = -96877162;    int zCztKGQhhV26099415 = -263933108;    int zCztKGQhhV49033273 = -37116475;    int zCztKGQhhV34665157 = -989371049;    int zCztKGQhhV28578936 = -381277364;    int zCztKGQhhV73886933 = -627203041;    int zCztKGQhhV44733331 = -344088426;    int zCztKGQhhV24741329 = -396059284;    int zCztKGQhhV56863562 = -224243746;    int zCztKGQhhV92357116 = -387889117;    int zCztKGQhhV82564642 = -106982511;    int zCztKGQhhV25025772 = -509148766;    int zCztKGQhhV58563855 = -19061345;    int zCztKGQhhV73057150 = -318711684;    int zCztKGQhhV18883960 = -983076753;    int zCztKGQhhV87715711 = -745898749;    int zCztKGQhhV49613953 = -477043021;    int zCztKGQhhV88713514 = -44024625;    int zCztKGQhhV70055521 = 61705331;    int zCztKGQhhV6371316 = -997383432;    int zCztKGQhhV9531610 = -535726720;    int zCztKGQhhV79484310 = -489589843;    int zCztKGQhhV40990378 = -652943663;    int zCztKGQhhV20244864 = 79728570;    int zCztKGQhhV68637111 = -17464337;    int zCztKGQhhV12527409 = -663684962;    int zCztKGQhhV51879505 = -282385421;    int zCztKGQhhV79853114 = -621016915;    int zCztKGQhhV94416343 = -579148582;    int zCztKGQhhV59323663 = -663988099;    int zCztKGQhhV2652535 = -281341404;    int zCztKGQhhV94207668 = -599951327;    int zCztKGQhhV42583481 = -673447430;    int zCztKGQhhV27464668 = -479299112;    int zCztKGQhhV7619826 = -758102654;    int zCztKGQhhV57335786 = -476423349;    int zCztKGQhhV92965012 = -874903061;    int zCztKGQhhV25202155 = -819903556;    int zCztKGQhhV35156016 = -41123173;    int zCztKGQhhV76901865 = -261563405;    int zCztKGQhhV70352497 = -671777741;    int zCztKGQhhV37261998 = 3712707;    int zCztKGQhhV20328074 = -279301216;    int zCztKGQhhV504182 = -517076160;    int zCztKGQhhV80632696 = -52935497;    int zCztKGQhhV64544367 = -696356989;    int zCztKGQhhV98014359 = -174996013;    int zCztKGQhhV52972190 = 69626884;    int zCztKGQhhV62514442 = -806773859;    int zCztKGQhhV50998211 = -650262454;    int zCztKGQhhV58302431 = -362761886;    int zCztKGQhhV13185823 = -779668239;    int zCztKGQhhV48057357 = -711035404;    int zCztKGQhhV6542114 = -883820976;    int zCztKGQhhV27916056 = -449240327;    int zCztKGQhhV40339645 = -607287320;    int zCztKGQhhV85109036 = -610989446;    int zCztKGQhhV28788409 = -16845046;    int zCztKGQhhV66028045 = -871906712;    int zCztKGQhhV16051527 = -717592403;    int zCztKGQhhV22007429 = -244817621;    int zCztKGQhhV64880216 = -723071511;    int zCztKGQhhV30324986 = -816910703;    int zCztKGQhhV97539898 = -560255647;    int zCztKGQhhV89704582 = -6547713;    int zCztKGQhhV88356973 = -507031185;    int zCztKGQhhV82442290 = -835701336;    int zCztKGQhhV31099188 = -539762233;    int zCztKGQhhV65437324 = -560609030;    int zCztKGQhhV61548174 = -406653405;    int zCztKGQhhV94750698 = -870995689;    int zCztKGQhhV24411799 = -657139465;    int zCztKGQhhV53557498 = 97098547;    int zCztKGQhhV93153656 = -676731264;    int zCztKGQhhV36018818 = -225605692;    int zCztKGQhhV72269612 = -439439428;    int zCztKGQhhV59156237 = -110288627;    int zCztKGQhhV40486197 = -35867503;    int zCztKGQhhV39612168 = -867335934;    int zCztKGQhhV4092745 = -321107348;    int zCztKGQhhV14513050 = -388688950;    int zCztKGQhhV98907314 = -252012306;    int zCztKGQhhV17338673 = -814243057;    int zCztKGQhhV43418133 = -928886128;    int zCztKGQhhV1021233 = -201226214;    int zCztKGQhhV89466712 = -501673166;    int zCztKGQhhV46150311 = -888915924;    int zCztKGQhhV36041367 = -789626454;    int zCztKGQhhV99548611 = 69941214;    int zCztKGQhhV67280181 = -50815335;    int zCztKGQhhV72226749 = -865433903;    int zCztKGQhhV64176604 = -758058016;    int zCztKGQhhV59174109 = -947996844;    int zCztKGQhhV19104490 = -323530770;    int zCztKGQhhV54894436 = 83254215;    int zCztKGQhhV5472282 = -948706231;    int zCztKGQhhV6937013 = -179376591;    int zCztKGQhhV22788176 = -881204408;     zCztKGQhhV12913429 = zCztKGQhhV37447666;     zCztKGQhhV37447666 = zCztKGQhhV19823956;     zCztKGQhhV19823956 = zCztKGQhhV26099415;     zCztKGQhhV26099415 = zCztKGQhhV49033273;     zCztKGQhhV49033273 = zCztKGQhhV34665157;     zCztKGQhhV34665157 = zCztKGQhhV28578936;     zCztKGQhhV28578936 = zCztKGQhhV73886933;     zCztKGQhhV73886933 = zCztKGQhhV44733331;     zCztKGQhhV44733331 = zCztKGQhhV24741329;     zCztKGQhhV24741329 = zCztKGQhhV56863562;     zCztKGQhhV56863562 = zCztKGQhhV92357116;     zCztKGQhhV92357116 = zCztKGQhhV82564642;     zCztKGQhhV82564642 = zCztKGQhhV25025772;     zCztKGQhhV25025772 = zCztKGQhhV58563855;     zCztKGQhhV58563855 = zCztKGQhhV73057150;     zCztKGQhhV73057150 = zCztKGQhhV18883960;     zCztKGQhhV18883960 = zCztKGQhhV87715711;     zCztKGQhhV87715711 = zCztKGQhhV49613953;     zCztKGQhhV49613953 = zCztKGQhhV88713514;     zCztKGQhhV88713514 = zCztKGQhhV70055521;     zCztKGQhhV70055521 = zCztKGQhhV6371316;     zCztKGQhhV6371316 = zCztKGQhhV9531610;     zCztKGQhhV9531610 = zCztKGQhhV79484310;     zCztKGQhhV79484310 = zCztKGQhhV40990378;     zCztKGQhhV40990378 = zCztKGQhhV20244864;     zCztKGQhhV20244864 = zCztKGQhhV68637111;     zCztKGQhhV68637111 = zCztKGQhhV12527409;     zCztKGQhhV12527409 = zCztKGQhhV51879505;     zCztKGQhhV51879505 = zCztKGQhhV79853114;     zCztKGQhhV79853114 = zCztKGQhhV94416343;     zCztKGQhhV94416343 = zCztKGQhhV59323663;     zCztKGQhhV59323663 = zCztKGQhhV2652535;     zCztKGQhhV2652535 = zCztKGQhhV94207668;     zCztKGQhhV94207668 = zCztKGQhhV42583481;     zCztKGQhhV42583481 = zCztKGQhhV27464668;     zCztKGQhhV27464668 = zCztKGQhhV7619826;     zCztKGQhhV7619826 = zCztKGQhhV57335786;     zCztKGQhhV57335786 = zCztKGQhhV92965012;     zCztKGQhhV92965012 = zCztKGQhhV25202155;     zCztKGQhhV25202155 = zCztKGQhhV35156016;     zCztKGQhhV35156016 = zCztKGQhhV76901865;     zCztKGQhhV76901865 = zCztKGQhhV70352497;     zCztKGQhhV70352497 = zCztKGQhhV37261998;     zCztKGQhhV37261998 = zCztKGQhhV20328074;     zCztKGQhhV20328074 = zCztKGQhhV504182;     zCztKGQhhV504182 = zCztKGQhhV80632696;     zCztKGQhhV80632696 = zCztKGQhhV64544367;     zCztKGQhhV64544367 = zCztKGQhhV98014359;     zCztKGQhhV98014359 = zCztKGQhhV52972190;     zCztKGQhhV52972190 = zCztKGQhhV62514442;     zCztKGQhhV62514442 = zCztKGQhhV50998211;     zCztKGQhhV50998211 = zCztKGQhhV58302431;     zCztKGQhhV58302431 = zCztKGQhhV13185823;     zCztKGQhhV13185823 = zCztKGQhhV48057357;     zCztKGQhhV48057357 = zCztKGQhhV6542114;     zCztKGQhhV6542114 = zCztKGQhhV27916056;     zCztKGQhhV27916056 = zCztKGQhhV40339645;     zCztKGQhhV40339645 = zCztKGQhhV85109036;     zCztKGQhhV85109036 = zCztKGQhhV28788409;     zCztKGQhhV28788409 = zCztKGQhhV66028045;     zCztKGQhhV66028045 = zCztKGQhhV16051527;     zCztKGQhhV16051527 = zCztKGQhhV22007429;     zCztKGQhhV22007429 = zCztKGQhhV64880216;     zCztKGQhhV64880216 = zCztKGQhhV30324986;     zCztKGQhhV30324986 = zCztKGQhhV97539898;     zCztKGQhhV97539898 = zCztKGQhhV89704582;     zCztKGQhhV89704582 = zCztKGQhhV88356973;     zCztKGQhhV88356973 = zCztKGQhhV82442290;     zCztKGQhhV82442290 = zCztKGQhhV31099188;     zCztKGQhhV31099188 = zCztKGQhhV65437324;     zCztKGQhhV65437324 = zCztKGQhhV61548174;     zCztKGQhhV61548174 = zCztKGQhhV94750698;     zCztKGQhhV94750698 = zCztKGQhhV24411799;     zCztKGQhhV24411799 = zCztKGQhhV53557498;     zCztKGQhhV53557498 = zCztKGQhhV93153656;     zCztKGQhhV93153656 = zCztKGQhhV36018818;     zCztKGQhhV36018818 = zCztKGQhhV72269612;     zCztKGQhhV72269612 = zCztKGQhhV59156237;     zCztKGQhhV59156237 = zCztKGQhhV40486197;     zCztKGQhhV40486197 = zCztKGQhhV39612168;     zCztKGQhhV39612168 = zCztKGQhhV4092745;     zCztKGQhhV4092745 = zCztKGQhhV14513050;     zCztKGQhhV14513050 = zCztKGQhhV98907314;     zCztKGQhhV98907314 = zCztKGQhhV17338673;     zCztKGQhhV17338673 = zCztKGQhhV43418133;     zCztKGQhhV43418133 = zCztKGQhhV1021233;     zCztKGQhhV1021233 = zCztKGQhhV89466712;     zCztKGQhhV89466712 = zCztKGQhhV46150311;     zCztKGQhhV46150311 = zCztKGQhhV36041367;     zCztKGQhhV36041367 = zCztKGQhhV99548611;     zCztKGQhhV99548611 = zCztKGQhhV67280181;     zCztKGQhhV67280181 = zCztKGQhhV72226749;     zCztKGQhhV72226749 = zCztKGQhhV64176604;     zCztKGQhhV64176604 = zCztKGQhhV59174109;     zCztKGQhhV59174109 = zCztKGQhhV19104490;     zCztKGQhhV19104490 = zCztKGQhhV54894436;     zCztKGQhhV54894436 = zCztKGQhhV5472282;     zCztKGQhhV5472282 = zCztKGQhhV6937013;     zCztKGQhhV6937013 = zCztKGQhhV22788176;     zCztKGQhhV22788176 = zCztKGQhhV12913429;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rbhThxHfgO36326220() {     int mUclGBhxlG96357734 = -661870088;    int mUclGBhxlG54319755 = 15650766;    int mUclGBhxlG16232463 = -990402390;    int mUclGBhxlG88891665 = -457732944;    int mUclGBhxlG55154596 = -943983966;    int mUclGBhxlG33169558 = -768442702;    int mUclGBhxlG99618253 = -121188678;    int mUclGBhxlG88234376 = -614361071;    int mUclGBhxlG3263032 = -685547777;    int mUclGBhxlG57286981 = -864435396;    int mUclGBhxlG6279500 = -414359619;    int mUclGBhxlG55562516 = -375507363;    int mUclGBhxlG682418 = -998643752;    int mUclGBhxlG4652664 = -297439122;    int mUclGBhxlG19418318 = -113084327;    int mUclGBhxlG59446311 = -106791009;    int mUclGBhxlG12635845 = -736421780;    int mUclGBhxlG68567157 = -993203626;    int mUclGBhxlG12054096 = -475717891;    int mUclGBhxlG78080956 = -573610617;    int mUclGBhxlG25695908 = -627823700;    int mUclGBhxlG18518634 = -988535203;    int mUclGBhxlG13357318 = -575615917;    int mUclGBhxlG14190869 = -561415239;    int mUclGBhxlG82566336 = -112596421;    int mUclGBhxlG15587013 = -900781146;    int mUclGBhxlG18091851 = -745548679;    int mUclGBhxlG52259034 = -733639846;    int mUclGBhxlG87960598 = -144890563;    int mUclGBhxlG95686455 = -33866767;    int mUclGBhxlG12511658 = -216905783;    int mUclGBhxlG22430090 = -579183179;    int mUclGBhxlG46191422 = 97786418;    int mUclGBhxlG36366179 = -191553497;    int mUclGBhxlG21310212 = -871112165;    int mUclGBhxlG39853852 = -267323018;    int mUclGBhxlG76234004 = -636951834;    int mUclGBhxlG97914617 = -184893869;    int mUclGBhxlG41960091 = 40199141;    int mUclGBhxlG74697290 = -572497078;    int mUclGBhxlG68804941 = -712672146;    int mUclGBhxlG27389651 = -835471727;    int mUclGBhxlG93919567 = -122834984;    int mUclGBhxlG29166938 = -852470064;    int mUclGBhxlG12272571 = -7458169;    int mUclGBhxlG40110177 = -536687084;    int mUclGBhxlG56648126 = -185239726;    int mUclGBhxlG84214962 = -695078708;    int mUclGBhxlG28861472 = -760598095;    int mUclGBhxlG88304673 = -137393571;    int mUclGBhxlG35771890 = 72324719;    int mUclGBhxlG6499005 = -708521766;    int mUclGBhxlG72414890 = -574509202;    int mUclGBhxlG37862582 = -498081626;    int mUclGBhxlG10630312 = 40478474;    int mUclGBhxlG77839101 = -673334886;    int mUclGBhxlG40962438 = -408733317;    int mUclGBhxlG2041594 = -328987152;    int mUclGBhxlG6325330 = -245136523;    int mUclGBhxlG39567584 = 56797179;    int mUclGBhxlG15077707 = 77105977;    int mUclGBhxlG47359219 = -387548832;    int mUclGBhxlG273778 = -369470508;    int mUclGBhxlG7576576 = -551681010;    int mUclGBhxlG44775324 = -547529614;    int mUclGBhxlG83849409 = -835176440;    int mUclGBhxlG9371095 = -373293782;    int mUclGBhxlG64316238 = -707090256;    int mUclGBhxlG83342452 = -426326958;    int mUclGBhxlG79564466 = -845761309;    int mUclGBhxlG83212307 = -469839176;    int mUclGBhxlG14721227 = -451527912;    int mUclGBhxlG26607067 = -933402767;    int mUclGBhxlG37356805 = -903220814;    int mUclGBhxlG9276016 = -860938472;    int mUclGBhxlG98306257 = -792351974;    int mUclGBhxlG24599067 = -765700219;    int mUclGBhxlG84190379 = -723145854;    int mUclGBhxlG1918299 = -453957070;    int mUclGBhxlG42456159 = -575909338;    int mUclGBhxlG58938886 = -615541420;    int mUclGBhxlG33876889 = 49530028;    int mUclGBhxlG23397563 = -973041751;    int mUclGBhxlG99655925 = 92503007;    int mUclGBhxlG59914565 = -6191486;    int mUclGBhxlG6012653 = -508384018;    int mUclGBhxlG50015200 = 95326023;    int mUclGBhxlG8328840 = -404131956;    int mUclGBhxlG25735868 = -132031971;    int mUclGBhxlG43471110 = -97777279;    int mUclGBhxlG98891413 = -858589701;    int mUclGBhxlG74192411 = -207964683;    int mUclGBhxlG91589288 = -939757346;    int mUclGBhxlG2392508 = 83401962;    int mUclGBhxlG59619584 = -549603056;    int mUclGBhxlG21445722 = -225123314;    int mUclGBhxlG27115874 = -366001219;    int mUclGBhxlG86342991 = -571153974;    int mUclGBhxlG84391614 = -204940450;    int mUclGBhxlG28423162 = -661870088;     mUclGBhxlG96357734 = mUclGBhxlG54319755;     mUclGBhxlG54319755 = mUclGBhxlG16232463;     mUclGBhxlG16232463 = mUclGBhxlG88891665;     mUclGBhxlG88891665 = mUclGBhxlG55154596;     mUclGBhxlG55154596 = mUclGBhxlG33169558;     mUclGBhxlG33169558 = mUclGBhxlG99618253;     mUclGBhxlG99618253 = mUclGBhxlG88234376;     mUclGBhxlG88234376 = mUclGBhxlG3263032;     mUclGBhxlG3263032 = mUclGBhxlG57286981;     mUclGBhxlG57286981 = mUclGBhxlG6279500;     mUclGBhxlG6279500 = mUclGBhxlG55562516;     mUclGBhxlG55562516 = mUclGBhxlG682418;     mUclGBhxlG682418 = mUclGBhxlG4652664;     mUclGBhxlG4652664 = mUclGBhxlG19418318;     mUclGBhxlG19418318 = mUclGBhxlG59446311;     mUclGBhxlG59446311 = mUclGBhxlG12635845;     mUclGBhxlG12635845 = mUclGBhxlG68567157;     mUclGBhxlG68567157 = mUclGBhxlG12054096;     mUclGBhxlG12054096 = mUclGBhxlG78080956;     mUclGBhxlG78080956 = mUclGBhxlG25695908;     mUclGBhxlG25695908 = mUclGBhxlG18518634;     mUclGBhxlG18518634 = mUclGBhxlG13357318;     mUclGBhxlG13357318 = mUclGBhxlG14190869;     mUclGBhxlG14190869 = mUclGBhxlG82566336;     mUclGBhxlG82566336 = mUclGBhxlG15587013;     mUclGBhxlG15587013 = mUclGBhxlG18091851;     mUclGBhxlG18091851 = mUclGBhxlG52259034;     mUclGBhxlG52259034 = mUclGBhxlG87960598;     mUclGBhxlG87960598 = mUclGBhxlG95686455;     mUclGBhxlG95686455 = mUclGBhxlG12511658;     mUclGBhxlG12511658 = mUclGBhxlG22430090;     mUclGBhxlG22430090 = mUclGBhxlG46191422;     mUclGBhxlG46191422 = mUclGBhxlG36366179;     mUclGBhxlG36366179 = mUclGBhxlG21310212;     mUclGBhxlG21310212 = mUclGBhxlG39853852;     mUclGBhxlG39853852 = mUclGBhxlG76234004;     mUclGBhxlG76234004 = mUclGBhxlG97914617;     mUclGBhxlG97914617 = mUclGBhxlG41960091;     mUclGBhxlG41960091 = mUclGBhxlG74697290;     mUclGBhxlG74697290 = mUclGBhxlG68804941;     mUclGBhxlG68804941 = mUclGBhxlG27389651;     mUclGBhxlG27389651 = mUclGBhxlG93919567;     mUclGBhxlG93919567 = mUclGBhxlG29166938;     mUclGBhxlG29166938 = mUclGBhxlG12272571;     mUclGBhxlG12272571 = mUclGBhxlG40110177;     mUclGBhxlG40110177 = mUclGBhxlG56648126;     mUclGBhxlG56648126 = mUclGBhxlG84214962;     mUclGBhxlG84214962 = mUclGBhxlG28861472;     mUclGBhxlG28861472 = mUclGBhxlG88304673;     mUclGBhxlG88304673 = mUclGBhxlG35771890;     mUclGBhxlG35771890 = mUclGBhxlG6499005;     mUclGBhxlG6499005 = mUclGBhxlG72414890;     mUclGBhxlG72414890 = mUclGBhxlG37862582;     mUclGBhxlG37862582 = mUclGBhxlG10630312;     mUclGBhxlG10630312 = mUclGBhxlG77839101;     mUclGBhxlG77839101 = mUclGBhxlG40962438;     mUclGBhxlG40962438 = mUclGBhxlG2041594;     mUclGBhxlG2041594 = mUclGBhxlG6325330;     mUclGBhxlG6325330 = mUclGBhxlG39567584;     mUclGBhxlG39567584 = mUclGBhxlG15077707;     mUclGBhxlG15077707 = mUclGBhxlG47359219;     mUclGBhxlG47359219 = mUclGBhxlG273778;     mUclGBhxlG273778 = mUclGBhxlG7576576;     mUclGBhxlG7576576 = mUclGBhxlG44775324;     mUclGBhxlG44775324 = mUclGBhxlG83849409;     mUclGBhxlG83849409 = mUclGBhxlG9371095;     mUclGBhxlG9371095 = mUclGBhxlG64316238;     mUclGBhxlG64316238 = mUclGBhxlG83342452;     mUclGBhxlG83342452 = mUclGBhxlG79564466;     mUclGBhxlG79564466 = mUclGBhxlG83212307;     mUclGBhxlG83212307 = mUclGBhxlG14721227;     mUclGBhxlG14721227 = mUclGBhxlG26607067;     mUclGBhxlG26607067 = mUclGBhxlG37356805;     mUclGBhxlG37356805 = mUclGBhxlG9276016;     mUclGBhxlG9276016 = mUclGBhxlG98306257;     mUclGBhxlG98306257 = mUclGBhxlG24599067;     mUclGBhxlG24599067 = mUclGBhxlG84190379;     mUclGBhxlG84190379 = mUclGBhxlG1918299;     mUclGBhxlG1918299 = mUclGBhxlG42456159;     mUclGBhxlG42456159 = mUclGBhxlG58938886;     mUclGBhxlG58938886 = mUclGBhxlG33876889;     mUclGBhxlG33876889 = mUclGBhxlG23397563;     mUclGBhxlG23397563 = mUclGBhxlG99655925;     mUclGBhxlG99655925 = mUclGBhxlG59914565;     mUclGBhxlG59914565 = mUclGBhxlG6012653;     mUclGBhxlG6012653 = mUclGBhxlG50015200;     mUclGBhxlG50015200 = mUclGBhxlG8328840;     mUclGBhxlG8328840 = mUclGBhxlG25735868;     mUclGBhxlG25735868 = mUclGBhxlG43471110;     mUclGBhxlG43471110 = mUclGBhxlG98891413;     mUclGBhxlG98891413 = mUclGBhxlG74192411;     mUclGBhxlG74192411 = mUclGBhxlG91589288;     mUclGBhxlG91589288 = mUclGBhxlG2392508;     mUclGBhxlG2392508 = mUclGBhxlG59619584;     mUclGBhxlG59619584 = mUclGBhxlG21445722;     mUclGBhxlG21445722 = mUclGBhxlG27115874;     mUclGBhxlG27115874 = mUclGBhxlG86342991;     mUclGBhxlG86342991 = mUclGBhxlG84391614;     mUclGBhxlG84391614 = mUclGBhxlG28423162;     mUclGBhxlG28423162 = mUclGBhxlG96357734;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ACqpqRwiil6782093() {     int TIlAdetuEM9143902 = -796910780;    int TIlAdetuEM41090644 = -154683248;    int TIlAdetuEM33985262 = -298761771;    int TIlAdetuEM96832491 = -281935221;    int TIlAdetuEM48665701 = -335800431;    int TIlAdetuEM76479628 = -545366015;    int TIlAdetuEM20826012 = -666860748;    int TIlAdetuEM14953489 = 41120672;    int TIlAdetuEM98667841 = -647305869;    int TIlAdetuEM63505686 = -668523029;    int TIlAdetuEM33766882 = -529045634;    int TIlAdetuEM87513021 = -882840179;    int TIlAdetuEM70825720 = -902426409;    int TIlAdetuEM54276742 = -629854321;    int TIlAdetuEM42724871 = -20070158;    int TIlAdetuEM87871380 = -657682398;    int TIlAdetuEM68387662 = -400196247;    int TIlAdetuEM52125643 = -548413928;    int TIlAdetuEM13214726 = -385187087;    int TIlAdetuEM80735452 = -72854756;    int TIlAdetuEM5869965 = -19139964;    int TIlAdetuEM47566453 = -58393550;    int TIlAdetuEM52117613 = -506619812;    int TIlAdetuEM58697998 = -921760251;    int TIlAdetuEM88460595 = -323038346;    int TIlAdetuEM96905658 = -591212345;    int TIlAdetuEM29507388 = -279254960;    int TIlAdetuEM19611960 = -907437951;    int TIlAdetuEM4577652 = -359107803;    int TIlAdetuEM48437571 = -763752922;    int TIlAdetuEM56947653 = -493322533;    int TIlAdetuEM25748914 = -557902857;    int TIlAdetuEM92237370 = -878068366;    int TIlAdetuEM13442553 = -96307715;    int TIlAdetuEM4370744 = -381144045;    int TIlAdetuEM35077967 = -55886122;    int TIlAdetuEM12998065 = -426861184;    int TIlAdetuEM55618641 = -104513960;    int TIlAdetuEM47735477 = -901651436;    int TIlAdetuEM962020 = -416913924;    int TIlAdetuEM30236249 = 11853861;    int TIlAdetuEM35092107 = -733783061;    int TIlAdetuEM28803037 = -146268429;    int TIlAdetuEM73795476 = -93642306;    int TIlAdetuEM92281061 = 99771870;    int TIlAdetuEM13738307 = -610005524;    int TIlAdetuEM34230146 = -682754779;    int TIlAdetuEM13654929 = -850431487;    int TIlAdetuEM72925521 = -924200070;    int TIlAdetuEM35255095 = -748430034;    int TIlAdetuEM54099384 = -180559589;    int TIlAdetuEM34675552 = -79707104;    int TIlAdetuEM6900040 = -750512485;    int TIlAdetuEM82562306 = -87533368;    int TIlAdetuEM912129 = 46033987;    int TIlAdetuEM61577449 = -638517230;    int TIlAdetuEM88973030 = -648063436;    int TIlAdetuEM75287263 = -377001521;    int TIlAdetuEM8371896 = -958896875;    int TIlAdetuEM51760043 = -744588087;    int TIlAdetuEM46972241 = -166111056;    int TIlAdetuEM1214053 = -759422798;    int TIlAdetuEM10375837 = -599771525;    int TIlAdetuEM50230271 = -883552948;    int TIlAdetuEM6558033 = -75200497;    int TIlAdetuEM8017969 = -971142777;    int TIlAdetuEM95275650 = 95228186;    int TIlAdetuEM57383168 = -706118694;    int TIlAdetuEM49905999 = -148710276;    int TIlAdetuEM7646905 = -964184036;    int TIlAdetuEM74873315 = -130821214;    int TIlAdetuEM12769022 = -195682287;    int TIlAdetuEM4390166 = -646762492;    int TIlAdetuEM12252706 = -968273163;    int TIlAdetuEM50499204 = 15291382;    int TIlAdetuEM70777858 = -285356903;    int TIlAdetuEM18763416 = -912125122;    int TIlAdetuEM78322137 = -312977506;    int TIlAdetuEM66416937 = -921532122;    int TIlAdetuEM74722288 = -713032823;    int TIlAdetuEM62675512 = -908457567;    int TIlAdetuEM15852459 = -428823473;    int TIlAdetuEM46686438 = -983237881;    int TIlAdetuEM69322557 = -610677770;    int TIlAdetuEM94338187 = -483193333;    int TIlAdetuEM22272102 = -313615430;    int TIlAdetuEM18848874 = -807390373;    int TIlAdetuEM9675064 = -690534998;    int TIlAdetuEM12530425 = -42341703;    int TIlAdetuEM42793294 = -742626816;    int TIlAdetuEM46104936 = -407822687;    int TIlAdetuEM37710801 = 50140336;    int TIlAdetuEM47246745 = -145617086;    int TIlAdetuEM95975433 = -57063350;    int TIlAdetuEM53989779 = -150802868;    int TIlAdetuEM29022197 = -228723342;    int TIlAdetuEM24716270 = -34011537;    int TIlAdetuEM78572766 = -262715481;    int TIlAdetuEM67237443 = 81558190;    int TIlAdetuEM84263093 = -796910780;     TIlAdetuEM9143902 = TIlAdetuEM41090644;     TIlAdetuEM41090644 = TIlAdetuEM33985262;     TIlAdetuEM33985262 = TIlAdetuEM96832491;     TIlAdetuEM96832491 = TIlAdetuEM48665701;     TIlAdetuEM48665701 = TIlAdetuEM76479628;     TIlAdetuEM76479628 = TIlAdetuEM20826012;     TIlAdetuEM20826012 = TIlAdetuEM14953489;     TIlAdetuEM14953489 = TIlAdetuEM98667841;     TIlAdetuEM98667841 = TIlAdetuEM63505686;     TIlAdetuEM63505686 = TIlAdetuEM33766882;     TIlAdetuEM33766882 = TIlAdetuEM87513021;     TIlAdetuEM87513021 = TIlAdetuEM70825720;     TIlAdetuEM70825720 = TIlAdetuEM54276742;     TIlAdetuEM54276742 = TIlAdetuEM42724871;     TIlAdetuEM42724871 = TIlAdetuEM87871380;     TIlAdetuEM87871380 = TIlAdetuEM68387662;     TIlAdetuEM68387662 = TIlAdetuEM52125643;     TIlAdetuEM52125643 = TIlAdetuEM13214726;     TIlAdetuEM13214726 = TIlAdetuEM80735452;     TIlAdetuEM80735452 = TIlAdetuEM5869965;     TIlAdetuEM5869965 = TIlAdetuEM47566453;     TIlAdetuEM47566453 = TIlAdetuEM52117613;     TIlAdetuEM52117613 = TIlAdetuEM58697998;     TIlAdetuEM58697998 = TIlAdetuEM88460595;     TIlAdetuEM88460595 = TIlAdetuEM96905658;     TIlAdetuEM96905658 = TIlAdetuEM29507388;     TIlAdetuEM29507388 = TIlAdetuEM19611960;     TIlAdetuEM19611960 = TIlAdetuEM4577652;     TIlAdetuEM4577652 = TIlAdetuEM48437571;     TIlAdetuEM48437571 = TIlAdetuEM56947653;     TIlAdetuEM56947653 = TIlAdetuEM25748914;     TIlAdetuEM25748914 = TIlAdetuEM92237370;     TIlAdetuEM92237370 = TIlAdetuEM13442553;     TIlAdetuEM13442553 = TIlAdetuEM4370744;     TIlAdetuEM4370744 = TIlAdetuEM35077967;     TIlAdetuEM35077967 = TIlAdetuEM12998065;     TIlAdetuEM12998065 = TIlAdetuEM55618641;     TIlAdetuEM55618641 = TIlAdetuEM47735477;     TIlAdetuEM47735477 = TIlAdetuEM962020;     TIlAdetuEM962020 = TIlAdetuEM30236249;     TIlAdetuEM30236249 = TIlAdetuEM35092107;     TIlAdetuEM35092107 = TIlAdetuEM28803037;     TIlAdetuEM28803037 = TIlAdetuEM73795476;     TIlAdetuEM73795476 = TIlAdetuEM92281061;     TIlAdetuEM92281061 = TIlAdetuEM13738307;     TIlAdetuEM13738307 = TIlAdetuEM34230146;     TIlAdetuEM34230146 = TIlAdetuEM13654929;     TIlAdetuEM13654929 = TIlAdetuEM72925521;     TIlAdetuEM72925521 = TIlAdetuEM35255095;     TIlAdetuEM35255095 = TIlAdetuEM54099384;     TIlAdetuEM54099384 = TIlAdetuEM34675552;     TIlAdetuEM34675552 = TIlAdetuEM6900040;     TIlAdetuEM6900040 = TIlAdetuEM82562306;     TIlAdetuEM82562306 = TIlAdetuEM912129;     TIlAdetuEM912129 = TIlAdetuEM61577449;     TIlAdetuEM61577449 = TIlAdetuEM88973030;     TIlAdetuEM88973030 = TIlAdetuEM75287263;     TIlAdetuEM75287263 = TIlAdetuEM8371896;     TIlAdetuEM8371896 = TIlAdetuEM51760043;     TIlAdetuEM51760043 = TIlAdetuEM46972241;     TIlAdetuEM46972241 = TIlAdetuEM1214053;     TIlAdetuEM1214053 = TIlAdetuEM10375837;     TIlAdetuEM10375837 = TIlAdetuEM50230271;     TIlAdetuEM50230271 = TIlAdetuEM6558033;     TIlAdetuEM6558033 = TIlAdetuEM8017969;     TIlAdetuEM8017969 = TIlAdetuEM95275650;     TIlAdetuEM95275650 = TIlAdetuEM57383168;     TIlAdetuEM57383168 = TIlAdetuEM49905999;     TIlAdetuEM49905999 = TIlAdetuEM7646905;     TIlAdetuEM7646905 = TIlAdetuEM74873315;     TIlAdetuEM74873315 = TIlAdetuEM12769022;     TIlAdetuEM12769022 = TIlAdetuEM4390166;     TIlAdetuEM4390166 = TIlAdetuEM12252706;     TIlAdetuEM12252706 = TIlAdetuEM50499204;     TIlAdetuEM50499204 = TIlAdetuEM70777858;     TIlAdetuEM70777858 = TIlAdetuEM18763416;     TIlAdetuEM18763416 = TIlAdetuEM78322137;     TIlAdetuEM78322137 = TIlAdetuEM66416937;     TIlAdetuEM66416937 = TIlAdetuEM74722288;     TIlAdetuEM74722288 = TIlAdetuEM62675512;     TIlAdetuEM62675512 = TIlAdetuEM15852459;     TIlAdetuEM15852459 = TIlAdetuEM46686438;     TIlAdetuEM46686438 = TIlAdetuEM69322557;     TIlAdetuEM69322557 = TIlAdetuEM94338187;     TIlAdetuEM94338187 = TIlAdetuEM22272102;     TIlAdetuEM22272102 = TIlAdetuEM18848874;     TIlAdetuEM18848874 = TIlAdetuEM9675064;     TIlAdetuEM9675064 = TIlAdetuEM12530425;     TIlAdetuEM12530425 = TIlAdetuEM42793294;     TIlAdetuEM42793294 = TIlAdetuEM46104936;     TIlAdetuEM46104936 = TIlAdetuEM37710801;     TIlAdetuEM37710801 = TIlAdetuEM47246745;     TIlAdetuEM47246745 = TIlAdetuEM95975433;     TIlAdetuEM95975433 = TIlAdetuEM53989779;     TIlAdetuEM53989779 = TIlAdetuEM29022197;     TIlAdetuEM29022197 = TIlAdetuEM24716270;     TIlAdetuEM24716270 = TIlAdetuEM78572766;     TIlAdetuEM78572766 = TIlAdetuEM67237443;     TIlAdetuEM67237443 = TIlAdetuEM84263093;     TIlAdetuEM84263093 = TIlAdetuEM9143902;}
// Junk Finished
