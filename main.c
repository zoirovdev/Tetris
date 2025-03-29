#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//---------------------------------------------------------------------------
//  Some Defines
//---------------------------------------------------------------------------
#define SQUARE_SIZE             20
#define GRID_HORIZONTAL_SIZE    12
#define GRID_VERTICAL_SIZE      10

#define LATERAL_SPEED           10
#define TURNING_SPEED           12
#define FAST_FALL_AWAIT_COUNTER 30

#define FADING_TIME             33


//---------------------------------------------------------------------------
// Types and Structures Definitions
//---------------------------------------------------------------------------
typedef enum GridSquare { EMPTY, MOVING, FULL, BLOCK, FADING } GridSquare;


//---------------------------------------------------------------------------
// Global Variables Declaration
//---------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;


// Matrices
static GridSquare grid [GRID_HORIZONTAL_SIZE][GRID_VERTICAL_SIZE];
static GridSquare piece [4][4];
static GridSquare incomingPiece [4][4];


// These variables keep track of the active piece position
static int piecePositionX = 0;
static int piecePositionY = 0;


// Game parametres
static Color fadingColor;

static bool beginPlay = true;
static bool pieceActive = false;
static bool detection = false;
static bool lineToDelete = false;


// Statistics
static int level = 1;
static int lines = 0;


// Counters
static int gravityMovementCounter = 0;
static int lateralMovementCounter = 0;
static int turnMovementCounter = 0;
static int fastFallMovementCounter = 0;

static int fadeLineCounter = 0;

// Based on level
static int gravitySpeed = 30;


//---------------------------------------------------------------
// Module Functions Declaration
//---------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)


// Additional Module Functions
static bool Createpiece();
static void GetRandompiece();
static void ResolveFallingMovement(bool *detection, bool *pieceActive);
static bool ResolveLateralMovement();
static bool ResolveTurnMovement();
static void CheckDetection(bool *detection);
static void CheckCompletion(bool *lineToDelete);
static int DeleteCompleteLines();


//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    InitWindow(screenWindth, screenHeight, "classic game: tetris");

    InitGame();

    SetTargetFPS(60);

    // Main game loop
    while(!WindowShouldClose())
    {
        // Update and Draw
        UpdateDrawFrame();
    }

    //DeInitialization
    UnloadGame();           // Unload loaded data (textures, sounds, models...)

    CloseWindow();          // Close window and OpenGL context

    return 0;
}


//--------------------------------------------------------
// Game Module Functions Definition
//--------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    // Initialize game statistics
    level = 1;
    lines = 0;

    fadingColor = GRAY;

    piecePositionX = 0;
    piecePositionY = 0;

    puase = false;

    beginPlay = true;
    pieceActive = false;
    detection = false;
    lineToDelete = false;


    // Counters
    gravityMovementCounter = 0;
    lateralMovementCounter = 0;
    turnMovementCounter = 0;
    fastFallMovementCounter = 0;

    fadeLineCounter = 0;
    gravitySpeed = 30;


    // Initialize grid matrices
    for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
    {
        for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
        {
            if ((j == GRID_VERTICAL_SIZE - 1) || (i == 0) || (i == GRID_HORIZONTAL_SIZE - 1)) grid[i][j] = BLOCK;
            else grid[i][j] = EMPTY;
        }
    }


    // Initialize incoming piece matrices
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            incomingPiece[i][j] = EMPTY;
        }
    }
}


// Update game (one frame)
void UpdateGame(void)
{
    if(!gameOver)
    {
        if(IsKeyPressed('P')) pause = !puase;

        if(!pause)
        {
            if(!lineToDelete)
            {
                if(!pieceActive)
                {
                    // Get another piece
                    pieceActive = Createpiece();

                    // We leave a little time before starting the fast falling down
                    fatsFallMovementCounter = 0;
                }
                else // Piece falling
                {
                    // Counters update 
                    fastFallMovementCounter++;
                    gravityMovementCounter++;
                    lateralMovementCounter++;
                    turnMovementCounter++;

                    // We make sure to move if we've pressed the key this frame
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) lateralMovementCounter = LATERAL_SPEED;
                    if (IsKeyPressed(KEY_UP)) turnMovementCounter = TURNING_SPEED;
                
                    // Fall down
                    if (IsKeyDown(KEY_DOWN) && (fastFallMovementCounter >= FAST_FALL_AWAIT_COUNTER))
                    {
                        // We make sure the piece is going to fall this frame
                        gravityMovementCounter += gravitySpeed;
                    }

                    if (gravityMovementCounter >= gravitySpeed)
                    {
                        // Basic falling movement
                        CheckDetection(&detection);

                        // Check if the piece has collided with another piece or with the boundings
                        ResolveFallingMovement(&detection, &pieceActive);

                        // Check if we fullfilled a line and if so, erase the line and pull down the lines above
                        CheckCompletion(&lineToDelete);

                        gravityMovementCounter = 0;
                    }

                    // move laterally at player's will
                    if (lateralMovementCounter >= LATERAL_SPEED)
                    {
                        // Update the lateral movement and if success, reset the lateral counter
                        if (!ResolveLateralMovement()) lateralMovementCounter = 0;
                    }

                    // turn the piece at player's will
                    if (turnMovementCounter >= TURNING_SPEED)
                    {
                        // Update the turning movement and reset the turning counter
                        if (ResolveTurnMovement()) turnMovementCounter = 0;
                    }
                }

                // Game over logic
                for  (int j = 0; j < 2; j++)
                {
                    for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
                    {
                        if (grid[i][j] == FULL)
                        {
                            gameOver = true;
                        }
                    }
                }
            }
            else 
            {
                // Animation when deleting lines 
                fadeLineCounter++;

                if (fadeLineCounter%8 < 4) fadingColor = MAROON;
                else fadingColor = GRAY;

                if (fadeLineCounter >= FADING_TIME)
                {
                    int deletedLines = 0;
                    deletedLines = DeleteCompleteLines();
                    fadeLineCounter = 0;
                    lineToDelete = false;
                
                    lines += deletedLines;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}


// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        // Draw gameplay area
        Vector2 offset;
        offset.x = screenWidth/2 - (GRID_HORIZONTAL_SIZE*SQUARE_SIZE/2) - 50;
        offset.y = screenHeight/2 - ((GRID_VERTICAL_SIZE - 1)*SQUARE_SIZE/2) + SQUARE_SIZE*2;

        offset.y -= 50;     // NOTE: Hardcoded position;

        int controller = offset.x;

        for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
        {
            for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
            {
                // Draw each square of the grid
                if (grid[i][j] == EMPTY)
                {









