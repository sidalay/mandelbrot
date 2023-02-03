#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <condition_variable>
#include <atomic>
#include <complex>
#include <cstdlib>
#include <immintrin.h>

constexpr int nMaxThreads = 32;

class olcFractalExplorer : public olc::PixelGameEngine
{
public:
	olcFractalExplorer()
	{
		sAppName = "Brute Force Processing";
	}

	int* pFractal = nullptr;
	int nMode = 1;
	int nIterations = 64;

public:
	bool OnUserCreate() override
	{
		pFractal = (int*)_aligned_malloc(size_t(ScreenWidth()) * size_t(ScreenHeight()) * sizeof(int), 64);
		return true;
	}

	// Method 1) - Super simple, no effort at optimising
	void CreateFractalBasic(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));
		
		for (int y = pix_tl.y; y < pix_br.y; y++)
		{
			for (int x = pix_tl.x; x < pix_br.x; x++)
			{
				std::complex<double> c(x * x_scale + frac_tl.x, y * y_scale + frac_tl.y);
				std::complex<double> z(0, 0);

				int n = 0;
				while (abs(z) < 2.0 && n < iterations)
				{
					z = (z * z) + c;
					n++;
				}

				pFractal[y * ScreenWidth() + x] = n;
			}
		}
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Get mouse location this frame
		olc::vd2d vMouse = { (double)GetMouseX(), (double)GetMouseY() };

		// Handle Pan & Zoom
		if (GetMouse(2).bPressed)
		{
			vStartPan = vMouse;
		}

		if (GetMouse(2).bHeld)
		{
			vOffset -= (vMouse - vStartPan) / vScale;
			vStartPan = vMouse;
		}

		olc::vd2d vMouseBeforeZoom;
		ScreenToWorld(vMouse, vMouseBeforeZoom);

		if (GetKey(olc::Key::Q).bHeld || GetMouseWheel() > 0) vScale *= 1.1;
		if (GetKey(olc::Key::A).bHeld || GetMouseWheel() < 0) vScale *= 0.9;
		
		olc::vd2d vMouseAfterZoom;
		ScreenToWorld(vMouse, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);
		
		olc::vi2d pix_tl = { 0,0 };
		olc::vi2d pix_br = { ScreenWidth(), ScreenHeight() };
		olc::vd2d frac_tl = { -2.0, -1.0 };
		olc::vd2d frac_br = { 1.0, 1.0 };

		ScreenToWorld(pix_tl, frac_tl);
		ScreenToWorld(pix_br, frac_br);

		// Handle User Input
		if (GetKey(olc::K1).bPressed) nMode = 0;
		if (GetKey(olc::UP).bPressed) nIterations += 64;
		if (GetKey(olc::DOWN).bPressed) nIterations -= 64;
		if (nIterations < 64) nIterations = 64;


		// START TIMING
		auto tp1 = std::chrono::high_resolution_clock::now();

		// Do the computation
		switch (nMode)
		{
		case 0: CreateFractalBasic(pix_tl, pix_br, frac_tl, frac_br, nIterations); break;
		}

		// STOP TIMING
		auto tp2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsedTime = tp2 - tp1;


		// Render result to screen
		for (int y = 0; y < ScreenHeight(); y++)
		{
			for (int x = 0; x < ScreenWidth(); x++)
			{
				int i = pFractal[y * ScreenWidth() + x];
				float n = (float)i;
				float a = 0.1f;
				// Thank you @Eriksonn - Wonderful Magic Fractal Oddball Man
				Draw(x, y, olc::PixelF(0.5f * sin(a * n) + 0.5f, 0.5f * sin(a * n + 2.094f) + 0.5f,  0.5f * sin(a * n + 4.188f) + 0.5f));
			}
		}

		// Render UI
		switch (nMode)
		{
		case 0: DrawString(0, 0, "1) Naive Method", olc::WHITE, 3); break;
		case 1: DrawString(0, 0, "4) Vector Extensions (AVX2) Method", olc::WHITE, 3); break;
		}

		DrawString(0, 30, "Time Taken: " + std::to_string(elapsedTime.count()) + "s", olc::WHITE, 3);
		DrawString(0, 60, "Iterations: " + std::to_string(nIterations), olc::WHITE, 3);
		return !(GetKey(olc::Key::ESCAPE).bPressed);
	}

	// Pan & Zoom variables
	olc::vd2d vOffset = { 0.0, 0.0 };
	olc::vd2d vStartPan = { 0.0, 0.0 };
	olc::vd2d vScale = { 1280.0 / 2.0, 720.0 };
	

	// Convert coordinates from World Space --> Screen Space
	void WorldToScreen(const olc::vd2d& v, olc::vi2d &n)
	{
		n.x = (int)((v.x - vOffset.x) * vScale.x);
		n.y = (int)((v.y - vOffset.y) * vScale.y);
	}

	// Convert coordinates from Screen Space --> World Space
	void ScreenToWorld(const olc::vi2d& n, olc::vd2d& v)
	{
		v.x = (double)(n.x) / vScale.x + vOffset.x;
		v.y = (double)(n.y) / vScale.y + vOffset.y;
	}
};

int main()
{
	olcFractalExplorer demo;
	if (demo.Construct(1280, 720, 1, 1, false, false))
		demo.Start();
	return 0;
}