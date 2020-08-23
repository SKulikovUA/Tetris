#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

#include "CTetris.h"

const int blockSize = 40;

enum class ELabelType
{
    SCORE_LABEL,
    SCORES,
    NEXT_FIGURE_LABEL,
    NEW_GAME_LABEL,
    PAUSE_LABEL,
    GAME_OVER_LABEL
};



using TLabelsMap = std::unordered_map<ELabelType, std::unique_ptr<sf::Text>>;
using TLabelsMapPtr = std::shared_ptr<TLabelsMap>;

std::mutex guard;

struct SSharedResources
{
    sf::RenderWindow* mWindow{ nullptr };
    std::shared_ptr<sf::Sprite> mBackGound;
    std::shared_ptr<sf::Sprite> mFigureSprite;
    std::shared_ptr<TLabelsMap> mLabelsMap;
    std::atomic<bool> mExitFlag{ false };
};

SSharedResources resources;

void renderThreadFunc()
{
    game::CTetris& theGame = game::CTetris::getInstance();
    auto fieldWidth = theGame.getFieldWidth();

    auto drawField = [&theGame](sf::RenderWindow* window, sf::Sprite* figureSprite) {
        auto field = theGame.getField();
        for (int i = 0; i < field.size(); ++i)
        {
            for (int j = 0; j < field[0].size(); ++j)
            {
                if (field[i][j] == 0)
                {
                    continue;
                }
                figureSprite->setTextureRect(sf::IntRect(field[i][j] * blockSize, 0, blockSize, blockSize));
                figureSprite->setPosition(static_cast<float>(j * blockSize), static_cast<float>(i * blockSize));
                window->draw(*figureSprite);
            }
        }
    };
    
    resources.mWindow->setActive(true);
    while (!resources.mExitFlag)
    {
        resources.mWindow->clear(sf::Color(0, 0, 30));
        resources.mWindow->draw(*resources.mBackGound);
        drawField(resources.mWindow, resources.mFigureSprite.get());

        const game::Point *figure = theGame.getCurrentFigure();
        const game::Point *nextFigure = theGame.getNextFigure();
        const int color = theGame.getFigureColor();

        if (theGame.getGameState() == game::EGameState::STATE_INGAME || 
            theGame.getGameState() == game::EGameState::STATE_PAUSE)
        {
            for (int i = 0; i < 4; ++i)
            {
                resources.mFigureSprite->setTextureRect(sf::IntRect(color * blockSize, 0, blockSize, blockSize));
                resources.mFigureSprite->setPosition(static_cast<float>(figure[i].x * blockSize), static_cast<float>(figure[i].y * blockSize));
                resources.mWindow->draw(*resources.mFigureSprite.get());

                resources.mFigureSprite->setTextureRect(sf::IntRect(1, 0, blockSize, blockSize));
                resources.mFigureSprite->setPosition(
                    static_cast<float>(nextFigure[i].x * blockSize) + static_cast<float>(fieldWidth * blockSize + 40),
                    static_cast<float>(nextFigure[i].y * blockSize) + 100);
                resources.mWindow->draw(*resources.mFigureSprite.get());
            }
        }

        switch(theGame.getGameState())
        {
            case game::EGameState::STATE_MAIN_MENU:
                resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL));
                break;

            case game::EGameState::STATE_GAMEOVER:
                resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL));
                break;

            case game::EGameState::STATE_PAUSE:
                resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::PAUSE_LABEL));
                break;
        }
            
        resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::SCORE_LABEL));
        resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::SCORES));
        resources.mWindow->draw(*resources.mLabelsMap->at(ELabelType::NEXT_FIGURE_LABEL));

        resources.mWindow->display();
    }
}

void updateThreadFunc()
{
    sf::Clock clock;
    
    game::CTetris& tetris = game::CTetris::getInstance();
    
    while(!resources.mExitFlag)
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();

        tetris.update(time);
        std::string scString = std::to_string(tetris.getScores());
        resources.mLabelsMap->at(ELabelType::SCORES)->setString(scString);
    }
}

int main(int argv, char* argc[])
{
    #ifdef __linux__
    XInitThreads();
    #endif

    srand(0);
    using namespace sf;

    game::CTetris& tetris = game::CTetris::getInstance();

    const game::TFieldType& field = tetris.getField();

    VideoMode mode(
        static_cast<int>(tetris.getFieldWidth()) * blockSize + 150, 
        static_cast<int>(tetris.getFieldHeight()) * blockSize);
    RenderWindow renderWindow(mode, "Tetris", sf::Style::Close);
    resources.mWindow = &renderWindow;

    Texture t;
    t.loadFromFile("images/blocks.png");

    Texture back;
    back.loadFromFile("images/back.jpg");

    resources.mFigureSprite = std::make_shared<sf::Sprite>(t);

    resources.mBackGound = std::make_shared<sf::Sprite>(back);
    resources.mBackGound->scale(sf::Vector2f(0.42f, 1.0f));

    std::unique_ptr<sf::Font> font = std::make_unique<sf::Font>();
    font->loadFromFile("images/font.ttf");

    int fieldWidth = tetris.getFieldWidth();

    resources.mLabelsMap = std::make_shared<TLabelsMap>();

    resources.mLabelsMap->emplace(ELabelType::SCORE_LABEL, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::SCORE_LABEL)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::SCORE_LABEL)->setString("Score:");
    sf::Vector2f labelsPos(static_cast<float>(fieldWidth * blockSize), 0.0f); 
    resources.mLabelsMap->at(ELabelType::SCORE_LABEL)->setPosition(labelsPos);

    int stringOffset = resources.mLabelsMap->at(ELabelType::SCORE_LABEL)->getCharacterSize();

    resources.mLabelsMap->emplace(ELabelType::SCORES, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::SCORES)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::SCORES)->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset)));
    resources.mLabelsMap->at(ELabelType::SCORES)->setString("12345");

    resources.mLabelsMap->emplace(ELabelType::NEXT_FIGURE_LABEL, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::NEXT_FIGURE_LABEL)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::NEXT_FIGURE_LABEL)->setString("Next");
    resources.mLabelsMap->at(ELabelType::NEXT_FIGURE_LABEL)->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset * 3)));

    resources.mLabelsMap->emplace(ELabelType::NEW_GAME_LABEL, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setString("  Press 'N' key \nto start new game");
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setPosition(sf::Vector2f(50.0f, 200.0f));
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setFillColor(sf::Color::Yellow);
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setCharacterSize(40);
    resources.mLabelsMap->at(ELabelType::NEW_GAME_LABEL)->setStyle(sf::Text::Bold);

    resources.mLabelsMap->emplace(ELabelType::GAME_OVER_LABEL, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setString("  Game over. \nPress 'R' key to\nstart new game");
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setPosition(sf::Vector2f(50.0f, 200.0f));
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setFillColor(sf::Color::Red);
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setCharacterSize(40);
    resources.mLabelsMap->at(ELabelType::GAME_OVER_LABEL)->setStyle(sf::Text::Bold);

    resources.mLabelsMap->emplace(ELabelType::PAUSE_LABEL, std::make_unique<sf::Text>());
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setFont(*font);
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setString("   Pause\nPress 'P' key\nto continue");
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setPosition(sf::Vector2f(50.0f, 200.0f));
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setFillColor(sf::Color::Blue);
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setCharacterSize(40);
    resources.mLabelsMap->at(ELabelType::PAUSE_LABEL)->setStyle(sf::Text::Bold);

    std::thread updateTh(updateThreadFunc);
    updateTh.detach();

    resources.mWindow->setActive(false);
    std::thread renderTh(renderThreadFunc);
    renderTh.detach();

    while (!resources.mExitFlag)
    {
        std::lock_guard<std::mutex> lock(guard);
        Event e;
        while (resources.mWindow->pollEvent(e))
        {
            if (e.type == Event::Closed)
            {
                 resources.mExitFlag = true;
            }

            if (e.type == Event::KeyPressed)
            {
                switch (e.key.code)
                {
                case Keyboard::Up:
                    tetris.rotate();
                    break;

                case Keyboard::Left:
                    tetris.move(-1);
                    break;

                case Keyboard::Right:
                    tetris.move(1);
                    break;

                case Keyboard::Down:
                    tetris.drop();
                    break;

                case Keyboard::N:
                    tetris.setGameState(game::EGameState::STATE_INGAME);
                    break;

                case Keyboard::P:
                    tetris.setGamePause();
                    break;
                    
                case Keyboard::R:
                    tetris.resetGame();
                    break;
                }
            }
        }
        if(resources.mExitFlag)
        {
            resources.mWindow->close();
            break;
        }
    }

    return 0;
}