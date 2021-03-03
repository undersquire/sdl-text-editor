#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct line {
	char *text;
	int textcount;
};

int
main(int argc, char *argv[])
{
	/* Initialization */
	
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	
	/* Setup */
	
	SDL_Window *window = SDL_CreateWindow(
		"EasyDevelop",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1280,
		720,
		SDL_WINDOW_OPENGL
	);
	
	if (!window) {
		printf("error: failed to open display\n");
		return 1;
	}
	
	SDL_Renderer *renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);
	
	if (!renderer) {
		printf("error: failed to create graphics\n");
		return 1;
	}
	
	int font_size = 16;
	
	TTF_Font *font = TTF_OpenFont("font.ttf", font_size);
	
	if (!font) {
		printf("error: could not open font\n");
		return 1;
	}
	
	/* Data */
	
	int running = 1;
	
	SDL_Event event;
	
	int cursorX = 0, cursorY = 0;
	
	int linecount = 1, scrolloffset = 0;
	struct line *lines = (struct line *)SDL_malloc(sizeof(struct line));
	
	int i, j, k, l; /* used for text processing */
	
	/* initialize text for lines */
	for (i = 0; i < linecount; i++) {
		lines[i].text = (char *)SDL_malloc(1);
		lines[i].text[0] = 0;
		lines[i].textcount = 0;
	}
	
	int fw, fh;
	
	TTF_SizeText(font, "X", &fw, &fh);
	
	SDL_Rect cursor;
	
	cursor.x = cursorX;
	cursor.y = cursorY;
	cursor.w = 2;
	cursor.h = font_size;
	
	int rectcount = 720 / fh;
	SDL_Rect *rects = (SDL_Rect *)SDL_malloc(sizeof(SDL_Rect) * rectcount);
	
	for (i = 0; i < rectcount; i++) {
		rects[i].x = 0;
		rects[i].y = i * (font_size + 3);
		rects[i].w = 0;
		rects[i].h = font_size + 3;
	}
	
	SDL_Color color = { 255, 255, 255 };
	
	SDL_Surface *text_surface;
	SDL_Texture *text_texture;
	
	/* Logic */
	
	SDL_StartTextInput();
	
	while (running) {
		/* Events */
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							if (cursorY > 0) {
								--cursorY;
								if (cursorX > lines[cursorY].textcount - 1)
									cursorX = lines[cursorY].textcount;
							}
							break;
						case SDLK_DOWN:
							if (cursorY < linecount - 1) {
								++cursorY;
								if (cursorX > lines[cursorY].textcount - 1)
									cursorX = lines[cursorY].textcount;
							}
							break;
						case SDLK_LEFT:
							if (cursorX > 0)
								--cursorX;
							else {
								if (cursorY - 1 >= 0) {
									--cursorY;
									cursorX = lines[cursorY].textcount;
								}
							}
							break;
						case SDLK_RIGHT:
							if (cursorX < lines[cursorY].textcount)
								++cursorX;
							else {
								if (cursorY + 1 < linecount) {
									++cursorY;
									cursorX = 0;
								}
							}
							break;
						case SDLK_BACKSPACE:
							if (cursorX > 0) {
								for (i = cursorX - 1; i < lines[cursorY].textcount; i++)
									lines[cursorY].text[i] = lines[cursorY].text[i + 1];
								
								lines[cursorY].textcount--;
								
								lines[cursorY].text = (char *)SDL_realloc(
									lines[cursorY].text,
									lines[cursorY].textcount + 1
								);
								
								--cursorX;
							} else {
								if (cursorY > 0) {
									/* TODO: Implement this */
								}
							}
							
							break;
						case SDLK_RETURN:
							++linecount;
							
							lines = (struct line *)SDL_realloc(
								lines,
								sizeof(struct line) * linecount
							);
							
							for (i = linecount - 1; i > cursorY + 1; i--)
								lines[i] = lines[i - 1];
							
							++cursorY;
							
							j = (lines[cursorY - 1].textcount) - cursorX;
							
							lines[cursorY].text = (char *)SDL_malloc(j + 1);
							lines[cursorY].textcount = j;
							
							for (i = 0; i < j + 1; i++)
								lines[cursorY].text[i] = lines[cursorY - 1].text[cursorX + i];
							
							lines[cursorY - 1].text = (char *)SDL_realloc(
								lines[cursorY - 1].text,
								cursorX + 1
							);
							
							lines[cursorY - 1].text[cursorX] = 0;
							lines[cursorY - 1].textcount = cursorX;
							
							cursorX = 0;
							
							break;
					}
					break;
				case SDL_TEXTINPUT:
					lines[cursorY].textcount++;
					
					lines[cursorY].text = (char *)SDL_realloc(
						lines[cursorY].text,
						lines[cursorY].textcount + 1
					);
					
					for (i = lines[cursorY].textcount; i > cursorX; i--)
						lines[cursorY].text[i] = lines[cursorY].text[i - 1];
					
					lines[cursorY].text[cursorX] = event.text.text[0];
					
					++cursorX;
					
					break;
			}
		}
		
		/* Update */
		
		l = 0;
		
		/* Rendering */
		
		SDL_RenderClear(renderer);
		
		for (i = 0; i < rectcount; i++) {
			
			if (i >= linecount)
				break;
			
			if (lines[i + scrolloffset].textcount > 0) {
				rects[i].x = 0;
				
				for (j = 0; lines[i + scrolloffset].text[j] != 0; j++) {		
					k = lines[i + scrolloffset].text[j];
					
					text_surface = TTF_RenderGlyph_Blended(font, k, color);
					
					rects[i].w = text_surface->w;
					
					if (i + scrolloffset == cursorY && j < cursorX)
						l += text_surface->w;
					
					text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
					
					SDL_FreeSurface(text_surface);
					
					SDL_RenderCopy(renderer, text_texture, NULL, &rects[i]);
					
					rects[i].x += rects[i].w;
					
					SDL_DestroyTexture(text_texture);
				}
			}
		}
		
		/* a hacky way of fixing font overshooting */
		cursor.x = l;
		cursor.y = cursorY * (font_size + 3);
		
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		
		SDL_RenderDrawRect(renderer, &cursor);
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		
		SDL_RenderPresent(renderer);
	}
	
	/* Cleanup */
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	TTF_Quit();
	SDL_Quit();
	
	return 0;
}
