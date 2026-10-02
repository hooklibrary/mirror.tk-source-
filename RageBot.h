#pragma once
#include "Hacks.h"
struct AntiaimData_t
{
	AntiaimData_t(const float& dist, const bool& inair, int player)
	{
		this->flDist = dist;
		this->bInAir = inair;
		this->iPlayer = player;
	}
	float flDist;
	bool bInAir;
	int	iPlayer;
};
class CAimbot : public CHack
{
public:

	void Init();
	void Draw();
	int choked = -1;
	bool aim_at_point = false;
	bool was_firing;
	bool was_firing_test;
	bool has_entity = false;
	bool can_autowall = false;
	bool can_shoot = false;
	bool shot_refined = false;
	bool shot_this_tick;

	bool valid_hitchance;
	
	float hitchance();
	
	bool CanOpenFire(IClientEntity * local);
	bool should_baim(IClientEntity * pEntity);
	
	void missed_shot_log(CUserCmd * pcmd, C_BaseCombatWeapon * pWeapon);

	bool should_prefer_head(IClientEntity * pEntity);
	bool enemy_is_slow_walking(IClientEntity * entity);
	bool TargetMeetsRequirements(IClientEntity * pEntity, IClientEntity * local);

	int TargetID;
	int get_target_fov(IClientEntity* pLocal);
	int automatic_hitscan(IClientEntity * entity);

	std::vector<int> head_hitscan();
	std::vector<int> upperbody_hitscan();
	std::vector<int> lowerbody_hitscan();
	std::vector<int> arms_hitscan();
	std::vector<int> legs_hitscan();

	std::vector<int> awp_minimal_hitscan();

	std::vector<int> minimal_hitscan();

	std::vector<int> essential_hitscan();

	std::vector<int> maximal_hitscan();

	int HitScan(IClientEntity * pEntity, IClientEntity * pLocal);

	void auto_revolver(CUserCmd * m_pcmd);
	void sim_time_delay(IClientEntity * entity);
	void delay_shot(IClientEntity * ent, CUserCmd * pcmd);
	void mirror_console_debug(IClientEntity * the_nignog);

	void faxzee_extrapolation(IClientEntity * pTarget, Vector AP);
	float interpolation_time();
	void DoAimbot(CUserCmd *pCmd, bool &bSendPacket);
	void DoNoRecoil(CUserCmd *pCmd);
	void aimAtPlayer(CUserCmd * pCmd, IClientEntity * pLocal);
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	CUserCmd* cmd;

	Vector target_point;
	Vector origin_fl;

	IClientEntity * pTarget;
private:
	std::vector<AntiaimData_t> Entities;

	bool AimAtPoint(IClientEntity * pLocal, Vector point, CUserCmd * pCmd, bool & bSendPacket);

	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	//	void Base_AntiAim(CUserCmd * pCmd, IClientEntity * pLocal);
	bool IsAimStepping;
	Vector LastAimstepAngle;
	Vector LastAngle;

	bool IsLocked;

	int HitBox;
	Vector AimPoint;

};
extern CAimbot * ragebot;

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4    
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10
