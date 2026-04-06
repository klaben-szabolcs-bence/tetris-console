#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include <random>
using namespace std;

// Gameplay characters
constexpr unsigned char CHAR_INDEX_EMPTY = 0;
constexpr unsigned char CHAR_INDEX_LINE_CLEAR = 8;
constexpr unsigned char CHAR_INDEX_BORDER = 9;
constexpr unsigned char CHAR_X = L'X';
constexpr unsigned char CHAR_A = 65;

// Global constants
constexpr int PIECE_LENGTH = 4;
constexpr int FIELD_WIDTH = 12;
constexpr int FIELD_HEIGHT = 18;
constexpr int SCREEN_WIDTH = 80;
constexpr int SCREEN_HEIGHT = 20;
constexpr int FIELD_OFFSET_X = 2;
constexpr int FIELD_OFFSET_Y = -1;
constexpr int PAUSE_LENGTH = 1000 * 100; // 100 milliseconds

// Global variables

constexpr int TETROMINO_COUNT = 7;
static std::random_device RandomDevice;
static std::mt19937 rng(RandomDevice());
static std::uniform_int_distribution<int> tetromino_dist(0, TETROMINO_COUNT - 1);

wstring tetromino[7];
unsigned char *pField = nullptr;
auto screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];

bool bGameOver = false;
int nCurrentPiece = 1;
int nCurrentRotation = 0;
int nCurrentPosX = FIELD_WIDTH / 2;
int nCurrentPosY = 0;

struct termios t;

void input_enter_off() {
    tcgetattr(STDIN_FILENO, &t);
    t.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    t.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void input_enter_on() {
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int Rotate(const int px, const int py, const int r) {
    switch (r % PIECE_LENGTH) {
        case 0:
            return py * PIECE_LENGTH + px; // 0 degrees
        case 1:
            return ( PIECE_LENGTH * ( PIECE_LENGTH - 1 ) ) + py - (PIECE_LENGTH  * px); // 90 degrees
        case 2:
            return ( (PIECE_LENGTH * PIECE_LENGTH ) - 1 ) - ( py * PIECE_LENGTH ) - px; // 180 degrees
        case 3:
            return ( PIECE_LENGTH - 1 ) - py + ( px * PIECE_LENGTH ); // 270 degrees
        default:
            return 0;
    }
}

bool DoesPieceFit(const int nTetromino, const int nRotation, const int nPosX, const int nPosY) {
    for (int px = 0; px < PIECE_LENGTH; ++px) {
        for (int py = 0; py < PIECE_LENGTH; ++py) {

            // Get index into the piece
            const int piece_index = Rotate(px, py, nRotation);

            // Get index into the field
            const int field_index = (nPosY + py) * FIELD_WIDTH + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < FIELD_WIDTH // In-between field width
                && nPosY + py >= 0 && nPosY + py < FIELD_HEIGHT // In-between field height
                && tetromino[nTetromino][piece_index] == CHAR_X && pField[field_index] != 0 // Collision detection
                )
                return false; // Fail on the first hit
        }
    }
    return true;
}

void DrawGameToScreen( vector<int>* vLines, int nScore) {
    // Draw the field
    for (int x = 0; x < FIELD_WIDTH; ++x) {
        for (int y = 0; y < FIELD_HEIGHT; ++y) {
            screen[(y + FIELD_OFFSET_Y) * SCREEN_WIDTH + (x + FIELD_OFFSET_X)] = L" ABCDEFG=#"[pField[x + y * FIELD_WIDTH]];
        }
    }

    // Draw Current Piece
    for (int px = 0; px < PIECE_LENGTH; ++px) {
        for (int py = 0; py < PIECE_LENGTH; ++py) {
            int piece_index = Rotate(px, py, nCurrentRotation);
            if (tetromino[nCurrentPiece][piece_index] == CHAR_X)
                screen[(nCurrentPosY + py + FIELD_OFFSET_Y) * SCREEN_WIDTH + (nCurrentPosX + px + FIELD_OFFSET_X)] = nCurrentPiece + CHAR_A;
        }
    }

    // Draw the Score out
    swprintf(&screen[2 * SCREEN_WIDTH + FIELD_WIDTH + 6], 16, L"SCORE: %8d", nScore);

    // Clear screen
    for (int i = 0; i < SCREEN_HEIGHT; ++i) { cout << endl; }

    if (!vLines->empty()) {
        // Cheat by drawing lines
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            if (i % SCREEN_WIDTH == 0) { cout << endl; }
            wcout << screen[i];
        }
        usleep(PAUSE_LENGTH * 2); // Little celebratory pause

        // Shift lines down
        for (const auto &v : *vLines)
            for (int px = 1; px < FIELD_WIDTH - 1; ++px) {
                for (int py = v; py > 0; --py) {
                    pField[py * FIELD_WIDTH + px] = pField[(py - 1) * FIELD_WIDTH + px];
                }
                pField[px] = CHAR_INDEX_EMPTY;
            }

        vLines->clear();
    }

    // Clear screen
    for (int i = 0; i < SCREEN_HEIGHT; ++i) { cout << endl; }

    // Print the current game screen
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; ++i) {
        if (i % SCREEN_WIDTH == 0) { cout << endl; }
        wcout << screen[i];
    }
}

void UserInput(const unsigned char user_input) {
    // If A key, try to move to the left
    nCurrentPosX -= user_input == 'a' && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentPosX - 1,
                                                      nCurrentPosY) ? 1 : 0;

    // If D key, try to move to the right
    nCurrentPosX += user_input == 'd' && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentPosX + 1,
                                                      nCurrentPosY) ? 1 : 0;

    // If S key, try to move down fast
    nCurrentPosY += user_input == 's' && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentPosX,
                                                      nCurrentPosY + 1) ? 1 : 0;

    // if W key, try to rotate clockwise
    nCurrentRotation += user_input == 'w' && DoesPieceFit(nCurrentPiece, (nCurrentRotation + 1) % 4, nCurrentPosX,
                                                          nCurrentPosY) ? 1 : 0;
}

void GameLoop() {
    // Game Logic initialization

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCounter = 0;
    int nScore = 0;

    vector<int> vLines;

    while (!bGameOver) {

        // GAME TIMING
        usleep(PAUSE_LENGTH);
        nSpeedCounter++;
        bForceDown = ( nSpeedCounter == nSpeed );

        // INPUT

        // char user_input = getchar();
        static unsigned char buffer[200];
        auto rd = read(STDIN_FILENO, &buffer, 1);

        // GAME LOGIC

        for (int i = 0; i < rd; ++i) {
            UserInput(buffer[i]);
        }

        // Make sure Rotation does not go OOB
        nCurrentRotation %= 4;

        if (bForceDown) {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentPosX, nCurrentPosY + 1)) {
                nCurrentPosY += 1; // We can move it down
            } else {
                // Lock the current piece in the field
                for (int px = 0; px < PIECE_LENGTH; ++px) {
                    for (int py = 0; py < PIECE_LENGTH; ++py) {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == CHAR_X) {
                            pField[FIELD_WIDTH * (nCurrentPosY + py) + (nCurrentPosX + px)] = nCurrentPiece + 1;
                        }
                    }
                }

                // Increase the difficulty based on piece count
                nPieceCounter++;
                if (nPieceCounter % 10 == 0) {
                    if (nSpeed >= 10) {
                        // Speed is how much time in a game tick: smaller values mean faster game
                        nSpeed--;
                    }
                }

                // Check if we have got any full lines
                for (int py = 0; py < FIELD_HEIGHT; ++py) {
                    if (nCurrentPosY + py < FIELD_HEIGHT - 1) {
                        bool bLineFull = true;

                        // If there are empty spaces, it cannot be a full line
                        for (int px = 1; px < FIELD_WIDTH - 1; ++px) {
                            bLineFull &= pField[(nCurrentPosY + py) * FIELD_WIDTH + px] != CHAR_INDEX_EMPTY;
                        }

                        // We have a full line!
                        if (bLineFull) {
                            for (int px = 1; px < FIELD_WIDTH - 1; ++px) {
                                pField[(nCurrentPosY + py) * FIELD_WIDTH + px] = CHAR_INDEX_LINE_CLEAR;
                            }

                            vLines.push_back(nCurrentPosY + py);

                        }
                    }
                }

                // Increase the score
                nScore += 25; // Landing each piece nets 25 points

                if (!vLines.empty()) {
                    nScore += (1 << vLines.size()) * 100; // Clearing lines nets exponential points
                }

                // Choose the next piece
                nCurrentPosX = FIELD_WIDTH / 2;
                nCurrentPosY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = tetromino_dist(rng);

                // If the next piece does not piece, game over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentPosX, nCurrentPosY);
            }

            nSpeedCounter = 0;
        }

        // RENDER OUTPUT
        DrawGameToScreen(&vLines, nScore);

    }

    // GAME OVER
    swprintf(&screen[1 * SCREEN_WIDTH + FIELD_WIDTH + 6], 16, L"GAME OVER");
    DrawGameToScreen(&vLines, nScore);
    cout << endl << "Press Enter to exit...";
    cin.get();
}

int main() {

    // Create assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"....");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"..X.");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L".X..");
    tetromino[5].append(L".X..");

    tetromino[6].append(L"..X.");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"....");

    // Initialize the field array
    pField = new unsigned char[FIELD_WIDTH * FIELD_HEIGHT];
    for (int x = 0; x < FIELD_WIDTH; ++x) {
        for (int y = 0; y < FIELD_HEIGHT; ++y) {
            pField[x + y * FIELD_WIDTH] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? CHAR_INDEX_BORDER : CHAR_INDEX_EMPTY;
        }
    }

    // Initialize screen
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) { screen[i] = L' '; }

    // Disable Enter
    input_enter_off();

    // Enter the game
    GameLoop();

    // Enable enter
    input_enter_on();


    return 0;
}
