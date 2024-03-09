#include <stdio.h>
#include <assert.h>
#include "../raylib/raylib-5.0_macos/include/raylib.h"

#define APP_NAME "paint.xd"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

// Sizes
#define TOP_PANE_HEIGHT 24
#define BOTTOM_PANE_HEIGHT 68
#define LEFT_PANE_WIDTH BOTTOM_PANE_HEIGHT
#define DRAWING_AREA_MARGIN 10
#define DRAWING_AREA_WIDTH SCREEN_WIDTH - LEFT_PANE_WIDTH - DRAWING_AREA_MARGIN*2
#define DRAWING_AREA_HEIGHT SCREEN_HEIGHT - TOP_PANE_HEIGHT - BOTTOM_PANE_HEIGHT - DRAWING_AREA_MARGIN*2
#define TOOL_PANE_MARGIN 4
#define TOOL_BUTTON_SIZE 30

// Colors
#define PANE_COLOR LIGHTGRAY
#define DRAWING_AREA_COLOR WHITE
#define BACKGROUND_COLOR DARKGRAY
#define BUTTON_BORDER DARKGRAY

typedef enum {
    CIRC,
    RECT,
    ERASER,
    PENCIL,
    SELECT,
    TOOLS_COUNT
} Tool;

typedef enum {
    DECREASE_TOOL_SIZE,
    INCREASE_TOOL_SIZE,
    CLEAR_DRAW_AREA,
    NONE_1,
    NONE_2,
    NONE_3,
    UNDO,
    ACTIONS_COUNT
} Action;

typedef struct {
    Tool tool;
    Texture2D texture;
} ToolIcon;

ToolIcon toolIcons[TOOLS_COUNT];

Tool selectedTool = CIRC;
Color selectedColor = BLACK;
size_t selectedSize = 15;
Color eraserColor = WHITE;

size_t screenWidth = SCREEN_WIDTH;
size_t screenHeight = SCREEN_HEIGHT;

const Color colors[] = {
    BLACK, RED, YELLOW, PINK, SKYBLUE, GOLD, LIME,
    WHITE, BLUE, GREEN, PURPLE, VIOLET, ORANGE, DARKBLUE
};

#define MAX_HISTORY 256

size_t texturesCount = 0;
RenderTexture2D textures[MAX_HISTORY] = {0};
RenderTexture2D drawingBuffer = {0};
RenderTexture2D tempBuffer = {0};

bool isMousePressed = false;
Vector2 startPoint = {0};
Vector2 endPoint = {0};

void AddDrawingBufferToPainting() {
    Rectangle rect = { 0, 0, (float)drawingBuffer.texture.width, (float)-drawingBuffer.texture.height };
    Vector2 pos = { 0, 0 };

    RenderTexture2D texture = LoadRenderTexture(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
    BeginTextureMode(texture);
    DrawTextureRec(drawingBuffer.texture, rect, pos, WHITE);
    EndTextureMode();

    textures[texturesCount] = texture;
    texturesCount++;

    UnloadRenderTexture(drawingBuffer);
    drawingBuffer = LoadRenderTexture(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
}

void ClearPaitingArea() {
    printf("Clearing canvas, there is %lu textures to remove\n", texturesCount);

    for (size_t i = 0; i < texturesCount; i++)
    {
        printf("Clearing texture: %lu\n", i);
        UnloadRenderTexture(textures[i]);
    }
    texturesCount = 0;

    BeginTextureMode(drawingBuffer);
    ClearBackground(DRAWING_AREA_COLOR);
    EndTextureMode();

    AddDrawingBufferToPainting();

    printf("Canvas was cleaned, there is now %lu in canvas\n", texturesCount);
}

void RenderBrushAdvanced(Tool tool, int size, bool ignorePosition) {
    Rectangle drawArea = {
        .x = LEFT_PANE_WIDTH + DRAWING_AREA_MARGIN,
        .y = TOP_PANE_HEIGHT + DRAWING_AREA_MARGIN,
        .width = DRAWING_AREA_WIDTH,
        .height = DRAWING_AREA_HEIGHT
    };
    Vector2 mousePosition = GetMousePosition();

    if (CheckCollisionPointRec(mousePosition, drawArea) || ignorePosition) {
        switch (tool)
        {
            case RECT: {
                DrawRectangle(GetMouseX() - size/2, GetMouseY() - size/2, size, size, selectedColor);
                break;
            }
            
            case CIRC: {
                DrawCircle(GetMouseX(), GetMouseY(), size, selectedColor);
                break;
            }

            case ERASER: {
                DrawRectangle(GetMouseX() - size/2, GetMouseY() - size/2, size, size, eraserColor);
                break;
            }

            case PENCIL: {
                DrawRectangle(GetMouseX(), GetMouseY(), 1, 1, selectedColor);
                break;
            }

            case SELECT: {
                break;
            }

            default:
                break;
        }
    }
}

void RenderBrushWithTool(Tool tool, bool ignorePosition) {
    RenderBrushAdvanced(tool, selectedSize, ignorePosition);
}

void RenderBrush(bool ignorePosition) {    
    RenderBrushAdvanced(selectedTool, selectedSize, ignorePosition);
}

void AddToDrawing(int drawX, int drawY) {
    if (selectedTool == SELECT) {
        UnloadRenderTexture(tempBuffer);
        tempBuffer = LoadRenderTexture(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
    }
    RenderTexture2D currentRenderingBuffer = selectedTool == SELECT ? tempBuffer : drawingBuffer;
    BeginTextureMode(currentRenderingBuffer);
    {
        switch (selectedTool)
        {
            case RECT: {
                DrawRectangle(drawX - selectedSize/2, drawY - selectedSize/2, selectedSize, selectedSize, selectedColor);
                break;
            }
            
            case CIRC: {
                DrawCircle(drawX, drawY, selectedSize, selectedColor);
                break;
            }

            case ERASER: {
                DrawRectangle(drawX - selectedSize/2, drawY - selectedSize/2, selectedSize, selectedSize, eraserColor);
                break;
            }

            case PENCIL: {
                DrawRectangle(drawX, drawY, 1, 1, selectedColor);
                break;
            }

            case SELECT: {
                if (!isMousePressed) {
                    if (startPoint.x != endPoint.x && startPoint.x != endPoint.x) {
                        
                        Rectangle selection = {
                            .x = startPoint.x > endPoint.x ? endPoint.x : startPoint.x,
                            .y = startPoint.y > endPoint.y ? endPoint.y : startPoint.y,
                            .width = endPoint.x > startPoint.x ? endPoint.x - startPoint.x : startPoint.x - endPoint.x,
                            .height = endPoint.y > startPoint.y ? endPoint.y - startPoint.y : startPoint.y - endPoint.y,
                        };
                        DrawRectangleLinesEx(selection, 1, BLACK);
                    }
                }
                break;
            }

            default:
                break;
        }
    }
    EndTextureMode();
}

void RenderPainting() {
    Rectangle rect = { 0, 0, (float)drawingBuffer.texture.width, (float)-drawingBuffer.texture.height };
    Vector2 pos = { LEFT_PANE_WIDTH + DRAWING_AREA_MARGIN, TOP_PANE_HEIGHT + DRAWING_AREA_MARGIN };

    for (size_t i = 0; i < texturesCount; i++)
    {
        DrawTextureRec(textures[i].texture, rect, pos, WHITE);
    }
    DrawTextureRec(drawingBuffer.texture, rect, pos, WHITE);
    DrawTextureRec(tempBuffer.texture, rect, pos, WHITE);
}

void HandleDrawing() {
    static bool drawingMode = false;
    Rectangle drawArea = {
        .x = LEFT_PANE_WIDTH + DRAWING_AREA_MARGIN,
        .y = TOP_PANE_HEIGHT + DRAWING_AREA_MARGIN,
        .width = DRAWING_AREA_WIDTH,
        .height = DRAWING_AREA_HEIGHT
    };

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePosition = GetMousePosition();
        if (CheckCollisionPointRec(mousePosition, drawArea)) {
            int drawX = mousePosition.x - LEFT_PANE_WIDTH - DRAWING_AREA_MARGIN;
            int drawY = mousePosition.y - TOP_PANE_HEIGHT - DRAWING_AREA_MARGIN;
            if (!isMousePressed) {
                startPoint = (Vector2) { .x = drawX, .y = drawY };
                printf("start  - x = %f y = %f\n", startPoint.x, startPoint.y);
            }
            isMousePressed = true;
            AddToDrawing(drawX, drawY);
            drawingMode = true;
        }
        else {
            isMousePressed = false;
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && drawingMode) {
        if (isMousePressed) {
            printf("Adding new texture, before add there  %lu textures in buffer\n", texturesCount);
            assert(texturesCount + 1 < MAX_HISTORY && "MAX HISTORY REACHED!");
            AddDrawingBufferToPainting();
            drawingMode = false;
            printf("Exiting drawing mode, there is %lu textures in buffer now\n", texturesCount);
        }
        isMousePressed = false;
        if (selectedTool == SELECT) {
            Vector2 pos = GetMousePosition();
            int drawX = pos.x - LEFT_PANE_WIDTH - DRAWING_AREA_MARGIN;
            int drawY = pos.y - TOP_PANE_HEIGHT - DRAWING_AREA_MARGIN;
            endPoint = (Vector2) { .x = drawX, .y = drawY };
            AddToDrawing(endPoint.x, endPoint.y);
        }
    }

    RenderPainting();
}

void RenderPane(int x, int y, int width, int heigth) {
    DrawRectangle(x, y, width, heigth, PANE_COLOR);
}

void RenderTopPane() {
    RenderPane(0, 0, screenWidth, TOP_PANE_HEIGHT);
}

void RenderToolButton(int x, int y, Tool tool) {
    Rectangle toolButtonRect = {
        .x = x,
        .y = y,
        .width = TOOL_BUTTON_SIZE,
        .height = TOOL_BUTTON_SIZE
    };

    Vector2 mousePosition = GetMousePosition();
    if (CheckCollisionPointRec(mousePosition, toolButtonRect)) {
        RenderBrushWithTool(tool, true);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, toolButtonRect)) {
            selectedTool = tool;
        }
    }

    Texture2D toolTexture;
    for (size_t i = 0; i < TOOLS_COUNT; i++)
    {
        if ((Tool)i == tool) {
            toolTexture = toolIcons[i].texture;
        }
    }
    DrawTextureEx(toolTexture, (Vector2) { x = x + 4, y = y + 4}, 1, 1, WHITE);
    DrawRectangleLinesEx(toolButtonRect, 1, BUTTON_BORDER);
}

void RenderActionButton(int x, int y, Action action) {
    Rectangle toolButtonRect = {
        .x = x,
        .y = y,
        .width = TOOL_BUTTON_SIZE,
        .height = TOOL_BUTTON_SIZE
    };

    Vector2 mousePosition = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePosition, toolButtonRect) && (action == INCREASE_TOOL_SIZE || action == DECREASE_TOOL_SIZE)) {
        RenderBrush(true);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, toolButtonRect)) {
            switch (action)
            {
                case INCREASE_TOOL_SIZE: {
                    selectedSize += 1;
                    break;
                }
                case DECREASE_TOOL_SIZE: {
                    selectedSize -= 1;
                    break;
                }
                case CLEAR_DRAW_AREA: {
                    ClearPaitingArea();
                    break;
                }
                
                case UNDO: {
                    if (texturesCount > 1) {
                        printf("Unloading texture on undo: %lu\n", texturesCount);
                        UnloadRenderTexture(textures[texturesCount]);
                        texturesCount = texturesCount - 1;
                    }
                    break;
                }
                
                default:
                    break;
            }
        }
    }

    DrawRectangleLinesEx(toolButtonRect, 1, BLACK);
}

void RenderColorButton(int x, int y, Color color) {
    Rectangle toolButtonRect = {
        .x = x,
        .y = y,
        .width = TOOL_BUTTON_SIZE,
        .height = TOOL_BUTTON_SIZE
    };

    DrawRectangleRec(toolButtonRect, color);
    DrawRectangleLinesEx(toolButtonRect, 1, BUTTON_BORDER);
    Vector2 mousePosition = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, toolButtonRect)) {
            selectedColor = color;
        }
    }
}

void RenderToolsPane() {
    int startPosX = TOOL_PANE_MARGIN;
    int startPoxY = TOP_PANE_HEIGHT;
    for (size_t i = 0; i < TOOLS_COUNT; i++)
    {
        int addX = i % 2 == 0 ? 0 : TOOL_BUTTON_SIZE - (i != 0 ? 1 : 0);
        int addY = 30 * (i / 2) - (i == 0 || i == 1 ? 0 : 1);
        RenderToolButton(startPosX + addX, startPoxY + addY, (Tool)i);
    }
}

void RenderActionsPane() {
    int startPosX = TOOL_PANE_MARGIN;
    int startPoxY = TOP_PANE_HEIGHT + 200;
    for (size_t i = 0; i < ACTIONS_COUNT; i++)
    {
        int addX = i % 2 == 0 ? 0 : TOOL_BUTTON_SIZE - (i != 0 ? 1 : 0);
        int addY = 30 * (i / 2) - (i == 0 || i == 1 ? 0 : 1);
        if (i != NONE_1 && i != NONE_2 && i != NONE_3) {
            RenderActionButton(startPosX + addX, startPoxY + addY, (Action)i);
        }
    }
}

void RenderLeftPane() {
    RenderPane(0, TOP_PANE_HEIGHT, LEFT_PANE_WIDTH, screenHeight - TOP_PANE_HEIGHT - BOTTOM_PANE_HEIGHT);
    RenderToolsPane();
    RenderActionsPane();
}

void RenderBottomPane() {
    int posX = screenHeight - BOTTOM_PANE_HEIGHT;
    RenderPane(0, posX, screenWidth, BOTTOM_PANE_HEIGHT);

    assert(sizeof(colors) / sizeof(Color) % 2 == 0 && "SIZE OF COLOR ARRAY SHOULD BE DIVISIBLE BY 2");
    int arrPosition = 0;
    for (size_t i = 0; i < 2; i++)
    {
        for (size_t j = 0; j < sizeof(colors) / sizeof(Color) / 2; j++)
        {
            int x = (LEFT_PANE_WIDTH + j*TOOL_BUTTON_SIZE) - (j == 0 || j == 7 ? 0 : 1*j);
            int y = (posX + TOOL_PANE_MARGIN + i*TOOL_BUTTON_SIZE) - (i == 1 ? 1 : 0);
            RenderColorButton(x, y, colors[arrPosition]);
            arrPosition += 1;
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, APP_NAME);
    SetTargetFPS(120);

    char filePath[10+4+1+4+1] = {0};
    for (size_t i = 0; i < TOOLS_COUNT; i++)
    {
        sprintf(filePath, "resources/tool%lu.png", i + 1);
        Image image = LoadImage(filePath);
        Texture2D texture = LoadTextureFromImage(image);
        toolIcons[i] = (ToolIcon) { .texture = texture, .tool = (Tool)i };
        UnloadImage(image);
    }

    drawingBuffer = LoadRenderTexture(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
    BeginTextureMode(drawingBuffer);
    ClearBackground(DRAWING_AREA_COLOR);
    EndTextureMode();

    tempBuffer = LoadRenderTexture(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);

    AddDrawingBufferToPainting();

    printf("Painting begined with %lu textures in buffer\n", texturesCount);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();

            ClearBackground(BACKGROUND_COLOR);
            RenderTopPane();
            RenderLeftPane();
            RenderBottomPane();

            HandleDrawing();
            RenderPainting();
            RenderBrush(false);
        }
        EndDrawing();
    }
    
    UnloadRenderTexture(drawingBuffer);
    for (size_t i = 0; i < texturesCount; i++)
    {
        UnloadRenderTexture(textures[i]);
    }

    CloseWindow();
    printf("%s closed\n", APP_NAME);
    return 0;
}