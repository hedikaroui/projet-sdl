#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// Color structure for RGBA
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color;

// Sequence structure - represents a screen element
typedef struct {
    int id;                    // Unique identifier
    char name[64];             // Name of the sequence
    int x, y;                  // Position on screen
    int w, h;                  // Size (width, height)
    Color color;               // Background color (RGBA)
    char text_content[256];    // Text to display
    int shadow_offset_x;       // Shadow offset X (default: 2)
    int shadow_offset_y;       // Shadow offset Y (default: 2)
    Color shadow_color;        // Shadow color
    TTF_Font* font;            // Font for text rendering
    char font_path[256];       // Path to font file
    int font_size;             // Font size
    Color text_color;          // Text color
    int visible;               // Visibility flag (1 = visible, 0 = hidden)
    SDL_Texture* image;        // Image texture to display
    int image_width;           // Original image width
    int image_height;          // Original image height
    // Input field fields
    int is_input;              // 1 = this sequence is an input field
    int is_focused;            // 1 = currently active / receiving input
    char input_buffer[256];    // What the user has typed
    char placeholder[128];     // Hint text shown when empty
    int cursor_pos;            // Cursor position in input_buffer
    int cursor_visible;        // Cursor blink state (1 = shown)
    Uint32 cursor_timer;       // Timer for cursor blinking
} Sequence;

// Round Sequence structure - circular/round screen element
typedef struct {
    int id;                    // Unique identifier
    char name[64];             // Name of the round sequence
    int center_x, center_y;    // Center position
    int radius;                // Radius of the circle
    Color color;               // Fill color (RGBA)
    char text_content[256];    // Text to display
    TTF_Font* font;            // Font for text rendering
    int font_size;             // Font size
    Color text_color;          // Text color
    int visible;               // Visibility flag (1 = visible, 0 = hidden)
    int filled;                // 1 = filled circle, 0 = outline only
    int outline_thickness;     // Thickness of outline (if not filled)
} RoundSequence;

// Background structure
typedef struct {
    SDL_Texture* texture;
    SDL_Rect dest_rect;
    int width;
    int height;
    Mix_Music* music;      // Background music
    int music_volume;       // Volume (0-128)
} Background;

// Global variables
extern Background background;
extern Sequence sequences[100];  // Array to hold sequences
extern int sequence_count;       // Current number of sequences
extern RoundSequence round_sequences[50];  // Array to hold round sequences
extern int round_sequence_count;           // Current number of round sequences

// Background-related function declarations
int init_background(SDL_Renderer* renderer, const char* image_path);
void draw_background(SDL_Renderer* renderer);
void update_background(void);
void cleanup_background(void);

// Background music functions
int init_background_music(const char* music_path, int volume);
void play_background_music(void);
void stop_background_music(void);
void pause_background_music(void);
void resume_background_music(void);
void set_background_music_volume(int volume);
int get_background_music_volume(void);
int is_music_playing(void);
int is_music_paused(void);

// Sequence-related function declarations
void init_sequences(void);
int create_sequence(int id, const char* name, int x, int y, int w, int h, 
                    Color color, const char* text, int font_size);
void draw_sequence(SDL_Renderer* renderer, Sequence* seq);
void draw_all_sequences(SDL_Renderer* renderer);
Sequence* get_sequence_by_id(int id);
Sequence* get_sequence_by_name(const char* name);
void update_sequence_text(Sequence* seq, const char* new_text);
void update_sequence_position(Sequence* seq, int x, int y);
void update_sequence_color(Sequence* seq, Color new_color);
void set_sequence_visibility(Sequence* seq, int visible);
int load_sequence_image(SDL_Renderer* renderer, Sequence* seq, const char* image_path);
int load_sequence_font(Sequence* seq, const char* font_path, int font_size);
int load_font_all_sequences(const char* font_path);
void cleanup_sequences(void);

// Input field functions
void set_sequence_input(Sequence* seq, const char* placeholder);
void focus_input(Sequence* seq);
void unfocus_all_inputs(void);
void handle_input_event(SDL_Event* event);
void update_input_cursors(void);
void draw_input_sequence(SDL_Renderer* renderer, Sequence* seq);
Sequence* get_focused_input(void);

// Round Sequence-related function declarations
void init_round_sequences(void);
int create_round_sequence(int id, const char* name, int center_x, int center_y, 
                          int radius, Color color, const char* text, int font_size, 
                          int filled);
void draw_round_sequence(SDL_Renderer* renderer, RoundSequence* seq);
void draw_all_round_sequences(SDL_Renderer* renderer);
RoundSequence* get_round_sequence_by_id(int id);
RoundSequence* get_round_sequence_by_name(const char* name);
void update_round_sequence_text(RoundSequence* seq, const char* new_text);
void update_round_sequence_position(RoundSequence* seq, int center_x, int center_y);
void update_round_sequence_color(RoundSequence* seq, Color new_color);
void set_round_sequence_visibility(RoundSequence* seq, int visible);
void cleanup_round_sequences(void);

// Helper function to create colors easily
Color create_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#endif // HEADER_H
