#include "CTetris.h"
#include <iostream>
#include <assert.h>

namespace game
{
    CTetris::CTetris()
    : mFieldWidth(mDefaultFieldWidth)
    , mFieldHeight(mDefaultFieldHeight)
    , mCurrentFigure(-1)
    , mNextFigure(-1)
    , mTime(0.)
    , mFigureColor(-1)
    , mCurrentSpeed(mDefaultSpeed)
    , mCurrentNumLines(0)
    , mScores(0)
    , mGameState(EGameState::STATE_MAIN_MENU)
    {
        initField();
        resetField();
        spawnFigure();
    }

    void CTetris::initField()
    {
        mField.resize(mFieldHeight);
        for(int i = 0; i < mFieldHeight; ++i)
        {
            mField[i].resize(mFieldWidth);
        }
    };

    void CTetris::resetField()
    {
        for(int i = 0; i < mFieldHeight; ++i)
        {
            for(int j = 0; j < mFieldWidth; ++j)
            {
                mField[i][j] = 0;
            }
        }
    }

    void CTetris::spawnFigure()
    {
        if (mNextFigure == -1)
        {
            mNextFigure = rand() % 7;
            mCurrentFigure = rand() % 7;
        }
        else
        {
            mCurrentFigure = mNextFigure;
            mNextFigure = rand() % 7;
        }

        mFigureColor = 1 + rand() % 7;
        for (int i = 0; i < 4; i++)
        {
            mA[i].x = (mFigures[mCurrentFigure][i] % 2) + mFieldWidth / 2;
            mA[i].y = (mFigures[mCurrentFigure][i] / 2) - 3;

            mNext[i].x = mFigures[mNextFigure][i] % 2;
            mNext[i].y = mFigures[mNextFigure][i] / 2;
        }
    }

    bool CTetris::isCollided()
    {
        bool result = false;
        for (int i = 0; i < 4; ++i)
        {
            if (mA[i].x < 0 || mA[i].x >= mFieldWidth || mA[i].y >= mFieldHeight)
            {
                result = true;
                break;
            }
            else if (mA[i].y >= 0 && mField[mA[i].y][mA[i].x])
            {
                result = true;
                break;
            }
        }
        return result;
    }

    const TFieldType& CTetris::getField() const
    {
        return mField;
    }

    void CTetris::move(int deltaX)
    {
        std::lock_guard<std::mutex> lock(mGuard);
        for (int i = 0; i < 4; ++i)
        {
            mB[i] = mA[i];
            mA[i].x += deltaX;
        }
        if (isCollided())
        {
            for (int i = 0; i < 4; ++i)
            {
                mA[i] = mB[i];
            }
        }
    }

    void CTetris::rotate()
    {
        std::lock_guard<std::mutex> lock(mGuard);
        if (mCurrentFigure != 6) //Hard code for a 'O' figure because rotate a 'O' figure doesn't need
        {
            Point p = mA[1];
            for (int i = 0; i < 4; ++i)
            {
                mB[i] = mA[i];
                int x = mA[i].y - p.y;
                int y = mA[i].x - p.x;
                mA[i].x = p.x - x;
                mA[i].y = p.y + y;
            }
            if (isCollided())
            {
                for (int i = 0; i < 4; ++i)
                {
                    mA[i] = mB[i];
                }
            }
        }
    }

    void CTetris::drop()
    {
        std::lock_guard<std::mutex> lock(mGuard);
        mCurrentSpeed = mDropDefaultSpeed;
    }

    const Point* CTetris::getCurrentFigure() const
    {
        return &mA[0];
    }

    const int CTetris::getFigureColor() const
    {
        return mFigureColor;
    }

    void CTetris::update(float dt)
    {
        if(mGameState != EGameState::STATE_INGAME)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(mGuard);

        mTime += dt;
        if (mTime > mCurrentSpeed)
        {
            for (int i = 0; i < 4; ++i)
            {
                mB[i] = mA[i];
                mA[i].y += 1;
            }
            if (isCollided())
            {
                for (int i = 0; i < 4; ++i)
                {
                    mA[i] = mB[i];
                    if(mA[i].y < 0)
                    {
                        mGameState = EGameState::STATE_GAMEOVER;
                    }
                    else
                    {
                        mField[mA[i].y][mA[i].x] = mFigureColor;
                    } 
                }

                if(mGameState != EGameState::STATE_GAMEOVER)
                {
                    spawnFigure();
                    scanLines();
                }

                if(mCurrentNumLines != 0)
                {
                    switch(mCurrentNumLines)
                    {
                        case 1:
                            mScores += 100;
                            break;

                        case 2:
                            mScores += 250;
                            break;
                        
                        case 3:
                            mScores += 350;
                            break;

                        case 4:
                            mScores += 700;
                            break;
                    }
                    mCurrentNumLines = 0;
                }
            }
            mTime = 0.;
            mCurrentSpeed = mDefaultSpeed;
        }
    }

    void CTetris::scanLines()
    {
        int lines = mFieldHeight - 1;
        for (int i = mFieldHeight - 1; i > 0; --i)
        {
            int count = 0;
            for (int j = 0; j < mFieldWidth; j++)
            {
                if (mField[i][j])
                {
                    count++;
                }
                mField[lines][j] = mField[i][j];
            }
            if (count < mFieldWidth)
            {
                lines--;
            }
            else
            {
                ++mCurrentNumLines;
            }
        }
    }

    const int CTetris::getScores() const
    {
        return mScores;
    }

    const Point* CTetris::getNextFigure() const
    {
        return &mNext[0];
    }

    const int CTetris::getFieldWidth() const
    {
        return mFieldWidth;
    }

    const int CTetris::getFieldHeight() const
    {
        return mFieldHeight;
    }

    void CTetris::setGameState(EGameState state)
    {
        mGameState = state;
    }

    const EGameState CTetris::getGameState() const
    {
        return mGameState;
    }

    void CTetris::setGamePause()
    {
        if(mGameState == EGameState::STATE_INGAME)
        {
            mGameState = EGameState::STATE_PAUSE;
        }
        else if(mGameState == EGameState::STATE_PAUSE)
        {
            mGameState = EGameState::STATE_INGAME;
        }
    }

    void CTetris::resetGame()
    {
        if(mGameState == EGameState::STATE_INGAME || mGameState == EGameState::STATE_GAMEOVER)
        {
            resetField();
            spawnFigure();
            mScores = 0;
            mGameState = EGameState::STATE_INGAME;
        }
    }
}