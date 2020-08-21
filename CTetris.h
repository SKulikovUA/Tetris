#pragma once
#include <vector>
#include <memory>

namespace game
{

using TFieldType = std::vector<std::vector<int>>;

enum class EGameState
{
    STATE_MAIN_MENU,
    STATE_PAUSE,
    STATE_INGAME,
    STATE_GAMEOVER
};

struct Point
{
    int x;
    int y;
};

class CTetris
{
public: 
    CTetris();
    CTetris(const int fieldWidth, const int fieldHeight);
    ~CTetris() = default;
    const TFieldType& getField() const;
    void move(int deltaX);
    void rotate();
    void drop();
    void update(float dt);
    const Point* getCurrentFigure() const;
    const Point* getNextFigure() const;
    const int getFigureColor() const;
    const int getScores() const;
    const int getFieldWidth() const;
    const int getFieldHeight() const;
    void setGameState(EGameState state);
    const EGameState getGameState() const;
    void setGamePause();
    void resetGame();

    CTetris(const CTetris& other) = delete;
    CTetris& operator=(const CTetris& other) = delete;

private:
    void initField();
    void resetField();
    void spawnFigure();
    bool isCollided();
    void scanLines();

private:
    int mFieldWidth;
    int mFieldHeight;
    int mCurrentFigure;
    int mNextFigure;
    TFieldType mField;
    float mTime;
    int mFigureColor;
    float mCurrentSpeed;
    int mCurrentNumLines;
    int mScores;
    EGameState mGameState;

    static const int mDefaultFieldWidth = 10;
    static const int mDefaultFieldHeight = 20;
    const float mDefaultSpeed = 0.3f;
    const float mDropDefaultSpeed = 0.01f;

    Point mA[4];
    Point mB[4];
    Point mNext[4];

    int mFigures[7][4] = {
        1, 3, 5, 7,
        2, 4, 5, 7,
        3, 5, 4, 6,
        3, 5, 4, 7,
        2, 3, 5, 7,
        3, 5, 7, 6,
        2, 3, 4, 5
    };
};

}
