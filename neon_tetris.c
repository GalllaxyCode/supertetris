#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define CELL_SIZE 30
#define OFFSET_X 100
#define OFFSET_Y 50
#define MAX_PARTICLES 400

// Neon Color Palette
const Color COLORS[] = {
    BLANK,                           // 0: Empty
    (Color){ 0, 255, 255, 255 },     // 1: Cyan (I)
    (Color){ 0, 121, 241, 255 },     // 2: Blue (J)
    (Color){ 255, 161, 0, 255 },     // 3: Orange (L)
    (Color){ 253, 249, 0, 255 },     // 4: Yellow (O)
    (Color){ 0, 228, 48, 255 },      // 5: Green (S)
    (Color){ 200, 122, 255, 255 },   // 6: Purple (T)
    (Color){ 230, 41, 55, 255 }      // 7: Red (Z)
};

const int SHAPES[7][4][16] = {
    {{0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0}, {0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0}},
    {{0,0,0,0, 2,2,2,0, 0,0,2,0, 0,0,0,0}, {0,2,0,0, 0,2,0,0, 2,2,0,0, 0,0,0,0}, {2,0,0,0, 2,2,2,0, 0,0,0,0, 0,0,0,0}, {0,2,2,0, 0,2,0,0, 0,2,0,0, 0,0,0,0}},
    {{0,0,0,0, 3,3,3,0, 3,0,0,0, 0,0,0,0}, {3,3,0,0, 0,3,0,0, 0,3,0,0, 0,0,0,0}, {0,0,3,0, 3,3,3,0, 0,0,0,0, 0,0,0,0}, {0,3,0,0, 0,3,0,0, 0,3,3,0, 0,0,0,0}},
    {{0,0,0,0, 0,4,4,0, 0,4,4,0, 0,0,0,0}, {0,0,0,0, 0,4,4,0, 0,4,4,0, 0,0,0,0}, {0,0,0,0, 0,4,4,0, 0,4,4,0, 0,0,0,0}, {0,0,0,0, 0,4,4,0, 0,4,4,0, 0,0,0,0}},
    {{0,0,0,0, 0,5,5,0, 5,5,0,0, 0,0,0,0}, {0,5,0,0, 0,5,5,0, 0,0,5,0, 0,0,0,0}, {0,0,0,0, 0,5,5,0, 5,5,0,0, 0,0,0,0}, {0,5,0,0, 0,5,5,0, 0,0,5,0, 0,0,0,0}},
    {{0,0,0,0, 6,6,6,0, 0,6,0,0, 0,0,0,0}, {0,6,0,0, 6,6,0,0, 0,6,0,0, 0,0,0,0}, {0,6,0,0, 6,6,6,0, 0,0,0,0, 0,0,0,0}, {0,6,0,0, 0,6,6,0, 0,6,0,0, 0,0,0,0}},
    {{0,0,0,0, 7,7,0,0, 0,7,7,0, 0,0,0,0}, {0,0,7,0, 0,7,7,0, 0,7,0,0, 0,0,0,0}, {0,0,0,0, 7,7,0,0, 0,7,7,0, 0,0,0,0}, {0,0,7,0, 0,7,7,0, 0,7,0,0, 0,0,0,0}}
};

int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};
int score = 0;
int game_over = 0;
int current_piece, next_piece, current_rot;
int current_x, current_y;

// Animation States
int clearing_lines_frames = 0;
int lines_to_clear[4] = {-1, -1, -1, -1};
int lines_count = 0;

// Particle Systems
typedef struct {
    float x, y, vx, vy;
    Color color;
    float alpha, rotation, rot_speed;
    bool active;
} BlockParticle;

typedef struct {
    float x, y;
    float float_speed, alpha;
    bool active;
} TextParticle;

BlockParticle debris[MAX_PARTICLES] = {0};
TextParticle floating_texts[MAX_PARTICLES] = {0};

// Spawns physical falling block debris and floating text
void SpawnExplosion(int x, int y, Color color) {
    // Block Particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!debris[i].active) {
            debris[i] = (BlockParticle){
                (float)x, (float)y,
                (float)GetRandomValue(-300, 300) / 100.0f, // Random X velocity
                (float)GetRandomValue(-800, -300) / 100.0f, // Random Y jump
                color, 1.0f,
                (float)GetRandomValue(0, 360),
                (float)GetRandomValue(-15, 15), // Rotation speed
                true
            };
            break;
        }
    }
    // "+36" Text Particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!floating_texts[i].active) {
            floating_texts[i] = (TextParticle){
                (float)x + GetRandomValue(-10, 10),
                (float)y + GetRandomValue(-10, 10),
                (float)GetRandomValue(50, 150) / 100.0f, // Upward float speed
                1.5f, // Starts > 1.0 so it stays fully opaque for a moment before fading
                true
            };
            break;
        }
    }
}

// Cleaned up block drawing (No outward glow)
void DrawBlock(int x, int y, Color color, float alpha) {
    Color baseColor = Fade(color, alpha);
    Color highlight = Fade(WHITE, alpha * 0.4f);

    // Solid Core
    DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, baseColor);
    // Inner glass highlight
    DrawRectangle(x + 3, y + 3, CELL_SIZE - 6, CELL_SIZE - 6, highlight);
    // Sharp outline
    DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, Fade(BLACK, alpha * 0.8f));
}

int CheckCollision(int piece, int rot, int cx, int cy) {
    for (int i = 0; i < 16; i++) {
        if (SHAPES[piece][rot][i]) {
            int px = cx + (i % 4);
            int py = cy + (i / 4);
            if (px < 0 || px >= BOARD_WIDTH || py >= BOARD_HEIGHT) return 1;
            if (py >= 0 && board[py][px]) return 1;
        }
    }
    return 0;
}

void SpawnPiece() {
    current_piece = next_piece;
    next_piece = GetRandomValue(0, 6);
    current_rot = 0;
    current_x = BOARD_WIDTH / 2 - 2;
    current_y = 0;
    if (CheckCollision(current_piece, current_rot, current_x, current_y)) game_over = 1;
}

void LockPiece() {
    for (int i = 0; i < 16; i++) {
        if (SHAPES[current_piece][current_rot][i]) {
            int px = current_x + (i % 4);
            int py = current_y + (i / 4);
            if (py >= 0) board[py][px] = SHAPES[current_piece][current_rot][i];
        }
    }
}

void CheckLines() {
    lines_count = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        int full = 1;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (!board[y][x]) { full = 0; break; }
        }
        if (full) lines_to_clear[lines_count++] = y;
    }
    
    if (lines_count > 0) {
        clearing_lines_frames = 20; // Brief pause for impact
        
        // Explode the blocks immediately
        for (int i = 0; i < lines_count; i++) {
            int y = lines_to_clear[i];
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x]) {
                    SpawnExplosion(OFFSET_X + x * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, COLORS[board[y][x]]);
                    board[y][x] = 0; // Erase from board so particles take over visually
                    score += 36;     // Exact score requested!
                }
            }
        }
    } else {
        SpawnPiece();
    }
}

void RemoveLines() {
    for (int i = 0; i < lines_count; i++) {
        int clear_y = lines_to_clear[i] + i; 
        for (int y = clear_y; y > 0; y--) {
            for (int x = 0; x < BOARD_WIDTH; x++) board[y][x] = board[y - 1][x];
        }
        for (int x = 0; x < BOARD_WIDTH; x++) board[0][x] = 0;
    }
    SpawnPiece();
}

int GetGhostY() {
    int ghost_y = current_y;
    while (!CheckCollision(current_piece, current_rot, current_x, ghost_y + 1)) {
        ghost_y++;
    }
    return ghost_y;
}

int main(void) {
    InitWindow(600, 700, "Neon Tetris");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));

    next_piece = GetRandomValue(0, 6);
    SpawnPiece();

    float fall_timer = 0.0f;
    float fall_speed = 0.5f;

    while (!WindowShouldClose()) {
        // --- LOGIC UPDATE ---
        
        // Update Particles (They keep moving even if game is paused for line clears!)
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (debris[i].active) {
                debris[i].vy += 0.4f; // Gravity
                debris[i].x += debris[i].vx;
                debris[i].y += debris[i].vy;
                debris[i].rotation += debris[i].rot_speed;
                debris[i].alpha -= 0.01f;
                if (debris[i].alpha <= 0 || debris[i].y > 800) debris[i].active = false;
            }
            if (floating_texts[i].active) {
                floating_texts[i].y -= floating_texts[i].float_speed;
                floating_texts[i].alpha -= 0.015f;
                if (floating_texts[i].alpha <= 0) floating_texts[i].active = false;
            }
        }

        if (!game_over) {
            if (clearing_lines_frames > 0) {
                clearing_lines_frames--;
                if (clearing_lines_frames == 0) RemoveLines();
            } else {
                if (IsKeyPressed(KEY_LEFT) && !CheckCollision(current_piece, current_rot, current_x - 1, current_y)) current_x--;
                if (IsKeyPressed(KEY_RIGHT) && !CheckCollision(current_piece, current_rot, current_x + 1, current_y)) current_x++;
                if (IsKeyPressed(KEY_UP)) {
                    int next_rot = (current_rot + 1) % 4;
                    if (!CheckCollision(current_piece, next_rot, current_x, current_y)) current_rot = next_rot;
                }
                
                if (IsKeyPressed(KEY_SPACE)) {
                    current_y = GetGhostY();
                    LockPiece();
                    CheckLines();
                    fall_timer = 0;
                }

                float current_speed = IsKeyDown(KEY_DOWN) ? 0.05f : fall_speed;
                fall_timer += GetFrameTime();
                if (fall_timer >= current_speed) {
                    fall_timer = 0.0f;
                    if (!CheckCollision(current_piece, current_rot, current_x, current_y + 1)) {
                        current_y++;
                    } else {
                        LockPiece();
                        CheckLines();
                    }
                }
            }
        }

        // --- DRAWING ---
        BeginDrawing();
        ClearBackground((Color){ 15, 15, 20, 255 }); 

        // Draw Background Grid
        for (int y = 0; y <= BOARD_HEIGHT; y++) 
            DrawLine(OFFSET_X, OFFSET_Y + y * CELL_SIZE, OFFSET_X + BOARD_WIDTH * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, Fade(DARKGRAY, 0.3f));
        for (int x = 0; x <= BOARD_WIDTH; x++) 
            DrawLine(OFFSET_X + x * CELL_SIZE, OFFSET_Y, OFFSET_X + x * CELL_SIZE, OFFSET_Y + BOARD_HEIGHT * CELL_SIZE, Fade(DARKGRAY, 0.3f));

        // Draw Outer Frame Glow (Multiple transparent layers stacking up)
        BeginBlendMode(BLEND_ADDITIVE);
        for (int i = 1; i <= 10; i++) {
            DrawRectangleLinesEx(
                (Rectangle){OFFSET_X - i, OFFSET_Y - i, BOARD_WIDTH * CELL_SIZE + i * 2, BOARD_HEIGHT * CELL_SIZE + i * 2}, 
                2, Fade(SKYBLUE, 0.05f)
            );
        }
        EndBlendMode();
        // Inner sharp frame
        DrawRectangleLinesEx((Rectangle){OFFSET_X - 2, OFFSET_Y - 2, BOARD_WIDTH * CELL_SIZE + 4, BOARD_HEIGHT * CELL_SIZE + 4}, 2, SKYBLUE);

        // Draw locked blocks
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x]) {
                    DrawBlock(OFFSET_X + x * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, COLORS[board[y][x]], 1.0f);
                }
            }
        }

        if (clearing_lines_frames == 0 && !game_over) {
            // Draw Ghost Piece
            int ghost_y = GetGhostY();
            for (int i = 0; i < 16; i++) {
                if (SHAPES[current_piece][current_rot][i]) {
                    int px = current_x + (i % 4);
                    int py = ghost_y + (i / 4);
                    DrawBlock(OFFSET_X + px * CELL_SIZE, OFFSET_Y + py * CELL_SIZE, COLORS[SHAPES[current_piece][current_rot][i]], 0.3f);
                }
            }

            // Draw active piece
            for (int i = 0; i < 16; i++) {
                if (SHAPES[current_piece][current_rot][i]) {
                    int px = current_x + (i % 4);
                    int py = current_y + (i / 4);
                    DrawBlock(OFFSET_X + px * CELL_SIZE, OFFSET_Y + py * CELL_SIZE, COLORS[SHAPES[current_piece][current_rot][i]], 1.0f);
                }
            }
        }

        // Draw Debris Particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (debris[i].active) {
                float a = debris[i].alpha > 1.0f ? 1.0f : debris[i].alpha; // Clamp alpha
                Rectangle rect = { debris[i].x + CELL_SIZE/2, debris[i].y + CELL_SIZE/2, CELL_SIZE, CELL_SIZE };
                Vector2 origin = { CELL_SIZE/2, CELL_SIZE/2 };
                
                // Draw rotating solid block
                DrawRectanglePro(rect, origin, debris[i].rotation, Fade(debris[i].color, a));
                
                // Draw rotating inner highlight
                Rectangle inner_rect = { debris[i].x + CELL_SIZE/2, debris[i].y + CELL_SIZE/2, CELL_SIZE - 6, CELL_SIZE - 6 };
                Vector2 inner_origin = { (CELL_SIZE - 6)/2, (CELL_SIZE - 6)/2 };
                DrawRectanglePro(inner_rect, inner_origin, debris[i].rotation, Fade(WHITE, a * 0.4f));
            }
        }

        // Draw Floating Text (+36)
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (floating_texts[i].active) {
                float a = floating_texts[i].alpha > 1.0f ? 1.0f : floating_texts[i].alpha;
                DrawText("+36", (int)floating_texts[i].x, (int)floating_texts[i].y, 18, Fade(LIME, a));
            }
        }

        // --- UI SIDEBAR ---
        DrawText("NEON TETRIS", 430, 50, 20, WHITE);
        DrawText(TextFormat("SCORE: %06d", score), 430, 80, 20, GREEN);
        DrawText("NEXT PIECE:", 430, 130, 20, LIGHTGRAY);
        
        for (int i = 0; i < 16; i++) {
            if (SHAPES[next_piece][0][i]) {
                int px = (i % 4);
                int py = (i / 4);
                DrawBlock(430 + px * CELL_SIZE, 170 + py * CELL_SIZE, COLORS[SHAPES[next_piece][0][i]], 1.0f);
            }
        }

        DrawText("CONTROLS:", 430, 350, 20, LIGHTGRAY);
        DrawText("Arrows : Move/Rot", 430, 380, 15, DARKGRAY);
        DrawText("Down   : Soft Drop", 430, 400, 15, DARKGRAY);
        DrawText("Space  : Hard Drop", 430, 420, 15, DARKGRAY);

        if (game_over) {
            DrawRectangle(0, 0, 600, 700, Fade(BLACK, 0.8f));
            DrawText("GAME OVER", 200, 300, 40, RED);
            DrawText(TextFormat("FINAL SCORE: %d", score), 220, 350, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
