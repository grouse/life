#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"

using i32 = int;
using u32 = unsigned int;
using f32 = float;
using f64 = double;

#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define MIN(a, b) ((a) < (b)) ? (a) : (b)


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    i32 resolution_x = 1920;
    i32 resolution_y = 1080;
    constexpr i32 history_size = 234;

    InitWindow(resolution_x, resolution_y, "life");
    SetTargetFPS(60);
    
    constexpr i32 width = 256;
    constexpr i32 height = 256;
   
    f32 zoom = 1.0f;
    f32 square_size = 15.0f;
    f32 border = 1.0f;
    
    i32 *cells = (i32*)malloc(width*height*history_size*sizeof *cells);
    memset(cells, 0, width*height*history_size*sizeof *cells);
    
    bool *simulated = (bool*)malloc(history_size * sizeof *simulated);
    memset(simulated, 0, history_size * sizeof *simulated);
    
    i32 tail = 0;
    i32 head = 0;
    i32 target = 0;
    
    Color alive_col = ColorFromNormalized(Vector4{ 1.0f, 1.0f, 0.0f, 1.0f });
    Color dead_col  = ColorFromNormalized(Vector4{ 0.1f, 0.1f, 0.1f, 1.0f });
    
    Color alive_hl = ColorFromNormalized(Vector4{ 0.0f, 1.0f, 0.0f, 0.2f });
    Color dead_hl = ColorFromNormalized(Vector4{ 0.0f, 1.0f, 0.0f, 0.2f });
    
    Color scrub_bg = ColorFromNormalized(Vector4{ 0.2f, 0.2f, 0.2f, 1.0f });
    Color frame_bg{ 0xfc, 0x77, 0x02, 0xff };
    Color cur_frame_bg = ColorFromNormalized(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
    i32 frame_size = (f32)resolution_x/(f32)history_size - 1.0f;
    i32 frame_start_x = (resolution_x - (frame_size * history_size) - history_size) / 2.0f;
    
    
    
    Vector2 offset{ 
        resolution_x/2.0f - width*(square_size+border)/2.0f, 
        resolution_y/2.0f - height*(square_size+border)/2.0f 
    };
    Vector2 drag_start{ 0.0f, 0.0f };
    
    bool simulate = false;
    f32 dt_accum = 0.0f;
    f32 simulate_period = 0.5f;
    
    bool scrubbing_frame = false;
    
    char buffer[256];
    (void)buffer;
    
    Vector2 prev_mouse_pos = GetMousePosition();
    
    while (!WindowShouldClose()) {
        dt_accum += GetFrameTime();

        Vector2 mouse_pos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
            drag_start = mouse_pos;
        }

        if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            offset.x += mouse_pos.x - drag_start.x;
            offset.y += mouse_pos.y - drag_start.y;
            drag_start = mouse_pos;
        }
        
        if (IsKeyPressed(KEY_R)) {
            head = 0;
            tail = 0;
            target = 0;
            memset(simulated, 0, history_size*sizeof *simulated);
            memset(cells, 0, width*height*sizeof *cells);
        }
        
        i32 wheel = GetMouseWheelMove();
        if (wheel != 0) {
            f32 new_zoom = zoom + wheel/50.0f;
            new_zoom = MIN(MAX(new_zoom, 0.1f), 1.0f);
            
            offset.x = offset.x + ((i32)(square_size*zoom)+border)*width*0.5f - ((i32)(square_size*new_zoom)+border)*width*0.5f;
            offset.y = offset.y + ((i32)(square_size*zoom)+border)*height*0.5f - ((i32)(square_size*new_zoom)+border)*height*0.5f;

            zoom = new_zoom;
        }

        if (!simulate) {
            if (scrubbing_frame || mouse_pos.y >= resolution_y-20) {
                scrubbing_frame = false;
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    i32 frame = (i32)((mouse_pos.x - frame_start_x)/(frame_size+1));
                    if (target != (frame+tail)%history_size && frame >= 0 && frame < history_size) {
                        target = (frame+tail)%history_size;
                    }
                    scrubbing_frame = true;
                } 
            } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) ||
                       IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
            {
                i32 x = (mouse_pos.x - offset.x)/((i32)(square_size*zoom)+border);
                i32 y = (mouse_pos.y - offset.y)/((i32)(square_size*zoom)+border);

                if (x > 0 && x < width && y > 0 && y < height) {
                    i32 prev_x = (prev_mouse_pos.x - offset.x)/((i32)(square_size*zoom)+border);
                    i32 prev_y = (prev_mouse_pos.y - offset.y)/((i32)(square_size*zoom)+border);

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ||
                        IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) ||
                        x != prev_x || y != prev_y) 
                    {
                        cells[head*width*height + y*width+x] = !IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
                        memset(simulated, 0, history_size*sizeof *simulated);
                        simulated[head] = true;
                        tail = head;
                        target = head;
                    }
                }
            }

            if (IsKeyReleased(KEY_SPACE)) {
                simulate = true;
                scrubbing_frame = false;
                dt_accum = 0.0f;
            }
        } else {
            if (IsKeyReleased(KEY_SPACE)) {
                simulate = false;
            }
        }
        
        if (simulate && dt_accum >= simulate_period) {
            dt_accum -= simulate_period;
            target = (head+1) % history_size;
            
            if (target == tail) simulated[target] = false;
        }

        
        if (target != head && simulated[target]) {
            head = target;
        }
        
        while (target != head) {
            head = (head+1) % history_size;
            if (head == tail) {
                simulated[head] = false;
                tail = (tail+1) % history_size;
            }

            if (!simulated[head]) {
                i32 *curr = &cells[(head > 0 ? head-1 : history_size-1)*width*height];
                i32 *next = &cells[head*width*height];
                
                for (i32 i = 0; i < height; i++) {
                    for (i32 j = 0; j < width; j++) {
                        i32 neighbors = 0;

                        neighbors += i > 0 ? curr[(i-1)*width+j] : 0; // 0, -1
                        neighbors += i < height-1 ? curr[(i+1)*width+j] : 0; // 0, +1
                        neighbors += j > 0 ? curr[i*width+j-1] : 0; // -1, 0
                        neighbors += j < width-1 ? curr[i*width+j+1] : 0; // +1, 0
                        neighbors += i > 0 && j > 0 ? curr[(i-1)*width+j-1] : 0; // -1, -1
                        neighbors += i > 0 && j < width-1 ? curr[(i-1)*width+j+1] : 0; // +1, -1
                        neighbors += i < height-1 && j > 0 ? curr[(i+1)*width+j-1] : 0; // -1, +1
                        neighbors += i < height-1 && j < width-1 ? curr[(i+1)*width+j+1] : 0; // +1, +1

                        if (curr[i*width+j] && (neighbors == 2 || neighbors == 3)) {
                            next[i*width+j] = 1; // continue living
                        } else if (!curr[i*width+j] && neighbors == 3) {
                            next[i*width+j] = 1; // reproduction
                        } else if (curr[i*width+j]) {
                            next[i*width+j] = 0; // die
                        } else {
                            next[i*width+j] = curr[i*width+j]; // continue living
                        }
                    }
                }

                simulated[head] = true;
            }
        }
        
        
        BeginDrawing(); {
            ClearBackground(BLACK);
            
            i32 *curr = &cells[head*width*height];
            for (i32 i = 0; i < height; i++) {
                i32 y = ((i32)(square_size*zoom)+border)*i;

                for (i32 j = 0; j < width; j++) {
                    i32 x = ((i32)(square_size*zoom)+border)*j;
                    Color col = curr[i*width+j] ? alive_col : dead_col;
                    DrawRectangle(
                        x + offset.x,
                        y + offset.y,
                        (i32)(square_size*zoom), (i32)(square_size*zoom), col);
                }
            }

            if (!simulate && !scrubbing_frame) {
                i32 x = (mouse_pos.x - offset.x)/((i32)(square_size*zoom)+border);
                i32 y = (mouse_pos.y - offset.y)/((i32)(square_size*zoom)+border);

                if (mouse_pos.y < resolution_y-20 &&
                    x > 0 && x < width && y > 0 && y < height)
                {
                    Color col = curr[y*width+x] ? alive_hl : dead_hl;
                    DrawRectangle(
                        ((i32)(square_size*zoom)+border)*x+offset.x, 
                        ((i32)(square_size*zoom)+border)*y+offset.y, 
                        (i32)(square_size*zoom), (i32)(square_size*zoom), 
                        col);
                }
            }
            

            DrawRectangle(0, resolution_y-20, resolution_x, 20, scrub_bg);
            for (i32 i = 0; i < history_size; i++) {
                DrawRectangle(frame_start_x+(frame_size+1)*i, resolution_y-20, frame_size, 20, frame_bg);
            }
            
            i32 frame = head-tail;
            frame = frame < 0 ? history_size+frame : frame;
            DrawRectangle(frame_start_x+(frame_size+1)*frame+frame_size/2.0f-1.0f, resolution_y-20, 2, 20, cur_frame_bg);


            EndDrawing();
        }
        
        prev_mouse_pos = mouse_pos;
    }
    
    CloseWindow();
    return 0;
}
            