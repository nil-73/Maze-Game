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

#define MAZE_SIZE 32
#define MAZE_DRAW_SIZE 8
#define POINT_DENISTY 4

typedef struct Point{
    int x;
    int y;
} Point;

typedef struct Item{
    Point cell;
    bool picked;
    int points;
    int damage;
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
    
    bool mazeEditMode = false;
    Point mousePoint = {0};
    Point imagePoint = {0};
    Point position = {0};
    
    position.x = GetScreenWidth()/2 - texMaze.width*MAZE_DRAW_SIZE/2;
    position.y = GetScreenHeight()/2 - texMaze.height*MAZE_DRAW_SIZE/2;
    
    Texture2D texBiomes[4] = { 0 };
    texBiomes[0] = LoadTexture("resources/maze_atlas01.png");
    texBiomes[1] = LoadTexture("resources/maze_atlas02.png");
    texBiomes[2] = LoadTexture("resources/maze_atlas03.png");
    texBiomes[3] = LoadTexture("resources/maze_atlas04.png");
    int currentBiome = 0;
    Vector2 mapPosition = {0};
    Vector2 playerCell = {0};

    Rectangle player = {160,160,40,40};
    float playerSpeed = 200.0f;
    Rectangle playerBounds[4] = {0};

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
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
        
        if (IsKeyPressed(KEY_SPACE)) mazeEditMode = !mazeEditMode;
        if (mazeEditMode)
        {
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
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, BLACK);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                } 
            }
        }
        else 
        {
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
            
            if (IsKeyDown(KEY_ONE)) currentBiome = 0;
            if (IsKeyDown(KEY_TWO)) currentBiome = 1;
            if (IsKeyDown(KEY_THREE)) currentBiome = 2;
            if (IsKeyDown(KEY_FOUR)) currentBiome = 3;
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(SKYBLUE);

            DrawTextureEx(texMaze, (Vector2) { position.x, 
                position.y }, 0.0f, MAZE_DRAW_SIZE, WHITE);
                
            if (mazeEditMode){            
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
            else 
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
            
            DrawFPS(screenWidth-80, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texMaze);
    UnloadImage(imMaze);
    
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