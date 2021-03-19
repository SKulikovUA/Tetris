#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <memory>
#include <future>
#include <chrono>
#include <cassert>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

#include "CTetris.h"
#include "CResourceManager.h"
#include "resources_id.h"

const int blockSize = 40;

std::shared_ptr<sf::RenderWindow> mWindow;

void loadResources()
{
    using namespace game;

    CResourceManager& manager = CResourceManager::getInstance();
    CTetris& game = CTetris::getInstance();

    std::shared_ptr<sf::Font> fontRes = manager.addResource<sf::Font>(ID_GAME_FONT);
    fontRes->loadFromFile("images/font.ttf");

    std::shared_ptr<sf::Texture> texRes = manager.addResource<sf::Texture>(ID_BACKGROUND_TEXTURE);
    texRes->loadFromFile("images/back.jpg");

    std::shared_ptr<sf::Sprite> sprite = manager.addResource<sf::Sprite>(ID_BACKGROUND_SPRITE);
    sprite->setTexture(*texRes.get());
    sprite->scale(sf::Vector2f(0.42f, 1.0f));

    texRes = manager.addResource<sf::Texture>(ID_SPRITE_TEXTURE);
    texRes->loadFromFile("images/blocks.png");
    sprite = manager.addResource<sf::Sprite>(ID_GAME_SPRITE);
    sprite->setTexture(*texRes.get());

    std::shared_ptr<sf::Text> text = manager.addResource<sf::Text>(game::SCORE_LABEL);
    text->setFont(*manager.getResourceFast<sf::Font>(ID_GAME_FONT));
    text->setString("Score:");
    sf::Vector2f labelsPos(static_cast<float>(game.getFieldWidth() * blockSize), 0.0f); 
    text->setPosition(labelsPos);

    int stringOffset = manager.getResourceFast<sf::Text>(game::SCORE_LABEL)->getCharacterSize();

    text = manager.addResource<sf::Text>(SCORES);
    text->setFont(*manager.getResourceFast<sf::Font>(game::ID_GAME_FONT));
    text->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset)));
    text->setString("12345");

    text = manager.addResource<sf::Text>(NEXT_FIGURE_LABEL);
    text->setFont(*manager.getResourceFast<sf::Font>(ID_GAME_FONT));
    text->setString("Next");
    text->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset * 3)));

    text = manager.addResource<sf::Text>(NEW_GAME_LABEL);
    text->setFont(*manager.getResourceFast<sf::Font>(ID_GAME_FONT));
    text->setString("  Press 'N' key \nto start new game");
    text->setPosition(sf::Vector2f(50.0f, 200.0f));
    text->setFillColor(sf::Color::Yellow);
    text->setCharacterSize(40);
    text->setStyle(sf::Text::Bold);

    text = manager.addResource<sf::Text>(GAME_OVER_LABEL);
    text->setFont(*manager.getResourceFast<sf::Font>(ID_GAME_FONT));
    text->setString("  Game over. \nPress 'R' key to\nstart new game");
    text->setPosition(sf::Vector2f(50.0f, 200.0f));
    text->setFillColor(sf::Color::Red);
    text->setCharacterSize(40);
    text->setStyle(sf::Text::Bold);

    text = manager.addResource<sf::Text>(PAUSE_LABEL);
    text->setFont(*manager.getResourceFast<sf::Font>(ID_GAME_FONT));
    text->setString("   Pause\nPress 'P' key\nto continue");
    text->setPosition(sf::Vector2f(50.0f, 200.0f));
    text->setFillColor(sf::Color::Blue);
    text->setCharacterSize(40);
    text->setStyle(sf::Text::Bold);
}

void render()
{
    game::CTetris& theGame = game::CTetris::getInstance();
    auto fieldWidth = theGame.getFieldWidth();

    auto drawField = [&theGame](sf::RenderWindow* window) {
        std::shared_ptr<sf::Sprite> sprite = game::CResourceManager::getInstance().getResource<sf::Sprite>(game::ID_GAME_SPRITE);
        auto field = theGame.getField();
        for (int i = 0; i < field.size(); ++i)
        {
            for (int j = 0; j < field[0].size(); ++j)
            {
                if (field[i][j] == 0)
                {
                    continue;
                }
                sprite->setTextureRect(sf::IntRect(field[i][j] * blockSize, 0, blockSize, blockSize));
                sprite->setPosition(static_cast<float>(j * blockSize), static_cast<float>(i * blockSize));
                window->draw(*sprite);
            }
        }
    };

    game::CResourceManager& manager = game::CResourceManager::getInstance();
    mWindow->clear(sf::Color(0, 0, 30));
    mWindow->draw(*manager.getResourceFast<sf::Sprite>(game::ID_BACKGROUND_SPRITE));
    drawField(mWindow.get());

    const game::Point *figure = theGame.getCurrentFigure();
    const game::Point *nextFigure = theGame.getNextFigure();
    const int color = theGame.getFigureColor();

    if (theGame.getGameState() == game::EGameState::STATE_INGAME ||
        theGame.getGameState() == game::EGameState::STATE_PAUSE)
    {
        std::shared_ptr<sf::Sprite> sprite = manager.getResource<sf::Sprite>(game::ID_GAME_SPRITE);
        for (int i = 0; i < 4; ++i)
        {
            sprite->setTextureRect(sf::IntRect(color * blockSize, 0, blockSize, blockSize));
            sprite->setPosition(static_cast<float>(figure[i].x * blockSize), static_cast<float>(figure[i].y * blockSize));
            mWindow->draw(*sprite.get());

            sprite->setTextureRect(sf::IntRect(1, 0, blockSize, blockSize));
            sprite->setPosition(
                    static_cast<float>(nextFigure[i].x * blockSize) + static_cast<float>(fieldWidth * blockSize + 40),
                    static_cast<float>(nextFigure[i].y * blockSize) + 100);
            mWindow->draw(*sprite.get());
        }
    }

    switch(theGame.getGameState())
    {
        case game::EGameState::STATE_MAIN_MENU:
            mWindow->draw(*manager.getResourceFast<sf::Text>(game::NEW_GAME_LABEL));
            break;

        case game::EGameState::STATE_GAMEOVER:
            //mWindow->draw(*manager.getResourceFast<sf::Text>(game::GAME_OVER_LABEL));
            break;

        case game::EGameState::STATE_PAUSE:
            //mWindow->draw(*manager.getResourceFast<sf::Text>(game::PAUSE_LABEL));
            break;
            
        default:
            break;
    }
            
    mWindow->draw(*manager.getResourceFast<sf::Text>(game::SCORE_LABEL));
    mWindow->draw(*manager.getResourceFast<sf::Text>(game::SCORES));
    mWindow->draw(*manager.getResourceFast<sf::Text>(game::NEXT_FIGURE_LABEL));

    mWindow->display();
}

int main(int argv, char* argc[])
{
    #ifdef __linux__
    XInitThreads();
    #endif

    srand(0);
    using namespace sf;

    loadResources();

    game::CTetris& tetris = game::CTetris::getInstance();

    const game::TFieldType& field = tetris.getField();

    VideoMode mode(
        static_cast<int>(tetris.getFieldWidth()) * blockSize + 150, 
        static_cast<int>(tetris.getFieldHeight()) * blockSize);

    mWindow = std::make_shared<RenderWindow>(mode, "Tetris", sf::Style::Close);
    mWindow->setVerticalSyncEnabled(true);

    std::shared_ptr<sf::Font> font = game::CResourceManager::getInstance().getResource<sf::Font>(game::ID_GAME_FONT);

    int fieldWidth = tetris.getFieldWidth();
    
    sf::Clock clock;
    
    auto update = [&tetris, &clock]()
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        tetris.update(time);
        std::string scString = std::to_string(tetris.getScores());
        game::CResourceManager::getInstance().getResource<sf::Text>(game::SCORES)->setString(scString);
    };

    while (mWindow->isOpen())
    {
        Event e;
        while (mWindow->pollEvent(e))
        {
            if (e.type == Event::Closed)
            {
                mWindow->close();
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
        update();
        render();
    }
    
    return 0;
}
