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
bool DebugMode = true;
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

class Player;

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

            float fontSize  = radius*0.35f;
            if (fontSize < 6.0f) fontSize = 6.0f;

            Font font = GetFontDefault();
            Vector2 textSize = MeasureTextEx(font, name.c_str(), fontSize, 0);
            
            float maxTextWidth = radius * 1.8f;
            if(textSize.x > maxTextWidth){
                float scale = maxTextWidth / textSize.x;
                fontSize += scale;
                textSize = MeasureTextEx(font, name.c_str(), fontSize, 0);
            }

            Vector2 drawPos = {posX - textSize.x*0.5f, posY - textSize.y*0.5f};
            DrawTextEx(font, name.c_str(), drawPos, fontSize, 0, BLACK);
        }
    }

    

    
};

class Food: public Entity{
    public:
    float velX = 0.0f;
    float velY = 0.0f;

    Food(){
        radius = 5.0f;
    }

    void Update(){
        posX += velX*GetFrameTime();
        posY += velY * GetFrameTime();

        velX *= 0.90f;
        velY *= 0.90f;
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
    void eatEnemy(std::list<Enemy>& enemyList, Player& player);

    
};

class PlayerCell : public Entity{
        public: 
        float targetRadius = 10.0f;
        float velX = 0.0f;
        float velY = 0.0f;
        float mergeTimer = 0.0f;

        PlayerCell(float r, float x, float y, Color c){
            radius = r;
            posX = x;
            posY = y;
            color = c;
            speed = 200.0f * pow(radius, -0.439);
            name = "Player";
        }
        void Update(){
            radius += (targetRadius - radius) * 0.1f;
            speed = 200.0f * pow(radius, -0.439);

            posX += velX * GetFrameTime();
            posY += velY * GetFrameTime();
            velX *= 0.85f;
            velY *= 0.85f;

            if (mergeTimer > 0.0f) mergeTimer -= GetFrameTime();
        }
    };

class Player{
    public: 
    std::list<PlayerCell> cells;
    std::list<Enemy>* enemies;
    std::list<Food>* foods;
    Camera2D camera = {0};
    Color color;
    Player(std::list<Enemy>& enemyList, std::list<Food>& foodList){
        enemies = &enemyList;
        foods = &foodList;
        color = WHITE;

        camera.offset = (Vector2{winW/2.0f, winH/2.0f});
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        
        cells.emplace_back(200.0f, 0.0f, 0.0f, color);

        
    }

    Vector2 getCenter(){
        float x = 0, y = 0;
        for (auto& cell : cells){
            x += cell.posX; 
            y += cell.posY;
            
        }
        x/= cells.size();
        y /= cells.size();
        return {x, y};
    }

    float getLargestRadius(){
        float r = 0;
        for (auto& cell : cells) r = fmaxf(r, cell.radius);
        return r;
    }


    void UpdateCamera(){
        Vector2 center = getCenter();
        float maxDist = getLargestRadius();
        for (auto& cell : cells){
            float ddx = cell.posX - center.x;
            float ddy = cell.posY - center.y;
            float dist = sqrtf(ddx*ddx+ddy*ddy) + cell.radius;
            if (dist > maxDist) maxDist = dist;
        }
        float targetZoom = (winH / 2.0f) / (maxDist * 1.2f);
        camera.zoom += (targetZoom - camera.zoom) * 0.05f;
        camera.target = center;
    }

    

    void Movement(){

        Vector2 mousePos = GetMousePosition();
        Vector2 center = {winW / 2.0f, winH / 2.0f};

        float dx = mousePos.x - center.x;
        float dy = mousePos.y - center.y;
        float len = sqrtf(dx*dx+dy*dy);
        for (auto& cell : cells){
            if (len > 5.0f){
                cell.posX += (dx/len) * cell.speed * GetFrameTime();
                cell.posY += (dy/len) * cell.speed * GetFrameTime();
            }
            cell.Update();
        }

        if(IsKeyPressed(KEY_SPACE)){
            std::list<PlayerCell> toAdd;
            for (auto& cell : cells){
                if(cell.targetRadius >= 20.0f){
                    float newR = cell.targetRadius / sqrtf(2.0f);
                    cell.targetRadius = newR;
                    cell.radius = newR;

                    float ndx = len > 0 ? dx/len : 1.0f;
                    float ndy = len > 0 ? dy/len : 0.0f;

                    PlayerCell newCell(newR,
                    cell.posX + ndx * (newR + 2.0f +2.0f),
                    cell.posY + ndy * (newR + 2.0f +2.0f),
                    color);
                    newCell.targetRadius = newR;
                    newCell.radius = newR;
                    newCell.velX = ndx*400.0f;
                    newCell.velY = ndy*400.0f;
                    newCell.mergeTimer = 10.0f;
                    cell.mergeTimer = 10.0f;
                    toAdd.push_back(newCell);

                }
            }
            for (auto& cell : toAdd) cells.push_back(cell);
        }
        

        if(IsKeyDown(KEY_W)){
            for (auto& cell : cells){
                if (cell.targetRadius >= 20.0f){
                    cell.targetRadius -= 0.5f;

                    float ndx = len > 0 ? dx/len : 1.0f;
                    float ndy = len > 0 ? dy/len : 0.0f;

                    float angleOffset = GetRandomValue(-15, 15) * (PI / 180.0f);
                    float cosA = cosf(angleOffset);
                    float sinA = sinf(angleOffset);
                    float rdx = ndx*cosA - ndy*sinA;
                    float rdy = ndx*sinA + ndy*cosA;

                    float ejectSpeed = GetRandomValue(300.0f, 500.0f);

                    Food food;
                    food.posX = cell.posX + rdx * (cell.radius + food.radius + 2.0f);
                    food.posY = cell.posY + rdy * (cell.radius + food.radius + 2.0f);
                    food.velX = rdx * ejectSpeed;
                    food.velY = rdy * ejectSpeed;
                    food.color = color;

                    foods->push_back(food);

                }
            }
            

        }
        
        for (auto it1 = cells.begin(); it1 != cells.end(); ++it1){
            for (auto it2 = std::next(it1); it2 != cells.end(); ){
                if (it1->mergeTimer <= 0.0f && it2->mergeTimer <= 0.0f){
                    float ddx = it1->posX - it2->posX;
                    float ddy = it1->posY - it2->posY;
                    float dist = sqrtf(ddx*ddx + ddy*ddy);

                    auto& bigger = it1->radius >= it2->radius ? *it1 : *it2;
                    

                    
                    if(dist < bigger.radius){
                        float r1 = it1->targetRadius;
                        float r2 = it2->targetRadius;
                        it1->targetRadius = sqrtf(r1*r1 + r2*r2);
                        it1->radius = it1->targetRadius;
                        it2 = cells.erase(it2);
                        UpdateCamera();
                        continue;
                    }
                    

                }
                ++it2;
                
            }

            
            
        }

        
        

        
        
        
    }

    void eatFood(){
        for (auto& cell : cells){
            for (auto& food : *foods){
                float ddx = cell.posX - food.posX;
                float ddy = cell.posY - food.posY;
                if (sqrt(ddx*ddx+ddy*ddy) < cell.radius ){
                    cell.targetRadius = sqrt(cell.targetRadius*cell.targetRadius + food.radius*food.radius);
                    food.radius = 5.0f;
                    food.posX = GetRandomValue(-MAPW/2,MAPW/2);
                    food.posY = GetRandomValue(-MAPH/2, winH/2);
                }
            }
        }
        
    }


    

    

    void eatEnemy(){

        for (auto& cell : cells){
            for (auto& enemy : *enemies){
                float ddx = cell.posX - enemy.posX;
                float ddy = cell.posY - enemy.posY;
                float distance = sqrt(ddx*ddx+ddy*ddy);

                if (distance < cell.radius || distance < enemy.radius * 1.2f){
                    if(cell.targetRadius > enemy.radius*1.2f){
                        cell.targetRadius += enemy.radius * 0.2f;
                        enemy.posX = GetRandomValue(-MAPW/2,MAPW/2);
                        enemy.posY = GetRandomValue(-MAPH/2, winH/2);
                        enemy.radius = GetRandomValue(10.0f, 30.0f);
                    }
                    else if(enemy.radius > cell.radius*1.2f){
                        // running = false;
                    }
                    
                }
            }
        }

        
        
    }

    void draw(){
            for (auto& cell : cells) cell.drawEntity(true);
        }
    

};

void Enemy::eatEnemy(std::list<Enemy>& enemyList, Player& player){
        for (auto& enemy : enemyList){
            if(&enemy == this) continue;
            float dx = posX - enemy.posX;
            float dy = posY - enemy.posY;
            float distance = sqrt(dx*dx+dy*dy);

            if (distance < radius || distance < enemy.radius){
                if(radius > enemy.radius*1.2f){
                    radius += enemy.radius * 0.2f;
                    speed = 200.0f * pow(radius, -0.439);
                    enemy.posX = GetRandomValue(-MAPW/2,MAPW/2);
                    enemy.posY = GetRandomValue(-MAPH/2, MAPH/2);
                    enemy.radius = GetRandomValue(10.0f, 30.0f);
                }
                
                
            }

            
        }
        for (auto it = player.cells.begin(); it != player.cells.end(); ){
            float pdx = posX - it->posX;
            float pdy = posY - it->posY;
            float pDist = sqrtf(pdx*pdx + pdy*pdy);

            if (pDist < radius && radius > it->radius * 1.2f){
                radius = sqrtf(radius*radius + it->radius * it->radius * 0.2f);
                speed = 200.0f * pow(radius, -0.439f);

                if (player.cells.size() > 1){
                    it = player.cells.erase(it);
                } else {
                    running = false;
                    ++it;
                }
                continue;
            }
            ++it;
        }
        
    }




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



    

    

    while (!WindowShouldClose()){
        
        player.Movement();
        player.eatFood();
        player.eatEnemy();
        player.UpdateCamera();

        for (auto& enemy : enemyList){
            enemy.eatFood();
            enemy.eatEnemy(enemyList, player);
        }

        BeginDrawing();
        ClearBackground(BLACK);
            if (running){
                
                BeginMode2D(player.camera);

                
                
                for(int i = -MAPW/2; i<=MAPW/2; i+=200) DrawLine(i, -MAPH/2, i, MAPH/2, DARKGRAY);
                for(int i = -MAPH/2; i<=MAPH/2; i+=200) DrawLine(-MAPW/2, i, MAPH/2, i, DARKGRAY);
                    

                for (auto& food : foodList){
                    food.Update();
                    food.drawEntity(false);
                }
                for (auto& enemy : enemyList){
                    enemy.drawEntity(true);
                }
                

                
                player.draw();

                

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