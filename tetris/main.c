#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#define HEIGHT 14
#define WIDTH 18
#define BLOCK_H 4
#define BLOCK_W 4

HANDLE hand;
int cur_x, cur_y;
int block_cur[BLOCK_H][BLOCK_W] = { 0 };
int screen[HEIGHT][WIDTH] = { 0 };
// block type
int blockO[BLOCK_H][BLOCK_W] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0} };
int blockL[BLOCK_H][BLOCK_W] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0} };
int blockJ[BLOCK_H][BLOCK_W] = {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0} };
int blockI[BLOCK_H][BLOCK_W] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0} };
int blockZ[BLOCK_H][BLOCK_W] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0} };
int blockS[BLOCK_H][BLOCK_W] = {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0} };
int blockT[BLOCK_H][BLOCK_W] = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0} };
enum block_type
{
    O,
    J,
    L,
    I,
    Z,
    S,
    T
};
int blockTypeNum = 7;
int score = 0;
int tm = 800;
UINT_PTR timerId;

void PrintStart()
{
    printf("######## ######## ######## ########  ####  ######  \n");
    printf("   ##    ##          ##    ##     ##  ##  ##    ## \n");
    printf("   ##    ##          ##    ##     ##  ##  ##       \n");
    printf("   ##    ######      ##    ########   ##   ######  \n");
    printf("   ##    ##          ##    ##   ##    ##        ## \n");
    printf("   ##    ##          ##    ##    ##   ##  ##    ## \n");
    printf("   ##    ########    ##    ##     ## ####  ######  \n");
}

void PrintControl()
{
    printf("                                                   \n");
    printf("              press 🠕 to rotate block              \n");
    printf("              press ←/→ to move block              \n");
    printf("          press ↓ to accelerate drop speed         \n");
    printf("      press space to move the block to the floor   \n");
    printf("                                                   \n");
}

void SetCursorVisible(int v)
{
    CONSOLE_CURSOR_INFO info = { 1, v };
    SetConsoleCursorInfo(hand, &info);
}
void Gotoxy(int x, int y)
{
    COORD pos = { .X = x, .Y = y };
    SetConsoleCursorPosition(hand, pos);
}
static void TimerSet(int t)
{
    KillTimer(NULL, timerId);
    timerId = SetTimer(NULL, 0, t, NULL);
}
static void TimerClose()
{
    KillTimer(NULL, timerId);
}

void XYPrint(char c, int x, int y) // print block
{
    Gotoxy(x, y);
    if (c == '|')
        printf("\033[1;31m"); // red
    else if (c == 'W' || c == 'M')
        printf("\033[1;33m"); // yellow
    else if (c == 'O')
        printf("\033[1;36m"); // cyan
    else
        printf("\033[1;37m"); // reset white

    printf("%c", c);
}

void PrintScreen()
{
    int i, j;
    for (i = 0; i < WIDTH; i++)
    {
        // ceiling
        XYPrint('W', i + 6, 1);
        // floor
        XYPrint('M', i + 6, 2 + HEIGHT);
    }
    for (j = -1; j < HEIGHT + 1; j++)
    {
        // wall
        XYPrint('|', 5, j + 2);
        XYPrint('|', 6 + WIDTH, j + 2);
    }
    for (i = 0; i < WIDTH; i++)
    {
        for (j = 0; j < HEIGHT; j++)
        {
            // block
            if (screen[j][i] == 0)
                XYPrint(' ', i + 6, j + 2);

            else
                XYPrint('O', i + 6, j + 2);
        }
    }

    Gotoxy(0, 20);
    printf("score:%d", score);
}
void PrintBlock(int block[][BLOCK_W], int x, int y)
{
    int i, j;
    for (i = 0; i < BLOCK_H; i++)
    {
        for (j = 0; j < BLOCK_W; j++)
        {
            if (block[i][j] == 1)
                XYPrint('O', x + 6 + j, y + 2 + i);
        }
    }
}
void EraseBlock(int block[][BLOCK_W], int x, int y) // clear block
{
    int i, j;
    for (i = 0; i < BLOCK_W; i++)
    {
        for (j = 0; j < BLOCK_H; j++)
        {
            if (block[i][j] == 1)
                XYPrint(' ', x + 6 + j, y + 2 + i);
        }
    }
}

// fill block
int GetBlockYFill(int row)
{
    int ans = 0;
    int col;
    for (col = 0; col < BLOCK_W; col++)
    {
        if (block_cur[row][col] == 1)
            ans = 1;
    }
    return ans;
}
int GetBlockXFill(int col)
{
    int ans = 0;
    int row;
    for (row = 0; row < BLOCK_W; row++)
    {
        if (block_cur[row][col] == 1)
            ans = 1;
    }
    return ans;
}
void GetBlockLeftRightMargin(int* left_margin, int* right_margin)
{
    int col;
    for (col = 0; col < 2; col++)
    {
        if (GetBlockXFill(col) == 0)
            *left_margin = *left_margin + 1;
    }
    for (col = 2; col < 4; col++)
    {
        if (GetBlockXFill(col) == 0)
            *right_margin = *right_margin + 1;
    }
    return;
}
void GetBlockTopButtonMargin(int* top_margin, int* button_margin)
{
    for (int row = 0; row < 2; row++)
    {
        if (GetBlockYFill(row) == 0)
            *top_margin = *top_margin + 1;
    }
    for (int row = 2; row < 4; row++)
    {
        if (GetBlockYFill(row) == 0)
            *button_margin = *button_margin + 1;
    }
}

int IsCollision(int x, int y) // check if collided
{
    int top_margin = 0;
    int button_margin = 0;
    int left_margin = 0;
    int right_margin = 0;

    GetBlockTopButtonMargin(&top_margin, &button_margin);
    GetBlockLeftRightMargin(&left_margin, &right_margin);

    if (x < 0 - left_margin || y < 0 - top_margin || x > WIDTH - BLOCK_W + right_margin || y > HEIGHT - BLOCK_H + button_margin)
        return 1;

    int w, h;
    for (w = 0; w < BLOCK_W; w++)
    {
        for (h = 0; h < BLOCK_H; h++)
        {
            if (block_cur[h][w] == 1)
            {
                if (screen[y + h][x + w] == 1)
                    return 1;
            }
        }
    }
    return 0;
}

int IsLineFill(int h)
{
    int w;
    for (w = 0; w < WIDTH; w++)
    {
        if (screen[h][w] == 0)
            return 0;
    }
    return 1;
}
void ClearOneLine(int h)
{
    int w, row;
    for (row = h; row > 0; row--)
    {
        for (w = 0; w < WIDTH; w++)
        {
            screen[row][w] = screen[row - 1][w];
        }
    }
    for (w = 0; w < WIDTH; w++)
    {
        screen[0][w] = 0;
    }
}
void CheckLine()
{
    int totalLine = 0;
    int h;
    for (h = cur_y; h < HEIGHT; h++)
    {
        if (IsLineFill(h) == 1)
        {
            ClearOneLine(h);
            PrintScreen();
            totalLine++;
        }
    }
    score += totalLine;
}
void LandBlock()
{
    int w, h;
    for (w = 0; w < BLOCK_W; w++)
    {
        for (h = 0; h < BLOCK_H; h++)
        {
            if (block_cur[h][w] == 1)
                screen[cur_y + h][cur_x + w] = 1;
        }
    }
}

void CopyBlock(int block_s[][BLOCK_W], int block_d[][BLOCK_W])
{
    int w, h;
    for (w = 0; w < BLOCK_W; w++)
    {
        for (h = 0; h < BLOCK_H; h++)
        {
            block_d[h][w] = block_s[h][w];
        }
    }
}
void RotateBlock()
{
    int temp[BLOCK_H][BLOCK_W] = { 0 };
    CopyBlock(block_cur, temp);
    int w, h;
    for (w = 0; w < BLOCK_W; w++)
    {
        for (h = 0; h < BLOCK_H; h++)
        {
            block_cur[h][w] = temp[w][BLOCK_H - 1 - h];
        }
    }
}
void MakeNewBlock()
{
    enum block_type type = (int)(rand() % 7);
    switch (type)
    {
    case O:
        CopyBlock(blockO, block_cur);
        break;
    case J:
        CopyBlock(blockJ, block_cur);
        break;
    case L:
        CopyBlock(blockL, block_cur);
        break;
    case I:
        CopyBlock(blockI, block_cur);
        break;
    case Z:
        CopyBlock(blockZ, block_cur);
        break;
    case S:
        CopyBlock(blockS, block_cur);
        break;
    case T:
        CopyBlock(blockT, block_cur);
        break;
    default:
        break;
    }
    cur_x = (WIDTH - BLOCK_W) / 2;
    cur_y = 0;
    PrintBlock(block_cur, cur_x, cur_y);
}
void MoveBlockDown()
{
    if (IsCollision(cur_x, cur_y + 1) != 1)
    {
        EraseBlock(block_cur, cur_x, cur_y);
        cur_y++;
        PrintBlock(block_cur, cur_x, cur_y);
    }
    else
    {
        LandBlock();
        CheckLine();
        MakeNewBlock();
        PrintBlock(block_cur, cur_x, cur_y);
        if (IsCollision(cur_x, cur_y) == 1)
            TimerClose();
    }
}
void KeyControl()
{
    MSG msg;
    while (1)
    {
        if (PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE) != 0)
            MoveBlockDown();

        if (_kbhit() != 0)
        {
            char ch = _getch();
            switch (ch)
            {
            case 72: // up
                EraseBlock(block_cur, cur_x, cur_y);
                RotateBlock();
                if (IsCollision(cur_x, cur_y) == 1)
                {
                    RotateBlock();
                    RotateBlock();
                    RotateBlock();
                }
                PrintBlock(block_cur, cur_x, cur_y);
                break;
            case 80: // down
                if (IsCollision(cur_x, cur_y + 1) != 1)
                {
                    EraseBlock(block_cur, cur_x, cur_y);
                    cur_y++;
                    PrintBlock(block_cur, cur_x, cur_y);
                }
                break;
            case 75: // left
                if (IsCollision(cur_x - 1, cur_y) != 1)
                {
                    EraseBlock(block_cur, cur_x, cur_y);
                    cur_x--;
                    PrintBlock(block_cur, cur_x, cur_y);
                }
                break;
            case 77: // right
                if (IsCollision(cur_x + 1, cur_y) != 1)
                {
                    EraseBlock(block_cur, cur_x, cur_y);
                    cur_x++;
                    PrintBlock(block_cur, cur_x, cur_y);
                }
                break;
            case 32: // space
                while (IsCollision(cur_x, cur_y + 1) != 1)
                {
                    EraseBlock(block_cur, cur_x, cur_y);
                    cur_y++;
                    PrintBlock(block_cur, cur_x, cur_y);
                }
                break;
            }
        }
    }
}

int main()
{
    PrintStart();
    system("pause");
    PrintControl();
    Sleep(3);
    system("cls");

    hand = GetStdHandle(STD_OUTPUT_HANDLE);
    srand(time(NULL));
    SetCursorVisible(0);
    TimerSet(tm);

    int i, j;
    for (i = 0; i < WIDTH; i++)
    {
        for (j = 0; j < HEIGHT; j++)
        {
            screen[j][i] = 0;
        }
    }

    PrintScreen();
    MakeNewBlock();
    KeyControl();
    system("pause");
    return 0;
}
