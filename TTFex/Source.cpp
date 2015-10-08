#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <SDL2\SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2_image.lib")
#pragma comment (lib, "SDL2_ttf.lib")

#ifdef main
#undef main
#endif

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font *gFont = nullptr;
bool quit = false;
int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;

using namespace std;

class TextObject
{
public:
	void update();
	void chooseFont(TTF_Font* _font);
	void makeText(const string& _text);

	string text;
	SDL_Texture *texture = nullptr;
	SDL_Rect rect;
	TTF_Font *font = nullptr;
	int lineHeight;
} textArea;

void TextObject::chooseFont(TTF_Font* _font)
{
	font = _font;
	lineHeight = TTF_FontLineSkip(font);
	update();
}

void TextObject::update()
{
	stringstream ss = stringstream(text);

	int textHeight = 0;
	int textWidth = 0;

	vector<SDL_Surface*> lineSurfs;

	unsigned c = 0; // character

	while (c < text.length())
	{
		string line;
		int lineWidth = 0;
		while (lineWidth < SCREEN_WIDTH && c < text.length()) //add char
		{
			if (text[c] == '\n') //newline
			{
				c++;
				break;
			}
			line += text[c];
			TTF_SizeText(font, line.c_str(), &lineWidth, nullptr);
			c++;		
		}
		if (lineWidth > SCREEN_WIDTH) //line too long
		{
			line.pop_back();
			TTF_SizeText(font, line.c_str(), &lineWidth, nullptr);
			c--;
		}
		if (lineWidth > textWidth) //longest line
		{
			textWidth = lineWidth;
		}
		//cout << line << endl;
		textHeight += lineHeight;

		auto surf = TTF_RenderText_Blended(font, line.c_str(), { 0x00, 0x00, 0x00 });
		lineSurfs.push_back(surf);
	}

	auto finalSurf = SDL_CreateRGBSurface(
		0,
		textWidth,
		textHeight,
		32,
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
		);

	int i = 0;
	for (auto surf : lineSurfs)
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = lineHeight * i++;
		rect.w = surf->w;
		rect.h = surf->h;

		SDL_BlitSurface(surf, nullptr, finalSurf, &rect);
		SDL_FreeSurface(surf);
	}

	lineSurfs.clear();

	if (texture)
	{
		SDL_DestroyTexture(texture);
	}
	texture = SDL_CreateTextureFromSurface(gRenderer, finalSurf);
	SDL_FreeSurface(finalSurf);

	rect.x = 0;
	rect.y = 0;
	rect.w = textWidth;
	rect.h = textHeight;
}

void TextObject::makeText(const string& _text)
{
	text = _text;
	update();
}

bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	//Initialize SDL_TTF
	if (TTF_Init())
	{
		printf("SDL_TTF could not initialize! SDL_TTF Error: %s\n", TTF_GetError());
		return false;
	}

	return true;
}

TTF_Font* loadFont(const char* file, unsigned size)
{
	TTF_Font *font = TTF_OpenFont(file, size);
	if (!font)
	{
		printf("Couldn't load font '%s'!\n", file);
		return nullptr;
	}
	return font;
}

void close()
{
	//Destroy global things	
	if (gRenderer)
	{
		SDL_DestroyRenderer(gRenderer);
	}

	if (gWindow)
	{
		SDL_DestroyWindow(gWindow);
	}
	if (gFont)
	{
		TTF_CloseFont(gFont);
	}

	//Quit SDL subsystems
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();

}

void handleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_KEYUP: break;
		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_BACKSPACE && textArea.text.length() > 0)
			{
				//lop off character
				textArea.text.pop_back();
			}
			else if (e.key.keysym.sym == SDLK_RETURN)
			{
				textArea.text += '\n';
			}
			else if (e.key.keysym.sym == SDLK_TAB)
			{
				textArea.text += "   ";
			}
			textArea.update();
			break;
		case SDL_TEXTINPUT:
			textArea.text += e.text.text[0];
			textArea.update();
			break;
		case SDL_QUIT: quit = true; break;
		case SDL_WINDOWEVENT:

			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GetWindowSize(gWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
				textArea.update();
				SDL_RenderPresent(gRenderer);
			}
			break;
		}
	}
}

int main()
{
	init();

	//TextObject textArea;
	gFont = loadFont("lucon.ttf", 36);
	textArea.chooseFont(gFont);
	//textArea.makeText("qwertyu\niopåasdfghjklöäzxcvbnmQWERTYUIOPÅASDFGHJKLÖÄZXCVBNM");

	//cout << textArea.text;

	int x = 0;
	int w = 10;
	int h = 10;
	SDL_Color color1 = { 255,255,255 };
	SDL_Color color2 = { 0,0,0 };

	while (!quit)
	{
		handleEvents();

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(gRenderer, NULL);

		for (int i = 0; i < h; i++)
		{

			SDL_SetRenderDrawColor(gRenderer, color1.r, color1.g, color1.b, 255);
			SDL_RenderDrawPoint(gRenderer, x, 100+i);

			for (int j = 1; j < w; j++)
			{
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
				SDL_RenderDrawPoint(gRenderer, x + j, 100+i);
			}

			SDL_SetRenderDrawColor(gRenderer, color2.r, color2.g, color2.b, 255);
			SDL_RenderDrawPoint(gRenderer, x + w, 100+i);

			if (color1.r) { color1.r--; color2.r++; }
			else if (color1.g) { color1.g--; color2.g++; }
			else if (color1.b) { color1.b--; color2.b++; }
			else
			{
				color1 = { 255,255,255 }; color2 = { 0,0,0 };
				x++;
			}
		}

		SDL_RenderPresent(gRenderer);
		SDL_Delay(17*2);
	}
	quit = false;

	while (!quit)
	{
		handleEvents();

		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
		SDL_RenderFillRect(gRenderer, NULL);

		SDL_RenderCopy(gRenderer, textArea.texture, NULL, &textArea.rect);

		SDL_RenderPresent(gRenderer);
	}

	close();
}