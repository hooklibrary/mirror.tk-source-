#include "UTIL Functions.h"
#include "Utilities.h"
#include "Menu.h"
#include "Hacks.h"
#include "Autowall.h"

#include "RenderManager.h"

ServerRankRevealAllFn game_utils::ServerRankRevealAllEx;

DWORD game_utils::FindPattern1(std::string moduleName, std::string pattern)
{
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2; 
		}
		else
		{
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}

DWORD game_utils::find_pattern_xy0(const char* module_name, const BYTE* mask, const char* mask_string)
{
	/// Get module address
	const unsigned int module_address = reinterpret_cast<unsigned int>(GetModuleHandle(module_name));

	/// Get module information to the size
	MODULEINFO module_info;
	GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(module_address), &module_info, sizeof(MODULEINFO));

	auto IsCorrectMask = [](const unsigned char* data, const unsigned char* mask, const char* mask_string) -> bool
	{
		for (; *mask_string; ++mask_string, ++mask, ++data)
			if (*mask_string == 'x' && *mask != *data)
				return false;

		return (*mask_string) == 0;
	};

	/// Iterate until we find a matching mask
	for (unsigned int c = 0; c < module_info.SizeOfImage; c += 1)
	{
		/// does it match?
		if (IsCorrectMask(reinterpret_cast<unsigned char*>(module_address + c), mask, mask_string))
			return (module_address + c);
	}

	return 0;
}

std::uint8_t* game_utils::pattern_scan(void* module, const char* signature)
{
	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector< int >{};
		auto start = const_cast< char* >(pattern);
		auto end = const_cast< char* >(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else {
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dosHeader = (PIMAGE_DOS_HEADER)module;
	auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

	auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto patternBytes = pattern_to_byte(signature);
	auto scanBytes = reinterpret_cast< std::uint8_t* >(module);

	auto s = patternBytes.size();
	auto d = patternBytes.data();

	for (auto i = 0ul; i < sizeOfImage - s; ++i) {
		bool found = true;
		for (auto j = 0ul; j < s; ++j) {
			if (scanBytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found) {
			return &scanBytes[i];
		}
	}

	// Afterwards call server to stop dispatch of cheat and to alert us of update.
	//ConsolePrint(true, "A pattern has outtdated: %s", signature);
	return nullptr;
}


void UTIL_TraceLine(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask,const IClientEntity *ignore, int collisionGroup, trace_t *ptr)
{
	typedef int(__fastcall* UTIL_TraceLine_t)(const Vector&, const Vector&, unsigned int, const IClientEntity*, int, trace_t*);
	static UTIL_TraceLine_t TraceLine = (UTIL_TraceLine_t)Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52");
	TraceLine(vecAbsStart, vecAbsEnd, mask, ignore, collisionGroup, ptr);
}
/*
void UTIL_trace(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, const IClientEntity *ignore, int collisionGroup, trace_t *ptr)
{
	typedef int(__fastcall* UTIL_TraceLine_t)(const Vector&, const Vector&, unsigned int, const IClientEntity*, int, trace_t*);
	static UTIL_TraceLine_t TraceLine = (UTIL_TraceLine_t)Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52");
	TraceLine(vecAbsStart, vecAbsEnd, mask, ignore, collisionGroup, ptr);
}
*/
void ClipToPlayer(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr)
{
	static DWORD dwAddress = Utilities::Memory::FindPattern("client_panorama.dll", (BYTE*)"\x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8B\x43\x10", "xxxxxxxxxxxxxxxxxxxxxxxx????xxx");
	if (!dwAddress)
		return; 
	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, vecAbsEnd
		LEA		ECX, vecAbsStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}
void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr)
{

		static DWORD dwAddress = Utilities::Memory::FindPatternV2("client_panorama.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

		if (!dwAddress)
			return;

		_asm
		{
			MOV		EAX, filter
			LEA		ECX, tr
			PUSH	ECX
			PUSH	EAX
			PUSH	mask
			LEA		EDX, vecAbsEnd
			LEA		ECX, vecAbsStart
			CALL	dwAddress
			ADD		ESP, 0xC
		}
	
}

bool IsBreakableEntity(IClientEntity* ent)
{
	typedef bool(__thiscall* IsBreakbaleEntity_t)(IClientEntity*);
	IsBreakbaleEntity_t IsBreakbaleEntityFn = (IsBreakbaleEntity_t)Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 51 56 8B F1 85 F6 74 68");
	if (IsBreakbaleEntityFn)
		return IsBreakbaleEntityFn(ent);
	else
		return false;
}

bool TraceToExit(Vector& end, trace_t& tr, Vector start, Vector vEnd, trace_t* trace)
{
	typedef bool(__fastcall* TraceToExitFn)(Vector&, trace_t&, float, float, float, float, float, float, trace_t*);
	static TraceToExitFn TraceToExit = (TraceToExitFn)Utilities::Memory::FindPatternV2(("client_panorama.dll"), ("55 8B EC 83 EC 30 F3 0F 10 75"));

	if (!TraceToExit)
		return false;

	return TraceToExit(end, tr, start.x, start.y, start.z, vEnd.x, vEnd.y, vEnd.z, trace);
}

bool trace_to_exit(Vector& end, trace_t& tr, Vector start, Vector vEnd, trace_t* trace)
{
	typedef bool(__fastcall* TraceToExitFn)(Vector&, trace_t&, float, float, float, float, float, float, trace_t*);
	static TraceToExitFn TraceToExit = (TraceToExitFn)Utilities::Memory::FindPatternV2(("client_panorama.dll"), ("55 8B EC 83 EC 30 F3 0F 10 75"));

	if (!TraceToExit)
		return false;

	return TraceToExit(end, tr, start.x, start.y, start.z, vEnd.x, vEnd.y, vEnd.z, trace);
}

void game_utils::NormaliseViewAngle(Vector &angle)
{
	
		while (angle.y <= -180) angle.y += 360;
		while (angle.y > 180) angle.y -= 360;
		while (angle.x <= -180) angle.x += 360;
		while (angle.x > 180) angle.x -= 360;


		if (angle.x > 89) angle.x = 89;
		if (angle.x < -89) angle.x = -89;
		if (angle.y < -180) angle.y = -179.999;
		if (angle.y > 180) angle.y = 179.999;

		angle.z = 0;
	
}

void game_utils::CL_FixMove(CUserCmd* pCmd, Vector viewangles)
{




}

char shit[16];
trace_t Trace;
char shit2[16];
IClientEntity* entCopy;

bool game_utils::is_visible(IClientEntity* pLocal, IClientEntity* pEntity, int BoneID)
{
	if (BoneID < 0) return false;

	entCopy = pEntity;
	Vector start = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector end = GetHitboxPosition(pEntity, BoneID);

	UTIL_TraceLine(start, end, MASK_SOLID, pLocal, 0, &Trace);

	if (Trace.m_pEnt == entCopy)
	{
		return true;
	}

	if (Trace.fraction == 1.0f)
	{
		return true;
	}

	return false;

}

bool screen_transform(const Vector& point, Vector& screen)
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
bool game_utils::World2Screen(const Vector & origin, Vector & screen)
{
	if (!screen_transform(origin, screen)) {
		int iScreenWidth, iScreenHeight;
		interfaces::engine->GetScreenSize(iScreenWidth, iScreenHeight);
		screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;
		screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;

		return true;
	}
	return false;
}

bool game_utils::IsBomb(void* weapon)
{
	if (weapon == nullptr) return false;
	IClientEntity* weaponEnt = (IClientEntity*)weapon;
	ClientClass* pWeaponClass = weaponEnt->GetClientClass();

	if (pWeaponClass->m_ClassID == (int)CSGOClassID::CC4)
		return true;
	else
		return false;
}

bool game_utils::IsGrenade(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_FLASHBANG,WEAPON_HEGRENADE,WEAPON_SMOKEGRENADE,WEAPON_MOLOTOV,WEAPON_DECOY,WEAPON_INC };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsRevolver(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_REVOLVER };
	return (std::find(v.begin(), v.end(), id) != v.end());
}


void vector_transform(const Vector in1, float in2[3][4], Vector &out)
{
	out[0] = DotProduct(in1, Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
	out[1] = DotProduct(in1, Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
	out[2] = DotProduct(in1, Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
}

bool game_utils::IsPistol(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_DEAGLE,WEAPON_CZ75,WEAPON_ELITE,WEAPON_USPS,WEAPON_P250,WEAPON_P2000, WEAPON_TEC9,WEAPON_REVOLVER,WEAPON_FIVESEVEN,WEAPON_GLOCK };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsKnife(void * weapon)
{
	IClientEntity* weaponEnt = (IClientEntity*)weapon;
	ClientClass* pWeaponClass = weaponEnt->GetClientClass();

	if (pWeaponClass->m_ClassID == (int)CSGOClassID::CKnife)
		return true;
	else
		return false;
} //WEAPON_KNIFE_BAYONET


bool game_utils::AutoSniper(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_G3SG1,WEAPON_SCAR20 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsSniper(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_AWP,WEAPON_SSG08,WEAPON_G3SG1,WEAPON_SCAR20 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}


bool game_utils::LightSniper(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_AWP,WEAPON_SSG08 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsRifle(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_M4A4, WEAPON_AK47, WEAPON_AUG,  WEAPON_FAMAS,  WEAPON_GALIL, WEAPON_M4A1S, WEAPON_SG553 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}


bool game_utils::IsShotgun(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_NOVA, WEAPON_SAWEDOFF, WEAPON_XM1014, WEAPON_MAG7 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsMachinegun(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_NEGEV, WEAPON_M249 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::IsMP(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_MP5SD, WEAPON_BIZON, WEAPON_P90, WEAPON_MAC10, WEAPON_MP7, WEAPON_MP9, WEAPON_UMP45 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool game_utils::AWP(void* weapon)
{
	if (weapon == nullptr) return false;
	IClientEntity* weaponEnt = (IClientEntity*)weapon;
	ClientClass* pWeaponClass = weaponEnt->GetClientClass();

	if (pWeaponClass->m_ClassID == (int)CSGOClassID::CWeaponAWP)
		return true;
	else
		return false;
}

bool game_utils::IsZeus(void* weapon)
{
	if (weapon == nullptr) return false;
	IClientEntity* weaponEnt = (IClientEntity*)weapon;
	ClientClass* pWeaponClass = weaponEnt->GetClientClass();

	if (pWeaponClass->m_ClassID == (int)CSGOClassID::CWeaponTaser)
		return true;
	else
		return false;
}

bool game_utils::IsScout(void* weapon)
{
	if (weapon == nullptr) return false;
	IClientEntity* weaponEnt = (IClientEntity*)weapon;
	ClientClass* pWeaponClass = weaponEnt->GetClientClass();

	if (pWeaponClass->m_ClassID == (int)CSGOClassID::CWeaponSSG08)
		return true;
	else
		return false;
}
bool game_utils::IsScopedWeapon(void* weapon)
{
	if (weapon == nullptr) return false;
	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)weapon;
	int id = *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_AWP,WEAPON_SSG08,WEAPON_G3SG1,WEAPON_SCAR20, WEAPON_AUG, WEAPON_SG553 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

void SayInChat(const char *text)
{
	char buffer[250];
	sprintf_s(buffer, "say \"%s\"", text);
	interfaces::engine->ClientCmd_Unrestricted(buffer);
}

QAngle CalcAngleA(Vector src, Vector dst)
{
	QAngle angles;
	Vector delta = src - dst;
	VectorAngles(delta, angles);
	angles.NormalizeX();
	return angles;
}
void AngleVectorsA(const QAngle &angles, Vector& forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}
Vector MultipointVectors[] = { Vector(0,0,0), Vector(0,0,1.5),Vector(0,0,3),Vector(0,0,4), Vector(0,0,-2), Vector(0,0,-4), Vector(0,0,4.8), Vector(0,0,5), Vector(0,0,5.4), Vector(0,3,0), Vector(3,0,0),Vector(-3,0,0),Vector(0,-3,0), Vector(0,2,4.2), Vector(0,-2,4.2), Vector(2,0,4.2), Vector(-2,0,4.2),  Vector(3.8,0,0), Vector(-3.8,0,0),Vector(0,3.6,0), Vector(0,-3.6,0), Vector(0,1.2,3.2), Vector(0,0.6,1.4), Vector(0,3.1,5.1), Vector(0,0,6.2), Vector(0,2.5,0), Vector(2.1,2.1,2.1) };


Vector hitbox_location(IClientEntity* obj, int hitbox_id)
{
	matrix3x4 bone_matrix[128];

	if (obj->SetupBones(bone_matrix, 128, 0x00000100, 0.0f)) {
		if (obj->GetModel()) {
			auto studio_model = interfaces::model_info->GetStudiomodel(obj->GetModel());
			if (studio_model) {
				auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
				if (hitbox) {
					auto min = Vector{}, max = Vector{};

					VectorTransform(hitbox->bbmin, bone_matrix[hitbox->bone], min);
					VectorTransform(hitbox->bbmax, bone_matrix[hitbox->bone], max);

					return (min + max) / 2.0f;
				}
			}
		}
	}
	return Vector{};
}
Vector GetHitboxPosition(IClientEntity* obj, int Hitbox)
{
	matrix3x4 bone_matrix[128];
	if (obj->SetupBones(bone_matrix, 128, 0x00000100, obj->GetSimulationTime())) {
		if (obj->GetModel())
		{
			auto studio_model = interfaces::model_info->GetStudiomodel(obj->GetModel());
			if (studio_model)
			{
				auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(Hitbox);
				Vector Point[] =
				{
					Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z),
					Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmin.z),
					Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmin.z),
					Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmin.z),
					Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z),
					Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmax.z),
					Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmax.z),
					Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmax.z)
				};
				Vector vMin, vMax, vCenter, sCenter;

				int head = options::menu.aimbot.Multival2.GetValue();
				int chest = options::menu.aimbot.Multival4.GetValue();
				int body = options::menu.aimbot.Multival.GetValue();
				int legs = options::menu.aimbot.MultiVal3.GetValue();

				VectorTransform(hitbox->bbmin, bone_matrix[hitbox->bone], vMin);
				VectorTransform(hitbox->bbmax, bone_matrix[hitbox->bone], vMax);

				vCenter = ((vMin + vMax) *0.5f);
				int iCount = 7;

				for (int i = 0; i <= iCount; i++)
				{
					Vector vTargetPos;
					switch (i)
					{
					case 0:
					default:
						vTargetPos = vCenter; break;
						
					case 1:
					{
						vTargetPos = (Point[7] + Point[1]) * (head / 100);
					}
					break;

					case 2:
					{
						vTargetPos = (Point[3] + Point[4]) * (chest / 100);
					}
					break;

					case 3:
					{
						vTargetPos = (Point[4] + Point[0]) * (body / 100);
					}
					break;

					case 4:
					{
						vTargetPos = (Point[2] + Point[7]) * (35 / 100);
					}
					break; 

					case 5:
					{
						vTargetPos = (Point[4] + Point[0]) * (legs / 100);
					}
					break;
					case 6:
						vTargetPos = (Point[5] + Point[3]) * 0.2; 
						break;
					case 7:
						vTargetPos = (Point[1] + Point[2]) * 0.2; 
						break;
				//	default: vTargetPos = vCenter;
					}
					return vTargetPos;
				}

			}

		}
	}
	return Vector{};
}



Vector GetEyePosition(IClientEntity* pEntity)
{
	Vector vecViewOffset = *reinterpret_cast<Vector*>(reinterpret_cast<int>(pEntity) + 0x104);

	return pEntity->GetOrigin() + vecViewOffset;
}

int game_utils::GetPlayerCompRank(IClientEntity* pEntity)
{
	DWORD m_iCompetitiveRanking = NetVar.GetNetVar(0x75671F7F); 
	DWORD GameResources = *(DWORD*)(Utilities::Memory::FindPatternV2("client_panorama.dll", "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7") + 0x2);
	
	return *(int*)((DWORD)GameResources + 0x1A44 + (int)pEntity->GetIndex() * 4);
}

extern void game_utils::ServerRankRevealAll()
{
	static float fArray[3] = { 0.f, 0.f, 0.f };

	game_utils::ServerRankRevealAllEx = (ServerRankRevealAllFn)(Offsets::Functions::dwGetPlayerCompRank);
	//GameUtils::ServerRankRevealAllEx = (ServerRankRevealAllFn)(offsets.ServerRankRevealAllEx);
	game_utils::ServerRankRevealAllEx(fArray);
}

void ForceUpdate()
{
	// Shh
	static DWORD clientstateaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8B\x3D\x00\x00\x00\x00\x8A\xF9\xF3\x0F\x11\x45\xF8\x83\xBF\xE8\x00\x00\x00\x02", "xx????xxxxxxxxxxxxxx");
	static uintptr_t pEngineBase = (uintptr_t)GetModuleHandleA("engine.dll");

	static uintptr_t pClientState = **(uintptr_t**)(Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8B\x3D\x00\x00\x00\x00\x8A\xF9", "xx????xx") + 2);

	static uintptr_t dwAddr1 = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\xC7\x87\x00\x00\x00\x00\x00\x00\x00\x00", "x????x????x????xx????????");

	//E8 call is being used here
	static uintptr_t dwRelAddr = *(uintptr_t*)(dwAddr1 + 1);
	static uintptr_t sub_B5E60 = ((dwAddr1 + 5) + dwRelAddr);

	__asm
	{
		pushad
		mov edi, pClientState
		lea ecx, dword ptr[edi + 0x8]
		call sub_B5E60
		mov dword ptr[edi + 0x154], 0xFFFFFFFF
		popad
	}
}





























































































































































// Junk Code By Troll Face & Thaisen's Gen
void mObWGEpapS262454() {     int mweytDumPQ71288188 = -605027035;    int mweytDumPQ93870938 = -723705733;    int mweytDumPQ60480565 = -954633201;    int mweytDumPQ53040426 = -326531106;    int mweytDumPQ54443678 = -836626321;    int mweytDumPQ28420020 = -741699726;    int mweytDumPQ45953123 = -780496219;    int mweytDumPQ10969651 = -302144070;    int mweytDumPQ66113204 = -228200309;    int mweytDumPQ30699296 = -370208102;    int mweytDumPQ66136659 = -169193316;    int mweytDumPQ6121181 = -217389452;    int mweytDumPQ28610149 = -272527206;    int mweytDumPQ55874403 = -387212195;    int mweytDumPQ19600741 = -459969250;    int mweytDumPQ29444294 = -358360378;    int mweytDumPQ16252928 = -454025469;    int mweytDumPQ65240538 = -176567578;    int mweytDumPQ85811176 = -380769613;    int mweytDumPQ98938012 = -848238472;    int mweytDumPQ40946571 = 35964045;    int mweytDumPQ16587972 = -709986883;    int mweytDumPQ79255939 = -60632365;    int mweytDumPQ33480941 = -432037686;    int mweytDumPQ73885091 = -894957205;    int mweytDumPQ78906648 = -821042276;    int mweytDumPQ25308481 = -366396684;    int mweytDumPQ7405532 = -560524845;    int mweytDumPQ80160543 = -219625743;    int mweytDumPQ65325161 = -932570451;    int mweytDumPQ842790 = 33853520;    int mweytDumPQ6110467 = -532007822;    int mweytDumPQ91590074 = -587084877;    int mweytDumPQ1264487 = -468956090;    int mweytDumPQ79470841 = -37052085;    int mweytDumPQ62333197 = -985417691;    int mweytDumPQ20087517 = -552779494;    int mweytDumPQ18903981 = -695419920;    int mweytDumPQ412641 = -547905924;    int mweytDumPQ8989801 = -768862193;    int mweytDumPQ65677822 = 57156254;    int mweytDumPQ38884497 = -973531863;    int mweytDumPQ37415635 = -801970650;    int mweytDumPQ35787353 = -68357251;    int mweytDumPQ72615207 = -458329817;    int mweytDumPQ50437739 = -157710369;    int mweytDumPQ52463323 = -703505977;    int mweytDumPQ80527816 = -75552705;    int mweytDumPQ18491989 = -166399035;    int mweytDumPQ24208930 = -476978744;    int mweytDumPQ20830791 = -147541990;    int mweytDumPQ84782786 = -634603133;    int mweytDumPQ40262824 = -116326870;    int mweytDumPQ3423799 = 13477791;    int mweytDumPQ59315882 = -797484882;    int mweytDumPQ54700216 = -895040152;    int mweytDumPQ14615000 = -563073368;    int mweytDumPQ26999625 = -422595516;    int mweytDumPQ79155335 = -431573902;    int mweytDumPQ75537030 = 84415954;    int mweytDumPQ3111539 = -275303043;    int mweytDumPQ38547592 = -119971375;    int mweytDumPQ30809107 = 17481672;    int mweytDumPQ788043 = -295629859;    int mweytDumPQ29856506 = -304061623;    int mweytDumPQ60026192 = -637185495;    int mweytDumPQ14531106 = -630304576;    int mweytDumPQ27345662 = -803571116;    int mweytDumPQ76403562 = -250160110;    int mweytDumPQ57267543 = -474551560;    int mweytDumPQ9356777 = -805580884;    int mweytDumPQ97348947 = -758605550;    int mweytDumPQ64827897 = -628661655;    int mweytDumPQ76821376 = -611907421;    int mweytDumPQ33260190 = -805394727;    int mweytDumPQ2062074 = 9495907;    int mweytDumPQ79172336 = -908016234;    int mweytDumPQ43468586 = -992275115;    int mweytDumPQ60865734 = -973707870;    int mweytDumPQ23447353 = -637246836;    int mweytDumPQ26443325 = -17536299;    int mweytDumPQ44780664 = -190843979;    int mweytDumPQ88913543 = -294125811;    int mweytDumPQ55951614 = -742646999;    int mweytDumPQ44494371 = -685028461;    int mweytDumPQ16060004 = -331543347;    int mweytDumPQ65847642 = -315680952;    int mweytDumPQ88166276 = -500562669;    int mweytDumPQ41948604 = -671471209;    int mweytDumPQ24770625 = -142011934;    int mweytDumPQ47718198 = -322344323;    int mweytDumPQ93087892 = -30183979;    int mweytDumPQ39748646 = -163846019;    int mweytDumPQ24875611 = -532321879;    int mweytDumPQ5878262 = -393559151;    int mweytDumPQ27130231 = -822872371;    int mweytDumPQ8075390 = -891013536;    int mweytDumPQ36627593 = -406340792;    int mweytDumPQ5930848 = -764295629;    int mweytDumPQ12589015 = -605027035;     mweytDumPQ71288188 = mweytDumPQ93870938;     mweytDumPQ93870938 = mweytDumPQ60480565;     mweytDumPQ60480565 = mweytDumPQ53040426;     mweytDumPQ53040426 = mweytDumPQ54443678;     mweytDumPQ54443678 = mweytDumPQ28420020;     mweytDumPQ28420020 = mweytDumPQ45953123;     mweytDumPQ45953123 = mweytDumPQ10969651;     mweytDumPQ10969651 = mweytDumPQ66113204;     mweytDumPQ66113204 = mweytDumPQ30699296;     mweytDumPQ30699296 = mweytDumPQ66136659;     mweytDumPQ66136659 = mweytDumPQ6121181;     mweytDumPQ6121181 = mweytDumPQ28610149;     mweytDumPQ28610149 = mweytDumPQ55874403;     mweytDumPQ55874403 = mweytDumPQ19600741;     mweytDumPQ19600741 = mweytDumPQ29444294;     mweytDumPQ29444294 = mweytDumPQ16252928;     mweytDumPQ16252928 = mweytDumPQ65240538;     mweytDumPQ65240538 = mweytDumPQ85811176;     mweytDumPQ85811176 = mweytDumPQ98938012;     mweytDumPQ98938012 = mweytDumPQ40946571;     mweytDumPQ40946571 = mweytDumPQ16587972;     mweytDumPQ16587972 = mweytDumPQ79255939;     mweytDumPQ79255939 = mweytDumPQ33480941;     mweytDumPQ33480941 = mweytDumPQ73885091;     mweytDumPQ73885091 = mweytDumPQ78906648;     mweytDumPQ78906648 = mweytDumPQ25308481;     mweytDumPQ25308481 = mweytDumPQ7405532;     mweytDumPQ7405532 = mweytDumPQ80160543;     mweytDumPQ80160543 = mweytDumPQ65325161;     mweytDumPQ65325161 = mweytDumPQ842790;     mweytDumPQ842790 = mweytDumPQ6110467;     mweytDumPQ6110467 = mweytDumPQ91590074;     mweytDumPQ91590074 = mweytDumPQ1264487;     mweytDumPQ1264487 = mweytDumPQ79470841;     mweytDumPQ79470841 = mweytDumPQ62333197;     mweytDumPQ62333197 = mweytDumPQ20087517;     mweytDumPQ20087517 = mweytDumPQ18903981;     mweytDumPQ18903981 = mweytDumPQ412641;     mweytDumPQ412641 = mweytDumPQ8989801;     mweytDumPQ8989801 = mweytDumPQ65677822;     mweytDumPQ65677822 = mweytDumPQ38884497;     mweytDumPQ38884497 = mweytDumPQ37415635;     mweytDumPQ37415635 = mweytDumPQ35787353;     mweytDumPQ35787353 = mweytDumPQ72615207;     mweytDumPQ72615207 = mweytDumPQ50437739;     mweytDumPQ50437739 = mweytDumPQ52463323;     mweytDumPQ52463323 = mweytDumPQ80527816;     mweytDumPQ80527816 = mweytDumPQ18491989;     mweytDumPQ18491989 = mweytDumPQ24208930;     mweytDumPQ24208930 = mweytDumPQ20830791;     mweytDumPQ20830791 = mweytDumPQ84782786;     mweytDumPQ84782786 = mweytDumPQ40262824;     mweytDumPQ40262824 = mweytDumPQ3423799;     mweytDumPQ3423799 = mweytDumPQ59315882;     mweytDumPQ59315882 = mweytDumPQ54700216;     mweytDumPQ54700216 = mweytDumPQ14615000;     mweytDumPQ14615000 = mweytDumPQ26999625;     mweytDumPQ26999625 = mweytDumPQ79155335;     mweytDumPQ79155335 = mweytDumPQ75537030;     mweytDumPQ75537030 = mweytDumPQ3111539;     mweytDumPQ3111539 = mweytDumPQ38547592;     mweytDumPQ38547592 = mweytDumPQ30809107;     mweytDumPQ30809107 = mweytDumPQ788043;     mweytDumPQ788043 = mweytDumPQ29856506;     mweytDumPQ29856506 = mweytDumPQ60026192;     mweytDumPQ60026192 = mweytDumPQ14531106;     mweytDumPQ14531106 = mweytDumPQ27345662;     mweytDumPQ27345662 = mweytDumPQ76403562;     mweytDumPQ76403562 = mweytDumPQ57267543;     mweytDumPQ57267543 = mweytDumPQ9356777;     mweytDumPQ9356777 = mweytDumPQ97348947;     mweytDumPQ97348947 = mweytDumPQ64827897;     mweytDumPQ64827897 = mweytDumPQ76821376;     mweytDumPQ76821376 = mweytDumPQ33260190;     mweytDumPQ33260190 = mweytDumPQ2062074;     mweytDumPQ2062074 = mweytDumPQ79172336;     mweytDumPQ79172336 = mweytDumPQ43468586;     mweytDumPQ43468586 = mweytDumPQ60865734;     mweytDumPQ60865734 = mweytDumPQ23447353;     mweytDumPQ23447353 = mweytDumPQ26443325;     mweytDumPQ26443325 = mweytDumPQ44780664;     mweytDumPQ44780664 = mweytDumPQ88913543;     mweytDumPQ88913543 = mweytDumPQ55951614;     mweytDumPQ55951614 = mweytDumPQ44494371;     mweytDumPQ44494371 = mweytDumPQ16060004;     mweytDumPQ16060004 = mweytDumPQ65847642;     mweytDumPQ65847642 = mweytDumPQ88166276;     mweytDumPQ88166276 = mweytDumPQ41948604;     mweytDumPQ41948604 = mweytDumPQ24770625;     mweytDumPQ24770625 = mweytDumPQ47718198;     mweytDumPQ47718198 = mweytDumPQ93087892;     mweytDumPQ93087892 = mweytDumPQ39748646;     mweytDumPQ39748646 = mweytDumPQ24875611;     mweytDumPQ24875611 = mweytDumPQ5878262;     mweytDumPQ5878262 = mweytDumPQ27130231;     mweytDumPQ27130231 = mweytDumPQ8075390;     mweytDumPQ8075390 = mweytDumPQ36627593;     mweytDumPQ36627593 = mweytDumPQ5930848;     mweytDumPQ5930848 = mweytDumPQ12589015;     mweytDumPQ12589015 = mweytDumPQ71288188;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sHXETIwAbw70718326() {     int NOwMGXRVBf84074355 = -740067727;    int NOwMGXRVBf80641826 = -894039747;    int NOwMGXRVBf78233365 = -262992583;    int NOwMGXRVBf60981252 = -150733382;    int NOwMGXRVBf47954782 = -228442786;    int NOwMGXRVBf71730090 = -518623039;    int NOwMGXRVBf67160881 = -226168290;    int NOwMGXRVBf37688763 = -746662327;    int NOwMGXRVBf61518015 = -189958402;    int NOwMGXRVBf36918001 = -174295735;    int NOwMGXRVBf93624041 = -283879331;    int NOwMGXRVBf38071685 = -724722268;    int NOwMGXRVBf98753451 = -176309863;    int NOwMGXRVBf5498482 = -719627393;    int NOwMGXRVBf42907294 = -366955081;    int NOwMGXRVBf57869362 = -909251767;    int NOwMGXRVBf72004745 = -117799936;    int NOwMGXRVBf48799023 = -831777881;    int NOwMGXRVBf86971807 = -290238808;    int NOwMGXRVBf1592510 = -347482611;    int NOwMGXRVBf21120628 = -455352219;    int NOwMGXRVBf45635791 = -879845231;    int NOwMGXRVBf18016235 = 8363740;    int NOwMGXRVBf77988070 = -792382698;    int NOwMGXRVBf79779350 = -5399129;    int NOwMGXRVBf60225293 = -511473475;    int NOwMGXRVBf36724017 = 99897036;    int NOwMGXRVBf74758456 = -734322950;    int NOwMGXRVBf96777596 = -433842983;    int NOwMGXRVBf18076277 = -562456606;    int NOwMGXRVBf45278786 = -242563230;    int NOwMGXRVBf9429291 = -510727500;    int NOwMGXRVBf37636024 = -462939661;    int NOwMGXRVBf78340859 = -373710309;    int NOwMGXRVBf62531373 = -647083966;    int NOwMGXRVBf57557312 = -773980795;    int NOwMGXRVBf56851577 = -342688845;    int NOwMGXRVBf76608003 = -615040011;    int NOwMGXRVBf6188027 = -389756501;    int NOwMGXRVBf35254530 = -613279039;    int NOwMGXRVBf27109131 = -318317739;    int NOwMGXRVBf46586953 = -871843197;    int NOwMGXRVBf72299105 = -825404095;    int NOwMGXRVBf80415891 = -409529494;    int NOwMGXRVBf52623697 = -351099777;    int NOwMGXRVBf24065868 = -231028808;    int NOwMGXRVBf30045343 = -101021029;    int NOwMGXRVBf9967783 = -230905484;    int NOwMGXRVBf62556038 = -330001009;    int NOwMGXRVBf71159350 = 11984793;    int NOwMGXRVBf39158284 = -400426298;    int NOwMGXRVBf12959334 = -5788471;    int NOwMGXRVBf74747974 = -292330153;    int NOwMGXRVBf48123524 = -675973951;    int NOwMGXRVBf49597699 = -791929368;    int NOwMGXRVBf38438565 = -860222496;    int NOwMGXRVBf62625592 = -802403487;    int NOwMGXRVBf245295 = -470609885;    int NOwMGXRVBf81201901 = -45334254;    int NOwMGXRVBf87729489 = -716969312;    int NOwMGXRVBf35006073 = -518520076;    int NOwMGXRVBf92402424 = -491845341;    int NOwMGXRVBf40911167 = -212819344;    int NOwMGXRVBf43441738 = -627501796;    int NOwMGXRVBf91639214 = -931732506;    int NOwMGXRVBf84194750 = -773151832;    int NOwMGXRVBf435662 = -161782607;    int NOwMGXRVBf20412592 = -802599555;    int NOwMGXRVBf42967109 = 27456572;    int NOwMGXRVBf85349981 = -592974286;    int NOwMGXRVBf1017785 = -466562923;    int NOwMGXRVBf95396741 = -502759925;    int NOwMGXRVBf42610997 = -342021380;    int NOwMGXRVBf51717277 = -676959770;    int NOwMGXRVBf74483378 = 70835127;    int NOwMGXRVBf74533674 = -583509023;    int NOwMGXRVBf73336685 = 45558863;    int NOwMGXRVBf37600344 = -582106767;    int NOwMGXRVBf25364373 = -341282922;    int NOwMGXRVBf55713483 = -774370321;    int NOwMGXRVBf30179951 = -310452446;    int NOwMGXRVBf26756234 = -669197480;    int NOwMGXRVBf12202419 = -304321941;    int NOwMGXRVBf25618247 = -345827776;    int NOwMGXRVBf78917993 = -62030308;    int NOwMGXRVBf32319452 = -136774759;    int NOwMGXRVBf34681316 = -118397348;    int NOwMGXRVBf89512500 = -786965711;    int NOwMGXRVBf28743161 = -581780941;    int NOwMGXRVBf24092809 = -786861470;    int NOwMGXRVBf94931720 = -971577308;    int NOwMGXRVBf56606282 = -872078960;    int NOwMGXRVBf95406102 = -469705758;    int NOwMGXRVBf18458538 = -672787190;    int NOwMGXRVBf248457 = 5241037;    int NOwMGXRVBf34706706 = -826472399;    int NOwMGXRVBf5675787 = -559023853;    int NOwMGXRVBf28857367 = -97902299;    int NOwMGXRVBf88776676 = -477796989;    int NOwMGXRVBf68428947 = -740067727;     NOwMGXRVBf84074355 = NOwMGXRVBf80641826;     NOwMGXRVBf80641826 = NOwMGXRVBf78233365;     NOwMGXRVBf78233365 = NOwMGXRVBf60981252;     NOwMGXRVBf60981252 = NOwMGXRVBf47954782;     NOwMGXRVBf47954782 = NOwMGXRVBf71730090;     NOwMGXRVBf71730090 = NOwMGXRVBf67160881;     NOwMGXRVBf67160881 = NOwMGXRVBf37688763;     NOwMGXRVBf37688763 = NOwMGXRVBf61518015;     NOwMGXRVBf61518015 = NOwMGXRVBf36918001;     NOwMGXRVBf36918001 = NOwMGXRVBf93624041;     NOwMGXRVBf93624041 = NOwMGXRVBf38071685;     NOwMGXRVBf38071685 = NOwMGXRVBf98753451;     NOwMGXRVBf98753451 = NOwMGXRVBf5498482;     NOwMGXRVBf5498482 = NOwMGXRVBf42907294;     NOwMGXRVBf42907294 = NOwMGXRVBf57869362;     NOwMGXRVBf57869362 = NOwMGXRVBf72004745;     NOwMGXRVBf72004745 = NOwMGXRVBf48799023;     NOwMGXRVBf48799023 = NOwMGXRVBf86971807;     NOwMGXRVBf86971807 = NOwMGXRVBf1592510;     NOwMGXRVBf1592510 = NOwMGXRVBf21120628;     NOwMGXRVBf21120628 = NOwMGXRVBf45635791;     NOwMGXRVBf45635791 = NOwMGXRVBf18016235;     NOwMGXRVBf18016235 = NOwMGXRVBf77988070;     NOwMGXRVBf77988070 = NOwMGXRVBf79779350;     NOwMGXRVBf79779350 = NOwMGXRVBf60225293;     NOwMGXRVBf60225293 = NOwMGXRVBf36724017;     NOwMGXRVBf36724017 = NOwMGXRVBf74758456;     NOwMGXRVBf74758456 = NOwMGXRVBf96777596;     NOwMGXRVBf96777596 = NOwMGXRVBf18076277;     NOwMGXRVBf18076277 = NOwMGXRVBf45278786;     NOwMGXRVBf45278786 = NOwMGXRVBf9429291;     NOwMGXRVBf9429291 = NOwMGXRVBf37636024;     NOwMGXRVBf37636024 = NOwMGXRVBf78340859;     NOwMGXRVBf78340859 = NOwMGXRVBf62531373;     NOwMGXRVBf62531373 = NOwMGXRVBf57557312;     NOwMGXRVBf57557312 = NOwMGXRVBf56851577;     NOwMGXRVBf56851577 = NOwMGXRVBf76608003;     NOwMGXRVBf76608003 = NOwMGXRVBf6188027;     NOwMGXRVBf6188027 = NOwMGXRVBf35254530;     NOwMGXRVBf35254530 = NOwMGXRVBf27109131;     NOwMGXRVBf27109131 = NOwMGXRVBf46586953;     NOwMGXRVBf46586953 = NOwMGXRVBf72299105;     NOwMGXRVBf72299105 = NOwMGXRVBf80415891;     NOwMGXRVBf80415891 = NOwMGXRVBf52623697;     NOwMGXRVBf52623697 = NOwMGXRVBf24065868;     NOwMGXRVBf24065868 = NOwMGXRVBf30045343;     NOwMGXRVBf30045343 = NOwMGXRVBf9967783;     NOwMGXRVBf9967783 = NOwMGXRVBf62556038;     NOwMGXRVBf62556038 = NOwMGXRVBf71159350;     NOwMGXRVBf71159350 = NOwMGXRVBf39158284;     NOwMGXRVBf39158284 = NOwMGXRVBf12959334;     NOwMGXRVBf12959334 = NOwMGXRVBf74747974;     NOwMGXRVBf74747974 = NOwMGXRVBf48123524;     NOwMGXRVBf48123524 = NOwMGXRVBf49597699;     NOwMGXRVBf49597699 = NOwMGXRVBf38438565;     NOwMGXRVBf38438565 = NOwMGXRVBf62625592;     NOwMGXRVBf62625592 = NOwMGXRVBf245295;     NOwMGXRVBf245295 = NOwMGXRVBf81201901;     NOwMGXRVBf81201901 = NOwMGXRVBf87729489;     NOwMGXRVBf87729489 = NOwMGXRVBf35006073;     NOwMGXRVBf35006073 = NOwMGXRVBf92402424;     NOwMGXRVBf92402424 = NOwMGXRVBf40911167;     NOwMGXRVBf40911167 = NOwMGXRVBf43441738;     NOwMGXRVBf43441738 = NOwMGXRVBf91639214;     NOwMGXRVBf91639214 = NOwMGXRVBf84194750;     NOwMGXRVBf84194750 = NOwMGXRVBf435662;     NOwMGXRVBf435662 = NOwMGXRVBf20412592;     NOwMGXRVBf20412592 = NOwMGXRVBf42967109;     NOwMGXRVBf42967109 = NOwMGXRVBf85349981;     NOwMGXRVBf85349981 = NOwMGXRVBf1017785;     NOwMGXRVBf1017785 = NOwMGXRVBf95396741;     NOwMGXRVBf95396741 = NOwMGXRVBf42610997;     NOwMGXRVBf42610997 = NOwMGXRVBf51717277;     NOwMGXRVBf51717277 = NOwMGXRVBf74483378;     NOwMGXRVBf74483378 = NOwMGXRVBf74533674;     NOwMGXRVBf74533674 = NOwMGXRVBf73336685;     NOwMGXRVBf73336685 = NOwMGXRVBf37600344;     NOwMGXRVBf37600344 = NOwMGXRVBf25364373;     NOwMGXRVBf25364373 = NOwMGXRVBf55713483;     NOwMGXRVBf55713483 = NOwMGXRVBf30179951;     NOwMGXRVBf30179951 = NOwMGXRVBf26756234;     NOwMGXRVBf26756234 = NOwMGXRVBf12202419;     NOwMGXRVBf12202419 = NOwMGXRVBf25618247;     NOwMGXRVBf25618247 = NOwMGXRVBf78917993;     NOwMGXRVBf78917993 = NOwMGXRVBf32319452;     NOwMGXRVBf32319452 = NOwMGXRVBf34681316;     NOwMGXRVBf34681316 = NOwMGXRVBf89512500;     NOwMGXRVBf89512500 = NOwMGXRVBf28743161;     NOwMGXRVBf28743161 = NOwMGXRVBf24092809;     NOwMGXRVBf24092809 = NOwMGXRVBf94931720;     NOwMGXRVBf94931720 = NOwMGXRVBf56606282;     NOwMGXRVBf56606282 = NOwMGXRVBf95406102;     NOwMGXRVBf95406102 = NOwMGXRVBf18458538;     NOwMGXRVBf18458538 = NOwMGXRVBf248457;     NOwMGXRVBf248457 = NOwMGXRVBf34706706;     NOwMGXRVBf34706706 = NOwMGXRVBf5675787;     NOwMGXRVBf5675787 = NOwMGXRVBf28857367;     NOwMGXRVBf28857367 = NOwMGXRVBf88776676;     NOwMGXRVBf88776676 = NOwMGXRVBf68428947;     NOwMGXRVBf68428947 = NOwMGXRVBf84074355;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IJDAxycnWL88931667() {     int ZvBSArKqaP67518660 = -520733407;    int ZvBSArKqaP97513915 = -893421935;    int ZvBSArKqaP74641872 = -56517811;    int ZvBSArKqaP23773504 = -344533218;    int ZvBSArKqaP54076106 = -35310277;    int ZvBSArKqaP70234491 = -297694692;    int ZvBSArKqaP38200199 = 33920397;    int ZvBSArKqaP52036206 = -733820356;    int ZvBSArKqaP20047716 = -531417753;    int ZvBSArKqaP69463652 = -642671847;    int ZvBSArKqaP43039979 = -473995204;    int ZvBSArKqaP1277085 = -712340514;    int ZvBSArKqaP16871228 = 32028896;    int ZvBSArKqaP85125373 = -507917750;    int ZvBSArKqaP3761758 = -460978063;    int ZvBSArKqaP44258524 = -697331092;    int ZvBSArKqaP65756630 = -971144963;    int ZvBSArKqaP29650470 = 20917243;    int ZvBSArKqaP49411949 = -288913679;    int ZvBSArKqaP90959951 = -877068604;    int ZvBSArKqaP76761014 = -44881250;    int ZvBSArKqaP57783109 = -870997001;    int ZvBSArKqaP21841942 = -31525457;    int ZvBSArKqaP12694629 = -864208094;    int ZvBSArKqaP21355309 = -565051888;    int ZvBSArKqaP55567442 = -391983190;    int ZvBSArKqaP86178757 = -628187306;    int ZvBSArKqaP14490082 = -804277834;    int ZvBSArKqaP32858691 = -296348125;    int ZvBSArKqaP33909618 = 24693543;    int ZvBSArKqaP63374099 = -980320431;    int ZvBSArKqaP72535717 = -425922580;    int ZvBSArKqaP81174911 = -83811839;    int ZvBSArKqaP20499371 = 34687521;    int ZvBSArKqaP41258103 = -844748701;    int ZvBSArKqaP69946496 = -562004701;    int ZvBSArKqaP25465756 = -221538025;    int ZvBSArKqaP17186836 = -323510531;    int ZvBSArKqaP55183104 = -574654299;    int ZvBSArKqaP84749665 = -365872561;    int ZvBSArKqaP60758055 = -989866712;    int ZvBSArKqaP97074739 = -345751518;    int ZvBSArKqaP95866174 = -276461338;    int ZvBSArKqaP72320831 = -165712265;    int ZvBSArKqaP44568194 = -79256730;    int ZvBSArKqaP63671864 = -250639732;    int ZvBSArKqaP6060773 = -233325259;    int ZvBSArKqaP29638379 = -229627203;    int ZvBSArKqaP93403150 = -915603092;    int ZvBSArKqaP6491834 = -195035662;    int ZvBSArKqaP12415732 = -621327721;    int ZvBSArKqaP68460128 = -64047783;    int ZvBSArKqaP88860433 = -504077469;    int ZvBSArKqaP72800282 = -394387338;    int ZvBSArKqaP12170653 = -40415491;    int ZvBSArKqaP9735552 = -649736406;    int ZvBSArKqaP75671973 = -761896478;    int ZvBSArKqaP61947243 = -192309717;    int ZvBSArKqaP2418195 = -779481331;    int ZvBSArKqaP98508663 = -643327087;    int ZvBSArKqaP84055734 = -669507386;    int ZvBSArKqaP23710117 = -161801770;    int ZvBSArKqaP19177516 = -337472232;    int ZvBSArKqaP86138097 = -456111296;    int ZvBSArKqaP6089553 = -662351417;    int ZvBSArKqaP70504261 = 51927376;    int ZvBSArKqaP20102174 = -528528676;    int ZvBSArKqaP96371857 = 97341374;    int ZvBSArKqaP43867270 = -663169049;    int ZvBSArKqaP33815261 = -898973363;    int ZvBSArKqaP18792768 = -375793068;    int ZvBSArKqaP48569795 = -547634432;    int ZvBSArKqaP74467365 = -404428459;    int ZvBSArKqaP64662284 = -923041119;    int ZvBSArKqaP30201897 = -887201892;    int ZvBSArKqaP79686275 = -699129732;    int ZvBSArKqaP61916934 = -494535664;    int ZvBSArKqaP49521111 = -865813193;    int ZvBSArKqaP68126434 = -684951365;    int ZvBSArKqaP57683445 = -214412156;    int ZvBSArKqaP49506670 = -58657932;    int ZvBSArKqaP56540379 = -298560104;    int ZvBSArKqaP21086932 = -888674742;    int ZvBSArKqaP26366857 = -1312463;    int ZvBSArKqaP21493886 = -353978737;    int ZvBSArKqaP94913971 = -816272648;    int ZvBSArKqaP83675284 = -921845111;    int ZvBSArKqaP8374629 = -689424501;    int ZvBSArKqaP8328718 = -924896988;    int ZvBSArKqaP31522552 = -95012295;    int ZvBSArKqaP94274523 = -800108223;    int ZvBSArKqaP63518512 = 70771692;    int ZvBSArKqaP14768641 = -544029201;    int ZvBSArKqaP56674440 = -931327212;    int ZvBSArKqaP693932 = -696365175;    int ZvBSArKqaP37047938 = -728064942;    int ZvBSArKqaP77897223 = 91720713;    int ZvBSArKqaP9728078 = -820350042;    int ZvBSArKqaP66231278 = -503360848;    int ZvBSArKqaP74063932 = -520733407;     ZvBSArKqaP67518660 = ZvBSArKqaP97513915;     ZvBSArKqaP97513915 = ZvBSArKqaP74641872;     ZvBSArKqaP74641872 = ZvBSArKqaP23773504;     ZvBSArKqaP23773504 = ZvBSArKqaP54076106;     ZvBSArKqaP54076106 = ZvBSArKqaP70234491;     ZvBSArKqaP70234491 = ZvBSArKqaP38200199;     ZvBSArKqaP38200199 = ZvBSArKqaP52036206;     ZvBSArKqaP52036206 = ZvBSArKqaP20047716;     ZvBSArKqaP20047716 = ZvBSArKqaP69463652;     ZvBSArKqaP69463652 = ZvBSArKqaP43039979;     ZvBSArKqaP43039979 = ZvBSArKqaP1277085;     ZvBSArKqaP1277085 = ZvBSArKqaP16871228;     ZvBSArKqaP16871228 = ZvBSArKqaP85125373;     ZvBSArKqaP85125373 = ZvBSArKqaP3761758;     ZvBSArKqaP3761758 = ZvBSArKqaP44258524;     ZvBSArKqaP44258524 = ZvBSArKqaP65756630;     ZvBSArKqaP65756630 = ZvBSArKqaP29650470;     ZvBSArKqaP29650470 = ZvBSArKqaP49411949;     ZvBSArKqaP49411949 = ZvBSArKqaP90959951;     ZvBSArKqaP90959951 = ZvBSArKqaP76761014;     ZvBSArKqaP76761014 = ZvBSArKqaP57783109;     ZvBSArKqaP57783109 = ZvBSArKqaP21841942;     ZvBSArKqaP21841942 = ZvBSArKqaP12694629;     ZvBSArKqaP12694629 = ZvBSArKqaP21355309;     ZvBSArKqaP21355309 = ZvBSArKqaP55567442;     ZvBSArKqaP55567442 = ZvBSArKqaP86178757;     ZvBSArKqaP86178757 = ZvBSArKqaP14490082;     ZvBSArKqaP14490082 = ZvBSArKqaP32858691;     ZvBSArKqaP32858691 = ZvBSArKqaP33909618;     ZvBSArKqaP33909618 = ZvBSArKqaP63374099;     ZvBSArKqaP63374099 = ZvBSArKqaP72535717;     ZvBSArKqaP72535717 = ZvBSArKqaP81174911;     ZvBSArKqaP81174911 = ZvBSArKqaP20499371;     ZvBSArKqaP20499371 = ZvBSArKqaP41258103;     ZvBSArKqaP41258103 = ZvBSArKqaP69946496;     ZvBSArKqaP69946496 = ZvBSArKqaP25465756;     ZvBSArKqaP25465756 = ZvBSArKqaP17186836;     ZvBSArKqaP17186836 = ZvBSArKqaP55183104;     ZvBSArKqaP55183104 = ZvBSArKqaP84749665;     ZvBSArKqaP84749665 = ZvBSArKqaP60758055;     ZvBSArKqaP60758055 = ZvBSArKqaP97074739;     ZvBSArKqaP97074739 = ZvBSArKqaP95866174;     ZvBSArKqaP95866174 = ZvBSArKqaP72320831;     ZvBSArKqaP72320831 = ZvBSArKqaP44568194;     ZvBSArKqaP44568194 = ZvBSArKqaP63671864;     ZvBSArKqaP63671864 = ZvBSArKqaP6060773;     ZvBSArKqaP6060773 = ZvBSArKqaP29638379;     ZvBSArKqaP29638379 = ZvBSArKqaP93403150;     ZvBSArKqaP93403150 = ZvBSArKqaP6491834;     ZvBSArKqaP6491834 = ZvBSArKqaP12415732;     ZvBSArKqaP12415732 = ZvBSArKqaP68460128;     ZvBSArKqaP68460128 = ZvBSArKqaP88860433;     ZvBSArKqaP88860433 = ZvBSArKqaP72800282;     ZvBSArKqaP72800282 = ZvBSArKqaP12170653;     ZvBSArKqaP12170653 = ZvBSArKqaP9735552;     ZvBSArKqaP9735552 = ZvBSArKqaP75671973;     ZvBSArKqaP75671973 = ZvBSArKqaP61947243;     ZvBSArKqaP61947243 = ZvBSArKqaP2418195;     ZvBSArKqaP2418195 = ZvBSArKqaP98508663;     ZvBSArKqaP98508663 = ZvBSArKqaP84055734;     ZvBSArKqaP84055734 = ZvBSArKqaP23710117;     ZvBSArKqaP23710117 = ZvBSArKqaP19177516;     ZvBSArKqaP19177516 = ZvBSArKqaP86138097;     ZvBSArKqaP86138097 = ZvBSArKqaP6089553;     ZvBSArKqaP6089553 = ZvBSArKqaP70504261;     ZvBSArKqaP70504261 = ZvBSArKqaP20102174;     ZvBSArKqaP20102174 = ZvBSArKqaP96371857;     ZvBSArKqaP96371857 = ZvBSArKqaP43867270;     ZvBSArKqaP43867270 = ZvBSArKqaP33815261;     ZvBSArKqaP33815261 = ZvBSArKqaP18792768;     ZvBSArKqaP18792768 = ZvBSArKqaP48569795;     ZvBSArKqaP48569795 = ZvBSArKqaP74467365;     ZvBSArKqaP74467365 = ZvBSArKqaP64662284;     ZvBSArKqaP64662284 = ZvBSArKqaP30201897;     ZvBSArKqaP30201897 = ZvBSArKqaP79686275;     ZvBSArKqaP79686275 = ZvBSArKqaP61916934;     ZvBSArKqaP61916934 = ZvBSArKqaP49521111;     ZvBSArKqaP49521111 = ZvBSArKqaP68126434;     ZvBSArKqaP68126434 = ZvBSArKqaP57683445;     ZvBSArKqaP57683445 = ZvBSArKqaP49506670;     ZvBSArKqaP49506670 = ZvBSArKqaP56540379;     ZvBSArKqaP56540379 = ZvBSArKqaP21086932;     ZvBSArKqaP21086932 = ZvBSArKqaP26366857;     ZvBSArKqaP26366857 = ZvBSArKqaP21493886;     ZvBSArKqaP21493886 = ZvBSArKqaP94913971;     ZvBSArKqaP94913971 = ZvBSArKqaP83675284;     ZvBSArKqaP83675284 = ZvBSArKqaP8374629;     ZvBSArKqaP8374629 = ZvBSArKqaP8328718;     ZvBSArKqaP8328718 = ZvBSArKqaP31522552;     ZvBSArKqaP31522552 = ZvBSArKqaP94274523;     ZvBSArKqaP94274523 = ZvBSArKqaP63518512;     ZvBSArKqaP63518512 = ZvBSArKqaP14768641;     ZvBSArKqaP14768641 = ZvBSArKqaP56674440;     ZvBSArKqaP56674440 = ZvBSArKqaP693932;     ZvBSArKqaP693932 = ZvBSArKqaP37047938;     ZvBSArKqaP37047938 = ZvBSArKqaP77897223;     ZvBSArKqaP77897223 = ZvBSArKqaP9728078;     ZvBSArKqaP9728078 = ZvBSArKqaP66231278;     ZvBSArKqaP66231278 = ZvBSArKqaP74063932;     ZvBSArKqaP74063932 = ZvBSArKqaP67518660;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void izQUGPZvZn59387541() {     int trGbPcsXMv80304827 = -655774099;    int trGbPcsXMv84284803 = 36244052;    int trGbPcsXMv92394672 = -464877193;    int trGbPcsXMv31714329 = -168735495;    int trGbPcsXMv47587211 = -527126742;    int trGbPcsXMv13544562 = -74618006;    int trGbPcsXMv59407957 = -511751674;    int trGbPcsXMv78755318 = -78338613;    int trGbPcsXMv15452526 = -493175845;    int trGbPcsXMv75682357 = -446759480;    int trGbPcsXMv70527361 = -588681219;    int trGbPcsXMv33227590 = -119673330;    int trGbPcsXMv87014530 = -971753762;    int trGbPcsXMv34749452 = -840332948;    int trGbPcsXMv27068310 = -367963894;    int trGbPcsXMv72683592 = -148222482;    int trGbPcsXMv21508448 = -634919430;    int trGbPcsXMv13208955 = -634293059;    int trGbPcsXMv50572580 = -198382874;    int trGbPcsXMv93614447 = -376312743;    int trGbPcsXMv56935071 = -536197515;    int trGbPcsXMv86830928 = 59144651;    int trGbPcsXMv60602238 = 37470648;    int trGbPcsXMv57201758 = -124553107;    int trGbPcsXMv27249568 = -775493812;    int trGbPcsXMv36886088 = -82414390;    int trGbPcsXMv97594293 = -161893587;    int trGbPcsXMv81843007 = -978075938;    int trGbPcsXMv49475744 = -510565365;    int trGbPcsXMv86660733 = -705192612;    int trGbPcsXMv7810096 = -156737181;    int trGbPcsXMv75854541 = -404642258;    int trGbPcsXMv27220860 = 40333377;    int trGbPcsXMv97575743 = -970066697;    int trGbPcsXMv24318636 = -354780581;    int trGbPcsXMv65170611 = -350567805;    int trGbPcsXMv62229816 = -11447375;    int trGbPcsXMv74890858 = -243130623;    int trGbPcsXMv60958490 = -416504876;    int trGbPcsXMv11014396 = -210289406;    int trGbPcsXMv22189364 = -265340705;    int trGbPcsXMv4777195 = -244062853;    int trGbPcsXMv30749645 = -299894783;    int trGbPcsXMv16949369 = -506884508;    int trGbPcsXMv24576685 = 27973309;    int trGbPcsXMv37299993 = -323958171;    int trGbPcsXMv83642792 = -730840311;    int trGbPcsXMv59078345 = -384979982;    int trGbPcsXMv37467199 = 20794933;    int trGbPcsXMv53442254 = -806072125;    int trGbPcsXMv30743226 = -874212029;    int trGbPcsXMv96636675 = -535233121;    int trGbPcsXMv23345584 = -680080752;    int trGbPcsXMv17500008 = 16160920;    int trGbPcsXMv2452470 = -34859977;    int trGbPcsXMv93473899 = -614918750;    int trGbPcsXMv23682566 = 98773403;    int trGbPcsXMv35192914 = -240324086;    int trGbPcsXMv4464762 = -393241684;    int trGbPcsXMv10701124 = -344712353;    int trGbPcsXMv15950269 = -912724420;    int trGbPcsXMv77564950 = -533675736;    int trGbPcsXMv29279575 = -567773248;    int trGbPcsXMv28791792 = -787983234;    int trGbPcsXMv67872262 = -190022300;    int trGbPcsXMv94672820 = -84038961;    int trGbPcsXMv6006730 = -60006708;    int trGbPcsXMv89438787 = 98312935;    int trGbPcsXMv10430817 = -385552367;    int trGbPcsXMv61897699 = 82603911;    int trGbPcsXMv10453777 = -36775107;    int trGbPcsXMv46617590 = -291788807;    int trGbPcsXMv52250464 = -117788184;    int trGbPcsXMv39558185 = -988093468;    int trGbPcsXMv71425084 = -10972038;    int trGbPcsXMv52157876 = -192134662;    int trGbPcsXMv56081283 = -640960567;    int trGbPcsXMv43652869 = -455644845;    int trGbPcsXMv32625074 = -52526417;    int trGbPcsXMv89949574 = -351535641;    int trGbPcsXMv53243295 = -351574079;    int trGbPcsXMv38515949 = -776913605;    int trGbPcsXMv44375808 = -898870872;    int trGbPcsXMv96033489 = -704493241;    int trGbPcsXMv55917508 = -830980583;    int trGbPcsXMv11173421 = -621504060;    int trGbPcsXMv52508958 = -724561507;    int trGbPcsXMv9720853 = -975827544;    int trGbPcsXMv95123273 = -835206720;    int trGbPcsXMv30844736 = -739861832;    int trGbPcsXMv41488045 = -349341209;    int trGbPcsXMv27036903 = -771123289;    int trGbPcsXMv70426097 = -849888940;    int trGbPcsXMv50257367 = 28207477;    int trGbPcsXMv95064126 = -297564987;    int trGbPcsXMv44624413 = -731664970;    int trGbPcsXMv75497620 = -676289605;    int trGbPcsXMv1957853 = -511911550;    int trGbPcsXMv49077107 = -216862208;    int trGbPcsXMv29903865 = -655774099;     trGbPcsXMv80304827 = trGbPcsXMv84284803;     trGbPcsXMv84284803 = trGbPcsXMv92394672;     trGbPcsXMv92394672 = trGbPcsXMv31714329;     trGbPcsXMv31714329 = trGbPcsXMv47587211;     trGbPcsXMv47587211 = trGbPcsXMv13544562;     trGbPcsXMv13544562 = trGbPcsXMv59407957;     trGbPcsXMv59407957 = trGbPcsXMv78755318;     trGbPcsXMv78755318 = trGbPcsXMv15452526;     trGbPcsXMv15452526 = trGbPcsXMv75682357;     trGbPcsXMv75682357 = trGbPcsXMv70527361;     trGbPcsXMv70527361 = trGbPcsXMv33227590;     trGbPcsXMv33227590 = trGbPcsXMv87014530;     trGbPcsXMv87014530 = trGbPcsXMv34749452;     trGbPcsXMv34749452 = trGbPcsXMv27068310;     trGbPcsXMv27068310 = trGbPcsXMv72683592;     trGbPcsXMv72683592 = trGbPcsXMv21508448;     trGbPcsXMv21508448 = trGbPcsXMv13208955;     trGbPcsXMv13208955 = trGbPcsXMv50572580;     trGbPcsXMv50572580 = trGbPcsXMv93614447;     trGbPcsXMv93614447 = trGbPcsXMv56935071;     trGbPcsXMv56935071 = trGbPcsXMv86830928;     trGbPcsXMv86830928 = trGbPcsXMv60602238;     trGbPcsXMv60602238 = trGbPcsXMv57201758;     trGbPcsXMv57201758 = trGbPcsXMv27249568;     trGbPcsXMv27249568 = trGbPcsXMv36886088;     trGbPcsXMv36886088 = trGbPcsXMv97594293;     trGbPcsXMv97594293 = trGbPcsXMv81843007;     trGbPcsXMv81843007 = trGbPcsXMv49475744;     trGbPcsXMv49475744 = trGbPcsXMv86660733;     trGbPcsXMv86660733 = trGbPcsXMv7810096;     trGbPcsXMv7810096 = trGbPcsXMv75854541;     trGbPcsXMv75854541 = trGbPcsXMv27220860;     trGbPcsXMv27220860 = trGbPcsXMv97575743;     trGbPcsXMv97575743 = trGbPcsXMv24318636;     trGbPcsXMv24318636 = trGbPcsXMv65170611;     trGbPcsXMv65170611 = trGbPcsXMv62229816;     trGbPcsXMv62229816 = trGbPcsXMv74890858;     trGbPcsXMv74890858 = trGbPcsXMv60958490;     trGbPcsXMv60958490 = trGbPcsXMv11014396;     trGbPcsXMv11014396 = trGbPcsXMv22189364;     trGbPcsXMv22189364 = trGbPcsXMv4777195;     trGbPcsXMv4777195 = trGbPcsXMv30749645;     trGbPcsXMv30749645 = trGbPcsXMv16949369;     trGbPcsXMv16949369 = trGbPcsXMv24576685;     trGbPcsXMv24576685 = trGbPcsXMv37299993;     trGbPcsXMv37299993 = trGbPcsXMv83642792;     trGbPcsXMv83642792 = trGbPcsXMv59078345;     trGbPcsXMv59078345 = trGbPcsXMv37467199;     trGbPcsXMv37467199 = trGbPcsXMv53442254;     trGbPcsXMv53442254 = trGbPcsXMv30743226;     trGbPcsXMv30743226 = trGbPcsXMv96636675;     trGbPcsXMv96636675 = trGbPcsXMv23345584;     trGbPcsXMv23345584 = trGbPcsXMv17500008;     trGbPcsXMv17500008 = trGbPcsXMv2452470;     trGbPcsXMv2452470 = trGbPcsXMv93473899;     trGbPcsXMv93473899 = trGbPcsXMv23682566;     trGbPcsXMv23682566 = trGbPcsXMv35192914;     trGbPcsXMv35192914 = trGbPcsXMv4464762;     trGbPcsXMv4464762 = trGbPcsXMv10701124;     trGbPcsXMv10701124 = trGbPcsXMv15950269;     trGbPcsXMv15950269 = trGbPcsXMv77564950;     trGbPcsXMv77564950 = trGbPcsXMv29279575;     trGbPcsXMv29279575 = trGbPcsXMv28791792;     trGbPcsXMv28791792 = trGbPcsXMv67872262;     trGbPcsXMv67872262 = trGbPcsXMv94672820;     trGbPcsXMv94672820 = trGbPcsXMv6006730;     trGbPcsXMv6006730 = trGbPcsXMv89438787;     trGbPcsXMv89438787 = trGbPcsXMv10430817;     trGbPcsXMv10430817 = trGbPcsXMv61897699;     trGbPcsXMv61897699 = trGbPcsXMv10453777;     trGbPcsXMv10453777 = trGbPcsXMv46617590;     trGbPcsXMv46617590 = trGbPcsXMv52250464;     trGbPcsXMv52250464 = trGbPcsXMv39558185;     trGbPcsXMv39558185 = trGbPcsXMv71425084;     trGbPcsXMv71425084 = trGbPcsXMv52157876;     trGbPcsXMv52157876 = trGbPcsXMv56081283;     trGbPcsXMv56081283 = trGbPcsXMv43652869;     trGbPcsXMv43652869 = trGbPcsXMv32625074;     trGbPcsXMv32625074 = trGbPcsXMv89949574;     trGbPcsXMv89949574 = trGbPcsXMv53243295;     trGbPcsXMv53243295 = trGbPcsXMv38515949;     trGbPcsXMv38515949 = trGbPcsXMv44375808;     trGbPcsXMv44375808 = trGbPcsXMv96033489;     trGbPcsXMv96033489 = trGbPcsXMv55917508;     trGbPcsXMv55917508 = trGbPcsXMv11173421;     trGbPcsXMv11173421 = trGbPcsXMv52508958;     trGbPcsXMv52508958 = trGbPcsXMv9720853;     trGbPcsXMv9720853 = trGbPcsXMv95123273;     trGbPcsXMv95123273 = trGbPcsXMv30844736;     trGbPcsXMv30844736 = trGbPcsXMv41488045;     trGbPcsXMv41488045 = trGbPcsXMv27036903;     trGbPcsXMv27036903 = trGbPcsXMv70426097;     trGbPcsXMv70426097 = trGbPcsXMv50257367;     trGbPcsXMv50257367 = trGbPcsXMv95064126;     trGbPcsXMv95064126 = trGbPcsXMv44624413;     trGbPcsXMv44624413 = trGbPcsXMv75497620;     trGbPcsXMv75497620 = trGbPcsXMv1957853;     trGbPcsXMv1957853 = trGbPcsXMv49077107;     trGbPcsXMv49077107 = trGbPcsXMv29903865;     trGbPcsXMv29903865 = trGbPcsXMv80304827;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HzAWIZmIEG77600882() {     int IwRmeDhtml63749133 = -436439780;    int IwRmeDhtml1156893 = 36861864;    int IwRmeDhtml88803179 = -258402421;    int IwRmeDhtml94506580 = -362535331;    int IwRmeDhtml53708534 = -333994233;    int IwRmeDhtml12048963 = -953689659;    int IwRmeDhtml30447275 = -251662987;    int IwRmeDhtml93102761 = -65496642;    int IwRmeDhtml73982226 = -834635196;    int IwRmeDhtml8228010 = -915135592;    int IwRmeDhtml19943299 = -778797092;    int IwRmeDhtml96432988 = -107291577;    int IwRmeDhtml5132307 = -763415003;    int IwRmeDhtml14376345 = -628623305;    int IwRmeDhtml87922773 = -461986876;    int IwRmeDhtml59072754 = 63698193;    int IwRmeDhtml15260333 = -388264457;    int IwRmeDhtml94060400 = -881597936;    int IwRmeDhtml13012722 = -197057745;    int IwRmeDhtml82981890 = -905898735;    int IwRmeDhtml12575459 = -125726546;    int IwRmeDhtml98978246 = 67992880;    int IwRmeDhtml64427945 = -2418549;    int IwRmeDhtml91908316 = -196378503;    int IwRmeDhtml68825526 = -235146571;    int IwRmeDhtml32228237 = 37075895;    int IwRmeDhtml47049034 = -889977929;    int IwRmeDhtml21574633 = 51969178;    int IwRmeDhtml85556837 = -373070507;    int IwRmeDhtml2494075 = -118042463;    int IwRmeDhtml25905410 = -894494382;    int IwRmeDhtml38960968 = -319837338;    int IwRmeDhtml70759747 = -680538800;    int IwRmeDhtml39734254 = -561668867;    int IwRmeDhtml3045366 = -552445317;    int IwRmeDhtml77559795 = -138591711;    int IwRmeDhtml30843995 = -990296555;    int IwRmeDhtml15469691 = 48398857;    int IwRmeDhtml9953569 = -601402673;    int IwRmeDhtml60509530 = 37117071;    int IwRmeDhtml55838288 = -936889679;    int IwRmeDhtml55264981 = -817971174;    int IwRmeDhtml54316714 = -850952025;    int IwRmeDhtml8854309 = -263067279;    int IwRmeDhtml16521182 = -800183644;    int IwRmeDhtml76905989 = -343569095;    int IwRmeDhtml59658222 = -863144541;    int IwRmeDhtml78748940 = -383701701;    int IwRmeDhtml68314311 = -564807149;    int IwRmeDhtml88774738 = 86907420;    int IwRmeDhtml4000674 = 4886549;    int IwRmeDhtml52137470 = -593492433;    int IwRmeDhtml37458042 = -891828068;    int IwRmeDhtml42176767 = -802252467;    int IwRmeDhtml65025423 = -383346100;    int IwRmeDhtml64770887 = -404432661;    int IwRmeDhtml36728948 = -960719587;    int IwRmeDhtml96894862 = 37976082;    int IwRmeDhtml25681055 = -27388760;    int IwRmeDhtml21480298 = -271070128;    int IwRmeDhtml64999929 = 36288270;    int IwRmeDhtml8872643 = -203632166;    int IwRmeDhtml7545924 = -692426135;    int IwRmeDhtml71488152 = -616592733;    int IwRmeDhtml82322600 = 79358789;    int IwRmeDhtml80982331 = -358959754;    int IwRmeDhtml25673242 = -426752777;    int IwRmeDhtml65398053 = -101746136;    int IwRmeDhtml11330979 = 23822011;    int IwRmeDhtml10362978 = -223395165;    int IwRmeDhtml28228760 = 53994747;    int IwRmeDhtml99790642 = -336663314;    int IwRmeDhtml84106832 = -180195263;    int IwRmeDhtml52503191 = -134174817;    int IwRmeDhtml27143603 = -969009057;    int IwRmeDhtml57310477 = -307755372;    int IwRmeDhtml44661532 = -81055095;    int IwRmeDhtml55573636 = -739351271;    int IwRmeDhtml75387135 = -396194859;    int IwRmeDhtml91919536 = -891577476;    int IwRmeDhtml72570014 = -99779565;    int IwRmeDhtml68300093 = -406276229;    int IwRmeDhtml53260321 = -383223673;    int IwRmeDhtml96782099 = -359977928;    int IwRmeDhtml98493401 = -22929012;    int IwRmeDhtml73767939 = -201001949;    int IwRmeDhtml1502926 = -428009271;    int IwRmeDhtml28582980 = -878286334;    int IwRmeDhtml74708830 = -78322767;    int IwRmeDhtml38274479 = -48012656;    int IwRmeDhtml40830848 = -177872124;    int IwRmeDhtml33949133 = -928272637;    int IwRmeDhtml89788636 = -924212383;    int IwRmeDhtml88473270 = -230332546;    int IwRmeDhtml95509601 = -999171199;    int IwRmeDhtml46965645 = -633257514;    int IwRmeDhtml47719058 = -25545039;    int IwRmeDhtml82828562 = -134359293;    int IwRmeDhtml26531709 = -242426068;    int IwRmeDhtml35538851 = -436439780;     IwRmeDhtml63749133 = IwRmeDhtml1156893;     IwRmeDhtml1156893 = IwRmeDhtml88803179;     IwRmeDhtml88803179 = IwRmeDhtml94506580;     IwRmeDhtml94506580 = IwRmeDhtml53708534;     IwRmeDhtml53708534 = IwRmeDhtml12048963;     IwRmeDhtml12048963 = IwRmeDhtml30447275;     IwRmeDhtml30447275 = IwRmeDhtml93102761;     IwRmeDhtml93102761 = IwRmeDhtml73982226;     IwRmeDhtml73982226 = IwRmeDhtml8228010;     IwRmeDhtml8228010 = IwRmeDhtml19943299;     IwRmeDhtml19943299 = IwRmeDhtml96432988;     IwRmeDhtml96432988 = IwRmeDhtml5132307;     IwRmeDhtml5132307 = IwRmeDhtml14376345;     IwRmeDhtml14376345 = IwRmeDhtml87922773;     IwRmeDhtml87922773 = IwRmeDhtml59072754;     IwRmeDhtml59072754 = IwRmeDhtml15260333;     IwRmeDhtml15260333 = IwRmeDhtml94060400;     IwRmeDhtml94060400 = IwRmeDhtml13012722;     IwRmeDhtml13012722 = IwRmeDhtml82981890;     IwRmeDhtml82981890 = IwRmeDhtml12575459;     IwRmeDhtml12575459 = IwRmeDhtml98978246;     IwRmeDhtml98978246 = IwRmeDhtml64427945;     IwRmeDhtml64427945 = IwRmeDhtml91908316;     IwRmeDhtml91908316 = IwRmeDhtml68825526;     IwRmeDhtml68825526 = IwRmeDhtml32228237;     IwRmeDhtml32228237 = IwRmeDhtml47049034;     IwRmeDhtml47049034 = IwRmeDhtml21574633;     IwRmeDhtml21574633 = IwRmeDhtml85556837;     IwRmeDhtml85556837 = IwRmeDhtml2494075;     IwRmeDhtml2494075 = IwRmeDhtml25905410;     IwRmeDhtml25905410 = IwRmeDhtml38960968;     IwRmeDhtml38960968 = IwRmeDhtml70759747;     IwRmeDhtml70759747 = IwRmeDhtml39734254;     IwRmeDhtml39734254 = IwRmeDhtml3045366;     IwRmeDhtml3045366 = IwRmeDhtml77559795;     IwRmeDhtml77559795 = IwRmeDhtml30843995;     IwRmeDhtml30843995 = IwRmeDhtml15469691;     IwRmeDhtml15469691 = IwRmeDhtml9953569;     IwRmeDhtml9953569 = IwRmeDhtml60509530;     IwRmeDhtml60509530 = IwRmeDhtml55838288;     IwRmeDhtml55838288 = IwRmeDhtml55264981;     IwRmeDhtml55264981 = IwRmeDhtml54316714;     IwRmeDhtml54316714 = IwRmeDhtml8854309;     IwRmeDhtml8854309 = IwRmeDhtml16521182;     IwRmeDhtml16521182 = IwRmeDhtml76905989;     IwRmeDhtml76905989 = IwRmeDhtml59658222;     IwRmeDhtml59658222 = IwRmeDhtml78748940;     IwRmeDhtml78748940 = IwRmeDhtml68314311;     IwRmeDhtml68314311 = IwRmeDhtml88774738;     IwRmeDhtml88774738 = IwRmeDhtml4000674;     IwRmeDhtml4000674 = IwRmeDhtml52137470;     IwRmeDhtml52137470 = IwRmeDhtml37458042;     IwRmeDhtml37458042 = IwRmeDhtml42176767;     IwRmeDhtml42176767 = IwRmeDhtml65025423;     IwRmeDhtml65025423 = IwRmeDhtml64770887;     IwRmeDhtml64770887 = IwRmeDhtml36728948;     IwRmeDhtml36728948 = IwRmeDhtml96894862;     IwRmeDhtml96894862 = IwRmeDhtml25681055;     IwRmeDhtml25681055 = IwRmeDhtml21480298;     IwRmeDhtml21480298 = IwRmeDhtml64999929;     IwRmeDhtml64999929 = IwRmeDhtml8872643;     IwRmeDhtml8872643 = IwRmeDhtml7545924;     IwRmeDhtml7545924 = IwRmeDhtml71488152;     IwRmeDhtml71488152 = IwRmeDhtml82322600;     IwRmeDhtml82322600 = IwRmeDhtml80982331;     IwRmeDhtml80982331 = IwRmeDhtml25673242;     IwRmeDhtml25673242 = IwRmeDhtml65398053;     IwRmeDhtml65398053 = IwRmeDhtml11330979;     IwRmeDhtml11330979 = IwRmeDhtml10362978;     IwRmeDhtml10362978 = IwRmeDhtml28228760;     IwRmeDhtml28228760 = IwRmeDhtml99790642;     IwRmeDhtml99790642 = IwRmeDhtml84106832;     IwRmeDhtml84106832 = IwRmeDhtml52503191;     IwRmeDhtml52503191 = IwRmeDhtml27143603;     IwRmeDhtml27143603 = IwRmeDhtml57310477;     IwRmeDhtml57310477 = IwRmeDhtml44661532;     IwRmeDhtml44661532 = IwRmeDhtml55573636;     IwRmeDhtml55573636 = IwRmeDhtml75387135;     IwRmeDhtml75387135 = IwRmeDhtml91919536;     IwRmeDhtml91919536 = IwRmeDhtml72570014;     IwRmeDhtml72570014 = IwRmeDhtml68300093;     IwRmeDhtml68300093 = IwRmeDhtml53260321;     IwRmeDhtml53260321 = IwRmeDhtml96782099;     IwRmeDhtml96782099 = IwRmeDhtml98493401;     IwRmeDhtml98493401 = IwRmeDhtml73767939;     IwRmeDhtml73767939 = IwRmeDhtml1502926;     IwRmeDhtml1502926 = IwRmeDhtml28582980;     IwRmeDhtml28582980 = IwRmeDhtml74708830;     IwRmeDhtml74708830 = IwRmeDhtml38274479;     IwRmeDhtml38274479 = IwRmeDhtml40830848;     IwRmeDhtml40830848 = IwRmeDhtml33949133;     IwRmeDhtml33949133 = IwRmeDhtml89788636;     IwRmeDhtml89788636 = IwRmeDhtml88473270;     IwRmeDhtml88473270 = IwRmeDhtml95509601;     IwRmeDhtml95509601 = IwRmeDhtml46965645;     IwRmeDhtml46965645 = IwRmeDhtml47719058;     IwRmeDhtml47719058 = IwRmeDhtml82828562;     IwRmeDhtml82828562 = IwRmeDhtml26531709;     IwRmeDhtml26531709 = IwRmeDhtml35538851;     IwRmeDhtml35538851 = IwRmeDhtml63749133;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FmONIJXkGw47726558() {     int PeKSvNzURb31196169 = -36483914;    int PeKSvNzURb96353083 = -670441838;    int PeKSvNzURb65975413 = 4982198;    int PeKSvNzURb88802486 = -372299189;    int PeKSvNzURb12831208 = -29890616;    int PeKSvNzURb34727998 = -60330996;    int PeKSvNzURb4208401 = -294691263;    int PeKSvNzURb79782927 = -206405814;    int PeKSvNzURb74421283 = -29600589;    int PeKSvNzURb439187 = -261217623;    int PeKSvNzURb5721371 = -459367608;    int PeKSvNzURb76856530 = 34429881;    int PeKSvNzURb7240011 = -449079490;    int PeKSvNzURb3122634 = -227989030;    int PeKSvNzURb96281291 = -313381486;    int PeKSvNzURb2700812 = -884557110;    int PeKSvNzURb14991155 = -706024182;    int PeKSvNzURb22214940 = -103301083;    int PeKSvNzURb94965683 = -874356222;    int PeKSvNzURb31197179 = -138484569;    int PeKSvNzURb47254479 = -374659587;    int PeKSvNzURb982389 = -858317692;    int PeKSvNzURb2779676 = -61208023;    int PeKSvNzURb94193706 = -374843809;    int PeKSvNzURb42029712 = -391808094;    int PeKSvNzURb38213752 = -848858161;    int PeKSvNzURb90232912 = -677728775;    int PeKSvNzURb6773033 = -620913799;    int PeKSvNzURb29393121 = -321462308;    int PeKSvNzURb73590729 = -363255212;    int PeKSvNzURb88634255 = -232690423;    int PeKSvNzURb88547544 = -318231783;    int PeKSvNzURb99009149 = -910966983;    int PeKSvNzURb11183682 = -269862162;    int PeKSvNzURb38252017 = -21026532;    int PeKSvNzURb17282263 = -95384665;    int PeKSvNzURb30371176 = -530979148;    int PeKSvNzURb16233273 = -849887576;    int PeKSvNzURb31185007 = -112520436;    int PeKSvNzURb38887762 = -956176688;    int PeKSvNzURb85373329 = -236969932;    int PeKSvNzURb13944435 = -30022513;    int PeKSvNzURb19917007 = -155760534;    int PeKSvNzURb3245348 = -520954744;    int PeKSvNzURb72495683 = -352211800;    int PeKSvNzURb65439752 = -673632479;    int PeKSvNzURb26016161 = -906436355;    int PeKSvNzURb56232635 = 92054504;    int PeKSvNzURb36062738 = 35624549;    int PeKSvNzURb60521398 = -77123451;    int PeKSvNzURb84182336 = -326658254;    int PeKSvNzURb55148909 = -97598005;    int PeKSvNzURb28222847 = -915692800;    int PeKSvNzURb34041979 = -855670843;    int PeKSvNzURb472079 = 83200010;    int PeKSvNzURb30213781 = -178166222;    int PeKSvNzURb93573408 = -509233816;    int PeKSvNzURb71781706 = -620173993;    int PeKSvNzURb46772775 = -980491095;    int PeKSvNzURb74617455 = -181032456;    int PeKSvNzURb44495086 = -382602222;    int PeKSvNzURb97435368 = -673777465;    int PeKSvNzURb50389806 = -884943507;    int PeKSvNzURb830555 = -666345377;    int PeKSvNzURb11804931 = 71472799;    int PeKSvNzURb17173826 = -41135825;    int PeKSvNzURb77847380 = -54603136;    int PeKSvNzURb96056328 = -79217328;    int PeKSvNzURb64870617 = -106962499;    int PeKSvNzURb78999028 = -117996821;    int PeKSvNzURb72329636 = -253577963;    int PeKSvNzURb98757881 = -856136606;    int PeKSvNzURb91029933 = -990780648;    int PeKSvNzURb56077921 = -918179534;    int PeKSvNzURb45823850 = -901514638;    int PeKSvNzURb33310045 = -244637075;    int PeKSvNzURb81065381 = -602557159;    int PeKSvNzURb99534327 = -540253280;    int PeKSvNzURb21698024 = 77367991;    int PeKSvNzURb76589959 = -718175616;    int PeKSvNzURb12197591 = -942421806;    int PeKSvNzURb34000278 = -669783279;    int PeKSvNzURb70710294 = -556538348;    int PeKSvNzURb68871722 = -144338857;    int PeKSvNzURb89408392 = 63403041;    int PeKSvNzURb33485346 = -35092418;    int PeKSvNzURb60324697 = -402538984;    int PeKSvNzURb64967171 = 44703859;    int PeKSvNzURb10711603 = -253062173;    int PeKSvNzURb8038236 = -842860310;    int PeKSvNzURb23708855 = -586150850;    int PeKSvNzURb58589469 = -910805155;    int PeKSvNzURb69460497 = -869396482;    int PeKSvNzURb56567551 = -931487981;    int PeKSvNzURb94392676 = -473574466;    int PeKSvNzURb87937960 = -563192468;    int PeKSvNzURb63554628 = -145079006;    int PeKSvNzURb19086453 = -489415158;    int PeKSvNzURb91440417 = -492427543;    int PeKSvNzURb55321857 = -36483914;     PeKSvNzURb31196169 = PeKSvNzURb96353083;     PeKSvNzURb96353083 = PeKSvNzURb65975413;     PeKSvNzURb65975413 = PeKSvNzURb88802486;     PeKSvNzURb88802486 = PeKSvNzURb12831208;     PeKSvNzURb12831208 = PeKSvNzURb34727998;     PeKSvNzURb34727998 = PeKSvNzURb4208401;     PeKSvNzURb4208401 = PeKSvNzURb79782927;     PeKSvNzURb79782927 = PeKSvNzURb74421283;     PeKSvNzURb74421283 = PeKSvNzURb439187;     PeKSvNzURb439187 = PeKSvNzURb5721371;     PeKSvNzURb5721371 = PeKSvNzURb76856530;     PeKSvNzURb76856530 = PeKSvNzURb7240011;     PeKSvNzURb7240011 = PeKSvNzURb3122634;     PeKSvNzURb3122634 = PeKSvNzURb96281291;     PeKSvNzURb96281291 = PeKSvNzURb2700812;     PeKSvNzURb2700812 = PeKSvNzURb14991155;     PeKSvNzURb14991155 = PeKSvNzURb22214940;     PeKSvNzURb22214940 = PeKSvNzURb94965683;     PeKSvNzURb94965683 = PeKSvNzURb31197179;     PeKSvNzURb31197179 = PeKSvNzURb47254479;     PeKSvNzURb47254479 = PeKSvNzURb982389;     PeKSvNzURb982389 = PeKSvNzURb2779676;     PeKSvNzURb2779676 = PeKSvNzURb94193706;     PeKSvNzURb94193706 = PeKSvNzURb42029712;     PeKSvNzURb42029712 = PeKSvNzURb38213752;     PeKSvNzURb38213752 = PeKSvNzURb90232912;     PeKSvNzURb90232912 = PeKSvNzURb6773033;     PeKSvNzURb6773033 = PeKSvNzURb29393121;     PeKSvNzURb29393121 = PeKSvNzURb73590729;     PeKSvNzURb73590729 = PeKSvNzURb88634255;     PeKSvNzURb88634255 = PeKSvNzURb88547544;     PeKSvNzURb88547544 = PeKSvNzURb99009149;     PeKSvNzURb99009149 = PeKSvNzURb11183682;     PeKSvNzURb11183682 = PeKSvNzURb38252017;     PeKSvNzURb38252017 = PeKSvNzURb17282263;     PeKSvNzURb17282263 = PeKSvNzURb30371176;     PeKSvNzURb30371176 = PeKSvNzURb16233273;     PeKSvNzURb16233273 = PeKSvNzURb31185007;     PeKSvNzURb31185007 = PeKSvNzURb38887762;     PeKSvNzURb38887762 = PeKSvNzURb85373329;     PeKSvNzURb85373329 = PeKSvNzURb13944435;     PeKSvNzURb13944435 = PeKSvNzURb19917007;     PeKSvNzURb19917007 = PeKSvNzURb3245348;     PeKSvNzURb3245348 = PeKSvNzURb72495683;     PeKSvNzURb72495683 = PeKSvNzURb65439752;     PeKSvNzURb65439752 = PeKSvNzURb26016161;     PeKSvNzURb26016161 = PeKSvNzURb56232635;     PeKSvNzURb56232635 = PeKSvNzURb36062738;     PeKSvNzURb36062738 = PeKSvNzURb60521398;     PeKSvNzURb60521398 = PeKSvNzURb84182336;     PeKSvNzURb84182336 = PeKSvNzURb55148909;     PeKSvNzURb55148909 = PeKSvNzURb28222847;     PeKSvNzURb28222847 = PeKSvNzURb34041979;     PeKSvNzURb34041979 = PeKSvNzURb472079;     PeKSvNzURb472079 = PeKSvNzURb30213781;     PeKSvNzURb30213781 = PeKSvNzURb93573408;     PeKSvNzURb93573408 = PeKSvNzURb71781706;     PeKSvNzURb71781706 = PeKSvNzURb46772775;     PeKSvNzURb46772775 = PeKSvNzURb74617455;     PeKSvNzURb74617455 = PeKSvNzURb44495086;     PeKSvNzURb44495086 = PeKSvNzURb97435368;     PeKSvNzURb97435368 = PeKSvNzURb50389806;     PeKSvNzURb50389806 = PeKSvNzURb830555;     PeKSvNzURb830555 = PeKSvNzURb11804931;     PeKSvNzURb11804931 = PeKSvNzURb17173826;     PeKSvNzURb17173826 = PeKSvNzURb77847380;     PeKSvNzURb77847380 = PeKSvNzURb96056328;     PeKSvNzURb96056328 = PeKSvNzURb64870617;     PeKSvNzURb64870617 = PeKSvNzURb78999028;     PeKSvNzURb78999028 = PeKSvNzURb72329636;     PeKSvNzURb72329636 = PeKSvNzURb98757881;     PeKSvNzURb98757881 = PeKSvNzURb91029933;     PeKSvNzURb91029933 = PeKSvNzURb56077921;     PeKSvNzURb56077921 = PeKSvNzURb45823850;     PeKSvNzURb45823850 = PeKSvNzURb33310045;     PeKSvNzURb33310045 = PeKSvNzURb81065381;     PeKSvNzURb81065381 = PeKSvNzURb99534327;     PeKSvNzURb99534327 = PeKSvNzURb21698024;     PeKSvNzURb21698024 = PeKSvNzURb76589959;     PeKSvNzURb76589959 = PeKSvNzURb12197591;     PeKSvNzURb12197591 = PeKSvNzURb34000278;     PeKSvNzURb34000278 = PeKSvNzURb70710294;     PeKSvNzURb70710294 = PeKSvNzURb68871722;     PeKSvNzURb68871722 = PeKSvNzURb89408392;     PeKSvNzURb89408392 = PeKSvNzURb33485346;     PeKSvNzURb33485346 = PeKSvNzURb60324697;     PeKSvNzURb60324697 = PeKSvNzURb64967171;     PeKSvNzURb64967171 = PeKSvNzURb10711603;     PeKSvNzURb10711603 = PeKSvNzURb8038236;     PeKSvNzURb8038236 = PeKSvNzURb23708855;     PeKSvNzURb23708855 = PeKSvNzURb58589469;     PeKSvNzURb58589469 = PeKSvNzURb69460497;     PeKSvNzURb69460497 = PeKSvNzURb56567551;     PeKSvNzURb56567551 = PeKSvNzURb94392676;     PeKSvNzURb94392676 = PeKSvNzURb87937960;     PeKSvNzURb87937960 = PeKSvNzURb63554628;     PeKSvNzURb63554628 = PeKSvNzURb19086453;     PeKSvNzURb19086453 = PeKSvNzURb91440417;     PeKSvNzURb91440417 = PeKSvNzURb55321857;     PeKSvNzURb55321857 = PeKSvNzURb31196169;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RGVNGGgMNH18182431() {     int aeEkYGoQFf43982336 = -171524606;    int aeEkYGoQFf83123972 = -840775852;    int aeEkYGoQFf83728213 = -403377184;    int aeEkYGoQFf96743312 = -196501466;    int aeEkYGoQFf6342312 = -521707081;    int aeEkYGoQFf78038068 = -937254310;    int aeEkYGoQFf25416159 = -840363334;    int aeEkYGoQFf6502040 = -650924071;    int aeEkYGoQFf69826094 = 8641319;    int aeEkYGoQFf6657891 = -65305256;    int aeEkYGoQFf33208753 = -574053623;    int aeEkYGoQFf8807035 = -472902935;    int aeEkYGoQFf77383313 = -352862147;    int aeEkYGoQFf52746712 = -560404228;    int aeEkYGoQFf19587844 = -220367317;    int aeEkYGoQFf31125880 = -335448499;    int aeEkYGoQFf70742972 = -369798649;    int aeEkYGoQFf5773426 = -758511385;    int aeEkYGoQFf96126313 = -783825417;    int aeEkYGoQFf33851676 = -737728708;    int aeEkYGoQFf27428536 = -865975851;    int aeEkYGoQFf30030208 = 71823960;    int aeEkYGoQFf41539971 = 7788082;    int aeEkYGoQFf38700836 = -735188821;    int aeEkYGoQFf47923971 = -602250019;    int aeEkYGoQFf19532398 = -539289360;    int aeEkYGoQFf1648449 = -211435055;    int aeEkYGoQFf74125957 = -794711904;    int aeEkYGoQFf46010174 = -535679548;    int aeEkYGoQFf26341845 = 6858633;    int aeEkYGoQFf33070251 = -509107173;    int aeEkYGoQFf91866368 = -296951462;    int aeEkYGoQFf45055099 = -786821768;    int aeEkYGoQFf88260054 = -174616381;    int aeEkYGoQFf21312549 = -631058412;    int aeEkYGoQFf12506378 = -983947770;    int aeEkYGoQFf67135236 = -320888498;    int aeEkYGoQFf73937295 = -769507668;    int aeEkYGoQFf36960393 = 45628987;    int aeEkYGoQFf65152492 = -800593533;    int aeEkYGoQFf46804637 = -612443925;    int aeEkYGoQFf21646891 = 71666153;    int aeEkYGoQFf54800477 = -179193979;    int aeEkYGoQFf47873886 = -862126987;    int aeEkYGoQFf52504173 = -244981761;    int aeEkYGoQFf39067882 = -746950918;    int aeEkYGoQFf3598181 = -303951407;    int aeEkYGoQFf85672601 = -63298276;    int aeEkYGoQFf80126787 = -127977426;    int aeEkYGoQFf7471819 = -688159914;    int aeEkYGoQFf2509831 = -579542563;    int aeEkYGoQFf83325456 = -568783343;    int aeEkYGoQFf62707997 = 8303917;    int aeEkYGoQFf78741704 = -445122585;    int aeEkYGoQFf90753895 = 88755523;    int aeEkYGoQFf13952129 = -143348566;    int aeEkYGoQFf41584001 = -748563935;    int aeEkYGoQFf45027377 = -668188363;    int aeEkYGoQFf48819341 = -594251448;    int aeEkYGoQFf86809914 = -982417722;    int aeEkYGoQFf76389620 = -625819255;    int aeEkYGoQFf51290202 = 54348570;    int aeEkYGoQFf60491866 = -15244524;    int aeEkYGoQFf43484249 = -998217314;    int aeEkYGoQFf73587639 = -556198084;    int aeEkYGoQFf41342385 = -177102161;    int aeEkYGoQFf63751936 = -686081168;    int aeEkYGoQFf89123258 = -78245767;    int aeEkYGoQFf31434164 = -929345817;    int aeEkYGoQFf7081467 = -236419548;    int aeEkYGoQFf63990644 = 85439999;    int aeEkYGoQFf96805676 = -600290982;    int aeEkYGoQFf68813032 = -704140373;    int aeEkYGoQFf30973822 = -983231884;    int aeEkYGoQFf87047038 = -25284784;    int aeEkYGoQFf5781646 = -837642004;    int aeEkYGoQFf75229731 = -748982062;    int aeEkYGoQFf93666085 = -130084931;    int aeEkYGoQFf86196663 = -390207061;    int aeEkYGoQFf8856090 = -855299101;    int aeEkYGoQFf15934217 = -135337953;    int aeEkYGoQFf15975848 = -48136780;    int aeEkYGoQFf93999169 = -566734478;    int aeEkYGoQFf38538355 = -847519635;    int aeEkYGoQFf23832015 = -413598805;    int aeEkYGoQFf49744795 = -940323830;    int aeEkYGoQFf29158371 = -205255380;    int aeEkYGoQFf66313394 = -241699183;    int aeEkYGoQFf97506159 = -163371905;    int aeEkYGoQFf7360420 = -387709847;    int aeEkYGoQFf70922377 = -135383835;    int aeEkYGoQFf22107859 = -652700136;    int aeEkYGoQFf25117954 = -75256221;    int aeEkYGoQFf50150478 = 28046708;    int aeEkYGoQFf88762871 = -74774279;    int aeEkYGoQFf95514435 = -566792495;    int aeEkYGoQFf61155025 = -913089324;    int aeEkYGoQFf11316228 = -180976665;    int aeEkYGoQFf74286246 = -205928903;    int aeEkYGoQFf11161789 = -171524606;     aeEkYGoQFf43982336 = aeEkYGoQFf83123972;     aeEkYGoQFf83123972 = aeEkYGoQFf83728213;     aeEkYGoQFf83728213 = aeEkYGoQFf96743312;     aeEkYGoQFf96743312 = aeEkYGoQFf6342312;     aeEkYGoQFf6342312 = aeEkYGoQFf78038068;     aeEkYGoQFf78038068 = aeEkYGoQFf25416159;     aeEkYGoQFf25416159 = aeEkYGoQFf6502040;     aeEkYGoQFf6502040 = aeEkYGoQFf69826094;     aeEkYGoQFf69826094 = aeEkYGoQFf6657891;     aeEkYGoQFf6657891 = aeEkYGoQFf33208753;     aeEkYGoQFf33208753 = aeEkYGoQFf8807035;     aeEkYGoQFf8807035 = aeEkYGoQFf77383313;     aeEkYGoQFf77383313 = aeEkYGoQFf52746712;     aeEkYGoQFf52746712 = aeEkYGoQFf19587844;     aeEkYGoQFf19587844 = aeEkYGoQFf31125880;     aeEkYGoQFf31125880 = aeEkYGoQFf70742972;     aeEkYGoQFf70742972 = aeEkYGoQFf5773426;     aeEkYGoQFf5773426 = aeEkYGoQFf96126313;     aeEkYGoQFf96126313 = aeEkYGoQFf33851676;     aeEkYGoQFf33851676 = aeEkYGoQFf27428536;     aeEkYGoQFf27428536 = aeEkYGoQFf30030208;     aeEkYGoQFf30030208 = aeEkYGoQFf41539971;     aeEkYGoQFf41539971 = aeEkYGoQFf38700836;     aeEkYGoQFf38700836 = aeEkYGoQFf47923971;     aeEkYGoQFf47923971 = aeEkYGoQFf19532398;     aeEkYGoQFf19532398 = aeEkYGoQFf1648449;     aeEkYGoQFf1648449 = aeEkYGoQFf74125957;     aeEkYGoQFf74125957 = aeEkYGoQFf46010174;     aeEkYGoQFf46010174 = aeEkYGoQFf26341845;     aeEkYGoQFf26341845 = aeEkYGoQFf33070251;     aeEkYGoQFf33070251 = aeEkYGoQFf91866368;     aeEkYGoQFf91866368 = aeEkYGoQFf45055099;     aeEkYGoQFf45055099 = aeEkYGoQFf88260054;     aeEkYGoQFf88260054 = aeEkYGoQFf21312549;     aeEkYGoQFf21312549 = aeEkYGoQFf12506378;     aeEkYGoQFf12506378 = aeEkYGoQFf67135236;     aeEkYGoQFf67135236 = aeEkYGoQFf73937295;     aeEkYGoQFf73937295 = aeEkYGoQFf36960393;     aeEkYGoQFf36960393 = aeEkYGoQFf65152492;     aeEkYGoQFf65152492 = aeEkYGoQFf46804637;     aeEkYGoQFf46804637 = aeEkYGoQFf21646891;     aeEkYGoQFf21646891 = aeEkYGoQFf54800477;     aeEkYGoQFf54800477 = aeEkYGoQFf47873886;     aeEkYGoQFf47873886 = aeEkYGoQFf52504173;     aeEkYGoQFf52504173 = aeEkYGoQFf39067882;     aeEkYGoQFf39067882 = aeEkYGoQFf3598181;     aeEkYGoQFf3598181 = aeEkYGoQFf85672601;     aeEkYGoQFf85672601 = aeEkYGoQFf80126787;     aeEkYGoQFf80126787 = aeEkYGoQFf7471819;     aeEkYGoQFf7471819 = aeEkYGoQFf2509831;     aeEkYGoQFf2509831 = aeEkYGoQFf83325456;     aeEkYGoQFf83325456 = aeEkYGoQFf62707997;     aeEkYGoQFf62707997 = aeEkYGoQFf78741704;     aeEkYGoQFf78741704 = aeEkYGoQFf90753895;     aeEkYGoQFf90753895 = aeEkYGoQFf13952129;     aeEkYGoQFf13952129 = aeEkYGoQFf41584001;     aeEkYGoQFf41584001 = aeEkYGoQFf45027377;     aeEkYGoQFf45027377 = aeEkYGoQFf48819341;     aeEkYGoQFf48819341 = aeEkYGoQFf86809914;     aeEkYGoQFf86809914 = aeEkYGoQFf76389620;     aeEkYGoQFf76389620 = aeEkYGoQFf51290202;     aeEkYGoQFf51290202 = aeEkYGoQFf60491866;     aeEkYGoQFf60491866 = aeEkYGoQFf43484249;     aeEkYGoQFf43484249 = aeEkYGoQFf73587639;     aeEkYGoQFf73587639 = aeEkYGoQFf41342385;     aeEkYGoQFf41342385 = aeEkYGoQFf63751936;     aeEkYGoQFf63751936 = aeEkYGoQFf89123258;     aeEkYGoQFf89123258 = aeEkYGoQFf31434164;     aeEkYGoQFf31434164 = aeEkYGoQFf7081467;     aeEkYGoQFf7081467 = aeEkYGoQFf63990644;     aeEkYGoQFf63990644 = aeEkYGoQFf96805676;     aeEkYGoQFf96805676 = aeEkYGoQFf68813032;     aeEkYGoQFf68813032 = aeEkYGoQFf30973822;     aeEkYGoQFf30973822 = aeEkYGoQFf87047038;     aeEkYGoQFf87047038 = aeEkYGoQFf5781646;     aeEkYGoQFf5781646 = aeEkYGoQFf75229731;     aeEkYGoQFf75229731 = aeEkYGoQFf93666085;     aeEkYGoQFf93666085 = aeEkYGoQFf86196663;     aeEkYGoQFf86196663 = aeEkYGoQFf8856090;     aeEkYGoQFf8856090 = aeEkYGoQFf15934217;     aeEkYGoQFf15934217 = aeEkYGoQFf15975848;     aeEkYGoQFf15975848 = aeEkYGoQFf93999169;     aeEkYGoQFf93999169 = aeEkYGoQFf38538355;     aeEkYGoQFf38538355 = aeEkYGoQFf23832015;     aeEkYGoQFf23832015 = aeEkYGoQFf49744795;     aeEkYGoQFf49744795 = aeEkYGoQFf29158371;     aeEkYGoQFf29158371 = aeEkYGoQFf66313394;     aeEkYGoQFf66313394 = aeEkYGoQFf97506159;     aeEkYGoQFf97506159 = aeEkYGoQFf7360420;     aeEkYGoQFf7360420 = aeEkYGoQFf70922377;     aeEkYGoQFf70922377 = aeEkYGoQFf22107859;     aeEkYGoQFf22107859 = aeEkYGoQFf25117954;     aeEkYGoQFf25117954 = aeEkYGoQFf50150478;     aeEkYGoQFf50150478 = aeEkYGoQFf88762871;     aeEkYGoQFf88762871 = aeEkYGoQFf95514435;     aeEkYGoQFf95514435 = aeEkYGoQFf61155025;     aeEkYGoQFf61155025 = aeEkYGoQFf11316228;     aeEkYGoQFf11316228 = aeEkYGoQFf74286246;     aeEkYGoQFf74286246 = aeEkYGoQFf11161789;     aeEkYGoQFf11161789 = aeEkYGoQFf43982336;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hYeoHmYUiK36395772() {     int orqxkwDLzi27426642 = 47809714;    int orqxkwDLzi99996060 = -840158039;    int orqxkwDLzi80136720 = -196902412;    int orqxkwDLzi59535564 = -390301301;    int orqxkwDLzi12463636 = -328574572;    int orqxkwDLzi76542469 = -716325963;    int orqxkwDLzi96455477 = -580274647;    int orqxkwDLzi20849483 = -638082101;    int orqxkwDLzi28355795 = -332818032;    int orqxkwDLzi39203543 = -533681369;    int orqxkwDLzi82624691 = -764169496;    int orqxkwDLzi72012434 = -460521181;    int orqxkwDLzi95501088 = -144523388;    int orqxkwDLzi32373604 = -348694585;    int orqxkwDLzi80442307 = -314390299;    int orqxkwDLzi17515042 = -123527824;    int orqxkwDLzi64494857 = -123143676;    int orqxkwDLzi86624871 = 94183738;    int orqxkwDLzi58566456 = -782500288;    int orqxkwDLzi23219118 = -167314701;    int orqxkwDLzi83068923 = -455504882;    int orqxkwDLzi42177526 = 80672190;    int orqxkwDLzi45365679 = -32101115;    int orqxkwDLzi73407394 = -807014217;    int orqxkwDLzi89499929 = -61902777;    int orqxkwDLzi14874546 = -419799075;    int orqxkwDLzi51103189 = -939519398;    int orqxkwDLzi13857583 = -864666787;    int orqxkwDLzi82091268 = -398184690;    int orqxkwDLzi42175186 = -505991219;    int orqxkwDLzi51165565 = -146864374;    int orqxkwDLzi54972795 = -212146542;    int orqxkwDLzi88593985 = -407693945;    int orqxkwDLzi30418565 = -866218551;    int orqxkwDLzi39280 = -828723147;    int orqxkwDLzi24895562 = -771971675;    int orqxkwDLzi35749415 = -199737678;    int orqxkwDLzi14516128 = -477978188;    int orqxkwDLzi85955471 = -139268811;    int orqxkwDLzi14647627 = -553187056;    int orqxkwDLzi80453561 = -183992898;    int orqxkwDLzi72134676 = -502242169;    int orqxkwDLzi78367546 = -730251222;    int orqxkwDLzi39778826 = -618309758;    int orqxkwDLzi44448670 = 26861286;    int orqxkwDLzi78673877 = -766561842;    int orqxkwDLzi79613611 = -436255637;    int orqxkwDLzi5343197 = -62019995;    int orqxkwDLzi10973900 = -713579509;    int orqxkwDLzi42804303 = -895180369;    int orqxkwDLzi75767278 = -800443985;    int orqxkwDLzi38826251 = -627042655;    int orqxkwDLzi76820456 = -203443399;    int orqxkwDLzi3418463 = -163535972;    int orqxkwDLzi53326849 = -259730600;    int orqxkwDLzi85249116 = 67137524;    int orqxkwDLzi54630382 = -708056925;    int orqxkwDLzi6729326 = -389888195;    int orqxkwDLzi70035635 = -228398525;    int orqxkwDLzi97589089 = -908775497;    int orqxkwDLzi25439281 = -776806565;    int orqxkwDLzi82597894 = -715607860;    int orqxkwDLzi38758214 = -139897411;    int orqxkwDLzi86180608 = -826826814;    int orqxkwDLzi88037977 = -286816995;    int orqxkwDLzi27651896 = -452022954;    int orqxkwDLzi83418449 = 47172763;    int orqxkwDLzi65082524 = -278304838;    int orqxkwDLzi32334325 = -519971438;    int orqxkwDLzi55546745 = -542418624;    int orqxkwDLzi81765627 = -923790147;    int orqxkwDLzi49978729 = -645165489;    int orqxkwDLzi669401 = -766547451;    int orqxkwDLzi43918829 = -129313233;    int orqxkwDLzi42765557 = -983321803;    int orqxkwDLzi10934247 = -953262714;    int orqxkwDLzi63809979 = -189076589;    int orqxkwDLzi5586854 = -413791358;    int orqxkwDLzi28958725 = -733875504;    int orqxkwDLzi10826052 = -295340936;    int orqxkwDLzi35260935 = -983543439;    int orqxkwDLzi45759992 = -777499404;    int orqxkwDLzi2883683 = -51087279;    int orqxkwDLzi39286965 = -503004322;    int orqxkwDLzi66407907 = -705547234;    int orqxkwDLzi12339315 = -519821719;    int orqxkwDLzi78152339 = 91296857;    int orqxkwDLzi85175522 = -144157973;    int orqxkwDLzi77091716 = -506487952;    int orqxkwDLzi14790163 = -795860671;    int orqxkwDLzi70265180 = 36085249;    int orqxkwDLzi29020089 = -809849484;    int orqxkwDLzi44480493 = -149579664;    int orqxkwDLzi88366381 = -230493314;    int orqxkwDLzi89208346 = -776380491;    int orqxkwDLzi97855667 = -468385039;    int orqxkwDLzi33376462 = -262344758;    int orqxkwDLzi92186937 = -903424408;    int orqxkwDLzi51740848 = -231492763;    int orqxkwDLzi16796775 = 47809714;     orqxkwDLzi27426642 = orqxkwDLzi99996060;     orqxkwDLzi99996060 = orqxkwDLzi80136720;     orqxkwDLzi80136720 = orqxkwDLzi59535564;     orqxkwDLzi59535564 = orqxkwDLzi12463636;     orqxkwDLzi12463636 = orqxkwDLzi76542469;     orqxkwDLzi76542469 = orqxkwDLzi96455477;     orqxkwDLzi96455477 = orqxkwDLzi20849483;     orqxkwDLzi20849483 = orqxkwDLzi28355795;     orqxkwDLzi28355795 = orqxkwDLzi39203543;     orqxkwDLzi39203543 = orqxkwDLzi82624691;     orqxkwDLzi82624691 = orqxkwDLzi72012434;     orqxkwDLzi72012434 = orqxkwDLzi95501088;     orqxkwDLzi95501088 = orqxkwDLzi32373604;     orqxkwDLzi32373604 = orqxkwDLzi80442307;     orqxkwDLzi80442307 = orqxkwDLzi17515042;     orqxkwDLzi17515042 = orqxkwDLzi64494857;     orqxkwDLzi64494857 = orqxkwDLzi86624871;     orqxkwDLzi86624871 = orqxkwDLzi58566456;     orqxkwDLzi58566456 = orqxkwDLzi23219118;     orqxkwDLzi23219118 = orqxkwDLzi83068923;     orqxkwDLzi83068923 = orqxkwDLzi42177526;     orqxkwDLzi42177526 = orqxkwDLzi45365679;     orqxkwDLzi45365679 = orqxkwDLzi73407394;     orqxkwDLzi73407394 = orqxkwDLzi89499929;     orqxkwDLzi89499929 = orqxkwDLzi14874546;     orqxkwDLzi14874546 = orqxkwDLzi51103189;     orqxkwDLzi51103189 = orqxkwDLzi13857583;     orqxkwDLzi13857583 = orqxkwDLzi82091268;     orqxkwDLzi82091268 = orqxkwDLzi42175186;     orqxkwDLzi42175186 = orqxkwDLzi51165565;     orqxkwDLzi51165565 = orqxkwDLzi54972795;     orqxkwDLzi54972795 = orqxkwDLzi88593985;     orqxkwDLzi88593985 = orqxkwDLzi30418565;     orqxkwDLzi30418565 = orqxkwDLzi39280;     orqxkwDLzi39280 = orqxkwDLzi24895562;     orqxkwDLzi24895562 = orqxkwDLzi35749415;     orqxkwDLzi35749415 = orqxkwDLzi14516128;     orqxkwDLzi14516128 = orqxkwDLzi85955471;     orqxkwDLzi85955471 = orqxkwDLzi14647627;     orqxkwDLzi14647627 = orqxkwDLzi80453561;     orqxkwDLzi80453561 = orqxkwDLzi72134676;     orqxkwDLzi72134676 = orqxkwDLzi78367546;     orqxkwDLzi78367546 = orqxkwDLzi39778826;     orqxkwDLzi39778826 = orqxkwDLzi44448670;     orqxkwDLzi44448670 = orqxkwDLzi78673877;     orqxkwDLzi78673877 = orqxkwDLzi79613611;     orqxkwDLzi79613611 = orqxkwDLzi5343197;     orqxkwDLzi5343197 = orqxkwDLzi10973900;     orqxkwDLzi10973900 = orqxkwDLzi42804303;     orqxkwDLzi42804303 = orqxkwDLzi75767278;     orqxkwDLzi75767278 = orqxkwDLzi38826251;     orqxkwDLzi38826251 = orqxkwDLzi76820456;     orqxkwDLzi76820456 = orqxkwDLzi3418463;     orqxkwDLzi3418463 = orqxkwDLzi53326849;     orqxkwDLzi53326849 = orqxkwDLzi85249116;     orqxkwDLzi85249116 = orqxkwDLzi54630382;     orqxkwDLzi54630382 = orqxkwDLzi6729326;     orqxkwDLzi6729326 = orqxkwDLzi70035635;     orqxkwDLzi70035635 = orqxkwDLzi97589089;     orqxkwDLzi97589089 = orqxkwDLzi25439281;     orqxkwDLzi25439281 = orqxkwDLzi82597894;     orqxkwDLzi82597894 = orqxkwDLzi38758214;     orqxkwDLzi38758214 = orqxkwDLzi86180608;     orqxkwDLzi86180608 = orqxkwDLzi88037977;     orqxkwDLzi88037977 = orqxkwDLzi27651896;     orqxkwDLzi27651896 = orqxkwDLzi83418449;     orqxkwDLzi83418449 = orqxkwDLzi65082524;     orqxkwDLzi65082524 = orqxkwDLzi32334325;     orqxkwDLzi32334325 = orqxkwDLzi55546745;     orqxkwDLzi55546745 = orqxkwDLzi81765627;     orqxkwDLzi81765627 = orqxkwDLzi49978729;     orqxkwDLzi49978729 = orqxkwDLzi669401;     orqxkwDLzi669401 = orqxkwDLzi43918829;     orqxkwDLzi43918829 = orqxkwDLzi42765557;     orqxkwDLzi42765557 = orqxkwDLzi10934247;     orqxkwDLzi10934247 = orqxkwDLzi63809979;     orqxkwDLzi63809979 = orqxkwDLzi5586854;     orqxkwDLzi5586854 = orqxkwDLzi28958725;     orqxkwDLzi28958725 = orqxkwDLzi10826052;     orqxkwDLzi10826052 = orqxkwDLzi35260935;     orqxkwDLzi35260935 = orqxkwDLzi45759992;     orqxkwDLzi45759992 = orqxkwDLzi2883683;     orqxkwDLzi2883683 = orqxkwDLzi39286965;     orqxkwDLzi39286965 = orqxkwDLzi66407907;     orqxkwDLzi66407907 = orqxkwDLzi12339315;     orqxkwDLzi12339315 = orqxkwDLzi78152339;     orqxkwDLzi78152339 = orqxkwDLzi85175522;     orqxkwDLzi85175522 = orqxkwDLzi77091716;     orqxkwDLzi77091716 = orqxkwDLzi14790163;     orqxkwDLzi14790163 = orqxkwDLzi70265180;     orqxkwDLzi70265180 = orqxkwDLzi29020089;     orqxkwDLzi29020089 = orqxkwDLzi44480493;     orqxkwDLzi44480493 = orqxkwDLzi88366381;     orqxkwDLzi88366381 = orqxkwDLzi89208346;     orqxkwDLzi89208346 = orqxkwDLzi97855667;     orqxkwDLzi97855667 = orqxkwDLzi33376462;     orqxkwDLzi33376462 = orqxkwDLzi92186937;     orqxkwDLzi92186937 = orqxkwDLzi51740848;     orqxkwDLzi51740848 = orqxkwDLzi16796775;     orqxkwDLzi16796775 = orqxkwDLzi27426642;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wwFTbdocAl6851646() {     int IPSrTYlkZE40212809 = -87230978;    int IPSrTYlkZE86766949 = 89507947;    int IPSrTYlkZE97889519 = -605261793;    int IPSrTYlkZE67476389 = -214503578;    int IPSrTYlkZE5974741 = -820391037;    int IPSrTYlkZE19852540 = -493249276;    int IPSrTYlkZE17663236 = -25946717;    int IPSrTYlkZE47568595 = 17399643;    int IPSrTYlkZE23760605 = -294576124;    int IPSrTYlkZE45422248 = -337769001;    int IPSrTYlkZE10112073 = -878855511;    int IPSrTYlkZE3962940 = -967853997;    int IPSrTYlkZE65644392 = -48306045;    int IPSrTYlkZE81997682 = -681109783;    int IPSrTYlkZE3748861 = -221376130;    int IPSrTYlkZE45940111 = -674419213;    int IPSrTYlkZE20246675 = -886918143;    int IPSrTYlkZE70183356 = -561026564;    int IPSrTYlkZE59727086 = -691969483;    int IPSrTYlkZE25873614 = -766558840;    int IPSrTYlkZE63242980 = -946821146;    int IPSrTYlkZE71225345 = -89186158;    int IPSrTYlkZE84125974 = 36894990;    int IPSrTYlkZE17914524 = -67359230;    int IPSrTYlkZE95394188 = -272344702;    int IPSrTYlkZE96193191 = -110230274;    int IPSrTYlkZE62518725 = -473225678;    int IPSrTYlkZE81210508 = 61535108;    int IPSrTYlkZE98708321 = -612401931;    int IPSrTYlkZE94926301 = -135877373;    int IPSrTYlkZE95601561 = -423281124;    int IPSrTYlkZE58291619 = -190866220;    int IPSrTYlkZE34639935 = -283548729;    int IPSrTYlkZE7494939 = -770972769;    int IPSrTYlkZE83099811 = -338755028;    int IPSrTYlkZE20119677 = -560534780;    int IPSrTYlkZE72513475 = 10352972;    int IPSrTYlkZE72220150 = -397598280;    int IPSrTYlkZE91730857 = 18880612;    int IPSrTYlkZE40912357 = -397603901;    int IPSrTYlkZE41884870 = -559466892;    int IPSrTYlkZE79837132 = -400553503;    int IPSrTYlkZE13251016 = -753684667;    int IPSrTYlkZE84407363 = -959482001;    int IPSrTYlkZE24457161 = -965908674;    int IPSrTYlkZE52302007 = -839880282;    int IPSrTYlkZE57195631 = -933770689;    int IPSrTYlkZE34783163 = -217372774;    int IPSrTYlkZE55037949 = -877181483;    int IPSrTYlkZE89754723 = -406216832;    int IPSrTYlkZE94094771 = 46671707;    int IPSrTYlkZE67002798 = 1772007;    int IPSrTYlkZE11305607 = -379446682;    int IPSrTYlkZE48118188 = -852987714;    int IPSrTYlkZE43608666 = -254175086;    int IPSrTYlkZE68987464 = -998044820;    int IPSrTYlkZE2640975 = -947387044;    int IPSrTYlkZE79974996 = -437902564;    int IPSrTYlkZE72082201 = -942158877;    int IPSrTYlkZE9781549 = -610160763;    int IPSrTYlkZE57333815 = 79976401;    int IPSrTYlkZE36452727 = 12518174;    int IPSrTYlkZE48860274 = -370198427;    int IPSrTYlkZE28834304 = -58698751;    int IPSrTYlkZE49820687 = -914487878;    int IPSrTYlkZE51820454 = -587989291;    int IPSrTYlkZE69323005 = -584305268;    int IPSrTYlkZE58149454 = -277333276;    int IPSrTYlkZE98897871 = -242354756;    int IPSrTYlkZE83629183 = -660841351;    int IPSrTYlkZE73426636 = -584772185;    int IPSrTYlkZE48026524 = -389319864;    int IPSrTYlkZE78452499 = -479907177;    int IPSrTYlkZE18814730 = -194365582;    int IPSrTYlkZE83988744 = -107091949;    int IPSrTYlkZE83405847 = -446267644;    int IPSrTYlkZE57974329 = -335501492;    int IPSrTYlkZE99718610 = -3623010;    int IPSrTYlkZE93457363 = -101450556;    int IPSrTYlkZE43092181 = -432464421;    int IPSrTYlkZE38997561 = -176459586;    int IPSrTYlkZE27735562 = -155852904;    int IPSrTYlkZE26172559 = -61283409;    int IPSrTYlkZE8953598 = -106185099;    int IPSrTYlkZE831530 = -82549080;    int IPSrTYlkZE28598763 = -325053131;    int IPSrTYlkZE46986013 = -811419539;    int IPSrTYlkZE86521746 = -430561016;    int IPSrTYlkZE63886273 = -416797683;    int IPSrTYlkZE14112347 = -340710208;    int IPSrTYlkZE17478702 = -613147736;    int IPSrTYlkZE92538478 = -551744465;    int IPSrTYlkZE137950 = -455439403;    int IPSrTYlkZE81949308 = -370958626;    int IPSrTYlkZE83578541 = -377580303;    int IPSrTYlkZE5432143 = -471985066;    int IPSrTYlkZE30976859 = 69644924;    int IPSrTYlkZE84416712 = -594985916;    int IPSrTYlkZE34586677 = 55005877;    int IPSrTYlkZE72636706 = -87230978;     IPSrTYlkZE40212809 = IPSrTYlkZE86766949;     IPSrTYlkZE86766949 = IPSrTYlkZE97889519;     IPSrTYlkZE97889519 = IPSrTYlkZE67476389;     IPSrTYlkZE67476389 = IPSrTYlkZE5974741;     IPSrTYlkZE5974741 = IPSrTYlkZE19852540;     IPSrTYlkZE19852540 = IPSrTYlkZE17663236;     IPSrTYlkZE17663236 = IPSrTYlkZE47568595;     IPSrTYlkZE47568595 = IPSrTYlkZE23760605;     IPSrTYlkZE23760605 = IPSrTYlkZE45422248;     IPSrTYlkZE45422248 = IPSrTYlkZE10112073;     IPSrTYlkZE10112073 = IPSrTYlkZE3962940;     IPSrTYlkZE3962940 = IPSrTYlkZE65644392;     IPSrTYlkZE65644392 = IPSrTYlkZE81997682;     IPSrTYlkZE81997682 = IPSrTYlkZE3748861;     IPSrTYlkZE3748861 = IPSrTYlkZE45940111;     IPSrTYlkZE45940111 = IPSrTYlkZE20246675;     IPSrTYlkZE20246675 = IPSrTYlkZE70183356;     IPSrTYlkZE70183356 = IPSrTYlkZE59727086;     IPSrTYlkZE59727086 = IPSrTYlkZE25873614;     IPSrTYlkZE25873614 = IPSrTYlkZE63242980;     IPSrTYlkZE63242980 = IPSrTYlkZE71225345;     IPSrTYlkZE71225345 = IPSrTYlkZE84125974;     IPSrTYlkZE84125974 = IPSrTYlkZE17914524;     IPSrTYlkZE17914524 = IPSrTYlkZE95394188;     IPSrTYlkZE95394188 = IPSrTYlkZE96193191;     IPSrTYlkZE96193191 = IPSrTYlkZE62518725;     IPSrTYlkZE62518725 = IPSrTYlkZE81210508;     IPSrTYlkZE81210508 = IPSrTYlkZE98708321;     IPSrTYlkZE98708321 = IPSrTYlkZE94926301;     IPSrTYlkZE94926301 = IPSrTYlkZE95601561;     IPSrTYlkZE95601561 = IPSrTYlkZE58291619;     IPSrTYlkZE58291619 = IPSrTYlkZE34639935;     IPSrTYlkZE34639935 = IPSrTYlkZE7494939;     IPSrTYlkZE7494939 = IPSrTYlkZE83099811;     IPSrTYlkZE83099811 = IPSrTYlkZE20119677;     IPSrTYlkZE20119677 = IPSrTYlkZE72513475;     IPSrTYlkZE72513475 = IPSrTYlkZE72220150;     IPSrTYlkZE72220150 = IPSrTYlkZE91730857;     IPSrTYlkZE91730857 = IPSrTYlkZE40912357;     IPSrTYlkZE40912357 = IPSrTYlkZE41884870;     IPSrTYlkZE41884870 = IPSrTYlkZE79837132;     IPSrTYlkZE79837132 = IPSrTYlkZE13251016;     IPSrTYlkZE13251016 = IPSrTYlkZE84407363;     IPSrTYlkZE84407363 = IPSrTYlkZE24457161;     IPSrTYlkZE24457161 = IPSrTYlkZE52302007;     IPSrTYlkZE52302007 = IPSrTYlkZE57195631;     IPSrTYlkZE57195631 = IPSrTYlkZE34783163;     IPSrTYlkZE34783163 = IPSrTYlkZE55037949;     IPSrTYlkZE55037949 = IPSrTYlkZE89754723;     IPSrTYlkZE89754723 = IPSrTYlkZE94094771;     IPSrTYlkZE94094771 = IPSrTYlkZE67002798;     IPSrTYlkZE67002798 = IPSrTYlkZE11305607;     IPSrTYlkZE11305607 = IPSrTYlkZE48118188;     IPSrTYlkZE48118188 = IPSrTYlkZE43608666;     IPSrTYlkZE43608666 = IPSrTYlkZE68987464;     IPSrTYlkZE68987464 = IPSrTYlkZE2640975;     IPSrTYlkZE2640975 = IPSrTYlkZE79974996;     IPSrTYlkZE79974996 = IPSrTYlkZE72082201;     IPSrTYlkZE72082201 = IPSrTYlkZE9781549;     IPSrTYlkZE9781549 = IPSrTYlkZE57333815;     IPSrTYlkZE57333815 = IPSrTYlkZE36452727;     IPSrTYlkZE36452727 = IPSrTYlkZE48860274;     IPSrTYlkZE48860274 = IPSrTYlkZE28834304;     IPSrTYlkZE28834304 = IPSrTYlkZE49820687;     IPSrTYlkZE49820687 = IPSrTYlkZE51820454;     IPSrTYlkZE51820454 = IPSrTYlkZE69323005;     IPSrTYlkZE69323005 = IPSrTYlkZE58149454;     IPSrTYlkZE58149454 = IPSrTYlkZE98897871;     IPSrTYlkZE98897871 = IPSrTYlkZE83629183;     IPSrTYlkZE83629183 = IPSrTYlkZE73426636;     IPSrTYlkZE73426636 = IPSrTYlkZE48026524;     IPSrTYlkZE48026524 = IPSrTYlkZE78452499;     IPSrTYlkZE78452499 = IPSrTYlkZE18814730;     IPSrTYlkZE18814730 = IPSrTYlkZE83988744;     IPSrTYlkZE83988744 = IPSrTYlkZE83405847;     IPSrTYlkZE83405847 = IPSrTYlkZE57974329;     IPSrTYlkZE57974329 = IPSrTYlkZE99718610;     IPSrTYlkZE99718610 = IPSrTYlkZE93457363;     IPSrTYlkZE93457363 = IPSrTYlkZE43092181;     IPSrTYlkZE43092181 = IPSrTYlkZE38997561;     IPSrTYlkZE38997561 = IPSrTYlkZE27735562;     IPSrTYlkZE27735562 = IPSrTYlkZE26172559;     IPSrTYlkZE26172559 = IPSrTYlkZE8953598;     IPSrTYlkZE8953598 = IPSrTYlkZE831530;     IPSrTYlkZE831530 = IPSrTYlkZE28598763;     IPSrTYlkZE28598763 = IPSrTYlkZE46986013;     IPSrTYlkZE46986013 = IPSrTYlkZE86521746;     IPSrTYlkZE86521746 = IPSrTYlkZE63886273;     IPSrTYlkZE63886273 = IPSrTYlkZE14112347;     IPSrTYlkZE14112347 = IPSrTYlkZE17478702;     IPSrTYlkZE17478702 = IPSrTYlkZE92538478;     IPSrTYlkZE92538478 = IPSrTYlkZE137950;     IPSrTYlkZE137950 = IPSrTYlkZE81949308;     IPSrTYlkZE81949308 = IPSrTYlkZE83578541;     IPSrTYlkZE83578541 = IPSrTYlkZE5432143;     IPSrTYlkZE5432143 = IPSrTYlkZE30976859;     IPSrTYlkZE30976859 = IPSrTYlkZE84416712;     IPSrTYlkZE84416712 = IPSrTYlkZE34586677;     IPSrTYlkZE34586677 = IPSrTYlkZE72636706;     IPSrTYlkZE72636706 = IPSrTYlkZE40212809;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lNGJLgVMDy25064987() {     int JDQbIExOBb23657115 = -967896659;    int JDQbIExOBb3639039 = 90125759;    int JDQbIExOBb94298026 = -398787021;    int JDQbIExOBb30268641 = -408303414;    int JDQbIExOBb12096065 = -627258528;    int JDQbIExOBb18356941 = -272320929;    int JDQbIExOBb88702553 = -865858031;    int JDQbIExOBb61916038 = 30241613;    int JDQbIExOBb82290305 = -636035475;    int JDQbIExOBb77967899 = -806145114;    int JDQbIExOBb59528011 = 31028616;    int JDQbIExOBb67168338 = -955472243;    int JDQbIExOBb83762167 = -939967286;    int JDQbIExOBb61624575 = -469400140;    int JDQbIExOBb64603324 = -315399112;    int JDQbIExOBb32329272 = -462498539;    int JDQbIExOBb13998560 = -640263170;    int JDQbIExOBb51034803 = -808331440;    int JDQbIExOBb22167229 = -690644354;    int JDQbIExOBb15241057 = -196144832;    int JDQbIExOBb18883367 = -536350177;    int JDQbIExOBb83372663 = -80337929;    int JDQbIExOBb87951682 = -2994207;    int JDQbIExOBb52621082 = -139184626;    int JDQbIExOBb36970146 = -831997461;    int JDQbIExOBb91535340 = 9260010;    int JDQbIExOBb11973465 = -101310020;    int JDQbIExOBb20942134 = -8419776;    int JDQbIExOBb34789415 = -474907073;    int JDQbIExOBb10759643 = -648727225;    int JDQbIExOBb13696875 = -61038325;    int JDQbIExOBb21398046 = -106061300;    int JDQbIExOBb78178821 = 95579093;    int JDQbIExOBb49653449 = -362574939;    int JDQbIExOBb61826541 = -536419763;    int JDQbIExOBb32508861 = -348558686;    int JDQbIExOBb41127653 = -968496208;    int JDQbIExOBb12798983 = -106068800;    int JDQbIExOBb40725935 = -166017186;    int JDQbIExOBb90407491 = -150197424;    int JDQbIExOBb75533794 = -131015865;    int JDQbIExOBb30324919 = -974461824;    int JDQbIExOBb36818086 = -204741909;    int JDQbIExOBb76312303 = -715664771;    int JDQbIExOBb16401657 = -694065627;    int JDQbIExOBb91908002 = -859491205;    int JDQbIExOBb33211061 = 33925082;    int JDQbIExOBb54453759 = -216094493;    int JDQbIExOBb85885061 = -362783566;    int JDQbIExOBb25087207 = -613237287;    int JDQbIExOBb67352220 = -174229716;    int JDQbIExOBb22503593 = -56487304;    int JDQbIExOBb25418065 = -591193998;    int JDQbIExOBb72794947 = -571401101;    int JDQbIExOBb6181620 = -602661209;    int JDQbIExOBb40284452 = -787558731;    int JDQbIExOBb15687357 = -906880034;    int JDQbIExOBb41676945 = -159602396;    int JDQbIExOBb93298494 = -576305954;    int JDQbIExOBb20560724 = -536518538;    int JDQbIExOBb6383476 = -71010909;    int JDQbIExOBb67760420 = -757438255;    int JDQbIExOBb27126623 = -494851315;    int JDQbIExOBb71530663 = -987308251;    int JDQbIExOBb64271025 = -645106789;    int JDQbIExOBb38129965 = -862910084;    int JDQbIExOBb88989517 = -951051337;    int JDQbIExOBb34108719 = -477392347;    int JDQbIExOBb99798033 = -932980378;    int JDQbIExOBb32094463 = -966840427;    int JDQbIExOBb91201618 = -494002331;    int JDQbIExOBb1199578 = -434194371;    int JDQbIExOBb10308868 = -542314255;    int JDQbIExOBb31759737 = -440446931;    int JDQbIExOBb39707263 = 34871032;    int JDQbIExOBb88558448 = -561888353;    int JDQbIExOBb46554577 = -875596020;    int JDQbIExOBb11639379 = -287329436;    int JDQbIExOBb36219425 = -445118999;    int JDQbIExOBb45062144 = -972506256;    int JDQbIExOBb58324279 = 75334928;    int JDQbIExOBb57519706 = -885215528;    int JDQbIExOBb35057072 = -645636211;    int JDQbIExOBb9702208 = -861669786;    int JDQbIExOBb43407423 = -374497510;    int JDQbIExOBb91193282 = 95448979;    int JDQbIExOBb95979980 = -514867303;    int JDQbIExOBb5383875 = -333019806;    int JDQbIExOBb43471829 = -759913730;    int JDQbIExOBb21542090 = -748861033;    int JDQbIExOBb16821505 = -441678652;    int JDQbIExOBb99450708 = -708893813;    int JDQbIExOBb19500488 = -529762846;    int JDQbIExOBb20165212 = -629498648;    int JDQbIExOBb84024016 = 20813485;    int JDQbIExOBb7773375 = -373577610;    int JDQbIExOBb3198296 = -379610510;    int JDQbIExOBb65287422 = -217433659;    int JDQbIExOBb12041279 = 29442018;    int JDQbIExOBb78271692 = -967896659;     JDQbIExOBb23657115 = JDQbIExOBb3639039;     JDQbIExOBb3639039 = JDQbIExOBb94298026;     JDQbIExOBb94298026 = JDQbIExOBb30268641;     JDQbIExOBb30268641 = JDQbIExOBb12096065;     JDQbIExOBb12096065 = JDQbIExOBb18356941;     JDQbIExOBb18356941 = JDQbIExOBb88702553;     JDQbIExOBb88702553 = JDQbIExOBb61916038;     JDQbIExOBb61916038 = JDQbIExOBb82290305;     JDQbIExOBb82290305 = JDQbIExOBb77967899;     JDQbIExOBb77967899 = JDQbIExOBb59528011;     JDQbIExOBb59528011 = JDQbIExOBb67168338;     JDQbIExOBb67168338 = JDQbIExOBb83762167;     JDQbIExOBb83762167 = JDQbIExOBb61624575;     JDQbIExOBb61624575 = JDQbIExOBb64603324;     JDQbIExOBb64603324 = JDQbIExOBb32329272;     JDQbIExOBb32329272 = JDQbIExOBb13998560;     JDQbIExOBb13998560 = JDQbIExOBb51034803;     JDQbIExOBb51034803 = JDQbIExOBb22167229;     JDQbIExOBb22167229 = JDQbIExOBb15241057;     JDQbIExOBb15241057 = JDQbIExOBb18883367;     JDQbIExOBb18883367 = JDQbIExOBb83372663;     JDQbIExOBb83372663 = JDQbIExOBb87951682;     JDQbIExOBb87951682 = JDQbIExOBb52621082;     JDQbIExOBb52621082 = JDQbIExOBb36970146;     JDQbIExOBb36970146 = JDQbIExOBb91535340;     JDQbIExOBb91535340 = JDQbIExOBb11973465;     JDQbIExOBb11973465 = JDQbIExOBb20942134;     JDQbIExOBb20942134 = JDQbIExOBb34789415;     JDQbIExOBb34789415 = JDQbIExOBb10759643;     JDQbIExOBb10759643 = JDQbIExOBb13696875;     JDQbIExOBb13696875 = JDQbIExOBb21398046;     JDQbIExOBb21398046 = JDQbIExOBb78178821;     JDQbIExOBb78178821 = JDQbIExOBb49653449;     JDQbIExOBb49653449 = JDQbIExOBb61826541;     JDQbIExOBb61826541 = JDQbIExOBb32508861;     JDQbIExOBb32508861 = JDQbIExOBb41127653;     JDQbIExOBb41127653 = JDQbIExOBb12798983;     JDQbIExOBb12798983 = JDQbIExOBb40725935;     JDQbIExOBb40725935 = JDQbIExOBb90407491;     JDQbIExOBb90407491 = JDQbIExOBb75533794;     JDQbIExOBb75533794 = JDQbIExOBb30324919;     JDQbIExOBb30324919 = JDQbIExOBb36818086;     JDQbIExOBb36818086 = JDQbIExOBb76312303;     JDQbIExOBb76312303 = JDQbIExOBb16401657;     JDQbIExOBb16401657 = JDQbIExOBb91908002;     JDQbIExOBb91908002 = JDQbIExOBb33211061;     JDQbIExOBb33211061 = JDQbIExOBb54453759;     JDQbIExOBb54453759 = JDQbIExOBb85885061;     JDQbIExOBb85885061 = JDQbIExOBb25087207;     JDQbIExOBb25087207 = JDQbIExOBb67352220;     JDQbIExOBb67352220 = JDQbIExOBb22503593;     JDQbIExOBb22503593 = JDQbIExOBb25418065;     JDQbIExOBb25418065 = JDQbIExOBb72794947;     JDQbIExOBb72794947 = JDQbIExOBb6181620;     JDQbIExOBb6181620 = JDQbIExOBb40284452;     JDQbIExOBb40284452 = JDQbIExOBb15687357;     JDQbIExOBb15687357 = JDQbIExOBb41676945;     JDQbIExOBb41676945 = JDQbIExOBb93298494;     JDQbIExOBb93298494 = JDQbIExOBb20560724;     JDQbIExOBb20560724 = JDQbIExOBb6383476;     JDQbIExOBb6383476 = JDQbIExOBb67760420;     JDQbIExOBb67760420 = JDQbIExOBb27126623;     JDQbIExOBb27126623 = JDQbIExOBb71530663;     JDQbIExOBb71530663 = JDQbIExOBb64271025;     JDQbIExOBb64271025 = JDQbIExOBb38129965;     JDQbIExOBb38129965 = JDQbIExOBb88989517;     JDQbIExOBb88989517 = JDQbIExOBb34108719;     JDQbIExOBb34108719 = JDQbIExOBb99798033;     JDQbIExOBb99798033 = JDQbIExOBb32094463;     JDQbIExOBb32094463 = JDQbIExOBb91201618;     JDQbIExOBb91201618 = JDQbIExOBb1199578;     JDQbIExOBb1199578 = JDQbIExOBb10308868;     JDQbIExOBb10308868 = JDQbIExOBb31759737;     JDQbIExOBb31759737 = JDQbIExOBb39707263;     JDQbIExOBb39707263 = JDQbIExOBb88558448;     JDQbIExOBb88558448 = JDQbIExOBb46554577;     JDQbIExOBb46554577 = JDQbIExOBb11639379;     JDQbIExOBb11639379 = JDQbIExOBb36219425;     JDQbIExOBb36219425 = JDQbIExOBb45062144;     JDQbIExOBb45062144 = JDQbIExOBb58324279;     JDQbIExOBb58324279 = JDQbIExOBb57519706;     JDQbIExOBb57519706 = JDQbIExOBb35057072;     JDQbIExOBb35057072 = JDQbIExOBb9702208;     JDQbIExOBb9702208 = JDQbIExOBb43407423;     JDQbIExOBb43407423 = JDQbIExOBb91193282;     JDQbIExOBb91193282 = JDQbIExOBb95979980;     JDQbIExOBb95979980 = JDQbIExOBb5383875;     JDQbIExOBb5383875 = JDQbIExOBb43471829;     JDQbIExOBb43471829 = JDQbIExOBb21542090;     JDQbIExOBb21542090 = JDQbIExOBb16821505;     JDQbIExOBb16821505 = JDQbIExOBb99450708;     JDQbIExOBb99450708 = JDQbIExOBb19500488;     JDQbIExOBb19500488 = JDQbIExOBb20165212;     JDQbIExOBb20165212 = JDQbIExOBb84024016;     JDQbIExOBb84024016 = JDQbIExOBb7773375;     JDQbIExOBb7773375 = JDQbIExOBb3198296;     JDQbIExOBb3198296 = JDQbIExOBb65287422;     JDQbIExOBb65287422 = JDQbIExOBb12041279;     JDQbIExOBb12041279 = JDQbIExOBb78271692;     JDQbIExOBb78271692 = JDQbIExOBb23657115;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GcZvrwgUdT25507090() {     int GyUvZZnKju58836870 = -864817172;    int GyUvZZnKju36185110 = 13843816;    int GyUvZZnKju39506893 = 10328367;    int GyUvZZnKju68136294 = -418372392;    int GyUvZZnKju94941320 = -141776673;    int GyUvZZnKju41744696 = -210419809;    int GyUvZZnKju33518714 = -428980940;    int GyUvZZnKju95054958 = -80695971;    int GyUvZZnKju92118083 = -768343536;    int GyUvZZnKju79310675 = -716167208;    int GyUvZZnKju4236648 = -120809728;    int GyUvZZnKju40730115 = -878071990;    int GyUvZZnKju89060737 = -340808788;    int GyUvZZnKju19186 = -984371044;    int GyUvZZnKju79473045 = -987149804;    int GyUvZZnKju30445706 = 37738180;    int GyUvZZnKju63720969 = -761702887;    int GyUvZZnKju17569172 = -418212812;    int GyUvZZnKju44181220 = -117233408;    int GyUvZZnKju24338074 = -435998974;    int GyUvZZnKju20271107 = -208687376;    int GyUvZZnKju97939434 = -244970707;    int GyUvZZnKju93126904 = -751120852;    int GyUvZZnKju79977890 = -82601973;    int GyUvZZnKju49961963 = -237304656;    int GyUvZZnKju44582903 = -216859485;    int GyUvZZnKju256840 = -191803081;    int GyUvZZnKju58802983 = -461705346;    int GyUvZZnKju45620582 = -387311117;    int GyUvZZnKju96578067 = -523477872;    int GyUvZZnKju69010997 = -31677992;    int GyUvZZnKju97534203 = -345030572;    int GyUvZZnKju19811018 = -107674970;    int GyUvZZnKju85835671 = -714774275;    int GyUvZZnKju38758400 = -950894141;    int GyUvZZnKju26597656 = -372751420;    int GyUvZZnKju59390058 = -391700132;    int GyUvZZnKju54211427 = -998051684;    int GyUvZZnKju59495856 = -796232378;    int GyUvZZnKju64985043 = -521406612;    int GyUvZZnKju77866805 = -921723626;    int GyUvZZnKju40838105 = -436889767;    int GyUvZZnKju16968388 = -656575684;    int GyUvZZnKju61153062 = -397236220;    int GyUvZZnKju80375361 = 77280336;    int GyUvZZnKju33208446 = -202994069;    int GyUvZZnKju95392685 = -560719602;    int GyUvZZnKju53108819 = -619220907;    int GyUvZZnKju5750627 = -912338377;    int GyUvZZnKju49075950 = -679269123;    int GyUvZZnKju81289560 = -103635294;    int GyUvZZnKju9984140 = -576346176;    int GyUvZZnKju62769270 = -547054503;    int GyUvZZnKju67530946 = 95386199;    int GyUvZZnKju5235984 = -980910533;    int GyUvZZnKju60897436 = -519846466;    int GyUvZZnKju43058206 = -235035333;    int GyUvZZnKju59529003 = -907069661;    int GyUvZZnKju18174332 = -81067737;    int GyUvZZnKju50358418 = -924917188;    int GyUvZZnKju41487856 = 81383271;    int GyUvZZnKju74715730 = -967275595;    int GyUvZZnKju49434376 = -693384854;    int GyUvZZnKju95540015 = -144865665;    int GyUvZZnKju10299679 = -584489217;    int GyUvZZnKju6702445 = -775779157;    int GyUvZZnKju20919098 = -670397020;    int GyUvZZnKju3225066 = -626034514;    int GyUvZZnKju61260785 = 66523097;    int GyUvZZnKju52875390 = -514398385;    int GyUvZZnKju71055648 = -570561688;    int GyUvZZnKju9509543 = -763651203;    int GyUvZZnKju58073316 = -621980434;    int GyUvZZnKju79196176 = -595826796;    int GyUvZZnKju46471268 = -514275349;    int GyUvZZnKju79433002 = -771797609;    int GyUvZZnKju80971047 = -588395023;    int GyUvZZnKju31973842 = -253884632;    int GyUvZZnKju99602528 = -59882310;    int GyUvZZnKju16753517 = 65689413;    int GyUvZZnKju49190218 = -656139884;    int GyUvZZnKju47148021 = -572582174;    int GyUvZZnKju53052357 = -549366969;    int GyUvZZnKju96544631 = -708041995;    int GyUvZZnKju15288508 = -319842579;    int GyUvZZnKju59026858 = -455331816;    int GyUvZZnKju34764933 = -797976070;    int GyUvZZnKju52280071 = -103061170;    int GyUvZZnKju80599688 = -733863742;    int GyUvZZnKju77860964 = -331047676;    int GyUvZZnKju83539449 = -37716088;    int GyUvZZnKju99861055 = -484630472;    int GyUvZZnKju36037096 = -816983948;    int GyUvZZnKju9137439 = -871315190;    int GyUvZZnKju23497187 = -502789884;    int GyUvZZnKju3151075 = -954448031;    int GyUvZZnKju91403728 = -743504914;    int GyUvZZnKju21428372 = -411710019;    int GyUvZZnKju50853384 = -812747004;    int GyUvZZnKju73672917 = -864817172;     GyUvZZnKju58836870 = GyUvZZnKju36185110;     GyUvZZnKju36185110 = GyUvZZnKju39506893;     GyUvZZnKju39506893 = GyUvZZnKju68136294;     GyUvZZnKju68136294 = GyUvZZnKju94941320;     GyUvZZnKju94941320 = GyUvZZnKju41744696;     GyUvZZnKju41744696 = GyUvZZnKju33518714;     GyUvZZnKju33518714 = GyUvZZnKju95054958;     GyUvZZnKju95054958 = GyUvZZnKju92118083;     GyUvZZnKju92118083 = GyUvZZnKju79310675;     GyUvZZnKju79310675 = GyUvZZnKju4236648;     GyUvZZnKju4236648 = GyUvZZnKju40730115;     GyUvZZnKju40730115 = GyUvZZnKju89060737;     GyUvZZnKju89060737 = GyUvZZnKju19186;     GyUvZZnKju19186 = GyUvZZnKju79473045;     GyUvZZnKju79473045 = GyUvZZnKju30445706;     GyUvZZnKju30445706 = GyUvZZnKju63720969;     GyUvZZnKju63720969 = GyUvZZnKju17569172;     GyUvZZnKju17569172 = GyUvZZnKju44181220;     GyUvZZnKju44181220 = GyUvZZnKju24338074;     GyUvZZnKju24338074 = GyUvZZnKju20271107;     GyUvZZnKju20271107 = GyUvZZnKju97939434;     GyUvZZnKju97939434 = GyUvZZnKju93126904;     GyUvZZnKju93126904 = GyUvZZnKju79977890;     GyUvZZnKju79977890 = GyUvZZnKju49961963;     GyUvZZnKju49961963 = GyUvZZnKju44582903;     GyUvZZnKju44582903 = GyUvZZnKju256840;     GyUvZZnKju256840 = GyUvZZnKju58802983;     GyUvZZnKju58802983 = GyUvZZnKju45620582;     GyUvZZnKju45620582 = GyUvZZnKju96578067;     GyUvZZnKju96578067 = GyUvZZnKju69010997;     GyUvZZnKju69010997 = GyUvZZnKju97534203;     GyUvZZnKju97534203 = GyUvZZnKju19811018;     GyUvZZnKju19811018 = GyUvZZnKju85835671;     GyUvZZnKju85835671 = GyUvZZnKju38758400;     GyUvZZnKju38758400 = GyUvZZnKju26597656;     GyUvZZnKju26597656 = GyUvZZnKju59390058;     GyUvZZnKju59390058 = GyUvZZnKju54211427;     GyUvZZnKju54211427 = GyUvZZnKju59495856;     GyUvZZnKju59495856 = GyUvZZnKju64985043;     GyUvZZnKju64985043 = GyUvZZnKju77866805;     GyUvZZnKju77866805 = GyUvZZnKju40838105;     GyUvZZnKju40838105 = GyUvZZnKju16968388;     GyUvZZnKju16968388 = GyUvZZnKju61153062;     GyUvZZnKju61153062 = GyUvZZnKju80375361;     GyUvZZnKju80375361 = GyUvZZnKju33208446;     GyUvZZnKju33208446 = GyUvZZnKju95392685;     GyUvZZnKju95392685 = GyUvZZnKju53108819;     GyUvZZnKju53108819 = GyUvZZnKju5750627;     GyUvZZnKju5750627 = GyUvZZnKju49075950;     GyUvZZnKju49075950 = GyUvZZnKju81289560;     GyUvZZnKju81289560 = GyUvZZnKju9984140;     GyUvZZnKju9984140 = GyUvZZnKju62769270;     GyUvZZnKju62769270 = GyUvZZnKju67530946;     GyUvZZnKju67530946 = GyUvZZnKju5235984;     GyUvZZnKju5235984 = GyUvZZnKju60897436;     GyUvZZnKju60897436 = GyUvZZnKju43058206;     GyUvZZnKju43058206 = GyUvZZnKju59529003;     GyUvZZnKju59529003 = GyUvZZnKju18174332;     GyUvZZnKju18174332 = GyUvZZnKju50358418;     GyUvZZnKju50358418 = GyUvZZnKju41487856;     GyUvZZnKju41487856 = GyUvZZnKju74715730;     GyUvZZnKju74715730 = GyUvZZnKju49434376;     GyUvZZnKju49434376 = GyUvZZnKju95540015;     GyUvZZnKju95540015 = GyUvZZnKju10299679;     GyUvZZnKju10299679 = GyUvZZnKju6702445;     GyUvZZnKju6702445 = GyUvZZnKju20919098;     GyUvZZnKju20919098 = GyUvZZnKju3225066;     GyUvZZnKju3225066 = GyUvZZnKju61260785;     GyUvZZnKju61260785 = GyUvZZnKju52875390;     GyUvZZnKju52875390 = GyUvZZnKju71055648;     GyUvZZnKju71055648 = GyUvZZnKju9509543;     GyUvZZnKju9509543 = GyUvZZnKju58073316;     GyUvZZnKju58073316 = GyUvZZnKju79196176;     GyUvZZnKju79196176 = GyUvZZnKju46471268;     GyUvZZnKju46471268 = GyUvZZnKju79433002;     GyUvZZnKju79433002 = GyUvZZnKju80971047;     GyUvZZnKju80971047 = GyUvZZnKju31973842;     GyUvZZnKju31973842 = GyUvZZnKju99602528;     GyUvZZnKju99602528 = GyUvZZnKju16753517;     GyUvZZnKju16753517 = GyUvZZnKju49190218;     GyUvZZnKju49190218 = GyUvZZnKju47148021;     GyUvZZnKju47148021 = GyUvZZnKju53052357;     GyUvZZnKju53052357 = GyUvZZnKju96544631;     GyUvZZnKju96544631 = GyUvZZnKju15288508;     GyUvZZnKju15288508 = GyUvZZnKju59026858;     GyUvZZnKju59026858 = GyUvZZnKju34764933;     GyUvZZnKju34764933 = GyUvZZnKju52280071;     GyUvZZnKju52280071 = GyUvZZnKju80599688;     GyUvZZnKju80599688 = GyUvZZnKju77860964;     GyUvZZnKju77860964 = GyUvZZnKju83539449;     GyUvZZnKju83539449 = GyUvZZnKju99861055;     GyUvZZnKju99861055 = GyUvZZnKju36037096;     GyUvZZnKju36037096 = GyUvZZnKju9137439;     GyUvZZnKju9137439 = GyUvZZnKju23497187;     GyUvZZnKju23497187 = GyUvZZnKju3151075;     GyUvZZnKju3151075 = GyUvZZnKju91403728;     GyUvZZnKju91403728 = GyUvZZnKju21428372;     GyUvZZnKju21428372 = GyUvZZnKju50853384;     GyUvZZnKju50853384 = GyUvZZnKju73672917;     GyUvZZnKju73672917 = GyUvZZnKju58836870;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sFJjoDLUkI95962962() {     int OxBcriPpfp71623037 = -999857864;    int OxBcriPpfp22955999 = -156490197;    int OxBcriPpfp57259693 = -398031015;    int OxBcriPpfp76077119 = -242574669;    int OxBcriPpfp88452425 = -633593138;    int OxBcriPpfp85054766 = 12656878;    int OxBcriPpfp54726472 = -974653011;    int OxBcriPpfp21774071 = -525214227;    int OxBcriPpfp87522893 = -730101629;    int OxBcriPpfp85529380 = -520254841;    int OxBcriPpfp31724030 = -235495743;    int OxBcriPpfp72680620 = -285404806;    int OxBcriPpfp59204040 = -244591445;    int OxBcriPpfp49643264 = -216786242;    int OxBcriPpfp2779598 = -894135635;    int OxBcriPpfp58870775 = -513153209;    int OxBcriPpfp19472787 = -425477354;    int OxBcriPpfp1127657 = 26576886;    int OxBcriPpfp45341850 = -26702603;    int OxBcriPpfp26992570 = 64756887;    int OxBcriPpfp445164 = -700003641;    int OxBcriPpfp26987254 = -414829055;    int OxBcriPpfp31887200 = -682124746;    int OxBcriPpfp24485020 = -442946985;    int OxBcriPpfp55856222 = -447746581;    int OxBcriPpfp25901549 = 92709316;    int OxBcriPpfp11672377 = -825509361;    int OxBcriPpfp26155909 = -635503451;    int OxBcriPpfp62237635 = -601528357;    int OxBcriPpfp49329183 = -153364027;    int OxBcriPpfp13446994 = -308094742;    int OxBcriPpfp853027 = -323750250;    int OxBcriPpfp65856967 = 16470245;    int OxBcriPpfp62912044 = -619528493;    int OxBcriPpfp21818932 = -460926022;    int OxBcriPpfp21821770 = -161314524;    int OxBcriPpfp96154118 = -181609483;    int OxBcriPpfp11915450 = -917671776;    int OxBcriPpfp65271242 = -638082956;    int OxBcriPpfp91249772 = -365823458;    int OxBcriPpfp39298114 = -197197619;    int OxBcriPpfp48540561 = -335201102;    int OxBcriPpfp51851857 = -680009129;    int OxBcriPpfp5781601 = -738408463;    int OxBcriPpfp60383852 = -915489624;    int OxBcriPpfp6836576 = -276312509;    int OxBcriPpfp72974705 = 41765346;    int OxBcriPpfp82548785 = -774573686;    int OxBcriPpfp49814676 = 24059648;    int OxBcriPpfp96026371 = -190305586;    int OxBcriPpfp99617053 = -356519602;    int OxBcriPpfp38160687 = 52468486;    int OxBcriPpfp97254420 = -723057785;    int OxBcriPpfp12230672 = -594065543;    int OxBcriPpfp95517799 = -975355019;    int OxBcriPpfp44635784 = -485028810;    int OxBcriPpfp91068798 = -474365452;    int OxBcriPpfp32774673 = -955084030;    int OxBcriPpfp20220898 = -794828089;    int OxBcriPpfp62550877 = -626302454;    int OxBcriPpfp73382390 = -161833762;    int OxBcriPpfp28570564 = -239149561;    int OxBcriPpfp59536436 = -923685871;    int OxBcriPpfp38193711 = -476737602;    int OxBcriPpfp72082387 = -112160099;    int OxBcriPpfp30871003 = -911745494;    int OxBcriPpfp6823654 = -201875052;    int OxBcriPpfp96291995 = -625062953;    int OxBcriPpfp27824332 = -755860221;    int OxBcriPpfp80957827 = -632821111;    int OxBcriPpfp62716656 = -231543727;    int OxBcriPpfp7557338 = -507805579;    int OxBcriPpfp35856415 = -335340159;    int OxBcriPpfp54092077 = -660879145;    int OxBcriPpfp87694456 = -738045494;    int OxBcriPpfp51904603 = -264802539;    int OxBcriPpfp75135396 = -734819927;    int OxBcriPpfp26105599 = -943716284;    int OxBcriPpfp64101168 = -527457362;    int OxBcriPpfp49019647 = -71434072;    int OxBcriPpfp52926844 = -949056030;    int OxBcriPpfp29123591 = 49064325;    int OxBcriPpfp76341232 = -559563099;    int OxBcriPpfp66211264 = -311222772;    int OxBcriPpfp49712129 = -796844425;    int OxBcriPpfp75286306 = -260563228;    int OxBcriPpfp3598607 = -600692466;    int OxBcriPpfp53626295 = -389464212;    int OxBcriPpfp67394245 = -644173474;    int OxBcriPpfp77183148 = -975897212;    int OxBcriPpfp30752972 = -686949073;    int OxBcriPpfp63379445 = -226525453;    int OxBcriPpfp91694552 = -22843687;    int OxBcriPpfp2720365 = 88219498;    int OxBcriPpfp17867383 = -103989697;    int OxBcriPpfp10727550 = -958048059;    int OxBcriPpfp89004124 = -411515232;    int OxBcriPpfp13658147 = -103271527;    int OxBcriPpfp33699213 = -526248364;    int OxBcriPpfp29512849 = -999857864;     OxBcriPpfp71623037 = OxBcriPpfp22955999;     OxBcriPpfp22955999 = OxBcriPpfp57259693;     OxBcriPpfp57259693 = OxBcriPpfp76077119;     OxBcriPpfp76077119 = OxBcriPpfp88452425;     OxBcriPpfp88452425 = OxBcriPpfp85054766;     OxBcriPpfp85054766 = OxBcriPpfp54726472;     OxBcriPpfp54726472 = OxBcriPpfp21774071;     OxBcriPpfp21774071 = OxBcriPpfp87522893;     OxBcriPpfp87522893 = OxBcriPpfp85529380;     OxBcriPpfp85529380 = OxBcriPpfp31724030;     OxBcriPpfp31724030 = OxBcriPpfp72680620;     OxBcriPpfp72680620 = OxBcriPpfp59204040;     OxBcriPpfp59204040 = OxBcriPpfp49643264;     OxBcriPpfp49643264 = OxBcriPpfp2779598;     OxBcriPpfp2779598 = OxBcriPpfp58870775;     OxBcriPpfp58870775 = OxBcriPpfp19472787;     OxBcriPpfp19472787 = OxBcriPpfp1127657;     OxBcriPpfp1127657 = OxBcriPpfp45341850;     OxBcriPpfp45341850 = OxBcriPpfp26992570;     OxBcriPpfp26992570 = OxBcriPpfp445164;     OxBcriPpfp445164 = OxBcriPpfp26987254;     OxBcriPpfp26987254 = OxBcriPpfp31887200;     OxBcriPpfp31887200 = OxBcriPpfp24485020;     OxBcriPpfp24485020 = OxBcriPpfp55856222;     OxBcriPpfp55856222 = OxBcriPpfp25901549;     OxBcriPpfp25901549 = OxBcriPpfp11672377;     OxBcriPpfp11672377 = OxBcriPpfp26155909;     OxBcriPpfp26155909 = OxBcriPpfp62237635;     OxBcriPpfp62237635 = OxBcriPpfp49329183;     OxBcriPpfp49329183 = OxBcriPpfp13446994;     OxBcriPpfp13446994 = OxBcriPpfp853027;     OxBcriPpfp853027 = OxBcriPpfp65856967;     OxBcriPpfp65856967 = OxBcriPpfp62912044;     OxBcriPpfp62912044 = OxBcriPpfp21818932;     OxBcriPpfp21818932 = OxBcriPpfp21821770;     OxBcriPpfp21821770 = OxBcriPpfp96154118;     OxBcriPpfp96154118 = OxBcriPpfp11915450;     OxBcriPpfp11915450 = OxBcriPpfp65271242;     OxBcriPpfp65271242 = OxBcriPpfp91249772;     OxBcriPpfp91249772 = OxBcriPpfp39298114;     OxBcriPpfp39298114 = OxBcriPpfp48540561;     OxBcriPpfp48540561 = OxBcriPpfp51851857;     OxBcriPpfp51851857 = OxBcriPpfp5781601;     OxBcriPpfp5781601 = OxBcriPpfp60383852;     OxBcriPpfp60383852 = OxBcriPpfp6836576;     OxBcriPpfp6836576 = OxBcriPpfp72974705;     OxBcriPpfp72974705 = OxBcriPpfp82548785;     OxBcriPpfp82548785 = OxBcriPpfp49814676;     OxBcriPpfp49814676 = OxBcriPpfp96026371;     OxBcriPpfp96026371 = OxBcriPpfp99617053;     OxBcriPpfp99617053 = OxBcriPpfp38160687;     OxBcriPpfp38160687 = OxBcriPpfp97254420;     OxBcriPpfp97254420 = OxBcriPpfp12230672;     OxBcriPpfp12230672 = OxBcriPpfp95517799;     OxBcriPpfp95517799 = OxBcriPpfp44635784;     OxBcriPpfp44635784 = OxBcriPpfp91068798;     OxBcriPpfp91068798 = OxBcriPpfp32774673;     OxBcriPpfp32774673 = OxBcriPpfp20220898;     OxBcriPpfp20220898 = OxBcriPpfp62550877;     OxBcriPpfp62550877 = OxBcriPpfp73382390;     OxBcriPpfp73382390 = OxBcriPpfp28570564;     OxBcriPpfp28570564 = OxBcriPpfp59536436;     OxBcriPpfp59536436 = OxBcriPpfp38193711;     OxBcriPpfp38193711 = OxBcriPpfp72082387;     OxBcriPpfp72082387 = OxBcriPpfp30871003;     OxBcriPpfp30871003 = OxBcriPpfp6823654;     OxBcriPpfp6823654 = OxBcriPpfp96291995;     OxBcriPpfp96291995 = OxBcriPpfp27824332;     OxBcriPpfp27824332 = OxBcriPpfp80957827;     OxBcriPpfp80957827 = OxBcriPpfp62716656;     OxBcriPpfp62716656 = OxBcriPpfp7557338;     OxBcriPpfp7557338 = OxBcriPpfp35856415;     OxBcriPpfp35856415 = OxBcriPpfp54092077;     OxBcriPpfp54092077 = OxBcriPpfp87694456;     OxBcriPpfp87694456 = OxBcriPpfp51904603;     OxBcriPpfp51904603 = OxBcriPpfp75135396;     OxBcriPpfp75135396 = OxBcriPpfp26105599;     OxBcriPpfp26105599 = OxBcriPpfp64101168;     OxBcriPpfp64101168 = OxBcriPpfp49019647;     OxBcriPpfp49019647 = OxBcriPpfp52926844;     OxBcriPpfp52926844 = OxBcriPpfp29123591;     OxBcriPpfp29123591 = OxBcriPpfp76341232;     OxBcriPpfp76341232 = OxBcriPpfp66211264;     OxBcriPpfp66211264 = OxBcriPpfp49712129;     OxBcriPpfp49712129 = OxBcriPpfp75286306;     OxBcriPpfp75286306 = OxBcriPpfp3598607;     OxBcriPpfp3598607 = OxBcriPpfp53626295;     OxBcriPpfp53626295 = OxBcriPpfp67394245;     OxBcriPpfp67394245 = OxBcriPpfp77183148;     OxBcriPpfp77183148 = OxBcriPpfp30752972;     OxBcriPpfp30752972 = OxBcriPpfp63379445;     OxBcriPpfp63379445 = OxBcriPpfp91694552;     OxBcriPpfp91694552 = OxBcriPpfp2720365;     OxBcriPpfp2720365 = OxBcriPpfp17867383;     OxBcriPpfp17867383 = OxBcriPpfp10727550;     OxBcriPpfp10727550 = OxBcriPpfp89004124;     OxBcriPpfp89004124 = OxBcriPpfp13658147;     OxBcriPpfp13658147 = OxBcriPpfp33699213;     OxBcriPpfp33699213 = OxBcriPpfp29512849;     OxBcriPpfp29512849 = OxBcriPpfp71623037;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SMekNowpVA14176305() {     int bWCwFgdnQp55067343 = -780523545;    int bWCwFgdnQp39828088 = -155872385;    int bWCwFgdnQp53668200 = -191556243;    int bWCwFgdnQp38869371 = -436374505;    int bWCwFgdnQp94573749 = -440460629;    int bWCwFgdnQp83559167 = -866414775;    int bWCwFgdnQp25765790 = -714564324;    int bWCwFgdnQp36121514 = -512372257;    int bWCwFgdnQp46052594 = 28439020;    int bWCwFgdnQp18075033 = -988630953;    int bWCwFgdnQp81139967 = -425611616;    int bWCwFgdnQp35886020 = -273023052;    int bWCwFgdnQp77321816 = -36252686;    int bWCwFgdnQp29270156 = -5076599;    int bWCwFgdnQp63634061 = -988158617;    int bWCwFgdnQp45259937 = -301232534;    int bWCwFgdnQp13224672 = -178822381;    int bWCwFgdnQp81979103 = -220727990;    int bWCwFgdnQp7781993 = -25377474;    int bWCwFgdnQp16360012 = -464829105;    int bWCwFgdnQp56085551 = -289532671;    int bWCwFgdnQp39134572 = -405980825;    int bWCwFgdnQp35712907 = -722013944;    int bWCwFgdnQp59191578 = -514772381;    int bWCwFgdnQp97432180 = 92600660;    int bWCwFgdnQp21243698 = -887800400;    int bWCwFgdnQp61127116 = -453593703;    int bWCwFgdnQp65887534 = -705458334;    int bWCwFgdnQp98318729 = -464033499;    int bWCwFgdnQp65162524 = -666213878;    int bWCwFgdnQp31542307 = 54148057;    int bWCwFgdnQp63959454 = -238945330;    int bWCwFgdnQp9395854 = -704401932;    int bWCwFgdnQp5070555 = -211130663;    int bWCwFgdnQp545663 = -658590757;    int bWCwFgdnQp34210955 = 50661570;    int bWCwFgdnQp64768297 = -60458662;    int bWCwFgdnQp52494282 = -626142296;    int bWCwFgdnQp14266320 = -822980753;    int bWCwFgdnQp40744908 = -118416980;    int bWCwFgdnQp72947038 = -868746592;    int bWCwFgdnQp99028346 = -909109423;    int bWCwFgdnQp75418927 = -131066371;    int bWCwFgdnQp97686540 = -494591234;    int bWCwFgdnQp52328349 = -643646577;    int bWCwFgdnQp46442571 = -295923433;    int bWCwFgdnQp48990135 = -90538883;    int bWCwFgdnQp2219381 = -773295405;    int bWCwFgdnQp80661788 = -561542434;    int bWCwFgdnQp31358855 = -397326041;    int bWCwFgdnQp72874502 = -577421024;    int bWCwFgdnQp93661481 = -5790826;    int bWCwFgdnQp11366880 = -934805102;    int bWCwFgdnQp36907431 = -312478930;    int bWCwFgdnQp58090754 = -223841142;    int bWCwFgdnQp15932771 = -274542720;    int bWCwFgdnQp4115181 = -433858442;    int bWCwFgdnQp94476621 = -676783862;    int bWCwFgdnQp41437191 = -428975166;    int bWCwFgdnQp73330052 = -552660230;    int bWCwFgdnQp22432051 = -312821073;    int bWCwFgdnQp59878256 = 90894010;    int bWCwFgdnQp37802785 = 51661242;    int bWCwFgdnQp80890070 = -305347102;    int bWCwFgdnQp86532725 = -942779011;    int bWCwFgdnQp17180514 = -86666286;    int bWCwFgdnQp26490166 = -568621121;    int bWCwFgdnQp72251261 = -825122024;    int bWCwFgdnQp28724494 = -346485843;    int bWCwFgdnQp29423107 = -938820187;    int bWCwFgdnQp80491639 = -140773872;    int bWCwFgdnQp60730390 = -552680086;    int bWCwFgdnQp67712783 = -397747237;    int bWCwFgdnQp67037084 = -906960494;    int bWCwFgdnQp43412974 = -596082514;    int bWCwFgdnQp57057204 = -380423249;    int bWCwFgdnQp63715645 = -174914454;    int bWCwFgdnQp38026367 = -127422710;    int bWCwFgdnQp6863230 = -871125805;    int bWCwFgdnQp50989609 = -611475907;    int bWCwFgdnQp72253562 = -697261517;    int bWCwFgdnQp58907736 = -680298299;    int bWCwFgdnQp85225745 = -43915900;    int bWCwFgdnQp66959874 = 33292541;    int bWCwFgdnQp92288022 = 11207146;    int bWCwFgdnQp37880826 = -940061118;    int bWCwFgdnQp52592575 = -304140229;    int bWCwFgdnQp72488423 = -291923002;    int bWCwFgdnQp46979801 = -987289521;    int bWCwFgdnQp84612891 = -284048037;    int bWCwFgdnQp30095774 = -515479989;    int bWCwFgdnQp70291675 = -383674801;    int bWCwFgdnQp11057091 = -97167130;    int bWCwFgdnQp40936268 = -170320524;    int bWCwFgdnQp18312857 = -805595908;    int bWCwFgdnQp13068782 = -859640603;    int bWCwFgdnQp61225562 = -860770666;    int bWCwFgdnQp94528856 = -825719270;    int bWCwFgdnQp11153815 = -551812223;    int bWCwFgdnQp35147835 = -780523545;     bWCwFgdnQp55067343 = bWCwFgdnQp39828088;     bWCwFgdnQp39828088 = bWCwFgdnQp53668200;     bWCwFgdnQp53668200 = bWCwFgdnQp38869371;     bWCwFgdnQp38869371 = bWCwFgdnQp94573749;     bWCwFgdnQp94573749 = bWCwFgdnQp83559167;     bWCwFgdnQp83559167 = bWCwFgdnQp25765790;     bWCwFgdnQp25765790 = bWCwFgdnQp36121514;     bWCwFgdnQp36121514 = bWCwFgdnQp46052594;     bWCwFgdnQp46052594 = bWCwFgdnQp18075033;     bWCwFgdnQp18075033 = bWCwFgdnQp81139967;     bWCwFgdnQp81139967 = bWCwFgdnQp35886020;     bWCwFgdnQp35886020 = bWCwFgdnQp77321816;     bWCwFgdnQp77321816 = bWCwFgdnQp29270156;     bWCwFgdnQp29270156 = bWCwFgdnQp63634061;     bWCwFgdnQp63634061 = bWCwFgdnQp45259937;     bWCwFgdnQp45259937 = bWCwFgdnQp13224672;     bWCwFgdnQp13224672 = bWCwFgdnQp81979103;     bWCwFgdnQp81979103 = bWCwFgdnQp7781993;     bWCwFgdnQp7781993 = bWCwFgdnQp16360012;     bWCwFgdnQp16360012 = bWCwFgdnQp56085551;     bWCwFgdnQp56085551 = bWCwFgdnQp39134572;     bWCwFgdnQp39134572 = bWCwFgdnQp35712907;     bWCwFgdnQp35712907 = bWCwFgdnQp59191578;     bWCwFgdnQp59191578 = bWCwFgdnQp97432180;     bWCwFgdnQp97432180 = bWCwFgdnQp21243698;     bWCwFgdnQp21243698 = bWCwFgdnQp61127116;     bWCwFgdnQp61127116 = bWCwFgdnQp65887534;     bWCwFgdnQp65887534 = bWCwFgdnQp98318729;     bWCwFgdnQp98318729 = bWCwFgdnQp65162524;     bWCwFgdnQp65162524 = bWCwFgdnQp31542307;     bWCwFgdnQp31542307 = bWCwFgdnQp63959454;     bWCwFgdnQp63959454 = bWCwFgdnQp9395854;     bWCwFgdnQp9395854 = bWCwFgdnQp5070555;     bWCwFgdnQp5070555 = bWCwFgdnQp545663;     bWCwFgdnQp545663 = bWCwFgdnQp34210955;     bWCwFgdnQp34210955 = bWCwFgdnQp64768297;     bWCwFgdnQp64768297 = bWCwFgdnQp52494282;     bWCwFgdnQp52494282 = bWCwFgdnQp14266320;     bWCwFgdnQp14266320 = bWCwFgdnQp40744908;     bWCwFgdnQp40744908 = bWCwFgdnQp72947038;     bWCwFgdnQp72947038 = bWCwFgdnQp99028346;     bWCwFgdnQp99028346 = bWCwFgdnQp75418927;     bWCwFgdnQp75418927 = bWCwFgdnQp97686540;     bWCwFgdnQp97686540 = bWCwFgdnQp52328349;     bWCwFgdnQp52328349 = bWCwFgdnQp46442571;     bWCwFgdnQp46442571 = bWCwFgdnQp48990135;     bWCwFgdnQp48990135 = bWCwFgdnQp2219381;     bWCwFgdnQp2219381 = bWCwFgdnQp80661788;     bWCwFgdnQp80661788 = bWCwFgdnQp31358855;     bWCwFgdnQp31358855 = bWCwFgdnQp72874502;     bWCwFgdnQp72874502 = bWCwFgdnQp93661481;     bWCwFgdnQp93661481 = bWCwFgdnQp11366880;     bWCwFgdnQp11366880 = bWCwFgdnQp36907431;     bWCwFgdnQp36907431 = bWCwFgdnQp58090754;     bWCwFgdnQp58090754 = bWCwFgdnQp15932771;     bWCwFgdnQp15932771 = bWCwFgdnQp4115181;     bWCwFgdnQp4115181 = bWCwFgdnQp94476621;     bWCwFgdnQp94476621 = bWCwFgdnQp41437191;     bWCwFgdnQp41437191 = bWCwFgdnQp73330052;     bWCwFgdnQp73330052 = bWCwFgdnQp22432051;     bWCwFgdnQp22432051 = bWCwFgdnQp59878256;     bWCwFgdnQp59878256 = bWCwFgdnQp37802785;     bWCwFgdnQp37802785 = bWCwFgdnQp80890070;     bWCwFgdnQp80890070 = bWCwFgdnQp86532725;     bWCwFgdnQp86532725 = bWCwFgdnQp17180514;     bWCwFgdnQp17180514 = bWCwFgdnQp26490166;     bWCwFgdnQp26490166 = bWCwFgdnQp72251261;     bWCwFgdnQp72251261 = bWCwFgdnQp28724494;     bWCwFgdnQp28724494 = bWCwFgdnQp29423107;     bWCwFgdnQp29423107 = bWCwFgdnQp80491639;     bWCwFgdnQp80491639 = bWCwFgdnQp60730390;     bWCwFgdnQp60730390 = bWCwFgdnQp67712783;     bWCwFgdnQp67712783 = bWCwFgdnQp67037084;     bWCwFgdnQp67037084 = bWCwFgdnQp43412974;     bWCwFgdnQp43412974 = bWCwFgdnQp57057204;     bWCwFgdnQp57057204 = bWCwFgdnQp63715645;     bWCwFgdnQp63715645 = bWCwFgdnQp38026367;     bWCwFgdnQp38026367 = bWCwFgdnQp6863230;     bWCwFgdnQp6863230 = bWCwFgdnQp50989609;     bWCwFgdnQp50989609 = bWCwFgdnQp72253562;     bWCwFgdnQp72253562 = bWCwFgdnQp58907736;     bWCwFgdnQp58907736 = bWCwFgdnQp85225745;     bWCwFgdnQp85225745 = bWCwFgdnQp66959874;     bWCwFgdnQp66959874 = bWCwFgdnQp92288022;     bWCwFgdnQp92288022 = bWCwFgdnQp37880826;     bWCwFgdnQp37880826 = bWCwFgdnQp52592575;     bWCwFgdnQp52592575 = bWCwFgdnQp72488423;     bWCwFgdnQp72488423 = bWCwFgdnQp46979801;     bWCwFgdnQp46979801 = bWCwFgdnQp84612891;     bWCwFgdnQp84612891 = bWCwFgdnQp30095774;     bWCwFgdnQp30095774 = bWCwFgdnQp70291675;     bWCwFgdnQp70291675 = bWCwFgdnQp11057091;     bWCwFgdnQp11057091 = bWCwFgdnQp40936268;     bWCwFgdnQp40936268 = bWCwFgdnQp18312857;     bWCwFgdnQp18312857 = bWCwFgdnQp13068782;     bWCwFgdnQp13068782 = bWCwFgdnQp61225562;     bWCwFgdnQp61225562 = bWCwFgdnQp94528856;     bWCwFgdnQp94528856 = bWCwFgdnQp11153815;     bWCwFgdnQp11153815 = bWCwFgdnQp35147835;     bWCwFgdnQp35147835 = bWCwFgdnQp55067343;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WBzRXOULiB84632177() {     int neGVEXIRcM67853510 = -915564237;    int neGVEXIRcM26598976 = -326206399;    int neGVEXIRcM71421000 = -599915625;    int neGVEXIRcM46810197 = -260576782;    int neGVEXIRcM88084853 = -932277094;    int neGVEXIRcM26869238 = -643338089;    int neGVEXIRcM46973548 = -160236395;    int neGVEXIRcM62840626 = -956890513;    int neGVEXIRcM41457405 = 66680928;    int neGVEXIRcM24293737 = -792718586;    int neGVEXIRcM8627350 = -540297631;    int neGVEXIRcM67836524 = -780355868;    int neGVEXIRcM47465119 = 59964657;    int neGVEXIRcM78894234 = -337491798;    int neGVEXIRcM86940614 = -895144448;    int neGVEXIRcM73685005 = -852123923;    int neGVEXIRcM68976489 = -942596848;    int neGVEXIRcM65537588 = -875938292;    int neGVEXIRcM8942623 = 65153331;    int neGVEXIRcM19014509 = 35926756;    int neGVEXIRcM36259608 = -780848936;    int neGVEXIRcM68182391 = -575839173;    int neGVEXIRcM74473202 = -653017838;    int neGVEXIRcM3698708 = -875117394;    int neGVEXIRcM3326440 = -117841264;    int neGVEXIRcM2562343 = -578231599;    int neGVEXIRcM72542652 = 12700017;    int neGVEXIRcM33240459 = -879256439;    int neGVEXIRcM14935783 = -678250740;    int neGVEXIRcM17913640 = -296100033;    int neGVEXIRcM75978303 = -222268693;    int neGVEXIRcM67278277 = -217665008;    int neGVEXIRcM55441803 = -580256716;    int neGVEXIRcM82146927 = -115884882;    int neGVEXIRcM83606194 = -168622637;    int neGVEXIRcM29435069 = -837901534;    int neGVEXIRcM1532358 = -950368013;    int neGVEXIRcM10198305 = -545762387;    int neGVEXIRcM20041706 = -664831330;    int neGVEXIRcM67009638 = 37166174;    int neGVEXIRcM34378346 = -144220586;    int neGVEXIRcM6730803 = -807420757;    int neGVEXIRcM10302397 = -154499816;    int neGVEXIRcM42315078 = -835763477;    int neGVEXIRcM32336839 = -536416537;    int neGVEXIRcM20070701 = -369241872;    int neGVEXIRcM26572155 = -588053936;    int neGVEXIRcM31659347 = -928648184;    int neGVEXIRcM24725838 = -725144409;    int neGVEXIRcM78309275 = 91637496;    int neGVEXIRcM91201995 = -830305333;    int neGVEXIRcM21838029 = -476976164;    int neGVEXIRcM45852029 = -10808384;    int neGVEXIRcM81607156 = 98069328;    int neGVEXIRcM48372570 = -218285629;    int neGVEXIRcM99671118 = -239725064;    int neGVEXIRcM52125773 = -673188561;    int neGVEXIRcM67722292 = -724798231;    int neGVEXIRcM43483757 = -42735518;    int neGVEXIRcM85522511 = -254045496;    int neGVEXIRcM54326585 = -556038106;    int neGVEXIRcM13733090 = -280979956;    int neGVEXIRcM47904844 = -178639774;    int neGVEXIRcM23543765 = -637219039;    int neGVEXIRcM48315434 = -470449894;    int neGVEXIRcM41349072 = -222632623;    int neGVEXIRcM12394722 = -100099152;    int neGVEXIRcM65318191 = -824150462;    int neGVEXIRcM95288039 = -68869161;    int neGVEXIRcM57505545 = 42757086;    int neGVEXIRcM72152647 = -901755911;    int neGVEXIRcM58778185 = -296834461;    int neGVEXIRcM45495882 = -111106962;    int neGVEXIRcM41932985 = -972012844;    int neGVEXIRcM84636162 = -819852659;    int neGVEXIRcM29528805 = -973428179;    int neGVEXIRcM57879994 = -321339357;    int neGVEXIRcM32158125 = -817254362;    int neGVEXIRcM71361868 = -238700857;    int neGVEXIRcM83255739 = -748599392;    int neGVEXIRcM75990188 = -990177663;    int neGVEXIRcM40883306 = -58651800;    int neGVEXIRcM8514621 = -54112030;    int neGVEXIRcM36626507 = -669888236;    int neGVEXIRcM26711644 = -465794701;    int neGVEXIRcM54140275 = -745292530;    int neGVEXIRcM21426248 = -106856625;    int neGVEXIRcM73834646 = -578326045;    int neGVEXIRcM33774358 = -897599253;    int neGVEXIRcM83935075 = -928897574;    int neGVEXIRcM77309296 = -64712974;    int neGVEXIRcM33810066 = -125569782;    int neGVEXIRcM66714547 = -403026869;    int neGVEXIRcM34519195 = -310785835;    int neGVEXIRcM12683053 = -406795721;    int neGVEXIRcM20645257 = -863240630;    int neGVEXIRcM58825959 = -528780984;    int neGVEXIRcM86758631 = -517280777;    int neGVEXIRcM93999644 = -265313583;    int neGVEXIRcM90987767 = -915564237;     neGVEXIRcM67853510 = neGVEXIRcM26598976;     neGVEXIRcM26598976 = neGVEXIRcM71421000;     neGVEXIRcM71421000 = neGVEXIRcM46810197;     neGVEXIRcM46810197 = neGVEXIRcM88084853;     neGVEXIRcM88084853 = neGVEXIRcM26869238;     neGVEXIRcM26869238 = neGVEXIRcM46973548;     neGVEXIRcM46973548 = neGVEXIRcM62840626;     neGVEXIRcM62840626 = neGVEXIRcM41457405;     neGVEXIRcM41457405 = neGVEXIRcM24293737;     neGVEXIRcM24293737 = neGVEXIRcM8627350;     neGVEXIRcM8627350 = neGVEXIRcM67836524;     neGVEXIRcM67836524 = neGVEXIRcM47465119;     neGVEXIRcM47465119 = neGVEXIRcM78894234;     neGVEXIRcM78894234 = neGVEXIRcM86940614;     neGVEXIRcM86940614 = neGVEXIRcM73685005;     neGVEXIRcM73685005 = neGVEXIRcM68976489;     neGVEXIRcM68976489 = neGVEXIRcM65537588;     neGVEXIRcM65537588 = neGVEXIRcM8942623;     neGVEXIRcM8942623 = neGVEXIRcM19014509;     neGVEXIRcM19014509 = neGVEXIRcM36259608;     neGVEXIRcM36259608 = neGVEXIRcM68182391;     neGVEXIRcM68182391 = neGVEXIRcM74473202;     neGVEXIRcM74473202 = neGVEXIRcM3698708;     neGVEXIRcM3698708 = neGVEXIRcM3326440;     neGVEXIRcM3326440 = neGVEXIRcM2562343;     neGVEXIRcM2562343 = neGVEXIRcM72542652;     neGVEXIRcM72542652 = neGVEXIRcM33240459;     neGVEXIRcM33240459 = neGVEXIRcM14935783;     neGVEXIRcM14935783 = neGVEXIRcM17913640;     neGVEXIRcM17913640 = neGVEXIRcM75978303;     neGVEXIRcM75978303 = neGVEXIRcM67278277;     neGVEXIRcM67278277 = neGVEXIRcM55441803;     neGVEXIRcM55441803 = neGVEXIRcM82146927;     neGVEXIRcM82146927 = neGVEXIRcM83606194;     neGVEXIRcM83606194 = neGVEXIRcM29435069;     neGVEXIRcM29435069 = neGVEXIRcM1532358;     neGVEXIRcM1532358 = neGVEXIRcM10198305;     neGVEXIRcM10198305 = neGVEXIRcM20041706;     neGVEXIRcM20041706 = neGVEXIRcM67009638;     neGVEXIRcM67009638 = neGVEXIRcM34378346;     neGVEXIRcM34378346 = neGVEXIRcM6730803;     neGVEXIRcM6730803 = neGVEXIRcM10302397;     neGVEXIRcM10302397 = neGVEXIRcM42315078;     neGVEXIRcM42315078 = neGVEXIRcM32336839;     neGVEXIRcM32336839 = neGVEXIRcM20070701;     neGVEXIRcM20070701 = neGVEXIRcM26572155;     neGVEXIRcM26572155 = neGVEXIRcM31659347;     neGVEXIRcM31659347 = neGVEXIRcM24725838;     neGVEXIRcM24725838 = neGVEXIRcM78309275;     neGVEXIRcM78309275 = neGVEXIRcM91201995;     neGVEXIRcM91201995 = neGVEXIRcM21838029;     neGVEXIRcM21838029 = neGVEXIRcM45852029;     neGVEXIRcM45852029 = neGVEXIRcM81607156;     neGVEXIRcM81607156 = neGVEXIRcM48372570;     neGVEXIRcM48372570 = neGVEXIRcM99671118;     neGVEXIRcM99671118 = neGVEXIRcM52125773;     neGVEXIRcM52125773 = neGVEXIRcM67722292;     neGVEXIRcM67722292 = neGVEXIRcM43483757;     neGVEXIRcM43483757 = neGVEXIRcM85522511;     neGVEXIRcM85522511 = neGVEXIRcM54326585;     neGVEXIRcM54326585 = neGVEXIRcM13733090;     neGVEXIRcM13733090 = neGVEXIRcM47904844;     neGVEXIRcM47904844 = neGVEXIRcM23543765;     neGVEXIRcM23543765 = neGVEXIRcM48315434;     neGVEXIRcM48315434 = neGVEXIRcM41349072;     neGVEXIRcM41349072 = neGVEXIRcM12394722;     neGVEXIRcM12394722 = neGVEXIRcM65318191;     neGVEXIRcM65318191 = neGVEXIRcM95288039;     neGVEXIRcM95288039 = neGVEXIRcM57505545;     neGVEXIRcM57505545 = neGVEXIRcM72152647;     neGVEXIRcM72152647 = neGVEXIRcM58778185;     neGVEXIRcM58778185 = neGVEXIRcM45495882;     neGVEXIRcM45495882 = neGVEXIRcM41932985;     neGVEXIRcM41932985 = neGVEXIRcM84636162;     neGVEXIRcM84636162 = neGVEXIRcM29528805;     neGVEXIRcM29528805 = neGVEXIRcM57879994;     neGVEXIRcM57879994 = neGVEXIRcM32158125;     neGVEXIRcM32158125 = neGVEXIRcM71361868;     neGVEXIRcM71361868 = neGVEXIRcM83255739;     neGVEXIRcM83255739 = neGVEXIRcM75990188;     neGVEXIRcM75990188 = neGVEXIRcM40883306;     neGVEXIRcM40883306 = neGVEXIRcM8514621;     neGVEXIRcM8514621 = neGVEXIRcM36626507;     neGVEXIRcM36626507 = neGVEXIRcM26711644;     neGVEXIRcM26711644 = neGVEXIRcM54140275;     neGVEXIRcM54140275 = neGVEXIRcM21426248;     neGVEXIRcM21426248 = neGVEXIRcM73834646;     neGVEXIRcM73834646 = neGVEXIRcM33774358;     neGVEXIRcM33774358 = neGVEXIRcM83935075;     neGVEXIRcM83935075 = neGVEXIRcM77309296;     neGVEXIRcM77309296 = neGVEXIRcM33810066;     neGVEXIRcM33810066 = neGVEXIRcM66714547;     neGVEXIRcM66714547 = neGVEXIRcM34519195;     neGVEXIRcM34519195 = neGVEXIRcM12683053;     neGVEXIRcM12683053 = neGVEXIRcM20645257;     neGVEXIRcM20645257 = neGVEXIRcM58825959;     neGVEXIRcM58825959 = neGVEXIRcM86758631;     neGVEXIRcM86758631 = neGVEXIRcM93999644;     neGVEXIRcM93999644 = neGVEXIRcM90987767;     neGVEXIRcM90987767 = neGVEXIRcM67853510;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uCicYYGRdY2845519() {     int cDQHqjcVEz51297816 = -696229917;    int cDQHqjcVEz43471065 = -325588586;    int cDQHqjcVEz67829507 = -393440852;    int cDQHqjcVEz9602448 = -454376618;    int cDQHqjcVEz94206177 = -739144585;    int cDQHqjcVEz25373639 = -422409742;    int cDQHqjcVEz18012867 = 99852292;    int cDQHqjcVEz77188069 = -944048543;    int cDQHqjcVEz99987105 = -274778423;    int cDQHqjcVEz56839389 = -161094698;    int cDQHqjcVEz58043287 = -730413504;    int cDQHqjcVEz31041924 = -767974114;    int cDQHqjcVEz65582895 = -831696584;    int cDQHqjcVEz58521127 = -125782155;    int cDQHqjcVEz47795078 = -989167430;    int cDQHqjcVEz60074167 = -640203249;    int cDQHqjcVEz62728374 = -695941875;    int cDQHqjcVEz46389035 = -23243169;    int cDQHqjcVEz71382765 = 66478460;    int cDQHqjcVEz8381951 = -493659237;    int cDQHqjcVEz91899994 = -370377967;    int cDQHqjcVEz80329709 = -566990943;    int cDQHqjcVEz78298910 = -692907035;    int cDQHqjcVEz38405266 = -946942790;    int cDQHqjcVEz44902397 = -677494023;    int cDQHqjcVEz97904491 = -458741314;    int cDQHqjcVEz21997393 = -715384326;    int cDQHqjcVEz72972084 = -949211323;    int cDQHqjcVEz51016876 = -540755882;    int cDQHqjcVEz33746981 = -808949884;    int cDQHqjcVEz94073616 = -960025894;    int cDQHqjcVEz30384705 = -132860088;    int cDQHqjcVEz98980689 = -201128894;    int cDQHqjcVEz24305439 = -807487052;    int cDQHqjcVEz62332925 = -366287372;    int cDQHqjcVEz41824254 = -625925440;    int cDQHqjcVEz70146536 = -829217193;    int cDQHqjcVEz50777136 = -254232907;    int cDQHqjcVEz69036784 = -849729128;    int cDQHqjcVEz16504773 = -815427348;    int cDQHqjcVEz68027270 = -815769559;    int cDQHqjcVEz57218589 = -281329079;    int cDQHqjcVEz33869467 = -705557059;    int cDQHqjcVEz34220019 = -591946247;    int cDQHqjcVEz24281336 = -264573491;    int cDQHqjcVEz59676696 = -388852796;    int cDQHqjcVEz2587585 = -720358165;    int cDQHqjcVEz51329942 = -927369903;    int cDQHqjcVEz55572950 = -210746492;    int cDQHqjcVEz13641760 = -115382959;    int cDQHqjcVEz64459443 = 48793245;    int cDQHqjcVEz77338822 = -535235475;    int cDQHqjcVEz59964488 = -222555701;    int cDQHqjcVEz6283915 = -720344060;    int cDQHqjcVEz10945525 = -566771752;    int cDQHqjcVEz70968106 = -29238975;    int cDQHqjcVEz65172154 = -632681551;    int cDQHqjcVEz29424241 = -446498063;    int cDQHqjcVEz64700051 = -776882595;    int cDQHqjcVEz96301685 = -180403271;    int cDQHqjcVEz3376247 = -707025416;    int cDQHqjcVEz45040782 = 49063615;    int cDQHqjcVEz26171193 = -303292662;    int cDQHqjcVEz66240125 = -465828539;    int cDQHqjcVEz62765772 = -201068805;    int cDQHqjcVEz27658583 = -497553416;    int cDQHqjcVEz32061234 = -466845221;    int cDQHqjcVEz41277456 = 75790467;    int cDQHqjcVEz96188201 = -759494783;    int cDQHqjcVEz5970825 = -263241990;    int cDQHqjcVEz89927630 = -810986056;    int cDQHqjcVEz11951238 = -341708968;    int cDQHqjcVEz77352250 = -173514041;    int cDQHqjcVEz54877992 = -118094192;    int cDQHqjcVEz40354681 = -677889679;    int cDQHqjcVEz34681406 = 10951112;    int cDQHqjcVEz46460243 = -861433885;    int cDQHqjcVEz44078892 = -960788;    int cDQHqjcVEz14123930 = -582369299;    int cDQHqjcVEz85225701 = -188641227;    int cDQHqjcVEz95316906 = -738383149;    int cDQHqjcVEz70667450 = -788014424;    int cDQHqjcVEz17399134 = -638464832;    int cDQHqjcVEz37375117 = -325372923;    int cDQHqjcVEz69287537 = -757743130;    int cDQHqjcVEz16734794 = -324790419;    int cDQHqjcVEz70420216 = -910304388;    int cDQHqjcVEz92696774 = -480784835;    int cDQHqjcVEz13359914 = -140715300;    int cDQHqjcVEz91364818 = -237048398;    int cDQHqjcVEz76652099 = -993243889;    int cDQHqjcVEz40722296 = -282719130;    int cDQHqjcVEz86077085 = -477350312;    int cDQHqjcVEz72735098 = -569325858;    int cDQHqjcVEz13128527 = -8401933;    int cDQHqjcVEz22986489 = -764833174;    int cDQHqjcVEz31047396 = -978036418;    int cDQHqjcVEz67629342 = -139728521;    int cDQHqjcVEz71454246 = -290877443;    int cDQHqjcVEz96622752 = -696229917;     cDQHqjcVEz51297816 = cDQHqjcVEz43471065;     cDQHqjcVEz43471065 = cDQHqjcVEz67829507;     cDQHqjcVEz67829507 = cDQHqjcVEz9602448;     cDQHqjcVEz9602448 = cDQHqjcVEz94206177;     cDQHqjcVEz94206177 = cDQHqjcVEz25373639;     cDQHqjcVEz25373639 = cDQHqjcVEz18012867;     cDQHqjcVEz18012867 = cDQHqjcVEz77188069;     cDQHqjcVEz77188069 = cDQHqjcVEz99987105;     cDQHqjcVEz99987105 = cDQHqjcVEz56839389;     cDQHqjcVEz56839389 = cDQHqjcVEz58043287;     cDQHqjcVEz58043287 = cDQHqjcVEz31041924;     cDQHqjcVEz31041924 = cDQHqjcVEz65582895;     cDQHqjcVEz65582895 = cDQHqjcVEz58521127;     cDQHqjcVEz58521127 = cDQHqjcVEz47795078;     cDQHqjcVEz47795078 = cDQHqjcVEz60074167;     cDQHqjcVEz60074167 = cDQHqjcVEz62728374;     cDQHqjcVEz62728374 = cDQHqjcVEz46389035;     cDQHqjcVEz46389035 = cDQHqjcVEz71382765;     cDQHqjcVEz71382765 = cDQHqjcVEz8381951;     cDQHqjcVEz8381951 = cDQHqjcVEz91899994;     cDQHqjcVEz91899994 = cDQHqjcVEz80329709;     cDQHqjcVEz80329709 = cDQHqjcVEz78298910;     cDQHqjcVEz78298910 = cDQHqjcVEz38405266;     cDQHqjcVEz38405266 = cDQHqjcVEz44902397;     cDQHqjcVEz44902397 = cDQHqjcVEz97904491;     cDQHqjcVEz97904491 = cDQHqjcVEz21997393;     cDQHqjcVEz21997393 = cDQHqjcVEz72972084;     cDQHqjcVEz72972084 = cDQHqjcVEz51016876;     cDQHqjcVEz51016876 = cDQHqjcVEz33746981;     cDQHqjcVEz33746981 = cDQHqjcVEz94073616;     cDQHqjcVEz94073616 = cDQHqjcVEz30384705;     cDQHqjcVEz30384705 = cDQHqjcVEz98980689;     cDQHqjcVEz98980689 = cDQHqjcVEz24305439;     cDQHqjcVEz24305439 = cDQHqjcVEz62332925;     cDQHqjcVEz62332925 = cDQHqjcVEz41824254;     cDQHqjcVEz41824254 = cDQHqjcVEz70146536;     cDQHqjcVEz70146536 = cDQHqjcVEz50777136;     cDQHqjcVEz50777136 = cDQHqjcVEz69036784;     cDQHqjcVEz69036784 = cDQHqjcVEz16504773;     cDQHqjcVEz16504773 = cDQHqjcVEz68027270;     cDQHqjcVEz68027270 = cDQHqjcVEz57218589;     cDQHqjcVEz57218589 = cDQHqjcVEz33869467;     cDQHqjcVEz33869467 = cDQHqjcVEz34220019;     cDQHqjcVEz34220019 = cDQHqjcVEz24281336;     cDQHqjcVEz24281336 = cDQHqjcVEz59676696;     cDQHqjcVEz59676696 = cDQHqjcVEz2587585;     cDQHqjcVEz2587585 = cDQHqjcVEz51329942;     cDQHqjcVEz51329942 = cDQHqjcVEz55572950;     cDQHqjcVEz55572950 = cDQHqjcVEz13641760;     cDQHqjcVEz13641760 = cDQHqjcVEz64459443;     cDQHqjcVEz64459443 = cDQHqjcVEz77338822;     cDQHqjcVEz77338822 = cDQHqjcVEz59964488;     cDQHqjcVEz59964488 = cDQHqjcVEz6283915;     cDQHqjcVEz6283915 = cDQHqjcVEz10945525;     cDQHqjcVEz10945525 = cDQHqjcVEz70968106;     cDQHqjcVEz70968106 = cDQHqjcVEz65172154;     cDQHqjcVEz65172154 = cDQHqjcVEz29424241;     cDQHqjcVEz29424241 = cDQHqjcVEz64700051;     cDQHqjcVEz64700051 = cDQHqjcVEz96301685;     cDQHqjcVEz96301685 = cDQHqjcVEz3376247;     cDQHqjcVEz3376247 = cDQHqjcVEz45040782;     cDQHqjcVEz45040782 = cDQHqjcVEz26171193;     cDQHqjcVEz26171193 = cDQHqjcVEz66240125;     cDQHqjcVEz66240125 = cDQHqjcVEz62765772;     cDQHqjcVEz62765772 = cDQHqjcVEz27658583;     cDQHqjcVEz27658583 = cDQHqjcVEz32061234;     cDQHqjcVEz32061234 = cDQHqjcVEz41277456;     cDQHqjcVEz41277456 = cDQHqjcVEz96188201;     cDQHqjcVEz96188201 = cDQHqjcVEz5970825;     cDQHqjcVEz5970825 = cDQHqjcVEz89927630;     cDQHqjcVEz89927630 = cDQHqjcVEz11951238;     cDQHqjcVEz11951238 = cDQHqjcVEz77352250;     cDQHqjcVEz77352250 = cDQHqjcVEz54877992;     cDQHqjcVEz54877992 = cDQHqjcVEz40354681;     cDQHqjcVEz40354681 = cDQHqjcVEz34681406;     cDQHqjcVEz34681406 = cDQHqjcVEz46460243;     cDQHqjcVEz46460243 = cDQHqjcVEz44078892;     cDQHqjcVEz44078892 = cDQHqjcVEz14123930;     cDQHqjcVEz14123930 = cDQHqjcVEz85225701;     cDQHqjcVEz85225701 = cDQHqjcVEz95316906;     cDQHqjcVEz95316906 = cDQHqjcVEz70667450;     cDQHqjcVEz70667450 = cDQHqjcVEz17399134;     cDQHqjcVEz17399134 = cDQHqjcVEz37375117;     cDQHqjcVEz37375117 = cDQHqjcVEz69287537;     cDQHqjcVEz69287537 = cDQHqjcVEz16734794;     cDQHqjcVEz16734794 = cDQHqjcVEz70420216;     cDQHqjcVEz70420216 = cDQHqjcVEz92696774;     cDQHqjcVEz92696774 = cDQHqjcVEz13359914;     cDQHqjcVEz13359914 = cDQHqjcVEz91364818;     cDQHqjcVEz91364818 = cDQHqjcVEz76652099;     cDQHqjcVEz76652099 = cDQHqjcVEz40722296;     cDQHqjcVEz40722296 = cDQHqjcVEz86077085;     cDQHqjcVEz86077085 = cDQHqjcVEz72735098;     cDQHqjcVEz72735098 = cDQHqjcVEz13128527;     cDQHqjcVEz13128527 = cDQHqjcVEz22986489;     cDQHqjcVEz22986489 = cDQHqjcVEz31047396;     cDQHqjcVEz31047396 = cDQHqjcVEz67629342;     cDQHqjcVEz67629342 = cDQHqjcVEz71454246;     cDQHqjcVEz71454246 = cDQHqjcVEz96622752;     cDQHqjcVEz96622752 = cDQHqjcVEz51297816;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ySFcIUnuQL73301392() {     int lBxQMYnrQI64083983 = -831270609;    int lBxQMYnrQI30241953 = -495922600;    int lBxQMYnrQI85582306 = -801800234;    int lBxQMYnrQI17543274 = -278578895;    int lBxQMYnrQI87717282 = -130961050;    int lBxQMYnrQI68683709 = -199333055;    int lBxQMYnrQI39220624 = -445819778;    int lBxQMYnrQI3907182 = -288566800;    int lBxQMYnrQI95391915 = -236536515;    int lBxQMYnrQI63058094 = 34817669;    int lBxQMYnrQI85530669 = -845099519;    int lBxQMYnrQI62992429 = -175306930;    int lBxQMYnrQI35726198 = -735479242;    int lBxQMYnrQI8145206 = -458197353;    int lBxQMYnrQI71101631 = -896153261;    int lBxQMYnrQI88499235 = -91094638;    int lBxQMYnrQI18480192 = -359716342;    int lBxQMYnrQI29947520 = -678453471;    int lBxQMYnrQI72543395 = -942990735;    int lBxQMYnrQI11036448 = 7096624;    int lBxQMYnrQI72074051 = -861694231;    int lBxQMYnrQI9377529 = -736849291;    int lBxQMYnrQI17059206 = -623910930;    int lBxQMYnrQI82912395 = -207287802;    int lBxQMYnrQI50796657 = -887935947;    int lBxQMYnrQI79223137 = -149172513;    int lBxQMYnrQI33412929 = -249090606;    int lBxQMYnrQI40325009 = -23009428;    int lBxQMYnrQI67633929 = -754973122;    int lBxQMYnrQI86498096 = -438836039;    int lBxQMYnrQI38509613 = -136442644;    int lBxQMYnrQI33703528 = -111579767;    int lBxQMYnrQI45026639 = -76983678;    int lBxQMYnrQI1381812 = -712241270;    int lBxQMYnrQI45393457 = -976319253;    int lBxQMYnrQI37048368 = -414488544;    int lBxQMYnrQI6910597 = -619126543;    int lBxQMYnrQI8481160 = -173852999;    int lBxQMYnrQI74812170 = -691579705;    int lBxQMYnrQI42769503 = -659844194;    int lBxQMYnrQI29458579 = -91243552;    int lBxQMYnrQI64921045 = -179640413;    int lBxQMYnrQI68752936 = -728990504;    int lBxQMYnrQI78848556 = -933118490;    int lBxQMYnrQI4289827 = -157343451;    int lBxQMYnrQI33304826 = -462171236;    int lBxQMYnrQI80169604 = -117873218;    int lBxQMYnrQI80769908 = 17277318;    int lBxQMYnrQI99636999 = -374348466;    int lBxQMYnrQI60592180 = -726419422;    int lBxQMYnrQI82786937 = -204091063;    int lBxQMYnrQI5515370 = 93579187;    int lBxQMYnrQI94449638 = -398558983;    int lBxQMYnrQI50983640 = -309795802;    int lBxQMYnrQI1227341 = -561216238;    int lBxQMYnrQI54706454 = 5578682;    int lBxQMYnrQI13182747 = -872011670;    int lBxQMYnrQI2669912 = -494512433;    int lBxQMYnrQI66746617 = -390642948;    int lBxQMYnrQI8494146 = -981788537;    int lBxQMYnrQI35270781 = -950242450;    int lBxQMYnrQI98895615 = -322810351;    int lBxQMYnrQI36273252 = -533593678;    int lBxQMYnrQI8893820 = -797700476;    int lBxQMYnrQI24548482 = -828739688;    int lBxQMYnrQI51827142 = -633519753;    int lBxQMYnrQI17965790 = 1676747;    int lBxQMYnrQI34344387 = 76762028;    int lBxQMYnrQI62751748 = -481878101;    int lBxQMYnrQI34053263 = -381664717;    int lBxQMYnrQI81588639 = -471968095;    int lBxQMYnrQI9999033 = -85863343;    int lBxQMYnrQI55135350 = -986873766;    int lBxQMYnrQI29773893 = -183146542;    int lBxQMYnrQI81577869 = -901659824;    int lBxQMYnrQI7153007 = -582053818;    int lBxQMYnrQI40624592 = 92141212;    int lBxQMYnrQI38210650 = -690792440;    int lBxQMYnrQI78622569 = 50055648;    int lBxQMYnrQI17491831 = -325764712;    int lBxQMYnrQI99053532 = 68700704;    int lBxQMYnrQI52643020 = -166367924;    int lBxQMYnrQI40688010 = -648660962;    int lBxQMYnrQI7041750 = 71446300;    int lBxQMYnrQI3711160 = -134744976;    int lBxQMYnrQI32994243 = -130021831;    int lBxQMYnrQI39253890 = -713020784;    int lBxQMYnrQI94042998 = -767187877;    int lBxQMYnrQI154471 = -51025032;    int lBxQMYnrQI90687002 = -881897935;    int lBxQMYnrQI23865622 = -542476875;    int lBxQMYnrQI4240686 = -24614111;    int lBxQMYnrQI41734542 = -783210052;    int lBxQMYnrQI66318025 = -709791169;    int lBxQMYnrQI7498722 = -709601745;    int lBxQMYnrQI30562963 = -768433201;    int lBxQMYnrQI28647793 = -646046735;    int lBxQMYnrQI59859116 = -931290028;    int lBxQMYnrQI54300075 = -4378803;    int lBxQMYnrQI52462685 = -831270609;     lBxQMYnrQI64083983 = lBxQMYnrQI30241953;     lBxQMYnrQI30241953 = lBxQMYnrQI85582306;     lBxQMYnrQI85582306 = lBxQMYnrQI17543274;     lBxQMYnrQI17543274 = lBxQMYnrQI87717282;     lBxQMYnrQI87717282 = lBxQMYnrQI68683709;     lBxQMYnrQI68683709 = lBxQMYnrQI39220624;     lBxQMYnrQI39220624 = lBxQMYnrQI3907182;     lBxQMYnrQI3907182 = lBxQMYnrQI95391915;     lBxQMYnrQI95391915 = lBxQMYnrQI63058094;     lBxQMYnrQI63058094 = lBxQMYnrQI85530669;     lBxQMYnrQI85530669 = lBxQMYnrQI62992429;     lBxQMYnrQI62992429 = lBxQMYnrQI35726198;     lBxQMYnrQI35726198 = lBxQMYnrQI8145206;     lBxQMYnrQI8145206 = lBxQMYnrQI71101631;     lBxQMYnrQI71101631 = lBxQMYnrQI88499235;     lBxQMYnrQI88499235 = lBxQMYnrQI18480192;     lBxQMYnrQI18480192 = lBxQMYnrQI29947520;     lBxQMYnrQI29947520 = lBxQMYnrQI72543395;     lBxQMYnrQI72543395 = lBxQMYnrQI11036448;     lBxQMYnrQI11036448 = lBxQMYnrQI72074051;     lBxQMYnrQI72074051 = lBxQMYnrQI9377529;     lBxQMYnrQI9377529 = lBxQMYnrQI17059206;     lBxQMYnrQI17059206 = lBxQMYnrQI82912395;     lBxQMYnrQI82912395 = lBxQMYnrQI50796657;     lBxQMYnrQI50796657 = lBxQMYnrQI79223137;     lBxQMYnrQI79223137 = lBxQMYnrQI33412929;     lBxQMYnrQI33412929 = lBxQMYnrQI40325009;     lBxQMYnrQI40325009 = lBxQMYnrQI67633929;     lBxQMYnrQI67633929 = lBxQMYnrQI86498096;     lBxQMYnrQI86498096 = lBxQMYnrQI38509613;     lBxQMYnrQI38509613 = lBxQMYnrQI33703528;     lBxQMYnrQI33703528 = lBxQMYnrQI45026639;     lBxQMYnrQI45026639 = lBxQMYnrQI1381812;     lBxQMYnrQI1381812 = lBxQMYnrQI45393457;     lBxQMYnrQI45393457 = lBxQMYnrQI37048368;     lBxQMYnrQI37048368 = lBxQMYnrQI6910597;     lBxQMYnrQI6910597 = lBxQMYnrQI8481160;     lBxQMYnrQI8481160 = lBxQMYnrQI74812170;     lBxQMYnrQI74812170 = lBxQMYnrQI42769503;     lBxQMYnrQI42769503 = lBxQMYnrQI29458579;     lBxQMYnrQI29458579 = lBxQMYnrQI64921045;     lBxQMYnrQI64921045 = lBxQMYnrQI68752936;     lBxQMYnrQI68752936 = lBxQMYnrQI78848556;     lBxQMYnrQI78848556 = lBxQMYnrQI4289827;     lBxQMYnrQI4289827 = lBxQMYnrQI33304826;     lBxQMYnrQI33304826 = lBxQMYnrQI80169604;     lBxQMYnrQI80169604 = lBxQMYnrQI80769908;     lBxQMYnrQI80769908 = lBxQMYnrQI99636999;     lBxQMYnrQI99636999 = lBxQMYnrQI60592180;     lBxQMYnrQI60592180 = lBxQMYnrQI82786937;     lBxQMYnrQI82786937 = lBxQMYnrQI5515370;     lBxQMYnrQI5515370 = lBxQMYnrQI94449638;     lBxQMYnrQI94449638 = lBxQMYnrQI50983640;     lBxQMYnrQI50983640 = lBxQMYnrQI1227341;     lBxQMYnrQI1227341 = lBxQMYnrQI54706454;     lBxQMYnrQI54706454 = lBxQMYnrQI13182747;     lBxQMYnrQI13182747 = lBxQMYnrQI2669912;     lBxQMYnrQI2669912 = lBxQMYnrQI66746617;     lBxQMYnrQI66746617 = lBxQMYnrQI8494146;     lBxQMYnrQI8494146 = lBxQMYnrQI35270781;     lBxQMYnrQI35270781 = lBxQMYnrQI98895615;     lBxQMYnrQI98895615 = lBxQMYnrQI36273252;     lBxQMYnrQI36273252 = lBxQMYnrQI8893820;     lBxQMYnrQI8893820 = lBxQMYnrQI24548482;     lBxQMYnrQI24548482 = lBxQMYnrQI51827142;     lBxQMYnrQI51827142 = lBxQMYnrQI17965790;     lBxQMYnrQI17965790 = lBxQMYnrQI34344387;     lBxQMYnrQI34344387 = lBxQMYnrQI62751748;     lBxQMYnrQI62751748 = lBxQMYnrQI34053263;     lBxQMYnrQI34053263 = lBxQMYnrQI81588639;     lBxQMYnrQI81588639 = lBxQMYnrQI9999033;     lBxQMYnrQI9999033 = lBxQMYnrQI55135350;     lBxQMYnrQI55135350 = lBxQMYnrQI29773893;     lBxQMYnrQI29773893 = lBxQMYnrQI81577869;     lBxQMYnrQI81577869 = lBxQMYnrQI7153007;     lBxQMYnrQI7153007 = lBxQMYnrQI40624592;     lBxQMYnrQI40624592 = lBxQMYnrQI38210650;     lBxQMYnrQI38210650 = lBxQMYnrQI78622569;     lBxQMYnrQI78622569 = lBxQMYnrQI17491831;     lBxQMYnrQI17491831 = lBxQMYnrQI99053532;     lBxQMYnrQI99053532 = lBxQMYnrQI52643020;     lBxQMYnrQI52643020 = lBxQMYnrQI40688010;     lBxQMYnrQI40688010 = lBxQMYnrQI7041750;     lBxQMYnrQI7041750 = lBxQMYnrQI3711160;     lBxQMYnrQI3711160 = lBxQMYnrQI32994243;     lBxQMYnrQI32994243 = lBxQMYnrQI39253890;     lBxQMYnrQI39253890 = lBxQMYnrQI94042998;     lBxQMYnrQI94042998 = lBxQMYnrQI154471;     lBxQMYnrQI154471 = lBxQMYnrQI90687002;     lBxQMYnrQI90687002 = lBxQMYnrQI23865622;     lBxQMYnrQI23865622 = lBxQMYnrQI4240686;     lBxQMYnrQI4240686 = lBxQMYnrQI41734542;     lBxQMYnrQI41734542 = lBxQMYnrQI66318025;     lBxQMYnrQI66318025 = lBxQMYnrQI7498722;     lBxQMYnrQI7498722 = lBxQMYnrQI30562963;     lBxQMYnrQI30562963 = lBxQMYnrQI28647793;     lBxQMYnrQI28647793 = lBxQMYnrQI59859116;     lBxQMYnrQI59859116 = lBxQMYnrQI54300075;     lBxQMYnrQI54300075 = lBxQMYnrQI52462685;     lBxQMYnrQI52462685 = lBxQMYnrQI64083983;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tusALSNjvM91514733() {     int jGnjqoSZCB47528288 = -611936290;    int jGnjqoSZCB47114042 = -495304787;    int jGnjqoSZCB81990813 = -595325462;    int jGnjqoSZCB80335525 = -472378730;    int jGnjqoSZCB93838606 = 62171459;    int jGnjqoSZCB67188110 = 21595292;    int jGnjqoSZCB10259943 = -185731092;    int jGnjqoSZCB18254625 = -275724829;    int jGnjqoSZCB53921616 = -577995866;    int jGnjqoSZCB95603746 = -433558443;    int jGnjqoSZCB34946608 = 64784608;    int jGnjqoSZCB26197828 = -162925177;    int jGnjqoSZCB53843973 = -527140483;    int jGnjqoSZCB87772097 = -246487710;    int jGnjqoSZCB31956094 = -990176243;    int jGnjqoSZCB74888397 = -979173963;    int jGnjqoSZCB12232077 = -113061369;    int jGnjqoSZCB10798967 = -925758347;    int jGnjqoSZCB34983538 = -941665606;    int jGnjqoSZCB403890 = -522489368;    int jGnjqoSZCB27714439 = -451223262;    int jGnjqoSZCB21524847 = -728001061;    int jGnjqoSZCB20884914 = -663800127;    int jGnjqoSZCB17618954 = -279113198;    int jGnjqoSZCB92372614 = -347588706;    int jGnjqoSZCB74565286 = -29682229;    int jGnjqoSZCB82867669 = -977174949;    int jGnjqoSZCB80056634 = -92964311;    int jGnjqoSZCB3715024 = -617478264;    int jGnjqoSZCB2331438 = -951685891;    int jGnjqoSZCB56604926 = -874199845;    int jGnjqoSZCB96809955 = -26774847;    int jGnjqoSZCB88565525 = -797855856;    int jGnjqoSZCB43540322 = -303843440;    int jGnjqoSZCB24120188 = -73983988;    int jGnjqoSZCB49437552 = -202512450;    int jGnjqoSZCB75524775 = -497975723;    int jGnjqoSZCB49059991 = -982323519;    int jGnjqoSZCB23807248 = -876477503;    int jGnjqoSZCB92264637 = -412437716;    int jGnjqoSZCB63107503 = -762792525;    int jGnjqoSZCB15408831 = -753548735;    int jGnjqoSZCB92320006 = -180047747;    int jGnjqoSZCB70753496 = -689301261;    int jGnjqoSZCB96234323 = -985500404;    int jGnjqoSZCB72910821 = -481782159;    int jGnjqoSZCB56185035 = -250177447;    int jGnjqoSZCB440505 = 18555599;    int jGnjqoSZCB30484112 = -959950549;    int jGnjqoSZCB95924663 = -933439877;    int jGnjqoSZCB56044385 = -424992486;    int jGnjqoSZCB61016164 = 35319875;    int jGnjqoSZCB8562098 = -610306300;    int jGnjqoSZCB75660399 = -28209189;    int jGnjqoSZCB63800295 = -909702361;    int jGnjqoSZCB26003442 = -883935229;    int jGnjqoSZCB26229129 = -831504661;    int jGnjqoSZCB64371860 = -216212264;    int jGnjqoSZCB87962910 = -24790025;    int jGnjqoSZCB19273320 = -908146312;    int jGnjqoSZCB84320441 = -1229760;    int jGnjqoSZCB30203308 = 7233219;    int jGnjqoSZCB14539601 = -658246565;    int jGnjqoSZCB51590179 = -626309976;    int jGnjqoSZCB38998820 = -559358599;    int jGnjqoSZCB38136652 = -908440546;    int jGnjqoSZCB37632303 = -365069322;    int jGnjqoSZCB10303652 = -123297043;    int jGnjqoSZCB63651910 = -72503722;    int jGnjqoSZCB82518541 = -687663793;    int jGnjqoSZCB99363622 = -381198241;    int jGnjqoSZCB63172086 = -130737850;    int jGnjqoSZCB86991718 = 50719155;    int jGnjqoSZCB42718900 = -429227890;    int jGnjqoSZCB37296387 = -759696844;    int jGnjqoSZCB12305608 = -697674528;    int jGnjqoSZCB29204841 = -447953315;    int jGnjqoSZCB50131417 = -974498867;    int jGnjqoSZCB21384631 = -293612794;    int jGnjqoSZCB19461793 = -865806547;    int jGnjqoSZCB18380252 = -779504782;    int jGnjqoSZCB82427165 = -895730548;    int jGnjqoSZCB49572523 = -133013763;    int jGnjqoSZCB7790360 = -684038387;    int jGnjqoSZCB46287052 = -426693405;    int jGnjqoSZCB95588762 = -809519720;    int jGnjqoSZCB88247858 = -416468548;    int jGnjqoSZCB12905127 = -669646667;    int jGnjqoSZCB79740027 = -394141079;    int jGnjqoSZCB98116745 = -190048760;    int jGnjqoSZCB23208424 = -371007790;    int jGnjqoSZCB11152916 = -181763459;    int jGnjqoSZCB61097081 = -857533495;    int jGnjqoSZCB4533929 = -968331191;    int jGnjqoSZCB7944197 = -311207957;    int jGnjqoSZCB32904195 = -670025745;    int jGnjqoSZCB869230 = 4697830;    int jGnjqoSZCB40729827 = -553737771;    int jGnjqoSZCB31754677 = -29942663;    int jGnjqoSZCB58097671 = -611936290;     jGnjqoSZCB47528288 = jGnjqoSZCB47114042;     jGnjqoSZCB47114042 = jGnjqoSZCB81990813;     jGnjqoSZCB81990813 = jGnjqoSZCB80335525;     jGnjqoSZCB80335525 = jGnjqoSZCB93838606;     jGnjqoSZCB93838606 = jGnjqoSZCB67188110;     jGnjqoSZCB67188110 = jGnjqoSZCB10259943;     jGnjqoSZCB10259943 = jGnjqoSZCB18254625;     jGnjqoSZCB18254625 = jGnjqoSZCB53921616;     jGnjqoSZCB53921616 = jGnjqoSZCB95603746;     jGnjqoSZCB95603746 = jGnjqoSZCB34946608;     jGnjqoSZCB34946608 = jGnjqoSZCB26197828;     jGnjqoSZCB26197828 = jGnjqoSZCB53843973;     jGnjqoSZCB53843973 = jGnjqoSZCB87772097;     jGnjqoSZCB87772097 = jGnjqoSZCB31956094;     jGnjqoSZCB31956094 = jGnjqoSZCB74888397;     jGnjqoSZCB74888397 = jGnjqoSZCB12232077;     jGnjqoSZCB12232077 = jGnjqoSZCB10798967;     jGnjqoSZCB10798967 = jGnjqoSZCB34983538;     jGnjqoSZCB34983538 = jGnjqoSZCB403890;     jGnjqoSZCB403890 = jGnjqoSZCB27714439;     jGnjqoSZCB27714439 = jGnjqoSZCB21524847;     jGnjqoSZCB21524847 = jGnjqoSZCB20884914;     jGnjqoSZCB20884914 = jGnjqoSZCB17618954;     jGnjqoSZCB17618954 = jGnjqoSZCB92372614;     jGnjqoSZCB92372614 = jGnjqoSZCB74565286;     jGnjqoSZCB74565286 = jGnjqoSZCB82867669;     jGnjqoSZCB82867669 = jGnjqoSZCB80056634;     jGnjqoSZCB80056634 = jGnjqoSZCB3715024;     jGnjqoSZCB3715024 = jGnjqoSZCB2331438;     jGnjqoSZCB2331438 = jGnjqoSZCB56604926;     jGnjqoSZCB56604926 = jGnjqoSZCB96809955;     jGnjqoSZCB96809955 = jGnjqoSZCB88565525;     jGnjqoSZCB88565525 = jGnjqoSZCB43540322;     jGnjqoSZCB43540322 = jGnjqoSZCB24120188;     jGnjqoSZCB24120188 = jGnjqoSZCB49437552;     jGnjqoSZCB49437552 = jGnjqoSZCB75524775;     jGnjqoSZCB75524775 = jGnjqoSZCB49059991;     jGnjqoSZCB49059991 = jGnjqoSZCB23807248;     jGnjqoSZCB23807248 = jGnjqoSZCB92264637;     jGnjqoSZCB92264637 = jGnjqoSZCB63107503;     jGnjqoSZCB63107503 = jGnjqoSZCB15408831;     jGnjqoSZCB15408831 = jGnjqoSZCB92320006;     jGnjqoSZCB92320006 = jGnjqoSZCB70753496;     jGnjqoSZCB70753496 = jGnjqoSZCB96234323;     jGnjqoSZCB96234323 = jGnjqoSZCB72910821;     jGnjqoSZCB72910821 = jGnjqoSZCB56185035;     jGnjqoSZCB56185035 = jGnjqoSZCB440505;     jGnjqoSZCB440505 = jGnjqoSZCB30484112;     jGnjqoSZCB30484112 = jGnjqoSZCB95924663;     jGnjqoSZCB95924663 = jGnjqoSZCB56044385;     jGnjqoSZCB56044385 = jGnjqoSZCB61016164;     jGnjqoSZCB61016164 = jGnjqoSZCB8562098;     jGnjqoSZCB8562098 = jGnjqoSZCB75660399;     jGnjqoSZCB75660399 = jGnjqoSZCB63800295;     jGnjqoSZCB63800295 = jGnjqoSZCB26003442;     jGnjqoSZCB26003442 = jGnjqoSZCB26229129;     jGnjqoSZCB26229129 = jGnjqoSZCB64371860;     jGnjqoSZCB64371860 = jGnjqoSZCB87962910;     jGnjqoSZCB87962910 = jGnjqoSZCB19273320;     jGnjqoSZCB19273320 = jGnjqoSZCB84320441;     jGnjqoSZCB84320441 = jGnjqoSZCB30203308;     jGnjqoSZCB30203308 = jGnjqoSZCB14539601;     jGnjqoSZCB14539601 = jGnjqoSZCB51590179;     jGnjqoSZCB51590179 = jGnjqoSZCB38998820;     jGnjqoSZCB38998820 = jGnjqoSZCB38136652;     jGnjqoSZCB38136652 = jGnjqoSZCB37632303;     jGnjqoSZCB37632303 = jGnjqoSZCB10303652;     jGnjqoSZCB10303652 = jGnjqoSZCB63651910;     jGnjqoSZCB63651910 = jGnjqoSZCB82518541;     jGnjqoSZCB82518541 = jGnjqoSZCB99363622;     jGnjqoSZCB99363622 = jGnjqoSZCB63172086;     jGnjqoSZCB63172086 = jGnjqoSZCB86991718;     jGnjqoSZCB86991718 = jGnjqoSZCB42718900;     jGnjqoSZCB42718900 = jGnjqoSZCB37296387;     jGnjqoSZCB37296387 = jGnjqoSZCB12305608;     jGnjqoSZCB12305608 = jGnjqoSZCB29204841;     jGnjqoSZCB29204841 = jGnjqoSZCB50131417;     jGnjqoSZCB50131417 = jGnjqoSZCB21384631;     jGnjqoSZCB21384631 = jGnjqoSZCB19461793;     jGnjqoSZCB19461793 = jGnjqoSZCB18380252;     jGnjqoSZCB18380252 = jGnjqoSZCB82427165;     jGnjqoSZCB82427165 = jGnjqoSZCB49572523;     jGnjqoSZCB49572523 = jGnjqoSZCB7790360;     jGnjqoSZCB7790360 = jGnjqoSZCB46287052;     jGnjqoSZCB46287052 = jGnjqoSZCB95588762;     jGnjqoSZCB95588762 = jGnjqoSZCB88247858;     jGnjqoSZCB88247858 = jGnjqoSZCB12905127;     jGnjqoSZCB12905127 = jGnjqoSZCB79740027;     jGnjqoSZCB79740027 = jGnjqoSZCB98116745;     jGnjqoSZCB98116745 = jGnjqoSZCB23208424;     jGnjqoSZCB23208424 = jGnjqoSZCB11152916;     jGnjqoSZCB11152916 = jGnjqoSZCB61097081;     jGnjqoSZCB61097081 = jGnjqoSZCB4533929;     jGnjqoSZCB4533929 = jGnjqoSZCB7944197;     jGnjqoSZCB7944197 = jGnjqoSZCB32904195;     jGnjqoSZCB32904195 = jGnjqoSZCB869230;     jGnjqoSZCB869230 = jGnjqoSZCB40729827;     jGnjqoSZCB40729827 = jGnjqoSZCB31754677;     jGnjqoSZCB31754677 = jGnjqoSZCB58097671;     jGnjqoSZCB58097671 = jGnjqoSZCB47528288;}
// Junk Finished
