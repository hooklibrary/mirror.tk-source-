#include "Controls.h"
#include "RenderManager.h"
#include "Menu.h"
#include "Gui.h"

#define UI_COL_MAIN2		Color(27, 220, 117, MenuAlpha)
#define UI_COL_SHADOW2		Color(0, 0, 0, MenuAlpha)

#define white Color(MenuAlpha, MenuAlpha, MenuAlpha)
#define mixed Color(90, 90, 90)
#define lighter_gray Color(48, 48, 48)
#define light_gray Color(40, 40, 40)
#define gray Color(28, 28, 28)
#define dark_gray Color(21, 21, 19)
#define darker_gray Color(19, 19, 19)
#define black Color(0, 0, 0)
#pragma region Base Control


void CControl::SetPosition(int x, int y)
{
	m_x = x;
	m_y = y;
}

void CControl::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CControl::GetSize(int &w, int &h)
{
	w = m_iWidth;
	h = m_iHeight;
}

bool CControl::Flag(int f)
{
	if (m_Flags & f)
		return true;
	else
		return false;
}

POINT CControl::GetAbsolutePos()
{
	POINT p;
	RECT client = parent->GetClientArea();
	if (parent)
	{
		p.x = m_x + client.left;
		p.y = m_y + client.top + 29;
	}

	return p;
}

void CControl::SetFileId(std::string fid)
{
	FileIdentifier = fid;
}
#pragma endregion Implementations of the Base control functions
CDropBox::CDropBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 20;
	FileControlType = UIControlTypes::UIC_dropdown;
}

void CDropBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };
	if (GUI.IsMouseInRegion(Region)) {
		Render::gradient_verticle(a.x, a.y, m_iWidth, 20, Color(55, 55, 55, 255), Color(31, 31, 31, 255));
		Render::Outline(a.x, a.y, m_iWidth, 20, Color(62, 62, 62, 255));
	}
	else {
		Render::gradient_verticle(a.x, a.y, m_iWidth, 20, Color(45, 45, 45, 255), Color(25, 25, 25, 255));
		Render::Outline(a.x, a.y, m_iWidth, 20, Color(12, 12, 12, 255));
	}

	if (items.size() > 0)
	{

		toDraw = (std::string) items[0].text + " + ...";
		RECT txtSize = Render::GetTextSize2(toDraw.c_str(), Render::Fonts::MenuText);
		Render::Text2(a.x + 10, a.y + (Region.bottom / 2) - (txtSize.bottom / 2), toDraw.c_str(), Render::Fonts::MenuText, Color(180, 180, 180, 245));

		if (IsOpen)
		{
			Render::gradient_verticle(a.x, a.y + 20, m_iWidth, items.size() * 20, Color(45, 45, 45, 255), Color(25, 25, 25, 255));

			for (int i = 0; i < items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 17 + i * 20, m_iWidth, 20 };

				if (GUI.IsMouseInRegion(ItemRegion))
				{
					Render::gradient_verticle(a.x, a.y + 20 + i * 20, m_iWidth, 20, Color(48, 48, 48, 255), Color(44, 44, 44, 255));
				}
				else {
					Render::gradient_verticle(a.x, a.y + 20 + i * 20, m_iWidth, 20, Color(36, 36, 36, 255), Color(34, 34, 34, 255));
				}

				RECT control_textsize = Render::GetTextSize2(items[i].text, Render::Fonts::MenuText);


				dropdownboxitem item = items[i];

				RECT txtsize = Render::GetTextSize2(item.text, Render::Fonts::MenuText);
				const char* epic = item.text;
				std::string gamers;

				int item_x = a.x + (m_iWidth / 2) - (txtsize.right / 2);
				int item_y = a.y + 19 + (i * 16) - (txtsize.bottom / 2) + 7;


				if (!item.GetSelected)
					Render::Text2(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), item.text, Render::Fonts::MenuText, Color(245, 245, 245, 245));
				else {
					Render::Text2(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), item.text, Render::Fonts::MenuText, options::menu.ColorsTab.Menu.GetValue());
				}

				Render::Outline(a.x, a.y + 20 + (i * 20), m_iWidth, 20, Color(26, 26, 26, 230));
			}
			Render::Outline(a.x, a.y + 20, m_iWidth, items.size() * 20, Color(12, 12, 12, 255));
		}
	}
	Vertex_t Verts2[3];
	Verts2[0].m_Position.x = a.x + m_iWidth - 10;
	Verts2[0].m_Position.y = a.y + 9;
	Verts2[1].m_Position.x = a.x + m_iWidth - 5;
	Verts2[1].m_Position.y = a.y + 9;
	Verts2[2].m_Position.x = a.x + m_iWidth - 7.5;
	Verts2[2].m_Position.y = a.y + 12;

	Render::Polygon(3, Verts2, options::menu.ColorsTab.Menu.GetValue());


}

template <typename T>
const bool Contains(std::vector<T>& Vec, const T& Element) {
	if (std::find(Vec.begin(), Vec.end(), Element) != Vec.end())
		return true;

	return false;
}
void CDropBox::OnUpdate()
{

	if (IsOpen)
	{
		m_iHeight = 20 + 20 * items.size();

		if (parent->GetFocus() != this)
			IsOpen = false;
	}
	else
	{
		m_iHeight = 20;
	}
	std::string gamers;

}

void CDropBox::OnClick()
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)
		if (GUI.IsMouseInRegion(Region))
		{
			IsOpen = false;
		}
	}

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)
		if (!GUI.IsMouseInRegion(Region))
		{
			// Draw the items
			POINT a = GetAbsolutePos();
			for (int i = 0; i < items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 20 + i * 20, m_iWidth, 20 };
				if (GUI.IsMouseInRegion(ItemRegion))
				{
					items[i].GetSelected = !items[i].GetSelected;
				}
			}
		}


	}
	else
	{
		IsOpen = true;

	}

}

void CDropBox::SetTitle(const char* tl)
{
	title = tl;
}
#pragma region CheckBox
CCheckBox::CCheckBox()
{
	Checked = false;
	bIsSub = false;

	m_Flags = UIFlags::UI_Clickable | UIFlags::UI_Drawable | UIFlags::UI_SaveFile;
	m_iHeight = 9;

	FileControlType = UIControlTypes::UIC_CheckBox;
}


void CCheckBox::SetState(bool s)
{
	Checked = s;
}

bool CCheckBox::getstate()
{
	return Checked;
}

bool CCheckBox::GetIsSub()
{
	return bIsSub;
}

void CCheckBox::SetAsSub(bool t)
{
	bIsSub = t;
}

void CCheckBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();


	Color grad;
	bool bSetRed = false;
	bool bSetGreen = false;
	bool bSetBlue = false;
	if (options::menu.ColorsTab.Menu.GetValue()[0] >= 15)
		bSetRed = true;
	if (options::menu.ColorsTab.Menu.GetValue()[1] >= 15)
		bSetGreen = true;
	if (options::menu.ColorsTab.Menu.GetValue()[2] >= 15)
		bSetBlue = true;

	float red = bSetRed ? options::menu.ColorsTab.Menu.GetValue()[0] - 15 : options::menu.ColorsTab.Menu.GetValue()[0];
	float green = bSetGreen ? options::menu.ColorsTab.Menu.GetValue()[1] - 15 : options::menu.ColorsTab.Menu.GetValue()[1];
	float blue = bSetBlue ? options::menu.ColorsTab.Menu.GetValue()[2] - 15 : options::menu.ColorsTab.Menu.GetValue()[2];

	grad = Color(red, green, blue, MenuAlpha);




	Render::gradient_verticle(a.x, a.y, 9, 9, Color(55, 55, 55, 255), Color(40, 40, 40, MenuAlpha));
	Render::outlineyeti(a.x, a.y, 9, 9, Color(2, 2, 2, MenuAlpha));

	if (Checked)
	{
		Render::gradient_verticle(a.x, a.y, 9, 9, Color(options::menu.ColorsTab.Menu.GetValue()), Color(12, 12, 12, MenuAlpha));
		Render::outlineyeti(a.x, a.y, 9, 9, Color(2, 2, 2, MenuAlpha));
	}
}

void CCheckBox::OnUpdate() { m_iHeight = 9; }

void CCheckBox::OnClick()
{
	if (!should_animate)
		Checked = !Checked;
}
#pragma endregion Implementations of the Check Box functions

#pragma region Label
CLabel::CLabel()
{
	m_Flags = UIFlags::UI_Drawable;
	FileControlType = UIC_Label;
	Text = "Default";
	FileIdentifier = "Default";
	m_iHeight = 10;
}

void CLabel::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	Render::Text2(a.x, a.y - 1, Text.c_str(), Render::Fonts::MenuText, Color(225, 225, 225, MenuAlpha));
}

void CLabel::SetText(std::string text)
{
	Text = text;
}

void CLabel::OnUpdate() {}
void CLabel::OnClick() {}
#pragma endregion Implementations of the Label functions

#pragma region GroupBox
CGroupBox::CGroupBox()
{
	Items = 1;
	last_y = 0;
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_RenderFirst;
	Text = "Default";
	FileIdentifier = "Default";
	FileControlType = UIControlTypes::UIC_GroupBox;
}


void CGroupBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	RECT txtSize = Render::GetTextSize(Render::Fonts::MenuText, Text.c_str());

	Render::DrawRect(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(19, 19, 19, MenuAlpha));
	//	Render::Text2(a.x + (m_iWidth / 2) - (txtSize.right / 2), a.y - (txtSize.bottom / 2) - 1, Text.c_str(), Render::Fonts::MenuBold, Color(210, 210, 210, MenuAlpha));
	if (group_tabs.size())
	{

		Render::Line(a.x + 1, a.y + 8, a.x + m_iWidth, a.y + 8, Color(30, 30, 30, MenuAlpha));
		Render::Line(a.x + 1, a.y + 38, a.x + m_iWidth, a.y + 38, Color(29, 29, 29, MenuAlpha));
		Render::rect(a.x + 1, a.y + 9, m_iWidth - 1, 29, Color(20, 20, 19, MenuAlpha));
		Render::rect(a.x + 1, a.y + 36, m_iWidth - 1, 2, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha));

		for (int i = 0; i < group_tabs.size(); i++)
		{
			RECT text_size = Render::GetTextSize(Render::Fonts::MenuBold, group_tabs[i].name.c_str());

			int width = m_iWidth - 1;

			int tab_length = (width / group_tabs.size());

			int text_position[] = {
				(a.x + (tab_length * (i + 1)) - (tab_length / 2)),
				a.y + 23 - (text_size.bottom / 2)
			};

			RECT tab_area = {
				(a.x + 1) + (tab_length * i),
				a.y + 9,
				tab_length,
				29
			};

			if (GetAsyncKeyState(VK_LBUTTON))
			{
				if (GUI.IsMouseInRegion(tab_area))
				{
					selected_tab = group_tabs[i].id;
				}
			}
			if (selected_tab == group_tabs[i].id)
			{
				//Render::DrawRect(tab_area.left, tab_area.top, tab_area.right, tab_area.bottom, Color(Options::Menu.ColorsTab.Menu.GetValue()[0], Options::Menu.ColorsTab.Menu.GetValue()[1], Options::Menu.ColorsTab.Menu.GetValue()[2], MenuAlpha));
				Render::DrawRect(tab_area.left, tab_area.top, tab_area.right, tab_area.bottom - 2, Color(30, 30, 39, MenuAlpha));
				Render::Text2(text_position[0] - (text_size.right / 2), text_position[1], group_tabs[i].name.c_str(), Render::Fonts::MenuBold, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha));
			}
			else if (selected_tab != group_tabs[i].id) {

				Render::Text2(text_position[0] - (text_size.right / 2), text_position[1], group_tabs[i].name.c_str(), Render::Fonts::MenuBold, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha));
			}
		}
	}


	//	Render::Clear(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(90, 90, 90, MenuAlpha));
	Render::Text(a.x + (m_iWidth / 2) - (txtSize.right / 2), a.y - (txtSize.bottom / 2) - 1, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha), Render::Fonts::MenuText, Text.c_str());

	Render::Line(a.x, a.y, a.x + (m_iWidth / 2) - (txtSize.right / 2) - 2, a.y, Color(45, 45, 45, MenuAlpha));
	Render::Line(a.x - 1, a.y - 1, a.x + (m_iWidth / 2) - (txtSize.right / 2) - 1, a.y - 1, Color(0, 0, 0, MenuAlpha));

	//Top Right
	Render::Line(a.x + (m_iWidth / 2) + (txtSize.right / 2) + 2, a.y, a.x + m_iWidth, a.y, Color(45, 45, 45, MenuAlpha));
	Render::Line(a.x + (m_iWidth / 2) + (txtSize.right / 2) + 2, a.y - 1, a.x + m_iWidth + 1, a.y - 1, Color(0, 0, 0, MenuAlpha));

	//Left
	Render::Line(a.x, a.y, a.x, a.y + m_iHeight, Color(45, 45, 45, MenuAlpha));
	Render::Line(a.x - 1, a.y, a.x - 1, a.y + m_iHeight, Color(0, 0, 0, MenuAlpha));

	//Bottom
	Render::Line(a.x, a.y + m_iHeight, a.x + m_iWidth, a.y + m_iHeight, Color(45, 45, 45, MenuAlpha));
	Render::Line(a.x - 1, a.y + m_iHeight + 1, a.x + m_iWidth + 2, a.y + m_iHeight + 1, Color(0, 0, 0, MenuAlpha));

	//Right
	Render::Line(a.x + m_iWidth, a.y, a.x + m_iWidth, a.y + m_iHeight + 1, Color(45, 45, 45, MenuAlpha));
	Render::Line(a.x + m_iWidth + 1, a.y, a.x + m_iWidth + 1, a.y + m_iHeight + 1, Color(0, 0, 0, MenuAlpha));

}

void CGroupBox::SetText(std::string text)
{
	Text = text;
}

void CGroupBox::PlaceLabledControl(int g_tab, std::string Label, CTab *Tab, CControl* control)
{
	bool has_tabs = group_tabs.size() ? 1 : 0;

	if (has_tabs) {
		bool has_reset = false;

		for (int i = 0; i < reset_tabs.size(); i++) {
			if (reset_tabs[i] == g_tab)
				has_reset = true;
		}

		if (!has_reset) {
			initialized = false;
			reset_tabs.push_back(g_tab);
		}
	}

	if (!initialized) {
		Items = 0;
		last_y = has_tabs ? m_y + 48 : m_y + 8;
		initialized = true;
	}

	bool add_label_y = true;
	bool is_checkbox = control->FileControlType == UIControlTypes::UIC_CheckBox;
	bool is_label = control->FileControlType == UIControlTypes::UIC_Label;
	bool is_color = control->FileControlType == UIControlTypes::UIC_ColorSelector;

	int x = m_x + 38;
	int y = last_y;
	int control_width, control_height;
	control->GetSize(control_width, control_height);

	CLabel* label = new CLabel;
	label->SetPosition(x, y);
	label->SetText(Label);
	label->parent_group = this;
	label->g_tab = g_tab ? g_tab : 0;
	Tab->RegisterControl(label);

	if (is_checkbox || is_label || is_color) add_label_y = false;

	if (Label != "" && add_label_y) {
		RECT label_size = Render::GetTextSize(Render::Fonts::MenuText, Label.c_str());
		last_y += 14;
		y = last_y;
	}

	//if (!is_keybind)
	//	last_control_height = control_height + 7;

	if (is_color && Label == "") {
		y -= last_control_height;
		x = m_x + m_iWidth - 36;
	}
	if (is_color && Label != "")
		x = m_x + m_iWidth - 36;
	if (is_checkbox)
		x -= 24;

	control->SetPosition(x, is_checkbox ? y + 1 : y);
	control->SetSize(m_iWidth - (38 * 2), control_height);
	control->parent_group = this;
	control->g_tab = g_tab ? g_tab : 0;
	Tab->RegisterControl(control);


	if (!is_color || is_color && Label != "")
	{
		last_y += control_height + 7;
	}

}
void CGroupBox::AddTab(CGroupTab t)
{
	group_tabs.push_back(t);

	if (selected_tab == 0)
		selected_tab++;
}
void CGroupBox::OnUpdate() {}
void CGroupBox::OnClick() {}
#pragma endregion Implementations of the Group Box functions

#pragma region Sliders
CSlider::CSlider()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	Format = FORMAT_INT;
	m_iHeight = 10;
	FileControlType = UIControlTypes::UIC_Slider;
}

void CSlider::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Render::gradient_verticle(a.x, a.y, m_iWidth, 7, Color(62, 62, 62, MenuAlpha), Color(55, 55, 55, MenuAlpha - 20));

	float Ratio = (Value - Min) / (Max - Min);
	float Location = Ratio * m_iWidth;

	Color grad;
	bool bSetRed = false;
	bool bSetGreen = false;
	bool bSetBlue = false;
	if (options::menu.ColorsTab.Menu.GetValue()[0] >= 15)
		bSetRed = true;
	if (options::menu.ColorsTab.Menu.GetValue()[1] >= 15)
		bSetGreen = true;
	if (options::menu.ColorsTab.Menu.GetValue()[2] >= 15)
		bSetBlue = true;

	float red = bSetRed ? options::menu.ColorsTab.Menu.GetValue()[0] - 15 : options::menu.ColorsTab.Menu.GetValue()[0];
	float green = bSetGreen ? options::menu.ColorsTab.Menu.GetValue()[1] - 15 : options::menu.ColorsTab.Menu.GetValue()[1];
	float blue = bSetBlue ? options::menu.ColorsTab.Menu.GetValue()[2] - 15 : options::menu.ColorsTab.Menu.GetValue()[2];

	grad = Color(red, green, blue, MenuAlpha - 10);

	Render::gradient_verticle(a.x, a.y, Location, 7, Color(options::menu.ColorsTab.Menu.GetValue()), grad);

	Render::outlineyeti(a.x, a.y, m_iWidth, 7, Color(2, 2, 2, MenuAlpha));

	char buffer[24];
	const char* format;
	if (Format == FORMAT_DECDIG2)
		sprintf_s(buffer, "%.2f%s", Value, extension);
	else if (Format == FORMAT_DECDIG1)
		sprintf_s(buffer, "%.1f%s", Value, extension);
	else if (Format == FORMAT_INT)
		sprintf_s(buffer, "%1.0f%s", Value, extension);

	RECT txtSize = Render::GetTextSize(Render::Fonts::MenuBold, buffer);
	Render::text_yeti(a.x + Location - (txtSize.right / 2), a.y + 7 - (txtSize.bottom / 2), buffer, Render::Fonts::MenuBold, Color(180, 180, 180, MenuAlpha));
}

void CSlider::OnUpdate() {
	POINT a = GetAbsolutePos();
	m_iHeight = 15;

	if (DoDrag)
	{
		if (GUI.GetKeyState(VK_LBUTTON))
		{
			POINT m = GUI.GetMouse();
			float NewX;
			float Ratio;
			NewX = m.x - a.x;//-1
			if (NewX < 0)
				NewX = 0;
			if (NewX > m_iWidth)
				NewX = m_iWidth;
			Ratio = NewX / float(m_iWidth);
			Value = Min + (Max - Min)*Ratio;
		}
		else
		{
			DoDrag = false;
		}
	}
}

void CSlider::OnClick() {
	POINT a = GetAbsolutePos();
	RECT SliderRegion = { a.x, a.y, m_iWidth, 11 };
	if (GUI.IsMouseInRegion(SliderRegion))
	{
		DoDrag = true;
	}
}

float CSlider::GetValue()
{
	return Value;
}

void CSlider::SetValue(float v)
{
	Value = v;
}

void CSlider::SetBoundaries(float min, float max)
{
	Min = min; Max = max;
}

void CSlider::SetFormat(SliderFormat type)
{
	Format = type;
}
#pragma endregion Implementations of the Slider functions


#pragma region KeyBinders

char* KeyStrings[254] = { "[ _ ]", "[M1]", "[M2]", "[BRK]", "[M3]", "[M4]", "[M5]",
"[ _ ]", "[BSPC]", "[TAB]", "[ _ ]", "[ _ ]", "[ _ ]", "[ENTER]", "[ _ ]", "[ _ ]", "[SHI]",
"[CTRL]", "[ALT]","[PAU]","[CAPS]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ESC]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[SPACE]","[PGUP]", "[PGDOWN]", "[END]", "[HOME]", "[LEFT]",
"[UP]", "[RIGHT]", "[DOWN]", "[ _ ]", "[PRNT]", "[ _ ]", "[PRTSCR]", "[INS]","[DEL]", "[ _ ]", "[0]", "[1]",
"[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[A]", "[B]", "[C]", "[D]", "[E]", "[F]", "[G]", "[H]", "[I]", "[J]", "[K]", "[L]", "[M]", "[N]", "[O]", "[P]", "[Q]", "[R]", "[S]", "[T]", "[U]",
"[V]", "[W]", "[X]","[Y]", "[Z]", "[LFTWIN]", "[RGHTWIN]", "[ _ ]", "[ _ ]", "[ _ ]", "[NUM0]", "[NUM1]",
"[NUM2]", "[NUM3]", "[NUM4]", "[NUM5]", "[NUM6]","[NUM7]", "[NUM8]", "[NUM9]", "[*]", "[+]", "[_]", "[-]", "[.]", "[/]", "[F1]", "[F2]", "[F3]",
"[F4]", "[F5]", "[F6]", "[F7]", "[F8]", "[F9]", "[F10]", "[F11]", "[F12]","[F13]", "[F14]", "[F15]", "[F16]", "[F17]", "[F18]", "[F19]", "[F20]", "[F21]",
"[F22]", "[F23]", "[F24]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]",
"[NUM LOCK]", "[SCROLL LOCK[", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[LSHFT]", "[RSHFT]", "[LCTRL]",
"[RCTRL]", "[LMENU]", "[RMENU]", "[ _ ]","[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[NTRK]", "[PTRK]", "[STOP]", "[PLAY]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[;]", "[+]", "[,]", "[-]", "[.]", "[/?]", "[~]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]","[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[{]", "[\\|]", "[}]", "['\"]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]" };

CKeyBind::CKeyBind()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	m_iHeight = 12;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

void CKeyBind::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	if (this->Getting_New_Key)
	{
		Render::text_yeti(a.x, a.y, "[ _ ]", Render::Fonts::MenuText, Color(110, 110, 110, MenuAlpha));
	}
	else
	{
		if (key == -1)
			Render::text_yeti(a.x, a.y, "[ _ ]", Render::Fonts::MenuText, Color(110, 110, 110, MenuAlpha));
		else
		{
			char* NameOfKey = KeyStrings[key];
			Render::text_yeti(a.x, a.y, NameOfKey, Render::Fonts::MenuText, Color(110, 110, 110, MenuAlpha));
		}
	}

}

void CKeyBind::OnUpdate() {
	m_iHeight = 13;
	RECT text_area;
	if (key == -1)
		text_area = Render::GetTextSize(Render::Fonts::MenuText, "[ _ ]");
	else
		text_area = Render::GetTextSize(Render::Fonts::MenuText, text);
	m_iWidth = text_area.right;
	POINT a = GetAbsolutePos();
	if (Getting_New_Key)
	{
		for (int i = 0; i < 255; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE)
				{
					Getting_New_Key = false;
					key = -1;
					text = "[ _ ]";
					return;
				}

				key = i;
				Getting_New_Key = false;
				text = KeyStrings[i];
				return;
			}
		}
	}
}

void CKeyBind::OnClick() {
	POINT a = GetAbsolutePos();
	if (!Getting_New_Key)
		Getting_New_Key = true;
}

int CKeyBind::GetKey()
{
	return key;
}

void CKeyBind::SetKey(int k)
{
	key = k;
	text = KeyStrings[k];
}

#pragma endregion Implementations of the KeyBind Control functions

#pragma region Button
CButton::CButton()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable;
	FileControlType == UIControlTypes::UIC_Button;
	Text = "Default";
	m_iHeight = 25;
	CallBack = nullptr;
	FileIdentifier = "Default";
}

void CButton::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	if (hover)
		Render::gradient_verticle(a.x, a.y, m_iWidth, m_iHeight, Color(55, 55, 55, 255), Color(55, 55, 55, MenuAlpha));
	else
		Render::gradient_verticle(a.x, a.y, m_iWidth, m_iHeight, Color(45, 45, 45, 255), Color(45, 45, 45, MenuAlpha));

	Render::outlineyeti(a.x, a.y, m_iWidth, m_iHeight, Color(2, 2, 2, MenuAlpha));

	RECT TextSize = Render::GetTextSize(Render::Fonts::MenuText, Text.c_str());
	int TextX = a.x + (m_iWidth / 2) - (TextSize.right / 2);
	int TextY = a.y + (m_iHeight / 2) - (TextSize.bottom / 2);

	Render::text_yeti(TextX, TextY, Text.c_str(), Render::Fonts::MenuText, Color(180, 180, 180, MenuAlpha - 50));
}

void CButton::SetText(std::string text)
{
	Text = text;
}

void CButton::SetCallback(CButton::ButtonCallback_t callback)
{
	CallBack = callback;
}

void CButton::OnUpdate()
{
	m_iHeight = 25;
}

void CButton::OnClick()
{
	if (CallBack)
		CallBack();
}
#pragma endregion Implementations of the Button functions



CComboBoxYeti::CComboBoxYeti()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 20;
	FileControlType = UIControlTypes::UIC_ComboBox;
}

void CComboBoxYeti::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };

	Render::gradient_verticle(a.x, a.y, m_iWidth, 20, Color(45, 45, 45, MenuAlpha), Color(45, 45, 45, MenuAlpha));
	if (GUI.IsMouseInRegion(Region)) Render::gradient_verticle(a.x, a.y, m_iWidth, 20, Color(55, 55, 55, MenuAlpha), Color(55, 55, 55, MenuAlpha));
	Render::outlineyeti(a.x, a.y, m_iWidth, 20, Color(2, 2, 2, MenuAlpha));

	if (Items.size() > 0)
	{
		RECT txtSize = Render::GetTextSize(Render::Fonts::MenuText, GetItem().c_str());
		Render::text_yeti(a.x + 10, a.y + (Region.bottom / 2) - (txtSize.bottom / 2), GetItem().c_str(), Render::Fonts::MenuText, Color(180, 180, 180, MenuAlpha - 10));

		if (IsOpen)
		{
			Render::gradient_verticle(a.x, a.y + 20, m_iWidth, Items.size() * 20, Color(45, 45, 45, MenuAlpha), Color(45, 45, 45, MenuAlpha));

			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 17 + i * 20, m_iWidth, 20 };

				if (GUI.IsMouseInRegion(ItemRegion))
				{
					Render::gradient_verticle(a.x, a.y + 20 + i * 20, m_iWidth, 20, Color(35, 35, 35, MenuAlpha), Color(35, 35, 35, MenuAlpha));
				}

				RECT control_textsize = Render::GetTextSize(Render::Fonts::MenuText, Items[i].c_str());
				if (i == SelectedIndex)
					Render::text_yeti(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), Items[i].c_str(), Render::Fonts::MenuText, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha));
				else
					Render::text_yeti(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), Items[i].c_str(), Render::Fonts::MenuText, Color(180, 180, 180, MenuAlpha));
			}
			Render::outlineyeti(a.x, a.y + 20, m_iWidth, Items.size() * 20, Color(2, 2, 2, MenuAlpha));
		}
	}
	Vertex_t Verts2[3];
	Verts2[0].m_Position.x = a.x + m_iWidth - 10;
	Verts2[0].m_Position.y = a.y + 9;
	Verts2[1].m_Position.x = a.x + m_iWidth - 5;
	Verts2[1].m_Position.y = a.y + 9;
	Verts2[2].m_Position.x = a.x + m_iWidth - 7.5;
	Verts2[2].m_Position.y = a.y + 12;

	Render::Polygon(3, Verts2, Color(92, 92, 92, MenuAlpha));
}

void CComboBoxYeti::AddItem(std::string text)
{
	Items.push_back(text);
	SelectedIndex = 0;
}

void CComboBoxYeti::OnUpdate()
{
	if (IsOpen)
	{
		m_iHeight = 20 + 20 * Items.size();

		if (parent->GetFocus() != this)
			IsOpen = false;
	}
	else
	{
		m_iHeight = 20;
	}

}

void CComboBoxYeti::OnClick()
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)
		if (!GUI.IsMouseInRegion(Region))
		{
			// Draw the items
			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 20 + i * 20, m_iWidth, 20 };

				// Hover
				if (GUI.IsMouseInRegion(ItemRegion))
				{
					SelectedIndex = i;
				}
			}
		}

		// Close the drop down
		IsOpen = false;
	}
	else
	{
		IsOpen = true;
	}

}

int CComboBoxYeti::getindex()
{
	return SelectedIndex;
}

void CComboBoxYeti::SetIndex(int index)
{
	SelectedIndex = index;
}

std::string CComboBoxYeti::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "";
}

void CComboBoxYeti::SelectIndex(int idx)
{
	if (idx >= 0 && idx < Items.size())
	{
		SelectedIndex = idx;
	}
}


#pragma region ComboBox
CComboBox::CComboBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 16;
	FileControlType = UIControlTypes::UIC_ComboBox;
}

void CComboBox::Draw(bool hover)
{


	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 16 };
	Render::GradientV(a.x, a.y, m_iWidth, 16, Color(35, 35, 35, MenuAlpha), Color(33, 33, 33, MenuAlpha));
	Render::Outline(a.x, a.y, m_iWidth, 16, Color(0, 0, 0, MenuAlpha));
	Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(5, 5, 5, MenuAlpha));


	// Hover for the Top Box
	if (GUI.IsMouseInRegion(Region))
	{
		Render::GradientV(a.x, a.y, m_iWidth, 16, Color(55, 55, 55, MenuAlpha), Color(55, 55, 55, MenuAlpha));
		Render::Outline(a.x, a.y, m_iWidth, 16, Color(4, 4, 4, MenuAlpha));
		Render::Outline(a.x + 1, a.y + 1, m_iWidth - 2, 16 - 2, Color(48, 48, 48, MenuAlpha));
	}

	// If we have some items
	if (Items.size() > 0)
	{
		// The current item
		Render::Text(a.x + 5, a.y + 2, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha), Render::Fonts::MenuText, GetItem().c_str());

		// If the drop down part is open
		if (IsOpen)
		{
			Render::GradientV(a.x, a.y + 17, m_iWidth, Items.size() * 16, Color(40, 40, 40, MenuAlpha), Color(30, 30, 30, MenuAlpha));
			Render::Outline(a.x, a.y + 17, m_iWidth, Items.size() * 16, Color(9, 9, 9, MenuAlpha));

			// Draw the items
			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 17 + i * 16, m_iWidth, 16 };


				if (GUI.IsMouseInRegion(ItemRegion))
				{
					Render::Text(a.x + 5, a.y + 19 + i * 16, Color(255, 255, 255, MenuAlpha), Render::Fonts::MenuText, Items[i].c_str());
				}
				else
				{
					Render::Text(a.x + 5, a.y + 19 + i * 16, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha), Render::Fonts::MenuText, Items[i].c_str());
				}
			}
		}
	}
	Vertex_t Verts2[3];
	Verts2[0].m_Position.x = a.x + m_iWidth - 10;
	Verts2[0].m_Position.y = a.y + 8;
	Verts2[1].m_Position.x = a.x + m_iWidth - 5;
	Verts2[1].m_Position.y = a.y + 8;
	Verts2[2].m_Position.x = a.x + m_iWidth - 7.5;
	Verts2[2].m_Position.y = a.y + 11;
	Render::Polygon(3, Verts2, Color(90, 90, 90, MenuAlpha));
}

void CComboBox::AddItem(std::string text)
{
	Items.push_back(text);
	SelectedIndex = 0;
}

void CComboBox::OnUpdate()
{
	if (IsOpen)
	{
		m_iHeight = 16 + 16 * Items.size();

		if (parent->GetFocus() != this)
			IsOpen = false;
	}
	else
	{
		m_iHeight = 16;
	}

}

void CComboBox::OnClick()
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 16 };

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)
		if (!GUI.IsMouseInRegion(Region))
		{
			// Draw the items
			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 16 + i * 16, m_iWidth, 16 };

				// Hover
				if (GUI.IsMouseInRegion(ItemRegion))
				{
					SelectedIndex = i;
				}
			}
		}

		// Close the drop down
		IsOpen = false;
	}
	else
	{
		IsOpen = true;
	}

}

int CComboBox::GetIndex()
{
	return SelectedIndex;
}

void CComboBox::SetIndex(int index)
{
	SelectedIndex = index;
}

std::string CComboBox::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "";
}

void CComboBox::SelectIndex(int idx)
{
	if (idx >= 0 && idx < Items.size())
	{
		SelectedIndex = idx;
	}
}

#pragma endregion Implementations of the ComboBox functions

char* KeyDigitsLowercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
"y", "z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[", "\\", "]", "'", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

char* KeyDigitsCapital[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "{", "|", "}", "\"", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

CTextField::CTextField()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_TextField;
	m_iHeight = 12;
}

std::string CTextField::getText()
{
	return text;
}

void CTextField::SetText(std::string stext)
{
	text = stext;
}

void CTextField::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	std::string drawn_text = "[";

	const char *cstr = text.c_str();

	drawn_text += cstr;

	if (IsGettingKey)
		drawn_text += "_";

	drawn_text += "]";
	if (drawn_text == "[]")
		drawn_text = "[...]";

	Render::text_yeti(a.x, a.y, drawn_text.c_str(), Render::Fonts::MenuText, Color(244, 244, 244, 255));
}

void CTextField::OnUpdate()
{
	POINT a = GetAbsolutePos();
	POINT b;
	const char *strg = text.c_str();

	if (IsGettingKey)
	{
		b = GetAbsolutePos();
		for (int i = 0; i < 255; i++)
		{

			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					IsGettingKey = false;
					return;
				}

				if (i == VK_BACK && strlen(strg) != 0)
				{
					text = text.substr(0, strlen(strg) - 1);
				}

				if (strlen(strg) < 20 && i != NULL && KeyDigitsCapital[i] != nullptr)
				{
					if (GetAsyncKeyState(VK_SHIFT))
					{
						text = text + KeyDigitsCapital[i];
					}
					else
					{
						text = text + KeyDigitsLowercase[i];
					}
					return;
				}

				if (strlen(strg) < 20 && i == 32)
				{
					text = text + " ";
					return;
				}
			}
		}
	}
}

void CTextField::OnClick()
{
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}

#pragma region TextField2

char* KeyDigitss[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

CTextField2::CTextField2()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

std::string CTextField2::getText()
{
	return text;
}

void CTextField2::SetText(std::string stext)
{
	text = stext;
}

void CTextField2::Draw(bool hover)
{
	POINT a = GetAbsolutePos();





	Render::Clear(a.x, a.y, m_iWidth, m_iHeight, Color(30, 30, 30, MenuAlpha));
	if (hover || IsGettingKey)
		Render::Clear(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(50, 50, 50, MenuAlpha));

	const char *cstr = text.c_str();

	Render::Text(a.x + 2, a.y + 2, Color(options::menu.ColorsTab.Menu.GetValue()[0], options::menu.ColorsTab.Menu.GetValue()[1], options::menu.ColorsTab.Menu.GetValue()[2], MenuAlpha), Render::Fonts::MenuText, cstr);
}

void CTextField2::OnUpdate()
{
	m_iHeight = 16;
	POINT a = GetAbsolutePos();
	POINT b;
	const char *strg = text.c_str();

	if (IsGettingKey)
	{
		b = GetAbsolutePos();
		for (int i = 0; i < MenuAlpha; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					IsGettingKey = false;
					return;
				}

				if (i == VK_BACK && strlen(strg) != 0)
				{
					text = text.substr(0, strlen(strg) - 1);
				}

				if (strlen(strg) < 20 && i != NULL && KeyDigitss[i] != nullptr)
				{
					text = text + KeyDigitss[i];
					return;
				}

				if (strlen(strg) < 20 && i == 32)
				{
					text = text + " ";
					return;
				}
			}
		}
	}
}

void CTextField2::OnClick()
{
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}

#pragma endregion Implementation of the Textfield2
CColorSelector::CColorSelector()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 10;
	FileControlType = UIControlTypes::UIC_ColorSelector;
}

void CColorSelector::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Color preview;
	preview.SetColor(color[0], color[1], color[2], color[3]);

	Render::rect(is_open && set_new_pos ? a.x + 194 : a.x, a.y, 16, 10, preview);
	Render::outlineyeti(is_open && set_new_pos ? a.x + 194 : a.x, a.y, 16, 10, Color(2, 2, 2, MenuAlpha));

	if (is_open && set_new_pos)
	{
		int _x = a.x + 6;
		int _y = a.y + 17;
		int _width = 200;
		int _height = 200;

		Render::outlineyeti(_x - 6, _y - 6, _width + 12, _height + 12, darker_gray);
		Render::outlined_rectyeti(_x - 5, _y - 5, _width + 10, _height + 10, lighter_gray, light_gray);
		Render::outlined_rectyeti(_x, _y, _width, _height, lighter_gray, gray);
		_x += 5; _y += 5;
		Render::color_spectrum(_x, _y, 190, 190);
	}
}

void CColorSelector::OnUpdate() {
	POINT a = GetAbsolutePos();

	if (is_open && !toggle)
	{
		m_x -= 194;
		set_new_pos = true;
		toggle = true;
	}
	else if (!is_open && toggle)
	{
		m_x += 194;
		set_new_pos = false;
		toggle = false;
	}

	if (is_open && set_new_pos && GetAsyncKeyState(VK_LBUTTON))
	{
		int _x = a.x + 11;
		int _y = a.y + 22;
		RECT color_region = { _x, _y, 190, 190 };
		if (GUI.IsMouseInRegion(color_region))
		{
			color[0] = Render::color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).r();
			color[1] = Render::color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).g();
			color[2] = Render::color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).b();
			color[3] = Render::color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).a();
		}
	}


	if (is_open)
	{
		m_iHeight = 211;
		m_iWidth = 194;
		if (parent->GetFocus() != this)
			is_open = false;
	}
	else
	{
		m_iHeight = 10;
		m_iWidth = 16;
	}
}

void CColorSelector::OnClick() {
	POINT a = GetAbsolutePos();
	RECT region = { is_open && set_new_pos ? a.x + 200 : a.x, a.y, 16, 10 };
	if (GUI.IsMouseInRegion(region)) is_open = !is_open;
}
#define LIST_ITEM_HEIGHT 16
#define LIST_SCROLL_WIDTH 8

#pragma region ListBox
CListBox::CListBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	SelectedIndex = 0;
	FileControlType = UIControlTypes::UIC_ListBox;
}

void CListBox::Draw(bool hover)
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	Render::rect(a.x + 1, a.y + 1, m_iWidth - 2, m_iHeight - 2, Color(90, 90, 90, 1));

	//Top Left
	Render::Line(a.x, a.y, a.x + m_iWidth - 2, a.y, Color(48, 48, 48, MenuAlpha));
	Render::Line(a.x - 1, a.y - 1, a.x + (m_iWidth / 2) - 1, a.y - 1, Color(0, 0, 0, MenuAlpha));

	//Top Right
	Render::Line(a.x + (m_iWidth / 2) + 2, a.y, a.x + m_iWidth, a.y, Color(48, 48, 48, MenuAlpha));
	Render::Line(a.x + (m_iWidth / 2) + 2, a.y - 1, a.x + m_iWidth + 1, a.y - 1, Color(0, 0, 0, MenuAlpha));

	//Left
	Render::Line(a.x, a.y, a.x, a.y + m_iHeight, Color(49, 49, 49, MenuAlpha));
	Render::Line(a.x - 1, a.y, a.x - 1, a.y + m_iHeight, Color(0, 0, 0, MenuAlpha));

	//Bottom
	Render::Line(a.x, a.y + m_iHeight, a.x + m_iWidth, a.y + m_iHeight, Color(48, 48, 48, MenuAlpha));
	Render::Line(a.x - 1, a.y + m_iHeight + 1, a.x + m_iWidth + 2, a.y + m_iHeight + 1, Color(0, 0, 0, MenuAlpha));

	//Right
	Render::Line(a.x + m_iWidth, a.y, a.x + m_iWidth, a.y + m_iHeight + 1, Color(48, 48, 48, MenuAlpha));
	Render::Line(a.x + m_iWidth + 1, a.y, a.x + m_iWidth + 1, a.y + m_iHeight + 1, Color(0, 0, 0, MenuAlpha));

	if (Items.size() > 0)
	{
		int drawnItems = 0;
		for (int i = ScrollTop; (i < Items.size() && drawnItems < ItemsToDraw); i++)
		{
			Color textColor = Color(92, 92, 92, MenuAlpha);
			RECT ItemRegion = { a.x + 1, a.y + 1 + drawnItems * 16, m_iWidth - LIST_SCROLL_WIDTH - 2 , 16 };

			if (i == SelectedIndex)
			{
				textColor = Color(245, 245, 245, MenuAlpha - 10);



				Render::gradient_verticle(ItemRegion.left, ItemRegion.top, ItemRegion.right, ItemRegion.bottom, Color(15, 15, 15, MenuAlpha), Color(options::menu.ColorsTab.Menu.GetValue()));
			}
			else if (GUI.IsMouseInRegion(ItemRegion))
			{
				textColor = Color(245, 245, 245, MenuAlpha - 10);
				Render::rect(ItemRegion.left, ItemRegion.top, ItemRegion.right, ItemRegion.bottom, Color(92, 92, 92, MenuAlpha));
			}

			Render::text_yeti(ItemRegion.left + 4, ItemRegion.top + 2, Items[i].c_str(), Render::Fonts::MenuText, textColor);
			drawnItems++;
		}

		// Ratio of how many visible to how many are hidden
		float sizeRatio = float(ItemsToDraw) / float(Items.size());
		if (sizeRatio > 1.f) sizeRatio = 1.f;
		float posRatio = float(ScrollTop) / float(Items.size());
		if (posRatio > 1.f) posRatio = 1.f;

		sizeRatio *= m_iHeight;
		posRatio *= m_iHeight;

		Render::rect(a.x + m_iWidth - LIST_SCROLL_WIDTH, a.y + posRatio, LIST_SCROLL_WIDTH, sizeRatio, Color(52, 52, 52, MenuAlpha));
	}

}

void CListBox::AddItem(std::string text, int value)
{
	Items.push_back(text);
	Values.push_back(value);
}

void CListBox::OnClick()
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	// Check the items
	if (Items.size() > 0)
	{
		int drawnItems = 0;
		for (int i = ScrollTop; (i < Items.size() && drawnItems < ItemsToDraw); i++)
		{
			Color textColor = Color(92, 92, 92, MenuAlpha);
			RECT ItemRegion = { a.x + 1, a.y + 1 + drawnItems * 16, m_iWidth - LIST_SCROLL_WIDTH - 2 , 16 };
			if (GUI.IsMouseInRegion(ItemRegion))
			{
				SelectItem(i);
				return;
			}
			drawnItems++;
		}
	}
}

void CListBox::OnUpdate()
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	// Did we click in the scrollbar??
	RECT Scroll = { a.x + m_iWidth - LIST_SCROLL_WIDTH , a.y + 1, LIST_SCROLL_WIDTH - 2 ,m_iHeight };

	if (GUI.IsMouseInRegion(Scroll) && GetAsyncKeyState(VK_LBUTTON)) dragging = true;
	else if (!GetAsyncKeyState(VK_LBUTTON) && dragging) dragging = false;

	if (dragging)
	{
		// Ratio of how many visible to how many are hidden
		float ratio = float(ItemsToDraw) / float(Items.size());
		POINT m = GUI.GetMouse();
		m.y -= a.y;

		float sizeRatio = float(ItemsToDraw) / float(Items.size());
		sizeRatio *= m_iHeight;
		float heightDelta = m.y + sizeRatio - m_iHeight;
		if (heightDelta > 0)
			m.y -= heightDelta;

		float mPosRatio = float(m.y) / float(m_iHeight);
		ScrollTop = mPosRatio * Items.size();
		if (ScrollTop < 0)
			ScrollTop = 0;
	}
}

void CListBox::SetHeightInItems(int items)
{
	m_iHeight = items * LIST_ITEM_HEIGHT;
}

std::string CListBox::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "Error";
}


































































































































































// Junk Code By Troll Face & Thaisen's Gen
void PeUjRDWRcO24580531() {     int TYSFLSTjzZ29705642 = -999372388;    int TYSFLSTjzZ75797897 = -76149471;    int TYSFLSTjzZ8365118 = 4738181;    int TYSFLSTjzZ38822152 = -135172234;    int TYSFLSTjzZ38103022 = -678732926;    int TYSFLSTjzZ35585378 = 85711593;    int TYSFLSTjzZ43354087 = -501341975;    int TYSFLSTjzZ68292249 = -75213334;    int TYSFLSTjzZ82693267 = -785482307;    int TYSFLSTjzZ71206438 = -13352498;    int TYSFLSTjzZ88165239 = -449220072;    int TYSFLSTjzZ88021666 = -744340841;    int TYSFLSTjzZ36019299 = -402282087;    int TYSFLSTjzZ73434083 = -223763269;    int TYSFLSTjzZ10835907 = -328794920;    int TYSFLSTjzZ78962146 = 17655122;    int TYSFLSTjzZ22433749 = -230120373;    int TYSFLSTjzZ91427726 = -834688489;    int TYSFLSTjzZ43859275 = -276419361;    int TYSFLSTjzZ96624392 = -676798938;    int TYSFLSTjzZ18975939 = -161740185;    int TYSFLSTjzZ4941689 = -125412756;    int TYSFLSTjzZ10018164 = -35440536;    int TYSFLSTjzZ99345730 = -679828617;    int TYSFLSTjzZ50610180 = -924469826;    int TYSFLSTjzZ60060878 = -62016074;    int TYSFLSTjzZ36649710 = -960250053;    int TYSFLSTjzZ88973506 = -933790705;    int TYSFLSTjzZ80038520 = -69218551;    int TYSFLSTjzZ67265984 = -756023787;    int TYSFLSTjzZ23429688 = -987938289;    int TYSFLSTjzZ55400684 = -341411353;    int TYSFLSTjzZ55113539 = -748819745;    int TYSFLSTjzZ76968333 = -529402244;    int TYSFLSTjzZ34545773 = -806532654;    int TYSFLSTjzZ84874630 = -636592023;    int TYSFLSTjzZ1355134 = -834100963;    int TYSFLSTjzZ3516045 = -936521008;    int TYSFLSTjzZ31725422 = -515787567;    int TYSFLSTjzZ29089224 = -439592110;    int TYSFLSTjzZ14412659 = -196314836;    int TYSFLSTjzZ3066575 = -202636376;    int TYSFLSTjzZ30248639 = -627115534;    int TYSFLSTjzZ67480173 = -101646346;    int TYSFLSTjzZ44664336 = -343179903;    int TYSFLSTjzZ32965184 = 54384709;    int TYSFLSTjzZ43037377 = -582024701;    int TYSFLSTjzZ30228145 = -507891935;    int TYSFLSTjzZ4581983 = 19310972;    int TYSFLSTjzZ6813111 = -585966007;    int TYSFLSTjzZ17618758 = -9526769;    int TYSFLSTjzZ5034853 = 97629785;    int TYSFLSTjzZ98841566 = -460545737;    int TYSFLSTjzZ1713343 = -6463415;    int TYSFLSTjzZ5604592 = -607362231;    int TYSFLSTjzZ24763953 = -773959632;    int TYSFLSTjzZ65779733 = 59291065;    int TYSFLSTjzZ9019387 = -315433203;    int TYSFLSTjzZ88211972 = -210702408;    int TYSFLSTjzZ78042144 = -516716852;    int TYSFLSTjzZ98935667 = 45961645;    int TYSFLSTjzZ54380580 = -567551270;    int TYSFLSTjzZ88253729 = 94005217;    int TYSFLSTjzZ15427284 = 70541480;    int TYSFLSTjzZ47776750 = -25414209;    int TYSFLSTjzZ32764555 = -7808719;    int TYSFLSTjzZ32908128 = -995521097;    int TYSFLSTjzZ59050965 = -872879843;    int TYSFLSTjzZ38888310 = -417230615;    int TYSFLSTjzZ25961277 = -692202897;    int TYSFLSTjzZ77607012 = -148243916;    int TYSFLSTjzZ18917704 = -293599366;    int TYSFLSTjzZ59702304 = -218900923;    int TYSFLSTjzZ14770052 = -836827251;    int TYSFLSTjzZ82211734 = -380484103;    int TYSFLSTjzZ15909365 = -959103809;    int TYSFLSTjzZ74693050 = -498297222;    int TYSFLSTjzZ42537991 = -933794191;    int TYSFLSTjzZ54681395 = -236648714;    int TYSFLSTjzZ17644997 = -878854536;    int TYSFLSTjzZ17023501 = -479991373;    int TYSFLSTjzZ6421566 = -352358118;    int TYSFLSTjzZ84391523 = -853101678;    int TYSFLSTjzZ73225410 = -483252545;    int TYSFLSTjzZ49647226 = -646497019;    int TYSFLSTjzZ18394836 = -985568074;    int TYSFLSTjzZ56559117 = -880865617;    int TYSFLSTjzZ53400196 = -642356331;    int TYSFLSTjzZ71363741 = -922040014;    int TYSFLSTjzZ9781821 = 67426978;    int TYSFLSTjzZ19094897 = -595883089;    int TYSFLSTjzZ92335746 = -418667760;    int TYSFLSTjzZ15304073 = -625818600;    int TYSFLSTjzZ53683277 = -999070715;    int TYSFLSTjzZ30153556 = -385553756;    int TYSFLSTjzZ60032078 = -628763566;    int TYSFLSTjzZ14812846 = -196641593;    int TYSFLSTjzZ14821355 = -597657015;    int TYSFLSTjzZ19703423 = 23767862;    int TYSFLSTjzZ11899781 = -999372388;     TYSFLSTjzZ29705642 = TYSFLSTjzZ75797897;     TYSFLSTjzZ75797897 = TYSFLSTjzZ8365118;     TYSFLSTjzZ8365118 = TYSFLSTjzZ38822152;     TYSFLSTjzZ38822152 = TYSFLSTjzZ38103022;     TYSFLSTjzZ38103022 = TYSFLSTjzZ35585378;     TYSFLSTjzZ35585378 = TYSFLSTjzZ43354087;     TYSFLSTjzZ43354087 = TYSFLSTjzZ68292249;     TYSFLSTjzZ68292249 = TYSFLSTjzZ82693267;     TYSFLSTjzZ82693267 = TYSFLSTjzZ71206438;     TYSFLSTjzZ71206438 = TYSFLSTjzZ88165239;     TYSFLSTjzZ88165239 = TYSFLSTjzZ88021666;     TYSFLSTjzZ88021666 = TYSFLSTjzZ36019299;     TYSFLSTjzZ36019299 = TYSFLSTjzZ73434083;     TYSFLSTjzZ73434083 = TYSFLSTjzZ10835907;     TYSFLSTjzZ10835907 = TYSFLSTjzZ78962146;     TYSFLSTjzZ78962146 = TYSFLSTjzZ22433749;     TYSFLSTjzZ22433749 = TYSFLSTjzZ91427726;     TYSFLSTjzZ91427726 = TYSFLSTjzZ43859275;     TYSFLSTjzZ43859275 = TYSFLSTjzZ96624392;     TYSFLSTjzZ96624392 = TYSFLSTjzZ18975939;     TYSFLSTjzZ18975939 = TYSFLSTjzZ4941689;     TYSFLSTjzZ4941689 = TYSFLSTjzZ10018164;     TYSFLSTjzZ10018164 = TYSFLSTjzZ99345730;     TYSFLSTjzZ99345730 = TYSFLSTjzZ50610180;     TYSFLSTjzZ50610180 = TYSFLSTjzZ60060878;     TYSFLSTjzZ60060878 = TYSFLSTjzZ36649710;     TYSFLSTjzZ36649710 = TYSFLSTjzZ88973506;     TYSFLSTjzZ88973506 = TYSFLSTjzZ80038520;     TYSFLSTjzZ80038520 = TYSFLSTjzZ67265984;     TYSFLSTjzZ67265984 = TYSFLSTjzZ23429688;     TYSFLSTjzZ23429688 = TYSFLSTjzZ55400684;     TYSFLSTjzZ55400684 = TYSFLSTjzZ55113539;     TYSFLSTjzZ55113539 = TYSFLSTjzZ76968333;     TYSFLSTjzZ76968333 = TYSFLSTjzZ34545773;     TYSFLSTjzZ34545773 = TYSFLSTjzZ84874630;     TYSFLSTjzZ84874630 = TYSFLSTjzZ1355134;     TYSFLSTjzZ1355134 = TYSFLSTjzZ3516045;     TYSFLSTjzZ3516045 = TYSFLSTjzZ31725422;     TYSFLSTjzZ31725422 = TYSFLSTjzZ29089224;     TYSFLSTjzZ29089224 = TYSFLSTjzZ14412659;     TYSFLSTjzZ14412659 = TYSFLSTjzZ3066575;     TYSFLSTjzZ3066575 = TYSFLSTjzZ30248639;     TYSFLSTjzZ30248639 = TYSFLSTjzZ67480173;     TYSFLSTjzZ67480173 = TYSFLSTjzZ44664336;     TYSFLSTjzZ44664336 = TYSFLSTjzZ32965184;     TYSFLSTjzZ32965184 = TYSFLSTjzZ43037377;     TYSFLSTjzZ43037377 = TYSFLSTjzZ30228145;     TYSFLSTjzZ30228145 = TYSFLSTjzZ4581983;     TYSFLSTjzZ4581983 = TYSFLSTjzZ6813111;     TYSFLSTjzZ6813111 = TYSFLSTjzZ17618758;     TYSFLSTjzZ17618758 = TYSFLSTjzZ5034853;     TYSFLSTjzZ5034853 = TYSFLSTjzZ98841566;     TYSFLSTjzZ98841566 = TYSFLSTjzZ1713343;     TYSFLSTjzZ1713343 = TYSFLSTjzZ5604592;     TYSFLSTjzZ5604592 = TYSFLSTjzZ24763953;     TYSFLSTjzZ24763953 = TYSFLSTjzZ65779733;     TYSFLSTjzZ65779733 = TYSFLSTjzZ9019387;     TYSFLSTjzZ9019387 = TYSFLSTjzZ88211972;     TYSFLSTjzZ88211972 = TYSFLSTjzZ78042144;     TYSFLSTjzZ78042144 = TYSFLSTjzZ98935667;     TYSFLSTjzZ98935667 = TYSFLSTjzZ54380580;     TYSFLSTjzZ54380580 = TYSFLSTjzZ88253729;     TYSFLSTjzZ88253729 = TYSFLSTjzZ15427284;     TYSFLSTjzZ15427284 = TYSFLSTjzZ47776750;     TYSFLSTjzZ47776750 = TYSFLSTjzZ32764555;     TYSFLSTjzZ32764555 = TYSFLSTjzZ32908128;     TYSFLSTjzZ32908128 = TYSFLSTjzZ59050965;     TYSFLSTjzZ59050965 = TYSFLSTjzZ38888310;     TYSFLSTjzZ38888310 = TYSFLSTjzZ25961277;     TYSFLSTjzZ25961277 = TYSFLSTjzZ77607012;     TYSFLSTjzZ77607012 = TYSFLSTjzZ18917704;     TYSFLSTjzZ18917704 = TYSFLSTjzZ59702304;     TYSFLSTjzZ59702304 = TYSFLSTjzZ14770052;     TYSFLSTjzZ14770052 = TYSFLSTjzZ82211734;     TYSFLSTjzZ82211734 = TYSFLSTjzZ15909365;     TYSFLSTjzZ15909365 = TYSFLSTjzZ74693050;     TYSFLSTjzZ74693050 = TYSFLSTjzZ42537991;     TYSFLSTjzZ42537991 = TYSFLSTjzZ54681395;     TYSFLSTjzZ54681395 = TYSFLSTjzZ17644997;     TYSFLSTjzZ17644997 = TYSFLSTjzZ17023501;     TYSFLSTjzZ17023501 = TYSFLSTjzZ6421566;     TYSFLSTjzZ6421566 = TYSFLSTjzZ84391523;     TYSFLSTjzZ84391523 = TYSFLSTjzZ73225410;     TYSFLSTjzZ73225410 = TYSFLSTjzZ49647226;     TYSFLSTjzZ49647226 = TYSFLSTjzZ18394836;     TYSFLSTjzZ18394836 = TYSFLSTjzZ56559117;     TYSFLSTjzZ56559117 = TYSFLSTjzZ53400196;     TYSFLSTjzZ53400196 = TYSFLSTjzZ71363741;     TYSFLSTjzZ71363741 = TYSFLSTjzZ9781821;     TYSFLSTjzZ9781821 = TYSFLSTjzZ19094897;     TYSFLSTjzZ19094897 = TYSFLSTjzZ92335746;     TYSFLSTjzZ92335746 = TYSFLSTjzZ15304073;     TYSFLSTjzZ15304073 = TYSFLSTjzZ53683277;     TYSFLSTjzZ53683277 = TYSFLSTjzZ30153556;     TYSFLSTjzZ30153556 = TYSFLSTjzZ60032078;     TYSFLSTjzZ60032078 = TYSFLSTjzZ14812846;     TYSFLSTjzZ14812846 = TYSFLSTjzZ14821355;     TYSFLSTjzZ14821355 = TYSFLSTjzZ19703423;     TYSFLSTjzZ19703423 = TYSFLSTjzZ11899781;     TYSFLSTjzZ11899781 = TYSFLSTjzZ29705642;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jodLVbvleE42793872() {     int rspdBLTerH13149947 = -780038068;    int rspdBLTerH92669986 = -75531659;    int rspdBLTerH4773625 = -888787047;    int rspdBLTerH1614404 = -328972070;    int rspdBLTerH44224346 = -485600417;    int rspdBLTerH34089779 = -793360060;    int rspdBLTerH14393405 = -241253288;    int rspdBLTerH82639692 = -62371363;    int rspdBLTerH41222968 = -26941658;    int rspdBLTerH3752090 = -481728610;    int rspdBLTerH37581177 = -639335945;    int rspdBLTerH51227066 = -731959088;    int rspdBLTerH54137075 = -193943328;    int rspdBLTerH53060975 = -12053626;    int rspdBLTerH71690370 = -422817902;    int rspdBLTerH65351308 = -870424204;    int rspdBLTerH16185634 = 16534600;    int rspdBLTerH72279172 = 18006635;    int rspdBLTerH6299417 = -275094232;    int rspdBLTerH85991835 = -106384931;    int rspdBLTerH74616326 = -851269216;    int rspdBLTerH17089007 = -116564526;    int rspdBLTerH13843872 = -75329734;    int rspdBLTerH34052289 = -751654013;    int rspdBLTerH92186137 = -384122585;    int rspdBLTerH55403027 = 57474210;    int rspdBLTerH86104450 = -588334395;    int rspdBLTerH28705132 = 96254411;    int rspdBLTerH16119615 = 68276307;    int rspdBLTerH83099325 = -168873638;    int rspdBLTerH41525001 = -625695490;    int rspdBLTerH18507111 = -256606433;    int rspdBLTerH98652425 = -369691923;    int rspdBLTerH19126844 = -121004414;    int rspdBLTerH13272504 = 95802611;    int rspdBLTerH97263814 = -424615929;    int rspdBLTerH69969312 = -712950142;    int rspdBLTerH44094876 = -644991528;    int rspdBLTerH80720500 = -700685365;    int rspdBLTerH78584358 = -192185633;    int rspdBLTerH48061583 = -867863809;    int rspdBLTerH53554360 = -776544697;    int rspdBLTerH53815708 = -78172777;    int rspdBLTerH59385113 = -957829117;    int rspdBLTerH36608833 = -71336856;    int rspdBLTerH72571179 = 34773786;    int rspdBLTerH19052808 = -714328931;    int rspdBLTerH49898740 = -506613654;    int rspdBLTerH35429095 = -566291110;    int rspdBLTerH42145594 = -792986462;    int rspdBLTerH90876206 = -230428191;    int rspdBLTerH60535646 = 39370473;    int rspdBLTerH12954026 = -672293053;    int rspdBLTerH26390102 = -824876802;    int rspdBLTerH68177546 = -955848354;    int rspdBLTerH96060939 = -563473543;    int rspdBLTerH78826114 = 99798074;    int rspdBLTerH70721335 = -37133035;    int rspdBLTerH9428266 = -944849485;    int rspdBLTerH88821319 = -443074628;    int rspdBLTerH47985328 = -105025665;    int rspdBLTerH85688273 = -237507700;    int rspdBLTerH66520078 = -30647671;    int rspdBLTerH58123643 = -858068020;    int rspdBLTerH62227088 = -856033121;    int rspdBLTerH19074066 = -282729512;    int rspdBLTerH52574640 = -262267166;    int rspdBLTerH35010231 = 27061086;    int rspdBLTerH39788472 = -7856237;    int rspdBLTerH74426556 = -998201974;    int rspdBLTerH95381995 = -57474062;    int rspdBLTerH72090757 = -338473873;    int rspdBLTerH91558672 = -281308001;    int rspdBLTerH27715059 = 17091400;    int rspdBLTerH37930252 = -238521122;    int rspdBLTerH21061966 = 25275481;    int rspdBLTerH63273298 = 61608250;    int rspdBLTerH54458759 = -117500617;    int rspdBLTerH97443456 = -580317157;    int rspdBLTerH19614959 = -318896371;    int rspdBLTerH36350219 = -228196860;    int rspdBLTerH36205710 = 18279258;    int rspdBLTerH93276036 = -337454479;    int rspdBLTerH73974020 = -138737232;    int rspdBLTerH92223119 = -938445448;    int rspdBLTerH80989355 = -565065964;    int rspdBLTerH5553086 = -584313380;    int rspdBLTerH72262324 = -544815121;    int rspdBLTerH50949298 = -165156061;    int rspdBLTerH17211564 = -340723847;    int rspdBLTerH18437700 = -424414004;    int rspdBLTerH99247976 = -575817108;    int rspdBLTerH34666611 = -700142043;    int rspdBLTerH91899180 = -157610737;    int rspdBLTerH30599031 = 12840032;    int rspdBLTerH62373310 = -530356110;    int rspdBLTerH87034282 = -645897027;    int rspdBLTerH95692065 = -220104758;    int rspdBLTerH97158024 = -1795997;    int rspdBLTerH17534767 = -780038068;     rspdBLTerH13149947 = rspdBLTerH92669986;     rspdBLTerH92669986 = rspdBLTerH4773625;     rspdBLTerH4773625 = rspdBLTerH1614404;     rspdBLTerH1614404 = rspdBLTerH44224346;     rspdBLTerH44224346 = rspdBLTerH34089779;     rspdBLTerH34089779 = rspdBLTerH14393405;     rspdBLTerH14393405 = rspdBLTerH82639692;     rspdBLTerH82639692 = rspdBLTerH41222968;     rspdBLTerH41222968 = rspdBLTerH3752090;     rspdBLTerH3752090 = rspdBLTerH37581177;     rspdBLTerH37581177 = rspdBLTerH51227066;     rspdBLTerH51227066 = rspdBLTerH54137075;     rspdBLTerH54137075 = rspdBLTerH53060975;     rspdBLTerH53060975 = rspdBLTerH71690370;     rspdBLTerH71690370 = rspdBLTerH65351308;     rspdBLTerH65351308 = rspdBLTerH16185634;     rspdBLTerH16185634 = rspdBLTerH72279172;     rspdBLTerH72279172 = rspdBLTerH6299417;     rspdBLTerH6299417 = rspdBLTerH85991835;     rspdBLTerH85991835 = rspdBLTerH74616326;     rspdBLTerH74616326 = rspdBLTerH17089007;     rspdBLTerH17089007 = rspdBLTerH13843872;     rspdBLTerH13843872 = rspdBLTerH34052289;     rspdBLTerH34052289 = rspdBLTerH92186137;     rspdBLTerH92186137 = rspdBLTerH55403027;     rspdBLTerH55403027 = rspdBLTerH86104450;     rspdBLTerH86104450 = rspdBLTerH28705132;     rspdBLTerH28705132 = rspdBLTerH16119615;     rspdBLTerH16119615 = rspdBLTerH83099325;     rspdBLTerH83099325 = rspdBLTerH41525001;     rspdBLTerH41525001 = rspdBLTerH18507111;     rspdBLTerH18507111 = rspdBLTerH98652425;     rspdBLTerH98652425 = rspdBLTerH19126844;     rspdBLTerH19126844 = rspdBLTerH13272504;     rspdBLTerH13272504 = rspdBLTerH97263814;     rspdBLTerH97263814 = rspdBLTerH69969312;     rspdBLTerH69969312 = rspdBLTerH44094876;     rspdBLTerH44094876 = rspdBLTerH80720500;     rspdBLTerH80720500 = rspdBLTerH78584358;     rspdBLTerH78584358 = rspdBLTerH48061583;     rspdBLTerH48061583 = rspdBLTerH53554360;     rspdBLTerH53554360 = rspdBLTerH53815708;     rspdBLTerH53815708 = rspdBLTerH59385113;     rspdBLTerH59385113 = rspdBLTerH36608833;     rspdBLTerH36608833 = rspdBLTerH72571179;     rspdBLTerH72571179 = rspdBLTerH19052808;     rspdBLTerH19052808 = rspdBLTerH49898740;     rspdBLTerH49898740 = rspdBLTerH35429095;     rspdBLTerH35429095 = rspdBLTerH42145594;     rspdBLTerH42145594 = rspdBLTerH90876206;     rspdBLTerH90876206 = rspdBLTerH60535646;     rspdBLTerH60535646 = rspdBLTerH12954026;     rspdBLTerH12954026 = rspdBLTerH26390102;     rspdBLTerH26390102 = rspdBLTerH68177546;     rspdBLTerH68177546 = rspdBLTerH96060939;     rspdBLTerH96060939 = rspdBLTerH78826114;     rspdBLTerH78826114 = rspdBLTerH70721335;     rspdBLTerH70721335 = rspdBLTerH9428266;     rspdBLTerH9428266 = rspdBLTerH88821319;     rspdBLTerH88821319 = rspdBLTerH47985328;     rspdBLTerH47985328 = rspdBLTerH85688273;     rspdBLTerH85688273 = rspdBLTerH66520078;     rspdBLTerH66520078 = rspdBLTerH58123643;     rspdBLTerH58123643 = rspdBLTerH62227088;     rspdBLTerH62227088 = rspdBLTerH19074066;     rspdBLTerH19074066 = rspdBLTerH52574640;     rspdBLTerH52574640 = rspdBLTerH35010231;     rspdBLTerH35010231 = rspdBLTerH39788472;     rspdBLTerH39788472 = rspdBLTerH74426556;     rspdBLTerH74426556 = rspdBLTerH95381995;     rspdBLTerH95381995 = rspdBLTerH72090757;     rspdBLTerH72090757 = rspdBLTerH91558672;     rspdBLTerH91558672 = rspdBLTerH27715059;     rspdBLTerH27715059 = rspdBLTerH37930252;     rspdBLTerH37930252 = rspdBLTerH21061966;     rspdBLTerH21061966 = rspdBLTerH63273298;     rspdBLTerH63273298 = rspdBLTerH54458759;     rspdBLTerH54458759 = rspdBLTerH97443456;     rspdBLTerH97443456 = rspdBLTerH19614959;     rspdBLTerH19614959 = rspdBLTerH36350219;     rspdBLTerH36350219 = rspdBLTerH36205710;     rspdBLTerH36205710 = rspdBLTerH93276036;     rspdBLTerH93276036 = rspdBLTerH73974020;     rspdBLTerH73974020 = rspdBLTerH92223119;     rspdBLTerH92223119 = rspdBLTerH80989355;     rspdBLTerH80989355 = rspdBLTerH5553086;     rspdBLTerH5553086 = rspdBLTerH72262324;     rspdBLTerH72262324 = rspdBLTerH50949298;     rspdBLTerH50949298 = rspdBLTerH17211564;     rspdBLTerH17211564 = rspdBLTerH18437700;     rspdBLTerH18437700 = rspdBLTerH99247976;     rspdBLTerH99247976 = rspdBLTerH34666611;     rspdBLTerH34666611 = rspdBLTerH91899180;     rspdBLTerH91899180 = rspdBLTerH30599031;     rspdBLTerH30599031 = rspdBLTerH62373310;     rspdBLTerH62373310 = rspdBLTerH87034282;     rspdBLTerH87034282 = rspdBLTerH95692065;     rspdBLTerH95692065 = rspdBLTerH97158024;     rspdBLTerH97158024 = rspdBLTerH17534767;     rspdBLTerH17534767 = rspdBLTerH13149947;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void opVCrXuAvY13249745() {     int WzHKLHyBHE25936114 = -915078760;    int WzHKLHyBHE79440874 = -245865673;    int WzHKLHyBHE22526424 = -197146429;    int WzHKLHyBHE9555229 = -153174347;    int WzHKLHyBHE37735451 = -977416882;    int WzHKLHyBHE77399849 = -570283374;    int WzHKLHyBHE35601163 = -786925359;    int WzHKLHyBHE9358805 = -506889620;    int WzHKLHyBHE36627779 = 11300250;    int WzHKLHyBHE9970795 = -285816243;    int WzHKLHyBHE65068559 = -754021960;    int WzHKLHyBHE83177570 = -139291904;    int WzHKLHyBHE24280378 = -97725985;    int WzHKLHyBHE2685054 = -344468824;    int WzHKLHyBHE94996923 = -329803733;    int WzHKLHyBHE93776376 = -321315593;    int WzHKLHyBHE71937451 = -747239867;    int WzHKLHyBHE55837658 = -637203668;    int WzHKLHyBHE7460048 = -184563427;    int WzHKLHyBHE88646331 = -705629070;    int WzHKLHyBHE54790383 = -242585480;    int WzHKLHyBHE46136826 = -286422874;    int WzHKLHyBHE52604167 = -6333628;    int WzHKLHyBHE78559418 = -11999025;    int WzHKLHyBHE98080396 = -594564510;    int WzHKLHyBHE36721672 = -732956989;    int WzHKLHyBHE97519986 = -122040675;    int WzHKLHyBHE96058056 = -77543694;    int WzHKLHyBHE32736668 = -145940933;    int WzHKLHyBHE35850441 = -898759793;    int WzHKLHyBHE85960997 = -902112240;    int WzHKLHyBHE21825935 = -235326111;    int WzHKLHyBHE44698375 = -245546707;    int WzHKLHyBHE96203216 = -25758633;    int WzHKLHyBHE96333035 = -514229270;    int WzHKLHyBHE92487929 = -213179033;    int WzHKLHyBHE6733373 = -502859493;    int WzHKLHyBHE1798900 = -564611620;    int WzHKLHyBHE86495886 = -542535942;    int WzHKLHyBHE4849089 = -36602478;    int WzHKLHyBHE9492892 = -143337802;    int WzHKLHyBHE61256816 = -674856032;    int WzHKLHyBHE88699178 = -101606222;    int WzHKLHyBHE4013651 = -199001360;    int WzHKLHyBHE16617323 = 35893184;    int WzHKLHyBHE46199309 = -38544654;    int WzHKLHyBHE96634827 = -111843983;    int WzHKLHyBHE79338706 = -661966433;    int WzHKLHyBHE79493144 = -729893085;    int WzHKLHyBHE89096014 = -304022925;    int WzHKLHyBHE9203700 = -483312499;    int WzHKLHyBHE88712193 = -431814865;    int WzHKLHyBHE47439176 = -848296336;    int WzHKLHyBHE71089826 = -414328544;    int WzHKLHyBHE58459362 = -950292840;    int WzHKLHyBHE79799288 = -528655886;    int WzHKLHyBHE26836707 = -139532045;    int WzHKLHyBHE43967006 = -85147404;    int WzHKLHyBHE11474832 = -558609838;    int WzHKLHyBHE1013779 = -144459894;    int WzHKLHyBHE79879862 = -348242699;    int WzHKLHyBHE39543106 = -609381666;    int WzHKLHyBHE76622137 = -260948687;    int WzHKLHyBHE777339 = -89939957;    int WzHKLHyBHE24009798 = -383704003;    int WzHKLHyBHE43242625 = -418695849;    int WzHKLHyBHE38479196 = -893745197;    int WzHKLHyBHE28077161 = 28032647;    int WzHKLHyBHE6352019 = -830239555;    int WzHKLHyBHE2508994 = -16624700;    int WzHKLHyBHE87043004 = -818456100;    int WzHKLHyBHE70138551 = -82628248;    int WzHKLHyBHE69341772 = 5332274;    int WzHKLHyBHE2610960 = -47960950;    int WzHKLHyBHE79153440 = -462291268;    int WzHKLHyBHE93533566 = -567729449;    int WzHKLHyBHE57437648 = -84816653;    int WzHKLHyBHE48590516 = -807332269;    int WzHKLHyBHE61942095 = 52107791;    int WzHKLHyBHE51881088 = -456019856;    int WzHKLHyBHE40086845 = -521113006;    int WzHKLHyBHE18181280 = -460074243;    int WzHKLHyBHE16564913 = -347650609;    int WzHKLHyBHE43640653 = -841918009;    int WzHKLHyBHE26646741 = -315447294;    int WzHKLHyBHE97248803 = -370297376;    int WzHKLHyBHE74386759 = -387029776;    int WzHKLHyBHE73608548 = -831218163;    int WzHKLHyBHE37743854 = -75465793;    int WzHKLHyBHE16533748 = -985573384;    int WzHKLHyBHE65651222 = 26353011;    int WzHKLHyBHE62766366 = -317712089;    int WzHKLHyBHE90324067 = 93998218;    int WzHKLHyBHE85482107 = -298076049;    int WzHKLHyBHE24969226 = -688359780;    int WzHKLHyBHE69949785 = -533956137;    int WzHKLHyBHE84634679 = -313907345;    int WzHKLHyBHE87921839 = 88333735;    int WzHKLHyBHE80003853 = -815297357;    int WzHKLHyBHE73374698 = -915078760;     WzHKLHyBHE25936114 = WzHKLHyBHE79440874;     WzHKLHyBHE79440874 = WzHKLHyBHE22526424;     WzHKLHyBHE22526424 = WzHKLHyBHE9555229;     WzHKLHyBHE9555229 = WzHKLHyBHE37735451;     WzHKLHyBHE37735451 = WzHKLHyBHE77399849;     WzHKLHyBHE77399849 = WzHKLHyBHE35601163;     WzHKLHyBHE35601163 = WzHKLHyBHE9358805;     WzHKLHyBHE9358805 = WzHKLHyBHE36627779;     WzHKLHyBHE36627779 = WzHKLHyBHE9970795;     WzHKLHyBHE9970795 = WzHKLHyBHE65068559;     WzHKLHyBHE65068559 = WzHKLHyBHE83177570;     WzHKLHyBHE83177570 = WzHKLHyBHE24280378;     WzHKLHyBHE24280378 = WzHKLHyBHE2685054;     WzHKLHyBHE2685054 = WzHKLHyBHE94996923;     WzHKLHyBHE94996923 = WzHKLHyBHE93776376;     WzHKLHyBHE93776376 = WzHKLHyBHE71937451;     WzHKLHyBHE71937451 = WzHKLHyBHE55837658;     WzHKLHyBHE55837658 = WzHKLHyBHE7460048;     WzHKLHyBHE7460048 = WzHKLHyBHE88646331;     WzHKLHyBHE88646331 = WzHKLHyBHE54790383;     WzHKLHyBHE54790383 = WzHKLHyBHE46136826;     WzHKLHyBHE46136826 = WzHKLHyBHE52604167;     WzHKLHyBHE52604167 = WzHKLHyBHE78559418;     WzHKLHyBHE78559418 = WzHKLHyBHE98080396;     WzHKLHyBHE98080396 = WzHKLHyBHE36721672;     WzHKLHyBHE36721672 = WzHKLHyBHE97519986;     WzHKLHyBHE97519986 = WzHKLHyBHE96058056;     WzHKLHyBHE96058056 = WzHKLHyBHE32736668;     WzHKLHyBHE32736668 = WzHKLHyBHE35850441;     WzHKLHyBHE35850441 = WzHKLHyBHE85960997;     WzHKLHyBHE85960997 = WzHKLHyBHE21825935;     WzHKLHyBHE21825935 = WzHKLHyBHE44698375;     WzHKLHyBHE44698375 = WzHKLHyBHE96203216;     WzHKLHyBHE96203216 = WzHKLHyBHE96333035;     WzHKLHyBHE96333035 = WzHKLHyBHE92487929;     WzHKLHyBHE92487929 = WzHKLHyBHE6733373;     WzHKLHyBHE6733373 = WzHKLHyBHE1798900;     WzHKLHyBHE1798900 = WzHKLHyBHE86495886;     WzHKLHyBHE86495886 = WzHKLHyBHE4849089;     WzHKLHyBHE4849089 = WzHKLHyBHE9492892;     WzHKLHyBHE9492892 = WzHKLHyBHE61256816;     WzHKLHyBHE61256816 = WzHKLHyBHE88699178;     WzHKLHyBHE88699178 = WzHKLHyBHE4013651;     WzHKLHyBHE4013651 = WzHKLHyBHE16617323;     WzHKLHyBHE16617323 = WzHKLHyBHE46199309;     WzHKLHyBHE46199309 = WzHKLHyBHE96634827;     WzHKLHyBHE96634827 = WzHKLHyBHE79338706;     WzHKLHyBHE79338706 = WzHKLHyBHE79493144;     WzHKLHyBHE79493144 = WzHKLHyBHE89096014;     WzHKLHyBHE89096014 = WzHKLHyBHE9203700;     WzHKLHyBHE9203700 = WzHKLHyBHE88712193;     WzHKLHyBHE88712193 = WzHKLHyBHE47439176;     WzHKLHyBHE47439176 = WzHKLHyBHE71089826;     WzHKLHyBHE71089826 = WzHKLHyBHE58459362;     WzHKLHyBHE58459362 = WzHKLHyBHE79799288;     WzHKLHyBHE79799288 = WzHKLHyBHE26836707;     WzHKLHyBHE26836707 = WzHKLHyBHE43967006;     WzHKLHyBHE43967006 = WzHKLHyBHE11474832;     WzHKLHyBHE11474832 = WzHKLHyBHE1013779;     WzHKLHyBHE1013779 = WzHKLHyBHE79879862;     WzHKLHyBHE79879862 = WzHKLHyBHE39543106;     WzHKLHyBHE39543106 = WzHKLHyBHE76622137;     WzHKLHyBHE76622137 = WzHKLHyBHE777339;     WzHKLHyBHE777339 = WzHKLHyBHE24009798;     WzHKLHyBHE24009798 = WzHKLHyBHE43242625;     WzHKLHyBHE43242625 = WzHKLHyBHE38479196;     WzHKLHyBHE38479196 = WzHKLHyBHE28077161;     WzHKLHyBHE28077161 = WzHKLHyBHE6352019;     WzHKLHyBHE6352019 = WzHKLHyBHE2508994;     WzHKLHyBHE2508994 = WzHKLHyBHE87043004;     WzHKLHyBHE87043004 = WzHKLHyBHE70138551;     WzHKLHyBHE70138551 = WzHKLHyBHE69341772;     WzHKLHyBHE69341772 = WzHKLHyBHE2610960;     WzHKLHyBHE2610960 = WzHKLHyBHE79153440;     WzHKLHyBHE79153440 = WzHKLHyBHE93533566;     WzHKLHyBHE93533566 = WzHKLHyBHE57437648;     WzHKLHyBHE57437648 = WzHKLHyBHE48590516;     WzHKLHyBHE48590516 = WzHKLHyBHE61942095;     WzHKLHyBHE61942095 = WzHKLHyBHE51881088;     WzHKLHyBHE51881088 = WzHKLHyBHE40086845;     WzHKLHyBHE40086845 = WzHKLHyBHE18181280;     WzHKLHyBHE18181280 = WzHKLHyBHE16564913;     WzHKLHyBHE16564913 = WzHKLHyBHE43640653;     WzHKLHyBHE43640653 = WzHKLHyBHE26646741;     WzHKLHyBHE26646741 = WzHKLHyBHE97248803;     WzHKLHyBHE97248803 = WzHKLHyBHE74386759;     WzHKLHyBHE74386759 = WzHKLHyBHE73608548;     WzHKLHyBHE73608548 = WzHKLHyBHE37743854;     WzHKLHyBHE37743854 = WzHKLHyBHE16533748;     WzHKLHyBHE16533748 = WzHKLHyBHE65651222;     WzHKLHyBHE65651222 = WzHKLHyBHE62766366;     WzHKLHyBHE62766366 = WzHKLHyBHE90324067;     WzHKLHyBHE90324067 = WzHKLHyBHE85482107;     WzHKLHyBHE85482107 = WzHKLHyBHE24969226;     WzHKLHyBHE24969226 = WzHKLHyBHE69949785;     WzHKLHyBHE69949785 = WzHKLHyBHE84634679;     WzHKLHyBHE84634679 = WzHKLHyBHE87921839;     WzHKLHyBHE87921839 = WzHKLHyBHE80003853;     WzHKLHyBHE80003853 = WzHKLHyBHE73374698;     WzHKLHyBHE73374698 = WzHKLHyBHE25936114;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void syYIiQrUYo31463087() {     int zhNmFZBNpX9380420 = -695744441;    int zhNmFZBNpX96312963 = -245247860;    int zhNmFZBNpX18934931 = 9328344;    int zhNmFZBNpX72347480 = -346974183;    int zhNmFZBNpX43856774 = -784284373;    int zhNmFZBNpX75904250 = -349355027;    int zhNmFZBNpX6640481 = -526836672;    int zhNmFZBNpX23706248 = -494047649;    int zhNmFZBNpX95157479 = -330159101;    int zhNmFZBNpX42516447 = -754192355;    int zhNmFZBNpX14484497 = -944137833;    int zhNmFZBNpX46382970 = -126910150;    int zhNmFZBNpX42398154 = -989387226;    int zhNmFZBNpX82311946 = -132759181;    int zhNmFZBNpX55851387 = -423826715;    int zhNmFZBNpX80165538 = -109394918;    int zhNmFZBNpX65689336 = -500584894;    int zhNmFZBNpX36689104 = -884508544;    int zhNmFZBNpX69900189 = -183238298;    int zhNmFZBNpX78013773 = -135215062;    int zhNmFZBNpX10430770 = -932114511;    int zhNmFZBNpX58284144 = -277574645;    int zhNmFZBNpX56429875 = -46222826;    int zhNmFZBNpX13265977 = -83824421;    int zhNmFZBNpX39656355 = -54217269;    int zhNmFZBNpX32063821 = -613466704;    int zhNmFZBNpX46974727 = -850125018;    int zhNmFZBNpX35789682 = -147498578;    int zhNmFZBNpX68817761 = -8446075;    int zhNmFZBNpX51683782 = -311609644;    int zhNmFZBNpX4056312 = -539869441;    int zhNmFZBNpX84932361 = -150521191;    int zhNmFZBNpX88237261 = -966418884;    int zhNmFZBNpX38361727 = -717360803;    int zhNmFZBNpX75059766 = -711894005;    int zhNmFZBNpX4877114 = -1202939;    int zhNmFZBNpX75347551 = -381708673;    int zhNmFZBNpX42377731 = -273082140;    int zhNmFZBNpX35490964 = -727433739;    int zhNmFZBNpX54344223 = -889196001;    int zhNmFZBNpX43141816 = -814886775;    int zhNmFZBNpX11744603 = -148764353;    int zhNmFZBNpX12266248 = -652663465;    int zhNmFZBNpX95918590 = 44815869;    int zhNmFZBNpX8561820 = -792263769;    int zhNmFZBNpX85805304 = -58155578;    int zhNmFZBNpX72650257 = -244148212;    int zhNmFZBNpX99009302 = -660688152;    int zhNmFZBNpX10340257 = -215495168;    int zhNmFZBNpX24428499 = -511043380;    int zhNmFZBNpX82461147 = -704213921;    int zhNmFZBNpX44212988 = -490074176;    int zhNmFZBNpX61551634 = 39956348;    int zhNmFZBNpX95766585 = -132741932;    int zhNmFZBNpX21032317 = -198778963;    int zhNmFZBNpX51096275 = -318169797;    int zhNmFZBNpX39883089 = -99025035;    int zhNmFZBNpX5668955 = -906847236;    int zhNmFZBNpX32691125 = -192756915;    int zhNmFZBNpX11792954 = -70817669;    int zhNmFZBNpX28929523 = -499230009;    int zhNmFZBNpX70850798 = -279338095;    int zhNmFZBNpX54888486 = -385601574;    int zhNmFZBNpX43473698 = 81450543;    int zhNmFZBNpX38460136 = -114322915;    int zhNmFZBNpX29552136 = -693616642;    int zhNmFZBNpX58145708 = -160491266;    int zhNmFZBNpX4036427 = -172026424;    int zhNmFZBNpX7252180 = -420865177;    int zhNmFZBNpX50974273 = -322623777;    int zhNmFZBNpX4817988 = -727686246;    int zhNmFZBNpX23311605 = -127502755;    int zhNmFZBNpX1198141 = -57074805;    int zhNmFZBNpX15555967 = -294042298;    int zhNmFZBNpX34871958 = -320328287;    int zhNmFZBNpX98686167 = -683350158;    int zhNmFZBNpX46017896 = -624911180;    int zhNmFZBNpX60511284 = 8961305;    int zhNmFZBNpX4704157 = -291560652;    int zhNmFZBNpX53851050 = -996061691;    int zhNmFZBNpX59413564 = -269318492;    int zhNmFZBNpX47965425 = -89436867;    int zhNmFZBNpX25449426 = -932003411;    int zhNmFZBNpX44389263 = -497402696;    int zhNmFZBNpX69222634 = -607395723;    int zhNmFZBNpX59843323 = 50204735;    int zhNmFZBNpX23380727 = -90477540;    int zhNmFZBNpX92470676 = -733676953;    int zhNmFZBNpX17329411 = -418581840;    int zhNmFZBNpX23963491 = -293724208;    int zhNmFZBNpX64994025 = -902177905;    int zhNmFZBNpX69678596 = -474861437;    int zhNmFZBNpX9686607 = 19674774;    int zhNmFZBNpX23698011 = -556616071;    int zhNmFZBNpX25414701 = -289965992;    int zhNmFZBNpX72291017 = -435548681;    int zhNmFZBNpX56856116 = -763162779;    int zhNmFZBNpX68792550 = -634114009;    int zhNmFZBNpX57458455 = -840861217;    int zhNmFZBNpX79009684 = -695744441;     zhNmFZBNpX9380420 = zhNmFZBNpX96312963;     zhNmFZBNpX96312963 = zhNmFZBNpX18934931;     zhNmFZBNpX18934931 = zhNmFZBNpX72347480;     zhNmFZBNpX72347480 = zhNmFZBNpX43856774;     zhNmFZBNpX43856774 = zhNmFZBNpX75904250;     zhNmFZBNpX75904250 = zhNmFZBNpX6640481;     zhNmFZBNpX6640481 = zhNmFZBNpX23706248;     zhNmFZBNpX23706248 = zhNmFZBNpX95157479;     zhNmFZBNpX95157479 = zhNmFZBNpX42516447;     zhNmFZBNpX42516447 = zhNmFZBNpX14484497;     zhNmFZBNpX14484497 = zhNmFZBNpX46382970;     zhNmFZBNpX46382970 = zhNmFZBNpX42398154;     zhNmFZBNpX42398154 = zhNmFZBNpX82311946;     zhNmFZBNpX82311946 = zhNmFZBNpX55851387;     zhNmFZBNpX55851387 = zhNmFZBNpX80165538;     zhNmFZBNpX80165538 = zhNmFZBNpX65689336;     zhNmFZBNpX65689336 = zhNmFZBNpX36689104;     zhNmFZBNpX36689104 = zhNmFZBNpX69900189;     zhNmFZBNpX69900189 = zhNmFZBNpX78013773;     zhNmFZBNpX78013773 = zhNmFZBNpX10430770;     zhNmFZBNpX10430770 = zhNmFZBNpX58284144;     zhNmFZBNpX58284144 = zhNmFZBNpX56429875;     zhNmFZBNpX56429875 = zhNmFZBNpX13265977;     zhNmFZBNpX13265977 = zhNmFZBNpX39656355;     zhNmFZBNpX39656355 = zhNmFZBNpX32063821;     zhNmFZBNpX32063821 = zhNmFZBNpX46974727;     zhNmFZBNpX46974727 = zhNmFZBNpX35789682;     zhNmFZBNpX35789682 = zhNmFZBNpX68817761;     zhNmFZBNpX68817761 = zhNmFZBNpX51683782;     zhNmFZBNpX51683782 = zhNmFZBNpX4056312;     zhNmFZBNpX4056312 = zhNmFZBNpX84932361;     zhNmFZBNpX84932361 = zhNmFZBNpX88237261;     zhNmFZBNpX88237261 = zhNmFZBNpX38361727;     zhNmFZBNpX38361727 = zhNmFZBNpX75059766;     zhNmFZBNpX75059766 = zhNmFZBNpX4877114;     zhNmFZBNpX4877114 = zhNmFZBNpX75347551;     zhNmFZBNpX75347551 = zhNmFZBNpX42377731;     zhNmFZBNpX42377731 = zhNmFZBNpX35490964;     zhNmFZBNpX35490964 = zhNmFZBNpX54344223;     zhNmFZBNpX54344223 = zhNmFZBNpX43141816;     zhNmFZBNpX43141816 = zhNmFZBNpX11744603;     zhNmFZBNpX11744603 = zhNmFZBNpX12266248;     zhNmFZBNpX12266248 = zhNmFZBNpX95918590;     zhNmFZBNpX95918590 = zhNmFZBNpX8561820;     zhNmFZBNpX8561820 = zhNmFZBNpX85805304;     zhNmFZBNpX85805304 = zhNmFZBNpX72650257;     zhNmFZBNpX72650257 = zhNmFZBNpX99009302;     zhNmFZBNpX99009302 = zhNmFZBNpX10340257;     zhNmFZBNpX10340257 = zhNmFZBNpX24428499;     zhNmFZBNpX24428499 = zhNmFZBNpX82461147;     zhNmFZBNpX82461147 = zhNmFZBNpX44212988;     zhNmFZBNpX44212988 = zhNmFZBNpX61551634;     zhNmFZBNpX61551634 = zhNmFZBNpX95766585;     zhNmFZBNpX95766585 = zhNmFZBNpX21032317;     zhNmFZBNpX21032317 = zhNmFZBNpX51096275;     zhNmFZBNpX51096275 = zhNmFZBNpX39883089;     zhNmFZBNpX39883089 = zhNmFZBNpX5668955;     zhNmFZBNpX5668955 = zhNmFZBNpX32691125;     zhNmFZBNpX32691125 = zhNmFZBNpX11792954;     zhNmFZBNpX11792954 = zhNmFZBNpX28929523;     zhNmFZBNpX28929523 = zhNmFZBNpX70850798;     zhNmFZBNpX70850798 = zhNmFZBNpX54888486;     zhNmFZBNpX54888486 = zhNmFZBNpX43473698;     zhNmFZBNpX43473698 = zhNmFZBNpX38460136;     zhNmFZBNpX38460136 = zhNmFZBNpX29552136;     zhNmFZBNpX29552136 = zhNmFZBNpX58145708;     zhNmFZBNpX58145708 = zhNmFZBNpX4036427;     zhNmFZBNpX4036427 = zhNmFZBNpX7252180;     zhNmFZBNpX7252180 = zhNmFZBNpX50974273;     zhNmFZBNpX50974273 = zhNmFZBNpX4817988;     zhNmFZBNpX4817988 = zhNmFZBNpX23311605;     zhNmFZBNpX23311605 = zhNmFZBNpX1198141;     zhNmFZBNpX1198141 = zhNmFZBNpX15555967;     zhNmFZBNpX15555967 = zhNmFZBNpX34871958;     zhNmFZBNpX34871958 = zhNmFZBNpX98686167;     zhNmFZBNpX98686167 = zhNmFZBNpX46017896;     zhNmFZBNpX46017896 = zhNmFZBNpX60511284;     zhNmFZBNpX60511284 = zhNmFZBNpX4704157;     zhNmFZBNpX4704157 = zhNmFZBNpX53851050;     zhNmFZBNpX53851050 = zhNmFZBNpX59413564;     zhNmFZBNpX59413564 = zhNmFZBNpX47965425;     zhNmFZBNpX47965425 = zhNmFZBNpX25449426;     zhNmFZBNpX25449426 = zhNmFZBNpX44389263;     zhNmFZBNpX44389263 = zhNmFZBNpX69222634;     zhNmFZBNpX69222634 = zhNmFZBNpX59843323;     zhNmFZBNpX59843323 = zhNmFZBNpX23380727;     zhNmFZBNpX23380727 = zhNmFZBNpX92470676;     zhNmFZBNpX92470676 = zhNmFZBNpX17329411;     zhNmFZBNpX17329411 = zhNmFZBNpX23963491;     zhNmFZBNpX23963491 = zhNmFZBNpX64994025;     zhNmFZBNpX64994025 = zhNmFZBNpX69678596;     zhNmFZBNpX69678596 = zhNmFZBNpX9686607;     zhNmFZBNpX9686607 = zhNmFZBNpX23698011;     zhNmFZBNpX23698011 = zhNmFZBNpX25414701;     zhNmFZBNpX25414701 = zhNmFZBNpX72291017;     zhNmFZBNpX72291017 = zhNmFZBNpX56856116;     zhNmFZBNpX56856116 = zhNmFZBNpX68792550;     zhNmFZBNpX68792550 = zhNmFZBNpX57458455;     zhNmFZBNpX57458455 = zhNmFZBNpX79009684;     zhNmFZBNpX79009684 = zhNmFZBNpX9380420;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tBwqENQysS1918960() {     int MdmRjwPjQb22166587 = -830785133;    int MdmRjwPjQb83083851 = -415581874;    int MdmRjwPjQb36687731 = -399031038;    int MdmRjwPjQb80288305 = -171176460;    int MdmRjwPjQb37367879 = -176100838;    int MdmRjwPjQb19214321 = -126278340;    int MdmRjwPjQb27848239 = 27491257;    int MdmRjwPjQb50425360 = -938565906;    int MdmRjwPjQb90562290 = -291917193;    int MdmRjwPjQb48735152 = -558279988;    int MdmRjwPjQb41971879 = 41176152;    int MdmRjwPjQb78333475 = -634242966;    int MdmRjwPjQb12541457 = -893169883;    int MdmRjwPjQb31936025 = -465174379;    int MdmRjwPjQb79157939 = -330812546;    int MdmRjwPjQb8590607 = -660286307;    int MdmRjwPjQb21441154 = -164359361;    int MdmRjwPjQb20247589 = -439718846;    int MdmRjwPjQb71060820 = -92707493;    int MdmRjwPjQb80668270 = -734459201;    int MdmRjwPjQb90604826 = -323430775;    int MdmRjwPjQb87331963 = -447432993;    int MdmRjwPjQb95190170 = 22773280;    int MdmRjwPjQb57773106 = -444169433;    int MdmRjwPjQb45550614 = -264659193;    int MdmRjwPjQb13382467 = -303897903;    int MdmRjwPjQb58390263 = -383831298;    int MdmRjwPjQb3142607 = -321296682;    int MdmRjwPjQb85434814 = -222663315;    int MdmRjwPjQb4434898 = 58504201;    int MdmRjwPjQb48492307 = -816286191;    int MdmRjwPjQb88251185 = -129240870;    int MdmRjwPjQb34283211 = -842273669;    int MdmRjwPjQb15438101 = -622115021;    int MdmRjwPjQb58120298 = -221925885;    int MdmRjwPjQb101229 = -889766044;    int MdmRjwPjQb12111612 = -171618023;    int MdmRjwPjQb81755 = -192702231;    int MdmRjwPjQb41266350 = -569284316;    int MdmRjwPjQb80608953 = -733612846;    int MdmRjwPjQb4573124 = -90360769;    int MdmRjwPjQb19447059 = -47075687;    int MdmRjwPjQb47149718 = -676096910;    int MdmRjwPjQb40547129 = -296356374;    int MdmRjwPjQb88570310 = -685033730;    int MdmRjwPjQb59433434 = -131474017;    int MdmRjwPjQb50232277 = -741663265;    int MdmRjwPjQb28449269 = -816040931;    int MdmRjwPjQb54404306 = -379097142;    int MdmRjwPjQb71378919 = -22079843;    int MdmRjwPjQb788642 = -957098230;    int MdmRjwPjQb72389535 = -961259514;    int MdmRjwPjQb96036784 = -136046935;    int MdmRjwPjQb40466311 = -822193674;    int MdmRjwPjQb11314133 = -193223450;    int MdmRjwPjQb34834623 = -283352141;    int MdmRjwPjQb87893681 = -338355154;    int MdmRjwPjQb78914625 = -954861605;    int MdmRjwPjQb34737691 = -906517267;    int MdmRjwPjQb23985413 = -872202935;    int MdmRjwPjQb60824057 = -742447042;    int MdmRjwPjQb24705632 = -651212061;    int MdmRjwPjQb64990545 = -615902591;    int MdmRjwPjQb86127393 = -250421394;    int MdmRjwPjQb242845 = -741993797;    int MdmRjwPjQb53720694 = -829582979;    int MdmRjwPjQb44050264 = -791969298;    int MdmRjwPjQb97103356 = -171054863;    int MdmRjwPjQb73815726 = -143248495;    int MdmRjwPjQb79056711 = -441046503;    int MdmRjwPjQb96478995 = -388668285;    int MdmRjwPjQb21359400 = -971657130;    int MdmRjwPjQb78981239 = -870434530;    int MdmRjwPjQb90451866 = -359094648;    int MdmRjwPjQb76095146 = -544098433;    int MdmRjwPjQb71157768 = -176355088;    int MdmRjwPjQb40182246 = -771336083;    int MdmRjwPjQb54643042 = -680870347;    int MdmRjwPjQb69202796 = -759135704;    int MdmRjwPjQb86117180 = -33185176;    int MdmRjwPjQb63150189 = -562234639;    int MdmRjwPjQb29940995 = -567790367;    int MdmRjwPjQb48738301 = -942199541;    int MdmRjwPjQb14055896 = -100583473;    int MdmRjwPjQb3646256 = 15602430;    int MdmRjwPjQb76102772 = -855026677;    int MdmRjwPjQb92214400 = -993193935;    int MdmRjwPjQb93816899 = 79920005;    int MdmRjwPjQb4123968 = -328891572;    int MdmRjwPjQb23285675 = -938573745;    int MdmRjwPjQb12207547 = -451410890;    int MdmRjwPjQb33196987 = -216756418;    int MdmRjwPjQb65344063 = -286184965;    int MdmRjwPjQb17280938 = -697081382;    int MdmRjwPjQb19784896 = -991165804;    int MdmRjwPjQb79867492 = -439148708;    int MdmRjwPjQb54456513 = -431173097;    int MdmRjwPjQb61022325 = -325675516;    int MdmRjwPjQb40304284 = -554362577;    int MdmRjwPjQb34849616 = -830785133;     MdmRjwPjQb22166587 = MdmRjwPjQb83083851;     MdmRjwPjQb83083851 = MdmRjwPjQb36687731;     MdmRjwPjQb36687731 = MdmRjwPjQb80288305;     MdmRjwPjQb80288305 = MdmRjwPjQb37367879;     MdmRjwPjQb37367879 = MdmRjwPjQb19214321;     MdmRjwPjQb19214321 = MdmRjwPjQb27848239;     MdmRjwPjQb27848239 = MdmRjwPjQb50425360;     MdmRjwPjQb50425360 = MdmRjwPjQb90562290;     MdmRjwPjQb90562290 = MdmRjwPjQb48735152;     MdmRjwPjQb48735152 = MdmRjwPjQb41971879;     MdmRjwPjQb41971879 = MdmRjwPjQb78333475;     MdmRjwPjQb78333475 = MdmRjwPjQb12541457;     MdmRjwPjQb12541457 = MdmRjwPjQb31936025;     MdmRjwPjQb31936025 = MdmRjwPjQb79157939;     MdmRjwPjQb79157939 = MdmRjwPjQb8590607;     MdmRjwPjQb8590607 = MdmRjwPjQb21441154;     MdmRjwPjQb21441154 = MdmRjwPjQb20247589;     MdmRjwPjQb20247589 = MdmRjwPjQb71060820;     MdmRjwPjQb71060820 = MdmRjwPjQb80668270;     MdmRjwPjQb80668270 = MdmRjwPjQb90604826;     MdmRjwPjQb90604826 = MdmRjwPjQb87331963;     MdmRjwPjQb87331963 = MdmRjwPjQb95190170;     MdmRjwPjQb95190170 = MdmRjwPjQb57773106;     MdmRjwPjQb57773106 = MdmRjwPjQb45550614;     MdmRjwPjQb45550614 = MdmRjwPjQb13382467;     MdmRjwPjQb13382467 = MdmRjwPjQb58390263;     MdmRjwPjQb58390263 = MdmRjwPjQb3142607;     MdmRjwPjQb3142607 = MdmRjwPjQb85434814;     MdmRjwPjQb85434814 = MdmRjwPjQb4434898;     MdmRjwPjQb4434898 = MdmRjwPjQb48492307;     MdmRjwPjQb48492307 = MdmRjwPjQb88251185;     MdmRjwPjQb88251185 = MdmRjwPjQb34283211;     MdmRjwPjQb34283211 = MdmRjwPjQb15438101;     MdmRjwPjQb15438101 = MdmRjwPjQb58120298;     MdmRjwPjQb58120298 = MdmRjwPjQb101229;     MdmRjwPjQb101229 = MdmRjwPjQb12111612;     MdmRjwPjQb12111612 = MdmRjwPjQb81755;     MdmRjwPjQb81755 = MdmRjwPjQb41266350;     MdmRjwPjQb41266350 = MdmRjwPjQb80608953;     MdmRjwPjQb80608953 = MdmRjwPjQb4573124;     MdmRjwPjQb4573124 = MdmRjwPjQb19447059;     MdmRjwPjQb19447059 = MdmRjwPjQb47149718;     MdmRjwPjQb47149718 = MdmRjwPjQb40547129;     MdmRjwPjQb40547129 = MdmRjwPjQb88570310;     MdmRjwPjQb88570310 = MdmRjwPjQb59433434;     MdmRjwPjQb59433434 = MdmRjwPjQb50232277;     MdmRjwPjQb50232277 = MdmRjwPjQb28449269;     MdmRjwPjQb28449269 = MdmRjwPjQb54404306;     MdmRjwPjQb54404306 = MdmRjwPjQb71378919;     MdmRjwPjQb71378919 = MdmRjwPjQb788642;     MdmRjwPjQb788642 = MdmRjwPjQb72389535;     MdmRjwPjQb72389535 = MdmRjwPjQb96036784;     MdmRjwPjQb96036784 = MdmRjwPjQb40466311;     MdmRjwPjQb40466311 = MdmRjwPjQb11314133;     MdmRjwPjQb11314133 = MdmRjwPjQb34834623;     MdmRjwPjQb34834623 = MdmRjwPjQb87893681;     MdmRjwPjQb87893681 = MdmRjwPjQb78914625;     MdmRjwPjQb78914625 = MdmRjwPjQb34737691;     MdmRjwPjQb34737691 = MdmRjwPjQb23985413;     MdmRjwPjQb23985413 = MdmRjwPjQb60824057;     MdmRjwPjQb60824057 = MdmRjwPjQb24705632;     MdmRjwPjQb24705632 = MdmRjwPjQb64990545;     MdmRjwPjQb64990545 = MdmRjwPjQb86127393;     MdmRjwPjQb86127393 = MdmRjwPjQb242845;     MdmRjwPjQb242845 = MdmRjwPjQb53720694;     MdmRjwPjQb53720694 = MdmRjwPjQb44050264;     MdmRjwPjQb44050264 = MdmRjwPjQb97103356;     MdmRjwPjQb97103356 = MdmRjwPjQb73815726;     MdmRjwPjQb73815726 = MdmRjwPjQb79056711;     MdmRjwPjQb79056711 = MdmRjwPjQb96478995;     MdmRjwPjQb96478995 = MdmRjwPjQb21359400;     MdmRjwPjQb21359400 = MdmRjwPjQb78981239;     MdmRjwPjQb78981239 = MdmRjwPjQb90451866;     MdmRjwPjQb90451866 = MdmRjwPjQb76095146;     MdmRjwPjQb76095146 = MdmRjwPjQb71157768;     MdmRjwPjQb71157768 = MdmRjwPjQb40182246;     MdmRjwPjQb40182246 = MdmRjwPjQb54643042;     MdmRjwPjQb54643042 = MdmRjwPjQb69202796;     MdmRjwPjQb69202796 = MdmRjwPjQb86117180;     MdmRjwPjQb86117180 = MdmRjwPjQb63150189;     MdmRjwPjQb63150189 = MdmRjwPjQb29940995;     MdmRjwPjQb29940995 = MdmRjwPjQb48738301;     MdmRjwPjQb48738301 = MdmRjwPjQb14055896;     MdmRjwPjQb14055896 = MdmRjwPjQb3646256;     MdmRjwPjQb3646256 = MdmRjwPjQb76102772;     MdmRjwPjQb76102772 = MdmRjwPjQb92214400;     MdmRjwPjQb92214400 = MdmRjwPjQb93816899;     MdmRjwPjQb93816899 = MdmRjwPjQb4123968;     MdmRjwPjQb4123968 = MdmRjwPjQb23285675;     MdmRjwPjQb23285675 = MdmRjwPjQb12207547;     MdmRjwPjQb12207547 = MdmRjwPjQb33196987;     MdmRjwPjQb33196987 = MdmRjwPjQb65344063;     MdmRjwPjQb65344063 = MdmRjwPjQb17280938;     MdmRjwPjQb17280938 = MdmRjwPjQb19784896;     MdmRjwPjQb19784896 = MdmRjwPjQb79867492;     MdmRjwPjQb79867492 = MdmRjwPjQb54456513;     MdmRjwPjQb54456513 = MdmRjwPjQb61022325;     MdmRjwPjQb61022325 = MdmRjwPjQb40304284;     MdmRjwPjQb40304284 = MdmRjwPjQb34849616;     MdmRjwPjQb34849616 = MdmRjwPjQb22166587;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wQpSNtPgdI20132301() {     int NYlQOgFksD5610893 = -611450813;    int NYlQOgFksD99955940 = -414964061;    int NYlQOgFksD33096238 = -192556266;    int NYlQOgFksD43080557 = -364976295;    int NYlQOgFksD43489203 = 17031671;    int NYlQOgFksD17718722 = 94650007;    int NYlQOgFksD98887556 = -812420056;    int NYlQOgFksD64772803 = -925723935;    int NYlQOgFksD49091991 = -633376544;    int NYlQOgFksD81280803 = 73343900;    int NYlQOgFksD91387817 = -148939721;    int NYlQOgFksD41538874 = -621861212;    int NYlQOgFksD30659233 = -684831124;    int NYlQOgFksD11562917 = -253464736;    int NYlQOgFksD40012403 = -424835528;    int NYlQOgFksD94979768 = -448365633;    int NYlQOgFksD15193039 = 82295612;    int NYlQOgFksD1099036 = -687023723;    int NYlQOgFksD33500962 = -91382364;    int NYlQOgFksD70035712 = -164045194;    int NYlQOgFksD46245214 = 87040194;    int NYlQOgFksD99479281 = -438584763;    int NYlQOgFksD99015877 = -17115918;    int NYlQOgFksD92479664 = -515994829;    int NYlQOgFksD87126572 = -824311952;    int NYlQOgFksD8724616 = -184407619;    int NYlQOgFksD7845004 = -11915641;    int NYlQOgFksD42874232 = -391251566;    int NYlQOgFksD21515909 = -85168458;    int NYlQOgFksD20268239 = -454345651;    int NYlQOgFksD66587621 = -454043392;    int NYlQOgFksD51357612 = -44435950;    int NYlQOgFksD77822097 = -463145846;    int NYlQOgFksD57596611 = -213717191;    int NYlQOgFksD36847029 = -419590620;    int NYlQOgFksD12490413 = -677789949;    int NYlQOgFksD80725790 = -50467203;    int NYlQOgFksD40660586 = 98827249;    int NYlQOgFksD90261428 = -754182114;    int NYlQOgFksD30104088 = -486206368;    int NYlQOgFksD38222048 = -761909742;    int NYlQOgFksD69934844 = -620984009;    int NYlQOgFksD70716787 = -127154153;    int NYlQOgFksD32452069 = -52539145;    int NYlQOgFksD80514806 = -413190683;    int NYlQOgFksD99039429 = -151084941;    int NYlQOgFksD26247707 = -873967494;    int NYlQOgFksD48119864 = -814762650;    int NYlQOgFksD85251418 = -964699225;    int NYlQOgFksD6711404 = -229100298;    int NYlQOgFksD74046089 = -77999652;    int NYlQOgFksD27890330 = 80481174;    int NYlQOgFksD10149244 = -347794251;    int NYlQOgFksD65143070 = -540607061;    int NYlQOgFksD73887087 = -541709573;    int NYlQOgFksD6131611 = -72866051;    int NYlQOgFksD940063 = -297848144;    int NYlQOgFksD40616574 = -676561437;    int NYlQOgFksD55953985 = -540664344;    int NYlQOgFksD34764588 = -798560710;    int NYlQOgFksD9873719 = -893434353;    int NYlQOgFksD56013324 = -321168490;    int NYlQOgFksD43256894 = -740555478;    int NYlQOgFksD28823753 = -79030894;    int NYlQOgFksD14693183 = -472612709;    int NYlQOgFksD40030205 = -4503772;    int NYlQOgFksD63716776 = -58715367;    int NYlQOgFksD73062621 = -371113934;    int NYlQOgFksD74715888 = -833874116;    int NYlQOgFksD27521991 = -747045579;    int NYlQOgFksD14253979 = -297898430;    int NYlQOgFksD74532452 = 83468363;    int NYlQOgFksD10837608 = -932841609;    int NYlQOgFksD3396874 = -605175996;    int NYlQOgFksD31813665 = -402135452;    int NYlQOgFksD76310369 = -291975798;    int NYlQOgFksD28762494 = -211430611;    int NYlQOgFksD66563809 = -964576773;    int NYlQOgFksD11964858 = -2804147;    int NYlQOgFksD88087142 = -573227011;    int NYlQOgFksD82476908 = -310440125;    int NYlQOgFksD59725139 = -197152991;    int NYlQOgFksD57622814 = -426552342;    int NYlQOgFksD14804506 = -856068160;    int NYlQOgFksD46222149 = -276345999;    int NYlQOgFksD38697291 = -434524567;    int NYlQOgFksD41208369 = -696641699;    int NYlQOgFksD12679028 = -922538785;    int NYlQOgFksD83709523 = -672007619;    int NYlQOgFksD30715418 = -246724570;    int NYlQOgFksD11550350 = -279941806;    int NYlQOgFksD40109217 = -373905766;    int NYlQOgFksD84706601 = -360508408;    int NYlQOgFksD55496841 = -955621404;    int NYlQOgFksD20230370 = -592772016;    int NYlQOgFksD82208723 = -340741252;    int NYlQOgFksD26677950 = -880428531;    int NYlQOgFksD41893035 = 51876741;    int NYlQOgFksD17758886 = -579926437;    int NYlQOgFksD40484602 = -611450813;     NYlQOgFksD5610893 = NYlQOgFksD99955940;     NYlQOgFksD99955940 = NYlQOgFksD33096238;     NYlQOgFksD33096238 = NYlQOgFksD43080557;     NYlQOgFksD43080557 = NYlQOgFksD43489203;     NYlQOgFksD43489203 = NYlQOgFksD17718722;     NYlQOgFksD17718722 = NYlQOgFksD98887556;     NYlQOgFksD98887556 = NYlQOgFksD64772803;     NYlQOgFksD64772803 = NYlQOgFksD49091991;     NYlQOgFksD49091991 = NYlQOgFksD81280803;     NYlQOgFksD81280803 = NYlQOgFksD91387817;     NYlQOgFksD91387817 = NYlQOgFksD41538874;     NYlQOgFksD41538874 = NYlQOgFksD30659233;     NYlQOgFksD30659233 = NYlQOgFksD11562917;     NYlQOgFksD11562917 = NYlQOgFksD40012403;     NYlQOgFksD40012403 = NYlQOgFksD94979768;     NYlQOgFksD94979768 = NYlQOgFksD15193039;     NYlQOgFksD15193039 = NYlQOgFksD1099036;     NYlQOgFksD1099036 = NYlQOgFksD33500962;     NYlQOgFksD33500962 = NYlQOgFksD70035712;     NYlQOgFksD70035712 = NYlQOgFksD46245214;     NYlQOgFksD46245214 = NYlQOgFksD99479281;     NYlQOgFksD99479281 = NYlQOgFksD99015877;     NYlQOgFksD99015877 = NYlQOgFksD92479664;     NYlQOgFksD92479664 = NYlQOgFksD87126572;     NYlQOgFksD87126572 = NYlQOgFksD8724616;     NYlQOgFksD8724616 = NYlQOgFksD7845004;     NYlQOgFksD7845004 = NYlQOgFksD42874232;     NYlQOgFksD42874232 = NYlQOgFksD21515909;     NYlQOgFksD21515909 = NYlQOgFksD20268239;     NYlQOgFksD20268239 = NYlQOgFksD66587621;     NYlQOgFksD66587621 = NYlQOgFksD51357612;     NYlQOgFksD51357612 = NYlQOgFksD77822097;     NYlQOgFksD77822097 = NYlQOgFksD57596611;     NYlQOgFksD57596611 = NYlQOgFksD36847029;     NYlQOgFksD36847029 = NYlQOgFksD12490413;     NYlQOgFksD12490413 = NYlQOgFksD80725790;     NYlQOgFksD80725790 = NYlQOgFksD40660586;     NYlQOgFksD40660586 = NYlQOgFksD90261428;     NYlQOgFksD90261428 = NYlQOgFksD30104088;     NYlQOgFksD30104088 = NYlQOgFksD38222048;     NYlQOgFksD38222048 = NYlQOgFksD69934844;     NYlQOgFksD69934844 = NYlQOgFksD70716787;     NYlQOgFksD70716787 = NYlQOgFksD32452069;     NYlQOgFksD32452069 = NYlQOgFksD80514806;     NYlQOgFksD80514806 = NYlQOgFksD99039429;     NYlQOgFksD99039429 = NYlQOgFksD26247707;     NYlQOgFksD26247707 = NYlQOgFksD48119864;     NYlQOgFksD48119864 = NYlQOgFksD85251418;     NYlQOgFksD85251418 = NYlQOgFksD6711404;     NYlQOgFksD6711404 = NYlQOgFksD74046089;     NYlQOgFksD74046089 = NYlQOgFksD27890330;     NYlQOgFksD27890330 = NYlQOgFksD10149244;     NYlQOgFksD10149244 = NYlQOgFksD65143070;     NYlQOgFksD65143070 = NYlQOgFksD73887087;     NYlQOgFksD73887087 = NYlQOgFksD6131611;     NYlQOgFksD6131611 = NYlQOgFksD940063;     NYlQOgFksD940063 = NYlQOgFksD40616574;     NYlQOgFksD40616574 = NYlQOgFksD55953985;     NYlQOgFksD55953985 = NYlQOgFksD34764588;     NYlQOgFksD34764588 = NYlQOgFksD9873719;     NYlQOgFksD9873719 = NYlQOgFksD56013324;     NYlQOgFksD56013324 = NYlQOgFksD43256894;     NYlQOgFksD43256894 = NYlQOgFksD28823753;     NYlQOgFksD28823753 = NYlQOgFksD14693183;     NYlQOgFksD14693183 = NYlQOgFksD40030205;     NYlQOgFksD40030205 = NYlQOgFksD63716776;     NYlQOgFksD63716776 = NYlQOgFksD73062621;     NYlQOgFksD73062621 = NYlQOgFksD74715888;     NYlQOgFksD74715888 = NYlQOgFksD27521991;     NYlQOgFksD27521991 = NYlQOgFksD14253979;     NYlQOgFksD14253979 = NYlQOgFksD74532452;     NYlQOgFksD74532452 = NYlQOgFksD10837608;     NYlQOgFksD10837608 = NYlQOgFksD3396874;     NYlQOgFksD3396874 = NYlQOgFksD31813665;     NYlQOgFksD31813665 = NYlQOgFksD76310369;     NYlQOgFksD76310369 = NYlQOgFksD28762494;     NYlQOgFksD28762494 = NYlQOgFksD66563809;     NYlQOgFksD66563809 = NYlQOgFksD11964858;     NYlQOgFksD11964858 = NYlQOgFksD88087142;     NYlQOgFksD88087142 = NYlQOgFksD82476908;     NYlQOgFksD82476908 = NYlQOgFksD59725139;     NYlQOgFksD59725139 = NYlQOgFksD57622814;     NYlQOgFksD57622814 = NYlQOgFksD14804506;     NYlQOgFksD14804506 = NYlQOgFksD46222149;     NYlQOgFksD46222149 = NYlQOgFksD38697291;     NYlQOgFksD38697291 = NYlQOgFksD41208369;     NYlQOgFksD41208369 = NYlQOgFksD12679028;     NYlQOgFksD12679028 = NYlQOgFksD83709523;     NYlQOgFksD83709523 = NYlQOgFksD30715418;     NYlQOgFksD30715418 = NYlQOgFksD11550350;     NYlQOgFksD11550350 = NYlQOgFksD40109217;     NYlQOgFksD40109217 = NYlQOgFksD84706601;     NYlQOgFksD84706601 = NYlQOgFksD55496841;     NYlQOgFksD55496841 = NYlQOgFksD20230370;     NYlQOgFksD20230370 = NYlQOgFksD82208723;     NYlQOgFksD82208723 = NYlQOgFksD26677950;     NYlQOgFksD26677950 = NYlQOgFksD41893035;     NYlQOgFksD41893035 = NYlQOgFksD17758886;     NYlQOgFksD17758886 = NYlQOgFksD40484602;     NYlQOgFksD40484602 = NYlQOgFksD5610893;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WinQmvqPyp90257976() {     int MbtRlVWHNf73057928 = -211494948;    int MbtRlVWHNf95152131 = -22267764;    int MbtRlVWHNf10268472 = 70828353;    int MbtRlVWHNf37376463 = -374740153;    int MbtRlVWHNf2611876 = -778864712;    int MbtRlVWHNf40397757 = -111991331;    int MbtRlVWHNf72648682 = -855448332;    int MbtRlVWHNf51452968 = 33366892;    int MbtRlVWHNf49531048 = -928341937;    int MbtRlVWHNf73491980 = -372738131;    int MbtRlVWHNf77165889 = -929510236;    int MbtRlVWHNf21962416 = -480139754;    int MbtRlVWHNf32766936 = -370495611;    int MbtRlVWHNf309206 = -952830461;    int MbtRlVWHNf48370920 = -276230139;    int MbtRlVWHNf38607826 = -296620935;    int MbtRlVWHNf14923860 = -235464114;    int MbtRlVWHNf29253575 = 91273130;    int MbtRlVWHNf15453924 = -768680841;    int MbtRlVWHNf18251002 = -496631028;    int MbtRlVWHNf80924234 = -161892847;    int MbtRlVWHNf1483424 = -264895335;    int MbtRlVWHNf37367608 = -75905391;    int MbtRlVWHNf94765054 = -694460136;    int MbtRlVWHNf60330758 = -980973475;    int MbtRlVWHNf14710131 = 29658326;    int MbtRlVWHNf51028882 = -899666487;    int MbtRlVWHNf28072633 = 35865457;    int MbtRlVWHNf65352192 = -33560258;    int MbtRlVWHNf91364892 = -699558400;    int MbtRlVWHNf29316467 = -892239433;    int MbtRlVWHNf944189 = -42830395;    int MbtRlVWHNf6071500 = -693574029;    int MbtRlVWHNf29046039 = 78089514;    int MbtRlVWHNf72053679 = -988171836;    int MbtRlVWHNf52212880 = -634582904;    int MbtRlVWHNf80252970 = -691149796;    int MbtRlVWHNf41424168 = -799459185;    int MbtRlVWHNf11492867 = -265299877;    int MbtRlVWHNf8482320 = -379500127;    int MbtRlVWHNf67757089 = -61989995;    int MbtRlVWHNf28614298 = -933035348;    int MbtRlVWHNf36317080 = -531962661;    int MbtRlVWHNf26843108 = -310426610;    int MbtRlVWHNf36489308 = 34781161;    int MbtRlVWHNf87573192 = -481148325;    int MbtRlVWHNf92605646 = -917259308;    int MbtRlVWHNf25603559 = -339006445;    int MbtRlVWHNf52999845 = -364267527;    int MbtRlVWHNf78458063 = -393131169;    int MbtRlVWHNf54227753 = -409544455;    int MbtRlVWHNf30901769 = -523624398;    int MbtRlVWHNf914049 = -371658983;    int MbtRlVWHNf57008281 = -594025436;    int MbtRlVWHNf9333743 = -75163463;    int MbtRlVWHNf71574504 = -946599613;    int MbtRlVWHNf57784523 = -946362373;    int MbtRlVWHNf15503418 = -234711512;    int MbtRlVWHNf77045705 = -393766679;    int MbtRlVWHNf87901745 = -708523038;    int MbtRlVWHNf89368874 = -212324845;    int MbtRlVWHNf44576050 = -791313789;    int MbtRlVWHNf86100776 = -933072850;    int MbtRlVWHNf58166155 = -128783538;    int MbtRlVWHNf44175513 = -480498699;    int MbtRlVWHNf76221700 = -786679842;    int MbtRlVWHNf15890916 = -786565726;    int MbtRlVWHNf3720898 = -348585126;    int MbtRlVWHNf28255527 = -964658626;    int MbtRlVWHNf96158040 = -641647235;    int MbtRlVWHNf58354855 = -605471140;    int MbtRlVWHNf73499691 = -436004929;    int MbtRlVWHNf17760709 = -643426994;    int MbtRlVWHNf6971604 = -289180714;    int MbtRlVWHNf50493912 = -334641033;    int MbtRlVWHNf52309936 = -228857500;    int MbtRlVWHNf65166344 = -732932675;    int MbtRlVWHNf10524501 = -765478782;    int MbtRlVWHNf58275746 = -629241297;    int MbtRlVWHNf72757565 = -399825151;    int MbtRlVWHNf22104485 = -53082367;    int MbtRlVWHNf25425324 = -460660042;    int MbtRlVWHNf75072787 = -599867017;    int MbtRlVWHNf86894128 = -640429090;    int MbtRlVWHNf37137140 = -190013945;    int MbtRlVWHNf98414697 = -268615035;    int MbtRlVWHNf30141 = -671171412;    int MbtRlVWHNf49063218 = 451407;    int MbtRlVWHNf19712297 = -846747024;    int MbtRlVWHNf479176 = 58427777;    int MbtRlVWHNf94428356 = -688220532;    int MbtRlVWHNf64749553 = -356438284;    int MbtRlVWHNf64378463 = -305692507;    int MbtRlVWHNf23591122 = -556776840;    int MbtRlVWHNf19113445 = -67175283;    int MbtRlVWHNf23181040 = -270676206;    int MbtRlVWHNf42513521 = -999962498;    int MbtRlVWHNf78150925 = -303179124;    int MbtRlVWHNf82667594 = -829927912;    int MbtRlVWHNf60267608 = -211494948;     MbtRlVWHNf73057928 = MbtRlVWHNf95152131;     MbtRlVWHNf95152131 = MbtRlVWHNf10268472;     MbtRlVWHNf10268472 = MbtRlVWHNf37376463;     MbtRlVWHNf37376463 = MbtRlVWHNf2611876;     MbtRlVWHNf2611876 = MbtRlVWHNf40397757;     MbtRlVWHNf40397757 = MbtRlVWHNf72648682;     MbtRlVWHNf72648682 = MbtRlVWHNf51452968;     MbtRlVWHNf51452968 = MbtRlVWHNf49531048;     MbtRlVWHNf49531048 = MbtRlVWHNf73491980;     MbtRlVWHNf73491980 = MbtRlVWHNf77165889;     MbtRlVWHNf77165889 = MbtRlVWHNf21962416;     MbtRlVWHNf21962416 = MbtRlVWHNf32766936;     MbtRlVWHNf32766936 = MbtRlVWHNf309206;     MbtRlVWHNf309206 = MbtRlVWHNf48370920;     MbtRlVWHNf48370920 = MbtRlVWHNf38607826;     MbtRlVWHNf38607826 = MbtRlVWHNf14923860;     MbtRlVWHNf14923860 = MbtRlVWHNf29253575;     MbtRlVWHNf29253575 = MbtRlVWHNf15453924;     MbtRlVWHNf15453924 = MbtRlVWHNf18251002;     MbtRlVWHNf18251002 = MbtRlVWHNf80924234;     MbtRlVWHNf80924234 = MbtRlVWHNf1483424;     MbtRlVWHNf1483424 = MbtRlVWHNf37367608;     MbtRlVWHNf37367608 = MbtRlVWHNf94765054;     MbtRlVWHNf94765054 = MbtRlVWHNf60330758;     MbtRlVWHNf60330758 = MbtRlVWHNf14710131;     MbtRlVWHNf14710131 = MbtRlVWHNf51028882;     MbtRlVWHNf51028882 = MbtRlVWHNf28072633;     MbtRlVWHNf28072633 = MbtRlVWHNf65352192;     MbtRlVWHNf65352192 = MbtRlVWHNf91364892;     MbtRlVWHNf91364892 = MbtRlVWHNf29316467;     MbtRlVWHNf29316467 = MbtRlVWHNf944189;     MbtRlVWHNf944189 = MbtRlVWHNf6071500;     MbtRlVWHNf6071500 = MbtRlVWHNf29046039;     MbtRlVWHNf29046039 = MbtRlVWHNf72053679;     MbtRlVWHNf72053679 = MbtRlVWHNf52212880;     MbtRlVWHNf52212880 = MbtRlVWHNf80252970;     MbtRlVWHNf80252970 = MbtRlVWHNf41424168;     MbtRlVWHNf41424168 = MbtRlVWHNf11492867;     MbtRlVWHNf11492867 = MbtRlVWHNf8482320;     MbtRlVWHNf8482320 = MbtRlVWHNf67757089;     MbtRlVWHNf67757089 = MbtRlVWHNf28614298;     MbtRlVWHNf28614298 = MbtRlVWHNf36317080;     MbtRlVWHNf36317080 = MbtRlVWHNf26843108;     MbtRlVWHNf26843108 = MbtRlVWHNf36489308;     MbtRlVWHNf36489308 = MbtRlVWHNf87573192;     MbtRlVWHNf87573192 = MbtRlVWHNf92605646;     MbtRlVWHNf92605646 = MbtRlVWHNf25603559;     MbtRlVWHNf25603559 = MbtRlVWHNf52999845;     MbtRlVWHNf52999845 = MbtRlVWHNf78458063;     MbtRlVWHNf78458063 = MbtRlVWHNf54227753;     MbtRlVWHNf54227753 = MbtRlVWHNf30901769;     MbtRlVWHNf30901769 = MbtRlVWHNf914049;     MbtRlVWHNf914049 = MbtRlVWHNf57008281;     MbtRlVWHNf57008281 = MbtRlVWHNf9333743;     MbtRlVWHNf9333743 = MbtRlVWHNf71574504;     MbtRlVWHNf71574504 = MbtRlVWHNf57784523;     MbtRlVWHNf57784523 = MbtRlVWHNf15503418;     MbtRlVWHNf15503418 = MbtRlVWHNf77045705;     MbtRlVWHNf77045705 = MbtRlVWHNf87901745;     MbtRlVWHNf87901745 = MbtRlVWHNf89368874;     MbtRlVWHNf89368874 = MbtRlVWHNf44576050;     MbtRlVWHNf44576050 = MbtRlVWHNf86100776;     MbtRlVWHNf86100776 = MbtRlVWHNf58166155;     MbtRlVWHNf58166155 = MbtRlVWHNf44175513;     MbtRlVWHNf44175513 = MbtRlVWHNf76221700;     MbtRlVWHNf76221700 = MbtRlVWHNf15890916;     MbtRlVWHNf15890916 = MbtRlVWHNf3720898;     MbtRlVWHNf3720898 = MbtRlVWHNf28255527;     MbtRlVWHNf28255527 = MbtRlVWHNf96158040;     MbtRlVWHNf96158040 = MbtRlVWHNf58354855;     MbtRlVWHNf58354855 = MbtRlVWHNf73499691;     MbtRlVWHNf73499691 = MbtRlVWHNf17760709;     MbtRlVWHNf17760709 = MbtRlVWHNf6971604;     MbtRlVWHNf6971604 = MbtRlVWHNf50493912;     MbtRlVWHNf50493912 = MbtRlVWHNf52309936;     MbtRlVWHNf52309936 = MbtRlVWHNf65166344;     MbtRlVWHNf65166344 = MbtRlVWHNf10524501;     MbtRlVWHNf10524501 = MbtRlVWHNf58275746;     MbtRlVWHNf58275746 = MbtRlVWHNf72757565;     MbtRlVWHNf72757565 = MbtRlVWHNf22104485;     MbtRlVWHNf22104485 = MbtRlVWHNf25425324;     MbtRlVWHNf25425324 = MbtRlVWHNf75072787;     MbtRlVWHNf75072787 = MbtRlVWHNf86894128;     MbtRlVWHNf86894128 = MbtRlVWHNf37137140;     MbtRlVWHNf37137140 = MbtRlVWHNf98414697;     MbtRlVWHNf98414697 = MbtRlVWHNf30141;     MbtRlVWHNf30141 = MbtRlVWHNf49063218;     MbtRlVWHNf49063218 = MbtRlVWHNf19712297;     MbtRlVWHNf19712297 = MbtRlVWHNf479176;     MbtRlVWHNf479176 = MbtRlVWHNf94428356;     MbtRlVWHNf94428356 = MbtRlVWHNf64749553;     MbtRlVWHNf64749553 = MbtRlVWHNf64378463;     MbtRlVWHNf64378463 = MbtRlVWHNf23591122;     MbtRlVWHNf23591122 = MbtRlVWHNf19113445;     MbtRlVWHNf19113445 = MbtRlVWHNf23181040;     MbtRlVWHNf23181040 = MbtRlVWHNf42513521;     MbtRlVWHNf42513521 = MbtRlVWHNf78150925;     MbtRlVWHNf78150925 = MbtRlVWHNf82667594;     MbtRlVWHNf82667594 = MbtRlVWHNf60267608;     MbtRlVWHNf60267608 = MbtRlVWHNf73057928;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AMSRRyorKp60713850() {     int vfWIwlqQyp85844095 = -346535639;    int vfWIwlqQyp81923019 = -192601777;    int vfWIwlqQyp28021272 = -337531029;    int vfWIwlqQyp45317289 = -198942430;    int vfWIwlqQyp96122980 = -170681177;    int vfWIwlqQyp83707827 = -988914644;    int vfWIwlqQyp93856440 = -301120403;    int vfWIwlqQyp78172081 = -411151364;    int vfWIwlqQyp44935858 = -890100029;    int vfWIwlqQyp79710685 = -176825764;    int vfWIwlqQyp4653272 = 55803748;    int vfWIwlqQyp53912920 = -987472570;    int vfWIwlqQyp2910239 = -274278269;    int vfWIwlqQyp49933284 = -185245659;    int vfWIwlqQyp71677473 = -183215970;    int vfWIwlqQyp67032894 = -847512325;    int vfWIwlqQyp70675677 = -999238581;    int vfWIwlqQyp12812060 = -563937172;    int vfWIwlqQyp16614554 = -678150036;    int vfWIwlqQyp20905498 = 4124833;    int vfWIwlqQyp61098291 = -653209112;    int vfWIwlqQyp30531243 = -434753683;    int vfWIwlqQyp76127904 = -6909286;    int vfWIwlqQyp39272184 = 45194852;    int vfWIwlqQyp66225017 = -91415399;    int vfWIwlqQyp96028776 = -760772874;    int vfWIwlqQyp62444418 = -433372767;    int vfWIwlqQyp95425557 = -137932648;    int vfWIwlqQyp81969244 = -247777498;    int vfWIwlqQyp44116008 = -329444554;    int vfWIwlqQyp73752463 = -68656183;    int vfWIwlqQyp4263013 = -21550073;    int vfWIwlqQyp52117449 = -569428813;    int vfWIwlqQyp6122412 = -926664705;    int vfWIwlqQyp55114211 = -498203716;    int vfWIwlqQyp47436995 = -423146008;    int vfWIwlqQyp17017031 = -481059146;    int vfWIwlqQyp99128191 = -719079276;    int vfWIwlqQyp17268253 = -107150454;    int vfWIwlqQyp34747050 = -223916973;    int vfWIwlqQyp29188398 = -437463988;    int vfWIwlqQyp36316754 = -831346682;    int vfWIwlqQyp71200550 = -555396106;    int vfWIwlqQyp71471645 = -651598853;    int vfWIwlqQyp16497799 = -957988800;    int vfWIwlqQyp61201322 = -554466764;    int vfWIwlqQyp70187665 = -314774361;    int vfWIwlqQyp55043525 = -494359225;    int vfWIwlqQyp97063894 = -527869502;    int vfWIwlqQyp25408484 = 95832368;    int vfWIwlqQyp72555246 = -662428764;    int vfWIwlqQyp59078316 = -994809736;    int vfWIwlqQyp35399199 = -547662265;    int vfWIwlqQyp1708007 = -183477178;    int vfWIwlqQyp99615558 = -69607949;    int vfWIwlqQyp55312852 = -911781956;    int vfWIwlqQyp5795116 = -85692492;    int vfWIwlqQyp88749088 = -282725882;    int vfWIwlqQyp79092271 = -7527031;    int vfWIwlqQyp94205 = -409908304;    int vfWIwlqQyp21263410 = -455541878;    int vfWIwlqQyp98430883 = -63187755;    int vfWIwlqQyp96202836 = -63373866;    int vfWIwlqQyp819850 = -460655475;    int vfWIwlqQyp5958223 = -8169582;    int vfWIwlqQyp390259 = -922646179;    int vfWIwlqQyp1795472 = -318043758;    int vfWIwlqQyp96787827 = -347613565;    int vfWIwlqQyp94819072 = -687041944;    int vfWIwlqQyp24240479 = -760069962;    int vfWIwlqQyp50015864 = -266453179;    int vfWIwlqQyp71547486 = -180159305;    int vfWIwlqQyp95543807 = -356786719;    int vfWIwlqQyp81867504 = -354233064;    int vfWIwlqQyp91717100 = -558411179;    int vfWIwlqQyp24781538 = -821862430;    int vfWIwlqQyp59330693 = -879357578;    int vfWIwlqQyp4656259 = -355310434;    int vfWIwlqQyp22774385 = 3183651;    int vfWIwlqQyp5023696 = -536948636;    int vfWIwlqQyp25841111 = -345998514;    int vfWIwlqQyp7400894 = -939013543;    int vfWIwlqQyp98361663 = -610063147;    int vfWIwlqQyp56560761 = -243609867;    int vfWIwlqQyp71560762 = -667015791;    int vfWIwlqQyp14674147 = -73846447;    int vfWIwlqQyp68863814 = -473887808;    int vfWIwlqQyp50409442 = -285951635;    int vfWIwlqQyp6506854 = -757056756;    int vfWIwlqQyp99801359 = -586421760;    int vfWIwlqQyp41641879 = -237453517;    int vfWIwlqQyp28267943 = -98333265;    int vfWIwlqQyp20035920 = -611552246;    int vfWIwlqQyp17174048 = -697242151;    int vfWIwlqQyp13483641 = -768375096;    int vfWIwlqQyp30757514 = -274276233;    int vfWIwlqQyp40113918 = -667972816;    int vfWIwlqQyp70380700 = 5259369;    int vfWIwlqQyp65513423 = -543429272;    int vfWIwlqQyp16107540 = -346535639;     vfWIwlqQyp85844095 = vfWIwlqQyp81923019;     vfWIwlqQyp81923019 = vfWIwlqQyp28021272;     vfWIwlqQyp28021272 = vfWIwlqQyp45317289;     vfWIwlqQyp45317289 = vfWIwlqQyp96122980;     vfWIwlqQyp96122980 = vfWIwlqQyp83707827;     vfWIwlqQyp83707827 = vfWIwlqQyp93856440;     vfWIwlqQyp93856440 = vfWIwlqQyp78172081;     vfWIwlqQyp78172081 = vfWIwlqQyp44935858;     vfWIwlqQyp44935858 = vfWIwlqQyp79710685;     vfWIwlqQyp79710685 = vfWIwlqQyp4653272;     vfWIwlqQyp4653272 = vfWIwlqQyp53912920;     vfWIwlqQyp53912920 = vfWIwlqQyp2910239;     vfWIwlqQyp2910239 = vfWIwlqQyp49933284;     vfWIwlqQyp49933284 = vfWIwlqQyp71677473;     vfWIwlqQyp71677473 = vfWIwlqQyp67032894;     vfWIwlqQyp67032894 = vfWIwlqQyp70675677;     vfWIwlqQyp70675677 = vfWIwlqQyp12812060;     vfWIwlqQyp12812060 = vfWIwlqQyp16614554;     vfWIwlqQyp16614554 = vfWIwlqQyp20905498;     vfWIwlqQyp20905498 = vfWIwlqQyp61098291;     vfWIwlqQyp61098291 = vfWIwlqQyp30531243;     vfWIwlqQyp30531243 = vfWIwlqQyp76127904;     vfWIwlqQyp76127904 = vfWIwlqQyp39272184;     vfWIwlqQyp39272184 = vfWIwlqQyp66225017;     vfWIwlqQyp66225017 = vfWIwlqQyp96028776;     vfWIwlqQyp96028776 = vfWIwlqQyp62444418;     vfWIwlqQyp62444418 = vfWIwlqQyp95425557;     vfWIwlqQyp95425557 = vfWIwlqQyp81969244;     vfWIwlqQyp81969244 = vfWIwlqQyp44116008;     vfWIwlqQyp44116008 = vfWIwlqQyp73752463;     vfWIwlqQyp73752463 = vfWIwlqQyp4263013;     vfWIwlqQyp4263013 = vfWIwlqQyp52117449;     vfWIwlqQyp52117449 = vfWIwlqQyp6122412;     vfWIwlqQyp6122412 = vfWIwlqQyp55114211;     vfWIwlqQyp55114211 = vfWIwlqQyp47436995;     vfWIwlqQyp47436995 = vfWIwlqQyp17017031;     vfWIwlqQyp17017031 = vfWIwlqQyp99128191;     vfWIwlqQyp99128191 = vfWIwlqQyp17268253;     vfWIwlqQyp17268253 = vfWIwlqQyp34747050;     vfWIwlqQyp34747050 = vfWIwlqQyp29188398;     vfWIwlqQyp29188398 = vfWIwlqQyp36316754;     vfWIwlqQyp36316754 = vfWIwlqQyp71200550;     vfWIwlqQyp71200550 = vfWIwlqQyp71471645;     vfWIwlqQyp71471645 = vfWIwlqQyp16497799;     vfWIwlqQyp16497799 = vfWIwlqQyp61201322;     vfWIwlqQyp61201322 = vfWIwlqQyp70187665;     vfWIwlqQyp70187665 = vfWIwlqQyp55043525;     vfWIwlqQyp55043525 = vfWIwlqQyp97063894;     vfWIwlqQyp97063894 = vfWIwlqQyp25408484;     vfWIwlqQyp25408484 = vfWIwlqQyp72555246;     vfWIwlqQyp72555246 = vfWIwlqQyp59078316;     vfWIwlqQyp59078316 = vfWIwlqQyp35399199;     vfWIwlqQyp35399199 = vfWIwlqQyp1708007;     vfWIwlqQyp1708007 = vfWIwlqQyp99615558;     vfWIwlqQyp99615558 = vfWIwlqQyp55312852;     vfWIwlqQyp55312852 = vfWIwlqQyp5795116;     vfWIwlqQyp5795116 = vfWIwlqQyp88749088;     vfWIwlqQyp88749088 = vfWIwlqQyp79092271;     vfWIwlqQyp79092271 = vfWIwlqQyp94205;     vfWIwlqQyp94205 = vfWIwlqQyp21263410;     vfWIwlqQyp21263410 = vfWIwlqQyp98430883;     vfWIwlqQyp98430883 = vfWIwlqQyp96202836;     vfWIwlqQyp96202836 = vfWIwlqQyp819850;     vfWIwlqQyp819850 = vfWIwlqQyp5958223;     vfWIwlqQyp5958223 = vfWIwlqQyp390259;     vfWIwlqQyp390259 = vfWIwlqQyp1795472;     vfWIwlqQyp1795472 = vfWIwlqQyp96787827;     vfWIwlqQyp96787827 = vfWIwlqQyp94819072;     vfWIwlqQyp94819072 = vfWIwlqQyp24240479;     vfWIwlqQyp24240479 = vfWIwlqQyp50015864;     vfWIwlqQyp50015864 = vfWIwlqQyp71547486;     vfWIwlqQyp71547486 = vfWIwlqQyp95543807;     vfWIwlqQyp95543807 = vfWIwlqQyp81867504;     vfWIwlqQyp81867504 = vfWIwlqQyp91717100;     vfWIwlqQyp91717100 = vfWIwlqQyp24781538;     vfWIwlqQyp24781538 = vfWIwlqQyp59330693;     vfWIwlqQyp59330693 = vfWIwlqQyp4656259;     vfWIwlqQyp4656259 = vfWIwlqQyp22774385;     vfWIwlqQyp22774385 = vfWIwlqQyp5023696;     vfWIwlqQyp5023696 = vfWIwlqQyp25841111;     vfWIwlqQyp25841111 = vfWIwlqQyp7400894;     vfWIwlqQyp7400894 = vfWIwlqQyp98361663;     vfWIwlqQyp98361663 = vfWIwlqQyp56560761;     vfWIwlqQyp56560761 = vfWIwlqQyp71560762;     vfWIwlqQyp71560762 = vfWIwlqQyp14674147;     vfWIwlqQyp14674147 = vfWIwlqQyp68863814;     vfWIwlqQyp68863814 = vfWIwlqQyp50409442;     vfWIwlqQyp50409442 = vfWIwlqQyp6506854;     vfWIwlqQyp6506854 = vfWIwlqQyp99801359;     vfWIwlqQyp99801359 = vfWIwlqQyp41641879;     vfWIwlqQyp41641879 = vfWIwlqQyp28267943;     vfWIwlqQyp28267943 = vfWIwlqQyp20035920;     vfWIwlqQyp20035920 = vfWIwlqQyp17174048;     vfWIwlqQyp17174048 = vfWIwlqQyp13483641;     vfWIwlqQyp13483641 = vfWIwlqQyp30757514;     vfWIwlqQyp30757514 = vfWIwlqQyp40113918;     vfWIwlqQyp40113918 = vfWIwlqQyp70380700;     vfWIwlqQyp70380700 = vfWIwlqQyp65513423;     vfWIwlqQyp65513423 = vfWIwlqQyp16107540;     vfWIwlqQyp16107540 = vfWIwlqQyp85844095;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FNNEPirVWO78927191() {     int tuutxosEKn69288401 = -127201320;    int tuutxosEKn98795108 = -191983965;    int tuutxosEKn24429779 = -131056257;    int tuutxosEKn8109541 = -392742266;    int tuutxosEKn2244305 = 22451332;    int tuutxosEKn82212228 = -767986297;    int tuutxosEKn64895758 = -41031716;    int tuutxosEKn92519523 = -398309394;    int tuutxosEKn3465559 = -131559380;    int tuutxosEKn12256337 = -645201876;    int tuutxosEKn54069209 = -134312125;    int tuutxosEKn17118320 = -975090817;    int tuutxosEKn21028015 = -65939510;    int tuutxosEKn29560177 = 26463984;    int tuutxosEKn32531937 = -277238952;    int tuutxosEKn53422056 = -635591650;    int tuutxosEKn64427562 = -752583608;    int tuutxosEKn93663506 = -811242049;    int tuutxosEKn79054696 = -676824907;    int tuutxosEKn10272941 = -525461159;    int tuutxosEKn16738678 = -242738143;    int tuutxosEKn42678561 = -425905454;    int tuutxosEKn79953611 = -46798483;    int tuutxosEKn73978742 = -26630544;    int tuutxosEKn7800976 = -651068158;    int tuutxosEKn91370925 = -641282589;    int tuutxosEKn11899159 = -61457109;    int tuutxosEKn35157183 = -207887532;    int tuutxosEKn18050339 = -110282641;    int tuutxosEKn59949349 = -842294406;    int tuutxosEKn91847776 = -806413384;    int tuutxosEKn67369439 = 63254847;    int tuutxosEKn95656335 = -190300991;    int tuutxosEKn48280922 = -518266875;    int tuutxosEKn33840942 = -695868451;    int tuutxosEKn59826179 = -211169914;    int tuutxosEKn85631209 = -359908326;    int tuutxosEKn39707023 = -427549796;    int tuutxosEKn66263330 = -292048252;    int tuutxosEKn84242184 = 23489505;    int tuutxosEKn62837322 = -9012961;    int tuutxosEKn86804539 = -305255003;    int tuutxosEKn94767619 = -6453349;    int tuutxosEKn63376585 = -407781624;    int tuutxosEKn8442296 = -686145753;    int tuutxosEKn807319 = -574077688;    int tuutxosEKn46203096 = -447078590;    int tuutxosEKn74714120 = -493080943;    int tuutxosEKn27911007 = -13471584;    int tuutxosEKn60740968 = -111188087;    int tuutxosEKn45812694 = -883330186;    int tuutxosEKn14579111 = 46930952;    int tuutxosEKn49511657 = -759409582;    int tuutxosEKn26384766 = 98109434;    int tuutxosEKn62188513 = -418094072;    int tuutxosEKn26609840 = -701295867;    int tuutxosEKn18841498 = -45185482;    int tuutxosEKn50451037 = -4425713;    int tuutxosEKn308565 = -741674108;    int tuutxosEKn10873380 = -336266079;    int tuutxosEKn70313070 = -606529188;    int tuutxosEKn29738576 = -833144185;    int tuutxosEKn74469185 = -188026754;    int tuutxosEKn43516209 = -289264975;    int tuutxosEKn20408561 = -838788493;    int tuutxosEKn86699769 = -97566972;    int tuutxosEKn21461984 = -684789827;    int tuutxosEKn72747092 = -547672636;    int tuutxosEKn95719234 = -277667566;    int tuutxosEKn72705758 = 33930962;    int tuutxosEKn67790846 = -175683324;    int tuutxosEKn24720540 = -225033812;    int tuutxosEKn27400176 = -419193798;    int tuutxosEKn94812511 = -600314412;    int tuutxosEKn47435618 = -416448198;    int tuutxosEKn29934138 = -937483140;    int tuutxosEKn47910942 = -319452105;    int tuutxosEKn16577026 = -639016860;    int tuutxosEKn65536446 = -340484792;    int tuutxosEKn6993658 = 23009529;    int tuutxosEKn45167829 = -94204000;    int tuutxosEKn37185038 = -568376166;    int tuutxosEKn7246177 = -94415948;    int tuutxosEKn57309371 = -999094554;    int tuutxosEKn14136656 = -958964221;    int tuutxosEKn77268666 = -753344336;    int tuutxosEKn17857782 = -177335572;    int tuutxosEKn69271570 = -188410425;    int tuutxosEKn86092409 = -172803;    int tuutxosEKn7231103 = -994572585;    int tuutxosEKn40984682 = -65984432;    int tuutxosEKn35180173 = -255482613;    int tuutxosEKn39398458 = -685875689;    int tuutxosEKn55389951 = -955782173;    int tuutxosEKn13929115 = -369981307;    int tuutxosEKn33098746 = -175868777;    int tuutxosEKn12335355 = -17228250;    int tuutxosEKn51251410 = -717188375;    int tuutxosEKn42968025 = -568993132;    int tuutxosEKn21742526 = -127201320;     tuutxosEKn69288401 = tuutxosEKn98795108;     tuutxosEKn98795108 = tuutxosEKn24429779;     tuutxosEKn24429779 = tuutxosEKn8109541;     tuutxosEKn8109541 = tuutxosEKn2244305;     tuutxosEKn2244305 = tuutxosEKn82212228;     tuutxosEKn82212228 = tuutxosEKn64895758;     tuutxosEKn64895758 = tuutxosEKn92519523;     tuutxosEKn92519523 = tuutxosEKn3465559;     tuutxosEKn3465559 = tuutxosEKn12256337;     tuutxosEKn12256337 = tuutxosEKn54069209;     tuutxosEKn54069209 = tuutxosEKn17118320;     tuutxosEKn17118320 = tuutxosEKn21028015;     tuutxosEKn21028015 = tuutxosEKn29560177;     tuutxosEKn29560177 = tuutxosEKn32531937;     tuutxosEKn32531937 = tuutxosEKn53422056;     tuutxosEKn53422056 = tuutxosEKn64427562;     tuutxosEKn64427562 = tuutxosEKn93663506;     tuutxosEKn93663506 = tuutxosEKn79054696;     tuutxosEKn79054696 = tuutxosEKn10272941;     tuutxosEKn10272941 = tuutxosEKn16738678;     tuutxosEKn16738678 = tuutxosEKn42678561;     tuutxosEKn42678561 = tuutxosEKn79953611;     tuutxosEKn79953611 = tuutxosEKn73978742;     tuutxosEKn73978742 = tuutxosEKn7800976;     tuutxosEKn7800976 = tuutxosEKn91370925;     tuutxosEKn91370925 = tuutxosEKn11899159;     tuutxosEKn11899159 = tuutxosEKn35157183;     tuutxosEKn35157183 = tuutxosEKn18050339;     tuutxosEKn18050339 = tuutxosEKn59949349;     tuutxosEKn59949349 = tuutxosEKn91847776;     tuutxosEKn91847776 = tuutxosEKn67369439;     tuutxosEKn67369439 = tuutxosEKn95656335;     tuutxosEKn95656335 = tuutxosEKn48280922;     tuutxosEKn48280922 = tuutxosEKn33840942;     tuutxosEKn33840942 = tuutxosEKn59826179;     tuutxosEKn59826179 = tuutxosEKn85631209;     tuutxosEKn85631209 = tuutxosEKn39707023;     tuutxosEKn39707023 = tuutxosEKn66263330;     tuutxosEKn66263330 = tuutxosEKn84242184;     tuutxosEKn84242184 = tuutxosEKn62837322;     tuutxosEKn62837322 = tuutxosEKn86804539;     tuutxosEKn86804539 = tuutxosEKn94767619;     tuutxosEKn94767619 = tuutxosEKn63376585;     tuutxosEKn63376585 = tuutxosEKn8442296;     tuutxosEKn8442296 = tuutxosEKn807319;     tuutxosEKn807319 = tuutxosEKn46203096;     tuutxosEKn46203096 = tuutxosEKn74714120;     tuutxosEKn74714120 = tuutxosEKn27911007;     tuutxosEKn27911007 = tuutxosEKn60740968;     tuutxosEKn60740968 = tuutxosEKn45812694;     tuutxosEKn45812694 = tuutxosEKn14579111;     tuutxosEKn14579111 = tuutxosEKn49511657;     tuutxosEKn49511657 = tuutxosEKn26384766;     tuutxosEKn26384766 = tuutxosEKn62188513;     tuutxosEKn62188513 = tuutxosEKn26609840;     tuutxosEKn26609840 = tuutxosEKn18841498;     tuutxosEKn18841498 = tuutxosEKn50451037;     tuutxosEKn50451037 = tuutxosEKn308565;     tuutxosEKn308565 = tuutxosEKn10873380;     tuutxosEKn10873380 = tuutxosEKn70313070;     tuutxosEKn70313070 = tuutxosEKn29738576;     tuutxosEKn29738576 = tuutxosEKn74469185;     tuutxosEKn74469185 = tuutxosEKn43516209;     tuutxosEKn43516209 = tuutxosEKn20408561;     tuutxosEKn20408561 = tuutxosEKn86699769;     tuutxosEKn86699769 = tuutxosEKn21461984;     tuutxosEKn21461984 = tuutxosEKn72747092;     tuutxosEKn72747092 = tuutxosEKn95719234;     tuutxosEKn95719234 = tuutxosEKn72705758;     tuutxosEKn72705758 = tuutxosEKn67790846;     tuutxosEKn67790846 = tuutxosEKn24720540;     tuutxosEKn24720540 = tuutxosEKn27400176;     tuutxosEKn27400176 = tuutxosEKn94812511;     tuutxosEKn94812511 = tuutxosEKn47435618;     tuutxosEKn47435618 = tuutxosEKn29934138;     tuutxosEKn29934138 = tuutxosEKn47910942;     tuutxosEKn47910942 = tuutxosEKn16577026;     tuutxosEKn16577026 = tuutxosEKn65536446;     tuutxosEKn65536446 = tuutxosEKn6993658;     tuutxosEKn6993658 = tuutxosEKn45167829;     tuutxosEKn45167829 = tuutxosEKn37185038;     tuutxosEKn37185038 = tuutxosEKn7246177;     tuutxosEKn7246177 = tuutxosEKn57309371;     tuutxosEKn57309371 = tuutxosEKn14136656;     tuutxosEKn14136656 = tuutxosEKn77268666;     tuutxosEKn77268666 = tuutxosEKn17857782;     tuutxosEKn17857782 = tuutxosEKn69271570;     tuutxosEKn69271570 = tuutxosEKn86092409;     tuutxosEKn86092409 = tuutxosEKn7231103;     tuutxosEKn7231103 = tuutxosEKn40984682;     tuutxosEKn40984682 = tuutxosEKn35180173;     tuutxosEKn35180173 = tuutxosEKn39398458;     tuutxosEKn39398458 = tuutxosEKn55389951;     tuutxosEKn55389951 = tuutxosEKn13929115;     tuutxosEKn13929115 = tuutxosEKn33098746;     tuutxosEKn33098746 = tuutxosEKn12335355;     tuutxosEKn12335355 = tuutxosEKn51251410;     tuutxosEKn51251410 = tuutxosEKn42968025;     tuutxosEKn42968025 = tuutxosEKn21742526;     tuutxosEKn21742526 = tuutxosEKn69288401;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CvfCCSkUNA49383064() {     int TPyNGQOowV82074568 = -262242012;    int TPyNGQOowV85565997 = -362317979;    int TPyNGQOowV42182579 = -539415639;    int TPyNGQOowV16050366 = -216944543;    int TPyNGQOowV95755408 = -469365133;    int TPyNGQOowV25522299 = -544909611;    int TPyNGQOowV86103516 = -586703786;    int TPyNGQOowV19238637 = -842827650;    int TPyNGQOowV98870369 = -93317472;    int TPyNGQOowV18475042 = -449289509;    int TPyNGQOowV81556591 = -248998140;    int TPyNGQOowV49068825 = -382423633;    int TPyNGQOowV91171317 = 30277833;    int TPyNGQOowV79184255 = -305951215;    int TPyNGQOowV55838490 = -184224783;    int TPyNGQOowV81847124 = -86483039;    int TPyNGQOowV20179380 = -416358074;    int TPyNGQOowV77221991 = -366452351;    int TPyNGQOowV80215326 = -586294102;    int TPyNGQOowV12927437 = -24705298;    int TPyNGQOowV96912734 = -734054407;    int TPyNGQOowV71726380 = -595763802;    int TPyNGQOowV18713907 = 22197622;    int TPyNGQOowV18485872 = -386975556;    int TPyNGQOowV13695235 = -861510083;    int TPyNGQOowV72689570 = -331713788;    int TPyNGQOowV23314695 = -695163390;    int TPyNGQOowV2510108 = -381685636;    int TPyNGQOowV34667392 = -324499881;    int TPyNGQOowV12700465 = -472180560;    int TPyNGQOowV36283773 = 17169866;    int TPyNGQOowV70688263 = 84535168;    int TPyNGQOowV41702285 = -66155775;    int TPyNGQOowV25357295 = -423021093;    int TPyNGQOowV16901474 = -205900332;    int TPyNGQOowV55050294 = 266982;    int TPyNGQOowV22395270 = -149817677;    int TPyNGQOowV97411046 = -347169888;    int TPyNGQOowV72038716 = -133898829;    int TPyNGQOowV10506915 = -920927341;    int TPyNGQOowV24268630 = -384486955;    int TPyNGQOowV94506995 = -203566338;    int TPyNGQOowV29651089 = -29886794;    int TPyNGQOowV8005124 = -748953867;    int TPyNGQOowV88450785 = -578915713;    int TPyNGQOowV74435447 = -647396127;    int TPyNGQOowV23785116 = -944593642;    int TPyNGQOowV4154087 = -648433723;    int TPyNGQOowV71975056 = -177073559;    int TPyNGQOowV7691389 = -722224550;    int TPyNGQOowV64140188 = -36214494;    int TPyNGQOowV42755658 = -424254386;    int TPyNGQOowV83996807 = -935412864;    int TPyNGQOowV71084491 = -591342308;    int TPyNGQOowV52470329 = -412538559;    int TPyNGQOowV10348188 = -666478211;    int TPyNGQOowV66852090 = -284515601;    int TPyNGQOowV23696707 = -52440083;    int TPyNGQOowV2355132 = -355434461;    int TPyNGQOowV23065839 = -37651345;    int TPyNGQOowV2207605 = -849746222;    int TPyNGQOowV83593408 = -105018151;    int TPyNGQOowV84571244 = -418327770;    int TPyNGQOowV86169904 = -621136912;    int TPyNGQOowV82191269 = -366459376;    int TPyNGQOowV10868328 = -233533309;    int TPyNGQOowV7366540 = -216267858;    int TPyNGQOowV65814022 = -546701074;    int TPyNGQOowV62282781 = -50884;    int TPyNGQOowV788197 = -84491765;    int TPyNGQOowV59451855 = -936665363;    int TPyNGQOowV22768334 = 30811813;    int TPyNGQOowV5183275 = -132553523;    int TPyNGQOowV69708412 = -665366762;    int TPyNGQOowV88658806 = -640218344;    int TPyNGQOowV2405740 = -430488070;    int TPyNGQOowV42075291 = -465877008;    int TPyNGQOowV10708784 = -228848512;    int TPyNGQOowV30035086 = -808059844;    int TPyNGQOowV39259787 = -114113956;    int TPyNGQOowV48904455 = -387120146;    int TPyNGQOowV19160608 = 53270333;    int TPyNGQOowV30535052 = -104612078;    int TPyNGQOowV26976004 = -602275331;    int TPyNGQOowV48560277 = -335966067;    int TPyNGQOowV93528114 = -558575748;    int TPyNGQOowV86691455 = 19948032;    int TPyNGQOowV70617794 = -474813468;    int TPyNGQOowV72886966 = 89517465;    int TPyNGQOowV6553287 = -539422121;    int TPyNGQOowV88198203 = -715217418;    int TPyNGQOowV98698562 = 2622406;    int TPyNGQOowV95055914 = -991735428;    int TPyNGQOowV48972878 = 3752516;    int TPyNGQOowV8299311 = 28818880;    int TPyNGQOowV40675221 = -179468805;    int TPyNGQOowV9935752 = -785238568;    int TPyNGQOowV43481185 = -408749882;    int TPyNGQOowV25813854 = -282494492;    int TPyNGQOowV77582458 = -262242012;     TPyNGQOowV82074568 = TPyNGQOowV85565997;     TPyNGQOowV85565997 = TPyNGQOowV42182579;     TPyNGQOowV42182579 = TPyNGQOowV16050366;     TPyNGQOowV16050366 = TPyNGQOowV95755408;     TPyNGQOowV95755408 = TPyNGQOowV25522299;     TPyNGQOowV25522299 = TPyNGQOowV86103516;     TPyNGQOowV86103516 = TPyNGQOowV19238637;     TPyNGQOowV19238637 = TPyNGQOowV98870369;     TPyNGQOowV98870369 = TPyNGQOowV18475042;     TPyNGQOowV18475042 = TPyNGQOowV81556591;     TPyNGQOowV81556591 = TPyNGQOowV49068825;     TPyNGQOowV49068825 = TPyNGQOowV91171317;     TPyNGQOowV91171317 = TPyNGQOowV79184255;     TPyNGQOowV79184255 = TPyNGQOowV55838490;     TPyNGQOowV55838490 = TPyNGQOowV81847124;     TPyNGQOowV81847124 = TPyNGQOowV20179380;     TPyNGQOowV20179380 = TPyNGQOowV77221991;     TPyNGQOowV77221991 = TPyNGQOowV80215326;     TPyNGQOowV80215326 = TPyNGQOowV12927437;     TPyNGQOowV12927437 = TPyNGQOowV96912734;     TPyNGQOowV96912734 = TPyNGQOowV71726380;     TPyNGQOowV71726380 = TPyNGQOowV18713907;     TPyNGQOowV18713907 = TPyNGQOowV18485872;     TPyNGQOowV18485872 = TPyNGQOowV13695235;     TPyNGQOowV13695235 = TPyNGQOowV72689570;     TPyNGQOowV72689570 = TPyNGQOowV23314695;     TPyNGQOowV23314695 = TPyNGQOowV2510108;     TPyNGQOowV2510108 = TPyNGQOowV34667392;     TPyNGQOowV34667392 = TPyNGQOowV12700465;     TPyNGQOowV12700465 = TPyNGQOowV36283773;     TPyNGQOowV36283773 = TPyNGQOowV70688263;     TPyNGQOowV70688263 = TPyNGQOowV41702285;     TPyNGQOowV41702285 = TPyNGQOowV25357295;     TPyNGQOowV25357295 = TPyNGQOowV16901474;     TPyNGQOowV16901474 = TPyNGQOowV55050294;     TPyNGQOowV55050294 = TPyNGQOowV22395270;     TPyNGQOowV22395270 = TPyNGQOowV97411046;     TPyNGQOowV97411046 = TPyNGQOowV72038716;     TPyNGQOowV72038716 = TPyNGQOowV10506915;     TPyNGQOowV10506915 = TPyNGQOowV24268630;     TPyNGQOowV24268630 = TPyNGQOowV94506995;     TPyNGQOowV94506995 = TPyNGQOowV29651089;     TPyNGQOowV29651089 = TPyNGQOowV8005124;     TPyNGQOowV8005124 = TPyNGQOowV88450785;     TPyNGQOowV88450785 = TPyNGQOowV74435447;     TPyNGQOowV74435447 = TPyNGQOowV23785116;     TPyNGQOowV23785116 = TPyNGQOowV4154087;     TPyNGQOowV4154087 = TPyNGQOowV71975056;     TPyNGQOowV71975056 = TPyNGQOowV7691389;     TPyNGQOowV7691389 = TPyNGQOowV64140188;     TPyNGQOowV64140188 = TPyNGQOowV42755658;     TPyNGQOowV42755658 = TPyNGQOowV83996807;     TPyNGQOowV83996807 = TPyNGQOowV71084491;     TPyNGQOowV71084491 = TPyNGQOowV52470329;     TPyNGQOowV52470329 = TPyNGQOowV10348188;     TPyNGQOowV10348188 = TPyNGQOowV66852090;     TPyNGQOowV66852090 = TPyNGQOowV23696707;     TPyNGQOowV23696707 = TPyNGQOowV2355132;     TPyNGQOowV2355132 = TPyNGQOowV23065839;     TPyNGQOowV23065839 = TPyNGQOowV2207605;     TPyNGQOowV2207605 = TPyNGQOowV83593408;     TPyNGQOowV83593408 = TPyNGQOowV84571244;     TPyNGQOowV84571244 = TPyNGQOowV86169904;     TPyNGQOowV86169904 = TPyNGQOowV82191269;     TPyNGQOowV82191269 = TPyNGQOowV10868328;     TPyNGQOowV10868328 = TPyNGQOowV7366540;     TPyNGQOowV7366540 = TPyNGQOowV65814022;     TPyNGQOowV65814022 = TPyNGQOowV62282781;     TPyNGQOowV62282781 = TPyNGQOowV788197;     TPyNGQOowV788197 = TPyNGQOowV59451855;     TPyNGQOowV59451855 = TPyNGQOowV22768334;     TPyNGQOowV22768334 = TPyNGQOowV5183275;     TPyNGQOowV5183275 = TPyNGQOowV69708412;     TPyNGQOowV69708412 = TPyNGQOowV88658806;     TPyNGQOowV88658806 = TPyNGQOowV2405740;     TPyNGQOowV2405740 = TPyNGQOowV42075291;     TPyNGQOowV42075291 = TPyNGQOowV10708784;     TPyNGQOowV10708784 = TPyNGQOowV30035086;     TPyNGQOowV30035086 = TPyNGQOowV39259787;     TPyNGQOowV39259787 = TPyNGQOowV48904455;     TPyNGQOowV48904455 = TPyNGQOowV19160608;     TPyNGQOowV19160608 = TPyNGQOowV30535052;     TPyNGQOowV30535052 = TPyNGQOowV26976004;     TPyNGQOowV26976004 = TPyNGQOowV48560277;     TPyNGQOowV48560277 = TPyNGQOowV93528114;     TPyNGQOowV93528114 = TPyNGQOowV86691455;     TPyNGQOowV86691455 = TPyNGQOowV70617794;     TPyNGQOowV70617794 = TPyNGQOowV72886966;     TPyNGQOowV72886966 = TPyNGQOowV6553287;     TPyNGQOowV6553287 = TPyNGQOowV88198203;     TPyNGQOowV88198203 = TPyNGQOowV98698562;     TPyNGQOowV98698562 = TPyNGQOowV95055914;     TPyNGQOowV95055914 = TPyNGQOowV48972878;     TPyNGQOowV48972878 = TPyNGQOowV8299311;     TPyNGQOowV8299311 = TPyNGQOowV40675221;     TPyNGQOowV40675221 = TPyNGQOowV9935752;     TPyNGQOowV9935752 = TPyNGQOowV43481185;     TPyNGQOowV43481185 = TPyNGQOowV25813854;     TPyNGQOowV25813854 = TPyNGQOowV77582458;     TPyNGQOowV77582458 = TPyNGQOowV82074568;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fhRtULCxax67596406() {     int IiQYkYXJij65518873 = -42907692;    int IiQYkYXJij2438086 = -361700166;    int IiQYkYXJij38591085 = -332940867;    int IiQYkYXJij78842617 = -410744379;    int IiQYkYXJij1876733 = -276232624;    int IiQYkYXJij24026700 = -323981264;    int IiQYkYXJij57142834 = -326615100;    int IiQYkYXJij33586079 = -829985680;    int IiQYkYXJij57400070 = -434776823;    int IiQYkYXJij51020694 = -917665621;    int IiQYkYXJij30972529 = -439114013;    int IiQYkYXJij12274224 = -370041879;    int IiQYkYXJij9289094 = -861383408;    int IiQYkYXJij58811147 = -94241572;    int IiQYkYXJij16692954 = -278247765;    int IiQYkYXJij68236286 = -974562364;    int IiQYkYXJij13931265 = -169703101;    int IiQYkYXJij58073438 = -613757227;    int IiQYkYXJij42655469 = -584968973;    int IiQYkYXJij2294879 = -554291291;    int IiQYkYXJij52553122 = -323583438;    int IiQYkYXJij83873698 = -586915572;    int IiQYkYXJij22539615 = -17691575;    int IiQYkYXJij53192430 = -458800952;    int IiQYkYXJij55271193 = -321162841;    int IiQYkYXJij68031719 = -212223504;    int IiQYkYXJij72769435 = -323247732;    int IiQYkYXJij42241733 = -451640520;    int IiQYkYXJij70748486 = -187005023;    int IiQYkYXJij28533806 = -985030412;    int IiQYkYXJij54379086 = -720587335;    int IiQYkYXJij33794690 = -930659911;    int IiQYkYXJij85241171 = -787027952;    int IiQYkYXJij67515806 = -14623263;    int IiQYkYXJij95628204 = -403565067;    int IiQYkYXJij67439478 = -887756924;    int IiQYkYXJij91009448 = -28666856;    int IiQYkYXJij37989878 = -55640408;    int IiQYkYXJij21033795 = -318796626;    int IiQYkYXJij60002049 = -673520863;    int IiQYkYXJij57917554 = 43964072;    int IiQYkYXJij44994782 = -777474659;    int IiQYkYXJij53218159 = -580944036;    int IiQYkYXJij99910063 = -505136638;    int IiQYkYXJij80395282 = -307072666;    int IiQYkYXJij14041444 = -667007051;    int IiQYkYXJij99800545 = 23102128;    int IiQYkYXJij23824682 = -647155441;    int IiQYkYXJij2822169 = -762675641;    int IiQYkYXJij43023872 = -929245005;    int IiQYkYXJij37397636 = -257115916;    int IiQYkYXJij98256452 = -482513697;    int IiQYkYXJij98109266 = -47160181;    int IiQYkYXJij95761249 = -309755695;    int IiQYkYXJij15043284 = -761024682;    int IiQYkYXJij81645175 = -455992121;    int IiQYkYXJij79898471 = -244008591;    int IiQYkYXJij85398655 = -874139915;    int IiQYkYXJij23571425 = 10418462;    int IiQYkYXJij33845013 = 35990880;    int IiQYkYXJij51257265 = 99266468;    int IiQYkYXJij14901102 = -874974580;    int IiQYkYXJij62837593 = -542980657;    int IiQYkYXJij28866264 = -449746412;    int IiQYkYXJij96641607 = -97078287;    int IiQYkYXJij97177838 = -508454102;    int IiQYkYXJij27033052 = -583013927;    int IiQYkYXJij41773288 = -746760145;    int IiQYkYXJij63182943 = -690676505;    int IiQYkYXJij49253475 = -390490841;    int IiQYkYXJij77226838 = -845895508;    int IiQYkYXJij75941387 = -14062694;    int IiQYkYXJij37039643 = -194960602;    int IiQYkYXJij82653419 = -911448110;    int IiQYkYXJij44377325 = -498255363;    int IiQYkYXJij7558341 = -546108779;    int IiQYkYXJij30655540 = 94028464;    int IiQYkYXJij22629551 = -512554938;    int IiQYkYXJij72797147 = -51728286;    int IiQYkYXJij41229749 = -654155791;    int IiQYkYXJij68231174 = -135325633;    int IiQYkYXJij48944753 = -676092291;    int IiQYkYXJij39419565 = -688964879;    int IiQYkYXJij27724614 = -257760018;    int IiQYkYXJij91136170 = -627914496;    int IiQYkYXJij56122634 = -138073638;    int IiQYkYXJij35685424 = -783499731;    int IiQYkYXJij89479922 = -377272258;    int IiQYkYXJij52472522 = -253598582;    int IiQYkYXJij13983030 = -947572946;    int IiQYkYXJij87541006 = -543748333;    int IiQYkYXJij5610793 = -154526942;    int IiQYkYXJij14418454 = 33941129;    int IiQYkYXJij87188781 = -254787507;    int IiQYkYXJij8744785 = -672787332;    int IiQYkYXJij43016453 = -81061349;    int IiQYkYXJij82157188 = -134494002;    int IiQYkYXJij24351895 = -31197625;    int IiQYkYXJij3268456 = -308058351;    int IiQYkYXJij83217443 = -42907692;     IiQYkYXJij65518873 = IiQYkYXJij2438086;     IiQYkYXJij2438086 = IiQYkYXJij38591085;     IiQYkYXJij38591085 = IiQYkYXJij78842617;     IiQYkYXJij78842617 = IiQYkYXJij1876733;     IiQYkYXJij1876733 = IiQYkYXJij24026700;     IiQYkYXJij24026700 = IiQYkYXJij57142834;     IiQYkYXJij57142834 = IiQYkYXJij33586079;     IiQYkYXJij33586079 = IiQYkYXJij57400070;     IiQYkYXJij57400070 = IiQYkYXJij51020694;     IiQYkYXJij51020694 = IiQYkYXJij30972529;     IiQYkYXJij30972529 = IiQYkYXJij12274224;     IiQYkYXJij12274224 = IiQYkYXJij9289094;     IiQYkYXJij9289094 = IiQYkYXJij58811147;     IiQYkYXJij58811147 = IiQYkYXJij16692954;     IiQYkYXJij16692954 = IiQYkYXJij68236286;     IiQYkYXJij68236286 = IiQYkYXJij13931265;     IiQYkYXJij13931265 = IiQYkYXJij58073438;     IiQYkYXJij58073438 = IiQYkYXJij42655469;     IiQYkYXJij42655469 = IiQYkYXJij2294879;     IiQYkYXJij2294879 = IiQYkYXJij52553122;     IiQYkYXJij52553122 = IiQYkYXJij83873698;     IiQYkYXJij83873698 = IiQYkYXJij22539615;     IiQYkYXJij22539615 = IiQYkYXJij53192430;     IiQYkYXJij53192430 = IiQYkYXJij55271193;     IiQYkYXJij55271193 = IiQYkYXJij68031719;     IiQYkYXJij68031719 = IiQYkYXJij72769435;     IiQYkYXJij72769435 = IiQYkYXJij42241733;     IiQYkYXJij42241733 = IiQYkYXJij70748486;     IiQYkYXJij70748486 = IiQYkYXJij28533806;     IiQYkYXJij28533806 = IiQYkYXJij54379086;     IiQYkYXJij54379086 = IiQYkYXJij33794690;     IiQYkYXJij33794690 = IiQYkYXJij85241171;     IiQYkYXJij85241171 = IiQYkYXJij67515806;     IiQYkYXJij67515806 = IiQYkYXJij95628204;     IiQYkYXJij95628204 = IiQYkYXJij67439478;     IiQYkYXJij67439478 = IiQYkYXJij91009448;     IiQYkYXJij91009448 = IiQYkYXJij37989878;     IiQYkYXJij37989878 = IiQYkYXJij21033795;     IiQYkYXJij21033795 = IiQYkYXJij60002049;     IiQYkYXJij60002049 = IiQYkYXJij57917554;     IiQYkYXJij57917554 = IiQYkYXJij44994782;     IiQYkYXJij44994782 = IiQYkYXJij53218159;     IiQYkYXJij53218159 = IiQYkYXJij99910063;     IiQYkYXJij99910063 = IiQYkYXJij80395282;     IiQYkYXJij80395282 = IiQYkYXJij14041444;     IiQYkYXJij14041444 = IiQYkYXJij99800545;     IiQYkYXJij99800545 = IiQYkYXJij23824682;     IiQYkYXJij23824682 = IiQYkYXJij2822169;     IiQYkYXJij2822169 = IiQYkYXJij43023872;     IiQYkYXJij43023872 = IiQYkYXJij37397636;     IiQYkYXJij37397636 = IiQYkYXJij98256452;     IiQYkYXJij98256452 = IiQYkYXJij98109266;     IiQYkYXJij98109266 = IiQYkYXJij95761249;     IiQYkYXJij95761249 = IiQYkYXJij15043284;     IiQYkYXJij15043284 = IiQYkYXJij81645175;     IiQYkYXJij81645175 = IiQYkYXJij79898471;     IiQYkYXJij79898471 = IiQYkYXJij85398655;     IiQYkYXJij85398655 = IiQYkYXJij23571425;     IiQYkYXJij23571425 = IiQYkYXJij33845013;     IiQYkYXJij33845013 = IiQYkYXJij51257265;     IiQYkYXJij51257265 = IiQYkYXJij14901102;     IiQYkYXJij14901102 = IiQYkYXJij62837593;     IiQYkYXJij62837593 = IiQYkYXJij28866264;     IiQYkYXJij28866264 = IiQYkYXJij96641607;     IiQYkYXJij96641607 = IiQYkYXJij97177838;     IiQYkYXJij97177838 = IiQYkYXJij27033052;     IiQYkYXJij27033052 = IiQYkYXJij41773288;     IiQYkYXJij41773288 = IiQYkYXJij63182943;     IiQYkYXJij63182943 = IiQYkYXJij49253475;     IiQYkYXJij49253475 = IiQYkYXJij77226838;     IiQYkYXJij77226838 = IiQYkYXJij75941387;     IiQYkYXJij75941387 = IiQYkYXJij37039643;     IiQYkYXJij37039643 = IiQYkYXJij82653419;     IiQYkYXJij82653419 = IiQYkYXJij44377325;     IiQYkYXJij44377325 = IiQYkYXJij7558341;     IiQYkYXJij7558341 = IiQYkYXJij30655540;     IiQYkYXJij30655540 = IiQYkYXJij22629551;     IiQYkYXJij22629551 = IiQYkYXJij72797147;     IiQYkYXJij72797147 = IiQYkYXJij41229749;     IiQYkYXJij41229749 = IiQYkYXJij68231174;     IiQYkYXJij68231174 = IiQYkYXJij48944753;     IiQYkYXJij48944753 = IiQYkYXJij39419565;     IiQYkYXJij39419565 = IiQYkYXJij27724614;     IiQYkYXJij27724614 = IiQYkYXJij91136170;     IiQYkYXJij91136170 = IiQYkYXJij56122634;     IiQYkYXJij56122634 = IiQYkYXJij35685424;     IiQYkYXJij35685424 = IiQYkYXJij89479922;     IiQYkYXJij89479922 = IiQYkYXJij52472522;     IiQYkYXJij52472522 = IiQYkYXJij13983030;     IiQYkYXJij13983030 = IiQYkYXJij87541006;     IiQYkYXJij87541006 = IiQYkYXJij5610793;     IiQYkYXJij5610793 = IiQYkYXJij14418454;     IiQYkYXJij14418454 = IiQYkYXJij87188781;     IiQYkYXJij87188781 = IiQYkYXJij8744785;     IiQYkYXJij8744785 = IiQYkYXJij43016453;     IiQYkYXJij43016453 = IiQYkYXJij82157188;     IiQYkYXJij82157188 = IiQYkYXJij24351895;     IiQYkYXJij24351895 = IiQYkYXJij3268456;     IiQYkYXJij3268456 = IiQYkYXJij83217443;     IiQYkYXJij83217443 = IiQYkYXJij65518873;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hatwQyosZD68038509() {     int ByfmodJbeF698630 = 60171794;    int ByfmodJbeF34984158 = -437982109;    int ByfmodJbeF83799951 = 76174521;    int ByfmodJbeF16710271 = -420813357;    int ByfmodJbeF84721989 = -890750768;    int ByfmodJbeF47414455 = -262080143;    int ByfmodJbeF1958996 = -989738009;    int ByfmodJbeF66725000 = -940923264;    int ByfmodJbeF67227847 = -567084885;    int ByfmodJbeF52363470 = -827687716;    int ByfmodJbeF75681165 = -590952357;    int ByfmodJbeF85836000 = -292641626;    int ByfmodJbeF14587663 = -262224910;    int ByfmodJbeF97205757 = -609212475;    int ByfmodJbeF31562674 = -949998457;    int ByfmodJbeF66352720 = -474325645;    int ByfmodJbeF63653675 = -291142818;    int ByfmodJbeF24607807 = -223638598;    int ByfmodJbeF64669460 = -11558027;    int ByfmodJbeF11391896 = -794145432;    int ByfmodJbeF53940862 = 4079363;    int ByfmodJbeF98440470 = -751548350;    int ByfmodJbeF27714837 = -765818220;    int ByfmodJbeF80549238 = -402218299;    int ByfmodJbeF68263009 = -826470037;    int ByfmodJbeF21079282 = -438342999;    int ByfmodJbeF61052810 = -413740792;    int ByfmodJbeF80102583 = -904926090;    int ByfmodJbeF81579653 = -99409067;    int ByfmodJbeF14352231 = -859781059;    int ByfmodJbeF9693209 = -691227002;    int ByfmodJbeF9930848 = -69629183;    int ByfmodJbeF26873368 = -990282016;    int ByfmodJbeF3698029 = -366822599;    int ByfmodJbeF72560063 = -818039445;    int ByfmodJbeF61528272 = -911949659;    int ByfmodJbeF9271854 = -551870780;    int ByfmodJbeF79402322 = -947623292;    int ByfmodJbeF39803715 = -949011819;    int ByfmodJbeF34579601 = 55269948;    int ByfmodJbeF60250566 = -746743689;    int ByfmodJbeF55507968 = -239902602;    int ByfmodJbeF33368461 = 67222189;    int ByfmodJbeF84750822 = -186708086;    int ByfmodJbeF44368987 = -635726703;    int ByfmodJbeF55341886 = -10509915;    int ByfmodJbeF61982170 = -571542555;    int ByfmodJbeF22479742 = 49718144;    int ByfmodJbeF22687734 = -212230453;    int ByfmodJbeF67012615 = -995276841;    int ByfmodJbeF51334976 = -186521495;    int ByfmodJbeF85736999 = 97627431;    int ByfmodJbeF35460472 = -3020685;    int ByfmodJbeF90497249 = -742968395;    int ByfmodJbeF14097647 = -39274005;    int ByfmodJbeF2258160 = -188279856;    int ByfmodJbeF7269322 = -672163890;    int ByfmodJbeF3250714 = -521607180;    int ByfmodJbeF48447261 = -594343320;    int ByfmodJbeF63642707 = -352407770;    int ByfmodJbeF86361645 = -848339352;    int ByfmodJbeF21856412 = 15188080;    int ByfmodJbeF85145347 = -741514197;    int ByfmodJbeF52875616 = -707303826;    int ByfmodJbeF42670261 = -36460714;    int ByfmodJbeF65750318 = -421323174;    int ByfmodJbeF58962632 = -302359610;    int ByfmodJbeF10889635 = -895402312;    int ByfmodJbeF24645695 = -791173031;    int ByfmodJbeF70034402 = 61951201;    int ByfmodJbeF57080867 = -922454866;    int ByfmodJbeF84251352 = -343519526;    int ByfmodJbeF84804091 = -274626780;    int ByfmodJbeF30089859 = 33172025;    int ByfmodJbeF51141330 = 52598256;    int ByfmodJbeF98432893 = -756018035;    int ByfmodJbeF65072009 = -718770539;    int ByfmodJbeF42964015 = -479110134;    int ByfmodJbeF36180251 = -766491597;    int ByfmodJbeF12921123 = -715960122;    int ByfmodJbeF59097112 = -866800444;    int ByfmodJbeF38573068 = -363458937;    int ByfmodJbeF57414850 = -592695638;    int ByfmodJbeF14567038 = -104132227;    int ByfmodJbeF63017255 = -573259565;    int ByfmodJbeF23956210 = -688854434;    int ByfmodJbeF74470376 = 33391502;    int ByfmodJbeF36376119 = -147313622;    int ByfmodJbeF89600381 = -227548594;    int ByfmodJbeF70301904 = -529759589;    int ByfmodJbeF54258951 = -139785769;    int ByfmodJbeF6021140 = 69736399;    int ByfmodJbeF30955061 = -253279973;    int ByfmodJbeF76161008 = -496604049;    int ByfmodJbeF48217956 = -96390701;    int ByfmodJbeF38394154 = -661931770;    int ByfmodJbeF70362621 = -498388406;    int ByfmodJbeF80492844 = -225473986;    int ByfmodJbeF42080561 = -50247372;    int ByfmodJbeF78618669 = 60171794;     ByfmodJbeF698630 = ByfmodJbeF34984158;     ByfmodJbeF34984158 = ByfmodJbeF83799951;     ByfmodJbeF83799951 = ByfmodJbeF16710271;     ByfmodJbeF16710271 = ByfmodJbeF84721989;     ByfmodJbeF84721989 = ByfmodJbeF47414455;     ByfmodJbeF47414455 = ByfmodJbeF1958996;     ByfmodJbeF1958996 = ByfmodJbeF66725000;     ByfmodJbeF66725000 = ByfmodJbeF67227847;     ByfmodJbeF67227847 = ByfmodJbeF52363470;     ByfmodJbeF52363470 = ByfmodJbeF75681165;     ByfmodJbeF75681165 = ByfmodJbeF85836000;     ByfmodJbeF85836000 = ByfmodJbeF14587663;     ByfmodJbeF14587663 = ByfmodJbeF97205757;     ByfmodJbeF97205757 = ByfmodJbeF31562674;     ByfmodJbeF31562674 = ByfmodJbeF66352720;     ByfmodJbeF66352720 = ByfmodJbeF63653675;     ByfmodJbeF63653675 = ByfmodJbeF24607807;     ByfmodJbeF24607807 = ByfmodJbeF64669460;     ByfmodJbeF64669460 = ByfmodJbeF11391896;     ByfmodJbeF11391896 = ByfmodJbeF53940862;     ByfmodJbeF53940862 = ByfmodJbeF98440470;     ByfmodJbeF98440470 = ByfmodJbeF27714837;     ByfmodJbeF27714837 = ByfmodJbeF80549238;     ByfmodJbeF80549238 = ByfmodJbeF68263009;     ByfmodJbeF68263009 = ByfmodJbeF21079282;     ByfmodJbeF21079282 = ByfmodJbeF61052810;     ByfmodJbeF61052810 = ByfmodJbeF80102583;     ByfmodJbeF80102583 = ByfmodJbeF81579653;     ByfmodJbeF81579653 = ByfmodJbeF14352231;     ByfmodJbeF14352231 = ByfmodJbeF9693209;     ByfmodJbeF9693209 = ByfmodJbeF9930848;     ByfmodJbeF9930848 = ByfmodJbeF26873368;     ByfmodJbeF26873368 = ByfmodJbeF3698029;     ByfmodJbeF3698029 = ByfmodJbeF72560063;     ByfmodJbeF72560063 = ByfmodJbeF61528272;     ByfmodJbeF61528272 = ByfmodJbeF9271854;     ByfmodJbeF9271854 = ByfmodJbeF79402322;     ByfmodJbeF79402322 = ByfmodJbeF39803715;     ByfmodJbeF39803715 = ByfmodJbeF34579601;     ByfmodJbeF34579601 = ByfmodJbeF60250566;     ByfmodJbeF60250566 = ByfmodJbeF55507968;     ByfmodJbeF55507968 = ByfmodJbeF33368461;     ByfmodJbeF33368461 = ByfmodJbeF84750822;     ByfmodJbeF84750822 = ByfmodJbeF44368987;     ByfmodJbeF44368987 = ByfmodJbeF55341886;     ByfmodJbeF55341886 = ByfmodJbeF61982170;     ByfmodJbeF61982170 = ByfmodJbeF22479742;     ByfmodJbeF22479742 = ByfmodJbeF22687734;     ByfmodJbeF22687734 = ByfmodJbeF67012615;     ByfmodJbeF67012615 = ByfmodJbeF51334976;     ByfmodJbeF51334976 = ByfmodJbeF85736999;     ByfmodJbeF85736999 = ByfmodJbeF35460472;     ByfmodJbeF35460472 = ByfmodJbeF90497249;     ByfmodJbeF90497249 = ByfmodJbeF14097647;     ByfmodJbeF14097647 = ByfmodJbeF2258160;     ByfmodJbeF2258160 = ByfmodJbeF7269322;     ByfmodJbeF7269322 = ByfmodJbeF3250714;     ByfmodJbeF3250714 = ByfmodJbeF48447261;     ByfmodJbeF48447261 = ByfmodJbeF63642707;     ByfmodJbeF63642707 = ByfmodJbeF86361645;     ByfmodJbeF86361645 = ByfmodJbeF21856412;     ByfmodJbeF21856412 = ByfmodJbeF85145347;     ByfmodJbeF85145347 = ByfmodJbeF52875616;     ByfmodJbeF52875616 = ByfmodJbeF42670261;     ByfmodJbeF42670261 = ByfmodJbeF65750318;     ByfmodJbeF65750318 = ByfmodJbeF58962632;     ByfmodJbeF58962632 = ByfmodJbeF10889635;     ByfmodJbeF10889635 = ByfmodJbeF24645695;     ByfmodJbeF24645695 = ByfmodJbeF70034402;     ByfmodJbeF70034402 = ByfmodJbeF57080867;     ByfmodJbeF57080867 = ByfmodJbeF84251352;     ByfmodJbeF84251352 = ByfmodJbeF84804091;     ByfmodJbeF84804091 = ByfmodJbeF30089859;     ByfmodJbeF30089859 = ByfmodJbeF51141330;     ByfmodJbeF51141330 = ByfmodJbeF98432893;     ByfmodJbeF98432893 = ByfmodJbeF65072009;     ByfmodJbeF65072009 = ByfmodJbeF42964015;     ByfmodJbeF42964015 = ByfmodJbeF36180251;     ByfmodJbeF36180251 = ByfmodJbeF12921123;     ByfmodJbeF12921123 = ByfmodJbeF59097112;     ByfmodJbeF59097112 = ByfmodJbeF38573068;     ByfmodJbeF38573068 = ByfmodJbeF57414850;     ByfmodJbeF57414850 = ByfmodJbeF14567038;     ByfmodJbeF14567038 = ByfmodJbeF63017255;     ByfmodJbeF63017255 = ByfmodJbeF23956210;     ByfmodJbeF23956210 = ByfmodJbeF74470376;     ByfmodJbeF74470376 = ByfmodJbeF36376119;     ByfmodJbeF36376119 = ByfmodJbeF89600381;     ByfmodJbeF89600381 = ByfmodJbeF70301904;     ByfmodJbeF70301904 = ByfmodJbeF54258951;     ByfmodJbeF54258951 = ByfmodJbeF6021140;     ByfmodJbeF6021140 = ByfmodJbeF30955061;     ByfmodJbeF30955061 = ByfmodJbeF76161008;     ByfmodJbeF76161008 = ByfmodJbeF48217956;     ByfmodJbeF48217956 = ByfmodJbeF38394154;     ByfmodJbeF38394154 = ByfmodJbeF70362621;     ByfmodJbeF70362621 = ByfmodJbeF80492844;     ByfmodJbeF80492844 = ByfmodJbeF42080561;     ByfmodJbeF42080561 = ByfmodJbeF78618669;     ByfmodJbeF78618669 = ByfmodJbeF698630;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hGBYcddBVv38494382() {     int PtcIMFXEJp13484797 = -74868898;    int PtcIMFXEJp21755046 = -608316123;    int PtcIMFXEJp1552752 = -332184860;    int PtcIMFXEJp24651096 = -245015634;    int PtcIMFXEJp78233093 = -282567234;    int PtcIMFXEJp90724525 = -39003457;    int PtcIMFXEJp23166754 = -435410080;    int PtcIMFXEJp93444112 = -285441520;    int PtcIMFXEJp62632658 = -528842977;    int PtcIMFXEJp58582174 = -631775349;    int PtcIMFXEJp3168548 = -705638372;    int PtcIMFXEJp17786506 = -799974441;    int PtcIMFXEJp84730966 = -166007567;    int PtcIMFXEJp46829836 = -941627674;    int PtcIMFXEJp54869227 = -856984287;    int PtcIMFXEJp94777789 = 74782965;    int PtcIMFXEJp19405493 = 45082715;    int PtcIMFXEJp8166292 = -878848901;    int PtcIMFXEJp65830090 = 78972778;    int PtcIMFXEJp14046392 = -293389571;    int PtcIMFXEJp34114919 = -487236901;    int PtcIMFXEJp27488290 = -921406698;    int PtcIMFXEJp66475132 = -696822115;    int PtcIMFXEJp25056368 = -762563312;    int PtcIMFXEJp74157268 = 63088039;    int PtcIMFXEJp2397928 = -128774198;    int PtcIMFXEJp72468346 = 52552928;    int PtcIMFXEJp47455509 = 21275805;    int PtcIMFXEJp98196706 = -313626307;    int PtcIMFXEJp67103346 = -489667214;    int PtcIMFXEJp54129205 = -967643752;    int PtcIMFXEJp13249671 = -48348861;    int PtcIMFXEJp72919317 = -866136800;    int PtcIMFXEJp80774401 = -271576817;    int PtcIMFXEJp55620595 = -328071325;    int PtcIMFXEJp56752387 = -700512763;    int PtcIMFXEJp46035914 = -341780131;    int PtcIMFXEJp37106345 = -867243384;    int PtcIMFXEJp45579101 = -790862396;    int PtcIMFXEJp60844331 = -889146898;    int PtcIMFXEJp21681874 = -22217682;    int PtcIMFXEJp63210424 = -138213936;    int PtcIMFXEJp68251930 = 43788744;    int PtcIMFXEJp29379360 = -527880329;    int PtcIMFXEJp24377478 = -528496663;    int PtcIMFXEJp28970016 = -83828355;    int PtcIMFXEJp39564190 = 30942393;    int PtcIMFXEJp51919709 = -105634635;    int PtcIMFXEJp66751783 = -375832427;    int PtcIMFXEJp13963036 = -506313304;    int PtcIMFXEJp69662469 = -439405803;    int PtcIMFXEJp13913547 = -373557907;    int PtcIMFXEJp69945621 = -179023968;    int PtcIMFXEJp35196975 = -332420137;    int PtcIMFXEJp4379463 = -33718492;    int PtcIMFXEJp85996507 = -153462200;    int PtcIMFXEJp55279914 = -911494009;    int PtcIMFXEJp76496384 = -569621549;    int PtcIMFXEJp50493828 = -208103673;    int PtcIMFXEJp75835166 = -53793036;    int PtcIMFXEJp18256180 = 8443615;    int PtcIMFXEJp75711245 = -356685886;    int PtcIMFXEJp95247406 = -971815213;    int PtcIMFXEJp95529311 = 60824237;    int PtcIMFXEJp4452970 = -664131597;    int PtcIMFXEJp89918876 = -557289511;    int PtcIMFXEJp44867188 = -933837642;    int PtcIMFXEJp3956565 = -894430751;    int PtcIMFXEJp91209241 = -513556349;    int PtcIMFXEJp98116840 = -56471525;    int PtcIMFXEJp48741875 = -583436904;    int PtcIMFXEJp82299147 = -87673902;    int PtcIMFXEJp62587190 = 12013495;    int PtcIMFXEJp4985760 = -31880325;    int PtcIMFXEJp92364518 = -171171890;    int PtcIMFXEJp70904494 = -249022965;    int PtcIMFXEJp59236359 = -865195443;    int PtcIMFXEJp37095772 = -68941786;    int PtcIMFXEJp678891 = -134066649;    int PtcIMFXEJp45187253 = -853083607;    int PtcIMFXEJp62833738 = -59716591;    int PtcIMFXEJp20548638 = -841812438;    int PtcIMFXEJp80703725 = -602891768;    int PtcIMFXEJp84233670 = -807313004;    int PtcIMFXEJp97440876 = 49738588;    int PtcIMFXEJp40215658 = -494085846;    int PtcIMFXEJp43304050 = -869324894;    int PtcIMFXEJp37722343 = -433716664;    int PtcIMFXEJp76394938 = -137858326;    int PtcIMFXEJp69624088 = -74609126;    int PtcIMFXEJp1472474 = -789018755;    int PtcIMFXEJp69539529 = -772158582;    int PtcIMFXEJp86612517 = -559139712;    int PtcIMFXEJp69743935 = -637069360;    int PtcIMFXEJp42588151 = -797590514;    int PtcIMFXEJp45970628 = -665531797;    int PtcIMFXEJp67963017 = -166398723;    int PtcIMFXEJp72722619 = 82964507;    int PtcIMFXEJp24926391 = -863748733;    int PtcIMFXEJp34458601 = -74868898;     PtcIMFXEJp13484797 = PtcIMFXEJp21755046;     PtcIMFXEJp21755046 = PtcIMFXEJp1552752;     PtcIMFXEJp1552752 = PtcIMFXEJp24651096;     PtcIMFXEJp24651096 = PtcIMFXEJp78233093;     PtcIMFXEJp78233093 = PtcIMFXEJp90724525;     PtcIMFXEJp90724525 = PtcIMFXEJp23166754;     PtcIMFXEJp23166754 = PtcIMFXEJp93444112;     PtcIMFXEJp93444112 = PtcIMFXEJp62632658;     PtcIMFXEJp62632658 = PtcIMFXEJp58582174;     PtcIMFXEJp58582174 = PtcIMFXEJp3168548;     PtcIMFXEJp3168548 = PtcIMFXEJp17786506;     PtcIMFXEJp17786506 = PtcIMFXEJp84730966;     PtcIMFXEJp84730966 = PtcIMFXEJp46829836;     PtcIMFXEJp46829836 = PtcIMFXEJp54869227;     PtcIMFXEJp54869227 = PtcIMFXEJp94777789;     PtcIMFXEJp94777789 = PtcIMFXEJp19405493;     PtcIMFXEJp19405493 = PtcIMFXEJp8166292;     PtcIMFXEJp8166292 = PtcIMFXEJp65830090;     PtcIMFXEJp65830090 = PtcIMFXEJp14046392;     PtcIMFXEJp14046392 = PtcIMFXEJp34114919;     PtcIMFXEJp34114919 = PtcIMFXEJp27488290;     PtcIMFXEJp27488290 = PtcIMFXEJp66475132;     PtcIMFXEJp66475132 = PtcIMFXEJp25056368;     PtcIMFXEJp25056368 = PtcIMFXEJp74157268;     PtcIMFXEJp74157268 = PtcIMFXEJp2397928;     PtcIMFXEJp2397928 = PtcIMFXEJp72468346;     PtcIMFXEJp72468346 = PtcIMFXEJp47455509;     PtcIMFXEJp47455509 = PtcIMFXEJp98196706;     PtcIMFXEJp98196706 = PtcIMFXEJp67103346;     PtcIMFXEJp67103346 = PtcIMFXEJp54129205;     PtcIMFXEJp54129205 = PtcIMFXEJp13249671;     PtcIMFXEJp13249671 = PtcIMFXEJp72919317;     PtcIMFXEJp72919317 = PtcIMFXEJp80774401;     PtcIMFXEJp80774401 = PtcIMFXEJp55620595;     PtcIMFXEJp55620595 = PtcIMFXEJp56752387;     PtcIMFXEJp56752387 = PtcIMFXEJp46035914;     PtcIMFXEJp46035914 = PtcIMFXEJp37106345;     PtcIMFXEJp37106345 = PtcIMFXEJp45579101;     PtcIMFXEJp45579101 = PtcIMFXEJp60844331;     PtcIMFXEJp60844331 = PtcIMFXEJp21681874;     PtcIMFXEJp21681874 = PtcIMFXEJp63210424;     PtcIMFXEJp63210424 = PtcIMFXEJp68251930;     PtcIMFXEJp68251930 = PtcIMFXEJp29379360;     PtcIMFXEJp29379360 = PtcIMFXEJp24377478;     PtcIMFXEJp24377478 = PtcIMFXEJp28970016;     PtcIMFXEJp28970016 = PtcIMFXEJp39564190;     PtcIMFXEJp39564190 = PtcIMFXEJp51919709;     PtcIMFXEJp51919709 = PtcIMFXEJp66751783;     PtcIMFXEJp66751783 = PtcIMFXEJp13963036;     PtcIMFXEJp13963036 = PtcIMFXEJp69662469;     PtcIMFXEJp69662469 = PtcIMFXEJp13913547;     PtcIMFXEJp13913547 = PtcIMFXEJp69945621;     PtcIMFXEJp69945621 = PtcIMFXEJp35196975;     PtcIMFXEJp35196975 = PtcIMFXEJp4379463;     PtcIMFXEJp4379463 = PtcIMFXEJp85996507;     PtcIMFXEJp85996507 = PtcIMFXEJp55279914;     PtcIMFXEJp55279914 = PtcIMFXEJp76496384;     PtcIMFXEJp76496384 = PtcIMFXEJp50493828;     PtcIMFXEJp50493828 = PtcIMFXEJp75835166;     PtcIMFXEJp75835166 = PtcIMFXEJp18256180;     PtcIMFXEJp18256180 = PtcIMFXEJp75711245;     PtcIMFXEJp75711245 = PtcIMFXEJp95247406;     PtcIMFXEJp95247406 = PtcIMFXEJp95529311;     PtcIMFXEJp95529311 = PtcIMFXEJp4452970;     PtcIMFXEJp4452970 = PtcIMFXEJp89918876;     PtcIMFXEJp89918876 = PtcIMFXEJp44867188;     PtcIMFXEJp44867188 = PtcIMFXEJp3956565;     PtcIMFXEJp3956565 = PtcIMFXEJp91209241;     PtcIMFXEJp91209241 = PtcIMFXEJp98116840;     PtcIMFXEJp98116840 = PtcIMFXEJp48741875;     PtcIMFXEJp48741875 = PtcIMFXEJp82299147;     PtcIMFXEJp82299147 = PtcIMFXEJp62587190;     PtcIMFXEJp62587190 = PtcIMFXEJp4985760;     PtcIMFXEJp4985760 = PtcIMFXEJp92364518;     PtcIMFXEJp92364518 = PtcIMFXEJp70904494;     PtcIMFXEJp70904494 = PtcIMFXEJp59236359;     PtcIMFXEJp59236359 = PtcIMFXEJp37095772;     PtcIMFXEJp37095772 = PtcIMFXEJp678891;     PtcIMFXEJp678891 = PtcIMFXEJp45187253;     PtcIMFXEJp45187253 = PtcIMFXEJp62833738;     PtcIMFXEJp62833738 = PtcIMFXEJp20548638;     PtcIMFXEJp20548638 = PtcIMFXEJp80703725;     PtcIMFXEJp80703725 = PtcIMFXEJp84233670;     PtcIMFXEJp84233670 = PtcIMFXEJp97440876;     PtcIMFXEJp97440876 = PtcIMFXEJp40215658;     PtcIMFXEJp40215658 = PtcIMFXEJp43304050;     PtcIMFXEJp43304050 = PtcIMFXEJp37722343;     PtcIMFXEJp37722343 = PtcIMFXEJp76394938;     PtcIMFXEJp76394938 = PtcIMFXEJp69624088;     PtcIMFXEJp69624088 = PtcIMFXEJp1472474;     PtcIMFXEJp1472474 = PtcIMFXEJp69539529;     PtcIMFXEJp69539529 = PtcIMFXEJp86612517;     PtcIMFXEJp86612517 = PtcIMFXEJp69743935;     PtcIMFXEJp69743935 = PtcIMFXEJp42588151;     PtcIMFXEJp42588151 = PtcIMFXEJp45970628;     PtcIMFXEJp45970628 = PtcIMFXEJp67963017;     PtcIMFXEJp67963017 = PtcIMFXEJp72722619;     PtcIMFXEJp72722619 = PtcIMFXEJp24926391;     PtcIMFXEJp24926391 = PtcIMFXEJp34458601;     PtcIMFXEJp34458601 = PtcIMFXEJp13484797;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tYoDYhwHUd56707723() {     int ORszxFsOut96929102 = -955534578;    int ORszxFsOut38627135 = -607698311;    int ORszxFsOut97961258 = -125710088;    int ORszxFsOut87443347 = -438815470;    int ORszxFsOut84354417 = -89434724;    int ORszxFsOut89228926 = -918075110;    int ORszxFsOut94206071 = -175321393;    int ORszxFsOut7791556 = -272599550;    int ORszxFsOut21162359 = -870302328;    int ORszxFsOut91127826 = -151461;    int ORszxFsOut52584485 = -895754245;    int ORszxFsOut80991904 = -787592688;    int ORszxFsOut2848742 = 42331192;    int ORszxFsOut26456728 = -729918031;    int ORszxFsOut15723691 = -951007270;    int ORszxFsOut81166951 = -813296360;    int ORszxFsOut13157378 = -808262312;    int ORszxFsOut89017738 = -26153777;    int ORszxFsOut28270233 = 80297907;    int ORszxFsOut3413835 = -822975564;    int ORszxFsOut89755305 = -76765932;    int ORszxFsOut39635608 = -912558468;    int ORszxFsOut70300840 = -736711312;    int ORszxFsOut59762926 = -834388708;    int ORszxFsOut15733227 = -496564720;    int ORszxFsOut97740076 = -9283913;    int ORszxFsOut21923086 = -675531415;    int ORszxFsOut87187134 = -48679079;    int ORszxFsOut34277800 = -176131450;    int ORszxFsOut82936687 = 97482934;    int ORszxFsOut72224518 = -605400953;    int ORszxFsOut76356098 = 36456059;    int ORszxFsOut16458204 = -487008978;    int ORszxFsOut22932912 = -963178987;    int ORszxFsOut34347325 = -525736061;    int ORszxFsOut69141571 = -488536669;    int ORszxFsOut14650093 = -220629311;    int ORszxFsOut77685177 = -575713904;    int ORszxFsOut94574179 = -975760194;    int ORszxFsOut10339466 = -641740420;    int ORszxFsOut55330798 = -693766656;    int ORszxFsOut13698210 = -712122258;    int ORszxFsOut91819000 = -507268499;    int ORszxFsOut21284300 = -284063100;    int ORszxFsOut16321975 = -256653616;    int ORszxFsOut68576012 = -103439279;    int ORszxFsOut15579620 = -101361837;    int ORszxFsOut71590304 = -104356354;    int ORszxFsOut97598895 = -961434510;    int ORszxFsOut49295520 = -713333759;    int ORszxFsOut42919918 = -660307225;    int ORszxFsOut69414341 = -431817219;    int ORszxFsOut84058080 = -390771284;    int ORszxFsOut59873734 = -50833524;    int ORszxFsOut66952417 = -382204615;    int ORszxFsOut57293494 = 57023889;    int ORszxFsOut68326295 = -870986999;    int ORszxFsOut38198333 = -291321381;    int ORszxFsOut71710121 = -942250750;    int ORszxFsOut86614341 = 19849188;    int ORszxFsOut67305840 = -142543695;    int ORszxFsOut7018938 = -26642315;    int ORszxFsOut73513755 = 3531899;    int ORszxFsOut38225671 = -867785263;    int ORszxFsOut18903308 = -394750508;    int ORszxFsOut76228387 = -832210304;    int ORszxFsOut64533700 = -200583711;    int ORszxFsOut79915830 = 5510178;    int ORszxFsOut92109403 = -104181971;    int ORszxFsOut46582120 = -362470601;    int ORszxFsOut66516858 = -492667050;    int ORszxFsOut35472200 = -132548409;    int ORszxFsOut94443558 = -50393584;    int ORszxFsOut17930767 = -277961673;    int ORszxFsOut48083036 = -29208909;    int ORszxFsOut76057095 = -364643674;    int ORszxFsOut47816608 = -305289970;    int ORszxFsOut49016540 = -352648212;    int ORszxFsOut43440952 = -477735092;    int ORszxFsOut47157215 = -293125442;    int ORszxFsOut82160456 = -907922077;    int ORszxFsOut50332782 = -471175062;    int ORszxFsOut89588238 = -87244569;    int ORszxFsOut84982280 = -462797691;    int ORszxFsOut40016770 = -242209841;    int ORszxFsOut2810178 = -73583735;    int ORszxFsOut92298017 = -572772657;    int ORszxFsOut56584470 = -336175454;    int ORszxFsOut55980494 = -480974373;    int ORszxFsOut77053831 = -482759950;    int ORszxFsOut815276 = -617549670;    int ORszxFsOut76451759 = -929307930;    int ORszxFsOut5975057 = -633463155;    int ORszxFsOut7959839 = -895609383;    int ORszxFsOut43033626 = -399196725;    int ORszxFsOut48311860 = -567124341;    int ORszxFsOut40184455 = -615654158;    int ORszxFsOut53593329 = -639483236;    int ORszxFsOut2380993 = -889312592;    int ORszxFsOut40093587 = -955534578;     ORszxFsOut96929102 = ORszxFsOut38627135;     ORszxFsOut38627135 = ORszxFsOut97961258;     ORszxFsOut97961258 = ORszxFsOut87443347;     ORszxFsOut87443347 = ORszxFsOut84354417;     ORszxFsOut84354417 = ORszxFsOut89228926;     ORszxFsOut89228926 = ORszxFsOut94206071;     ORszxFsOut94206071 = ORszxFsOut7791556;     ORszxFsOut7791556 = ORszxFsOut21162359;     ORszxFsOut21162359 = ORszxFsOut91127826;     ORszxFsOut91127826 = ORszxFsOut52584485;     ORszxFsOut52584485 = ORszxFsOut80991904;     ORszxFsOut80991904 = ORszxFsOut2848742;     ORszxFsOut2848742 = ORszxFsOut26456728;     ORszxFsOut26456728 = ORszxFsOut15723691;     ORszxFsOut15723691 = ORszxFsOut81166951;     ORszxFsOut81166951 = ORszxFsOut13157378;     ORszxFsOut13157378 = ORszxFsOut89017738;     ORszxFsOut89017738 = ORszxFsOut28270233;     ORszxFsOut28270233 = ORszxFsOut3413835;     ORszxFsOut3413835 = ORszxFsOut89755305;     ORszxFsOut89755305 = ORszxFsOut39635608;     ORszxFsOut39635608 = ORszxFsOut70300840;     ORszxFsOut70300840 = ORszxFsOut59762926;     ORszxFsOut59762926 = ORszxFsOut15733227;     ORszxFsOut15733227 = ORszxFsOut97740076;     ORszxFsOut97740076 = ORszxFsOut21923086;     ORszxFsOut21923086 = ORszxFsOut87187134;     ORszxFsOut87187134 = ORszxFsOut34277800;     ORszxFsOut34277800 = ORszxFsOut82936687;     ORszxFsOut82936687 = ORszxFsOut72224518;     ORszxFsOut72224518 = ORszxFsOut76356098;     ORszxFsOut76356098 = ORszxFsOut16458204;     ORszxFsOut16458204 = ORszxFsOut22932912;     ORszxFsOut22932912 = ORszxFsOut34347325;     ORszxFsOut34347325 = ORszxFsOut69141571;     ORszxFsOut69141571 = ORszxFsOut14650093;     ORszxFsOut14650093 = ORszxFsOut77685177;     ORszxFsOut77685177 = ORszxFsOut94574179;     ORszxFsOut94574179 = ORszxFsOut10339466;     ORszxFsOut10339466 = ORszxFsOut55330798;     ORszxFsOut55330798 = ORszxFsOut13698210;     ORszxFsOut13698210 = ORszxFsOut91819000;     ORszxFsOut91819000 = ORszxFsOut21284300;     ORszxFsOut21284300 = ORszxFsOut16321975;     ORszxFsOut16321975 = ORszxFsOut68576012;     ORszxFsOut68576012 = ORszxFsOut15579620;     ORszxFsOut15579620 = ORszxFsOut71590304;     ORszxFsOut71590304 = ORszxFsOut97598895;     ORszxFsOut97598895 = ORszxFsOut49295520;     ORszxFsOut49295520 = ORszxFsOut42919918;     ORszxFsOut42919918 = ORszxFsOut69414341;     ORszxFsOut69414341 = ORszxFsOut84058080;     ORszxFsOut84058080 = ORszxFsOut59873734;     ORszxFsOut59873734 = ORszxFsOut66952417;     ORszxFsOut66952417 = ORszxFsOut57293494;     ORszxFsOut57293494 = ORszxFsOut68326295;     ORszxFsOut68326295 = ORszxFsOut38198333;     ORszxFsOut38198333 = ORszxFsOut71710121;     ORszxFsOut71710121 = ORszxFsOut86614341;     ORszxFsOut86614341 = ORszxFsOut67305840;     ORszxFsOut67305840 = ORszxFsOut7018938;     ORszxFsOut7018938 = ORszxFsOut73513755;     ORszxFsOut73513755 = ORszxFsOut38225671;     ORszxFsOut38225671 = ORszxFsOut18903308;     ORszxFsOut18903308 = ORszxFsOut76228387;     ORszxFsOut76228387 = ORszxFsOut64533700;     ORszxFsOut64533700 = ORszxFsOut79915830;     ORszxFsOut79915830 = ORszxFsOut92109403;     ORszxFsOut92109403 = ORszxFsOut46582120;     ORszxFsOut46582120 = ORszxFsOut66516858;     ORszxFsOut66516858 = ORszxFsOut35472200;     ORszxFsOut35472200 = ORszxFsOut94443558;     ORszxFsOut94443558 = ORszxFsOut17930767;     ORszxFsOut17930767 = ORszxFsOut48083036;     ORszxFsOut48083036 = ORszxFsOut76057095;     ORszxFsOut76057095 = ORszxFsOut47816608;     ORszxFsOut47816608 = ORszxFsOut49016540;     ORszxFsOut49016540 = ORszxFsOut43440952;     ORszxFsOut43440952 = ORszxFsOut47157215;     ORszxFsOut47157215 = ORszxFsOut82160456;     ORszxFsOut82160456 = ORszxFsOut50332782;     ORszxFsOut50332782 = ORszxFsOut89588238;     ORszxFsOut89588238 = ORszxFsOut84982280;     ORszxFsOut84982280 = ORszxFsOut40016770;     ORszxFsOut40016770 = ORszxFsOut2810178;     ORszxFsOut2810178 = ORszxFsOut92298017;     ORszxFsOut92298017 = ORszxFsOut56584470;     ORszxFsOut56584470 = ORszxFsOut55980494;     ORszxFsOut55980494 = ORszxFsOut77053831;     ORszxFsOut77053831 = ORszxFsOut815276;     ORszxFsOut815276 = ORszxFsOut76451759;     ORszxFsOut76451759 = ORszxFsOut5975057;     ORszxFsOut5975057 = ORszxFsOut7959839;     ORszxFsOut7959839 = ORszxFsOut43033626;     ORszxFsOut43033626 = ORszxFsOut48311860;     ORszxFsOut48311860 = ORszxFsOut40184455;     ORszxFsOut40184455 = ORszxFsOut53593329;     ORszxFsOut53593329 = ORszxFsOut2380993;     ORszxFsOut2380993 = ORszxFsOut40093587;     ORszxFsOut40093587 = ORszxFsOut96929102;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sJOiWPCDRd27163596() {     int SsfQdfdcwv9715270 = 9424730;    int SsfQdfdcwv25398024 = -778032324;    int SsfQdfdcwv15714059 = -534069470;    int SsfQdfdcwv95384173 = -263017746;    int SsfQdfdcwv77865522 = -581251190;    int SsfQdfdcwv32538997 = -694998423;    int SsfQdfdcwv15413830 = -720993464;    int SsfQdfdcwv34510668 = -717117806;    int SsfQdfdcwv16567169 = -832060420;    int SsfQdfdcwv97346531 = -904239094;    int SsfQdfdcwv80071867 = 89559740;    int SsfQdfdcwv12942410 = -194925504;    int SsfQdfdcwv72992044 = -961451465;    int SsfQdfdcwv76080807 = 37666771;    int SsfQdfdcwv39030244 = -857993100;    int SsfQdfdcwv9592020 = -264187749;    int SsfQdfdcwv68909195 = -472036779;    int SsfQdfdcwv72576223 = -681364079;    int SsfQdfdcwv29430863 = -929171288;    int SsfQdfdcwv6068331 = -322219703;    int SsfQdfdcwv69929362 = -568082196;    int SsfQdfdcwv68683427 = 17583184;    int SsfQdfdcwv9061136 = -667715207;    int SsfQdfdcwv4270056 = -94733720;    int SsfQdfdcwv21627486 = -707006645;    int SsfQdfdcwv79058721 = -799715113;    int SsfQdfdcwv33338622 = -209237695;    int SsfQdfdcwv54540059 = -222477183;    int SsfQdfdcwv50894853 = -390348690;    int SsfQdfdcwv35687803 = -632403220;    int SsfQdfdcwv16660515 = -881817703;    int SsfQdfdcwv79674921 = 57736380;    int SsfQdfdcwv62504153 = -362863762;    int SsfQdfdcwv9285 = -867933205;    int SsfQdfdcwv17407858 = -35767941;    int SsfQdfdcwv64365686 = -277099773;    int SsfQdfdcwv51414153 = -10538661;    int SsfQdfdcwv35389200 = -495333996;    int SsfQdfdcwv349566 = -817610771;    int SsfQdfdcwv36604196 = -486157266;    int SsfQdfdcwv16762107 = 30759351;    int SsfQdfdcwv21400666 = -610433592;    int SsfQdfdcwv26702470 = -530701943;    int SsfQdfdcwv65912838 = -625235343;    int SsfQdfdcwv96330464 = -149423577;    int SsfQdfdcwv42204141 = -176757718;    int SsfQdfdcwv93161639 = -598876889;    int SsfQdfdcwv1030271 = -259709133;    int SsfQdfdcwv41662945 = -25036485;    int SsfQdfdcwv96245940 = -224370222;    int SsfQdfdcwv61247411 = -913191534;    int SsfQdfdcwv97590888 = -903002557;    int SsfQdfdcwv18543231 = -566774567;    int SsfQdfdcwv4573459 = -740285266;    int SsfQdfdcwv57234233 = -376649101;    int SsfQdfdcwv41031843 = 91841545;    int SsfQdfdcwv16336888 = -10317118;    int SsfQdfdcwv11444003 = -339335750;    int SsfQdfdcwv73756687 = -556011102;    int SsfQdfdcwv98806800 = -781536078;    int SsfQdfdcwv99200374 = -385760729;    int SsfQdfdcwv60873771 = -398516281;    int SsfQdfdcwv83615814 = -226769117;    int SsfQdfdcwv80879366 = -99657200;    int SsfQdfdcwv80686017 = 77578609;    int SsfQdfdcwv396947 = -968176641;    int SsfQdfdcwv50438257 = -832061742;    int SsfQdfdcwv72982760 = 6481740;    int SsfQdfdcwv58672949 = -926565288;    int SsfQdfdcwv74664557 = -480893328;    int SsfQdfdcwv58177867 = -153649089;    int SsfQdfdcwv33519995 = -976702784;    int SsfQdfdcwv72226657 = -863753309;    int SsfQdfdcwv92826667 = -343014023;    int SsfQdfdcwv89306224 = -252979055;    int SsfQdfdcwv48528697 = -957648604;    int SsfQdfdcwv41980957 = -451714873;    int SsfQdfdcwv43148297 = 57520136;    int SsfQdfdcwv7939591 = -945310144;    int SsfQdfdcwv79423344 = -430248927;    int SsfQdfdcwv85897082 = -100838224;    int SsfQdfdcwv32308352 = -949528562;    int SsfQdfdcwv12877115 = -97440699;    int SsfQdfdcwv54648913 = -65978469;    int SsfQdfdcwv74440392 = -719211687;    int SsfQdfdcwv19069627 = -978815147;    int SsfQdfdcwv61131691 = -375489053;    int SsfQdfdcwv57930694 = -622578497;    int SsfQdfdcwv42775051 = -391284105;    int SsfQdfdcwv76376015 = -27609487;    int SsfQdfdcwv48028798 = -166782655;    int SsfQdfdcwv39970150 = -671202911;    int SsfQdfdcwv61632513 = -939322894;    int SsfQdfdcwv1542766 = 63925306;    int SsfQdfdcwv37403821 = -396538;    int SsfQdfdcwv55888335 = -570724369;    int SsfQdfdcwv37784851 = -283664475;    int SsfQdfdcwv45823104 = -331044744;    int SsfQdfdcwv85226821 = -602813952;    int SsfQdfdcwv95933518 = 9424730;     SsfQdfdcwv9715270 = SsfQdfdcwv25398024;     SsfQdfdcwv25398024 = SsfQdfdcwv15714059;     SsfQdfdcwv15714059 = SsfQdfdcwv95384173;     SsfQdfdcwv95384173 = SsfQdfdcwv77865522;     SsfQdfdcwv77865522 = SsfQdfdcwv32538997;     SsfQdfdcwv32538997 = SsfQdfdcwv15413830;     SsfQdfdcwv15413830 = SsfQdfdcwv34510668;     SsfQdfdcwv34510668 = SsfQdfdcwv16567169;     SsfQdfdcwv16567169 = SsfQdfdcwv97346531;     SsfQdfdcwv97346531 = SsfQdfdcwv80071867;     SsfQdfdcwv80071867 = SsfQdfdcwv12942410;     SsfQdfdcwv12942410 = SsfQdfdcwv72992044;     SsfQdfdcwv72992044 = SsfQdfdcwv76080807;     SsfQdfdcwv76080807 = SsfQdfdcwv39030244;     SsfQdfdcwv39030244 = SsfQdfdcwv9592020;     SsfQdfdcwv9592020 = SsfQdfdcwv68909195;     SsfQdfdcwv68909195 = SsfQdfdcwv72576223;     SsfQdfdcwv72576223 = SsfQdfdcwv29430863;     SsfQdfdcwv29430863 = SsfQdfdcwv6068331;     SsfQdfdcwv6068331 = SsfQdfdcwv69929362;     SsfQdfdcwv69929362 = SsfQdfdcwv68683427;     SsfQdfdcwv68683427 = SsfQdfdcwv9061136;     SsfQdfdcwv9061136 = SsfQdfdcwv4270056;     SsfQdfdcwv4270056 = SsfQdfdcwv21627486;     SsfQdfdcwv21627486 = SsfQdfdcwv79058721;     SsfQdfdcwv79058721 = SsfQdfdcwv33338622;     SsfQdfdcwv33338622 = SsfQdfdcwv54540059;     SsfQdfdcwv54540059 = SsfQdfdcwv50894853;     SsfQdfdcwv50894853 = SsfQdfdcwv35687803;     SsfQdfdcwv35687803 = SsfQdfdcwv16660515;     SsfQdfdcwv16660515 = SsfQdfdcwv79674921;     SsfQdfdcwv79674921 = SsfQdfdcwv62504153;     SsfQdfdcwv62504153 = SsfQdfdcwv9285;     SsfQdfdcwv9285 = SsfQdfdcwv17407858;     SsfQdfdcwv17407858 = SsfQdfdcwv64365686;     SsfQdfdcwv64365686 = SsfQdfdcwv51414153;     SsfQdfdcwv51414153 = SsfQdfdcwv35389200;     SsfQdfdcwv35389200 = SsfQdfdcwv349566;     SsfQdfdcwv349566 = SsfQdfdcwv36604196;     SsfQdfdcwv36604196 = SsfQdfdcwv16762107;     SsfQdfdcwv16762107 = SsfQdfdcwv21400666;     SsfQdfdcwv21400666 = SsfQdfdcwv26702470;     SsfQdfdcwv26702470 = SsfQdfdcwv65912838;     SsfQdfdcwv65912838 = SsfQdfdcwv96330464;     SsfQdfdcwv96330464 = SsfQdfdcwv42204141;     SsfQdfdcwv42204141 = SsfQdfdcwv93161639;     SsfQdfdcwv93161639 = SsfQdfdcwv1030271;     SsfQdfdcwv1030271 = SsfQdfdcwv41662945;     SsfQdfdcwv41662945 = SsfQdfdcwv96245940;     SsfQdfdcwv96245940 = SsfQdfdcwv61247411;     SsfQdfdcwv61247411 = SsfQdfdcwv97590888;     SsfQdfdcwv97590888 = SsfQdfdcwv18543231;     SsfQdfdcwv18543231 = SsfQdfdcwv4573459;     SsfQdfdcwv4573459 = SsfQdfdcwv57234233;     SsfQdfdcwv57234233 = SsfQdfdcwv41031843;     SsfQdfdcwv41031843 = SsfQdfdcwv16336888;     SsfQdfdcwv16336888 = SsfQdfdcwv11444003;     SsfQdfdcwv11444003 = SsfQdfdcwv73756687;     SsfQdfdcwv73756687 = SsfQdfdcwv98806800;     SsfQdfdcwv98806800 = SsfQdfdcwv99200374;     SsfQdfdcwv99200374 = SsfQdfdcwv60873771;     SsfQdfdcwv60873771 = SsfQdfdcwv83615814;     SsfQdfdcwv83615814 = SsfQdfdcwv80879366;     SsfQdfdcwv80879366 = SsfQdfdcwv80686017;     SsfQdfdcwv80686017 = SsfQdfdcwv396947;     SsfQdfdcwv396947 = SsfQdfdcwv50438257;     SsfQdfdcwv50438257 = SsfQdfdcwv72982760;     SsfQdfdcwv72982760 = SsfQdfdcwv58672949;     SsfQdfdcwv58672949 = SsfQdfdcwv74664557;     SsfQdfdcwv74664557 = SsfQdfdcwv58177867;     SsfQdfdcwv58177867 = SsfQdfdcwv33519995;     SsfQdfdcwv33519995 = SsfQdfdcwv72226657;     SsfQdfdcwv72226657 = SsfQdfdcwv92826667;     SsfQdfdcwv92826667 = SsfQdfdcwv89306224;     SsfQdfdcwv89306224 = SsfQdfdcwv48528697;     SsfQdfdcwv48528697 = SsfQdfdcwv41980957;     SsfQdfdcwv41980957 = SsfQdfdcwv43148297;     SsfQdfdcwv43148297 = SsfQdfdcwv7939591;     SsfQdfdcwv7939591 = SsfQdfdcwv79423344;     SsfQdfdcwv79423344 = SsfQdfdcwv85897082;     SsfQdfdcwv85897082 = SsfQdfdcwv32308352;     SsfQdfdcwv32308352 = SsfQdfdcwv12877115;     SsfQdfdcwv12877115 = SsfQdfdcwv54648913;     SsfQdfdcwv54648913 = SsfQdfdcwv74440392;     SsfQdfdcwv74440392 = SsfQdfdcwv19069627;     SsfQdfdcwv19069627 = SsfQdfdcwv61131691;     SsfQdfdcwv61131691 = SsfQdfdcwv57930694;     SsfQdfdcwv57930694 = SsfQdfdcwv42775051;     SsfQdfdcwv42775051 = SsfQdfdcwv76376015;     SsfQdfdcwv76376015 = SsfQdfdcwv48028798;     SsfQdfdcwv48028798 = SsfQdfdcwv39970150;     SsfQdfdcwv39970150 = SsfQdfdcwv61632513;     SsfQdfdcwv61632513 = SsfQdfdcwv1542766;     SsfQdfdcwv1542766 = SsfQdfdcwv37403821;     SsfQdfdcwv37403821 = SsfQdfdcwv55888335;     SsfQdfdcwv55888335 = SsfQdfdcwv37784851;     SsfQdfdcwv37784851 = SsfQdfdcwv45823104;     SsfQdfdcwv45823104 = SsfQdfdcwv85226821;     SsfQdfdcwv85226821 = SsfQdfdcwv95933518;     SsfQdfdcwv95933518 = SsfQdfdcwv9715270;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OijsLsSbOP45376938() {     int BVTZDRkytK93159574 = -871240951;    int BVTZDRkytK42270112 = -777414512;    int BVTZDRkytK12122566 = -327594698;    int BVTZDRkytK58176425 = -456817582;    int BVTZDRkytK83986846 = -388118680;    int BVTZDRkytK31043398 = -474070076;    int BVTZDRkytK86453147 = -460904777;    int BVTZDRkytK48858111 = -704275836;    int BVTZDRkytK75096869 = -73519771;    int BVTZDRkytK29892184 = -272615206;    int BVTZDRkytK29487806 = -100556133;    int BVTZDRkytK76147809 = -182543750;    int BVTZDRkytK91109820 = -753112706;    int BVTZDRkytK55707699 = -850623586;    int BVTZDRkytK99884707 = -952016083;    int BVTZDRkytK95981181 = -52267074;    int BVTZDRkytK62661080 = -225381806;    int BVTZDRkytK53427669 = -928668956;    int BVTZDRkytK91871005 = -927846159;    int BVTZDRkytK95435773 = -851805695;    int BVTZDRkytK25569750 = -157611227;    int BVTZDRkytK80830745 = 26431414;    int BVTZDRkytK12886843 = -707604404;    int BVTZDRkytK38976614 = -166559116;    int BVTZDRkytK63203444 = -166659404;    int BVTZDRkytK74400870 = -680224828;    int BVTZDRkytK82793362 = -937322037;    int BVTZDRkytK94271684 = -292432067;    int BVTZDRkytK86975947 = -252853832;    int BVTZDRkytK51521144 = -45253072;    int BVTZDRkytK34755828 = -519574904;    int BVTZDRkytK42781349 = -957458700;    int BVTZDRkytK6043041 = 16264061;    int BVTZDRkytK42167795 = -459535375;    int BVTZDRkytK96134587 = -233432676;    int BVTZDRkytK76754870 = -65123679;    int BVTZDRkytK20028332 = -989387841;    int BVTZDRkytK75968032 = -203804516;    int BVTZDRkytK49344644 = 97491431;    int BVTZDRkytK86099330 = -238750788;    int BVTZDRkytK50411031 = -640789622;    int BVTZDRkytK71888452 = -84341914;    int BVTZDRkytK50269540 = 18240814;    int BVTZDRkytK57817778 = -381418114;    int BVTZDRkytK88274961 = -977580530;    int BVTZDRkytK81810137 = -196368642;    int BVTZDRkytK69177069 = -731181119;    int BVTZDRkytK20700866 = -258430852;    int BVTZDRkytK72510057 = -610638567;    int BVTZDRkytK31578425 = -431390677;    int BVTZDRkytK34504859 = -34092956;    int BVTZDRkytK53091683 = -961261868;    int BVTZDRkytK32655690 = -778521883;    int BVTZDRkytK29250218 = -458698653;    int BVTZDRkytK19807188 = -725135224;    int BVTZDRkytK12328830 = -797672365;    int BVTZDRkytK29383270 = 30189892;    int BVTZDRkytK73145952 = -61035582;    int BVTZDRkytK94972980 = -190158179;    int BVTZDRkytK9585976 = -707893853;    int BVTZDRkytK48250035 = -536748039;    int BVTZDRkytK92181463 = -68472710;    int BVTZDRkytK61882163 = -351422005;    int BVTZDRkytK23575726 = 71733300;    int BVTZDRkytK95136355 = -753040302;    int BVTZDRkytK86706456 = -143097434;    int BVTZDRkytK70104769 = -98807811;    int BVTZDRkytK48942025 = -193577331;    int BVTZDRkytK59573111 = -517190910;    int BVTZDRkytK23129837 = -786892404;    int BVTZDRkytK75952850 = -62879234;    int BVTZDRkytK86693047 = 78422709;    int BVTZDRkytK4083026 = -926160387;    int BVTZDRkytK5771675 = -589095372;    int BVTZDRkytK45024743 = -111016074;    int BVTZDRkytK53681298 = 26730686;    int BVTZDRkytK30561206 = -991809401;    int BVTZDRkytK55069065 = -226186291;    int BVTZDRkytK50701652 = -188978587;    int BVTZDRkytK81393306 = -970290762;    int BVTZDRkytK5223801 = -949043710;    int BVTZDRkytK62092496 = -578891186;    int BVTZDRkytK21761628 = -681793501;    int BVTZDRkytK55397523 = -821463156;    int BVTZDRkytK17016285 = 88839884;    int BVTZDRkytK81664145 = -558313036;    int BVTZDRkytK10125660 = -78936817;    int BVTZDRkytK76792822 = -525037287;    int BVTZDRkytK22360608 = -734400152;    int BVTZDRkytK83805758 = -435760312;    int BVTZDRkytK47371601 = 4686429;    int BVTZDRkytK46882380 = -828352259;    int BVTZDRkytK80995051 = 86353663;    int BVTZDRkytK39758668 = -194614716;    int BVTZDRkytK37849296 = -702002749;    int BVTZDRkytK58229567 = -472316912;    int BVTZDRkytK10006289 = -732919910;    int BVTZDRkytK26693815 = 46507513;    int BVTZDRkytK62681423 = -628377812;    int BVTZDRkytK1568505 = -871240951;     BVTZDRkytK93159574 = BVTZDRkytK42270112;     BVTZDRkytK42270112 = BVTZDRkytK12122566;     BVTZDRkytK12122566 = BVTZDRkytK58176425;     BVTZDRkytK58176425 = BVTZDRkytK83986846;     BVTZDRkytK83986846 = BVTZDRkytK31043398;     BVTZDRkytK31043398 = BVTZDRkytK86453147;     BVTZDRkytK86453147 = BVTZDRkytK48858111;     BVTZDRkytK48858111 = BVTZDRkytK75096869;     BVTZDRkytK75096869 = BVTZDRkytK29892184;     BVTZDRkytK29892184 = BVTZDRkytK29487806;     BVTZDRkytK29487806 = BVTZDRkytK76147809;     BVTZDRkytK76147809 = BVTZDRkytK91109820;     BVTZDRkytK91109820 = BVTZDRkytK55707699;     BVTZDRkytK55707699 = BVTZDRkytK99884707;     BVTZDRkytK99884707 = BVTZDRkytK95981181;     BVTZDRkytK95981181 = BVTZDRkytK62661080;     BVTZDRkytK62661080 = BVTZDRkytK53427669;     BVTZDRkytK53427669 = BVTZDRkytK91871005;     BVTZDRkytK91871005 = BVTZDRkytK95435773;     BVTZDRkytK95435773 = BVTZDRkytK25569750;     BVTZDRkytK25569750 = BVTZDRkytK80830745;     BVTZDRkytK80830745 = BVTZDRkytK12886843;     BVTZDRkytK12886843 = BVTZDRkytK38976614;     BVTZDRkytK38976614 = BVTZDRkytK63203444;     BVTZDRkytK63203444 = BVTZDRkytK74400870;     BVTZDRkytK74400870 = BVTZDRkytK82793362;     BVTZDRkytK82793362 = BVTZDRkytK94271684;     BVTZDRkytK94271684 = BVTZDRkytK86975947;     BVTZDRkytK86975947 = BVTZDRkytK51521144;     BVTZDRkytK51521144 = BVTZDRkytK34755828;     BVTZDRkytK34755828 = BVTZDRkytK42781349;     BVTZDRkytK42781349 = BVTZDRkytK6043041;     BVTZDRkytK6043041 = BVTZDRkytK42167795;     BVTZDRkytK42167795 = BVTZDRkytK96134587;     BVTZDRkytK96134587 = BVTZDRkytK76754870;     BVTZDRkytK76754870 = BVTZDRkytK20028332;     BVTZDRkytK20028332 = BVTZDRkytK75968032;     BVTZDRkytK75968032 = BVTZDRkytK49344644;     BVTZDRkytK49344644 = BVTZDRkytK86099330;     BVTZDRkytK86099330 = BVTZDRkytK50411031;     BVTZDRkytK50411031 = BVTZDRkytK71888452;     BVTZDRkytK71888452 = BVTZDRkytK50269540;     BVTZDRkytK50269540 = BVTZDRkytK57817778;     BVTZDRkytK57817778 = BVTZDRkytK88274961;     BVTZDRkytK88274961 = BVTZDRkytK81810137;     BVTZDRkytK81810137 = BVTZDRkytK69177069;     BVTZDRkytK69177069 = BVTZDRkytK20700866;     BVTZDRkytK20700866 = BVTZDRkytK72510057;     BVTZDRkytK72510057 = BVTZDRkytK31578425;     BVTZDRkytK31578425 = BVTZDRkytK34504859;     BVTZDRkytK34504859 = BVTZDRkytK53091683;     BVTZDRkytK53091683 = BVTZDRkytK32655690;     BVTZDRkytK32655690 = BVTZDRkytK29250218;     BVTZDRkytK29250218 = BVTZDRkytK19807188;     BVTZDRkytK19807188 = BVTZDRkytK12328830;     BVTZDRkytK12328830 = BVTZDRkytK29383270;     BVTZDRkytK29383270 = BVTZDRkytK73145952;     BVTZDRkytK73145952 = BVTZDRkytK94972980;     BVTZDRkytK94972980 = BVTZDRkytK9585976;     BVTZDRkytK9585976 = BVTZDRkytK48250035;     BVTZDRkytK48250035 = BVTZDRkytK92181463;     BVTZDRkytK92181463 = BVTZDRkytK61882163;     BVTZDRkytK61882163 = BVTZDRkytK23575726;     BVTZDRkytK23575726 = BVTZDRkytK95136355;     BVTZDRkytK95136355 = BVTZDRkytK86706456;     BVTZDRkytK86706456 = BVTZDRkytK70104769;     BVTZDRkytK70104769 = BVTZDRkytK48942025;     BVTZDRkytK48942025 = BVTZDRkytK59573111;     BVTZDRkytK59573111 = BVTZDRkytK23129837;     BVTZDRkytK23129837 = BVTZDRkytK75952850;     BVTZDRkytK75952850 = BVTZDRkytK86693047;     BVTZDRkytK86693047 = BVTZDRkytK4083026;     BVTZDRkytK4083026 = BVTZDRkytK5771675;     BVTZDRkytK5771675 = BVTZDRkytK45024743;     BVTZDRkytK45024743 = BVTZDRkytK53681298;     BVTZDRkytK53681298 = BVTZDRkytK30561206;     BVTZDRkytK30561206 = BVTZDRkytK55069065;     BVTZDRkytK55069065 = BVTZDRkytK50701652;     BVTZDRkytK50701652 = BVTZDRkytK81393306;     BVTZDRkytK81393306 = BVTZDRkytK5223801;     BVTZDRkytK5223801 = BVTZDRkytK62092496;     BVTZDRkytK62092496 = BVTZDRkytK21761628;     BVTZDRkytK21761628 = BVTZDRkytK55397523;     BVTZDRkytK55397523 = BVTZDRkytK17016285;     BVTZDRkytK17016285 = BVTZDRkytK81664145;     BVTZDRkytK81664145 = BVTZDRkytK10125660;     BVTZDRkytK10125660 = BVTZDRkytK76792822;     BVTZDRkytK76792822 = BVTZDRkytK22360608;     BVTZDRkytK22360608 = BVTZDRkytK83805758;     BVTZDRkytK83805758 = BVTZDRkytK47371601;     BVTZDRkytK47371601 = BVTZDRkytK46882380;     BVTZDRkytK46882380 = BVTZDRkytK80995051;     BVTZDRkytK80995051 = BVTZDRkytK39758668;     BVTZDRkytK39758668 = BVTZDRkytK37849296;     BVTZDRkytK37849296 = BVTZDRkytK58229567;     BVTZDRkytK58229567 = BVTZDRkytK10006289;     BVTZDRkytK10006289 = BVTZDRkytK26693815;     BVTZDRkytK26693815 = BVTZDRkytK62681423;     BVTZDRkytK62681423 = BVTZDRkytK1568505;     BVTZDRkytK1568505 = BVTZDRkytK93159574;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AzfPncPmFO15832811() {     int dxbnpVjXeZ5945742 = 93718357;    int dxbnpVjXeZ29041001 = -947748525;    int dxbnpVjXeZ29875365 = -735954079;    int dxbnpVjXeZ66117250 = -281019859;    int dxbnpVjXeZ77497950 = -879935145;    int dxbnpVjXeZ74353468 = -250993390;    int dxbnpVjXeZ7660906 = 93423153;    int dxbnpVjXeZ75577223 = -48794093;    int dxbnpVjXeZ70501680 = -35277863;    int dxbnpVjXeZ36110888 = -76702839;    int dxbnpVjXeZ56975188 = -215242148;    int dxbnpVjXeZ8098314 = -689876566;    int dxbnpVjXeZ61253123 = -656895363;    int dxbnpVjXeZ5331778 = -83038784;    int dxbnpVjXeZ23191260 = -859001913;    int dxbnpVjXeZ24406250 = -603158464;    int dxbnpVjXeZ18412898 = -989156273;    int dxbnpVjXeZ36986155 = -483879258;    int dxbnpVjXeZ93031635 = -837315355;    int dxbnpVjXeZ98090269 = -351049834;    int dxbnpVjXeZ5743807 = -648927491;    int dxbnpVjXeZ9878565 = -143426934;    int dxbnpVjXeZ51647138 = -638608299;    int dxbnpVjXeZ83483743 = -526904129;    int dxbnpVjXeZ69097703 = -377101328;    int dxbnpVjXeZ55719516 = -370656027;    int dxbnpVjXeZ94208898 = -471028318;    int dxbnpVjXeZ61624609 = -466230172;    int dxbnpVjXeZ3593001 = -467071072;    int dxbnpVjXeZ4272260 = -775139226;    int dxbnpVjXeZ79191824 = -795991654;    int dxbnpVjXeZ46100172 = -936178378;    int dxbnpVjXeZ52088989 = -959590724;    int dxbnpVjXeZ19244169 = -364289594;    int dxbnpVjXeZ79195120 = -843464557;    int dxbnpVjXeZ71978985 = -953686783;    int dxbnpVjXeZ56792392 = -779297191;    int dxbnpVjXeZ33672055 = -123424607;    int dxbnpVjXeZ55120030 = -844359146;    int dxbnpVjXeZ12364061 = -83167634;    int dxbnpVjXeZ11842339 = 83736385;    int dxbnpVjXeZ79590908 = 17346752;    int dxbnpVjXeZ85153009 = -5192631;    int dxbnpVjXeZ2446316 = -722590357;    int dxbnpVjXeZ68283452 = -870350490;    int dxbnpVjXeZ55438266 = -269687082;    int dxbnpVjXeZ46759089 = -128696171;    int dxbnpVjXeZ50140832 = -413783631;    int dxbnpVjXeZ16574106 = -774240542;    int dxbnpVjXeZ78528845 = 57572860;    int dxbnpVjXeZ52832353 = -286977264;    int dxbnpVjXeZ81268229 = -332447206;    int dxbnpVjXeZ67140839 = -954525166;    int dxbnpVjXeZ73949943 = -48150395;    int dxbnpVjXeZ10089004 = -719579711;    int dxbnpVjXeZ96067177 = -762854709;    int dxbnpVjXeZ77393862 = -209140227;    int dxbnpVjXeZ46391622 = -109049951;    int dxbnpVjXeZ97019547 = -903918532;    int dxbnpVjXeZ21778435 = -409279119;    int dxbnpVjXeZ80144569 = -779965072;    int dxbnpVjXeZ46036297 = -440346676;    int dxbnpVjXeZ71984223 = -581723021;    int dxbnpVjXeZ66229420 = -260138637;    int dxbnpVjXeZ56919064 = -280711185;    int dxbnpVjXeZ10875016 = -279063771;    int dxbnpVjXeZ56009325 = -730285843;    int dxbnpVjXeZ42008955 = -192605770;    int dxbnpVjXeZ26136658 = -239574228;    int dxbnpVjXeZ51212275 = -905315131;    int dxbnpVjXeZ67613858 = -823861273;    int dxbnpVjXeZ84740842 = -765731666;    int dxbnpVjXeZ81866125 = -639520113;    int dxbnpVjXeZ80667575 = -654147721;    int dxbnpVjXeZ86247930 = -334786220;    int dxbnpVjXeZ26152899 = -566274244;    int dxbnpVjXeZ24725555 = -38234304;    int dxbnpVjXeZ49200823 = -916017942;    int dxbnpVjXeZ15200292 = -656553639;    int dxbnpVjXeZ13659437 = -7414247;    int dxbnpVjXeZ8960427 = -141959857;    int dxbnpVjXeZ44068066 = 42755313;    int dxbnpVjXeZ45050503 = -691989631;    int dxbnpVjXeZ25064156 = -424643933;    int dxbnpVjXeZ51439907 = -388161963;    int dxbnpVjXeZ97923594 = -363544448;    int dxbnpVjXeZ78959332 = -981653213;    int dxbnpVjXeZ78139046 = -811440329;    int dxbnpVjXeZ9155165 = -644709883;    int dxbnpVjXeZ83127942 = 19390152;    int dxbnpVjXeZ94585123 = -644546556;    int dxbnpVjXeZ10400770 = -570247240;    int dxbnpVjXeZ36652508 = -219506076;    int dxbnpVjXeZ33341595 = -335080028;    int dxbnpVjXeZ32219491 = -303202562;    int dxbnpVjXeZ65806042 = -475916940;    int dxbnpVjXeZ7606686 = -400930227;    int dxbnpVjXeZ18923589 = -745053994;    int dxbnpVjXeZ45527252 = -341879172;    int dxbnpVjXeZ57408436 = 93718357;     dxbnpVjXeZ5945742 = dxbnpVjXeZ29041001;     dxbnpVjXeZ29041001 = dxbnpVjXeZ29875365;     dxbnpVjXeZ29875365 = dxbnpVjXeZ66117250;     dxbnpVjXeZ66117250 = dxbnpVjXeZ77497950;     dxbnpVjXeZ77497950 = dxbnpVjXeZ74353468;     dxbnpVjXeZ74353468 = dxbnpVjXeZ7660906;     dxbnpVjXeZ7660906 = dxbnpVjXeZ75577223;     dxbnpVjXeZ75577223 = dxbnpVjXeZ70501680;     dxbnpVjXeZ70501680 = dxbnpVjXeZ36110888;     dxbnpVjXeZ36110888 = dxbnpVjXeZ56975188;     dxbnpVjXeZ56975188 = dxbnpVjXeZ8098314;     dxbnpVjXeZ8098314 = dxbnpVjXeZ61253123;     dxbnpVjXeZ61253123 = dxbnpVjXeZ5331778;     dxbnpVjXeZ5331778 = dxbnpVjXeZ23191260;     dxbnpVjXeZ23191260 = dxbnpVjXeZ24406250;     dxbnpVjXeZ24406250 = dxbnpVjXeZ18412898;     dxbnpVjXeZ18412898 = dxbnpVjXeZ36986155;     dxbnpVjXeZ36986155 = dxbnpVjXeZ93031635;     dxbnpVjXeZ93031635 = dxbnpVjXeZ98090269;     dxbnpVjXeZ98090269 = dxbnpVjXeZ5743807;     dxbnpVjXeZ5743807 = dxbnpVjXeZ9878565;     dxbnpVjXeZ9878565 = dxbnpVjXeZ51647138;     dxbnpVjXeZ51647138 = dxbnpVjXeZ83483743;     dxbnpVjXeZ83483743 = dxbnpVjXeZ69097703;     dxbnpVjXeZ69097703 = dxbnpVjXeZ55719516;     dxbnpVjXeZ55719516 = dxbnpVjXeZ94208898;     dxbnpVjXeZ94208898 = dxbnpVjXeZ61624609;     dxbnpVjXeZ61624609 = dxbnpVjXeZ3593001;     dxbnpVjXeZ3593001 = dxbnpVjXeZ4272260;     dxbnpVjXeZ4272260 = dxbnpVjXeZ79191824;     dxbnpVjXeZ79191824 = dxbnpVjXeZ46100172;     dxbnpVjXeZ46100172 = dxbnpVjXeZ52088989;     dxbnpVjXeZ52088989 = dxbnpVjXeZ19244169;     dxbnpVjXeZ19244169 = dxbnpVjXeZ79195120;     dxbnpVjXeZ79195120 = dxbnpVjXeZ71978985;     dxbnpVjXeZ71978985 = dxbnpVjXeZ56792392;     dxbnpVjXeZ56792392 = dxbnpVjXeZ33672055;     dxbnpVjXeZ33672055 = dxbnpVjXeZ55120030;     dxbnpVjXeZ55120030 = dxbnpVjXeZ12364061;     dxbnpVjXeZ12364061 = dxbnpVjXeZ11842339;     dxbnpVjXeZ11842339 = dxbnpVjXeZ79590908;     dxbnpVjXeZ79590908 = dxbnpVjXeZ85153009;     dxbnpVjXeZ85153009 = dxbnpVjXeZ2446316;     dxbnpVjXeZ2446316 = dxbnpVjXeZ68283452;     dxbnpVjXeZ68283452 = dxbnpVjXeZ55438266;     dxbnpVjXeZ55438266 = dxbnpVjXeZ46759089;     dxbnpVjXeZ46759089 = dxbnpVjXeZ50140832;     dxbnpVjXeZ50140832 = dxbnpVjXeZ16574106;     dxbnpVjXeZ16574106 = dxbnpVjXeZ78528845;     dxbnpVjXeZ78528845 = dxbnpVjXeZ52832353;     dxbnpVjXeZ52832353 = dxbnpVjXeZ81268229;     dxbnpVjXeZ81268229 = dxbnpVjXeZ67140839;     dxbnpVjXeZ67140839 = dxbnpVjXeZ73949943;     dxbnpVjXeZ73949943 = dxbnpVjXeZ10089004;     dxbnpVjXeZ10089004 = dxbnpVjXeZ96067177;     dxbnpVjXeZ96067177 = dxbnpVjXeZ77393862;     dxbnpVjXeZ77393862 = dxbnpVjXeZ46391622;     dxbnpVjXeZ46391622 = dxbnpVjXeZ97019547;     dxbnpVjXeZ97019547 = dxbnpVjXeZ21778435;     dxbnpVjXeZ21778435 = dxbnpVjXeZ80144569;     dxbnpVjXeZ80144569 = dxbnpVjXeZ46036297;     dxbnpVjXeZ46036297 = dxbnpVjXeZ71984223;     dxbnpVjXeZ71984223 = dxbnpVjXeZ66229420;     dxbnpVjXeZ66229420 = dxbnpVjXeZ56919064;     dxbnpVjXeZ56919064 = dxbnpVjXeZ10875016;     dxbnpVjXeZ10875016 = dxbnpVjXeZ56009325;     dxbnpVjXeZ56009325 = dxbnpVjXeZ42008955;     dxbnpVjXeZ42008955 = dxbnpVjXeZ26136658;     dxbnpVjXeZ26136658 = dxbnpVjXeZ51212275;     dxbnpVjXeZ51212275 = dxbnpVjXeZ67613858;     dxbnpVjXeZ67613858 = dxbnpVjXeZ84740842;     dxbnpVjXeZ84740842 = dxbnpVjXeZ81866125;     dxbnpVjXeZ81866125 = dxbnpVjXeZ80667575;     dxbnpVjXeZ80667575 = dxbnpVjXeZ86247930;     dxbnpVjXeZ86247930 = dxbnpVjXeZ26152899;     dxbnpVjXeZ26152899 = dxbnpVjXeZ24725555;     dxbnpVjXeZ24725555 = dxbnpVjXeZ49200823;     dxbnpVjXeZ49200823 = dxbnpVjXeZ15200292;     dxbnpVjXeZ15200292 = dxbnpVjXeZ13659437;     dxbnpVjXeZ13659437 = dxbnpVjXeZ8960427;     dxbnpVjXeZ8960427 = dxbnpVjXeZ44068066;     dxbnpVjXeZ44068066 = dxbnpVjXeZ45050503;     dxbnpVjXeZ45050503 = dxbnpVjXeZ25064156;     dxbnpVjXeZ25064156 = dxbnpVjXeZ51439907;     dxbnpVjXeZ51439907 = dxbnpVjXeZ97923594;     dxbnpVjXeZ97923594 = dxbnpVjXeZ78959332;     dxbnpVjXeZ78959332 = dxbnpVjXeZ78139046;     dxbnpVjXeZ78139046 = dxbnpVjXeZ9155165;     dxbnpVjXeZ9155165 = dxbnpVjXeZ83127942;     dxbnpVjXeZ83127942 = dxbnpVjXeZ94585123;     dxbnpVjXeZ94585123 = dxbnpVjXeZ10400770;     dxbnpVjXeZ10400770 = dxbnpVjXeZ36652508;     dxbnpVjXeZ36652508 = dxbnpVjXeZ33341595;     dxbnpVjXeZ33341595 = dxbnpVjXeZ32219491;     dxbnpVjXeZ32219491 = dxbnpVjXeZ65806042;     dxbnpVjXeZ65806042 = dxbnpVjXeZ7606686;     dxbnpVjXeZ7606686 = dxbnpVjXeZ18923589;     dxbnpVjXeZ18923589 = dxbnpVjXeZ45527252;     dxbnpVjXeZ45527252 = dxbnpVjXeZ57408436;     dxbnpVjXeZ57408436 = dxbnpVjXeZ5945742;}
// Junk Finished
