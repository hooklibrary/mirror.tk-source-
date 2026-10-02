#include "GUI.h"
#include "Menu.h"
#include "RenderManager.h"
#include <algorithm>
#include "tinyxml2.h"
#include <sstream>
#include "Controls.h"
float MenuAlpha = 0.05f;
float MenuAlpha3 = 0.05f;
float MenuAlpha5 = 0.05f;
float MenuAlpha7 = 0.05f;
float MenuAlpha9 = 0.05f;
float MenuAlpha11 = 0.05f;
float MenuAlpha13 = 0.05f;
float MenuAlpha15 = 0.05f;
float MenuAlpha17 = 0.05f;
float MenuAlpha19 = 0.05f;
float MenuAlpha21 = 0.05f;
float MenuAlpha23 = 0.05f;
float Globals::MenuAlpha24 = MenuAlpha23;
float Globals::MenuAlpha22 = MenuAlpha21;
float Globals::MenuAlpha20 = MenuAlpha19;
float Globals::MenuAlpha18 = MenuAlpha17;
float Globals::MenuAlpha16 = MenuAlpha15;
float Globals::MenuAlpha14 = MenuAlpha13;
float Globals::MenuAlpha12 = MenuAlpha11;
float Globals::MenuAlpha10 = MenuAlpha9;
float Globals::MenuAlpha8 = MenuAlpha7;
float Globals::MenuAlpha6 = MenuAlpha5;
float Globals::MenuAlpha2 = MenuAlpha;
float Globals::MenuAlpha4 = MenuAlpha3;
CGUI GUI;
bool SaveFile = false;
bool LoadFile = false;
CGUI::CGUI()
{
}
#define UI_CURSORSIZE       12
#define UI_CURSORFILL       Color(255,255,255)
#define UI_CURSOROUTLINE    Color(20,20,20,255)
#define UI_WIN_TOPHEIGHT	26
#define UI_WIN_TITLEHEIGHT	0
#define UI_TAB_WIDTH		150
#define UI_TAB_HEIGHT		32
#define UI_WIN_CLOSE_X		20
#define UI_WIN_CLOSE_Y      6
#define UI_CHK_SIZE			16
#define UI_COL_MAIN			Color(27, 220, 117, 255)
#define UI_COL_SHADOW		Color(0, 0, 0, 255)
#define COL_WHITE           Color(255, 100, 50, 255)
#define UI_COL_MAINE        Color(0, 204, 0, 255)
#define UI_COL_MAINDARK     Color(113, 236, 159, 255)
#define UI_COURSOUR			Color(255, 255, 255, 255)
#define UI_COL_FADEMAIN     Color(27, 206, 94, 255)
#define UI_COL_SHADOW		Color(0, 0, 0, 255)
#define UI_COL_CLIENTBACK   Color(238, 0, 50, 255)
#define UI_COL_TABSEPERATOR Color(229, 229, 229, 255)
#define UI_COL_TABTEXT      Color(255, 255, 255, 255)
#define UI_COL_GROUPOUTLINE Color(222, 100, 150, 255)
void CGUI::Draw()
{
	bool ShouldDrawCursor = false;
	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
			MenuAlpha = min(MenuAlpha + 15, 255);
		else
			MenuAlpha = max(MenuAlpha - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha5 = min(MenuAlpha5 + 15, 120);
		else
			MenuAlpha5 = max(MenuAlpha5 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha3 = min(MenuAlpha3 + 15, 15);
		else
			MenuAlpha3 = max(MenuAlpha3 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha7 = min(MenuAlpha7 + 15, 200);
		else
			MenuAlpha7 = max(MenuAlpha7 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha9 = min(MenuAlpha9 + 15, 245);
		else
			MenuAlpha9 = max(MenuAlpha9 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha11 = min(MenuAlpha11 + 15, 80);
		else
			MenuAlpha11 = max(MenuAlpha11 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha13 = min(MenuAlpha13 + 15, 140);
		else
			MenuAlpha13 = max(MenuAlpha13 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha15 = min(MenuAlpha15 + 15, 40);
		else
			MenuAlpha15 = max(MenuAlpha15 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha17 = min(MenuAlpha17 + 15, 50);
		else
			MenuAlpha17 = max(MenuAlpha17 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha21 = min(MenuAlpha21 + 15, 1);
		else
			MenuAlpha21 = max(MenuAlpha21 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha19 = min(MenuAlpha19 + 15, 100);
		else
			MenuAlpha19 = max(MenuAlpha19 - 15, 0);
		if (window->m_bIsOpen)
			MenuAlpha23 = min(MenuAlpha23 + 15, 255);
		else
			MenuAlpha23 = max(MenuAlpha23 - 15, 0);
		if (window->m_bIsOpen)
		{
			ShouldDrawCursor = true;
			DrawWindow(window);
		}
	}

}

int getfps()
{
	return static_cast<int>(1.f / interfaces::globals->frametime);
}

std::string GetTimeString()
{
	time_t current_time;
	struct tm *time_info;
	static char timeString[10];
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%X", time_info);
	return timeString;
}


void CGUI::Update()
{
	//Key Array
	std::copy(keys, keys + 255, oldKeys);
	for (int x = 0; x < 255; x++)
	{
		//oldKeys[x] = oldKeys[x] & keys[x];
		keys[x] = (GetAsyncKeyState(x));
	}
	// Mouse Location
	POINT mp; GetCursorPos(&mp);
	Mouse.x = mp.x; Mouse.y = mp.y;
	RECT Screen = Render::GetViewport();
	// Window Binds
	for (auto& bind : WindowBinds)
	{
		if (GetKeyPress(bind.first))
		{
			bind.second->Toggle();
		}
	}
	// Stop dragging
	if (IsDraggingWindow && !GetKeyState(VK_LBUTTON))
	{
		IsDraggingWindow = false;
		DraggingWindow = nullptr;
	}
	// If we are in the proccess of dragging a window
	if (IsDraggingWindow && GetKeyState(VK_LBUTTON) && !GetKeyPress(VK_LBUTTON))
	{
		if (DraggingWindow)
		{
			DraggingWindow->m_x = Mouse.x - DragOffsetX;
			DraggingWindow->m_y = Mouse.y - DragOffsetY;
		}
	}
	int w, h;
	int centerW, centerh;
	interfaces::engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	// Process some windows
	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
		{
			// Used to tell the widget processing that there could be a click
			bool bCheckWidgetClicks = false;

			float owo = options::menu.ColorsTab.owo_slider.GetValue() / 500;
			//--

			Render::Clear(0, 0, w * 4, h * 4, Color(10, 10, 10, (MenuAlpha / (1.25 + owo))));
			// If the user clicks inside the window
			if (GetKeyPress(VK_LBUTTON))
			{
				if (IsMouseInRegion(window->m_x, window->m_y, window->m_x + window->m_iWidth, window->m_y + window->m_iHeight))
				{
					// Is it inside the client area?
					if (IsMouseInRegion(window->GetClientArea()))
					{
						// User is selecting a new tab
						if (IsMouseInRegion(window->GetTabArea()))
						{
							// Loose focus on the control
							window->IsFocusingControl = false;
							window->FocusedControl = nullptr;
							int iTab = 0;
							int TabCount = window->Tabs.size();
							if (TabCount) // If there are some tabs
							{
								int TabSize = (window->m_iWidth - 4 - 12) / TabCount;
								int Dist = Mouse.x - (window->m_x + 8);
								while (Dist > TabSize)
								{
									if (Dist > TabSize)
									{
										iTab++;
										Dist -= TabSize;
									}
								}
								window->SelectedTab = window->Tabs[iTab];
							}
						}
						else
							bCheckWidgetClicks = true;
					}
					else
					{
						// Must be in the around the title or side of the window
						// So we assume the user is trying to drag the window
						IsDraggingWindow = true;
						DraggingWindow = window;
						DragOffsetX = Mouse.x - window->m_x;
						DragOffsetY = Mouse.y - window->m_y;
						// Loose focus on the control
						window->IsFocusingControl = false;
						window->FocusedControl = nullptr;
					}
				}
				else
				{
					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}
			}
			// Controls 
			if (window->SelectedTab != nullptr)
			{
				// Focused widget
				bool SkipWidget = false;
				CControl* SkipMe = nullptr;
				// this window is focusing on a widget??
				if (window->IsFocusingControl)
				{
					if (window->FocusedControl != nullptr)
					{
						CControl* control = window->FocusedControl;
						CGroupBox* group;
						if (control->FileControlType != UIControlTypes::UIC_GroupBox) group = control->parent_group ? (CGroupBox*)control->parent_group : nullptr;
						if (group != nullptr && control->FileControlType != UIControlTypes::UIC_GroupBox)
						{
							if ((group->group_tabs.size() > 0 && control->g_tab == group->selected_tab) || group->group_tabs.size() == 0)
							{
								// We've processed it once, skip it later
								SkipWidget = true;
								SkipMe = window->FocusedControl;
								POINT cAbs = window->FocusedControl->GetAbsolutePos();
								RECT controlRect = { cAbs.x, cAbs.y, window->FocusedControl->m_iWidth, window->FocusedControl->m_iHeight };
								window->FocusedControl->OnUpdate();
								if (window->FocusedControl->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
								{
									window->FocusedControl->OnClick();
									bCheckWidgetClicks = false;
								}
							}
						}
						else if (control->FileControlType == UIControlTypes::UIC_GroupBox || control->FileControlType != UIControlTypes::UIC_GroupBox && !control->parent_group)
						{
							// We've processed it once, skip it later
							SkipWidget = true;
							SkipMe = window->FocusedControl;
							POINT cAbs = window->FocusedControl->GetAbsolutePos();
							RECT controlRect = { cAbs.x, cAbs.y, window->FocusedControl->m_iWidth, window->FocusedControl->m_iHeight };
							window->FocusedControl->OnUpdate();
							if (window->FocusedControl->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
							{
								window->FocusedControl->OnClick();
								// If it gets clicked we loose focus
								window->IsFocusingControl = false;
								window->FocusedControl = nullptr;
								bCheckWidgetClicks = false;
							}
						}
					}
				}
				// Itterate over the rest of the control
				for (auto control : window->SelectedTab->Controls)
				{
					if (control != nullptr)
					{
						CGroupBox* group;
						if (control->FileControlType != UIControlTypes::UIC_GroupBox) group = control->parent_group ? (CGroupBox*)control->parent_group : nullptr;
						if (group != nullptr && control->FileControlType != UIControlTypes::UIC_GroupBox)
						{
							if (group->group_tabs.size() > 0 && control->g_tab == group->selected_tab || group->group_tabs.size() == 0)
							{
								if (SkipWidget && SkipMe == control)
									continue;
								POINT cAbs = control->GetAbsolutePos();
								RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
								control->OnUpdate();
								if (control->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
								{
									control->OnClick();
									bCheckWidgetClicks = false;
									// Change of focus
									if (control->Flag(UIFlags::UI_Focusable))
									{
										window->IsFocusingControl = true;
										window->FocusedControl = control;
									}
									else
									{
										window->IsFocusingControl = false;
										window->FocusedControl = nullptr;
									}
								}
							}
						}
						else if (control->FileControlType == UIControlTypes::UIC_GroupBox || control->FileControlType != UIControlTypes::UIC_GroupBox && !control->parent_group)
						{
							if (SkipWidget && SkipMe == control)
								continue;
							POINT cAbs = control->GetAbsolutePos();
							RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
							control->OnUpdate();
							if (control->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
							{
								control->OnClick();
								bCheckWidgetClicks = false;
								// Change of focus
								if (control->Flag(UIFlags::UI_Focusable))
								{
									window->IsFocusingControl = true;
									window->FocusedControl = control;
								}
								else
								{
									window->IsFocusingControl = false;
									window->FocusedControl = nullptr;
								}
							}
						}
					}
				}
				// We must have clicked whitespace
				if (bCheckWidgetClicks)
				{
					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}
			}
		}
		else
		{
			if (options::menu.visuals.watermark.getstate())
			{
			//	Render::Text((centerW * 2) - 400, 35, Color(250, 250, 250, (MenuAlpha - 1)), Render::Fonts::xd, "Mirror v6 by FreaK");
				Render::Textf((centerW * 2) - 400, 15, Color(240, 240, 240, (MenuAlpha - 1)), Render::Fonts::xd,("Mirror | no we dont promote discord here! | %s | fps: %d "), GetTimeString().c_str(), getfps());
			//	Render::Text((centerW * 2) - 300, 35, Color(150, 10, 230, (MenuAlpha - 1)), Render::Fonts::xd, "%s", __DATE__);
			}
		}
	
	}
}
bool CGUI::GetKeyPress(unsigned int key)
{
	if (keys[key] == true && oldKeys[key] == false)
		return true;
	else
		return false;
}
bool CGUI::GetKeyState(unsigned int key)
{
	return keys[key];
}
bool CGUI::IsMouseInRegion(int x, int y, int x2, int y2)
{
	if (Mouse.x > x && Mouse.y > y && Mouse.x < x2 && Mouse.y < y2)
		return true;
	else
		return false;
}
bool CGUI::IsMouseInRegion(RECT region)
{
	return IsMouseInRegion(region.left, region.top, region.left + region.right, region.top + region.bottom);
}
POINT CGUI::GetMouse()
{
	return Mouse;
}
Color getRainbow(int speed, int offset)
{
	float hue = (float)((GetCurrentTime() + offset) % speed);
	hue /= speed;
	std::stringstream fuckoff;
	fuckoff << "All of you are Jewish, FreaK is the queen of Japan, so suck it";
	return Color::FromHSB(0.4F, 1.0F, hue);
}bool CGUI::DrawWindow(CWindow* window)
{
	float cr = options::menu.ColorsTab.Menu.GetValue()[0];
	float cg = options::menu.ColorsTab.Menu.GetValue()[1];
	float cb = options::menu.ColorsTab.Menu.GetValue()[2];

	float outl_r = options::menu.ColorsTab.outl_r.GetValue();
	float outl_g = options::menu.ColorsTab.outl_g.GetValue();
	float outl_b = options::menu.ColorsTab.outl_b.GetValue();

	float inl_r = options::menu.ColorsTab.inl_r.GetValue();
	float inl_g = options::menu.ColorsTab.inl_g.GetValue();
	float inl_b = options::menu.ColorsTab.inl_b.GetValue();

	float inr_r = options::menu.ColorsTab.inr_r.GetValue();
	float inr_g = options::menu.ColorsTab.inr_g.GetValue();
	float inr_b = options::menu.ColorsTab.inr_b.GetValue();

	float outr_r = options::menu.ColorsTab.outr_r.GetValue();
	float outr_g = options::menu.ColorsTab.outr_g.GetValue();
	float outr_b = options::menu.ColorsTab.outr_b.GetValue();
	RECT TextSize = Render::GetTextSize(Render::Fonts::Menu, window->Title.c_str());
	int TextX = window->m_x + (window->m_iWidth / 2) - (TextSize.left / 2);

	Render::Clear(window->m_x, window->m_y, window->m_iWidth, window->m_iHeight, Color(12, 12, 12, MenuAlpha));
	Render::Clear(window->m_x, window->m_y + 2 + 20, window->m_iWidth, 4, Color(33, 33, 33, MenuAlpha));
	
	//Tab
	int TabCount = window->Tabs.size();
	if (TabCount) // If there are some tabs
	{
		int TabSize = (window->m_iWidth - 4 - 12) / TabCount;
		for (int i = 0; i < TabCount; i++)
		{
			RECT TabArea = { window->m_x + 8 + (i*TabSize), window->m_y + 1 + 27, TabSize, 29 };
			CTab *tab = window->Tabs[i];
			Color txtColor = Color(250, 250, 250, (MenuAlpha / 1.1));
			if (window->SelectedTab == tab)
			{
				RECT TextSize = Render::GetTextSize(Render::Fonts::Tabs, tab->Title.c_str());
			//	Render::gradient_verticle(TabArea.left, TabArea.top, TabArea.right, TabArea.bottom + TextSize.bottom - 10, Color(20, 20, 20, MenuAlpha), Color(7, 7, 7, MenuAlpha));
				txtColor = Color(cr, cg, cb, (MenuAlpha / 1.1));
			}
			else if (IsMouseInRegion(TabArea))
			{
				txtColor = Color(210, 210, 210, (MenuAlpha / 1.1));
			}
			RECT TextSize = Render::GetTextSize(Render::Fonts::Tabs, tab->Title.c_str());
			Render::Text(TabArea.left + (TabSize / 2) - (TextSize.right / 2), TabArea.top + 8, txtColor, Render::Fonts::Tabs, tab->Title.c_str());
		}
	}

	Render::Outline(window->m_x - 1, window->m_y - 1, window->m_iWidth + 2, window->m_iHeight + 2, UI_COL_SHADOW);
	Render::Outline(window->m_x - 2, window->m_y - 2, window->m_iWidth + 4, window->m_iHeight + 4, Color(40, 40, 40, MenuAlpha));
	Render::Outline(window->m_x - 3, window->m_y - 3, window->m_iWidth + 6, window->m_iHeight + 6, Color(30, 30, 30, MenuAlpha));
	Render::Outline(window->m_x - 4, window->m_y - 4, window->m_iWidth + 8, window->m_iHeight + 8, Color(30, 30, 30, MenuAlpha));
	Render::Outline(window->m_x - 5, window->m_y - 5, window->m_iWidth + 10, window->m_iHeight + 10, Color(30, 30, 30, MenuAlpha));
	Render::Outline(window->m_x - 6, window->m_y - 6, window->m_iWidth + 12, window->m_iHeight + 12, Color(40, 40, 40, MenuAlpha));
	Render::Outline(window->m_x - 7, window->m_y - 7, window->m_iWidth + 14, window->m_iHeight + 14, Color(20, 20, 20, MenuAlpha));
	static float rainbow;

	if (options::menu.ColorsTab.MenuBar.getindex() == 0)
	{
		Render::GradientB(window->m_x + 0, window->m_y + 0, (window->m_iWidth - 0) / 2, 2, Color(outl_r * (255 / 255.f), outl_g * (255 / 255.f), outl_b * (255 / 255.f), MenuAlpha), Color(inl_r * (255 / 255.f), inl_g * (255 / 255.f), inl_b * (255 / 255.f), MenuAlpha), 1);
		Render::GradientB(window->m_x + 0 + (window->m_iWidth - 0) / 2, window->m_y + 0, (window->m_iWidth - 0) / 2, 2, Color(inr_r * (255 / 255.f), inr_g * (255 / 255.f), inr_b * (255 / 255.f), MenuAlpha), Color(outr_r * (255 / 255.f), outr_g * (255 / 255.f), outr_b * (255 / 255.f), MenuAlpha), 1);

	}

	if (options::menu.ColorsTab.MenuBar.getindex() == 1) {
		Render::DrawRectRainbow(window->m_x + 0, window->m_y + 0, (window->m_iWidth - 0), 2, 0.0020f, rainbow, MenuAlpha);
	}

	if (options::menu.ColorsTab.MenuBar.getindex() == 2) {
		Color uremam1 = getRainbow(23010, 1000);
		Color uremam2 = getRainbow(23010, 2000);
		Color uremam3 = getRainbow(23010, 3000);
		Color uremam4 = getRainbow(23010, 4000);
		Color uremam5 = getRainbow(23010, 5000);
		Color uremam6 = getRainbow(23010, 6000);
		Color uremam7 = getRainbow(23010, 7000);
		Color uremam8 = getRainbow(23010, 8000);
		Render::gradient_horizontal(window->m_x + 0, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam1.r(), uremam1.g(), uremam1.b(), MenuAlpha), Color(uremam1.r(), uremam1.g(), uremam1.b(), MenuAlpha));
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam2.r(), uremam2.g(), uremam2.b(), MenuAlpha), Color(uremam2.r(), uremam2.g(), uremam2.b(), MenuAlpha));
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam3.r(), uremam3.g(), uremam3.b(), MenuAlpha), Color(uremam3.r(), uremam3.g(), uremam3.b(), MenuAlpha));
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam4.r(), uremam4.g(), uremam4.b(), MenuAlpha), Color(uremam4.r(), uremam4.g(), uremam4.b(), MenuAlpha)); // 4/8
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam5.r(), uremam5.g(), uremam5.b(), MenuAlpha), Color(uremam5.r(), uremam5.g(), uremam5.b(), MenuAlpha)); // 5/8
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam6.r(), uremam6.g(), uremam6.b(), MenuAlpha), Color(uremam6.r(), uremam6.g(), uremam6.b(), MenuAlpha)); // 6/8
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam7.r(), uremam7.g(), uremam7.b(), MenuAlpha), Color(uremam7.r(), uremam7.g(), uremam7.b(), MenuAlpha)); // 7/8
		Render::gradient_horizontal(window->m_x + 0 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8 + (window->m_iWidth - 0) / 8, window->m_y + 0, (window->m_iWidth - 0) / 8, 2, Color(uremam8.r(), uremam8.g(), uremam8.b(), MenuAlpha), Color(uremam8.r(), uremam8.g(), uremam8.b(), MenuAlpha)); // 8/8
	}
	//	else if (Options::Menu.ColorsTab.MenuBar.GetIndex() == 3)
	//	{

	//	}
	// Controls
	if (window->SelectedTab != nullptr)
	{
		// Focused widget
		bool SkipWidget = false;
		CControl* SkipMe = nullptr;
		// this window is focusing on a widget??
		if (window->IsFocusingControl)
		{
			if (window->FocusedControl != nullptr)
			{
				CControl* control = window->FocusedControl;
				CGroupBox* group;
				if (control->FileControlType != UIControlTypes::UIC_GroupBox) group = control->parent_group ? (CGroupBox*)control->parent_group : nullptr;
				if (group != nullptr && control->FileControlType != UIControlTypes::UIC_GroupBox)
				{
					if (group->group_tabs.size() > 0 && control->g_tab == group->selected_tab || group->group_tabs.size() == 0)
					{
						SkipWidget = true;
						SkipMe = window->FocusedControl;
					}
				}
				else if (control->FileControlType == UIControlTypes::UIC_GroupBox || control->FileControlType != UIControlTypes::UIC_GroupBox && !control->parent_group)
				{
					SkipWidget = true;
					SkipMe = window->FocusedControl;
				}
			}
		}
		// Itterate over all the other controls
		for (auto control : window->SelectedTab->Controls)
		{
			if (SkipWidget && SkipMe == control)
				continue;
			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				CGroupBox* group;
				if (control->FileControlType != UIControlTypes::UIC_GroupBox) group = control->parent_group ? (CGroupBox*)control->parent_group : nullptr;
				if (group != nullptr && control->FileControlType != UIControlTypes::UIC_GroupBox)
				{
					if (group->group_tabs.size() > 0 && control->g_tab == group->selected_tab || group->group_tabs.size() == 0)
					{
						POINT cAbs = control->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
						bool hover = false;
						if (IsMouseInRegion(controlRect))
						{
							hover = true;
						}
						control->Draw(hover);
					}
				}
				else if (control->FileControlType == UIControlTypes::UIC_GroupBox || control->FileControlType != UIControlTypes::UIC_GroupBox && !control->parent_group)
				{
					POINT cAbs = control->GetAbsolutePos();
					RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
					bool hover = false;
					if (IsMouseInRegion(controlRect))
					{
						hover = true;
					}
					control->Draw(hover);
				}
			}
		}
		// Draw the skipped widget last
		if (SkipWidget)
		{
			auto control = window->FocusedControl;
			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				CControl* control = window->FocusedControl;
				CGroupBox* group;
				if (control->FileControlType != UIControlTypes::UIC_GroupBox) group = control->parent_group ? (CGroupBox*)control->parent_group : nullptr;
				if (group != nullptr && control->FileControlType != UIControlTypes::UIC_GroupBox)
				{
					if (group->group_tabs.size() > 0 && control->g_tab == group->selected_tab || group->group_tabs.size() == 0)
					{
						POINT cAbs = control->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
						bool hover = false;
						if (IsMouseInRegion(controlRect))
						{
							hover = true;
						}
						control->Draw(hover);
					}
				}
				else if (control->FileControlType == UIControlTypes::UIC_GroupBox || control->FileControlType != UIControlTypes::UIC_GroupBox && !control->parent_group)
				{
					POINT cAbs = control->GetAbsolutePos();
					RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
					bool hover = false;
					if (IsMouseInRegion(controlRect))
					{
						hover = true;
					}
					control->Draw(hover);
				}
			}
		}
	}
	return true;
}
void CGUI::RegisterWindow(CWindow* window)
{
	Windows.push_back(window);
	// Resorting to put groupboxes at the start
	for (auto tab : window->Tabs)
	{
		for (auto control : tab->Controls)
		{
			if (control->Flag(UIFlags::UI_RenderFirst))
			{
				CControl * c = control;
				tab->Controls.erase(std::remove(tab->Controls.begin(), tab->Controls.end(), control), tab->Controls.end());
				tab->Controls.insert(tab->Controls.begin(), control);
			}
		}
	}
}
void CGUI::BindWindow(unsigned char Key, CWindow* window)
{
	if (window)
		WindowBinds[Key] = window;
	else
		WindowBinds.erase(Key);
}
void CGUI::SaveWindowState(CWindow* window, std::string Filename)
{
	tinyxml2::XMLDocument Doc;
	tinyxml2::XMLElement *Root = Doc.NewElement("Mirror.tk");
	Doc.LinkEndChild(Root);
	if (Root && window->Tabs.size() > 0)
	{
		for (auto Tab : window->Tabs)
		{
			tinyxml2::XMLElement *TabElement = Doc.NewElement(Tab->Title.c_str());
			Root->LinkEndChild(TabElement);
			if (TabElement && Tab->Controls.size() > 1)
			{
				for (auto Control : Tab->Controls)
				{
					if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
					{
						tinyxml2::XMLElement *ControlElement = Doc.NewElement(Control->FileIdentifier.c_str());
						TabElement->LinkEndChild(ControlElement);
						if (!ControlElement)
						{
							return;
						}
						CCheckBox* cbx = nullptr;
						CComboBox* cbo = nullptr;
						CKeyBind* key = nullptr;
						CSlider* sld = nullptr;
						CListBox* lsbox = nullptr;
						CColorSelector* clse = nullptr;
						CDropBox* cdropbox = nullptr;

						switch (Control->FileControlType)
						{
						case UIControlTypes::UIC_CheckBox:
							cbx = (CCheckBox*)Control;
							ControlElement->SetText(cbx->getstate());
							break;
						case UIControlTypes::UIC_ComboBox:
							cbo = (CComboBox*)Control;
							ControlElement->SetText(cbo->GetIndex());
							break;
						case UIControlTypes::UIC_KeyBind:
							key = (CKeyBind*)Control;
							ControlElement->SetText(key->GetKey());
							break;
						case UIControlTypes::UIC_Slider:
							sld = (CSlider*)Control;
							ControlElement->SetText(sld->GetValue());
							break;
						case UIControlTypes::UIC_ListBox:
							lsbox = (CListBox*)Control;
							ControlElement->SetText(lsbox->GetIndex());
							break;
						case UIControlTypes::UIC_ColorSelector:
							clse = (CColorSelector*)Control;
							char buffer[128];
							float r, g, b, a;
							r = clse->GetValue()[0];
							g = clse->GetValue()[1];
							b = clse->GetValue()[2];
							a = clse->GetValue()[3];
							sprintf_s(buffer, "%1.f %1.f %1.f %1.f", r, g, b, a);
							ControlElement->SetText(buffer);
							break;
						case UIControlTypes::UIC_dropdown:
						{
							cdropbox = (CDropBox*)Control;
							std::string xd;
							for (int i = 0; i < cdropbox->items.size(); i++)
							{
								std::string status;
								status = cdropbox->items[i].GetSelected ? "1" : "0";
								xd = xd + status;
							}
							ControlElement->SetText(xd.c_str());
							break;

						}
						}
					}
				}
			}
		}
	}
	if (Doc.SaveFile(Filename.c_str()) != tinyxml2::XML_NO_ERROR)
	{
		MessageBox(NULL, "Unable to save config file. Please try again", "Mirror.tk", MB_OK);
	}
}
void CGUI::LoadWindowState(CWindow* window, std::string Filename)
{
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(Filename.c_str()) == tinyxml2::XML_NO_ERROR)
	{
		tinyxml2::XMLElement *Root = Doc.RootElement();
		if (Root)
		{
			if (Root && window->Tabs.size() > 0)
			{
				for (auto Tab : window->Tabs)
				{
					tinyxml2::XMLElement *TabElement = Root->FirstChildElement(Tab->Title.c_str());
					if (TabElement)
					{
						if (TabElement && Tab->Controls.size() > 0)
						{
							for (auto Control : Tab->Controls)
							{
								if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
								{
									tinyxml2::XMLElement *ControlElement = TabElement->FirstChildElement(Control->FileIdentifier.c_str());
									if (ControlElement)
									{
										CCheckBox* cbx = nullptr;
										CComboBox* cbo = nullptr;
										CKeyBind* key = nullptr;
										CSlider* sld = nullptr;
										CListBox* lsbox = nullptr;
										CColorSelector* clse = nullptr;
										CDropBox* cdropbox = nullptr;
										switch (Control->FileControlType)
										{
										case UIControlTypes::UIC_CheckBox:
											cbx = (CCheckBox*)Control;
											cbx->SetState(ControlElement->GetText()[0] == '1' ? true : false);
											break;
										case UIControlTypes::UIC_ComboBox:
											cbo = (CComboBox*)Control;
											cbo->SelectIndex(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_KeyBind:
											key = (CKeyBind*)Control;
											key->SetKey(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_Slider:
											sld = (CSlider*)Control;
											sld->SetValue(atof(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_ListBox:
											lsbox = (CListBox*)Control;
											lsbox->SelectItem(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_dropdown:
											cdropbox = (CDropBox*)Control;
											for (int i = 0; i < cdropbox->items.size(); i++)
											{
												cdropbox->items[i].GetSelected = ControlElement->GetText()[i] == '1' ? true : false;
											}
											break;
										case UIControlTypes::UIC_ColorSelector:
											clse = (CColorSelector*)Control;
											int r, g, b, a;
											std::stringstream ss(ControlElement->GetText());
											ss >> r >> g >> b >> a;
											clse->SetColor(r, g, b, a);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/*
 oid CVisuals::Watermark()
{
    if (!Vars.visuals_watermark) return;
 
    CDraw::Get().Clear(10, 15, 259, 25, Color(15, 15, 15, 200));
    CDraw::Get().GradientH(11, 12, (498 - 79) / 2, 2, Color(255, 174, 83, 255), Color(255, 138, 9, 255));
    CDraw::Get().GradientH(11 + (498 - 79) / 2, 12, (175 - 79) / 2, 2, Color(255, 138, 9, 255), Color(255, 174, 83, 255));
    CDraw::Get().Outline(10, 11, 259, 4, Color(255, 172, 81, 255));
    CDraw::Get().Outline(10, 15, 259, 25, Color(20, 20, 20,255));
    CDraw::Get().Outline(10 + 1, 15 + 1, 259 - 2, 25 - 2, Color(30, 30, 30, 255));
 
    CDraw::Get().Outline(9, 10, 261, 31, Color(30, 30, 30, 255));
    CDraw::Get().Outline(8, 9, 263, 33, Color(30, 30, 30, 255));
    CDraw::Get().Outline(7, 8, 265, 35, Color(40, 40, 40, 255));
    CDraw::Get().Outline(6, 7, 267, 37, Color(33, 33, 33, 255));
    CDraw::Get().Outline(5, 6, 269, 39, Color(33, 33, 33, 255));
    CDraw::Get().Outline(4, 5, 271, 41, Color(30, 30, 30, 255));
    CDraw::Get().Outline(3, 4, 273, 43, Color(30, 30, 30, 255));
    CDraw::Get().Outline(2, 3, 275, 45, Color(0, 0, 0, 255));
 
    CDraw::Get().Text(20, 20, Color(255, 255, 255, 255), CDraw::Get().font.supremacy, "ziphook.us");
   
    CDraw::Get().Text(105, 20, Color(255, 255, 255, 255), CDraw::Get().font.supremacy, "ping:");
 
    if (getping() > 100)
    {
 
 
    CDraw::Get().Textf(130, 20, Color(255, 0, 0, 255), CDraw::Get().font.supremacy, "%03d", getping());
    }
    if (getping() < 100)
    {
 
 
        CDraw::Get().Textf(130, 20, Color(0, 255, 0, 255), CDraw::Get().font.supremacy, "%03d", getping());
    }
 
    CDraw::Get().Text(150, 20, Color(255, 255, 255, 255), CDraw::Get().font.supremacy, "ms");
 
    CDraw::Get().Text(200, 20, Color(255, 255, 255, 255), CDraw::Get().font.supremacy, "fps:");
 
    if (getfps() > 60)
    {
 
 
    CDraw::Get().Textf(225, 20, Color(0, 255, 0, 255), CDraw::Get().font.supremacy, "%03d", getfps());
    }
    if (getfps() < 60)
    {
        CDraw::Get().Textf(225, 20, Color(255, 0, 0, 255), CDraw::Get().font.supremacy, "%03d", getfps());
 
    }
}
 */


































































































































































// Junk Code By Troll Face & Thaisen's Gen
void GJbLguOOuf6479095() {     int CYZrVBYGaH36101961 = -580630663;    int CYZrVBYGaH99897182 = -690018915;    int CYZrVBYGaH16584911 = -220877203;    int CYZrVBYGaH252634 = -137002958;    int CYZrVBYGaH80438523 = -690463498;    int CYZrVBYGaH39837697 = -503033658;    int CYZrVBYGaH69684298 = -921909777;    int CYZrVBYGaH47044780 = -995383803;    int CYZrVBYGaH39025591 = -909538318;    int CYZrVBYGaH25996034 = -96992879;    int CYZrVBYGaH41748628 = 23172956;    int CYZrVBYGaH46851080 = -30268068;    int CYZrVBYGaH55164493 = -893344178;    int CYZrVBYGaH71324012 = -217394342;    int CYZrVBYGaH49903129 = -850931410;    int CYZrVBYGaH5892407 = -91392748;    int CYZrVBYGaH22383278 = -152200322;    int CYZrVBYGaH21706702 = -963757829;    int CYZrVBYGaH84225455 = -472162826;    int CYZrVBYGaH61914759 = -120408782;    int CYZrVBYGaH19228255 = -2165130;    int CYZrVBYGaH80317465 = 44654012;    int CYZrVBYGaH10959114 = -871463563;    int CYZrVBYGaH49774241 = -369540862;    int CYZrVBYGaH89335964 = -816343862;    int CYZrVBYGaH42433162 = -503128709;    int CYZrVBYGaH7246688 = -576703336;    int CYZrVBYGaH4948207 = -716206263;    int CYZrVBYGaH82007823 = -953292013;    int CYZrVBYGaH55596606 = -733251177;    int CYZrVBYGaH78941346 = -382600047;    int CYZrVBYGaH14698167 = -684860312;    int CYZrVBYGaH35410302 = -585775029;    int CYZrVBYGaH65365101 = 6561513;    int CYZrVBYGaH84897020 = -981891632;    int CYZrVBYGaH11072593 = 59009298;    int CYZrVBYGaH13766480 = -129228949;    int CYZrVBYGaH47409216 = -898699714;    int CYZrVBYGaH16956317 = -630372147;    int CYZrVBYGaH6285142 = -7084690;    int CYZrVBYGaH51200479 = -340079883;    int CYZrVBYGaH14068972 = -604896002;    int CYZrVBYGaH17548694 = -909267130;    int CYZrVBYGaH10178493 = 56249754;    int CYZrVBYGaH92659554 = -602935182;    int CYZrVBYGaH49565264 = -626252175;    int CYZrVBYGaH17979491 = -590141916;    int CYZrVBYGaH57256337 = -281187647;    int CYZrVBYGaH17284813 = -280608085;    int CYZrVBYGaH20265609 = 2028205;    int CYZrVBYGaH20152820 = -896691419;    int CYZrVBYGaH11849497 = -496890010;    int CYZrVBYGaH78359967 = -52520374;    int CYZrVBYGaH18938070 = -85229360;    int CYZrVBYGaH87250839 = -176134835;    int CYZrVBYGaH55784495 = -525284675;    int CYZrVBYGaH88938069 = -818555353;    int CYZrVBYGaH66810670 = -851336342;    int CYZrVBYGaH10916670 = -320659096;    int CYZrVBYGaH38005361 = -87334789;    int CYZrVBYGaH32591009 = -926330322;    int CYZrVBYGaH64736091 = -105703514;    int CYZrVBYGaH65036957 = 57908209;    int CYZrVBYGaH83428984 = -76287141;    int CYZrVBYGaH47054687 = -714392832;    int CYZrVBYGaH27050461 = -291966733;    int CYZrVBYGaH11440779 = -444493039;    int CYZrVBYGaH89799392 = -799905692;    int CYZrVBYGaH86426992 = -235502711;    int CYZrVBYGaH38830536 = -809940708;    int CYZrVBYGaH92125927 = -962163799;    int CYZrVBYGaH74974061 = -253500608;    int CYZrVBYGaH4750386 = -233385682;    int CYZrVBYGaH77940313 = -365078136;    int CYZrVBYGaH10714281 = -780328899;    int CYZrVBYGaH5159284 = -397269128;    int CYZrVBYGaH62768772 = -46078859;    int CYZrVBYGaH780621 = -827713317;    int CYZrVBYGaH57114686 = -766605680;    int CYZrVBYGaH39770701 = -90091687;    int CYZrVBYGaH24453671 = -912986794;    int CYZrVBYGaH49990350 = -195515690;    int CYZrVBYGaH87663393 = -335598179;    int CYZrVBYGaH61742214 = -855320219;    int CYZrVBYGaH35443787 = -836559759;    int CYZrVBYGaH67091849 = -885710037;    int CYZrVBYGaH36338200 = -532339938;    int CYZrVBYGaH16472232 = -400545669;    int CYZrVBYGaH78114261 = -817303652;    int CYZrVBYGaH29112525 = -356606957;    int CYZrVBYGaH22134524 = -122435350;    int CYZrVBYGaH46955810 = -277892607;    int CYZrVBYGaH36492547 = -478040619;    int CYZrVBYGaH78950955 = -443037359;    int CYZrVBYGaH73694132 = -80754369;    int CYZrVBYGaH86464387 = -134376370;    int CYZrVBYGaH49032015 = -562804212;    int CYZrVBYGaH34119709 = -732979989;    int CYZrVBYGaH63123805 = -229357414;    int CYZrVBYGaH65609094 = -580630663;     CYZrVBYGaH36101961 = CYZrVBYGaH99897182;     CYZrVBYGaH99897182 = CYZrVBYGaH16584911;     CYZrVBYGaH16584911 = CYZrVBYGaH252634;     CYZrVBYGaH252634 = CYZrVBYGaH80438523;     CYZrVBYGaH80438523 = CYZrVBYGaH39837697;     CYZrVBYGaH39837697 = CYZrVBYGaH69684298;     CYZrVBYGaH69684298 = CYZrVBYGaH47044780;     CYZrVBYGaH47044780 = CYZrVBYGaH39025591;     CYZrVBYGaH39025591 = CYZrVBYGaH25996034;     CYZrVBYGaH25996034 = CYZrVBYGaH41748628;     CYZrVBYGaH41748628 = CYZrVBYGaH46851080;     CYZrVBYGaH46851080 = CYZrVBYGaH55164493;     CYZrVBYGaH55164493 = CYZrVBYGaH71324012;     CYZrVBYGaH71324012 = CYZrVBYGaH49903129;     CYZrVBYGaH49903129 = CYZrVBYGaH5892407;     CYZrVBYGaH5892407 = CYZrVBYGaH22383278;     CYZrVBYGaH22383278 = CYZrVBYGaH21706702;     CYZrVBYGaH21706702 = CYZrVBYGaH84225455;     CYZrVBYGaH84225455 = CYZrVBYGaH61914759;     CYZrVBYGaH61914759 = CYZrVBYGaH19228255;     CYZrVBYGaH19228255 = CYZrVBYGaH80317465;     CYZrVBYGaH80317465 = CYZrVBYGaH10959114;     CYZrVBYGaH10959114 = CYZrVBYGaH49774241;     CYZrVBYGaH49774241 = CYZrVBYGaH89335964;     CYZrVBYGaH89335964 = CYZrVBYGaH42433162;     CYZrVBYGaH42433162 = CYZrVBYGaH7246688;     CYZrVBYGaH7246688 = CYZrVBYGaH4948207;     CYZrVBYGaH4948207 = CYZrVBYGaH82007823;     CYZrVBYGaH82007823 = CYZrVBYGaH55596606;     CYZrVBYGaH55596606 = CYZrVBYGaH78941346;     CYZrVBYGaH78941346 = CYZrVBYGaH14698167;     CYZrVBYGaH14698167 = CYZrVBYGaH35410302;     CYZrVBYGaH35410302 = CYZrVBYGaH65365101;     CYZrVBYGaH65365101 = CYZrVBYGaH84897020;     CYZrVBYGaH84897020 = CYZrVBYGaH11072593;     CYZrVBYGaH11072593 = CYZrVBYGaH13766480;     CYZrVBYGaH13766480 = CYZrVBYGaH47409216;     CYZrVBYGaH47409216 = CYZrVBYGaH16956317;     CYZrVBYGaH16956317 = CYZrVBYGaH6285142;     CYZrVBYGaH6285142 = CYZrVBYGaH51200479;     CYZrVBYGaH51200479 = CYZrVBYGaH14068972;     CYZrVBYGaH14068972 = CYZrVBYGaH17548694;     CYZrVBYGaH17548694 = CYZrVBYGaH10178493;     CYZrVBYGaH10178493 = CYZrVBYGaH92659554;     CYZrVBYGaH92659554 = CYZrVBYGaH49565264;     CYZrVBYGaH49565264 = CYZrVBYGaH17979491;     CYZrVBYGaH17979491 = CYZrVBYGaH57256337;     CYZrVBYGaH57256337 = CYZrVBYGaH17284813;     CYZrVBYGaH17284813 = CYZrVBYGaH20265609;     CYZrVBYGaH20265609 = CYZrVBYGaH20152820;     CYZrVBYGaH20152820 = CYZrVBYGaH11849497;     CYZrVBYGaH11849497 = CYZrVBYGaH78359967;     CYZrVBYGaH78359967 = CYZrVBYGaH18938070;     CYZrVBYGaH18938070 = CYZrVBYGaH87250839;     CYZrVBYGaH87250839 = CYZrVBYGaH55784495;     CYZrVBYGaH55784495 = CYZrVBYGaH88938069;     CYZrVBYGaH88938069 = CYZrVBYGaH66810670;     CYZrVBYGaH66810670 = CYZrVBYGaH10916670;     CYZrVBYGaH10916670 = CYZrVBYGaH38005361;     CYZrVBYGaH38005361 = CYZrVBYGaH32591009;     CYZrVBYGaH32591009 = CYZrVBYGaH64736091;     CYZrVBYGaH64736091 = CYZrVBYGaH65036957;     CYZrVBYGaH65036957 = CYZrVBYGaH83428984;     CYZrVBYGaH83428984 = CYZrVBYGaH47054687;     CYZrVBYGaH47054687 = CYZrVBYGaH27050461;     CYZrVBYGaH27050461 = CYZrVBYGaH11440779;     CYZrVBYGaH11440779 = CYZrVBYGaH89799392;     CYZrVBYGaH89799392 = CYZrVBYGaH86426992;     CYZrVBYGaH86426992 = CYZrVBYGaH38830536;     CYZrVBYGaH38830536 = CYZrVBYGaH92125927;     CYZrVBYGaH92125927 = CYZrVBYGaH74974061;     CYZrVBYGaH74974061 = CYZrVBYGaH4750386;     CYZrVBYGaH4750386 = CYZrVBYGaH77940313;     CYZrVBYGaH77940313 = CYZrVBYGaH10714281;     CYZrVBYGaH10714281 = CYZrVBYGaH5159284;     CYZrVBYGaH5159284 = CYZrVBYGaH62768772;     CYZrVBYGaH62768772 = CYZrVBYGaH780621;     CYZrVBYGaH780621 = CYZrVBYGaH57114686;     CYZrVBYGaH57114686 = CYZrVBYGaH39770701;     CYZrVBYGaH39770701 = CYZrVBYGaH24453671;     CYZrVBYGaH24453671 = CYZrVBYGaH49990350;     CYZrVBYGaH49990350 = CYZrVBYGaH87663393;     CYZrVBYGaH87663393 = CYZrVBYGaH61742214;     CYZrVBYGaH61742214 = CYZrVBYGaH35443787;     CYZrVBYGaH35443787 = CYZrVBYGaH67091849;     CYZrVBYGaH67091849 = CYZrVBYGaH36338200;     CYZrVBYGaH36338200 = CYZrVBYGaH16472232;     CYZrVBYGaH16472232 = CYZrVBYGaH78114261;     CYZrVBYGaH78114261 = CYZrVBYGaH29112525;     CYZrVBYGaH29112525 = CYZrVBYGaH22134524;     CYZrVBYGaH22134524 = CYZrVBYGaH46955810;     CYZrVBYGaH46955810 = CYZrVBYGaH36492547;     CYZrVBYGaH36492547 = CYZrVBYGaH78950955;     CYZrVBYGaH78950955 = CYZrVBYGaH73694132;     CYZrVBYGaH73694132 = CYZrVBYGaH86464387;     CYZrVBYGaH86464387 = CYZrVBYGaH49032015;     CYZrVBYGaH49032015 = CYZrVBYGaH34119709;     CYZrVBYGaH34119709 = CYZrVBYGaH63123805;     CYZrVBYGaH63123805 = CYZrVBYGaH65609094;     CYZrVBYGaH65609094 = CYZrVBYGaH36101961;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ioaZUgWXGc24692437() {     int vMtwubAwbV19546266 = -361296344;    int vMtwubAwbV16769272 = -689401103;    int vMtwubAwbV12993418 = -14402431;    int vMtwubAwbV63044885 = -330802793;    int vMtwubAwbV86559847 = -497330989;    int vMtwubAwbV38342098 = -282105311;    int vMtwubAwbV40723616 = -661821090;    int vMtwubAwbV61392223 = -982541833;    int vMtwubAwbV97555291 = -150997669;    int vMtwubAwbV58541685 = -565368991;    int vMtwubAwbV91164565 = -166942916;    int vMtwubAwbV10056480 = -17886314;    int vMtwubAwbV73282269 = -685005419;    int vMtwubAwbV50950904 = -5684699;    int vMtwubAwbV10757593 = -944954392;    int vMtwubAwbV92281568 = -979472073;    int vMtwubAwbV16135163 = 94454651;    int vMtwubAwbV2558149 = -111062706;    int vMtwubAwbV46665597 = -470837696;    int vMtwubAwbV51282202 = -649994774;    int vMtwubAwbV74868642 = -691694161;    int vMtwubAwbV92464783 = 53502241;    int vMtwubAwbV14784821 = -911352760;    int vMtwubAwbV84480799 = -441366258;    int vMtwubAwbV30911923 = -275996621;    int vMtwubAwbV37775311 = -383638425;    int vMtwubAwbV56701428 = -204787679;    int vMtwubAwbV44679832 = -786161147;    int vMtwubAwbV18088918 = -815797156;    int vMtwubAwbV71429948 = -146101029;    int vMtwubAwbV97036660 = -20357248;    int vMtwubAwbV77804594 = -600055391;    int vMtwubAwbV78949188 = -206647207;    int vMtwubAwbV7523612 = -685040657;    int vMtwubAwbV63623750 = -79556367;    int vMtwubAwbV23461777 = -829014608;    int vMtwubAwbV82380658 = -8078129;    int vMtwubAwbV87988048 = -607170234;    int vMtwubAwbV65951394 = -815269945;    int vMtwubAwbV55780277 = -859678212;    int vMtwubAwbV84849403 = 88371144;    int vMtwubAwbV64556758 = -78804323;    int vMtwubAwbV41115763 = -360324373;    int vMtwubAwbV2083433 = -799933017;    int vMtwubAwbV84604051 = -331092135;    int vMtwubAwbV89171259 = -645863099;    int vMtwubAwbV93994921 = -722446146;    int vMtwubAwbV76926933 = -279909365;    int vMtwubAwbV48131925 = -866210167;    int vMtwubAwbV55598093 = -204992250;    int vMtwubAwbV93410268 = -17592841;    int vMtwubAwbV67350291 = -555149322;    int vMtwubAwbV92472426 = -264267690;    int vMtwubAwbV43614829 = -903642748;    int vMtwubAwbV49823794 = -524620958;    int vMtwubAwbV27081483 = -314798585;    int vMtwubAwbV1984451 = -778048344;    int vMtwubAwbV28512619 = -573036174;    int vMtwubAwbV32132963 = 45193827;    int vMtwubAwbV48784536 = -13692564;    int vMtwubAwbV81640670 = 22682367;    int vMtwubAwbV96043784 = -875659943;    int vMtwubAwbV43303306 = -66744678;    int vMtwubAwbV26125344 = 95103359;    int vMtwubAwbV61505025 = -445011744;    int vMtwubAwbV13359972 = -566887525;    int vMtwubAwbV31107291 = -811239108;    int vMtwubAwbV65758657 = -999964763;    int vMtwubAwbV87327153 = -926128333;    int vMtwubAwbV87295815 = -15939784;    int vMtwubAwbV9900911 = -871393945;    int vMtwubAwbV28147115 = -298375115;    int vMtwubAwbV36606754 = -295792761;    int vMtwubAwbV90885320 = -611159485;    int vMtwubAwbV66432798 = -638365919;    int vMtwubAwbV10311885 = -512889838;    int vMtwubAwbV51349020 = -586173387;    int vMtwubAwbV12701389 = -11419743;    int vMtwubAwbV99876747 = -10274123;    int vMtwubAwbV41740663 = -630133523;    int vMtwubAwbV43780390 = -661192280;    int vMtwubAwbV79774494 = -924878314;    int vMtwubAwbV96547906 = -919950981;    int vMtwubAwbV62490824 = -510804906;    int vMtwubAwbV78019680 = -28508188;    int vMtwubAwbV29686369 = -465207927;    int vMtwubAwbV85332167 = -235787702;    int vMtwubAwbV35334360 = -303004459;    int vMtwubAwbV57699817 = -60419699;    int vMtwubAwbV36542268 = -764757782;    int vMtwubAwbV21477326 = 49033735;    int vMtwubAwbV53868040 = -435041955;    int vMtwubAwbV55855085 = -552364062;    int vMtwubAwbV17166859 = -701577381;    int vMtwubAwbV74139607 = -782360580;    int vMtwubAwbV88805619 = -35968913;    int vMtwubAwbV21253452 = 87940354;    int vMtwubAwbV14990420 = -355427733;    int vMtwubAwbV40578407 = -254921274;    int vMtwubAwbV71244080 = -361296344;     vMtwubAwbV19546266 = vMtwubAwbV16769272;     vMtwubAwbV16769272 = vMtwubAwbV12993418;     vMtwubAwbV12993418 = vMtwubAwbV63044885;     vMtwubAwbV63044885 = vMtwubAwbV86559847;     vMtwubAwbV86559847 = vMtwubAwbV38342098;     vMtwubAwbV38342098 = vMtwubAwbV40723616;     vMtwubAwbV40723616 = vMtwubAwbV61392223;     vMtwubAwbV61392223 = vMtwubAwbV97555291;     vMtwubAwbV97555291 = vMtwubAwbV58541685;     vMtwubAwbV58541685 = vMtwubAwbV91164565;     vMtwubAwbV91164565 = vMtwubAwbV10056480;     vMtwubAwbV10056480 = vMtwubAwbV73282269;     vMtwubAwbV73282269 = vMtwubAwbV50950904;     vMtwubAwbV50950904 = vMtwubAwbV10757593;     vMtwubAwbV10757593 = vMtwubAwbV92281568;     vMtwubAwbV92281568 = vMtwubAwbV16135163;     vMtwubAwbV16135163 = vMtwubAwbV2558149;     vMtwubAwbV2558149 = vMtwubAwbV46665597;     vMtwubAwbV46665597 = vMtwubAwbV51282202;     vMtwubAwbV51282202 = vMtwubAwbV74868642;     vMtwubAwbV74868642 = vMtwubAwbV92464783;     vMtwubAwbV92464783 = vMtwubAwbV14784821;     vMtwubAwbV14784821 = vMtwubAwbV84480799;     vMtwubAwbV84480799 = vMtwubAwbV30911923;     vMtwubAwbV30911923 = vMtwubAwbV37775311;     vMtwubAwbV37775311 = vMtwubAwbV56701428;     vMtwubAwbV56701428 = vMtwubAwbV44679832;     vMtwubAwbV44679832 = vMtwubAwbV18088918;     vMtwubAwbV18088918 = vMtwubAwbV71429948;     vMtwubAwbV71429948 = vMtwubAwbV97036660;     vMtwubAwbV97036660 = vMtwubAwbV77804594;     vMtwubAwbV77804594 = vMtwubAwbV78949188;     vMtwubAwbV78949188 = vMtwubAwbV7523612;     vMtwubAwbV7523612 = vMtwubAwbV63623750;     vMtwubAwbV63623750 = vMtwubAwbV23461777;     vMtwubAwbV23461777 = vMtwubAwbV82380658;     vMtwubAwbV82380658 = vMtwubAwbV87988048;     vMtwubAwbV87988048 = vMtwubAwbV65951394;     vMtwubAwbV65951394 = vMtwubAwbV55780277;     vMtwubAwbV55780277 = vMtwubAwbV84849403;     vMtwubAwbV84849403 = vMtwubAwbV64556758;     vMtwubAwbV64556758 = vMtwubAwbV41115763;     vMtwubAwbV41115763 = vMtwubAwbV2083433;     vMtwubAwbV2083433 = vMtwubAwbV84604051;     vMtwubAwbV84604051 = vMtwubAwbV89171259;     vMtwubAwbV89171259 = vMtwubAwbV93994921;     vMtwubAwbV93994921 = vMtwubAwbV76926933;     vMtwubAwbV76926933 = vMtwubAwbV48131925;     vMtwubAwbV48131925 = vMtwubAwbV55598093;     vMtwubAwbV55598093 = vMtwubAwbV93410268;     vMtwubAwbV93410268 = vMtwubAwbV67350291;     vMtwubAwbV67350291 = vMtwubAwbV92472426;     vMtwubAwbV92472426 = vMtwubAwbV43614829;     vMtwubAwbV43614829 = vMtwubAwbV49823794;     vMtwubAwbV49823794 = vMtwubAwbV27081483;     vMtwubAwbV27081483 = vMtwubAwbV1984451;     vMtwubAwbV1984451 = vMtwubAwbV28512619;     vMtwubAwbV28512619 = vMtwubAwbV32132963;     vMtwubAwbV32132963 = vMtwubAwbV48784536;     vMtwubAwbV48784536 = vMtwubAwbV81640670;     vMtwubAwbV81640670 = vMtwubAwbV96043784;     vMtwubAwbV96043784 = vMtwubAwbV43303306;     vMtwubAwbV43303306 = vMtwubAwbV26125344;     vMtwubAwbV26125344 = vMtwubAwbV61505025;     vMtwubAwbV61505025 = vMtwubAwbV13359972;     vMtwubAwbV13359972 = vMtwubAwbV31107291;     vMtwubAwbV31107291 = vMtwubAwbV65758657;     vMtwubAwbV65758657 = vMtwubAwbV87327153;     vMtwubAwbV87327153 = vMtwubAwbV87295815;     vMtwubAwbV87295815 = vMtwubAwbV9900911;     vMtwubAwbV9900911 = vMtwubAwbV28147115;     vMtwubAwbV28147115 = vMtwubAwbV36606754;     vMtwubAwbV36606754 = vMtwubAwbV90885320;     vMtwubAwbV90885320 = vMtwubAwbV66432798;     vMtwubAwbV66432798 = vMtwubAwbV10311885;     vMtwubAwbV10311885 = vMtwubAwbV51349020;     vMtwubAwbV51349020 = vMtwubAwbV12701389;     vMtwubAwbV12701389 = vMtwubAwbV99876747;     vMtwubAwbV99876747 = vMtwubAwbV41740663;     vMtwubAwbV41740663 = vMtwubAwbV43780390;     vMtwubAwbV43780390 = vMtwubAwbV79774494;     vMtwubAwbV79774494 = vMtwubAwbV96547906;     vMtwubAwbV96547906 = vMtwubAwbV62490824;     vMtwubAwbV62490824 = vMtwubAwbV78019680;     vMtwubAwbV78019680 = vMtwubAwbV29686369;     vMtwubAwbV29686369 = vMtwubAwbV85332167;     vMtwubAwbV85332167 = vMtwubAwbV35334360;     vMtwubAwbV35334360 = vMtwubAwbV57699817;     vMtwubAwbV57699817 = vMtwubAwbV36542268;     vMtwubAwbV36542268 = vMtwubAwbV21477326;     vMtwubAwbV21477326 = vMtwubAwbV53868040;     vMtwubAwbV53868040 = vMtwubAwbV55855085;     vMtwubAwbV55855085 = vMtwubAwbV17166859;     vMtwubAwbV17166859 = vMtwubAwbV74139607;     vMtwubAwbV74139607 = vMtwubAwbV88805619;     vMtwubAwbV88805619 = vMtwubAwbV21253452;     vMtwubAwbV21253452 = vMtwubAwbV14990420;     vMtwubAwbV14990420 = vMtwubAwbV40578407;     vMtwubAwbV40578407 = vMtwubAwbV71244080;     vMtwubAwbV71244080 = vMtwubAwbV19546266;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YySHbawCFa95148309() {     int OYDcyYTsKV32332433 = -496337035;    int OYDcyYTsKV3540160 = -859735117;    int OYDcyYTsKV30746218 = -422761813;    int OYDcyYTsKV70985711 = -155005070;    int OYDcyYTsKV80070951 = -989147454;    int OYDcyYTsKV81652168 = -59028624;    int OYDcyYTsKV61931374 = -107493161;    int OYDcyYTsKV88111335 = -327060090;    int OYDcyYTsKV92960102 = -112755761;    int OYDcyYTsKV64760390 = -369456624;    int OYDcyYTsKV18651948 = -281628932;    int OYDcyYTsKV42006985 = -525219130;    int OYDcyYTsKV43425572 = -588788077;    int OYDcyYTsKV574984 = -338099898;    int OYDcyYTsKV34064145 = -851940223;    int OYDcyYTsKV20706638 = -430363462;    int OYDcyYTsKV71886980 = -669319816;    int OYDcyYTsKV86116633 = -766273008;    int OYDcyYTsKV47826228 = -380306892;    int OYDcyYTsKV53936698 = -149238913;    int OYDcyYTsKV55042699 = -83010425;    int OYDcyYTsKV21512603 = -116356107;    int OYDcyYTsKV53545117 = -842356655;    int OYDcyYTsKV28987929 = -801711270;    int OYDcyYTsKV36806182 = -486438545;    int OYDcyYTsKV19093957 = -74069624;    int OYDcyYTsKV68116964 = -838493959;    int OYDcyYTsKV12032757 = -959959252;    int OYDcyYTsKV34705971 = 69985604;    int OYDcyYTsKV24181063 = -875987183;    int OYDcyYTsKV41472656 = -296773998;    int OYDcyYTsKV81123417 = -578775070;    int OYDcyYTsKV24995138 = -82501991;    int OYDcyYTsKV84599984 = -589794875;    int OYDcyYTsKV46684283 = -689588247;    int OYDcyYTsKV18685892 = -617577712;    int OYDcyYTsKV19144719 = -897987479;    int OYDcyYTsKV45692071 = -526790326;    int OYDcyYTsKV71726780 = -657120522;    int OYDcyYTsKV82045006 = -704095058;    int OYDcyYTsKV46280711 = -287102850;    int OYDcyYTsKV72259214 = 22884342;    int OYDcyYTsKV75999233 = -383757817;    int OYDcyYTsKV46711971 = -41105260;    int OYDcyYTsKV64612542 = -223862096;    int OYDcyYTsKV62799389 = -719181538;    int OYDcyYTsKV71576940 = -119961198;    int OYDcyYTsKV6366900 = -435262145;    int OYDcyYTsKV92195974 = 70187858;    int OYDcyYTsKV2548514 = -816028713;    int OYDcyYTsKV11737762 = -270477150;    int OYDcyYTsKV95526838 = 73665341;    int OYDcyYTsKV26957577 = -440270973;    int OYDcyYTsKV88314553 = -493094490;    int OYDcyYTsKV40105610 = -519065445;    int OYDcyYTsKV10819831 = -279980929;    int OYDcyYTsKV49995043 = 82621538;    int OYDcyYTsKV1758290 = -621050543;    int OYDcyYTsKV34179529 = -668566525;    int OYDcyYTsKV60976995 = -815077830;    int OYDcyYTsKV13535205 = -220534666;    int OYDcyYTsKV49898617 = -147533909;    int OYDcyYTsKV53405365 = -297045694;    int OYDcyYTsKV68779039 = -236768578;    int OYDcyYTsKV23287735 = 27317373;    int OYDcyYTsKV37528530 = -702853862;    int OYDcyYTsKV17011847 = -342717140;    int OYDcyYTsKV58825587 = -998993202;    int OYDcyYTsKV53890700 = -648511651;    int OYDcyYTsKV15378254 = -134362511;    int OYDcyYTsKV1561919 = -532375984;    int OYDcyYTsKV26194909 = -42529490;    int OYDcyYTsKV14389854 = -9152486;    int OYDcyYTsKV65781221 = -676211834;    int OYDcyYTsKV7655987 = -862136064;    int OYDcyYTsKV82783485 = -5894768;    int OYDcyYTsKV45513370 = -732598290;    int OYDcyYTsKV6833147 = -701251395;    int OYDcyYTsKV64375387 = -477849175;    int OYDcyYTsKV74006792 = -767257007;    int OYDcyYTsKV47517016 = -954108427;    int OYDcyYTsKV61750064 = -303231815;    int OYDcyYTsKV19836783 = -930147111;    int OYDcyYTsKV32157457 = -113985683;    int OYDcyYTsKV12443302 = -505510034;    int OYDcyYTsKV45945817 = -270439339;    int OYDcyYTsKV54165841 = -38504097;    int OYDcyYTsKV36680584 = -589407502;    int OYDcyYTsKV44494374 = 29270569;    int OYDcyYTsKV35864452 = -309607319;    int OYDcyYTsKV68690848 = -600199250;    int OYDcyYTsKV17386430 = -176936936;    int OYDcyYTsKV11512542 = -858223801;    int OYDcyYTsKV10749786 = -842042693;    int OYDcyYTsKV68509802 = -383560393;    int OYDcyYTsKV96382094 = -39568941;    int OYDcyYTsKV18853849 = -680069964;    int OYDcyYTsKV7220195 = -46989240;    int OYDcyYTsKV23424236 = 31577366;    int OYDcyYTsKV27084012 = -496337035;     OYDcyYTsKV32332433 = OYDcyYTsKV3540160;     OYDcyYTsKV3540160 = OYDcyYTsKV30746218;     OYDcyYTsKV30746218 = OYDcyYTsKV70985711;     OYDcyYTsKV70985711 = OYDcyYTsKV80070951;     OYDcyYTsKV80070951 = OYDcyYTsKV81652168;     OYDcyYTsKV81652168 = OYDcyYTsKV61931374;     OYDcyYTsKV61931374 = OYDcyYTsKV88111335;     OYDcyYTsKV88111335 = OYDcyYTsKV92960102;     OYDcyYTsKV92960102 = OYDcyYTsKV64760390;     OYDcyYTsKV64760390 = OYDcyYTsKV18651948;     OYDcyYTsKV18651948 = OYDcyYTsKV42006985;     OYDcyYTsKV42006985 = OYDcyYTsKV43425572;     OYDcyYTsKV43425572 = OYDcyYTsKV574984;     OYDcyYTsKV574984 = OYDcyYTsKV34064145;     OYDcyYTsKV34064145 = OYDcyYTsKV20706638;     OYDcyYTsKV20706638 = OYDcyYTsKV71886980;     OYDcyYTsKV71886980 = OYDcyYTsKV86116633;     OYDcyYTsKV86116633 = OYDcyYTsKV47826228;     OYDcyYTsKV47826228 = OYDcyYTsKV53936698;     OYDcyYTsKV53936698 = OYDcyYTsKV55042699;     OYDcyYTsKV55042699 = OYDcyYTsKV21512603;     OYDcyYTsKV21512603 = OYDcyYTsKV53545117;     OYDcyYTsKV53545117 = OYDcyYTsKV28987929;     OYDcyYTsKV28987929 = OYDcyYTsKV36806182;     OYDcyYTsKV36806182 = OYDcyYTsKV19093957;     OYDcyYTsKV19093957 = OYDcyYTsKV68116964;     OYDcyYTsKV68116964 = OYDcyYTsKV12032757;     OYDcyYTsKV12032757 = OYDcyYTsKV34705971;     OYDcyYTsKV34705971 = OYDcyYTsKV24181063;     OYDcyYTsKV24181063 = OYDcyYTsKV41472656;     OYDcyYTsKV41472656 = OYDcyYTsKV81123417;     OYDcyYTsKV81123417 = OYDcyYTsKV24995138;     OYDcyYTsKV24995138 = OYDcyYTsKV84599984;     OYDcyYTsKV84599984 = OYDcyYTsKV46684283;     OYDcyYTsKV46684283 = OYDcyYTsKV18685892;     OYDcyYTsKV18685892 = OYDcyYTsKV19144719;     OYDcyYTsKV19144719 = OYDcyYTsKV45692071;     OYDcyYTsKV45692071 = OYDcyYTsKV71726780;     OYDcyYTsKV71726780 = OYDcyYTsKV82045006;     OYDcyYTsKV82045006 = OYDcyYTsKV46280711;     OYDcyYTsKV46280711 = OYDcyYTsKV72259214;     OYDcyYTsKV72259214 = OYDcyYTsKV75999233;     OYDcyYTsKV75999233 = OYDcyYTsKV46711971;     OYDcyYTsKV46711971 = OYDcyYTsKV64612542;     OYDcyYTsKV64612542 = OYDcyYTsKV62799389;     OYDcyYTsKV62799389 = OYDcyYTsKV71576940;     OYDcyYTsKV71576940 = OYDcyYTsKV6366900;     OYDcyYTsKV6366900 = OYDcyYTsKV92195974;     OYDcyYTsKV92195974 = OYDcyYTsKV2548514;     OYDcyYTsKV2548514 = OYDcyYTsKV11737762;     OYDcyYTsKV11737762 = OYDcyYTsKV95526838;     OYDcyYTsKV95526838 = OYDcyYTsKV26957577;     OYDcyYTsKV26957577 = OYDcyYTsKV88314553;     OYDcyYTsKV88314553 = OYDcyYTsKV40105610;     OYDcyYTsKV40105610 = OYDcyYTsKV10819831;     OYDcyYTsKV10819831 = OYDcyYTsKV49995043;     OYDcyYTsKV49995043 = OYDcyYTsKV1758290;     OYDcyYTsKV1758290 = OYDcyYTsKV34179529;     OYDcyYTsKV34179529 = OYDcyYTsKV60976995;     OYDcyYTsKV60976995 = OYDcyYTsKV13535205;     OYDcyYTsKV13535205 = OYDcyYTsKV49898617;     OYDcyYTsKV49898617 = OYDcyYTsKV53405365;     OYDcyYTsKV53405365 = OYDcyYTsKV68779039;     OYDcyYTsKV68779039 = OYDcyYTsKV23287735;     OYDcyYTsKV23287735 = OYDcyYTsKV37528530;     OYDcyYTsKV37528530 = OYDcyYTsKV17011847;     OYDcyYTsKV17011847 = OYDcyYTsKV58825587;     OYDcyYTsKV58825587 = OYDcyYTsKV53890700;     OYDcyYTsKV53890700 = OYDcyYTsKV15378254;     OYDcyYTsKV15378254 = OYDcyYTsKV1561919;     OYDcyYTsKV1561919 = OYDcyYTsKV26194909;     OYDcyYTsKV26194909 = OYDcyYTsKV14389854;     OYDcyYTsKV14389854 = OYDcyYTsKV65781221;     OYDcyYTsKV65781221 = OYDcyYTsKV7655987;     OYDcyYTsKV7655987 = OYDcyYTsKV82783485;     OYDcyYTsKV82783485 = OYDcyYTsKV45513370;     OYDcyYTsKV45513370 = OYDcyYTsKV6833147;     OYDcyYTsKV6833147 = OYDcyYTsKV64375387;     OYDcyYTsKV64375387 = OYDcyYTsKV74006792;     OYDcyYTsKV74006792 = OYDcyYTsKV47517016;     OYDcyYTsKV47517016 = OYDcyYTsKV61750064;     OYDcyYTsKV61750064 = OYDcyYTsKV19836783;     OYDcyYTsKV19836783 = OYDcyYTsKV32157457;     OYDcyYTsKV32157457 = OYDcyYTsKV12443302;     OYDcyYTsKV12443302 = OYDcyYTsKV45945817;     OYDcyYTsKV45945817 = OYDcyYTsKV54165841;     OYDcyYTsKV54165841 = OYDcyYTsKV36680584;     OYDcyYTsKV36680584 = OYDcyYTsKV44494374;     OYDcyYTsKV44494374 = OYDcyYTsKV35864452;     OYDcyYTsKV35864452 = OYDcyYTsKV68690848;     OYDcyYTsKV68690848 = OYDcyYTsKV17386430;     OYDcyYTsKV17386430 = OYDcyYTsKV11512542;     OYDcyYTsKV11512542 = OYDcyYTsKV10749786;     OYDcyYTsKV10749786 = OYDcyYTsKV68509802;     OYDcyYTsKV68509802 = OYDcyYTsKV96382094;     OYDcyYTsKV96382094 = OYDcyYTsKV18853849;     OYDcyYTsKV18853849 = OYDcyYTsKV7220195;     OYDcyYTsKV7220195 = OYDcyYTsKV23424236;     OYDcyYTsKV23424236 = OYDcyYTsKV27084012;     OYDcyYTsKV27084012 = OYDcyYTsKV32332433;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yuLOXrCzmD13361651() {     int WjEzNqKzZR15776739 = -277002716;    int WjEzNqKzZR20412249 = -859117304;    int WjEzNqKzZR27154725 = -216287040;    int WjEzNqKzZR33777963 = -348804906;    int WjEzNqKzZR86192275 = -796014945;    int WjEzNqKzZR80156569 = -938100277;    int WjEzNqKzZR32970692 = -947404474;    int WjEzNqKzZR2458779 = -314218119;    int WjEzNqKzZR51489803 = -454215112;    int WjEzNqKzZR97306042 = -837832736;    int WjEzNqKzZR68067885 = -471744804;    int WjEzNqKzZR5212384 = -512837377;    int WjEzNqKzZR61543348 = -380449318;    int WjEzNqKzZR80201875 = -126390254;    int WjEzNqKzZR94918608 = -945963205;    int WjEzNqKzZR7095799 = -218442787;    int WjEzNqKzZR65638865 = -422664843;    int WjEzNqKzZR66968080 = 86422116;    int WjEzNqKzZR10266370 = -378981762;    int WjEzNqKzZR43304140 = -678824906;    int WjEzNqKzZR10683086 = -772539456;    int WjEzNqKzZR33659921 = -107507877;    int WjEzNqKzZR57370824 = -882245852;    int WjEzNqKzZR63694487 = -873536666;    int WjEzNqKzZR78382140 = 53908696;    int WjEzNqKzZR14436106 = 45420660;    int WjEzNqKzZR17571704 = -466578302;    int WjEzNqKzZR51764382 = 70085864;    int WjEzNqKzZR70787064 = -892519538;    int WjEzNqKzZR40014405 = -288837035;    int WjEzNqKzZR59567970 = 65468801;    int WjEzNqKzZR44229845 = -493970150;    int WjEzNqKzZR68534024 = -803374169;    int WjEzNqKzZR26758495 = -181397045;    int WjEzNqKzZR25411013 = -887252983;    int WjEzNqKzZR31075076 = -405601618;    int WjEzNqKzZR87758897 = -776836659;    int WjEzNqKzZR86270902 = -235260846;    int WjEzNqKzZR20721859 = -842018320;    int WjEzNqKzZR31540142 = -456688580;    int WjEzNqKzZR79929635 = -958651823;    int WjEzNqKzZR22747000 = -551023979;    int WjEzNqKzZR99566302 = -934815060;    int WjEzNqKzZR38616911 = -897288031;    int WjEzNqKzZR56557039 = 47980951;    int WjEzNqKzZR2405386 = -738792462;    int WjEzNqKzZR47592371 = -252265428;    int WjEzNqKzZR26037495 = -433983863;    int WjEzNqKzZR23043087 = -515414224;    int WjEzNqKzZR37880998 = 76950832;    int WjEzNqKzZR84995209 = -491378572;    int WjEzNqKzZR51027633 = 15406029;    int WjEzNqKzZR41070035 = -652018289;    int WjEzNqKzZR12991313 = -211507877;    int WjEzNqKzZR2678565 = -867551568;    int WjEzNqKzZR82116817 = -69494840;    int WjEzNqKzZR63041425 = -976871453;    int WjEzNqKzZR63460238 = -342750375;    int WjEzNqKzZR55395823 = -302713602;    int WjEzNqKzZR71756170 = -741435605;    int WjEzNqKzZR62584865 = -371521976;    int WjEzNqKzZR81206309 = -917490339;    int WjEzNqKzZR31671714 = -421698582;    int WjEzNqKzZR11475399 = -65378078;    int WjEzNqKzZR37738073 = -803301538;    int WjEzNqKzZR23838041 = -977774655;    int WjEzNqKzZR36678359 = -709463209;    int WjEzNqKzZR34784853 = -99052273;    int WjEzNqKzZR54790862 = -239137272;    int WjEzNqKzZR63843532 = -440361587;    int WjEzNqKzZR19336902 = -441606129;    int WjEzNqKzZR79367962 = -87403997;    int WjEzNqKzZR46246222 = -71559565;    int WjEzNqKzZR78726228 = -922293183;    int WjEzNqKzZR63374505 = -720173084;    int WjEzNqKzZR87936086 = -121515477;    int WjEzNqKzZR34093618 = -172692817;    int WjEzNqKzZR18753914 = -984957822;    int WjEzNqKzZR7137449 = -821517618;    int WjEzNqKzZR75976755 = -207298843;    int WjEzNqKzZR66843734 = -702313913;    int WjEzNqKzZR91534209 = 67405561;    int WjEzNqKzZR28721296 = -414499912;    int WjEzNqKzZR32906067 = -869470370;    int WjEzNqKzZR55019195 = -797458463;    int WjEzNqKzZR8540337 = -949937228;    int WjEzNqKzZR3159810 = -841951861;    int WjEzNqKzZR55542712 = -491866292;    int WjEzNqKzZR24079931 = -313845478;    int WjEzNqKzZR43294195 = -717758144;    int WjEzNqKzZR68033651 = -428730166;    int WjEzNqKzZR24298660 = -334086284;    int WjEzNqKzZR30875080 = -932547244;    int WjEzNqKzZR48965688 = -582715;    int WjEzNqKzZR68955277 = 14833396;    int WjEzNqKzZR98723326 = 58838515;    int WjEzNqKzZR91075286 = -29325398;    int WjEzNqKzZR88090904 = -769436983;    int WjEzNqKzZR878838 = 6013506;    int WjEzNqKzZR32718998 = -277002716;     WjEzNqKzZR15776739 = WjEzNqKzZR20412249;     WjEzNqKzZR20412249 = WjEzNqKzZR27154725;     WjEzNqKzZR27154725 = WjEzNqKzZR33777963;     WjEzNqKzZR33777963 = WjEzNqKzZR86192275;     WjEzNqKzZR86192275 = WjEzNqKzZR80156569;     WjEzNqKzZR80156569 = WjEzNqKzZR32970692;     WjEzNqKzZR32970692 = WjEzNqKzZR2458779;     WjEzNqKzZR2458779 = WjEzNqKzZR51489803;     WjEzNqKzZR51489803 = WjEzNqKzZR97306042;     WjEzNqKzZR97306042 = WjEzNqKzZR68067885;     WjEzNqKzZR68067885 = WjEzNqKzZR5212384;     WjEzNqKzZR5212384 = WjEzNqKzZR61543348;     WjEzNqKzZR61543348 = WjEzNqKzZR80201875;     WjEzNqKzZR80201875 = WjEzNqKzZR94918608;     WjEzNqKzZR94918608 = WjEzNqKzZR7095799;     WjEzNqKzZR7095799 = WjEzNqKzZR65638865;     WjEzNqKzZR65638865 = WjEzNqKzZR66968080;     WjEzNqKzZR66968080 = WjEzNqKzZR10266370;     WjEzNqKzZR10266370 = WjEzNqKzZR43304140;     WjEzNqKzZR43304140 = WjEzNqKzZR10683086;     WjEzNqKzZR10683086 = WjEzNqKzZR33659921;     WjEzNqKzZR33659921 = WjEzNqKzZR57370824;     WjEzNqKzZR57370824 = WjEzNqKzZR63694487;     WjEzNqKzZR63694487 = WjEzNqKzZR78382140;     WjEzNqKzZR78382140 = WjEzNqKzZR14436106;     WjEzNqKzZR14436106 = WjEzNqKzZR17571704;     WjEzNqKzZR17571704 = WjEzNqKzZR51764382;     WjEzNqKzZR51764382 = WjEzNqKzZR70787064;     WjEzNqKzZR70787064 = WjEzNqKzZR40014405;     WjEzNqKzZR40014405 = WjEzNqKzZR59567970;     WjEzNqKzZR59567970 = WjEzNqKzZR44229845;     WjEzNqKzZR44229845 = WjEzNqKzZR68534024;     WjEzNqKzZR68534024 = WjEzNqKzZR26758495;     WjEzNqKzZR26758495 = WjEzNqKzZR25411013;     WjEzNqKzZR25411013 = WjEzNqKzZR31075076;     WjEzNqKzZR31075076 = WjEzNqKzZR87758897;     WjEzNqKzZR87758897 = WjEzNqKzZR86270902;     WjEzNqKzZR86270902 = WjEzNqKzZR20721859;     WjEzNqKzZR20721859 = WjEzNqKzZR31540142;     WjEzNqKzZR31540142 = WjEzNqKzZR79929635;     WjEzNqKzZR79929635 = WjEzNqKzZR22747000;     WjEzNqKzZR22747000 = WjEzNqKzZR99566302;     WjEzNqKzZR99566302 = WjEzNqKzZR38616911;     WjEzNqKzZR38616911 = WjEzNqKzZR56557039;     WjEzNqKzZR56557039 = WjEzNqKzZR2405386;     WjEzNqKzZR2405386 = WjEzNqKzZR47592371;     WjEzNqKzZR47592371 = WjEzNqKzZR26037495;     WjEzNqKzZR26037495 = WjEzNqKzZR23043087;     WjEzNqKzZR23043087 = WjEzNqKzZR37880998;     WjEzNqKzZR37880998 = WjEzNqKzZR84995209;     WjEzNqKzZR84995209 = WjEzNqKzZR51027633;     WjEzNqKzZR51027633 = WjEzNqKzZR41070035;     WjEzNqKzZR41070035 = WjEzNqKzZR12991313;     WjEzNqKzZR12991313 = WjEzNqKzZR2678565;     WjEzNqKzZR2678565 = WjEzNqKzZR82116817;     WjEzNqKzZR82116817 = WjEzNqKzZR63041425;     WjEzNqKzZR63041425 = WjEzNqKzZR63460238;     WjEzNqKzZR63460238 = WjEzNqKzZR55395823;     WjEzNqKzZR55395823 = WjEzNqKzZR71756170;     WjEzNqKzZR71756170 = WjEzNqKzZR62584865;     WjEzNqKzZR62584865 = WjEzNqKzZR81206309;     WjEzNqKzZR81206309 = WjEzNqKzZR31671714;     WjEzNqKzZR31671714 = WjEzNqKzZR11475399;     WjEzNqKzZR11475399 = WjEzNqKzZR37738073;     WjEzNqKzZR37738073 = WjEzNqKzZR23838041;     WjEzNqKzZR23838041 = WjEzNqKzZR36678359;     WjEzNqKzZR36678359 = WjEzNqKzZR34784853;     WjEzNqKzZR34784853 = WjEzNqKzZR54790862;     WjEzNqKzZR54790862 = WjEzNqKzZR63843532;     WjEzNqKzZR63843532 = WjEzNqKzZR19336902;     WjEzNqKzZR19336902 = WjEzNqKzZR79367962;     WjEzNqKzZR79367962 = WjEzNqKzZR46246222;     WjEzNqKzZR46246222 = WjEzNqKzZR78726228;     WjEzNqKzZR78726228 = WjEzNqKzZR63374505;     WjEzNqKzZR63374505 = WjEzNqKzZR87936086;     WjEzNqKzZR87936086 = WjEzNqKzZR34093618;     WjEzNqKzZR34093618 = WjEzNqKzZR18753914;     WjEzNqKzZR18753914 = WjEzNqKzZR7137449;     WjEzNqKzZR7137449 = WjEzNqKzZR75976755;     WjEzNqKzZR75976755 = WjEzNqKzZR66843734;     WjEzNqKzZR66843734 = WjEzNqKzZR91534209;     WjEzNqKzZR91534209 = WjEzNqKzZR28721296;     WjEzNqKzZR28721296 = WjEzNqKzZR32906067;     WjEzNqKzZR32906067 = WjEzNqKzZR55019195;     WjEzNqKzZR55019195 = WjEzNqKzZR8540337;     WjEzNqKzZR8540337 = WjEzNqKzZR3159810;     WjEzNqKzZR3159810 = WjEzNqKzZR55542712;     WjEzNqKzZR55542712 = WjEzNqKzZR24079931;     WjEzNqKzZR24079931 = WjEzNqKzZR43294195;     WjEzNqKzZR43294195 = WjEzNqKzZR68033651;     WjEzNqKzZR68033651 = WjEzNqKzZR24298660;     WjEzNqKzZR24298660 = WjEzNqKzZR30875080;     WjEzNqKzZR30875080 = WjEzNqKzZR48965688;     WjEzNqKzZR48965688 = WjEzNqKzZR68955277;     WjEzNqKzZR68955277 = WjEzNqKzZR98723326;     WjEzNqKzZR98723326 = WjEzNqKzZR91075286;     WjEzNqKzZR91075286 = WjEzNqKzZR88090904;     WjEzNqKzZR88090904 = WjEzNqKzZR878838;     WjEzNqKzZR878838 = WjEzNqKzZR32718998;     WjEzNqKzZR32718998 = WjEzNqKzZR15776739;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ajDhWGxdeP83817523() {     int IiaBLfFYRp28562906 = -412043408;    int IiaBLfFYRp7183138 = 70548682;    int IiaBLfFYRp44907525 = -624646422;    int IiaBLfFYRp41718788 = -173007183;    int IiaBLfFYRp79703380 = -187831410;    int IiaBLfFYRp23466640 = -715023591;    int IiaBLfFYRp54178450 = -393076544;    int IiaBLfFYRp29177891 = -758736376;    int IiaBLfFYRp46894613 = -415973204;    int IiaBLfFYRp3524748 = -641920369;    int IiaBLfFYRp95555267 = -586430820;    int IiaBLfFYRp37162889 = 79829807;    int IiaBLfFYRp31686651 = -284231975;    int IiaBLfFYRp29825954 = -458805453;    int IiaBLfFYRp18225162 = -852949036;    int IiaBLfFYRp35520868 = -769334177;    int IiaBLfFYRp21390683 = -86439310;    int IiaBLfFYRp50526565 = -568788186;    int IiaBLfFYRp11427001 = -288450958;    int IiaBLfFYRp45958637 = -178069045;    int IiaBLfFYRp90857142 = -163855720;    int IiaBLfFYRp62707740 = -277366225;    int IiaBLfFYRp96131119 = -813249747;    int IiaBLfFYRp8201617 = -133881678;    int IiaBLfFYRp84276399 = -156533229;    int IiaBLfFYRp95754750 = -745010539;    int IiaBLfFYRp28987240 = -284582;    int IiaBLfFYRp19117307 = -103712241;    int IiaBLfFYRp87404117 = -6736778;    int IiaBLfFYRp92765519 = 81276810;    int IiaBLfFYRp4003966 = -210947949;    int IiaBLfFYRp47548668 = -472689828;    int IiaBLfFYRp14579974 = -679228953;    int IiaBLfFYRp3834868 = -86151264;    int IiaBLfFYRp8471546 = -397284863;    int IiaBLfFYRp26299191 = -194164723;    int IiaBLfFYRp24522958 = -566746009;    int IiaBLfFYRp43974926 = -154880938;    int IiaBLfFYRp26497245 = -683868897;    int IiaBLfFYRp57804871 = -301105426;    int IiaBLfFYRp41360944 = -234125816;    int IiaBLfFYRp30449456 = -449335313;    int IiaBLfFYRp34449773 = -958248505;    int IiaBLfFYRp83245448 = -138460274;    int IiaBLfFYRp36565529 = -944789009;    int IiaBLfFYRp76033514 = -812110902;    int IiaBLfFYRp25174391 = -749780480;    int IiaBLfFYRp55477461 = -589336643;    int IiaBLfFYRp67107136 = -679016199;    int IiaBLfFYRp84831418 = -534085631;    int IiaBLfFYRp3322704 = -744262880;    int IiaBLfFYRp79204180 = -455779309;    int IiaBLfFYRp75555185 = -828021572;    int IiaBLfFYRp57691038 = -900959619;    int IiaBLfFYRp92960380 = -861996054;    int IiaBLfFYRp65855166 = -34677183;    int IiaBLfFYRp11052018 = -116201572;    int IiaBLfFYRp36705908 = -390764744;    int IiaBLfFYRp57442389 = 83526045;    int IiaBLfFYRp83948629 = -442820871;    int IiaBLfFYRp94479399 = -614739010;    int IiaBLfFYRp35061143 = -189364304;    int IiaBLfFYRp41773774 = -651999598;    int IiaBLfFYRp54129093 = -397250015;    int IiaBLfFYRp99520781 = -330972421;    int IiaBLfFYRp48006599 = -13740992;    int IiaBLfFYRp22582915 = -240941240;    int IiaBLfFYRp27851783 = -98080711;    int IiaBLfFYRp21354409 = 38479410;    int IiaBLfFYRp91925970 = -558784314;    int IiaBLfFYRp10997910 = -102588168;    int IiaBLfFYRp77415756 = -931558373;    int IiaBLfFYRp24029321 = -884919290;    int IiaBLfFYRp53622129 = -987345532;    int IiaBLfFYRp4597693 = -943943229;    int IiaBLfFYRp60407687 = -714520407;    int IiaBLfFYRp28257968 = -319117720;    int IiaBLfFYRp12885672 = -574789473;    int IiaBLfFYRp71636087 = -189092670;    int IiaBLfFYRp8242885 = -344422327;    int IiaBLfFYRp70580360 = -995230059;    int IiaBLfFYRp73509779 = -410947939;    int IiaBLfFYRp52010171 = -424696042;    int IiaBLfFYRp2572700 = -472651147;    int IiaBLfFYRp89442816 = -174460310;    int IiaBLfFYRp24799786 = -755168640;    int IiaBLfFYRp71993483 = -644668257;    int IiaBLfFYRp56888936 = -778269334;    int IiaBLfFYRp10874488 = -224155210;    int IiaBLfFYRp42616379 = -262607680;    int IiaBLfFYRp15247174 = 22036849;    int IiaBLfFYRp87817049 = -75981266;    int IiaBLfFYRp86532536 = -138406983;    int IiaBLfFYRp42548615 = -141048026;    int IiaBLfFYRp63325472 = -686366417;    int IiaBLfFYRp6299801 = 55238488;    int IiaBLfFYRp88675682 = -797335716;    int IiaBLfFYRp80320679 = -460998490;    int IiaBLfFYRp83724667 = -807487854;    int IiaBLfFYRp88558929 = -412043408;     IiaBLfFYRp28562906 = IiaBLfFYRp7183138;     IiaBLfFYRp7183138 = IiaBLfFYRp44907525;     IiaBLfFYRp44907525 = IiaBLfFYRp41718788;     IiaBLfFYRp41718788 = IiaBLfFYRp79703380;     IiaBLfFYRp79703380 = IiaBLfFYRp23466640;     IiaBLfFYRp23466640 = IiaBLfFYRp54178450;     IiaBLfFYRp54178450 = IiaBLfFYRp29177891;     IiaBLfFYRp29177891 = IiaBLfFYRp46894613;     IiaBLfFYRp46894613 = IiaBLfFYRp3524748;     IiaBLfFYRp3524748 = IiaBLfFYRp95555267;     IiaBLfFYRp95555267 = IiaBLfFYRp37162889;     IiaBLfFYRp37162889 = IiaBLfFYRp31686651;     IiaBLfFYRp31686651 = IiaBLfFYRp29825954;     IiaBLfFYRp29825954 = IiaBLfFYRp18225162;     IiaBLfFYRp18225162 = IiaBLfFYRp35520868;     IiaBLfFYRp35520868 = IiaBLfFYRp21390683;     IiaBLfFYRp21390683 = IiaBLfFYRp50526565;     IiaBLfFYRp50526565 = IiaBLfFYRp11427001;     IiaBLfFYRp11427001 = IiaBLfFYRp45958637;     IiaBLfFYRp45958637 = IiaBLfFYRp90857142;     IiaBLfFYRp90857142 = IiaBLfFYRp62707740;     IiaBLfFYRp62707740 = IiaBLfFYRp96131119;     IiaBLfFYRp96131119 = IiaBLfFYRp8201617;     IiaBLfFYRp8201617 = IiaBLfFYRp84276399;     IiaBLfFYRp84276399 = IiaBLfFYRp95754750;     IiaBLfFYRp95754750 = IiaBLfFYRp28987240;     IiaBLfFYRp28987240 = IiaBLfFYRp19117307;     IiaBLfFYRp19117307 = IiaBLfFYRp87404117;     IiaBLfFYRp87404117 = IiaBLfFYRp92765519;     IiaBLfFYRp92765519 = IiaBLfFYRp4003966;     IiaBLfFYRp4003966 = IiaBLfFYRp47548668;     IiaBLfFYRp47548668 = IiaBLfFYRp14579974;     IiaBLfFYRp14579974 = IiaBLfFYRp3834868;     IiaBLfFYRp3834868 = IiaBLfFYRp8471546;     IiaBLfFYRp8471546 = IiaBLfFYRp26299191;     IiaBLfFYRp26299191 = IiaBLfFYRp24522958;     IiaBLfFYRp24522958 = IiaBLfFYRp43974926;     IiaBLfFYRp43974926 = IiaBLfFYRp26497245;     IiaBLfFYRp26497245 = IiaBLfFYRp57804871;     IiaBLfFYRp57804871 = IiaBLfFYRp41360944;     IiaBLfFYRp41360944 = IiaBLfFYRp30449456;     IiaBLfFYRp30449456 = IiaBLfFYRp34449773;     IiaBLfFYRp34449773 = IiaBLfFYRp83245448;     IiaBLfFYRp83245448 = IiaBLfFYRp36565529;     IiaBLfFYRp36565529 = IiaBLfFYRp76033514;     IiaBLfFYRp76033514 = IiaBLfFYRp25174391;     IiaBLfFYRp25174391 = IiaBLfFYRp55477461;     IiaBLfFYRp55477461 = IiaBLfFYRp67107136;     IiaBLfFYRp67107136 = IiaBLfFYRp84831418;     IiaBLfFYRp84831418 = IiaBLfFYRp3322704;     IiaBLfFYRp3322704 = IiaBLfFYRp79204180;     IiaBLfFYRp79204180 = IiaBLfFYRp75555185;     IiaBLfFYRp75555185 = IiaBLfFYRp57691038;     IiaBLfFYRp57691038 = IiaBLfFYRp92960380;     IiaBLfFYRp92960380 = IiaBLfFYRp65855166;     IiaBLfFYRp65855166 = IiaBLfFYRp11052018;     IiaBLfFYRp11052018 = IiaBLfFYRp36705908;     IiaBLfFYRp36705908 = IiaBLfFYRp57442389;     IiaBLfFYRp57442389 = IiaBLfFYRp83948629;     IiaBLfFYRp83948629 = IiaBLfFYRp94479399;     IiaBLfFYRp94479399 = IiaBLfFYRp35061143;     IiaBLfFYRp35061143 = IiaBLfFYRp41773774;     IiaBLfFYRp41773774 = IiaBLfFYRp54129093;     IiaBLfFYRp54129093 = IiaBLfFYRp99520781;     IiaBLfFYRp99520781 = IiaBLfFYRp48006599;     IiaBLfFYRp48006599 = IiaBLfFYRp22582915;     IiaBLfFYRp22582915 = IiaBLfFYRp27851783;     IiaBLfFYRp27851783 = IiaBLfFYRp21354409;     IiaBLfFYRp21354409 = IiaBLfFYRp91925970;     IiaBLfFYRp91925970 = IiaBLfFYRp10997910;     IiaBLfFYRp10997910 = IiaBLfFYRp77415756;     IiaBLfFYRp77415756 = IiaBLfFYRp24029321;     IiaBLfFYRp24029321 = IiaBLfFYRp53622129;     IiaBLfFYRp53622129 = IiaBLfFYRp4597693;     IiaBLfFYRp4597693 = IiaBLfFYRp60407687;     IiaBLfFYRp60407687 = IiaBLfFYRp28257968;     IiaBLfFYRp28257968 = IiaBLfFYRp12885672;     IiaBLfFYRp12885672 = IiaBLfFYRp71636087;     IiaBLfFYRp71636087 = IiaBLfFYRp8242885;     IiaBLfFYRp8242885 = IiaBLfFYRp70580360;     IiaBLfFYRp70580360 = IiaBLfFYRp73509779;     IiaBLfFYRp73509779 = IiaBLfFYRp52010171;     IiaBLfFYRp52010171 = IiaBLfFYRp2572700;     IiaBLfFYRp2572700 = IiaBLfFYRp89442816;     IiaBLfFYRp89442816 = IiaBLfFYRp24799786;     IiaBLfFYRp24799786 = IiaBLfFYRp71993483;     IiaBLfFYRp71993483 = IiaBLfFYRp56888936;     IiaBLfFYRp56888936 = IiaBLfFYRp10874488;     IiaBLfFYRp10874488 = IiaBLfFYRp42616379;     IiaBLfFYRp42616379 = IiaBLfFYRp15247174;     IiaBLfFYRp15247174 = IiaBLfFYRp87817049;     IiaBLfFYRp87817049 = IiaBLfFYRp86532536;     IiaBLfFYRp86532536 = IiaBLfFYRp42548615;     IiaBLfFYRp42548615 = IiaBLfFYRp63325472;     IiaBLfFYRp63325472 = IiaBLfFYRp6299801;     IiaBLfFYRp6299801 = IiaBLfFYRp88675682;     IiaBLfFYRp88675682 = IiaBLfFYRp80320679;     IiaBLfFYRp80320679 = IiaBLfFYRp83724667;     IiaBLfFYRp83724667 = IiaBLfFYRp88558929;     IiaBLfFYRp88558929 = IiaBLfFYRp28562906;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JmQRHwNKGe2030866() {     int mdHdZpnuCa12007212 = -192709088;    int mdHdZpnuCa24055227 = 71166494;    int mdHdZpnuCa41316032 = -418171650;    int mdHdZpnuCa4511040 = -366807019;    int mdHdZpnuCa85824704 = 5301099;    int mdHdZpnuCa21971041 = -494095244;    int mdHdZpnuCa25217768 = -132987858;    int mdHdZpnuCa43525334 = -745894405;    int mdHdZpnuCa5424314 = -757432555;    int mdHdZpnuCa36070399 = -10296481;    int mdHdZpnuCa44971205 = -776546693;    int mdHdZpnuCa368288 = 92211561;    int mdHdZpnuCa49804427 = -75893216;    int mdHdZpnuCa9452846 = -247095810;    int mdHdZpnuCa79079625 = -946972018;    int mdHdZpnuCa21910030 = -557413502;    int mdHdZpnuCa15142568 = -939784337;    int mdHdZpnuCa31378012 = -816093063;    int mdHdZpnuCa73867142 = -287125829;    int mdHdZpnuCa35326079 = -707655038;    int mdHdZpnuCa46497530 = -853384751;    int mdHdZpnuCa74855058 = -268517995;    int mdHdZpnuCa99956827 = -853138944;    int mdHdZpnuCa42908175 = -205707074;    int mdHdZpnuCa25852357 = -716185987;    int mdHdZpnuCa91096899 = -625520254;    int mdHdZpnuCa78441980 = -728368924;    int mdHdZpnuCa58848932 = -173667124;    int mdHdZpnuCa23485212 = -969241920;    int mdHdZpnuCa8598861 = -431573041;    int mdHdZpnuCa22099280 = -948705150;    int mdHdZpnuCa10655096 = -387884908;    int mdHdZpnuCa58118860 = -300101130;    int mdHdZpnuCa45993379 = -777753434;    int mdHdZpnuCa87198275 = -594949598;    int mdHdZpnuCa38688375 = 17811372;    int mdHdZpnuCa93137136 = -445595189;    int mdHdZpnuCa84553757 = -963351458;    int mdHdZpnuCa75492323 = -868766695;    int mdHdZpnuCa7300007 = -53698948;    int mdHdZpnuCa75009868 = -905674789;    int mdHdZpnuCa80937242 = 76756365;    int mdHdZpnuCa58016842 = -409305748;    int mdHdZpnuCa75150388 = -994643045;    int mdHdZpnuCa28510026 = -672945962;    int mdHdZpnuCa15639511 = -831721825;    int mdHdZpnuCa1189821 = -882084709;    int mdHdZpnuCa75148056 = -588058361;    int mdHdZpnuCa97954248 = -164618281;    int mdHdZpnuCa20163902 = -741106086;    int mdHdZpnuCa76580151 = -965164303;    int mdHdZpnuCa34704975 = -514038621;    int mdHdZpnuCa89667644 = 60231112;    int mdHdZpnuCa82367797 = -619373007;    int mdHdZpnuCa55533335 = -110482177;    int mdHdZpnuCa37152153 = -924191094;    int mdHdZpnuCa24098399 = -75694562;    int mdHdZpnuCa98407856 = -112464576;    int mdHdZpnuCa78658682 = -650621032;    int mdHdZpnuCa94727804 = -369178647;    int mdHdZpnuCa43529060 = -765726320;    int mdHdZpnuCa66368835 = -959320734;    int mdHdZpnuCa20040122 = -776652485;    int mdHdZpnuCa96825452 = -225859515;    int mdHdZpnuCa13971120 = -61591332;    int mdHdZpnuCa34316110 = -288661785;    int mdHdZpnuCa42249428 = -607687309;    int mdHdZpnuCa3811049 = -298139782;    int mdHdZpnuCa22254570 = -652146212;    int mdHdZpnuCa40391250 = -864783390;    int mdHdZpnuCa28772893 = -11818313;    int mdHdZpnuCa30588810 = -976432880;    int mdHdZpnuCa55885689 = -947326369;    int mdHdZpnuCa66567136 = -133426881;    int mdHdZpnuCa60316211 = -801980249;    int mdHdZpnuCa65560288 = -830141117;    int mdHdZpnuCa16838216 = -859212248;    int mdHdZpnuCa24806439 = -858495900;    int mdHdZpnuCa14398149 = -532761113;    int mdHdZpnuCa10212847 = -884464162;    int mdHdZpnuCa89907078 = -743435546;    int mdHdZpnuCa3293924 = -40310563;    int mdHdZpnuCa60894684 = 90951156;    int mdHdZpnuCa3321310 = -128135834;    int mdHdZpnuCa32018710 = -466408739;    int mdHdZpnuCa87394305 = -334666529;    int mdHdZpnuCa20987451 = -348116020;    int mdHdZpnuCa75751063 = -680728124;    int mdHdZpnuCa90460043 = -567271257;    int mdHdZpnuCa50046122 = -670758505;    int mdHdZpnuCa14589976 = -906494067;    int mdHdZpnuCa94729279 = -233130614;    int mdHdZpnuCa5895076 = -212730426;    int mdHdZpnuCa80764518 = -399588049;    int mdHdZpnuCa63770946 = -287972629;    int mdHdZpnuCa8641033 = -946354056;    int mdHdZpnuCa60897120 = -146591150;    int mdHdZpnuCa61191389 = -83446234;    int mdHdZpnuCa61179269 = -833051713;    int mdHdZpnuCa94193915 = -192709088;     mdHdZpnuCa12007212 = mdHdZpnuCa24055227;     mdHdZpnuCa24055227 = mdHdZpnuCa41316032;     mdHdZpnuCa41316032 = mdHdZpnuCa4511040;     mdHdZpnuCa4511040 = mdHdZpnuCa85824704;     mdHdZpnuCa85824704 = mdHdZpnuCa21971041;     mdHdZpnuCa21971041 = mdHdZpnuCa25217768;     mdHdZpnuCa25217768 = mdHdZpnuCa43525334;     mdHdZpnuCa43525334 = mdHdZpnuCa5424314;     mdHdZpnuCa5424314 = mdHdZpnuCa36070399;     mdHdZpnuCa36070399 = mdHdZpnuCa44971205;     mdHdZpnuCa44971205 = mdHdZpnuCa368288;     mdHdZpnuCa368288 = mdHdZpnuCa49804427;     mdHdZpnuCa49804427 = mdHdZpnuCa9452846;     mdHdZpnuCa9452846 = mdHdZpnuCa79079625;     mdHdZpnuCa79079625 = mdHdZpnuCa21910030;     mdHdZpnuCa21910030 = mdHdZpnuCa15142568;     mdHdZpnuCa15142568 = mdHdZpnuCa31378012;     mdHdZpnuCa31378012 = mdHdZpnuCa73867142;     mdHdZpnuCa73867142 = mdHdZpnuCa35326079;     mdHdZpnuCa35326079 = mdHdZpnuCa46497530;     mdHdZpnuCa46497530 = mdHdZpnuCa74855058;     mdHdZpnuCa74855058 = mdHdZpnuCa99956827;     mdHdZpnuCa99956827 = mdHdZpnuCa42908175;     mdHdZpnuCa42908175 = mdHdZpnuCa25852357;     mdHdZpnuCa25852357 = mdHdZpnuCa91096899;     mdHdZpnuCa91096899 = mdHdZpnuCa78441980;     mdHdZpnuCa78441980 = mdHdZpnuCa58848932;     mdHdZpnuCa58848932 = mdHdZpnuCa23485212;     mdHdZpnuCa23485212 = mdHdZpnuCa8598861;     mdHdZpnuCa8598861 = mdHdZpnuCa22099280;     mdHdZpnuCa22099280 = mdHdZpnuCa10655096;     mdHdZpnuCa10655096 = mdHdZpnuCa58118860;     mdHdZpnuCa58118860 = mdHdZpnuCa45993379;     mdHdZpnuCa45993379 = mdHdZpnuCa87198275;     mdHdZpnuCa87198275 = mdHdZpnuCa38688375;     mdHdZpnuCa38688375 = mdHdZpnuCa93137136;     mdHdZpnuCa93137136 = mdHdZpnuCa84553757;     mdHdZpnuCa84553757 = mdHdZpnuCa75492323;     mdHdZpnuCa75492323 = mdHdZpnuCa7300007;     mdHdZpnuCa7300007 = mdHdZpnuCa75009868;     mdHdZpnuCa75009868 = mdHdZpnuCa80937242;     mdHdZpnuCa80937242 = mdHdZpnuCa58016842;     mdHdZpnuCa58016842 = mdHdZpnuCa75150388;     mdHdZpnuCa75150388 = mdHdZpnuCa28510026;     mdHdZpnuCa28510026 = mdHdZpnuCa15639511;     mdHdZpnuCa15639511 = mdHdZpnuCa1189821;     mdHdZpnuCa1189821 = mdHdZpnuCa75148056;     mdHdZpnuCa75148056 = mdHdZpnuCa97954248;     mdHdZpnuCa97954248 = mdHdZpnuCa20163902;     mdHdZpnuCa20163902 = mdHdZpnuCa76580151;     mdHdZpnuCa76580151 = mdHdZpnuCa34704975;     mdHdZpnuCa34704975 = mdHdZpnuCa89667644;     mdHdZpnuCa89667644 = mdHdZpnuCa82367797;     mdHdZpnuCa82367797 = mdHdZpnuCa55533335;     mdHdZpnuCa55533335 = mdHdZpnuCa37152153;     mdHdZpnuCa37152153 = mdHdZpnuCa24098399;     mdHdZpnuCa24098399 = mdHdZpnuCa98407856;     mdHdZpnuCa98407856 = mdHdZpnuCa78658682;     mdHdZpnuCa78658682 = mdHdZpnuCa94727804;     mdHdZpnuCa94727804 = mdHdZpnuCa43529060;     mdHdZpnuCa43529060 = mdHdZpnuCa66368835;     mdHdZpnuCa66368835 = mdHdZpnuCa20040122;     mdHdZpnuCa20040122 = mdHdZpnuCa96825452;     mdHdZpnuCa96825452 = mdHdZpnuCa13971120;     mdHdZpnuCa13971120 = mdHdZpnuCa34316110;     mdHdZpnuCa34316110 = mdHdZpnuCa42249428;     mdHdZpnuCa42249428 = mdHdZpnuCa3811049;     mdHdZpnuCa3811049 = mdHdZpnuCa22254570;     mdHdZpnuCa22254570 = mdHdZpnuCa40391250;     mdHdZpnuCa40391250 = mdHdZpnuCa28772893;     mdHdZpnuCa28772893 = mdHdZpnuCa30588810;     mdHdZpnuCa30588810 = mdHdZpnuCa55885689;     mdHdZpnuCa55885689 = mdHdZpnuCa66567136;     mdHdZpnuCa66567136 = mdHdZpnuCa60316211;     mdHdZpnuCa60316211 = mdHdZpnuCa65560288;     mdHdZpnuCa65560288 = mdHdZpnuCa16838216;     mdHdZpnuCa16838216 = mdHdZpnuCa24806439;     mdHdZpnuCa24806439 = mdHdZpnuCa14398149;     mdHdZpnuCa14398149 = mdHdZpnuCa10212847;     mdHdZpnuCa10212847 = mdHdZpnuCa89907078;     mdHdZpnuCa89907078 = mdHdZpnuCa3293924;     mdHdZpnuCa3293924 = mdHdZpnuCa60894684;     mdHdZpnuCa60894684 = mdHdZpnuCa3321310;     mdHdZpnuCa3321310 = mdHdZpnuCa32018710;     mdHdZpnuCa32018710 = mdHdZpnuCa87394305;     mdHdZpnuCa87394305 = mdHdZpnuCa20987451;     mdHdZpnuCa20987451 = mdHdZpnuCa75751063;     mdHdZpnuCa75751063 = mdHdZpnuCa90460043;     mdHdZpnuCa90460043 = mdHdZpnuCa50046122;     mdHdZpnuCa50046122 = mdHdZpnuCa14589976;     mdHdZpnuCa14589976 = mdHdZpnuCa94729279;     mdHdZpnuCa94729279 = mdHdZpnuCa5895076;     mdHdZpnuCa5895076 = mdHdZpnuCa80764518;     mdHdZpnuCa80764518 = mdHdZpnuCa63770946;     mdHdZpnuCa63770946 = mdHdZpnuCa8641033;     mdHdZpnuCa8641033 = mdHdZpnuCa60897120;     mdHdZpnuCa60897120 = mdHdZpnuCa61191389;     mdHdZpnuCa61191389 = mdHdZpnuCa61179269;     mdHdZpnuCa61179269 = mdHdZpnuCa94193915;     mdHdZpnuCa94193915 = mdHdZpnuCa12007212;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iFMxQGbCpf72156541() {     int rSYknxHsOf79454247 = -892753223;    int rSYknxHsOf19251418 = -636137208;    int rSYknxHsOf18488266 = -154787031;    int rSYknxHsOf98806945 = -376570877;    int rSYknxHsOf44947377 = -790595283;    int rSYknxHsOf44650076 = -700736582;    int rSYknxHsOf98978893 = -176016134;    int rSYknxHsOf30205500 = -886803577;    int rSYknxHsOf5863371 = 47602052;    int rSYknxHsOf28281576 = -456378512;    int rSYknxHsOf30749277 = -457117208;    int rSYknxHsOf80791829 = -866066981;    int rSYknxHsOf51912131 = -861557703;    int rSYknxHsOf98199134 = -946461535;    int rSYknxHsOf87438142 = -798366628;    int rSYknxHsOf65538086 = -405668805;    int rSYknxHsOf14873389 = -157544062;    int rSYknxHsOf59532551 = -37796210;    int rSYknxHsOf55820104 = -964424305;    int rSYknxHsOf83541368 = 59759128;    int rSYknxHsOf81176550 = -2317793;    int rSYknxHsOf76859200 = -94828568;    int rSYknxHsOf38308558 = -911928418;    int rSYknxHsOf45193565 = -384172381;    int rSYknxHsOf99056542 = -872847511;    int rSYknxHsOf97082414 = -411454310;    int rSYknxHsOf21625859 = -516119770;    int rSYknxHsOf44047333 = -846550101;    int rSYknxHsOf67321495 = -917633721;    int rSYknxHsOf79695515 = -676785790;    int rSYknxHsOf84828125 = -286901191;    int rSYknxHsOf60241672 = -386279353;    int rSYknxHsOf86368263 = -530529313;    int rSYknxHsOf17442807 = -485946729;    int rSYknxHsOf22404927 = -63530813;    int rSYknxHsOf78410842 = 61018417;    int rSYknxHsOf92664316 = 13722218;    int rSYknxHsOf85317340 = -761637891;    int rSYknxHsOf96723761 = -379884457;    int rSYknxHsOf85678238 = 53007293;    int rSYknxHsOf4544910 = -205755042;    int rSYknxHsOf39616695 = -235294974;    int rSYknxHsOf23617135 = -814114256;    int rSYknxHsOf69541427 = -152530510;    int rSYknxHsOf84484527 = -224974119;    int rSYknxHsOf4173274 = -61785209;    int rSYknxHsOf67547759 = -925376523;    int rSYknxHsOf52631751 = -112302157;    int rSYknxHsOf65702675 = -664186584;    int rSYknxHsOf91910562 = -905136957;    int rSYknxHsOf56761814 = -196709106;    int rSYknxHsOf37716414 = -18144193;    int rSYknxHsOf80432449 = 36366380;    int rSYknxHsOf74233008 = -672791382;    int rSYknxHsOf90979990 = -743936067;    int rSYknxHsOf2595047 = -697924655;    int rSYknxHsOf80942859 = -724208791;    int rSYknxHsOf73294701 = -770614651;    int rSYknxHsOf99750402 = -503723366;    int rSYknxHsOf47864962 = -279140974;    int rSYknxHsOf23024217 = -84616812;    int rSYknxHsOf54931561 = -329466033;    int rSYknxHsOf62884004 = -969169857;    int rSYknxHsOf26167855 = -275612158;    int rSYknxHsOf43453450 = -69477322;    int rSYknxHsOf70507605 = 29162145;    int rSYknxHsOf94423566 = -235537669;    int rSYknxHsOf34469324 = -275610974;    int rSYknxHsOf75794208 = -782930722;    int rSYknxHsOf9027300 = -759385046;    int rSYknxHsOf72873769 = -319391023;    int rSYknxHsOf29556049 = -395906172;    int rSYknxHsOf62808790 = -657911754;    int rSYknxHsOf70141865 = -917431598;    int rSYknxHsOf78996458 = -734485830;    int rSYknxHsOf41559855 = -767022820;    int rSYknxHsOf53242066 = -280714312;    int rSYknxHsOf68767130 = -659397908;    int rSYknxHsOf60709037 = -59198262;    int rSYknxHsOf94883269 = -711062302;    int rSYknxHsOf29534656 = -486077787;    int rSYknxHsOf68994108 = -303817614;    int rSYknxHsOf78344657 = -82363518;    int rSYknxHsOf75410933 = 87503236;    int rSYknxHsOf22933701 = -380076685;    int rSYknxHsOf47111711 = -168756998;    int rSYknxHsOf79809222 = -322645734;    int rSYknxHsOf12135255 = -857737932;    int rSYknxHsOf26462817 = -742010663;    int rSYknxHsOf19809880 = -365606158;    int rSYknxHsOf97467982 = -214772793;    int rSYknxHsOf19369616 = -215663131;    int rSYknxHsOf85566937 = -157914525;    int rSYknxHsOf48858799 = -743484;    int rSYknxHsOf62654022 = -862375896;    int rSYknxHsOf49613348 = -876289010;    int rSYknxHsOf76732690 = -266125117;    int rSYknxHsOf97449279 = -438502099;    int rSYknxHsOf26087977 = 16946812;    int rSYknxHsOf13976922 = -892753223;     rSYknxHsOf79454247 = rSYknxHsOf19251418;     rSYknxHsOf19251418 = rSYknxHsOf18488266;     rSYknxHsOf18488266 = rSYknxHsOf98806945;     rSYknxHsOf98806945 = rSYknxHsOf44947377;     rSYknxHsOf44947377 = rSYknxHsOf44650076;     rSYknxHsOf44650076 = rSYknxHsOf98978893;     rSYknxHsOf98978893 = rSYknxHsOf30205500;     rSYknxHsOf30205500 = rSYknxHsOf5863371;     rSYknxHsOf5863371 = rSYknxHsOf28281576;     rSYknxHsOf28281576 = rSYknxHsOf30749277;     rSYknxHsOf30749277 = rSYknxHsOf80791829;     rSYknxHsOf80791829 = rSYknxHsOf51912131;     rSYknxHsOf51912131 = rSYknxHsOf98199134;     rSYknxHsOf98199134 = rSYknxHsOf87438142;     rSYknxHsOf87438142 = rSYknxHsOf65538086;     rSYknxHsOf65538086 = rSYknxHsOf14873389;     rSYknxHsOf14873389 = rSYknxHsOf59532551;     rSYknxHsOf59532551 = rSYknxHsOf55820104;     rSYknxHsOf55820104 = rSYknxHsOf83541368;     rSYknxHsOf83541368 = rSYknxHsOf81176550;     rSYknxHsOf81176550 = rSYknxHsOf76859200;     rSYknxHsOf76859200 = rSYknxHsOf38308558;     rSYknxHsOf38308558 = rSYknxHsOf45193565;     rSYknxHsOf45193565 = rSYknxHsOf99056542;     rSYknxHsOf99056542 = rSYknxHsOf97082414;     rSYknxHsOf97082414 = rSYknxHsOf21625859;     rSYknxHsOf21625859 = rSYknxHsOf44047333;     rSYknxHsOf44047333 = rSYknxHsOf67321495;     rSYknxHsOf67321495 = rSYknxHsOf79695515;     rSYknxHsOf79695515 = rSYknxHsOf84828125;     rSYknxHsOf84828125 = rSYknxHsOf60241672;     rSYknxHsOf60241672 = rSYknxHsOf86368263;     rSYknxHsOf86368263 = rSYknxHsOf17442807;     rSYknxHsOf17442807 = rSYknxHsOf22404927;     rSYknxHsOf22404927 = rSYknxHsOf78410842;     rSYknxHsOf78410842 = rSYknxHsOf92664316;     rSYknxHsOf92664316 = rSYknxHsOf85317340;     rSYknxHsOf85317340 = rSYknxHsOf96723761;     rSYknxHsOf96723761 = rSYknxHsOf85678238;     rSYknxHsOf85678238 = rSYknxHsOf4544910;     rSYknxHsOf4544910 = rSYknxHsOf39616695;     rSYknxHsOf39616695 = rSYknxHsOf23617135;     rSYknxHsOf23617135 = rSYknxHsOf69541427;     rSYknxHsOf69541427 = rSYknxHsOf84484527;     rSYknxHsOf84484527 = rSYknxHsOf4173274;     rSYknxHsOf4173274 = rSYknxHsOf67547759;     rSYknxHsOf67547759 = rSYknxHsOf52631751;     rSYknxHsOf52631751 = rSYknxHsOf65702675;     rSYknxHsOf65702675 = rSYknxHsOf91910562;     rSYknxHsOf91910562 = rSYknxHsOf56761814;     rSYknxHsOf56761814 = rSYknxHsOf37716414;     rSYknxHsOf37716414 = rSYknxHsOf80432449;     rSYknxHsOf80432449 = rSYknxHsOf74233008;     rSYknxHsOf74233008 = rSYknxHsOf90979990;     rSYknxHsOf90979990 = rSYknxHsOf2595047;     rSYknxHsOf2595047 = rSYknxHsOf80942859;     rSYknxHsOf80942859 = rSYknxHsOf73294701;     rSYknxHsOf73294701 = rSYknxHsOf99750402;     rSYknxHsOf99750402 = rSYknxHsOf47864962;     rSYknxHsOf47864962 = rSYknxHsOf23024217;     rSYknxHsOf23024217 = rSYknxHsOf54931561;     rSYknxHsOf54931561 = rSYknxHsOf62884004;     rSYknxHsOf62884004 = rSYknxHsOf26167855;     rSYknxHsOf26167855 = rSYknxHsOf43453450;     rSYknxHsOf43453450 = rSYknxHsOf70507605;     rSYknxHsOf70507605 = rSYknxHsOf94423566;     rSYknxHsOf94423566 = rSYknxHsOf34469324;     rSYknxHsOf34469324 = rSYknxHsOf75794208;     rSYknxHsOf75794208 = rSYknxHsOf9027300;     rSYknxHsOf9027300 = rSYknxHsOf72873769;     rSYknxHsOf72873769 = rSYknxHsOf29556049;     rSYknxHsOf29556049 = rSYknxHsOf62808790;     rSYknxHsOf62808790 = rSYknxHsOf70141865;     rSYknxHsOf70141865 = rSYknxHsOf78996458;     rSYknxHsOf78996458 = rSYknxHsOf41559855;     rSYknxHsOf41559855 = rSYknxHsOf53242066;     rSYknxHsOf53242066 = rSYknxHsOf68767130;     rSYknxHsOf68767130 = rSYknxHsOf60709037;     rSYknxHsOf60709037 = rSYknxHsOf94883269;     rSYknxHsOf94883269 = rSYknxHsOf29534656;     rSYknxHsOf29534656 = rSYknxHsOf68994108;     rSYknxHsOf68994108 = rSYknxHsOf78344657;     rSYknxHsOf78344657 = rSYknxHsOf75410933;     rSYknxHsOf75410933 = rSYknxHsOf22933701;     rSYknxHsOf22933701 = rSYknxHsOf47111711;     rSYknxHsOf47111711 = rSYknxHsOf79809222;     rSYknxHsOf79809222 = rSYknxHsOf12135255;     rSYknxHsOf12135255 = rSYknxHsOf26462817;     rSYknxHsOf26462817 = rSYknxHsOf19809880;     rSYknxHsOf19809880 = rSYknxHsOf97467982;     rSYknxHsOf97467982 = rSYknxHsOf19369616;     rSYknxHsOf19369616 = rSYknxHsOf85566937;     rSYknxHsOf85566937 = rSYknxHsOf48858799;     rSYknxHsOf48858799 = rSYknxHsOf62654022;     rSYknxHsOf62654022 = rSYknxHsOf49613348;     rSYknxHsOf49613348 = rSYknxHsOf76732690;     rSYknxHsOf76732690 = rSYknxHsOf97449279;     rSYknxHsOf97449279 = rSYknxHsOf26087977;     rSYknxHsOf26087977 = rSYknxHsOf13976922;     rSYknxHsOf13976922 = rSYknxHsOf79454247;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zDOPfzqeAK42612414() {     int VSIoYBusIo92240414 = 72206086;    int VSIoYBusIo6022306 = -806471222;    int VSIoYBusIo36241066 = -563146413;    int VSIoYBusIo6747772 = -200773153;    int VSIoYBusIo38458482 = -182411748;    int VSIoYBusIo87960146 = -477659895;    int VSIoYBusIo20186652 = -721688204;    int VSIoYBusIo56924612 = -231321834;    int VSIoYBusIo1268182 = 85843960;    int VSIoYBusIo34500281 = -260466145;    int VSIoYBusIo58236659 = -571803223;    int VSIoYBusIo12742335 = -273399797;    int VSIoYBusIo22055434 = -765340360;    int VSIoYBusIo47823214 = -178876733;    int VSIoYBusIo10744696 = -705352459;    int VSIoYBusIo93963155 = -956560194;    int VSIoYBusIo70625206 = -921318529;    int VSIoYBusIo43091036 = -693006513;    int VSIoYBusIo56980734 = -873893500;    int VSIoYBusIo86195864 = -539485011;    int VSIoYBusIo61350607 = -493634057;    int VSIoYBusIo5907020 = -264686916;    int VSIoYBusIo77068853 = -842932312;    int VSIoYBusIo89700694 = -744517393;    int VSIoYBusIo4950803 = 16710565;    int VSIoYBusIo78401060 = -101885509;    int VSIoYBusIo33041396 = -49826051;    int VSIoYBusIo11400258 = 79651794;    int VSIoYBusIo83938548 = -31850961;    int VSIoYBusIo32446631 = -306671945;    int VSIoYBusIo29264122 = -563317941;    int VSIoYBusIo63560496 = -364999032;    int VSIoYBusIo32414212 = -406384097;    int VSIoYBusIo94519179 = -390700947;    int VSIoYBusIo5465459 = -673562694;    int VSIoYBusIo73634957 = -827544687;    int VSIoYBusIo29428377 = -876187132;    int VSIoYBusIo43021363 = -681257983;    int VSIoYBusIo2499148 = -221735034;    int VSIoYBusIo11942968 = -891409553;    int VSIoYBusIo65976217 = -581229036;    int VSIoYBusIo47319151 = -133606308;    int VSIoYBusIo58500605 = -837547701;    int VSIoYBusIo14169966 = -493702753;    int VSIoYBusIo64493017 = -117744079;    int VSIoYBusIo77801402 = -135103649;    int VSIoYBusIo45129779 = -322891576;    int VSIoYBusIo82071717 = -267654937;    int VSIoYBusIo9766725 = -827788558;    int VSIoYBusIo38860983 = -416173420;    int VSIoYBusIo75089308 = -449593414;    int VSIoYBusIo65892961 = -489329531;    int VSIoYBusIo14917600 = -139636903;    int VSIoYBusIo18932734 = -262243124;    int VSIoYBusIo81261806 = -738380553;    int VSIoYBusIo86333395 = -663106999;    int VSIoYBusIo28953452 = -963538910;    int VSIoYBusIo46540371 = -818629021;    int VSIoYBusIo1796970 = -117483719;    int VSIoYBusIo60057421 = 19473760;    int VSIoYBusIo54918751 = -327833845;    int VSIoYBusIo8786395 = -701339999;    int VSIoYBusIo72986064 = -99470874;    int VSIoYBusIo68821550 = -607484096;    int VSIoYBusIo5236160 = -697148205;    int VSIoYBusIo94676163 = -106804192;    int VSIoYBusIo80328122 = -867015700;    int VSIoYBusIo27536254 = -274639413;    int VSIoYBusIo42357755 = -505314040;    int VSIoYBusIo37109738 = -877807772;    int VSIoYBusIo64534778 = 19626938;    int VSIoYBusIo27603844 = -140060547;    int VSIoYBusIo40591889 = -371271479;    int VSIoYBusIo45037766 = -982483948;    int VSIoYBusIo20219647 = -958255975;    int VSIoYBusIo14031456 = -260027749;    int VSIoYBusIo47406415 = -427139215;    int VSIoYBusIo62898888 = -249229560;    int VSIoYBusIo25207677 = -526773314;    int VSIoYBusIo27149400 = -848185787;    int VSIoYBusIo33271281 = -778993934;    int VSIoYBusIo50969678 = -782171115;    int VSIoYBusIo1633534 = -92559648;    int VSIoYBusIo45077565 = -615677541;    int VSIoYBusIo57357323 = -857078531;    int VSIoYBusIo63371160 = 26011590;    int VSIoYBusIo48642896 = -125362130;    int VSIoYBusIo13481478 = -44140974;    int VSIoYBusIo13257373 = -652320394;    int VSIoYBusIo19132064 = 89544305;    int VSIoYBusIo44681505 = -864005778;    int VSIoYBusIo82888006 = 42441888;    int VSIoYBusIo41224394 = -463774264;    int VSIoYBusIo42441726 = -141208795;    int VSIoYBusIo57024217 = -463575708;    int VSIoYBusIo57189823 = -879889037;    int VSIoYBusIo74333087 = 65864565;    int VSIoYBusIo89679054 = -130063606;    int VSIoYBusIo8933806 = -796554548;    int VSIoYBusIo69816853 = 72206086;     VSIoYBusIo92240414 = VSIoYBusIo6022306;     VSIoYBusIo6022306 = VSIoYBusIo36241066;     VSIoYBusIo36241066 = VSIoYBusIo6747772;     VSIoYBusIo6747772 = VSIoYBusIo38458482;     VSIoYBusIo38458482 = VSIoYBusIo87960146;     VSIoYBusIo87960146 = VSIoYBusIo20186652;     VSIoYBusIo20186652 = VSIoYBusIo56924612;     VSIoYBusIo56924612 = VSIoYBusIo1268182;     VSIoYBusIo1268182 = VSIoYBusIo34500281;     VSIoYBusIo34500281 = VSIoYBusIo58236659;     VSIoYBusIo58236659 = VSIoYBusIo12742335;     VSIoYBusIo12742335 = VSIoYBusIo22055434;     VSIoYBusIo22055434 = VSIoYBusIo47823214;     VSIoYBusIo47823214 = VSIoYBusIo10744696;     VSIoYBusIo10744696 = VSIoYBusIo93963155;     VSIoYBusIo93963155 = VSIoYBusIo70625206;     VSIoYBusIo70625206 = VSIoYBusIo43091036;     VSIoYBusIo43091036 = VSIoYBusIo56980734;     VSIoYBusIo56980734 = VSIoYBusIo86195864;     VSIoYBusIo86195864 = VSIoYBusIo61350607;     VSIoYBusIo61350607 = VSIoYBusIo5907020;     VSIoYBusIo5907020 = VSIoYBusIo77068853;     VSIoYBusIo77068853 = VSIoYBusIo89700694;     VSIoYBusIo89700694 = VSIoYBusIo4950803;     VSIoYBusIo4950803 = VSIoYBusIo78401060;     VSIoYBusIo78401060 = VSIoYBusIo33041396;     VSIoYBusIo33041396 = VSIoYBusIo11400258;     VSIoYBusIo11400258 = VSIoYBusIo83938548;     VSIoYBusIo83938548 = VSIoYBusIo32446631;     VSIoYBusIo32446631 = VSIoYBusIo29264122;     VSIoYBusIo29264122 = VSIoYBusIo63560496;     VSIoYBusIo63560496 = VSIoYBusIo32414212;     VSIoYBusIo32414212 = VSIoYBusIo94519179;     VSIoYBusIo94519179 = VSIoYBusIo5465459;     VSIoYBusIo5465459 = VSIoYBusIo73634957;     VSIoYBusIo73634957 = VSIoYBusIo29428377;     VSIoYBusIo29428377 = VSIoYBusIo43021363;     VSIoYBusIo43021363 = VSIoYBusIo2499148;     VSIoYBusIo2499148 = VSIoYBusIo11942968;     VSIoYBusIo11942968 = VSIoYBusIo65976217;     VSIoYBusIo65976217 = VSIoYBusIo47319151;     VSIoYBusIo47319151 = VSIoYBusIo58500605;     VSIoYBusIo58500605 = VSIoYBusIo14169966;     VSIoYBusIo14169966 = VSIoYBusIo64493017;     VSIoYBusIo64493017 = VSIoYBusIo77801402;     VSIoYBusIo77801402 = VSIoYBusIo45129779;     VSIoYBusIo45129779 = VSIoYBusIo82071717;     VSIoYBusIo82071717 = VSIoYBusIo9766725;     VSIoYBusIo9766725 = VSIoYBusIo38860983;     VSIoYBusIo38860983 = VSIoYBusIo75089308;     VSIoYBusIo75089308 = VSIoYBusIo65892961;     VSIoYBusIo65892961 = VSIoYBusIo14917600;     VSIoYBusIo14917600 = VSIoYBusIo18932734;     VSIoYBusIo18932734 = VSIoYBusIo81261806;     VSIoYBusIo81261806 = VSIoYBusIo86333395;     VSIoYBusIo86333395 = VSIoYBusIo28953452;     VSIoYBusIo28953452 = VSIoYBusIo46540371;     VSIoYBusIo46540371 = VSIoYBusIo1796970;     VSIoYBusIo1796970 = VSIoYBusIo60057421;     VSIoYBusIo60057421 = VSIoYBusIo54918751;     VSIoYBusIo54918751 = VSIoYBusIo8786395;     VSIoYBusIo8786395 = VSIoYBusIo72986064;     VSIoYBusIo72986064 = VSIoYBusIo68821550;     VSIoYBusIo68821550 = VSIoYBusIo5236160;     VSIoYBusIo5236160 = VSIoYBusIo94676163;     VSIoYBusIo94676163 = VSIoYBusIo80328122;     VSIoYBusIo80328122 = VSIoYBusIo27536254;     VSIoYBusIo27536254 = VSIoYBusIo42357755;     VSIoYBusIo42357755 = VSIoYBusIo37109738;     VSIoYBusIo37109738 = VSIoYBusIo64534778;     VSIoYBusIo64534778 = VSIoYBusIo27603844;     VSIoYBusIo27603844 = VSIoYBusIo40591889;     VSIoYBusIo40591889 = VSIoYBusIo45037766;     VSIoYBusIo45037766 = VSIoYBusIo20219647;     VSIoYBusIo20219647 = VSIoYBusIo14031456;     VSIoYBusIo14031456 = VSIoYBusIo47406415;     VSIoYBusIo47406415 = VSIoYBusIo62898888;     VSIoYBusIo62898888 = VSIoYBusIo25207677;     VSIoYBusIo25207677 = VSIoYBusIo27149400;     VSIoYBusIo27149400 = VSIoYBusIo33271281;     VSIoYBusIo33271281 = VSIoYBusIo50969678;     VSIoYBusIo50969678 = VSIoYBusIo1633534;     VSIoYBusIo1633534 = VSIoYBusIo45077565;     VSIoYBusIo45077565 = VSIoYBusIo57357323;     VSIoYBusIo57357323 = VSIoYBusIo63371160;     VSIoYBusIo63371160 = VSIoYBusIo48642896;     VSIoYBusIo48642896 = VSIoYBusIo13481478;     VSIoYBusIo13481478 = VSIoYBusIo13257373;     VSIoYBusIo13257373 = VSIoYBusIo19132064;     VSIoYBusIo19132064 = VSIoYBusIo44681505;     VSIoYBusIo44681505 = VSIoYBusIo82888006;     VSIoYBusIo82888006 = VSIoYBusIo41224394;     VSIoYBusIo41224394 = VSIoYBusIo42441726;     VSIoYBusIo42441726 = VSIoYBusIo57024217;     VSIoYBusIo57024217 = VSIoYBusIo57189823;     VSIoYBusIo57189823 = VSIoYBusIo74333087;     VSIoYBusIo74333087 = VSIoYBusIo89679054;     VSIoYBusIo89679054 = VSIoYBusIo8933806;     VSIoYBusIo8933806 = VSIoYBusIo69816853;     VSIoYBusIo69816853 = VSIoYBusIo92240414;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OYedyrVCHz60825755() {     int nytSxLDGHr75684720 = -808459595;    int nytSxLDGHr22894395 = -805853409;    int nytSxLDGHr32649573 = -356671641;    int nytSxLDGHr69540023 = -394572989;    int nytSxLDGHr44579805 = 10720761;    int nytSxLDGHr86464547 = -256731548;    int nytSxLDGHr91225969 = -461599518;    int nytSxLDGHr71272055 = -218479863;    int nytSxLDGHr59797882 = -255615391;    int nytSxLDGHr67045932 = -728842257;    int nytSxLDGHr7652598 = -761919096;    int nytSxLDGHr75947733 = -261018043;    int nytSxLDGHr40173209 = -557001601;    int nytSxLDGHr27450106 = 32832910;    int nytSxLDGHr71599159 = -799375441;    int nytSxLDGHr80352317 = -744639519;    int nytSxLDGHr64377091 = -674663556;    int nytSxLDGHr23942483 = -940311389;    int nytSxLDGHr19420877 = -872568371;    int nytSxLDGHr75563307 = 30928997;    int nytSxLDGHr16990995 = -83163088;    int nytSxLDGHr18054338 = -255838686;    int nytSxLDGHr80894561 = -882821510;    int nytSxLDGHr24407253 = -816342789;    int nytSxLDGHr46526760 = -542942194;    int nytSxLDGHr73743209 = 17604775;    int nytSxLDGHr82496135 = -777910393;    int nytSxLDGHr51131883 = 9696910;    int nytSxLDGHr20019642 = -994356103;    int nytSxLDGHr48279972 = -819521796;    int nytSxLDGHr47359435 = -201075142;    int nytSxLDGHr26666923 = -280194112;    int nytSxLDGHr75953099 = -27256275;    int nytSxLDGHr36677690 = 17696883;    int nytSxLDGHr84192188 = -871227429;    int nytSxLDGHr86024141 = -615568593;    int nytSxLDGHr98042555 = -755036312;    int nytSxLDGHr83600195 = -389728503;    int nytSxLDGHr51494225 = -406632832;    int nytSxLDGHr61438103 = -644003075;    int nytSxLDGHr99625142 = -152778009;    int nytSxLDGHr97806937 = -707514629;    int nytSxLDGHr82067674 = -288604944;    int nytSxLDGHr6074906 = -249885524;    int nytSxLDGHr56437514 = -945901032;    int nytSxLDGHr17407399 = -154714572;    int nytSxLDGHr21145210 = -455195805;    int nytSxLDGHr1742314 = -266376655;    int nytSxLDGHr40613837 = -313390641;    int nytSxLDGHr74193466 = -623193875;    int nytSxLDGHr48346756 = -670494836;    int nytSxLDGHr21393756 = -547588843;    int nytSxLDGHr29030058 = -351384219;    int nytSxLDGHr43609493 = 19343489;    int nytSxLDGHr43834761 = 13133324;    int nytSxLDGHr57630382 = -452620910;    int nytSxLDGHr41999834 = -923031900;    int nytSxLDGHr8242320 = -540328853;    int nytSxLDGHr23013263 = -851630796;    int nytSxLDGHr70836596 = 93115985;    int nytSxLDGHr3968412 = -478821156;    int nytSxLDGHr40094087 = -371296428;    int nytSxLDGHr51252413 = -224123761;    int nytSxLDGHr11517910 = -436093595;    int nytSxLDGHr19686498 = -427767116;    int nytSxLDGHr80985674 = -381724985;    int nytSxLDGHr99994634 = -133761769;    int nytSxLDGHr3495520 = -474698484;    int nytSxLDGHr43257917 = -95939661;    int nytSxLDGHr85575017 = -83806849;    int nytSxLDGHr82309761 = -989603207;    int nytSxLDGHr80776896 = -184935054;    int nytSxLDGHr72448257 = -433678557;    int nytSxLDGHr57982773 = -128565297;    int nytSxLDGHr75938165 = -816292995;    int nytSxLDGHr19184057 = -375648459;    int nytSxLDGHr35986664 = -967233742;    int nytSxLDGHr74819655 = -532935986;    int nytSxLDGHr67969738 = -870441757;    int nytSxLDGHr29119362 = -288227622;    int nytSxLDGHr52598000 = -527199420;    int nytSxLDGHr80753822 = -411533738;    int nytSxLDGHr10518047 = -676912450;    int nytSxLDGHr45826175 = -271162228;    int nytSxLDGHr99933216 = -49026960;    int nytSxLDGHr25965680 = -653486299;    int nytSxLDGHr97636864 = -928809893;    int nytSxLDGHr32343606 = 53400236;    int nytSxLDGHr92842929 = -995436441;    int nytSxLDGHr26561807 = -318606520;    int nytSxLDGHr44024308 = -692536694;    int nytSxLDGHr89800236 = -114707460;    int nytSxLDGHr60586932 = -538097707;    int nytSxLDGHr80657629 = -399748817;    int nytSxLDGHr57469691 = -65181920;    int nytSxLDGHr59531055 = -781481581;    int nytSxLDGHr46554525 = -383390869;    int nytSxLDGHr70549764 = -852511349;    int nytSxLDGHr86388408 = -822118408;    int nytSxLDGHr75451839 = -808459595;     nytSxLDGHr75684720 = nytSxLDGHr22894395;     nytSxLDGHr22894395 = nytSxLDGHr32649573;     nytSxLDGHr32649573 = nytSxLDGHr69540023;     nytSxLDGHr69540023 = nytSxLDGHr44579805;     nytSxLDGHr44579805 = nytSxLDGHr86464547;     nytSxLDGHr86464547 = nytSxLDGHr91225969;     nytSxLDGHr91225969 = nytSxLDGHr71272055;     nytSxLDGHr71272055 = nytSxLDGHr59797882;     nytSxLDGHr59797882 = nytSxLDGHr67045932;     nytSxLDGHr67045932 = nytSxLDGHr7652598;     nytSxLDGHr7652598 = nytSxLDGHr75947733;     nytSxLDGHr75947733 = nytSxLDGHr40173209;     nytSxLDGHr40173209 = nytSxLDGHr27450106;     nytSxLDGHr27450106 = nytSxLDGHr71599159;     nytSxLDGHr71599159 = nytSxLDGHr80352317;     nytSxLDGHr80352317 = nytSxLDGHr64377091;     nytSxLDGHr64377091 = nytSxLDGHr23942483;     nytSxLDGHr23942483 = nytSxLDGHr19420877;     nytSxLDGHr19420877 = nytSxLDGHr75563307;     nytSxLDGHr75563307 = nytSxLDGHr16990995;     nytSxLDGHr16990995 = nytSxLDGHr18054338;     nytSxLDGHr18054338 = nytSxLDGHr80894561;     nytSxLDGHr80894561 = nytSxLDGHr24407253;     nytSxLDGHr24407253 = nytSxLDGHr46526760;     nytSxLDGHr46526760 = nytSxLDGHr73743209;     nytSxLDGHr73743209 = nytSxLDGHr82496135;     nytSxLDGHr82496135 = nytSxLDGHr51131883;     nytSxLDGHr51131883 = nytSxLDGHr20019642;     nytSxLDGHr20019642 = nytSxLDGHr48279972;     nytSxLDGHr48279972 = nytSxLDGHr47359435;     nytSxLDGHr47359435 = nytSxLDGHr26666923;     nytSxLDGHr26666923 = nytSxLDGHr75953099;     nytSxLDGHr75953099 = nytSxLDGHr36677690;     nytSxLDGHr36677690 = nytSxLDGHr84192188;     nytSxLDGHr84192188 = nytSxLDGHr86024141;     nytSxLDGHr86024141 = nytSxLDGHr98042555;     nytSxLDGHr98042555 = nytSxLDGHr83600195;     nytSxLDGHr83600195 = nytSxLDGHr51494225;     nytSxLDGHr51494225 = nytSxLDGHr61438103;     nytSxLDGHr61438103 = nytSxLDGHr99625142;     nytSxLDGHr99625142 = nytSxLDGHr97806937;     nytSxLDGHr97806937 = nytSxLDGHr82067674;     nytSxLDGHr82067674 = nytSxLDGHr6074906;     nytSxLDGHr6074906 = nytSxLDGHr56437514;     nytSxLDGHr56437514 = nytSxLDGHr17407399;     nytSxLDGHr17407399 = nytSxLDGHr21145210;     nytSxLDGHr21145210 = nytSxLDGHr1742314;     nytSxLDGHr1742314 = nytSxLDGHr40613837;     nytSxLDGHr40613837 = nytSxLDGHr74193466;     nytSxLDGHr74193466 = nytSxLDGHr48346756;     nytSxLDGHr48346756 = nytSxLDGHr21393756;     nytSxLDGHr21393756 = nytSxLDGHr29030058;     nytSxLDGHr29030058 = nytSxLDGHr43609493;     nytSxLDGHr43609493 = nytSxLDGHr43834761;     nytSxLDGHr43834761 = nytSxLDGHr57630382;     nytSxLDGHr57630382 = nytSxLDGHr41999834;     nytSxLDGHr41999834 = nytSxLDGHr8242320;     nytSxLDGHr8242320 = nytSxLDGHr23013263;     nytSxLDGHr23013263 = nytSxLDGHr70836596;     nytSxLDGHr70836596 = nytSxLDGHr3968412;     nytSxLDGHr3968412 = nytSxLDGHr40094087;     nytSxLDGHr40094087 = nytSxLDGHr51252413;     nytSxLDGHr51252413 = nytSxLDGHr11517910;     nytSxLDGHr11517910 = nytSxLDGHr19686498;     nytSxLDGHr19686498 = nytSxLDGHr80985674;     nytSxLDGHr80985674 = nytSxLDGHr99994634;     nytSxLDGHr99994634 = nytSxLDGHr3495520;     nytSxLDGHr3495520 = nytSxLDGHr43257917;     nytSxLDGHr43257917 = nytSxLDGHr85575017;     nytSxLDGHr85575017 = nytSxLDGHr82309761;     nytSxLDGHr82309761 = nytSxLDGHr80776896;     nytSxLDGHr80776896 = nytSxLDGHr72448257;     nytSxLDGHr72448257 = nytSxLDGHr57982773;     nytSxLDGHr57982773 = nytSxLDGHr75938165;     nytSxLDGHr75938165 = nytSxLDGHr19184057;     nytSxLDGHr19184057 = nytSxLDGHr35986664;     nytSxLDGHr35986664 = nytSxLDGHr74819655;     nytSxLDGHr74819655 = nytSxLDGHr67969738;     nytSxLDGHr67969738 = nytSxLDGHr29119362;     nytSxLDGHr29119362 = nytSxLDGHr52598000;     nytSxLDGHr52598000 = nytSxLDGHr80753822;     nytSxLDGHr80753822 = nytSxLDGHr10518047;     nytSxLDGHr10518047 = nytSxLDGHr45826175;     nytSxLDGHr45826175 = nytSxLDGHr99933216;     nytSxLDGHr99933216 = nytSxLDGHr25965680;     nytSxLDGHr25965680 = nytSxLDGHr97636864;     nytSxLDGHr97636864 = nytSxLDGHr32343606;     nytSxLDGHr32343606 = nytSxLDGHr92842929;     nytSxLDGHr92842929 = nytSxLDGHr26561807;     nytSxLDGHr26561807 = nytSxLDGHr44024308;     nytSxLDGHr44024308 = nytSxLDGHr89800236;     nytSxLDGHr89800236 = nytSxLDGHr60586932;     nytSxLDGHr60586932 = nytSxLDGHr80657629;     nytSxLDGHr80657629 = nytSxLDGHr57469691;     nytSxLDGHr57469691 = nytSxLDGHr59531055;     nytSxLDGHr59531055 = nytSxLDGHr46554525;     nytSxLDGHr46554525 = nytSxLDGHr70549764;     nytSxLDGHr70549764 = nytSxLDGHr86388408;     nytSxLDGHr86388408 = nytSxLDGHr75451839;     nytSxLDGHr75451839 = nytSxLDGHr75684720;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WMfiWvTHKS31281629() {     int hQoZVFSQPi88470887 = -943500287;    int hQoZVFSQPi9665283 = -976187423;    int hQoZVFSQPi50402372 = -765031023;    int hQoZVFSQPi77480848 = -218775266;    int hQoZVFSQPi38090910 = -481095704;    int hQoZVFSQPi29774618 = -33654861;    int hQoZVFSQPi12433728 = 92728412;    int hQoZVFSQPi97991167 = -662998120;    int hQoZVFSQPi55202692 = -217373483;    int hQoZVFSQPi73264637 = -532929890;    int hQoZVFSQPi35139980 = -876605112;    int hQoZVFSQPi7898239 = -768350859;    int hQoZVFSQPi10316513 = -460784258;    int hQoZVFSQPi77074184 = -299582288;    int hQoZVFSQPi94905711 = -706361272;    int hQoZVFSQPi8777386 = -195530908;    int hQoZVFSQPi20128909 = -338438023;    int hQoZVFSQPi7500968 = -495521691;    int hQoZVFSQPi20581507 = -782037567;    int hQoZVFSQPi78217803 = -568315142;    int hQoZVFSQPi97165051 = -574479352;    int hQoZVFSQPi47102157 = -425697034;    int hQoZVFSQPi19654857 = -813825404;    int hQoZVFSQPi68914382 = -76687801;    int hQoZVFSQPi52421019 = -753384118;    int hQoZVFSQPi55061854 = -772826424;    int hQoZVFSQPi93911671 = -311616673;    int hQoZVFSQPi18484808 = -164101194;    int hQoZVFSQPi36636695 = -108573343;    int hQoZVFSQPi1031088 = -449407951;    int hQoZVFSQPi91795431 = -477491892;    int hQoZVFSQPi29985746 = -258913790;    int hQoZVFSQPi21999048 = 96888941;    int hQoZVFSQPi13754063 = -987057336;    int hQoZVFSQPi67252721 = -381259309;    int hQoZVFSQPi81248256 = -404131697;    int hQoZVFSQPi34806616 = -544945663;    int hQoZVFSQPi41304218 = -309348594;    int hQoZVFSQPi57269611 = -248483409;    int hQoZVFSQPi87702833 = -488419921;    int hQoZVFSQPi61056450 = -528252002;    int hQoZVFSQPi5509394 = -605825964;    int hQoZVFSQPi16951144 = -312038389;    int hQoZVFSQPi50703443 = -591057766;    int hQoZVFSQPi36446005 = -838670993;    int hQoZVFSQPi91035528 = -228033012;    int hQoZVFSQPi98727228 = -952710858;    int hQoZVFSQPi31182280 = -421729435;    int hQoZVFSQPi84677886 = -476992615;    int hQoZVFSQPi21143887 = -134230338;    int hQoZVFSQPi66674249 = -923379145;    int hQoZVFSQPi49570303 = 81225819;    int hQoZVFSQPi63515208 = -527387502;    int hQoZVFSQPi88309218 = -670108253;    int hQoZVFSQPi34116577 = 18688837;    int hQoZVFSQPi41368730 = -417803253;    int hQoZVFSQPi90010426 = -62362019;    int hQoZVFSQPi81487990 = -588343222;    int hQoZVFSQPi25059829 = -465391148;    int hQoZVFSQPi83029055 = -708269281;    int hQoZVFSQPi35862946 = -722038189;    int hQoZVFSQPi93948919 = -743170394;    int hQoZVFSQPi61354472 = -454424777;    int hQoZVFSQPi54171605 = -767965533;    int hQoZVFSQPi81469206 = 44562001;    int hQoZVFSQPi5154234 = -517691322;    int hQoZVFSQPi85899190 = -765239800;    int hQoZVFSQPi96562449 = -473726923;    int hQoZVFSQPi9821464 = -918322979;    int hQoZVFSQPi13657456 = -202229575;    int hQoZVFSQPi73970769 = -650585246;    int hQoZVFSQPi78824691 = 70910571;    int hQoZVFSQPi50231356 = -147038283;    int hQoZVFSQPi32878674 = -193617646;    int hQoZVFSQPi17161353 = 59936859;    int hQoZVFSQPi91655658 = -968653389;    int hQoZVFSQPi30151013 = -13658645;    int hQoZVFSQPi68951413 = -122767638;    int hQoZVFSQPi32468378 = -238016809;    int hQoZVFSQPi61385491 = -425351107;    int hQoZVFSQPi56334625 = -820115567;    int hQoZVFSQPi62729392 = -889887239;    int hQoZVFSQPi33806922 = -687108580;    int hQoZVFSQPi15492808 = -974343006;    int hQoZVFSQPi34356838 = -526028807;    int hQoZVFSQPi42225128 = -458717711;    int hQoZVFSQPi66470538 = -731526289;    int hQoZVFSQPi33689830 = -233002807;    int hQoZVFSQPi79637486 = -905746173;    int hQoZVFSQPi25883991 = -963456056;    int hQoZVFSQPi91237830 = -241769679;    int hQoZVFSQPi53318626 = -956602441;    int hQoZVFSQPi16244389 = -843957446;    int hQoZVFSQPi74240556 = -540214128;    int hQoZVFSQPi51839887 = -766381733;    int hQoZVFSQPi67107530 = -785081609;    int hQoZVFSQPi44154921 = -51401187;    int hQoZVFSQPi62779539 = -544072856;    int hQoZVFSQPi69234237 = -535619768;    int hQoZVFSQPi31291772 = -943500287;     hQoZVFSQPi88470887 = hQoZVFSQPi9665283;     hQoZVFSQPi9665283 = hQoZVFSQPi50402372;     hQoZVFSQPi50402372 = hQoZVFSQPi77480848;     hQoZVFSQPi77480848 = hQoZVFSQPi38090910;     hQoZVFSQPi38090910 = hQoZVFSQPi29774618;     hQoZVFSQPi29774618 = hQoZVFSQPi12433728;     hQoZVFSQPi12433728 = hQoZVFSQPi97991167;     hQoZVFSQPi97991167 = hQoZVFSQPi55202692;     hQoZVFSQPi55202692 = hQoZVFSQPi73264637;     hQoZVFSQPi73264637 = hQoZVFSQPi35139980;     hQoZVFSQPi35139980 = hQoZVFSQPi7898239;     hQoZVFSQPi7898239 = hQoZVFSQPi10316513;     hQoZVFSQPi10316513 = hQoZVFSQPi77074184;     hQoZVFSQPi77074184 = hQoZVFSQPi94905711;     hQoZVFSQPi94905711 = hQoZVFSQPi8777386;     hQoZVFSQPi8777386 = hQoZVFSQPi20128909;     hQoZVFSQPi20128909 = hQoZVFSQPi7500968;     hQoZVFSQPi7500968 = hQoZVFSQPi20581507;     hQoZVFSQPi20581507 = hQoZVFSQPi78217803;     hQoZVFSQPi78217803 = hQoZVFSQPi97165051;     hQoZVFSQPi97165051 = hQoZVFSQPi47102157;     hQoZVFSQPi47102157 = hQoZVFSQPi19654857;     hQoZVFSQPi19654857 = hQoZVFSQPi68914382;     hQoZVFSQPi68914382 = hQoZVFSQPi52421019;     hQoZVFSQPi52421019 = hQoZVFSQPi55061854;     hQoZVFSQPi55061854 = hQoZVFSQPi93911671;     hQoZVFSQPi93911671 = hQoZVFSQPi18484808;     hQoZVFSQPi18484808 = hQoZVFSQPi36636695;     hQoZVFSQPi36636695 = hQoZVFSQPi1031088;     hQoZVFSQPi1031088 = hQoZVFSQPi91795431;     hQoZVFSQPi91795431 = hQoZVFSQPi29985746;     hQoZVFSQPi29985746 = hQoZVFSQPi21999048;     hQoZVFSQPi21999048 = hQoZVFSQPi13754063;     hQoZVFSQPi13754063 = hQoZVFSQPi67252721;     hQoZVFSQPi67252721 = hQoZVFSQPi81248256;     hQoZVFSQPi81248256 = hQoZVFSQPi34806616;     hQoZVFSQPi34806616 = hQoZVFSQPi41304218;     hQoZVFSQPi41304218 = hQoZVFSQPi57269611;     hQoZVFSQPi57269611 = hQoZVFSQPi87702833;     hQoZVFSQPi87702833 = hQoZVFSQPi61056450;     hQoZVFSQPi61056450 = hQoZVFSQPi5509394;     hQoZVFSQPi5509394 = hQoZVFSQPi16951144;     hQoZVFSQPi16951144 = hQoZVFSQPi50703443;     hQoZVFSQPi50703443 = hQoZVFSQPi36446005;     hQoZVFSQPi36446005 = hQoZVFSQPi91035528;     hQoZVFSQPi91035528 = hQoZVFSQPi98727228;     hQoZVFSQPi98727228 = hQoZVFSQPi31182280;     hQoZVFSQPi31182280 = hQoZVFSQPi84677886;     hQoZVFSQPi84677886 = hQoZVFSQPi21143887;     hQoZVFSQPi21143887 = hQoZVFSQPi66674249;     hQoZVFSQPi66674249 = hQoZVFSQPi49570303;     hQoZVFSQPi49570303 = hQoZVFSQPi63515208;     hQoZVFSQPi63515208 = hQoZVFSQPi88309218;     hQoZVFSQPi88309218 = hQoZVFSQPi34116577;     hQoZVFSQPi34116577 = hQoZVFSQPi41368730;     hQoZVFSQPi41368730 = hQoZVFSQPi90010426;     hQoZVFSQPi90010426 = hQoZVFSQPi81487990;     hQoZVFSQPi81487990 = hQoZVFSQPi25059829;     hQoZVFSQPi25059829 = hQoZVFSQPi83029055;     hQoZVFSQPi83029055 = hQoZVFSQPi35862946;     hQoZVFSQPi35862946 = hQoZVFSQPi93948919;     hQoZVFSQPi93948919 = hQoZVFSQPi61354472;     hQoZVFSQPi61354472 = hQoZVFSQPi54171605;     hQoZVFSQPi54171605 = hQoZVFSQPi81469206;     hQoZVFSQPi81469206 = hQoZVFSQPi5154234;     hQoZVFSQPi5154234 = hQoZVFSQPi85899190;     hQoZVFSQPi85899190 = hQoZVFSQPi96562449;     hQoZVFSQPi96562449 = hQoZVFSQPi9821464;     hQoZVFSQPi9821464 = hQoZVFSQPi13657456;     hQoZVFSQPi13657456 = hQoZVFSQPi73970769;     hQoZVFSQPi73970769 = hQoZVFSQPi78824691;     hQoZVFSQPi78824691 = hQoZVFSQPi50231356;     hQoZVFSQPi50231356 = hQoZVFSQPi32878674;     hQoZVFSQPi32878674 = hQoZVFSQPi17161353;     hQoZVFSQPi17161353 = hQoZVFSQPi91655658;     hQoZVFSQPi91655658 = hQoZVFSQPi30151013;     hQoZVFSQPi30151013 = hQoZVFSQPi68951413;     hQoZVFSQPi68951413 = hQoZVFSQPi32468378;     hQoZVFSQPi32468378 = hQoZVFSQPi61385491;     hQoZVFSQPi61385491 = hQoZVFSQPi56334625;     hQoZVFSQPi56334625 = hQoZVFSQPi62729392;     hQoZVFSQPi62729392 = hQoZVFSQPi33806922;     hQoZVFSQPi33806922 = hQoZVFSQPi15492808;     hQoZVFSQPi15492808 = hQoZVFSQPi34356838;     hQoZVFSQPi34356838 = hQoZVFSQPi42225128;     hQoZVFSQPi42225128 = hQoZVFSQPi66470538;     hQoZVFSQPi66470538 = hQoZVFSQPi33689830;     hQoZVFSQPi33689830 = hQoZVFSQPi79637486;     hQoZVFSQPi79637486 = hQoZVFSQPi25883991;     hQoZVFSQPi25883991 = hQoZVFSQPi91237830;     hQoZVFSQPi91237830 = hQoZVFSQPi53318626;     hQoZVFSQPi53318626 = hQoZVFSQPi16244389;     hQoZVFSQPi16244389 = hQoZVFSQPi74240556;     hQoZVFSQPi74240556 = hQoZVFSQPi51839887;     hQoZVFSQPi51839887 = hQoZVFSQPi67107530;     hQoZVFSQPi67107530 = hQoZVFSQPi44154921;     hQoZVFSQPi44154921 = hQoZVFSQPi62779539;     hQoZVFSQPi62779539 = hQoZVFSQPi69234237;     hQoZVFSQPi69234237 = hQoZVFSQPi31291772;     hQoZVFSQPi31291772 = hQoZVFSQPi88470887;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zkaiQMMQJk49494970() {     int uDzzyDDCha71915193 = -724165968;    int uDzzyDDCha26537372 = -975569610;    int uDzzyDDCha46810879 = -558556251;    int uDzzyDDCha40273100 = -412575102;    int uDzzyDDCha44212234 = -287963195;    int uDzzyDDCha28279019 = -912726515;    int uDzzyDDCha83473045 = -747182901;    int uDzzyDDCha12338611 = -650156150;    int uDzzyDDCha13732393 = -558832834;    int uDzzyDDCha5810290 = 98693998;    int uDzzyDDCha84555917 = 33279016;    int uDzzyDDCha71103637 = -755969106;    int uDzzyDDCha28434288 = -252445499;    int uDzzyDDCha56701076 = -87872645;    int uDzzyDDCha55760175 = -800384254;    int uDzzyDDCha95166547 = 16389766;    int uDzzyDDCha13880794 = -91783050;    int uDzzyDDCha88352414 = -742826568;    int uDzzyDDCha83021649 = -780712437;    int uDzzyDDCha67585245 = 2098865;    int uDzzyDDCha52805438 = -164008383;    int uDzzyDDCha59249475 = -416848804;    int uDzzyDDCha23480564 = -853714602;    int uDzzyDDCha3620941 = -148513197;    int uDzzyDDCha93996977 = -213036877;    int uDzzyDDCha50404003 = -653336139;    int uDzzyDDCha43366412 = 60298984;    int uDzzyDDCha58216433 = -234056078;    int uDzzyDDCha72717789 = 28921515;    int uDzzyDDCha16864429 = -962257802;    int uDzzyDDCha9890745 = -115249093;    int uDzzyDDCha93092173 = -174108870;    int uDzzyDDCha65537935 = -623983237;    int uDzzyDDCha55912573 = -578659506;    int uDzzyDDCha45979451 = -578924045;    int uDzzyDDCha93637440 = -192155603;    int uDzzyDDCha3420795 = -423794843;    int uDzzyDDCha81883049 = -17819114;    int uDzzyDDCha6264690 = -433381207;    int uDzzyDDCha37197968 = -241013443;    int uDzzyDDCha94705374 = -99800975;    int uDzzyDDCha55997179 = -79734285;    int uDzzyDDCha40518214 = -863095632;    int uDzzyDDCha42608383 = -347240537;    int uDzzyDDCha28390502 = -566827946;    int uDzzyDDCha30641524 = -247643936;    int uDzzyDDCha74742659 = 14984913;    int uDzzyDDCha50852875 = -420451153;    int uDzzyDDCha15524999 = 37405302;    int uDzzyDDCha56476371 = -341250793;    int uDzzyDDCha39931698 = -44280567;    int uDzzyDDCha5071098 = 22966508;    int uDzzyDDCha77627667 = -739134818;    int uDzzyDDCha12985977 = -388521641;    int uDzzyDDCha96689531 = -329797286;    int uDzzyDDCha12665718 = -207317164;    int uDzzyDDCha3056808 = -21855009;    int uDzzyDDCha43189939 = -310043054;    int uDzzyDDCha46276122 = -99538225;    int uDzzyDDCha93808230 = -634627057;    int uDzzyDDCha84912606 = -873025499;    int uDzzyDDCha25256613 = -413126824;    int uDzzyDDCha39620821 = -579077665;    int uDzzyDDCha96867964 = -596575033;    int uDzzyDDCha95919544 = -786056910;    int uDzzyDDCha91463744 = -792612115;    int uDzzyDDCha5565703 = -31985869;    int uDzzyDDCha72521714 = -673785994;    int uDzzyDDCha10721625 = -508948601;    int uDzzyDDCha62122735 = -508228651;    int uDzzyDDCha91745752 = -559815392;    int uDzzyDDCha31997745 = 26036064;    int uDzzyDDCha82087724 = -209445361;    int uDzzyDDCha45823681 = -439698995;    int uDzzyDDCha72879871 = -898100160;    int uDzzyDDCha96808259 = 15725902;    int uDzzyDDCha18731262 = -553753173;    int uDzzyDDCha80872181 = -406474065;    int uDzzyDDCha75230439 = -581685252;    int uDzzyDDCha63355453 = -965392942;    int uDzzyDDCha75661344 = -568321053;    int uDzzyDDCha92513537 = -519249863;    int uDzzyDDCha42691435 = -171461381;    int uDzzyDDCha16241418 = -629827693;    int uDzzyDDCha76932731 = -817977236;    int uDzzyDDCha4819648 = -38215601;    int uDzzyDDCha15464506 = -434974052;    int uDzzyDDCha52551958 = -135461597;    int uDzzyDDCha59223042 = -148862220;    int uDzzyDDCha33313734 = -271606881;    int uDzzyDDCha90580632 = -70300594;    int uDzzyDDCha60230856 = -13751789;    int uDzzyDDCha35606928 = -918280889;    int uDzzyDDCha12456459 = -798754151;    int uDzzyDDCha52285361 = -367987944;    int uDzzyDDCha69448762 = -686674152;    int uDzzyDDCha16376359 = -500656621;    int uDzzyDDCha43650250 = -166520600;    int uDzzyDDCha46688839 = -561183628;    int uDzzyDDCha36926758 = -724165968;     uDzzyDDCha71915193 = uDzzyDDCha26537372;     uDzzyDDCha26537372 = uDzzyDDCha46810879;     uDzzyDDCha46810879 = uDzzyDDCha40273100;     uDzzyDDCha40273100 = uDzzyDDCha44212234;     uDzzyDDCha44212234 = uDzzyDDCha28279019;     uDzzyDDCha28279019 = uDzzyDDCha83473045;     uDzzyDDCha83473045 = uDzzyDDCha12338611;     uDzzyDDCha12338611 = uDzzyDDCha13732393;     uDzzyDDCha13732393 = uDzzyDDCha5810290;     uDzzyDDCha5810290 = uDzzyDDCha84555917;     uDzzyDDCha84555917 = uDzzyDDCha71103637;     uDzzyDDCha71103637 = uDzzyDDCha28434288;     uDzzyDDCha28434288 = uDzzyDDCha56701076;     uDzzyDDCha56701076 = uDzzyDDCha55760175;     uDzzyDDCha55760175 = uDzzyDDCha95166547;     uDzzyDDCha95166547 = uDzzyDDCha13880794;     uDzzyDDCha13880794 = uDzzyDDCha88352414;     uDzzyDDCha88352414 = uDzzyDDCha83021649;     uDzzyDDCha83021649 = uDzzyDDCha67585245;     uDzzyDDCha67585245 = uDzzyDDCha52805438;     uDzzyDDCha52805438 = uDzzyDDCha59249475;     uDzzyDDCha59249475 = uDzzyDDCha23480564;     uDzzyDDCha23480564 = uDzzyDDCha3620941;     uDzzyDDCha3620941 = uDzzyDDCha93996977;     uDzzyDDCha93996977 = uDzzyDDCha50404003;     uDzzyDDCha50404003 = uDzzyDDCha43366412;     uDzzyDDCha43366412 = uDzzyDDCha58216433;     uDzzyDDCha58216433 = uDzzyDDCha72717789;     uDzzyDDCha72717789 = uDzzyDDCha16864429;     uDzzyDDCha16864429 = uDzzyDDCha9890745;     uDzzyDDCha9890745 = uDzzyDDCha93092173;     uDzzyDDCha93092173 = uDzzyDDCha65537935;     uDzzyDDCha65537935 = uDzzyDDCha55912573;     uDzzyDDCha55912573 = uDzzyDDCha45979451;     uDzzyDDCha45979451 = uDzzyDDCha93637440;     uDzzyDDCha93637440 = uDzzyDDCha3420795;     uDzzyDDCha3420795 = uDzzyDDCha81883049;     uDzzyDDCha81883049 = uDzzyDDCha6264690;     uDzzyDDCha6264690 = uDzzyDDCha37197968;     uDzzyDDCha37197968 = uDzzyDDCha94705374;     uDzzyDDCha94705374 = uDzzyDDCha55997179;     uDzzyDDCha55997179 = uDzzyDDCha40518214;     uDzzyDDCha40518214 = uDzzyDDCha42608383;     uDzzyDDCha42608383 = uDzzyDDCha28390502;     uDzzyDDCha28390502 = uDzzyDDCha30641524;     uDzzyDDCha30641524 = uDzzyDDCha74742659;     uDzzyDDCha74742659 = uDzzyDDCha50852875;     uDzzyDDCha50852875 = uDzzyDDCha15524999;     uDzzyDDCha15524999 = uDzzyDDCha56476371;     uDzzyDDCha56476371 = uDzzyDDCha39931698;     uDzzyDDCha39931698 = uDzzyDDCha5071098;     uDzzyDDCha5071098 = uDzzyDDCha77627667;     uDzzyDDCha77627667 = uDzzyDDCha12985977;     uDzzyDDCha12985977 = uDzzyDDCha96689531;     uDzzyDDCha96689531 = uDzzyDDCha12665718;     uDzzyDDCha12665718 = uDzzyDDCha3056808;     uDzzyDDCha3056808 = uDzzyDDCha43189939;     uDzzyDDCha43189939 = uDzzyDDCha46276122;     uDzzyDDCha46276122 = uDzzyDDCha93808230;     uDzzyDDCha93808230 = uDzzyDDCha84912606;     uDzzyDDCha84912606 = uDzzyDDCha25256613;     uDzzyDDCha25256613 = uDzzyDDCha39620821;     uDzzyDDCha39620821 = uDzzyDDCha96867964;     uDzzyDDCha96867964 = uDzzyDDCha95919544;     uDzzyDDCha95919544 = uDzzyDDCha91463744;     uDzzyDDCha91463744 = uDzzyDDCha5565703;     uDzzyDDCha5565703 = uDzzyDDCha72521714;     uDzzyDDCha72521714 = uDzzyDDCha10721625;     uDzzyDDCha10721625 = uDzzyDDCha62122735;     uDzzyDDCha62122735 = uDzzyDDCha91745752;     uDzzyDDCha91745752 = uDzzyDDCha31997745;     uDzzyDDCha31997745 = uDzzyDDCha82087724;     uDzzyDDCha82087724 = uDzzyDDCha45823681;     uDzzyDDCha45823681 = uDzzyDDCha72879871;     uDzzyDDCha72879871 = uDzzyDDCha96808259;     uDzzyDDCha96808259 = uDzzyDDCha18731262;     uDzzyDDCha18731262 = uDzzyDDCha80872181;     uDzzyDDCha80872181 = uDzzyDDCha75230439;     uDzzyDDCha75230439 = uDzzyDDCha63355453;     uDzzyDDCha63355453 = uDzzyDDCha75661344;     uDzzyDDCha75661344 = uDzzyDDCha92513537;     uDzzyDDCha92513537 = uDzzyDDCha42691435;     uDzzyDDCha42691435 = uDzzyDDCha16241418;     uDzzyDDCha16241418 = uDzzyDDCha76932731;     uDzzyDDCha76932731 = uDzzyDDCha4819648;     uDzzyDDCha4819648 = uDzzyDDCha15464506;     uDzzyDDCha15464506 = uDzzyDDCha52551958;     uDzzyDDCha52551958 = uDzzyDDCha59223042;     uDzzyDDCha59223042 = uDzzyDDCha33313734;     uDzzyDDCha33313734 = uDzzyDDCha90580632;     uDzzyDDCha90580632 = uDzzyDDCha60230856;     uDzzyDDCha60230856 = uDzzyDDCha35606928;     uDzzyDDCha35606928 = uDzzyDDCha12456459;     uDzzyDDCha12456459 = uDzzyDDCha52285361;     uDzzyDDCha52285361 = uDzzyDDCha69448762;     uDzzyDDCha69448762 = uDzzyDDCha16376359;     uDzzyDDCha16376359 = uDzzyDDCha43650250;     uDzzyDDCha43650250 = uDzzyDDCha46688839;     uDzzyDDCha46688839 = uDzzyDDCha36926758;     uDzzyDDCha36926758 = uDzzyDDCha71915193;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MuAiyAuDTA49937073() {     int ysCmXMXWhP7094949 = -621086481;    int ysCmXMXWhP59083444 = 48148446;    int ysCmXMXWhP92019745 = -149440863;    int ysCmXMXWhP78140753 = -422644080;    int ysCmXMXWhP27057491 = -902481340;    int ysCmXMXWhP51666774 = -850825394;    int ysCmXMXWhP28289207 = -310305811;    int ysCmXMXWhP45477531 = -761093733;    int ysCmXMXWhP23560171 = -691140896;    int ysCmXMXWhP7153066 = -911328097;    int ysCmXMXWhP29264554 = -118559328;    int ysCmXMXWhP44665415 = -678568852;    int ysCmXMXWhP33732858 = -753287001;    int ysCmXMXWhP95095686 = -602843549;    int ysCmXMXWhP70629896 = -372134946;    int ysCmXMXWhP93282981 = -583373515;    int ysCmXMXWhP63603204 = -213222767;    int ysCmXMXWhP54886782 = -352707939;    int ysCmXMXWhP5035641 = -207301491;    int ysCmXMXWhP76682262 = -237755276;    int ysCmXMXWhP54193178 = -936345582;    int ysCmXMXWhP73816247 = -581481582;    int ysCmXMXWhP28655786 = -501841246;    int ysCmXMXWhP30977749 = -91930544;    int ysCmXMXWhP6988794 = -718344073;    int ysCmXMXWhP3451567 = -879455634;    int ysCmXMXWhP31649787 = -30194076;    int ysCmXMXWhP96077283 = -687341648;    int ysCmXMXWhP83548956 = -983482530;    int ysCmXMXWhP2682854 = -837008450;    int ysCmXMXWhP65204867 = -85888760;    int ysCmXMXWhP69228330 = -413078141;    int ysCmXMXWhP7170132 = -827237300;    int ysCmXMXWhP92094796 = -930858842;    int ysCmXMXWhP22911310 = -993398423;    int ysCmXMXWhP87726234 = -216348338;    int ysCmXMXWhP21683200 = -946998766;    int ysCmXMXWhP23295494 = -909801999;    int ysCmXMXWhP25034610 = 36403600;    int ysCmXMXWhP11775520 = -612222632;    int ysCmXMXWhP97038385 = -890508736;    int ysCmXMXWhP66510365 = -642162228;    int ysCmXMXWhP20668516 = -214929406;    int ysCmXMXWhP27449142 = -28811986;    int ysCmXMXWhP92364206 = -895481982;    int ysCmXMXWhP71941967 = -691146800;    int ysCmXMXWhP36924284 = -579659770;    int ysCmXMXWhP49507935 = -823577567;    int ysCmXMXWhP35390564 = -512149510;    int ysCmXMXWhP80465114 = -407282629;    int ysCmXMXWhP53869038 = 26313855;    int ysCmXMXWhP92551644 = -496892364;    int ysCmXMXWhP14978873 = -694995323;    int ysCmXMXWhP7721977 = -821734340;    int ysCmXMXWhP95743894 = -708046610;    int ysCmXMXWhP33278702 = 60395101;    int ysCmXMXWhP30427658 = -450010308;    int ysCmXMXWhP61041997 = 42489681;    int ysCmXMXWhP71151959 = -704300008;    int ysCmXMXWhP23605924 = 76974293;    int ysCmXMXWhP20016987 = -720631319;    int ysCmXMXWhP32211923 = -622964163;    int ysCmXMXWhP61928575 = -777611204;    int ysCmXMXWhP20877317 = -854132446;    int ysCmXMXWhP41948198 = -725439337;    int ysCmXMXWhP60036223 = -705481187;    int ysCmXMXWhP37495284 = -851331553;    int ysCmXMXWhP41638061 = -822428160;    int ysCmXMXWhP72184377 = -609445126;    int ysCmXMXWhP82903661 = -55786609;    int ysCmXMXWhP71599781 = -636374749;    int ysCmXMXWhP40307710 = -303420769;    int ysCmXMXWhP29852173 = -289111540;    int ysCmXMXWhP93260120 = -595078860;    int ysCmXMXWhP79643876 = -347246540;    int ysCmXMXWhP87682812 = -194183354;    int ysCmXMXWhP53147731 = -266552176;    int ysCmXMXWhP1206645 = -373029261;    int ysCmXMXWhP38613543 = -196448563;    int ysCmXMXWhP35046827 = 72802727;    int ysCmXMXWhP66527283 = -199795864;    int ysCmXMXWhP82141852 = -206616509;    int ysCmXMXWhP60686720 = -75192139;    int ysCmXMXWhP3083842 = -476199901;    int ysCmXMXWhP48813816 = -763322305;    int ysCmXMXWhP72653223 = -588996396;    int ysCmXMXWhP54249458 = -718082819;    int ysCmXMXWhP99448154 = 94497039;    int ysCmXMXWhP96350901 = -122812232;    int ysCmXMXWhP89632608 = -953793524;    int ysCmXMXWhP57298577 = -766338030;    int ysCmXMXWhP60641203 = -889488448;    int ysCmXMXWhP52143535 = -105501991;    int ysCmXMXWhP1428686 = 59429307;    int ysCmXMXWhP91758532 = -891591314;    int ysCmXMXWhP64826462 = -167544574;    int ysCmXMXWhP4581791 = -864551025;    int ysCmXMXWhP99791198 = -360796960;    int ysCmXMXWhP85500944 = -303372649;    int ysCmXMXWhP32327983 = -621086481;     ysCmXMXWhP7094949 = ysCmXMXWhP59083444;     ysCmXMXWhP59083444 = ysCmXMXWhP92019745;     ysCmXMXWhP92019745 = ysCmXMXWhP78140753;     ysCmXMXWhP78140753 = ysCmXMXWhP27057491;     ysCmXMXWhP27057491 = ysCmXMXWhP51666774;     ysCmXMXWhP51666774 = ysCmXMXWhP28289207;     ysCmXMXWhP28289207 = ysCmXMXWhP45477531;     ysCmXMXWhP45477531 = ysCmXMXWhP23560171;     ysCmXMXWhP23560171 = ysCmXMXWhP7153066;     ysCmXMXWhP7153066 = ysCmXMXWhP29264554;     ysCmXMXWhP29264554 = ysCmXMXWhP44665415;     ysCmXMXWhP44665415 = ysCmXMXWhP33732858;     ysCmXMXWhP33732858 = ysCmXMXWhP95095686;     ysCmXMXWhP95095686 = ysCmXMXWhP70629896;     ysCmXMXWhP70629896 = ysCmXMXWhP93282981;     ysCmXMXWhP93282981 = ysCmXMXWhP63603204;     ysCmXMXWhP63603204 = ysCmXMXWhP54886782;     ysCmXMXWhP54886782 = ysCmXMXWhP5035641;     ysCmXMXWhP5035641 = ysCmXMXWhP76682262;     ysCmXMXWhP76682262 = ysCmXMXWhP54193178;     ysCmXMXWhP54193178 = ysCmXMXWhP73816247;     ysCmXMXWhP73816247 = ysCmXMXWhP28655786;     ysCmXMXWhP28655786 = ysCmXMXWhP30977749;     ysCmXMXWhP30977749 = ysCmXMXWhP6988794;     ysCmXMXWhP6988794 = ysCmXMXWhP3451567;     ysCmXMXWhP3451567 = ysCmXMXWhP31649787;     ysCmXMXWhP31649787 = ysCmXMXWhP96077283;     ysCmXMXWhP96077283 = ysCmXMXWhP83548956;     ysCmXMXWhP83548956 = ysCmXMXWhP2682854;     ysCmXMXWhP2682854 = ysCmXMXWhP65204867;     ysCmXMXWhP65204867 = ysCmXMXWhP69228330;     ysCmXMXWhP69228330 = ysCmXMXWhP7170132;     ysCmXMXWhP7170132 = ysCmXMXWhP92094796;     ysCmXMXWhP92094796 = ysCmXMXWhP22911310;     ysCmXMXWhP22911310 = ysCmXMXWhP87726234;     ysCmXMXWhP87726234 = ysCmXMXWhP21683200;     ysCmXMXWhP21683200 = ysCmXMXWhP23295494;     ysCmXMXWhP23295494 = ysCmXMXWhP25034610;     ysCmXMXWhP25034610 = ysCmXMXWhP11775520;     ysCmXMXWhP11775520 = ysCmXMXWhP97038385;     ysCmXMXWhP97038385 = ysCmXMXWhP66510365;     ysCmXMXWhP66510365 = ysCmXMXWhP20668516;     ysCmXMXWhP20668516 = ysCmXMXWhP27449142;     ysCmXMXWhP27449142 = ysCmXMXWhP92364206;     ysCmXMXWhP92364206 = ysCmXMXWhP71941967;     ysCmXMXWhP71941967 = ysCmXMXWhP36924284;     ysCmXMXWhP36924284 = ysCmXMXWhP49507935;     ysCmXMXWhP49507935 = ysCmXMXWhP35390564;     ysCmXMXWhP35390564 = ysCmXMXWhP80465114;     ysCmXMXWhP80465114 = ysCmXMXWhP53869038;     ysCmXMXWhP53869038 = ysCmXMXWhP92551644;     ysCmXMXWhP92551644 = ysCmXMXWhP14978873;     ysCmXMXWhP14978873 = ysCmXMXWhP7721977;     ysCmXMXWhP7721977 = ysCmXMXWhP95743894;     ysCmXMXWhP95743894 = ysCmXMXWhP33278702;     ysCmXMXWhP33278702 = ysCmXMXWhP30427658;     ysCmXMXWhP30427658 = ysCmXMXWhP61041997;     ysCmXMXWhP61041997 = ysCmXMXWhP71151959;     ysCmXMXWhP71151959 = ysCmXMXWhP23605924;     ysCmXMXWhP23605924 = ysCmXMXWhP20016987;     ysCmXMXWhP20016987 = ysCmXMXWhP32211923;     ysCmXMXWhP32211923 = ysCmXMXWhP61928575;     ysCmXMXWhP61928575 = ysCmXMXWhP20877317;     ysCmXMXWhP20877317 = ysCmXMXWhP41948198;     ysCmXMXWhP41948198 = ysCmXMXWhP60036223;     ysCmXMXWhP60036223 = ysCmXMXWhP37495284;     ysCmXMXWhP37495284 = ysCmXMXWhP41638061;     ysCmXMXWhP41638061 = ysCmXMXWhP72184377;     ysCmXMXWhP72184377 = ysCmXMXWhP82903661;     ysCmXMXWhP82903661 = ysCmXMXWhP71599781;     ysCmXMXWhP71599781 = ysCmXMXWhP40307710;     ysCmXMXWhP40307710 = ysCmXMXWhP29852173;     ysCmXMXWhP29852173 = ysCmXMXWhP93260120;     ysCmXMXWhP93260120 = ysCmXMXWhP79643876;     ysCmXMXWhP79643876 = ysCmXMXWhP87682812;     ysCmXMXWhP87682812 = ysCmXMXWhP53147731;     ysCmXMXWhP53147731 = ysCmXMXWhP1206645;     ysCmXMXWhP1206645 = ysCmXMXWhP38613543;     ysCmXMXWhP38613543 = ysCmXMXWhP35046827;     ysCmXMXWhP35046827 = ysCmXMXWhP66527283;     ysCmXMXWhP66527283 = ysCmXMXWhP82141852;     ysCmXMXWhP82141852 = ysCmXMXWhP60686720;     ysCmXMXWhP60686720 = ysCmXMXWhP3083842;     ysCmXMXWhP3083842 = ysCmXMXWhP48813816;     ysCmXMXWhP48813816 = ysCmXMXWhP72653223;     ysCmXMXWhP72653223 = ysCmXMXWhP54249458;     ysCmXMXWhP54249458 = ysCmXMXWhP99448154;     ysCmXMXWhP99448154 = ysCmXMXWhP96350901;     ysCmXMXWhP96350901 = ysCmXMXWhP89632608;     ysCmXMXWhP89632608 = ysCmXMXWhP57298577;     ysCmXMXWhP57298577 = ysCmXMXWhP60641203;     ysCmXMXWhP60641203 = ysCmXMXWhP52143535;     ysCmXMXWhP52143535 = ysCmXMXWhP1428686;     ysCmXMXWhP1428686 = ysCmXMXWhP91758532;     ysCmXMXWhP91758532 = ysCmXMXWhP64826462;     ysCmXMXWhP64826462 = ysCmXMXWhP4581791;     ysCmXMXWhP4581791 = ysCmXMXWhP99791198;     ysCmXMXWhP99791198 = ysCmXMXWhP85500944;     ysCmXMXWhP85500944 = ysCmXMXWhP32327983;     ysCmXMXWhP32327983 = ysCmXMXWhP7094949;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YAMyIkBoxc20392946() {     int HAGGelxgIK19881116 = -756127173;    int HAGGelxgIK45854332 = -122185567;    int HAGGelxgIK9772546 = -557800244;    int HAGGelxgIK86081578 = -246846357;    int HAGGelxgIK20568595 = -294297805;    int HAGGelxgIK94976844 = -627748707;    int HAGGelxgIK49496965 = -855977881;    int HAGGelxgIK72196643 = -105611990;    int HAGGelxgIK18964981 = -652898988;    int HAGGelxgIK13371771 = -715415730;    int HAGGelxgIK56751936 = -233245344;    int HAGGelxgIK76615919 = -85901668;    int HAGGelxgIK3876161 = -657069658;    int HAGGelxgIK44719766 = -935258747;    int HAGGelxgIK93936449 = -279120777;    int HAGGelxgIK21708050 = -34264904;    int HAGGelxgIK19355022 = -976997234;    int HAGGelxgIK38445268 = 92081759;    int HAGGelxgIK6196271 = -116770687;    int HAGGelxgIK79336758 = -836999415;    int HAGGelxgIK34367235 = -327661846;    int HAGGelxgIK2864067 = -751339930;    int HAGGelxgIK67416082 = -432845141;    int HAGGelxgIK75484878 = -452275557;    int HAGGelxgIK12883054 = -928785997;    int HAGGelxgIK84770211 = -569886833;    int HAGGelxgIK43065323 = -663900356;    int HAGGelxgIK63430208 = -861139753;    int HAGGelxgIK166010 = -97699770;    int HAGGelxgIK55433969 = -466894605;    int HAGGelxgIK9640864 = -362305510;    int HAGGelxgIK72547154 = -391797820;    int HAGGelxgIK53216080 = -703092084;    int HAGGelxgIK69171169 = -835613060;    int HAGGelxgIK5971842 = -503430303;    int HAGGelxgIK82950349 = -4911442;    int HAGGelxgIK58447260 = -736908117;    int HAGGelxgIK80999517 = -829422090;    int HAGGelxgIK30809996 = -905446977;    int HAGGelxgIK38040249 = -456639478;    int HAGGelxgIK58469694 = -165982730;    int HAGGelxgIK74212821 = -540473562;    int HAGGelxgIK55551985 = -238362851;    int HAGGelxgIK72077680 = -369984229;    int HAGGelxgIK72372696 = -788251942;    int HAGGelxgIK45570096 = -764465239;    int HAGGelxgIK14506304 = 22825177;    int HAGGelxgIK78947901 = -978930347;    int HAGGelxgIK79454612 = -675751484;    int HAGGelxgIK27415535 = 81680908;    int HAGGelxgIK72196531 = -226570453;    int HAGGelxgIK20728192 = -968077702;    int HAGGelxgIK49464022 = -870998605;    int HAGGelxgIK52421702 = -411186082;    int HAGGelxgIK86025711 = -702491096;    int HAGGelxgIK17017050 = 95212757;    int HAGGelxgIK78438250 = -689340427;    int HAGGelxgIK34287668 = -5524688;    int HAGGelxgIK73198525 = -318060360;    int HAGGelxgIK35798383 = -724410973;    int HAGGelxgIK51911521 = -963848352;    int HAGGelxgIK86066756 = -994838129;    int HAGGelxgIK72030634 = 92087779;    int HAGGelxgIK63531012 = -86004384;    int HAGGelxgIK3730907 = -253110220;    int HAGGelxgIK84204782 = -841447524;    int HAGGelxgIK23399840 = -382809584;    int HAGGelxgIK34704991 = -821456599;    int HAGGelxgIK38747924 = -331828444;    int HAGGelxgIK10986100 = -174209336;    int HAGGelxgIK63260790 = -297356787;    int HAGGelxgIK38355505 = -47575144;    int HAGGelxgIK7635272 = -2471265;    int HAGGelxgIK68156021 = -660131209;    int HAGGelxgIK20867065 = -571016686;    int HAGGelxgIK60154413 = -787188284;    int HAGGelxgIK47312081 = -412977080;    int HAGGelxgIK95338401 = 37139087;    int HAGGelxgIK3112182 = -664023615;    int HAGGelxgIK67312957 = -64320758;    int HAGGelxgIK70263908 = -492712011;    int HAGGelxgIK64117422 = -684970010;    int HAGGelxgIK83975595 = -85388269;    int HAGGelxgIK72750474 = -79380679;    int HAGGelxgIK83237437 = -140324152;    int HAGGelxgIK88912671 = -394227808;    int HAGGelxgIK23083132 = -520799215;    int HAGGelxgIK794379 = -191906003;    int HAGGelxgIK83145458 = -33121964;    int HAGGelxgIK88954792 = -498643061;    int HAGGelxgIK4512100 = -315571016;    int HAGGelxgIK24159593 = -631383429;    int HAGGelxgIK7800992 = -411361730;    int HAGGelxgIK95011612 = -81036005;    int HAGGelxgIK86128727 = -492791126;    int HAGGelxgIK72402937 = -171144601;    int HAGGelxgIK2182188 = -532561342;    int HAGGelxgIK92020973 = -52358468;    int HAGGelxgIK68346773 = -16874009;    int HAGGelxgIK88167914 = -756127173;     HAGGelxgIK19881116 = HAGGelxgIK45854332;     HAGGelxgIK45854332 = HAGGelxgIK9772546;     HAGGelxgIK9772546 = HAGGelxgIK86081578;     HAGGelxgIK86081578 = HAGGelxgIK20568595;     HAGGelxgIK20568595 = HAGGelxgIK94976844;     HAGGelxgIK94976844 = HAGGelxgIK49496965;     HAGGelxgIK49496965 = HAGGelxgIK72196643;     HAGGelxgIK72196643 = HAGGelxgIK18964981;     HAGGelxgIK18964981 = HAGGelxgIK13371771;     HAGGelxgIK13371771 = HAGGelxgIK56751936;     HAGGelxgIK56751936 = HAGGelxgIK76615919;     HAGGelxgIK76615919 = HAGGelxgIK3876161;     HAGGelxgIK3876161 = HAGGelxgIK44719766;     HAGGelxgIK44719766 = HAGGelxgIK93936449;     HAGGelxgIK93936449 = HAGGelxgIK21708050;     HAGGelxgIK21708050 = HAGGelxgIK19355022;     HAGGelxgIK19355022 = HAGGelxgIK38445268;     HAGGelxgIK38445268 = HAGGelxgIK6196271;     HAGGelxgIK6196271 = HAGGelxgIK79336758;     HAGGelxgIK79336758 = HAGGelxgIK34367235;     HAGGelxgIK34367235 = HAGGelxgIK2864067;     HAGGelxgIK2864067 = HAGGelxgIK67416082;     HAGGelxgIK67416082 = HAGGelxgIK75484878;     HAGGelxgIK75484878 = HAGGelxgIK12883054;     HAGGelxgIK12883054 = HAGGelxgIK84770211;     HAGGelxgIK84770211 = HAGGelxgIK43065323;     HAGGelxgIK43065323 = HAGGelxgIK63430208;     HAGGelxgIK63430208 = HAGGelxgIK166010;     HAGGelxgIK166010 = HAGGelxgIK55433969;     HAGGelxgIK55433969 = HAGGelxgIK9640864;     HAGGelxgIK9640864 = HAGGelxgIK72547154;     HAGGelxgIK72547154 = HAGGelxgIK53216080;     HAGGelxgIK53216080 = HAGGelxgIK69171169;     HAGGelxgIK69171169 = HAGGelxgIK5971842;     HAGGelxgIK5971842 = HAGGelxgIK82950349;     HAGGelxgIK82950349 = HAGGelxgIK58447260;     HAGGelxgIK58447260 = HAGGelxgIK80999517;     HAGGelxgIK80999517 = HAGGelxgIK30809996;     HAGGelxgIK30809996 = HAGGelxgIK38040249;     HAGGelxgIK38040249 = HAGGelxgIK58469694;     HAGGelxgIK58469694 = HAGGelxgIK74212821;     HAGGelxgIK74212821 = HAGGelxgIK55551985;     HAGGelxgIK55551985 = HAGGelxgIK72077680;     HAGGelxgIK72077680 = HAGGelxgIK72372696;     HAGGelxgIK72372696 = HAGGelxgIK45570096;     HAGGelxgIK45570096 = HAGGelxgIK14506304;     HAGGelxgIK14506304 = HAGGelxgIK78947901;     HAGGelxgIK78947901 = HAGGelxgIK79454612;     HAGGelxgIK79454612 = HAGGelxgIK27415535;     HAGGelxgIK27415535 = HAGGelxgIK72196531;     HAGGelxgIK72196531 = HAGGelxgIK20728192;     HAGGelxgIK20728192 = HAGGelxgIK49464022;     HAGGelxgIK49464022 = HAGGelxgIK52421702;     HAGGelxgIK52421702 = HAGGelxgIK86025711;     HAGGelxgIK86025711 = HAGGelxgIK17017050;     HAGGelxgIK17017050 = HAGGelxgIK78438250;     HAGGelxgIK78438250 = HAGGelxgIK34287668;     HAGGelxgIK34287668 = HAGGelxgIK73198525;     HAGGelxgIK73198525 = HAGGelxgIK35798383;     HAGGelxgIK35798383 = HAGGelxgIK51911521;     HAGGelxgIK51911521 = HAGGelxgIK86066756;     HAGGelxgIK86066756 = HAGGelxgIK72030634;     HAGGelxgIK72030634 = HAGGelxgIK63531012;     HAGGelxgIK63531012 = HAGGelxgIK3730907;     HAGGelxgIK3730907 = HAGGelxgIK84204782;     HAGGelxgIK84204782 = HAGGelxgIK23399840;     HAGGelxgIK23399840 = HAGGelxgIK34704991;     HAGGelxgIK34704991 = HAGGelxgIK38747924;     HAGGelxgIK38747924 = HAGGelxgIK10986100;     HAGGelxgIK10986100 = HAGGelxgIK63260790;     HAGGelxgIK63260790 = HAGGelxgIK38355505;     HAGGelxgIK38355505 = HAGGelxgIK7635272;     HAGGelxgIK7635272 = HAGGelxgIK68156021;     HAGGelxgIK68156021 = HAGGelxgIK20867065;     HAGGelxgIK20867065 = HAGGelxgIK60154413;     HAGGelxgIK60154413 = HAGGelxgIK47312081;     HAGGelxgIK47312081 = HAGGelxgIK95338401;     HAGGelxgIK95338401 = HAGGelxgIK3112182;     HAGGelxgIK3112182 = HAGGelxgIK67312957;     HAGGelxgIK67312957 = HAGGelxgIK70263908;     HAGGelxgIK70263908 = HAGGelxgIK64117422;     HAGGelxgIK64117422 = HAGGelxgIK83975595;     HAGGelxgIK83975595 = HAGGelxgIK72750474;     HAGGelxgIK72750474 = HAGGelxgIK83237437;     HAGGelxgIK83237437 = HAGGelxgIK88912671;     HAGGelxgIK88912671 = HAGGelxgIK23083132;     HAGGelxgIK23083132 = HAGGelxgIK794379;     HAGGelxgIK794379 = HAGGelxgIK83145458;     HAGGelxgIK83145458 = HAGGelxgIK88954792;     HAGGelxgIK88954792 = HAGGelxgIK4512100;     HAGGelxgIK4512100 = HAGGelxgIK24159593;     HAGGelxgIK24159593 = HAGGelxgIK7800992;     HAGGelxgIK7800992 = HAGGelxgIK95011612;     HAGGelxgIK95011612 = HAGGelxgIK86128727;     HAGGelxgIK86128727 = HAGGelxgIK72402937;     HAGGelxgIK72402937 = HAGGelxgIK2182188;     HAGGelxgIK2182188 = HAGGelxgIK92020973;     HAGGelxgIK92020973 = HAGGelxgIK68346773;     HAGGelxgIK68346773 = HAGGelxgIK88167914;     HAGGelxgIK88167914 = HAGGelxgIK19881116;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BqvNPqVWLM38606288() {     int SlQlwNXFfF3325422 = -536792853;    int SlQlwNXFfF62726421 = -121567755;    int SlQlwNXFfF6181053 = -351325472;    int SlQlwNXFfF48873830 = -440646193;    int SlQlwNXFfF26689919 = -101165296;    int SlQlwNXFfF93481245 = -406820361;    int SlQlwNXFfF20536283 = -595889195;    int SlQlwNXFfF86544086 = -92770020;    int SlQlwNXFfF77494681 = -994358339;    int SlQlwNXFfF45917422 = -83791842;    int SlQlwNXFfF6167874 = -423361216;    int SlQlwNXFfF39821319 = -73519914;    int SlQlwNXFfF21993937 = -448730899;    int SlQlwNXFfF24346658 = -723549104;    int SlQlwNXFfF54790913 = -373143759;    int SlQlwNXFfF8097212 = -922344229;    int SlQlwNXFfF13106907 = -730342261;    int SlQlwNXFfF19296714 = -155223117;    int SlQlwNXFfF68636413 = -115445557;    int SlQlwNXFfF68704201 = -266585408;    int SlQlwNXFfF90007622 = 82809123;    int SlQlwNXFfF15011385 = -742491700;    int SlQlwNXFfF71241789 = -472734338;    int SlQlwNXFfF10191437 = -524100953;    int SlQlwNXFfF54459011 = -388438756;    int SlQlwNXFfF80112360 = -450396549;    int SlQlwNXFfF92520063 = -291984698;    int SlQlwNXFfF3161834 = -931094637;    int SlQlwNXFfF36247103 = 39795088;    int SlQlwNXFfF71267310 = -979744456;    int SlQlwNXFfF27736177 = -62711;    int SlQlwNXFfF35653581 = -306992900;    int SlQlwNXFfF96754967 = -323964262;    int SlQlwNXFfF11329680 = -427215230;    int SlQlwNXFfF84698572 = -701095038;    int SlQlwNXFfF95339533 = -892935348;    int SlQlwNXFfF27061439 = -615757297;    int SlQlwNXFfF21578349 = -537892610;    int SlQlwNXFfF79805074 = 9655226;    int SlQlwNXFfF87535384 = -209233000;    int SlQlwNXFfF92118618 = -837531703;    int SlQlwNXFfF24700608 = -14381884;    int SlQlwNXFfF79119055 = -789420094;    int SlQlwNXFfF63982620 = -126167000;    int SlQlwNXFfF64317193 = -516408896;    int SlQlwNXFfF85176092 = -784076163;    int SlQlwNXFfF90521733 = -109479052;    int SlQlwNXFfF98618496 = -977652065;    int SlQlwNXFfF10301725 = -161353567;    int SlQlwNXFfF62748019 = -125339547;    int SlQlwNXFfF45453979 = -447471876;    int SlQlwNXFfF76228986 = 73662986;    int SlQlwNXFfF63576481 = 17254078;    int SlQlwNXFfF77098461 = -129599469;    int SlQlwNXFfF48598665 = 49022781;    int SlQlwNXFfF88314037 = -794301154;    int SlQlwNXFfF91484631 = -648833417;    int SlQlwNXFfF95989616 = -827224520;    int SlQlwNXFfF94414818 = 47792563;    int SlQlwNXFfF46577558 = -650768748;    int SlQlwNXFfF961183 = -14835663;    int SlQlwNXFfF17374449 = -664794559;    int SlQlwNXFfF50296983 = -32565108;    int SlQlwNXFfF6227372 = 85386116;    int SlQlwNXFfF18181245 = 16270869;    int SlQlwNXFfF70514293 = -16368317;    int SlQlwNXFfF43066352 = -749555653;    int SlQlwNXFfF10664257 = 78484330;    int SlQlwNXFfF39648085 = 77545934;    int SlQlwNXFfF59451379 = -480208412;    int SlQlwNXFfF81035773 = -206586933;    int SlQlwNXFfF91528557 = -92449651;    int SlQlwNXFfF39491640 = -64878343;    int SlQlwNXFfF81101028 = -906212558;    int SlQlwNXFfF76585582 = -429053705;    int SlQlwNXFfF65307014 = -902808994;    int SlQlwNXFfF35892329 = -953071607;    int SlQlwNXFfF7259170 = -246567339;    int SlQlwNXFfF45874243 = 92307942;    int SlQlwNXFfF69282919 = -604362593;    int SlQlwNXFfF89590627 = -240917497;    int SlQlwNXFfF93901566 = -314332634;    int SlQlwNXFfF92860108 = -669741071;    int SlQlwNXFfF73499084 = -834865366;    int SlQlwNXFfF25813331 = -432272581;    int SlQlwNXFfF51507191 = 26274302;    int SlQlwNXFfF72077100 = -224246979;    int SlQlwNXFfF19656506 = -94364793;    int SlQlwNXFfF62731014 = -376238011;    int SlQlwNXFfF96384535 = -906793885;    int SlQlwNXFfF3854903 = -144101931;    int SlQlwNXFfF31071823 = -788532777;    int SlQlwNXFfF27163530 = -485685173;    int SlQlwNXFfF33227516 = -339576027;    int SlQlwNXFfF86574202 = -94397338;    int SlQlwNXFfF74744169 = -72737145;    int SlQlwNXFfF74403624 = -981816776;    int SlQlwNXFfF72891684 = -774806211;    int SlQlwNXFfF45801375 = -42437869;    int SlQlwNXFfF93802900 = -536792853;     SlQlwNXFfF3325422 = SlQlwNXFfF62726421;     SlQlwNXFfF62726421 = SlQlwNXFfF6181053;     SlQlwNXFfF6181053 = SlQlwNXFfF48873830;     SlQlwNXFfF48873830 = SlQlwNXFfF26689919;     SlQlwNXFfF26689919 = SlQlwNXFfF93481245;     SlQlwNXFfF93481245 = SlQlwNXFfF20536283;     SlQlwNXFfF20536283 = SlQlwNXFfF86544086;     SlQlwNXFfF86544086 = SlQlwNXFfF77494681;     SlQlwNXFfF77494681 = SlQlwNXFfF45917422;     SlQlwNXFfF45917422 = SlQlwNXFfF6167874;     SlQlwNXFfF6167874 = SlQlwNXFfF39821319;     SlQlwNXFfF39821319 = SlQlwNXFfF21993937;     SlQlwNXFfF21993937 = SlQlwNXFfF24346658;     SlQlwNXFfF24346658 = SlQlwNXFfF54790913;     SlQlwNXFfF54790913 = SlQlwNXFfF8097212;     SlQlwNXFfF8097212 = SlQlwNXFfF13106907;     SlQlwNXFfF13106907 = SlQlwNXFfF19296714;     SlQlwNXFfF19296714 = SlQlwNXFfF68636413;     SlQlwNXFfF68636413 = SlQlwNXFfF68704201;     SlQlwNXFfF68704201 = SlQlwNXFfF90007622;     SlQlwNXFfF90007622 = SlQlwNXFfF15011385;     SlQlwNXFfF15011385 = SlQlwNXFfF71241789;     SlQlwNXFfF71241789 = SlQlwNXFfF10191437;     SlQlwNXFfF10191437 = SlQlwNXFfF54459011;     SlQlwNXFfF54459011 = SlQlwNXFfF80112360;     SlQlwNXFfF80112360 = SlQlwNXFfF92520063;     SlQlwNXFfF92520063 = SlQlwNXFfF3161834;     SlQlwNXFfF3161834 = SlQlwNXFfF36247103;     SlQlwNXFfF36247103 = SlQlwNXFfF71267310;     SlQlwNXFfF71267310 = SlQlwNXFfF27736177;     SlQlwNXFfF27736177 = SlQlwNXFfF35653581;     SlQlwNXFfF35653581 = SlQlwNXFfF96754967;     SlQlwNXFfF96754967 = SlQlwNXFfF11329680;     SlQlwNXFfF11329680 = SlQlwNXFfF84698572;     SlQlwNXFfF84698572 = SlQlwNXFfF95339533;     SlQlwNXFfF95339533 = SlQlwNXFfF27061439;     SlQlwNXFfF27061439 = SlQlwNXFfF21578349;     SlQlwNXFfF21578349 = SlQlwNXFfF79805074;     SlQlwNXFfF79805074 = SlQlwNXFfF87535384;     SlQlwNXFfF87535384 = SlQlwNXFfF92118618;     SlQlwNXFfF92118618 = SlQlwNXFfF24700608;     SlQlwNXFfF24700608 = SlQlwNXFfF79119055;     SlQlwNXFfF79119055 = SlQlwNXFfF63982620;     SlQlwNXFfF63982620 = SlQlwNXFfF64317193;     SlQlwNXFfF64317193 = SlQlwNXFfF85176092;     SlQlwNXFfF85176092 = SlQlwNXFfF90521733;     SlQlwNXFfF90521733 = SlQlwNXFfF98618496;     SlQlwNXFfF98618496 = SlQlwNXFfF10301725;     SlQlwNXFfF10301725 = SlQlwNXFfF62748019;     SlQlwNXFfF62748019 = SlQlwNXFfF45453979;     SlQlwNXFfF45453979 = SlQlwNXFfF76228986;     SlQlwNXFfF76228986 = SlQlwNXFfF63576481;     SlQlwNXFfF63576481 = SlQlwNXFfF77098461;     SlQlwNXFfF77098461 = SlQlwNXFfF48598665;     SlQlwNXFfF48598665 = SlQlwNXFfF88314037;     SlQlwNXFfF88314037 = SlQlwNXFfF91484631;     SlQlwNXFfF91484631 = SlQlwNXFfF95989616;     SlQlwNXFfF95989616 = SlQlwNXFfF94414818;     SlQlwNXFfF94414818 = SlQlwNXFfF46577558;     SlQlwNXFfF46577558 = SlQlwNXFfF961183;     SlQlwNXFfF961183 = SlQlwNXFfF17374449;     SlQlwNXFfF17374449 = SlQlwNXFfF50296983;     SlQlwNXFfF50296983 = SlQlwNXFfF6227372;     SlQlwNXFfF6227372 = SlQlwNXFfF18181245;     SlQlwNXFfF18181245 = SlQlwNXFfF70514293;     SlQlwNXFfF70514293 = SlQlwNXFfF43066352;     SlQlwNXFfF43066352 = SlQlwNXFfF10664257;     SlQlwNXFfF10664257 = SlQlwNXFfF39648085;     SlQlwNXFfF39648085 = SlQlwNXFfF59451379;     SlQlwNXFfF59451379 = SlQlwNXFfF81035773;     SlQlwNXFfF81035773 = SlQlwNXFfF91528557;     SlQlwNXFfF91528557 = SlQlwNXFfF39491640;     SlQlwNXFfF39491640 = SlQlwNXFfF81101028;     SlQlwNXFfF81101028 = SlQlwNXFfF76585582;     SlQlwNXFfF76585582 = SlQlwNXFfF65307014;     SlQlwNXFfF65307014 = SlQlwNXFfF35892329;     SlQlwNXFfF35892329 = SlQlwNXFfF7259170;     SlQlwNXFfF7259170 = SlQlwNXFfF45874243;     SlQlwNXFfF45874243 = SlQlwNXFfF69282919;     SlQlwNXFfF69282919 = SlQlwNXFfF89590627;     SlQlwNXFfF89590627 = SlQlwNXFfF93901566;     SlQlwNXFfF93901566 = SlQlwNXFfF92860108;     SlQlwNXFfF92860108 = SlQlwNXFfF73499084;     SlQlwNXFfF73499084 = SlQlwNXFfF25813331;     SlQlwNXFfF25813331 = SlQlwNXFfF51507191;     SlQlwNXFfF51507191 = SlQlwNXFfF72077100;     SlQlwNXFfF72077100 = SlQlwNXFfF19656506;     SlQlwNXFfF19656506 = SlQlwNXFfF62731014;     SlQlwNXFfF62731014 = SlQlwNXFfF96384535;     SlQlwNXFfF96384535 = SlQlwNXFfF3854903;     SlQlwNXFfF3854903 = SlQlwNXFfF31071823;     SlQlwNXFfF31071823 = SlQlwNXFfF27163530;     SlQlwNXFfF27163530 = SlQlwNXFfF33227516;     SlQlwNXFfF33227516 = SlQlwNXFfF86574202;     SlQlwNXFfF86574202 = SlQlwNXFfF74744169;     SlQlwNXFfF74744169 = SlQlwNXFfF74403624;     SlQlwNXFfF74403624 = SlQlwNXFfF72891684;     SlQlwNXFfF72891684 = SlQlwNXFfF45801375;     SlQlwNXFfF45801375 = SlQlwNXFfF93802900;     SlQlwNXFfF93802900 = SlQlwNXFfF3325422;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HmKaMiFSnT9062161() {     int CxRsiKoVAr16111589 = -671833545;    int CxRsiKoVAr49497309 = -291901768;    int CxRsiKoVAr23933853 = -759684854;    int CxRsiKoVAr56814655 = -264848470;    int CxRsiKoVAr20201024 = -592981761;    int CxRsiKoVAr36791316 = -183743674;    int CxRsiKoVAr41744041 = -41561265;    int CxRsiKoVAr13263199 = -537288276;    int CxRsiKoVAr72899492 = -956116431;    int CxRsiKoVAr52136127 = -987879475;    int CxRsiKoVAr33655256 = -538047232;    int CxRsiKoVAr71771824 = -580852730;    int CxRsiKoVAr92137239 = -352513557;    int CxRsiKoVAr73970736 = 44035698;    int CxRsiKoVAr78097465 = -280129590;    int CxRsiKoVAr36522280 = -373235618;    int CxRsiKoVAr68858724 = -394116728;    int CxRsiKoVAr2855200 = -810433419;    int CxRsiKoVAr69797043 = -24914753;    int CxRsiKoVAr71358697 = -865829547;    int CxRsiKoVAr70181679 = -408507141;    int CxRsiKoVAr44059204 = -912350048;    int CxRsiKoVAr10002085 = -403738233;    int CxRsiKoVAr54698566 = -884445965;    int CxRsiKoVAr60353270 = -598880680;    int CxRsiKoVAr61431006 = -140827748;    int CxRsiKoVAr3935600 = -925690979;    int CxRsiKoVAr70514759 = -4892741;    int CxRsiKoVAr52864156 = -174422152;    int CxRsiKoVAr24018426 = -609630611;    int CxRsiKoVAr72172173 = -276479461;    int CxRsiKoVAr38972405 = -285712578;    int CxRsiKoVAr42800916 = -199819046;    int CxRsiKoVAr88406052 = -331969448;    int CxRsiKoVAr67759104 = -211126919;    int CxRsiKoVAr90563648 = -681498452;    int CxRsiKoVAr63825499 = -405666647;    int CxRsiKoVAr79282371 = -457512702;    int CxRsiKoVAr85580460 = -932195352;    int CxRsiKoVAr13800114 = -53649846;    int CxRsiKoVAr53549926 = -113005696;    int CxRsiKoVAr32403064 = 87306782;    int CxRsiKoVAr14002525 = -812853539;    int CxRsiKoVAr8611158 = -467339243;    int CxRsiKoVAr44325684 = -409178856;    int CxRsiKoVAr58804222 = -857394603;    int CxRsiKoVAr68103753 = -606994104;    int CxRsiKoVAr28058463 = -33004845;    int CxRsiKoVAr54365774 = -324955541;    int CxRsiKoVAr9698440 = -736376010;    int CxRsiKoVAr63781473 = -700356184;    int CxRsiKoVAr4405534 = -397522352;    int CxRsiKoVAr98061631 = -158749204;    int CxRsiKoVAr21798186 = -819051211;    int CxRsiKoVAr38880482 = 54578294;    int CxRsiKoVAr72052385 = -759483497;    int CxRsiKoVAr39495224 = -888163536;    int CxRsiKoVAr69235286 = -875238889;    int CxRsiKoVAr96461385 = -665967790;    int CxRsiKoVAr58770017 = -352154014;    int CxRsiKoVAr32855717 = -258052696;    int CxRsiKoVAr71229282 = 63331476;    int CxRsiKoVAr60399043 = -262866124;    int CxRsiKoVAr48881066 = -246485821;    int CxRsiKoVAr79963954 = -611400014;    int CxRsiKoVAr94682851 = -152334654;    int CxRsiKoVAr28970908 = -281033685;    int CxRsiKoVAr3731187 = 79455891;    int CxRsiKoVAr6211632 = -744837384;    int CxRsiKoVAr87533817 = -598631138;    int CxRsiKoVAr72696781 = -967568972;    int CxRsiKoVAr89576352 = -936604026;    int CxRsiKoVAr17274739 = -878238068;    int CxRsiKoVAr55996929 = -971264908;    int CxRsiKoVAr17808771 = -652823851;    int CxRsiKoVAr37778616 = -395813924;    int CxRsiKoVAr30056679 = 503490;    int CxRsiKoVAr1390927 = -936398991;    int CxRsiKoVAr10372883 = -375267110;    int CxRsiKoVAr1549049 = -741486078;    int CxRsiKoVAr93327252 = -533833644;    int CxRsiKoVAr75877136 = -792686134;    int CxRsiKoVAr16148985 = -679937201;    int CxRsiKoVAr43165717 = -438046143;    int CxRsiKoVAr60236953 = -909274427;    int CxRsiKoVAr67766640 = -878957110;    int CxRsiKoVAr40910773 = -26963375;    int CxRsiKoVAr21002730 = -380767835;    int CxRsiKoVAr49525571 = -286547743;    int CxRsiKoVAr95706719 = -451643422;    int CxRsiKoVAr51068424 = -793334916;    int CxRsiKoVAr94590212 = -530427758;    int CxRsiKoVAr82820986 = -791544913;    int CxRsiKoVAr26810443 = -480041338;    int CxRsiKoVAr80944397 = -795597150;    int CxRsiKoVAr82320644 = -76337172;    int CxRsiKoVAr72004021 = -649827094;    int CxRsiKoVAr65121458 = -466367718;    int CxRsiKoVAr28647204 = -855939229;    int CxRsiKoVAr49642832 = -671833545;     CxRsiKoVAr16111589 = CxRsiKoVAr49497309;     CxRsiKoVAr49497309 = CxRsiKoVAr23933853;     CxRsiKoVAr23933853 = CxRsiKoVAr56814655;     CxRsiKoVAr56814655 = CxRsiKoVAr20201024;     CxRsiKoVAr20201024 = CxRsiKoVAr36791316;     CxRsiKoVAr36791316 = CxRsiKoVAr41744041;     CxRsiKoVAr41744041 = CxRsiKoVAr13263199;     CxRsiKoVAr13263199 = CxRsiKoVAr72899492;     CxRsiKoVAr72899492 = CxRsiKoVAr52136127;     CxRsiKoVAr52136127 = CxRsiKoVAr33655256;     CxRsiKoVAr33655256 = CxRsiKoVAr71771824;     CxRsiKoVAr71771824 = CxRsiKoVAr92137239;     CxRsiKoVAr92137239 = CxRsiKoVAr73970736;     CxRsiKoVAr73970736 = CxRsiKoVAr78097465;     CxRsiKoVAr78097465 = CxRsiKoVAr36522280;     CxRsiKoVAr36522280 = CxRsiKoVAr68858724;     CxRsiKoVAr68858724 = CxRsiKoVAr2855200;     CxRsiKoVAr2855200 = CxRsiKoVAr69797043;     CxRsiKoVAr69797043 = CxRsiKoVAr71358697;     CxRsiKoVAr71358697 = CxRsiKoVAr70181679;     CxRsiKoVAr70181679 = CxRsiKoVAr44059204;     CxRsiKoVAr44059204 = CxRsiKoVAr10002085;     CxRsiKoVAr10002085 = CxRsiKoVAr54698566;     CxRsiKoVAr54698566 = CxRsiKoVAr60353270;     CxRsiKoVAr60353270 = CxRsiKoVAr61431006;     CxRsiKoVAr61431006 = CxRsiKoVAr3935600;     CxRsiKoVAr3935600 = CxRsiKoVAr70514759;     CxRsiKoVAr70514759 = CxRsiKoVAr52864156;     CxRsiKoVAr52864156 = CxRsiKoVAr24018426;     CxRsiKoVAr24018426 = CxRsiKoVAr72172173;     CxRsiKoVAr72172173 = CxRsiKoVAr38972405;     CxRsiKoVAr38972405 = CxRsiKoVAr42800916;     CxRsiKoVAr42800916 = CxRsiKoVAr88406052;     CxRsiKoVAr88406052 = CxRsiKoVAr67759104;     CxRsiKoVAr67759104 = CxRsiKoVAr90563648;     CxRsiKoVAr90563648 = CxRsiKoVAr63825499;     CxRsiKoVAr63825499 = CxRsiKoVAr79282371;     CxRsiKoVAr79282371 = CxRsiKoVAr85580460;     CxRsiKoVAr85580460 = CxRsiKoVAr13800114;     CxRsiKoVAr13800114 = CxRsiKoVAr53549926;     CxRsiKoVAr53549926 = CxRsiKoVAr32403064;     CxRsiKoVAr32403064 = CxRsiKoVAr14002525;     CxRsiKoVAr14002525 = CxRsiKoVAr8611158;     CxRsiKoVAr8611158 = CxRsiKoVAr44325684;     CxRsiKoVAr44325684 = CxRsiKoVAr58804222;     CxRsiKoVAr58804222 = CxRsiKoVAr68103753;     CxRsiKoVAr68103753 = CxRsiKoVAr28058463;     CxRsiKoVAr28058463 = CxRsiKoVAr54365774;     CxRsiKoVAr54365774 = CxRsiKoVAr9698440;     CxRsiKoVAr9698440 = CxRsiKoVAr63781473;     CxRsiKoVAr63781473 = CxRsiKoVAr4405534;     CxRsiKoVAr4405534 = CxRsiKoVAr98061631;     CxRsiKoVAr98061631 = CxRsiKoVAr21798186;     CxRsiKoVAr21798186 = CxRsiKoVAr38880482;     CxRsiKoVAr38880482 = CxRsiKoVAr72052385;     CxRsiKoVAr72052385 = CxRsiKoVAr39495224;     CxRsiKoVAr39495224 = CxRsiKoVAr69235286;     CxRsiKoVAr69235286 = CxRsiKoVAr96461385;     CxRsiKoVAr96461385 = CxRsiKoVAr58770017;     CxRsiKoVAr58770017 = CxRsiKoVAr32855717;     CxRsiKoVAr32855717 = CxRsiKoVAr71229282;     CxRsiKoVAr71229282 = CxRsiKoVAr60399043;     CxRsiKoVAr60399043 = CxRsiKoVAr48881066;     CxRsiKoVAr48881066 = CxRsiKoVAr79963954;     CxRsiKoVAr79963954 = CxRsiKoVAr94682851;     CxRsiKoVAr94682851 = CxRsiKoVAr28970908;     CxRsiKoVAr28970908 = CxRsiKoVAr3731187;     CxRsiKoVAr3731187 = CxRsiKoVAr6211632;     CxRsiKoVAr6211632 = CxRsiKoVAr87533817;     CxRsiKoVAr87533817 = CxRsiKoVAr72696781;     CxRsiKoVAr72696781 = CxRsiKoVAr89576352;     CxRsiKoVAr89576352 = CxRsiKoVAr17274739;     CxRsiKoVAr17274739 = CxRsiKoVAr55996929;     CxRsiKoVAr55996929 = CxRsiKoVAr17808771;     CxRsiKoVAr17808771 = CxRsiKoVAr37778616;     CxRsiKoVAr37778616 = CxRsiKoVAr30056679;     CxRsiKoVAr30056679 = CxRsiKoVAr1390927;     CxRsiKoVAr1390927 = CxRsiKoVAr10372883;     CxRsiKoVAr10372883 = CxRsiKoVAr1549049;     CxRsiKoVAr1549049 = CxRsiKoVAr93327252;     CxRsiKoVAr93327252 = CxRsiKoVAr75877136;     CxRsiKoVAr75877136 = CxRsiKoVAr16148985;     CxRsiKoVAr16148985 = CxRsiKoVAr43165717;     CxRsiKoVAr43165717 = CxRsiKoVAr60236953;     CxRsiKoVAr60236953 = CxRsiKoVAr67766640;     CxRsiKoVAr67766640 = CxRsiKoVAr40910773;     CxRsiKoVAr40910773 = CxRsiKoVAr21002730;     CxRsiKoVAr21002730 = CxRsiKoVAr49525571;     CxRsiKoVAr49525571 = CxRsiKoVAr95706719;     CxRsiKoVAr95706719 = CxRsiKoVAr51068424;     CxRsiKoVAr51068424 = CxRsiKoVAr94590212;     CxRsiKoVAr94590212 = CxRsiKoVAr82820986;     CxRsiKoVAr82820986 = CxRsiKoVAr26810443;     CxRsiKoVAr26810443 = CxRsiKoVAr80944397;     CxRsiKoVAr80944397 = CxRsiKoVAr82320644;     CxRsiKoVAr82320644 = CxRsiKoVAr72004021;     CxRsiKoVAr72004021 = CxRsiKoVAr65121458;     CxRsiKoVAr65121458 = CxRsiKoVAr28647204;     CxRsiKoVAr28647204 = CxRsiKoVAr49642832;     CxRsiKoVAr49642832 = CxRsiKoVAr16111589;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UNItYuBudC27275502() {     int tFKwPGDoVU99555894 = -452499226;    int tFKwPGDoVU66369398 = -291283956;    int tFKwPGDoVU20342360 = -553210082;    int tFKwPGDoVU19606907 = -458648306;    int tFKwPGDoVU26322347 = -399849252;    int tFKwPGDoVU35295717 = 37184673;    int tFKwPGDoVU12783359 = -881472579;    int tFKwPGDoVU27610642 = -524446306;    int tFKwPGDoVU31429193 = -197575782;    int tFKwPGDoVU84681779 = -356255587;    int tFKwPGDoVU83071194 = -728163105;    int tFKwPGDoVU34977223 = -568470977;    int tFKwPGDoVU10255015 = -144174798;    int tFKwPGDoVU53597628 = -844254659;    int tFKwPGDoVU38951929 = -374152572;    int tFKwPGDoVU22911442 = -161314944;    int tFKwPGDoVU62610609 = -147461755;    int tFKwPGDoVU83706645 = 42261704;    int tFKwPGDoVU32237186 = -23589623;    int tFKwPGDoVU60726140 = -295415539;    int tFKwPGDoVU25822066 = 1963828;    int tFKwPGDoVU56206522 = -903501819;    int tFKwPGDoVU13827793 = -443627430;    int tFKwPGDoVU89405124 = -956271361;    int tFKwPGDoVU1929229 = -58533439;    int tFKwPGDoVU56773155 = -21337464;    int tFKwPGDoVU53390340 = -553775321;    int tFKwPGDoVU10246385 = -74847625;    int tFKwPGDoVU88945250 = -36927294;    int tFKwPGDoVU39851767 = -22480462;    int tFKwPGDoVU90267486 = 85763338;    int tFKwPGDoVU2078832 = -200907658;    int tFKwPGDoVU86339803 = -920691224;    int tFKwPGDoVU30564563 = 76428382;    int tFKwPGDoVU46485835 = -408791654;    int tFKwPGDoVU2952833 = -469522358;    int tFKwPGDoVU32439678 = -284515827;    int tFKwPGDoVU19861204 = -165983222;    int tFKwPGDoVU34575538 = -17093149;    int tFKwPGDoVU63295249 = -906243368;    int tFKwPGDoVU87198850 = -784554670;    int tFKwPGDoVU82890849 = -486601540;    int tFKwPGDoVU37569595 = -263910782;    int tFKwPGDoVU516098 = -223522013;    int tFKwPGDoVU36270180 = -137335809;    int tFKwPGDoVU98410217 = -877005526;    int tFKwPGDoVU44119183 = -739298334;    int tFKwPGDoVU47729059 = -31726563;    int tFKwPGDoVU85212886 = -910557624;    int tFKwPGDoVU45030923 = -943396465;    int tFKwPGDoVU37038921 = -921257606;    int tFKwPGDoVU59906327 = -455781663;    int tFKwPGDoVU12174091 = -370496521;    int tFKwPGDoVU46474945 = -537464599;    int tFKwPGDoVU1453436 = -293907829;    int tFKwPGDoVU43349372 = -548997408;    int tFKwPGDoVU52541606 = -847656526;    int tFKwPGDoVU30937235 = -596938721;    int tFKwPGDoVU17677679 = -300114867;    int tFKwPGDoVU69549192 = -278511789;    int tFKwPGDoVU81905377 = -409040006;    int tFKwPGDoVU2536975 = -706624954;    int tFKwPGDoVU38665391 = -387519012;    int tFKwPGDoVU91577425 = -75095321;    int tFKwPGDoVU94414292 = -342018925;    int tFKwPGDoVU80992362 = -427255447;    int tFKwPGDoVU48637420 = -647779753;    int tFKwPGDoVU79690452 = -120603180;    int tFKwPGDoVU7111794 = -335463006;    int tFKwPGDoVU35999096 = -904630215;    int tFKwPGDoVU90471764 = -876799117;    int tFKwPGDoVU42749405 = -981478533;    int tFKwPGDoVU49131107 = -940645147;    int tFKwPGDoVU68941936 = -117346256;    int tFKwPGDoVU73527289 = -510860870;    int tFKwPGDoVU42931216 = -511434633;    int tFKwPGDoVU18636928 = -539591038;    int tFKwPGDoVU13311695 = -120105417;    int tFKwPGDoVU53134944 = -718935553;    int tFKwPGDoVU3519011 = -181527913;    int tFKwPGDoVU12653972 = -282039130;    int tFKwPGDoVU5661282 = -422048758;    int tFKwPGDoVU25033498 = -164290002;    int tFKwPGDoVU43914327 = -93530830;    int tFKwPGDoVU2812846 = -101222856;    int tFKwPGDoVU30361160 = -458454999;    int tFKwPGDoVU89904741 = -830411138;    int tFKwPGDoVU39864858 = -283226625;    int tFKwPGDoVU29111128 = -629663790;    int tFKwPGDoVU3136463 = -859794247;    int tFKwPGDoVU50411227 = -621865832;    int tFKwPGDoVU1502443 = -687577106;    int tFKwPGDoVU2183526 = -865868356;    int tFKwPGDoVU65026346 = -738581360;    int tFKwPGDoVU81389872 = -397203362;    int tFKwPGDoVU84661876 = 22070284;    int tFKwPGDoVU44225458 = 917472;    int tFKwPGDoVU45992169 = -88815462;    int tFKwPGDoVU6101806 = -881503088;    int tFKwPGDoVU55277818 = -452499226;     tFKwPGDoVU99555894 = tFKwPGDoVU66369398;     tFKwPGDoVU66369398 = tFKwPGDoVU20342360;     tFKwPGDoVU20342360 = tFKwPGDoVU19606907;     tFKwPGDoVU19606907 = tFKwPGDoVU26322347;     tFKwPGDoVU26322347 = tFKwPGDoVU35295717;     tFKwPGDoVU35295717 = tFKwPGDoVU12783359;     tFKwPGDoVU12783359 = tFKwPGDoVU27610642;     tFKwPGDoVU27610642 = tFKwPGDoVU31429193;     tFKwPGDoVU31429193 = tFKwPGDoVU84681779;     tFKwPGDoVU84681779 = tFKwPGDoVU83071194;     tFKwPGDoVU83071194 = tFKwPGDoVU34977223;     tFKwPGDoVU34977223 = tFKwPGDoVU10255015;     tFKwPGDoVU10255015 = tFKwPGDoVU53597628;     tFKwPGDoVU53597628 = tFKwPGDoVU38951929;     tFKwPGDoVU38951929 = tFKwPGDoVU22911442;     tFKwPGDoVU22911442 = tFKwPGDoVU62610609;     tFKwPGDoVU62610609 = tFKwPGDoVU83706645;     tFKwPGDoVU83706645 = tFKwPGDoVU32237186;     tFKwPGDoVU32237186 = tFKwPGDoVU60726140;     tFKwPGDoVU60726140 = tFKwPGDoVU25822066;     tFKwPGDoVU25822066 = tFKwPGDoVU56206522;     tFKwPGDoVU56206522 = tFKwPGDoVU13827793;     tFKwPGDoVU13827793 = tFKwPGDoVU89405124;     tFKwPGDoVU89405124 = tFKwPGDoVU1929229;     tFKwPGDoVU1929229 = tFKwPGDoVU56773155;     tFKwPGDoVU56773155 = tFKwPGDoVU53390340;     tFKwPGDoVU53390340 = tFKwPGDoVU10246385;     tFKwPGDoVU10246385 = tFKwPGDoVU88945250;     tFKwPGDoVU88945250 = tFKwPGDoVU39851767;     tFKwPGDoVU39851767 = tFKwPGDoVU90267486;     tFKwPGDoVU90267486 = tFKwPGDoVU2078832;     tFKwPGDoVU2078832 = tFKwPGDoVU86339803;     tFKwPGDoVU86339803 = tFKwPGDoVU30564563;     tFKwPGDoVU30564563 = tFKwPGDoVU46485835;     tFKwPGDoVU46485835 = tFKwPGDoVU2952833;     tFKwPGDoVU2952833 = tFKwPGDoVU32439678;     tFKwPGDoVU32439678 = tFKwPGDoVU19861204;     tFKwPGDoVU19861204 = tFKwPGDoVU34575538;     tFKwPGDoVU34575538 = tFKwPGDoVU63295249;     tFKwPGDoVU63295249 = tFKwPGDoVU87198850;     tFKwPGDoVU87198850 = tFKwPGDoVU82890849;     tFKwPGDoVU82890849 = tFKwPGDoVU37569595;     tFKwPGDoVU37569595 = tFKwPGDoVU516098;     tFKwPGDoVU516098 = tFKwPGDoVU36270180;     tFKwPGDoVU36270180 = tFKwPGDoVU98410217;     tFKwPGDoVU98410217 = tFKwPGDoVU44119183;     tFKwPGDoVU44119183 = tFKwPGDoVU47729059;     tFKwPGDoVU47729059 = tFKwPGDoVU85212886;     tFKwPGDoVU85212886 = tFKwPGDoVU45030923;     tFKwPGDoVU45030923 = tFKwPGDoVU37038921;     tFKwPGDoVU37038921 = tFKwPGDoVU59906327;     tFKwPGDoVU59906327 = tFKwPGDoVU12174091;     tFKwPGDoVU12174091 = tFKwPGDoVU46474945;     tFKwPGDoVU46474945 = tFKwPGDoVU1453436;     tFKwPGDoVU1453436 = tFKwPGDoVU43349372;     tFKwPGDoVU43349372 = tFKwPGDoVU52541606;     tFKwPGDoVU52541606 = tFKwPGDoVU30937235;     tFKwPGDoVU30937235 = tFKwPGDoVU17677679;     tFKwPGDoVU17677679 = tFKwPGDoVU69549192;     tFKwPGDoVU69549192 = tFKwPGDoVU81905377;     tFKwPGDoVU81905377 = tFKwPGDoVU2536975;     tFKwPGDoVU2536975 = tFKwPGDoVU38665391;     tFKwPGDoVU38665391 = tFKwPGDoVU91577425;     tFKwPGDoVU91577425 = tFKwPGDoVU94414292;     tFKwPGDoVU94414292 = tFKwPGDoVU80992362;     tFKwPGDoVU80992362 = tFKwPGDoVU48637420;     tFKwPGDoVU48637420 = tFKwPGDoVU79690452;     tFKwPGDoVU79690452 = tFKwPGDoVU7111794;     tFKwPGDoVU7111794 = tFKwPGDoVU35999096;     tFKwPGDoVU35999096 = tFKwPGDoVU90471764;     tFKwPGDoVU90471764 = tFKwPGDoVU42749405;     tFKwPGDoVU42749405 = tFKwPGDoVU49131107;     tFKwPGDoVU49131107 = tFKwPGDoVU68941936;     tFKwPGDoVU68941936 = tFKwPGDoVU73527289;     tFKwPGDoVU73527289 = tFKwPGDoVU42931216;     tFKwPGDoVU42931216 = tFKwPGDoVU18636928;     tFKwPGDoVU18636928 = tFKwPGDoVU13311695;     tFKwPGDoVU13311695 = tFKwPGDoVU53134944;     tFKwPGDoVU53134944 = tFKwPGDoVU3519011;     tFKwPGDoVU3519011 = tFKwPGDoVU12653972;     tFKwPGDoVU12653972 = tFKwPGDoVU5661282;     tFKwPGDoVU5661282 = tFKwPGDoVU25033498;     tFKwPGDoVU25033498 = tFKwPGDoVU43914327;     tFKwPGDoVU43914327 = tFKwPGDoVU2812846;     tFKwPGDoVU2812846 = tFKwPGDoVU30361160;     tFKwPGDoVU30361160 = tFKwPGDoVU89904741;     tFKwPGDoVU89904741 = tFKwPGDoVU39864858;     tFKwPGDoVU39864858 = tFKwPGDoVU29111128;     tFKwPGDoVU29111128 = tFKwPGDoVU3136463;     tFKwPGDoVU3136463 = tFKwPGDoVU50411227;     tFKwPGDoVU50411227 = tFKwPGDoVU1502443;     tFKwPGDoVU1502443 = tFKwPGDoVU2183526;     tFKwPGDoVU2183526 = tFKwPGDoVU65026346;     tFKwPGDoVU65026346 = tFKwPGDoVU81389872;     tFKwPGDoVU81389872 = tFKwPGDoVU84661876;     tFKwPGDoVU84661876 = tFKwPGDoVU44225458;     tFKwPGDoVU44225458 = tFKwPGDoVU45992169;     tFKwPGDoVU45992169 = tFKwPGDoVU6101806;     tFKwPGDoVU6101806 = tFKwPGDoVU55277818;     tFKwPGDoVU55277818 = tFKwPGDoVU99555894;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GDBfguxzCD97731374() {     int DwGJozwWas12342062 = -587539918;    int DwGJozwWas53140286 = -461617970;    int DwGJozwWas38095159 = -961569463;    int DwGJozwWas27547733 = -282850582;    int DwGJozwWas19833452 = -891665717;    int DwGJozwWas78605787 = -839738640;    int DwGJozwWas33991117 = -327144649;    int DwGJozwWas54329754 = -968964562;    int DwGJozwWas26834003 = -159333874;    int DwGJozwWas90900483 = -160343220;    int DwGJozwWas10558577 = -842849120;    int DwGJozwWas66927728 = 24196207;    int DwGJozwWas80398318 = -47957455;    int DwGJozwWas3221707 = -76669857;    int DwGJozwWas62258482 = -281138403;    int DwGJozwWas51336511 = -712206333;    int DwGJozwWas18362427 = -911236222;    int DwGJozwWas67265130 = -612948598;    int DwGJozwWas33397816 = 66941181;    int DwGJozwWas63380636 = -894659678;    int DwGJozwWas5996123 = -489352436;    int DwGJozwWas85254341 = 26639833;    int DwGJozwWas52588088 = -374631325;    int DwGJozwWas33912254 = -216616373;    int DwGJozwWas7823488 = -268975364;    int DwGJozwWas38091800 = -811768663;    int DwGJozwWas64805876 = -87481601;    int DwGJozwWas77599309 = -248645730;    int DwGJozwWas5562304 = -251144535;    int DwGJozwWas92602882 = -752366617;    int DwGJozwWas34703483 = -190653412;    int DwGJozwWas5397656 = -179627337;    int DwGJozwWas32385752 = -796546008;    int DwGJozwWas7640936 = -928325837;    int DwGJozwWas29546367 = 81176466;    int DwGJozwWas98176947 = -258085462;    int DwGJozwWas69203738 = -74425178;    int DwGJozwWas77565226 = -85603314;    int DwGJozwWas40350924 = -958943726;    int DwGJozwWas89559978 = -750660214;    int DwGJozwWas48630159 = -60028663;    int DwGJozwWas90593305 = -384912874;    int DwGJozwWas72453064 = -287344226;    int DwGJozwWas45144636 = -564694256;    int DwGJozwWas16278671 = -30105769;    int DwGJozwWas72038347 = -950323966;    int DwGJozwWas21701203 = -136813386;    int DwGJozwWas77169025 = -187079343;    int DwGJozwWas29276936 = 25840402;    int DwGJozwWas91981343 = -454432928;    int DwGJozwWas55366414 = -74141915;    int DwGJozwWas88082874 = -926967001;    int DwGJozwWas46659240 = -546499803;    int DwGJozwWas91174670 = -126916341;    int DwGJozwWas91735252 = -288352315;    int DwGJozwWas27087721 = -514179752;    int DwGJozwWas552199 = 13013355;    int DwGJozwWas4182906 = -644953091;    int DwGJozwWas19724245 = 86124781;    int DwGJozwWas81741651 = 20102945;    int DwGJozwWas13799912 = -652257040;    int DwGJozwWas56391808 = 21501080;    int DwGJozwWas48767451 = -617820028;    int DwGJozwWas34231121 = -406967258;    int DwGJozwWas56197001 = -969689808;    int DwGJozwWas5160921 = -563221784;    int DwGJozwWas34541976 = -179257785;    int DwGJozwWas72757382 = -119631619;    int DwGJozwWas73675340 = -57846324;    int DwGJozwWas64081534 = 76947059;    int DwGJozwWas82132772 = -537781156;    int DwGJozwWas40797200 = -725632909;    int DwGJozwWas26914207 = -654004872;    int DwGJozwWas43837837 = -182398606;    int DwGJozwWas14750477 = -734631016;    int DwGJozwWas15402818 = -4439563;    int DwGJozwWas12801277 = -686015941;    int DwGJozwWas7443453 = -809937069;    int DwGJozwWas17633583 = -86510605;    int DwGJozwWas35785141 = -318651398;    int DwGJozwWas16390598 = -574955277;    int DwGJozwWas87636851 = -900402259;    int DwGJozwWas48322373 = -174486132;    int DwGJozwWas13580960 = -796711607;    int DwGJozwWas37236468 = -578224703;    int DwGJozwWas46620608 = -263686411;    int DwGJozwWas58738415 = -633127534;    int DwGJozwWas41211082 = -569629668;    int DwGJozwWas15905684 = -539973522;    int DwGJozwWas2458647 = -404643783;    int DwGJozwWas97624749 = -171098817;    int DwGJozwWas65020833 = -429472088;    int DwGJozwWas57840982 = -71728095;    int DwGJozwWas58609273 = -879046672;    int DwGJozwWas75760067 = 1596825;    int DwGJozwWas92238351 = 18470256;    int DwGJozwWas41825855 = -767092846;    int DwGJozwWas38221944 = -880376969;    int DwGJozwWas88947634 = -595004449;    int DwGJozwWas11117750 = -587539918;     DwGJozwWas12342062 = DwGJozwWas53140286;     DwGJozwWas53140286 = DwGJozwWas38095159;     DwGJozwWas38095159 = DwGJozwWas27547733;     DwGJozwWas27547733 = DwGJozwWas19833452;     DwGJozwWas19833452 = DwGJozwWas78605787;     DwGJozwWas78605787 = DwGJozwWas33991117;     DwGJozwWas33991117 = DwGJozwWas54329754;     DwGJozwWas54329754 = DwGJozwWas26834003;     DwGJozwWas26834003 = DwGJozwWas90900483;     DwGJozwWas90900483 = DwGJozwWas10558577;     DwGJozwWas10558577 = DwGJozwWas66927728;     DwGJozwWas66927728 = DwGJozwWas80398318;     DwGJozwWas80398318 = DwGJozwWas3221707;     DwGJozwWas3221707 = DwGJozwWas62258482;     DwGJozwWas62258482 = DwGJozwWas51336511;     DwGJozwWas51336511 = DwGJozwWas18362427;     DwGJozwWas18362427 = DwGJozwWas67265130;     DwGJozwWas67265130 = DwGJozwWas33397816;     DwGJozwWas33397816 = DwGJozwWas63380636;     DwGJozwWas63380636 = DwGJozwWas5996123;     DwGJozwWas5996123 = DwGJozwWas85254341;     DwGJozwWas85254341 = DwGJozwWas52588088;     DwGJozwWas52588088 = DwGJozwWas33912254;     DwGJozwWas33912254 = DwGJozwWas7823488;     DwGJozwWas7823488 = DwGJozwWas38091800;     DwGJozwWas38091800 = DwGJozwWas64805876;     DwGJozwWas64805876 = DwGJozwWas77599309;     DwGJozwWas77599309 = DwGJozwWas5562304;     DwGJozwWas5562304 = DwGJozwWas92602882;     DwGJozwWas92602882 = DwGJozwWas34703483;     DwGJozwWas34703483 = DwGJozwWas5397656;     DwGJozwWas5397656 = DwGJozwWas32385752;     DwGJozwWas32385752 = DwGJozwWas7640936;     DwGJozwWas7640936 = DwGJozwWas29546367;     DwGJozwWas29546367 = DwGJozwWas98176947;     DwGJozwWas98176947 = DwGJozwWas69203738;     DwGJozwWas69203738 = DwGJozwWas77565226;     DwGJozwWas77565226 = DwGJozwWas40350924;     DwGJozwWas40350924 = DwGJozwWas89559978;     DwGJozwWas89559978 = DwGJozwWas48630159;     DwGJozwWas48630159 = DwGJozwWas90593305;     DwGJozwWas90593305 = DwGJozwWas72453064;     DwGJozwWas72453064 = DwGJozwWas45144636;     DwGJozwWas45144636 = DwGJozwWas16278671;     DwGJozwWas16278671 = DwGJozwWas72038347;     DwGJozwWas72038347 = DwGJozwWas21701203;     DwGJozwWas21701203 = DwGJozwWas77169025;     DwGJozwWas77169025 = DwGJozwWas29276936;     DwGJozwWas29276936 = DwGJozwWas91981343;     DwGJozwWas91981343 = DwGJozwWas55366414;     DwGJozwWas55366414 = DwGJozwWas88082874;     DwGJozwWas88082874 = DwGJozwWas46659240;     DwGJozwWas46659240 = DwGJozwWas91174670;     DwGJozwWas91174670 = DwGJozwWas91735252;     DwGJozwWas91735252 = DwGJozwWas27087721;     DwGJozwWas27087721 = DwGJozwWas552199;     DwGJozwWas552199 = DwGJozwWas4182906;     DwGJozwWas4182906 = DwGJozwWas19724245;     DwGJozwWas19724245 = DwGJozwWas81741651;     DwGJozwWas81741651 = DwGJozwWas13799912;     DwGJozwWas13799912 = DwGJozwWas56391808;     DwGJozwWas56391808 = DwGJozwWas48767451;     DwGJozwWas48767451 = DwGJozwWas34231121;     DwGJozwWas34231121 = DwGJozwWas56197001;     DwGJozwWas56197001 = DwGJozwWas5160921;     DwGJozwWas5160921 = DwGJozwWas34541976;     DwGJozwWas34541976 = DwGJozwWas72757382;     DwGJozwWas72757382 = DwGJozwWas73675340;     DwGJozwWas73675340 = DwGJozwWas64081534;     DwGJozwWas64081534 = DwGJozwWas82132772;     DwGJozwWas82132772 = DwGJozwWas40797200;     DwGJozwWas40797200 = DwGJozwWas26914207;     DwGJozwWas26914207 = DwGJozwWas43837837;     DwGJozwWas43837837 = DwGJozwWas14750477;     DwGJozwWas14750477 = DwGJozwWas15402818;     DwGJozwWas15402818 = DwGJozwWas12801277;     DwGJozwWas12801277 = DwGJozwWas7443453;     DwGJozwWas7443453 = DwGJozwWas17633583;     DwGJozwWas17633583 = DwGJozwWas35785141;     DwGJozwWas35785141 = DwGJozwWas16390598;     DwGJozwWas16390598 = DwGJozwWas87636851;     DwGJozwWas87636851 = DwGJozwWas48322373;     DwGJozwWas48322373 = DwGJozwWas13580960;     DwGJozwWas13580960 = DwGJozwWas37236468;     DwGJozwWas37236468 = DwGJozwWas46620608;     DwGJozwWas46620608 = DwGJozwWas58738415;     DwGJozwWas58738415 = DwGJozwWas41211082;     DwGJozwWas41211082 = DwGJozwWas15905684;     DwGJozwWas15905684 = DwGJozwWas2458647;     DwGJozwWas2458647 = DwGJozwWas97624749;     DwGJozwWas97624749 = DwGJozwWas65020833;     DwGJozwWas65020833 = DwGJozwWas57840982;     DwGJozwWas57840982 = DwGJozwWas58609273;     DwGJozwWas58609273 = DwGJozwWas75760067;     DwGJozwWas75760067 = DwGJozwWas92238351;     DwGJozwWas92238351 = DwGJozwWas41825855;     DwGJozwWas41825855 = DwGJozwWas38221944;     DwGJozwWas38221944 = DwGJozwWas88947634;     DwGJozwWas88947634 = DwGJozwWas11117750;     DwGJozwWas11117750 = DwGJozwWas12342062;}
// Junk Finished
