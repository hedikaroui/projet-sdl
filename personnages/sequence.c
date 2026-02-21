#include <stdio.h>
#include <string.h>
#include <math.h>
#include "header.h"

// Global variables
Sequence sequences[100];
int sequence_count = 0;
RoundSequence round_sequences[50];
int round_sequence_count = 0;

// Helper function to create colors
Color create_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    Color color = {r, g, b, a};
    return color;
}

// Initialize sequences system
void init_sequences(void) {
    sequence_count = 0;
    memset(sequences, 0, sizeof(sequences));
    printf("Sequences system initialized\n");
}

// Create a new sequence
int create_sequence(int id, const char* name, int x, int y, int w, int h, 
                    Color color, const char* text, int font_size) {
    if (sequence_count >= 100) {
        printf("Error: Maximum sequences limit reached (100)\n");
        return -1;
    }
    
    // Check if ID already exists
    for (int i = 0; i < sequence_count; i++) {
        if (sequences[i].id == id) {
            printf("Error: Sequence with ID %d already exists\n", id);
            return -1;
        }
    }
    
    Sequence* seq = &sequences[sequence_count];
    
    // Set basic properties
    seq->id = id;
    strncpy(seq->name, name, sizeof(seq->name) - 1);
    seq->name[sizeof(seq->name) - 1] = '\0';
    
    seq->x = x;
    seq->y = y;
    seq->w = w;
    seq->h = h;
    
    seq->color = color;
    
    // Set text content
    strncpy(seq->text_content, text, sizeof(seq->text_content) - 1);
    seq->text_content[sizeof(seq->text_content) - 1] = '\0';
    
    // Set shadow properties (default offset: x+2, y+2)
    seq->shadow_offset_x = 2;
    seq->shadow_offset_y = 2;
    seq->shadow_color = create_color(0, 0, 0, 180); // Semi-transparent black
    
    // Set font properties
    seq->font_size = font_size;
    seq->font = NULL; // Will be loaded via load_sequence_font()
    seq->font_path[0] = '\0';
    
    // Default text color (white)
    seq->text_color = create_color(255, 255, 255, 255);
    
    // Visible by default
    seq->visible = 1;
    
    // No image by default
    seq->image        = NULL;
    seq->image_width  = 0;
    seq->image_height = 0;

    // Input field - disabled by default
    seq->is_input       = 0;
    seq->is_focused     = 0;
    seq->input_buffer[0] = '\0';
    seq->placeholder[0]  = '\0';
    seq->cursor_pos     = 0;
    seq->cursor_visible = 0;
    seq->cursor_timer   = 0;
    
    sequence_count++;
    printf("Created sequence: ID=%d, Name='%s' at (%d, %d) size %dx%d\n", 
           id, name, x, y, w, h);
    
    return sequence_count - 1; // Return index
}

// Draw a single sequence
void draw_sequence(SDL_Renderer* renderer, Sequence* seq) {
    if (!seq || !seq->visible) return;

    // Delegate input fields to their own draw function
    if (seq->is_input) {
        draw_input_sequence(renderer, seq);
        return;
    }
    
    // Special handling for volume_indicator - draw as circle
    if (strcmp(seq->name, "volume_indicator") == 0) {
        // Draw filled circle for volume indicator
        int centerX = seq->x + seq->w / 2;
        int centerY = seq->y + seq->h / 2;
        int radius = (seq->w < seq->h ? seq->w : seq->h) / 2;
        
        // Draw filled circle using multiple horizontal lines
        SDL_SetRenderDrawColor(renderer, seq->color.r, seq->color.g, seq->color.b, seq->color.a);
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)sqrt(radius * radius - dy * dy);
            SDL_RenderDrawLine(renderer, centerX - dx, centerY + dy, centerX + dx, centerY + dy);
        }
        
        // Draw circle border
        SDL_SetRenderDrawColor(renderer, 
                              seq->color.r / 2, 
                              seq->color.g / 2, 
                              seq->color.b / 2, 
                              255);
        // Simple circle outline using points
        for (int angle = 0; angle < 360; angle += 2) {
            float rad = angle * 3.14159 / 180.0;
            int x = centerX + (int)(radius * cos(rad));
            int y = centerY + (int)(radius * sin(rad));
            SDL_RenderDrawPoint(renderer, x, y);
        }
    } else {
        // Normal rectangle drawing for other sequences
        SDL_Rect rect = {seq->x, seq->y, seq->w, seq->h};
        SDL_SetRenderDrawColor(renderer, seq->color.r, seq->color.g, seq->color.b, seq->color.a);
        SDL_RenderFillRect(renderer, &rect);
        
        // Draw border (optional - darker version of the color)
        SDL_SetRenderDrawColor(renderer, 
                              seq->color.r / 2, 
                              seq->color.g / 2, 
                              seq->color.b / 2, 
                              255);
        SDL_RenderDrawRect(renderer, &rect);
        
        // Draw image if present (scaled to fit sequence size)
        if (seq->image) {
            SDL_RenderCopy(renderer, seq->image, NULL, &rect);
        }
    }
    
    // Draw text if present (centered inside the sequence)
    if (strlen(seq->text_content) > 0 && seq->font) {
        // Render shadow
        SDL_Surface* shadow_surface = TTF_RenderUTF8_Blended(
            seq->font,
            seq->text_content,
            (SDL_Color){seq->shadow_color.r, seq->shadow_color.g,
                       seq->shadow_color.b, seq->shadow_color.a}
        );

        if (shadow_surface) {
            SDL_Texture* shadow_texture = SDL_CreateTextureFromSurface(renderer, shadow_surface);
            if (shadow_texture) {
                SDL_Rect shadow_rect = {
                    seq->x + (seq->w - shadow_surface->w) / 2 + seq->shadow_offset_x,
                    seq->y + (seq->h - shadow_surface->h) / 2 + seq->shadow_offset_y,
                    shadow_surface->w,
                    shadow_surface->h
                };
                SDL_RenderCopy(renderer, shadow_texture, NULL, &shadow_rect);
                SDL_DestroyTexture(shadow_texture);
            }
            SDL_FreeSurface(shadow_surface);
        }

        // Render main text centered
        SDL_Surface* text_surface = TTF_RenderUTF8_Blended(
            seq->font,
            seq->text_content,
            (SDL_Color){seq->text_color.r, seq->text_color.g,
                       seq->text_color.b, seq->text_color.a}
        );

        if (text_surface) {
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            if (text_texture) {
                SDL_Rect text_rect = {
                    seq->x + (seq->w - text_surface->w) / 2,
                    seq->y + (seq->h - text_surface->h) / 2,
                    text_surface->w,
                    text_surface->h
                };
                SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

// Draw all sequences
void draw_all_sequences(SDL_Renderer* renderer) {
    for (int i = 0; i < sequence_count; i++) {
        draw_sequence(renderer, &sequences[i]);
    }
}

// Get sequence by ID
Sequence* get_sequence_by_id(int id) {
    for (int i = 0; i < sequence_count; i++) {
        if (sequences[i].id == id) {
            return &sequences[i];
        }
    }
    printf("Warning: Sequence with ID %d not found\n", id);
    return NULL;
}

// Get sequence by name
Sequence* get_sequence_by_name(const char* name) {
    for (int i = 0; i < sequence_count; i++) {
        if (strcmp(sequences[i].name, name) == 0) {
            return &sequences[i];
        }
    }
    printf("Warning: Sequence with name '%s' not found\n", name);
    return NULL;
}

// Update sequence text content
void update_sequence_text(Sequence* seq, const char* new_text) {
    if (!seq) return;
    
    strncpy(seq->text_content, new_text, sizeof(seq->text_content) - 1);
    seq->text_content[sizeof(seq->text_content) - 1] = '\0';
}

// Update sequence position
void update_sequence_position(Sequence* seq, int x, int y) {
    if (!seq) return;
    
    seq->x = x;
    seq->y = y;
}

// Update sequence color
void update_sequence_color(Sequence* seq, Color new_color) {
    if (!seq) return;
    
    seq->color = new_color;
}

// Set sequence visibility
void set_sequence_visibility(Sequence* seq, int visible) {
    if (!seq) return;
    
    seq->visible = visible;
}

// Load a font into a specific sequence
int load_sequence_font(Sequence* seq, const char* font_path, int font_size) {
    if (!seq) return -1;
    
    // Close previous font if any
    if (seq->font) {
        TTF_CloseFont(seq->font);
        seq->font = NULL;
    }
    
    seq->font = TTF_OpenFont(font_path, font_size);
    if (!seq->font) {
        printf("Failed to load font '%s': %s\n", font_path, TTF_GetError());
        return -1;
    }
    
    // Store font path and size
    strncpy(seq->font_path, font_path, sizeof(seq->font_path) - 1);
    seq->font_path[sizeof(seq->font_path) - 1] = '\0';
    seq->font_size = font_size;
    
    printf("Font loaded for sequence '%s': %s (size %d)\n", seq->name, font_path, font_size);
    return 0;
}

// Load a font for ALL sequences at once
int load_font_all_sequences(const char* font_path) {
    int loaded = 0;
    for (int i = 0; i < sequence_count; i++) {
        if (strlen(sequences[i].text_content) > 0) {
            if (load_sequence_font(&sequences[i], font_path, sequences[i].font_size) == 0) {
                loaded++;
            }
        }
    }
    printf("Font loaded for %d sequences\n", loaded);
    return loaded;
}

// Load an image into a sequence
int load_sequence_image(SDL_Renderer* renderer, Sequence* seq, const char* image_path) {
    if (!seq || !renderer) {
        printf("Error: Invalid sequence or renderer\n");
        return -1;
    }
    
    printf("Loading image for sequence '%s': %s\n", seq->name, image_path);
    
    // Load image as surface
    SDL_Surface* surface = IMG_Load(image_path);
    if (!surface) {
        printf("Failed to load image '%s': %s\n", image_path, IMG_GetError());
        return -1;
    }
    
    // Store original dimensions
    seq->image_width = surface->w;
    seq->image_height = surface->h;
    
    // Create texture from surface
    seq->image = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!seq->image) {
        printf("Failed to create texture from image: %s\n", SDL_GetError());
        return -1;
    }
    
    printf("Image loaded successfully for sequence '%s' (%dx%d)\n", 
           seq->name, seq->image_width, seq->image_height);
    return 0;
}

// Cleanup sequences
void cleanup_sequences(void) {
    for (int i = 0; i < sequence_count; i++) {
        if (sequences[i].font) {
            TTF_CloseFont(sequences[i].font);
            sequences[i].font = NULL;
        }
        // Free image texture if present
        if (sequences[i].image) {
            SDL_DestroyTexture(sequences[i].image);
            sequences[i].image = NULL;
        }
    }
    sequence_count = 0;
    printf("Sequences cleaned up\n");
}

// =============================================================================
// ROUND SEQUENCE FUNCTIONS
// =============================================================================

// Helper function to draw filled circle
static void draw_filled_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
}

// Helper function to draw circle outline
static void draw_circle_outline(SDL_Renderer* renderer, int center_x, int center_y, int radius, int thickness) {
    for (int t = 0; t < thickness; t++) {
        int r = radius - t;
        int x = r;
        int y = 0;
        int err = 0;
        
        while (x >= y) {
            SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
            SDL_RenderDrawPoint(renderer, center_x + y, center_y + x);
            SDL_RenderDrawPoint(renderer, center_x - y, center_y + x);
            SDL_RenderDrawPoint(renderer, center_x - x, center_y + y);
            SDL_RenderDrawPoint(renderer, center_x - x, center_y - y);
            SDL_RenderDrawPoint(renderer, center_x - y, center_y - x);
            SDL_RenderDrawPoint(renderer, center_x + y, center_y - x);
            SDL_RenderDrawPoint(renderer, center_x + x, center_y - y);
            
            if (err <= 0) {
                y += 1;
                err += 2*y + 1;
            }
            if (err > 0) {
                x -= 1;
                err -= 2*x + 1;
            }
        }
    }
}

// Initialize round sequences system
void init_round_sequences(void) {
    round_sequence_count = 0;
    memset(round_sequences, 0, sizeof(round_sequences));
    printf("Round sequences system initialized\n");
}

// Create a new round sequence
int create_round_sequence(int id, const char* name, int center_x, int center_y, 
                          int radius, Color color, const char* text, int font_size, 
                          int filled) {
    if (round_sequence_count >= 50) {
        printf("Error: Maximum round sequences limit reached (50)\n");
        return -1;
    }
    
    // Check if ID already exists
    for (int i = 0; i < round_sequence_count; i++) {
        if (round_sequences[i].id == id) {
            printf("Error: Round sequence with ID %d already exists\n", id);
            return -1;
        }
    }
    
    RoundSequence* seq = &round_sequences[round_sequence_count];
    
    // Set basic properties
    seq->id = id;
    strncpy(seq->name, name, sizeof(seq->name) - 1);
    seq->name[sizeof(seq->name) - 1] = '\0';
    
    seq->center_x = center_x;
    seq->center_y = center_y;
    seq->radius = radius;
    seq->color = color;
    
    // Set text content
    strncpy(seq->text_content, text, sizeof(seq->text_content) - 1);
    seq->text_content[sizeof(seq->text_content) - 1] = '\0';
    
    // Set font properties
    seq->font_size = font_size;
    seq->font = NULL; // Will be loaded when needed
    
    // Default text color (white)
    seq->text_color = create_color(255, 255, 255, 255);
    
    // Visible by default
    seq->visible = 1;
    
    // Set filled/outline properties
    seq->filled = filled;
    seq->outline_thickness = 3; // Default outline thickness
    
    round_sequence_count++;
    printf("Created round sequence: ID=%d, Name='%s' at (%d, %d) radius=%d\n", 
           id, name, center_x, center_y, radius);
    
    return round_sequence_count - 1; // Return index
}

// Draw a single round sequence
void draw_round_sequence(SDL_Renderer* renderer, RoundSequence* seq) {
    if (!seq || !seq->visible) {
        return;
    }
    
    // Set color with alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, seq->color.r, seq->color.g, seq->color.b, seq->color.a);
    
    // Draw circle (filled or outline)
    if (seq->filled) {
        draw_filled_circle(renderer, seq->center_x, seq->center_y, seq->radius);
    } else {
        draw_circle_outline(renderer, seq->center_x, seq->center_y, seq->radius, seq->outline_thickness);
    }
    
    // Draw text if present
    if (strlen(seq->text_content) > 0 && seq->font) {
        SDL_Surface* text_surface = TTF_RenderUTF8_Blended(
            seq->font, 
            seq->text_content, 
            (SDL_Color){seq->text_color.r, seq->text_color.g, 
                       seq->text_color.b, seq->text_color.a}
        );
        
        if (text_surface) {
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            if (text_texture) {
                SDL_Rect text_rect = {
                    seq->center_x - text_surface->w / 2,
                    seq->center_y - text_surface->h / 2,
                    text_surface->w,
                    text_surface->h
                };
                SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

// Draw all round sequences
void draw_all_round_sequences(SDL_Renderer* renderer) {
    for (int i = 0; i < round_sequence_count; i++) {
        draw_round_sequence(renderer, &round_sequences[i]);
    }
}

// Get round sequence by ID
RoundSequence* get_round_sequence_by_id(int id) {
    for (int i = 0; i < round_sequence_count; i++) {
        if (round_sequences[i].id == id) {
            return &round_sequences[i];
        }
    }
    printf("Warning: Round sequence with ID %d not found\n", id);
    return NULL;
}

// Get round sequence by name
RoundSequence* get_round_sequence_by_name(const char* name) {
    for (int i = 0; i < round_sequence_count; i++) {
        if (strcmp(round_sequences[i].name, name) == 0) {
            return &round_sequences[i];
        }
    }
    printf("Warning: Round sequence with name '%s' not found\n", name);
    return NULL;
}

// Update round sequence text content
void update_round_sequence_text(RoundSequence* seq, const char* new_text) {
    if (!seq) return;
    
    strncpy(seq->text_content, new_text, sizeof(seq->text_content) - 1);
    seq->text_content[sizeof(seq->text_content) - 1] = '\0';
}

// Update round sequence position
void update_round_sequence_position(RoundSequence* seq, int center_x, int center_y) {
    if (!seq) return;
    
    seq->center_x = center_x;
    seq->center_y = center_y;
}

// Update round sequence color
void update_round_sequence_color(RoundSequence* seq, Color new_color) {
    if (!seq) return;
    
    seq->color = new_color;
}

// Set round sequence visibility
void set_round_sequence_visibility(RoundSequence* seq, int visible) {
    if (!seq) return;
    
    seq->visible = visible;
}

// Cleanup round sequences
void cleanup_round_sequences(void) {
    for (int i = 0; i < round_sequence_count; i++) {
        if (round_sequences[i].font) {
            TTF_CloseFont(round_sequences[i].font);
            round_sequences[i].font = NULL;
        }
    }
    round_sequence_count = 0;
    printf("Round sequences cleaned up\n");
}
