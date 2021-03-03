#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/* globals */

char *USER_FONT = "font.ttf";

SDL_Color bg_color = { 20, 20, 20, 255 };
SDL_Color text_color = { 150, 150, 150, 255 };
SDL_Color cursor_color = { 255, 255, 255, 255 };
SDL_Color line_number_color = { 255, 255, 255, 255 };
SDL_Color line_number_box_color = { 30, 30, 30, 255 };

int FONT_SIZE   = 16;
int TAB_SIZE    = 8;
int AUTO_INDENT = 1; /* 0 for off, 1 for on */

#define MIN_FONT_SIZE 15 /* warning: changing this can cause issues */
#define MAX_FONT_SIZE 30 /* warning: changing this can cause issues */

/* program */

struct line {
	char *text;
	int textcount;
};

char *
file_read(char *filepath)
{
	SDL_RWops *rw = SDL_RWFromFile(filepath, "rb");
	
	if (!rw) {
		printf("error: failed to open file '%s'\n", filepath);
		return NULL;
	}
	
	Sint64 fs = SDL_RWsize(rw);
	char *buffer = (char *)SDL_malloc(fs + 1);
	
	Sint64 read = 1, total = 0;
	
	char *buf = buffer;
	while (total < fs & read != 0) {
		read = SDL_RWread(rw, buf, 1, (fs - total));
		total += read;
		buf += read;
	}
	
	SDL_RWclose(rw);
	
	if (total != fs) {
		free(buffer);
		printf("error: failed to read file '%s'\n", filepath);
		
		return NULL;
	}
	
	buffer[total] = 0;
	
	return buffer;
}

char *
num_to_str(int number)
{
	int len = snprintf(NULL, 0, "%d", number);
	
	char *buf = (char *)SDL_malloc(len + 1);
	snprintf(buf, len + 1, "%d", number);
	
	return buf;
}

int
main(int argc, char *argv[])
{	
	/* Parse file */
	char *file = NULL, *filepath = argc > 1 ? argv[1] : "out.txt";
	
	if (argc > 1)
		file = file_read(filepath);
	
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
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	
	if (!window) {
		printf("error: failed to open display\n");
		return 1;
	}
	
	SDL_Renderer *renderer = SDL_GetRenderer(window);
	
	if (!renderer)
		renderer = SDL_CreateRenderer(
			window,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		);
	
	if (!renderer) {
		printf("failed to create graphics\n");
		return 1;
	}
	
	TTF_Font *font = TTF_OpenFont(USER_FONT, FONT_SIZE);
	
	if (!font) {
		printf("could not open font\n");
		return 1;
	}
	
	SDL_Cursor* cursorInput = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	
	if (!cursorInput) {
		printf("could not initalize cursor\n");
	}
	
	SDL_SetCursor(cursorInput);
	
	/* Data */
	
	int wndw, wndh, pwndw, pwndh;
	
	SDL_GetWindowSize(window, &wndw, &wndh);
	
	pwndw = wndw;
	pwndh = wndh;
	
	int previous_font = FONT_SIZE;
	
	int running = 1;
	
	SDL_Event event;
	
	int cursorX = 0, cursorY = 0;
	
	int linecount = 1, scrolloffset = 0, sideoffset = 0, horizontal_offset = 100;
	struct line *lines = (struct line *)SDL_malloc(sizeof(struct line));
	
	int i, j, k, l, m; /* used for text processing */
	
	/* initialize text for lines */
	for (i = 0; i < linecount; i++) {
		lines[i].text = (char *)SDL_malloc(1);
		lines[i].text[0] = 0;
		lines[i].textcount = 0;
	}
	
	/* load file if specified */
	
	if (file) {
		for (i = 0; file[i] != 0; i++) {
			if (file[i] == '\n') {
				++linecount;
				
				lines = (struct line *)SDL_realloc(
					lines,
					sizeof(struct line) * linecount
				);
				
				lines[linecount - 1].textcount = 0;
				lines[linecount - 1].text = (char *)SDL_malloc(1);
				
				lines[linecount - 1].text[0] = 0;
			}
			
			lines[linecount - 1].textcount++;
			
			lines[linecount - 1].text = (char *)SDL_realloc(
				lines[linecount - 1].text,
				lines[linecount - 1].textcount + 1
			);
			
			lines[linecount - 1].text[lines[linecount - 1].textcount - 1] = file[i];
			lines[linecount - 1].text[lines[linecount - 1].textcount] = 0;
		}
	}
	
	char *tab = (char *)malloc(TAB_SIZE + 1);
	
	for (i = 0; i < TAB_SIZE; i++)
		tab[i] = ' ';
	
	tab[TAB_SIZE] = 0;
	
	int fw, fh;
	
	TTF_SizeText(font, "X", &fw, &fh);
	
	SDL_Rect cursor;
	
	cursor.x = cursorX;
	cursor.y = cursorY;
	cursor.w = 1;
	cursor.h = FONT_SIZE;
	
	SDL_Rect linenumber;
	
	linenumber.x = 0;
	linenumber.h = FONT_SIZE + 2;
	
	int rectcount = wndh / fh;
	SDL_Rect *rects = (SDL_Rect *)SDL_malloc(sizeof(SDL_Rect) * rectcount);
	
	for (i = 0; i < rectcount; i++) {
		rects[i].x = horizontal_offset;
		rects[i].y = i * (FONT_SIZE + 2);
		rects[i].w = 0;
		rects[i].h = FONT_SIZE + 2;
	}
	
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
								if (cursorY < scrolloffset)
									--scrolloffset;
							}
							break;
						case SDLK_DOWN:
							if (cursorY < linecount - 1) {
								++cursorY;
								if (cursorX > lines[cursorY].textcount - 1)
									cursorX = lines[cursorY].textcount;
								if (cursorY >= rectcount + scrolloffset)
									++scrolloffset;
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
									lines[cursorY - 1].textcount += lines[cursorY].textcount;
									
									lines[cursorY - 1].text = (char *)SDL_realloc(
										lines[cursorY - 1].text,
										lines[cursorY - 1].textcount + 1
									);
									
									for (i = 0; i < lines[cursorY].textcount + 1; i++)
										lines[cursorY - 1].text[lines[cursorY - 1].textcount - lines[cursorY].textcount + i] = lines[cursorY].text[i];
									
									SDL_free(lines[cursorY].text);
									
									cursorX = lines[cursorY - 1].textcount - lines[cursorY].textcount;
									
									for (i = cursorY; i < linecount - 1; i++)
										lines[i] = lines[i + 1];
									
									--linecount;
									
									lines = (struct line *)SDL_realloc(
										lines,
										sizeof(struct line) * linecount
									);
									
									--cursorY;
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
							
							if (cursorY >= rectcount + scrolloffset)
								++scrolloffset;
							
							j = (lines[cursorY - 1].textcount) - cursorX;
							
							m = 0;
							
							if (AUTO_INDENT) {
								for (i = 0; i < lines[cursorY - 1].textcount; i++) {
									if (lines[cursorY - 1].text[i] == '\t')
										++m;
									else
										break;
								}
							}
							
							lines[cursorY].text = (char *)SDL_malloc(j + m + 1);
							lines[cursorY].textcount = j + m;
							
							for (i = 0; i < m; i++)
								lines[cursorY].text[i] = '\t';
							
							for (i = 0; i < j + 1; i++)
								lines[cursorY].text[i + m] = lines[cursorY - 1].text[cursorX + i];
							
							lines[cursorY - 1].text = (char *)SDL_realloc(
								lines[cursorY - 1].text,
								cursorX + 1
							);
							
							lines[cursorY - 1].text[cursorX] = 0;
							lines[cursorY - 1].textcount = cursorX;
							
							cursorX = m;
							
							break;
						case SDLK_TAB:
							lines[cursorY].textcount++;
							
							lines[cursorY].text = (char *)SDL_realloc(
								lines[cursorY].text,
								lines[cursorY].textcount + 1
							);
							
							for (i = lines[cursorY].textcount; i > cursorX; i--)
								lines[cursorY].text[i] = lines[cursorY].text[i - 1];
							
							lines[cursorY].text[cursorX] = '\t';
							
							++cursorX;
							
							break;
					}
					break;
				case SDL_TEXTINPUT:
					if (SDL_GetModState() & KMOD_CTRL) {
						switch (event.text.text[0]) {
							case '=':
								if (FONT_SIZE < MAX_FONT_SIZE)
									FONT_SIZE++;
								break;
							case '-':
								if (FONT_SIZE > MIN_FONT_SIZE)
									FONT_SIZE--;
								break;
							case 's':
								
								break;
						}
						break;
					}
					
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
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0) {
						if (SDL_GetModState() & KMOD_CTRL) {
							if (cursorX < lines[cursorY].textcount) {
								++cursorX;
							}
						} else {
							if (cursorY > 0) {
								--cursorY;
								if (cursorX > lines[cursorY].textcount - 1)
									cursorX = lines[cursorY].textcount;
								if (cursorY < scrolloffset)
									--scrolloffset;
							}
						}
					} else if (event.wheel.y < 0) {
						if (SDL_GetModState() & KMOD_CTRL) {
							if (cursorX > 0) {
								--cursorX;
							}
						} else {
							if (cursorY < linecount - 1) {
								++cursorY;
								if (cursorX > lines[cursorY].textcount - 1)
									cursorX = lines[cursorY].textcount;
								if (cursorY >= rectcount + scrolloffset)
									++scrolloffset;
							}
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					m = (int)((event.button.y / (FONT_SIZE + 2)) + scrolloffset);
					
					if (m < linecount)
						cursorY = m;
					
					/* TODO: Implement horizontal mouse selection */
					
					break;
			}
		}
		
		/* Update */
		
		l = 0;
		
		SDL_GetWindowSize(window, &wndw, &wndh);
		
		if (pwndw != wndw || pwndh != wndh) {
			SDL_free(rects);
	
			rectcount = wndh / fh;
			rects = (SDL_Rect *)SDL_malloc(sizeof(SDL_Rect) * rectcount);
			
			for (i = 0; i < rectcount; i++) {
				rects[i].x = horizontal_offset;
				rects[i].y = i * (FONT_SIZE + 2);
				rects[i].w = 0;
				rects[i].h = FONT_SIZE + 2;
			}
			
			pwndw = wndw;
			pwndh = wndh;
		}
		
		if (previous_font != FONT_SIZE) {
			TTF_CloseFont(font);
			
			font = TTF_OpenFont(USER_FONT, FONT_SIZE);
			
			if (!font) {
				printf("error: failed to open font\n");
				return 1;
			}
			
			TTF_SizeText(font, "X", &fw, &fh);
			
			cursor.h = FONT_SIZE;
			
			SDL_free(rects);
	
			rectcount = wndh / fh;
			rects = (SDL_Rect *)SDL_malloc(sizeof(SDL_Rect) * rectcount);
			
			for (i = 0; i < rectcount; i++) {
				rects[i].x = horizontal_offset;
				rects[i].y = i * (FONT_SIZE + 2);
				rects[i].w = 0;
				rects[i].h = FONT_SIZE + 2;
			}
		}
		
		/* Rendering */
		
		SDL_RenderClear(renderer);
		
		char *max = num_to_str(linecount);
			
		linenumber.h = FONT_SIZE + 2;
		
		TTF_SizeText(font, max, &linenumber.w, NULL);
		
		SDL_free(max);
		
		horizontal_offset = linenumber.w + 50;
	
		for (i = 0; i < rectcount; i++) {
			if ((i + scrolloffset) >= linecount)
				break;
			
			linenumber.y = rects[i].y;
			
			SDL_SetRenderDrawColor(
				renderer,
				line_number_box_color.r,
				line_number_box_color.g,
				line_number_box_color.b,
				line_number_box_color.a
			);
			
			SDL_RenderDrawRect(renderer, &linenumber);
			
			char *linenum = num_to_str(i + scrolloffset + 1);
			
			TTF_SizeText(font, linenum, &linenumber.w, NULL);
			
			text_surface = TTF_RenderText_Blended(font, linenum, line_number_color);
			text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
			
			SDL_free(linenum);
			
			SDL_FreeSurface(text_surface);
			
			SDL_RenderCopy(renderer, text_texture, NULL, &linenumber);
			
			SDL_DestroyTexture(text_texture);
			
			SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
			
			if (lines[i + scrolloffset].textcount > 0) {
				rects[i].x = horizontal_offset;
				
				for (j = 0; lines[i + scrolloffset].text[j] != 0; j++) {		
					k = lines[i + scrolloffset].text[j];
					
					if (k == '\t')
						text_surface = TTF_RenderText_Blended(font, tab, text_color);
					else if (k < 32)
						continue;
					else
						text_surface = TTF_RenderGlyph_Blended(font, k, text_color);
					
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
		cursor.x = horizontal_offset + l;
		cursor.y = (cursorY - scrolloffset) * (FONT_SIZE + 2);
		
		SDL_SetRenderDrawColor(renderer, cursor_color.r, cursor_color.g, cursor_color.b, cursor_color.a);
		
		SDL_RenderDrawRect(renderer, &cursor);
		
		SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
		
		SDL_RenderPresent(renderer);
	}
	
	/* Cleanup */
	
	SDL_StopTextInput();
	
	SDL_free(tab);
	
	for (i = 0; i < linecount; i++)
		SDL_free(lines[i].text);
	
	SDL_free(lines);
	SDL_free(rects);
	
	SDL_FreeCursor(cursorInput);
	
	TTF_CloseFont(font);
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	TTF_Quit();
	SDL_Quit();
	
	return 0;
}
