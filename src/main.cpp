#include <raylib.h>
#include <cmath>
#include <iostream>
#include <list>


int winW = 1080;
int winH = 720;
int targetFPS = 60;
const int MAPW = 4000;
const int MAPH = 3000;
int foodAmount = 1500;
int enemyAmount = 60;
bool DebugMode = false;
bool running = true;

std::list<std::string> names = {
    "Spectral", "Vortex", "Apex", "Lunar", "Rift", "Zephyr", "Kinetix", "Nova",
    "Zero", "Echo", "Flux", "Omega", "Solo", "Draft", "Clutch", "Fury",
    "Alex", "Sam_2004", "Jordan", "Charlie", "Skyler", "Casey", "Riley", "Taylor",
    "Morgan", "Blake", "Parker", "Jamie", "Quinn", "Avery", "River", "Dakota",
    "FeedMe", "DontEatMe", "Big_Chungus", "Speedy", "Orbital", "Wobble", "Blobby",
    "Tiny", "Giant", "The_Void", "Glitch", "Pixel", "NomNom", "Hunter",
    "...", "???", "!", "A", "Z", "001", "Player", "Guest"
};



class Entity{
    public:
    float radius;
    float speed;
    float posX;
    float posY;
    Color color = {255, 255, 255, 255};
    std::string name = "Player";
    

    void drawEntity(bool showName){
        DrawCircleV({posX, posY}, radius, color);
        if(showName){

            int fontSize  = (int)(radius/2.0f);
            if(fontSize <10) fontSize = 10.0f;
            int textWidth = MeasureText(name.c_str(), fontSize);
            DrawText(name.c_str(), (int)posX-textWidth/2, (int)posY-fontSize/2, fontSize, BLACK);
        }
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
            float dx = posX - food.posX;
            float dy = posY - food.posY;
            if (sqrt(dx*dx + dy*dy) < radius){
                radius += 0.5f;
                food.posX = GetRandomValue(-MAPW/2,MAPW/2);
                food.posY = GetRandomValue(-MAPH/2, MAPH/2);
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
        posX = 0;
        posY = 0;
        name = "Player";

        camera.target = (Vector2{posX, posY});
        camera.offset = (Vector2{winW/2.0f, winH/2.0f});
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        
        speed = 200.0f * pow(radius, -0.439);

        
    }

    void UpdateCamera(){
        float targetZoom = 100.0f/ radius;
        camera.zoom += (targetZoom - camera.zoom) * 0.1f;
        
        camera.target = (Vector2){posX, posY};
    }

    void Movement(){
        Vector2 mousePos = GetMousePosition();
        Vector2 center = {winW / 2.0f, winH / 2.0f};

        Vector2 dir = {mousePos.x - center.x, mousePos.y - center.y};
        

        float len = sqrt(dir.x * dir.x + dir.y* dir.y);

        if(len > 5.0f){
            posX += (dir.x/len) * speed * GetFrameTime(); 
            posY += (dir.y/len) * speed * GetFrameTime(); 
        }
        
        
    }

    void eatFood(){
        for (auto& food : *foods){
            float dx = posX - food.posX;
            float dy = posY - food.posY;
            if (sqrt(dx*dx+dy*dy) < radius ){
                radius += 0.5f;
                speed = 200.0f * pow(radius, -0.439);
                food.posX = GetRandomValue(-MAPW/2,MAPW/2);
                food.posY = GetRandomValue(-MAPH/2, winH/2);
            }
        }
    }

    void eatEnemy(){
        for (auto& enemy : *enemies){
            float dx = posX - enemy.posX;
            float dy = posY - enemy.posY;
            float distance = sqrt(dx*dx+dy*dy);

            if (distance < radius || distance < enemy.radius){
                if(radius > enemy.radius*1.2f){
                    radius += enemy.radius * 0.2f;
                    speed = 200.0f * pow(radius, -0.439);
                    enemy.posX = GetRandomValue(-MAPW/2,MAPW/2);
                    enemy.posY = GetRandomValue(-MAPH/2, winH/2);
                    enemy.radius = GetRandomValue(10.0f, 30.0f);
                }
                else if(enemy.radius > radius*1.2f){
                    // running = false;
                }
                
            }
        }
    }
    

};




int main(){

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(winW, winH, "Agario");
    SetTargetFPS(targetFPS);

    
    
    
    std::list<Food> foodList;

    for (int i = 0; i <= foodAmount; i++){
        Food food;
        food.posX = GetRandomValue((-MAPW/2), (MAPW/2));
        food.posY = GetRandomValue((-MAPH/2), (MAPH/2));
        food.color = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        foodList.push_back(food);
    }


    std::list<Enemy> enemyList;
    

    for (int i = 0; i <= enemyAmount; i++){
        Enemy enemy(foodList);
        enemy.posX = GetRandomValue((-MAPW/2), (MAPW/2));
        enemy.posY = GetRandomValue((-MAPH/2), (MAPH/2));
        enemy.color = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        enemy.radius = GetRandomValue(10, 200);
        enemy.speed = 200.0f * pow(enemy.radius, -0.439);
        

        int randomIDX = GetRandomValue(0, names.size() -1);
        auto it = names.begin();
        std::advance(it, randomIDX);
        enemy.name = *it;

        enemyList.push_back(enemy);
        

    }

    Player player(enemyList, foodList);
    player.radius = 200.0f;


    

    

    while (!WindowShouldClose()){
        
        player.Movement();
        player.eatFood();
        player.eatEnemy();
        player.UpdateCamera();

        for (auto& enemy : enemyList){
            enemy.eatFood();
        }

        BeginDrawing();
        ClearBackground(BLACK);
            if (running){
                
                BeginMode2D(player.camera);

                
                
                for(int i = -MAPW/2; i<=MAPW/2; i+=200) DrawLine(i, -MAPH/2, i, MAPH/2, DARKGRAY);
                for(int i = -MAPH/2; i<=MAPH/2; i+=200) DrawLine(-MAPW/2, i, MAPH/2, i, DARKGRAY);
                    

                for (auto& food : foodList){
                    food.drawEntity(false);
                }
                for (auto& enemy : enemyList){
                    enemy.drawEntity(true);
                }
                

                
                player.drawEntity(true);

                

            EndMode2D();
            }else{
                std::cout<<"Meny\n";
            }
       
            

            if(DebugMode){
                DrawFPS(10, 10);
            }
        

        EndDrawing();
    }

    CloseWindow();

    return 0;
}