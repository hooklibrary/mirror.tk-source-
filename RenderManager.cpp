#pragma once

#include "RenderManager.h"

#define _CRT_SECURE_NO_WARNINGS

#define M_PI 3.14159265358979323846

namespace Render
{
	namespace Fonts
	{
		DWORD Default;
		DWORD Menu;
		DWORD MenuBold;
		DWORD esp;
		DWORD MenuText;
		DWORD Icon;
		DWORD MenuTabs;
		DWORD Text;
		DWORD IconESP;
		DWORD Slider;
		DWORD smallassfont;
		DWORD Clock;
		DWORD slickESP;
		DWORD LBY;
		DWORD Tabs;
		DWORD CheckBox;
		DWORD BOMB;
		DWORD MenuSymbols;
		DWORD nameaiz;
		DWORD LBYIndicator;
		DWORD WeaponIcon;
		DWORD nnbruda;
		DWORD niggerbomb;
		DWORD xd;
		DWORD gay;
	};
};


enum EFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};

void Render::Initialise()
{
	Fonts::Default = 0x1D;
	Fonts::Menu = interfaces::surface->FontCreate();
	Fonts::MenuBold = interfaces::surface->FontCreate();
	Fonts::esp = interfaces::surface->FontCreate();
	Fonts::MenuText = interfaces::surface->FontCreate();
	Fonts::Icon = interfaces::surface->FontCreate();
	Fonts::MenuTabs = interfaces::surface->FontCreate();
	Fonts::Slider = interfaces::surface->FontCreate();
	Fonts::Clock = interfaces::surface->FontCreate();
	Fonts::nameaiz = interfaces::surface->FontCreate();
	Fonts::BOMB = interfaces::surface->FontCreate();
	Fonts::nnbruda = interfaces::surface->FontCreate();
	Fonts::LBY = interfaces::surface->FontCreate();
	Fonts::Tabs = interfaces::surface->FontCreate();
	Fonts::CheckBox = interfaces::surface->FontCreate();
	Fonts::MenuSymbols = interfaces::surface->FontCreate();
	Fonts::LBYIndicator = interfaces::surface->FontCreate();
	Fonts::niggerbomb = interfaces::surface->FontCreate();
	Fonts::WeaponIcon = interfaces::surface->FontCreate();
	Fonts::smallassfont = interfaces::surface->FontCreate();
	Fonts::xd = interfaces::surface->FontCreate();
	Fonts::slickESP = interfaces::surface->FontCreate();
	Fonts::IconESP = interfaces::surface->FontCreate();
	Fonts::gay = interfaces::surface->FontCreate();


	interfaces::surface->SetFontGlyphSet(Fonts::nameaiz, "Tahoma", 12, 300, 0, 0, FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::BOMB, "Verdana", 45, 1000, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::Menu, "Verdana", 12, 400, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::Text, "DINPro-Regular", 30, 500, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::MenuBold, "Tahoma", 12, 700, 0, 0,FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::esp, "Smallest Pixel-7", 10, 100, 0, 0, FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::smallassfont, "Smallest Pixel-7", 8, 100, 0, 0, FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::slickESP, "Smallest Pixel-7", 13, 700, 0, 0, FONTFLAG_OUTLINE | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::MenuText, "Verdana", 12,400, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::MenuTabs, "MyScriptFont", 18, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::Slider, "Smallest Pixel-7", 17, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::Clock, "Arial", 22, 575, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::Tabs, "cherryfont", 30, 600, 0, 0, FONTFLAG_ANTIALIAS );
	interfaces::surface->SetFontGlyphSet(Fonts::IconESP, "icomoon", 14, 700, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::nnbruda, "Verdana", 12, 700, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::LBY, "Verdana", 24, 700, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::niggerbomb, "Verdana", 24, 700, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	interfaces::surface->SetFontGlyphSet(Fonts::CheckBox, "eagle", 14, 900, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::Icon, "undefeated", 10, 500, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW | FONTFLAG_OUTLINE);
	interfaces::surface->SetFontGlyphSet(Fonts::MenuSymbols, "Tahoma", 16, 500, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::LBYIndicator, "Smallest Pixel-7", 13, 700, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::WeaponIcon, "cs", 20, 500, 0, 0, FONTFLAG_NONE);
	interfaces::surface->SetFontGlyphSet(Fonts::gay, "Smallest Pixel-7", 11, 250, 0, 0, FONTFLAG_ANTIALIAS);
	interfaces::surface->SetFontGlyphSet(Fonts::xd, "Smallest Pixel-7", 12, 140, 0, 0, FONTFLAG_OUTLINE);

	Utilities::Log("Render System Ready");
}

RECT Render::GetViewport()
{
	RECT Viewport = { 0, 0, 0, 0 };
	int w, h;
	interfaces::engine->GetScreenSize(w, h);
	Viewport.right = w; Viewport.bottom = h;
	return Viewport;
}

void Render::Clear(int x, int y, int w, int h, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawFilledRect(x, y, x + w, y + h);
}

void Render::Outline(float x, float y, float w, float h, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawOutlinedRect(x, y, x + w, y + h);
}
void Render::OutlinedRect(int x, int y, int w, int h, Color color_out, Color color_in)
{
	interfaces::surface->DrawSetColor(color_in);
	interfaces::surface->DrawFilledRect(x, y, x + w, y + h);

	interfaces::surface->DrawSetColor(color_out);
	interfaces::surface->DrawOutlinedRect(x, y, x + w, y + h);
}
void Render::drawRECT(int x1, int y1, int x2, int y2, Color clr)
{
	interfaces::surface->DrawSetColor(clr);
	interfaces::surface->DrawFilledRect(x1, y1, x2, y2);
}

void Render::DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow,float alpha)
{
	Color colColor(0, 0, 0);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		colRainbow.SetAlpha(alpha);
		Render::DrawRect(x + i, y, 1, height, colRainbow);
	}
}
void Render::Line(int x, int y, int x2, int y2, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawLine(x, y, x2, y2);
}

void Render::Line_3(Vector2D start_pos, Vector2D end_pos, Color col)
{
	Render::Line(start_pos.x, start_pos.y, end_pos.x, end_pos.y, col);
}

void Render::PolyLine(int *x, int *y, int count, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawPolyLine(x, y, count);
}



void Render::TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color)
{
	static int texture_id = interfaces::surface->CreateNewTextureID(true); // 
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	interfaces::surface->DrawSetTextureRGBA(texture_id, buf, 1, 1); //
	interfaces::surface->DrawSetColor(color); //
	interfaces::surface->DrawSetTexture(texture_id); //
	interfaces::surface->DrawTexturedPolygon(n, vertice.data()); //
}


void Render::DrawOutlinedRect(int x, int y, int w, int h, Color col)
{
	interfaces::surface->DrawSetColor(col);
	interfaces::surface->DrawOutlinedRect(x, y, x + w, y + h);
}

void Render::DrawLine(int x0, int y0, int x1, int y1, Color col)
{
	interfaces::surface->DrawSetColor(col);
	interfaces::surface->DrawLine(x0, y0, x1, y1);
}
void Render::DrawRect(int x, int y, int w, int h, Color col)
{
	interfaces::surface->DrawSetColor(col);
	interfaces::surface->DrawFilledRect(x, y, x + w, y + h);
}

void Render::DrawFilledRect(int x1, int y1, int x2, int y2, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawFilledRect(x1, y1, x2, y2);
}

void Render::DrawEmptyRect(int x1, int y1, int x2, int y2, Color color, unsigned char ignore_flags)
{
	interfaces::surface->DrawSetColor(color);

	if (!(ignore_flags & 0b1))
		interfaces::surface->DrawLine(x1, y1, x2, y1);

	if (!(ignore_flags & 0b10))
		interfaces::surface->DrawLine(x2, y1, x2, y2);

	if (!(ignore_flags & 0b100))
		interfaces::surface->DrawLine(x2, y2, x1, y2);

	if (!(ignore_flags & 0b1000))
		interfaces::surface->DrawLine(x1, y2, x1, y1);
}



void Render::rect(int x, int y, int w, int h, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawFilledRect(x, y, x + w, y + h);
}
void Render::outlineyeti(int x, int y, int w, int h, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawOutlinedRect(x, y, x + w, y + h);
}

void Render::gradient_verticle(int x, int y, int w, int h, Color c1, Color c2)
{
	Render::rect(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < h; i++)
	{
		float fi = i, fh = h;
		float a = fi / fh;
		DWORD ia = a * 255;
		Render::rect(x, y + i, w, 1, Color(first, second, third, ia));
	}
}

void Render::nonamegey(int x, int y, const char* _Input, int font, Color color)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };
	va_list Args;
	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);

	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextFont(font);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
}

void Render::textT(int x, int y, int font, Color color, const char* _Input, ...)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };
	va_list Args;
	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);
	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextFont(font);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
}

void Render::text_yeti(int x, int y, const char* _Input, int font, Color color)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };
	va_list Args;
	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);

	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextFont(font);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
}

void Render::color_spectrum(int x, int y, int w, int h)
{
	static int GradientTexture = 0;
	static std::unique_ptr<Color[]> Gradient = nullptr;
	if (!Gradient)
	{
		Gradient = std::make_unique<Color[]>(w * h);

		for (int i = 0; i < w; i++)
		{
			int div = w / 6;
			int phase = i / div;
			float t = (i % div) / (float)div;
			int r, g, b;

			switch (phase)
			{
			case(0):
				r = 255;
				g = 255 * t;
				b = 0;
				break;
			case(1):
				r = 255 * (1.f - t);
				g = 255;
				b = 0;
				break;
			case(2):
				r = 0;
				g = 255;
				b = 255 * t;
				break;
			case(3):
				r = 0;
				g = 255 * (1.f - t);
				b = 255;
				break;
			case(4):
				r = 255 * t;
				g = 0;
				b = 255;
				break;
			case(5):
				r = 255;
				g = 0;
				b = 255 * (1.f - t);
				break;
			case 6:
				r = 255;
				g = 255;
				b = 255;
				break;
			}

			for (int k = 0; k < h; k++)
			{
				float sat = k / (float)h;
				int _r = r + sat * (128 - r);
				int _g = g + sat * (128 - g);
				int _b = b + sat * (128 - b);

				*reinterpret_cast<Color*>(Gradient.get() + i + k * w) = Color(_r, _g, _b);
			}
		}

		GradientTexture = interfaces::surface->CreateNewTextureID(true);
		interfaces::surface->DrawSetTextureRGBA(GradientTexture, (unsigned char*)Gradient.get(), w, h);
	}
	interfaces::surface->DrawSetColor(Color(255, 255, 255, 255));
	interfaces::surface->DrawSetTexture(GradientTexture);
	interfaces::surface->DrawTexturedRect(x, y, x + w, y + h);
}

void Render::outlined_rectyeti(int x, int y, int w, int h, Color color_out, Color color_in)
{
	interfaces::surface->DrawSetColor(color_in);
	interfaces::surface->DrawFilledRect(x, y, x + w, y + h);

	interfaces::surface->DrawSetColor(color_out);
	interfaces::surface->DrawOutlinedRect(x, y, x + w, y + h);
}
bool Render::TransformScreen(const Vector& in, Vector& out)
{
	static ptrdiff_t ptrViewMatrix;
	if (!ptrViewMatrix)
	{//                                                          
		ptrViewMatrix = static_cast<ptrdiff_t>(Utilities::Memory::FindPatternV2("client_panorama.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));
		ptrViewMatrix += 0x3;
		ptrViewMatrix = *reinterpret_cast<uintptr_t*>(ptrViewMatrix);
		ptrViewMatrix += 176;
	}
	const matrix3x4& worldToScreen = interfaces::engine->WorldToScreenMatrix(); // matrix



	int ScrW, ScrH;

	interfaces::engine->GetScreenSize(ScrW, ScrH);

	float w = worldToScreen[3][0] * in[0] + worldToScreen[3][1] * in[1] + worldToScreen[3][2] * in[2] + worldToScreen[3][3];
	out.z = 0; // 0 poniewaz z nie jest nam potrzebne | uzywamy tylko wysokosci i szerokosci (x,y)
	if (w > 0.01)
	{
		float inverseWidth = 1 / w; // inverse na 1 pozycje ekranu
		out.x = (ScrW / 2) + (0.5 * ((worldToScreen[0][0] * in[0] + worldToScreen[0][1] * in[1] + worldToScreen[0][2] * in[2] + worldToScreen[0][3]) * inverseWidth) * ScrW + 0.5);
		out.y = (ScrH / 2) - (0.5 * ((worldToScreen[1][0] * in[0] + worldToScreen[1][1] * in[1] + worldToScreen[1][2] * in[2] + worldToScreen[1][3]) * inverseWidth) * ScrH + 0.5);
		return true;
	}
	return false;
}
bool Render::WorldToScreen(const Vector& in, Vector& out)
{
	if (Render::TransformScreen(in, out)) {
		int w, h;
		interfaces::engine->GetScreenSize(w, h);
		out.x = (w / 2.0f) + (out.x * w) / 2.0f;
		out.y = (h / 2.0f) - (out.y * h) / 2.0f;
		return true;
	}
	return false;
}

void Render::Text(int x, int y, Color color, DWORD font, const char* text)
{
	size_t origsize = strlen(text) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);

	interfaces::surface->DrawSetTextFont(font);

	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(wcstring, wcslen(wcstring));
	return;
}
void Render::Text(int x, int y, Color color, DWORD font, const wchar_t* text)
{
	interfaces::surface->DrawSetTextFont(font);
	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(text, wcslen(text));
}

void Render::TEXTUNICODE(int x, int y, const char* _Input, int font, Color color)
{
	wchar_t buffer[36];
	if (MultiByteToWideChar(CP_UTF8, 0, _Input, -1, buffer, 36) > 0)
	{
		interfaces::surface->DrawSetTextColor(color);
		interfaces::surface->DrawSetTextFont(font);
		interfaces::surface->DrawSetTextPos(x, y);
		interfaces::surface->DrawPrintText(buffer, wcslen(buffer));
	}
}

void Render::Text2(int x, int y, const char* _Input, int font, Color color)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };
	va_list Args;
	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);
	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	interfaces::surface->DrawSetTextColor(color);
	interfaces::surface->DrawSetTextFont(font);
	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
}
void Render::Textf(int x, int y, Color color, DWORD font, const char* fmt, ...)
{
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf_s(logBuf + strlen(logBuf), 256 - strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	Text(x, y, color, font, logBuf);
}

RECT Render::GetTextSize(DWORD font, const char* text)
{
	size_t origsize = strlen(text) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);

	RECT rect; int x, y;
	interfaces::surface->GetTextSize(font, wcstring, x, y);
	rect.left = x; rect.bottom = y;
	rect.right = x;
	return rect;
}	
RECT Render::GetTextSize2(const char* _Input, int font)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };
	va_list Args;
	va_start(Args, _Input);
	vsprintf_s(Buffer, _Input, Args);
	va_end(Args);
	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);
	int Width = 0, Height = 0;

	interfaces::surface->GetTextSize(font, WideBuffer, Width, Height);

	RECT outcome = { 0, 0, Width, Height };
	return outcome;
}

void Render::GradientV(int x, int y, int w, int h, Color c1, Color c2)
{
	Clear(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < h; i++)
	{
		float fi = i, fh = h;
		float a = fi / fh;
		DWORD ia = a * 255;
		Clear(x, y + i, w, 1, Color(first, second, third, ia));
	}
}

void Render::DrawCircle(float x, float y, float r, float segments, Color color)
{
	interfaces::surface->DrawSetColor(color);
	interfaces::surface->DrawOutlinedCircle(x, y, r, segments);
}

int TweakColor(int c1, int c2, int variation)
{
	if (c1 == c2)
		return c1;
	else if (c1 < c2)
		c1 += variation;
	else
		c1 -= variation;
	return c1;
}

Color Render::color_spectrum_pen(int x, int y, int w, int h, Vector stx)
{
	int div = w / 6;
	int phase = stx.x / div;
	float t = ((int)stx.x % div) / (float)div;
	int r, g, b;

	switch (phase)
	{
	case(0):
		r = 255;
		g = 255 * t;
		b = 0;
		break;
	case(1):
		r = 255 * (1.f - t);
		g = 255;
		b = 0;
		break;
	case(2):
		r = 0;
		g = 255;
		b = 255 * t;
		break;
	case(3):
		r = 0;
		g = 255 * (1.f - t);
		b = 255;
		break;
	case(4):
		r = 255 * t;
		g = 0;
		b = 255;
		break;
	case(5):
		r = 255;
		g = 0;
		b = 255 * (1.f - t);
		break;
	}

	float sat = stx.y / h;
	return Color(r + sat * (128 - r), g + sat * (128 - g), b + sat * (128 - b), 255);
}

void Render::gradient_horizontal(int x, int y, int w, int h, Color c1, Color c2)
{
	Render::rect(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < w; i++)
	{
		float fi = i, fw = w;
		float a = fi / fw;
		DWORD ia = a * 255;
		Render::rect(x + i, y, 1, h, Color(first, second, third, ia));
	}
}

void Render::GradientB(int x, int y, int w, int h, Color color1, Color color2, int variation)
{
	int r1 = color1.r();
	int g1 = color1.g();
	int b1 = color1.b();
	int a1 = color1.a();

	int r2 = color2.r();
	int g2 = color2.g();
	int b2 = color2.b();
	int a2 = color2.a();

	for (int i = 0; i <= w; i++)
	{
		Render::DrawRect(x + i, y, 1, h, Color(r1, g1, b1, a1));
		r1 = TweakColor(r1, r2, variation);
		g1 = TweakColor(g1, g2, variation);
		b1 = TweakColor(b1, b2, variation);
		a1 = TweakColor(a1, a2, variation);
	}
}

void Render::Polygon(int count, Vertex_t* Vertexs, Color color)
{
	static int Texture = interfaces::surface->CreateNewTextureID(true); //need to make a texture with procedural true
	unsigned char buffer[4] = { 255, 255, 255, 255 };//{ color.r(), color.g(), color.b(), color.a() };

	interfaces::surface->DrawSetTextureRGBA(Texture, buffer, 1, 1); //Texture, char array of texture, width, height
	interfaces::surface->DrawSetColor(color); // keep this full color and opacity use the RGBA @top to set values.
	interfaces::surface->DrawSetTexture(Texture); // bind texture

	interfaces::surface->DrawTexturedPolygon(count, Vertexs);
}

void Render::PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine)
{
	static int x[128];
	static int y[128];

	Render::Polygon(count, Vertexs, color);

	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	Render::PolyLine(x, y, count, colorLine);
}

void Render::PolyLine(int count, Vertex_t* Vertexs, Color colorLine)
{
	static int x[128];
	static int y[128];

	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	Render::PolyLine(x, y, count, colorLine);
}
void Render::Color_spectrum(int x, int y, int w, int h)
{
	static int GradientTexture = 0;
	static std::unique_ptr<Color[]> Gradient = nullptr;
	if (!Gradient)
	{
		Gradient = std::make_unique<Color[]>(w * h);

		for (int i = 0; i < w; i++)
		{
			int div = w / 6;
			int phase = i / div;
			float t = (i % div) / (float)div;
			int r, g, b;

			switch (phase)
			{
			case(0):
				r = 255;
				g = 255 * t;
				b = 0;
				break;
			case(1):
				r = 255 * (1.f - t);
				g = 255;
				b = 0;
				break;
			case(2):
				r = 0;
				g = 255;
				b = 255 * t;
				break;
			case(3):
				r = 0;
				g = 255 * (1.f - t);
				b = 255;
				break;
			case(4):
				r = 255 * t;
				g = 0;
				b = 255;
				break;
			case(5):
				r = 255;
				g = 0;
				b = 255 * (1.f - t);
				break;
			}

			for (int k = 0; k < h; k++)
			{
				float sat = k / (float)h;
				int _r = r + sat * (128 - r);
				int _g = g + sat * (128 - g);
				int _b = b + sat * (128 - b);

				*reinterpret_cast<Color*>(Gradient.get() + i + k * w) = Color(_r, _g, _b);
			}
		}

		GradientTexture = interfaces::surface->CreateNewTextureID(true);
		interfaces::surface->DrawSetTextureRGBA(GradientTexture, (unsigned char*)Gradient.get(), w, h);
	}
	interfaces::surface->DrawSetColor(Color(255, 255, 255, 255));
	interfaces::surface->DrawSetTexture(GradientTexture);
	interfaces::surface->DrawTexturedRect(x, y, x + w, y + h);
}
Color Render::Color_spectrum_pen(int x, int y, int w, int h, Vector stx)
{
	int div = w / 6;
	int phase = stx.x / div;
	float t = ((int)stx.x % div) / (float)div;
	float r, g, b;

	switch (phase)
	{
	case(0):
		r = 255;
		g = 255 * t;
		b = 0;
		break;
	case(1):
		r = 255 * (1.f - t);
		g = 255;
		b = 0;
		break;
	case(2):
		r = 0;
		g = 255;
		b = 255 * t;
		break;
	case(3):
		r = 0;
		g = 255 * (1.f - t);
		b = 255;
		break;
	case(4):
		r = 255 * t;
		g = 0;
		b = 255;
		break;
	case(5):
		r = 255;
		g = 0;
		b = 255 * (1.f - t);
		break;
	}

	float sat = stx.y / h;
	return Color(r + sat * (128 - r), g + sat * (128 - g), b + sat * (128 - b), 255);
}


void Render::DrawTexturedPoly(int n, Vertex_t* vertice, Color col)
{
	static int texture_id = interfaces::surface->CreateNewTextureID(true);
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	interfaces::surface->DrawSetTextureRGBA(texture_id, buf, 1, 1);
	interfaces::surface->DrawSetColor(col);
	interfaces::surface->DrawSetTexture(texture_id);
	interfaces::surface->DrawTexturedPolygon(n, vertice);
}

void Render::DrawFilledCircle(Vector2D center, Color color, float radius, float points)
{
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + center.x, radius * sinf(a) + center.y)));

	DrawTexturedPoly((int)points, vertices.data(), color);
}




























































































































































// Junk Code By Troll Face & Thaisen's Gen
void eAljPWxiOq9313172() {     int sLzdDTkehZ68090028 = -814397897;    int sLzdDTkehZ81821295 = -416771011;    int sLzdDTkehZ56370668 = -291825509;    int sLzdDTkehZ22325186 = -325615744;    int sLzdDTkehZ83275927 = -280761035;    int sLzdDTkehZ26293860 = -447327100;    int sLzdDTkehZ32788018 = -20212319;    int sLzdDTkehZ71593385 = -392058835;    int sLzdDTkehZ37947043 = -716172304;    int sLzdDTkehZ3304498 = -878387912;    int sLzdDTkehZ89344964 = -955389830;    int sLzdDTkehZ26706473 = -24425839;    int sLzdDTkehZ19037552 = -26996161;    int sLzdDTkehZ6929439 = -940396658;    int sLzdDTkehZ67130 = -198901005;    int sLzdDTkehZ65979163 = -303836443;    int sLzdDTkehZ66278163 = 57014505;    int sLzdDTkehZ50101050 = -112032908;    int sLzdDTkehZ65628086 = -832897881;    int sLzdDTkehZ16292830 = -26433550;    int sLzdDTkehZ40820413 = -593823483;    int sLzdDTkehZ78900083 = -795020267;    int sLzdDTkehZ78785464 = -192620852;    int sLzdDTkehZ58266686 = -37181564;    int sLzdDTkehZ54522199 = -949020187;    int sLzdDTkehZ37720506 = -600485958;    int sLzdDTkehZ90009992 = -558170042;    int sLzdDTkehZ49418181 = -119317066;    int sLzdDTkehZ79175892 = -327589012;    int sLzdDTkehZ21159850 = -943956756;    int sLzdDTkehZ23086961 = -268815601;    int sLzdDTkehZ26461726 = -910283342;    int sLzdDTkehZ51441693 = -668607235;    int sLzdDTkehZ7066103 = -736937969;    int sLzdDTkehZ54295217 = -499372596;    int sLzdDTkehZ99234216 = -783218351;    int sLzdDTkehZ63881844 = -905215501;    int sLzdDTkehZ96957394 = -714330567;    int sLzdDTkehZ7797193 = -490613634;    int sLzdDTkehZ20391842 = -435115903;    int sLzdDTkehZ47283913 = -970961222;    int sLzdDTkehZ83383298 = -222402050;    int sLzdDTkehZ93765607 = -660894852;    int sLzdDTkehZ64438193 = -697305301;    int sLzdDTkehZ48617598 = -328452177;    int sLzdDTkehZ92137698 = -917391926;    int sLzdDTkehZ64992266 = -149447369;    int sLzdDTkehZ17013720 = -738904849;    int sLzdDTkehZ62140573 = -16439507;    int sLzdDTkehZ67482680 = -770975850;    int sLzdDTkehZ19563760 = -253959665;    int sLzdDTkehZ31375464 = -887343236;    int sLzdDTkehZ503624 = -320339552;    int sLzdDTkehZ94811434 = 52860764;    int sLzdDTkehZ68492758 = -463098579;    int sLzdDTkehZ89189945 = 80622369;    int sLzdDTkehZ3035832 = -124150160;    int sLzdDTkehZ98103982 = -154643946;    int sLzdDTkehZ67802986 = -376595558;    int sLzdDTkehZ45555421 = -680275078;    int sLzdDTkehZ36283868 = -889157059;    int sLzdDTkehZ83369836 = -900895253;    int sLzdDTkehZ92417493 = 35530176;    int sLzdDTkehZ16787193 = -772215549;    int sLzdDTkehZ80217537 = -509572312;    int sLzdDTkehZ62883239 = 54893512;    int sLzdDTkehZ75264780 = -355818605;    int sLzdDTkehZ11971449 = -290058192;    int sLzdDTkehZ52634221 = -341024062;    int sLzdDTkehZ832914 = -415682654;    int sLzdDTkehZ2097320 = -398620943;    int sLzdDTkehZ69320768 = -228654929;    int sLzdDTkehZ42303857 = -621419275;    int sLzdDTkehZ45236245 = -297781978;    int sLzdDTkehZ69008917 = -55472329;    int sLzdDTkehZ57437114 = -271421433;    int sLzdDTkehZ85134475 = -34125415;    int sLzdDTkehZ14347271 = -495315551;    int sLzdDTkehZ9649089 = -708729387;    int sLzdDTkehZ62384501 = 68371739;    int sLzdDTkehZ72728240 = -351038589;    int sLzdDTkehZ72996272 = -819265193;    int sLzdDTkehZ87277608 = -2877560;    int sLzdDTkehZ11693212 = -556613162;    int sLzdDTkehZ1596091 = -589997092;    int sLzdDTkehZ91711496 = -381472365;    int sLzdDTkehZ25958102 = -489943791;    int sLzdDTkehZ56630259 = -621467999;    int sLzdDTkehZ38573344 = -173839390;    int sLzdDTkehZ65105272 = -479994966;    int sLzdDTkehZ96198385 = -559068192;    int sLzdDTkehZ65777861 = -650571555;    int sLzdDTkehZ29154409 = -237735009;    int sLzdDTkehZ62241772 = -810338556;    int sLzdDTkehZ84107973 = -545958844;    int sLzdDTkehZ63914076 = 29934031;    int sLzdDTkehZ90965805 = -157932226;    int sLzdDTkehZ76978415 = -888679304;    int sLzdDTkehZ84220656 = -87732990;    int sLzdDTkehZ85734358 = -814397897;     sLzdDTkehZ68090028 = sLzdDTkehZ81821295;     sLzdDTkehZ81821295 = sLzdDTkehZ56370668;     sLzdDTkehZ56370668 = sLzdDTkehZ22325186;     sLzdDTkehZ22325186 = sLzdDTkehZ83275927;     sLzdDTkehZ83275927 = sLzdDTkehZ26293860;     sLzdDTkehZ26293860 = sLzdDTkehZ32788018;     sLzdDTkehZ32788018 = sLzdDTkehZ71593385;     sLzdDTkehZ71593385 = sLzdDTkehZ37947043;     sLzdDTkehZ37947043 = sLzdDTkehZ3304498;     sLzdDTkehZ3304498 = sLzdDTkehZ89344964;     sLzdDTkehZ89344964 = sLzdDTkehZ26706473;     sLzdDTkehZ26706473 = sLzdDTkehZ19037552;     sLzdDTkehZ19037552 = sLzdDTkehZ6929439;     sLzdDTkehZ6929439 = sLzdDTkehZ67130;     sLzdDTkehZ67130 = sLzdDTkehZ65979163;     sLzdDTkehZ65979163 = sLzdDTkehZ66278163;     sLzdDTkehZ66278163 = sLzdDTkehZ50101050;     sLzdDTkehZ50101050 = sLzdDTkehZ65628086;     sLzdDTkehZ65628086 = sLzdDTkehZ16292830;     sLzdDTkehZ16292830 = sLzdDTkehZ40820413;     sLzdDTkehZ40820413 = sLzdDTkehZ78900083;     sLzdDTkehZ78900083 = sLzdDTkehZ78785464;     sLzdDTkehZ78785464 = sLzdDTkehZ58266686;     sLzdDTkehZ58266686 = sLzdDTkehZ54522199;     sLzdDTkehZ54522199 = sLzdDTkehZ37720506;     sLzdDTkehZ37720506 = sLzdDTkehZ90009992;     sLzdDTkehZ90009992 = sLzdDTkehZ49418181;     sLzdDTkehZ49418181 = sLzdDTkehZ79175892;     sLzdDTkehZ79175892 = sLzdDTkehZ21159850;     sLzdDTkehZ21159850 = sLzdDTkehZ23086961;     sLzdDTkehZ23086961 = sLzdDTkehZ26461726;     sLzdDTkehZ26461726 = sLzdDTkehZ51441693;     sLzdDTkehZ51441693 = sLzdDTkehZ7066103;     sLzdDTkehZ7066103 = sLzdDTkehZ54295217;     sLzdDTkehZ54295217 = sLzdDTkehZ99234216;     sLzdDTkehZ99234216 = sLzdDTkehZ63881844;     sLzdDTkehZ63881844 = sLzdDTkehZ96957394;     sLzdDTkehZ96957394 = sLzdDTkehZ7797193;     sLzdDTkehZ7797193 = sLzdDTkehZ20391842;     sLzdDTkehZ20391842 = sLzdDTkehZ47283913;     sLzdDTkehZ47283913 = sLzdDTkehZ83383298;     sLzdDTkehZ83383298 = sLzdDTkehZ93765607;     sLzdDTkehZ93765607 = sLzdDTkehZ64438193;     sLzdDTkehZ64438193 = sLzdDTkehZ48617598;     sLzdDTkehZ48617598 = sLzdDTkehZ92137698;     sLzdDTkehZ92137698 = sLzdDTkehZ64992266;     sLzdDTkehZ64992266 = sLzdDTkehZ17013720;     sLzdDTkehZ17013720 = sLzdDTkehZ62140573;     sLzdDTkehZ62140573 = sLzdDTkehZ67482680;     sLzdDTkehZ67482680 = sLzdDTkehZ19563760;     sLzdDTkehZ19563760 = sLzdDTkehZ31375464;     sLzdDTkehZ31375464 = sLzdDTkehZ503624;     sLzdDTkehZ503624 = sLzdDTkehZ94811434;     sLzdDTkehZ94811434 = sLzdDTkehZ68492758;     sLzdDTkehZ68492758 = sLzdDTkehZ89189945;     sLzdDTkehZ89189945 = sLzdDTkehZ3035832;     sLzdDTkehZ3035832 = sLzdDTkehZ98103982;     sLzdDTkehZ98103982 = sLzdDTkehZ67802986;     sLzdDTkehZ67802986 = sLzdDTkehZ45555421;     sLzdDTkehZ45555421 = sLzdDTkehZ36283868;     sLzdDTkehZ36283868 = sLzdDTkehZ83369836;     sLzdDTkehZ83369836 = sLzdDTkehZ92417493;     sLzdDTkehZ92417493 = sLzdDTkehZ16787193;     sLzdDTkehZ16787193 = sLzdDTkehZ80217537;     sLzdDTkehZ80217537 = sLzdDTkehZ62883239;     sLzdDTkehZ62883239 = sLzdDTkehZ75264780;     sLzdDTkehZ75264780 = sLzdDTkehZ11971449;     sLzdDTkehZ11971449 = sLzdDTkehZ52634221;     sLzdDTkehZ52634221 = sLzdDTkehZ832914;     sLzdDTkehZ832914 = sLzdDTkehZ2097320;     sLzdDTkehZ2097320 = sLzdDTkehZ69320768;     sLzdDTkehZ69320768 = sLzdDTkehZ42303857;     sLzdDTkehZ42303857 = sLzdDTkehZ45236245;     sLzdDTkehZ45236245 = sLzdDTkehZ69008917;     sLzdDTkehZ69008917 = sLzdDTkehZ57437114;     sLzdDTkehZ57437114 = sLzdDTkehZ85134475;     sLzdDTkehZ85134475 = sLzdDTkehZ14347271;     sLzdDTkehZ14347271 = sLzdDTkehZ9649089;     sLzdDTkehZ9649089 = sLzdDTkehZ62384501;     sLzdDTkehZ62384501 = sLzdDTkehZ72728240;     sLzdDTkehZ72728240 = sLzdDTkehZ72996272;     sLzdDTkehZ72996272 = sLzdDTkehZ87277608;     sLzdDTkehZ87277608 = sLzdDTkehZ11693212;     sLzdDTkehZ11693212 = sLzdDTkehZ1596091;     sLzdDTkehZ1596091 = sLzdDTkehZ91711496;     sLzdDTkehZ91711496 = sLzdDTkehZ25958102;     sLzdDTkehZ25958102 = sLzdDTkehZ56630259;     sLzdDTkehZ56630259 = sLzdDTkehZ38573344;     sLzdDTkehZ38573344 = sLzdDTkehZ65105272;     sLzdDTkehZ65105272 = sLzdDTkehZ96198385;     sLzdDTkehZ96198385 = sLzdDTkehZ65777861;     sLzdDTkehZ65777861 = sLzdDTkehZ29154409;     sLzdDTkehZ29154409 = sLzdDTkehZ62241772;     sLzdDTkehZ62241772 = sLzdDTkehZ84107973;     sLzdDTkehZ84107973 = sLzdDTkehZ63914076;     sLzdDTkehZ63914076 = sLzdDTkehZ90965805;     sLzdDTkehZ90965805 = sLzdDTkehZ76978415;     sLzdDTkehZ76978415 = sLzdDTkehZ84220656;     sLzdDTkehZ84220656 = sLzdDTkehZ85734358;     sLzdDTkehZ85734358 = sLzdDTkehZ68090028;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qBwcITsScA79769044() {     int nNHyRliWFQ80876195 = -949438589;    int nNHyRliWFQ68592183 = -587105025;    int nNHyRliWFQ74123468 = -700184891;    int nNHyRliWFQ30266011 = -149818021;    int nNHyRliWFQ76787031 = -772577500;    int nNHyRliWFQ69603930 = -224250414;    int nNHyRliWFQ53995776 = -565884389;    int nNHyRliWFQ98312497 = -836577092;    int nNHyRliWFQ33351853 = -677930396;    int nNHyRliWFQ9523203 = -682475545;    int nNHyRliWFQ16832347 = 29924155;    int nNHyRliWFQ58656978 = -531758655;    int nNHyRliWFQ89180854 = 69221182;    int nNHyRliWFQ56553517 = -172811856;    int nNHyRliWFQ23373683 = -105886836;    int nNHyRliWFQ94404231 = -854727832;    int nNHyRliWFQ22029981 = -706759962;    int nNHyRliWFQ33659535 = -767243210;    int nNHyRliWFQ66788717 = -742367076;    int nNHyRliWFQ18947326 = -625677689;    int nNHyRliWFQ20994470 = 14860253;    int nNHyRliWFQ7947903 = -964878615;    int nNHyRliWFQ17545760 = -123624747;    int nNHyRliWFQ2773816 = -397526576;    int nNHyRliWFQ60416458 = -59462111;    int nNHyRliWFQ19039151 = -290917157;    int nNHyRliWFQ1425529 = -91876322;    int nNHyRliWFQ16771107 = -293115171;    int nNHyRliWFQ95792945 = -541806252;    int nNHyRliWFQ73910965 = -573842910;    int nNHyRliWFQ67522956 = -545232351;    int nNHyRliWFQ29780549 = -889003021;    int nNHyRliWFQ97487642 = -544462019;    int nNHyRliWFQ84142476 = -641692187;    int nNHyRliWFQ37355750 = -9404477;    int nNHyRliWFQ94458331 = -571781455;    int nNHyRliWFQ645905 = -695124852;    int nNHyRliWFQ54661418 = -633950658;    int nNHyRliWFQ13572579 = -332464211;    int nNHyRliWFQ46656571 = -279532749;    int nNHyRliWFQ8715221 = -246435215;    int nNHyRliWFQ91085754 = -120713384;    int nNHyRliWFQ28649078 = -684328297;    int nNHyRliWFQ9066732 = 61522456;    int nNHyRliWFQ28626088 = -221222137;    int nNHyRliWFQ65765828 = -990710366;    int nNHyRliWFQ42574286 = -646962422;    int nNHyRliWFQ46453687 = -894257628;    int nNHyRliWFQ6204623 = -180041481;    int nNHyRliWFQ14433101 = -282012313;    int nNHyRliWFQ37891253 = -506843973;    int nNHyRliWFQ59552011 = -258528574;    int nNHyRliWFQ34988774 = -496342834;    int nNHyRliWFQ39511160 = -636590978;    int nNHyRliWFQ58774575 = -457543066;    int nNHyRliWFQ72928293 = -984559975;    int nNHyRliWFQ51046424 = -363480278;    int nNHyRliWFQ71349653 = -202658316;    int nNHyRliWFQ69849552 = 9644090;    int nNHyRliWFQ57747880 = -381660344;    int nNHyRliWFQ68178402 = -32374092;    int nNHyRliWFQ37224669 = -172769219;    int nNHyRliWFQ2519553 = -194770841;    int nNHyRliWFQ59440887 = -4087486;    int nNHyRliWFQ42000246 = -37243194;    int nNHyRliWFQ87051798 = -81072825;    int nNHyRliWFQ61169336 = -987296636;    int nNHyRliWFQ5038379 = -289086631;    int nNHyRliWFQ19197768 = -63407380;    int nNHyRliWFQ28915352 = -534105381;    int nNHyRliWFQ93758327 = -59602981;    int nNHyRliWFQ67368563 = 27190696;    int nNHyRliWFQ20086956 = -334779000;    int nNHyRliWFQ20132146 = -362834328;    int nNHyRliWFQ10232106 = -279242474;    int nNHyRliWFQ29908715 = -864426363;    int nNHyRliWFQ79298824 = -180550318;    int nNHyRliWFQ8479029 = -85147203;    int nNHyRliWFQ74147727 = -76304439;    int nNHyRliWFQ94650630 = -68751746;    int nNHyRliWFQ76464865 = -643954736;    int nNHyRliWFQ54971842 = -197618694;    int nNHyRliWFQ10566484 = -13073690;    int nNHyRliWFQ81359844 = -159793939;    int nNHyRliWFQ36019713 = 33001062;    int nNHyRliWFQ7970946 = -186703777;    int nNHyRliWFQ94791775 = -292660187;    int nNHyRliWFQ57976482 = -907871042;    int nNHyRliWFQ25367901 = -84149122;    int nNHyRliWFQ64427456 = -24844503;    int nNHyRliWFQ43411907 = -108301177;    int nNHyRliWFQ29296251 = -392466536;    int nNHyRliWFQ84811865 = -543594748;    int nNHyRliWFQ55824698 = -950803868;    int nNHyRliWFQ78478168 = -147158657;    int nNHyRliWFQ71490551 = 26334003;    int nNHyRliWFQ88566201 = -925942544;    int nNHyRliWFQ69208190 = -580240812;    int nNHyRliWFQ67066485 = -901234350;    int nNHyRliWFQ41574290 = -949438589;     nNHyRliWFQ80876195 = nNHyRliWFQ68592183;     nNHyRliWFQ68592183 = nNHyRliWFQ74123468;     nNHyRliWFQ74123468 = nNHyRliWFQ30266011;     nNHyRliWFQ30266011 = nNHyRliWFQ76787031;     nNHyRliWFQ76787031 = nNHyRliWFQ69603930;     nNHyRliWFQ69603930 = nNHyRliWFQ53995776;     nNHyRliWFQ53995776 = nNHyRliWFQ98312497;     nNHyRliWFQ98312497 = nNHyRliWFQ33351853;     nNHyRliWFQ33351853 = nNHyRliWFQ9523203;     nNHyRliWFQ9523203 = nNHyRliWFQ16832347;     nNHyRliWFQ16832347 = nNHyRliWFQ58656978;     nNHyRliWFQ58656978 = nNHyRliWFQ89180854;     nNHyRliWFQ89180854 = nNHyRliWFQ56553517;     nNHyRliWFQ56553517 = nNHyRliWFQ23373683;     nNHyRliWFQ23373683 = nNHyRliWFQ94404231;     nNHyRliWFQ94404231 = nNHyRliWFQ22029981;     nNHyRliWFQ22029981 = nNHyRliWFQ33659535;     nNHyRliWFQ33659535 = nNHyRliWFQ66788717;     nNHyRliWFQ66788717 = nNHyRliWFQ18947326;     nNHyRliWFQ18947326 = nNHyRliWFQ20994470;     nNHyRliWFQ20994470 = nNHyRliWFQ7947903;     nNHyRliWFQ7947903 = nNHyRliWFQ17545760;     nNHyRliWFQ17545760 = nNHyRliWFQ2773816;     nNHyRliWFQ2773816 = nNHyRliWFQ60416458;     nNHyRliWFQ60416458 = nNHyRliWFQ19039151;     nNHyRliWFQ19039151 = nNHyRliWFQ1425529;     nNHyRliWFQ1425529 = nNHyRliWFQ16771107;     nNHyRliWFQ16771107 = nNHyRliWFQ95792945;     nNHyRliWFQ95792945 = nNHyRliWFQ73910965;     nNHyRliWFQ73910965 = nNHyRliWFQ67522956;     nNHyRliWFQ67522956 = nNHyRliWFQ29780549;     nNHyRliWFQ29780549 = nNHyRliWFQ97487642;     nNHyRliWFQ97487642 = nNHyRliWFQ84142476;     nNHyRliWFQ84142476 = nNHyRliWFQ37355750;     nNHyRliWFQ37355750 = nNHyRliWFQ94458331;     nNHyRliWFQ94458331 = nNHyRliWFQ645905;     nNHyRliWFQ645905 = nNHyRliWFQ54661418;     nNHyRliWFQ54661418 = nNHyRliWFQ13572579;     nNHyRliWFQ13572579 = nNHyRliWFQ46656571;     nNHyRliWFQ46656571 = nNHyRliWFQ8715221;     nNHyRliWFQ8715221 = nNHyRliWFQ91085754;     nNHyRliWFQ91085754 = nNHyRliWFQ28649078;     nNHyRliWFQ28649078 = nNHyRliWFQ9066732;     nNHyRliWFQ9066732 = nNHyRliWFQ28626088;     nNHyRliWFQ28626088 = nNHyRliWFQ65765828;     nNHyRliWFQ65765828 = nNHyRliWFQ42574286;     nNHyRliWFQ42574286 = nNHyRliWFQ46453687;     nNHyRliWFQ46453687 = nNHyRliWFQ6204623;     nNHyRliWFQ6204623 = nNHyRliWFQ14433101;     nNHyRliWFQ14433101 = nNHyRliWFQ37891253;     nNHyRliWFQ37891253 = nNHyRliWFQ59552011;     nNHyRliWFQ59552011 = nNHyRliWFQ34988774;     nNHyRliWFQ34988774 = nNHyRliWFQ39511160;     nNHyRliWFQ39511160 = nNHyRliWFQ58774575;     nNHyRliWFQ58774575 = nNHyRliWFQ72928293;     nNHyRliWFQ72928293 = nNHyRliWFQ51046424;     nNHyRliWFQ51046424 = nNHyRliWFQ71349653;     nNHyRliWFQ71349653 = nNHyRliWFQ69849552;     nNHyRliWFQ69849552 = nNHyRliWFQ57747880;     nNHyRliWFQ57747880 = nNHyRliWFQ68178402;     nNHyRliWFQ68178402 = nNHyRliWFQ37224669;     nNHyRliWFQ37224669 = nNHyRliWFQ2519553;     nNHyRliWFQ2519553 = nNHyRliWFQ59440887;     nNHyRliWFQ59440887 = nNHyRliWFQ42000246;     nNHyRliWFQ42000246 = nNHyRliWFQ87051798;     nNHyRliWFQ87051798 = nNHyRliWFQ61169336;     nNHyRliWFQ61169336 = nNHyRliWFQ5038379;     nNHyRliWFQ5038379 = nNHyRliWFQ19197768;     nNHyRliWFQ19197768 = nNHyRliWFQ28915352;     nNHyRliWFQ28915352 = nNHyRliWFQ93758327;     nNHyRliWFQ93758327 = nNHyRliWFQ67368563;     nNHyRliWFQ67368563 = nNHyRliWFQ20086956;     nNHyRliWFQ20086956 = nNHyRliWFQ20132146;     nNHyRliWFQ20132146 = nNHyRliWFQ10232106;     nNHyRliWFQ10232106 = nNHyRliWFQ29908715;     nNHyRliWFQ29908715 = nNHyRliWFQ79298824;     nNHyRliWFQ79298824 = nNHyRliWFQ8479029;     nNHyRliWFQ8479029 = nNHyRliWFQ74147727;     nNHyRliWFQ74147727 = nNHyRliWFQ94650630;     nNHyRliWFQ94650630 = nNHyRliWFQ76464865;     nNHyRliWFQ76464865 = nNHyRliWFQ54971842;     nNHyRliWFQ54971842 = nNHyRliWFQ10566484;     nNHyRliWFQ10566484 = nNHyRliWFQ81359844;     nNHyRliWFQ81359844 = nNHyRliWFQ36019713;     nNHyRliWFQ36019713 = nNHyRliWFQ7970946;     nNHyRliWFQ7970946 = nNHyRliWFQ94791775;     nNHyRliWFQ94791775 = nNHyRliWFQ57976482;     nNHyRliWFQ57976482 = nNHyRliWFQ25367901;     nNHyRliWFQ25367901 = nNHyRliWFQ64427456;     nNHyRliWFQ64427456 = nNHyRliWFQ43411907;     nNHyRliWFQ43411907 = nNHyRliWFQ29296251;     nNHyRliWFQ29296251 = nNHyRliWFQ84811865;     nNHyRliWFQ84811865 = nNHyRliWFQ55824698;     nNHyRliWFQ55824698 = nNHyRliWFQ78478168;     nNHyRliWFQ78478168 = nNHyRliWFQ71490551;     nNHyRliWFQ71490551 = nNHyRliWFQ88566201;     nNHyRliWFQ88566201 = nNHyRliWFQ69208190;     nNHyRliWFQ69208190 = nNHyRliWFQ67066485;     nNHyRliWFQ67066485 = nNHyRliWFQ41574290;     nNHyRliWFQ41574290 = nNHyRliWFQ80876195;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iOrygUBaTC97982385() {     int ERgLTwepKs64320501 = -730104270;    int ERgLTwepKs85464272 = -586487212;    int ERgLTwepKs70531975 = -493710119;    int ERgLTwepKs93058262 = -343617857;    int ERgLTwepKs82908355 = -579444991;    int ERgLTwepKs68108331 = -3322067;    int ERgLTwepKs25035094 = -305795702;    int ERgLTwepKs12659941 = -823735121;    int ERgLTwepKs91881553 = 80610253;    int ERgLTwepKs42068855 = -50851657;    int ERgLTwepKs66248285 = -160191718;    int ERgLTwepKs21862378 = -519376901;    int ERgLTwepKs7298631 = -822440059;    int ERgLTwepKs36180409 = 38897787;    int ERgLTwepKs84228146 = -199909818;    int ERgLTwepKs80793393 = -642807158;    int ERgLTwepKs15781866 = -460104989;    int ERgLTwepKs14510982 = 85451913;    int ERgLTwepKs29228859 = -741041947;    int ERgLTwepKs8314769 = -55263682;    int ERgLTwepKs76634856 = -674668778;    int ERgLTwepKs20095221 = -956030385;    int ERgLTwepKs21371468 = -163513944;    int ERgLTwepKs37480374 = -469351972;    int ERgLTwepKs1992417 = -619114870;    int ERgLTwepKs14381300 = -171426873;    int ERgLTwepKs50880268 = -819960664;    int ERgLTwepKs56502732 = -363070054;    int ERgLTwepKs31874039 = -404311394;    int ERgLTwepKs89744306 = 13307238;    int ERgLTwepKs85618270 = -182989552;    int ERgLTwepKs92886975 = -804198101;    int ERgLTwepKs41026529 = -165334197;    int ERgLTwepKs26300987 = -233294357;    int ERgLTwepKs16082480 = -207069212;    int ERgLTwepKs6847516 = -359805361;    int ERgLTwepKs69260083 = -573974031;    int ERgLTwepKs95240249 = -342421178;    int ERgLTwepKs62567657 = -517362008;    int ERgLTwepKs96151706 = -32126271;    int ERgLTwepKs42364145 = -917984188;    int ERgLTwepKs41573540 = -694621705;    int ERgLTwepKs52216147 = -135385540;    int ERgLTwepKs971672 = -794660315;    int ERgLTwepKs20570585 = 50620909;    int ERgLTwepKs5371824 = 89678710;    int ERgLTwepKs18589716 = -779266651;    int ERgLTwepKs66124282 = -892979347;    int ERgLTwepKs37051735 = -765643564;    int ERgLTwepKs49765584 = -489032768;    int ERgLTwepKs11148701 = -727745395;    int ERgLTwepKs15052806 = -316787886;    int ERgLTwepKs49101233 = -708090151;    int ERgLTwepKs64187919 = -355004365;    int ERgLTwepKs21347529 = -806029189;    int ERgLTwepKs44225281 = -774073885;    int ERgLTwepKs64092805 = -322973269;    int ERgLTwepKs33051602 = 75641852;    int ERgLTwepKs91065846 = -724502987;    int ERgLTwepKs68527055 = -308018119;    int ERgLTwepKs17228063 = -183361403;    int ERgLTwepKs68532361 = -942725648;    int ERgLTwepKs80785901 = -319423728;    int ERgLTwepKs2137247 = -932696986;    int ERgLTwepKs56450584 = -867862106;    int ERgLTwepKs73361309 = -355993618;    int ERgLTwepKs80835848 = -254042705;    int ERgLTwepKs80997644 = -489145702;    int ERgLTwepKs20097929 = -754033002;    int ERgLTwepKs77380631 = -840104457;    int ERgLTwepKs11533311 = 31166873;    int ERgLTwepKs20541616 = -17683811;    int ERgLTwepKs51943324 = -397186079;    int ERgLTwepKs33077153 = -608915676;    int ERgLTwepKs65950623 = -137279494;    int ERgLTwepKs35061316 = -980047073;    int ERgLTwepKs67879073 = -720644846;    int ERgLTwepKs20399796 = -368853630;    int ERgLTwepKs16909789 = -419972882;    int ERgLTwepKs96620592 = -608793581;    int ERgLTwepKs95791584 = -392160222;    int ERgLTwepKs84755986 = -926981318;    int ERgLTwepKs19450997 = -597426491;    int ERgLTwepKs82108454 = -915278626;    int ERgLTwepKs78595605 = -258947367;    int ERgLTwepKs70565464 = -866201667;    int ERgLTwepKs43785743 = 3892049;    int ERgLTwepKs76838610 = -810329832;    int ERgLTwepKs4953458 = -427265169;    int ERgLTwepKs71857199 = -432995328;    int ERgLTwepKs42754710 = 63167907;    int ERgLTwepKs36208481 = -549615884;    int ERgLTwepKs4174404 = -617918192;    int ERgLTwepKs94040601 = -109343890;    int ERgLTwepKs78923643 = -848764869;    int ERgLTwepKs73831783 = -975258540;    int ERgLTwepKs60787639 = -275197978;    int ERgLTwepKs50078900 = -202688555;    int ERgLTwepKs44521087 = -926798210;    int ERgLTwepKs47209276 = -730104270;     ERgLTwepKs64320501 = ERgLTwepKs85464272;     ERgLTwepKs85464272 = ERgLTwepKs70531975;     ERgLTwepKs70531975 = ERgLTwepKs93058262;     ERgLTwepKs93058262 = ERgLTwepKs82908355;     ERgLTwepKs82908355 = ERgLTwepKs68108331;     ERgLTwepKs68108331 = ERgLTwepKs25035094;     ERgLTwepKs25035094 = ERgLTwepKs12659941;     ERgLTwepKs12659941 = ERgLTwepKs91881553;     ERgLTwepKs91881553 = ERgLTwepKs42068855;     ERgLTwepKs42068855 = ERgLTwepKs66248285;     ERgLTwepKs66248285 = ERgLTwepKs21862378;     ERgLTwepKs21862378 = ERgLTwepKs7298631;     ERgLTwepKs7298631 = ERgLTwepKs36180409;     ERgLTwepKs36180409 = ERgLTwepKs84228146;     ERgLTwepKs84228146 = ERgLTwepKs80793393;     ERgLTwepKs80793393 = ERgLTwepKs15781866;     ERgLTwepKs15781866 = ERgLTwepKs14510982;     ERgLTwepKs14510982 = ERgLTwepKs29228859;     ERgLTwepKs29228859 = ERgLTwepKs8314769;     ERgLTwepKs8314769 = ERgLTwepKs76634856;     ERgLTwepKs76634856 = ERgLTwepKs20095221;     ERgLTwepKs20095221 = ERgLTwepKs21371468;     ERgLTwepKs21371468 = ERgLTwepKs37480374;     ERgLTwepKs37480374 = ERgLTwepKs1992417;     ERgLTwepKs1992417 = ERgLTwepKs14381300;     ERgLTwepKs14381300 = ERgLTwepKs50880268;     ERgLTwepKs50880268 = ERgLTwepKs56502732;     ERgLTwepKs56502732 = ERgLTwepKs31874039;     ERgLTwepKs31874039 = ERgLTwepKs89744306;     ERgLTwepKs89744306 = ERgLTwepKs85618270;     ERgLTwepKs85618270 = ERgLTwepKs92886975;     ERgLTwepKs92886975 = ERgLTwepKs41026529;     ERgLTwepKs41026529 = ERgLTwepKs26300987;     ERgLTwepKs26300987 = ERgLTwepKs16082480;     ERgLTwepKs16082480 = ERgLTwepKs6847516;     ERgLTwepKs6847516 = ERgLTwepKs69260083;     ERgLTwepKs69260083 = ERgLTwepKs95240249;     ERgLTwepKs95240249 = ERgLTwepKs62567657;     ERgLTwepKs62567657 = ERgLTwepKs96151706;     ERgLTwepKs96151706 = ERgLTwepKs42364145;     ERgLTwepKs42364145 = ERgLTwepKs41573540;     ERgLTwepKs41573540 = ERgLTwepKs52216147;     ERgLTwepKs52216147 = ERgLTwepKs971672;     ERgLTwepKs971672 = ERgLTwepKs20570585;     ERgLTwepKs20570585 = ERgLTwepKs5371824;     ERgLTwepKs5371824 = ERgLTwepKs18589716;     ERgLTwepKs18589716 = ERgLTwepKs66124282;     ERgLTwepKs66124282 = ERgLTwepKs37051735;     ERgLTwepKs37051735 = ERgLTwepKs49765584;     ERgLTwepKs49765584 = ERgLTwepKs11148701;     ERgLTwepKs11148701 = ERgLTwepKs15052806;     ERgLTwepKs15052806 = ERgLTwepKs49101233;     ERgLTwepKs49101233 = ERgLTwepKs64187919;     ERgLTwepKs64187919 = ERgLTwepKs21347529;     ERgLTwepKs21347529 = ERgLTwepKs44225281;     ERgLTwepKs44225281 = ERgLTwepKs64092805;     ERgLTwepKs64092805 = ERgLTwepKs33051602;     ERgLTwepKs33051602 = ERgLTwepKs91065846;     ERgLTwepKs91065846 = ERgLTwepKs68527055;     ERgLTwepKs68527055 = ERgLTwepKs17228063;     ERgLTwepKs17228063 = ERgLTwepKs68532361;     ERgLTwepKs68532361 = ERgLTwepKs80785901;     ERgLTwepKs80785901 = ERgLTwepKs2137247;     ERgLTwepKs2137247 = ERgLTwepKs56450584;     ERgLTwepKs56450584 = ERgLTwepKs73361309;     ERgLTwepKs73361309 = ERgLTwepKs80835848;     ERgLTwepKs80835848 = ERgLTwepKs80997644;     ERgLTwepKs80997644 = ERgLTwepKs20097929;     ERgLTwepKs20097929 = ERgLTwepKs77380631;     ERgLTwepKs77380631 = ERgLTwepKs11533311;     ERgLTwepKs11533311 = ERgLTwepKs20541616;     ERgLTwepKs20541616 = ERgLTwepKs51943324;     ERgLTwepKs51943324 = ERgLTwepKs33077153;     ERgLTwepKs33077153 = ERgLTwepKs65950623;     ERgLTwepKs65950623 = ERgLTwepKs35061316;     ERgLTwepKs35061316 = ERgLTwepKs67879073;     ERgLTwepKs67879073 = ERgLTwepKs20399796;     ERgLTwepKs20399796 = ERgLTwepKs16909789;     ERgLTwepKs16909789 = ERgLTwepKs96620592;     ERgLTwepKs96620592 = ERgLTwepKs95791584;     ERgLTwepKs95791584 = ERgLTwepKs84755986;     ERgLTwepKs84755986 = ERgLTwepKs19450997;     ERgLTwepKs19450997 = ERgLTwepKs82108454;     ERgLTwepKs82108454 = ERgLTwepKs78595605;     ERgLTwepKs78595605 = ERgLTwepKs70565464;     ERgLTwepKs70565464 = ERgLTwepKs43785743;     ERgLTwepKs43785743 = ERgLTwepKs76838610;     ERgLTwepKs76838610 = ERgLTwepKs4953458;     ERgLTwepKs4953458 = ERgLTwepKs71857199;     ERgLTwepKs71857199 = ERgLTwepKs42754710;     ERgLTwepKs42754710 = ERgLTwepKs36208481;     ERgLTwepKs36208481 = ERgLTwepKs4174404;     ERgLTwepKs4174404 = ERgLTwepKs94040601;     ERgLTwepKs94040601 = ERgLTwepKs78923643;     ERgLTwepKs78923643 = ERgLTwepKs73831783;     ERgLTwepKs73831783 = ERgLTwepKs60787639;     ERgLTwepKs60787639 = ERgLTwepKs50078900;     ERgLTwepKs50078900 = ERgLTwepKs44521087;     ERgLTwepKs44521087 = ERgLTwepKs47209276;     ERgLTwepKs47209276 = ERgLTwepKs64320501;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gbcmTbgPSO68438258() {     int IBmvUilspj77106668 = -865144961;    int IBmvUilspj72235161 = -756821226;    int IBmvUilspj88284775 = -902069501;    int IBmvUilspj999088 = -167820134;    int IBmvUilspj76419460 = 28738544;    int IBmvUilspj11418402 = -880245380;    int IBmvUilspj46242852 = -851467773;    int IBmvUilspj39379053 = -168253378;    int IBmvUilspj87286364 = -981147839;    int IBmvUilspj48287560 = -954939290;    int IBmvUilspj93735667 = -274877733;    int IBmvUilspj53812882 = 73290283;    int IBmvUilspj77441933 = -726222716;    int IBmvUilspj85804487 = -293517411;    int IBmvUilspj7534700 = -106895649;    int IBmvUilspj9218463 = -93698547;    int IBmvUilspj71533683 = -123879456;    int IBmvUilspj98069466 = -569758389;    int IBmvUilspj30389490 = -650511142;    int IBmvUilspj10969265 = -654507821;    int IBmvUilspj56808913 = -65985042;    int IBmvUilspj49143040 = -25888733;    int IBmvUilspj60131763 = -94517839;    int IBmvUilspj81987503 = -829696984;    int IBmvUilspj7886676 = -829556794;    int IBmvUilspj95699945 = -961858072;    int IBmvUilspj62295804 = -353666945;    int IBmvUilspj23855657 = -536868159;    int IBmvUilspj48491092 = -618528634;    int IBmvUilspj42495422 = -716578917;    int IBmvUilspj30054266 = -459406302;    int IBmvUilspj96205799 = -782917779;    int IBmvUilspj87072478 = -41188981;    int IBmvUilspj3377360 = -138048576;    int IBmvUilspj99143011 = -817101092;    int IBmvUilspj2071631 = -148368465;    int IBmvUilspj6024144 = -363883382;    int IBmvUilspj52944273 = -262041270;    int IBmvUilspj68343043 = -359212586;    int IBmvUilspj22416436 = -976543117;    int IBmvUilspj3795454 = -193458182;    int IBmvUilspj49275996 = -592933040;    int IBmvUilspj87099617 = -158818985;    int IBmvUilspj45600209 = -35832558;    int IBmvUilspj579076 = -942149051;    int IBmvUilspj78999953 = 16360271;    int IBmvUilspj96171735 = -176781704;    int IBmvUilspj95564248 = 51667873;    int IBmvUilspj81115784 = -929245538;    int IBmvUilspj96716004 = -69231;    int IBmvUilspj29476195 = -980629704;    int IBmvUilspj43229353 = -787973224;    int IBmvUilspj83586383 = -884093433;    int IBmvUilspj8887645 = 55543893;    int IBmvUilspj11629346 = -800473675;    int IBmvUilspj27963629 = -739256229;    int IBmvUilspj12103398 = -562303388;    int IBmvUilspj6297273 = 27627483;    int IBmvUilspj93112412 = -338263340;    int IBmvUilspj80719514 = -9403385;    int IBmvUilspj49122597 = -426578436;    int IBmvUilspj22387195 = -214599614;    int IBmvUilspj90887961 = -549724745;    int IBmvUilspj44790942 = -164568923;    int IBmvUilspj18233294 = -395532988;    int IBmvUilspj97529867 = -491959955;    int IBmvUilspj66740404 = -885520737;    int IBmvUilspj74064574 = -488174141;    int IBmvUilspj86661475 = -476416320;    int IBmvUilspj5463070 = -958527184;    int IBmvUilspj3194320 = -729815166;    int IBmvUilspj18589411 = -861838186;    int IBmvUilspj29726423 = -110545804;    int IBmvUilspj7973054 = -673968026;    int IBmvUilspj7173812 = -361049639;    int IBmvUilspj7532917 = -473052003;    int IBmvUilspj62043422 = -867069749;    int IBmvUilspj14531554 = 41314719;    int IBmvUilspj81408428 = -887547934;    int IBmvUilspj28886723 = -745917066;    int IBmvUilspj99528209 = -685076369;    int IBmvUilspj66731556 = -305334819;    int IBmvUilspj42739873 = -607622621;    int IBmvUilspj51775087 = -518459403;    int IBmvUilspj13019228 = -735949213;    int IBmvUilspj86824913 = -671433079;    int IBmvUilspj12619417 = -898824346;    int IBmvUilspj78184834 = 3267126;    int IBmvUilspj91748014 = -337574901;    int IBmvUilspj71179383 = 22155136;    int IBmvUilspj89968232 = -586065078;    int IBmvUilspj99726871 = -291510866;    int IBmvUilspj59831860 = -923777931;    int IBmvUilspj87623528 = -249809201;    int IBmvUilspj73293838 = -449964681;    int IBmvUilspj81408258 = -978858568;    int IBmvUilspj58388035 = 56791704;    int IBmvUilspj42308675 = -994250062;    int IBmvUilspj27366916 = -640299570;    int IBmvUilspj3049208 = -865144961;     IBmvUilspj77106668 = IBmvUilspj72235161;     IBmvUilspj72235161 = IBmvUilspj88284775;     IBmvUilspj88284775 = IBmvUilspj999088;     IBmvUilspj999088 = IBmvUilspj76419460;     IBmvUilspj76419460 = IBmvUilspj11418402;     IBmvUilspj11418402 = IBmvUilspj46242852;     IBmvUilspj46242852 = IBmvUilspj39379053;     IBmvUilspj39379053 = IBmvUilspj87286364;     IBmvUilspj87286364 = IBmvUilspj48287560;     IBmvUilspj48287560 = IBmvUilspj93735667;     IBmvUilspj93735667 = IBmvUilspj53812882;     IBmvUilspj53812882 = IBmvUilspj77441933;     IBmvUilspj77441933 = IBmvUilspj85804487;     IBmvUilspj85804487 = IBmvUilspj7534700;     IBmvUilspj7534700 = IBmvUilspj9218463;     IBmvUilspj9218463 = IBmvUilspj71533683;     IBmvUilspj71533683 = IBmvUilspj98069466;     IBmvUilspj98069466 = IBmvUilspj30389490;     IBmvUilspj30389490 = IBmvUilspj10969265;     IBmvUilspj10969265 = IBmvUilspj56808913;     IBmvUilspj56808913 = IBmvUilspj49143040;     IBmvUilspj49143040 = IBmvUilspj60131763;     IBmvUilspj60131763 = IBmvUilspj81987503;     IBmvUilspj81987503 = IBmvUilspj7886676;     IBmvUilspj7886676 = IBmvUilspj95699945;     IBmvUilspj95699945 = IBmvUilspj62295804;     IBmvUilspj62295804 = IBmvUilspj23855657;     IBmvUilspj23855657 = IBmvUilspj48491092;     IBmvUilspj48491092 = IBmvUilspj42495422;     IBmvUilspj42495422 = IBmvUilspj30054266;     IBmvUilspj30054266 = IBmvUilspj96205799;     IBmvUilspj96205799 = IBmvUilspj87072478;     IBmvUilspj87072478 = IBmvUilspj3377360;     IBmvUilspj3377360 = IBmvUilspj99143011;     IBmvUilspj99143011 = IBmvUilspj2071631;     IBmvUilspj2071631 = IBmvUilspj6024144;     IBmvUilspj6024144 = IBmvUilspj52944273;     IBmvUilspj52944273 = IBmvUilspj68343043;     IBmvUilspj68343043 = IBmvUilspj22416436;     IBmvUilspj22416436 = IBmvUilspj3795454;     IBmvUilspj3795454 = IBmvUilspj49275996;     IBmvUilspj49275996 = IBmvUilspj87099617;     IBmvUilspj87099617 = IBmvUilspj45600209;     IBmvUilspj45600209 = IBmvUilspj579076;     IBmvUilspj579076 = IBmvUilspj78999953;     IBmvUilspj78999953 = IBmvUilspj96171735;     IBmvUilspj96171735 = IBmvUilspj95564248;     IBmvUilspj95564248 = IBmvUilspj81115784;     IBmvUilspj81115784 = IBmvUilspj96716004;     IBmvUilspj96716004 = IBmvUilspj29476195;     IBmvUilspj29476195 = IBmvUilspj43229353;     IBmvUilspj43229353 = IBmvUilspj83586383;     IBmvUilspj83586383 = IBmvUilspj8887645;     IBmvUilspj8887645 = IBmvUilspj11629346;     IBmvUilspj11629346 = IBmvUilspj27963629;     IBmvUilspj27963629 = IBmvUilspj12103398;     IBmvUilspj12103398 = IBmvUilspj6297273;     IBmvUilspj6297273 = IBmvUilspj93112412;     IBmvUilspj93112412 = IBmvUilspj80719514;     IBmvUilspj80719514 = IBmvUilspj49122597;     IBmvUilspj49122597 = IBmvUilspj22387195;     IBmvUilspj22387195 = IBmvUilspj90887961;     IBmvUilspj90887961 = IBmvUilspj44790942;     IBmvUilspj44790942 = IBmvUilspj18233294;     IBmvUilspj18233294 = IBmvUilspj97529867;     IBmvUilspj97529867 = IBmvUilspj66740404;     IBmvUilspj66740404 = IBmvUilspj74064574;     IBmvUilspj74064574 = IBmvUilspj86661475;     IBmvUilspj86661475 = IBmvUilspj5463070;     IBmvUilspj5463070 = IBmvUilspj3194320;     IBmvUilspj3194320 = IBmvUilspj18589411;     IBmvUilspj18589411 = IBmvUilspj29726423;     IBmvUilspj29726423 = IBmvUilspj7973054;     IBmvUilspj7973054 = IBmvUilspj7173812;     IBmvUilspj7173812 = IBmvUilspj7532917;     IBmvUilspj7532917 = IBmvUilspj62043422;     IBmvUilspj62043422 = IBmvUilspj14531554;     IBmvUilspj14531554 = IBmvUilspj81408428;     IBmvUilspj81408428 = IBmvUilspj28886723;     IBmvUilspj28886723 = IBmvUilspj99528209;     IBmvUilspj99528209 = IBmvUilspj66731556;     IBmvUilspj66731556 = IBmvUilspj42739873;     IBmvUilspj42739873 = IBmvUilspj51775087;     IBmvUilspj51775087 = IBmvUilspj13019228;     IBmvUilspj13019228 = IBmvUilspj86824913;     IBmvUilspj86824913 = IBmvUilspj12619417;     IBmvUilspj12619417 = IBmvUilspj78184834;     IBmvUilspj78184834 = IBmvUilspj91748014;     IBmvUilspj91748014 = IBmvUilspj71179383;     IBmvUilspj71179383 = IBmvUilspj89968232;     IBmvUilspj89968232 = IBmvUilspj99726871;     IBmvUilspj99726871 = IBmvUilspj59831860;     IBmvUilspj59831860 = IBmvUilspj87623528;     IBmvUilspj87623528 = IBmvUilspj73293838;     IBmvUilspj73293838 = IBmvUilspj81408258;     IBmvUilspj81408258 = IBmvUilspj58388035;     IBmvUilspj58388035 = IBmvUilspj42308675;     IBmvUilspj42308675 = IBmvUilspj27366916;     IBmvUilspj27366916 = IBmvUilspj3049208;     IBmvUilspj3049208 = IBmvUilspj77106668;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PlhJhufqXa86651600() {     int FOwFOOBSmR60550973 = -645810642;    int FOwFOOBSmR89107249 = -756203414;    int FOwFOOBSmR84693282 = -695594729;    int FOwFOOBSmR63791339 = -361619969;    int FOwFOOBSmR82540784 = -878128947;    int FOwFOOBSmR9922803 = -659317033;    int FOwFOOBSmR17282170 = -591379086;    int FOwFOOBSmR53726496 = -155411407;    int FOwFOOBSmR45816065 = -222607190;    int FOwFOOBSmR80833211 = -323315402;    int FOwFOOBSmR43151605 = -464993606;    int FOwFOOBSmR17018282 = 85672037;    int FOwFOOBSmR95559709 = -517883957;    int FOwFOOBSmR65431379 = -81807768;    int FOwFOOBSmR68389162 = -200918631;    int FOwFOOBSmR95607623 = -981777872;    int FOwFOOBSmR65285568 = -977224483;    int FOwFOOBSmR78920913 = -817063265;    int FOwFOOBSmR92829631 = -649186013;    int FOwFOOBSmR336708 = -84093813;    int FOwFOOBSmR12449301 = -755514073;    int FOwFOOBSmR61290358 = -17040503;    int FOwFOOBSmR63957470 = -134407036;    int FOwFOOBSmR16694062 = -901522380;    int FOwFOOBSmR49462634 = -289209553;    int FOwFOOBSmR91042094 = -842367787;    int FOwFOOBSmR11750545 = 18248713;    int FOwFOOBSmR63587282 = -606823043;    int FOwFOOBSmR84572186 = -481033776;    int FOwFOOBSmR58328763 = -129428768;    int FOwFOOBSmR48149580 = -97163503;    int FOwFOOBSmR59312226 = -698112859;    int FOwFOOBSmR30611365 = -762061158;    int FOwFOOBSmR45535870 = -829650746;    int FOwFOOBSmR77869742 = 85234172;    int FOwFOOBSmR14460815 = 63607629;    int FOwFOOBSmR74638322 = -242732562;    int FOwFOOBSmR93523104 = 29488210;    int FOwFOOBSmR17338122 = -544110383;    int FOwFOOBSmR71911571 = -729136639;    int FOwFOOBSmR37444378 = -865007155;    int FOwFOOBSmR99763782 = -66841361;    int FOwFOOBSmR10666687 = -709876228;    int FOwFOOBSmR37505149 = -892015329;    int FOwFOOBSmR92523571 = -670306004;    int FOwFOOBSmR18605949 = -3250653;    int FOwFOOBSmR72187165 = -309085933;    int FOwFOOBSmR15234844 = 52946155;    int FOwFOOBSmR11962897 = -414847621;    int FOwFOOBSmR32048489 = -207089686;    int FOwFOOBSmR2733643 = -101531126;    int FOwFOOBSmR98730147 = -846232535;    int FOwFOOBSmR97698841 = 4159250;    int FOwFOOBSmR33564403 = -762869495;    int FOwFOOBSmR74202299 = -48959798;    int FOwFOOBSmR99260615 = -528770139;    int FOwFOOBSmR25149780 = -521796378;    int FOwFOOBSmR67999221 = -794072349;    int FOwFOOBSmR14328706 = 27589583;    int FOwFOOBSmR91498689 = 64238840;    int FOwFOOBSmR98172258 = -577565747;    int FOwFOOBSmR53694887 = -984556044;    int FOwFOOBSmR69154309 = -674377632;    int FOwFOOBSmR87487301 = 6821577;    int FOwFOOBSmR32683632 = -126151900;    int FOwFOOBSmR83839378 = -766880748;    int FOwFOOBSmR86406916 = -152266805;    int FOwFOOBSmR50023840 = -688233212;    int FOwFOOBSmR87561637 = -67041941;    int FOwFOOBSmR53928348 = -164526260;    int FOwFOOBSmR20969302 = -639045311;    int FOwFOOBSmR71762463 = -906712693;    int FOwFOOBSmR61582791 = -172952883;    int FOwFOOBSmR20918061 = -920049375;    int FOwFOOBSmR62892329 = -219086659;    int FOwFOOBSmR12685518 = -588672712;    int FOwFOOBSmR50623671 = -307164276;    int FOwFOOBSmR26452322 = -242391708;    int FOwFOOBSmR24170490 = -131216377;    int FOwFOOBSmR30856685 = -185958901;    int FOwFOOBSmR18854929 = -433281855;    int FOwFOOBSmR96515700 = 65302557;    int FOwFOOBSmR51624386 = -91975423;    int FOwFOOBSmR52523697 = -173944090;    int FOwFOOBSmR55595121 = 72102357;    int FOwFOOBSmR49419433 = -250930968;    int FOwFOOBSmR61613385 = -602272110;    int FOwFOOBSmR97046962 = -999191664;    int FOwFOOBSmR71333570 = -680690948;    int FOwFOOBSmR78609126 = -385995689;    int FOwFOOBSmR89311035 = -414595994;    int FOwFOOBSmR6639101 = -448660214;    int FOwFOOBSmR79194399 = -998101374;    int FOwFOOBSmR25839432 = -508349224;    int FOwFOOBSmR73739313 = -51570893;    int FOwFOOBSmR83749490 = -880451112;    int FOwFOOBSmR30609473 = -392463730;    int FOwFOOBSmR23179386 = -616697806;    int FOwFOOBSmR4821518 = -665863430;    int FOwFOOBSmR8684194 = -645810642;     FOwFOOBSmR60550973 = FOwFOOBSmR89107249;     FOwFOOBSmR89107249 = FOwFOOBSmR84693282;     FOwFOOBSmR84693282 = FOwFOOBSmR63791339;     FOwFOOBSmR63791339 = FOwFOOBSmR82540784;     FOwFOOBSmR82540784 = FOwFOOBSmR9922803;     FOwFOOBSmR9922803 = FOwFOOBSmR17282170;     FOwFOOBSmR17282170 = FOwFOOBSmR53726496;     FOwFOOBSmR53726496 = FOwFOOBSmR45816065;     FOwFOOBSmR45816065 = FOwFOOBSmR80833211;     FOwFOOBSmR80833211 = FOwFOOBSmR43151605;     FOwFOOBSmR43151605 = FOwFOOBSmR17018282;     FOwFOOBSmR17018282 = FOwFOOBSmR95559709;     FOwFOOBSmR95559709 = FOwFOOBSmR65431379;     FOwFOOBSmR65431379 = FOwFOOBSmR68389162;     FOwFOOBSmR68389162 = FOwFOOBSmR95607623;     FOwFOOBSmR95607623 = FOwFOOBSmR65285568;     FOwFOOBSmR65285568 = FOwFOOBSmR78920913;     FOwFOOBSmR78920913 = FOwFOOBSmR92829631;     FOwFOOBSmR92829631 = FOwFOOBSmR336708;     FOwFOOBSmR336708 = FOwFOOBSmR12449301;     FOwFOOBSmR12449301 = FOwFOOBSmR61290358;     FOwFOOBSmR61290358 = FOwFOOBSmR63957470;     FOwFOOBSmR63957470 = FOwFOOBSmR16694062;     FOwFOOBSmR16694062 = FOwFOOBSmR49462634;     FOwFOOBSmR49462634 = FOwFOOBSmR91042094;     FOwFOOBSmR91042094 = FOwFOOBSmR11750545;     FOwFOOBSmR11750545 = FOwFOOBSmR63587282;     FOwFOOBSmR63587282 = FOwFOOBSmR84572186;     FOwFOOBSmR84572186 = FOwFOOBSmR58328763;     FOwFOOBSmR58328763 = FOwFOOBSmR48149580;     FOwFOOBSmR48149580 = FOwFOOBSmR59312226;     FOwFOOBSmR59312226 = FOwFOOBSmR30611365;     FOwFOOBSmR30611365 = FOwFOOBSmR45535870;     FOwFOOBSmR45535870 = FOwFOOBSmR77869742;     FOwFOOBSmR77869742 = FOwFOOBSmR14460815;     FOwFOOBSmR14460815 = FOwFOOBSmR74638322;     FOwFOOBSmR74638322 = FOwFOOBSmR93523104;     FOwFOOBSmR93523104 = FOwFOOBSmR17338122;     FOwFOOBSmR17338122 = FOwFOOBSmR71911571;     FOwFOOBSmR71911571 = FOwFOOBSmR37444378;     FOwFOOBSmR37444378 = FOwFOOBSmR99763782;     FOwFOOBSmR99763782 = FOwFOOBSmR10666687;     FOwFOOBSmR10666687 = FOwFOOBSmR37505149;     FOwFOOBSmR37505149 = FOwFOOBSmR92523571;     FOwFOOBSmR92523571 = FOwFOOBSmR18605949;     FOwFOOBSmR18605949 = FOwFOOBSmR72187165;     FOwFOOBSmR72187165 = FOwFOOBSmR15234844;     FOwFOOBSmR15234844 = FOwFOOBSmR11962897;     FOwFOOBSmR11962897 = FOwFOOBSmR32048489;     FOwFOOBSmR32048489 = FOwFOOBSmR2733643;     FOwFOOBSmR2733643 = FOwFOOBSmR98730147;     FOwFOOBSmR98730147 = FOwFOOBSmR97698841;     FOwFOOBSmR97698841 = FOwFOOBSmR33564403;     FOwFOOBSmR33564403 = FOwFOOBSmR74202299;     FOwFOOBSmR74202299 = FOwFOOBSmR99260615;     FOwFOOBSmR99260615 = FOwFOOBSmR25149780;     FOwFOOBSmR25149780 = FOwFOOBSmR67999221;     FOwFOOBSmR67999221 = FOwFOOBSmR14328706;     FOwFOOBSmR14328706 = FOwFOOBSmR91498689;     FOwFOOBSmR91498689 = FOwFOOBSmR98172258;     FOwFOOBSmR98172258 = FOwFOOBSmR53694887;     FOwFOOBSmR53694887 = FOwFOOBSmR69154309;     FOwFOOBSmR69154309 = FOwFOOBSmR87487301;     FOwFOOBSmR87487301 = FOwFOOBSmR32683632;     FOwFOOBSmR32683632 = FOwFOOBSmR83839378;     FOwFOOBSmR83839378 = FOwFOOBSmR86406916;     FOwFOOBSmR86406916 = FOwFOOBSmR50023840;     FOwFOOBSmR50023840 = FOwFOOBSmR87561637;     FOwFOOBSmR87561637 = FOwFOOBSmR53928348;     FOwFOOBSmR53928348 = FOwFOOBSmR20969302;     FOwFOOBSmR20969302 = FOwFOOBSmR71762463;     FOwFOOBSmR71762463 = FOwFOOBSmR61582791;     FOwFOOBSmR61582791 = FOwFOOBSmR20918061;     FOwFOOBSmR20918061 = FOwFOOBSmR62892329;     FOwFOOBSmR62892329 = FOwFOOBSmR12685518;     FOwFOOBSmR12685518 = FOwFOOBSmR50623671;     FOwFOOBSmR50623671 = FOwFOOBSmR26452322;     FOwFOOBSmR26452322 = FOwFOOBSmR24170490;     FOwFOOBSmR24170490 = FOwFOOBSmR30856685;     FOwFOOBSmR30856685 = FOwFOOBSmR18854929;     FOwFOOBSmR18854929 = FOwFOOBSmR96515700;     FOwFOOBSmR96515700 = FOwFOOBSmR51624386;     FOwFOOBSmR51624386 = FOwFOOBSmR52523697;     FOwFOOBSmR52523697 = FOwFOOBSmR55595121;     FOwFOOBSmR55595121 = FOwFOOBSmR49419433;     FOwFOOBSmR49419433 = FOwFOOBSmR61613385;     FOwFOOBSmR61613385 = FOwFOOBSmR97046962;     FOwFOOBSmR97046962 = FOwFOOBSmR71333570;     FOwFOOBSmR71333570 = FOwFOOBSmR78609126;     FOwFOOBSmR78609126 = FOwFOOBSmR89311035;     FOwFOOBSmR89311035 = FOwFOOBSmR6639101;     FOwFOOBSmR6639101 = FOwFOOBSmR79194399;     FOwFOOBSmR79194399 = FOwFOOBSmR25839432;     FOwFOOBSmR25839432 = FOwFOOBSmR73739313;     FOwFOOBSmR73739313 = FOwFOOBSmR83749490;     FOwFOOBSmR83749490 = FOwFOOBSmR30609473;     FOwFOOBSmR30609473 = FOwFOOBSmR23179386;     FOwFOOBSmR23179386 = FOwFOOBSmR4821518;     FOwFOOBSmR4821518 = FOwFOOBSmR8684194;     FOwFOOBSmR8684194 = FOwFOOBSmR60550973;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CgxPtBPEPi56777276() {     int naKZwDSaMm27998010 = -245854776;    int naKZwDSaMm84303441 = -363507116;    int naKZwDSaMm61865516 = -432210110;    int naKZwDSaMm58087245 = -371383827;    int naKZwDSaMm41663457 = -574025330;    int naKZwDSaMm32601838 = -865958371;    int naKZwDSaMm91043295 = -634407362;    int naKZwDSaMm40406662 = -296320580;    int naKZwDSaMm46255122 = -517572583;    int naKZwDSaMm73044388 = -769397433;    int naKZwDSaMm28929677 = -145564122;    int naKZwDSaMm97441822 = -872606506;    int naKZwDSaMm97667412 = -203548444;    int naKZwDSaMm54177669 = -781173493;    int naKZwDSaMm76747680 = -52313241;    int naKZwDSaMm39235681 = -830033175;    int naKZwDSaMm65016390 = -194984208;    int naKZwDSaMm7075452 = -38766413;    int naKZwDSaMm74782593 = -226484489;    int naKZwDSaMm48551996 = -416679647;    int naKZwDSaMm47128321 = 95552886;    int naKZwDSaMm63294500 = -943351076;    int naKZwDSaMm2309201 = -193196510;    int naKZwDSaMm18979452 = 20012313;    int naKZwDSaMm22666820 = -445871076;    int naKZwDSaMm97027609 = -628301843;    int naKZwDSaMm54934424 = -869502133;    int naKZwDSaMm48785682 = -179706020;    int naKZwDSaMm28408470 = -429425577;    int naKZwDSaMm29425418 = -374641517;    int naKZwDSaMm10878426 = -535359544;    int naKZwDSaMm8898804 = -696507304;    int naKZwDSaMm58860768 = -992489341;    int naKZwDSaMm16985298 = -537844041;    int naKZwDSaMm13076393 = -483347043;    int naKZwDSaMm54183282 = -993185326;    int naKZwDSaMm74165502 = -883415155;    int naKZwDSaMm94286686 = -868798223;    int naKZwDSaMm38569560 = -55228146;    int naKZwDSaMm50289803 = -622430398;    int naKZwDSaMm66979419 = -165087408;    int naKZwDSaMm58443236 = -378892700;    int naKZwDSaMm76266979 = -14684736;    int naKZwDSaMm31896188 = -49902794;    int naKZwDSaMm48498073 = -222334161;    int naKZwDSaMm7139712 = -333314036;    int naKZwDSaMm38545105 = -352377747;    int naKZwDSaMm92718538 = -571297641;    int naKZwDSaMm79711323 = -914415923;    int naKZwDSaMm3795149 = -371120557;    int naKZwDSaMm82915305 = -433075929;    int naKZwDSaMm1741587 = -350338108;    int naKZwDSaMm88463646 = -19705481;    int naKZwDSaMm25429615 = -816287870;    int naKZwDSaMm9648955 = -682413688;    int naKZwDSaMm64703509 = -302503701;    int naKZwDSaMm81994240 = -70310607;    int naKZwDSaMm42886065 = -352222424;    int naKZwDSaMm35420426 = -925512751;    int naKZwDSaMm44635847 = -945723488;    int naKZwDSaMm77667414 = -996456238;    int naKZwDSaMm42257613 = -354701343;    int naKZwDSaMm11998192 = -866895003;    int naKZwDSaMm16829704 = -42931066;    int naKZwDSaMm62165962 = -134037890;    int naKZwDSaMm20030874 = -449056818;    int naKZwDSaMm38581055 = -880117165;    int naKZwDSaMm80682115 = -665704404;    int naKZwDSaMm41101276 = -197826451;    int naKZwDSaMm22564399 = -59127916;    int naKZwDSaMm65070179 = -946618021;    int naKZwDSaMm70729703 = -326185985;    int naKZwDSaMm68505892 = -983538268;    int naKZwDSaMm24492790 = -604054092;    int naKZwDSaMm81572577 = -151592240;    int naKZwDSaMm88685085 = -525554415;    int naKZwDSaMm87027520 = -828666340;    int naKZwDSaMm70413013 = -43293716;    int naKZwDSaMm70481378 = -757653526;    int naKZwDSaMm15527108 = -12557040;    int naKZwDSaMm58482505 = -175924096;    int naKZwDSaMm62215885 = -198204493;    int naKZwDSaMm69074359 = -265290097;    int naKZwDSaMm24613321 = 41694980;    int naKZwDSaMm46510112 = -941565589;    int naKZwDSaMm9136840 = -85021437;    int naKZwDSaMm20435157 = -576801823;    int naKZwDSaMm33431153 = -76201472;    int naKZwDSaMm7336344 = -855430353;    int naKZwDSaMm48372884 = -80843342;    int naKZwDSaMm72189042 = -822874720;    int naKZwDSaMm31279437 = -431192731;    int naKZwDSaMm58866261 = -943285473;    int naKZwDSaMm93933712 = -109504659;    int naKZwDSaMm72622388 = -625974160;    int naKZwDSaMm24721806 = -810386066;    int naKZwDSaMm46445044 = -511997697;    int naKZwDSaMm59437275 = -971753670;    int naKZwDSaMm69730226 = -915864905;    int naKZwDSaMm28467200 = -245854776;     naKZwDSaMm27998010 = naKZwDSaMm84303441;     naKZwDSaMm84303441 = naKZwDSaMm61865516;     naKZwDSaMm61865516 = naKZwDSaMm58087245;     naKZwDSaMm58087245 = naKZwDSaMm41663457;     naKZwDSaMm41663457 = naKZwDSaMm32601838;     naKZwDSaMm32601838 = naKZwDSaMm91043295;     naKZwDSaMm91043295 = naKZwDSaMm40406662;     naKZwDSaMm40406662 = naKZwDSaMm46255122;     naKZwDSaMm46255122 = naKZwDSaMm73044388;     naKZwDSaMm73044388 = naKZwDSaMm28929677;     naKZwDSaMm28929677 = naKZwDSaMm97441822;     naKZwDSaMm97441822 = naKZwDSaMm97667412;     naKZwDSaMm97667412 = naKZwDSaMm54177669;     naKZwDSaMm54177669 = naKZwDSaMm76747680;     naKZwDSaMm76747680 = naKZwDSaMm39235681;     naKZwDSaMm39235681 = naKZwDSaMm65016390;     naKZwDSaMm65016390 = naKZwDSaMm7075452;     naKZwDSaMm7075452 = naKZwDSaMm74782593;     naKZwDSaMm74782593 = naKZwDSaMm48551996;     naKZwDSaMm48551996 = naKZwDSaMm47128321;     naKZwDSaMm47128321 = naKZwDSaMm63294500;     naKZwDSaMm63294500 = naKZwDSaMm2309201;     naKZwDSaMm2309201 = naKZwDSaMm18979452;     naKZwDSaMm18979452 = naKZwDSaMm22666820;     naKZwDSaMm22666820 = naKZwDSaMm97027609;     naKZwDSaMm97027609 = naKZwDSaMm54934424;     naKZwDSaMm54934424 = naKZwDSaMm48785682;     naKZwDSaMm48785682 = naKZwDSaMm28408470;     naKZwDSaMm28408470 = naKZwDSaMm29425418;     naKZwDSaMm29425418 = naKZwDSaMm10878426;     naKZwDSaMm10878426 = naKZwDSaMm8898804;     naKZwDSaMm8898804 = naKZwDSaMm58860768;     naKZwDSaMm58860768 = naKZwDSaMm16985298;     naKZwDSaMm16985298 = naKZwDSaMm13076393;     naKZwDSaMm13076393 = naKZwDSaMm54183282;     naKZwDSaMm54183282 = naKZwDSaMm74165502;     naKZwDSaMm74165502 = naKZwDSaMm94286686;     naKZwDSaMm94286686 = naKZwDSaMm38569560;     naKZwDSaMm38569560 = naKZwDSaMm50289803;     naKZwDSaMm50289803 = naKZwDSaMm66979419;     naKZwDSaMm66979419 = naKZwDSaMm58443236;     naKZwDSaMm58443236 = naKZwDSaMm76266979;     naKZwDSaMm76266979 = naKZwDSaMm31896188;     naKZwDSaMm31896188 = naKZwDSaMm48498073;     naKZwDSaMm48498073 = naKZwDSaMm7139712;     naKZwDSaMm7139712 = naKZwDSaMm38545105;     naKZwDSaMm38545105 = naKZwDSaMm92718538;     naKZwDSaMm92718538 = naKZwDSaMm79711323;     naKZwDSaMm79711323 = naKZwDSaMm3795149;     naKZwDSaMm3795149 = naKZwDSaMm82915305;     naKZwDSaMm82915305 = naKZwDSaMm1741587;     naKZwDSaMm1741587 = naKZwDSaMm88463646;     naKZwDSaMm88463646 = naKZwDSaMm25429615;     naKZwDSaMm25429615 = naKZwDSaMm9648955;     naKZwDSaMm9648955 = naKZwDSaMm64703509;     naKZwDSaMm64703509 = naKZwDSaMm81994240;     naKZwDSaMm81994240 = naKZwDSaMm42886065;     naKZwDSaMm42886065 = naKZwDSaMm35420426;     naKZwDSaMm35420426 = naKZwDSaMm44635847;     naKZwDSaMm44635847 = naKZwDSaMm77667414;     naKZwDSaMm77667414 = naKZwDSaMm42257613;     naKZwDSaMm42257613 = naKZwDSaMm11998192;     naKZwDSaMm11998192 = naKZwDSaMm16829704;     naKZwDSaMm16829704 = naKZwDSaMm62165962;     naKZwDSaMm62165962 = naKZwDSaMm20030874;     naKZwDSaMm20030874 = naKZwDSaMm38581055;     naKZwDSaMm38581055 = naKZwDSaMm80682115;     naKZwDSaMm80682115 = naKZwDSaMm41101276;     naKZwDSaMm41101276 = naKZwDSaMm22564399;     naKZwDSaMm22564399 = naKZwDSaMm65070179;     naKZwDSaMm65070179 = naKZwDSaMm70729703;     naKZwDSaMm70729703 = naKZwDSaMm68505892;     naKZwDSaMm68505892 = naKZwDSaMm24492790;     naKZwDSaMm24492790 = naKZwDSaMm81572577;     naKZwDSaMm81572577 = naKZwDSaMm88685085;     naKZwDSaMm88685085 = naKZwDSaMm87027520;     naKZwDSaMm87027520 = naKZwDSaMm70413013;     naKZwDSaMm70413013 = naKZwDSaMm70481378;     naKZwDSaMm70481378 = naKZwDSaMm15527108;     naKZwDSaMm15527108 = naKZwDSaMm58482505;     naKZwDSaMm58482505 = naKZwDSaMm62215885;     naKZwDSaMm62215885 = naKZwDSaMm69074359;     naKZwDSaMm69074359 = naKZwDSaMm24613321;     naKZwDSaMm24613321 = naKZwDSaMm46510112;     naKZwDSaMm46510112 = naKZwDSaMm9136840;     naKZwDSaMm9136840 = naKZwDSaMm20435157;     naKZwDSaMm20435157 = naKZwDSaMm33431153;     naKZwDSaMm33431153 = naKZwDSaMm7336344;     naKZwDSaMm7336344 = naKZwDSaMm48372884;     naKZwDSaMm48372884 = naKZwDSaMm72189042;     naKZwDSaMm72189042 = naKZwDSaMm31279437;     naKZwDSaMm31279437 = naKZwDSaMm58866261;     naKZwDSaMm58866261 = naKZwDSaMm93933712;     naKZwDSaMm93933712 = naKZwDSaMm72622388;     naKZwDSaMm72622388 = naKZwDSaMm24721806;     naKZwDSaMm24721806 = naKZwDSaMm46445044;     naKZwDSaMm46445044 = naKZwDSaMm59437275;     naKZwDSaMm59437275 = naKZwDSaMm69730226;     naKZwDSaMm69730226 = naKZwDSaMm28467200;     naKZwDSaMm28467200 = naKZwDSaMm27998010;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ViuAgYktBW27233149() {     int TtHqFnjWEI40784177 = -380895468;    int TtHqFnjWEI71074329 = -533841130;    int TtHqFnjWEI79618316 = -840569492;    int TtHqFnjWEI66028071 = -195586104;    int TtHqFnjWEI35174562 = 34158205;    int TtHqFnjWEI75911909 = -642881684;    int TtHqFnjWEI12251054 = -80079433;    int TtHqFnjWEI67125774 = -740838836;    int TtHqFnjWEI41659932 = -479330675;    int TtHqFnjWEI79263093 = -573485066;    int TtHqFnjWEI56417059 = -260250137;    int TtHqFnjWEI29392328 = -279939321;    int TtHqFnjWEI67810716 = -107331101;    int TtHqFnjWEI3801748 = -13588692;    int TtHqFnjWEI54234 = 40700928;    int TtHqFnjWEI67660750 = -280924564;    int TtHqFnjWEI20768208 = -958758675;    int TtHqFnjWEI90633937 = -693976715;    int TtHqFnjWEI75943223 = -135953685;    int TtHqFnjWEI51206492 = 84076214;    int TtHqFnjWEI27302378 = -395763379;    int TtHqFnjWEI92342319 = -13209424;    int TtHqFnjWEI41069497 = -124200404;    int TtHqFnjWEI63486581 = -340332699;    int TtHqFnjWEI28561079 = -656313001;    int TtHqFnjWEI78346255 = -318733042;    int TtHqFnjWEI66349960 = -403208414;    int TtHqFnjWEI16138608 = -353504124;    int TtHqFnjWEI45025523 = -643642817;    int TtHqFnjWEI82176533 = -4527672;    int TtHqFnjWEI55314422 = -811776294;    int TtHqFnjWEI12217627 = -675226983;    int TtHqFnjWEI4906717 = -868344125;    int TtHqFnjWEI94061670 = -442598259;    int TtHqFnjWEI96136925 = 6621077;    int TtHqFnjWEI49407396 = -781748430;    int TtHqFnjWEI10929563 = -673324505;    int TtHqFnjWEI51990710 = -788418315;    int TtHqFnjWEI44344946 = -997078723;    int TtHqFnjWEI76554532 = -466847243;    int TtHqFnjWEI28410727 = -540561401;    int TtHqFnjWEI66145691 = -277204034;    int TtHqFnjWEI11150450 = -38118181;    int TtHqFnjWEI76524726 = -391075037;    int TtHqFnjWEI28506564 = -115104121;    int TtHqFnjWEI80767841 = -406632476;    int TtHqFnjWEI16127124 = -849892800;    int TtHqFnjWEI22158505 = -726650420;    int TtHqFnjWEI23775373 = 21982102;    int TtHqFnjWEI50745569 = -982157020;    int TtHqFnjWEI1242800 = -685960237;    int TtHqFnjWEI29918134 = -821523446;    int TtHqFnjWEI22948797 = -195708764;    int TtHqFnjWEI70129340 = -405739612;    int TtHqFnjWEI99930771 = -676858174;    int TtHqFnjWEI48441858 = -267686045;    int TtHqFnjWEI30004833 = -309640726;    int TtHqFnjWEI16131736 = -400236793;    int TtHqFnjWEI37466993 = -539273104;    int TtHqFnjWEI56828306 = -647108754;    int TtHqFnjWEI9561950 = -139673271;    int TtHqFnjWEI96112446 = -726575309;    int TtHqFnjWEI22100252 = 2803980;    int TtHqFnjWEI59483399 = -374803004;    int TtHqFnjWEI23948671 = -761708773;    int TtHqFnjWEI44199432 = -585023155;    int TtHqFnjWEI24485611 = -411595197;    int TtHqFnjWEI73749045 = -664732842;    int TtHqFnjWEI7664823 = 79790231;    int TtHqFnjWEI50646837 = -177550643;    int TtHqFnjWEI56731187 = -607600060;    int TtHqFnjWEI68777498 = -70340361;    int TtHqFnjWEI46288992 = -696897993;    int TtHqFnjWEI99388690 = -669106442;    int TtHqFnjWEI22795765 = -375362386;    int TtHqFnjWEI61156686 = -18559345;    int TtHqFnjWEI81191870 = -975091243;    int TtHqFnjWEI64544771 = -733125368;    int TtHqFnjWEI34980017 = -125228579;    int TtHqFnjWEI47793237 = -149680525;    int TtHqFnjWEI62219131 = -468840243;    int TtHqFnjWEI44191455 = -676557994;    int TtHqFnjWEI92363234 = -275486227;    int TtHqFnjWEI94279953 = -661485797;    int TtHqFnjWEI80933734 = -318567435;    int TtHqFnjWEI25396288 = -990252849;    int TtHqFnjWEI89268830 = -379518219;    int TtHqFnjWEI34777377 = -362604514;    int TtHqFnjWEI94130899 = -765740085;    int TtHqFnjWEI47695068 = -725692879;    int TtHqFnjWEI19402564 = -372107705;    int TtHqFnjWEI94797827 = -173087712;    int TtHqFnjWEI14523718 = -149145212;    int TtHqFnjWEI87516639 = -249969970;    int TtHqFnjWEI66992583 = -227173973;    int TtHqFnjWEI32298281 = -813986093;    int TtHqFnjWEI44045440 = -180008015;    int TtHqFnjWEI51667050 = -663315178;    int TtHqFnjWEI52576055 = -629366265;    int TtHqFnjWEI84307131 = -380895468;     TtHqFnjWEI40784177 = TtHqFnjWEI71074329;     TtHqFnjWEI71074329 = TtHqFnjWEI79618316;     TtHqFnjWEI79618316 = TtHqFnjWEI66028071;     TtHqFnjWEI66028071 = TtHqFnjWEI35174562;     TtHqFnjWEI35174562 = TtHqFnjWEI75911909;     TtHqFnjWEI75911909 = TtHqFnjWEI12251054;     TtHqFnjWEI12251054 = TtHqFnjWEI67125774;     TtHqFnjWEI67125774 = TtHqFnjWEI41659932;     TtHqFnjWEI41659932 = TtHqFnjWEI79263093;     TtHqFnjWEI79263093 = TtHqFnjWEI56417059;     TtHqFnjWEI56417059 = TtHqFnjWEI29392328;     TtHqFnjWEI29392328 = TtHqFnjWEI67810716;     TtHqFnjWEI67810716 = TtHqFnjWEI3801748;     TtHqFnjWEI3801748 = TtHqFnjWEI54234;     TtHqFnjWEI54234 = TtHqFnjWEI67660750;     TtHqFnjWEI67660750 = TtHqFnjWEI20768208;     TtHqFnjWEI20768208 = TtHqFnjWEI90633937;     TtHqFnjWEI90633937 = TtHqFnjWEI75943223;     TtHqFnjWEI75943223 = TtHqFnjWEI51206492;     TtHqFnjWEI51206492 = TtHqFnjWEI27302378;     TtHqFnjWEI27302378 = TtHqFnjWEI92342319;     TtHqFnjWEI92342319 = TtHqFnjWEI41069497;     TtHqFnjWEI41069497 = TtHqFnjWEI63486581;     TtHqFnjWEI63486581 = TtHqFnjWEI28561079;     TtHqFnjWEI28561079 = TtHqFnjWEI78346255;     TtHqFnjWEI78346255 = TtHqFnjWEI66349960;     TtHqFnjWEI66349960 = TtHqFnjWEI16138608;     TtHqFnjWEI16138608 = TtHqFnjWEI45025523;     TtHqFnjWEI45025523 = TtHqFnjWEI82176533;     TtHqFnjWEI82176533 = TtHqFnjWEI55314422;     TtHqFnjWEI55314422 = TtHqFnjWEI12217627;     TtHqFnjWEI12217627 = TtHqFnjWEI4906717;     TtHqFnjWEI4906717 = TtHqFnjWEI94061670;     TtHqFnjWEI94061670 = TtHqFnjWEI96136925;     TtHqFnjWEI96136925 = TtHqFnjWEI49407396;     TtHqFnjWEI49407396 = TtHqFnjWEI10929563;     TtHqFnjWEI10929563 = TtHqFnjWEI51990710;     TtHqFnjWEI51990710 = TtHqFnjWEI44344946;     TtHqFnjWEI44344946 = TtHqFnjWEI76554532;     TtHqFnjWEI76554532 = TtHqFnjWEI28410727;     TtHqFnjWEI28410727 = TtHqFnjWEI66145691;     TtHqFnjWEI66145691 = TtHqFnjWEI11150450;     TtHqFnjWEI11150450 = TtHqFnjWEI76524726;     TtHqFnjWEI76524726 = TtHqFnjWEI28506564;     TtHqFnjWEI28506564 = TtHqFnjWEI80767841;     TtHqFnjWEI80767841 = TtHqFnjWEI16127124;     TtHqFnjWEI16127124 = TtHqFnjWEI22158505;     TtHqFnjWEI22158505 = TtHqFnjWEI23775373;     TtHqFnjWEI23775373 = TtHqFnjWEI50745569;     TtHqFnjWEI50745569 = TtHqFnjWEI1242800;     TtHqFnjWEI1242800 = TtHqFnjWEI29918134;     TtHqFnjWEI29918134 = TtHqFnjWEI22948797;     TtHqFnjWEI22948797 = TtHqFnjWEI70129340;     TtHqFnjWEI70129340 = TtHqFnjWEI99930771;     TtHqFnjWEI99930771 = TtHqFnjWEI48441858;     TtHqFnjWEI48441858 = TtHqFnjWEI30004833;     TtHqFnjWEI30004833 = TtHqFnjWEI16131736;     TtHqFnjWEI16131736 = TtHqFnjWEI37466993;     TtHqFnjWEI37466993 = TtHqFnjWEI56828306;     TtHqFnjWEI56828306 = TtHqFnjWEI9561950;     TtHqFnjWEI9561950 = TtHqFnjWEI96112446;     TtHqFnjWEI96112446 = TtHqFnjWEI22100252;     TtHqFnjWEI22100252 = TtHqFnjWEI59483399;     TtHqFnjWEI59483399 = TtHqFnjWEI23948671;     TtHqFnjWEI23948671 = TtHqFnjWEI44199432;     TtHqFnjWEI44199432 = TtHqFnjWEI24485611;     TtHqFnjWEI24485611 = TtHqFnjWEI73749045;     TtHqFnjWEI73749045 = TtHqFnjWEI7664823;     TtHqFnjWEI7664823 = TtHqFnjWEI50646837;     TtHqFnjWEI50646837 = TtHqFnjWEI56731187;     TtHqFnjWEI56731187 = TtHqFnjWEI68777498;     TtHqFnjWEI68777498 = TtHqFnjWEI46288992;     TtHqFnjWEI46288992 = TtHqFnjWEI99388690;     TtHqFnjWEI99388690 = TtHqFnjWEI22795765;     TtHqFnjWEI22795765 = TtHqFnjWEI61156686;     TtHqFnjWEI61156686 = TtHqFnjWEI81191870;     TtHqFnjWEI81191870 = TtHqFnjWEI64544771;     TtHqFnjWEI64544771 = TtHqFnjWEI34980017;     TtHqFnjWEI34980017 = TtHqFnjWEI47793237;     TtHqFnjWEI47793237 = TtHqFnjWEI62219131;     TtHqFnjWEI62219131 = TtHqFnjWEI44191455;     TtHqFnjWEI44191455 = TtHqFnjWEI92363234;     TtHqFnjWEI92363234 = TtHqFnjWEI94279953;     TtHqFnjWEI94279953 = TtHqFnjWEI80933734;     TtHqFnjWEI80933734 = TtHqFnjWEI25396288;     TtHqFnjWEI25396288 = TtHqFnjWEI89268830;     TtHqFnjWEI89268830 = TtHqFnjWEI34777377;     TtHqFnjWEI34777377 = TtHqFnjWEI94130899;     TtHqFnjWEI94130899 = TtHqFnjWEI47695068;     TtHqFnjWEI47695068 = TtHqFnjWEI19402564;     TtHqFnjWEI19402564 = TtHqFnjWEI94797827;     TtHqFnjWEI94797827 = TtHqFnjWEI14523718;     TtHqFnjWEI14523718 = TtHqFnjWEI87516639;     TtHqFnjWEI87516639 = TtHqFnjWEI66992583;     TtHqFnjWEI66992583 = TtHqFnjWEI32298281;     TtHqFnjWEI32298281 = TtHqFnjWEI44045440;     TtHqFnjWEI44045440 = TtHqFnjWEI51667050;     TtHqFnjWEI51667050 = TtHqFnjWEI52576055;     TtHqFnjWEI52576055 = TtHqFnjWEI84307131;     TtHqFnjWEI84307131 = TtHqFnjWEI40784177;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VnZquluZgy45446490() {     int bHDMbiohzA24228482 = -161561149;    int bHDMbiohzA87946418 = -533223317;    int bHDMbiohzA76026823 = -634094720;    int bHDMbiohzA28820323 = -389385940;    int bHDMbiohzA41295885 = -872709286;    int bHDMbiohzA74416310 = -421953337;    int bHDMbiohzA83290371 = -919990746;    int bHDMbiohzA81473216 = -727996866;    int bHDMbiohzA189633 = -820790026;    int bHDMbiohzA11808745 = 58138822;    int bHDMbiohzA5832997 = -450366010;    int bHDMbiohzA92597727 = -267557568;    int bHDMbiohzA85928491 = -998992342;    int bHDMbiohzA83428639 = -901879048;    int bHDMbiohzA60908696 = -53322054;    int bHDMbiohzA54049911 = -69003890;    int bHDMbiohzA14520093 = -712103702;    int bHDMbiohzA71485383 = -941281592;    int bHDMbiohzA38383366 = -134628556;    int bHDMbiohzA40573935 = -445509779;    int bHDMbiohzA82942764 = 14707590;    int bHDMbiohzA4489638 = -4361194;    int bHDMbiohzA44895204 = -164089602;    int bHDMbiohzA98193139 = -412158095;    int bHDMbiohzA70137036 = -115965760;    int bHDMbiohzA73688404 = -199242758;    int bHDMbiohzA15804700 = -31292756;    int bHDMbiohzA55870233 = -423459008;    int bHDMbiohzA81106616 = -506147959;    int bHDMbiohzA98009874 = -517377523;    int bHDMbiohzA73409736 = -449533495;    int bHDMbiohzA75324054 = -590422063;    int bHDMbiohzA48445604 = -489216303;    int bHDMbiohzA36220182 = -34200429;    int bHDMbiohzA74863655 = -191043658;    int bHDMbiohzA61796580 = -569772336;    int bHDMbiohzA79543741 = -552173685;    int bHDMbiohzA92569541 = -496888835;    int bHDMbiohzA93340023 = -81976521;    int bHDMbiohzA26049668 = -219440766;    int bHDMbiohzA62059651 = -112110374;    int bHDMbiohzA16633478 = -851112356;    int bHDMbiohzA34717519 = -589175424;    int bHDMbiohzA68429666 = -147257808;    int bHDMbiohzA20451061 = -943261074;    int bHDMbiohzA20373837 = -426243400;    int bHDMbiohzA92142554 = -982197029;    int bHDMbiohzA41829100 = -725372139;    int bHDMbiohzA54622485 = -563619980;    int bHDMbiohzA86078053 = -89177475;    int bHDMbiohzA74500247 = -906861660;    int bHDMbiohzA85418928 = -879782757;    int bHDMbiohzA37061256 = -407456080;    int bHDMbiohzA94806099 = -124152999;    int bHDMbiohzA62503725 = 74655703;    int bHDMbiohzA19738845 = -57199955;    int bHDMbiohzA43051214 = -269133716;    int bHDMbiohzA77833684 = -121936625;    int bHDMbiohzA58683286 = -173420181;    int bHDMbiohzA67607481 = -573466529;    int bHDMbiohzA58611610 = -290660582;    int bHDMbiohzA27420139 = -396531738;    int bHDMbiohzA366601 = -121848907;    int bHDMbiohzA2179759 = -203412503;    int bHDMbiohzA38399009 = -492327684;    int bHDMbiohzA30508943 = -859943948;    int bHDMbiohzA44152123 = -778341265;    int bHDMbiohzA49708310 = -864791913;    int bHDMbiohzA8564984 = -610835391;    int bHDMbiohzA99112115 = -483549719;    int bHDMbiohzA74506170 = -516830205;    int bHDMbiohzA21950551 = -115214868;    int bHDMbiohzA78145360 = -759305072;    int bHDMbiohzA12333698 = -915187790;    int bHDMbiohzA78514283 = -233399405;    int bHDMbiohzA66309287 = -134180054;    int bHDMbiohzA69772118 = -415185771;    int bHDMbiohzA76465538 = 83168206;    int bHDMbiohzA77742078 = -468897021;    int bHDMbiohzA49763199 = -689722360;    int bHDMbiohzA81545849 = -217045729;    int bHDMbiohzA73975599 = -305920618;    int bHDMbiohzA1247748 = -859839029;    int bHDMbiohzA95028563 = -316970484;    int bHDMbiohzA23509627 = -610515864;    int bHDMbiohzA87990807 = -569750738;    int bHDMbiohzA38262798 = -82965983;    int bHDMbiohzA53639505 = -265063304;    int bHDMbiohzA73716456 = -8856132;    int bHDMbiohzA55124811 = -33843704;    int bHDMbiohzA18745367 = -200638620;    int bHDMbiohzA1710058 = -330237060;    int bHDMbiohzA33886256 = -223468655;    int bHDMbiohzA25732543 = -508509992;    int bHDMbiohzA67438058 = -928780184;    int bHDMbiohzA34639513 = -715578637;    int bHDMbiohzA16266878 = -629263449;    int bHDMbiohzA32537761 = -285762921;    int bHDMbiohzA30030657 = -654930124;    int bHDMbiohzA89942117 = -161561149;     bHDMbiohzA24228482 = bHDMbiohzA87946418;     bHDMbiohzA87946418 = bHDMbiohzA76026823;     bHDMbiohzA76026823 = bHDMbiohzA28820323;     bHDMbiohzA28820323 = bHDMbiohzA41295885;     bHDMbiohzA41295885 = bHDMbiohzA74416310;     bHDMbiohzA74416310 = bHDMbiohzA83290371;     bHDMbiohzA83290371 = bHDMbiohzA81473216;     bHDMbiohzA81473216 = bHDMbiohzA189633;     bHDMbiohzA189633 = bHDMbiohzA11808745;     bHDMbiohzA11808745 = bHDMbiohzA5832997;     bHDMbiohzA5832997 = bHDMbiohzA92597727;     bHDMbiohzA92597727 = bHDMbiohzA85928491;     bHDMbiohzA85928491 = bHDMbiohzA83428639;     bHDMbiohzA83428639 = bHDMbiohzA60908696;     bHDMbiohzA60908696 = bHDMbiohzA54049911;     bHDMbiohzA54049911 = bHDMbiohzA14520093;     bHDMbiohzA14520093 = bHDMbiohzA71485383;     bHDMbiohzA71485383 = bHDMbiohzA38383366;     bHDMbiohzA38383366 = bHDMbiohzA40573935;     bHDMbiohzA40573935 = bHDMbiohzA82942764;     bHDMbiohzA82942764 = bHDMbiohzA4489638;     bHDMbiohzA4489638 = bHDMbiohzA44895204;     bHDMbiohzA44895204 = bHDMbiohzA98193139;     bHDMbiohzA98193139 = bHDMbiohzA70137036;     bHDMbiohzA70137036 = bHDMbiohzA73688404;     bHDMbiohzA73688404 = bHDMbiohzA15804700;     bHDMbiohzA15804700 = bHDMbiohzA55870233;     bHDMbiohzA55870233 = bHDMbiohzA81106616;     bHDMbiohzA81106616 = bHDMbiohzA98009874;     bHDMbiohzA98009874 = bHDMbiohzA73409736;     bHDMbiohzA73409736 = bHDMbiohzA75324054;     bHDMbiohzA75324054 = bHDMbiohzA48445604;     bHDMbiohzA48445604 = bHDMbiohzA36220182;     bHDMbiohzA36220182 = bHDMbiohzA74863655;     bHDMbiohzA74863655 = bHDMbiohzA61796580;     bHDMbiohzA61796580 = bHDMbiohzA79543741;     bHDMbiohzA79543741 = bHDMbiohzA92569541;     bHDMbiohzA92569541 = bHDMbiohzA93340023;     bHDMbiohzA93340023 = bHDMbiohzA26049668;     bHDMbiohzA26049668 = bHDMbiohzA62059651;     bHDMbiohzA62059651 = bHDMbiohzA16633478;     bHDMbiohzA16633478 = bHDMbiohzA34717519;     bHDMbiohzA34717519 = bHDMbiohzA68429666;     bHDMbiohzA68429666 = bHDMbiohzA20451061;     bHDMbiohzA20451061 = bHDMbiohzA20373837;     bHDMbiohzA20373837 = bHDMbiohzA92142554;     bHDMbiohzA92142554 = bHDMbiohzA41829100;     bHDMbiohzA41829100 = bHDMbiohzA54622485;     bHDMbiohzA54622485 = bHDMbiohzA86078053;     bHDMbiohzA86078053 = bHDMbiohzA74500247;     bHDMbiohzA74500247 = bHDMbiohzA85418928;     bHDMbiohzA85418928 = bHDMbiohzA37061256;     bHDMbiohzA37061256 = bHDMbiohzA94806099;     bHDMbiohzA94806099 = bHDMbiohzA62503725;     bHDMbiohzA62503725 = bHDMbiohzA19738845;     bHDMbiohzA19738845 = bHDMbiohzA43051214;     bHDMbiohzA43051214 = bHDMbiohzA77833684;     bHDMbiohzA77833684 = bHDMbiohzA58683286;     bHDMbiohzA58683286 = bHDMbiohzA67607481;     bHDMbiohzA67607481 = bHDMbiohzA58611610;     bHDMbiohzA58611610 = bHDMbiohzA27420139;     bHDMbiohzA27420139 = bHDMbiohzA366601;     bHDMbiohzA366601 = bHDMbiohzA2179759;     bHDMbiohzA2179759 = bHDMbiohzA38399009;     bHDMbiohzA38399009 = bHDMbiohzA30508943;     bHDMbiohzA30508943 = bHDMbiohzA44152123;     bHDMbiohzA44152123 = bHDMbiohzA49708310;     bHDMbiohzA49708310 = bHDMbiohzA8564984;     bHDMbiohzA8564984 = bHDMbiohzA99112115;     bHDMbiohzA99112115 = bHDMbiohzA74506170;     bHDMbiohzA74506170 = bHDMbiohzA21950551;     bHDMbiohzA21950551 = bHDMbiohzA78145360;     bHDMbiohzA78145360 = bHDMbiohzA12333698;     bHDMbiohzA12333698 = bHDMbiohzA78514283;     bHDMbiohzA78514283 = bHDMbiohzA66309287;     bHDMbiohzA66309287 = bHDMbiohzA69772118;     bHDMbiohzA69772118 = bHDMbiohzA76465538;     bHDMbiohzA76465538 = bHDMbiohzA77742078;     bHDMbiohzA77742078 = bHDMbiohzA49763199;     bHDMbiohzA49763199 = bHDMbiohzA81545849;     bHDMbiohzA81545849 = bHDMbiohzA73975599;     bHDMbiohzA73975599 = bHDMbiohzA1247748;     bHDMbiohzA1247748 = bHDMbiohzA95028563;     bHDMbiohzA95028563 = bHDMbiohzA23509627;     bHDMbiohzA23509627 = bHDMbiohzA87990807;     bHDMbiohzA87990807 = bHDMbiohzA38262798;     bHDMbiohzA38262798 = bHDMbiohzA53639505;     bHDMbiohzA53639505 = bHDMbiohzA73716456;     bHDMbiohzA73716456 = bHDMbiohzA55124811;     bHDMbiohzA55124811 = bHDMbiohzA18745367;     bHDMbiohzA18745367 = bHDMbiohzA1710058;     bHDMbiohzA1710058 = bHDMbiohzA33886256;     bHDMbiohzA33886256 = bHDMbiohzA25732543;     bHDMbiohzA25732543 = bHDMbiohzA67438058;     bHDMbiohzA67438058 = bHDMbiohzA34639513;     bHDMbiohzA34639513 = bHDMbiohzA16266878;     bHDMbiohzA16266878 = bHDMbiohzA32537761;     bHDMbiohzA32537761 = bHDMbiohzA30030657;     bHDMbiohzA30030657 = bHDMbiohzA89942117;     bHDMbiohzA89942117 = bHDMbiohzA24228482;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iBvJlUVOoJ15902364() {     int TzXHFZsHlg37014650 = -296601841;    int TzXHFZsHlg74717306 = -703557331;    int TzXHFZsHlg93779623 = 57545899;    int TzXHFZsHlg36761148 = -213588217;    int TzXHFZsHlg34806990 = -264525751;    int TzXHFZsHlg17726381 = -198876651;    int TzXHFZsHlg4498130 = -365662816;    int TzXHFZsHlg8192330 = -72515122;    int TzXHFZsHlg95594443 = -782548119;    int TzXHFZsHlg18027450 = -845948811;    int TzXHFZsHlg33320379 = -565052025;    int TzXHFZsHlg24548232 = -774890384;    int TzXHFZsHlg56071795 = -902774999;    int TzXHFZsHlg33052718 = -134294247;    int TzXHFZsHlg84215249 = 39692115;    int TzXHFZsHlg82474980 = -619895279;    int TzXHFZsHlg70271910 = -375878169;    int TzXHFZsHlg55043869 = -496491894;    int TzXHFZsHlg39543996 = -44097751;    int TzXHFZsHlg43228431 = 55246082;    int TzXHFZsHlg63116821 = -476608674;    int TzXHFZsHlg33537457 = -174219542;    int TzXHFZsHlg83655499 = -95093496;    int TzXHFZsHlg42700269 = -772503107;    int TzXHFZsHlg76031295 = -326407684;    int TzXHFZsHlg55007049 = -989673957;    int TzXHFZsHlg27220236 = -664999036;    int TzXHFZsHlg23223158 = -597257113;    int TzXHFZsHlg97723669 = -720365199;    int TzXHFZsHlg50760990 = -147263678;    int TzXHFZsHlg17845732 = -725950245;    int TzXHFZsHlg78642877 = -569141741;    int TzXHFZsHlg94491552 = -365071087;    int TzXHFZsHlg13296555 = 61045352;    int TzXHFZsHlg57924188 = -801075539;    int TzXHFZsHlg57020695 = -358335440;    int TzXHFZsHlg16307802 = -342083035;    int TzXHFZsHlg50273565 = -416508926;    int TzXHFZsHlg99115409 = 76172902;    int TzXHFZsHlg52314397 = -63857611;    int TzXHFZsHlg23490960 = -487584368;    int TzXHFZsHlg24335934 = -749423690;    int TzXHFZsHlg69600988 = -612608869;    int TzXHFZsHlg13058204 = -488430051;    int TzXHFZsHlg459551 = -836031034;    int TzXHFZsHlg94001966 = -499561839;    int TzXHFZsHlg69724574 = -379712081;    int TzXHFZsHlg71269066 = -880724918;    int TzXHFZsHlg98686534 = -727221955;    int TzXHFZsHlg33028474 = -700213938;    int TzXHFZsHlg92827740 = -59745968;    int TzXHFZsHlg13595476 = -250968095;    int TzXHFZsHlg71546406 = -583459363;    int TzXHFZsHlg39505825 = -813604741;    int TzXHFZsHlg52785542 = 80211216;    int TzXHFZsHlg3477193 = -22382299;    int TzXHFZsHlg91061806 = -508463835;    int TzXHFZsHlg51079354 = -169950994;    int TzXHFZsHlg60729852 = -887180533;    int TzXHFZsHlg79799940 = -274851795;    int TzXHFZsHlg90506144 = -533877615;    int TzXHFZsHlg81274971 = -768405704;    int TzXHFZsHlg10468660 = -352149924;    int TzXHFZsHlg44833454 = -535284441;    int TzXHFZsHlg181719 = -19998567;    int TzXHFZsHlg54677501 = -995910285;    int TzXHFZsHlg30056679 = -309819297;    int TzXHFZsHlg42775240 = -863820352;    int TzXHFZsHlg75128530 = -333218708;    int TzXHFZsHlg27194554 = -601972446;    int TzXHFZsHlg66167178 = -177812244;    int TzXHFZsHlg19998346 = -959369243;    int TzXHFZsHlg55928459 = -472664797;    int TzXHFZsHlg87229598 = -980240140;    int TzXHFZsHlg19737472 = -457169551;    int TzXHFZsHlg38780888 = -727184984;    int TzXHFZsHlg63936468 = -561610674;    int TzXHFZsHlg70597296 = -606663446;    int TzXHFZsHlg42240718 = -936472073;    int TzXHFZsHlg82029329 = -826845845;    int TzXHFZsHlg85282475 = -509961876;    int TzXHFZsHlg55951169 = -784274118;    int TzXHFZsHlg24536624 = -870035159;    int TzXHFZsHlg64695196 = 79848738;    int TzXHFZsHlg57933249 = 12482290;    int TzXHFZsHlg4250257 = -374982150;    int TzXHFZsHlg7096472 = -985682379;    int TzXHFZsHlg54985728 = -551466346;    int TzXHFZsHlg60511013 = 80834136;    int TzXHFZsHlg54446995 = -678693240;    int TzXHFZsHlg65958889 = -849871606;    int TzXHFZsHlg65228447 = -72132041;    int TzXHFZsHlg89543712 = -529328394;    int TzXHFZsHlg19315469 = -648975304;    int TzXHFZsHlg61808253 = -529979997;    int TzXHFZsHlg42215988 = -719178664;    int TzXHFZsHlg13867274 = -297273767;    int TzXHFZsHlg24767535 = 22675572;    int TzXHFZsHlg12876486 = -368431485;    int TzXHFZsHlg45782049 = -296601841;     TzXHFZsHlg37014650 = TzXHFZsHlg74717306;     TzXHFZsHlg74717306 = TzXHFZsHlg93779623;     TzXHFZsHlg93779623 = TzXHFZsHlg36761148;     TzXHFZsHlg36761148 = TzXHFZsHlg34806990;     TzXHFZsHlg34806990 = TzXHFZsHlg17726381;     TzXHFZsHlg17726381 = TzXHFZsHlg4498130;     TzXHFZsHlg4498130 = TzXHFZsHlg8192330;     TzXHFZsHlg8192330 = TzXHFZsHlg95594443;     TzXHFZsHlg95594443 = TzXHFZsHlg18027450;     TzXHFZsHlg18027450 = TzXHFZsHlg33320379;     TzXHFZsHlg33320379 = TzXHFZsHlg24548232;     TzXHFZsHlg24548232 = TzXHFZsHlg56071795;     TzXHFZsHlg56071795 = TzXHFZsHlg33052718;     TzXHFZsHlg33052718 = TzXHFZsHlg84215249;     TzXHFZsHlg84215249 = TzXHFZsHlg82474980;     TzXHFZsHlg82474980 = TzXHFZsHlg70271910;     TzXHFZsHlg70271910 = TzXHFZsHlg55043869;     TzXHFZsHlg55043869 = TzXHFZsHlg39543996;     TzXHFZsHlg39543996 = TzXHFZsHlg43228431;     TzXHFZsHlg43228431 = TzXHFZsHlg63116821;     TzXHFZsHlg63116821 = TzXHFZsHlg33537457;     TzXHFZsHlg33537457 = TzXHFZsHlg83655499;     TzXHFZsHlg83655499 = TzXHFZsHlg42700269;     TzXHFZsHlg42700269 = TzXHFZsHlg76031295;     TzXHFZsHlg76031295 = TzXHFZsHlg55007049;     TzXHFZsHlg55007049 = TzXHFZsHlg27220236;     TzXHFZsHlg27220236 = TzXHFZsHlg23223158;     TzXHFZsHlg23223158 = TzXHFZsHlg97723669;     TzXHFZsHlg97723669 = TzXHFZsHlg50760990;     TzXHFZsHlg50760990 = TzXHFZsHlg17845732;     TzXHFZsHlg17845732 = TzXHFZsHlg78642877;     TzXHFZsHlg78642877 = TzXHFZsHlg94491552;     TzXHFZsHlg94491552 = TzXHFZsHlg13296555;     TzXHFZsHlg13296555 = TzXHFZsHlg57924188;     TzXHFZsHlg57924188 = TzXHFZsHlg57020695;     TzXHFZsHlg57020695 = TzXHFZsHlg16307802;     TzXHFZsHlg16307802 = TzXHFZsHlg50273565;     TzXHFZsHlg50273565 = TzXHFZsHlg99115409;     TzXHFZsHlg99115409 = TzXHFZsHlg52314397;     TzXHFZsHlg52314397 = TzXHFZsHlg23490960;     TzXHFZsHlg23490960 = TzXHFZsHlg24335934;     TzXHFZsHlg24335934 = TzXHFZsHlg69600988;     TzXHFZsHlg69600988 = TzXHFZsHlg13058204;     TzXHFZsHlg13058204 = TzXHFZsHlg459551;     TzXHFZsHlg459551 = TzXHFZsHlg94001966;     TzXHFZsHlg94001966 = TzXHFZsHlg69724574;     TzXHFZsHlg69724574 = TzXHFZsHlg71269066;     TzXHFZsHlg71269066 = TzXHFZsHlg98686534;     TzXHFZsHlg98686534 = TzXHFZsHlg33028474;     TzXHFZsHlg33028474 = TzXHFZsHlg92827740;     TzXHFZsHlg92827740 = TzXHFZsHlg13595476;     TzXHFZsHlg13595476 = TzXHFZsHlg71546406;     TzXHFZsHlg71546406 = TzXHFZsHlg39505825;     TzXHFZsHlg39505825 = TzXHFZsHlg52785542;     TzXHFZsHlg52785542 = TzXHFZsHlg3477193;     TzXHFZsHlg3477193 = TzXHFZsHlg91061806;     TzXHFZsHlg91061806 = TzXHFZsHlg51079354;     TzXHFZsHlg51079354 = TzXHFZsHlg60729852;     TzXHFZsHlg60729852 = TzXHFZsHlg79799940;     TzXHFZsHlg79799940 = TzXHFZsHlg90506144;     TzXHFZsHlg90506144 = TzXHFZsHlg81274971;     TzXHFZsHlg81274971 = TzXHFZsHlg10468660;     TzXHFZsHlg10468660 = TzXHFZsHlg44833454;     TzXHFZsHlg44833454 = TzXHFZsHlg181719;     TzXHFZsHlg181719 = TzXHFZsHlg54677501;     TzXHFZsHlg54677501 = TzXHFZsHlg30056679;     TzXHFZsHlg30056679 = TzXHFZsHlg42775240;     TzXHFZsHlg42775240 = TzXHFZsHlg75128530;     TzXHFZsHlg75128530 = TzXHFZsHlg27194554;     TzXHFZsHlg27194554 = TzXHFZsHlg66167178;     TzXHFZsHlg66167178 = TzXHFZsHlg19998346;     TzXHFZsHlg19998346 = TzXHFZsHlg55928459;     TzXHFZsHlg55928459 = TzXHFZsHlg87229598;     TzXHFZsHlg87229598 = TzXHFZsHlg19737472;     TzXHFZsHlg19737472 = TzXHFZsHlg38780888;     TzXHFZsHlg38780888 = TzXHFZsHlg63936468;     TzXHFZsHlg63936468 = TzXHFZsHlg70597296;     TzXHFZsHlg70597296 = TzXHFZsHlg42240718;     TzXHFZsHlg42240718 = TzXHFZsHlg82029329;     TzXHFZsHlg82029329 = TzXHFZsHlg85282475;     TzXHFZsHlg85282475 = TzXHFZsHlg55951169;     TzXHFZsHlg55951169 = TzXHFZsHlg24536624;     TzXHFZsHlg24536624 = TzXHFZsHlg64695196;     TzXHFZsHlg64695196 = TzXHFZsHlg57933249;     TzXHFZsHlg57933249 = TzXHFZsHlg4250257;     TzXHFZsHlg4250257 = TzXHFZsHlg7096472;     TzXHFZsHlg7096472 = TzXHFZsHlg54985728;     TzXHFZsHlg54985728 = TzXHFZsHlg60511013;     TzXHFZsHlg60511013 = TzXHFZsHlg54446995;     TzXHFZsHlg54446995 = TzXHFZsHlg65958889;     TzXHFZsHlg65958889 = TzXHFZsHlg65228447;     TzXHFZsHlg65228447 = TzXHFZsHlg89543712;     TzXHFZsHlg89543712 = TzXHFZsHlg19315469;     TzXHFZsHlg19315469 = TzXHFZsHlg61808253;     TzXHFZsHlg61808253 = TzXHFZsHlg42215988;     TzXHFZsHlg42215988 = TzXHFZsHlg13867274;     TzXHFZsHlg13867274 = TzXHFZsHlg24767535;     TzXHFZsHlg24767535 = TzXHFZsHlg12876486;     TzXHFZsHlg12876486 = TzXHFZsHlg45782049;     TzXHFZsHlg45782049 = TzXHFZsHlg37014650;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZqeZywdoJW34115705() {     int OMEpIsUNXP20458955 = -77267521;    int OMEpIsUNXP91589395 = -702939519;    int OMEpIsUNXP90188129 = -835979329;    int OMEpIsUNXP99553399 = -407388052;    int OMEpIsUNXP40928314 = -71393242;    int OMEpIsUNXP16230782 = 22051696;    int OMEpIsUNXP75537447 = -105574130;    int OMEpIsUNXP22539772 = -59673152;    int OMEpIsUNXP54124144 = -24007469;    int OMEpIsUNXP50573102 = -214324923;    int OMEpIsUNXP82736316 = -755167898;    int OMEpIsUNXP87753631 = -762508630;    int OMEpIsUNXP74189570 = -694436240;    int OMEpIsUNXP12679610 = 77415396;    int OMEpIsUNXP45069713 = -54330867;    int OMEpIsUNXP68864142 = -407974604;    int OMEpIsUNXP64023795 = -129223196;    int OMEpIsUNXP35895315 = -743796770;    int OMEpIsUNXP1984139 = -42772622;    int OMEpIsUNXP32595874 = -474339910;    int OMEpIsUNXP18757209 = -66137705;    int OMEpIsUNXP45684775 = -165371313;    int OMEpIsUNXP87481207 = -134982694;    int OMEpIsUNXP77406827 = -844328503;    int OMEpIsUNXP17607254 = -886060443;    int OMEpIsUNXP50349198 = -870183672;    int OMEpIsUNXP76674976 = -293083379;    int OMEpIsUNXP62954783 = -667211997;    int OMEpIsUNXP33804764 = -582870341;    int OMEpIsUNXP66594331 = -660113529;    int OMEpIsUNXP35941046 = -363707446;    int OMEpIsUNXP41749305 = -484336821;    int OMEpIsUNXP38030440 = 14056735;    int OMEpIsUNXP55455065 = -630556818;    int OMEpIsUNXP36650918 = -998740274;    int OMEpIsUNXP69409879 = -146359346;    int OMEpIsUNXP84921980 = -220932215;    int OMEpIsUNXP90852396 = -124979447;    int OMEpIsUNXP48110488 = -108724895;    int OMEpIsUNXP1809533 = -916451134;    int OMEpIsUNXP57139884 = -59133341;    int OMEpIsUNXP74823720 = -223332011;    int OMEpIsUNXP93168058 = -63666112;    int OMEpIsUNXP4963144 = -244612822;    int OMEpIsUNXP92404047 = -564187988;    int OMEpIsUNXP33607963 = -519172763;    int OMEpIsUNXP45740004 = -512016311;    int OMEpIsUNXP90939662 = -879446637;    int OMEpIsUNXP29533647 = -212824037;    int OMEpIsUNXP68360958 = -907234393;    int OMEpIsUNXP66085189 = -280647390;    int OMEpIsUNXP69096270 = -309227407;    int OMEpIsUNXP85658864 = -795206679;    int OMEpIsUNXP64182583 = -532018129;    int OMEpIsUNXP15358496 = -268274907;    int OMEpIsUNXP74774180 = -911896209;    int OMEpIsUNXP4108188 = -467956825;    int OMEpIsUNXP12781303 = -991650826;    int OMEpIsUNXP81946145 = -521327610;    int OMEpIsUNXP90579115 = -201209570;    int OMEpIsUNXP39555805 = -684864926;    int OMEpIsUNXP12582665 = -438362133;    int OMEpIsUNXP88735008 = -476802811;    int OMEpIsUNXP87529813 = -363893941;    int OMEpIsUNXP14632057 = -850617478;    int OMEpIsUNXP40987012 = -170831077;    int OMEpIsUNXP49723192 = -676565366;    int OMEpIsUNXP18734506 = 36120577;    int OMEpIsUNXP76028692 = 76155670;    int OMEpIsUNXP75659833 = -907971522;    int OMEpIsUNXP83942161 = -87042389;    int OMEpIsUNXP73171398 = 95756250;    int OMEpIsUNXP87784827 = -535071875;    int OMEpIsUNXP174606 = -126321488;    int OMEpIsUNXP75455989 = -315206570;    int OMEpIsUNXP43933489 = -842805694;    int OMEpIsUNXP52516716 = -1705201;    int OMEpIsUNXP82518063 = -890369873;    int OMEpIsUNXP85002779 = -180140516;    int OMEpIsUNXP83999291 = -266887680;    int OMEpIsUNXP4609195 = -258167362;    int OMEpIsUNXP85735314 = -413636742;    int OMEpIsUNXP33421137 = -354387960;    int OMEpIsUNXP65443806 = -675635949;    int OMEpIsUNXP509143 = -279466140;    int OMEpIsUNXP66844775 = 45519961;    int OMEpIsUNXP56090440 = -689130142;    int OMEpIsUNXP73847856 = -453925136;    int OMEpIsUNXP40096569 = -262281911;    int OMEpIsUNXP61876738 = 13155935;    int OMEpIsUNXP65301691 = -678402521;    int OMEpIsUNXP72140677 = -229281389;    int OMEpIsUNXP8906251 = -603651837;    int OMEpIsUNXP57531372 = -907515326;    int OMEpIsUNXP62253727 = -131586208;    int OMEpIsUNXP44557220 = -620771208;    int OMEpIsUNXP86088711 = -746529201;    int OMEpIsUNXP5638246 = -699772172;    int OMEpIsUNXP90331087 = -393995344;    int OMEpIsUNXP51417035 = -77267521;     OMEpIsUNXP20458955 = OMEpIsUNXP91589395;     OMEpIsUNXP91589395 = OMEpIsUNXP90188129;     OMEpIsUNXP90188129 = OMEpIsUNXP99553399;     OMEpIsUNXP99553399 = OMEpIsUNXP40928314;     OMEpIsUNXP40928314 = OMEpIsUNXP16230782;     OMEpIsUNXP16230782 = OMEpIsUNXP75537447;     OMEpIsUNXP75537447 = OMEpIsUNXP22539772;     OMEpIsUNXP22539772 = OMEpIsUNXP54124144;     OMEpIsUNXP54124144 = OMEpIsUNXP50573102;     OMEpIsUNXP50573102 = OMEpIsUNXP82736316;     OMEpIsUNXP82736316 = OMEpIsUNXP87753631;     OMEpIsUNXP87753631 = OMEpIsUNXP74189570;     OMEpIsUNXP74189570 = OMEpIsUNXP12679610;     OMEpIsUNXP12679610 = OMEpIsUNXP45069713;     OMEpIsUNXP45069713 = OMEpIsUNXP68864142;     OMEpIsUNXP68864142 = OMEpIsUNXP64023795;     OMEpIsUNXP64023795 = OMEpIsUNXP35895315;     OMEpIsUNXP35895315 = OMEpIsUNXP1984139;     OMEpIsUNXP1984139 = OMEpIsUNXP32595874;     OMEpIsUNXP32595874 = OMEpIsUNXP18757209;     OMEpIsUNXP18757209 = OMEpIsUNXP45684775;     OMEpIsUNXP45684775 = OMEpIsUNXP87481207;     OMEpIsUNXP87481207 = OMEpIsUNXP77406827;     OMEpIsUNXP77406827 = OMEpIsUNXP17607254;     OMEpIsUNXP17607254 = OMEpIsUNXP50349198;     OMEpIsUNXP50349198 = OMEpIsUNXP76674976;     OMEpIsUNXP76674976 = OMEpIsUNXP62954783;     OMEpIsUNXP62954783 = OMEpIsUNXP33804764;     OMEpIsUNXP33804764 = OMEpIsUNXP66594331;     OMEpIsUNXP66594331 = OMEpIsUNXP35941046;     OMEpIsUNXP35941046 = OMEpIsUNXP41749305;     OMEpIsUNXP41749305 = OMEpIsUNXP38030440;     OMEpIsUNXP38030440 = OMEpIsUNXP55455065;     OMEpIsUNXP55455065 = OMEpIsUNXP36650918;     OMEpIsUNXP36650918 = OMEpIsUNXP69409879;     OMEpIsUNXP69409879 = OMEpIsUNXP84921980;     OMEpIsUNXP84921980 = OMEpIsUNXP90852396;     OMEpIsUNXP90852396 = OMEpIsUNXP48110488;     OMEpIsUNXP48110488 = OMEpIsUNXP1809533;     OMEpIsUNXP1809533 = OMEpIsUNXP57139884;     OMEpIsUNXP57139884 = OMEpIsUNXP74823720;     OMEpIsUNXP74823720 = OMEpIsUNXP93168058;     OMEpIsUNXP93168058 = OMEpIsUNXP4963144;     OMEpIsUNXP4963144 = OMEpIsUNXP92404047;     OMEpIsUNXP92404047 = OMEpIsUNXP33607963;     OMEpIsUNXP33607963 = OMEpIsUNXP45740004;     OMEpIsUNXP45740004 = OMEpIsUNXP90939662;     OMEpIsUNXP90939662 = OMEpIsUNXP29533647;     OMEpIsUNXP29533647 = OMEpIsUNXP68360958;     OMEpIsUNXP68360958 = OMEpIsUNXP66085189;     OMEpIsUNXP66085189 = OMEpIsUNXP69096270;     OMEpIsUNXP69096270 = OMEpIsUNXP85658864;     OMEpIsUNXP85658864 = OMEpIsUNXP64182583;     OMEpIsUNXP64182583 = OMEpIsUNXP15358496;     OMEpIsUNXP15358496 = OMEpIsUNXP74774180;     OMEpIsUNXP74774180 = OMEpIsUNXP4108188;     OMEpIsUNXP4108188 = OMEpIsUNXP12781303;     OMEpIsUNXP12781303 = OMEpIsUNXP81946145;     OMEpIsUNXP81946145 = OMEpIsUNXP90579115;     OMEpIsUNXP90579115 = OMEpIsUNXP39555805;     OMEpIsUNXP39555805 = OMEpIsUNXP12582665;     OMEpIsUNXP12582665 = OMEpIsUNXP88735008;     OMEpIsUNXP88735008 = OMEpIsUNXP87529813;     OMEpIsUNXP87529813 = OMEpIsUNXP14632057;     OMEpIsUNXP14632057 = OMEpIsUNXP40987012;     OMEpIsUNXP40987012 = OMEpIsUNXP49723192;     OMEpIsUNXP49723192 = OMEpIsUNXP18734506;     OMEpIsUNXP18734506 = OMEpIsUNXP76028692;     OMEpIsUNXP76028692 = OMEpIsUNXP75659833;     OMEpIsUNXP75659833 = OMEpIsUNXP83942161;     OMEpIsUNXP83942161 = OMEpIsUNXP73171398;     OMEpIsUNXP73171398 = OMEpIsUNXP87784827;     OMEpIsUNXP87784827 = OMEpIsUNXP174606;     OMEpIsUNXP174606 = OMEpIsUNXP75455989;     OMEpIsUNXP75455989 = OMEpIsUNXP43933489;     OMEpIsUNXP43933489 = OMEpIsUNXP52516716;     OMEpIsUNXP52516716 = OMEpIsUNXP82518063;     OMEpIsUNXP82518063 = OMEpIsUNXP85002779;     OMEpIsUNXP85002779 = OMEpIsUNXP83999291;     OMEpIsUNXP83999291 = OMEpIsUNXP4609195;     OMEpIsUNXP4609195 = OMEpIsUNXP85735314;     OMEpIsUNXP85735314 = OMEpIsUNXP33421137;     OMEpIsUNXP33421137 = OMEpIsUNXP65443806;     OMEpIsUNXP65443806 = OMEpIsUNXP509143;     OMEpIsUNXP509143 = OMEpIsUNXP66844775;     OMEpIsUNXP66844775 = OMEpIsUNXP56090440;     OMEpIsUNXP56090440 = OMEpIsUNXP73847856;     OMEpIsUNXP73847856 = OMEpIsUNXP40096569;     OMEpIsUNXP40096569 = OMEpIsUNXP61876738;     OMEpIsUNXP61876738 = OMEpIsUNXP65301691;     OMEpIsUNXP65301691 = OMEpIsUNXP72140677;     OMEpIsUNXP72140677 = OMEpIsUNXP8906251;     OMEpIsUNXP8906251 = OMEpIsUNXP57531372;     OMEpIsUNXP57531372 = OMEpIsUNXP62253727;     OMEpIsUNXP62253727 = OMEpIsUNXP44557220;     OMEpIsUNXP44557220 = OMEpIsUNXP86088711;     OMEpIsUNXP86088711 = OMEpIsUNXP5638246;     OMEpIsUNXP5638246 = OMEpIsUNXP90331087;     OMEpIsUNXP90331087 = OMEpIsUNXP51417035;     OMEpIsUNXP51417035 = OMEpIsUNXP20458955;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UVQFykmKJE34557808() {     int CRzqyHILdN55638711 = 25811965;    int CRzqyHILdN24135468 = -779221462;    int CRzqyHILdN35396996 = -426863941;    int CRzqyHILdN37421053 = -417457031;    int CRzqyHILdN23773571 = -685911387;    int CRzqyHILdN39618537 = 83952817;    int CRzqyHILdN20353609 = -768697039;    int CRzqyHILdN55678693 = -170610736;    int CRzqyHILdN63951921 = -156315531;    int CRzqyHILdN51915878 = -124347018;    int CRzqyHILdN27444953 = -907006242;    int CRzqyHILdN61315408 = -685108377;    int CRzqyHILdN79488140 = -95277743;    int CRzqyHILdN51074221 = -437555507;    int CRzqyHILdN59939434 = -726081559;    int CRzqyHILdN66980576 = 92262115;    int CRzqyHILdN13746205 = -250662913;    int CRzqyHILdN2429684 = -353678141;    int CRzqyHILdN23998130 = -569361676;    int CRzqyHILdN41692890 = -714194052;    int CRzqyHILdN20144949 = -838474904;    int CRzqyHILdN60251546 = -330004091;    int CRzqyHILdN92656429 = -883109338;    int CRzqyHILdN4763636 = -787745850;    int CRzqyHILdN30599071 = -291367639;    int CRzqyHILdN3396762 = 3696833;    int CRzqyHILdN64958351 = -383576439;    int CRzqyHILdN815634 = -20497567;    int CRzqyHILdN44635931 = -495274386;    int CRzqyHILdN52412756 = -534864177;    int CRzqyHILdN91255168 = -334347113;    int CRzqyHILdN17885462 = -723306092;    int CRzqyHILdN79662636 = -189197328;    int CRzqyHILdN91637287 = -982756153;    int CRzqyHILdN13582777 = -313214652;    int CRzqyHILdN63498674 = -170552081;    int CRzqyHILdN3184386 = -744136139;    int CRzqyHILdN32264841 = 83037669;    int CRzqyHILdN66880408 = -738940088;    int CRzqyHILdN76387084 = -187660323;    int CRzqyHILdN59472895 = -849841102;    int CRzqyHILdN85336906 = -785759954;    int CRzqyHILdN73318360 = -515499886;    int CRzqyHILdN89803902 = 73815730;    int CRzqyHILdN56377752 = -892842024;    int CRzqyHILdN74908405 = -962675627;    int CRzqyHILdN7921629 = -6660994;    int CRzqyHILdN89594722 = -182573051;    int CRzqyHILdN49399212 = -762378849;    int CRzqyHILdN92349701 = -973266229;    int CRzqyHILdN80022529 = -210052968;    int CRzqyHILdN56576817 = -829086279;    int CRzqyHILdN23010070 = -751067184;    int CRzqyHILdN58918583 = -965230828;    int CRzqyHILdN14412860 = -646524231;    int CRzqyHILdN95387164 = -644183945;    int CRzqyHILdN31479038 = -896112124;    int CRzqyHILdN30633361 = -639118091;    int CRzqyHILdN6821983 = -26089393;    int CRzqyHILdN20376809 = -589608220;    int CRzqyHILdN74660185 = -532470745;    int CRzqyHILdN19537975 = -648199473;    int CRzqyHILdN11042763 = -675336350;    int CRzqyHILdN11539166 = -621451354;    int CRzqyHILdN60660710 = -789999905;    int CRzqyHILdN9559492 = -83700150;    int CRzqyHILdN81652772 = -395911049;    int CRzqyHILdN87850852 = -112521590;    int CRzqyHILdN37491444 = -24340856;    int CRzqyHILdN96440759 = -455529479;    int CRzqyHILdN63796191 = -163601747;    int CRzqyHILdN81481364 = -233700582;    int CRzqyHILdN35549275 = -614738054;    int CRzqyHILdN47611046 = -281701354;    int CRzqyHILdN82219995 = -864352950;    int CRzqyHILdN34808043 = 47285050;    int CRzqyHILdN86933186 = -814504205;    int CRzqyHILdN2852527 = -856925069;    int CRzqyHILdN48385883 = -894903827;    int CRzqyHILdN55690665 = -328692012;    int CRzqyHILdN95475132 = -989642174;    int CRzqyHILdN75363629 = -101003388;    int CRzqyHILdN51416422 = -258118718;    int CRzqyHILdN52286230 = -522008157;    int CRzqyHILdN72390227 = -224811209;    int CRzqyHILdN34678351 = -505260835;    int CRzqyHILdN94875391 = -972238909;    int CRzqyHILdN20744053 = -223966500;    int CRzqyHILdN77224428 = -236231923;    int CRzqyHILdN18195613 = -669030708;    int CRzqyHILdN32019636 = -274439957;    int CRzqyHILdN72551024 = -5018048;    int CRzqyHILdN25442859 = -890872939;    int CRzqyHILdN46503599 = -49331868;    int CRzqyHILdN1726899 = -655189578;    int CRzqyHILdN39934920 = -101641629;    int CRzqyHILdN74294143 = -10423605;    int CRzqyHILdN61779195 = -894048532;    int CRzqyHILdN29143193 = -136184365;    int CRzqyHILdN46818261 = 25811965;     CRzqyHILdN55638711 = CRzqyHILdN24135468;     CRzqyHILdN24135468 = CRzqyHILdN35396996;     CRzqyHILdN35396996 = CRzqyHILdN37421053;     CRzqyHILdN37421053 = CRzqyHILdN23773571;     CRzqyHILdN23773571 = CRzqyHILdN39618537;     CRzqyHILdN39618537 = CRzqyHILdN20353609;     CRzqyHILdN20353609 = CRzqyHILdN55678693;     CRzqyHILdN55678693 = CRzqyHILdN63951921;     CRzqyHILdN63951921 = CRzqyHILdN51915878;     CRzqyHILdN51915878 = CRzqyHILdN27444953;     CRzqyHILdN27444953 = CRzqyHILdN61315408;     CRzqyHILdN61315408 = CRzqyHILdN79488140;     CRzqyHILdN79488140 = CRzqyHILdN51074221;     CRzqyHILdN51074221 = CRzqyHILdN59939434;     CRzqyHILdN59939434 = CRzqyHILdN66980576;     CRzqyHILdN66980576 = CRzqyHILdN13746205;     CRzqyHILdN13746205 = CRzqyHILdN2429684;     CRzqyHILdN2429684 = CRzqyHILdN23998130;     CRzqyHILdN23998130 = CRzqyHILdN41692890;     CRzqyHILdN41692890 = CRzqyHILdN20144949;     CRzqyHILdN20144949 = CRzqyHILdN60251546;     CRzqyHILdN60251546 = CRzqyHILdN92656429;     CRzqyHILdN92656429 = CRzqyHILdN4763636;     CRzqyHILdN4763636 = CRzqyHILdN30599071;     CRzqyHILdN30599071 = CRzqyHILdN3396762;     CRzqyHILdN3396762 = CRzqyHILdN64958351;     CRzqyHILdN64958351 = CRzqyHILdN815634;     CRzqyHILdN815634 = CRzqyHILdN44635931;     CRzqyHILdN44635931 = CRzqyHILdN52412756;     CRzqyHILdN52412756 = CRzqyHILdN91255168;     CRzqyHILdN91255168 = CRzqyHILdN17885462;     CRzqyHILdN17885462 = CRzqyHILdN79662636;     CRzqyHILdN79662636 = CRzqyHILdN91637287;     CRzqyHILdN91637287 = CRzqyHILdN13582777;     CRzqyHILdN13582777 = CRzqyHILdN63498674;     CRzqyHILdN63498674 = CRzqyHILdN3184386;     CRzqyHILdN3184386 = CRzqyHILdN32264841;     CRzqyHILdN32264841 = CRzqyHILdN66880408;     CRzqyHILdN66880408 = CRzqyHILdN76387084;     CRzqyHILdN76387084 = CRzqyHILdN59472895;     CRzqyHILdN59472895 = CRzqyHILdN85336906;     CRzqyHILdN85336906 = CRzqyHILdN73318360;     CRzqyHILdN73318360 = CRzqyHILdN89803902;     CRzqyHILdN89803902 = CRzqyHILdN56377752;     CRzqyHILdN56377752 = CRzqyHILdN74908405;     CRzqyHILdN74908405 = CRzqyHILdN7921629;     CRzqyHILdN7921629 = CRzqyHILdN89594722;     CRzqyHILdN89594722 = CRzqyHILdN49399212;     CRzqyHILdN49399212 = CRzqyHILdN92349701;     CRzqyHILdN92349701 = CRzqyHILdN80022529;     CRzqyHILdN80022529 = CRzqyHILdN56576817;     CRzqyHILdN56576817 = CRzqyHILdN23010070;     CRzqyHILdN23010070 = CRzqyHILdN58918583;     CRzqyHILdN58918583 = CRzqyHILdN14412860;     CRzqyHILdN14412860 = CRzqyHILdN95387164;     CRzqyHILdN95387164 = CRzqyHILdN31479038;     CRzqyHILdN31479038 = CRzqyHILdN30633361;     CRzqyHILdN30633361 = CRzqyHILdN6821983;     CRzqyHILdN6821983 = CRzqyHILdN20376809;     CRzqyHILdN20376809 = CRzqyHILdN74660185;     CRzqyHILdN74660185 = CRzqyHILdN19537975;     CRzqyHILdN19537975 = CRzqyHILdN11042763;     CRzqyHILdN11042763 = CRzqyHILdN11539166;     CRzqyHILdN11539166 = CRzqyHILdN60660710;     CRzqyHILdN60660710 = CRzqyHILdN9559492;     CRzqyHILdN9559492 = CRzqyHILdN81652772;     CRzqyHILdN81652772 = CRzqyHILdN87850852;     CRzqyHILdN87850852 = CRzqyHILdN37491444;     CRzqyHILdN37491444 = CRzqyHILdN96440759;     CRzqyHILdN96440759 = CRzqyHILdN63796191;     CRzqyHILdN63796191 = CRzqyHILdN81481364;     CRzqyHILdN81481364 = CRzqyHILdN35549275;     CRzqyHILdN35549275 = CRzqyHILdN47611046;     CRzqyHILdN47611046 = CRzqyHILdN82219995;     CRzqyHILdN82219995 = CRzqyHILdN34808043;     CRzqyHILdN34808043 = CRzqyHILdN86933186;     CRzqyHILdN86933186 = CRzqyHILdN2852527;     CRzqyHILdN2852527 = CRzqyHILdN48385883;     CRzqyHILdN48385883 = CRzqyHILdN55690665;     CRzqyHILdN55690665 = CRzqyHILdN95475132;     CRzqyHILdN95475132 = CRzqyHILdN75363629;     CRzqyHILdN75363629 = CRzqyHILdN51416422;     CRzqyHILdN51416422 = CRzqyHILdN52286230;     CRzqyHILdN52286230 = CRzqyHILdN72390227;     CRzqyHILdN72390227 = CRzqyHILdN34678351;     CRzqyHILdN34678351 = CRzqyHILdN94875391;     CRzqyHILdN94875391 = CRzqyHILdN20744053;     CRzqyHILdN20744053 = CRzqyHILdN77224428;     CRzqyHILdN77224428 = CRzqyHILdN18195613;     CRzqyHILdN18195613 = CRzqyHILdN32019636;     CRzqyHILdN32019636 = CRzqyHILdN72551024;     CRzqyHILdN72551024 = CRzqyHILdN25442859;     CRzqyHILdN25442859 = CRzqyHILdN46503599;     CRzqyHILdN46503599 = CRzqyHILdN1726899;     CRzqyHILdN1726899 = CRzqyHILdN39934920;     CRzqyHILdN39934920 = CRzqyHILdN74294143;     CRzqyHILdN74294143 = CRzqyHILdN61779195;     CRzqyHILdN61779195 = CRzqyHILdN29143193;     CRzqyHILdN29143193 = CRzqyHILdN46818261;     CRzqyHILdN46818261 = CRzqyHILdN55638711;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DjywopySBM5013681() {     int KbDepQOxFe68424878 = -109228726;    int KbDepQOxFe10906356 = -949555475;    int KbDepQOxFe53149796 = -835223323;    int KbDepQOxFe45361878 = -241659308;    int KbDepQOxFe17284675 = -77727852;    int KbDepQOxFe82928607 = -792970497;    int KbDepQOxFe41561367 = -214369110;    int KbDepQOxFe82397805 = -615128992;    int KbDepQOxFe59356732 = -118073623;    int KbDepQOxFe58134582 = 71565349;    int KbDepQOxFe54932335 = 78307743;    int KbDepQOxFe93265913 = -92441193;    int KbDepQOxFe49631443 = 939600;    int KbDepQOxFe698300 = -769970706;    int KbDepQOxFe83245987 = -633067390;    int KbDepQOxFe95405644 = -458629274;    int KbDepQOxFe69498022 = 85562620;    int KbDepQOxFe85988168 = 91111556;    int KbDepQOxFe25158760 = -478830871;    int KbDepQOxFe44347386 = -213438191;    int KbDepQOxFe319006 = -229791168;    int KbDepQOxFe89299365 = -499862438;    int KbDepQOxFe31416725 = -814113233;    int KbDepQOxFe49270765 = -48090863;    int KbDepQOxFe36493330 = -501809563;    int KbDepQOxFe84715406 = -786734366;    int KbDepQOxFe76373887 = 82717281;    int KbDepQOxFe68168558 = -194295671;    int KbDepQOxFe61252984 = -709491626;    int KbDepQOxFe5163872 = -164750332;    int KbDepQOxFe35691164 = -610763863;    int KbDepQOxFe21204286 = -702025771;    int KbDepQOxFe25708585 = -65052112;    int KbDepQOxFe68713660 = -887510372;    int KbDepQOxFe96643308 = -923246533;    int KbDepQOxFe58722789 = 40884815;    int KbDepQOxFe39948446 = -534045489;    int KbDepQOxFe89968863 = -936582423;    int KbDepQOxFe72655794 = -580790665;    int KbDepQOxFe2651814 = -32077168;    int KbDepQOxFe20904204 = -125315095;    int KbDepQOxFe93039362 = -684071289;    int KbDepQOxFe8201830 = -538933331;    int KbDepQOxFe34432441 = -267356513;    int KbDepQOxFe36386243 = -785611984;    int KbDepQOxFe48536535 = 64005933;    int KbDepQOxFe85503648 = -504176046;    int KbDepQOxFe19034689 = -337925830;    int KbDepQOxFe93463261 = -925980824;    int KbDepQOxFe39300122 = -484302692;    int KbDepQOxFe98350022 = -462937277;    int KbDepQOxFe84753364 = -200271617;    int KbDepQOxFe57495220 = -927070467;    int KbDepQOxFe3618309 = -554682570;    int KbDepQOxFe4694676 = -640968717;    int KbDepQOxFe79125512 = -609366288;    int KbDepQOxFe79489630 = -35442243;    int KbDepQOxFe3879032 = -687132461;    int KbDepQOxFe8868549 = -739849745;    int KbDepQOxFe32569268 = -290993486;    int KbDepQOxFe6554720 = -775687778;    int KbDepQOxFe73392808 = 79926561;    int KbDepQOxFe21144822 = -905637367;    int KbDepQOxFe54192860 = -953323292;    int KbDepQOxFe22443419 = -317670788;    int KbDepQOxFe33728050 = -219666487;    int KbDepQOxFe67557328 = 72610919;    int KbDepQOxFe80917782 = -111550028;    int KbDepQOxFe4054991 = -846724174;    int KbDepQOxFe24523198 = -573952206;    int KbDepQOxFe55457199 = -924583785;    int KbDepQOxFe79529158 = 22145042;    int KbDepQOxFe13332375 = -328097779;    int KbDepQOxFe22506947 = -346753703;    int KbDepQOxFe23443183 = 11876904;    int KbDepQOxFe7279644 = -545719880;    int KbDepQOxFe81097535 = -960929108;    int KbDepQOxFe96984284 = -446756721;    int KbDepQOxFe12884523 = -262478879;    int KbDepQOxFe87956794 = -465815497;    int KbDepQOxFe99211758 = -182558320;    int KbDepQOxFe57339199 = -579356889;    int KbDepQOxFe74705297 = -268314848;    int KbDepQOxFe21952862 = -125188935;    int KbDepQOxFe6813849 = -701813055;    int KbDepQOxFe50937800 = -310492247;    int KbDepQOxFe63709065 = -774955305;    int KbDepQOxFe22090277 = -510369543;    int KbDepQOxFe64018985 = -146541655;    int KbDepQOxFe17517797 = -213880245;    int KbDepQOxFe79233158 = -923672942;    int KbDepQOxFe36069414 = -846913029;    int KbDepQOxFe81100315 = -96732678;    int KbDepQOxFe40086526 = -189797180;    int KbDepQOxFe96097094 = -256389390;    int KbDepQOxFe47511395 = -105241657;    int KbDepQOxFe71894540 = -778433922;    int KbDepQOxFe54008969 = -585610039;    int KbDepQOxFe11989022 = -949685725;    int KbDepQOxFe2658193 = -109228726;     KbDepQOxFe68424878 = KbDepQOxFe10906356;     KbDepQOxFe10906356 = KbDepQOxFe53149796;     KbDepQOxFe53149796 = KbDepQOxFe45361878;     KbDepQOxFe45361878 = KbDepQOxFe17284675;     KbDepQOxFe17284675 = KbDepQOxFe82928607;     KbDepQOxFe82928607 = KbDepQOxFe41561367;     KbDepQOxFe41561367 = KbDepQOxFe82397805;     KbDepQOxFe82397805 = KbDepQOxFe59356732;     KbDepQOxFe59356732 = KbDepQOxFe58134582;     KbDepQOxFe58134582 = KbDepQOxFe54932335;     KbDepQOxFe54932335 = KbDepQOxFe93265913;     KbDepQOxFe93265913 = KbDepQOxFe49631443;     KbDepQOxFe49631443 = KbDepQOxFe698300;     KbDepQOxFe698300 = KbDepQOxFe83245987;     KbDepQOxFe83245987 = KbDepQOxFe95405644;     KbDepQOxFe95405644 = KbDepQOxFe69498022;     KbDepQOxFe69498022 = KbDepQOxFe85988168;     KbDepQOxFe85988168 = KbDepQOxFe25158760;     KbDepQOxFe25158760 = KbDepQOxFe44347386;     KbDepQOxFe44347386 = KbDepQOxFe319006;     KbDepQOxFe319006 = KbDepQOxFe89299365;     KbDepQOxFe89299365 = KbDepQOxFe31416725;     KbDepQOxFe31416725 = KbDepQOxFe49270765;     KbDepQOxFe49270765 = KbDepQOxFe36493330;     KbDepQOxFe36493330 = KbDepQOxFe84715406;     KbDepQOxFe84715406 = KbDepQOxFe76373887;     KbDepQOxFe76373887 = KbDepQOxFe68168558;     KbDepQOxFe68168558 = KbDepQOxFe61252984;     KbDepQOxFe61252984 = KbDepQOxFe5163872;     KbDepQOxFe5163872 = KbDepQOxFe35691164;     KbDepQOxFe35691164 = KbDepQOxFe21204286;     KbDepQOxFe21204286 = KbDepQOxFe25708585;     KbDepQOxFe25708585 = KbDepQOxFe68713660;     KbDepQOxFe68713660 = KbDepQOxFe96643308;     KbDepQOxFe96643308 = KbDepQOxFe58722789;     KbDepQOxFe58722789 = KbDepQOxFe39948446;     KbDepQOxFe39948446 = KbDepQOxFe89968863;     KbDepQOxFe89968863 = KbDepQOxFe72655794;     KbDepQOxFe72655794 = KbDepQOxFe2651814;     KbDepQOxFe2651814 = KbDepQOxFe20904204;     KbDepQOxFe20904204 = KbDepQOxFe93039362;     KbDepQOxFe93039362 = KbDepQOxFe8201830;     KbDepQOxFe8201830 = KbDepQOxFe34432441;     KbDepQOxFe34432441 = KbDepQOxFe36386243;     KbDepQOxFe36386243 = KbDepQOxFe48536535;     KbDepQOxFe48536535 = KbDepQOxFe85503648;     KbDepQOxFe85503648 = KbDepQOxFe19034689;     KbDepQOxFe19034689 = KbDepQOxFe93463261;     KbDepQOxFe93463261 = KbDepQOxFe39300122;     KbDepQOxFe39300122 = KbDepQOxFe98350022;     KbDepQOxFe98350022 = KbDepQOxFe84753364;     KbDepQOxFe84753364 = KbDepQOxFe57495220;     KbDepQOxFe57495220 = KbDepQOxFe3618309;     KbDepQOxFe3618309 = KbDepQOxFe4694676;     KbDepQOxFe4694676 = KbDepQOxFe79125512;     KbDepQOxFe79125512 = KbDepQOxFe79489630;     KbDepQOxFe79489630 = KbDepQOxFe3879032;     KbDepQOxFe3879032 = KbDepQOxFe8868549;     KbDepQOxFe8868549 = KbDepQOxFe32569268;     KbDepQOxFe32569268 = KbDepQOxFe6554720;     KbDepQOxFe6554720 = KbDepQOxFe73392808;     KbDepQOxFe73392808 = KbDepQOxFe21144822;     KbDepQOxFe21144822 = KbDepQOxFe54192860;     KbDepQOxFe54192860 = KbDepQOxFe22443419;     KbDepQOxFe22443419 = KbDepQOxFe33728050;     KbDepQOxFe33728050 = KbDepQOxFe67557328;     KbDepQOxFe67557328 = KbDepQOxFe80917782;     KbDepQOxFe80917782 = KbDepQOxFe4054991;     KbDepQOxFe4054991 = KbDepQOxFe24523198;     KbDepQOxFe24523198 = KbDepQOxFe55457199;     KbDepQOxFe55457199 = KbDepQOxFe79529158;     KbDepQOxFe79529158 = KbDepQOxFe13332375;     KbDepQOxFe13332375 = KbDepQOxFe22506947;     KbDepQOxFe22506947 = KbDepQOxFe23443183;     KbDepQOxFe23443183 = KbDepQOxFe7279644;     KbDepQOxFe7279644 = KbDepQOxFe81097535;     KbDepQOxFe81097535 = KbDepQOxFe96984284;     KbDepQOxFe96984284 = KbDepQOxFe12884523;     KbDepQOxFe12884523 = KbDepQOxFe87956794;     KbDepQOxFe87956794 = KbDepQOxFe99211758;     KbDepQOxFe99211758 = KbDepQOxFe57339199;     KbDepQOxFe57339199 = KbDepQOxFe74705297;     KbDepQOxFe74705297 = KbDepQOxFe21952862;     KbDepQOxFe21952862 = KbDepQOxFe6813849;     KbDepQOxFe6813849 = KbDepQOxFe50937800;     KbDepQOxFe50937800 = KbDepQOxFe63709065;     KbDepQOxFe63709065 = KbDepQOxFe22090277;     KbDepQOxFe22090277 = KbDepQOxFe64018985;     KbDepQOxFe64018985 = KbDepQOxFe17517797;     KbDepQOxFe17517797 = KbDepQOxFe79233158;     KbDepQOxFe79233158 = KbDepQOxFe36069414;     KbDepQOxFe36069414 = KbDepQOxFe81100315;     KbDepQOxFe81100315 = KbDepQOxFe40086526;     KbDepQOxFe40086526 = KbDepQOxFe96097094;     KbDepQOxFe96097094 = KbDepQOxFe47511395;     KbDepQOxFe47511395 = KbDepQOxFe71894540;     KbDepQOxFe71894540 = KbDepQOxFe54008969;     KbDepQOxFe54008969 = KbDepQOxFe11989022;     KbDepQOxFe11989022 = KbDepQOxFe2658193;     KbDepQOxFe2658193 = KbDepQOxFe68424878;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void odJUxIuzUT23227022() {     int PFHgbfBfKG51869183 = -989894407;    int PFHgbfBfKG27778445 = -948937663;    int PFHgbfBfKG49558303 = -628748551;    int PFHgbfBfKG8154130 = -435459143;    int PFHgbfBfKG23405999 = -984595343;    int PFHgbfBfKG81433008 = -572042150;    int PFHgbfBfKG12600685 = 45719577;    int PFHgbfBfKG96745248 = -602287022;    int PFHgbfBfKG17886433 = -459532974;    int PFHgbfBfKG90680234 = -396810763;    int PFHgbfBfKG4348274 = -111808130;    int PFHgbfBfKG56471312 = -80059439;    int PFHgbfBfKG67749219 = -890721641;    int PFHgbfBfKG80325191 = -558261063;    int PFHgbfBfKG44100450 = -727090372;    int PFHgbfBfKG81794806 = -246708600;    int PFHgbfBfKG63249907 = -767782407;    int PFHgbfBfKG66839615 = -156193320;    int PFHgbfBfKG87598902 = -477505742;    int PFHgbfBfKG33714829 = -743024183;    int PFHgbfBfKG55959392 = -919320199;    int PFHgbfBfKG1446684 = -491014209;    int PFHgbfBfKG35242433 = -854002430;    int PFHgbfBfKG83977323 = -119916259;    int PFHgbfBfKG78069287 = 38537678;    int PFHgbfBfKG80057555 = -667244082;    int PFHgbfBfKG25828628 = -645367061;    int PFHgbfBfKG7900184 = -264250555;    int PFHgbfBfKG97334077 = -571996768;    int PFHgbfBfKG20997213 = -677600183;    int PFHgbfBfKG53786478 = -248521064;    int PFHgbfBfKG84310712 = -617220851;    int PFHgbfBfKG69247472 = -785924290;    int PFHgbfBfKG10872171 = -479112542;    int PFHgbfBfKG75370039 = -20911268;    int PFHgbfBfKG71111973 = -847139091;    int PFHgbfBfKG8562625 = -412894669;    int PFHgbfBfKG30547696 = -645052943;    int PFHgbfBfKG21650873 = -765688463;    int PFHgbfBfKG52146949 = -884670691;    int PFHgbfBfKG54553128 = -796864069;    int PFHgbfBfKG43527148 = -157979610;    int PFHgbfBfKG31768900 = 10009426;    int PFHgbfBfKG26337381 = -23539284;    int PFHgbfBfKG28330740 = -513768937;    int PFHgbfBfKG88142530 = 44395009;    int PFHgbfBfKG61519078 = -636480276;    int PFHgbfBfKG38705284 = -336647549;    int PFHgbfBfKG24310374 = -411582906;    int PFHgbfBfKG74632605 = -691323147;    int PFHgbfBfKG71607471 = -683838699;    int PFHgbfBfKG40254159 = -258530928;    int PFHgbfBfKG71607678 = -38817783;    int PFHgbfBfKG28295067 = -273095958;    int PFHgbfBfKG67267630 = -989454840;    int PFHgbfBfKG50422500 = -398880199;    int PFHgbfBfKG92536012 = 5064767;    int PFHgbfBfKG65580980 = -408832293;    int PFHgbfBfKG30084842 = -373996822;    int PFHgbfBfKG43348443 = -217351262;    int PFHgbfBfKG55604380 = -926675089;    int PFHgbfBfKG4700501 = -690029868;    int PFHgbfBfKG99411170 = 69709746;    int PFHgbfBfKG96889219 = -781932791;    int PFHgbfBfKG36893757 = -48289699;    int PFHgbfBfKG20037561 = -494587280;    int PFHgbfBfKG87223840 = -294135150;    int PFHgbfBfKG56877048 = -311609099;    int PFHgbfBfKG4955153 = -437349795;    int PFHgbfBfKG72988477 = -879951282;    int PFHgbfBfKG73232182 = -833813931;    int PFHgbfBfKG32702212 = -22729465;    int PFHgbfBfKG45188743 = -390504858;    int PFHgbfBfKG35451954 = -592835052;    int PFHgbfBfKG79161701 = -946160115;    int PFHgbfBfKG12432245 = -661340589;    int PFHgbfBfKG69677784 = -401023636;    int PFHgbfBfKG8905052 = -730463147;    int PFHgbfBfKG55646584 = -606147322;    int PFHgbfBfKG89926757 = 94142668;    int PFHgbfBfKG18538477 = 69236194;    int PFHgbfBfKG87123343 = -208719513;    int PFHgbfBfKG83589810 = -852667650;    int PFHgbfBfKG22701473 = -880673622;    int PFHgbfBfKG49389742 = -993761484;    int PFHgbfBfKG13532319 = -989990136;    int PFHgbfBfKG12703034 = -478403068;    int PFHgbfBfKG40952405 = -412828333;    int PFHgbfBfKG43604541 = -489657702;    int PFHgbfBfKG24947540 = -622031069;    int PFHgbfBfKG78575961 = -752203858;    int PFHgbfBfKG42981644 = 95937623;    int PFHgbfBfKG462854 = -171056121;    int PFHgbfBfKG78302429 = -448337202;    int PFHgbfBfKG96542568 = -957995602;    int PFHgbfBfKG49852627 = -6834201;    int PFHgbfBfKG44115978 = -127689356;    int PFHgbfBfKG34879680 = -208057783;    int PFHgbfBfKG89443623 = -975249585;    int PFHgbfBfKG8293179 = -989894407;     PFHgbfBfKG51869183 = PFHgbfBfKG27778445;     PFHgbfBfKG27778445 = PFHgbfBfKG49558303;     PFHgbfBfKG49558303 = PFHgbfBfKG8154130;     PFHgbfBfKG8154130 = PFHgbfBfKG23405999;     PFHgbfBfKG23405999 = PFHgbfBfKG81433008;     PFHgbfBfKG81433008 = PFHgbfBfKG12600685;     PFHgbfBfKG12600685 = PFHgbfBfKG96745248;     PFHgbfBfKG96745248 = PFHgbfBfKG17886433;     PFHgbfBfKG17886433 = PFHgbfBfKG90680234;     PFHgbfBfKG90680234 = PFHgbfBfKG4348274;     PFHgbfBfKG4348274 = PFHgbfBfKG56471312;     PFHgbfBfKG56471312 = PFHgbfBfKG67749219;     PFHgbfBfKG67749219 = PFHgbfBfKG80325191;     PFHgbfBfKG80325191 = PFHgbfBfKG44100450;     PFHgbfBfKG44100450 = PFHgbfBfKG81794806;     PFHgbfBfKG81794806 = PFHgbfBfKG63249907;     PFHgbfBfKG63249907 = PFHgbfBfKG66839615;     PFHgbfBfKG66839615 = PFHgbfBfKG87598902;     PFHgbfBfKG87598902 = PFHgbfBfKG33714829;     PFHgbfBfKG33714829 = PFHgbfBfKG55959392;     PFHgbfBfKG55959392 = PFHgbfBfKG1446684;     PFHgbfBfKG1446684 = PFHgbfBfKG35242433;     PFHgbfBfKG35242433 = PFHgbfBfKG83977323;     PFHgbfBfKG83977323 = PFHgbfBfKG78069287;     PFHgbfBfKG78069287 = PFHgbfBfKG80057555;     PFHgbfBfKG80057555 = PFHgbfBfKG25828628;     PFHgbfBfKG25828628 = PFHgbfBfKG7900184;     PFHgbfBfKG7900184 = PFHgbfBfKG97334077;     PFHgbfBfKG97334077 = PFHgbfBfKG20997213;     PFHgbfBfKG20997213 = PFHgbfBfKG53786478;     PFHgbfBfKG53786478 = PFHgbfBfKG84310712;     PFHgbfBfKG84310712 = PFHgbfBfKG69247472;     PFHgbfBfKG69247472 = PFHgbfBfKG10872171;     PFHgbfBfKG10872171 = PFHgbfBfKG75370039;     PFHgbfBfKG75370039 = PFHgbfBfKG71111973;     PFHgbfBfKG71111973 = PFHgbfBfKG8562625;     PFHgbfBfKG8562625 = PFHgbfBfKG30547696;     PFHgbfBfKG30547696 = PFHgbfBfKG21650873;     PFHgbfBfKG21650873 = PFHgbfBfKG52146949;     PFHgbfBfKG52146949 = PFHgbfBfKG54553128;     PFHgbfBfKG54553128 = PFHgbfBfKG43527148;     PFHgbfBfKG43527148 = PFHgbfBfKG31768900;     PFHgbfBfKG31768900 = PFHgbfBfKG26337381;     PFHgbfBfKG26337381 = PFHgbfBfKG28330740;     PFHgbfBfKG28330740 = PFHgbfBfKG88142530;     PFHgbfBfKG88142530 = PFHgbfBfKG61519078;     PFHgbfBfKG61519078 = PFHgbfBfKG38705284;     PFHgbfBfKG38705284 = PFHgbfBfKG24310374;     PFHgbfBfKG24310374 = PFHgbfBfKG74632605;     PFHgbfBfKG74632605 = PFHgbfBfKG71607471;     PFHgbfBfKG71607471 = PFHgbfBfKG40254159;     PFHgbfBfKG40254159 = PFHgbfBfKG71607678;     PFHgbfBfKG71607678 = PFHgbfBfKG28295067;     PFHgbfBfKG28295067 = PFHgbfBfKG67267630;     PFHgbfBfKG67267630 = PFHgbfBfKG50422500;     PFHgbfBfKG50422500 = PFHgbfBfKG92536012;     PFHgbfBfKG92536012 = PFHgbfBfKG65580980;     PFHgbfBfKG65580980 = PFHgbfBfKG30084842;     PFHgbfBfKG30084842 = PFHgbfBfKG43348443;     PFHgbfBfKG43348443 = PFHgbfBfKG55604380;     PFHgbfBfKG55604380 = PFHgbfBfKG4700501;     PFHgbfBfKG4700501 = PFHgbfBfKG99411170;     PFHgbfBfKG99411170 = PFHgbfBfKG96889219;     PFHgbfBfKG96889219 = PFHgbfBfKG36893757;     PFHgbfBfKG36893757 = PFHgbfBfKG20037561;     PFHgbfBfKG20037561 = PFHgbfBfKG87223840;     PFHgbfBfKG87223840 = PFHgbfBfKG56877048;     PFHgbfBfKG56877048 = PFHgbfBfKG4955153;     PFHgbfBfKG4955153 = PFHgbfBfKG72988477;     PFHgbfBfKG72988477 = PFHgbfBfKG73232182;     PFHgbfBfKG73232182 = PFHgbfBfKG32702212;     PFHgbfBfKG32702212 = PFHgbfBfKG45188743;     PFHgbfBfKG45188743 = PFHgbfBfKG35451954;     PFHgbfBfKG35451954 = PFHgbfBfKG79161701;     PFHgbfBfKG79161701 = PFHgbfBfKG12432245;     PFHgbfBfKG12432245 = PFHgbfBfKG69677784;     PFHgbfBfKG69677784 = PFHgbfBfKG8905052;     PFHgbfBfKG8905052 = PFHgbfBfKG55646584;     PFHgbfBfKG55646584 = PFHgbfBfKG89926757;     PFHgbfBfKG89926757 = PFHgbfBfKG18538477;     PFHgbfBfKG18538477 = PFHgbfBfKG87123343;     PFHgbfBfKG87123343 = PFHgbfBfKG83589810;     PFHgbfBfKG83589810 = PFHgbfBfKG22701473;     PFHgbfBfKG22701473 = PFHgbfBfKG49389742;     PFHgbfBfKG49389742 = PFHgbfBfKG13532319;     PFHgbfBfKG13532319 = PFHgbfBfKG12703034;     PFHgbfBfKG12703034 = PFHgbfBfKG40952405;     PFHgbfBfKG40952405 = PFHgbfBfKG43604541;     PFHgbfBfKG43604541 = PFHgbfBfKG24947540;     PFHgbfBfKG24947540 = PFHgbfBfKG78575961;     PFHgbfBfKG78575961 = PFHgbfBfKG42981644;     PFHgbfBfKG42981644 = PFHgbfBfKG462854;     PFHgbfBfKG462854 = PFHgbfBfKG78302429;     PFHgbfBfKG78302429 = PFHgbfBfKG96542568;     PFHgbfBfKG96542568 = PFHgbfBfKG49852627;     PFHgbfBfKG49852627 = PFHgbfBfKG44115978;     PFHgbfBfKG44115978 = PFHgbfBfKG34879680;     PFHgbfBfKG34879680 = PFHgbfBfKG89443623;     PFHgbfBfKG89443623 = PFHgbfBfKG8293179;     PFHgbfBfKG8293179 = PFHgbfBfKG51869183;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void foMCBFkOZr93682895() {     int bGmSDAGyqb64655351 = -24935099;    int bGmSDAGyqb14549333 = -19271676;    int bGmSDAGyqb67311103 = 62892067;    int bGmSDAGyqb16094956 = -259661420;    int bGmSDAGyqb16917104 = -376411808;    int bGmSDAGyqb24743079 = -348965463;    int bGmSDAGyqb33808443 = -499952494;    int bGmSDAGyqb23464361 = 53194721;    int bGmSDAGyqb13291243 = -421291066;    int bGmSDAGyqb96898939 = -200898396;    int bGmSDAGyqb31835656 = -226494145;    int bGmSDAGyqb88421817 = -587392255;    int bGmSDAGyqb37892522 = -794504298;    int bGmSDAGyqb29949270 = -890676261;    int bGmSDAGyqb67407003 = -634076203;    int bGmSDAGyqb10219875 = -797599989;    int bGmSDAGyqb19001725 = -431556874;    int bGmSDAGyqb50398100 = -811403622;    int bGmSDAGyqb88759532 = -386974937;    int bGmSDAGyqb36369325 = -242268322;    int bGmSDAGyqb36133449 = -310636463;    int bGmSDAGyqb30494503 = -660872557;    int bGmSDAGyqb74002728 = -785006325;    int bGmSDAGyqb28484453 = -480261271;    int bGmSDAGyqb83963547 = -171904246;    int bGmSDAGyqb61376201 = -357675281;    int bGmSDAGyqb37244164 = -179073342;    int bGmSDAGyqb75253109 = -438048660;    int bGmSDAGyqb13951131 = -786214008;    int bGmSDAGyqb73748328 = -307486338;    int bGmSDAGyqb98222473 = -524937814;    int bGmSDAGyqb87629536 = -595940529;    int bGmSDAGyqb15293422 = -661779074;    int bGmSDAGyqb87948544 = -383866760;    int bGmSDAGyqb58430571 = -630943148;    int bGmSDAGyqb66336088 = -635702195;    int bGmSDAGyqb45326685 = -202804020;    int bGmSDAGyqb88251718 = -564673034;    int bGmSDAGyqb27426259 = -607539040;    int bGmSDAGyqb78411678 = -729087536;    int bGmSDAGyqb15984436 = -72338062;    int bGmSDAGyqb51229604 = -56290944;    int bGmSDAGyqb66652369 = -13424019;    int bGmSDAGyqb70965918 = -364711527;    int bGmSDAGyqb8339230 = -406538898;    int bGmSDAGyqb61770660 = -28923430;    int bGmSDAGyqb39101098 = -33995328;    int bGmSDAGyqb68145250 = -492000329;    int bGmSDAGyqb68374423 = -575184881;    int bGmSDAGyqb21583026 = -202359610;    int bGmSDAGyqb89934964 = -936723007;    int bGmSDAGyqb68430706 = -729716266;    int bGmSDAGyqb6092829 = -214821066;    int bGmSDAGyqb72994792 = -962547700;    int bGmSDAGyqb57549446 = -983899327;    int bGmSDAGyqb34160848 = -364062543;    int bGmSDAGyqb40546605 = -234265352;    int bGmSDAGyqb38826651 = -456846662;    int bGmSDAGyqb32131409 = 12242825;    int bGmSDAGyqb55540902 = 81263473;    int bGmSDAGyqb87498914 = -69892122;    int bGmSDAGyqb58555334 = 38096166;    int bGmSDAGyqb9513231 = -160591271;    int bGmSDAGyqb39542915 = -13804729;    int bGmSDAGyqb98676465 = -675960582;    int bGmSDAGyqb44206120 = -630553617;    int bGmSDAGyqb73128396 = -925613181;    int bGmSDAGyqb49943978 = -310637538;    int bGmSDAGyqb71518699 = -159733113;    int bGmSDAGyqb1070916 = -998374009;    int bGmSDAGyqb64893190 = -494795969;    int bGmSDAGyqb30750007 = -866883840;    int bGmSDAGyqb22971842 = -103864583;    int bGmSDAGyqb10347854 = -657887401;    int bGmSDAGyqb20384890 = -69930261;    int bGmSDAGyqb84903845 = -154345519;    int bGmSDAGyqb63842133 = -547448539;    int bGmSDAGyqb3036810 = -320294799;    int bGmSDAGyqb20145223 = 26277626;    int bGmSDAGyqb22192887 = -42980817;    int bGmSDAGyqb22275103 = -223679953;    int bGmSDAGyqb69098913 = -687073014;    int bGmSDAGyqb6878686 = -862863780;    int bGmSDAGyqb92368104 = -483854399;    int bGmSDAGyqb83813363 = -370763331;    int bGmSDAGyqb29791768 = -795221548;    int bGmSDAGyqb81536707 = -281119464;    int bGmSDAGyqb42298629 = -699231375;    int bGmSDAGyqb30399098 = -399967434;    int bGmSDAGyqb24269724 = -166880606;    int bGmSDAGyqb25789484 = -301436843;    int bGmSDAGyqb6500035 = -745957358;    int bGmSDAGyqb56120310 = -476915860;    int bGmSDAGyqb71885356 = -588802513;    int bGmSDAGyqb90912763 = -559195415;    int bGmSDAGyqb57429102 = -10434228;    int bGmSDAGyqb41716374 = -895699674;    int bGmSDAGyqb27109455 = -999619290;    int bGmSDAGyqb72289452 = -688750945;    int bGmSDAGyqb64133110 = -24935099;     bGmSDAGyqb64655351 = bGmSDAGyqb14549333;     bGmSDAGyqb14549333 = bGmSDAGyqb67311103;     bGmSDAGyqb67311103 = bGmSDAGyqb16094956;     bGmSDAGyqb16094956 = bGmSDAGyqb16917104;     bGmSDAGyqb16917104 = bGmSDAGyqb24743079;     bGmSDAGyqb24743079 = bGmSDAGyqb33808443;     bGmSDAGyqb33808443 = bGmSDAGyqb23464361;     bGmSDAGyqb23464361 = bGmSDAGyqb13291243;     bGmSDAGyqb13291243 = bGmSDAGyqb96898939;     bGmSDAGyqb96898939 = bGmSDAGyqb31835656;     bGmSDAGyqb31835656 = bGmSDAGyqb88421817;     bGmSDAGyqb88421817 = bGmSDAGyqb37892522;     bGmSDAGyqb37892522 = bGmSDAGyqb29949270;     bGmSDAGyqb29949270 = bGmSDAGyqb67407003;     bGmSDAGyqb67407003 = bGmSDAGyqb10219875;     bGmSDAGyqb10219875 = bGmSDAGyqb19001725;     bGmSDAGyqb19001725 = bGmSDAGyqb50398100;     bGmSDAGyqb50398100 = bGmSDAGyqb88759532;     bGmSDAGyqb88759532 = bGmSDAGyqb36369325;     bGmSDAGyqb36369325 = bGmSDAGyqb36133449;     bGmSDAGyqb36133449 = bGmSDAGyqb30494503;     bGmSDAGyqb30494503 = bGmSDAGyqb74002728;     bGmSDAGyqb74002728 = bGmSDAGyqb28484453;     bGmSDAGyqb28484453 = bGmSDAGyqb83963547;     bGmSDAGyqb83963547 = bGmSDAGyqb61376201;     bGmSDAGyqb61376201 = bGmSDAGyqb37244164;     bGmSDAGyqb37244164 = bGmSDAGyqb75253109;     bGmSDAGyqb75253109 = bGmSDAGyqb13951131;     bGmSDAGyqb13951131 = bGmSDAGyqb73748328;     bGmSDAGyqb73748328 = bGmSDAGyqb98222473;     bGmSDAGyqb98222473 = bGmSDAGyqb87629536;     bGmSDAGyqb87629536 = bGmSDAGyqb15293422;     bGmSDAGyqb15293422 = bGmSDAGyqb87948544;     bGmSDAGyqb87948544 = bGmSDAGyqb58430571;     bGmSDAGyqb58430571 = bGmSDAGyqb66336088;     bGmSDAGyqb66336088 = bGmSDAGyqb45326685;     bGmSDAGyqb45326685 = bGmSDAGyqb88251718;     bGmSDAGyqb88251718 = bGmSDAGyqb27426259;     bGmSDAGyqb27426259 = bGmSDAGyqb78411678;     bGmSDAGyqb78411678 = bGmSDAGyqb15984436;     bGmSDAGyqb15984436 = bGmSDAGyqb51229604;     bGmSDAGyqb51229604 = bGmSDAGyqb66652369;     bGmSDAGyqb66652369 = bGmSDAGyqb70965918;     bGmSDAGyqb70965918 = bGmSDAGyqb8339230;     bGmSDAGyqb8339230 = bGmSDAGyqb61770660;     bGmSDAGyqb61770660 = bGmSDAGyqb39101098;     bGmSDAGyqb39101098 = bGmSDAGyqb68145250;     bGmSDAGyqb68145250 = bGmSDAGyqb68374423;     bGmSDAGyqb68374423 = bGmSDAGyqb21583026;     bGmSDAGyqb21583026 = bGmSDAGyqb89934964;     bGmSDAGyqb89934964 = bGmSDAGyqb68430706;     bGmSDAGyqb68430706 = bGmSDAGyqb6092829;     bGmSDAGyqb6092829 = bGmSDAGyqb72994792;     bGmSDAGyqb72994792 = bGmSDAGyqb57549446;     bGmSDAGyqb57549446 = bGmSDAGyqb34160848;     bGmSDAGyqb34160848 = bGmSDAGyqb40546605;     bGmSDAGyqb40546605 = bGmSDAGyqb38826651;     bGmSDAGyqb38826651 = bGmSDAGyqb32131409;     bGmSDAGyqb32131409 = bGmSDAGyqb55540902;     bGmSDAGyqb55540902 = bGmSDAGyqb87498914;     bGmSDAGyqb87498914 = bGmSDAGyqb58555334;     bGmSDAGyqb58555334 = bGmSDAGyqb9513231;     bGmSDAGyqb9513231 = bGmSDAGyqb39542915;     bGmSDAGyqb39542915 = bGmSDAGyqb98676465;     bGmSDAGyqb98676465 = bGmSDAGyqb44206120;     bGmSDAGyqb44206120 = bGmSDAGyqb73128396;     bGmSDAGyqb73128396 = bGmSDAGyqb49943978;     bGmSDAGyqb49943978 = bGmSDAGyqb71518699;     bGmSDAGyqb71518699 = bGmSDAGyqb1070916;     bGmSDAGyqb1070916 = bGmSDAGyqb64893190;     bGmSDAGyqb64893190 = bGmSDAGyqb30750007;     bGmSDAGyqb30750007 = bGmSDAGyqb22971842;     bGmSDAGyqb22971842 = bGmSDAGyqb10347854;     bGmSDAGyqb10347854 = bGmSDAGyqb20384890;     bGmSDAGyqb20384890 = bGmSDAGyqb84903845;     bGmSDAGyqb84903845 = bGmSDAGyqb63842133;     bGmSDAGyqb63842133 = bGmSDAGyqb3036810;     bGmSDAGyqb3036810 = bGmSDAGyqb20145223;     bGmSDAGyqb20145223 = bGmSDAGyqb22192887;     bGmSDAGyqb22192887 = bGmSDAGyqb22275103;     bGmSDAGyqb22275103 = bGmSDAGyqb69098913;     bGmSDAGyqb69098913 = bGmSDAGyqb6878686;     bGmSDAGyqb6878686 = bGmSDAGyqb92368104;     bGmSDAGyqb92368104 = bGmSDAGyqb83813363;     bGmSDAGyqb83813363 = bGmSDAGyqb29791768;     bGmSDAGyqb29791768 = bGmSDAGyqb81536707;     bGmSDAGyqb81536707 = bGmSDAGyqb42298629;     bGmSDAGyqb42298629 = bGmSDAGyqb30399098;     bGmSDAGyqb30399098 = bGmSDAGyqb24269724;     bGmSDAGyqb24269724 = bGmSDAGyqb25789484;     bGmSDAGyqb25789484 = bGmSDAGyqb6500035;     bGmSDAGyqb6500035 = bGmSDAGyqb56120310;     bGmSDAGyqb56120310 = bGmSDAGyqb71885356;     bGmSDAGyqb71885356 = bGmSDAGyqb90912763;     bGmSDAGyqb90912763 = bGmSDAGyqb57429102;     bGmSDAGyqb57429102 = bGmSDAGyqb41716374;     bGmSDAGyqb41716374 = bGmSDAGyqb27109455;     bGmSDAGyqb27109455 = bGmSDAGyqb72289452;     bGmSDAGyqb72289452 = bGmSDAGyqb64133110;     bGmSDAGyqb64133110 = bGmSDAGyqb64655351;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BYRxxxlexn11896237() {     int iFsluvGLar48099656 = -905600780;    int iFsluvGLar31421422 = -18653864;    int iFsluvGLar63719610 = -830633160;    int iFsluvGLar78887207 = -453461256;    int iFsluvGLar23038427 = -183279299;    int iFsluvGLar23247480 = -128037116;    int iFsluvGLar4847761 = -239863807;    int iFsluvGLar37811804 = 66036692;    int iFsluvGLar71820944 = -762750417;    int iFsluvGLar29444592 = -669274508;    int iFsluvGLar81251593 = -416610018;    int iFsluvGLar51627217 = -575010501;    int iFsluvGLar56010297 = -586165539;    int iFsluvGLar9576162 = -678966618;    int iFsluvGLar28261467 = -728099185;    int iFsluvGLar96609036 = -585679314;    int iFsluvGLar12753610 = -184901901;    int iFsluvGLar31249547 = 41291501;    int iFsluvGLar51199675 = -385649808;    int iFsluvGLar25736768 = -771854315;    int iFsluvGLar91773836 = 99834506;    int iFsluvGLar42641821 = -652024327;    int iFsluvGLar77828435 = -824895522;    int iFsluvGLar63191011 = -552086667;    int iFsluvGLar25539505 = -731557005;    int iFsluvGLar56718350 = -238184997;    int iFsluvGLar86698904 = -907157684;    int iFsluvGLar14984735 = -508003544;    int iFsluvGLar50032225 = -648719150;    int iFsluvGLar89581669 = -820336189;    int iFsluvGLar16317788 = -162695015;    int iFsluvGLar50735963 = -511135609;    int iFsluvGLar58832308 = -282651252;    int iFsluvGLar30107055 = 24531070;    int iFsluvGLar37157302 = -828607883;    int iFsluvGLar78725272 = -423726101;    int iFsluvGLar13940863 = -81653200;    int iFsluvGLar28830551 = -273143554;    int iFsluvGLar76421336 = -792436838;    int iFsluvGLar27906814 = -481681058;    int iFsluvGLar49633360 = -743887035;    int iFsluvGLar1717391 = -630199266;    int iFsluvGLar90219439 = -564481261;    int iFsluvGLar62870858 = -120894298;    int iFsluvGLar283727 = -134695851;    int iFsluvGLar1376657 = -48534354;    int iFsluvGLar15116529 = -166299558;    int iFsluvGLar87815846 = -490722047;    int iFsluvGLar99221535 = -60786963;    int iFsluvGLar56915510 = -409380065;    int iFsluvGLar63192412 = -57624430;    int iFsluvGLar23931500 = -787975578;    int iFsluvGLar20205288 = -426568382;    int iFsluvGLar97671551 = -680961087;    int iFsluvGLar20122401 = -232385450;    int iFsluvGLar5457835 = -153576453;    int iFsluvGLar53592986 = -193758342;    int iFsluvGLar528600 = -178546494;    int iFsluvGLar53347702 = -721904252;    int iFsluvGLar66320077 = -945094303;    int iFsluvGLar36548575 = -220879433;    int iFsluvGLar89863026 = -731860264;    int iFsluvGLar87779578 = -285244158;    int iFsluvGLar82239274 = -942414229;    int iFsluvGLar13126804 = -406579493;    int iFsluvGLar30515631 = -905474410;    int iFsluvGLar92794908 = -192359250;    int iFsluvGLar25903243 = -510696609;    int iFsluvGLar72418860 = -850358735;    int iFsluvGLar49536195 = -204373085;    int iFsluvGLar82668173 = -404026115;    int iFsluvGLar83923059 = -911758347;    int iFsluvGLar54828210 = -166271661;    int iFsluvGLar23292861 = -903968750;    int iFsluvGLar76103407 = 72032720;    int iFsluvGLar90056446 = -269966229;    int iFsluvGLar52422382 = 12456934;    int iFsluvGLar14957577 = -604001225;    int iFsluvGLar62907284 = -317390817;    int iFsluvGLar24162849 = -583022652;    int iFsluvGLar41601822 = 28114561;    int iFsluvGLar98883058 = -316435638;    int iFsluvGLar15763199 = -347216581;    int iFsluvGLar93116715 = -139339086;    int iFsluvGLar26389257 = -662711760;    int iFsluvGLar92386287 = -374719438;    int iFsluvGLar30530675 = 15432772;    int iFsluvGLar61160756 = -601690165;    int iFsluvGLar9984655 = -743083481;    int iFsluvGLar31699467 = -575031431;    int iFsluvGLar25132286 = -129967759;    int iFsluvGLar13412264 = -903106706;    int iFsluvGLar75482848 = -551239303;    int iFsluvGLar10101260 = -847342536;    int iFsluvGLar91358238 = -160801626;    int iFsluvGLar59770334 = 87973228;    int iFsluvGLar13937812 = -244955108;    int iFsluvGLar7980165 = -622067033;    int iFsluvGLar49744054 = -714314805;    int iFsluvGLar69768096 = -905600780;     iFsluvGLar48099656 = iFsluvGLar31421422;     iFsluvGLar31421422 = iFsluvGLar63719610;     iFsluvGLar63719610 = iFsluvGLar78887207;     iFsluvGLar78887207 = iFsluvGLar23038427;     iFsluvGLar23038427 = iFsluvGLar23247480;     iFsluvGLar23247480 = iFsluvGLar4847761;     iFsluvGLar4847761 = iFsluvGLar37811804;     iFsluvGLar37811804 = iFsluvGLar71820944;     iFsluvGLar71820944 = iFsluvGLar29444592;     iFsluvGLar29444592 = iFsluvGLar81251593;     iFsluvGLar81251593 = iFsluvGLar51627217;     iFsluvGLar51627217 = iFsluvGLar56010297;     iFsluvGLar56010297 = iFsluvGLar9576162;     iFsluvGLar9576162 = iFsluvGLar28261467;     iFsluvGLar28261467 = iFsluvGLar96609036;     iFsluvGLar96609036 = iFsluvGLar12753610;     iFsluvGLar12753610 = iFsluvGLar31249547;     iFsluvGLar31249547 = iFsluvGLar51199675;     iFsluvGLar51199675 = iFsluvGLar25736768;     iFsluvGLar25736768 = iFsluvGLar91773836;     iFsluvGLar91773836 = iFsluvGLar42641821;     iFsluvGLar42641821 = iFsluvGLar77828435;     iFsluvGLar77828435 = iFsluvGLar63191011;     iFsluvGLar63191011 = iFsluvGLar25539505;     iFsluvGLar25539505 = iFsluvGLar56718350;     iFsluvGLar56718350 = iFsluvGLar86698904;     iFsluvGLar86698904 = iFsluvGLar14984735;     iFsluvGLar14984735 = iFsluvGLar50032225;     iFsluvGLar50032225 = iFsluvGLar89581669;     iFsluvGLar89581669 = iFsluvGLar16317788;     iFsluvGLar16317788 = iFsluvGLar50735963;     iFsluvGLar50735963 = iFsluvGLar58832308;     iFsluvGLar58832308 = iFsluvGLar30107055;     iFsluvGLar30107055 = iFsluvGLar37157302;     iFsluvGLar37157302 = iFsluvGLar78725272;     iFsluvGLar78725272 = iFsluvGLar13940863;     iFsluvGLar13940863 = iFsluvGLar28830551;     iFsluvGLar28830551 = iFsluvGLar76421336;     iFsluvGLar76421336 = iFsluvGLar27906814;     iFsluvGLar27906814 = iFsluvGLar49633360;     iFsluvGLar49633360 = iFsluvGLar1717391;     iFsluvGLar1717391 = iFsluvGLar90219439;     iFsluvGLar90219439 = iFsluvGLar62870858;     iFsluvGLar62870858 = iFsluvGLar283727;     iFsluvGLar283727 = iFsluvGLar1376657;     iFsluvGLar1376657 = iFsluvGLar15116529;     iFsluvGLar15116529 = iFsluvGLar87815846;     iFsluvGLar87815846 = iFsluvGLar99221535;     iFsluvGLar99221535 = iFsluvGLar56915510;     iFsluvGLar56915510 = iFsluvGLar63192412;     iFsluvGLar63192412 = iFsluvGLar23931500;     iFsluvGLar23931500 = iFsluvGLar20205288;     iFsluvGLar20205288 = iFsluvGLar97671551;     iFsluvGLar97671551 = iFsluvGLar20122401;     iFsluvGLar20122401 = iFsluvGLar5457835;     iFsluvGLar5457835 = iFsluvGLar53592986;     iFsluvGLar53592986 = iFsluvGLar528600;     iFsluvGLar528600 = iFsluvGLar53347702;     iFsluvGLar53347702 = iFsluvGLar66320077;     iFsluvGLar66320077 = iFsluvGLar36548575;     iFsluvGLar36548575 = iFsluvGLar89863026;     iFsluvGLar89863026 = iFsluvGLar87779578;     iFsluvGLar87779578 = iFsluvGLar82239274;     iFsluvGLar82239274 = iFsluvGLar13126804;     iFsluvGLar13126804 = iFsluvGLar30515631;     iFsluvGLar30515631 = iFsluvGLar92794908;     iFsluvGLar92794908 = iFsluvGLar25903243;     iFsluvGLar25903243 = iFsluvGLar72418860;     iFsluvGLar72418860 = iFsluvGLar49536195;     iFsluvGLar49536195 = iFsluvGLar82668173;     iFsluvGLar82668173 = iFsluvGLar83923059;     iFsluvGLar83923059 = iFsluvGLar54828210;     iFsluvGLar54828210 = iFsluvGLar23292861;     iFsluvGLar23292861 = iFsluvGLar76103407;     iFsluvGLar76103407 = iFsluvGLar90056446;     iFsluvGLar90056446 = iFsluvGLar52422382;     iFsluvGLar52422382 = iFsluvGLar14957577;     iFsluvGLar14957577 = iFsluvGLar62907284;     iFsluvGLar62907284 = iFsluvGLar24162849;     iFsluvGLar24162849 = iFsluvGLar41601822;     iFsluvGLar41601822 = iFsluvGLar98883058;     iFsluvGLar98883058 = iFsluvGLar15763199;     iFsluvGLar15763199 = iFsluvGLar93116715;     iFsluvGLar93116715 = iFsluvGLar26389257;     iFsluvGLar26389257 = iFsluvGLar92386287;     iFsluvGLar92386287 = iFsluvGLar30530675;     iFsluvGLar30530675 = iFsluvGLar61160756;     iFsluvGLar61160756 = iFsluvGLar9984655;     iFsluvGLar9984655 = iFsluvGLar31699467;     iFsluvGLar31699467 = iFsluvGLar25132286;     iFsluvGLar25132286 = iFsluvGLar13412264;     iFsluvGLar13412264 = iFsluvGLar75482848;     iFsluvGLar75482848 = iFsluvGLar10101260;     iFsluvGLar10101260 = iFsluvGLar91358238;     iFsluvGLar91358238 = iFsluvGLar59770334;     iFsluvGLar59770334 = iFsluvGLar13937812;     iFsluvGLar13937812 = iFsluvGLar7980165;     iFsluvGLar7980165 = iFsluvGLar49744054;     iFsluvGLar49744054 = iFsluvGLar69768096;     iFsluvGLar69768096 = iFsluvGLar48099656;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bujMJyFLuJ82352109() {     int dEAPSXrfHp60885823 = 59358529;    int dEAPSXrfHp18192310 = -188987878;    int dEAPSXrfHp81472409 = -138992542;    int dEAPSXrfHp86828032 = -277663533;    int dEAPSXrfHp16549532 = -675095764;    int dEAPSXrfHp66557550 = 95039570;    int dEAPSXrfHp26055519 = -785535878;    int dEAPSXrfHp64530916 = -378481565;    int dEAPSXrfHp67225754 = -724508509;    int dEAPSXrfHp35663296 = -473362141;    int dEAPSXrfHp8738976 = -531296034;    int dEAPSXrfHp83577721 = 17656683;    int dEAPSXrfHp26153601 = -489948196;    int dEAPSXrfHp59200241 = 88618184;    int dEAPSXrfHp51568020 = -635085016;    int dEAPSXrfHp25034106 = -36570703;    int dEAPSXrfHp68505427 = -948676367;    int dEAPSXrfHp14808032 = -613918801;    int dEAPSXrfHp52360305 = -295119003;    int dEAPSXrfHp28391264 = -271098454;    int dEAPSXrfHp71947893 = -391481758;    int dEAPSXrfHp71689640 = -821882675;    int dEAPSXrfHp16588731 = -755899417;    int dEAPSXrfHp7698141 = -912431680;    int dEAPSXrfHp31433764 = -941998929;    int dEAPSXrfHp38036995 = 71383804;    int dEAPSXrfHp98114440 = -440863964;    int dEAPSXrfHp82337659 = -681801649;    int dEAPSXrfHp66649278 = -862936391;    int dEAPSXrfHp42332785 = -450222344;    int dEAPSXrfHp60753783 = -439111765;    int dEAPSXrfHp54054786 = -489855288;    int dEAPSXrfHp4878258 = -158506036;    int dEAPSXrfHp7183428 = -980223148;    int dEAPSXrfHp20217834 = -338639764;    int dEAPSXrfHp73949387 = -212289205;    int dEAPSXrfHp50704923 = -971562550;    int dEAPSXrfHp86534573 = -192763646;    int dEAPSXrfHp82196722 = -634287415;    int dEAPSXrfHp54171543 = -326097904;    int dEAPSXrfHp11064669 = -19361028;    int dEAPSXrfHp9419846 = -528510600;    int dEAPSXrfHp25102909 = -587914706;    int dEAPSXrfHp7499397 = -462066540;    int dEAPSXrfHp80292217 = -27465811;    int dEAPSXrfHp75004785 = -121852793;    int dEAPSXrfHp92698547 = -663814610;    int dEAPSXrfHp17255813 = -646074827;    int dEAPSXrfHp43285585 = -224388938;    int dEAPSXrfHp3865931 = 79583472;    int dEAPSXrfHp81519906 = -310508738;    int dEAPSXrfHp52108047 = -159160916;    int dEAPSXrfHp54690438 = -602571665;    int dEAPSXrfHp42371277 = -270412829;    int dEAPSXrfHp10404217 = -226829936;    int dEAPSXrfHp89196183 = -118758797;    int dEAPSXrfHp1603579 = -433088461;    int dEAPSXrfHp73774269 = -226560863;    int dEAPSXrfHp55394268 = -335664604;    int dEAPSXrfHp78512536 = -646479569;    int dEAPSXrfHp68443109 = -464096466;    int dEAPSXrfHp43717860 = -3734229;    int dEAPSXrfHp97881638 = -515545175;    int dEAPSXrfHp24892970 = -174286166;    int dEAPSXrfHp74909513 = 65749624;    int dEAPSXrfHp54684189 = 58559254;    int dEAPSXrfHp78699464 = -823837281;    int dEAPSXrfHp18970173 = -509725048;    int dEAPSXrfHp38982407 = -572742053;    int dEAPSXrfHp77618633 = -322795812;    int dEAPSXrfHp74329182 = -65008154;    int dEAPSXrfHp81970854 = -655912722;    int dEAPSXrfHp32611309 = -979631386;    int dEAPSXrfHp98188761 = -969021099;    int dEAPSXrfHp17326596 = -151737426;    int dEAPSXrfHp62528047 = -862971158;    int dEAPSXrfHp46586732 = -133967969;    int dEAPSXrfHp9089335 = -193832877;    int dEAPSXrfHp27405924 = -784965869;    int dEAPSXrfHp56428979 = -720146136;    int dEAPSXrfHp45338447 = -264801586;    int dEAPSXrfHp80858628 = -794789138;    int dEAPSXrfHp39052075 = -357412711;    int dEAPSXrfHp62783347 = -842519863;    int dEAPSXrfHp60812879 = -39713606;    int dEAPSXrfHp8645736 = -179950850;    int dEAPSXrfHp99364348 = -887283624;    int dEAPSXrfHp62506980 = -888093208;    int dEAPSXrfHp96779210 = -653393213;    int dEAPSXrfHp31021651 = -119880967;    int dEAPSXrfHp72345808 = -779200744;    int dEAPSXrfHp76930654 = -645001688;    int dEAPSXrfHp31140305 = -857099042;    int dEAPSXrfHp3684187 = -987807847;    int dEAPSXrfHp85728433 = -862001439;    int dEAPSXrfHp67346808 = 84373201;    int dEAPSXrfHp11538208 = 87034574;    int dEAPSXrfHp209940 = -313628541;    int dEAPSXrfHp32589884 = -427816165;    int dEAPSXrfHp25608028 = 59358529;     dEAPSXrfHp60885823 = dEAPSXrfHp18192310;     dEAPSXrfHp18192310 = dEAPSXrfHp81472409;     dEAPSXrfHp81472409 = dEAPSXrfHp86828032;     dEAPSXrfHp86828032 = dEAPSXrfHp16549532;     dEAPSXrfHp16549532 = dEAPSXrfHp66557550;     dEAPSXrfHp66557550 = dEAPSXrfHp26055519;     dEAPSXrfHp26055519 = dEAPSXrfHp64530916;     dEAPSXrfHp64530916 = dEAPSXrfHp67225754;     dEAPSXrfHp67225754 = dEAPSXrfHp35663296;     dEAPSXrfHp35663296 = dEAPSXrfHp8738976;     dEAPSXrfHp8738976 = dEAPSXrfHp83577721;     dEAPSXrfHp83577721 = dEAPSXrfHp26153601;     dEAPSXrfHp26153601 = dEAPSXrfHp59200241;     dEAPSXrfHp59200241 = dEAPSXrfHp51568020;     dEAPSXrfHp51568020 = dEAPSXrfHp25034106;     dEAPSXrfHp25034106 = dEAPSXrfHp68505427;     dEAPSXrfHp68505427 = dEAPSXrfHp14808032;     dEAPSXrfHp14808032 = dEAPSXrfHp52360305;     dEAPSXrfHp52360305 = dEAPSXrfHp28391264;     dEAPSXrfHp28391264 = dEAPSXrfHp71947893;     dEAPSXrfHp71947893 = dEAPSXrfHp71689640;     dEAPSXrfHp71689640 = dEAPSXrfHp16588731;     dEAPSXrfHp16588731 = dEAPSXrfHp7698141;     dEAPSXrfHp7698141 = dEAPSXrfHp31433764;     dEAPSXrfHp31433764 = dEAPSXrfHp38036995;     dEAPSXrfHp38036995 = dEAPSXrfHp98114440;     dEAPSXrfHp98114440 = dEAPSXrfHp82337659;     dEAPSXrfHp82337659 = dEAPSXrfHp66649278;     dEAPSXrfHp66649278 = dEAPSXrfHp42332785;     dEAPSXrfHp42332785 = dEAPSXrfHp60753783;     dEAPSXrfHp60753783 = dEAPSXrfHp54054786;     dEAPSXrfHp54054786 = dEAPSXrfHp4878258;     dEAPSXrfHp4878258 = dEAPSXrfHp7183428;     dEAPSXrfHp7183428 = dEAPSXrfHp20217834;     dEAPSXrfHp20217834 = dEAPSXrfHp73949387;     dEAPSXrfHp73949387 = dEAPSXrfHp50704923;     dEAPSXrfHp50704923 = dEAPSXrfHp86534573;     dEAPSXrfHp86534573 = dEAPSXrfHp82196722;     dEAPSXrfHp82196722 = dEAPSXrfHp54171543;     dEAPSXrfHp54171543 = dEAPSXrfHp11064669;     dEAPSXrfHp11064669 = dEAPSXrfHp9419846;     dEAPSXrfHp9419846 = dEAPSXrfHp25102909;     dEAPSXrfHp25102909 = dEAPSXrfHp7499397;     dEAPSXrfHp7499397 = dEAPSXrfHp80292217;     dEAPSXrfHp80292217 = dEAPSXrfHp75004785;     dEAPSXrfHp75004785 = dEAPSXrfHp92698547;     dEAPSXrfHp92698547 = dEAPSXrfHp17255813;     dEAPSXrfHp17255813 = dEAPSXrfHp43285585;     dEAPSXrfHp43285585 = dEAPSXrfHp3865931;     dEAPSXrfHp3865931 = dEAPSXrfHp81519906;     dEAPSXrfHp81519906 = dEAPSXrfHp52108047;     dEAPSXrfHp52108047 = dEAPSXrfHp54690438;     dEAPSXrfHp54690438 = dEAPSXrfHp42371277;     dEAPSXrfHp42371277 = dEAPSXrfHp10404217;     dEAPSXrfHp10404217 = dEAPSXrfHp89196183;     dEAPSXrfHp89196183 = dEAPSXrfHp1603579;     dEAPSXrfHp1603579 = dEAPSXrfHp73774269;     dEAPSXrfHp73774269 = dEAPSXrfHp55394268;     dEAPSXrfHp55394268 = dEAPSXrfHp78512536;     dEAPSXrfHp78512536 = dEAPSXrfHp68443109;     dEAPSXrfHp68443109 = dEAPSXrfHp43717860;     dEAPSXrfHp43717860 = dEAPSXrfHp97881638;     dEAPSXrfHp97881638 = dEAPSXrfHp24892970;     dEAPSXrfHp24892970 = dEAPSXrfHp74909513;     dEAPSXrfHp74909513 = dEAPSXrfHp54684189;     dEAPSXrfHp54684189 = dEAPSXrfHp78699464;     dEAPSXrfHp78699464 = dEAPSXrfHp18970173;     dEAPSXrfHp18970173 = dEAPSXrfHp38982407;     dEAPSXrfHp38982407 = dEAPSXrfHp77618633;     dEAPSXrfHp77618633 = dEAPSXrfHp74329182;     dEAPSXrfHp74329182 = dEAPSXrfHp81970854;     dEAPSXrfHp81970854 = dEAPSXrfHp32611309;     dEAPSXrfHp32611309 = dEAPSXrfHp98188761;     dEAPSXrfHp98188761 = dEAPSXrfHp17326596;     dEAPSXrfHp17326596 = dEAPSXrfHp62528047;     dEAPSXrfHp62528047 = dEAPSXrfHp46586732;     dEAPSXrfHp46586732 = dEAPSXrfHp9089335;     dEAPSXrfHp9089335 = dEAPSXrfHp27405924;     dEAPSXrfHp27405924 = dEAPSXrfHp56428979;     dEAPSXrfHp56428979 = dEAPSXrfHp45338447;     dEAPSXrfHp45338447 = dEAPSXrfHp80858628;     dEAPSXrfHp80858628 = dEAPSXrfHp39052075;     dEAPSXrfHp39052075 = dEAPSXrfHp62783347;     dEAPSXrfHp62783347 = dEAPSXrfHp60812879;     dEAPSXrfHp60812879 = dEAPSXrfHp8645736;     dEAPSXrfHp8645736 = dEAPSXrfHp99364348;     dEAPSXrfHp99364348 = dEAPSXrfHp62506980;     dEAPSXrfHp62506980 = dEAPSXrfHp96779210;     dEAPSXrfHp96779210 = dEAPSXrfHp31021651;     dEAPSXrfHp31021651 = dEAPSXrfHp72345808;     dEAPSXrfHp72345808 = dEAPSXrfHp76930654;     dEAPSXrfHp76930654 = dEAPSXrfHp31140305;     dEAPSXrfHp31140305 = dEAPSXrfHp3684187;     dEAPSXrfHp3684187 = dEAPSXrfHp85728433;     dEAPSXrfHp85728433 = dEAPSXrfHp67346808;     dEAPSXrfHp67346808 = dEAPSXrfHp11538208;     dEAPSXrfHp11538208 = dEAPSXrfHp209940;     dEAPSXrfHp209940 = dEAPSXrfHp32589884;     dEAPSXrfHp32589884 = dEAPSXrfHp25608028;     dEAPSXrfHp25608028 = dEAPSXrfHp60885823;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GBnRGTmKhH565452() {     int IJujlBMYeP44330129 = -821307152;    int IJujlBMYeP35064399 = -188370065;    int IJujlBMYeP77880916 = 67482230;    int IJujlBMYeP49620284 = -471463369;    int IJujlBMYeP22670856 = -481963255;    int IJujlBMYeP65061951 = -784032083;    int IJujlBMYeP97094836 = -525447191;    int IJujlBMYeP78878359 = -365639594;    int IJujlBMYeP25755455 = 34032140;    int IJujlBMYeP68208948 = -941738253;    int IJujlBMYeP58154913 = -721411906;    int IJujlBMYeP46783121 = 30038437;    int IJujlBMYeP44271376 = -281609437;    int IJujlBMYeP38827133 = -799672173;    int IJujlBMYeP12422484 = -729107998;    int IJujlBMYeP11423267 = -924650029;    int IJujlBMYeP62257312 = -702021394;    int IJujlBMYeP95659478 = -861223677;    int IJujlBMYeP14800448 = -293793874;    int IJujlBMYeP17758707 = -800684447;    int IJujlBMYeP27588281 = 18989211;    int IJujlBMYeP83836958 = -813034445;    int IJujlBMYeP20414439 = -795788614;    int IJujlBMYeP42404699 = -984257076;    int IJujlBMYeP73009722 = -401651688;    int IJujlBMYeP33379144 = -909125911;    int IJujlBMYeP47569181 = -68948307;    int IJujlBMYeP22069285 = -751756532;    int IJujlBMYeP2730373 = -725441533;    int IJujlBMYeP58166126 = -963072195;    int IJujlBMYeP78849097 = -76868966;    int IJujlBMYeP17161214 = -405050367;    int IJujlBMYeP48417144 = -879378213;    int IJujlBMYeP49341938 = -571825318;    int IJujlBMYeP98944563 = -536304499;    int IJujlBMYeP86338571 = -313111;    int IJujlBMYeP19319102 = -850411730;    int IJujlBMYeP27113406 = 98765834;    int IJujlBMYeP31191801 = -819185213;    int IJujlBMYeP3666679 = -78691426;    int IJujlBMYeP44713593 = -690910002;    int IJujlBMYeP59907632 = -2418922;    int IJujlBMYeP48669979 = -38971949;    int IJujlBMYeP99404336 = -218249311;    int IJujlBMYeP72236713 = -855622764;    int IJujlBMYeP14610782 = -141463717;    int IJujlBMYeP68713978 = -796118840;    int IJujlBMYeP36926408 = -644796545;    int IJujlBMYeP74132697 = -809991020;    int IJujlBMYeP39198415 = -127436983;    int IJujlBMYeP54777354 = -531410160;    int IJujlBMYeP7608842 = -217420227;    int IJujlBMYeP68802896 = -814318981;    int IJujlBMYeP67048035 = 11173784;    int IJujlBMYeP72977171 = -575316059;    int IJujlBMYeP60493170 = 91727293;    int IJujlBMYeP14649961 = -392581452;    int IJujlBMYeP35476218 = 51739305;    int IJujlBMYeP76610561 = 30188319;    int IJujlBMYeP89291711 = -572837344;    int IJujlBMYeP17492771 = -615083777;    int IJujlBMYeP75025552 = -773690659;    int IJujlBMYeP76147987 = -640198062;    int IJujlBMYeP67589329 = -2895666;    int IJujlBMYeP89359851 = -764869287;    int IJujlBMYeP40993700 = -216361539;    int IJujlBMYeP98365977 = -90583350;    int IJujlBMYeP94929438 = -709784119;    int IJujlBMYeP39882569 = -163367675;    int IJujlBMYeP26083912 = -628794888;    int IJujlBMYeP92104164 = 25761701;    int IJujlBMYeP35143907 = -700787229;    int IJujlBMYeP64467677 = 57961535;    int IJujlBMYeP11133769 = -115102448;    int IJujlBMYeP73045114 = -9774445;    int IJujlBMYeP67680648 = -978591868;    int IJujlBMYeP35166980 = -674062497;    int IJujlBMYeP21010103 = -477539303;    int IJujlBMYeP70167985 = -28634312;    int IJujlBMYeP58398941 = -160187972;    int IJujlBMYeP64665166 = -13007072;    int IJujlBMYeP10642773 = -424151762;    int IJujlBMYeP47936588 = -941765512;    int IJujlBMYeP63531957 = -498004550;    int IJujlBMYeP3388772 = -331662035;    int IJujlBMYeP71240255 = -859448739;    int IJujlBMYeP48358317 = -590731387;    int IJujlBMYeP81369108 = -790551998;    int IJujlBMYeP76364767 = -996509260;    int IJujlBMYeP38451394 = -528031792;    int IJujlBMYeP71688611 = -607731660;    int IJujlBMYeP83842884 = -802151036;    int IJujlBMYeP50502844 = -931422485;    int IJujlBMYeP41900089 = -146347869;    int IJujlBMYeP86173908 = -463607650;    int IJujlBMYeP69688040 = -917219343;    int IJujlBMYeP83759645 = -362220860;    int IJujlBMYeP81080649 = 63923716;    int IJujlBMYeP10044486 = -453380025;    int IJujlBMYeP31243014 = -821307152;     IJujlBMYeP44330129 = IJujlBMYeP35064399;     IJujlBMYeP35064399 = IJujlBMYeP77880916;     IJujlBMYeP77880916 = IJujlBMYeP49620284;     IJujlBMYeP49620284 = IJujlBMYeP22670856;     IJujlBMYeP22670856 = IJujlBMYeP65061951;     IJujlBMYeP65061951 = IJujlBMYeP97094836;     IJujlBMYeP97094836 = IJujlBMYeP78878359;     IJujlBMYeP78878359 = IJujlBMYeP25755455;     IJujlBMYeP25755455 = IJujlBMYeP68208948;     IJujlBMYeP68208948 = IJujlBMYeP58154913;     IJujlBMYeP58154913 = IJujlBMYeP46783121;     IJujlBMYeP46783121 = IJujlBMYeP44271376;     IJujlBMYeP44271376 = IJujlBMYeP38827133;     IJujlBMYeP38827133 = IJujlBMYeP12422484;     IJujlBMYeP12422484 = IJujlBMYeP11423267;     IJujlBMYeP11423267 = IJujlBMYeP62257312;     IJujlBMYeP62257312 = IJujlBMYeP95659478;     IJujlBMYeP95659478 = IJujlBMYeP14800448;     IJujlBMYeP14800448 = IJujlBMYeP17758707;     IJujlBMYeP17758707 = IJujlBMYeP27588281;     IJujlBMYeP27588281 = IJujlBMYeP83836958;     IJujlBMYeP83836958 = IJujlBMYeP20414439;     IJujlBMYeP20414439 = IJujlBMYeP42404699;     IJujlBMYeP42404699 = IJujlBMYeP73009722;     IJujlBMYeP73009722 = IJujlBMYeP33379144;     IJujlBMYeP33379144 = IJujlBMYeP47569181;     IJujlBMYeP47569181 = IJujlBMYeP22069285;     IJujlBMYeP22069285 = IJujlBMYeP2730373;     IJujlBMYeP2730373 = IJujlBMYeP58166126;     IJujlBMYeP58166126 = IJujlBMYeP78849097;     IJujlBMYeP78849097 = IJujlBMYeP17161214;     IJujlBMYeP17161214 = IJujlBMYeP48417144;     IJujlBMYeP48417144 = IJujlBMYeP49341938;     IJujlBMYeP49341938 = IJujlBMYeP98944563;     IJujlBMYeP98944563 = IJujlBMYeP86338571;     IJujlBMYeP86338571 = IJujlBMYeP19319102;     IJujlBMYeP19319102 = IJujlBMYeP27113406;     IJujlBMYeP27113406 = IJujlBMYeP31191801;     IJujlBMYeP31191801 = IJujlBMYeP3666679;     IJujlBMYeP3666679 = IJujlBMYeP44713593;     IJujlBMYeP44713593 = IJujlBMYeP59907632;     IJujlBMYeP59907632 = IJujlBMYeP48669979;     IJujlBMYeP48669979 = IJujlBMYeP99404336;     IJujlBMYeP99404336 = IJujlBMYeP72236713;     IJujlBMYeP72236713 = IJujlBMYeP14610782;     IJujlBMYeP14610782 = IJujlBMYeP68713978;     IJujlBMYeP68713978 = IJujlBMYeP36926408;     IJujlBMYeP36926408 = IJujlBMYeP74132697;     IJujlBMYeP74132697 = IJujlBMYeP39198415;     IJujlBMYeP39198415 = IJujlBMYeP54777354;     IJujlBMYeP54777354 = IJujlBMYeP7608842;     IJujlBMYeP7608842 = IJujlBMYeP68802896;     IJujlBMYeP68802896 = IJujlBMYeP67048035;     IJujlBMYeP67048035 = IJujlBMYeP72977171;     IJujlBMYeP72977171 = IJujlBMYeP60493170;     IJujlBMYeP60493170 = IJujlBMYeP14649961;     IJujlBMYeP14649961 = IJujlBMYeP35476218;     IJujlBMYeP35476218 = IJujlBMYeP76610561;     IJujlBMYeP76610561 = IJujlBMYeP89291711;     IJujlBMYeP89291711 = IJujlBMYeP17492771;     IJujlBMYeP17492771 = IJujlBMYeP75025552;     IJujlBMYeP75025552 = IJujlBMYeP76147987;     IJujlBMYeP76147987 = IJujlBMYeP67589329;     IJujlBMYeP67589329 = IJujlBMYeP89359851;     IJujlBMYeP89359851 = IJujlBMYeP40993700;     IJujlBMYeP40993700 = IJujlBMYeP98365977;     IJujlBMYeP98365977 = IJujlBMYeP94929438;     IJujlBMYeP94929438 = IJujlBMYeP39882569;     IJujlBMYeP39882569 = IJujlBMYeP26083912;     IJujlBMYeP26083912 = IJujlBMYeP92104164;     IJujlBMYeP92104164 = IJujlBMYeP35143907;     IJujlBMYeP35143907 = IJujlBMYeP64467677;     IJujlBMYeP64467677 = IJujlBMYeP11133769;     IJujlBMYeP11133769 = IJujlBMYeP73045114;     IJujlBMYeP73045114 = IJujlBMYeP67680648;     IJujlBMYeP67680648 = IJujlBMYeP35166980;     IJujlBMYeP35166980 = IJujlBMYeP21010103;     IJujlBMYeP21010103 = IJujlBMYeP70167985;     IJujlBMYeP70167985 = IJujlBMYeP58398941;     IJujlBMYeP58398941 = IJujlBMYeP64665166;     IJujlBMYeP64665166 = IJujlBMYeP10642773;     IJujlBMYeP10642773 = IJujlBMYeP47936588;     IJujlBMYeP47936588 = IJujlBMYeP63531957;     IJujlBMYeP63531957 = IJujlBMYeP3388772;     IJujlBMYeP3388772 = IJujlBMYeP71240255;     IJujlBMYeP71240255 = IJujlBMYeP48358317;     IJujlBMYeP48358317 = IJujlBMYeP81369108;     IJujlBMYeP81369108 = IJujlBMYeP76364767;     IJujlBMYeP76364767 = IJujlBMYeP38451394;     IJujlBMYeP38451394 = IJujlBMYeP71688611;     IJujlBMYeP71688611 = IJujlBMYeP83842884;     IJujlBMYeP83842884 = IJujlBMYeP50502844;     IJujlBMYeP50502844 = IJujlBMYeP41900089;     IJujlBMYeP41900089 = IJujlBMYeP86173908;     IJujlBMYeP86173908 = IJujlBMYeP69688040;     IJujlBMYeP69688040 = IJujlBMYeP83759645;     IJujlBMYeP83759645 = IJujlBMYeP81080649;     IJujlBMYeP81080649 = IJujlBMYeP10044486;     IJujlBMYeP10044486 = IJujlBMYeP31243014;     IJujlBMYeP31243014 = IJujlBMYeP44330129;}
// Junk Finished
