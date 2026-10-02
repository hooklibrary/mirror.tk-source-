#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Interfaces.h"
#include "CRC32.h"
#include <fstream>
#include "XorStr.hpp"
#define WINDOW_WIDTH 575
#define WINDOW_HEIGHT 510 // 507
mirror_window options::menu;
struct Config_t {
	int id;
	std::string name;
};
std::vector<Config_t> configs;
typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void MsgX(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); 	char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}
void save_callback()
{
	int should_save = options::menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_save].name; config_directory += ".xml";
	GUI.SaveWindowState(&options::menu, XorStr(config_directory.c_str()));
	interfaces::cvar->ConsoleColorPrintf(Color(140, 10, 250, 255), "Mirror v6 ");
	std::string uremam = "Saved configuration.     \n";
	MsgX(uremam.c_str());
}
void load_callback()
{
	int should_load = options::menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_load].name; config_directory += ".xml";
	GUI.LoadWindowState(&options::menu, XorStr(config_directory.c_str()));
	interfaces::cvar->ConsoleColorPrintf(Color(140, 10, 250, 255), "Mirror v6 ");
	std::string uremam = "Loaded configuration.     \n";
	MsgX(uremam.c_str());
}

void list_configs() {
	configs.clear();
	options::menu.ColorsTab.ConfigListBox.ClearItems();
	std::ifstream file_in;
	file_in.open("miroawr\\cfg\\miroawr_configs.txt");
	if (file_in.fail()) {
		std::ofstream("miroawr\\cfg\\miroawr_configs.txt");
		file_in.open("miroawr\\cfg\\miroawr_configs.txt");
	}
	int line_count;
	while (!file_in.eof()) {
		Config_t config;
		file_in >> config.name;
		config.id = line_count;
		configs.push_back(config);
		line_count++;
		options::menu.ColorsTab.ConfigListBox.AddItem(config.name);
	}
	file_in.close();
	if (configs.size() > 7) options::menu.ColorsTab.ConfigListBox.AddItem(" ");
}

void add_config() {
	std::fstream file;
	file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	if (file.fail()) {
		std::fstream("miroawr\\cfg\\miroawr_configs.txt");
		file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	}
	file << std::endl << options::menu.ColorsTab.NewConfigName.getText();
	file.close();
	list_configs();
	int should_add = options::menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += options::menu.ColorsTab.NewConfigName.getText(); config_directory += ".xml";
	GUI.SaveWindowState(&options::menu, XorStr(config_directory.c_str()));
	options::menu.ColorsTab.NewConfigName.SetText("");
}

void remove_config() {
	int should_remove = options::menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_remove].name; config_directory += ".xml";
	std::remove(config_directory.c_str());
	std::ofstream ofs("miroawr\\cfg\\miroawr_configs.txt", std::ios::out | std::ios::trunc);
	ofs.close();
	std::fstream file;
	file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	if (file.fail()) {
		std::fstream("miroawr\\cfg\\miroawr_configs.txt");
		file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	}
	for (int i = 0; i < configs.size(); i++) {
		if (i == should_remove) continue;
		Config_t config = configs[i];
		file << std::endl << config.name;
	}
	file.close();
	list_configs();
}

void KnifeApplyCallbk()
{
	static ConVar* Meme = interfaces::cvar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	interfaces::engine->ClientCmd_Unrestricted("cl_fullupdate");
}

void UnLoadCallbk()
{
	DoUnload = true;
}
void mirror_window::Setup()
{
	SetPosition(350, 50);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("");
	RegisterTab(&aimbot);
	//	RegisterTab(&LegitBotTab);
	RegisterTab(&visuals);
	RegisterTab(&misc);
	RegisterTab(&ColorsTab);
	RegisterTab(&skin);

	RECT Client = GetClientArea();
	Client.bottom -= 29;
	aimbot.Setup();
	//	LegitBotTab.Setup();
	visuals.Setup();
	misc.Setup();
	ColorsTab.Setup();
	skin.Setup();

#pragma endregion
}
void CAimbotTab::Setup()
{
	SetTitle("J");
#pragma region Aimbot
	AimbotGroup.SetPosition(4, 25); // 15, 25
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(550, 225);
	AimbotGroup.AddTab(CGroupTab("Main", 1));
	AimbotGroup.AddTab(CGroupTab("Accuracy", 2));
	AimbotGroup.AddTab(CGroupTab("Lag Compensation", 3));
	AimbotGroup.AddTab(CGroupTab("BodyAim", 4));

	//	AimbotGroup.AddTab(CGroupTab("Debug", 5));
	//	AimbotGroup.AddTab(CGroupTab("Resolver Mods", 4));
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl(1, "Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotAutoFire.SetState(true);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl(1, "Choke Shot", this, &AimbotSilentAim);

	AccuracyAutoScope.SetFileId("acc_scope");
	AimbotGroup.PlaceLabledControl(1, "Auto Scope", this, &AccuracyAutoScope);

	AccuracyHitchance.SetFileId("base_hc");
	AccuracyHitchance.SetBoundaries(0.f, 100.f);
	AccuracyHitchance.SetValue(20);

	AccuracyMinimumDamage.SetFileId("base_md");
	AccuracyMinimumDamage.SetBoundaries(0.f, 100.f);
	AccuracyMinimumDamage.SetValue(20);

	preso.SetFileId("acc_zeusisgay");
	preso.AddItem("Default");
	preso.AddItem("Down");
	preso.AddItem("Up");
	preso.AddItem("Zero");
	preso.AddItem("Automatic");
	AimbotGroup.PlaceLabledControl(1, "Pitch Adjustment", this, &preso);

	resolver.SetFileId("acc_aaa");
	resolver.AddItem("Default");
	resolver.AddItem("Stickrpg");
	resolver.AddItem("Mirror Primary");
	resolver.AddItem("Gemini Software");
	//	resolver.AddItem("experimental");
	AimbotGroup.PlaceLabledControl(1, "Yaw Adjustment", this, &resolver);

	flip180.SetFileId("flip180");
	AimbotGroup.PlaceLabledControl(1, "Flip 180", this, &flip180);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.extension = XorStr("�");
	AimbotFov.SetValue(180.f);

	/*
	TargetPointscale.SetFileId("acc_hitbox_Scale");
	TargetPointscale.SetBoundaries(0, 100);
	TargetPointscale.SetValue(50);
	TargetPointscale.extension = ("%%");
	AimbotGroup.PlaceLabledControl(2, "Hitbox Scale", this, &TargetPointscale);
	*/

	Multienable.SetFileId("multipoint_enable");
	AimbotGroup.PlaceLabledControl(2, "Toggle Multipoint", this, &Multienable);

	Multival2.SetFileId("hitbox_scale_head");
	Multival2.SetBoundaries(0.1, 100);
	Multival2.SetValue(20);
	Multival2.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Head Multipoint", this, &Multival2);

	Multival4.SetFileId("hitbox_scale_upperbody");
	Multival4.SetBoundaries(0.1, 100);
	Multival4.SetValue(20);
	Multival4.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Upper Body Multipoint", this, &Multival4);

	Multival.SetFileId("hitbox_scale_body");
	Multival.SetBoundaries(0.1, 100);
	Multival.SetValue(20);
	Multival.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Lower Body Multipoint", this, &Multival);

	MultiVal3.SetFileId("hitbox_scale_legs");
	MultiVal3.SetBoundaries(0.1, 100);
	MultiVal3.SetValue(20);
	MultiVal3.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Leg Multipoint", this, &MultiVal3);

	lag_pred.SetFileId("lag_pred");
	lag_pred.AddItem("Off");
	lag_pred.AddItem("Classic");
	lag_pred.AddItem("Genuine");
	AimbotGroup.PlaceLabledControl(3, "Position Adjustment", this, &lag_pred);

	delay_shot.SetFileId("delay_shot");
	delay_shot.AddItem("Off");
	delay_shot.AddItem("Sim-Time");
	delay_shot.AddItem("Lag Compensation");
	delay_shot.AddItem("Refine Shot");
	AimbotGroup.PlaceLabledControl(3, "Delay Shot", this, &delay_shot);

	extrapolation.SetFileId("acc_extra_P_lation");
	AimbotGroup.PlaceLabledControl(3, "Extrapolation", this, &extrapolation);

	baim_fake.SetFileId("bodyaim_fake");
	AimbotGroup.PlaceLabledControl(4, "If Fake", this, &baim_fake); // if we have to resort to a brute

	baim_fakewalk.SetFileId("bodyaim_fakewalk");
	AimbotGroup.PlaceLabledControl(4, "If Slow Walk", this, &baim_fakewalk);

	baim_inair.SetFileId("bodyaim_inair");
	AimbotGroup.PlaceLabledControl(4, "If In Air", this, &baim_inair); //if they be flyin like a plane

	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealth");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.extension = XorStr("HP");
	BaimIfUnderXHealth.SetValue(0);
	AimbotGroup.PlaceLabledControl(4, "If HP Lower Than", this, &BaimIfUnderXHealth);

	bigbaim.SetFileId("acc_bigbaim");
	AimbotGroup.PlaceLabledControl(4, "On Key", this, &bigbaim);

	/*
	prefer_head.SetFileId("acc_prefer_head");
	prefer_head.items.push_back(dropdownboxitem(false, XorStr("If Moving")));
	//	prefer_head.items.push_back(dropdownboxitem(false, XorStr("Lower body Is Unhittable")));
	prefer_head.items.push_back(dropdownboxitem(false, XorStr("No Fake")));
	//	prefer_head.items.push_back(dropdownboxitem(false, XorStr("Head Is Visible")));
	//	prefer_head.items.push_back(dropdownboxitem(false, XorStr("")));
	AimbotGroup.PlaceLabledControl(4, "Prefer Head Aim Factors", this, &prefer_head);
	*/
	//	toggledebug.SetFileId("debugtoggle");
	//	AimbotGroup.PlaceLabledControl(5, "Print Debug Info In Console", this, &toggledebug);


	// -<--------------------------------------------------------------->- //
	/*
	legit_mode.SetFileId("aim_legit_toggle");
	AimbotGroup.PlaceLabledControl(4, "Enable", this, &legit_mode);

	legit_trigger.SetFileId("aim_legit_trigger");
	AimbotGroup.PlaceLabledControl(4, "Trigger", this, &legit_trigger);

	legit_trigger_key.SetFileId("aim_legit_trigger_key");
	AimbotGroup.PlaceLabledControl(4, "Trigger Key", this, &legit_trigger_key);

	apply_smooth.SetFileId("aim_legit_apply_smooth");
	AimbotGroup.PlaceLabledControl(4, "Apply Smoothness", this, &apply_smooth); */
	// -<--------------------------------------------------------------->- //

	weapongroup.SetText("Weapon Configurations");
	weapongroup.SetPosition(4, 260); // 15, 230
	weapongroup.SetSize(550, 180);
	weapongroup.AddTab(CGroupTab("Auto Sniper", 1));
	weapongroup.AddTab(CGroupTab("Pistols", 2));
	weapongroup.AddTab(CGroupTab("Scout", 3));
	weapongroup.AddTab(CGroupTab("Awp", 4));
	weapongroup.AddTab(CGroupTab("SMG", 5));
	weapongroup.AddTab(CGroupTab("Others", 6));
	RegisterControl(&weapongroup);

	/*
	target_auto.SetFileId("tgt_hitbox_auto");
	target_auto.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_auto.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_auto.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_auto.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_auto.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(1, "HitScan", this, &target_auto);

	custom_hitscan.SetFileId("auto_hitscan_auto");
	weapongroup.PlaceLabledControl(1, "Automatic HitScan", this, &custom_hitscan);

	hc_auto.SetFileId("auto_hitchance");
	hc_auto.SetBoundaries(0, 100);
	hc_auto.SetValue(25);
	hc_auto.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(1, "Hitchance", this, &hc_auto);

	md_auto.SetFileId("auto_minimumdamage");
	md_auto.SetBoundaries(0, 100);
	md_auto.SetValue(25);
	md_auto.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(1, "Minimum Damage", this, &md_auto);

	//	preset_auto.SetFileId("auto_automatic_cfg");
	//	weapongroup.PlaceLabledControl(1, "Automatic Auto Sniper Configuration", this, &preset_auto);
	//----------------------------------------------------------------------

	target_pistol.SetFileId("tgt_hitbox_pistol");
	target_pistol.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_pistol.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_pistol.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_pistol.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_pistol.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(2, "HitScan", this, &target_pistol);

	hc_pistol.SetFileId("pistol_hitchance");
	hc_pistol.SetBoundaries(0, 100);
	hc_pistol.SetValue(25);
	hc_pistol.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(2, "Hitchance", this, &hc_pistol);

	md_pistol.SetFileId("pistol_minimumdamage");
	md_pistol.SetBoundaries(0, 100);
	md_pistol.SetValue(25);
	md_pistol.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(2, "Minimum Damage", this, &md_pistol);


	//	preset_pistol.SetFileId("pistol_automatic_cfg");
	//	weapongroup.PlaceLabledControl(2, "Automatic Pistol Configuration", this, &preset_pistol);

	//----------------------------------------------------------------------

	target_scout.SetFileId("tgt_hitbox_scout");
	target_scout.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_scout.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_scout.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_scout.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_scout.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(3, "HitScan", this, &target_scout);

	hc_scout.SetFileId("scout_hitchance");
	hc_scout.SetBoundaries(0, 100);
	hc_scout.SetValue(25);
	hc_scout.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(3, "Hitchance", this, &hc_scout);

	md_scout.SetFileId("scout_minimumdamage");
	md_scout.SetBoundaries(0, 100);
	md_scout.SetValue(25);
	md_scout.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(3, "Minimum Damage", this, &md_scout);

	//	headonly_if_vis_scout.SetFileId("headonly_if_vis_scout");
	//	weapongroup.PlaceLabledControl(3, "Headshot Only If Hittable", this, &headonly_if_vis_scout);

	//	preset_scout.SetFileId("scout_automatic_cfg");
	//	weapongroup.PlaceLabledControl(3, "Automatic Scout Configuration", this, &preset_scout);
	//----------------------------------------------------------------------

	target_awp.SetFileId("tgt_hitbox_awp");
	target_awp.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_awp.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_awp.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_awp.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_awp.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(4, "HitScan", this, &target_awp);

	hc_awp.SetFileId("awp_hitchance");
	hc_awp.SetBoundaries(0, 100);
	hc_awp.SetValue(25);
	hc_awp.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(4, "Hitchance", this, &hc_awp);

	md_awp.SetFileId("awp_minimumdamage");
	md_awp.SetBoundaries(0, 125);
	md_awp.SetValue(25);
	md_awp.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(4, "Minimum Damage", this, &md_awp);

	min_damage_enemy_hp_awp.SetFileId("tgt_min_damage_enemy_hp_awp");
	weapongroup.PlaceLabledControl(4, "Minimum Damage Based Off Enemy Health", this, &min_damage_enemy_hp_awp);

	//----------------------------------------------------------------------

	target_smg.SetFileId("tgt_hitbox_smg");
	target_smg.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_smg.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_smg.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_smg.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_smg.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(5, "HitScan", this, &target_smg);

	hc_smg.SetFileId("smg_hitchance");
	hc_smg.SetBoundaries(0, 100);
	hc_smg.SetValue(25);
	hc_smg.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(5, "Hitchance", this, &hc_smg);

	md_smg.SetFileId("smg_minimumdamage");
	md_smg.SetBoundaries(0, 100);
	md_smg.SetValue(25);
	md_smg.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(5, "Minimum Damage", this, &md_smg);

	//----------------------------------------------------------------------

	target_otr.SetFileId("tgt_hitbox_otr");
	target_otr.items.push_back(dropdownboxitem(false, XorStr("Head")));
	target_otr.items.push_back(dropdownboxitem(false, XorStr("Upper Body")));
	target_otr.items.push_back(dropdownboxitem(false, XorStr("Lower Body")));
	target_otr.items.push_back(dropdownboxitem(false, XorStr("Arms")));
	target_otr.items.push_back(dropdownboxitem(false, XorStr("Legs")));
	weapongroup.PlaceLabledControl(6, "HitScan", this, &target_otr);

	hc_otr.SetFileId("otr_hitchance");
	hc_otr.SetBoundaries(0, 100);
	hc_otr.SetValue(25);
	hc_otr.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(6, "Hitchance", this, &hc_otr);

	md_otr.SetFileId("otr_minimumdamage");
	md_otr.SetBoundaries(0, 100);
	md_otr.SetValue(25);
	md_otr.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(6, "Minimum Damage", this, &md_otr);

	//	headonly_if_vis_otr.SetFileId("headonly_if_vis_otr");
	//	weapongroup.PlaceLabledControl(5, "Headshot Only If Hittable", this, &headonly_if_vis_otr);

	//	preset_otr.SetFileId("otr_automatic_cfg");
	//	weapongroup.PlaceLabledControl(5, "Automatic  Weapon Configuration", this, &preset_otr);

	*/

	/*		target_auto.SetFileId("tgt_hitbox_auto");
	target_auto.AddItem("Head");
	target_auto.AddItem("Neck");
	target_auto.AddItem("Chest");
	target_auto.AddItem("Pelvis");
	weapongroup.PlaceLabledControl(1, "Hitbox Priority", this, &target_auto); */

	target_auto2.SetFileId("tgt_hitscan_autosniper");
	target_auto2.AddItem("Off");
	target_auto2.AddItem("Minimal");
	target_auto2.AddItem("Essential");
	target_auto2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(1, "Hitscan", this, &target_auto2);

	hc_auto.SetFileId("auto_hitchance");
	hc_auto.SetBoundaries(0, 100);
	hc_auto.SetValue(25);
	hc_auto.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(1, "Hitchance", this, &hc_auto);

	md_auto.SetFileId("auto_minimumdamage");
	md_auto.SetBoundaries(0, 100);
	md_auto.SetValue(25);
	md_auto.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(1, "Minimum Damage", this, &md_auto);

	//----------------------------------------------------------------------

	/*		target_pistol.SetFileId("tgt_hitbox_pistol");
	target_pistol.AddItem("Head");
	target_pistol.AddItem("Neck");
	target_pistol.AddItem("Chest");
	target_pistol.AddItem("Pelvis");
	weapongroup.PlaceLabledControl(2, "Hitbox Priority", this, &target_pistol); */

	target_pistol2.SetFileId("tgt_hitscan_pistol");
	target_pistol2.AddItem("Off");
	target_pistol2.AddItem("Minimal");
	target_pistol2.AddItem("Essential");
	target_pistol2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(2, "Hitscan", this, &target_pistol2);

	hc_pistol.SetFileId("pistol_hitchance");
	hc_pistol.SetBoundaries(0, 100);
	hc_pistol.SetValue(25);
	hc_pistol.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(2, "Hitchance", this, &hc_pistol);

	md_pistol.SetFileId("pistol_minimumdamage");
	md_pistol.SetBoundaries(0, 100);
	md_pistol.SetValue(25);
	md_pistol.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(2, "Minimum Damage", this, &md_pistol);


	//----------------------------------------------------------------------

	/*		target_scout.SetFileId("tgt_hitbox_scout");
	target_scout.AddItem("Head");
	target_scout.AddItem("Neck");
	target_scout.AddItem("Chest");
	target_scout.AddItem("Pelvis");
	weapongroup.PlaceLabledControl(3, "Hitbox Priority", this, &target_scout); */

	target_scout2.SetFileId("tgt_hitscan_scout");
	target_scout2.AddItem("Off");
	target_scout2.AddItem("Minimal");
	target_scout2.AddItem("Essential");
	target_scout2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(3, "Hitscan", this, &target_scout2);

	hc_scout.SetFileId("scout_hitchance");
	hc_scout.SetBoundaries(0, 100);
	hc_scout.SetValue(25);
	hc_scout.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(3, "Hitchance", this, &hc_scout);

	md_scout.SetFileId("scout_minimumdamage");
	md_scout.SetBoundaries(0, 100);
	md_scout.SetValue(25);
	md_scout.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(3, "Minimum Damage", this, &md_scout);

	//----------------------------------------------------------------------

	/*		target_awp.SetFileId("tgt_hitbox_awp");
	target_awp.AddItem("Head");
	target_awp.AddItem("Neck");
	target_awp.AddItem("Chest");
	target_awp.AddItem("Pelvis");
	weapongroup.PlaceLabledControl(4, "Hitbox Priority", this, &target_awp); */

	target_awp2.SetFileId("tgt_hitscan_awp");
	target_awp2.AddItem("Off");
	target_awp2.AddItem("Minimal");
	target_awp2.AddItem("Essential");
	target_awp2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(4, "Hitscan", this, &target_awp2);

	hc_awp.SetFileId("awp_hitchance");
	hc_awp.SetBoundaries(0, 100);
	hc_awp.SetValue(25);
	hc_awp.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(4, "Hitchance", this, &hc_awp);

	md_awp.SetFileId("awp_minimumdamage");
	md_awp.SetBoundaries(0, 100);
	md_awp.SetValue(25);
	md_awp.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(4, "Minimum Damage", this, &md_awp);


	//----------------------------------------------------------------------

	/*		target_otr.SetFileId("tgt_hitbox_otr");
	target_otr.AddItem("Head");
	target_otr.AddItem("Neck");
	target_otr.AddItem("Chest");
	target_otr.AddItem("Pelvis");
	weapongroup.PlaceLabledControl(5, "Hitbox Priority", this, &target_otr); */

	target_smg2.SetFileId("tgt_hitscan_smg");
	target_smg2.AddItem("Off");
	target_smg2.AddItem("Minimal");
	target_smg2.AddItem("Essential");
	target_smg2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(5, "Hitscan", this, &target_smg2);

	hc_smg.SetFileId("otr_hitchance");
	hc_smg.SetBoundaries(0, 100);
	hc_smg.SetValue(25);
	hc_smg.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(5, "Hitchance", this, &hc_smg);

	md_smg.SetFileId("otr_minimumdamage");
	md_smg.SetBoundaries(0, 100);
	md_smg.SetValue(25);
	md_smg.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(5, "Minimum Damage", this, &md_smg);


	target_otr2.SetFileId("tgt_hitscan_otr");
	target_otr2.AddItem("Off");
	target_otr2.AddItem("Minimal");
	target_otr2.AddItem("Essential");
	target_otr2.AddItem("Maximal");
	weapongroup.PlaceLabledControl(6, "Hitscan", this, &target_otr2);

	hc_otr.SetFileId("otr_hitchance");
	hc_otr.SetBoundaries(0, 100);
	hc_otr.SetValue(25);
	hc_otr.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(6, "Hitchance", this, &hc_otr);

	md_otr.SetFileId("otr_minimumdamage");
	md_otr.SetBoundaries(0, 100);
	md_otr.SetValue(25);
	md_otr.extension = XorStr("%%");
	weapongroup.PlaceLabledControl(6, "Minimum Damage", this, &md_otr);


#pragma endregion  AntiAim controls get setup in here
}
/*
void CLegitBotTab::Setup()
{	/*
_      ______ _____ _____ _______
| |    |  ____/ ____|_   _|__   __|
| |    | |__ | |  __  | |    | |
| |    |  __|| | |_ | | |    | |
| |____| |___| |__| |_| |_   | |
|______|______\_____|_____|  |_|


SetTitle("B");
AimbotGroup.SetText("Main");
AimbotGroup.SetPosition(4, 25);
AimbotGroup.SetSize(550, 222);
AimbotGroup.AddTab(CGroupTab("Aimbot", 1));
AimbotGroup.AddTab(CGroupTab("Triggerbot", 2));
AimbotGroup.AddTab(CGroupTab("Filters", 3));
RegisterControl(&AimbotGroup);
RegisterControl(&Active);
Active.SetFileId("active");
AimbotGroup.PlaceLabledControl(1, "Activate Legitbot", this, &Active);
AimbotEnable.SetFileId("l_aimbot");
AimbotGroup.PlaceLabledControl(1, "Aimbot Enable", this, &AimbotEnable);
aimbotfiremode.SetFileId("l_autoaimbot");
aimbotfiremode.AddItem("On Shot");
aimbotfiremode.AddItem("Automatic");
AimbotGroup.PlaceLabledControl(1, "Fire Mode", this, &aimbotfiremode);
AimbotKeyBind.SetFileId("l_aimkey");
AimbotGroup.PlaceLabledControl(1, "KeyBind", this, &AimbotKeyBind);
BackTrack.SetFileId("l_backtrack");
AimbotGroup.PlaceLabledControl(1, "Backtrack", this, &BackTrack);
AimbotSmokeCheck.SetFileId("l_smokeaimbot");
AimbotGroup.PlaceLabledControl(1, "Smoke Check", this, &AimbotSmokeCheck);

legitresolver.SetFileId("l_B1GresolverTappingSkeet");
AimbotGroup.PlaceLabledControl(1, "Resolver", this, &legitresolver);

//---- Trigger ---//
TriggerEnable.SetFileId("t_triggerbotenable");
AimbotGroup.PlaceLabledControl(2, "Activate Trigger", this, &TriggerEnable);
triggertype.SetFileId("t_triggerbottyp");
triggertype.AddItem("None");
triggertype.AddItem("Automatic");
AimbotGroup.PlaceLabledControl(2, "Trigger Mode", this, &triggertype);
TriggerHitChanceAmmount.SetFileId("l_trigHC");
TriggerHitChanceAmmount.SetBoundaries(0, 100);
TriggerHitChanceAmmount.extension = XorStr("%%");
TriggerHitChanceAmmount.SetValue(0);
AimbotGroup.PlaceLabledControl(2, "Hitchance", this, &TriggerHitChanceAmmount);
TriggerSmokeCheck.SetFileId("l_trigsmoke");
AimbotGroup.PlaceLabledControl(2, "Smoke Check", this, &TriggerSmokeCheck);
TriggerRecoil.SetFileId("l_trigRCS");
TriggerRecoil.SetBoundaries(0.f, 100.f);
TriggerRecoil.extension = XorStr("%%");
TriggerRecoil.SetValue(5.00f);
AimbotGroup.PlaceLabledControl(2, "Recoil", this, &TriggerRecoil);
TriggerKeyBind.SetFileId("l_trigkey");
AimbotGroup.PlaceLabledControl(2, "KeyBind", this, &TriggerKeyBind);
// ---- Hitboxes ---- //
TriggerHead.SetFileId("l_trighead");
AimbotGroup.PlaceLabledControl(3, "Head", this, &TriggerHead);
TriggerChest.SetFileId("l_trigchest");
AimbotGroup.PlaceLabledControl(3, "Chest", this, &TriggerChest);
TriggerStomach.SetFileId("l_trigstomach");
AimbotGroup.PlaceLabledControl(3, "Stomach", this, &TriggerStomach);
TriggerArms.SetFileId("l_trigarms");
AimbotGroup.PlaceLabledControl(3, "Arms", this, &TriggerArms);
TriggerLegs.SetFileId("l_triglegs");
AimbotGroup.PlaceLabledControl(3, "Legs", this, &TriggerLegs);

//--------------------------legitaa--------------------------((
/*	aaenable.SetFileId("AA_anable");
AimbotGroup.PlaceLabledControl(4, "Legit AA Enable", this, &aaenable);

aatyp.SetFileId("AA_aatyp");
aatyp.AddItem("Static");
aatyp.AddItem("Lowerbody");
aatyp.AddItem("Freestanding");
AimbotGroup.PlaceLabledControl(4, "Anti-Aim", this, &aatyp);

aatyp2.SetFileId("AA_aatyp2");
aatyp2.AddItem("Default");
aatyp2.AddItem("Jitter");
aatyp2.AddItem("Shuffle");
aatyp2.AddItem("Spin");
AimbotGroup.PlaceLabledControl(4, "Anti-Aim Type", this, &aatyp2);

aatyp3.SetFileId("AA_aatyp3");
aatyp3.SetBoundaries(0, 90);
aatyp3.SetValue(20);
AimbotGroup.PlaceLabledControl(4, "Anti-Aim Value", this, &aatyp3);

aafl.SetFileId("AA_aaFL");
aafl.SetBoundaries(1, 4);
aafl.SetValue(1);
AimbotGroup.PlaceLabledControl(4, "Legit FakeLag", this, &aafl); */
//----------------------solid kys---------------------//
/*
weapongroup.SetText("Main");
weapongroup.SetPosition(4, 256);
weapongroup.SetSize(550, 183);
weapongroup.AddTab(CGroupTab("Rifle", 1));
weapongroup.AddTab(CGroupTab("Pistol", 2));
weapongroup.AddTab(CGroupTab("Sniper", 3));
weapongroup.AddTab(CGroupTab("SMG", 4));
weapongroup.AddTab(CGroupTab("Heavy", 5));
RegisterControl(&weapongroup);
RegisterControl(&Active);
WeaponMainHitbox.SetFileId("l_rhitbox");
WeaponMainHitbox.AddItem("Head");
WeaponMainHitbox.AddItem("Neck");
WeaponMainHitbox.AddItem("Chest");
WeaponMainHitbox.AddItem("Stomach");
WeaponMainHitbox.AddItem("Nearest");
weapongroup.PlaceLabledControl(1, "Hitbox", this, &WeaponMainHitbox);
WeaponMainSpeed.SetFileId("l_rspeed");
WeaponMainSpeed.SetBoundaries(0, 75);
WeaponMainSpeed.SetValue(5);
weapongroup.PlaceLabledControl(1, "Speed", this, &WeaponMainSpeed);
WeaponMainRecoil.SetFileId("l_rRecoil");
WeaponMainRecoil.SetBoundaries(0, 200);
WeaponMainRecoil.SetValue(165);
weapongroup.PlaceLabledControl(1, "Recoil", this, &WeaponMainRecoil);
WeaponMainFoV.SetFileId("l_fov");
WeaponMainFoV.SetBoundaries(0, 45);
WeaponMainFoV.SetValue(10);
weapongroup.PlaceLabledControl(1, "Field Of View", this, &WeaponMainFoV);
// --- Pistols --- //
WeaponPistHitbox.SetFileId("l_phitbox");
WeaponPistHitbox.AddItem("Head");
WeaponPistHitbox.AddItem("Neck");
WeaponPistHitbox.AddItem("Chest");
WeaponPistHitbox.AddItem("Stomach");
WeaponPistHitbox.AddItem("Nearest");
weapongroup.PlaceLabledControl(2, "Hitbox", this, &WeaponPistHitbox);
WeaponPistSpeed.SetFileId("l_pspeed");
WeaponPistSpeed.SetBoundaries(0, 75);
WeaponPistSpeed.SetValue(5);
weapongroup.PlaceLabledControl(2, "Speed", this, &WeaponPistSpeed);
WeaponPistRecoil.SetFileId("l_pRecoil");
WeaponPistRecoil.SetBoundaries(0, 200);
WeaponPistRecoil.SetValue(165);
weapongroup.PlaceLabledControl(2, "Recoil", this, &WeaponPistRecoil);
WeaponPistFoV.SetFileId("l_pfov");
WeaponPistFoV.SetBoundaries(0, 45);
WeaponPistFoV.SetValue(10);
weapongroup.PlaceLabledControl(2, "Field Of View", this, &WeaponPistFoV);
// --- Sniper --- //
WeaponSnipHitbox.SetFileId("l_shitbox");
WeaponSnipHitbox.AddItem("Head");
WeaponSnipHitbox.AddItem("Neck");
WeaponSnipHitbox.AddItem("Chest");
WeaponSnipHitbox.AddItem("Stomach");
WeaponSnipHitbox.AddItem("Nearest");
weapongroup.PlaceLabledControl(3, "Hitbox", this, &WeaponSnipHitbox);
WeaponSnipSpeed.SetFileId("l_sspeed");
WeaponSnipSpeed.SetBoundaries(0, 75);
WeaponSnipSpeed.SetValue(5);
weapongroup.PlaceLabledControl(3, "Speed", this, &WeaponSnipSpeed);
WeaponSnipRecoil.SetFileId("l_sRecoil");
WeaponSnipRecoil.SetBoundaries(0, 200);
WeaponSnipRecoil.SetValue(165);
weapongroup.PlaceLabledControl(3, "Recoil", this, &WeaponSnipRecoil);
WeaponSnipFoV.SetFileId("l_sfov");
WeaponSnipFoV.SetBoundaries(0, 45);
WeaponSnipFoV.SetValue(10);
weapongroup.PlaceLabledControl(3, "Field Of View", this, &WeaponSnipFoV);
// --- SMG --- //
WeaponMpHitbox.SetFileId("l_sniphitbox");
WeaponMpHitbox.AddItem("Head");
WeaponMpHitbox.AddItem("Neck");
WeaponMpHitbox.AddItem("Chest");
WeaponMpHitbox.AddItem("Stomach");
WeaponMpHitbox.AddItem("Nearest");
weapongroup.PlaceLabledControl(4, "Hitbox", this, &WeaponMpHitbox);
WeaponMpSpeed.SetFileId("l_sspeed");
WeaponMpSpeed.SetBoundaries(0, 75);
WeaponMpSpeed.SetValue(5);
weapongroup.PlaceLabledControl(4, "Speed", this, &WeaponMpSpeed);
WeaponMpRecoil.SetFileId("l_sRecoil");
WeaponMpRecoil.SetBoundaries(0, 200);
WeaponMpRecoil.SetValue(165);
weapongroup.PlaceLabledControl(4, "Recoil", this, &WeaponMpRecoil);
WeaponMpFoV.SetFileId("l_sfov");
WeaponMpFoV.SetBoundaries(0, 45);
WeaponMpFoV.SetValue(10);
weapongroup.PlaceLabledControl(4, "Field Of View", this, &WeaponMpFoV);
// --- MachineGun --- //
WeaponMGHitbox.SetFileId("l_mghitbox");
WeaponMGHitbox.AddItem("Head");
WeaponMGHitbox.AddItem("Neck");
WeaponMGHitbox.AddItem("Chest");
WeaponMGHitbox.AddItem("Stomach");
WeaponMGHitbox.AddItem("Nearest");
weapongroup.PlaceLabledControl(5, "Hitbox", this, &WeaponMGHitbox);
WeaponMGSpeed.SetFileId("l_mgspeed");
WeaponMGSpeed.SetBoundaries(0, 75);
WeaponMGSpeed.SetValue(5);
weapongroup.PlaceLabledControl(5, "Speed", this, &WeaponMGSpeed);
WeaponMGRecoil.SetFileId("l_mgRecoil");
WeaponMGRecoil.SetBoundaries(0, 200);
WeaponMGRecoil.SetValue(165);
weapongroup.PlaceLabledControl(5, "Recoil", this, &WeaponMGRecoil);
WeaponMGFoV.SetFileId("l_mgfov");
WeaponMGFoV.SetBoundaries(0, 45);
WeaponMGFoV.SetValue(10);
weapongroup.PlaceLabledControl(5, "Field Of View", this, &WeaponMGFoV);
}
*/
void CVisualTab::Setup()
{

	SetTitle("D");
#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(4, 30);
	OptionsGroup.SetSize(280, 412);
	OptionsGroup.AddTab(CGroupTab("Main", 1));
	OptionsGroup.AddTab(CGroupTab("Filters", 2));
	OptionsGroup.AddTab(CGroupTab("Misc", 3));
	OptionsGroup.AddTab(CGroupTab("Other", 4));
	RegisterControl(&OptionsGroup);
	RegisterControl(&Active);

	Active.SetFileId("active");
	OptionsGroup.PlaceLabledControl(1, "Activate Visuals", this, &Active);

	OptionsBox.SetFileId("opt_box");
	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Default");
	OptionsBox.AddItem("Genuine");
	OptionsBox.AddItem("Corners");
	OptionsGroup.PlaceLabledControl(1, "Box", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	OptionsName.AddItem("Off");
	OptionsName.AddItem("Top");
	//	OptionsName.AddItem("Right");
	OptionsGroup.PlaceLabledControl(1, "Name", this, &OptionsName);

	OptionsHealth.SetFileId("opt_hp");
	OptionsHealth.AddItem("Off");
	OptionsHealth.AddItem("Default");
	OptionsHealth.AddItem("Battery");
	OptionsHealth.AddItem("Bottom");

	OptionsGroup.PlaceLabledControl(1, "Health", this, &OptionsHealth);

	OptionsArmor.SetFileId("otr_armor");
	OptionsArmor.AddItem("Off");
	OptionsArmor.AddItem("Default");
	OptionsArmor.AddItem("Battery");
	OptionsArmor.AddItem("Bottom");
	OptionsGroup.PlaceLabledControl(1, "Armor Bar", this, &OptionsArmor); // here

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl(1, "Info", this, &OptionsInfo);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl(1, "Skeleton", this, &OptionsSkeleton);

	Weapons.SetFileId("kysquecest");
	OptionsGroup.PlaceLabledControl(1, "Weapons", this, &Weapons);

	Ammo.SetFileId("urmomsucksass");
	OptionsGroup.PlaceLabledControl(1, "Ammo Bar", this, &Ammo);

	//	OffscreenESP.SetFileId("otr_offscreenESP");
	//	OptionsGroup.PlaceLabledControl(1, "Offscreen ESP", this, &OffscreenESP);

	GlowZ.SetFileId("opt_glowz");
	GlowZ.SetValue(0.f);
	GlowZ.SetBoundaries(0.f, 100.f);
	GlowZ.extension = XorStr("%%");
	OptionsGroup.PlaceLabledControl(1, "Enemy Glow", this, &GlowZ);

	team_glow.SetFileId("opt_team_glow");
	team_glow.SetValue(0.f);
	team_glow.SetBoundaries(0.f, 100.f);
	team_glow.extension = XorStr("%%");
	OptionsGroup.PlaceLabledControl(1, "Team Glow", this, &team_glow);

	Glowz_lcl.SetFileId("opt_glowz_lcl");
	Glowz_lcl.SetValue(0.f);
	Glowz_lcl.SetBoundaries(0.f, 100.f);
	Glowz_lcl.extension = XorStr("%%");
	OptionsGroup.PlaceLabledControl(1, "Local Glow", this, &Glowz_lcl);

	//	debug_esp.SetFileId("opt_debug_Esp");
	//	OptionsGroup.PlaceLabledControl(1, "Debug Esp", this, &debug_esp);

	//	FiltersAll.SetFileId("ftr_all");
	//	OptionsGroup.PlaceLabledControl(2, "All", this, &FiltersAll);

	show_players.SetFileId("ftr_players");
	show_players.SetState(true);
	OptionsGroup.PlaceLabledControl(2, "Players", this, &show_players);

	show_team.SetFileId("ftr_enemyonly");
	OptionsGroup.PlaceLabledControl(2, "Show Team", this, &show_team);

	FiltersNades.SetFileId("ftr_nades");
	OptionsGroup.PlaceLabledControl(2, "Nades", this, &FiltersNades);

	FiltersC4.SetFileId("ftr_c4");
	OptionsGroup.PlaceLabledControl(2, "C4", this, &FiltersC4);

	//	show_hostage.SetFileId("ftr_hostage");
	//	OptionsGroup.PlaceLabledControl(2, "Hostage", this, &show_hostage);

	optimize.SetFileId("ftr_optimize");
	OptionsGroup.PlaceLabledControl(2, "Optimize Graphics", this, &optimize);

	WeaponFilterName.SetFileId("ftr_weapon_toggle");
	OptionsGroup.PlaceLabledControl(2, "Dropped Weapon Name", this, &WeaponFilterName);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersWeapons.AddItem("Off");
	FiltersWeapons.AddItem("Default");
	FiltersWeapons.AddItem("Genuine");
	FiltersWeapons.AddItem("Corners");
	OptionsGroup.PlaceLabledControl(2, "Dropped Weapon Box", this, &FiltersWeapons);

	//	asus_type.SetFileId("asus_wall_type");
	//	asus_type.AddItem("Props Only");
	//	asus_type.AddItem("Walls and Props");
	//	OptionsGroup.PlaceLabledControl(2, "Asus Type", this, &asus_type);


	//----------------------------------------------//


	SpreadCrosshair.SetFileId(XorStr("v_spreadcrosshair"));
	SpreadCrosshair.AddItem("Off");
	SpreadCrosshair.AddItem("Standard");
	SpreadCrosshair.AddItem("Colour");
	SpreadCrosshair.AddItem("Rainbow");
	SpreadCrosshair.AddItem("Rainbow Rotate");
	OptionsGroup.PlaceLabledControl(3, XorStr("Spread crosshair"), this, &SpreadCrosshair);

	/*	SpreadCrossSize.SetFileId("otr_spreadcross_size");
	SpreadCrossSize.SetBoundaries(1.f, 100.f); //we should take smth like 650 as max so i guess *6.5?
	SpreadCrossSize.extension = XorStr("%%");
	SpreadCrossSize.SetValue(45.f);
	OptionsGroup.PlaceLabledControl(3, "Size", this, &SpreadCrossSize); */

	crosshair.SetFileId("otr_crosshair");
	OptionsGroup.PlaceLabledControl(3, "Crosshair", this, &crosshair);

	//	DamageIndicator.SetFileId("otr_btracers");
	//	OptionsGroup.PlaceLabledControl(3, "Damage Indicator", this, &DamageIndicator);

	OtherNoScope.SetFileId("otr_noscope");
	OptionsGroup.PlaceLabledControl(3, "Remove scope", this, &OtherNoScope);

	RemoveZoom.SetFileId("otr_remv_zoom");
	OptionsGroup.PlaceLabledControl(3, "Remove zoom", this, &RemoveZoom);

	OtherNoFlash.SetFileId("otr_noflash");
	OptionsGroup.PlaceLabledControl(3, "Remove flash effect", this, &OtherNoFlash);

	flashAlpha.SetFileId("otr_stolen_from_punknown_muahahaha");
	flashAlpha.SetBoundaries(0, 100);
	flashAlpha.extension = XorStr("%%");
	flashAlpha.SetValue(10);
	OptionsGroup.PlaceLabledControl(3, "Flash Alpha", this, &flashAlpha);

	nosmoke.SetFileId("otr_nosmoke");
	OptionsGroup.PlaceLabledControl(3, "Remove smoke", this, &nosmoke);

	/*	nosmoke_slider.SetFileId("otr_nosmoke_alpha");
	nosmoke_slider.SetBoundaries(0, 100);
	nosmoke_slider.extension = ("%%");
	nosmoke_slider.SetValue(10);
	OptionsGroup.PlaceLabledControl(3, "Smoke Alpha", this, &nosmoke_slider); */


	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OptionsGroup.PlaceLabledControl(3, "No visual recoil", this, &OtherNoVisualRecoil);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(30.f, 120.f);
	OtherViewmodelFOV.SetValue(90.f);
	OptionsGroup.PlaceLabledControl(3, "Viewmodel fov", this, &OtherViewmodelFOV);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 50.f);
	OtherFOV.SetValue(0.f);
	OptionsGroup.PlaceLabledControl(3, "Override fov", this, &OtherFOV);

	override_viewmodel.SetFileId("otr_override_viewmodel_offset");
	OptionsGroup.PlaceLabledControl(3, "Override Viewmodel Offset", this, &override_viewmodel);

	offset_x.SetFileId("otr_offset_x");
	offset_x.SetBoundaries(-12, 12);
	offset_x.SetValue(2.5);
	OptionsGroup.PlaceLabledControl(3, "Offset X", this, &offset_x);

	offset_y.SetFileId("otr_offset_Y");
	offset_y.SetBoundaries(-12, 12);
	offset_y.SetValue(2.0);
	OptionsGroup.PlaceLabledControl(3, "Offset Y", this, &offset_y);

	offset_z.SetFileId("otr_offset_z");
	offset_z.SetBoundaries(-12, 12);
	offset_z.SetValue(-2.0);
	OptionsGroup.PlaceLabledControl(3, "Offset z", this, &offset_z);


	/*
	beamtime.SetFileId("otr_beamtime");
	beamtime.SetBoundaries(1.f, 5.f);
	beamtime.SetValue(2.f);
	OptionsGroup.PlaceLabledControl(4, "Beam Time", this, &beamtime);
	*/
	LBYIndicator.SetFileId("otr_LBYIndicator");
	LBYIndicator.AddItem("Off");
	LBYIndicator.AddItem("Classic");
	LBYIndicator.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(4, "LBY Indicator", this, &LBYIndicator);

	LCIndicator.SetFileId("otr_LCIndicator");
	LCIndicator.AddItem("Off");
	LCIndicator.AddItem("Classic");
	LCIndicator.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(4, "LagComp Indicator", this, &LCIndicator);

	FakeDuckIndicator.SetFileId("otr_FakeDuckIndicator");
	FakeDuckIndicator.AddItem("Off");
	FakeDuckIndicator.AddItem("Classic");
	FakeDuckIndicator.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(4, "FakeDuck Indicator", this, &FakeDuckIndicator);

	fake_indicator.SetFileId("otr_desync_indicator");
	fake_indicator.AddItem("Off");
	fake_indicator.AddItem("Classic");
	fake_indicator.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(4, "Desync Indicator", this, &fake_indicator);

	manualaa_type.SetFileId("manualaa");
	manualaa_type.AddItem("Off");
	manualaa_type.AddItem("Single Arrow");
	manualaa_type.AddItem("All Arrows");
	OptionsGroup.PlaceLabledControl(4, "Manual Indicator", this, &manualaa_type); // requested by: https://steamcommunity.com/id/9a-

	killfeed.SetFileId("otr_killfeed");
	OptionsGroup.PlaceLabledControl(4, "Preserve Killfeed", this, &killfeed); // requested by: https://steamcommunity.com/id/B1GN1GNN

	cheatinfo.SetFileId("cheatinfox");
	OptionsGroup.PlaceLabledControl(4, "Debug Info", this, &cheatinfo);

	//	CompRank.SetFileId("otr_reveal__rank");
	//	OptionsGroup.PlaceLabledControl(4, "Rank Reveal", this, &CompRank);

	OtherEntityGlow.SetFileId("otr_world_ent_glow");
	OptionsGroup.PlaceLabledControl(4, "World Entity Glow", this, &OtherEntityGlow);

	OtherHitmarker.SetFileId("otr_hitmarker");
	OptionsGroup.PlaceLabledControl(4, "Hitmarker", this, &OtherHitmarker);

	watermark.SetFileId("otr_watermark");
	watermark.SetState(true);
	OptionsGroup.PlaceLabledControl(4, "Watermark", this, &watermark);

//	bulletbeam.SetFileId("otr_bullet_beam");
//	OptionsGroup.PlaceLabledControl(4, "Bullet Beam", this, &bulletbeam);

	logs.SetFileId("otr_skeetpaste");
	logs.AddItem("Off");
	logs.AddItem("Default");
	logs.AddItem("Coloured");
	OptionsGroup.PlaceLabledControl(4, "Event Log", this, &logs);

	/*	BulletTrace.SetFileId("otr_bullet_tracers_local");
	OptionsGroup.PlaceLabledControl(4, "Local Bullet Tracers", this, &BulletTrace);

	BulletTrace_enemy.SetFileId("otr_bullet_tracers_enemy");
	OptionsGroup.PlaceLabledControl(4, "Enemy Bullet Tracers", this, &BulletTrace_enemy); */
	//----------------------------------------------//

	ChamsGroup.SetText("Chams");
	ChamsGroup.AddTab(CGroupTab("Player", 1));
	ChamsGroup.AddTab(CGroupTab("Viewmodel", 2));
	ChamsGroup.AddTab(CGroupTab("Mods", 3));
	ChamsGroup.SetPosition(294, 30); // 225, 30
	ChamsGroup.SetSize(260, 243);
	RegisterControl(&ChamsGroup);


	ChamsEnemy.SetFileId("chams_enenmy_selection");
	ChamsEnemy.AddItem("Off");
	ChamsEnemy.AddItem("Visible Only");
	ChamsEnemy.AddItem("Always");
	ChamsGroup.PlaceLabledControl(1, "Enemies", this, &ChamsEnemy); // *1

	ChamsTeamVis.SetFileId("chams_team_selection");
	ChamsTeamVis.AddItem("Off");
	ChamsTeamVis.AddItem("Visible Only");
	ChamsTeamVis.AddItem("Always");
	ChamsGroup.PlaceLabledControl(1, "Team", this, &ChamsTeamVis);

	visible_chams_type.SetFileId("otr_visiblechams_type");
	visible_chams_type.AddItem("Normal");
	visible_chams_type.AddItem("Flat"); // like my ex :)
	visible_chams_type.AddItem("Pulse");
	visible_chams_type.AddItem("Crystal");
	visible_chams_type.AddItem("Glass");
	ChamsGroup.PlaceLabledControl(1, "Visible Chams Type", this, &visible_chams_type);

	invisible_chams_type.SetFileId("otr_invisiblechams_type");
	invisible_chams_type.AddItem("Normal");
	invisible_chams_type.AddItem("Flat"); // i'm still sad about that :(
	invisible_chams_type.AddItem("Pulse");
	invisible_chams_type.AddItem("Crystal");
	invisible_chams_type.AddItem("Glass");
	ChamsGroup.PlaceLabledControl(1, "Invisible Chams Type", this, &invisible_chams_type);

	ChamsLocal.SetFileId("chams_local");
	ChamsGroup.PlaceLabledControl(1, "Apply Local Chams", this, &ChamsLocal);

	fakelag_ghost.SetFileId("chams_desyncyaw");
	ChamsGroup.PlaceLabledControl(1, "Server Angle Ghost", this, &fakelag_ghost);

	localmaterial.SetFileId("esp_localscopedmat");
	localmaterial.AddItem("Default");
	localmaterial.AddItem("Clear");
	localmaterial.AddItem("Cham");
	localmaterial.AddItem("Wireframe");
	localmaterial.AddItem("LSD");
	localmaterial.AddItem("Glass");
	localmaterial.AddItem("Pulse");
	ChamsGroup.PlaceLabledControl(2, "Scoped Materials", this, &localmaterial);

	HandCHAMS.SetFileId("chams_local_hand");
	HandCHAMS.AddItem("off");
	HandCHAMS.AddItem("Simple");
	HandCHAMS.AddItem("Wireframe");
	HandCHAMS.AddItem("Golden");
	HandCHAMS.AddItem("Glass");
	HandCHAMS.AddItem("Crystal");
	HandCHAMS.AddItem("Pulse");
	ChamsGroup.PlaceLabledControl(2, "Hand Chams", this, &HandCHAMS);

	GunCHAMS.SetFileId("chams_local_weapon");
	GunCHAMS.AddItem("off");
	GunCHAMS.AddItem("Simple");
	GunCHAMS.AddItem("Wireframe");
	GunCHAMS.AddItem("Golden");
	GunCHAMS.AddItem("Glass");
	GunCHAMS.AddItem("Crystal");
	GunCHAMS.AddItem("Pulse");
	ChamsGroup.PlaceLabledControl(2, "Weapon Chams", this, &GunCHAMS);

	SleeveChams.SetFileId("remove_Sleeve");
	SleeveChams.AddItem("off");
	SleeveChams.AddItem("Simple");
	SleeveChams.AddItem("Wireframe");
	SleeveChams.AddItem("Golden");
	SleeveChams.AddItem("Glass");
	SleeveChams.AddItem("Crystal");
	SleeveChams.AddItem("Pulse");
	SleeveChams.AddItem("Invisible");
	ChamsGroup.PlaceLabledControl(2, "Sleeve Chams", this, &SleeveChams);

	/*
	fakelag_ghost.SetFileId("otr_fakelag_ghost");
	fakelag_ghost.AddItem("Off");
	fakelag_ghost.AddItem("Default");
	fakelag_ghost.AddItem("Pulse");
	ChamsGroup.PlaceLabledControl(2, "Fake Lag Ghost", this, &fakelag_ghost);
	*/


	transparency.SetFileId("esp_transparency");
	transparency.SetBoundaries(0, 100);
	transparency.SetValue(20);
	transparency.extension = XorStr("%%");
	ChamsGroup.PlaceLabledControl(3, "Scoped Transparency", this, &transparency);


	hand_transparency.SetFileId("esp_hand_transparency");
	hand_transparency.SetBoundaries(0, 100);
	hand_transparency.SetValue(20);
	hand_transparency.extension = XorStr("%%");
	ChamsGroup.PlaceLabledControl(3, "Arm Transparency", this, &hand_transparency);

	gun_transparency.SetFileId("esp_gun_transparency");
	gun_transparency.SetBoundaries(0, 100);
	gun_transparency.SetValue(20);
	gun_transparency.extension = XorStr("%%");
	ChamsGroup.PlaceLabledControl(3, "Gun Transparency", this, &gun_transparency);

	sleeve_transparency.SetFileId("esp_sleeve_transparency");
	sleeve_transparency.SetBoundaries(0, 100);
	sleeve_transparency.SetValue(20);
	sleeve_transparency.extension = XorStr("%%");
	ChamsGroup.PlaceLabledControl(3, "Sleeve Transparency", this, &sleeve_transparency);

	blend_local.SetFileId("esp_teamblend");
	blend_local.SetBoundaries(0, 100);
	blend_local.SetValue(75);
	blend_local.extension = XorStr("%%");
	ChamsGroup.PlaceLabledControl(3, "Local Player Chams Blend", this, &blend_local);

	// think about you and I still can't focus

	//	 </3 

	// so I leave behind my feelings and emotions

	worldgroup.SetText("World");
	worldgroup.AddTab(CGroupTab("Main", 1));
	worldgroup.AddTab(CGroupTab("Mods", 2));
	worldgroup.SetPosition(294, 280);  // 225, 285
	worldgroup.SetSize(260, 162);
	RegisterControl(&worldgroup);
	colmodupdate.SetFileId("otr_night");
	worldgroup.PlaceLabledControl(1, "Force update Materials", this, &colmodupdate); //you could've just made this a button lol

	customskies.SetFileId("otr_skycustom");
	customskies.AddItem("Default");
	customskies.AddItem("Night");
	customskies.AddItem("NoSky");
	customskies.AddItem("Galaxy");

	worldgroup.PlaceLabledControl(1, "Change Sky", this, &customskies);

	colmod.SetFileId("night_amm");
	colmod.SetBoundaries(000.000f, 100.00f);
	colmod.extension = XorStr("%%");
	colmod.SetValue(020.0f);
	worldgroup.PlaceLabledControl(1, "Brightness percentage", this, &colmod);

	asusamount.SetFileId("otr_asusprops");
	asusamount.SetBoundaries(1.f, 100.f);
	asusamount.extension = XorStr("%%");
	asusamount.SetValue(95.f);
	worldgroup.PlaceLabledControl(1, "Asus percantage", this, &asusamount);

	ModulateSkyBox.SetFileId(XorStr("sky_box_color_enable"));
	worldgroup.PlaceLabledControl(2, XorStr("Enable Sky Color Changer"), this, &ModulateSkyBox);

	sky_r.SetFileId("sky_r");
	sky_r.SetBoundaries(0.f, 25.f);
	sky_r.SetValue(10.f);
	worldgroup.PlaceLabledControl(2, "Sky: Red", this, &sky_r);

	sky_g.SetFileId("sky_g");
	sky_g.SetBoundaries(0.f, 25.f);
	sky_g.SetValue(1.f);
	worldgroup.PlaceLabledControl(2, "Sky: Green", this, &sky_g);

	sky_b.SetFileId("sky_b");
	sky_b.SetBoundaries(0.f, 25.f);
	sky_b.SetValue(20.f);
	worldgroup.PlaceLabledControl(2, "Sky: Blue", this, &sky_b);

#pragma endregion Setting up the Other controls
}
void CMiscTab::Setup()
{
	/*
	__  __ _____  _____  _____
	|  \/  |_   _|/ ____|/ ____|
	| \  / | | | | (___ | |
	| |\/| | | |  \___ \| |
	| |  | |_| |_ ____) | |____
	|_|  |_|_____|_____/ \_____|


	*/
	SetTitle("C");
#pragma region Other
	OtherGroup.SetText("Other");
	OtherGroup.AddTab(CGroupTab("Main", 1));
	OtherGroup.AddTab(CGroupTab("BuyBot", 2));
	OtherGroup.SetPosition(4, 30);
	OtherGroup.SetSize(270, 400);
	RegisterControl(&OtherGroup);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.AddItem("Anti Untrusted");
	OtherSafeMode.AddItem("Anti VAC Kick");
	OtherSafeMode.AddItem("Danger Zone");

	OtherSafeMode.AddItem("Unrestricted (!)");
	OtherGroup.PlaceLabledControl(1, "Safety Mode", this, &OtherSafeMode);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl(1, "BunnyHop", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherGroup.PlaceLabledControl(1, "Air Strafe", this, &OtherAutoStrafe);

	hitmarker_sound.SetFileId("hitmarker_sound");
	hitmarker_sound.AddItem("Off");
	hitmarker_sound.AddItem("Cod");
	hitmarker_sound.AddItem("ArenaSwitch");
	hitmarker_sound.AddItem("Bubble");
	hitmarker_sound.AddItem("Bameware");
	hitmarker_sound.AddItem("Anime");
	hitmarker_sound.AddItem("Hitler"); // this is for CruZZ and Crytec. Heil hitler
	hitmarker_sound.AddItem("Bell"); // diiiing	
									 //	hitmarker_sound.AddItem("Light Switch");
	hitmarker_sound.AddItem("User Custom (csgo sound folder)");
	OtherGroup.PlaceLabledControl(1, "Hitmarker Sound", this, &hitmarker_sound);

	OtherThirdperson.SetFileId("aa_1thirpbind");
	OtherGroup.PlaceLabledControl(1, "Thirdperson", this, &OtherThirdperson);
	ThirdPersonKeyBind.SetFileId("aa_thirpbind");
	OtherGroup.PlaceLabledControl(1, "", this, &ThirdPersonKeyBind);

	Radar.SetFileId("Radar");
	OtherGroup.PlaceLabledControl(1, "Draw Radar", this, &Radar);

	RadarX.SetFileId("misc_radar_xax1");
	RadarX.SetBoundaries(0, 1920);
	RadarX.SetValue(0);
	OtherGroup.PlaceLabledControl(1, "X-Axis", this, &RadarX);

	RadarY.SetFileId("misc_radar_yax2");
	RadarY.SetBoundaries(0, 1080);
	RadarY.SetValue(0);
	OtherGroup.PlaceLabledControl(1, "Y-Axis", this, &RadarY);

	autojump_type.SetFileId("misc_autojump_type");
	autojump_type.AddItem("Normal");
	autojump_type.AddItem("Always On");
	OtherGroup.PlaceLabledControl(1, "BunnyHop Type", this, &autojump_type);

	killsay.SetFileId("misc_killsay");
	killsay.items.push_back(dropdownboxitem(false, XorStr("On Kill")));
	killsay.items.push_back(dropdownboxitem(false, XorStr("On Death")));
	OtherGroup.PlaceLabledControl(1, "Trash Talk", this, &killsay);

	NameChanger.SetFileId("misc_NameChanger");
	OtherGroup.PlaceLabledControl(1, "Name Stealer", this, &NameChanger);

	infinite_duck.SetFileId("infinteduck");
	OtherGroup.PlaceLabledControl(1, "No Crouch Delay", this, &infinite_duck);

	buybot_primary.SetFileId("buybot_primary");
	buybot_primary.AddItem("Off");
	buybot_primary.AddItem("Auto-Sniper");
	buybot_primary.AddItem("Scout (ssg08)");
	buybot_primary.AddItem("Awp");
	buybot_primary.AddItem("Ak-47 / M4");
	buybot_primary.AddItem("Aug / Sg553");
	buybot_primary.AddItem("Mp9 / Mac10");
	OtherGroup.PlaceLabledControl(2, "Primary Weapon", this, &buybot_primary);

	buybot_secondary.SetFileId("buybot_secondary");
	buybot_secondary.AddItem("Off");
	buybot_secondary.AddItem("Dual Berretas");
	buybot_secondary.AddItem("Revolver / Desert Eagle");
	buybot_secondary.AddItem("Five-Seven / Cz75");
	OtherGroup.PlaceLabledControl(2, "Secondary Weapon", this, &buybot_secondary);

	buybot_otr.SetFileId("buybot_other");
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("kevlar")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("he-grenade")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("flashbang")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("smoke grenade")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("molotov")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("zeus")));
	buybot_otr.items.push_back(dropdownboxitem(false, XorStr("defuse-kit")));
	OtherGroup.PlaceLabledControl(2, "Others", this, &buybot_otr);

	AntiAimGroup.SetPosition(286, 30);
	AntiAimGroup.SetText("Anti-Aim");
	AntiAimGroup.SetSize(270, 400);
	AntiAimGroup.AddTab(CGroupTab("Main", 1));
	AntiAimGroup.AddTab(CGroupTab("Modifiers", 2));
	AntiAimGroup.AddTab(CGroupTab("Misc", 3));
	AntiAimGroup.AddTab(CGroupTab("Desync", 4));
	RegisterControl(&AntiAimGroup);
	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl(1, "Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Up");
	AntiAimPitch.AddItem("Jitter");
	AntiAimPitch.AddItem("Random");
	AntiAimPitch.AddItem("Zero");
	AntiAimGroup.PlaceLabledControl(1, "Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Backward");
	AntiAimYaw.AddItem("Manual");
	AntiAimYaw.AddItem("Crooked");
	AntiAimYaw.AddItem("Freestanding");
	AntiAimYaw.AddItem("Jitter 180");
	AntiAimYaw.AddItem("Random Lowerbody");
	//	AntiAimYaw.AddItem("Twist");
	AntiAimGroup.PlaceLabledControl(1, "Standing Yaw", this, &AntiAimYaw);

	stand_jitter.SetFileId("c_addjitter_stand");
	stand_jitter.SetBoundaries(0.f, 90.f);
	stand_jitter.SetValue(0.f);
	AntiAimGroup.PlaceLabledControl(1, "Add Jitter", this, &stand_jitter);

	AntiAimYawrun.SetFileId("aa_y2");
	AntiAimYawrun.AddItem("Off");
	AntiAimYawrun.AddItem("Backward");
	AntiAimYawrun.AddItem("Manual");
	AntiAimYawrun.AddItem("Crooked");
	AntiAimYawrun.AddItem("Freestanding");
	AntiAimYawrun.AddItem("180 Jitter");
	AntiAimYawrun.AddItem("Random Lowerbody");
	AntiAimGroup.PlaceLabledControl(1, "Moving Yaw", this, &AntiAimYawrun);

	move_jitter.SetFileId("c_addjitter_move");
	move_jitter.SetBoundaries(0.f, 90.f);
	move_jitter.SetValue(0.f);
	AntiAimGroup.PlaceLabledControl(1, "Add Jitter", this, &move_jitter);

	AntiAimYaw3.SetFileId("aa_y3");
	AntiAimYaw3.AddItem("Off");
	AntiAimYaw3.AddItem("Backward");
	AntiAimYaw3.AddItem("Manual");
	AntiAimYaw3.AddItem("Crooked");
	AntiAimYaw3.AddItem("Freestanding");
	AntiAimYaw3.AddItem("180 Jitter");
	AntiAimYaw3.AddItem("Random Lowerbody");
	AntiAimGroup.PlaceLabledControl(1, "InAir Yaw", this, &AntiAimYaw3);

	desync_aa_stand.SetFileId("v_desync_aa_stand");
	AntiAimGroup.PlaceLabledControl(1, "Standing Desync", this, &desync_aa_stand);

	desync_aa_move.SetFileId("v_desync_aa_move");
	AntiAimGroup.PlaceLabledControl(1, "Moving Desync", this, &desync_aa_move);

	//	air_desync.SetFileId("v_air_desync");
	//	AntiAimGroup.PlaceLabledControl(1, "Air Desync", this, &air_desync);

	//	pitch_up.SetFileId("pitch_up");
	//	AntiAimGroup.PlaceLabledControl(1, "Pitch Flick", this, &pitch_up);


	minimal_walk.SetFileId("minimal_walk");
	AntiAimGroup.PlaceLabledControl(1, "Minimal Walk Key", this, &minimal_walk);

	antilby.SetFileId("otr_meh");
	//	antilby.AddItem("Off");
	//	antilby.AddItem("One Flick");
	//	antilby.AddItem("Two Flicks");
	//	antilby.AddItem("Relative");
	AntiAimGroup.PlaceLabledControl(1, "Anti-LBY", this, &antilby);

	disable_on_dormant.SetFileId("disable_on_dormant");
	AntiAimGroup.PlaceLabledControl(1, "Disable On Dormant", this, &disable_on_dormant);

	//	BreakLBYDelta2.SetFileId("b_antilby2");
	//	BreakLBYDelta2.SetBoundaries(-180, 180);
	//	BreakLBYDelta2.SetValue(90);
	//	AntiAimGroup.PlaceLabledControl(2, "Anti-LBY First Flick", this, &BreakLBYDelta2);

	//	BreakLBYDelta.SetFileId("b_antilby");
	//	BreakLBYDelta.SetBoundaries(-180, 180);
	//	BreakLBYDelta.SetValue(-90);
	//	AntiAimGroup.PlaceLabledControl(1, "Anti-LBY Range", this, &BreakLBYDelta);

	//	freerange.SetFileId("freestanding_range");
	//	freerange.SetBoundaries(0, 90);
	//	freerange.SetValue(35);
	//	AntiAimGroup.PlaceLabledControl(1, "Freestanding Value", this, &freerange);

	//preset_aa.SetFileId("preset_aa");
	//AntiAimGroup.PlaceLabledControl(1, "Pre-set AntiAim", this, &preset_aa);

	//choked_shot.SetFileId("choke_shot");
	//AntiAimGroup.PlaceLabledControl(1, "Choke Shot", this, &choked_shot);
	//-<------------------------------------->-//

	FakelagStand.SetFileId("fakelag_stand_val");
	FakelagStand.SetBoundaries(1, 14);
	FakelagStand.SetValue(1);
	AntiAimGroup.PlaceLabledControl(2, "Fakelag Standing", this, &FakelagStand);

	FakelagMove.SetFileId("fakelag_move_val");
	FakelagMove.SetBoundaries(1, 14);
	FakelagMove.SetValue(1);
	AntiAimGroup.PlaceLabledControl(2, "Fakelag Moving", this, &FakelagMove);

	Fakelagjump.SetFileId("fakelag_jump_val");
	Fakelagjump.SetBoundaries(1, 14);
	Fakelagjump.SetValue(1);
	AntiAimGroup.PlaceLabledControl(2, "Fakelag in Air", this, &Fakelagjump);

	fl_spike.SetFileId("fakelag_spike");
	fl_spike.AddItem("Default");
	fl_spike.AddItem("Enemy Sight");
	fl_spike.AddItem("Mirror Adaptive");
	fl_spike.AddItem("Aimware Adaptive");
	fl_spike.AddItem("Velocity Based");
	AntiAimGroup.PlaceLabledControl(2, "Fakelag Factor", this, &fl_spike);

	fakelag_key.SetFileId("fakelag_onkey");
	AntiAimGroup.PlaceLabledControl(2, "Fakelag Spike Key", this, &fakelag_key);

	FakelagBreakLC.SetFileId("fakelag_breaklc");
	AntiAimGroup.PlaceLabledControl(2, "Break Lag Compensation", this, &FakelagBreakLC);

	//	auto_fakelag.SetFileId("fakelag_auto");
	//	AntiAimGroup.PlaceLabledControl(2, "Dynamic Fakelag", this, &auto_fakelag);

	manualleft.SetFileId("otr_keybasedleft");
	AntiAimGroup.PlaceLabledControl(3, "Manual Right", this, &manualleft);

	manualright.SetFileId("otr_keybasedright");
	AntiAimGroup.PlaceLabledControl(3, "Manual Left", this, &manualright);

	manualback.SetFileId("otr_keybasedback");
	AntiAimGroup.PlaceLabledControl(3, "Manual Back", this, &manualback);

	manualfront.SetFileId("otr_manualfrontk");
	AntiAimGroup.PlaceLabledControl(3, "Manual Front", this, &manualfront);

	fw.SetFileId("fakewalk_key");
	AntiAimGroup.PlaceLabledControl(3, "FakeWalk Key", this, &fw);


	randlbyr.SetFileId("b_randlbyr");
	randlbyr.SetBoundaries(20, 180);
	randlbyr.SetValue(60);
	AntiAimGroup.PlaceLabledControl(3, "Random Lowerbody Ammount", this, &randlbyr);


	fake_crouch.SetFileId("fake_crouch");
	AntiAimGroup.PlaceLabledControl(3, "Fake Crouch", this, &fake_crouch);

	fake_crouch_key.SetFileId("fake_crouch_key");
	AntiAimGroup.PlaceLabledControl(3, "Fake Crouch Key", this, &fake_crouch_key);

	QuickStop.SetFileId("acc_quickstop");
	QuickStop.AddItem("Off");
	QuickStop.AddItem("Default");
	QuickStop.AddItem("Slow Walk");
	AntiAimGroup.PlaceLabledControl(3, "Quickstop", this, &QuickStop);

	QuickCrouch.SetFileId("acc_quickcrouch");
	AntiAimGroup.PlaceLabledControl(3, "Quickcrouch", this, &QuickCrouch);
	//-<------------------------------------->-//

	desync_type_stand.SetFileId("desync_type_stand");
	desync_type_stand.AddItem("Static");
	desync_type_stand.AddItem("Jitter");
	desync_type_stand.AddItem("Manual Stretch (real yaw override)");
	desync_type_stand.AddItem("Extend"); // gangster
	AntiAimGroup.PlaceLabledControl(4, "Standing Type", this, &desync_type_stand);

	//	desync_range_stand.SetFileId("desync_range_standing");
	//	desync_range_stand.SetBoundaries(0, 58);
	//	desync_range_stand.SetValue(40);
	//	AntiAimGroup.PlaceLabledControl(4, "Standing Range", this, &desync_range_stand);

	//	desync_swapsides_stand.SetFileId("desync_swapsides_stand");
	//	AntiAimGroup.PlaceLabledControl(4, "Swap Sides", this, &desync_swapsides_stand);

	desync_type_move.SetFileId("desync_type_moving");
	desync_type_move.AddItem("Static");
	desync_type_move.AddItem("Jitter");
	desync_type_move.AddItem("Manual Stretch (real yaw override)");
	desync_type_move.AddItem("Extend");
	AntiAimGroup.PlaceLabledControl(4, "Moving Type", this, &desync_type_move);

	//	desync_range_move.SetFileId("desync_range_move");
	//	desync_range_move.SetBoundaries(0, 58);
	//	desync_range_move.SetValue(40);
	//	AntiAimGroup.PlaceLabledControl(4, "Moving Range", this, &desync_range_move);

	desync_twist_onshot.SetFileId("desync_twist_onshot");
	AntiAimGroup.PlaceLabledControl(4, "Desync On Shot", this, &desync_twist_onshot);
}

void CSkinTab::Setup()
{
	SetTitle("B");
	knifegroup.SetText("Knives");
	knifegroup.SetPosition(4, 30);
	//	knifegroup.AddTab(CGroupTab("Terrorist", 1));
	//	knifegroup.AddTab(CGroupTab("Counter-Terrorist", 2));
	knifegroup.SetSize(270, 170);
	RegisterControl(&knifegroup);

#pragma region Terrorist ( to be replaced with team based stuff )
	t_knife_index.SetFileId("t_knife_index");
	t_knife_index.AddItem("Off");
	t_knife_index.AddItem("Bayonet");
	t_knife_index.AddItem("M9 Bayonet");
	t_knife_index.AddItem("Butterfly");
	t_knife_index.AddItem("Flip");
	t_knife_index.AddItem("Gut");
	t_knife_index.AddItem("Karambit");
	t_knife_index.AddItem("Huntsman");
	t_knife_index.AddItem("Falchion");
	t_knife_index.AddItem("Bowie");
	t_knife_index.AddItem("Shadow");
	t_knife_index.AddItem("Talon");
	t_knife_index.AddItem("Stiletto");
	t_knife_index.AddItem("Ursus");
	knifegroup.PlaceLabledControl(0, XorStr("Model"), this, &t_knife_index);

	t_knife_wear.SetFileId("t_knife_seed");
	t_knife_wear.SetBoundaries(1, 100);
	t_knife_wear.SetValue(1);
	t_knife_wear.extension = XorStr("%%");
	knifegroup.PlaceLabledControl(0, XorStr("Wear"), this, &t_knife_wear);

	t_knife_skin_id.SetFileId("t_knife_skin");
	t_knife_skin_id.AddItem("Default");
	t_knife_skin_id.AddItem("Ruby");
	t_knife_skin_id.AddItem("Sapphire");
	t_knife_skin_id.AddItem("Black Pearl");
	t_knife_skin_id.AddItem("Doppler");
	t_knife_skin_id.AddItem("Fade");
	t_knife_skin_id.AddItem("Marble Fade");
	t_knife_skin_id.AddItem("Gamma Doppler");
	t_knife_skin_id.AddItem("Emerald");
	t_knife_skin_id.AddItem("Slaughter");
	t_knife_skin_id.AddItem("Whiteout");
	t_knife_skin_id.AddItem("Ultraviolet");
	t_knife_skin_id.AddItem("Lore (M9)");
	knifegroup.PlaceLabledControl(0, XorStr("Skin"), this, &t_knife_skin_id);


	//-----------------
	/*
	snipergroup.SetText("Snipers");
	snipergroup.SetPosition(284, 30);
	snipergroup.AddTab(CGroupTab("Auto", 1));
	snipergroup.AddTab(CGroupTab("Bolt Action", 2));
	snipergroup.SetSize(270, 170);
	RegisterControl(&snipergroup);

	t_sniperSCAR_skin_id.SetFileId("t_scar20_skin");
	t_sniperSCAR_skin_id.AddItem("Default");
	t_sniperSCAR_skin_id.AddItem("Crimson Web");
	t_sniperSCAR_skin_id.AddItem("Splash Jam");
	t_sniperSCAR_skin_id.AddItem("Emerald");
	t_sniperSCAR_skin_id.AddItem("Cardiac");
	t_sniperSCAR_skin_id.AddItem("Cyrex");
	t_sniperSCAR_skin_id.AddItem("Whiteout");
	t_sniperSCAR_skin_id.AddItem("The Fuschia Is Now");
	snipergroup.PlaceLabledControl(1, XorStr("Scar20"), this, &t_sniperSCAR_skin_id);

	t_sniperSCAR_wear.SetFileId("t_sniperSCAR_wear");
	t_sniperSCAR_wear.SetBoundaries(1, 100);
	t_sniperSCAR_wear.SetValue(1);
	t_sniperSCAR_wear.extension = XorStr("%%");
	snipergroup.PlaceLabledControl(1, XorStr("Wear"), this, &t_sniperSCAR_wear);

	// --



	// --

	t_sniperAWP_skin_id.SetFileId("t_AWP_skin");
	t_sniperAWP_skin_id.AddItem("Default");
	t_sniperAWP_skin_id.AddItem("Dragon Lore");
	t_sniperAWP_skin_id.AddItem("Pink DDPAT");
	t_sniperAWP_skin_id.AddItem("Asiimov");
	t_sniperAWP_skin_id.AddItem("Redline");
	t_sniperAWP_skin_id.AddItem("Medusa");
	t_sniperAWP_skin_id.AddItem("Hyper Beast");
	t_sniperAWP_skin_id.AddItem("Whiteout");
	snipergroup.PlaceLabledControl(3, XorStr("Skin"), this, &t_sniperAWP_skin_id);

	t_sniperAWP_wear.SetFileId("t_sniperAWP_wear");
	t_sniperAWP_wear.SetBoundaries(1, 100);
	t_sniperAWP_wear.SetValue(1);
	t_sniperAWP_wear.extension = XorStr("%%");
	snipergroup.PlaceLabledControl(3, XorStr("Wear"), this, &t_sniperAWP_wear);
	*/
}

void CColorTab::Setup()
{
	SetTitle("G");
#pragma region Visual Colors
	ColorsGroup.SetText("Settings");
	ColorsGroup.SetPosition(4, 30);
	ColorsGroup.AddTab(CGroupTab("ESP", 1));
	ColorsGroup.AddTab(CGroupTab("ESP 2", 2));
	ColorsGroup.AddTab(CGroupTab("Menu 1", 3));
	ColorsGroup.AddTab(CGroupTab("Menu 2", 4));
	//	ColorsGroup.AddTab(CGroupTab("Misc", 6));
	ColorsGroup.SetSize(270, 335);
	RegisterControl(&ColorsGroup);

	/*---------------------- COL ----------------------*/
	/*---------------------- COL ----------------------*/
	/*---------------------- COL ----------------------*/

	NameCol.SetFileId(XorStr("player_espname_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Name"), this, &NameCol);

	BoxCol.SetFileId(XorStr("player_espbox_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Box"), this, &BoxCol);;

	Skeleton.SetFileId(XorStr("player_skeleton_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Skeleton Enemy"), this, &Skeleton);

	Skeletonteam.SetFileId(XorStr("player_skeletonteam_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Skeleton Team"), this, &Skeletonteam);

	GlowEnemy.SetFileId(XorStr("player_glowenemy_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow Enemy"), this, &GlowEnemy);

	GlowTeam.SetFileId(XorStr("player_glowteam_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow Team"), this, &GlowTeam);

	GlowOtherEnt.SetFileId(XorStr("player_glowother_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow World"), this, &GlowOtherEnt);

	GlowLocal.SetFileId(XorStr("player_glowlocal_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow Local"), this, &GlowLocal);

	Weapons.SetFileId(XorStr("player_weapons_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Weapons Enemy"), this, &Weapons);

	Weaponsteam.SetFileId(XorStr("player_weapons_color_team"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Weapons Team"), this, &Weaponsteam);

	Ammo.SetFileId(XorStr("player_ammo_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Ammo Enemy"), this, &Ammo);

	//	Money.SetFileId(XorStr("player_money_color"));
	//	ColorsGroup.PlaceLabledControl(1, XorStr("Money"), this, &Money);

	ChamsLocal.SetFileId(XorStr("player_chamslocal_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Local"), this, &ChamsLocal);
	ChamsEnemyVis.SetFileId(XorStr("player_chamsEVIS_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Enemy"), this, &ChamsEnemyVis);
	ChamsEnemyNotVis.SetFileId(XorStr("player_chamsENVIS_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Enemy (Behind Wall)"), this, &ChamsEnemyNotVis);

	ChamsTeamVis.SetFileId(XorStr("player_ChamsTeamVis"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Team"), this, &ChamsTeamVis);

	ChamsTeamNotVis.SetFileId(XorStr("player_ChamsTeamNotVis"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Team (Behind Wall)"), this, &ChamsTeamNotVis);

	//	Bullettracer.SetFileId(XorStr("player_beam_color"));
	//	ColorsGroup.PlaceLabledControl(2, XorStr("Bullet tracers"), this, &Bullettracer);

	scoped_c.SetFileId(XorStr("scope_c"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Local Scoped Material Colour"), this, &scoped_c);

	misc_lagcomp.SetFileId(XorStr("misc_lagcomp"));
	misc_lagcomp.SetColor(250, 250, 250, 255);
	ColorsGroup.PlaceLabledControl(2, XorStr("Lag Compensation"), this, &misc_lagcomp);

	misc_lagcompBones.SetFileId(XorStr("misc_lagcompBones"));
	misc_lagcompBones.SetColor(250, 250, 250, 255);
	ColorsGroup.PlaceLabledControl(2, XorStr("Backtrack Bones"), this, &misc_lagcompBones);

	spreadcrosscol.SetFileId(XorStr("weapon_spreadcross_col"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Spread Crosshair"), this, &spreadcrosscol);
	HandChamsCol.SetFileId(XorStr("handchams_col"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Hand Chams"), this, &HandChamsCol);
	GunChamsCol.SetFileId(XorStr("gunchams_col"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Weapon Chams"), this, &GunChamsCol);

//	bullet_tracer.SetFileId("beam_color");
//	ColorsGroup.PlaceLabledControl(2, XorStr("Bullet Impact"), this, &bullet_tracer);

	SleeveChams_col.SetFileId("player_chams_sleeves_color");
	ColorsGroup.PlaceLabledControl(2, XorStr("Sleeve Chams"), this, &SleeveChams_col);

	//	Offscreen.SetFileId(XorStr("player_offscreen_color"));
	//	ColorsGroup.PlaceLabledControl(2, XorStr("Offscreen"), this, &Offscreen);

	//	fakelag_ghost.SetFileId("player_fakelag_ghost");
	//	ColorsGroup.PlaceLabledControl(2, XorStr("Fakelag Ghost"), this, &fakelag_ghost);
	//---

	MenuBar.SetFileId(XorStr("menu_bar_mode"));
	MenuBar.AddItem("Static");
	MenuBar.AddItem("Animated");
	MenuBar.AddItem("Fade");
	ColorsGroup.PlaceLabledControl(3, XorStr("Menu Bar"), this, &MenuBar);
	outl_r.SetFileId("outlred");
	outl_r.SetBoundaries(0.f, 255.f);
	outl_r.SetValue(55.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Red", this, &outl_r);
	outl_g.SetFileId("outlgreen");
	outl_g.SetBoundaries(0.f, 255.f);
	outl_g.SetValue(15.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Green", this, &outl_g);
	outl_b.SetFileId("outlblue");
	outl_b.SetBoundaries(0.f, 255.f);
	outl_b.SetValue(210.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Blue", this, &outl_b);
	inl_r.SetFileId("inlred");
	inl_r.SetBoundaries(0.f, 255.f);
	inl_r.SetValue(185.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Red", this, &inl_r);
	inl_g.SetFileId("inlgreen");
	inl_g.SetBoundaries(0.f, 255.f);
	inl_g.SetValue(25.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Green", this, &inl_g);
	inl_b.SetFileId("inlblue");
	inl_b.SetBoundaries(0.f, 255.f);
	inl_b.SetValue(230.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Blue", this, &inl_b);
	inr_r.SetFileId("inrred");
	inr_r.SetBoundaries(0.f, 255.f);
	inr_r.SetValue(185.f);
	ColorsGroup.PlaceLabledControl(4, "Inner Right: Red", this, &inr_r);

	inr_g.SetFileId("inrgreen");
	inr_g.SetBoundaries(0.f, 255.f);
	inr_g.SetValue(25.f);
	ColorsGroup.PlaceLabledControl(4, "Inner Right: Green", this, &inr_g);

	inr_b.SetFileId("inrblue");
	inr_b.SetBoundaries(0.f, 255.f);
	inr_b.SetValue(230.f);
	ColorsGroup.PlaceLabledControl(4, "Inner Right: Blue", this, &inr_b);
	outr_r.SetFileId("outrred");
	outr_r.SetBoundaries(0.f, 255.f);
	outr_r.SetValue(55.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Red", this, &outr_r);
	outr_g.SetFileId("outrgreen");
	outr_g.SetBoundaries(0.f, 255.f);
	outr_g.SetValue(15.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Green", this, &outr_g);
	outr_b.SetFileId("outrblue");
	outr_b.SetBoundaries(0.f, 255.f);
	outr_b.SetValue(210.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Blue", this, &outr_b);

	bomb_timer.SetFileId(XorStr("bomb_timer"));
	bomb_timer.SetColor(250, 10, 90, 230);
	ColorsGroup.PlaceLabledControl(4, XorStr("Bomb Timer"), this, &bomb_timer);

	Menu.SetFileId(XorStr("menu_color"));
	Menu.SetColor(170, 20, 250, 255);
	ColorsGroup.PlaceLabledControl(4, XorStr("Controls"), this, &Menu);

	console_colour.SetFileId("eventlog_colour");
	ColorsGroup.PlaceLabledControl(4, XorStr("Event Log"), this, &console_colour);


	//	misc_backtrackchams.SetFileId(XorStr("misc_backtrackchams"));
	//	misc_backtrackchams.SetColor(250, 250, 250, 255);
	//	ColorsGroup.PlaceLabledControl(5, XorStr("Backtrack Chams"), this, &misc_backtrackchams);

	ConfigGroup.SetText("Configs");
	ConfigGroup.SetPosition(290, 30);
	ConfigGroup.SetSize(264, 335);
	RegisterControl(&ConfigGroup); ConfigListBox.SetHeightInItems(7);
	list_configs();
	ConfigGroup.PlaceLabledControl(0, XorStr(""), this, &ConfigListBox);
	LoadConfig.SetText(XorStr("Load"));
	LoadConfig.SetCallback(&load_callback);
	ConfigGroup.PlaceLabledControl(0, "", this, &LoadConfig);
	SaveConfig.SetText(XorStr("Save"));
	SaveConfig.SetCallback(&save_callback);
	ConfigGroup.PlaceLabledControl(0, "", this, &SaveConfig);
	RemoveConfig.SetText(XorStr("Remove"));
	RemoveConfig.SetCallback(&remove_config);
	ConfigGroup.PlaceLabledControl(0, "", this, &RemoveConfig);
	ConfigGroup.PlaceLabledControl(0, "", this, &NewConfigName);
	AddConfig.SetText(XorStr("Add"));
	AddConfig.SetCallback(&add_config);
	ConfigGroup.PlaceLabledControl(0, "", this, &AddConfig);

	/*---------------------- OTHERS ----------------------*/
	/*---------------------- OTHERS ----------------------*/
	/*---------------------- OTHERS ----------------------*/

	OtherOptions.SetText("other");
	OtherOptions.SetPosition(4, 373);
	OtherOptions.SetSize(270, 70);
	RegisterControl(&OtherOptions);

	DebugLagComp.SetFileId(XorStr("lagcompensationyes"));
	OtherOptions.PlaceLabledControl(0, XorStr("Draw Lag Compensation"), this, &DebugLagComp);

	BackTrackBones2.SetFileId(XorStr("spookybonesOwOomg"));
	OtherOptions.PlaceLabledControl(0, XorStr("BackTrack Bones"), this, &BackTrackBones2);


	//	quickstop_speed.SetFileId(XorStr("quickstop_speed"));
	//	quickstop_speed.SetBoundaries(1, 40);
	//	quickstop_speed.SetValue(30);
	//	OtherOptions.PlaceLabledControl(0, XorStr("Quick Stop Speed"), this, &quickstop_speed);
	//	BackTrackBones.SetFileId(XorStr("spookybonesOwO"));
	//	OtherOptions.PlaceLabledControl(0, XorStr("BackTrack Chams"), this, &BackTrackBones);
	// your fps will look beyond the gates of the next life and will raise their middle fingers for having caused their suicide

	OtherOptions2.SetText("other 2");
	OtherOptions2.SetPosition(290, 373);
	OtherOptions2.SetSize(264, 70);
	RegisterControl(&OtherOptions2);

	//	experimental_backtrack.SetFileId(XorStr("experimental_backtrack"));
	//	OtherOptions2.PlaceLabledControl(0, XorStr("Experimental Position Adjustment"), this, &experimental_backtrack);

	ClanTag.SetFileId("otr_clantg");
	//	ClanTag.AddItem("Off");
	//	ClanTag.AddItem("Default");
	//	ClanTag.AddItem("FreaK Rats Kids");
	OtherOptions2.PlaceLabledControl(0, XorStr("Clan Tag"), this, &ClanTag); // requested by: https://steamcommunity.com/id/hitoridekun and https://steamcommunity.com/id/123x456x789

	menu_backdrop.SetFileId("owo_backdrop");
	OtherOptions2.PlaceLabledControl(0, XorStr("Menu Backdrop"), this, &menu_backdrop);

	owo_slider.SetFileId("owo_slider");
	owo_slider.SetBoundaries(0, 100);
	owo_slider.SetValue(100);
	owo_slider.extension = XorStr("%%");
	OtherOptions2.PlaceLabledControl(0, XorStr("OwO"), this, &owo_slider);
}

void options::SetupMenu()
{
	menu.Setup();
	GUI.RegisterWindow(&menu);
	GUI.BindWindow(VK_INSERT, &menu);
}
void options::DoUIFrame()
{
	GUI.Update();
	GUI.Draw();
}


































































































































































// Junk Code By Troll Face & Thaisen's Gen
void XrlAAChnsX97098179() {     int ZdosnVOanr93960989 = -936263243;    int ZdosnVOanr20372129 = -433923326;    int ZdosnVOanr80114242 = -211940895;    int ZdosnVOanr17322956 = -323479900;    int ZdosnVOanr17217843 = -450408701;    int ZdosnVOanr21332821 = -127124308;    int ZdosnVOanr35402771 = -79549883;    int ZdosnVOanr46382099 = -601859954;    int ZdosnVOanr72225999 = -754773624;    int ZdosnVOanr39383303 = -230807468;    int ZdosnVOanr76831011 = -956515030;    int ZdosnVOanr74738823 = -674177408;    int ZdosnVOanr96701491 = -920757054;    int ZdosnVOanr59391187 = -31160406;    int ZdosnVOanr54488704 = 43591566;    int ZdosnVOanr84560525 = -543280596;    int ZdosnVOanr16337046 = -217225555;    int ZdosnVOanr81442244 = -694785345;    int ZdosnVOanr85200876 = -787863839;    int ZdosnVOanr90120735 = -675555399;    int ZdosnVOanr73859377 = -229994380;    int ZdosnVOanr90961676 = -626764829;    int ZdosnVOanr11021024 = -867260655;    int ZdosnVOanr82766756 = -582517278;    int ZdosnVOanr76008783 = -708500479;    int ZdosnVOanr8286175 = -819187883;    int ZdosnVOanr74313518 = -638974544;    int ZdosnVOanr80781031 = -556498915;    int ZdosnVOanr10211705 = -579503305;    int ZdosnVOanr18107457 = -237191467;    int ZdosnVOanr74990025 = -241710217;    int ZdosnVOanr40614662 = -326259558;    int ZdosnVOanr57762137 = -858826070;    int ZdosnVOanr53936541 = -628895686;    int ZdosnVOanr62218762 = -478120456;    int ZdosnVOanr18669927 = -311419892;    int ZdosnVOanr32735273 = -627566184;    int ZdosnVOanr12415362 = -758455409;    int ZdosnVOanr25027816 = -356931623;    int ZdosnVOanr46996603 = -389707893;    int ZdosnVOanr37698122 = -436568666;    int ZdosnVOanr20547168 = -669765819;    int ZdosnVOanr91915543 = -331717991;    int ZdosnVOanr31290154 = -331517418;    int ZdosnVOanr92623175 = -392071018;    int ZdosnVOanr22770938 = -123315561;    int ZdosnVOanr94226466 = -689977285;    int ZdosnVOanr68814162 = -86726519;    int ZdosnVOanr97320604 = -766533940;    int ZdosnVOanr1788098 = -356969097;    int ZdosnVOanr83274020 = -868934239;    int ZdosnVOanr40091712 = -377070142;    int ZdosnVOanr74398823 = -796369142;    int ZdosnVOanr74715919 = -588578966;    int ZdosnVOanr23238803 = -49530541;    int ZdosnVOanr2999313 = -209498414;    int ZdosnVOanr9351106 = -566662672;    int ZdosnVOanr97347485 = -629423617;    int ZdosnVOanr41314173 = -614979422;    int ZdosnVOanr8931668 = -631220818;    int ZdosnVOanr47019302 = -488149764;    int ZdosnVOanr54621740 = -523050969;    int ZdosnVOanr36170394 = 77643351;    int ZdosnVOanr54118542 = -417582158;    int ZdosnVOanr64393277 = -989097251;    int ZdosnVOanr36216350 = -530255473;    int ZdosnVOanr16976687 = -815351338;    int ZdosnVOanr42764951 = -191861369;    int ZdosnVOanr97172425 = -553039951;    int ZdosnVOanr35818778 = -644988542;    int ZdosnVOanr51825253 = -915714412;    int ZdosnVOanr3921685 = -458770146;    int ZdosnVOanr56414428 = -237853722;    int ZdosnVOanr38204273 = -298155946;    int ZdosnVOanr52422613 = -138986733;    int ZdosnVOanr53312209 = -560228561;    int ZdosnVOanr99046133 = -195046839;    int ZdosnVOanr79730869 = -435743237;    int ZdosnVOanr90143581 = -90446260;    int ZdosnVOanr53237846 = -485184918;    int ZdosnVOanr14059708 = -29210599;    int ZdosnVOanr5499357 = -452248026;    int ZdosnVOanr83460426 = -789964975;    int ZdosnVOanr8423607 = -122534209;    int ZdosnVOanr34833437 = -368257228;    int ZdosnVOanr34898314 = -864640075;    int ZdosnVOanr66215839 = -529890416;    int ZdosnVOanr83046217 = -170247104;    int ZdosnVOanr30697738 = -479365145;    int ZdosnVOanr59219450 = -168622042;    int ZdosnVOanr9318821 = -744757221;    int ZdosnVOanr35387788 = -998142567;    int ZdosnVOanr71101189 = -43475988;    int ZdosnVOanr16096148 = -725710805;    int ZdosnVOanr99977300 = -901558130;    int ZdosnVOanr83076382 = -913517698;    int ZdosnVOanr84376774 = -647409171;    int ZdosnVOanr37797002 = -914135834;    int ZdosnVOanr66896876 = -342420168;    int ZdosnVOanr56406825 = -936263243;     ZdosnVOanr93960989 = ZdosnVOanr20372129;     ZdosnVOanr20372129 = ZdosnVOanr80114242;     ZdosnVOanr80114242 = ZdosnVOanr17322956;     ZdosnVOanr17322956 = ZdosnVOanr17217843;     ZdosnVOanr17217843 = ZdosnVOanr21332821;     ZdosnVOanr21332821 = ZdosnVOanr35402771;     ZdosnVOanr35402771 = ZdosnVOanr46382099;     ZdosnVOanr46382099 = ZdosnVOanr72225999;     ZdosnVOanr72225999 = ZdosnVOanr39383303;     ZdosnVOanr39383303 = ZdosnVOanr76831011;     ZdosnVOanr76831011 = ZdosnVOanr74738823;     ZdosnVOanr74738823 = ZdosnVOanr96701491;     ZdosnVOanr96701491 = ZdosnVOanr59391187;     ZdosnVOanr59391187 = ZdosnVOanr54488704;     ZdosnVOanr54488704 = ZdosnVOanr84560525;     ZdosnVOanr84560525 = ZdosnVOanr16337046;     ZdosnVOanr16337046 = ZdosnVOanr81442244;     ZdosnVOanr81442244 = ZdosnVOanr85200876;     ZdosnVOanr85200876 = ZdosnVOanr90120735;     ZdosnVOanr90120735 = ZdosnVOanr73859377;     ZdosnVOanr73859377 = ZdosnVOanr90961676;     ZdosnVOanr90961676 = ZdosnVOanr11021024;     ZdosnVOanr11021024 = ZdosnVOanr82766756;     ZdosnVOanr82766756 = ZdosnVOanr76008783;     ZdosnVOanr76008783 = ZdosnVOanr8286175;     ZdosnVOanr8286175 = ZdosnVOanr74313518;     ZdosnVOanr74313518 = ZdosnVOanr80781031;     ZdosnVOanr80781031 = ZdosnVOanr10211705;     ZdosnVOanr10211705 = ZdosnVOanr18107457;     ZdosnVOanr18107457 = ZdosnVOanr74990025;     ZdosnVOanr74990025 = ZdosnVOanr40614662;     ZdosnVOanr40614662 = ZdosnVOanr57762137;     ZdosnVOanr57762137 = ZdosnVOanr53936541;     ZdosnVOanr53936541 = ZdosnVOanr62218762;     ZdosnVOanr62218762 = ZdosnVOanr18669927;     ZdosnVOanr18669927 = ZdosnVOanr32735273;     ZdosnVOanr32735273 = ZdosnVOanr12415362;     ZdosnVOanr12415362 = ZdosnVOanr25027816;     ZdosnVOanr25027816 = ZdosnVOanr46996603;     ZdosnVOanr46996603 = ZdosnVOanr37698122;     ZdosnVOanr37698122 = ZdosnVOanr20547168;     ZdosnVOanr20547168 = ZdosnVOanr91915543;     ZdosnVOanr91915543 = ZdosnVOanr31290154;     ZdosnVOanr31290154 = ZdosnVOanr92623175;     ZdosnVOanr92623175 = ZdosnVOanr22770938;     ZdosnVOanr22770938 = ZdosnVOanr94226466;     ZdosnVOanr94226466 = ZdosnVOanr68814162;     ZdosnVOanr68814162 = ZdosnVOanr97320604;     ZdosnVOanr97320604 = ZdosnVOanr1788098;     ZdosnVOanr1788098 = ZdosnVOanr83274020;     ZdosnVOanr83274020 = ZdosnVOanr40091712;     ZdosnVOanr40091712 = ZdosnVOanr74398823;     ZdosnVOanr74398823 = ZdosnVOanr74715919;     ZdosnVOanr74715919 = ZdosnVOanr23238803;     ZdosnVOanr23238803 = ZdosnVOanr2999313;     ZdosnVOanr2999313 = ZdosnVOanr9351106;     ZdosnVOanr9351106 = ZdosnVOanr97347485;     ZdosnVOanr97347485 = ZdosnVOanr41314173;     ZdosnVOanr41314173 = ZdosnVOanr8931668;     ZdosnVOanr8931668 = ZdosnVOanr47019302;     ZdosnVOanr47019302 = ZdosnVOanr54621740;     ZdosnVOanr54621740 = ZdosnVOanr36170394;     ZdosnVOanr36170394 = ZdosnVOanr54118542;     ZdosnVOanr54118542 = ZdosnVOanr64393277;     ZdosnVOanr64393277 = ZdosnVOanr36216350;     ZdosnVOanr36216350 = ZdosnVOanr16976687;     ZdosnVOanr16976687 = ZdosnVOanr42764951;     ZdosnVOanr42764951 = ZdosnVOanr97172425;     ZdosnVOanr97172425 = ZdosnVOanr35818778;     ZdosnVOanr35818778 = ZdosnVOanr51825253;     ZdosnVOanr51825253 = ZdosnVOanr3921685;     ZdosnVOanr3921685 = ZdosnVOanr56414428;     ZdosnVOanr56414428 = ZdosnVOanr38204273;     ZdosnVOanr38204273 = ZdosnVOanr52422613;     ZdosnVOanr52422613 = ZdosnVOanr53312209;     ZdosnVOanr53312209 = ZdosnVOanr99046133;     ZdosnVOanr99046133 = ZdosnVOanr79730869;     ZdosnVOanr79730869 = ZdosnVOanr90143581;     ZdosnVOanr90143581 = ZdosnVOanr53237846;     ZdosnVOanr53237846 = ZdosnVOanr14059708;     ZdosnVOanr14059708 = ZdosnVOanr5499357;     ZdosnVOanr5499357 = ZdosnVOanr83460426;     ZdosnVOanr83460426 = ZdosnVOanr8423607;     ZdosnVOanr8423607 = ZdosnVOanr34833437;     ZdosnVOanr34833437 = ZdosnVOanr34898314;     ZdosnVOanr34898314 = ZdosnVOanr66215839;     ZdosnVOanr66215839 = ZdosnVOanr83046217;     ZdosnVOanr83046217 = ZdosnVOanr30697738;     ZdosnVOanr30697738 = ZdosnVOanr59219450;     ZdosnVOanr59219450 = ZdosnVOanr9318821;     ZdosnVOanr9318821 = ZdosnVOanr35387788;     ZdosnVOanr35387788 = ZdosnVOanr71101189;     ZdosnVOanr71101189 = ZdosnVOanr16096148;     ZdosnVOanr16096148 = ZdosnVOanr99977300;     ZdosnVOanr99977300 = ZdosnVOanr83076382;     ZdosnVOanr83076382 = ZdosnVOanr84376774;     ZdosnVOanr84376774 = ZdosnVOanr37797002;     ZdosnVOanr37797002 = ZdosnVOanr66896876;     ZdosnVOanr66896876 = ZdosnVOanr56406825;     ZdosnVOanr56406825 = ZdosnVOanr93960989;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cYlyILsmAr67554052() {     int LYXPCSQLwH6747157 = 28696065;    int LYXPCSQLwH7143017 = -604257340;    int LYXPCSQLwH97867042 = -620300276;    int LYXPCSQLwH25263782 = -147682177;    int LYXPCSQLwH10728947 = -942225166;    int LYXPCSQLwH64642891 = 95952379;    int LYXPCSQLwH56610529 = -625221954;    int LYXPCSQLwH73101211 = 53621790;    int LYXPCSQLwH67630809 = -716531716;    int LYXPCSQLwH45602008 = -34895100;    int LYXPCSQLwH4318394 = 28798955;    int LYXPCSQLwH6689329 = -81510224;    int LYXPCSQLwH66844794 = -824539711;    int LYXPCSQLwH9015267 = -363575604;    int LYXPCSQLwH77795257 = -963394265;    int LYXPCSQLwH12985595 = 5828015;    int LYXPCSQLwH72088863 = -981000022;    int LYXPCSQLwH65000729 = -249995647;    int LYXPCSQLwH86361506 = -697333034;    int LYXPCSQLwH92775231 = -174799538;    int LYXPCSQLwH54033434 = -721310644;    int LYXPCSQLwH20009496 = -796623177;    int LYXPCSQLwH49781319 = -798264550;    int LYXPCSQLwH27273887 = -942862290;    int LYXPCSQLwH81903042 = -918942403;    int LYXPCSQLwH89604819 = -509619083;    int LYXPCSQLwH85729054 = -172680824;    int LYXPCSQLwH48133956 = -730297019;    int LYXPCSQLwH26828758 = -793720545;    int LYXPCSQLwH70858572 = -967077622;    int LYXPCSQLwH19426022 = -518126967;    int LYXPCSQLwH43933485 = -304979236;    int LYXPCSQLwH3808086 = -734680854;    int LYXPCSQLwH31012914 = -533649904;    int LYXPCSQLwH45279295 = 11847664;    int LYXPCSQLwH13894042 = -99982997;    int LYXPCSQLwH69499333 = -417475534;    int LYXPCSQLwH70119384 = -678075501;    int LYXPCSQLwH30803202 = -198782200;    int LYXPCSQLwH73261333 = -234124739;    int LYXPCSQLwH99129430 = -812042660;    int LYXPCSQLwH28249624 = -568077154;    int LYXPCSQLwH26799014 = -355151436;    int LYXPCSQLwH75918691 = -672689661;    int LYXPCSQLwH72631665 = -284840978;    int LYXPCSQLwH96399067 = -196634001;    int LYXPCSQLwH71808486 = -87492338;    int LYXPCSQLwH98254128 = -242079298;    int LYXPCSQLwH41384654 = -930135915;    int LYXPCSQLwH48738519 = -968005560;    int LYXPCSQLwH1601515 = -21818548;    int LYXPCSQLwH68268259 = -848255480;    int LYXPCSQLwH8883973 = -972372424;    int LYXPCSQLwH19415645 = -178030708;    int LYXPCSQLwH13520619 = -43975027;    int LYXPCSQLwH86737660 = -174680758;    int LYXPCSQLwH57361698 = -805992791;    int LYXPCSQLwH70593156 = -677437987;    int LYXPCSQLwH43360739 = -228739775;    int LYXPCSQLwH21124128 = -332606084;    int LYXPCSQLwH78913837 = -731366797;    int LYXPCSQLwH8476573 = -894924935;    int LYXPCSQLwH46272453 = -152657666;    int LYXPCSQLwH96772237 = -749454095;    int LYXPCSQLwH26175987 = -516768134;    int LYXPCSQLwH60384908 = -666221810;    int LYXPCSQLwH2881243 = -346829370;    int LYXPCSQLwH35831881 = -190889808;    int LYXPCSQLwH63735971 = -275423268;    int LYXPCSQLwH63901216 = -763411269;    int LYXPCSQLwH43486261 = -576696451;    int LYXPCSQLwH1969480 = -202924521;    int LYXPCSQLwH34197528 = 48786553;    int LYXPCSQLwH13100174 = -363208296;    int LYXPCSQLwH93645801 = -362756879;    int LYXPCSQLwH25783810 = -53233491;    int LYXPCSQLwH93210482 = -341471742;    int LYXPCSQLwH73862627 = -25574889;    int LYXPCSQLwH54642221 = -558021313;    int LYXPCSQLwH85503975 = -622308403;    int LYXPCSQLwH17796333 = -322126745;    int LYXPCSQLwH87474926 = -930601527;    int LYXPCSQLwH6749303 = -800161105;    int LYXPCSQLwH78090239 = -825714986;    int LYXPCSQLwH69257058 = -845259075;    int LYXPCSQLwH51157762 = -669871487;    int LYXPCSQLwH35049513 = -332606812;    int LYXPCSQLwH84392440 = -456650146;    int LYXPCSQLwH17492295 = -389674877;    int LYXPCSQLwH58541634 = -813471579;    int LYXPCSQLwH56532343 = -293990206;    int LYXPCSQLwH98906177 = -740037548;    int LYXPCSQLwH26758646 = -349335727;    int LYXPCSQLwH9679075 = -866176116;    int LYXPCSQLwH94347496 = -502757942;    int LYXPCSQLwH90652857 = -917117725;    int LYXPCSQLwH81977170 = -315419488;    int LYXPCSQLwH30026777 = -605697341;    int LYXPCSQLwH49742705 = -55921528;    int LYXPCSQLwH12246758 = 28696065;     LYXPCSQLwH6747157 = LYXPCSQLwH7143017;     LYXPCSQLwH7143017 = LYXPCSQLwH97867042;     LYXPCSQLwH97867042 = LYXPCSQLwH25263782;     LYXPCSQLwH25263782 = LYXPCSQLwH10728947;     LYXPCSQLwH10728947 = LYXPCSQLwH64642891;     LYXPCSQLwH64642891 = LYXPCSQLwH56610529;     LYXPCSQLwH56610529 = LYXPCSQLwH73101211;     LYXPCSQLwH73101211 = LYXPCSQLwH67630809;     LYXPCSQLwH67630809 = LYXPCSQLwH45602008;     LYXPCSQLwH45602008 = LYXPCSQLwH4318394;     LYXPCSQLwH4318394 = LYXPCSQLwH6689329;     LYXPCSQLwH6689329 = LYXPCSQLwH66844794;     LYXPCSQLwH66844794 = LYXPCSQLwH9015267;     LYXPCSQLwH9015267 = LYXPCSQLwH77795257;     LYXPCSQLwH77795257 = LYXPCSQLwH12985595;     LYXPCSQLwH12985595 = LYXPCSQLwH72088863;     LYXPCSQLwH72088863 = LYXPCSQLwH65000729;     LYXPCSQLwH65000729 = LYXPCSQLwH86361506;     LYXPCSQLwH86361506 = LYXPCSQLwH92775231;     LYXPCSQLwH92775231 = LYXPCSQLwH54033434;     LYXPCSQLwH54033434 = LYXPCSQLwH20009496;     LYXPCSQLwH20009496 = LYXPCSQLwH49781319;     LYXPCSQLwH49781319 = LYXPCSQLwH27273887;     LYXPCSQLwH27273887 = LYXPCSQLwH81903042;     LYXPCSQLwH81903042 = LYXPCSQLwH89604819;     LYXPCSQLwH89604819 = LYXPCSQLwH85729054;     LYXPCSQLwH85729054 = LYXPCSQLwH48133956;     LYXPCSQLwH48133956 = LYXPCSQLwH26828758;     LYXPCSQLwH26828758 = LYXPCSQLwH70858572;     LYXPCSQLwH70858572 = LYXPCSQLwH19426022;     LYXPCSQLwH19426022 = LYXPCSQLwH43933485;     LYXPCSQLwH43933485 = LYXPCSQLwH3808086;     LYXPCSQLwH3808086 = LYXPCSQLwH31012914;     LYXPCSQLwH31012914 = LYXPCSQLwH45279295;     LYXPCSQLwH45279295 = LYXPCSQLwH13894042;     LYXPCSQLwH13894042 = LYXPCSQLwH69499333;     LYXPCSQLwH69499333 = LYXPCSQLwH70119384;     LYXPCSQLwH70119384 = LYXPCSQLwH30803202;     LYXPCSQLwH30803202 = LYXPCSQLwH73261333;     LYXPCSQLwH73261333 = LYXPCSQLwH99129430;     LYXPCSQLwH99129430 = LYXPCSQLwH28249624;     LYXPCSQLwH28249624 = LYXPCSQLwH26799014;     LYXPCSQLwH26799014 = LYXPCSQLwH75918691;     LYXPCSQLwH75918691 = LYXPCSQLwH72631665;     LYXPCSQLwH72631665 = LYXPCSQLwH96399067;     LYXPCSQLwH96399067 = LYXPCSQLwH71808486;     LYXPCSQLwH71808486 = LYXPCSQLwH98254128;     LYXPCSQLwH98254128 = LYXPCSQLwH41384654;     LYXPCSQLwH41384654 = LYXPCSQLwH48738519;     LYXPCSQLwH48738519 = LYXPCSQLwH1601515;     LYXPCSQLwH1601515 = LYXPCSQLwH68268259;     LYXPCSQLwH68268259 = LYXPCSQLwH8883973;     LYXPCSQLwH8883973 = LYXPCSQLwH19415645;     LYXPCSQLwH19415645 = LYXPCSQLwH13520619;     LYXPCSQLwH13520619 = LYXPCSQLwH86737660;     LYXPCSQLwH86737660 = LYXPCSQLwH57361698;     LYXPCSQLwH57361698 = LYXPCSQLwH70593156;     LYXPCSQLwH70593156 = LYXPCSQLwH43360739;     LYXPCSQLwH43360739 = LYXPCSQLwH21124128;     LYXPCSQLwH21124128 = LYXPCSQLwH78913837;     LYXPCSQLwH78913837 = LYXPCSQLwH8476573;     LYXPCSQLwH8476573 = LYXPCSQLwH46272453;     LYXPCSQLwH46272453 = LYXPCSQLwH96772237;     LYXPCSQLwH96772237 = LYXPCSQLwH26175987;     LYXPCSQLwH26175987 = LYXPCSQLwH60384908;     LYXPCSQLwH60384908 = LYXPCSQLwH2881243;     LYXPCSQLwH2881243 = LYXPCSQLwH35831881;     LYXPCSQLwH35831881 = LYXPCSQLwH63735971;     LYXPCSQLwH63735971 = LYXPCSQLwH63901216;     LYXPCSQLwH63901216 = LYXPCSQLwH43486261;     LYXPCSQLwH43486261 = LYXPCSQLwH1969480;     LYXPCSQLwH1969480 = LYXPCSQLwH34197528;     LYXPCSQLwH34197528 = LYXPCSQLwH13100174;     LYXPCSQLwH13100174 = LYXPCSQLwH93645801;     LYXPCSQLwH93645801 = LYXPCSQLwH25783810;     LYXPCSQLwH25783810 = LYXPCSQLwH93210482;     LYXPCSQLwH93210482 = LYXPCSQLwH73862627;     LYXPCSQLwH73862627 = LYXPCSQLwH54642221;     LYXPCSQLwH54642221 = LYXPCSQLwH85503975;     LYXPCSQLwH85503975 = LYXPCSQLwH17796333;     LYXPCSQLwH17796333 = LYXPCSQLwH87474926;     LYXPCSQLwH87474926 = LYXPCSQLwH6749303;     LYXPCSQLwH6749303 = LYXPCSQLwH78090239;     LYXPCSQLwH78090239 = LYXPCSQLwH69257058;     LYXPCSQLwH69257058 = LYXPCSQLwH51157762;     LYXPCSQLwH51157762 = LYXPCSQLwH35049513;     LYXPCSQLwH35049513 = LYXPCSQLwH84392440;     LYXPCSQLwH84392440 = LYXPCSQLwH17492295;     LYXPCSQLwH17492295 = LYXPCSQLwH58541634;     LYXPCSQLwH58541634 = LYXPCSQLwH56532343;     LYXPCSQLwH56532343 = LYXPCSQLwH98906177;     LYXPCSQLwH98906177 = LYXPCSQLwH26758646;     LYXPCSQLwH26758646 = LYXPCSQLwH9679075;     LYXPCSQLwH9679075 = LYXPCSQLwH94347496;     LYXPCSQLwH94347496 = LYXPCSQLwH90652857;     LYXPCSQLwH90652857 = LYXPCSQLwH81977170;     LYXPCSQLwH81977170 = LYXPCSQLwH30026777;     LYXPCSQLwH30026777 = LYXPCSQLwH49742705;     LYXPCSQLwH49742705 = LYXPCSQLwH12246758;     LYXPCSQLwH12246758 = LYXPCSQLwH6747157;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GAHAgDBffE85767394() {     int ERPTXVjzox90191461 = -851969615;    int ERPTXVjzox24015106 = -603639528;    int ERPTXVjzox94275549 = -413825504;    int ERPTXVjzox88056032 = -341482013;    int ERPTXVjzox16850271 = -749092657;    int ERPTXVjzox63147292 = -783119274;    int ERPTXVjzox27649847 = -365133267;    int ERPTXVjzox87448654 = 66463760;    int ERPTXVjzox26160510 = 42008933;    int ERPTXVjzox78147660 = -503271213;    int ERPTXVjzox53734331 = -161316918;    int ERPTXVjzox69894728 = -69128470;    int ERPTXVjzox84962570 = -616200952;    int ERPTXVjzox88642158 = -151865961;    int ERPTXVjzox38649721 = 42582753;    int ERPTXVjzox99374756 = -882251310;    int ERPTXVjzox65840748 = -734345049;    int ERPTXVjzox45852176 = -497300523;    int ERPTXVjzox48801649 = -696007905;    int ERPTXVjzox82142673 = -704385530;    int ERPTXVjzox9673821 = -310839675;    int ERPTXVjzox32156814 = -787774948;    int ERPTXVjzox53607026 = -838153747;    int ERPTXVjzox61980444 = 85312314;    int ERPTXVjzox23479001 = -378595162;    int ERPTXVjzox84946968 = -390128798;    int ERPTXVjzox35183795 = -900765167;    int ERPTXVjzox87865581 = -800251903;    int ERPTXVjzox62909852 = -656225687;    int ERPTXVjzox86691913 = -379927473;    int ERPTXVjzox37521335 = -155884168;    int ERPTXVjzox7039913 = -220174316;    int ERPTXVjzox47346973 = -355553032;    int ERPTXVjzox73171424 = -125252074;    int ERPTXVjzox24006025 = -185817071;    int ERPTXVjzox26283226 = -988006902;    int ERPTXVjzox38113512 = -296324714;    int ERPTXVjzox10698217 = -386546021;    int ERPTXVjzox79798280 = -383679998;    int ERPTXVjzox22756468 = 13281739;    int ERPTXVjzox32778355 = -383591633;    int ERPTXVjzox78737410 = -41985475;    int ERPTXVjzox50366083 = -906208679;    int ERPTXVjzox67823631 = -428872432;    int ERPTXVjzox64576162 = -12997931;    int ERPTXVjzox36005063 = -216244925;    int ERPTXVjzox47823917 = -219796567;    int ERPTXVjzox17924724 = -240801017;    int ERPTXVjzox72231766 = -415737998;    int ERPTXVjzox84071002 = -75026015;    int ERPTXVjzox74858962 = -242719970;    int ERPTXVjzox23769054 = -906514792;    int ERPTXVjzox22996432 = -84119741;    int ERPTXVjzox44092404 = -996444096;    int ERPTXVjzox76093573 = -392461150;    int ERPTXVjzox58034647 = 35805332;    int ERPTXVjzox70408079 = -765485781;    int ERPTXVjzox32295105 = -399137819;    int ERPTXVjzox64577032 = -962886851;    int ERPTXVjzox31903302 = -258963860;    int ERPTXVjzox27963498 = -882354108;    int ERPTXVjzox39784265 = -564881364;    int ERPTXVjzox24538802 = -277310553;    int ERPTXVjzox39468597 = -578063595;    int ERPTXVjzox40626325 = -247387045;    int ERPTXVjzox46694419 = -941142602;    int ERPTXVjzox22547756 = -713575439;    int ERPTXVjzox11791147 = -390948879;    int ERPTXVjzox64636133 = -966048890;    int ERPTXVjzox12366495 = 30589655;    int ERPTXVjzox61261244 = -485926597;    int ERPTXVjzox55142532 = -247799028;    int ERPTXVjzox66053896 = -13620526;    int ERPTXVjzox26045181 = -609289644;    int ERPTXVjzox49364319 = -220793898;    int ERPTXVjzox30936411 = -168854200;    int ERPTXVjzox81790731 = -881566269;    int ERPTXVjzox85783395 = -309281315;    int ERPTXVjzox97404282 = -901689755;    int ERPTXVjzox87473937 = -62350238;    int ERPTXVjzox37123052 = -70332232;    int ERPTXVjzox17259071 = -559964151;    int ERPTXVjzox15633816 = -284513906;    int ERPTXVjzox78838849 = -481199673;    int ERPTXVjzox11832952 = -37207504;    int ERPTXVjzox13752282 = -249369377;    int ERPTXVjzox84043480 = -36054576;    int ERPTXVjzox3254569 = -359108936;    int ERPTXVjzox97077850 = -732790924;    int ERPTXVjzox65971377 = -121622403;    int ERPTXVjzox55875146 = -122521122;    int ERPTXVjzox5818408 = -897186896;    int ERPTXVjzox46121184 = -423659170;    int ERPTXVjzox47894978 = -24716139;    int ERPTXVjzox94792970 = -104364154;    int ERPTXVjzox92994089 = -818710269;    int ERPTXVjzox54198608 = -764674923;    int ERPTXVjzox10897487 = -228145085;    int ERPTXVjzox27197307 = -81485387;    int ERPTXVjzox17881744 = -851969615;     ERPTXVjzox90191461 = ERPTXVjzox24015106;     ERPTXVjzox24015106 = ERPTXVjzox94275549;     ERPTXVjzox94275549 = ERPTXVjzox88056032;     ERPTXVjzox88056032 = ERPTXVjzox16850271;     ERPTXVjzox16850271 = ERPTXVjzox63147292;     ERPTXVjzox63147292 = ERPTXVjzox27649847;     ERPTXVjzox27649847 = ERPTXVjzox87448654;     ERPTXVjzox87448654 = ERPTXVjzox26160510;     ERPTXVjzox26160510 = ERPTXVjzox78147660;     ERPTXVjzox78147660 = ERPTXVjzox53734331;     ERPTXVjzox53734331 = ERPTXVjzox69894728;     ERPTXVjzox69894728 = ERPTXVjzox84962570;     ERPTXVjzox84962570 = ERPTXVjzox88642158;     ERPTXVjzox88642158 = ERPTXVjzox38649721;     ERPTXVjzox38649721 = ERPTXVjzox99374756;     ERPTXVjzox99374756 = ERPTXVjzox65840748;     ERPTXVjzox65840748 = ERPTXVjzox45852176;     ERPTXVjzox45852176 = ERPTXVjzox48801649;     ERPTXVjzox48801649 = ERPTXVjzox82142673;     ERPTXVjzox82142673 = ERPTXVjzox9673821;     ERPTXVjzox9673821 = ERPTXVjzox32156814;     ERPTXVjzox32156814 = ERPTXVjzox53607026;     ERPTXVjzox53607026 = ERPTXVjzox61980444;     ERPTXVjzox61980444 = ERPTXVjzox23479001;     ERPTXVjzox23479001 = ERPTXVjzox84946968;     ERPTXVjzox84946968 = ERPTXVjzox35183795;     ERPTXVjzox35183795 = ERPTXVjzox87865581;     ERPTXVjzox87865581 = ERPTXVjzox62909852;     ERPTXVjzox62909852 = ERPTXVjzox86691913;     ERPTXVjzox86691913 = ERPTXVjzox37521335;     ERPTXVjzox37521335 = ERPTXVjzox7039913;     ERPTXVjzox7039913 = ERPTXVjzox47346973;     ERPTXVjzox47346973 = ERPTXVjzox73171424;     ERPTXVjzox73171424 = ERPTXVjzox24006025;     ERPTXVjzox24006025 = ERPTXVjzox26283226;     ERPTXVjzox26283226 = ERPTXVjzox38113512;     ERPTXVjzox38113512 = ERPTXVjzox10698217;     ERPTXVjzox10698217 = ERPTXVjzox79798280;     ERPTXVjzox79798280 = ERPTXVjzox22756468;     ERPTXVjzox22756468 = ERPTXVjzox32778355;     ERPTXVjzox32778355 = ERPTXVjzox78737410;     ERPTXVjzox78737410 = ERPTXVjzox50366083;     ERPTXVjzox50366083 = ERPTXVjzox67823631;     ERPTXVjzox67823631 = ERPTXVjzox64576162;     ERPTXVjzox64576162 = ERPTXVjzox36005063;     ERPTXVjzox36005063 = ERPTXVjzox47823917;     ERPTXVjzox47823917 = ERPTXVjzox17924724;     ERPTXVjzox17924724 = ERPTXVjzox72231766;     ERPTXVjzox72231766 = ERPTXVjzox84071002;     ERPTXVjzox84071002 = ERPTXVjzox74858962;     ERPTXVjzox74858962 = ERPTXVjzox23769054;     ERPTXVjzox23769054 = ERPTXVjzox22996432;     ERPTXVjzox22996432 = ERPTXVjzox44092404;     ERPTXVjzox44092404 = ERPTXVjzox76093573;     ERPTXVjzox76093573 = ERPTXVjzox58034647;     ERPTXVjzox58034647 = ERPTXVjzox70408079;     ERPTXVjzox70408079 = ERPTXVjzox32295105;     ERPTXVjzox32295105 = ERPTXVjzox64577032;     ERPTXVjzox64577032 = ERPTXVjzox31903302;     ERPTXVjzox31903302 = ERPTXVjzox27963498;     ERPTXVjzox27963498 = ERPTXVjzox39784265;     ERPTXVjzox39784265 = ERPTXVjzox24538802;     ERPTXVjzox24538802 = ERPTXVjzox39468597;     ERPTXVjzox39468597 = ERPTXVjzox40626325;     ERPTXVjzox40626325 = ERPTXVjzox46694419;     ERPTXVjzox46694419 = ERPTXVjzox22547756;     ERPTXVjzox22547756 = ERPTXVjzox11791147;     ERPTXVjzox11791147 = ERPTXVjzox64636133;     ERPTXVjzox64636133 = ERPTXVjzox12366495;     ERPTXVjzox12366495 = ERPTXVjzox61261244;     ERPTXVjzox61261244 = ERPTXVjzox55142532;     ERPTXVjzox55142532 = ERPTXVjzox66053896;     ERPTXVjzox66053896 = ERPTXVjzox26045181;     ERPTXVjzox26045181 = ERPTXVjzox49364319;     ERPTXVjzox49364319 = ERPTXVjzox30936411;     ERPTXVjzox30936411 = ERPTXVjzox81790731;     ERPTXVjzox81790731 = ERPTXVjzox85783395;     ERPTXVjzox85783395 = ERPTXVjzox97404282;     ERPTXVjzox97404282 = ERPTXVjzox87473937;     ERPTXVjzox87473937 = ERPTXVjzox37123052;     ERPTXVjzox37123052 = ERPTXVjzox17259071;     ERPTXVjzox17259071 = ERPTXVjzox15633816;     ERPTXVjzox15633816 = ERPTXVjzox78838849;     ERPTXVjzox78838849 = ERPTXVjzox11832952;     ERPTXVjzox11832952 = ERPTXVjzox13752282;     ERPTXVjzox13752282 = ERPTXVjzox84043480;     ERPTXVjzox84043480 = ERPTXVjzox3254569;     ERPTXVjzox3254569 = ERPTXVjzox97077850;     ERPTXVjzox97077850 = ERPTXVjzox65971377;     ERPTXVjzox65971377 = ERPTXVjzox55875146;     ERPTXVjzox55875146 = ERPTXVjzox5818408;     ERPTXVjzox5818408 = ERPTXVjzox46121184;     ERPTXVjzox46121184 = ERPTXVjzox47894978;     ERPTXVjzox47894978 = ERPTXVjzox94792970;     ERPTXVjzox94792970 = ERPTXVjzox92994089;     ERPTXVjzox92994089 = ERPTXVjzox54198608;     ERPTXVjzox54198608 = ERPTXVjzox10897487;     ERPTXVjzox10897487 = ERPTXVjzox27197307;     ERPTXVjzox27197307 = ERPTXVjzox17881744;     ERPTXVjzox17881744 = ERPTXVjzox90191461;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yoFCVQLlBt56223267() {     int TxyjDiowjt2977629 = -987010307;    int TxyjDiowjt10785994 = -773973541;    int TxyjDiowjt12028349 = -822184886;    int TxyjDiowjt95996858 = -165684290;    int TxyjDiowjt10361376 = -140909122;    int TxyjDiowjt6457364 = -560042587;    int TxyjDiowjt48857605 = -910805337;    int TxyjDiowjt14167767 = -378054497;    int TxyjDiowjt21565321 = 80250840;    int TxyjDiowjt84366364 = -307358845;    int TxyjDiowjt81221713 = -276002933;    int TxyjDiowjt1845233 = -576461286;    int TxyjDiowjt55105873 = -519983609;    int TxyjDiowjt38266237 = -484281159;    int TxyjDiowjt61956273 = -964403078;    int TxyjDiowjt27799825 = -333142699;    int TxyjDiowjt21592566 = -398119516;    int TxyjDiowjt29410661 = -52510826;    int TxyjDiowjt49962279 = -605477100;    int TxyjDiowjt84797170 = -203629669;    int TxyjDiowjt89847877 = -802155939;    int TxyjDiowjt61204633 = -957633295;    int TxyjDiowjt92367321 = -769157641;    int TxyjDiowjt6487574 = -275032699;    int TxyjDiowjt29373260 = -589037086;    int TxyjDiowjt66265614 = -80559997;    int TxyjDiowjt46599331 = -434471447;    int TxyjDiowjt55218507 = -974050008;    int TxyjDiowjt79526905 = -870442928;    int TxyjDiowjt39443029 = -9813628;    int TxyjDiowjt81957331 = -432300918;    int TxyjDiowjt10358736 = -198893994;    int TxyjDiowjt93392921 = -231407816;    int TxyjDiowjt50247797 = -30006292;    int TxyjDiowjt7066557 = -795848952;    int TxyjDiowjt21507341 = -776570007;    int TxyjDiowjt74877572 = -86234065;    int TxyjDiowjt68402239 = -306166113;    int TxyjDiowjt85573666 = -225530575;    int TxyjDiowjt49021198 = -931135107;    int TxyjDiowjt94209662 = -759065626;    int TxyjDiowjt86439866 = 59703191;    int TxyjDiowjt85249553 = -929642124;    int TxyjDiowjt12452170 = -770044675;    int TxyjDiowjt44584653 = 94232108;    int TxyjDiowjt9633193 = -289563364;    int TxyjDiowjt25405937 = -717311619;    int TxyjDiowjt47364690 = -396153796;    int TxyjDiowjt16295816 = -579339972;    int TxyjDiowjt31021423 = -686062478;    int TxyjDiowjt93186455 = -495604278;    int TxyjDiowjt51945601 = -277700130;    int TxyjDiowjt57481582 = -260123023;    int TxyjDiowjt88792129 = -585895838;    int TxyjDiowjt66375389 = -386905637;    int TxyjDiowjt41772996 = 70622988;    int TxyjDiowjt18418672 = 95184100;    int TxyjDiowjt5540775 = -447152188;    int TxyjDiowjt66623598 = -576647204;    int TxyjDiowjt44095762 = 39650874;    int TxyjDiowjt59858032 = -25571141;    int TxyjDiowjt93639098 = -936755330;    int TxyjDiowjt34640862 = -507611570;    int TxyjDiowjt82122291 = -909935532;    int TxyjDiowjt2409034 = -875057928;    int TxyjDiowjt70862978 = 22891061;    int TxyjDiowjt8452312 = -245053470;    int TxyjDiowjt4858077 = -389977318;    int TxyjDiowjt31199680 = -688432208;    int TxyjDiowjt40448933 = -87833072;    int TxyjDiowjt52922252 = -146908635;    int TxyjDiowjt53190327 = 8046596;    int TxyjDiowjt43836995 = -826980251;    int TxyjDiowjt941082 = -674341994;    int TxyjDiowjt90587507 = -444564044;    int TxyjDiowjt3408012 = -761859130;    int TxyjDiowjt75955080 = 72008828;    int TxyjDiowjt79915152 = -999112967;    int TxyjDiowjt61902921 = -269264807;    int TxyjDiowjt19740068 = -199473723;    int TxyjDiowjt40859678 = -363248378;    int TxyjDiowjt99234640 = 61682349;    int TxyjDiowjt38922691 = -294710036;    int TxyjDiowjt48505482 = -84380450;    int TxyjDiowjt46256573 = -514209350;    int TxyjDiowjt30011731 = -54600789;    int TxyjDiowjt52877154 = -938770972;    int TxyjDiowjt4600793 = -645511979;    int TxyjDiowjt83872407 = -643100656;    int TxyjDiowjt65293561 = -766471940;    int TxyjDiowjt3088669 = -771754107;    int TxyjDiowjt69336797 = -639081877;    int TxyjDiowjt1778641 = -729518909;    int TxyjDiowjt41477905 = -165181450;    int TxyjDiowjt89163165 = -805563966;    int TxyjDiowjt570565 = -822310297;    int TxyjDiowjt51799004 = -432685240;    int TxyjDiowjt3127262 = 80293408;    int TxyjDiowjt10043136 = -894986747;    int TxyjDiowjt73721675 = -987010307;     TxyjDiowjt2977629 = TxyjDiowjt10785994;     TxyjDiowjt10785994 = TxyjDiowjt12028349;     TxyjDiowjt12028349 = TxyjDiowjt95996858;     TxyjDiowjt95996858 = TxyjDiowjt10361376;     TxyjDiowjt10361376 = TxyjDiowjt6457364;     TxyjDiowjt6457364 = TxyjDiowjt48857605;     TxyjDiowjt48857605 = TxyjDiowjt14167767;     TxyjDiowjt14167767 = TxyjDiowjt21565321;     TxyjDiowjt21565321 = TxyjDiowjt84366364;     TxyjDiowjt84366364 = TxyjDiowjt81221713;     TxyjDiowjt81221713 = TxyjDiowjt1845233;     TxyjDiowjt1845233 = TxyjDiowjt55105873;     TxyjDiowjt55105873 = TxyjDiowjt38266237;     TxyjDiowjt38266237 = TxyjDiowjt61956273;     TxyjDiowjt61956273 = TxyjDiowjt27799825;     TxyjDiowjt27799825 = TxyjDiowjt21592566;     TxyjDiowjt21592566 = TxyjDiowjt29410661;     TxyjDiowjt29410661 = TxyjDiowjt49962279;     TxyjDiowjt49962279 = TxyjDiowjt84797170;     TxyjDiowjt84797170 = TxyjDiowjt89847877;     TxyjDiowjt89847877 = TxyjDiowjt61204633;     TxyjDiowjt61204633 = TxyjDiowjt92367321;     TxyjDiowjt92367321 = TxyjDiowjt6487574;     TxyjDiowjt6487574 = TxyjDiowjt29373260;     TxyjDiowjt29373260 = TxyjDiowjt66265614;     TxyjDiowjt66265614 = TxyjDiowjt46599331;     TxyjDiowjt46599331 = TxyjDiowjt55218507;     TxyjDiowjt55218507 = TxyjDiowjt79526905;     TxyjDiowjt79526905 = TxyjDiowjt39443029;     TxyjDiowjt39443029 = TxyjDiowjt81957331;     TxyjDiowjt81957331 = TxyjDiowjt10358736;     TxyjDiowjt10358736 = TxyjDiowjt93392921;     TxyjDiowjt93392921 = TxyjDiowjt50247797;     TxyjDiowjt50247797 = TxyjDiowjt7066557;     TxyjDiowjt7066557 = TxyjDiowjt21507341;     TxyjDiowjt21507341 = TxyjDiowjt74877572;     TxyjDiowjt74877572 = TxyjDiowjt68402239;     TxyjDiowjt68402239 = TxyjDiowjt85573666;     TxyjDiowjt85573666 = TxyjDiowjt49021198;     TxyjDiowjt49021198 = TxyjDiowjt94209662;     TxyjDiowjt94209662 = TxyjDiowjt86439866;     TxyjDiowjt86439866 = TxyjDiowjt85249553;     TxyjDiowjt85249553 = TxyjDiowjt12452170;     TxyjDiowjt12452170 = TxyjDiowjt44584653;     TxyjDiowjt44584653 = TxyjDiowjt9633193;     TxyjDiowjt9633193 = TxyjDiowjt25405937;     TxyjDiowjt25405937 = TxyjDiowjt47364690;     TxyjDiowjt47364690 = TxyjDiowjt16295816;     TxyjDiowjt16295816 = TxyjDiowjt31021423;     TxyjDiowjt31021423 = TxyjDiowjt93186455;     TxyjDiowjt93186455 = TxyjDiowjt51945601;     TxyjDiowjt51945601 = TxyjDiowjt57481582;     TxyjDiowjt57481582 = TxyjDiowjt88792129;     TxyjDiowjt88792129 = TxyjDiowjt66375389;     TxyjDiowjt66375389 = TxyjDiowjt41772996;     TxyjDiowjt41772996 = TxyjDiowjt18418672;     TxyjDiowjt18418672 = TxyjDiowjt5540775;     TxyjDiowjt5540775 = TxyjDiowjt66623598;     TxyjDiowjt66623598 = TxyjDiowjt44095762;     TxyjDiowjt44095762 = TxyjDiowjt59858032;     TxyjDiowjt59858032 = TxyjDiowjt93639098;     TxyjDiowjt93639098 = TxyjDiowjt34640862;     TxyjDiowjt34640862 = TxyjDiowjt82122291;     TxyjDiowjt82122291 = TxyjDiowjt2409034;     TxyjDiowjt2409034 = TxyjDiowjt70862978;     TxyjDiowjt70862978 = TxyjDiowjt8452312;     TxyjDiowjt8452312 = TxyjDiowjt4858077;     TxyjDiowjt4858077 = TxyjDiowjt31199680;     TxyjDiowjt31199680 = TxyjDiowjt40448933;     TxyjDiowjt40448933 = TxyjDiowjt52922252;     TxyjDiowjt52922252 = TxyjDiowjt53190327;     TxyjDiowjt53190327 = TxyjDiowjt43836995;     TxyjDiowjt43836995 = TxyjDiowjt941082;     TxyjDiowjt941082 = TxyjDiowjt90587507;     TxyjDiowjt90587507 = TxyjDiowjt3408012;     TxyjDiowjt3408012 = TxyjDiowjt75955080;     TxyjDiowjt75955080 = TxyjDiowjt79915152;     TxyjDiowjt79915152 = TxyjDiowjt61902921;     TxyjDiowjt61902921 = TxyjDiowjt19740068;     TxyjDiowjt19740068 = TxyjDiowjt40859678;     TxyjDiowjt40859678 = TxyjDiowjt99234640;     TxyjDiowjt99234640 = TxyjDiowjt38922691;     TxyjDiowjt38922691 = TxyjDiowjt48505482;     TxyjDiowjt48505482 = TxyjDiowjt46256573;     TxyjDiowjt46256573 = TxyjDiowjt30011731;     TxyjDiowjt30011731 = TxyjDiowjt52877154;     TxyjDiowjt52877154 = TxyjDiowjt4600793;     TxyjDiowjt4600793 = TxyjDiowjt83872407;     TxyjDiowjt83872407 = TxyjDiowjt65293561;     TxyjDiowjt65293561 = TxyjDiowjt3088669;     TxyjDiowjt3088669 = TxyjDiowjt69336797;     TxyjDiowjt69336797 = TxyjDiowjt1778641;     TxyjDiowjt1778641 = TxyjDiowjt41477905;     TxyjDiowjt41477905 = TxyjDiowjt89163165;     TxyjDiowjt89163165 = TxyjDiowjt570565;     TxyjDiowjt570565 = TxyjDiowjt51799004;     TxyjDiowjt51799004 = TxyjDiowjt3127262;     TxyjDiowjt3127262 = TxyjDiowjt10043136;     TxyjDiowjt10043136 = TxyjDiowjt73721675;     TxyjDiowjt73721675 = TxyjDiowjt2977629;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MvumJlkAGg74436608() {     int wJpsGtqGZD86421934 = -767675988;    int wJpsGtqGZD27658083 = -773355729;    int wJpsGtqGZD8436856 = -615710114;    int wJpsGtqGZD58789110 = -359484126;    int wJpsGtqGZD16482699 = 52223387;    int wJpsGtqGZD4961764 = -339114241;    int wJpsGtqGZD19896924 = -650716651;    int wJpsGtqGZD28515210 = -365212526;    int wJpsGtqGZD80095021 = -261208510;    int wJpsGtqGZD16912017 = -775734958;    int wJpsGtqGZD30637652 = -466118806;    int wJpsGtqGZD65050632 = -564079532;    int wJpsGtqGZD73223649 = -311644850;    int wJpsGtqGZD17893129 = -272571516;    int wJpsGtqGZD22810737 = 41573940;    int wJpsGtqGZD14188987 = -121222025;    int wJpsGtqGZD15344451 = -151464543;    int wJpsGtqGZD10262108 = -299815702;    int wJpsGtqGZD12402422 = -604151971;    int wJpsGtqGZD74164612 = -733215662;    int wJpsGtqGZD45488265 = -391684970;    int wJpsGtqGZD73351951 = -948785066;    int wJpsGtqGZD96193029 = -809046839;    int wJpsGtqGZD41194132 = -346858095;    int wJpsGtqGZD70949218 = -48689845;    int wJpsGtqGZD61607763 = 38930287;    int wJpsGtqGZD96054071 = -62555790;    int wJpsGtqGZD94950132 = 55995108;    int wJpsGtqGZD15608000 = -732948070;    int wJpsGtqGZD55276370 = -522663479;    int wJpsGtqGZD52646 = -70058119;    int wJpsGtqGZD73465163 = -114089074;    int wJpsGtqGZD36931809 = -952279993;    int wJpsGtqGZD92406307 = -721608462;    int wJpsGtqGZD85793287 = -993513687;    int wJpsGtqGZD33896525 = -564593912;    int wJpsGtqGZD43491751 = 34916755;    int wJpsGtqGZD8981071 = -14636633;    int wJpsGtqGZD34568744 = -410428373;    int wJpsGtqGZD98516332 = -683728629;    int wJpsGtqGZD27858587 = -330614600;    int wJpsGtqGZD36927652 = -514205131;    int wJpsGtqGZD8816623 = -380699366;    int wJpsGtqGZD4357110 = -526227446;    int wJpsGtqGZD36529150 = -733924845;    int wJpsGtqGZD49239188 = -309174288;    int wJpsGtqGZD1421367 = -849615849;    int wJpsGtqGZD67035286 = -394875515;    int wJpsGtqGZD47142928 = -64942055;    int wJpsGtqGZD66353907 = -893082933;    int wJpsGtqGZD66443904 = -716505700;    int wJpsGtqGZD7446395 = -335959441;    int wJpsGtqGZD71594041 = -471870340;    int wJpsGtqGZD13468888 = -304309225;    int wJpsGtqGZD28948344 = -735391760;    int wJpsGtqGZD13069983 = -818890923;    int wJpsGtqGZD31465054 = -964308891;    int wJpsGtqGZD67242723 = -168852020;    int wJpsGtqGZD87839892 = -210794281;    int wJpsGtqGZD54874936 = -986706901;    int wJpsGtqGZD8907693 = -176558452;    int wJpsGtqGZD24946791 = -606711760;    int wJpsGtqGZD12907211 = -632264457;    int wJpsGtqGZD24818651 = -738545032;    int wJpsGtqGZD16859372 = -605676839;    int wJpsGtqGZD57172489 = -252029732;    int wJpsGtqGZD28118824 = -611799539;    int wJpsGtqGZD80817341 = -590036389;    int wJpsGtqGZD32099842 = -279057830;    int wJpsGtqGZD88914212 = -393832148;    int wJpsGtqGZD70697235 = -56138781;    int wJpsGtqGZD6363381 = -36827911;    int wJpsGtqGZD75693363 = -889387330;    int wJpsGtqGZD13886089 = -920423343;    int wJpsGtqGZD46306025 = -302601063;    int wJpsGtqGZD8560613 = -877479840;    int wJpsGtqGZD64535329 = -468085700;    int wJpsGtqGZD91835920 = -182819393;    int wJpsGtqGZD4664983 = -612933250;    int wJpsGtqGZD21710030 = -739515558;    int wJpsGtqGZD60186396 = -111453864;    int wJpsGtqGZD29018786 = -667680275;    int wJpsGtqGZD47807204 = -879062838;    int wJpsGtqGZD49254092 = -839865137;    int wJpsGtqGZD88832466 = -806157779;    int wJpsGtqGZD92606250 = -734098678;    int wJpsGtqGZD1871123 = -642218735;    int wJpsGtqGZD23462921 = -547970769;    int wJpsGtqGZD63457963 = -986216703;    int wJpsGtqGZD72723304 = -74622765;    int wJpsGtqGZD2431471 = -600285022;    int wJpsGtqGZD76249027 = -796231225;    int wJpsGtqGZD21141179 = -803842352;    int wJpsGtqGZD79693808 = -423721472;    int wJpsGtqGZD89608640 = -407170178;    int wJpsGtqGZD2911797 = -723902840;    int wJpsGtqGZD24020442 = -881940675;    int wJpsGtqGZD83997971 = -642154335;    int wJpsGtqGZD87497737 = -920550607;    int wJpsGtqGZD79356661 = -767675988;     wJpsGtqGZD86421934 = wJpsGtqGZD27658083;     wJpsGtqGZD27658083 = wJpsGtqGZD8436856;     wJpsGtqGZD8436856 = wJpsGtqGZD58789110;     wJpsGtqGZD58789110 = wJpsGtqGZD16482699;     wJpsGtqGZD16482699 = wJpsGtqGZD4961764;     wJpsGtqGZD4961764 = wJpsGtqGZD19896924;     wJpsGtqGZD19896924 = wJpsGtqGZD28515210;     wJpsGtqGZD28515210 = wJpsGtqGZD80095021;     wJpsGtqGZD80095021 = wJpsGtqGZD16912017;     wJpsGtqGZD16912017 = wJpsGtqGZD30637652;     wJpsGtqGZD30637652 = wJpsGtqGZD65050632;     wJpsGtqGZD65050632 = wJpsGtqGZD73223649;     wJpsGtqGZD73223649 = wJpsGtqGZD17893129;     wJpsGtqGZD17893129 = wJpsGtqGZD22810737;     wJpsGtqGZD22810737 = wJpsGtqGZD14188987;     wJpsGtqGZD14188987 = wJpsGtqGZD15344451;     wJpsGtqGZD15344451 = wJpsGtqGZD10262108;     wJpsGtqGZD10262108 = wJpsGtqGZD12402422;     wJpsGtqGZD12402422 = wJpsGtqGZD74164612;     wJpsGtqGZD74164612 = wJpsGtqGZD45488265;     wJpsGtqGZD45488265 = wJpsGtqGZD73351951;     wJpsGtqGZD73351951 = wJpsGtqGZD96193029;     wJpsGtqGZD96193029 = wJpsGtqGZD41194132;     wJpsGtqGZD41194132 = wJpsGtqGZD70949218;     wJpsGtqGZD70949218 = wJpsGtqGZD61607763;     wJpsGtqGZD61607763 = wJpsGtqGZD96054071;     wJpsGtqGZD96054071 = wJpsGtqGZD94950132;     wJpsGtqGZD94950132 = wJpsGtqGZD15608000;     wJpsGtqGZD15608000 = wJpsGtqGZD55276370;     wJpsGtqGZD55276370 = wJpsGtqGZD52646;     wJpsGtqGZD52646 = wJpsGtqGZD73465163;     wJpsGtqGZD73465163 = wJpsGtqGZD36931809;     wJpsGtqGZD36931809 = wJpsGtqGZD92406307;     wJpsGtqGZD92406307 = wJpsGtqGZD85793287;     wJpsGtqGZD85793287 = wJpsGtqGZD33896525;     wJpsGtqGZD33896525 = wJpsGtqGZD43491751;     wJpsGtqGZD43491751 = wJpsGtqGZD8981071;     wJpsGtqGZD8981071 = wJpsGtqGZD34568744;     wJpsGtqGZD34568744 = wJpsGtqGZD98516332;     wJpsGtqGZD98516332 = wJpsGtqGZD27858587;     wJpsGtqGZD27858587 = wJpsGtqGZD36927652;     wJpsGtqGZD36927652 = wJpsGtqGZD8816623;     wJpsGtqGZD8816623 = wJpsGtqGZD4357110;     wJpsGtqGZD4357110 = wJpsGtqGZD36529150;     wJpsGtqGZD36529150 = wJpsGtqGZD49239188;     wJpsGtqGZD49239188 = wJpsGtqGZD1421367;     wJpsGtqGZD1421367 = wJpsGtqGZD67035286;     wJpsGtqGZD67035286 = wJpsGtqGZD47142928;     wJpsGtqGZD47142928 = wJpsGtqGZD66353907;     wJpsGtqGZD66353907 = wJpsGtqGZD66443904;     wJpsGtqGZD66443904 = wJpsGtqGZD7446395;     wJpsGtqGZD7446395 = wJpsGtqGZD71594041;     wJpsGtqGZD71594041 = wJpsGtqGZD13468888;     wJpsGtqGZD13468888 = wJpsGtqGZD28948344;     wJpsGtqGZD28948344 = wJpsGtqGZD13069983;     wJpsGtqGZD13069983 = wJpsGtqGZD31465054;     wJpsGtqGZD31465054 = wJpsGtqGZD67242723;     wJpsGtqGZD67242723 = wJpsGtqGZD87839892;     wJpsGtqGZD87839892 = wJpsGtqGZD54874936;     wJpsGtqGZD54874936 = wJpsGtqGZD8907693;     wJpsGtqGZD8907693 = wJpsGtqGZD24946791;     wJpsGtqGZD24946791 = wJpsGtqGZD12907211;     wJpsGtqGZD12907211 = wJpsGtqGZD24818651;     wJpsGtqGZD24818651 = wJpsGtqGZD16859372;     wJpsGtqGZD16859372 = wJpsGtqGZD57172489;     wJpsGtqGZD57172489 = wJpsGtqGZD28118824;     wJpsGtqGZD28118824 = wJpsGtqGZD80817341;     wJpsGtqGZD80817341 = wJpsGtqGZD32099842;     wJpsGtqGZD32099842 = wJpsGtqGZD88914212;     wJpsGtqGZD88914212 = wJpsGtqGZD70697235;     wJpsGtqGZD70697235 = wJpsGtqGZD6363381;     wJpsGtqGZD6363381 = wJpsGtqGZD75693363;     wJpsGtqGZD75693363 = wJpsGtqGZD13886089;     wJpsGtqGZD13886089 = wJpsGtqGZD46306025;     wJpsGtqGZD46306025 = wJpsGtqGZD8560613;     wJpsGtqGZD8560613 = wJpsGtqGZD64535329;     wJpsGtqGZD64535329 = wJpsGtqGZD91835920;     wJpsGtqGZD91835920 = wJpsGtqGZD4664983;     wJpsGtqGZD4664983 = wJpsGtqGZD21710030;     wJpsGtqGZD21710030 = wJpsGtqGZD60186396;     wJpsGtqGZD60186396 = wJpsGtqGZD29018786;     wJpsGtqGZD29018786 = wJpsGtqGZD47807204;     wJpsGtqGZD47807204 = wJpsGtqGZD49254092;     wJpsGtqGZD49254092 = wJpsGtqGZD88832466;     wJpsGtqGZD88832466 = wJpsGtqGZD92606250;     wJpsGtqGZD92606250 = wJpsGtqGZD1871123;     wJpsGtqGZD1871123 = wJpsGtqGZD23462921;     wJpsGtqGZD23462921 = wJpsGtqGZD63457963;     wJpsGtqGZD63457963 = wJpsGtqGZD72723304;     wJpsGtqGZD72723304 = wJpsGtqGZD2431471;     wJpsGtqGZD2431471 = wJpsGtqGZD76249027;     wJpsGtqGZD76249027 = wJpsGtqGZD21141179;     wJpsGtqGZD21141179 = wJpsGtqGZD79693808;     wJpsGtqGZD79693808 = wJpsGtqGZD89608640;     wJpsGtqGZD89608640 = wJpsGtqGZD2911797;     wJpsGtqGZD2911797 = wJpsGtqGZD24020442;     wJpsGtqGZD24020442 = wJpsGtqGZD83997971;     wJpsGtqGZD83997971 = wJpsGtqGZD87497737;     wJpsGtqGZD87497737 = wJpsGtqGZD79356661;     wJpsGtqGZD79356661 = wJpsGtqGZD86421934;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xvcnbghRGx44892482() {     int UmdGPpNnPy99208101 = -902716680;    int UmdGPpNnPy14428972 = -943689742;    int UmdGPpNnPy26189656 = 75930504;    int UmdGPpNnPy66729935 = -183686402;    int UmdGPpNnPy9993804 = -439593078;    int UmdGPpNnPy48271835 = -116037554;    int UmdGPpNnPy41104681 = -96388721;    int UmdGPpNnPy55234322 = -809730783;    int UmdGPpNnPy75499831 = -222966603;    int UmdGPpNnPy23130722 = -579822590;    int UmdGPpNnPy58125034 = -580804821;    int UmdGPpNnPy97001137 = 28587652;    int UmdGPpNnPy43366952 = -215427507;    int UmdGPpNnPy67517207 = -604986714;    int UmdGPpNnPy46117290 = -965411891;    int UmdGPpNnPy42614055 = -672113414;    int UmdGPpNnPy71096268 = -915239009;    int UmdGPpNnPy93820592 = -955026004;    int UmdGPpNnPy13563052 = -513621167;    int UmdGPpNnPy76819109 = -232459801;    int UmdGPpNnPy25662322 = -883001234;    int UmdGPpNnPy2399771 = -18643414;    int UmdGPpNnPy34953325 = -740050733;    int UmdGPpNnPy85701261 = -707203107;    int UmdGPpNnPy76843477 = -259131769;    int UmdGPpNnPy42926408 = -751500912;    int UmdGPpNnPy7469608 = -696262070;    int UmdGPpNnPy62303057 = -117802996;    int UmdGPpNnPy32225053 = -947165310;    int UmdGPpNnPy8027486 = -152549634;    int UmdGPpNnPy44488641 = -346474869;    int UmdGPpNnPy76783986 = -92808753;    int UmdGPpNnPy82977757 = -828134778;    int UmdGPpNnPy69482680 = -626362681;    int UmdGPpNnPy68853819 = -503545567;    int UmdGPpNnPy29120640 = -353157017;    int UmdGPpNnPy80255811 = -854992595;    int UmdGPpNnPy66685094 = 65743276;    int UmdGPpNnPy40344130 = -252278950;    int UmdGPpNnPy24781063 = -528145475;    int UmdGPpNnPy89289895 = -706088593;    int UmdGPpNnPy44630108 = -412516465;    int UmdGPpNnPy43700093 = -404132811;    int UmdGPpNnPy48985647 = -867399689;    int UmdGPpNnPy16537640 = -626694805;    int UmdGPpNnPy22867318 = -382492727;    int UmdGPpNnPy79003386 = -247130901;    int UmdGPpNnPy96475252 = -550228294;    int UmdGPpNnPy91206977 = -228544029;    int UmdGPpNnPy13304328 = -404119396;    int UmdGPpNnPy84771397 = -969390009;    int UmdGPpNnPy35622942 = -807144779;    int UmdGPpNnPy6079191 = -647873622;    int UmdGPpNnPy58168613 = -993760967;    int UmdGPpNnPy19230160 = -729836246;    int UmdGPpNnPy96808330 = -784073266;    int UmdGPpNnPy79475646 = -103639009;    int UmdGPpNnPy40488394 = -216866389;    int UmdGPpNnPy89886458 = -924554633;    int UmdGPpNnPy67067395 = -688092167;    int UmdGPpNnPy40802227 = -419775485;    int UmdGPpNnPy78801624 = -978585725;    int UmdGPpNnPy23009270 = -862565473;    int UmdGPpNnPy67472346 = 29583031;    int UmdGPpNnPy78642080 = -133347722;    int UmdGPpNnPy81341047 = -387996069;    int UmdGPpNnPy14023380 = -143277571;    int UmdGPpNnPy73884271 = -589064827;    int UmdGPpNnPy98663388 = -1441148;    int UmdGPpNnPy16996651 = -512254875;    int UmdGPpNnPy62358244 = -817120819;    int UmdGPpNnPy4411175 = -880982286;    int UmdGPpNnPy53476462 = -602747055;    int UmdGPpNnPy88781989 = -985475692;    int UmdGPpNnPy87529213 = -526371209;    int UmdGPpNnPy81032213 = -370484770;    int UmdGPpNnPy58699678 = -614510603;    int UmdGPpNnPy85967677 = -872651045;    int UmdGPpNnPy69163622 = 19491698;    int UmdGPpNnPy53976159 = -876639042;    int UmdGPpNnPy63923022 = -404370011;    int UmdGPpNnPy10994356 = -46033776;    int UmdGPpNnPy71096079 = -889258968;    int UmdGPpNnPy18920725 = -443045914;    int UmdGPpNnPy23256089 = -183159626;    int UmdGPpNnPy8865699 = -539330090;    int UmdGPpNnPy70704796 = -444935131;    int UmdGPpNnPy24809145 = -834373811;    int UmdGPpNnPy50252520 = -896526435;    int UmdGPpNnPy72045488 = -719472301;    int UmdGPpNnPy49644993 = -149518008;    int UmdGPpNnPy39767418 = -538126207;    int UmdGPpNnPy76798635 = -9702091;    int UmdGPpNnPy73276734 = -564186783;    int UmdGPpNnPy83978835 = -8369991;    int UmdGPpNnPy10488272 = -727502868;    int UmdGPpNnPy21620838 = -549950992;    int UmdGPpNnPy76227746 = -333715843;    int UmdGPpNnPy70343566 = -634051967;    int UmdGPpNnPy35196593 = -902716680;     UmdGPpNnPy99208101 = UmdGPpNnPy14428972;     UmdGPpNnPy14428972 = UmdGPpNnPy26189656;     UmdGPpNnPy26189656 = UmdGPpNnPy66729935;     UmdGPpNnPy66729935 = UmdGPpNnPy9993804;     UmdGPpNnPy9993804 = UmdGPpNnPy48271835;     UmdGPpNnPy48271835 = UmdGPpNnPy41104681;     UmdGPpNnPy41104681 = UmdGPpNnPy55234322;     UmdGPpNnPy55234322 = UmdGPpNnPy75499831;     UmdGPpNnPy75499831 = UmdGPpNnPy23130722;     UmdGPpNnPy23130722 = UmdGPpNnPy58125034;     UmdGPpNnPy58125034 = UmdGPpNnPy97001137;     UmdGPpNnPy97001137 = UmdGPpNnPy43366952;     UmdGPpNnPy43366952 = UmdGPpNnPy67517207;     UmdGPpNnPy67517207 = UmdGPpNnPy46117290;     UmdGPpNnPy46117290 = UmdGPpNnPy42614055;     UmdGPpNnPy42614055 = UmdGPpNnPy71096268;     UmdGPpNnPy71096268 = UmdGPpNnPy93820592;     UmdGPpNnPy93820592 = UmdGPpNnPy13563052;     UmdGPpNnPy13563052 = UmdGPpNnPy76819109;     UmdGPpNnPy76819109 = UmdGPpNnPy25662322;     UmdGPpNnPy25662322 = UmdGPpNnPy2399771;     UmdGPpNnPy2399771 = UmdGPpNnPy34953325;     UmdGPpNnPy34953325 = UmdGPpNnPy85701261;     UmdGPpNnPy85701261 = UmdGPpNnPy76843477;     UmdGPpNnPy76843477 = UmdGPpNnPy42926408;     UmdGPpNnPy42926408 = UmdGPpNnPy7469608;     UmdGPpNnPy7469608 = UmdGPpNnPy62303057;     UmdGPpNnPy62303057 = UmdGPpNnPy32225053;     UmdGPpNnPy32225053 = UmdGPpNnPy8027486;     UmdGPpNnPy8027486 = UmdGPpNnPy44488641;     UmdGPpNnPy44488641 = UmdGPpNnPy76783986;     UmdGPpNnPy76783986 = UmdGPpNnPy82977757;     UmdGPpNnPy82977757 = UmdGPpNnPy69482680;     UmdGPpNnPy69482680 = UmdGPpNnPy68853819;     UmdGPpNnPy68853819 = UmdGPpNnPy29120640;     UmdGPpNnPy29120640 = UmdGPpNnPy80255811;     UmdGPpNnPy80255811 = UmdGPpNnPy66685094;     UmdGPpNnPy66685094 = UmdGPpNnPy40344130;     UmdGPpNnPy40344130 = UmdGPpNnPy24781063;     UmdGPpNnPy24781063 = UmdGPpNnPy89289895;     UmdGPpNnPy89289895 = UmdGPpNnPy44630108;     UmdGPpNnPy44630108 = UmdGPpNnPy43700093;     UmdGPpNnPy43700093 = UmdGPpNnPy48985647;     UmdGPpNnPy48985647 = UmdGPpNnPy16537640;     UmdGPpNnPy16537640 = UmdGPpNnPy22867318;     UmdGPpNnPy22867318 = UmdGPpNnPy79003386;     UmdGPpNnPy79003386 = UmdGPpNnPy96475252;     UmdGPpNnPy96475252 = UmdGPpNnPy91206977;     UmdGPpNnPy91206977 = UmdGPpNnPy13304328;     UmdGPpNnPy13304328 = UmdGPpNnPy84771397;     UmdGPpNnPy84771397 = UmdGPpNnPy35622942;     UmdGPpNnPy35622942 = UmdGPpNnPy6079191;     UmdGPpNnPy6079191 = UmdGPpNnPy58168613;     UmdGPpNnPy58168613 = UmdGPpNnPy19230160;     UmdGPpNnPy19230160 = UmdGPpNnPy96808330;     UmdGPpNnPy96808330 = UmdGPpNnPy79475646;     UmdGPpNnPy79475646 = UmdGPpNnPy40488394;     UmdGPpNnPy40488394 = UmdGPpNnPy89886458;     UmdGPpNnPy89886458 = UmdGPpNnPy67067395;     UmdGPpNnPy67067395 = UmdGPpNnPy40802227;     UmdGPpNnPy40802227 = UmdGPpNnPy78801624;     UmdGPpNnPy78801624 = UmdGPpNnPy23009270;     UmdGPpNnPy23009270 = UmdGPpNnPy67472346;     UmdGPpNnPy67472346 = UmdGPpNnPy78642080;     UmdGPpNnPy78642080 = UmdGPpNnPy81341047;     UmdGPpNnPy81341047 = UmdGPpNnPy14023380;     UmdGPpNnPy14023380 = UmdGPpNnPy73884271;     UmdGPpNnPy73884271 = UmdGPpNnPy98663388;     UmdGPpNnPy98663388 = UmdGPpNnPy16996651;     UmdGPpNnPy16996651 = UmdGPpNnPy62358244;     UmdGPpNnPy62358244 = UmdGPpNnPy4411175;     UmdGPpNnPy4411175 = UmdGPpNnPy53476462;     UmdGPpNnPy53476462 = UmdGPpNnPy88781989;     UmdGPpNnPy88781989 = UmdGPpNnPy87529213;     UmdGPpNnPy87529213 = UmdGPpNnPy81032213;     UmdGPpNnPy81032213 = UmdGPpNnPy58699678;     UmdGPpNnPy58699678 = UmdGPpNnPy85967677;     UmdGPpNnPy85967677 = UmdGPpNnPy69163622;     UmdGPpNnPy69163622 = UmdGPpNnPy53976159;     UmdGPpNnPy53976159 = UmdGPpNnPy63923022;     UmdGPpNnPy63923022 = UmdGPpNnPy10994356;     UmdGPpNnPy10994356 = UmdGPpNnPy71096079;     UmdGPpNnPy71096079 = UmdGPpNnPy18920725;     UmdGPpNnPy18920725 = UmdGPpNnPy23256089;     UmdGPpNnPy23256089 = UmdGPpNnPy8865699;     UmdGPpNnPy8865699 = UmdGPpNnPy70704796;     UmdGPpNnPy70704796 = UmdGPpNnPy24809145;     UmdGPpNnPy24809145 = UmdGPpNnPy50252520;     UmdGPpNnPy50252520 = UmdGPpNnPy72045488;     UmdGPpNnPy72045488 = UmdGPpNnPy49644993;     UmdGPpNnPy49644993 = UmdGPpNnPy39767418;     UmdGPpNnPy39767418 = UmdGPpNnPy76798635;     UmdGPpNnPy76798635 = UmdGPpNnPy73276734;     UmdGPpNnPy73276734 = UmdGPpNnPy83978835;     UmdGPpNnPy83978835 = UmdGPpNnPy10488272;     UmdGPpNnPy10488272 = UmdGPpNnPy21620838;     UmdGPpNnPy21620838 = UmdGPpNnPy76227746;     UmdGPpNnPy76227746 = UmdGPpNnPy70343566;     UmdGPpNnPy70343566 = UmdGPpNnPy35196593;     UmdGPpNnPy35196593 = UmdGPpNnPy99208101;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yOrWmdvEdC15018158() {     int nxTkqBwKfw66655137 = -502760814;    int nxTkqBwKfw9625163 = -550993445;    int nxTkqBwKfw3361890 = -760684877;    int nxTkqBwKfw61025841 = -193450260;    int nxTkqBwKfw69116477 = -135489461;    int nxTkqBwKfw70950870 = -322678892;    int nxTkqBwKfw14865808 = -139416997;    int nxTkqBwKfw41914488 = -950639955;    int nxTkqBwKfw75938888 = -517931996;    int nxTkqBwKfw15341899 = 74095378;    int nxTkqBwKfw43903106 = -261375337;    int nxTkqBwKfw77424678 = -929690890;    int nxTkqBwKfw45474656 = 98908005;    int nxTkqBwKfw56263497 = -204352439;    int nxTkqBwKfw54475807 = -816806501;    int nxTkqBwKfw86242112 = -520368717;    int nxTkqBwKfw70827090 = -132998735;    int nxTkqBwKfw21975132 = -176729152;    int nxTkqBwKfw95516013 = -90919643;    int nxTkqBwKfw25034398 = -565045635;    int nxTkqBwKfw60341342 = -31934276;    int nxTkqBwKfw4403914 = -944953986;    int nxTkqBwKfw73305055 = -798840207;    int nxTkqBwKfw87986651 = -885668413;    int nxTkqBwKfw50047663 = -415793293;    int nxTkqBwKfw48911924 = -537434967;    int nxTkqBwKfw50653486 = -484012916;    int nxTkqBwKfw47501457 = -790685973;    int nxTkqBwKfw76061335 = -895557111;    int nxTkqBwKfw79124140 = -397762383;    int nxTkqBwKfw7217487 = -784670910;    int nxTkqBwKfw26370563 = -91203198;    int nxTkqBwKfw11227161 = 41437040;    int nxTkqBwKfw40932109 = -334555976;    int nxTkqBwKfw4060471 = 27873218;    int nxTkqBwKfw68843106 = -309949971;    int nxTkqBwKfw79782992 = -395675188;    int nxTkqBwKfw67448676 = -832543158;    int nxTkqBwKfw61575568 = -863396712;    int nxTkqBwKfw3159295 = -421439234;    int nxTkqBwKfw18824937 = -6168846;    int nxTkqBwKfw3309562 = -724567804;    int nxTkqBwKfw9300386 = -808941320;    int nxTkqBwKfw43376686 = -25287154;    int nxTkqBwKfw72512141 = -178722962;    int nxTkqBwKfw11401081 = -712556111;    int nxTkqBwKfw45361325 = -290422715;    int nxTkqBwKfw73958946 = -74472090;    int nxTkqBwKfw58955404 = -728112332;    int nxTkqBwKfw85050987 = -568150267;    int nxTkqBwKfw64953060 = -200934812;    int nxTkqBwKfw38634382 = -311250352;    int nxTkqBwKfw96843995 = -671738354;    int nxTkqBwKfw50033825 = 52820658;    int nxTkqBwKfw54676815 = -263290136;    int nxTkqBwKfw62251224 = -557806828;    int nxTkqBwKfw36320107 = -752153238;    int nxTkqBwKfw15375238 = -875016464;    int nxTkqBwKfw10978179 = -777656968;    int nxTkqBwKfw20204553 = -598054494;    int nxTkqBwKfw20297384 = -838665976;    int nxTkqBwKfw67364350 = -348731024;    int nxTkqBwKfw65853152 = 44917155;    int nxTkqBwKfw96814748 = -20169613;    int nxTkqBwKfw8124412 = -141233712;    int nxTkqBwKfw17532543 = -70172139;    int nxTkqBwKfw66197518 = -871127930;    int nxTkqBwKfw4542548 = -566536019;    int nxTkqBwKfw52203026 = -132225657;    int nxTkqBwKfw85632700 = -406856530;    int nxTkqBwKfw6459121 = -24693529;    int nxTkqBwKfw3378415 = -300455578;    int nxTkqBwKfw60399563 = -313332440;    int nxTkqBwKfw92356718 = -669480410;    int nxTkqBwKfw6209462 = -458876790;    int nxTkqBwKfw57031781 = -307366472;    int nxTkqBwKfw95103527 = -36012667;    int nxTkqBwKfw29928370 = -673553054;    int nxTkqBwKfw15474511 = -606945452;    int nxTkqBwKfw38646582 = -703237182;    int nxTkqBwKfw3550599 = -147012253;    int nxTkqBwKfw76694539 = -309540827;    int nxTkqBwKfw88546053 = 37426358;    int nxTkqBwKfw91010348 = -227406844;    int nxTkqBwKfw14171080 = -96827572;    int nxTkqBwKfw68583105 = -373420559;    int nxTkqBwKfw29526568 = -419464844;    int nxTkqBwKfw61193335 = 88616381;    int nxTkqBwKfw86255293 = 28734160;    int nxTkqBwKfw41809246 = -414319955;    int nxTkqBwKfw32523000 = -557796734;    int nxTkqBwKfw64407754 = -520658724;    int nxTkqBwKfw56470497 = 45113810;    int nxTkqBwKfw41371015 = -165342219;    int nxTkqBwKfw82861910 = -582773258;    int nxTkqBwKfw51460587 = -657437822;    int nxTkqBwKfw37456409 = -669484959;    int nxTkqBwKfw12485637 = -688771707;    int nxTkqBwKfw35252275 = -884053442;    int nxTkqBwKfw54979599 = -502760814;     nxTkqBwKfw66655137 = nxTkqBwKfw9625163;     nxTkqBwKfw9625163 = nxTkqBwKfw3361890;     nxTkqBwKfw3361890 = nxTkqBwKfw61025841;     nxTkqBwKfw61025841 = nxTkqBwKfw69116477;     nxTkqBwKfw69116477 = nxTkqBwKfw70950870;     nxTkqBwKfw70950870 = nxTkqBwKfw14865808;     nxTkqBwKfw14865808 = nxTkqBwKfw41914488;     nxTkqBwKfw41914488 = nxTkqBwKfw75938888;     nxTkqBwKfw75938888 = nxTkqBwKfw15341899;     nxTkqBwKfw15341899 = nxTkqBwKfw43903106;     nxTkqBwKfw43903106 = nxTkqBwKfw77424678;     nxTkqBwKfw77424678 = nxTkqBwKfw45474656;     nxTkqBwKfw45474656 = nxTkqBwKfw56263497;     nxTkqBwKfw56263497 = nxTkqBwKfw54475807;     nxTkqBwKfw54475807 = nxTkqBwKfw86242112;     nxTkqBwKfw86242112 = nxTkqBwKfw70827090;     nxTkqBwKfw70827090 = nxTkqBwKfw21975132;     nxTkqBwKfw21975132 = nxTkqBwKfw95516013;     nxTkqBwKfw95516013 = nxTkqBwKfw25034398;     nxTkqBwKfw25034398 = nxTkqBwKfw60341342;     nxTkqBwKfw60341342 = nxTkqBwKfw4403914;     nxTkqBwKfw4403914 = nxTkqBwKfw73305055;     nxTkqBwKfw73305055 = nxTkqBwKfw87986651;     nxTkqBwKfw87986651 = nxTkqBwKfw50047663;     nxTkqBwKfw50047663 = nxTkqBwKfw48911924;     nxTkqBwKfw48911924 = nxTkqBwKfw50653486;     nxTkqBwKfw50653486 = nxTkqBwKfw47501457;     nxTkqBwKfw47501457 = nxTkqBwKfw76061335;     nxTkqBwKfw76061335 = nxTkqBwKfw79124140;     nxTkqBwKfw79124140 = nxTkqBwKfw7217487;     nxTkqBwKfw7217487 = nxTkqBwKfw26370563;     nxTkqBwKfw26370563 = nxTkqBwKfw11227161;     nxTkqBwKfw11227161 = nxTkqBwKfw40932109;     nxTkqBwKfw40932109 = nxTkqBwKfw4060471;     nxTkqBwKfw4060471 = nxTkqBwKfw68843106;     nxTkqBwKfw68843106 = nxTkqBwKfw79782992;     nxTkqBwKfw79782992 = nxTkqBwKfw67448676;     nxTkqBwKfw67448676 = nxTkqBwKfw61575568;     nxTkqBwKfw61575568 = nxTkqBwKfw3159295;     nxTkqBwKfw3159295 = nxTkqBwKfw18824937;     nxTkqBwKfw18824937 = nxTkqBwKfw3309562;     nxTkqBwKfw3309562 = nxTkqBwKfw9300386;     nxTkqBwKfw9300386 = nxTkqBwKfw43376686;     nxTkqBwKfw43376686 = nxTkqBwKfw72512141;     nxTkqBwKfw72512141 = nxTkqBwKfw11401081;     nxTkqBwKfw11401081 = nxTkqBwKfw45361325;     nxTkqBwKfw45361325 = nxTkqBwKfw73958946;     nxTkqBwKfw73958946 = nxTkqBwKfw58955404;     nxTkqBwKfw58955404 = nxTkqBwKfw85050987;     nxTkqBwKfw85050987 = nxTkqBwKfw64953060;     nxTkqBwKfw64953060 = nxTkqBwKfw38634382;     nxTkqBwKfw38634382 = nxTkqBwKfw96843995;     nxTkqBwKfw96843995 = nxTkqBwKfw50033825;     nxTkqBwKfw50033825 = nxTkqBwKfw54676815;     nxTkqBwKfw54676815 = nxTkqBwKfw62251224;     nxTkqBwKfw62251224 = nxTkqBwKfw36320107;     nxTkqBwKfw36320107 = nxTkqBwKfw15375238;     nxTkqBwKfw15375238 = nxTkqBwKfw10978179;     nxTkqBwKfw10978179 = nxTkqBwKfw20204553;     nxTkqBwKfw20204553 = nxTkqBwKfw20297384;     nxTkqBwKfw20297384 = nxTkqBwKfw67364350;     nxTkqBwKfw67364350 = nxTkqBwKfw65853152;     nxTkqBwKfw65853152 = nxTkqBwKfw96814748;     nxTkqBwKfw96814748 = nxTkqBwKfw8124412;     nxTkqBwKfw8124412 = nxTkqBwKfw17532543;     nxTkqBwKfw17532543 = nxTkqBwKfw66197518;     nxTkqBwKfw66197518 = nxTkqBwKfw4542548;     nxTkqBwKfw4542548 = nxTkqBwKfw52203026;     nxTkqBwKfw52203026 = nxTkqBwKfw85632700;     nxTkqBwKfw85632700 = nxTkqBwKfw6459121;     nxTkqBwKfw6459121 = nxTkqBwKfw3378415;     nxTkqBwKfw3378415 = nxTkqBwKfw60399563;     nxTkqBwKfw60399563 = nxTkqBwKfw92356718;     nxTkqBwKfw92356718 = nxTkqBwKfw6209462;     nxTkqBwKfw6209462 = nxTkqBwKfw57031781;     nxTkqBwKfw57031781 = nxTkqBwKfw95103527;     nxTkqBwKfw95103527 = nxTkqBwKfw29928370;     nxTkqBwKfw29928370 = nxTkqBwKfw15474511;     nxTkqBwKfw15474511 = nxTkqBwKfw38646582;     nxTkqBwKfw38646582 = nxTkqBwKfw3550599;     nxTkqBwKfw3550599 = nxTkqBwKfw76694539;     nxTkqBwKfw76694539 = nxTkqBwKfw88546053;     nxTkqBwKfw88546053 = nxTkqBwKfw91010348;     nxTkqBwKfw91010348 = nxTkqBwKfw14171080;     nxTkqBwKfw14171080 = nxTkqBwKfw68583105;     nxTkqBwKfw68583105 = nxTkqBwKfw29526568;     nxTkqBwKfw29526568 = nxTkqBwKfw61193335;     nxTkqBwKfw61193335 = nxTkqBwKfw86255293;     nxTkqBwKfw86255293 = nxTkqBwKfw41809246;     nxTkqBwKfw41809246 = nxTkqBwKfw32523000;     nxTkqBwKfw32523000 = nxTkqBwKfw64407754;     nxTkqBwKfw64407754 = nxTkqBwKfw56470497;     nxTkqBwKfw56470497 = nxTkqBwKfw41371015;     nxTkqBwKfw41371015 = nxTkqBwKfw82861910;     nxTkqBwKfw82861910 = nxTkqBwKfw51460587;     nxTkqBwKfw51460587 = nxTkqBwKfw37456409;     nxTkqBwKfw37456409 = nxTkqBwKfw12485637;     nxTkqBwKfw12485637 = nxTkqBwKfw35252275;     nxTkqBwKfw35252275 = nxTkqBwKfw54979599;     nxTkqBwKfw54979599 = nxTkqBwKfw66655137;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CwOCdBnqYR33231499() {     int QAnlxgeQrK50099443 = -283426495;    int QAnlxgeQrK26497252 = -550375632;    int QAnlxgeQrK99770396 = -554210105;    int QAnlxgeQrK23818093 = -387250096;    int QAnlxgeQrK75237800 = 57643048;    int QAnlxgeQrK69455271 = -101750545;    int QAnlxgeQrK85905125 = -979328311;    int QAnlxgeQrK56261930 = -937797984;    int QAnlxgeQrK34468589 = -859391347;    int QAnlxgeQrK47887550 = -394280734;    int QAnlxgeQrK93319043 = -451491210;    int QAnlxgeQrK40630078 = -917309137;    int QAnlxgeQrK63592431 = -792753236;    int QAnlxgeQrK35890389 = 7357204;    int QAnlxgeQrK15330271 = -910829483;    int QAnlxgeQrK72631274 = -308448042;    int QAnlxgeQrK64578975 = -986343762;    int QAnlxgeQrK2826579 = -424034028;    int QAnlxgeQrK57956155 = -89594514;    int QAnlxgeQrK14401841 = 5368372;    int QAnlxgeQrK15981729 = -721463307;    int QAnlxgeQrK16551232 = -936105757;    int QAnlxgeQrK77130763 = -838729404;    int QAnlxgeQrK22693210 = -957493809;    int QAnlxgeQrK91623621 = -975446051;    int QAnlxgeQrK44254073 = -417944683;    int QAnlxgeQrK108227 = -112097258;    int QAnlxgeQrK87233082 = -860640857;    int QAnlxgeQrK12142430 = -758062253;    int QAnlxgeQrK94957481 = -910612234;    int QAnlxgeQrK25312801 = -422428111;    int QAnlxgeQrK89476990 = -6398278;    int QAnlxgeQrK54766047 = -679435138;    int QAnlxgeQrK83090619 = 73841854;    int QAnlxgeQrK82787200 = -169791518;    int QAnlxgeQrK81232291 = -97973877;    int QAnlxgeQrK48397171 = -274524368;    int QAnlxgeQrK8027509 = -541013678;    int QAnlxgeQrK10570647 = 51705490;    int QAnlxgeQrK52654429 = -174032756;    int QAnlxgeQrK52473861 = -677717819;    int QAnlxgeQrK53797347 = -198476125;    int QAnlxgeQrK32867455 = -259998563;    int QAnlxgeQrK35281626 = -881469925;    int QAnlxgeQrK64456638 = 93120085;    int QAnlxgeQrK51007077 = -732167035;    int QAnlxgeQrK21376755 = -422726945;    int QAnlxgeQrK93629542 = -73193808;    int QAnlxgeQrK89802516 = -213714414;    int QAnlxgeQrK20383472 = -775170722;    int QAnlxgeQrK38210509 = -421836234;    int QAnlxgeQrK94135176 = -369509663;    int QAnlxgeQrK10956455 = -883485670;    int QAnlxgeQrK74710584 = -765592730;    int QAnlxgeQrK17249770 = -611776259;    int QAnlxgeQrK33548212 = -347320738;    int QAnlxgeQrK49366488 = -711646229;    int QAnlxgeQrK77077187 = -596716296;    int QAnlxgeQrK32194472 = -411804045;    int QAnlxgeQrK30983728 = -524412270;    int QAnlxgeQrK69347044 = -989653287;    int QAnlxgeQrK98672042 = -18687454;    int QAnlxgeQrK44119501 = -79735732;    int QAnlxgeQrK39511108 = -948779113;    int QAnlxgeQrK22574750 = -971852624;    int QAnlxgeQrK3842054 = -345092932;    int QAnlxgeQrK85864030 = -137873999;    int QAnlxgeQrK80501812 = -766595090;    int QAnlxgeQrK53103188 = -822851279;    int QAnlxgeQrK34097980 = -712855607;    int QAnlxgeQrK24234104 = 66076325;    int QAnlxgeQrK56551467 = -345330085;    int QAnlxgeQrK92255931 = -375739519;    int QAnlxgeQrK5301726 = -915561758;    int QAnlxgeQrK61927979 = -316913809;    int QAnlxgeQrK62184382 = -422987182;    int QAnlxgeQrK83683776 = -576107194;    int QAnlxgeQrK41849137 = -957259480;    int QAnlxgeQrK58236572 = -950613895;    int QAnlxgeQrK40616545 = -143279017;    int QAnlxgeQrK22877318 = -995217739;    int QAnlxgeQrK6478685 = 61096550;    int QAnlxgeQrK97430566 = -546926444;    int QAnlxgeQrK91758958 = -982891531;    int QAnlxgeQrK56746973 = -388776001;    int QAnlxgeQrK31177625 = 47081552;    int QAnlxgeQrK78520535 = -122912608;    int QAnlxgeQrK80055463 = -913842409;    int QAnlxgeQrK65840849 = -314381887;    int QAnlxgeQrK49238989 = -822470780;    int QAnlxgeQrK31865803 = -386327649;    int QAnlxgeQrK71319983 = -677808072;    int QAnlxgeQrK75833036 = -29209633;    int QAnlxgeQrK79586918 = -423882241;    int QAnlxgeQrK83307385 = -184379470;    int QAnlxgeQrK53801819 = -559030366;    int QAnlxgeQrK9677847 = -18740394;    int QAnlxgeQrK93356347 = -311219451;    int QAnlxgeQrK12706877 = -909617302;    int QAnlxgeQrK60614585 = -283426495;     QAnlxgeQrK50099443 = QAnlxgeQrK26497252;     QAnlxgeQrK26497252 = QAnlxgeQrK99770396;     QAnlxgeQrK99770396 = QAnlxgeQrK23818093;     QAnlxgeQrK23818093 = QAnlxgeQrK75237800;     QAnlxgeQrK75237800 = QAnlxgeQrK69455271;     QAnlxgeQrK69455271 = QAnlxgeQrK85905125;     QAnlxgeQrK85905125 = QAnlxgeQrK56261930;     QAnlxgeQrK56261930 = QAnlxgeQrK34468589;     QAnlxgeQrK34468589 = QAnlxgeQrK47887550;     QAnlxgeQrK47887550 = QAnlxgeQrK93319043;     QAnlxgeQrK93319043 = QAnlxgeQrK40630078;     QAnlxgeQrK40630078 = QAnlxgeQrK63592431;     QAnlxgeQrK63592431 = QAnlxgeQrK35890389;     QAnlxgeQrK35890389 = QAnlxgeQrK15330271;     QAnlxgeQrK15330271 = QAnlxgeQrK72631274;     QAnlxgeQrK72631274 = QAnlxgeQrK64578975;     QAnlxgeQrK64578975 = QAnlxgeQrK2826579;     QAnlxgeQrK2826579 = QAnlxgeQrK57956155;     QAnlxgeQrK57956155 = QAnlxgeQrK14401841;     QAnlxgeQrK14401841 = QAnlxgeQrK15981729;     QAnlxgeQrK15981729 = QAnlxgeQrK16551232;     QAnlxgeQrK16551232 = QAnlxgeQrK77130763;     QAnlxgeQrK77130763 = QAnlxgeQrK22693210;     QAnlxgeQrK22693210 = QAnlxgeQrK91623621;     QAnlxgeQrK91623621 = QAnlxgeQrK44254073;     QAnlxgeQrK44254073 = QAnlxgeQrK108227;     QAnlxgeQrK108227 = QAnlxgeQrK87233082;     QAnlxgeQrK87233082 = QAnlxgeQrK12142430;     QAnlxgeQrK12142430 = QAnlxgeQrK94957481;     QAnlxgeQrK94957481 = QAnlxgeQrK25312801;     QAnlxgeQrK25312801 = QAnlxgeQrK89476990;     QAnlxgeQrK89476990 = QAnlxgeQrK54766047;     QAnlxgeQrK54766047 = QAnlxgeQrK83090619;     QAnlxgeQrK83090619 = QAnlxgeQrK82787200;     QAnlxgeQrK82787200 = QAnlxgeQrK81232291;     QAnlxgeQrK81232291 = QAnlxgeQrK48397171;     QAnlxgeQrK48397171 = QAnlxgeQrK8027509;     QAnlxgeQrK8027509 = QAnlxgeQrK10570647;     QAnlxgeQrK10570647 = QAnlxgeQrK52654429;     QAnlxgeQrK52654429 = QAnlxgeQrK52473861;     QAnlxgeQrK52473861 = QAnlxgeQrK53797347;     QAnlxgeQrK53797347 = QAnlxgeQrK32867455;     QAnlxgeQrK32867455 = QAnlxgeQrK35281626;     QAnlxgeQrK35281626 = QAnlxgeQrK64456638;     QAnlxgeQrK64456638 = QAnlxgeQrK51007077;     QAnlxgeQrK51007077 = QAnlxgeQrK21376755;     QAnlxgeQrK21376755 = QAnlxgeQrK93629542;     QAnlxgeQrK93629542 = QAnlxgeQrK89802516;     QAnlxgeQrK89802516 = QAnlxgeQrK20383472;     QAnlxgeQrK20383472 = QAnlxgeQrK38210509;     QAnlxgeQrK38210509 = QAnlxgeQrK94135176;     QAnlxgeQrK94135176 = QAnlxgeQrK10956455;     QAnlxgeQrK10956455 = QAnlxgeQrK74710584;     QAnlxgeQrK74710584 = QAnlxgeQrK17249770;     QAnlxgeQrK17249770 = QAnlxgeQrK33548212;     QAnlxgeQrK33548212 = QAnlxgeQrK49366488;     QAnlxgeQrK49366488 = QAnlxgeQrK77077187;     QAnlxgeQrK77077187 = QAnlxgeQrK32194472;     QAnlxgeQrK32194472 = QAnlxgeQrK30983728;     QAnlxgeQrK30983728 = QAnlxgeQrK69347044;     QAnlxgeQrK69347044 = QAnlxgeQrK98672042;     QAnlxgeQrK98672042 = QAnlxgeQrK44119501;     QAnlxgeQrK44119501 = QAnlxgeQrK39511108;     QAnlxgeQrK39511108 = QAnlxgeQrK22574750;     QAnlxgeQrK22574750 = QAnlxgeQrK3842054;     QAnlxgeQrK3842054 = QAnlxgeQrK85864030;     QAnlxgeQrK85864030 = QAnlxgeQrK80501812;     QAnlxgeQrK80501812 = QAnlxgeQrK53103188;     QAnlxgeQrK53103188 = QAnlxgeQrK34097980;     QAnlxgeQrK34097980 = QAnlxgeQrK24234104;     QAnlxgeQrK24234104 = QAnlxgeQrK56551467;     QAnlxgeQrK56551467 = QAnlxgeQrK92255931;     QAnlxgeQrK92255931 = QAnlxgeQrK5301726;     QAnlxgeQrK5301726 = QAnlxgeQrK61927979;     QAnlxgeQrK61927979 = QAnlxgeQrK62184382;     QAnlxgeQrK62184382 = QAnlxgeQrK83683776;     QAnlxgeQrK83683776 = QAnlxgeQrK41849137;     QAnlxgeQrK41849137 = QAnlxgeQrK58236572;     QAnlxgeQrK58236572 = QAnlxgeQrK40616545;     QAnlxgeQrK40616545 = QAnlxgeQrK22877318;     QAnlxgeQrK22877318 = QAnlxgeQrK6478685;     QAnlxgeQrK6478685 = QAnlxgeQrK97430566;     QAnlxgeQrK97430566 = QAnlxgeQrK91758958;     QAnlxgeQrK91758958 = QAnlxgeQrK56746973;     QAnlxgeQrK56746973 = QAnlxgeQrK31177625;     QAnlxgeQrK31177625 = QAnlxgeQrK78520535;     QAnlxgeQrK78520535 = QAnlxgeQrK80055463;     QAnlxgeQrK80055463 = QAnlxgeQrK65840849;     QAnlxgeQrK65840849 = QAnlxgeQrK49238989;     QAnlxgeQrK49238989 = QAnlxgeQrK31865803;     QAnlxgeQrK31865803 = QAnlxgeQrK71319983;     QAnlxgeQrK71319983 = QAnlxgeQrK75833036;     QAnlxgeQrK75833036 = QAnlxgeQrK79586918;     QAnlxgeQrK79586918 = QAnlxgeQrK83307385;     QAnlxgeQrK83307385 = QAnlxgeQrK53801819;     QAnlxgeQrK53801819 = QAnlxgeQrK9677847;     QAnlxgeQrK9677847 = QAnlxgeQrK93356347;     QAnlxgeQrK93356347 = QAnlxgeQrK12706877;     QAnlxgeQrK12706877 = QAnlxgeQrK60614585;     QAnlxgeQrK60614585 = QAnlxgeQrK50099443;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bumJGyebrc3687372() {     int fIzOrjzKFB62885610 = -418467186;    int fIzOrjzKFB13268140 = -720709646;    int fIzOrjzKFB17523197 = -962569487;    int fIzOrjzKFB31758919 = -211452373;    int fIzOrjzKFB68748905 = -434173417;    int fIzOrjzKFB12765342 = -978673858;    int fIzOrjzKFB7112884 = -425000381;    int fIzOrjzKFB82981043 = -282316241;    int fIzOrjzKFB29873400 = -821149439;    int fIzOrjzKFB54106255 = -198368367;    int fIzOrjzKFB20806426 = -566177225;    int fIzOrjzKFB72580582 = -324641953;    int fIzOrjzKFB33735735 = -696535893;    int fIzOrjzKFB85514467 = -325057994;    int fIzOrjzKFB38636824 = -817815314;    int fIzOrjzKFB1056343 = -859339431;    int fIzOrjzKFB20330793 = -650118229;    int fIzOrjzKFB86385063 = 20755670;    int fIzOrjzKFB59116786 = 936291;    int fIzOrjzKFB17056337 = -593875767;    int fIzOrjzKFB96155785 = -112779571;    int fIzOrjzKFB45599050 = -5964104;    int fIzOrjzKFB15891059 = -769733299;    int fIzOrjzKFB67200339 = -217838822;    int fIzOrjzKFB97517880 = -85887976;    int fIzOrjzKFB25572718 = -108375882;    int fIzOrjzKFB11523763 = -745803539;    int fIzOrjzKFB54586008 = 65561038;    int fIzOrjzKFB28759483 = -972279493;    int fIzOrjzKFB47708597 = -540498389;    int fIzOrjzKFB69748796 = -698844861;    int fIzOrjzKFB92795813 = 14882044;    int fIzOrjzKFB811997 = -555289922;    int fIzOrjzKFB60166992 = -930912364;    int fIzOrjzKFB65847733 = -779823398;    int fIzOrjzKFB76456405 = -986536981;    int fIzOrjzKFB85161231 = -64433718;    int fIzOrjzKFB65731531 = -460633769;    int fIzOrjzKFB16346033 = -890145087;    int fIzOrjzKFB78919159 = -18449602;    int fIzOrjzKFB13905169 = 46808188;    int fIzOrjzKFB61499803 = -96787460;    int fIzOrjzKFB67750924 = -283432008;    int fIzOrjzKFB79910164 = -122642168;    int fIzOrjzKFB44465129 = -899649875;    int fIzOrjzKFB24635206 = -805485474;    int fIzOrjzKFB98958774 = -920241997;    int fIzOrjzKFB23069509 = -228546588;    int fIzOrjzKFB33866566 = -377316389;    int fIzOrjzKFB67333892 = -286207185;    int fIzOrjzKFB56538002 = -674720542;    int fIzOrjzKFB22311724 = -840695001;    int fIzOrjzKFB45441605 = 40511047;    int fIzOrjzKFB19410310 = -355044472;    int fIzOrjzKFB7531586 = -606220746;    int fIzOrjzKFB17286560 = -312503082;    int fIzOrjzKFB97377080 = -950976347;    int fIzOrjzKFB50322857 = -644730665;    int fIzOrjzKFB34241039 = -25564397;    int fIzOrjzKFB43176187 = -225797536;    int fIzOrjzKFB1241579 = -132870320;    int fIzOrjzKFB52526875 = -390561420;    int fIzOrjzKFB54221561 = -310036749;    int fIzOrjzKFB82164803 = -180651050;    int fIzOrjzKFB84357458 = -499523506;    int fIzOrjzKFB28010612 = -481059269;    int fIzOrjzKFB71768586 = -769352031;    int fIzOrjzKFB73568742 = -765623529;    int fIzOrjzKFB19666735 = -545234597;    int fIzOrjzKFB62180418 = -831278333;    int fIzOrjzKFB15895112 = -694905714;    int fIzOrjzKFB54599262 = -89484460;    int fIzOrjzKFB70039030 = -89099244;    int fIzOrjzKFB80197626 = -980614108;    int fIzOrjzKFB3151168 = -540683955;    int fIzOrjzKFB34655983 = 84007888;    int fIzOrjzKFB77848125 = -722532097;    int fIzOrjzKFB35980895 = -547091132;    int fIzOrjzKFB22735211 = -318188947;    int fIzOrjzKFB72882674 = -280402502;    int fIzOrjzKFB26613943 = -188133886;    int fIzOrjzKFB88454254 = -417256951;    int fIzOrjzKFB20719442 = -557122574;    int fIzOrjzKFB61425590 = -586072308;    int fIzOrjzKFB91170594 = -865777847;    int fIzOrjzKFB47437073 = -858149860;    int fIzOrjzKFB47354209 = 74370996;    int fIzOrjzKFB81401686 = -100245451;    int fIzOrjzKFB52635406 = -224691619;    int fIzOrjzKFB48561173 = -367320316;    int fIzOrjzKFB79079325 = 64439366;    int fIzOrjzKFB34838374 = -419703053;    int fIzOrjzKFB31490493 = -335069372;    int fIzOrjzKFB73169845 = -564347552;    int fIzOrjzKFB77677580 = -885579282;    int fIzOrjzKFB61378293 = -562630393;    int fIzOrjzKFB7278243 = -786750711;    int fIzOrjzKFB85586121 = -2780958;    int fIzOrjzKFB95552705 = -623118662;    int fIzOrjzKFB16454517 = -418467186;     fIzOrjzKFB62885610 = fIzOrjzKFB13268140;     fIzOrjzKFB13268140 = fIzOrjzKFB17523197;     fIzOrjzKFB17523197 = fIzOrjzKFB31758919;     fIzOrjzKFB31758919 = fIzOrjzKFB68748905;     fIzOrjzKFB68748905 = fIzOrjzKFB12765342;     fIzOrjzKFB12765342 = fIzOrjzKFB7112884;     fIzOrjzKFB7112884 = fIzOrjzKFB82981043;     fIzOrjzKFB82981043 = fIzOrjzKFB29873400;     fIzOrjzKFB29873400 = fIzOrjzKFB54106255;     fIzOrjzKFB54106255 = fIzOrjzKFB20806426;     fIzOrjzKFB20806426 = fIzOrjzKFB72580582;     fIzOrjzKFB72580582 = fIzOrjzKFB33735735;     fIzOrjzKFB33735735 = fIzOrjzKFB85514467;     fIzOrjzKFB85514467 = fIzOrjzKFB38636824;     fIzOrjzKFB38636824 = fIzOrjzKFB1056343;     fIzOrjzKFB1056343 = fIzOrjzKFB20330793;     fIzOrjzKFB20330793 = fIzOrjzKFB86385063;     fIzOrjzKFB86385063 = fIzOrjzKFB59116786;     fIzOrjzKFB59116786 = fIzOrjzKFB17056337;     fIzOrjzKFB17056337 = fIzOrjzKFB96155785;     fIzOrjzKFB96155785 = fIzOrjzKFB45599050;     fIzOrjzKFB45599050 = fIzOrjzKFB15891059;     fIzOrjzKFB15891059 = fIzOrjzKFB67200339;     fIzOrjzKFB67200339 = fIzOrjzKFB97517880;     fIzOrjzKFB97517880 = fIzOrjzKFB25572718;     fIzOrjzKFB25572718 = fIzOrjzKFB11523763;     fIzOrjzKFB11523763 = fIzOrjzKFB54586008;     fIzOrjzKFB54586008 = fIzOrjzKFB28759483;     fIzOrjzKFB28759483 = fIzOrjzKFB47708597;     fIzOrjzKFB47708597 = fIzOrjzKFB69748796;     fIzOrjzKFB69748796 = fIzOrjzKFB92795813;     fIzOrjzKFB92795813 = fIzOrjzKFB811997;     fIzOrjzKFB811997 = fIzOrjzKFB60166992;     fIzOrjzKFB60166992 = fIzOrjzKFB65847733;     fIzOrjzKFB65847733 = fIzOrjzKFB76456405;     fIzOrjzKFB76456405 = fIzOrjzKFB85161231;     fIzOrjzKFB85161231 = fIzOrjzKFB65731531;     fIzOrjzKFB65731531 = fIzOrjzKFB16346033;     fIzOrjzKFB16346033 = fIzOrjzKFB78919159;     fIzOrjzKFB78919159 = fIzOrjzKFB13905169;     fIzOrjzKFB13905169 = fIzOrjzKFB61499803;     fIzOrjzKFB61499803 = fIzOrjzKFB67750924;     fIzOrjzKFB67750924 = fIzOrjzKFB79910164;     fIzOrjzKFB79910164 = fIzOrjzKFB44465129;     fIzOrjzKFB44465129 = fIzOrjzKFB24635206;     fIzOrjzKFB24635206 = fIzOrjzKFB98958774;     fIzOrjzKFB98958774 = fIzOrjzKFB23069509;     fIzOrjzKFB23069509 = fIzOrjzKFB33866566;     fIzOrjzKFB33866566 = fIzOrjzKFB67333892;     fIzOrjzKFB67333892 = fIzOrjzKFB56538002;     fIzOrjzKFB56538002 = fIzOrjzKFB22311724;     fIzOrjzKFB22311724 = fIzOrjzKFB45441605;     fIzOrjzKFB45441605 = fIzOrjzKFB19410310;     fIzOrjzKFB19410310 = fIzOrjzKFB7531586;     fIzOrjzKFB7531586 = fIzOrjzKFB17286560;     fIzOrjzKFB17286560 = fIzOrjzKFB97377080;     fIzOrjzKFB97377080 = fIzOrjzKFB50322857;     fIzOrjzKFB50322857 = fIzOrjzKFB34241039;     fIzOrjzKFB34241039 = fIzOrjzKFB43176187;     fIzOrjzKFB43176187 = fIzOrjzKFB1241579;     fIzOrjzKFB1241579 = fIzOrjzKFB52526875;     fIzOrjzKFB52526875 = fIzOrjzKFB54221561;     fIzOrjzKFB54221561 = fIzOrjzKFB82164803;     fIzOrjzKFB82164803 = fIzOrjzKFB84357458;     fIzOrjzKFB84357458 = fIzOrjzKFB28010612;     fIzOrjzKFB28010612 = fIzOrjzKFB71768586;     fIzOrjzKFB71768586 = fIzOrjzKFB73568742;     fIzOrjzKFB73568742 = fIzOrjzKFB19666735;     fIzOrjzKFB19666735 = fIzOrjzKFB62180418;     fIzOrjzKFB62180418 = fIzOrjzKFB15895112;     fIzOrjzKFB15895112 = fIzOrjzKFB54599262;     fIzOrjzKFB54599262 = fIzOrjzKFB70039030;     fIzOrjzKFB70039030 = fIzOrjzKFB80197626;     fIzOrjzKFB80197626 = fIzOrjzKFB3151168;     fIzOrjzKFB3151168 = fIzOrjzKFB34655983;     fIzOrjzKFB34655983 = fIzOrjzKFB77848125;     fIzOrjzKFB77848125 = fIzOrjzKFB35980895;     fIzOrjzKFB35980895 = fIzOrjzKFB22735211;     fIzOrjzKFB22735211 = fIzOrjzKFB72882674;     fIzOrjzKFB72882674 = fIzOrjzKFB26613943;     fIzOrjzKFB26613943 = fIzOrjzKFB88454254;     fIzOrjzKFB88454254 = fIzOrjzKFB20719442;     fIzOrjzKFB20719442 = fIzOrjzKFB61425590;     fIzOrjzKFB61425590 = fIzOrjzKFB91170594;     fIzOrjzKFB91170594 = fIzOrjzKFB47437073;     fIzOrjzKFB47437073 = fIzOrjzKFB47354209;     fIzOrjzKFB47354209 = fIzOrjzKFB81401686;     fIzOrjzKFB81401686 = fIzOrjzKFB52635406;     fIzOrjzKFB52635406 = fIzOrjzKFB48561173;     fIzOrjzKFB48561173 = fIzOrjzKFB79079325;     fIzOrjzKFB79079325 = fIzOrjzKFB34838374;     fIzOrjzKFB34838374 = fIzOrjzKFB31490493;     fIzOrjzKFB31490493 = fIzOrjzKFB73169845;     fIzOrjzKFB73169845 = fIzOrjzKFB77677580;     fIzOrjzKFB77677580 = fIzOrjzKFB61378293;     fIzOrjzKFB61378293 = fIzOrjzKFB7278243;     fIzOrjzKFB7278243 = fIzOrjzKFB85586121;     fIzOrjzKFB85586121 = fIzOrjzKFB95552705;     fIzOrjzKFB95552705 = fIzOrjzKFB16454517;     fIzOrjzKFB16454517 = fIzOrjzKFB62885610;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oMoYtVkDMP21900713() {     int VSFBJAalsf46329916 = -199132867;    int VSFBJAalsf30140229 = -720091834;    int VSFBJAalsf13931704 = -756094714;    int VSFBJAalsf94551170 = -405252209;    int VSFBJAalsf74870229 = -241040908;    int VSFBJAalsf11269743 = -757745511;    int VSFBJAalsf78152201 = -164911694;    int VSFBJAalsf97328485 = -269474270;    int VSFBJAalsf88403100 = -62608790;    int VSFBJAalsf86651907 = -666744479;    int VSFBJAalsf70222363 = -756293098;    int VSFBJAalsf35785982 = -312260199;    int VSFBJAalsf51853510 = -488197134;    int VSFBJAalsf65141359 = -113348351;    int VSFBJAalsf99491287 = -911838296;    int VSFBJAalsf87445504 = -647418757;    int VSFBJAalsf14082678 = -403463256;    int VSFBJAalsf67236509 = -226549207;    int VSFBJAalsf21556928 = 2261420;    int VSFBJAalsf6423779 = -23461759;    int VSFBJAalsf51796173 = -802308602;    int VSFBJAalsf57746369 = 2884125;    int VSFBJAalsf19716767 = -809622496;    int VSFBJAalsf1906898 = -289664218;    int VSFBJAalsf39093838 = -645540735;    int VSFBJAalsf20914867 = 11114402;    int VSFBJAalsf60978503 = -373887881;    int VSFBJAalsf94317633 = -4393846;    int VSFBJAalsf64840577 = -834784635;    int VSFBJAalsf63541938 = 46651759;    int VSFBJAalsf87844110 = -336602062;    int VSFBJAalsf55902241 = 99686964;    int VSFBJAalsf44350883 = -176162100;    int VSFBJAalsf2325503 = -522514534;    int VSFBJAalsf44574463 = -977488133;    int VSFBJAalsf88845590 = -774560887;    int VSFBJAalsf53775409 = 56717102;    int VSFBJAalsf6310363 = -169104289;    int VSFBJAalsf65341111 = 24957115;    int VSFBJAalsf28414294 = -871043124;    int VSFBJAalsf47554094 = -624740786;    int VSFBJAalsf11987590 = -670695781;    int VSFBJAalsf91317994 = -834489250;    int VSFBJAalsf71815104 = -978824939;    int VSFBJAalsf36409625 = -627806828;    int VSFBJAalsf64241202 = -825096398;    int VSFBJAalsf74974205 = 47453773;    int VSFBJAalsf42740104 = -227268306;    int VSFBJAalsf64713678 = -962918471;    int VSFBJAalsf2666376 = -493227640;    int VSFBJAalsf29795450 = -895621965;    int VSFBJAalsf77812517 = -898954313;    int VSFBJAalsf59554064 = -171236269;    int VSFBJAalsf44087068 = -73457859;    int VSFBJAalsf70104540 = -954706868;    int VSFBJAalsf88583547 = -102016993;    int VSFBJAalsf10423463 = -910469338;    int VSFBJAalsf12024806 = -366430497;    int VSFBJAalsf55457332 = -759711474;    int VSFBJAalsf53955362 = -152155311;    int VSFBJAalsf50291239 = -283857631;    int VSFBJAalsf83834568 = -60517849;    int VSFBJAalsf32487909 = -434689636;    int VSFBJAalsf24861163 = -9260550;    int VSFBJAalsf98807796 = -230142418;    int VSFBJAalsf14320123 = -755980062;    int VSFBJAalsf91435098 = -36098100;    int VSFBJAalsf49528008 = -965682600;    int VSFBJAalsf20566896 = -135860219;    int VSFBJAalsf10645698 = -37277410;    int VSFBJAalsf33670095 = -604135859;    int VSFBJAalsf7772315 = -134358967;    int VSFBJAalsf1895399 = -151506322;    int VSFBJAalsf93142633 = -126695456;    int VSFBJAalsf58869685 = -398720974;    int VSFBJAalsf39808584 = -31612821;    int VSFBJAalsf66428374 = -162626625;    int VSFBJAalsf47901662 = -830797558;    int VSFBJAalsf65497272 = -661857390;    int VSFBJAalsf74852636 = -820444337;    int VSFBJAalsf45940662 = 63660628;    int VSFBJAalsf18238399 = -46619575;    int VSFBJAalsf29603955 = -41475375;    int VSFBJAalsf62174201 = -241556995;    int VSFBJAalsf33746488 = -57726276;    int VSFBJAalsf10031593 = -437647749;    int VSFBJAalsf96348177 = -729076767;    int VSFBJAalsf263815 = -2704241;    int VSFBJAalsf32220962 = -567807666;    int VSFBJAalsf55990916 = -775471141;    int VSFBJAalsf78422127 = -864091550;    int VSFBJAalsf41750604 = -576852401;    int VSFBJAalsf50853031 = -409392815;    int VSFBJAalsf11385749 = -822887574;    int VSFBJAalsf78123055 = -487185494;    int VSFBJAalsf63719525 = -464222937;    int VSFBJAalsf79499680 = -136006146;    int VSFBJAalsf66456832 = -725228701;    int VSFBJAalsf73007307 = -648682522;    int VSFBJAalsf22089503 = -199132867;     VSFBJAalsf46329916 = VSFBJAalsf30140229;     VSFBJAalsf30140229 = VSFBJAalsf13931704;     VSFBJAalsf13931704 = VSFBJAalsf94551170;     VSFBJAalsf94551170 = VSFBJAalsf74870229;     VSFBJAalsf74870229 = VSFBJAalsf11269743;     VSFBJAalsf11269743 = VSFBJAalsf78152201;     VSFBJAalsf78152201 = VSFBJAalsf97328485;     VSFBJAalsf97328485 = VSFBJAalsf88403100;     VSFBJAalsf88403100 = VSFBJAalsf86651907;     VSFBJAalsf86651907 = VSFBJAalsf70222363;     VSFBJAalsf70222363 = VSFBJAalsf35785982;     VSFBJAalsf35785982 = VSFBJAalsf51853510;     VSFBJAalsf51853510 = VSFBJAalsf65141359;     VSFBJAalsf65141359 = VSFBJAalsf99491287;     VSFBJAalsf99491287 = VSFBJAalsf87445504;     VSFBJAalsf87445504 = VSFBJAalsf14082678;     VSFBJAalsf14082678 = VSFBJAalsf67236509;     VSFBJAalsf67236509 = VSFBJAalsf21556928;     VSFBJAalsf21556928 = VSFBJAalsf6423779;     VSFBJAalsf6423779 = VSFBJAalsf51796173;     VSFBJAalsf51796173 = VSFBJAalsf57746369;     VSFBJAalsf57746369 = VSFBJAalsf19716767;     VSFBJAalsf19716767 = VSFBJAalsf1906898;     VSFBJAalsf1906898 = VSFBJAalsf39093838;     VSFBJAalsf39093838 = VSFBJAalsf20914867;     VSFBJAalsf20914867 = VSFBJAalsf60978503;     VSFBJAalsf60978503 = VSFBJAalsf94317633;     VSFBJAalsf94317633 = VSFBJAalsf64840577;     VSFBJAalsf64840577 = VSFBJAalsf63541938;     VSFBJAalsf63541938 = VSFBJAalsf87844110;     VSFBJAalsf87844110 = VSFBJAalsf55902241;     VSFBJAalsf55902241 = VSFBJAalsf44350883;     VSFBJAalsf44350883 = VSFBJAalsf2325503;     VSFBJAalsf2325503 = VSFBJAalsf44574463;     VSFBJAalsf44574463 = VSFBJAalsf88845590;     VSFBJAalsf88845590 = VSFBJAalsf53775409;     VSFBJAalsf53775409 = VSFBJAalsf6310363;     VSFBJAalsf6310363 = VSFBJAalsf65341111;     VSFBJAalsf65341111 = VSFBJAalsf28414294;     VSFBJAalsf28414294 = VSFBJAalsf47554094;     VSFBJAalsf47554094 = VSFBJAalsf11987590;     VSFBJAalsf11987590 = VSFBJAalsf91317994;     VSFBJAalsf91317994 = VSFBJAalsf71815104;     VSFBJAalsf71815104 = VSFBJAalsf36409625;     VSFBJAalsf36409625 = VSFBJAalsf64241202;     VSFBJAalsf64241202 = VSFBJAalsf74974205;     VSFBJAalsf74974205 = VSFBJAalsf42740104;     VSFBJAalsf42740104 = VSFBJAalsf64713678;     VSFBJAalsf64713678 = VSFBJAalsf2666376;     VSFBJAalsf2666376 = VSFBJAalsf29795450;     VSFBJAalsf29795450 = VSFBJAalsf77812517;     VSFBJAalsf77812517 = VSFBJAalsf59554064;     VSFBJAalsf59554064 = VSFBJAalsf44087068;     VSFBJAalsf44087068 = VSFBJAalsf70104540;     VSFBJAalsf70104540 = VSFBJAalsf88583547;     VSFBJAalsf88583547 = VSFBJAalsf10423463;     VSFBJAalsf10423463 = VSFBJAalsf12024806;     VSFBJAalsf12024806 = VSFBJAalsf55457332;     VSFBJAalsf55457332 = VSFBJAalsf53955362;     VSFBJAalsf53955362 = VSFBJAalsf50291239;     VSFBJAalsf50291239 = VSFBJAalsf83834568;     VSFBJAalsf83834568 = VSFBJAalsf32487909;     VSFBJAalsf32487909 = VSFBJAalsf24861163;     VSFBJAalsf24861163 = VSFBJAalsf98807796;     VSFBJAalsf98807796 = VSFBJAalsf14320123;     VSFBJAalsf14320123 = VSFBJAalsf91435098;     VSFBJAalsf91435098 = VSFBJAalsf49528008;     VSFBJAalsf49528008 = VSFBJAalsf20566896;     VSFBJAalsf20566896 = VSFBJAalsf10645698;     VSFBJAalsf10645698 = VSFBJAalsf33670095;     VSFBJAalsf33670095 = VSFBJAalsf7772315;     VSFBJAalsf7772315 = VSFBJAalsf1895399;     VSFBJAalsf1895399 = VSFBJAalsf93142633;     VSFBJAalsf93142633 = VSFBJAalsf58869685;     VSFBJAalsf58869685 = VSFBJAalsf39808584;     VSFBJAalsf39808584 = VSFBJAalsf66428374;     VSFBJAalsf66428374 = VSFBJAalsf47901662;     VSFBJAalsf47901662 = VSFBJAalsf65497272;     VSFBJAalsf65497272 = VSFBJAalsf74852636;     VSFBJAalsf74852636 = VSFBJAalsf45940662;     VSFBJAalsf45940662 = VSFBJAalsf18238399;     VSFBJAalsf18238399 = VSFBJAalsf29603955;     VSFBJAalsf29603955 = VSFBJAalsf62174201;     VSFBJAalsf62174201 = VSFBJAalsf33746488;     VSFBJAalsf33746488 = VSFBJAalsf10031593;     VSFBJAalsf10031593 = VSFBJAalsf96348177;     VSFBJAalsf96348177 = VSFBJAalsf263815;     VSFBJAalsf263815 = VSFBJAalsf32220962;     VSFBJAalsf32220962 = VSFBJAalsf55990916;     VSFBJAalsf55990916 = VSFBJAalsf78422127;     VSFBJAalsf78422127 = VSFBJAalsf41750604;     VSFBJAalsf41750604 = VSFBJAalsf50853031;     VSFBJAalsf50853031 = VSFBJAalsf11385749;     VSFBJAalsf11385749 = VSFBJAalsf78123055;     VSFBJAalsf78123055 = VSFBJAalsf63719525;     VSFBJAalsf63719525 = VSFBJAalsf79499680;     VSFBJAalsf79499680 = VSFBJAalsf66456832;     VSFBJAalsf66456832 = VSFBJAalsf73007307;     VSFBJAalsf73007307 = VSFBJAalsf22089503;     VSFBJAalsf22089503 = VSFBJAalsf46329916;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void avFnzCWUlt22342816() {     int QylgbscBtZ81509671 = -96053380;    int QylgbscBtZ62686300 = -796373777;    int QylgbscBtZ59140570 = -346979327;    int QylgbscBtZ32418824 = -415321187;    int QylgbscBtZ57715485 = -855559053;    int QylgbscBtZ34657498 = -695844391;    int QylgbscBtZ22968363 = -828034604;    int QylgbscBtZ30467407 = -380411854;    int QylgbscBtZ98230877 = -194916851;    int QylgbscBtZ87994682 = -576766573;    int QylgbscBtZ14931000 = -908131442;    int QylgbscBtZ9347759 = -234859946;    int QylgbscBtZ57152080 = -989038636;    int QylgbscBtZ3535970 = -628319255;    int QylgbscBtZ14361009 = -483588988;    int QylgbscBtZ85561938 = -147182038;    int QylgbscBtZ63805088 = -524902973;    int QylgbscBtZ33770878 = -936430578;    int QylgbscBtZ43570919 = -524327634;    int QylgbscBtZ15520796 = -263315901;    int QylgbscBtZ53183913 = -474645801;    int QylgbscBtZ72313140 = -161748653;    int QylgbscBtZ24891989 = -457749141;    int QylgbscBtZ29263706 = -233081565;    int QylgbscBtZ52085655 = -50847930;    int QylgbscBtZ73962429 = -215005092;    int QylgbscBtZ49261878 = -464380941;    int QylgbscBtZ32178484 = -457679416;    int QylgbscBtZ75671743 = -747188679;    int QylgbscBtZ49360363 = -928098888;    int QylgbscBtZ43158233 = -307241729;    int QylgbscBtZ32038398 = -139282308;    int QylgbscBtZ85983079 = -379416163;    int QylgbscBtZ38507725 = -874713870;    int QylgbscBtZ21506322 = -291962511;    int QylgbscBtZ82934384 = -798753622;    int QylgbscBtZ72037814 = -466486822;    int QylgbscBtZ47722807 = 38912826;    int QylgbscBtZ84111031 = -605258078;    int QylgbscBtZ2991846 = -142252313;    int QylgbscBtZ49887105 = -315448547;    int QylgbscBtZ22500776 = -133123724;    int QylgbscBtZ71468296 = -186323025;    int QylgbscBtZ56655863 = -660396387;    int QylgbscBtZ383330 = -956460865;    int QylgbscBtZ5541645 = -168599262;    int QylgbscBtZ37155830 = -547190910;    int QylgbscBtZ41395164 = -630394721;    int QylgbscBtZ84579243 = -412473283;    int QylgbscBtZ26655119 = -559259476;    int QylgbscBtZ43732790 = -825027543;    int QylgbscBtZ65293064 = -318813185;    int QylgbscBtZ96905268 = -127096774;    int QylgbscBtZ38823068 = -506670559;    int QylgbscBtZ69158903 = -232956192;    int QylgbscBtZ9196532 = -934304728;    int QylgbscBtZ37794312 = -238624636;    int QylgbscBtZ29876864 = -13897762;    int QylgbscBtZ80333168 = -264473257;    int QylgbscBtZ83753056 = -540553961;    int QylgbscBtZ85395619 = -131463450;    int QylgbscBtZ90789879 = -270355189;    int QylgbscBtZ54795663 = -633223175;    int QylgbscBtZ48870515 = -266817964;    int QylgbscBtZ44836450 = -169524845;    int QylgbscBtZ82892602 = -668849135;    int QylgbscBtZ23364680 = -855443783;    int QylgbscBtZ18644355 = -14324766;    int QylgbscBtZ82029648 = -236356744;    int QylgbscBtZ31426624 = -684835367;    int QylgbscBtZ13524124 = -680695216;    int QylgbscBtZ16082281 = -463815800;    int QylgbscBtZ49659847 = -231172501;    int QylgbscBtZ40579074 = -282075322;    int QylgbscBtZ65633691 = -947867354;    int QylgbscBtZ30683137 = -241522077;    int QylgbscBtZ844845 = -975425628;    int QylgbscBtZ68236125 = -797352754;    int QylgbscBtZ28880376 = -276620700;    int QylgbscBtZ46544010 = -882248669;    int QylgbscBtZ36806600 = -667814183;    int QylgbscBtZ7866714 = -833986221;    int QylgbscBtZ47599240 = 54793867;    int QylgbscBtZ49016625 = -87929204;    int QylgbscBtZ5627573 = -3071346;    int QylgbscBtZ77865168 = -988428545;    int QylgbscBtZ35133129 = 87814466;    int QylgbscBtZ47160011 = -872745605;    int QylgbscBtZ69348822 = -541757678;    int QylgbscBtZ12309791 = -357657784;    int QylgbscBtZ45140072 = -460128986;    int QylgbscBtZ42160951 = -352589060;    int QylgbscBtZ67389638 = -696613917;    int QylgbscBtZ357976 = 35295883;    int QylgbscBtZ17596226 = 89211137;    int QylgbscBtZ59097226 = 54906642;    int QylgbscBtZ67705112 = -499900549;    int QylgbscBtZ22597781 = -919505062;    int QylgbscBtZ11819413 = -390871543;    int QylgbscBtZ17490728 = -96053380;     QylgbscBtZ81509671 = QylgbscBtZ62686300;     QylgbscBtZ62686300 = QylgbscBtZ59140570;     QylgbscBtZ59140570 = QylgbscBtZ32418824;     QylgbscBtZ32418824 = QylgbscBtZ57715485;     QylgbscBtZ57715485 = QylgbscBtZ34657498;     QylgbscBtZ34657498 = QylgbscBtZ22968363;     QylgbscBtZ22968363 = QylgbscBtZ30467407;     QylgbscBtZ30467407 = QylgbscBtZ98230877;     QylgbscBtZ98230877 = QylgbscBtZ87994682;     QylgbscBtZ87994682 = QylgbscBtZ14931000;     QylgbscBtZ14931000 = QylgbscBtZ9347759;     QylgbscBtZ9347759 = QylgbscBtZ57152080;     QylgbscBtZ57152080 = QylgbscBtZ3535970;     QylgbscBtZ3535970 = QylgbscBtZ14361009;     QylgbscBtZ14361009 = QylgbscBtZ85561938;     QylgbscBtZ85561938 = QylgbscBtZ63805088;     QylgbscBtZ63805088 = QylgbscBtZ33770878;     QylgbscBtZ33770878 = QylgbscBtZ43570919;     QylgbscBtZ43570919 = QylgbscBtZ15520796;     QylgbscBtZ15520796 = QylgbscBtZ53183913;     QylgbscBtZ53183913 = QylgbscBtZ72313140;     QylgbscBtZ72313140 = QylgbscBtZ24891989;     QylgbscBtZ24891989 = QylgbscBtZ29263706;     QylgbscBtZ29263706 = QylgbscBtZ52085655;     QylgbscBtZ52085655 = QylgbscBtZ73962429;     QylgbscBtZ73962429 = QylgbscBtZ49261878;     QylgbscBtZ49261878 = QylgbscBtZ32178484;     QylgbscBtZ32178484 = QylgbscBtZ75671743;     QylgbscBtZ75671743 = QylgbscBtZ49360363;     QylgbscBtZ49360363 = QylgbscBtZ43158233;     QylgbscBtZ43158233 = QylgbscBtZ32038398;     QylgbscBtZ32038398 = QylgbscBtZ85983079;     QylgbscBtZ85983079 = QylgbscBtZ38507725;     QylgbscBtZ38507725 = QylgbscBtZ21506322;     QylgbscBtZ21506322 = QylgbscBtZ82934384;     QylgbscBtZ82934384 = QylgbscBtZ72037814;     QylgbscBtZ72037814 = QylgbscBtZ47722807;     QylgbscBtZ47722807 = QylgbscBtZ84111031;     QylgbscBtZ84111031 = QylgbscBtZ2991846;     QylgbscBtZ2991846 = QylgbscBtZ49887105;     QylgbscBtZ49887105 = QylgbscBtZ22500776;     QylgbscBtZ22500776 = QylgbscBtZ71468296;     QylgbscBtZ71468296 = QylgbscBtZ56655863;     QylgbscBtZ56655863 = QylgbscBtZ383330;     QylgbscBtZ383330 = QylgbscBtZ5541645;     QylgbscBtZ5541645 = QylgbscBtZ37155830;     QylgbscBtZ37155830 = QylgbscBtZ41395164;     QylgbscBtZ41395164 = QylgbscBtZ84579243;     QylgbscBtZ84579243 = QylgbscBtZ26655119;     QylgbscBtZ26655119 = QylgbscBtZ43732790;     QylgbscBtZ43732790 = QylgbscBtZ65293064;     QylgbscBtZ65293064 = QylgbscBtZ96905268;     QylgbscBtZ96905268 = QylgbscBtZ38823068;     QylgbscBtZ38823068 = QylgbscBtZ69158903;     QylgbscBtZ69158903 = QylgbscBtZ9196532;     QylgbscBtZ9196532 = QylgbscBtZ37794312;     QylgbscBtZ37794312 = QylgbscBtZ29876864;     QylgbscBtZ29876864 = QylgbscBtZ80333168;     QylgbscBtZ80333168 = QylgbscBtZ83753056;     QylgbscBtZ83753056 = QylgbscBtZ85395619;     QylgbscBtZ85395619 = QylgbscBtZ90789879;     QylgbscBtZ90789879 = QylgbscBtZ54795663;     QylgbscBtZ54795663 = QylgbscBtZ48870515;     QylgbscBtZ48870515 = QylgbscBtZ44836450;     QylgbscBtZ44836450 = QylgbscBtZ82892602;     QylgbscBtZ82892602 = QylgbscBtZ23364680;     QylgbscBtZ23364680 = QylgbscBtZ18644355;     QylgbscBtZ18644355 = QylgbscBtZ82029648;     QylgbscBtZ82029648 = QylgbscBtZ31426624;     QylgbscBtZ31426624 = QylgbscBtZ13524124;     QylgbscBtZ13524124 = QylgbscBtZ16082281;     QylgbscBtZ16082281 = QylgbscBtZ49659847;     QylgbscBtZ49659847 = QylgbscBtZ40579074;     QylgbscBtZ40579074 = QylgbscBtZ65633691;     QylgbscBtZ65633691 = QylgbscBtZ30683137;     QylgbscBtZ30683137 = QylgbscBtZ844845;     QylgbscBtZ844845 = QylgbscBtZ68236125;     QylgbscBtZ68236125 = QylgbscBtZ28880376;     QylgbscBtZ28880376 = QylgbscBtZ46544010;     QylgbscBtZ46544010 = QylgbscBtZ36806600;     QylgbscBtZ36806600 = QylgbscBtZ7866714;     QylgbscBtZ7866714 = QylgbscBtZ47599240;     QylgbscBtZ47599240 = QylgbscBtZ49016625;     QylgbscBtZ49016625 = QylgbscBtZ5627573;     QylgbscBtZ5627573 = QylgbscBtZ77865168;     QylgbscBtZ77865168 = QylgbscBtZ35133129;     QylgbscBtZ35133129 = QylgbscBtZ47160011;     QylgbscBtZ47160011 = QylgbscBtZ69348822;     QylgbscBtZ69348822 = QylgbscBtZ12309791;     QylgbscBtZ12309791 = QylgbscBtZ45140072;     QylgbscBtZ45140072 = QylgbscBtZ42160951;     QylgbscBtZ42160951 = QylgbscBtZ67389638;     QylgbscBtZ67389638 = QylgbscBtZ357976;     QylgbscBtZ357976 = QylgbscBtZ17596226;     QylgbscBtZ17596226 = QylgbscBtZ59097226;     QylgbscBtZ59097226 = QylgbscBtZ67705112;     QylgbscBtZ67705112 = QylgbscBtZ22597781;     QylgbscBtZ22597781 = QylgbscBtZ11819413;     QylgbscBtZ11819413 = QylgbscBtZ17490728;     QylgbscBtZ17490728 = QylgbscBtZ81509671;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CNNHvRqLTd92798689() {     int vpyJpaQyOb94295838 = -231094072;    int vpyJpaQyOb49457189 = -966707790;    int vpyJpaQyOb76893370 = -755338708;    int vpyJpaQyOb40359649 = -239523464;    int vpyJpaQyOb51226590 = -247375518;    int vpyJpaQyOb77967568 = -472767704;    int vpyJpaQyOb44176121 = -273706675;    int vpyJpaQyOb57186519 = -824930111;    int vpyJpaQyOb93635688 = -156674943;    int vpyJpaQyOb94213387 = -380854206;    int vpyJpaQyOb42418382 = 77182543;    int vpyJpaQyOb41298264 = -742192761;    int vpyJpaQyOb27295383 = -892821293;    int vpyJpaQyOb53160049 = -960734453;    int vpyJpaQyOb37667561 = -390574819;    int vpyJpaQyOb13987007 = -698073427;    int vpyJpaQyOb19556906 = -188677440;    int vpyJpaQyOb17329364 = -491640880;    int vpyJpaQyOb44731550 = -433796829;    int vpyJpaQyOb18175292 = -862560040;    int vpyJpaQyOb33357970 = -965962065;    int vpyJpaQyOb1360960 = -331607001;    int vpyJpaQyOb63652284 = -388753036;    int vpyJpaQyOb73770836 = -593426577;    int vpyJpaQyOb57979914 = -261289855;    int vpyJpaQyOb55281075 = 94563708;    int vpyJpaQyOb60677414 = 1912779;    int vpyJpaQyOb99531408 = -631477520;    int vpyJpaQyOb92288796 = -961405920;    int vpyJpaQyOb2111479 = -557985043;    int vpyJpaQyOb87594228 = -583658479;    int vpyJpaQyOb35357222 = -118001986;    int vpyJpaQyOb32029029 = -255270947;    int vpyJpaQyOb15584098 = -779468088;    int vpyJpaQyOb4566854 = -901994392;    int vpyJpaQyOb78158499 = -587316726;    int vpyJpaQyOb8801875 = -256396172;    int vpyJpaQyOb5426831 = -980707265;    int vpyJpaQyOb89886417 = -447108655;    int vpyJpaQyOb29256576 = 13330842;    int vpyJpaQyOb11318413 = -690922540;    int vpyJpaQyOb30203232 = -31435058;    int vpyJpaQyOb6351766 = -209756470;    int vpyJpaQyOb1284401 = 98431370;    int vpyJpaQyOb80391820 = -849230825;    int vpyJpaQyOb79169774 = -241917702;    int vpyJpaQyOb14737849 = 55294038;    int vpyJpaQyOb70835130 = -785747500;    int vpyJpaQyOb28643293 = -576075257;    int vpyJpaQyOb73605539 = -70295939;    int vpyJpaQyOb62060284 = 22088149;    int vpyJpaQyOb93469611 = -789998523;    int vpyJpaQyOb31390419 = -303100056;    int vpyJpaQyOb83522793 = -96122301;    int vpyJpaQyOb59440719 = -227400679;    int vpyJpaQyOb92934879 = -899487072;    int vpyJpaQyOb85804904 = -477954755;    int vpyJpaQyOb3122535 = -61912132;    int vpyJpaQyOb82379735 = -978233609;    int vpyJpaQyOb95945515 = -241939227;    int vpyJpaQyOb17290154 = -374680483;    int vpyJpaQyOb44644712 = -642229155;    int vpyJpaQyOb64897722 = -863524192;    int vpyJpaQyOb91524210 = -598689901;    int vpyJpaQyOb6619159 = -797195728;    int vpyJpaQyOb7061161 = -804815472;    int vpyJpaQyOb9269236 = -386921814;    int vpyJpaQyOb11711285 = -13353205;    int vpyJpaQyOb48593195 = 41259938;    int vpyJpaQyOb59509062 = -803258094;    int vpyJpaQyOb5185133 = -341677255;    int vpyJpaQyOb14130076 = -207970175;    int vpyJpaQyOb27442946 = 55467774;    int vpyJpaQyOb15474975 = -347127671;    int vpyJpaQyOb6856879 = -71637500;    int vpyJpaQyOb3154739 = -834527007;    int vpyJpaQyOb95009193 = -21850531;    int vpyJpaQyOb62367883 = -387184406;    int vpyJpaQyOb93379015 = -744195753;    int vpyJpaQyOb78810139 = 80627846;    int vpyJpaQyOb40543226 = -960730330;    int vpyJpaQyOb89842283 = -212339722;    int vpyJpaQyOb70888115 = 44597737;    int vpyJpaQyOb18683257 = -791109981;    int vpyJpaQyOb40051194 = -480073192;    int vpyJpaQyOb94124617 = -793659957;    int vpyJpaQyOb3966803 = -814901930;    int vpyJpaQyOb48506235 = -59148647;    int vpyJpaQyOb56143379 = -452067410;    int vpyJpaQyOb11631975 = 97492680;    int vpyJpaQyOb92353594 = -9361971;    int vpyJpaQyOb5679341 = -94484041;    int vpyJpaQyOb23047095 = 97526344;    int vpyJpaQyOb93940902 = -105169428;    int vpyJpaQyOb11966422 = -611988676;    int vpyJpaQyOb66673701 = 51306614;    int vpyJpaQyOb65305509 = -167910867;    int vpyJpaQyOb14827556 = -611066569;    int vpyJpaQyOb94665241 = -104372903;    int vpyJpaQyOb73330659 = -231094072;     vpyJpaQyOb94295838 = vpyJpaQyOb49457189;     vpyJpaQyOb49457189 = vpyJpaQyOb76893370;     vpyJpaQyOb76893370 = vpyJpaQyOb40359649;     vpyJpaQyOb40359649 = vpyJpaQyOb51226590;     vpyJpaQyOb51226590 = vpyJpaQyOb77967568;     vpyJpaQyOb77967568 = vpyJpaQyOb44176121;     vpyJpaQyOb44176121 = vpyJpaQyOb57186519;     vpyJpaQyOb57186519 = vpyJpaQyOb93635688;     vpyJpaQyOb93635688 = vpyJpaQyOb94213387;     vpyJpaQyOb94213387 = vpyJpaQyOb42418382;     vpyJpaQyOb42418382 = vpyJpaQyOb41298264;     vpyJpaQyOb41298264 = vpyJpaQyOb27295383;     vpyJpaQyOb27295383 = vpyJpaQyOb53160049;     vpyJpaQyOb53160049 = vpyJpaQyOb37667561;     vpyJpaQyOb37667561 = vpyJpaQyOb13987007;     vpyJpaQyOb13987007 = vpyJpaQyOb19556906;     vpyJpaQyOb19556906 = vpyJpaQyOb17329364;     vpyJpaQyOb17329364 = vpyJpaQyOb44731550;     vpyJpaQyOb44731550 = vpyJpaQyOb18175292;     vpyJpaQyOb18175292 = vpyJpaQyOb33357970;     vpyJpaQyOb33357970 = vpyJpaQyOb1360960;     vpyJpaQyOb1360960 = vpyJpaQyOb63652284;     vpyJpaQyOb63652284 = vpyJpaQyOb73770836;     vpyJpaQyOb73770836 = vpyJpaQyOb57979914;     vpyJpaQyOb57979914 = vpyJpaQyOb55281075;     vpyJpaQyOb55281075 = vpyJpaQyOb60677414;     vpyJpaQyOb60677414 = vpyJpaQyOb99531408;     vpyJpaQyOb99531408 = vpyJpaQyOb92288796;     vpyJpaQyOb92288796 = vpyJpaQyOb2111479;     vpyJpaQyOb2111479 = vpyJpaQyOb87594228;     vpyJpaQyOb87594228 = vpyJpaQyOb35357222;     vpyJpaQyOb35357222 = vpyJpaQyOb32029029;     vpyJpaQyOb32029029 = vpyJpaQyOb15584098;     vpyJpaQyOb15584098 = vpyJpaQyOb4566854;     vpyJpaQyOb4566854 = vpyJpaQyOb78158499;     vpyJpaQyOb78158499 = vpyJpaQyOb8801875;     vpyJpaQyOb8801875 = vpyJpaQyOb5426831;     vpyJpaQyOb5426831 = vpyJpaQyOb89886417;     vpyJpaQyOb89886417 = vpyJpaQyOb29256576;     vpyJpaQyOb29256576 = vpyJpaQyOb11318413;     vpyJpaQyOb11318413 = vpyJpaQyOb30203232;     vpyJpaQyOb30203232 = vpyJpaQyOb6351766;     vpyJpaQyOb6351766 = vpyJpaQyOb1284401;     vpyJpaQyOb1284401 = vpyJpaQyOb80391820;     vpyJpaQyOb80391820 = vpyJpaQyOb79169774;     vpyJpaQyOb79169774 = vpyJpaQyOb14737849;     vpyJpaQyOb14737849 = vpyJpaQyOb70835130;     vpyJpaQyOb70835130 = vpyJpaQyOb28643293;     vpyJpaQyOb28643293 = vpyJpaQyOb73605539;     vpyJpaQyOb73605539 = vpyJpaQyOb62060284;     vpyJpaQyOb62060284 = vpyJpaQyOb93469611;     vpyJpaQyOb93469611 = vpyJpaQyOb31390419;     vpyJpaQyOb31390419 = vpyJpaQyOb83522793;     vpyJpaQyOb83522793 = vpyJpaQyOb59440719;     vpyJpaQyOb59440719 = vpyJpaQyOb92934879;     vpyJpaQyOb92934879 = vpyJpaQyOb85804904;     vpyJpaQyOb85804904 = vpyJpaQyOb3122535;     vpyJpaQyOb3122535 = vpyJpaQyOb82379735;     vpyJpaQyOb82379735 = vpyJpaQyOb95945515;     vpyJpaQyOb95945515 = vpyJpaQyOb17290154;     vpyJpaQyOb17290154 = vpyJpaQyOb44644712;     vpyJpaQyOb44644712 = vpyJpaQyOb64897722;     vpyJpaQyOb64897722 = vpyJpaQyOb91524210;     vpyJpaQyOb91524210 = vpyJpaQyOb6619159;     vpyJpaQyOb6619159 = vpyJpaQyOb7061161;     vpyJpaQyOb7061161 = vpyJpaQyOb9269236;     vpyJpaQyOb9269236 = vpyJpaQyOb11711285;     vpyJpaQyOb11711285 = vpyJpaQyOb48593195;     vpyJpaQyOb48593195 = vpyJpaQyOb59509062;     vpyJpaQyOb59509062 = vpyJpaQyOb5185133;     vpyJpaQyOb5185133 = vpyJpaQyOb14130076;     vpyJpaQyOb14130076 = vpyJpaQyOb27442946;     vpyJpaQyOb27442946 = vpyJpaQyOb15474975;     vpyJpaQyOb15474975 = vpyJpaQyOb6856879;     vpyJpaQyOb6856879 = vpyJpaQyOb3154739;     vpyJpaQyOb3154739 = vpyJpaQyOb95009193;     vpyJpaQyOb95009193 = vpyJpaQyOb62367883;     vpyJpaQyOb62367883 = vpyJpaQyOb93379015;     vpyJpaQyOb93379015 = vpyJpaQyOb78810139;     vpyJpaQyOb78810139 = vpyJpaQyOb40543226;     vpyJpaQyOb40543226 = vpyJpaQyOb89842283;     vpyJpaQyOb89842283 = vpyJpaQyOb70888115;     vpyJpaQyOb70888115 = vpyJpaQyOb18683257;     vpyJpaQyOb18683257 = vpyJpaQyOb40051194;     vpyJpaQyOb40051194 = vpyJpaQyOb94124617;     vpyJpaQyOb94124617 = vpyJpaQyOb3966803;     vpyJpaQyOb3966803 = vpyJpaQyOb48506235;     vpyJpaQyOb48506235 = vpyJpaQyOb56143379;     vpyJpaQyOb56143379 = vpyJpaQyOb11631975;     vpyJpaQyOb11631975 = vpyJpaQyOb92353594;     vpyJpaQyOb92353594 = vpyJpaQyOb5679341;     vpyJpaQyOb5679341 = vpyJpaQyOb23047095;     vpyJpaQyOb23047095 = vpyJpaQyOb93940902;     vpyJpaQyOb93940902 = vpyJpaQyOb11966422;     vpyJpaQyOb11966422 = vpyJpaQyOb66673701;     vpyJpaQyOb66673701 = vpyJpaQyOb65305509;     vpyJpaQyOb65305509 = vpyJpaQyOb14827556;     vpyJpaQyOb14827556 = vpyJpaQyOb94665241;     vpyJpaQyOb94665241 = vpyJpaQyOb73330659;     vpyJpaQyOb73330659 = vpyJpaQyOb94295838;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void srbeBLpHxF11012031() {     int lhCPSjjPSr77740144 = -11759753;    int lhCPSjjPSr66329278 = -966089978;    int lhCPSjjPSr73301877 = -548863936;    int lhCPSjjPSr3151901 = -433323300;    int lhCPSjjPSr57347914 = -54243009;    int lhCPSjjPSr76471969 = -251839357;    int lhCPSjjPSr15215439 = -13617988;    int lhCPSjjPSr71533962 = -812088140;    int lhCPSjjPSr52165389 = -498134294;    int lhCPSjjPSr26759040 = -849230318;    int lhCPSjjPSr91834320 = -112933330;    int lhCPSjjPSr4503663 = -729811008;    int lhCPSjjPSr45413159 = -684482534;    int lhCPSjjPSr32786941 = -749024810;    int lhCPSjjPSr98522024 = -484597801;    int lhCPSjjPSr376169 = -486152752;    int lhCPSjjPSr13308791 = 57977533;    int lhCPSjjPSr98180809 = -738945756;    int lhCPSjjPSr7171692 = -432471700;    int lhCPSjjPSr7542735 = -292146032;    int lhCPSjjPSr88998356 = -555491096;    int lhCPSjjPSr13508278 = -322758771;    int lhCPSjjPSr67477991 = -428642233;    int lhCPSjjPSr8477394 = -665251973;    int lhCPSjjPSr99555872 = -820942614;    int lhCPSjjPSr50623224 = -885946007;    int lhCPSjjPSr10132155 = -726171564;    int lhCPSjjPSr39263034 = -701432404;    int lhCPSjjPSr28369891 = -823911062;    int lhCPSjjPSr17944820 = 29165106;    int lhCPSjjPSr5689543 = -221415680;    int lhCPSjjPSr98463648 = -33197066;    int lhCPSjjPSr75567915 = -976143125;    int lhCPSjjPSr57742609 = -371070258;    int lhCPSjjPSr83293584 = 340873;    int lhCPSjjPSr90547683 = -375340632;    int lhCPSjjPSr77416053 = -135245352;    int lhCPSjjPSr46005662 = -689177785;    int lhCPSjjPSr38881496 = -632006452;    int lhCPSjjPSr78751710 = -839262681;    int lhCPSjjPSr44967337 = -262471513;    int lhCPSjjPSr80691017 = -605343380;    int lhCPSjjPSr29918836 = -760813712;    int lhCPSjjPSr93189340 = -757751401;    int lhCPSjjPSr72336317 = -577387778;    int lhCPSjjPSr18775770 = -261528625;    int lhCPSjjPSr90753279 = -77010192;    int lhCPSjjPSr90505725 = -784469219;    int lhCPSjjPSr59490405 = -61677340;    int lhCPSjjPSr8938024 = -277316394;    int lhCPSjjPSr35317732 = -198813273;    int lhCPSjjPSr48970406 = -848257834;    int lhCPSjjPSr45502878 = -514847373;    int lhCPSjjPSr8199553 = -914535688;    int lhCPSjjPSr22013674 = -575886802;    int lhCPSjjPSr64231867 = -689000982;    int lhCPSjjPSr98851286 = -437447745;    int lhCPSjjPSr64824483 = -883611964;    int lhCPSjjPSr3596029 = -612380686;    int lhCPSjjPSr6724690 = -168297002;    int lhCPSjjPSr66339814 = -525667794;    int lhCPSjjPSr75952404 = -312185584;    int lhCPSjjPSr43164071 = -988177079;    int lhCPSjjPSr34220570 = -427299401;    int lhCPSjjPSr21069497 = -527814639;    int lhCPSjjPSr93370671 = 20263736;    int lhCPSjjPSr28935748 = -753667883;    int lhCPSjjPSr87670550 = -213412276;    int lhCPSjjPSr49493356 = -649365684;    int lhCPSjjPSr7974342 = -9257170;    int lhCPSjjPSr22960116 = -250907400;    int lhCPSjjPSr67303128 = -252844682;    int lhCPSjjPSr59299314 = -6939305;    int lhCPSjjPSr28419981 = -593209020;    int lhCPSjjPSr62575397 = 70325480;    int lhCPSjjPSr8307340 = -950147717;    int lhCPSjjPSr83589442 = -561945059;    int lhCPSjjPSr74288650 = -670890833;    int lhCPSjjPSr36141077 = 12135805;    int lhCPSjjPSr80780102 = -459413989;    int lhCPSjjPSr59869945 = -708935816;    int lhCPSjjPSr19626429 = -941702346;    int lhCPSjjPSr79772628 = -539755065;    int lhCPSjjPSr19431868 = -446594668;    int lhCPSjjPSr82627087 = -772021621;    int lhCPSjjPSr56719136 = -373157846;    int lhCPSjjPSr52960771 = -518349694;    int lhCPSjjPSr67368363 = 38392563;    int lhCPSjjPSr35728935 = -795183457;    int lhCPSjjPSr19061718 = -310658145;    int lhCPSjjPSr91696397 = -937892887;    int lhCPSjjPSr12591571 = -251633389;    int lhCPSjjPSr42409634 = 23202901;    int lhCPSjjPSr32156806 = -363709451;    int lhCPSjjPSr12411896 = -213594887;    int lhCPSjjPSr69014933 = -950285929;    int lhCPSjjPSr37526947 = -617166301;    int lhCPSjjPSr95698266 = -233514312;    int lhCPSjjPSr72119843 = -129936762;    int lhCPSjjPSr78965645 = -11759753;     lhCPSjjPSr77740144 = lhCPSjjPSr66329278;     lhCPSjjPSr66329278 = lhCPSjjPSr73301877;     lhCPSjjPSr73301877 = lhCPSjjPSr3151901;     lhCPSjjPSr3151901 = lhCPSjjPSr57347914;     lhCPSjjPSr57347914 = lhCPSjjPSr76471969;     lhCPSjjPSr76471969 = lhCPSjjPSr15215439;     lhCPSjjPSr15215439 = lhCPSjjPSr71533962;     lhCPSjjPSr71533962 = lhCPSjjPSr52165389;     lhCPSjjPSr52165389 = lhCPSjjPSr26759040;     lhCPSjjPSr26759040 = lhCPSjjPSr91834320;     lhCPSjjPSr91834320 = lhCPSjjPSr4503663;     lhCPSjjPSr4503663 = lhCPSjjPSr45413159;     lhCPSjjPSr45413159 = lhCPSjjPSr32786941;     lhCPSjjPSr32786941 = lhCPSjjPSr98522024;     lhCPSjjPSr98522024 = lhCPSjjPSr376169;     lhCPSjjPSr376169 = lhCPSjjPSr13308791;     lhCPSjjPSr13308791 = lhCPSjjPSr98180809;     lhCPSjjPSr98180809 = lhCPSjjPSr7171692;     lhCPSjjPSr7171692 = lhCPSjjPSr7542735;     lhCPSjjPSr7542735 = lhCPSjjPSr88998356;     lhCPSjjPSr88998356 = lhCPSjjPSr13508278;     lhCPSjjPSr13508278 = lhCPSjjPSr67477991;     lhCPSjjPSr67477991 = lhCPSjjPSr8477394;     lhCPSjjPSr8477394 = lhCPSjjPSr99555872;     lhCPSjjPSr99555872 = lhCPSjjPSr50623224;     lhCPSjjPSr50623224 = lhCPSjjPSr10132155;     lhCPSjjPSr10132155 = lhCPSjjPSr39263034;     lhCPSjjPSr39263034 = lhCPSjjPSr28369891;     lhCPSjjPSr28369891 = lhCPSjjPSr17944820;     lhCPSjjPSr17944820 = lhCPSjjPSr5689543;     lhCPSjjPSr5689543 = lhCPSjjPSr98463648;     lhCPSjjPSr98463648 = lhCPSjjPSr75567915;     lhCPSjjPSr75567915 = lhCPSjjPSr57742609;     lhCPSjjPSr57742609 = lhCPSjjPSr83293584;     lhCPSjjPSr83293584 = lhCPSjjPSr90547683;     lhCPSjjPSr90547683 = lhCPSjjPSr77416053;     lhCPSjjPSr77416053 = lhCPSjjPSr46005662;     lhCPSjjPSr46005662 = lhCPSjjPSr38881496;     lhCPSjjPSr38881496 = lhCPSjjPSr78751710;     lhCPSjjPSr78751710 = lhCPSjjPSr44967337;     lhCPSjjPSr44967337 = lhCPSjjPSr80691017;     lhCPSjjPSr80691017 = lhCPSjjPSr29918836;     lhCPSjjPSr29918836 = lhCPSjjPSr93189340;     lhCPSjjPSr93189340 = lhCPSjjPSr72336317;     lhCPSjjPSr72336317 = lhCPSjjPSr18775770;     lhCPSjjPSr18775770 = lhCPSjjPSr90753279;     lhCPSjjPSr90753279 = lhCPSjjPSr90505725;     lhCPSjjPSr90505725 = lhCPSjjPSr59490405;     lhCPSjjPSr59490405 = lhCPSjjPSr8938024;     lhCPSjjPSr8938024 = lhCPSjjPSr35317732;     lhCPSjjPSr35317732 = lhCPSjjPSr48970406;     lhCPSjjPSr48970406 = lhCPSjjPSr45502878;     lhCPSjjPSr45502878 = lhCPSjjPSr8199553;     lhCPSjjPSr8199553 = lhCPSjjPSr22013674;     lhCPSjjPSr22013674 = lhCPSjjPSr64231867;     lhCPSjjPSr64231867 = lhCPSjjPSr98851286;     lhCPSjjPSr98851286 = lhCPSjjPSr64824483;     lhCPSjjPSr64824483 = lhCPSjjPSr3596029;     lhCPSjjPSr3596029 = lhCPSjjPSr6724690;     lhCPSjjPSr6724690 = lhCPSjjPSr66339814;     lhCPSjjPSr66339814 = lhCPSjjPSr75952404;     lhCPSjjPSr75952404 = lhCPSjjPSr43164071;     lhCPSjjPSr43164071 = lhCPSjjPSr34220570;     lhCPSjjPSr34220570 = lhCPSjjPSr21069497;     lhCPSjjPSr21069497 = lhCPSjjPSr93370671;     lhCPSjjPSr93370671 = lhCPSjjPSr28935748;     lhCPSjjPSr28935748 = lhCPSjjPSr87670550;     lhCPSjjPSr87670550 = lhCPSjjPSr49493356;     lhCPSjjPSr49493356 = lhCPSjjPSr7974342;     lhCPSjjPSr7974342 = lhCPSjjPSr22960116;     lhCPSjjPSr22960116 = lhCPSjjPSr67303128;     lhCPSjjPSr67303128 = lhCPSjjPSr59299314;     lhCPSjjPSr59299314 = lhCPSjjPSr28419981;     lhCPSjjPSr28419981 = lhCPSjjPSr62575397;     lhCPSjjPSr62575397 = lhCPSjjPSr8307340;     lhCPSjjPSr8307340 = lhCPSjjPSr83589442;     lhCPSjjPSr83589442 = lhCPSjjPSr74288650;     lhCPSjjPSr74288650 = lhCPSjjPSr36141077;     lhCPSjjPSr36141077 = lhCPSjjPSr80780102;     lhCPSjjPSr80780102 = lhCPSjjPSr59869945;     lhCPSjjPSr59869945 = lhCPSjjPSr19626429;     lhCPSjjPSr19626429 = lhCPSjjPSr79772628;     lhCPSjjPSr79772628 = lhCPSjjPSr19431868;     lhCPSjjPSr19431868 = lhCPSjjPSr82627087;     lhCPSjjPSr82627087 = lhCPSjjPSr56719136;     lhCPSjjPSr56719136 = lhCPSjjPSr52960771;     lhCPSjjPSr52960771 = lhCPSjjPSr67368363;     lhCPSjjPSr67368363 = lhCPSjjPSr35728935;     lhCPSjjPSr35728935 = lhCPSjjPSr19061718;     lhCPSjjPSr19061718 = lhCPSjjPSr91696397;     lhCPSjjPSr91696397 = lhCPSjjPSr12591571;     lhCPSjjPSr12591571 = lhCPSjjPSr42409634;     lhCPSjjPSr42409634 = lhCPSjjPSr32156806;     lhCPSjjPSr32156806 = lhCPSjjPSr12411896;     lhCPSjjPSr12411896 = lhCPSjjPSr69014933;     lhCPSjjPSr69014933 = lhCPSjjPSr37526947;     lhCPSjjPSr37526947 = lhCPSjjPSr95698266;     lhCPSjjPSr95698266 = lhCPSjjPSr72119843;     lhCPSjjPSr72119843 = lhCPSjjPSr78965645;     lhCPSjjPSr78965645 = lhCPSjjPSr77740144;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OLeTRoggdo81467903() {     int WkGGfOqVpV90526311 = -146800445;    int WkGGfOqVpV53100166 = -36423992;    int WkGGfOqVpV91054676 = -957223318;    int WkGGfOqVpV11092726 = -257525576;    int WkGGfOqVpV50859018 = -546059474;    int WkGGfOqVpV19782040 = -28762671;    int WkGGfOqVpV36423197 = -559290058;    int WkGGfOqVpV98253074 = -156606397;    int WkGGfOqVpV47570199 = -459892387;    int WkGGfOqVpV32977745 = -653317951;    int WkGGfOqVpV19321703 = -227619345;    int WkGGfOqVpV36454168 = -137143824;    int WkGGfOqVpV15556462 = -588265191;    int WkGGfOqVpV82411019 = 18559991;    int WkGGfOqVpV21828578 = -391583632;    int WkGGfOqVpV28801238 = 62955859;    int WkGGfOqVpV69060608 = -705796934;    int WkGGfOqVpV81739294 = -294156059;    int WkGGfOqVpV8332323 = -341940895;    int WkGGfOqVpV10197231 = -891390171;    int WkGGfOqVpV69172413 = 53192640;    int WkGGfOqVpV42556097 = -492617119;    int WkGGfOqVpV6238287 = -359646128;    int WkGGfOqVpV52984523 = 74403015;    int WkGGfOqVpV5450132 = 68615462;    int WkGGfOqVpV31941869 = -576377206;    int WkGGfOqVpV21547691 = -259877844;    int WkGGfOqVpV6615959 = -875230509;    int WkGGfOqVpV44986944 = 61871698;    int WkGGfOqVpV70695935 = -700721049;    int WkGGfOqVpV50125539 = -497832430;    int WkGGfOqVpV1782473 = -11916744;    int WkGGfOqVpV21613865 = -851997909;    int WkGGfOqVpV34818982 = -275824477;    int WkGGfOqVpV66354116 = -609691007;    int WkGGfOqVpV85771798 = -163903736;    int WkGGfOqVpV14180114 = 74845297;    int WkGGfOqVpV3709685 = -608797877;    int WkGGfOqVpV44656882 = -473857030;    int WkGGfOqVpV5016441 = -683679526;    int WkGGfOqVpV6398646 = -637945506;    int WkGGfOqVpV88393473 = -503654714;    int WkGGfOqVpV64802305 = -784247157;    int WkGGfOqVpV37817879 = 1076356;    int WkGGfOqVpV52344807 = -470157739;    int WkGGfOqVpV92403899 = -334847065;    int WkGGfOqVpV68335299 = -574525244;    int WkGGfOqVpV19945693 = -939821998;    int WkGGfOqVpV3554455 = -225279315;    int WkGGfOqVpV55888444 = -888352857;    int WkGGfOqVpV53645225 = -451697582;    int WkGGfOqVpV77146953 = -219443172;    int WkGGfOqVpV79988027 = -690850655;    int WkGGfOqVpV52899277 = -503987430;    int WkGGfOqVpV12295490 = -570331288;    int WkGGfOqVpV47970215 = -654183326;    int WkGGfOqVpV46861879 = -676777864;    int WkGGfOqVpV38070153 = -931626333;    int WkGGfOqVpV5642595 = -226141039;    int WkGGfOqVpV18917150 = -969682268;    int WkGGfOqVpV98234349 = -768884827;    int WkGGfOqVpV29807238 = -684059550;    int WkGGfOqVpV53266131 = -118478096;    int WkGGfOqVpV76874264 = -759171338;    int WkGGfOqVpV82852206 = -55485522;    int WkGGfOqVpV17539230 = -115702601;    int WkGGfOqVpV14840304 = -285145915;    int WkGGfOqVpV80737480 = -212440715;    int WkGGfOqVpV16056903 = -371749002;    int WkGGfOqVpV36056780 = -127679897;    int WkGGfOqVpV14621124 = 88110561;    int WkGGfOqVpV65350923 = 3000943;    int WkGGfOqVpV37082413 = -820299030;    int WkGGfOqVpV3315882 = -658261369;    int WkGGfOqVpV3798586 = -153444665;    int WkGGfOqVpV80778940 = -443152647;    int WkGGfOqVpV77753791 = -708369962;    int WkGGfOqVpV68420408 = -260722485;    int WkGGfOqVpV639717 = -455439247;    int WkGGfOqVpV13046232 = -596537473;    int WkGGfOqVpV63606570 = 98148037;    int WkGGfOqVpV1601999 = -320055846;    int WkGGfOqVpV3061505 = -549951195;    int WkGGfOqVpV89098499 = -49775446;    int WkGGfOqVpV17050710 = -149023467;    int WkGGfOqVpV72978585 = -178389258;    int WkGGfOqVpV21794445 = -321066089;    int WkGGfOqVpV68714587 = -248010480;    int WkGGfOqVpV22523492 = -705493189;    int WkGGfOqVpV18383902 = -955507682;    int WkGGfOqVpV38909919 = -487125872;    int WkGGfOqVpV76109960 = 6471630;    int WkGGfOqVpV98067090 = -282656838;    int WkGGfOqVpV25739732 = -504174762;    int WkGGfOqVpV6782092 = -914794700;    int WkGGfOqVpV76591408 = -953885957;    int WkGGfOqVpV35127343 = -285176619;    int WkGGfOqVpV87928041 = 74924180;    int WkGGfOqVpV54965673 = -943438122;    int WkGGfOqVpV34805578 = -146800445;     WkGGfOqVpV90526311 = WkGGfOqVpV53100166;     WkGGfOqVpV53100166 = WkGGfOqVpV91054676;     WkGGfOqVpV91054676 = WkGGfOqVpV11092726;     WkGGfOqVpV11092726 = WkGGfOqVpV50859018;     WkGGfOqVpV50859018 = WkGGfOqVpV19782040;     WkGGfOqVpV19782040 = WkGGfOqVpV36423197;     WkGGfOqVpV36423197 = WkGGfOqVpV98253074;     WkGGfOqVpV98253074 = WkGGfOqVpV47570199;     WkGGfOqVpV47570199 = WkGGfOqVpV32977745;     WkGGfOqVpV32977745 = WkGGfOqVpV19321703;     WkGGfOqVpV19321703 = WkGGfOqVpV36454168;     WkGGfOqVpV36454168 = WkGGfOqVpV15556462;     WkGGfOqVpV15556462 = WkGGfOqVpV82411019;     WkGGfOqVpV82411019 = WkGGfOqVpV21828578;     WkGGfOqVpV21828578 = WkGGfOqVpV28801238;     WkGGfOqVpV28801238 = WkGGfOqVpV69060608;     WkGGfOqVpV69060608 = WkGGfOqVpV81739294;     WkGGfOqVpV81739294 = WkGGfOqVpV8332323;     WkGGfOqVpV8332323 = WkGGfOqVpV10197231;     WkGGfOqVpV10197231 = WkGGfOqVpV69172413;     WkGGfOqVpV69172413 = WkGGfOqVpV42556097;     WkGGfOqVpV42556097 = WkGGfOqVpV6238287;     WkGGfOqVpV6238287 = WkGGfOqVpV52984523;     WkGGfOqVpV52984523 = WkGGfOqVpV5450132;     WkGGfOqVpV5450132 = WkGGfOqVpV31941869;     WkGGfOqVpV31941869 = WkGGfOqVpV21547691;     WkGGfOqVpV21547691 = WkGGfOqVpV6615959;     WkGGfOqVpV6615959 = WkGGfOqVpV44986944;     WkGGfOqVpV44986944 = WkGGfOqVpV70695935;     WkGGfOqVpV70695935 = WkGGfOqVpV50125539;     WkGGfOqVpV50125539 = WkGGfOqVpV1782473;     WkGGfOqVpV1782473 = WkGGfOqVpV21613865;     WkGGfOqVpV21613865 = WkGGfOqVpV34818982;     WkGGfOqVpV34818982 = WkGGfOqVpV66354116;     WkGGfOqVpV66354116 = WkGGfOqVpV85771798;     WkGGfOqVpV85771798 = WkGGfOqVpV14180114;     WkGGfOqVpV14180114 = WkGGfOqVpV3709685;     WkGGfOqVpV3709685 = WkGGfOqVpV44656882;     WkGGfOqVpV44656882 = WkGGfOqVpV5016441;     WkGGfOqVpV5016441 = WkGGfOqVpV6398646;     WkGGfOqVpV6398646 = WkGGfOqVpV88393473;     WkGGfOqVpV88393473 = WkGGfOqVpV64802305;     WkGGfOqVpV64802305 = WkGGfOqVpV37817879;     WkGGfOqVpV37817879 = WkGGfOqVpV52344807;     WkGGfOqVpV52344807 = WkGGfOqVpV92403899;     WkGGfOqVpV92403899 = WkGGfOqVpV68335299;     WkGGfOqVpV68335299 = WkGGfOqVpV19945693;     WkGGfOqVpV19945693 = WkGGfOqVpV3554455;     WkGGfOqVpV3554455 = WkGGfOqVpV55888444;     WkGGfOqVpV55888444 = WkGGfOqVpV53645225;     WkGGfOqVpV53645225 = WkGGfOqVpV77146953;     WkGGfOqVpV77146953 = WkGGfOqVpV79988027;     WkGGfOqVpV79988027 = WkGGfOqVpV52899277;     WkGGfOqVpV52899277 = WkGGfOqVpV12295490;     WkGGfOqVpV12295490 = WkGGfOqVpV47970215;     WkGGfOqVpV47970215 = WkGGfOqVpV46861879;     WkGGfOqVpV46861879 = WkGGfOqVpV38070153;     WkGGfOqVpV38070153 = WkGGfOqVpV5642595;     WkGGfOqVpV5642595 = WkGGfOqVpV18917150;     WkGGfOqVpV18917150 = WkGGfOqVpV98234349;     WkGGfOqVpV98234349 = WkGGfOqVpV29807238;     WkGGfOqVpV29807238 = WkGGfOqVpV53266131;     WkGGfOqVpV53266131 = WkGGfOqVpV76874264;     WkGGfOqVpV76874264 = WkGGfOqVpV82852206;     WkGGfOqVpV82852206 = WkGGfOqVpV17539230;     WkGGfOqVpV17539230 = WkGGfOqVpV14840304;     WkGGfOqVpV14840304 = WkGGfOqVpV80737480;     WkGGfOqVpV80737480 = WkGGfOqVpV16056903;     WkGGfOqVpV16056903 = WkGGfOqVpV36056780;     WkGGfOqVpV36056780 = WkGGfOqVpV14621124;     WkGGfOqVpV14621124 = WkGGfOqVpV65350923;     WkGGfOqVpV65350923 = WkGGfOqVpV37082413;     WkGGfOqVpV37082413 = WkGGfOqVpV3315882;     WkGGfOqVpV3315882 = WkGGfOqVpV3798586;     WkGGfOqVpV3798586 = WkGGfOqVpV80778940;     WkGGfOqVpV80778940 = WkGGfOqVpV77753791;     WkGGfOqVpV77753791 = WkGGfOqVpV68420408;     WkGGfOqVpV68420408 = WkGGfOqVpV639717;     WkGGfOqVpV639717 = WkGGfOqVpV13046232;     WkGGfOqVpV13046232 = WkGGfOqVpV63606570;     WkGGfOqVpV63606570 = WkGGfOqVpV1601999;     WkGGfOqVpV1601999 = WkGGfOqVpV3061505;     WkGGfOqVpV3061505 = WkGGfOqVpV89098499;     WkGGfOqVpV89098499 = WkGGfOqVpV17050710;     WkGGfOqVpV17050710 = WkGGfOqVpV72978585;     WkGGfOqVpV72978585 = WkGGfOqVpV21794445;     WkGGfOqVpV21794445 = WkGGfOqVpV68714587;     WkGGfOqVpV68714587 = WkGGfOqVpV22523492;     WkGGfOqVpV22523492 = WkGGfOqVpV18383902;     WkGGfOqVpV18383902 = WkGGfOqVpV38909919;     WkGGfOqVpV38909919 = WkGGfOqVpV76109960;     WkGGfOqVpV76109960 = WkGGfOqVpV98067090;     WkGGfOqVpV98067090 = WkGGfOqVpV25739732;     WkGGfOqVpV25739732 = WkGGfOqVpV6782092;     WkGGfOqVpV6782092 = WkGGfOqVpV76591408;     WkGGfOqVpV76591408 = WkGGfOqVpV35127343;     WkGGfOqVpV35127343 = WkGGfOqVpV87928041;     WkGGfOqVpV87928041 = WkGGfOqVpV54965673;     WkGGfOqVpV54965673 = WkGGfOqVpV34805578;     WkGGfOqVpV34805578 = WkGGfOqVpV90526311;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QoFzipydmD99681245() {     int HiYckkdRtt73970617 = 72533875;    int HiYckkdRtt69972255 = -35806179;    int HiYckkdRtt87463183 = -750748546;    int HiYckkdRtt73884977 = -451325412;    int HiYckkdRtt56980342 = -352926965;    int HiYckkdRtt18286441 = -907834324;    int HiYckkdRtt7462515 = -299201372;    int HiYckkdRtt12600518 = -143764427;    int HiYckkdRtt6099901 = -801351737;    int HiYckkdRtt65523396 = -21694063;    int HiYckkdRtt68737640 = -417735218;    int HiYckkdRtt99659567 = -124762070;    int HiYckkdRtt33674237 = -379926432;    int HiYckkdRtt62037911 = -869730365;    int HiYckkdRtt82683041 = -485606614;    int HiYckkdRtt15190399 = -825123467;    int HiYckkdRtt62812493 = -459141961;    int HiYckkdRtt62590741 = -541460935;    int HiYckkdRtt70772464 = -340615766;    int HiYckkdRtt99564673 = -320976164;    int HiYckkdRtt24812801 = -636336391;    int HiYckkdRtt54703415 = -483768889;    int HiYckkdRtt10063995 = -399535325;    int HiYckkdRtt87691081 = 2577619;    int HiYckkdRtt47026089 = -491037297;    int HiYckkdRtt27284018 = -456886922;    int HiYckkdRtt71002431 = -987962187;    int HiYckkdRtt46347584 = -945185393;    int HiYckkdRtt81068038 = -900633444;    int HiYckkdRtt86529276 = -113570900;    int HiYckkdRtt68220852 = -135589631;    int HiYckkdRtt64888899 = 72888176;    int HiYckkdRtt65152751 = -472870087;    int HiYckkdRtt76977492 = -967426647;    int HiYckkdRtt45080847 = -807355743;    int HiYckkdRtt98160982 = 48072358;    int HiYckkdRtt82794292 = -904003882;    int HiYckkdRtt44288517 = -317268397;    int HiYckkdRtt93651959 = -658754827;    int HiYckkdRtt54511575 = -436273049;    int HiYckkdRtt40047570 = -209494480;    int HiYckkdRtt38881260 = 22436964;    int HiYckkdRtt88369375 = -235304400;    int HiYckkdRtt29722819 = -855106415;    int HiYckkdRtt44289304 = -198314692;    int HiYckkdRtt32009896 = -354457989;    int HiYckkdRtt44350729 = -706829473;    int HiYckkdRtt39616288 = -938543717;    int HiYckkdRtt34401567 = -810881397;    int HiYckkdRtt91220928 = 4626688;    int HiYckkdRtt26902674 = -672599004;    int HiYckkdRtt32647748 = -277702484;    int HiYckkdRtt94100486 = -902597972;    int HiYckkdRtt77576036 = -222400817;    int HiYckkdRtt74868444 = -918817411;    int HiYckkdRtt19267202 = -443697237;    int HiYckkdRtt59908260 = -636270855;    int HiYckkdRtt99772101 = -653326165;    int HiYckkdRtt26858888 = -960288116;    int HiYckkdRtt29696324 = -896040044;    int HiYckkdRtt47284010 = -919872138;    int HiYckkdRtt61114930 = -354015980;    int HiYckkdRtt31532480 = -243130983;    int HiYckkdRtt19570624 = -587780838;    int HiYckkdRtt97302544 = -886104433;    int HiYckkdRtt3848741 = -390623394;    int HiYckkdRtt34506816 = -651891984;    int HiYckkdRtt56696745 = -412499786;    int HiYckkdRtt16957065 = 37625377;    int HiYckkdRtt84522058 = -433678973;    int HiYckkdRtt32396107 = -921119585;    int HiYckkdRtt18523976 = -41873564;    int HiYckkdRtt68938781 = -882706108;    int HiYckkdRtt16260889 = -904342718;    int HiYckkdRtt59517103 = -11481685;    int HiYckkdRtt85931541 = -558773356;    int HiYckkdRtt66334040 = -148464490;    int HiYckkdRtt80341175 = -544428911;    int HiYckkdRtt43401778 = -799107690;    int HiYckkdRtt15016194 = -36579309;    int HiYckkdRtt82933289 = -750057449;    int HiYckkdRtt31386143 = 50581530;    int HiYckkdRtt11946018 = -34303996;    int HiYckkdRtt89847109 = -805260133;    int HiYckkdRtt59626602 = -440971897;    int HiYckkdRtt35573105 = -857887148;    int HiYckkdRtt70788412 = -24513853;    int HiYckkdRtt87576714 = -150469270;    int HiYckkdRtt2109048 = 51390764;    int HiYckkdRtt25813645 = -263658506;    int HiYckkdRtt38252722 = -315656788;    int HiYckkdRtt83022190 = -150677718;    int HiYckkdRtt17429629 = -356980282;    int HiYckkdRtt63955635 = -762714784;    int HiYckkdRtt7227566 = -516400912;    int HiYckkdRtt78932639 = -855478501;    int HiYckkdRtt7348781 = -734432053;    int HiYckkdRtt68798751 = -647523563;    int HiYckkdRtt32420275 = -969001982;    int HiYckkdRtt40440564 = 72533875;     HiYckkdRtt73970617 = HiYckkdRtt69972255;     HiYckkdRtt69972255 = HiYckkdRtt87463183;     HiYckkdRtt87463183 = HiYckkdRtt73884977;     HiYckkdRtt73884977 = HiYckkdRtt56980342;     HiYckkdRtt56980342 = HiYckkdRtt18286441;     HiYckkdRtt18286441 = HiYckkdRtt7462515;     HiYckkdRtt7462515 = HiYckkdRtt12600518;     HiYckkdRtt12600518 = HiYckkdRtt6099901;     HiYckkdRtt6099901 = HiYckkdRtt65523396;     HiYckkdRtt65523396 = HiYckkdRtt68737640;     HiYckkdRtt68737640 = HiYckkdRtt99659567;     HiYckkdRtt99659567 = HiYckkdRtt33674237;     HiYckkdRtt33674237 = HiYckkdRtt62037911;     HiYckkdRtt62037911 = HiYckkdRtt82683041;     HiYckkdRtt82683041 = HiYckkdRtt15190399;     HiYckkdRtt15190399 = HiYckkdRtt62812493;     HiYckkdRtt62812493 = HiYckkdRtt62590741;     HiYckkdRtt62590741 = HiYckkdRtt70772464;     HiYckkdRtt70772464 = HiYckkdRtt99564673;     HiYckkdRtt99564673 = HiYckkdRtt24812801;     HiYckkdRtt24812801 = HiYckkdRtt54703415;     HiYckkdRtt54703415 = HiYckkdRtt10063995;     HiYckkdRtt10063995 = HiYckkdRtt87691081;     HiYckkdRtt87691081 = HiYckkdRtt47026089;     HiYckkdRtt47026089 = HiYckkdRtt27284018;     HiYckkdRtt27284018 = HiYckkdRtt71002431;     HiYckkdRtt71002431 = HiYckkdRtt46347584;     HiYckkdRtt46347584 = HiYckkdRtt81068038;     HiYckkdRtt81068038 = HiYckkdRtt86529276;     HiYckkdRtt86529276 = HiYckkdRtt68220852;     HiYckkdRtt68220852 = HiYckkdRtt64888899;     HiYckkdRtt64888899 = HiYckkdRtt65152751;     HiYckkdRtt65152751 = HiYckkdRtt76977492;     HiYckkdRtt76977492 = HiYckkdRtt45080847;     HiYckkdRtt45080847 = HiYckkdRtt98160982;     HiYckkdRtt98160982 = HiYckkdRtt82794292;     HiYckkdRtt82794292 = HiYckkdRtt44288517;     HiYckkdRtt44288517 = HiYckkdRtt93651959;     HiYckkdRtt93651959 = HiYckkdRtt54511575;     HiYckkdRtt54511575 = HiYckkdRtt40047570;     HiYckkdRtt40047570 = HiYckkdRtt38881260;     HiYckkdRtt38881260 = HiYckkdRtt88369375;     HiYckkdRtt88369375 = HiYckkdRtt29722819;     HiYckkdRtt29722819 = HiYckkdRtt44289304;     HiYckkdRtt44289304 = HiYckkdRtt32009896;     HiYckkdRtt32009896 = HiYckkdRtt44350729;     HiYckkdRtt44350729 = HiYckkdRtt39616288;     HiYckkdRtt39616288 = HiYckkdRtt34401567;     HiYckkdRtt34401567 = HiYckkdRtt91220928;     HiYckkdRtt91220928 = HiYckkdRtt26902674;     HiYckkdRtt26902674 = HiYckkdRtt32647748;     HiYckkdRtt32647748 = HiYckkdRtt94100486;     HiYckkdRtt94100486 = HiYckkdRtt77576036;     HiYckkdRtt77576036 = HiYckkdRtt74868444;     HiYckkdRtt74868444 = HiYckkdRtt19267202;     HiYckkdRtt19267202 = HiYckkdRtt59908260;     HiYckkdRtt59908260 = HiYckkdRtt99772101;     HiYckkdRtt99772101 = HiYckkdRtt26858888;     HiYckkdRtt26858888 = HiYckkdRtt29696324;     HiYckkdRtt29696324 = HiYckkdRtt47284010;     HiYckkdRtt47284010 = HiYckkdRtt61114930;     HiYckkdRtt61114930 = HiYckkdRtt31532480;     HiYckkdRtt31532480 = HiYckkdRtt19570624;     HiYckkdRtt19570624 = HiYckkdRtt97302544;     HiYckkdRtt97302544 = HiYckkdRtt3848741;     HiYckkdRtt3848741 = HiYckkdRtt34506816;     HiYckkdRtt34506816 = HiYckkdRtt56696745;     HiYckkdRtt56696745 = HiYckkdRtt16957065;     HiYckkdRtt16957065 = HiYckkdRtt84522058;     HiYckkdRtt84522058 = HiYckkdRtt32396107;     HiYckkdRtt32396107 = HiYckkdRtt18523976;     HiYckkdRtt18523976 = HiYckkdRtt68938781;     HiYckkdRtt68938781 = HiYckkdRtt16260889;     HiYckkdRtt16260889 = HiYckkdRtt59517103;     HiYckkdRtt59517103 = HiYckkdRtt85931541;     HiYckkdRtt85931541 = HiYckkdRtt66334040;     HiYckkdRtt66334040 = HiYckkdRtt80341175;     HiYckkdRtt80341175 = HiYckkdRtt43401778;     HiYckkdRtt43401778 = HiYckkdRtt15016194;     HiYckkdRtt15016194 = HiYckkdRtt82933289;     HiYckkdRtt82933289 = HiYckkdRtt31386143;     HiYckkdRtt31386143 = HiYckkdRtt11946018;     HiYckkdRtt11946018 = HiYckkdRtt89847109;     HiYckkdRtt89847109 = HiYckkdRtt59626602;     HiYckkdRtt59626602 = HiYckkdRtt35573105;     HiYckkdRtt35573105 = HiYckkdRtt70788412;     HiYckkdRtt70788412 = HiYckkdRtt87576714;     HiYckkdRtt87576714 = HiYckkdRtt2109048;     HiYckkdRtt2109048 = HiYckkdRtt25813645;     HiYckkdRtt25813645 = HiYckkdRtt38252722;     HiYckkdRtt38252722 = HiYckkdRtt83022190;     HiYckkdRtt83022190 = HiYckkdRtt17429629;     HiYckkdRtt17429629 = HiYckkdRtt63955635;     HiYckkdRtt63955635 = HiYckkdRtt7227566;     HiYckkdRtt7227566 = HiYckkdRtt78932639;     HiYckkdRtt78932639 = HiYckkdRtt7348781;     HiYckkdRtt7348781 = HiYckkdRtt68798751;     HiYckkdRtt68798751 = HiYckkdRtt32420275;     HiYckkdRtt32420275 = HiYckkdRtt40440564;     HiYckkdRtt40440564 = HiYckkdRtt73970617;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ISBtaBljie70137118() {     int nYsYnBhhIM86756784 = -62506817;    int nYsYnBhhIM56743143 = -206140193;    int nYsYnBhhIM5215984 = -59107927;    int nYsYnBhhIM81825803 = -275527689;    int nYsYnBhhIM50491447 = -844743430;    int nYsYnBhhIM61596511 = -684757637;    int nYsYnBhhIM28670273 = -844873442;    int nYsYnBhhIM39319630 = -588282683;    int nYsYnBhhIM1504711 = -763109830;    int nYsYnBhhIM71742101 = -925781696;    int nYsYnBhhIM96225022 = -532421233;    int nYsYnBhhIM31610072 = -632094886;    int nYsYnBhhIM3817541 = -283709089;    int nYsYnBhhIM11661990 = -102145564;    int nYsYnBhhIM5989595 = -392592445;    int nYsYnBhhIM43615468 = -276014856;    int nYsYnBhhIM18564311 = -122916428;    int nYsYnBhhIM46149226 = -96671237;    int nYsYnBhhIM71933095 = -250084961;    int nYsYnBhhIM2219170 = -920220303;    int nYsYnBhhIM4986858 = -27652655;    int nYsYnBhhIM83751234 = -653627237;    int nYsYnBhhIM48824290 = -330539220;    int nYsYnBhhIM32198211 = -357767394;    int nYsYnBhhIM52920349 = -701479221;    int nYsYnBhhIM8602664 = -147318121;    int nYsYnBhhIM82417967 = -521668467;    int nYsYnBhhIM13700510 = -18983497;    int nYsYnBhhIM97685091 = -14850684;    int nYsYnBhhIM39280392 = -843457055;    int nYsYnBhhIM12656849 = -412006381;    int nYsYnBhhIM68207723 = 94168497;    int nYsYnBhhIM11198701 = -348724871;    int nYsYnBhhIM54053865 = -872180865;    int nYsYnBhhIM28141379 = -317387623;    int nYsYnBhhIM93385097 = -840490746;    int nYsYnBhhIM19558353 = -693913233;    int nYsYnBhhIM1992540 = -236888489;    int nYsYnBhhIM99427345 = -500605404;    int nYsYnBhhIM80776305 = -280689894;    int nYsYnBhhIM1478878 = -584968473;    int nYsYnBhhIM46583716 = -975874370;    int nYsYnBhhIM23252845 = -258737845;    int nYsYnBhhIM74351356 = -96278657;    int nYsYnBhhIM24297795 = -91084652;    int nYsYnBhhIM5638025 = -427776428;    int nYsYnBhhIM21932749 = -104344526;    int nYsYnBhhIM69056254 = 6103504;    int nYsYnBhhIM78465616 = -974483372;    int nYsYnBhhIM38171349 = -606409775;    int nYsYnBhhIM45230167 = -925483312;    int nYsYnBhhIM60824295 = -748887822;    int nYsYnBhhIM28585637 = 21398745;    int nYsYnBhhIM22275762 = -911852559;    int nYsYnBhhIM65150260 = -913261898;    int nYsYnBhhIM3005550 = -408879580;    int nYsYnBhhIM7918853 = -875600974;    int nYsYnBhhIM73017772 = -701340534;    int nYsYnBhhIM28905455 = -574048468;    int nYsYnBhhIM41888784 = -597425310;    int nYsYnBhhIM79178544 = -63089171;    int nYsYnBhhIM14969764 = -725889945;    int nYsYnBhhIM41634539 = -473432000;    int nYsYnBhhIM62224319 = -919652775;    int nYsYnBhhIM59085253 = -413775316;    int nYsYnBhhIM28017300 = -526589731;    int nYsYnBhhIM20411372 = -183370015;    int nYsYnBhhIM49763675 = -411528225;    int nYsYnBhhIM83520611 = -784757941;    int nYsYnBhhIM12604497 = -552101700;    int nYsYnBhhIM24057115 = -582101623;    int nYsYnBhhIM16571771 = -886027940;    int nYsYnBhhIM46721881 = -596065833;    int nYsYnBhhIM91156789 = -969395068;    int nYsYnBhhIM740292 = -235251830;    int nYsYnBhhIM58403142 = -51778286;    int nYsYnBhhIM60498389 = -294889393;    int nYsYnBhhIM74472933 = -134260563;    int nYsYnBhhIM7900417 = -166682742;    int nYsYnBhhIM47282324 = -173702793;    int nYsYnBhhIM86669914 = 57026404;    int nYsYnBhhIM13361713 = -427771971;    int nYsYnBhhIM35234893 = -44500126;    int nYsYnBhhIM59513742 = -408440910;    int nYsYnBhhIM94050224 = -917973743;    int nYsYnBhhIM51832553 = -663118560;    int nYsYnBhhIM39622086 = -927230249;    int nYsYnBhhIM88922938 = -436872312;    int nYsYnBhhIM88903604 = -958918968;    int nYsYnBhhIM25135829 = -908508043;    int nYsYnBhhIM85466244 = -964889773;    int nYsYnBhhIM46540581 = -992572699;    int nYsYnBhhIM73087085 = -662840021;    int nYsYnBhhIM57538562 = -903180095;    int nYsYnBhhIM1597761 = -117600724;    int nYsYnBhhIM86509114 = -859078528;    int nYsYnBhhIM4949177 = -402442371;    int nYsYnBhhIM61028526 = -339085070;    int nYsYnBhhIM15266104 = -682503342;    int nYsYnBhhIM96280495 = -62506817;     nYsYnBhhIM86756784 = nYsYnBhhIM56743143;     nYsYnBhhIM56743143 = nYsYnBhhIM5215984;     nYsYnBhhIM5215984 = nYsYnBhhIM81825803;     nYsYnBhhIM81825803 = nYsYnBhhIM50491447;     nYsYnBhhIM50491447 = nYsYnBhhIM61596511;     nYsYnBhhIM61596511 = nYsYnBhhIM28670273;     nYsYnBhhIM28670273 = nYsYnBhhIM39319630;     nYsYnBhhIM39319630 = nYsYnBhhIM1504711;     nYsYnBhhIM1504711 = nYsYnBhhIM71742101;     nYsYnBhhIM71742101 = nYsYnBhhIM96225022;     nYsYnBhhIM96225022 = nYsYnBhhIM31610072;     nYsYnBhhIM31610072 = nYsYnBhhIM3817541;     nYsYnBhhIM3817541 = nYsYnBhhIM11661990;     nYsYnBhhIM11661990 = nYsYnBhhIM5989595;     nYsYnBhhIM5989595 = nYsYnBhhIM43615468;     nYsYnBhhIM43615468 = nYsYnBhhIM18564311;     nYsYnBhhIM18564311 = nYsYnBhhIM46149226;     nYsYnBhhIM46149226 = nYsYnBhhIM71933095;     nYsYnBhhIM71933095 = nYsYnBhhIM2219170;     nYsYnBhhIM2219170 = nYsYnBhhIM4986858;     nYsYnBhhIM4986858 = nYsYnBhhIM83751234;     nYsYnBhhIM83751234 = nYsYnBhhIM48824290;     nYsYnBhhIM48824290 = nYsYnBhhIM32198211;     nYsYnBhhIM32198211 = nYsYnBhhIM52920349;     nYsYnBhhIM52920349 = nYsYnBhhIM8602664;     nYsYnBhhIM8602664 = nYsYnBhhIM82417967;     nYsYnBhhIM82417967 = nYsYnBhhIM13700510;     nYsYnBhhIM13700510 = nYsYnBhhIM97685091;     nYsYnBhhIM97685091 = nYsYnBhhIM39280392;     nYsYnBhhIM39280392 = nYsYnBhhIM12656849;     nYsYnBhhIM12656849 = nYsYnBhhIM68207723;     nYsYnBhhIM68207723 = nYsYnBhhIM11198701;     nYsYnBhhIM11198701 = nYsYnBhhIM54053865;     nYsYnBhhIM54053865 = nYsYnBhhIM28141379;     nYsYnBhhIM28141379 = nYsYnBhhIM93385097;     nYsYnBhhIM93385097 = nYsYnBhhIM19558353;     nYsYnBhhIM19558353 = nYsYnBhhIM1992540;     nYsYnBhhIM1992540 = nYsYnBhhIM99427345;     nYsYnBhhIM99427345 = nYsYnBhhIM80776305;     nYsYnBhhIM80776305 = nYsYnBhhIM1478878;     nYsYnBhhIM1478878 = nYsYnBhhIM46583716;     nYsYnBhhIM46583716 = nYsYnBhhIM23252845;     nYsYnBhhIM23252845 = nYsYnBhhIM74351356;     nYsYnBhhIM74351356 = nYsYnBhhIM24297795;     nYsYnBhhIM24297795 = nYsYnBhhIM5638025;     nYsYnBhhIM5638025 = nYsYnBhhIM21932749;     nYsYnBhhIM21932749 = nYsYnBhhIM69056254;     nYsYnBhhIM69056254 = nYsYnBhhIM78465616;     nYsYnBhhIM78465616 = nYsYnBhhIM38171349;     nYsYnBhhIM38171349 = nYsYnBhhIM45230167;     nYsYnBhhIM45230167 = nYsYnBhhIM60824295;     nYsYnBhhIM60824295 = nYsYnBhhIM28585637;     nYsYnBhhIM28585637 = nYsYnBhhIM22275762;     nYsYnBhhIM22275762 = nYsYnBhhIM65150260;     nYsYnBhhIM65150260 = nYsYnBhhIM3005550;     nYsYnBhhIM3005550 = nYsYnBhhIM7918853;     nYsYnBhhIM7918853 = nYsYnBhhIM73017772;     nYsYnBhhIM73017772 = nYsYnBhhIM28905455;     nYsYnBhhIM28905455 = nYsYnBhhIM41888784;     nYsYnBhhIM41888784 = nYsYnBhhIM79178544;     nYsYnBhhIM79178544 = nYsYnBhhIM14969764;     nYsYnBhhIM14969764 = nYsYnBhhIM41634539;     nYsYnBhhIM41634539 = nYsYnBhhIM62224319;     nYsYnBhhIM62224319 = nYsYnBhhIM59085253;     nYsYnBhhIM59085253 = nYsYnBhhIM28017300;     nYsYnBhhIM28017300 = nYsYnBhhIM20411372;     nYsYnBhhIM20411372 = nYsYnBhhIM49763675;     nYsYnBhhIM49763675 = nYsYnBhhIM83520611;     nYsYnBhhIM83520611 = nYsYnBhhIM12604497;     nYsYnBhhIM12604497 = nYsYnBhhIM24057115;     nYsYnBhhIM24057115 = nYsYnBhhIM16571771;     nYsYnBhhIM16571771 = nYsYnBhhIM46721881;     nYsYnBhhIM46721881 = nYsYnBhhIM91156789;     nYsYnBhhIM91156789 = nYsYnBhhIM740292;     nYsYnBhhIM740292 = nYsYnBhhIM58403142;     nYsYnBhhIM58403142 = nYsYnBhhIM60498389;     nYsYnBhhIM60498389 = nYsYnBhhIM74472933;     nYsYnBhhIM74472933 = nYsYnBhhIM7900417;     nYsYnBhhIM7900417 = nYsYnBhhIM47282324;     nYsYnBhhIM47282324 = nYsYnBhhIM86669914;     nYsYnBhhIM86669914 = nYsYnBhhIM13361713;     nYsYnBhhIM13361713 = nYsYnBhhIM35234893;     nYsYnBhhIM35234893 = nYsYnBhhIM59513742;     nYsYnBhhIM59513742 = nYsYnBhhIM94050224;     nYsYnBhhIM94050224 = nYsYnBhhIM51832553;     nYsYnBhhIM51832553 = nYsYnBhhIM39622086;     nYsYnBhhIM39622086 = nYsYnBhhIM88922938;     nYsYnBhhIM88922938 = nYsYnBhhIM88903604;     nYsYnBhhIM88903604 = nYsYnBhhIM25135829;     nYsYnBhhIM25135829 = nYsYnBhhIM85466244;     nYsYnBhhIM85466244 = nYsYnBhhIM46540581;     nYsYnBhhIM46540581 = nYsYnBhhIM73087085;     nYsYnBhhIM73087085 = nYsYnBhhIM57538562;     nYsYnBhhIM57538562 = nYsYnBhhIM1597761;     nYsYnBhhIM1597761 = nYsYnBhhIM86509114;     nYsYnBhhIM86509114 = nYsYnBhhIM4949177;     nYsYnBhhIM4949177 = nYsYnBhhIM61028526;     nYsYnBhhIM61028526 = nYsYnBhhIM15266104;     nYsYnBhhIM15266104 = nYsYnBhhIM96280495;     nYsYnBhhIM96280495 = nYsYnBhhIM86756784;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JGRnOYeGtk88350459() {     int jjeEeFKNEe70201089 = -943172498;    int jjeEeFKNEe73615232 = -205522380;    int jjeEeFKNEe1624491 = -952633155;    int jjeEeFKNEe44618054 = -469327525;    int jjeEeFKNEe56612771 = -651610921;    int jjeEeFKNEe60100912 = -463829290;    int jjeEeFKNEe99709590 = -584784756;    int jjeEeFKNEe53667073 = -575440713;    int jjeEeFKNEe60034411 = -4569181;    int jjeEeFKNEe4287754 = -294157808;    int jjeEeFKNEe45640960 = -722537106;    int jjeEeFKNEe94815471 = -619713132;    int jjeEeFKNEe21935316 = -75370330;    int jjeEeFKNEe91288881 = -990435921;    int jjeEeFKNEe66844057 = -486615427;    int jjeEeFKNEe30004630 = -64094181;    int jjeEeFKNEe12316196 = -976261455;    int jjeEeFKNEe27000673 = -343976114;    int jjeEeFKNEe34373237 = -248759832;    int jjeEeFKNEe91586612 = -349806295;    int jjeEeFKNEe60627245 = -717181686;    int jjeEeFKNEe95898552 = -644779007;    int jjeEeFKNEe52649998 = -370428417;    int jjeEeFKNEe66904769 = -429592790;    int jjeEeFKNEe94496306 = -161131980;    int jjeEeFKNEe3944813 = -27827837;    int jjeEeFKNEe31872707 = -149752809;    int jjeEeFKNEe53432135 = -88938381;    int jjeEeFKNEe33766185 = -977355826;    int jjeEeFKNEe55113733 = -256306906;    int jjeEeFKNEe30752162 = -49763582;    int jjeEeFKNEe31314150 = -921026583;    int jjeEeFKNEe54737587 = 30402952;    int jjeEeFKNEe96212375 = -463783035;    int jjeEeFKNEe6868110 = -515052358;    int jjeEeFKNEe5774282 = -628514652;    int jjeEeFKNEe88172531 = -572762413;    int jjeEeFKNEe42571372 = 54640991;    int jjeEeFKNEe48422424 = -685503202;    int jjeEeFKNEe30271440 = -33283417;    int jjeEeFKNEe35127802 = -156517446;    int jjeEeFKNEe97071501 = -449782691;    int jjeEeFKNEe46819915 = -809795088;    int jjeEeFKNEe66256297 = -952461428;    int jjeEeFKNEe16242292 = -919241605;    int jjeEeFKNEe45244021 = -447387352;    int jjeEeFKNEe97948178 = -236648755;    int jjeEeFKNEe88726849 = 7381785;    int jjeEeFKNEe9312729 = -460085454;    int jjeEeFKNEe73503832 = -813430230;    int jjeEeFKNEe18487616 = -46384735;    int jjeEeFKNEe16325090 = -807147133;    int jjeEeFKNEe42698096 = -190348571;    int jjeEeFKNEe46952521 = -630265947;    int jjeEeFKNEe27723215 = -161748021;    int jjeEeFKNEe74302537 = -198393491;    int jjeEeFKNEe20965235 = -835093964;    int jjeEeFKNEe34719721 = -423040366;    int jjeEeFKNEe50121748 = -208195545;    int jjeEeFKNEe52667958 = -523783085;    int jjeEeFKNEe28228205 = -214076482;    int jjeEeFKNEe46277456 = -395846375;    int jjeEeFKNEe19900888 = -598084887;    int jjeEeFKNEe4920679 = -748262275;    int jjeEeFKNEe73535591 = -144394227;    int jjeEeFKNEe14326810 = -801510524;    int jjeEeFKNEe40077884 = -550116084;    int jjeEeFKNEe25722941 = -611587296;    int jjeEeFKNEe84420772 = -375383563;    int jjeEeFKNEe61069776 = -858100776;    int jjeEeFKNEe41832098 = -491331769;    int jjeEeFKNEe69744823 = -930902446;    int jjeEeFKNEe78578249 = -658472912;    int jjeEeFKNEe4101797 = -115476416;    int jjeEeFKNEe56458810 = -93288850;    int jjeEeFKNEe63555743 = -167398996;    int jjeEeFKNEe49078638 = -834983920;    int jjeEeFKNEe86393701 = -417966989;    int jjeEeFKNEe50662478 = -510351185;    int jjeEeFKNEe49252286 = -713744629;    int jjeEeFKNEe5996634 = -791179082;    int jjeEeFKNEe43145858 = -57134595;    int jjeEeFKNEe44119406 = -628852927;    int jjeEeFKNEe60262352 = -63925597;    int jjeEeFKNEe36626118 = -109922172;    int jjeEeFKNEe14427073 = -242616449;    int jjeEeFKNEe88616054 = -630678012;    int jjeEeFKNEe7785067 = -339331102;    int jjeEeFKNEe68489160 = -202035015;    int jjeEeFKNEe32565572 = -216658868;    int jjeEeFKNEe84809047 = -793420688;    int jjeEeFKNEe53452810 = -49722047;    int jjeEeFKNEe92449624 = -737163464;    int jjeEeFKNEe95754465 = -61720118;    int jjeEeFKNEe2043236 = -819206936;    int jjeEeFKNEe88850346 = -760671072;    int jjeEeFKNEe77170614 = -851697805;    int jjeEeFKNEe41899236 = 38467186;    int jjeEeFKNEe92720705 = -708067202;    int jjeEeFKNEe1915482 = -943172498;     jjeEeFKNEe70201089 = jjeEeFKNEe73615232;     jjeEeFKNEe73615232 = jjeEeFKNEe1624491;     jjeEeFKNEe1624491 = jjeEeFKNEe44618054;     jjeEeFKNEe44618054 = jjeEeFKNEe56612771;     jjeEeFKNEe56612771 = jjeEeFKNEe60100912;     jjeEeFKNEe60100912 = jjeEeFKNEe99709590;     jjeEeFKNEe99709590 = jjeEeFKNEe53667073;     jjeEeFKNEe53667073 = jjeEeFKNEe60034411;     jjeEeFKNEe60034411 = jjeEeFKNEe4287754;     jjeEeFKNEe4287754 = jjeEeFKNEe45640960;     jjeEeFKNEe45640960 = jjeEeFKNEe94815471;     jjeEeFKNEe94815471 = jjeEeFKNEe21935316;     jjeEeFKNEe21935316 = jjeEeFKNEe91288881;     jjeEeFKNEe91288881 = jjeEeFKNEe66844057;     jjeEeFKNEe66844057 = jjeEeFKNEe30004630;     jjeEeFKNEe30004630 = jjeEeFKNEe12316196;     jjeEeFKNEe12316196 = jjeEeFKNEe27000673;     jjeEeFKNEe27000673 = jjeEeFKNEe34373237;     jjeEeFKNEe34373237 = jjeEeFKNEe91586612;     jjeEeFKNEe91586612 = jjeEeFKNEe60627245;     jjeEeFKNEe60627245 = jjeEeFKNEe95898552;     jjeEeFKNEe95898552 = jjeEeFKNEe52649998;     jjeEeFKNEe52649998 = jjeEeFKNEe66904769;     jjeEeFKNEe66904769 = jjeEeFKNEe94496306;     jjeEeFKNEe94496306 = jjeEeFKNEe3944813;     jjeEeFKNEe3944813 = jjeEeFKNEe31872707;     jjeEeFKNEe31872707 = jjeEeFKNEe53432135;     jjeEeFKNEe53432135 = jjeEeFKNEe33766185;     jjeEeFKNEe33766185 = jjeEeFKNEe55113733;     jjeEeFKNEe55113733 = jjeEeFKNEe30752162;     jjeEeFKNEe30752162 = jjeEeFKNEe31314150;     jjeEeFKNEe31314150 = jjeEeFKNEe54737587;     jjeEeFKNEe54737587 = jjeEeFKNEe96212375;     jjeEeFKNEe96212375 = jjeEeFKNEe6868110;     jjeEeFKNEe6868110 = jjeEeFKNEe5774282;     jjeEeFKNEe5774282 = jjeEeFKNEe88172531;     jjeEeFKNEe88172531 = jjeEeFKNEe42571372;     jjeEeFKNEe42571372 = jjeEeFKNEe48422424;     jjeEeFKNEe48422424 = jjeEeFKNEe30271440;     jjeEeFKNEe30271440 = jjeEeFKNEe35127802;     jjeEeFKNEe35127802 = jjeEeFKNEe97071501;     jjeEeFKNEe97071501 = jjeEeFKNEe46819915;     jjeEeFKNEe46819915 = jjeEeFKNEe66256297;     jjeEeFKNEe66256297 = jjeEeFKNEe16242292;     jjeEeFKNEe16242292 = jjeEeFKNEe45244021;     jjeEeFKNEe45244021 = jjeEeFKNEe97948178;     jjeEeFKNEe97948178 = jjeEeFKNEe88726849;     jjeEeFKNEe88726849 = jjeEeFKNEe9312729;     jjeEeFKNEe9312729 = jjeEeFKNEe73503832;     jjeEeFKNEe73503832 = jjeEeFKNEe18487616;     jjeEeFKNEe18487616 = jjeEeFKNEe16325090;     jjeEeFKNEe16325090 = jjeEeFKNEe42698096;     jjeEeFKNEe42698096 = jjeEeFKNEe46952521;     jjeEeFKNEe46952521 = jjeEeFKNEe27723215;     jjeEeFKNEe27723215 = jjeEeFKNEe74302537;     jjeEeFKNEe74302537 = jjeEeFKNEe20965235;     jjeEeFKNEe20965235 = jjeEeFKNEe34719721;     jjeEeFKNEe34719721 = jjeEeFKNEe50121748;     jjeEeFKNEe50121748 = jjeEeFKNEe52667958;     jjeEeFKNEe52667958 = jjeEeFKNEe28228205;     jjeEeFKNEe28228205 = jjeEeFKNEe46277456;     jjeEeFKNEe46277456 = jjeEeFKNEe19900888;     jjeEeFKNEe19900888 = jjeEeFKNEe4920679;     jjeEeFKNEe4920679 = jjeEeFKNEe73535591;     jjeEeFKNEe73535591 = jjeEeFKNEe14326810;     jjeEeFKNEe14326810 = jjeEeFKNEe40077884;     jjeEeFKNEe40077884 = jjeEeFKNEe25722941;     jjeEeFKNEe25722941 = jjeEeFKNEe84420772;     jjeEeFKNEe84420772 = jjeEeFKNEe61069776;     jjeEeFKNEe61069776 = jjeEeFKNEe41832098;     jjeEeFKNEe41832098 = jjeEeFKNEe69744823;     jjeEeFKNEe69744823 = jjeEeFKNEe78578249;     jjeEeFKNEe78578249 = jjeEeFKNEe4101797;     jjeEeFKNEe4101797 = jjeEeFKNEe56458810;     jjeEeFKNEe56458810 = jjeEeFKNEe63555743;     jjeEeFKNEe63555743 = jjeEeFKNEe49078638;     jjeEeFKNEe49078638 = jjeEeFKNEe86393701;     jjeEeFKNEe86393701 = jjeEeFKNEe50662478;     jjeEeFKNEe50662478 = jjeEeFKNEe49252286;     jjeEeFKNEe49252286 = jjeEeFKNEe5996634;     jjeEeFKNEe5996634 = jjeEeFKNEe43145858;     jjeEeFKNEe43145858 = jjeEeFKNEe44119406;     jjeEeFKNEe44119406 = jjeEeFKNEe60262352;     jjeEeFKNEe60262352 = jjeEeFKNEe36626118;     jjeEeFKNEe36626118 = jjeEeFKNEe14427073;     jjeEeFKNEe14427073 = jjeEeFKNEe88616054;     jjeEeFKNEe88616054 = jjeEeFKNEe7785067;     jjeEeFKNEe7785067 = jjeEeFKNEe68489160;     jjeEeFKNEe68489160 = jjeEeFKNEe32565572;     jjeEeFKNEe32565572 = jjeEeFKNEe84809047;     jjeEeFKNEe84809047 = jjeEeFKNEe53452810;     jjeEeFKNEe53452810 = jjeEeFKNEe92449624;     jjeEeFKNEe92449624 = jjeEeFKNEe95754465;     jjeEeFKNEe95754465 = jjeEeFKNEe2043236;     jjeEeFKNEe2043236 = jjeEeFKNEe88850346;     jjeEeFKNEe88850346 = jjeEeFKNEe77170614;     jjeEeFKNEe77170614 = jjeEeFKNEe41899236;     jjeEeFKNEe41899236 = jjeEeFKNEe92720705;     jjeEeFKNEe92720705 = jjeEeFKNEe1915482;     jjeEeFKNEe1915482 = jjeEeFKNEe70201089;}
// Junk Finished
