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
            
            Image maze = GetImageMaze(MAZE_SIZE, MAZE_SIZE, POINT_DENISTY, POINT_DENISTY, 0.4f);
            
            texMaze = LoadTextureFromImage(imMaze);
        }
        if (IsKeyPressed(KEY_SPACE)) mazeEditMode = !mazeEditMode;
        if (mazeEditMode)
        {
            mousePoint.x = GetMouseX();
            mousePoint.y = GetMouseY();
            
            imagePoint.x = (mousePoint.x - position.x)/MAZE_DRAW_SIZE;
            imagePoint.y = (mousePoint.y - position.y)/MAZE_DRAW_SIZE;
            
            if (mazeEditMode && (imagePoint.x >= 0) && (imagePoint.y >= 0) && 
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
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(SKYBLUE);

            DrawTextureEx(texMaze, (Vector2) { position.x, 
                position.y }, 0.0f, MAZE_DRAW_SIZE, WHITE);
                
            DrawText(TextFormat("%i", seed), 20, GetScreenHeight() - 30, 20, DARKBLUE);
            
            if (mazeEditMode && (imagePoint.x >= 0) && (imagePoint.y >= 0) && 
               (imagePoint.x < imMaze.width) && (imagePoint.y < imMaze.height))
            {
                DrawRectangleLines(position.x+imagePoint.x*MAZE_DRAW_SIZE, 
                position.y+imagePoint.y*MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, MAZE_DRAW_SIZE, GREEN);
            }
            
            DrawText(TextFormat("MOUSE: [%i, %i]", mousePoint.x, mousePoint.y), 10, 40, 20, DARKBLUE);
            DrawText(TextFormat("IMAGE: [%i, %i]", imagePoint.x, imagePoint.y), 10, 70, 20, RED);
            
            DrawFPS(10, 10);

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
                        ImageDrawPixel(&imMaze, x, y, RED);
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