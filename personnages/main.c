#include <stdio.h>
#include "header.h"

// Screen dimensions (adjust as needed)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720 

int main(void) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    int running = 1;
    
    printf("Initializing SDL2...\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL_image for PNG/JPG support
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_ttf for text rendering
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_mixer for audio
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    printf("SDL_mixer initialized (44.1kHz, stereo)\n");
    
    // Create window
    window = SDL_CreateWindow("Background Display",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Initialize background
    if (init_background(renderer, "background_main.jpg") != 0) {
        printf("Failed to initialize background\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Initialize and play background music
    // Volume set to 32 (25% of max 128) for subtle background sound
    if (init_background_music("background_sound.wav", 32) == 0) {
        play_background_music();
    } else {
        printf("Warning: Background music failed to load, continuing without music\n");
    }
    
    // Initialize sequences system
    init_sequences();
    
    // ============================================================================
    // MAIN CONTAINER - Transparent background
    // ============================================================================
    create_sequence(0, "main_container", 50, 50, 1180, 620,
                   create_color(0, 0, 0, 0),  // Fully transparent
                   "", 16);
    
    // Note: Sequence 0 provides the background area
    // Left area: x=50, w=350 (50 to 400)
    // Right area: x=600, w=350 (600 to 950)
    // Sequences 1 and 2 are centered within these areas
    
    // ============================================================================
    // SECTION 1: TOP SECTION - Contains 2 side-by-side sequences
    // ============================================================================
    // Left sequence in section 1 - centered in left half
    // Center position: x = 50 + (350 / 2) - (175 / 2) = 137.5
    //                  y = 50 + (175 / 2) - (175 / 2) = 50
    create_sequence(1, "section1_left", 300, 60, 200, 300,
                   create_color(255, 100, 100, 40),  // Light red, very low opacity
                   "Section 1 - Left", 18);
    
    // Right sequence in section 1 - centered in right half
    // Center position: x = 600 + (350 / 2) - (175 / 2) = 687.5
    create_sequence(2, "section1_right", 850, 60, 200 , 300,
                   create_color(100, 255, 100, 40),  // Light green, very low opacity
                   "Section 1 - Right", 18);
    
    // Load player images into sequences 1 and 2
    Sequence* player1 = get_sequence_by_id(1);
    Sequence* player2 = get_sequence_by_id(2);
    
    if (player1) {
        if (load_sequence_image(renderer, player1, "first_player.png") != 0) {
            printf("Warning: Failed to load first_player image\n");
        }
    }
    
    if (player2) {
        if (load_sequence_image(renderer, player2, "second_player.png") != 0) {
            printf("Warning: Failed to load second_player image\n");
        }
    }
    
    // ============================================================================
    // SECTION 2: MIDDLE SECTION - Under section 1, contains 2 separated sequences
    // ============================================================================
    // Full section 2 container (w: 1100, h: 170)
    // Positioned under section 1 (y: 50 + 175 + spacing = 245)
    create_sequence(3, "section2_container", 80, 380, 1100, 170,
                   create_color(100, 100, 255, 30),  // Light blue, extremely low opacity
                   "Section 2 - Container", 18);
    
    // Left sequence in section 2
    create_sequence(4, "section2_left", 250, 400, 315, 155,
                   create_color(255, 255, 100, 40),  // Light yellow, very low opacity
                   "Player 1 Controls", 16);
    
    // Player 1 keyboard keys (Q, Z, D, S) - centered in cross pattern
    // Center of section2_left: x=250+157.5=407.5, y=400+77.5=477.5
    int p1_center_x = 407;
    int p1_center_y = 477;
    int key_size = 50;
    int key_spacing = 60;
    
    // Q - LEFT key
    create_sequence(10, "p1_key_left", p1_center_x - key_spacing-35, p1_center_y - key_size/2 +20, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "Q", 20);
    
    // Z - TOP key
    create_sequence(11, "p1_key_up", p1_center_x - key_size/2, p1_center_y - key_spacing, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "Z", 20);
    
    // D - RIGHT key
    create_sequence(12, "p1_key_right", p1_center_x + key_spacing+35 - key_size, p1_center_y - key_size/2 +20, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "D", 20);
    
    // S - DOWN key
    create_sequence(13, "p1_key_down", p1_center_x - key_size/2 , p1_center_y + key_spacing - key_size +6, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "S", 20);
    
    // Right sequence in section 2 (separated with space)
    create_sequence(5, "section2_right", 780, 400, 315, 155,
                   create_color(255, 100, 255,0),  // Light magenta, very low opacity
                   "Player 2 Controls", 16);
    
    // Player 2 arrow keys - centered in cross pattern
    // Center of section2_right: x=780+157.5=937.5, y=400+77.5=477.5
    int p2_center_x = 937;
    int p2_center_y = 477;
    // key_size and key_spacing already defined above
    
    // LEFT ARROW
    create_sequence(14, "p2_key_left", p2_center_x - key_spacing -35, p2_center_y - key_size/2 +20, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "←", 24);
    
    // UP ARROW
    create_sequence(15, "p2_key_up", p2_center_x - key_size/2, p2_center_y - key_spacing, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "↑", 24);
    
    // RIGHT ARROW
    create_sequence(16, "p2_key_right", p2_center_x + key_spacing - key_size +35, p2_center_y - key_size/2 +20, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "→", 24);
    
    // DOWN ARROW
    create_sequence(17, "p2_key_down", p2_center_x - key_size/2, p2_center_y + key_spacing - key_size, 
                   key_size, key_size,
                   create_color(255, 255, 255, 255),  // White, semi-transparent
                   "↓", 24);
    
   // ============================================================================
    // SECTION 3: BOTTOM SECTION - Gets remaining space, contains 2 sequences
    // ============================================================================
    // Section 3 starts after section 2 (y: 245 + 200 + spacing = 465)
    // Height fills remaining space (620 total - 465 used = ~205 remaining)
    create_sequence(6, "section3_container", 80, 615, 1100, 144,
                   create_color(100, 255, 255, 30),  // Light cyan, extremely low opacity
                   "Section 3 - Container", 18);
    
    // Left sequence in section 3 (directly under section2_left)
    create_sequence(7, "section3_left", 230, 625, 380, 90,
                   create_color(255, 180, 100, 40),  // Light orange, very low opacity
                   "Section 3 - Left Part", 16);
    
    // Right sequence in section 3 (directly under section2_right)
    create_sequence(8, "section3_right", 780, 625, 380, 90,
                   create_color(180, 100, 255, 40),  // Light purple, very low opacity
                   "Section 3 - Right Part", 16);

    // ============================================================================
    // INPUT FIELDS - Enable sequences 7 and 8 as text input fields
    // ============================================================================
    Sequence* input7 = get_sequence_by_id(7);
    Sequence* input8 = get_sequence_by_id(8);

    if (input7) set_sequence_input(input7, "Player 1 name...");
    if (input8) set_sequence_input(input8, "Player 2 name...");
    
    printf("\n=== SEQUENCE LAYOUT CREATED ===\n");
    printf("Main Container: Transparent background\n");
    printf("Section 1 (Top): 2 player images (200x300 each)\n");
    printf("Section 2 (Middle): 2 control indicators with keyboard keys\n");
    printf("  - Player 1: Q (left), Z (up), D (right), S (down)\n");
    printf("  - Player 2: ← (left), ↑ (up), → (right), ↓ (down)\n");
    printf("Section 3 (Bottom): Left and Right parts\n");
    printf("================================\n\n");

    // ============================================================================
    // FONT LOADING - Load font for all sequences so text is visible
    // ============================================================================
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        NULL
    };

    int font_loaded = 0;
    for (int i = 0; font_paths[i] != NULL; i++) {
        if (load_font_all_sequences(font_paths[i]) > 0) {
            printf("Using font: %s\n", font_paths[i]);
            font_loaded = 1;
            break;
        }
    }
    if (!font_loaded) {
        printf("Warning: No system font found - text will not be displayed\n");
    }

    // ============================================================================
    // ROUND SEQUENCES - Initialize and create volume indicator
    // ============================================================================
    init_round_sequences();
    
    // Volume indicator - Round sequence in top-right corner
    // Position: top-right corner with 50px margin
    // Center: x = 1280 - 50 = 1230, y = 50
    // Radius: 40px, very low opacity
    create_round_sequence(100, "volume_indicator", 1230, 50, 40,
                         create_color(50, 100, 50, 40),  // Green, very low opacity
                         "32", 18, 1);  // filled circle
    
    printf("\nSDL2 initialized successfully!\n");
    printf("\n=== CONTROLS ===\n");
    printf("ESC         - Exit (or unfocus input)\n");
    printf("+           - Increase volume (+5)\n");
    printf("-           - Decrease volume (-5)\n");
    printf("P           - Pause music\n");
    printf("R           - Resume music\n");
    printf("--- Input fields (seq 7 & 8) ---\n");
    printf("Click       - Focus input field\n");
    printf("Type        - Write text\n");
    printf("Backspace   - Delete character\n");
    printf("Left/Right  - Move cursor\n");
    printf("Home/End    - Jump to start/end\n");
    printf("Tab         - Switch between fields\n");
    printf("Enter       - Confirm input\n");
    printf("================\n\n");
    
    // Main game loop
    SDL_Event event;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            // Route event to input system first (clicks, text, backspace …)
            handle_input_event(&event);

            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // If an input is focused, ESC unfocuses it; otherwise quit
                    if (get_focused_input()) {
                        unfocus_all_inputs();
                    } else {
                        running = 0;
                    }
                }
                // Volume controls - only when no input field is focused
                else if (!get_focused_input() &&
                         (event.key.keysym.sym == SDLK_PLUS ||
                          event.key.keysym.sym == SDLK_EQUALS)) {
                    set_background_music_volume(background.music_volume + 5);
                }
                else if (!get_focused_input() &&
                         (event.key.keysym.sym == SDLK_MINUS ||
                          event.key.keysym.sym == SDLK_UNDERSCORE)) {
                    set_background_music_volume(background.music_volume - 5);
                }
                else if (event.key.keysym.sym == SDLK_p) {
                    // Pause music
                    pause_background_music();
                }
                else if (event.key.keysym.sym == SDLK_r) {
                    // Resume music
                    resume_background_music();
                }
            }
        }
        
        // Update
        update_background();
        update_input_cursors();
        
        // Update volume indicator display (round sequence)
        RoundSequence* vol_indicator = get_round_sequence_by_name("volume_indicator");
        if (vol_indicator) {
            char vol_text[64];
            snprintf(vol_text, sizeof(vol_text), "%d", background.music_volume);
            update_round_sequence_text(vol_indicator, vol_text);
            
            // Change color based on volume level (very low opacity)
            if (background.music_volume < 40) {
                vol_indicator->color = create_color(50, 100, 50, 40); // Green for low
            } else if (background.music_volume < 80) {
                vol_indicator->color = create_color(100, 100, 50, 40); // Yellow for medium
            } else {
                vol_indicator->color = create_color(100, 50, 50, 40); // Orange for high
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Draw background
        draw_background(renderer);
        
        // Draw all sequences
        draw_all_sequences(renderer);
        
        // Draw all round sequences
        draw_all_round_sequences(renderer);
        
        // Present
        SDL_RenderPresent(renderer);
        
        // Small delay to prevent high CPU usage
        SDL_Delay(16); // ~60 FPS
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    cleanup_sequences();
    cleanup_round_sequences();
    cleanup_background();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    printf("Program ended\n");
    return 0;
}
