#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <thread>

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

void renderThreadFunc(sf::RenderWindow* window, 
                      sf::Sprite* backGround, 
                      sf::Sprite* figureSprite,
                      const TLabelsMap& labelsMap,
                      game::CTetris* theGame)
{
    auto fieldWidth = theGame->getFieldWidth();

    auto drawField = [window, figureSprite, theGame]() {
        auto field = theGame->getField();
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

    window->setActive(true);
    while (window->isOpen())
    {
        window->clear(sf::Color(0, 0, 30));
        window->draw(*backGround);
        drawField();

        const game::Point *figure = theGame->getCurrentFigure();
        const game::Point *nextFigure = theGame->getNextFigure();
        const int color = theGame->getFigureColor();

        if (theGame->getGameState() == game::EGameState::STATE_INGAME || 
            theGame->getGameState() == game::EGameState::STATE_PAUSE)
        {
            for (int i = 0; i < 4; ++i)
            {
                figureSprite->setTextureRect(sf::IntRect(color * blockSize, 0, blockSize, blockSize));
                figureSprite->setPosition(static_cast<float>(figure[i].x * blockSize), static_cast<float>(figure[i].y * blockSize));
                window->draw(*figureSprite);

                figureSprite->setTextureRect(sf::IntRect(1, 0, blockSize, blockSize));
                figureSprite->setPosition(
                    static_cast<float>(nextFigure[i].x * blockSize) + static_cast<float>(fieldWidth * blockSize + 40),
                    static_cast<float>(nextFigure[i].y * blockSize) + 100);
                window->draw(*figureSprite);
            }
        }

        switch(theGame->getGameState())
        {
            case game::EGameState::STATE_MAIN_MENU:
                window->draw(*labelsMap.at(ELabelType::NEW_GAME_LABEL));
                break;

            case game::EGameState::STATE_GAMEOVER:
                window->draw(*labelsMap.at(ELabelType::GAME_OVER_LABEL));
                break;

            case game::EGameState::STATE_PAUSE:
                window->draw(*labelsMap.at(ELabelType::PAUSE_LABEL));
                break;
        }

        window->draw(*labelsMap.at(ELabelType::SCORE_LABEL));
        window->draw(*labelsMap.at(ELabelType::SCORES));
        window->draw(*labelsMap.at(ELabelType::NEXT_FIGURE_LABEL));

        window->display();
    }
    window->setActive(false);
}

int main(int argv, char* argc[])
{
    srand(0);
    using namespace sf;

    game::CTetris tetris;
    const game::TFieldType& field = tetris.getField();

    VideoMode mode(
        static_cast<int>(field[0].size()) * blockSize + 150, 
        static_cast<int>(field.size()) * blockSize);
    RenderWindow window(mode, "Tetris", sf::Style::Close);

    Texture t;
    t.loadFromFile("images/blocks.png");

    Texture back;
    back.loadFromFile("images/back.jpg");

    Sprite s(t);
    Sprite b(back);
    b.scale(sf::Vector2f(0.42f, 1.0f));

    sf::Font font;
    font.loadFromFile("images/font.ttf");

    int fieldWidth = tetris.getFieldWidth();
    TLabelsMap labelsMap;
    labelsMap.emplace(ELabelType::SCORE_LABEL, std::make_unique<sf::Text>());
    labelsMap[ELabelType::SCORE_LABEL]->setFont(font);
    labelsMap[ELabelType::SCORE_LABEL]->setString("Score:");
    sf::Vector2f labelsPos(static_cast<float>(fieldWidth * blockSize), 0.0f); 
    labelsMap[ELabelType::SCORE_LABEL]->setPosition(labelsPos);

    int stringOffset = labelsMap[ELabelType::SCORE_LABEL]->getCharacterSize();

    labelsMap.emplace(ELabelType::SCORES, std::make_unique<sf::Text>());
    labelsMap[ELabelType::SCORES]->setFont(font);
    labelsMap[ELabelType::SCORES]->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset)));

    labelsMap.emplace(ELabelType::NEXT_FIGURE_LABEL, std::make_unique<sf::Text>());
    labelsMap[ELabelType::NEXT_FIGURE_LABEL]->setFont(font);
    labelsMap[ELabelType::NEXT_FIGURE_LABEL]->setString("Next");
    labelsMap[ELabelType::NEXT_FIGURE_LABEL]->setPosition(sf::Vector2f(labelsPos.x, static_cast<float>(stringOffset * 3)));

    labelsMap.emplace(ELabelType::NEW_GAME_LABEL, std::make_unique<sf::Text>());
    labelsMap[ELabelType::NEW_GAME_LABEL]->setFont(font);
    labelsMap[ELabelType::NEW_GAME_LABEL]->setString("  Press 'N' key \nto start new game");
    labelsMap[ELabelType::NEW_GAME_LABEL]->setPosition(sf::Vector2f(50.0f, 200.0f));
    labelsMap[ELabelType::NEW_GAME_LABEL]->setFillColor(sf::Color::Yellow);
    labelsMap[ELabelType::NEW_GAME_LABEL]->setCharacterSize(40);
    labelsMap[ELabelType::NEW_GAME_LABEL]->setStyle(sf::Text::Bold);

    labelsMap.emplace(ELabelType::GAME_OVER_LABEL, std::make_unique<sf::Text>());
    labelsMap[ELabelType::GAME_OVER_LABEL]->setFont(font);
    labelsMap[ELabelType::GAME_OVER_LABEL]->setString("  Game over. \nPress 'R' key to\nstart new game");
    labelsMap[ELabelType::GAME_OVER_LABEL]->setPosition(sf::Vector2f(50.0f, 200.0f));
    labelsMap[ELabelType::GAME_OVER_LABEL]->setFillColor(sf::Color::Red);
    labelsMap[ELabelType::GAME_OVER_LABEL]->setCharacterSize(40);
    labelsMap[ELabelType::GAME_OVER_LABEL]->setStyle(sf::Text::Bold);

    labelsMap.emplace(ELabelType::PAUSE_LABEL, std::make_unique<sf::Text>());
    labelsMap[ELabelType::PAUSE_LABEL]->setFont(font);
    labelsMap[ELabelType::PAUSE_LABEL]->setString("   Pause\nPress 'P' key\nto continue");
    labelsMap[ELabelType::PAUSE_LABEL]->setPosition(sf::Vector2f(50.0f, 200.0f));
    labelsMap[ELabelType::PAUSE_LABEL]->setFillColor(sf::Color::Blue);
    labelsMap[ELabelType::PAUSE_LABEL]->setCharacterSize(40);
    labelsMap[ELabelType::PAUSE_LABEL]->setStyle(sf::Text::Bold);

    Clock clock;

    window.setActive(false);
    std::thread th(renderThreadFunc, &window, &b, &s, std::cref(labelsMap), &tetris);
    th.detach();

    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();

        Event e;
        std::string scString;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
            {
                window.close();
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

        tetris.update(time);
        scString = std::to_string(tetris.getScores());
        labelsMap[ELabelType::SCORES]->setString(scString);
    }
    return 0;
}