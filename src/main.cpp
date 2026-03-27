#include <raylib.h>
#include <cmath>
#include <iostream>
#include <list>


int winW = 1080;
int winH = 720;
int targetFPS = 60;
const int MAPW = 4000;
const int MAPH = 3000;



class Entity{
    public:
    float radius;
    float speed;
    float posX;
    float posY;
    Color color = {255, 255, 255, 255};

    void drawEntity(){
        DrawCircle(posX, posY, radius, color);
    }

    

    
};

class Food: public Entity{
    public:
    Food(){
        radius = 5.0f;
    }
};

class Enemy: public Entity{
    public:
    std::list<Food>* foods;
    Enemy(std::list<Food>& foodList){
        radius = 10.0f;
        
        foods = &foodList;

    }

    void eatFood(){
        for (auto& food : *foods){
            if (abs(posX - food.posX) < radius/2 && abs(posY - food.posY) < radius/2){
                radius += 0.5f;
                food.posX = GetRandomValue((-MAPW/2),(MAPW/2));
                food.posY = GetRandomValue((-MAPH/2), (winH/2));
            }
        }
    }
};

class Player: public Entity{
    public: 
    std::list<Enemy>* enemies;
    std::list<Food>* foods;
    Camera2D camera = {0};
    Player(std::list<Enemy>& enemyList, std::list<Food>& foodList){
        enemies = &enemyList;
        foods = &foodList;
        radius = 10.0f;
        posX = winW/2 - radius;
        posY = winH/2-radius;

        camera.target = (Vector2{posX, posY});
        camera.offset = (Vector2{winW/2.0f, winH/2.0f});
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        
        speed = 200.0f * pow(radius, -0.439);
    }

    void UpdateCamera(){
        float targetZoom = 40.0f/ radius;
        camera.zoom += (targetZoom - camera.zoom) * 0.1f;
        
        camera.target = (Vector2){posX, posY};
    }

    void Movement(){
        Vector2 mousePos = GetMousePosition();
        Vector2 dir = {mousePos.x - posX, mousePos.y - posY};
        

        float len = sqrt(dir.x * dir.x + dir.y* dir.y);

        if(len > radius){
            posX += (dir.x/len) * speed * GetFrameTime(); 
            posY += (dir.y/len) * speed * GetFrameTime(); 
        }
        
        
    }

    void eatFood(){
        for (auto& food : *foods){
            if (abs(posX - food.posX) < radius && abs(posY - food.posY) < radius ){
                radius += 0.5f;
                food.posX = GetRandomValue(-(MAPW/2),(MAPW/2));
                food.posY = GetRandomValue(-(MAPH/2), (winH/2));
            }
        }
    }
    

};




int main(){


    InitWindow(winW, winH, "Agario");
    SetTargetFPS(targetFPS);
    
    
    std::list<Food> foodList;

    for (int i = 0; i<300; i++){
        Food food;
        food.posX = GetRandomValue((-MAPW/2), (MAPW/2));
        food.posY = GetRandomValue((-MAPH/2), (MAPH/2));
        food.color = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        foodList.push_back(food);
    }


    std::list<Enemy> enemyList;
    

    for (int i = 0; i < 20; i++){
        Enemy enemy(foodList);
        enemy.posX = GetRandomValue((-MAPW/2), (MAPW/2));
        enemy.posY = GetRandomValue((-MAPH/2), (MAPH/2));
        enemy.color = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        enemy.radius = GetRandomValue(10, 200);
        enemy.speed = 200.0f * pow(enemy.radius, -0.439);
        enemyList.push_back(enemy);
    }

    Player player(enemyList, foodList);


    

    

    while (!WindowShouldClose()){
        
        
        player.eatFood();
        player.Movement();
        player.UpdateCamera();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(player.camera);
        for (auto& food : foodList){
            food.drawEntity();
        }
        for (auto& enemy : enemyList){
            enemy.drawEntity();
            enemy.eatFood();
        }
        player.drawEntity();

        EndMode2D();
        

        EndDrawing();
    }

    CloseWindow();

    return 0;
}