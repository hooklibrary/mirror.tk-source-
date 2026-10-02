#pragma once
#include "Hooks.h"
#include "Hacks.h"
#include "Singleton.hpp"

class animationlayer {
public:
	byte	pad_0x0[0x14];
	int		order;
	int		sequence;
	float	previous_cycle;
	float	weight;
	float	weight_delta_rate;
	float	playback_rate;
	float	cycle;
	void*	owner;
	byte	pad_0x38[0x4];
};
class c_animfix : public Singleton< c_animfix > {
public:

	bool           shoot[65];
	bool           hit[65];
	void local_player_fix_cm();
	struct player_anims
	{
		animationlayer m_layers[15];
//		std::array< float, 24 > pose_params;
		CBaseAnimState* animstate;


	};
	CBaseAnimState animstate;
//	std::array< bool, 64 > on_ground{ false };
//	std::array< bool, 64 > last_on_ground{ false };

//	std::array< vec_t, 64 > speed{ vec_t() };
//	std::array< vec_t, 64 > last_speed{ vec_t() };

	void update_animations(IClientEntity* entity);
	void re_work(ClientFrameStage_t stage);
	void fix_local_player_animations();
	player_anims player_data[64];

};

extern std::unique_ptr< c_animfix > animfix;