/*******************************************************************************************
*
*   raylib maze generator
*
*   Procedural maze generator using Maze Grid Algorithm
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h> // Required for: malloc(), free()
#include <math.h> // Required for: expf(), logf()

#define MAZE_SIZE 32
#define MAZE_DRAW_SIZE 8
#define POINT_DENISTY 4
#define NUM_BIOMES 4
#define PLAYER_STATUS 2
#define MAX_CATS 20

typedef struct Point{
    int x;
    int y;
} Point;

typedef struct Item{
    Point cell;
    bool picked;
    int points;
} Item;

static Image GetImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib maze generator");
    
    unsigned int seed = 12345678;
    
    SetRandomSeed(seed);
    
    Image imMaze = GetImageMaze(MAZE_SIZE, MAZE_SIZE, POINT_DENISTY, POINT_DENISTY, 0.4f);
    
    // Load texture from image
    Texture2D texMaze = LoadTextureFromImage(imMaze);
    
    int scene = 1; // scene 0 is editMode, scene 1 is playMode, scene 2 is ending 
    Point mousePoint = {0};
    Point imagePoint = {0};
    Point position = {0};
    Point end = {0};
    
    position.x = GetScreenWidth()/2 - texMaze.width*MAZE_DRAW_SIZE/2;
    position.y = GetScreenHeight()/2 - texMaze.height*MAZE_DRAW_SIZE/2;
    
    Texture2D texBiomes[4] = { 0 };
    texBiomes[0] = LoadTexture("resources/maze_atlas01.png");
    texBiomes[1] = LoadTexture("resources/maze_atlas02.png");
    texBiomes[2] = LoadTexture("resources/maze_atlas03.png");
    texBiomes[3] = LoadTexture("resources/maze_atlas04.png");
    
    Texture2D catBox = LoadTexture("resources/cat_box.png");
    Texture2D texPlayer[2] = { 0 };
    texPlayer[0] = LoadTexture("resources/player_idle.png");
    texPlayer[1] = LoadTexture("resources/player_run.png");
    
    Texture2D texWin = LoadTexture("resources/win.png");
    
    int currentBiome = 0;
    Vector2 mapPosition = {0};
    Vector2 playerCell = {0};

    Rectangle player = {160,160,40,40};
    float playerSpeed = 200.0f;
    Rectangle playerBounds[4] = {0};

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera.zoom = 1.0f;
    
    int score = 0;
    Point catsPosition[MAX_CATS] = {0};
    int totalCats = 0;
    int fullCats = 0;
    
    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("resources/music.wav");
    Sound pickCat = LoadSound("resources/pick_cat.wav");

    PlayMusicStream(music);

    float timePlayed = 0.0f;        // Time played normalized [0.0f..1.0f]
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
            // TODO: Set a new seed and re-generate maze
            
            UnloadTexture(texMaze);
            UnloadImage(imMaze);
            
            seed += 1;
            SetRandomSeed(seed);
            
            Image imMaze = GetImageMaze(MAZE_SIZE, MAZE_SIZE, POINT_DENISTY, POINT_DENISTY, 0.4f);
            
            texMaze = LoadTextureFromImage(imMaze);
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
        timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music);

        if (timePlayed > 1.0f) timePlayed = 1.0f;
        
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
            }
        }
        if (scene == 0)
        {
            pause = true;
            mousePoint.x = GetMouseX();
            mousePoint.y = GetMouseY();
            
            imagePoint.x = (mousePoint.x - position.x)/MAZE_DRAW_SIZE;
            imagePoint.y = (mousePoint.y - position.y)/MAZE_DRAW_SIZE;
            
            if ((imagePoint.x >= 0) && (imagePoint.y >= 0) && 
               (imagePoint.x < imMaze.width) && (imagePoint.y < imMaze.height))
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, WHITE);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                } 
                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                {
                    if (IsKeyDown(KEY_LEFT_CONTROL))
                    {
                        ImageDrawPixel(&imMaze, end.x, end.y, WHITE);
                        
                        end.x = imagePoint.x;
                        end.y = imagePoint.y;
                    
                        ImageDrawPixel(&imMaze, end.x, end.y, GREEN);
                    
                        UnloadTexture(texMaze);
                        texMaze = LoadTextureFromImage(imMaze);
                    }
                    else
                    {
                        ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, BLACK);
                        
                        UnloadTexture(texMaze);
                        texMaze = LoadTextureFromImage(imMaze); 
                    }
                    
                } 
                else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
                {
                    if(totalCats < MAX_CATS)
                    {
                        catsPosition[totalCats].x = imagePoint.x;
                        catsPosition[totalCats].y = imagePoint.y;
                        
                        ImageDrawPixel(&imMaze, catsPosition[totalCats].x, catsPosition[totalCats].y, RED);
                        totalCats++;
                    }
                    else
                    {
                        ImageDrawPixel(&imMaze, catsPosition[fullCats].x, catsPosition[fullCats].y, BLACK);
                        totalCats--;
                        fullCats++;
                        if(fullCats > MAX_CATS) fullCats = 0;
                    }
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                }
            }           
        }
        else if (scene == 1)
        {
            pause = false;
            
            Rectangle prevPlayer = player;
            
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.y -= (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.y += (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += (playerSpeed * GetFrameTime());
            
            //Get player pos in image map coords
            
            playerCell.x = (int)((player.x + player.width/2 - mapPosition.x)/128.0f);
            playerCell.y = (int)((player.y + player.height/2 - mapPosition.y)/128.0f);
            
            playerBounds[0] = (Rectangle){ mapPosition.x + playerCell.x*128.0f, mapPosition.y + (playerCell.y -1)*128.0f, 128.0f, 128.0f };
            playerBounds[1] = (Rectangle){ mapPosition.x + (playerCell.x -1)*128.0f, mapPosition.y + playerCell.y*128.0f, 128.0f, 128.0f };
            playerBounds[2] = (Rectangle){ mapPosition.x + playerCell.x*128.0f, mapPosition.y + (playerCell.y +1)*128.0f, 128.0f, 128.0f };
            playerBounds[3] = (Rectangle){ mapPosition.x + (playerCell.x +1)*128.0f, mapPosition.y + playerCell.y*128.0f, 128.0f, 128.0f };
            
            if ((CheckCollisionRecs(player, playerBounds[0]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x, playerCell.y - 1), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[1]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x - 1, playerCell.y), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[2]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x, playerCell.y + 1), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[3]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x + 1, playerCell.y), WHITE)))
            {
                player = prevPlayer;
            }
            
            camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
            camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
            else if (camera.zoom < 0.1f) camera.zoom = 0.1f;
            
            if (IsKeyDown(KEY_ONE)) currentBiome = 0;
            if (IsKeyDown(KEY_TWO)) currentBiome = 1;
            if (IsKeyDown(KEY_THREE)) currentBiome = 2;
            if (IsKeyDown(KEY_FOUR)) currentBiome = 3;
        }
        else if (scene == 2)
        {
            // victori is yes
            // win win yay
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(SKYBLUE);

            DrawTextureEx(texMaze, (Vector2) { position.x, 
                position.y }, 0.0f, MAZE_DRAW_SIZE, WHITE);
                
            if (scene == 0){            
                if((imagePoint.x >= 0) && (imagePoint.y >= 0) && 
                   (imagePoint.x < imMaze.width) && (imagePoint.y < imMaze.height))
                {
                    DrawRectangleLines(position.x+imagePoint.x*MAZE_DRAW_SIZE, 
                    position.y+imagePoint.y*MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, GREEN);
                }
                
                DrawText("EDIT MODE", 10, 10, 20, BLACK);
                
                DrawText(TextFormat("%i", seed), 20, GetScreenHeight() - 30, 20, DARKBLUE);
                DrawText(TextFormat("MOUSE: [%i, %i]", mousePoint.x, mousePoint.y), 10, 40, 20, DARKBLUE);
                DrawText(TextFormat("IMAGE: [%i, %i]", imagePoint.x, imagePoint.y), 10, 70, 20, RED);
            }
            else if (scene == 1)
            {
                
                BeginMode2D(camera);
                
                for (int y = 0; y < imMaze.height; y++)
                {
                    for (int x = 0; x < imMaze.width; x++) 
                    {
                        if (ColorIsEqual(GetImageColor(imMaze, x, y), WHITE))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle){ texBiomes[currentBiome].width/2, texBiomes[currentBiome].height/2, texBiomes[currentBiome].width/2, texBiomes[currentBiome].height/2 }, (Vector2) { mapPosition.x + x* texBiomes[currentBiome].width/2, mapPosition.y + y* texBiomes[currentBiome].height/2}, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(imMaze, x, y), BLACK))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle) { 0, 0, texBiomes[currentBiome].width / 2, texBiomes[currentBiome].height / 2 }, 
                            (Vector2) { mapPosition.x + x * texBiomes[currentBiome].width / 2, mapPosition.y + y * texBiomes[currentBiome].height / 2 }, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(imMaze, x, y), GREEN))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle) { 0, texBiomes[currentBiome].width/2, texBiomes[currentBiome].width / 2, texBiomes[currentBiome].height / 2 }, 
                            (Vector2) { mapPosition.x + x * texBiomes[currentBiome].width / 2, mapPosition.y + y * texBiomes[currentBiome].height / 2 }, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(imMaze, x, y), RED))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle) { 0, 0, texBiomes[currentBiome].width / 2, texBiomes[currentBiome].height / 2 }, 
                            (Vector2) { mapPosition.x + x * texBiomes[currentBiome].width / 2, mapPosition.y + y * texBiomes[currentBiome].height / 2 }, WHITE);
                            
                            DrawTextureRec(catBox, (Rectangle) {0, 0, 32, 32}, (Vector2) { mapPosition.x + x* texBiomes[currentBiome].width/2 + 48, mapPosition.y + y* texBiomes[currentBiome].height/2 + 48}, WHITE);
                        }
                    }
                }
                
                DrawRectangleRec(player, RED);
                
                for (int i = 0; i < 4; i++) 
                {
                    DrawRectangleLinesEx(playerBounds[i], 2.0f,GREEN);
                }
                
                EndMode2D();
                
                DrawText("GAME MODE", 10, 10, 20, BLACK);
                
                
            }
            else if (scene == 2)
            {
                // yes yes ganar
            }
            
            DrawFPS(screenWidth-80, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texMaze);
    for (int i = 0; i < NUM_BIOMES; i++)
    {
        UnloadTexture(texBiomes[i]);
    }
    for (int i = 0; i < PLAYER_STATUS; i++)
    {
        UnloadTexture(texPlayer[i]);
    }
    UnloadTexture(catBox);
    UnloadTexture(texWin);
    UnloadImage(imMaze);
    
    UnloadSound(pickCat);
    UnloadMusicStream(music);
    CloseAudioDevice();
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
static Image GetImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance)
{
    Image imMaze = GenImageColor(width, height, BLACK);
    
    // Point mazePoints[64] = { 0 }; // Static memory alloc
    
    Point *mazePoints = malloc(64*sizeof(Point));
    
    int mazePointCounter = 0;
    
    for (int y = 0; y < imMaze.height; y++)
    {
        for (int x = 0; x < imMaze.width; x++)
        {
            if ((x == 0) || (y == 0) || (x == (imMaze.width - 1)) || (y == (imMaze.height - 1))) 
            {
                ImageDrawPixel(&imMaze, x, y, WHITE);
            }
            else 
            {
                if ((x%spacingRows == 0) && (y%spacingCols == 0)) 
                {
                    if (GetRandomValue(0, 100)<pointChance*100)
                    {
                        ImageDrawPixel(&imMaze, x, y, WHITE);
                        mazePoints[mazePointCounter] = (Point) { x, y }; // Inicializa los valores: x = x , y = y
                        mazePointCounter++;
                    }
                }
            }
        }
    }
    
    Point dirIncrements[4] = {
        {0,-1},
        {1,0},
        {0,1},
        {-1,0}
    };
    
    int *indices = LoadRandomSequence(mazePointCounter, 0, mazePointCounter - 1);
    
    for (int i = 0; i < mazePointCounter; i++)
    {
        int dir = GetRandomValue(0,3);
        Point nextMazePoint = mazePoints[indices[i]];
        nextMazePoint.x += dirIncrements[dir].x;
        nextMazePoint.y += dirIncrements[dir].y;
        
        while (ColorIsEqual(GetImageColor(imMaze, nextMazePoint.x, nextMazePoint.y), BLACK))
        {    
            ImageDrawPixel(&imMaze, nextMazePoint.x, nextMazePoint.y, WHITE);
            
            nextMazePoint.x += dirIncrements[dir].x;
            nextMazePoint.y += dirIncrements[dir].y;
        }
    }
    return imMaze;
}