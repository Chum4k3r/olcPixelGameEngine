/*
	Blithering About Dithering (Floyd-Steinberg)
	"2022 lets go!" - javidx9
	
	License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2021 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
	Video:
	~~~~~~
	https://youtu.be/lseR6ZguBNY

	Use Q and W keys to view quantised image and dithered image respectively
	Use Left mouse button to pan, and mouse wheel to zoom to cursor
	
	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Homepage:	https://www.onelonecoder.com
	
	Author
	~~~~~~
	David Barr, aka javidx9, ŠOneLoneCoder 2019, 2020, 2021, 2022
*/


// Using olc::PixelGameEngine for input and visualisation
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Using a transformed view to handle pan and zoom
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

//#include <algorithm>

// Override base class with your custom functionality
class Dithering : public olc::PixelGameEngine
{
public:
	Dithering()
	{		
		sAppName = "Floyd Steinberg Dithering";
	}

	olc::TransformedView tv;
	std::unique_ptr<olc::Sprite> m_pImage;
	std::unique_ptr<olc::Sprite> m_pQuantised;
	std::unique_ptr<olc::Sprite> m_pDithered;

public:
	// Called once at start of application
	bool OnUserCreate() override
	{
		// Prepare Pan & Zoom
		tv.Initialise({ ScreenWidth(), ScreenHeight() });

		// Load Test Image
		m_pImage = std::make_unique<olc::Sprite>("./assets/Flower_640x480.png");

		// Create two more images with the same dimensions
		m_pQuantised = std::make_unique<olc::Sprite>(m_pImage->width, m_pImage->height);
		m_pDithered = std::make_unique<olc::Sprite>(m_pImage->width, m_pImage->height);

		// These lambda functions output a new olc::Pixel based on
		// the pixel it is given
		auto Convert_RGB_To_Greyscale = [](const olc::Pixel in)
		{
			uint8_t greyscale = uint8_t(0.2162f * float(in.r) + 0.7152f * float(in.g) + 0.0722f * float(in.b));
			return olc::Pixel(greyscale, greyscale, greyscale);
		};


		// Quantising functions
		auto Quantise_Greyscale_1Bit = [](const olc::Pixel in)
		{
			return in.r < 128 ? olc::BLACK : olc::WHITE;
		};

		auto Quantise_Greyscale_NBit = [](const olc::Pixel in)
		{
			constexpr int nBits = 2;
			constexpr float fLevels = (1 << nBits) - 1;
			uint8_t c = uint8_t(std::clamp(std::round(float(in.r) / 255.0f * fLevels) / fLevels * 255.0f, 0.0f, 255.0f));
			return olc::Pixel(c, c, c);
		};

		auto Quantise_RGB_NBit = [](const olc::Pixel in)
		{
			constexpr int nBits = 2;
			constexpr float fLevels = (1 << nBits) - 1;
			uint8_t cr = uint8_t(std::clamp(std::round(float(in.r) / 255.0f * fLevels) / fLevels * 255.0f, 0.0f, 255.0f));
			uint8_t cb = uint8_t(std::clamp(std::round(float(in.g) / 255.0f * fLevels) / fLevels * 255.0f, 0.0f, 255.0f));
			uint8_t cg = uint8_t(std::clamp(std::round(float(in.b) / 255.0f * fLevels) / fLevels * 255.0f, 0.0f, 255.0f));
			return olc::Pixel(cr, cb, cg);
		};

		auto Quantise_RGB_CustomPalette = [](const olc::Pixel in)
		{
			std::array<olc::Pixel, 5> nShades = { olc::BLACK, olc::WHITE, olc::YELLOW, olc::MAGENTA, olc::CYAN };
			
			float fClosest = INFINITY;
			olc::Pixel pClosest;

			for (const auto& c : nShades)
			{
				float fDistance = float(
					std::sqrt(
						std::pow(float(c.r) - float(in.r), 2) +
						std::pow(float(c.g) - float(in.g), 2) +
						std::pow(float(c.b) - float(in.b), 2)));

				if (fDistance < fClosest)
				{
					fClosest = fDistance;
					pClosest = c;
				}
			}
						
			return pClosest;
		};


		// We don't need greyscale for the final demonstration, which uses
		// RGB, but I've left this here as reference
		//std::transform(
		//	m_pImage->pColData.begin(), 
		//	m_pImage->pColData.end(), 
		//	m_pImage->pColData.begin(), Convert_RGB_To_Greyscale);


		// Quantise The Image
		std::transform(
			m_pImage->pColData.begin(),
			m_pImage->pColData.end(),
			m_pQuantised->pColData.begin(), Quantise_RGB_NBit);

		// Perform Dither 
		Dither_FloydSteinberg(m_pImage.get(), m_pDithered.get(), Quantise_RGB_NBit);

		return true;
	}

	

	void Dither_FloydSteinberg(const olc::Sprite* pSource, olc::Sprite* pDest, 
		std::function<olc::Pixel(const olc::Pixel)> funcQuantise)
	{		
		// The destination image is primed with the source image as the pixel
		// values become altered as the algorithm executes
		std::copy(pSource->pColData.begin(), pSource->pColData.end(), pDest->pColData.begin());		

		// Iterate through each pixel from top left to bottom right, compare the pixel
		// with that on the "allowed" list, and distribute that error to neighbours
		// not yet computed
		olc::vi2d vPixel;
		for (vPixel.y = 0; vPixel.y < pSource->height; vPixel.y++)
		{
			for (vPixel.x = 0; vPixel.x < pSource->width; vPixel.x++)
			{
				// Grap and get nearest pixel equivalent from our allowed
				// palette
				olc::Pixel op = pDest->GetPixel(vPixel);
				olc::Pixel qp = funcQuantise(op);

				// olc::Pixels are "inconveniently" clamped to sensible ranges using an unsigned type...
				// ...which means they cant be negative. This hampers us a tad here,
				// so will resort to manual alteration using a signed type
				int32_t error[3] =
				{
					op.r - qp.r,
					op.g - qp.g,
					op.b - qp.b
				};
				
				// Set destination pixel with nearest match from quantisation function
				pDest->SetPixel(vPixel, qp);

				// Distribute Error - Using a little utility lambda to keep the messy code
				// all in one place. It's important to allow pixels to temporarily become
				// negative in order to distribute the error to the neighbours in both
				// directions... value directions that is, not spatial!				
				auto UpdatePixel = [&vPixel, &pDest, &error](const olc::vi2d& vOffset, const float fErrorBias)
				{
					olc::Pixel p = pDest->GetPixel(vPixel + vOffset);
					int32_t k[3] = { p.r, p.g, p.b };
					k[0] += int32_t(float(error[0]) * fErrorBias);
					k[1] += int32_t(float(error[1]) * fErrorBias);
					k[2] += int32_t(float(error[2]) * fErrorBias);
					pDest->SetPixel(vPixel + vOffset, olc::Pixel(std::clamp(k[0], 0, 255), std::clamp(k[1], 0, 255), std::clamp(k[2], 0, 255)));
				};

				UpdatePixel({ +1,  0 }, 7.0f / 16.0f);
				UpdatePixel({ -1, +1 }, 3.0f / 16.0f);
				UpdatePixel({  0, +1 }, 5.0f / 16.0f);
				UpdatePixel({ +1, +1 }, 1.0f / 16.0f);
			}
		}
	}

	// Called every frame
	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle Pan & Zoom using defaults middle mouse button
		tv.HandlePanAndZoom(0);

		// Erase previous frame
		Clear(olc::BLACK);

		// Draw Source Image
		if (GetKey(olc::Key::Q).bHeld)
		{
			tv.DrawSprite({ 0,0 }, m_pQuantised.get());
		}
		else if (GetKey(olc::Key::W).bHeld)
		{
			tv.DrawSprite({ 0,0 }, m_pDithered.get());
		}
		else
		{
			tv.DrawSprite({ 0,0 }, m_pImage.get());
		}

		return true;
	}
};

int main()
{
	Dithering demo;
	if (demo.Construct(1280, 720, 1, 1))
		demo.Start();
	return 0;
}