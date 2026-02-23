#include <stdio.h>
#include <string.h>
#include "header.h"    

#define CURSOR_BLINK_MS 500  // Blink every 500ms

// Enable a sequence as an input field
void set_sequence_input(Sequence* seq, const char* placeholder) {
    if (!seq) return;

    seq->is_input        = 1;
    seq->is_focused     = 0;
    seq->input_buffer[0] = '\0';
    seq->cursor_pos     = 0;
    seq->cursor_visible = 1;
    seq->cursor_timer   = SDL_GetTicks();

    strncpy(seq->placeholder, placeholder, sizeof(seq->placeholder) - 1);
    seq->placeholder[sizeof(seq->placeholder) - 1] = '\0';

    printf("Input field enabled on sequence '%s' (placeholder: \"%s\")\n",
           seq->name, placeholder);
}

// Give focus to a specific input sequence
void focus_input(Sequence* seq) {
    if (!seq || !seq->is_input) return;

    // Unfocus everything first
    unfocus_all_inputs();

    seq->is_focused     = 1;
    seq->cursor_visible = 1;
    seq->cursor_timer   = SDL_GetTicks();

    // Tell SDL to start capturing text input
    SDL_StartTextInput();

    printf("Input focused: '%s'\n", seq->name);
}

// Remove focus from every input field
void unfocus_all_inputs(void) {
    for (int i = 0; i < sequence_count; i++) {
        if (sequences[i].is_input && sequences[i].is_focused) {
            sequences[i].is_focused = 0;
            printf("Input unfocused: '%s' | content: \"%s\"\n",
                   sequences[i].name, sequences[i].input_buffer);
        }
    }
    SDL_StopTextInput();
}

// Return the currently focused input, or NULL
Sequence* get_focused_input(void) {
    for (int i = 0; i < sequence_count; i++) {
        if (sequences[i].is_input && sequences[i].is_focused)
            return &sequences[i];
    }
    return NULL;
}

// Update cursor blink timer for all inputs
void update_input_cursors(void) {
    Uint32 now = SDL_GetTicks();
    for (int i = 0; i < sequence_count; i++) {
        Sequence* seq = &sequences[i];
        if (!seq->is_input || !seq->is_focused) continue;

        if (now - seq->cursor_timer >= CURSOR_BLINK_MS) {
            seq->cursor_visible = !seq->cursor_visible;
            seq->cursor_timer   = now;
        }
    }
}

// Handle SDL events for input fields
void handle_input_event(SDL_Event* event) {

    // ── Mouse click: focus / unfocus ──────────────────────────────────────────
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;
        int hit = 0;

        for (int i = 0; i < sequence_count; i++) {
            Sequence* seq = &sequences[i];
            if (!seq->is_input || !seq->visible) continue;

            if (mx >= seq->x && mx <= seq->x + seq->w &&
                my >= seq->y && my <= seq->y + seq->h) {
                focus_input(seq);
                hit = 1;
                break;
            }
        }
        if (!hit) unfocus_all_inputs();
        return;
    }

    // ── Text input (printable characters) ────────────────────────────────────
    if (event->type == SDL_TEXTINPUT) {
        Sequence* seq = get_focused_input();
        if (!seq) return;

        int buf_len = (int)strlen(seq->input_buffer);
        const char* txt = event->text.text;
        int txt_len = (int)strlen(txt);

        if (buf_len + txt_len < (int)sizeof(seq->input_buffer) - 1) {
            // Insert text at cursor position
            memmove(seq->input_buffer + seq->cursor_pos + txt_len,
                    seq->input_buffer + seq->cursor_pos,
                    buf_len - seq->cursor_pos + 1);
            memcpy(seq->input_buffer + seq->cursor_pos, txt, txt_len);
            seq->cursor_pos += txt_len;
        }
        return;
    }

    // ── Special keys ─────────────────────────────────────────────────────────
    if (event->type == SDL_KEYDOWN) {
        Sequence* seq = get_focused_input();
        if (!seq) return;

        int buf_len = (int)strlen(seq->input_buffer);

        switch (event->key.keysym.sym) {

            // Backspace: delete character before cursor
            case SDLK_BACKSPACE:
                if (seq->cursor_pos > 0) {
                    memmove(seq->input_buffer + seq->cursor_pos - 1,
                            seq->input_buffer + seq->cursor_pos,
                            buf_len - seq->cursor_pos + 1);
                    seq->cursor_pos--;
                }
                break;

            // Delete: delete character after cursor
            case SDLK_DELETE:
                if (seq->cursor_pos < buf_len) {
                    memmove(seq->input_buffer + seq->cursor_pos,
                            seq->input_buffer + seq->cursor_pos + 1,
                            buf_len - seq->cursor_pos);
                }
                break;

            // Left arrow: move cursor left
            case SDLK_LEFT:
                if (seq->cursor_pos > 0) seq->cursor_pos--;
                break;

            // Right arrow: move cursor right
            case SDLK_RIGHT:
                if (seq->cursor_pos < buf_len) seq->cursor_pos++;
                break;

            // Home: jump to start
            case SDLK_HOME:
                seq->cursor_pos = 0;
                break;

            // End: jump to end
            case SDLK_END:
                seq->cursor_pos = buf_len;
                break;

            // Enter: confirm and unfocus
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                printf("Input confirmed in '%s': \"%s\"\n",
                       seq->name, seq->input_buffer);
                unfocus_all_inputs();
                break;

            // Tab: cycle to next input field
            case SDLK_TAB: {
                int current_idx = -1;
                for (int i = 0; i < sequence_count; i++) {
                    if (sequences[i].is_input && sequences[i].is_focused) {
                        current_idx = i;
                        break;
                    }
                }
                if (current_idx >= 0) {
                    // Find next input field
                    for (int i = 1; i <= sequence_count; i++) {
                        int next = (current_idx + i) % sequence_count;
                        if (sequences[next].is_input && sequences[next].visible) {
                            focus_input(&sequences[next]);
                            break;
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

// Draw an input sequence (called by draw_sequence when is_input == 1)
void draw_input_sequence(SDL_Renderer* renderer, Sequence* seq) {
    if (!seq || !seq->visible) return;

    SDL_Rect rect = {seq->x, seq->y, seq->w, seq->h};

    // ── Background ───────────────────────────────────────────────────────────
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           seq->color.r, seq->color.g,
                           seq->color.b, seq->color.a);
    SDL_RenderFillRect(renderer, &rect);

    // ── Border: white when focused, dim when not ──────────────────────────────
    if (seq->is_focused) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
    } else {
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 120);
    }
    SDL_RenderDrawRect(renderer, &rect);

    // Inner border for focused field
    if (seq->is_focused) {
        SDL_Rect inner = {seq->x + 1, seq->y + 1, seq->w - 2, seq->h - 2};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
        SDL_RenderDrawRect(renderer, &inner);
    }

    if (!seq->font) return;

    // Padding inside the input box
    int pad_x = 10;
    int pad_y = (seq->h - seq->font_size) / 2;

    // ── Placeholder or typed text ─────────────────────────────────────────────
    int is_empty = (strlen(seq->input_buffer) == 0);
    const char* display_text = is_empty ? seq->placeholder : seq->input_buffer;
    SDL_Color text_color;

    if (is_empty && !seq->is_focused) {
        // Placeholder: dim gray
        text_color = (SDL_Color){160, 160, 160, 140};
    } else if (is_empty && seq->is_focused) {
        // Placeholder while focused: slightly brighter
        text_color = (SDL_Color){200, 200, 200, 160};
    } else {
        // Typed text: use sequence text_color
        text_color = (SDL_Color){seq->text_color.r, seq->text_color.g,
                                  seq->text_color.b, seq->text_color.a};
    }

    SDL_Surface* txt_surf = TTF_RenderUTF8_Blended(seq->font, display_text, text_color);
    if (txt_surf) {
        SDL_Texture* txt_tex = SDL_CreateTextureFromSurface(renderer, txt_surf);
        if (txt_tex) {
            // Clip text to the input area
            int max_w = seq->w - pad_x * 2;
            int draw_w = txt_surf->w > max_w ? max_w : txt_surf->w;
            SDL_Rect src  = {txt_surf->w - draw_w, 0, draw_w, txt_surf->h};
            SDL_Rect dest = {seq->x + pad_x,
                             seq->y + pad_y,
                             draw_w,
                             txt_surf->h};
            SDL_RenderCopy(renderer, txt_tex, &src, &dest);
            SDL_DestroyTexture(txt_tex);
        }
        SDL_FreeSurface(txt_surf);
    }

    // ── Blinking cursor ───────────────────────────────────────────────────────
    if (seq->is_focused && seq->cursor_visible && !is_empty) {
        // Measure text up to cursor position
        char before_cursor[256];
        strncpy(before_cursor, seq->input_buffer, seq->cursor_pos);
        before_cursor[seq->cursor_pos] = '\0';

        int cursor_x = seq->x + pad_x;
        if (strlen(before_cursor) > 0) {
            int tw, th;
            TTF_SizeUTF8(seq->font, before_cursor, &tw, &th);
            cursor_x += tw;
        }

        int cursor_y1 = seq->y + pad_y;
        int cursor_y2 = seq->y + seq->h - pad_y;

        // Keep cursor inside the box
        if (cursor_x <= seq->x + seq->w - pad_x) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
            SDL_RenderDrawLine(renderer, cursor_x, cursor_y1, cursor_x, cursor_y2);
            SDL_RenderDrawLine(renderer, cursor_x + 1, cursor_y1, cursor_x + 1, cursor_y2);
        }
    } else if (seq->is_focused && seq->cursor_visible && is_empty) {
        // Cursor when field is empty
        int cursor_x = seq->x + pad_x;
        int cursor_y1 = seq->y + pad_y;
        int cursor_y2 = seq->y + seq->h - pad_y;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
        SDL_RenderDrawLine(renderer, cursor_x, cursor_y1, cursor_x, cursor_y2);
        SDL_RenderDrawLine(renderer, cursor_x + 1, cursor_y1, cursor_x + 1, cursor_y2);
    }
}
