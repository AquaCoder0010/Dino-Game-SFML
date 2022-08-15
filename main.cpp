#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <random>

const unsigned int windowSize_x = 1000;
const unsigned int windowSize_y = 500;
const unsigned int groundOffset = windowSize_y - 150.f;
int gameSpeed = 8;
bool playerDead = false;
bool playDeadSound = false;

struct Fps_s
{
    sf::Font font;
    sf::Text text;
    sf::Clock clock;
    int Frame;
    int fps;
};
class Fps
{
    Fps_s fps;
    public:
        Fps()
        :fps()
        {
            if(fps.font.loadFromFile("rsrc/Fonts/font.ttf"))
            {
                fps.text.setFont(fps.font);
            }
            fps.text.setCharacterSize(15);
            fps.text.setPosition(sf::Vector2f(fps.text.getCharacterSize() + 10.f, fps.text.getCharacterSize()));
            fps.text.setFillColor(sf::Color(83, 83, 83));
        }
        void update()
        {
            if(fps.clock.getElapsedTime().asSeconds() >= 1.f)
            {
                fps.fps = fps.Frame; fps.Frame = 0; fps.clock.restart();
            }
            fps.Frame++;
            fps.text.setString("FPS :- " + std::to_string(fps.fps));
        }
        void drawTo(sf::RenderWindow& window)
        {
            window.draw(fps.text);
        }

};


class SoundManager
{
    public:
        sf::SoundBuffer dieBuffer;
        sf::SoundBuffer jumpBuffer;
        sf::SoundBuffer pointBuffer;
        sf::Sound dieSound;
        sf::Sound jumpSound;
        sf::Sound pointSound;

        SoundManager()
        :dieBuffer(), jumpBuffer(), pointBuffer(), dieSound(), jumpSound(), pointSound()
        {
            dieBuffer.loadFromFile("rsrc/Sounds/die.wav");
            jumpBuffer.loadFromFile("rsrc/Sounds/jump.wav");
            pointBuffer.loadFromFile("rsrc/Sounds/point.wav");
            
            dieSound.setBuffer(dieBuffer);
            jumpSound.setBuffer(jumpBuffer);
            pointSound.setBuffer(pointBuffer);
        }
};

class Ground
{
    public:
    sf::Sprite groundSprite;
    sf::Texture groundTexture;
    int offset{0};
    Ground()
    :groundSprite(), groundTexture()
    {
        if(groundTexture.loadFromFile("rsrc/Images/GroundImage.png"))
        {
            groundSprite.setTexture(groundTexture);
            groundSprite.setPosition(sf::Vector2f(0.f, windowSize_y - groundTexture.getSize().y - 50));
        }
    }   

    void updateGround()
    {
        if(playerDead == false)
        {
            if(offset > groundTexture.getSize().x - windowSize_x)
                offset = 0;

            offset += gameSpeed;
            groundSprite.setTextureRect(sf::IntRect(offset, 0, windowSize_x, windowSize_y));
        }

        if(playerDead == true)
            groundSprite.setTextureRect(sf::IntRect(offset, 0, windowSize_x, windowSize_y));
        
    }
    void reset()
    {
        offset = 0;
        groundSprite.setTextureRect(sf::IntRect(0, 0, windowSize_x, windowSize_y));
    }

};


class Obstacle
{
    public:
        sf::Sprite obstacleSprite;
        sf::FloatRect obstacleBounds{0.f, 0.f, 0.f, 0.f};
        Obstacle(sf::Texture& texture)
        :obstacleSprite(), obstacleBounds()
        {
            obstacleSprite.setTexture(texture);
            obstacleSprite.setPosition(sf::Vector2f(windowSize_x, groundOffset));
        }
};

class Obstacles
{
    public:
        std::vector<Obstacle> obstacles;
        
        sf::Time spawnTimer;
        sf::Texture obstacleTexture_1;
        sf::Texture obstacleTexture_2;
        sf::Texture obstacleTexture_3; 
        int randomNumber{0};

        Obstacles()
        :spawnTimer(sf::Time::Zero)
        {
            obstacles.reserve(5);
            
            if(obstacleTexture_1.loadFromFile("rsrc/Images/Cactus1.png"))
            {
                std::cout << "loaded cactus Image 1 " << std::endl;
            }

            if(obstacleTexture_2.loadFromFile("rsrc/Images/Cactus2.png"))
            {
                std::cout << "Loaded cactus Image 2" << std::endl;
            }

            if(obstacleTexture_3.loadFromFile("rsrc/Images/Cactus3.png"))
            {
                std::cout << "Loaded cactus Image 3" << std::endl;

            }

        }

        void update(sf::Time& deltaTime)
        {
            spawnTimer += deltaTime;
            if(spawnTimer.asSeconds() > 0.5f + gameSpeed/8)
            {
                randomNumber = (rand() % 3) + 1;
                if(randomNumber == 1)
                {
                    obstacles.emplace_back(Obstacle(obstacleTexture_1));
                }
                if(randomNumber == 2)
                {
                    obstacles.emplace_back(Obstacle(obstacleTexture_2));
                }
                if(randomNumber == 3)
                {
                    obstacles.emplace_back(Obstacle(obstacleTexture_2));
                }
                spawnTimer = sf::Time::Zero;
            }

            if(playerDead == false)
            {
                for(int i = 0; i < obstacles.size(); i++)
                {
                    obstacles[i].obstacleBounds = obstacles[i].obstacleSprite.getGlobalBounds();
                    obstacles[i].obstacleBounds.width -= 10.f;
                    obstacles[i].obstacleSprite.move(-1*gameSpeed, 0.f);
                    if(obstacles[i].obstacleSprite.getPosition().x < -150.f)
                    {
                        std::vector<Obstacle>::iterator obstacleIter = obstacles.begin() + i;
                        obstacles.erase(obstacleIter);
                    }
                }
            }

            if(playerDead == true)
            {
                for(auto& obstacles : obstacles)
                    {
                        obstacles.obstacleSprite.move(0.f, 0.f); 
                    }
            }
        
        }

        void drawTo(sf::RenderWindow& window)
        {
            for(auto& obstacles : obstacles)
            {
                window.draw(obstacles.obstacleSprite);
            }
        }

        void reset()
        {
            obstacles.erase(obstacles.begin(), obstacles.end());
        }
};



class Dino
{
    public:
        sf::Sprite dino;
        sf::Vector2f dinoPos{0.f, 0.f};
        sf::Vector2f dinoMotion{0.f, 0.f};
        sf::Texture dinoTex;
        sf::FloatRect dinoBounds;
        SoundManager soundManager;
        std::array<sf::IntRect, 6> frames;
        sf::Time timeTracker;
        int animationCounter{0};
        
        Dino()
        :dino(), dinoTex(), soundManager(), timeTracker()
        {
            if(dinoTex.loadFromFile("rsrc/Images/PlayerSpriteSheet.png"))
            {
                dino.setTexture(dinoTex);
                for(int i = 0; i < frames.size(); i++)
                    frames[i] = sf::IntRect(i * 90, 0, 90, 95);
                dino.setTextureRect(frames[0]);
                dinoPos = dino.getPosition();
            }
            else
            {
                std::cout << "Error loading the PlayerSprite texture" << std::endl;
            }
        }

        void update(sf::Time& deltaTime, std::vector<Obstacle>& obstacles)
        {
            dinoPos = dino.getPosition();
            dinoBounds = dino.getGlobalBounds();
            dinoBounds.height -= 15.f; dinoBounds.width -= 10.f;
            timeTracker += deltaTime;
            for(auto& obstacles: obstacles)
            {
                if(dinoBounds.intersects(obstacles.obstacleBounds))
                {
                    playerDead = true;
                }
            }

            

            if(!playerDead)
            {
                walk();
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == true && dinoPos.y >= windowSize_y - 150.f)
                {
                    animationCounter = 0;
                    dinoMotion.y = -20.f; dino.setTextureRect(frames[1]);
                    soundManager.jumpSound.play();
                }

                if(dinoPos.y < windowSize_y - 150.f)
                {
                    dinoMotion.y += 1.f; dino.setTextureRect(frames[1]);
                }

                if(dinoPos.y > windowSize_y - 150.f)
                {
                    dino.setPosition(sf::Vector2f(dino.getPosition().x, windowSize_y - 150.f));
                    dinoMotion.y = 0.f;

                }

                dino.move(dinoMotion);
            } 
            if(playerDead == true)
            {
                dinoMotion.y = 0.f;
                dino.setTextureRect(frames[3]);
                if(timeTracker.asMilliseconds() > 170.f)
                {
                    soundManager.dieSound.stop();
                    soundManager.dieSound.setLoop(false);
                    timeTracker = sf::Time::Zero;
                }
                else
                {
                    soundManager.dieSound.play();
                }
            }

        }

        void walk()
        {
            for(int i = 0; i < frames.size() - 3; i++)
                if(animationCounter == (i + 1) * 3)
                    dino.setTextureRect(frames[i]);

            if(animationCounter >= (frames.size() - 2) * 3)
                animationCounter = 0;

            animationCounter++;
        }
        void reset()
        {
            dinoMotion.y = 0; dino.setPosition(sf::Vector2f(dino.getPosition().x, windowSize_y - 150.f));
            dino.setTextureRect(frames[0]);
            
        }

}; 




class Scores
{
    public:
        sf::Text previousScoreText;
        sf::Text HIText;
        sf::Text scoresText;
        sf::Font scoresFont;
        SoundManager soundManager;
        short scores{0};
        short previousScore{0};
        short scoresIndex{0};
        short scoresDiff{0};
        short scoresInital;

        Scores()
        :scoresFont(), scoresText(), previousScoreText(), scoresInital(), soundManager()
        {
            if(scoresFont.loadFromFile("rsrc/Fonts/Font.ttf"))
            {
                scoresText.setFont(scoresFont);
                scoresText.setCharacterSize(15);
                scoresText.setPosition(sf::Vector2f(windowSize_x/2 + windowSize_x/4 + 185.f, scoresText.getCharacterSize() + 10.f));
                scoresText.setFillColor(sf::Color(83, 83, 83));
                
                previousScoreText.setFont(scoresFont);
                previousScoreText.setCharacterSize(15);
                previousScoreText.setPosition(sf::Vector2f(scoresText.getPosition().x - 100.f, scoresText.getPosition().y));
                previousScoreText.setFillColor(sf::Color(83, 83, 83));

                HIText.setFont(scoresFont);
                HIText.setCharacterSize(15);
                HIText.setPosition(sf::Vector2f(previousScoreText.getPosition().x - 50.f, previousScoreText.getPosition().y));
                HIText.setFillColor(sf::Color(83, 83, 83));
            }
            HIText.setString("HI");
            scoresInital = 0;
        }

        void update()
        {
            if(playerDead == false)
            {
                scoresIndex++;
                if(scoresIndex >= 5)
                {
                    scoresIndex = 0; 
                    scores++;
                }
                scoresDiff = scores - scoresInital;
                if(scoresDiff > 100)
                {
                    scoresInital += 100;
                    gameSpeed += 1;
                    soundManager.pointSound.play();
                }   

                scoresText.setString(std::to_string(scores));
                previousScoreText.setString(std::to_string(previousScore));
            }
            
        }

        void reset()
        {
            if(scores > previousScore)
                previousScore = scores;
            if(scores < previousScore)
                previousScore = previousScore;
            

            previousScoreText.setString(std::to_string(previousScore));
            scores = 0;
        }   

};

class RestartButton
{
    public:
        sf::Sprite restartButtonSprite;
        sf::FloatRect restartButtonSpriteBounds;
        sf::Texture restartButtonTexture;
        sf::Vector2f mousePos;
        bool checkPressed{false};

        RestartButton()
        :restartButtonSprite(), restartButtonTexture(), mousePos(0.f, 0.f), restartButtonSpriteBounds()
        {
            if(restartButtonTexture.loadFromFile("rsrc/Images/RestartButton.png"))
            {
                restartButtonSprite.setTexture(restartButtonTexture);
                restartButtonSprite.setPosition(sf::Vector2f(windowSize_x/2 - restartButtonTexture.getSize().x/2, windowSize_y/2));
                restartButtonSpriteBounds = restartButtonSprite.getGlobalBounds();
            }
        }
};



class Clouds
{
    public:
        std::vector<sf::Sprite> clouds;
        sf::Time currTime;
        sf::Texture cloudTexture;
        std::random_device dev;
        std::mt19937 rng{dev()};
        

        Clouds()
        :cloudTexture(), clouds(), currTime(), dev()
        {
            if(cloudTexture.loadFromFile("rsrc/Images/Clouds.png"))
            {
                std::cout << "Loaded CloudTexture" << std::endl;
            }
            clouds.reserve(4);
            clouds.emplace_back(sf::Sprite(cloudTexture));
            clouds.back().setPosition(sf::Vector2f(windowSize_x, windowSize_y/2 - 40.f));
        }

        void updateClouds(sf::Time& deltaTime)
        {
            
            currTime += deltaTime;
            if(currTime.asSeconds() > 8.f)
            {
                clouds.emplace_back(sf::Sprite(cloudTexture));

                std::uniform_int_distribution<std::mt19937::result_type> dist6( windowSize_y/2 - 200, windowSize_y/2 - 50);
                clouds.back().setPosition(sf::Vector2f(windowSize_x, dist6(rng)));
            
                currTime = sf::Time::Zero; 
            }
            

            for(int i = 0; i < clouds.size(); i++)
            {
                if(playerDead == false)
                    clouds[i].move(sf::Vector2f(-1.f, 0.f));
                if(playerDead == true)
                    clouds[i].move(sf::Vector2f(-0.5f, 0.f));

                if(clouds[i].getPosition().x < 0.f - cloudTexture.getSize().x)
                {
                    std::vector<sf::Sprite>::iterator cloudIter = clouds.begin() + i;
                    clouds.erase(cloudIter);
                }
            }
        }
    

        void drawTo(sf::RenderWindow& window)
        {
            for(auto& clouds: clouds)
            {
                window.draw(clouds);
            }
        }
    
};



class GameState
{
    public:
        Fps fps;
        Dino dino;
        Ground ground;
        Obstacles obstacles;
        Scores scores;
        Clouds clouds;
        RestartButton restartButton;
        sf::Font gameOverFont;
        sf::Text gameOverText;
        sf::Vector2f mousePos{0.f, 0.f};
    
        GameState()
        :fps(), dino(), ground(), obstacles(), scores(), clouds(), gameOverFont(), gameOverText()
        {
            gameOverFont.loadFromFile("rsrc/Fonts/Font.ttf");
            gameOverText.setFont(gameOverFont);
            dino.dino.setPosition(sf::Vector2f(windowSize_x/2 - windowSize_x/4, windowSize_y - 150.f));
            gameOverText.setString("Game Over");
            gameOverText.setPosition(sf::Vector2f(restartButton.restartButtonSprite.getPosition().x - gameOverText.getCharacterSize(),
                                                restartButton.restartButtonSprite.getPosition().y - 50));
            gameOverText.setFillColor(sf::Color(83, 83, 83));
        }
        void setMousePos(sf::Vector2i p_mousePos)
        {
            mousePos.x = p_mousePos.x;
            mousePos.y = p_mousePos.y;
        }

        void update(sf::Time deltaTime)
        {
            restartButton.checkPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
            if(playerDead == true && restartButton.restartButtonSpriteBounds.contains(mousePos) && 
                restartButton.checkPressed == true)
            {
                ground.reset();
                obstacles.reset();
                dino.reset();
                scores.reset();
                
                playerDead = false;
            }
            else
            {
                ground.updateGround();
                obstacles.update(deltaTime);
                dino.update(deltaTime, obstacles.obstacles);
                clouds.updateClouds(deltaTime);
                scores.update();
            }
            fps.update();
        }   

        void drawTo(sf::RenderWindow& window)
        {
            if(playerDead == true)
            {
                clouds.drawTo(window);
                window.draw(ground.groundSprite);
                obstacles.drawTo(window);
                window.draw(scores.scoresText);
                window.draw(scores.previousScoreText);
                window.draw(scores.HIText);
                window.draw(dino.dino); 
                window.draw(gameOverText);
                window.draw(restartButton.restartButtonSprite);
                fps.drawTo(window); 
            }
            else
            {
                clouds.drawTo(window);
                window.draw(ground.groundSprite);
                obstacles.drawTo(window);
                window.draw(scores.scoresText);
                window.draw(scores.previousScoreText);
                window.draw(scores.HIText);
                window.draw(dino.dino); 
                fps.drawTo(window);    
            }
        }

};


int main()
{
    sf::RenderWindow window(sf::VideoMode(windowSize_x, windowSize_y), "Google Chrome");
    window.setVerticalSyncEnabled(true);

    GameState gameState;

    sf::Event event;
	sf::Time deltaTime;
    sf::Clock deltaTimeClock;
    
    while(window.isOpen())
    {
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            gameState.setMousePos(sf::Mouse::getPosition(window));
        }
        deltaTime = deltaTimeClock.restart();

        gameState.update(deltaTime);

        window.clear(sf::Color::White);
        gameState.drawTo(window);
        window.display();
    }
}