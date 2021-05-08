#include <SDL2/SDL.h>
#include <cstdint>

#include "bus.h"

namespace Emulator {
		static SDL_Window * window;
		static SDL_Renderer *renderer;
		static SDL_Texture * tex;
		static SDL_Event event;

		struct Pallete
		{
			std::uint32_t COL0;
			std::uint32_t COL1;
			std::uint32_t COL2;
			std::uint32_t COL3;
		};

		// All The Colors Palletes I got from lospec.com
		static Pallete Palletes[5] = 
		{
			{ 0xFFFFF6d3, 0xFFF9A875, 0xFFEB6B6F, 0xFF7C3F58 }, // ICE CREAM GB PALLETE
			{ 0xFFFFE4C2, 0xFFDCA456, 0xFFA9604C, 0xFF422936 }, // GB CHOCOlATE PALLETE
			{ 0xFFDAD3AF, 0xFFD58863, 0xFFC23A73, 0xFF2C1E74 }, // AUTUMN CHILL PALLETE
			{ 0xFFF1F2DA, 0xFFFFCE96, 0xFFFF7777, 0xFF00303B }, // AYY4 PALETTE
			{ 0xFFD0D058, 0xFFA0A840, 0xFF708028, 0xFF405010 }  // NOSTALGIA PALETTE
		};

		static int CurrentPalleteIndex = 0;

		static Bus bus;
		void init()
		{
			SDL_Init(SDL_INIT_EVERYTHING);
			window = SDL_CreateWindow("",
					SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 288, SDL_WINDOW_RESIZABLE);
			renderer = SDL_CreateRenderer(window, -1, 0);

			tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
		}
		void Render()
		{
			SDL_UpdateTexture(tex, NULL, bus.pix, 160 * sizeof(uint32_t));

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, tex, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		void LoadRom(const char* FilePath)
		{
			bus.LoadRom(FilePath);
		}
		void RunLoop()
		{
			init();
			bus.SetRenderer(Render);
			bool quit = false;
			SDL_Event event;
			const int FPS = 60;
			const int FrameDelay = 1000 / FPS;
			unsigned int FrameStart;
			int FrameTime;
			while (!quit)
			{
				FrameStart = SDL_GetTicks();
				while(SDL_PollEvent(&event))
				{
					Bus::Key key;
					if (event.type == SDL_KEYDOWN)
					{
						switch (event.key.keysym.sym)
						{
							case SDLK_w:			bus.KeyPressed(Bus::KEY_UP); break;
							case SDLK_s:			bus.KeyPressed(Bus::KEY_DOWN); break;
							case SDLK_a:			bus.KeyPressed(Bus::KEY_LEFT); break;
							case SDLK_d:			bus.KeyPressed(Bus::KEY_RIGHT); break;
							case SDLK_SPACE:	bus.KeyPressed(Bus::KEY_SELECT); break;
							case SDLK_RETURN: bus.KeyPressed(Bus::KEY_START); break;
							case SDLK_h:			bus.KeyPressed(Bus::KEY_A); break;
							case SDLK_j:			bus.KeyPressed(Bus::KEY_B); break;
							case SDLK_c:			{ auto NewP = Palletes[(++CurrentPalleteIndex) % 5]; bus.ChangePallete(NewP.COL0, NewP.COL1, NewP.COL2, NewP.COL3); }; break;
						}
					}
					if (event.type == SDL_KEYUP)
					{
						switch (event.key.keysym.sym)
						{
							case SDLK_w:			bus.KeyReleased(Bus::KEY_UP); break;
							case SDLK_s:			bus.KeyReleased(Bus::KEY_DOWN); break;
							case SDLK_a:			bus.KeyReleased(Bus::KEY_LEFT); break;
							case SDLK_d:			bus.KeyReleased(Bus::KEY_RIGHT); break;
							case SDLK_SPACE:	bus.KeyReleased(Bus::KEY_SELECT); break;
							case SDLK_RETURN: bus.KeyReleased(Bus::KEY_START); break;
							case SDLK_h:			bus.KeyReleased(Bus::KEY_A); break;
							case SDLK_j:			bus.KeyReleased(Bus::KEY_B); break;
						}
					}
					if (event.type == SDL_QUIT)
					{
						quit = true;
					}
				}
				bus.clock();
				FrameTime = SDL_GetTicks() - FrameStart;
				if (FrameDelay > FrameTime)
					SDL_Delay(FrameDelay - FrameTime);
			}
		}
		void Quit()
		{
			SDL_DestroyTexture(tex);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
};
