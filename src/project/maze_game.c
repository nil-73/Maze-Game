/*******************************************************************************************
*
* raylib maze game
*
* Procedural maze generator using Maze Grid Algorithm
*
* This game has been created using raylib (www.raylib.com)
* raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
* Copyright (c) 2024-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h> // Required for: malloc(), free()
#include <math.h>   // Required for: expf(), logf()

#define MAZE_SIZE 32
#define MAZE_DRAW_SIZE 8
#define POINT_DENSITY 4
#define NUM_BIOMES 4
#define MAX_CATS 20

typedef struct point {
    int x;
    int y;
} point;

typedef struct item {
    point cell;
    bool picked;
    int points;
} item;

// Generate procedural maze image, using grid-based algorithm
// NOTE: Functions defined as static are internal to the module
static Image gen_image_maze(int width, int height, int spacing_rows, int spacing_cols, float point_chance);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screen_width = 800;
    const int screen_height = 450;
    point end = {0};
    
    InitWindow(screen_width, screen_height, "Delivery04 - maze game");
    
    unsigned int seed = 12345678;
    
    SetRandomSeed(seed);
    
    Image im_maze = gen_image_maze(MAZE_SIZE, MAZE_SIZE, POINT_DENSITY, POINT_DENSITY, 0.4f);
    
    do
    {
        end.x = GetRandomValue(1, im_maze.width - 2);
        end.y = GetRandomValue(1, im_maze.height - 2);
    }
    while (!ColorIsEqual(GetImageColor(im_maze, end.x, end.y), BLACK));

    ImageDrawPixel(&im_maze, end.x, end.y, GREEN);
    
    // Load texture from image
    Texture2D tex_maze = LoadTextureFromImage(im_maze);
    
    int scene = 1; // scene 0 is editMode, scene 1 is playMode, scene 2 is ending 
    point mouse_point = {0};
    point image_point = {0};
    point position = {0};
    
    position.x = GetScreenWidth()/2 - tex_maze.width*MAZE_DRAW_SIZE/2;
    position.y = GetScreenHeight()/2 - tex_maze.height*MAZE_DRAW_SIZE/2;
    
    Texture2D tex_biomes[4] = { 0 };
    tex_biomes[0] = LoadTexture("resources/maze_atlas01.png");
    tex_biomes[1] = LoadTexture("resources/maze_atlas02.png");
    tex_biomes[2] = LoadTexture("resources/maze_atlas03.png");
    tex_biomes[3] = LoadTexture("resources/maze_atlas04.png");
    
    Texture2D cat_box = LoadTexture("resources/cat_box.png");
    Texture2D tex_player = LoadTexture("resources/player_idle.png");
    
    Texture2D tex_win = LoadTexture("resources/win.png");
    
    int current_biome = 0;
    Vector2 map_position = {0};
    Vector2 player_cell = {0};

    Rectangle player = {160,160,32,32};
    float player_speed = 200.0f;
    Rectangle player_bounds[4] = {0};

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera.zoom = 1.0f;
    
    int score = 0;
    item cats[MAX_CATS] = {0};
    int total_cats = 0;
    int full_cats = 0;
    
    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("resources/music.wav");
    Sound pick_cat = LoadSound("resources/pick_cat.wav");

    PlayMusicStream(music);

    float time_played = 0.0f;        // Time played normalized [0.0f..1.0f]
    bool pause = false;             // Music playing paused

    float pan = 0.0f;               // Default audio pan center [-1.0f..1.0f]
    SetMusicPan(music, pan);

    float volume = 0.8f;            // Default audio volume [0.0f..1.0f]
    SetMusicVolume(music, volume);
    
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        // Reset maze for new random generation
        if (IsKeyPressed(KEY_R))
        {
            UnloadTexture(tex_maze);
            UnloadImage(im_maze);
            
            seed += 1;
            SetRandomSeed(seed);
            
            im_maze = gen_image_maze(MAZE_SIZE, MAZE_SIZE, POINT_DENSITY, POINT_DENSITY, 0.4f);
            
            do
            {
                end.x = GetRandomValue(1, im_maze.width - 2);
                end.y = GetRandomValue(1, im_maze.height - 2);
            }
            while (!ColorIsEqual(GetImageColor(im_maze, end.x, end.y), BLACK));

            ImageDrawPixel(&im_maze, end.x, end.y, GREEN);
            
            tex_maze = LoadTextureFromImage(im_maze);
        }
        
        UpdateMusicStream(music);   // Update music buffer with new stream data

        // Restart music playing (stop and play)
        // Pause/Resume music playing
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;

            if (pause) PauseMusicStream(music);
            else ResumeMusicStream(music);
        }

        // Set audio pan
        if (IsKeyDown(KEY_SEVEN))
        {
            pan -= 0.05f;
            if (pan < -1.0f) pan = -1.0f;
            SetMusicPan(music, pan);
        }
        else if (IsKeyDown(KEY_EIGHT))
        {
            pan += 0.05f;
            if (pan > 1.0f) pan = 1.0f;
            SetMusicPan(music, pan);
        }

        // Set audio volume
        if (IsKeyDown(KEY_NINE))
        {
            volume -= 0.05f;
            if (volume < 0.0f) volume = 0.0f;
            SetMusicVolume(music, volume);
        }
        else if (IsKeyDown(KEY_ZERO))
        {
            volume += 0.05f;
            if (volume > 1.0f) volume = 1.0f;
            SetMusicVolume(music, volume);
        }

        // Get normalized time played for current music stream
        time_played = GetMusicTimePlayed(music)/GetMusicTimeLength(music);

        if (time_played > 1.0f) time_played = 1.0f;
        
        if (IsKeyPressed(KEY_SPACE)) 
        {
            switch(scene)
            {
                case 0: 
                    scene = 1; 
                break;
                
                case 1: 
                    scene = 0;
                break;
                case 2:
                {
                    scene = 1;
                    score = 0;
                    total_cats = 0;
                    full_cats = 0;

                    UnloadTexture(tex_maze);
                    UnloadImage(im_maze);

                    seed += 1;
                    SetRandomSeed(seed);

                    im_maze = gen_image_maze(MAZE_SIZE, MAZE_SIZE, POINT_DENSITY, POINT_DENSITY, 0.4f);

                    do
                    {
                        end.x = GetRandomValue(1, im_maze.width - 2);
                        end.y = GetRandomValue(1, im_maze.height - 2);
                    }
                    while (!ColorIsEqual(GetImageColor(im_maze, end.x, end.y), BLACK));

                    ImageDrawPixel(&im_maze, end.x, end.y, GREEN);

                    tex_maze = LoadTextureFromImage(im_maze);

                    player.x = 160;
                    player.y = 160;
                    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
                    camera.zoom = 1.0f;
                }
                break;
            }
        }
        if (scene == 0)
        {
            pause = true;
            mouse_point.x = GetMouseX();
            mouse_point.y = GetMouseY();
            
            image_point.x = (mouse_point.x - position.x)/MAZE_DRAW_SIZE;
            image_point.y = (mouse_point.y - position.y)/MAZE_DRAW_SIZE;
            
            if ((image_point.x >= 0) && (image_point.y >= 0) && 
               (image_point.x < im_maze.width) && (image_point.y < im_maze.height))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    ImageDrawPixel(&im_maze, image_point.x, image_point.y, BLACK);
                    
                    UnloadTexture(tex_maze);
                    tex_maze = LoadTextureFromImage(im_maze);
                } 
                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                {
                    if (IsKeyDown(KEY_LEFT_CONTROL))
                    {
                        ImageDrawPixel(&im_maze, end.x, end.y, WHITE);
                        
                        end.x = image_point.x;
                        end.y = image_point.y;
                    
                        ImageDrawPixel(&im_maze, end.x, end.y, GREEN);
                    
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    }
                    else
                    {
                        ImageDrawPixel(&im_maze, image_point.x, image_point.y, WHITE);
                        
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze); 
                    }
                    
                } 
                else if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON))
                {
                    if (total_cats < MAX_CATS)
                    {
                        cats[total_cats].cell.x = image_point.x;
                        cats[total_cats].cell.y = image_point.y;
                        cats[total_cats].picked = false;
                        cats[total_cats].points = 1;

                        ImageDrawPixel(&im_maze, cats[total_cats].cell.x, cats[total_cats].cell.y, RED);
                        total_cats++;
                    }
                    else
                    {
                        ImageDrawPixel(&im_maze, cats[full_cats].cell.x, cats[full_cats].cell.y, BLACK);

                        cats[full_cats].cell.x = image_point.x;
                        cats[full_cats].cell.y = image_point.y;
                        cats[full_cats].picked = false;
                        cats[full_cats].points = 1;

                        ImageDrawPixel(&im_maze, cats[full_cats].cell.x, cats[full_cats].cell.y, RED);

                        full_cats++;
                        if (full_cats >= MAX_CATS) full_cats = 0;
                    }

                    UnloadTexture(tex_maze);
                    tex_maze = LoadTextureFromImage(im_maze);
                }
            }           
        }
        else if (scene == 1)
        {
            pause = false;
            
            Rectangle prev_player = player;
            
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.y -= (player_speed * GetFrameTime());
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.y += (player_speed * GetFrameTime());
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= (player_speed * GetFrameTime());
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += (player_speed * GetFrameTime());
            
            //Get player pos in image map coords
            
            player_cell.x = (int)((player.x + player.width/2 - map_position.x)/128.0f);
            player_cell.y = (int)((player.y + player.height/2 - map_position.y)/128.0f);
            
            player_bounds[0] = (Rectangle){ map_position.x + player_cell.x*128.0f, map_position.y + (player_cell.y -1)*128.0f, 128.0f, 128.0f };
            player_bounds[1] = (Rectangle){ map_position.x + (player_cell.x -1)*128.0f, map_position.y + player_cell.y*128.0f, 128.0f, 128.0f };
            player_bounds[2] = (Rectangle){ map_position.x + player_cell.x*128.0f, map_position.y + (player_cell.y +1)*128.0f, 128.0f, 128.0f };
            player_bounds[3] = (Rectangle){ map_position.x + (player_cell.x +1)*128.0f, map_position.y + player_cell.y*128.0f, 128.0f, 128.0f };
            
            if ((CheckCollisionRecs(player, player_bounds[0]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x, player_cell.y - 1), WHITE)) ||
                (CheckCollisionRecs(player, player_bounds[1]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x - 1, player_cell.y), WHITE)) ||
                (CheckCollisionRecs(player, player_bounds[2]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x, player_cell.y + 1), WHITE)) ||
                (CheckCollisionRecs(player, player_bounds[3]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x + 1, player_cell.y), WHITE)))
            {
                player = prev_player;
            }
            
            for (int i = 0; i < total_cats; i++)
            {
                if (!cats[i].picked)
                {
                    if ((cats[i].cell.x == (int)player_cell.x) && (cats[i].cell.y == (int)player_cell.y))
                    {
                        cats[i].picked = true;
                        score += cats[i].points;
                        PlaySound(pick_cat);

                        ImageDrawPixel(&im_maze, cats[i].cell.x, cats[i].cell.y, BLACK);

                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    }
                }
            }
            
            if ((int)player_cell.x == end.x && (int)player_cell.y == end.y)
            {
                scene = 2;
            }
            
            camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
            camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
            else if (camera.zoom < 0.1f) camera.zoom = 0.1f;
            
            if (IsKeyDown(KEY_ONE)) current_biome = 0;
            if (IsKeyDown(KEY_TWO)) current_biome = 1;
            if (IsKeyDown(KEY_THREE)) current_biome = 2;
            if (IsKeyDown(KEY_FOUR)) current_biome = 3;
        }
        else if (scene == 2)
        {
            
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(SKYBLUE);
                
            if (scene == 0){  
                DrawTextureEx(tex_maze, (Vector2){ position.x, position.y }, 0.0f, MAZE_DRAW_SIZE, WHITE);
                
                if((image_point.x >= 0) && (image_point.y >= 0) && 
                   (image_point.x < im_maze.width) && (image_point.y < im_maze.height))
                {
                    DrawRectangleLines(position.x+image_point.x*MAZE_DRAW_SIZE, 
                    position.y+image_point.y*MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, GREEN);
                }
                
                DrawText("EDIT MODE", 10, 10, 20, BLACK);
                
                DrawText(TextFormat("%i", seed), 20, GetScreenHeight() - 30, 20, DARKBLUE);
                DrawText(TextFormat("MOUSE: [%i, %i]", mouse_point.x, mouse_point.y), 10, 40, 20, DARKBLUE);
                DrawText(TextFormat("IMAGE: [%i, %i]", image_point.x, image_point.y), 10, 70, 20, RED);
            }
            else if (scene == 1)
            {
                
                BeginMode2D(camera);
                
                for (int y = 0; y < im_maze.height; y++)
                {
                    for (int x = 0; x < im_maze.width; x++) 
                    {
                        if (ColorIsEqual(GetImageColor(im_maze, x, y), WHITE))
                        {
                            DrawTextureRec(tex_biomes[current_biome], (Rectangle){ tex_biomes[current_biome].width/2, tex_biomes[current_biome].height/2, tex_biomes[current_biome].width/2, tex_biomes[current_biome].height/2 }, (Vector2) { map_position.x + x* tex_biomes[current_biome].width/2, map_position.y + y* tex_biomes[current_biome].height/2}, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(im_maze, x, y), BLACK))
                        {
                            DrawTextureRec(tex_biomes[current_biome], (Rectangle) { 0, 0, tex_biomes[current_biome].width / 2, tex_biomes[current_biome].height / 2 }, 
                            (Vector2) { map_position.x + x * tex_biomes[current_biome].width / 2, map_position.y + y * tex_biomes[current_biome].height / 2 }, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(im_maze, x, y), GREEN))
                        {
                            DrawTextureRec(tex_biomes[current_biome], (Rectangle) { 0, tex_biomes[current_biome].width/2, tex_biomes[current_biome].width / 2, tex_biomes[current_biome].height / 2 }, 
                            (Vector2) { map_position.x + x * tex_biomes[current_biome].width / 2, map_position.y + y * tex_biomes[current_biome].height / 2 }, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(im_maze, x, y), RED)) 
                        {
                            DrawTextureRec(tex_biomes[current_biome], (Rectangle) { 0, 0, tex_biomes[current_biome].width / 2, tex_biomes[current_biome].height / 2 }, 
                            (Vector2) { map_position.x + x * tex_biomes[current_biome].width / 2, map_position.y + y * tex_biomes[current_biome].height / 2 }, WHITE);
                        }
                    }
                }
                for (int i = 0; i < total_cats; i++)
                {
                    if (!cats[i].picked)
                    {
                        DrawTextureRec(
                            cat_box,
                            (Rectangle){ 0, 0, 32, 32 },
                            (Vector2){
                                map_position.x + cats[i].cell.x * tex_biomes[current_biome].width/2 + 48,
                                map_position.y + cats[i].cell.y * tex_biomes[current_biome].height/2 + 48
                            },
                            WHITE
                        );
                    }
                }
                
                DrawTextureEx(tex_player, (Vector2){ player.x - 32.0f, player.y - 32.0f }, 0.0f, 3.0f, WHITE);
                
                for (int i = 0; i < 4; i++) 
                {
                    DrawRectangleLinesEx(player_bounds[i], 2.0f,GREEN);
                }
                
                EndMode2D();
                DrawText(TextFormat("SCORE: [%i]", score), 20, GetScreenHeight() - 30, 20, WHITE); // score display
                DrawText("GAME MODE", 10, 10, 20, BLACK);
                
                
            }
            else if (scene == 2)
            {
                ClearBackground(WHITE);
                
                float scale = 0.5f;
                float width = tex_win.width * scale;
                float height = tex_win.height * scale;

                DrawTexturePro(
                    tex_win,
                    (Rectangle){ 0, 0, tex_win.width, tex_win.height },
                    (Rectangle){
                        screen_width/2 - width/2,
                        screen_height/2 - height/2,
                        width,
                        height
                    },
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
                );

                DrawText(TextFormat("Score: %i", score), screen_width/2 - 60, screen_height - 80, 20, BLACK);
            }
            
            DrawFPS(screen_width-80, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tex_maze);
    for (int i = 0; i < NUM_BIOMES; i++)
    {
        UnloadTexture(tex_biomes[i]);
    }
    UnloadTexture(tex_player);

    UnloadTexture(cat_box);
    UnloadTexture(tex_win);
    UnloadImage(im_maze);
    
    UnloadSound(pick_cat);
    UnloadMusicStream(music);
    CloseAudioDevice();
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static Image gen_image_maze(int width, int height, int spacing_rows, int spacing_cols, float point_chance)
{
    Image im_maze = GenImageColor(width, height, BLACK);
    
    point *maze_points = malloc(64*sizeof(point));
    
    int maze_point_counter = 0;
    
    for (int y = 0; y < im_maze.height; y++)
    {
        for (int x = 0; x < im_maze.width; x++)
        {
            if ((x == 0) || (y == 0) || (x == (im_maze.width - 1)) || (y == (im_maze.height - 1))) 
            {
                ImageDrawPixel(&im_maze, x, y, WHITE);
            }
            else 
            {
                if ((x%spacing_rows == 0) && (y%spacing_cols == 0)) 
                {
                    if (GetRandomValue(0, 100)<point_chance*100)
                    {
                        ImageDrawPixel(&im_maze, x, y, WHITE);
                        maze_points[maze_point_counter] = (point) { x, y }; 
                        maze_point_counter++;
                    }
                }
            }
        }
    }
    
    point dir_increments[4] = {
        {0,-1},
        {1,0},
        {0,1},
        {-1,0}
    };
    
    int *indices = LoadRandomSequence(maze_point_counter, 0, maze_point_counter - 1);
    
    for (int i = 0; i < maze_point_counter; i++)
    {
        int dir = GetRandomValue(0,3);
        point next_maze_point = maze_points[indices[i]];
        next_maze_point.x += dir_increments[dir].x;
        next_maze_point.y += dir_increments[dir].y;
        
        while (ColorIsEqual(GetImageColor(im_maze, next_maze_point.x, next_maze_point.y), BLACK))
        {    
            ImageDrawPixel(&im_maze, next_maze_point.x, next_maze_point.y, WHITE);
            
            next_maze_point.x += dir_increments[dir].x;
            next_maze_point.y += dir_increments[dir].y;
        }
    }
    free(maze_points);
    UnloadRandomSequence(indices);
    
    return im_maze;
}