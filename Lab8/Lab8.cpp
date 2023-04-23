// Lab8.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>
using namespace std;
using namespace sf;
using namespace sfp;

const float KB_SPEED = 0.2;

//error message if file not found
void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}

void MoveCrossbow(PhysicsSprite& crossbow, int elapsedMS) {
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x + (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x - (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
}

Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    //rendering window size for game
    RenderWindow window(VideoMode(800, 600), "Balloon Buster");
    World world(Vector2f(0, 0));
    int score(0);
    int arrows(3);

    //loading in crossbow image
    PhysicsSprite& crossBow = *new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400,
        600 - (sz.y / 2)));

    //loading arrow into file
    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);

    //creating a boundary box into game
    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);

    PhysicsRectangle left;
    left.setSize(Vector2f(10, 600));
    left.setCenter(Vector2f(5, 300));
    left.setStatic(true);
    world.AddPhysicsBody(left);

    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    //adding duck texture to display, adding functions for movement and collision
    Texture redTex;
    LoadTex(redTex, "images/Red_Balloon.png");
    PhysicsShapeList<PhysicsSprite> balloons;
    for (int i(0); i < 4; i++) {
        PhysicsSprite& balloon = balloons.Create();
        balloon.setTexture(redTex);
        int x = 50 + ((700 / 4) * i);
        Vector2f sz = balloon.getSize();
        balloon.setCenter(Vector2f(x, 20 + (sz.y / 2)));
        balloon.setVelocity(Vector2f(0.25, 0));
        world.AddPhysicsBody(balloon);
        balloon.onCollision =
            [&drawingArrow, &world, &arrow, &balloon, &balloons, &score, &right]
        (PhysicsBodyCollisionResult result) {
            if (result.object2 == right) {
                world.RemovePhysicsBody(balloon);
                balloons.QueueRemove(balloon);
            }
            if (result.object2 == arrow) {
                drawingArrow = false;
                world.RemovePhysicsBody(arrow);
                world.RemovePhysicsBody(balloon);
                balloons.QueueRemove(balloon);
                score += 10;
            }
        };
    }

    //adding collision function - collision true: score
    top.onCollision = [&drawingArrow, &world, &arrow]
    (PhysicsBodyCollisionResult result) {
        if (result.object2 == arrow) {
            drawingArrow = false;
            world.RemovePhysicsBody(arrow);
        }
    };

    //import text
    Font fnt;
    if (!fnt.loadFromFile("fonts/Tencent.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }

    //elasped time function
    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

    int spawn = 0;

    //arrows function ability - movement, collision function, placement
    while ((arrows > 0) || drawingArrow) {
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();
        if (deltaMS > 9) {
            lastTime = currentTime;
            world.UpdatePhysics(deltaMS);
            MoveCrossbow(crossBow, deltaMS);
            spawn += deltaMS;
            if (spawn >= 800) {
                spawn = 0;
                PhysicsSprite& balloon = balloons.Create();
                balloon.setTexture(redTex);
                int x = 50 + ((700 / 5) * 0);
                Vector2f sz = balloon.getSize();
                balloon.setCenter(Vector2f(x, 20 + (sz.y / 2)));
                balloon.setVelocity(Vector2f(0.25, 0));
                world.AddPhysicsBody(balloon);
                balloon.onCollision =
                    [&drawingArrow, &world, &arrow, &balloon, &balloons, &score, &right]
                (PhysicsBodyCollisionResult result) {
                    if (result.object2 == right) {
                        world.RemovePhysicsBody(balloon);
                        balloons.QueueRemove(balloon);
                    }
                    if (result.object2 == arrow) {
                        drawingArrow = false;
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(balloon);
                        balloons.QueueRemove(balloon);
                        score += 10;
                    }
                };
            }
            if (Keyboard::isKeyPressed(Keyboard::Space) &&
                !drawingArrow) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);
            }
        }

        //clearing window after game is over with completed parameters
        window.clear();
        if (drawingArrow) {
            window.draw(arrow);
        }

        //the removal of balloons upon collision
        balloons.DoRemovals();
        for (PhysicsShape& balloon : balloons) {
            window.draw((PhysicsSprite&)balloon);
        }

        //drawing crossbow
        window.draw(crossBow);
        //current score
        Text scoreText;
        scoreText.setString(to_string(score));
        scoreText.setFont(fnt);
        window.draw(scoreText);
        //how many arrows left
        Text arrowCountText;
        arrowCountText.setString(to_string(arrows));
        arrowCountText.setFont(fnt);
        arrowCountText.setPosition(Vector2f(790 - GetTextSize(arrowCountText).x, 0));
        window.draw(arrowCountText);
        //world.VisualizeAllBounds(window);                                    

        //displaying game window for gameplay
        window.display();
    }

    //window.display(); // this is needed to see the last frame
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();
    while (!Keyboard::isKeyPressed(Keyboard::Space));

}



