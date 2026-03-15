/*******************************************************************************************
*
*   raylib maze game
*
*   Procedural maze generator using Maze Grid Algorithm
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2024-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: malloc(), free()

#define MAZE_WIDTH          64
#define MAZE_HEIGHT         64
#define MAZE_SCALE          10.0f

#define MAX_MAZE_ITEMS      16

// Declare new data type: Point
typedef struct Point {
    int x;
    int y;
} Point;

// Generate procedural maze image, using grid-based algorithm
// NOTE: Functions defined as static are internal to the module
static Image GenImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Delivery04 - maze game");

    // Current application mode
    int currentMode = 1;    // 0-Game, 1-Editor

    // Random seed defines the random numbers generation,
    // always the same if using the same seed
    unsigned int seed = 0;
    SetRandomSeed(seed);

    // Generate maze image using the grid-based generator
    // TODO: [1p] Implement GenImageMaze() function with required parameters
    Image imMaze = GenImageMaze(MAZE_WIDTH, MAZE_HEIGHT, 4, 4, 0.75f);

    // Load a texture to be drawn on screen from our image data
    // WARNING: If imMaze pixel data is modified, texMaze needs to be re-loaded
    Texture texMaze = LoadTextureFromImage(imMaze);

    // Player start-position and end-position initialization
    Point startCell = { 1, 1 };
    Point endCell = { imMaze.width - 2, imMaze.height - 2 };

    // Maze drawing position (editor mode)
    Vector2 mazePosition = {
        GetScreenWidth()/2 - texMaze.width*MAZE_SCALE/2,
        GetScreenHeight()/2 - texMaze.height*MAZE_SCALE/2
    };

    // Define player position and size
    Rectangle player = { mazePosition.x + 1*MAZE_SCALE + 2, mazePosition.y + 1*MAZE_SCALE + 2, 4, 4 };

    // Camera 2D for 2d gameplay mode
    // TODO: [2p] Initialize camera parameters as required
    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.zoom = 1.0f;

    // Mouse selected cell for maze editing
    Point selectedCell = { 0 };

    // Maze items position and state
    Point mazeItems[MAX_MAZE_ITEMS] = { 0 };
    bool mazeItemPicked[MAX_MAZE_ITEMS] = { 0 };
    
    // Define textures to be used as our "biomes"
    Texture texBiomes[4] = { 0 };
    texBiomes[0] = LoadTexture("resources/maze_atlas01.png");
    texBiomes[1] = LoadTexture("resources/maze_atlas02.png");
    texBiomes[2] = LoadTexture("resources/maze_atlas03.png");
    texBiomes[3] = LoadTexture("resources/maze_atlas04.png");

    // TODO: Load additional textures for different biomes
    int currentBiome = 0;

    // TODO: Define all variables required for game UI elements (sprites, fonts...)

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Select current mode as desired
        if (IsKeyPressed(KEY_SPACE)) currentMode = !currentMode; // Toggle mode: 0-Game, 1-Editor

        if (currentMode == 0) // Game mode
        {
            // TODO: [2p] Player 2D movement from predefined Start-point to End-point
            // Implement maze 2D player movement logic (cursors || WASD)
            // Use imMaze pixel information to check collisions
            // Detect if current playerCell == endCell to finish game

            // TODO: [1p] Camera 2D system following player movement around the map
            // Update Camera2D parameters as required to follow player and zoom control

            // TODO: [2p] Maze items pickup logic
        }
        else if (currentMode == 1) // Editor mode
        {
            // TODO: [2p] Maze editor mode, edit image pixels with mouse.
            // Implement logic to selecte image cell from mouse position -> TIP: GetMousePosition()
            // NOTE: Mouse position is returned in screen coordinates and it has to 
            // transformed into image coordinates
            // Once the cell is selected, if mouse button pressed add/remove image pixels
            
            // WARNING: Remember that when imMaze changes, texMaze must be also updated!

            // TODO: [2p] Collectible map items: player score
            // Using same mechanism than maze editor, implement an items editor, registering
            // points in the map where items should be added for player pickup -> TIP: Use mazeItems[]
        }

        // TODO: [1p] Multiple maze biomes supported
        // Implement changing between the different textures to be used as biomes
        // NOTE: For the 3d model, the current selected texture must be applied to the model material  

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (currentMode == 0) // Game mode
            {
                // Draw maze using camera2d (for automatic positioning and scale)
                BeginMode2D(camera2d);

                    // TODO: Draw maze walls and floor using current texture biome 
             
                    // TODO: Draw player rectangle or sprite at player position

                    // TODO: Draw maze items 2d (using sprite texture?)

                EndMode2D();

                // TODO: Draw game UI (score, time...) using custom sprites/fonts
                // NOTE: Game UI does not receive the camera2d transformations,
                // it is drawn in screen space coordinates directly
            }
            else if (currentMode == 1) // Editor mode
            {
                // Draw generated maze texture, scaled and centered on screen 
                DrawTextureEx(texMaze, mazePosition, 0.0f, MAZE_SCALE, WHITE);

                // Draw lines rectangle over texture, scaled and centered on screen 
                DrawRectangleLines(mazePosition.x, mazePosition.y, MAZE_WIDTH*MAZE_SCALE, MAZE_HEIGHT*MAZE_SCALE, RED);

                // TODO: Draw player using a rectangle, consider maze screen coordinates!

                // TODO: Draw editor UI required elements
            }

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texMaze);     // Unload maze texture from VRAM (GPU)
    UnloadImage(imMaze);        // Unload maze image from RAM (CPU)

    // TODO: Unload all loaded resources
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Generate procedural maze image, using grid-based algorithm
// NOTE: Color scheme used: WHITE = Wall, BLACK = Walkable, RED = Item
static Image GetImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance)
{
    Image imMaze = GenImageColor(width, height, BLACK);

    // Point mazePoints[64] = { 0 }; // Static memory alloc

    Point* mazePoints = malloc(64 * sizeof(Point));

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
                if ((x % spacingRows == 0) && (y % spacingCols == 0))
                {
                    if (GetRandomValue(0, 100) < pointChance * 100)
                    {
                        ImageDrawPixel(&imMaze, x, y, WHITE);
                        mazePoints[mazePointCounter] = (Point){ x, y }; // Inicializa los valores: x = x , y = y
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

    int* indices = LoadRandomSequence(mazePointCounter, 0, mazePointCounter - 1);

    for (int i = 0; i < mazePointCounter; i++)
    {
        int dir = GetRandomValue(0, 3);
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
