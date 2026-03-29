#include <raylib.h>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>


int winW = 1080;
int winH = 720;
int targetFPS = 60;
const int MAPW = 4000;
const int MAPH = 3000;
int foodAmount = 500;
int enemyAmount = 40;
bool DebugMode = true;
bool running = false;
const Color BG = {27, 27, 27, 255};
bool fullScreen = false;

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
        DrawCircleV({posX, posY}, radius+2.0f, BLACK);
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
        if (fabsf(velX) > 0.1f || fabsf(velY) > 0.1f){
        posX += velX * GetFrameTime();
        posY += velY * GetFrameTime();
        velX *= 0.90f;
        velY *= 0.90f;
        }
        else {
            velX = 0; velY = 0;
        }
    }
};



class PlayerCell : public Entity{
        public: 
        float targetRadius;
        float velX = 0.0f;
        float velY = 0.0f;
        float mergeTimer = 0.0f;
        float speedNumber = 450.0f;

        PlayerCell(float r, float x, float y, Color c, std::string n){
            radius = r; 
            targetRadius = r;
            posX = x;
            posY = y;
            color = c;
            name = n;
        }
        void Update(){
            radius += (targetRadius - radius) * 0.1f;
            speed = speedNumber * pow(radius, -0.439);

            posX += velX * GetFrameTime();
            posY += velY * GetFrameTime();
            velX *= 0.85f;
            velY *= 0.85f;

            if (mergeTimer > 0.0f) mergeTimer -= GetFrameTime();
        }
    };

class Player{
    public: 
    std::vector<PlayerCell> cells;
    std::vector<Player>* players;
    std::vector<Food>* foods;
    Camera2D camera = {0};
    std::string name;
    Color color;
    bool isAI = false;
    float playerRadius = 190.0f;
    float AIWanderTimer = 0.0f;
    float AISplitTimer = 0.0f;
    float AIEjectTimer = 0.0f;
    Vector2 AIWanderTarget = {0, 0};
    Player(std::vector<Player>& playerList, std::vector<Food>& foodList, float x, float y, Color c, std::string n, bool ai){
        isAI = ai;
        color = c;
        name = n;
        players = &playerList;
        foods = &foodList;

        float startRadius = ai ? (float)GetRandomValue(10, 200) : playerRadius;
        cells.reserve(16);
        cells.emplace_back(startRadius, x, y, c, n);
        

        if (!isAI){
            camera.offset = (Vector2{winW/2.0f, winH/2.0f});
            camera.rotation = 0.0f;
            camera.zoom = 1.0f;
        }
        
        
        

        
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
        if (playerRadius >= 500){
            playerRadius = 500;
        }
        if (isAI) return;
        Vector2 center = getCenter();
        float maxDist = getLargestRadius();
        for (auto& cell : cells){
            float ddx = cell.posX - center.x;
            float ddy = cell.posY - center.y;
            float dist = sqrtf(ddx*ddx+ddy*ddy) + cell.radius;
            if (dist > maxDist) maxDist = dist;
        }
        float targetZoom = (winH / 2.0f) / (maxDist * 1.2f);
        targetZoom = fmaxf(0.3f, fminf(targetZoom, 3.0f));
        camera.zoom += (targetZoom - camera.zoom) * 0.01f;
        camera.target = center;
    }

    

    void Movement() {
    if (!isAI) {
        Vector2 mousePos = GetMousePosition();
        Vector2 center = { (float)winW / 2.0f, (float)winH / 2.0f };

        float dx = mousePos.x - center.x;
        float dy = mousePos.y - center.y;
        float len = sqrtf(dx * dx + dy * dy);

        for (auto& cell : cells) {
            if (len > 5.0f) {
                cell.posX += (dx / len) * cell.speed * GetFrameTime();
                cell.posY += (dy / len) * cell.speed * GetFrameTime();
            }
        }

       
        if (IsKeyPressed(KEY_SPACE)) {
            std::list<PlayerCell> toAdd;
            float baseAngle = atan2f(dy, dx); 

            for (auto& cell : cells) {
                if (cell.targetRadius >= 35.0f) { 
                    float newR = cell.targetRadius / sqrtf(2.0f);
                    cell.targetRadius = newR;
                    cell.radius = newR;

                    float splitOffset = (GetRandomValue(-50, 50) / 1000.0f); 
                    float finalAngle = baseAngle + splitOffset;

                    float ndx = cosf(finalAngle);
                    float ndy = sinf(finalAngle);

                    PlayerCell newCell(newR, cell.posX + ndx * (newR + 10.0f), cell.posY + ndy * (newR + 10.0f), color, name);

                    newCell.velX = ndx * 800.0f;
                    newCell.velY = ndy * 800.0f;
                    
                    newCell.mergeTimer = 15.0f; 
                    cell.mergeTimer = 15.0f;
                    
                    toAdd.push_back(newCell);
                }
            }
            for (auto& nc : toAdd) cells.push_back(nc);
        }

        
        if (IsKeyDown(KEY_W)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            for (auto& cell : cells) {
                if (cell.targetRadius >= 20.0f) {
                    float dx = mouseWorldPos.x - cell.posX;
                    float dy = mouseWorldPos.y - cell.posY;
                    float baseAngle = atan2f(dy, dx);

                    float ejectR = 5.0f;
                    float currentArea = cell.targetRadius * cell.targetRadius;
                    cell.targetRadius = sqrtf(currentArea - ejectR * ejectR);

                    float randomOffset = (GetRandomValue(-100, 100) / 1000.0f);
                    float finalAngle = baseAngle + randomOffset;
                    float ndx = cosf(finalAngle);
                    float ndy = sinf(finalAngle);

                    int idx = GetRandomValue(0, foods->size() - 1);
                    (*foods)[idx].posX = cell.posX + ndx * (cell.radius + 5.0f);
                    (*foods)[idx].posY = cell.posY + ndy * (cell.radius + 5.0f);
                    (*foods)[idx].velX = ndx * 600.0f;
                    (*foods)[idx].velY = ndy * 600.0f;
                    (*foods)[idx].color = color;
                }
            }
        }
    } else {
        Vector2 myCenter = getCenter();
        float myR = 0;
        for (auto& cell : cells) myR += cell.targetRadius * cell.targetRadius;
        myR = sqrtf(myR);

        float dx = 0;
        float dy = 0;
        bool foundTarget = false;

        for (auto& other : *players){
            if (&other == this) continue;
            if (other.cells.empty()) continue;

            Vector2 otherCenter = other.getCenter();
            float otherR = 0;
            for (auto& cell : other.cells) otherR += cell.targetRadius * cell.targetRadius;
            otherR = sqrtf(otherR);

            float ex = otherCenter.x - myCenter.x;
            float ey = otherCenter.y - myCenter.y;
            float dist = sqrtf(ex*ex + ey*ey);

            if (dist > myR * 12.0f) continue;

            if (myR > otherR * 1.15f) {
            dx = ex; dy = ey;
            if (AISplitTimer > 0.0f) AISplitTimer -= GetFrameTime();
            if (dist < myR * 1.5f && AISplitTimer <= 0.0f && cells.size() < 8 && myR > 80.0f) {
                std::list<PlayerCell> toAdd;
                for (auto& cell : cells) {
                    if (cell.targetRadius >= 35.0f) {
                        float newR = cell.targetRadius / sqrtf(2.0f);
                        cell.targetRadius = newR;
                        cell.radius = newR;
                        float len = sqrtf(ex*ex + ey*ey);
                        float ndx = ex/len, ndy = ey/len;
                        PlayerCell nc(newR, cell.posX + ndx*(newR+10), cell.posY + ndy*(newR+10), color, name);
                        nc.velX = ndx * 800.0f;
                        nc.velY = ndy * 800.0f;
                        nc.mergeTimer = cell.mergeTimer = 15.0f;
                        toAdd.push_back(nc);
                    }
                }
                for (auto& nc : toAdd) cells.push_back(nc);
                AISplitTimer = 10.0f; 
            }
        } else if (otherR > myR * 1.15f) {
            dx = -ex; dy = -ey;
            if (AIEjectTimer > 0.0f) AIEjectTimer -= GetFrameTime();
            if (myR > 50.0f && AIEjectTimer <= 0.0f) {
                for (auto& cell : cells) {
                    if (cell.targetRadius >= 20.0f) {
                        float len = sqrtf(ex*ex + ey*ey);
                        float baseAngle = atan2f(-ey, -ex);
                        float randomOffset = (GetRandomValue(-100, 100) / 1000.0f);
                        float finalAngle = baseAngle + randomOffset;
                        float ndx = cosf(finalAngle);
                        float ndy = sinf(finalAngle);

                        float ejectR = 5.0f;
                        cell.targetRadius = sqrtf(cell.targetRadius*cell.targetRadius - ejectR*ejectR);

                        int idx = GetRandomValue(0, foods->size() - 1);
                        (*foods)[idx].posX = cell.posX + ndx * (cell.radius + 5.0f);
                        (*foods)[idx].posY = cell.posY + ndy * (cell.radius + 5.0f);
                        (*foods)[idx].velX = ndx * 600.0f;
                        (*foods)[idx].velY = ndy * 600.0f;
                        (*foods)[idx].color = color;
                    }
                }
                AIEjectTimer = 0.5f; 
            }
        }
            
            foundTarget = true;
            break;
        }

        if(!foundTarget){
            AIWanderTimer -= GetFrameTime();
            if (AIWanderTimer <= 0.0f){
                AIWanderTarget = {
                    (float)GetRandomValue(-MAPW/2, MAPW/2),
                    (float)GetRandomValue(-MAPH/2, MAPH/2)
                };
                AIWanderTimer = (float)GetRandomValue(3, 8); 
            }
            dx = AIWanderTarget.x - myCenter.x;
            dy = AIWanderTarget.y - myCenter.y;
        }

        float len = sqrtf(dx*dx+dy*dy);
            if(len > 5.0f){
                dx /= len;
                dy /= len;
                for (auto& cell : cells){
                    cell.posX += dx * cell.speed * GetFrameTime();
                    cell.posY += dy * cell.speed * GetFrameTime();
                }
            }
        }
    

  
    for (auto& cell : cells) {
        cell.Update();
        
        if (cell.posX < -MAPW / 2) cell.posX = -MAPW / 2;
        if (cell.posX > MAPW / 2)  cell.posX = MAPW / 2;
        if (cell.posY < -MAPH / 2) cell.posY = -MAPH / 2;
        if (cell.posY > MAPH / 2)  cell.posY = MAPH / 2;
    }

   
    for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
        auto it2 = std::next(it1);
        while (it2 != cells.end()) {
            if (it1->mergeTimer <= 0.0f && it2->mergeTimer <= 0.0f) {
                float ddx = it1->posX - it2->posX;
                float ddy = it1->posY - it2->posY;
                float dist = sqrtf(ddx * ddx + ddy * ddy);

                if (dist < it1->radius * 0.8f) {
                    float area1 = it1->targetRadius * it1->targetRadius;
                    float area2 = it2->targetRadius * it2->targetRadius;
                    it1->targetRadius = sqrtf(area1 + area2);
                    it2 = cells.erase(it2); 
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
                if (fabsf(ddx) > cell.radius || fabsf(ddy) > cell.radius) continue;
                if (sqrtf(ddx*ddx+ddy*ddy) < cell.radius){
                    cell.targetRadius = sqrtf(cell.targetRadius*cell.targetRadius + food.radius*food.radius);
                    food.posX = GetRandomValue(-MAPW/2, MAPW/2);
                    food.posY = GetRandomValue(-MAPH/2, MAPH/2);
                    food.color = {(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),255};
                }
            }
        }
    }
    

    

    void eatEnemy() {
    Vector2 myCenter = getCenter();
    for (auto& myCell : cells) {
        for (auto& otherPlayer : *players) {
            if (&otherPlayer == this) continue;
            if (otherPlayer.cells.empty()) continue;

          
            Vector2 otherCenter = otherPlayer.getCenter();
            float cdx = myCenter.x - otherCenter.x;
            float cdy = myCenter.y - otherCenter.y;
            if (cdx*cdx + cdy*cdy > 800.0f * 800.0f) continue;

            for (auto it = otherPlayer.cells.begin(); it != otherPlayer.cells.end(); ) {
                float ddx = myCell.posX - it->posX;
                float ddy = myCell.posY - it->posY;
                float distanceSq = ddx * ddx + ddy * ddy;

                if (distanceSq < (myCell.radius * myCell.radius)) {
                    if (myCell.targetRadius > it->radius * 1.15f) {
                        myCell.targetRadius = sqrtf(myCell.targetRadius * myCell.targetRadius + it->radius * it->radius);

                        if (otherPlayer.isAI) {
                            it = otherPlayer.cells.erase(it);
                            if (otherPlayer.cells.empty()) {
                                float newR = GetRandomValue(20, 50);
                                float newX = GetRandomValue(-MAPW/2, MAPW/2);
                                float newY = GetRandomValue(-MAPH/2, MAPH/2);
                                Color newCol = {(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),255};
                                otherPlayer.cells.emplace_back(newR, newX, newY, newCol, otherPlayer.name);
                                otherPlayer.color = newCol;
                                break;
                            }
                        } else {
                            it = otherPlayer.cells.erase(it);
                        }
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
        }
    }
}

    void draw(){
            for (auto& cell : cells) cell.drawEntity(true);
        }
    

};

void createFood(std::vector<Food>& foodList) {
    foodList.clear();
    foodList.reserve(2500);
    for (int i = 0; i <= foodAmount; i++){
        Food food;
        food.posX = GetRandomValue(-MAPW/2, MAPW/2);
        food.posY = GetRandomValue(-MAPH/2, MAPH/2);
        food.color = {(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),255};
        foodList.push_back(food);
    }
}

void createPlayers(std::vector<Player>& playerList, std::vector<Food>& foodList, std::string playerName, Color playerColor) {
    playerList.clear();
    playerList.reserve(70);
    for (int i = 0; i <= enemyAmount; i++){
        int randomIDX = GetRandomValue(0, names.size()-1);
        auto it = names.begin();
        std::advance(it, randomIDX);
        Color randomCol = {(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),(unsigned char)GetRandomValue(0,255),255};
        playerList.emplace_back(playerList, foodList, GetRandomValue(-MAPW/2,MAPW/2), GetRandomValue(-MAPH/2,MAPH/2), randomCol, *it, true);
    }
    playerList.emplace_back(playerList, foodList, 0.0f, 0.0f, playerColor, playerName, false);
}




int main(){

    if(fullScreen){
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_FULLSCREEN_MODE | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
        InitWindow(0, 0, "Agario");
        winW = GetScreenWidth();
        winH = GetScreenHeight();
    }else{
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
        InitWindow(winW, winH, "Agario");
    }
    
    
    SetTargetFPS(targetFPS);
    Image icon = LoadImage("resources/Cell_in-game_2.png");
    SetWindowIcon(icon);
    SetExitKey(0);
    

    std::vector<Food> foodList;
    std::vector<Player> playerList;
    
    

    while (!WindowShouldClose()){
        
        Player* realPlayer = nullptr;

        

        for (auto& player : playerList){
            if(!player.isAI) realPlayer = &player;
            player.Movement();
            player.eatFood();
            player.eatEnemy();
            player.UpdateCamera();
        }

        if (running && realPlayer != nullptr && realPlayer->cells.empty()) {
            running = false;
        }

        BeginDrawing();
        ClearBackground(BG);
            if (running && realPlayer != nullptr && !realPlayer->cells.empty()){
                
                BeginMode2D(realPlayer->camera);

                
                
                for(int i = -MAPW/2; i<=MAPW/2; i+=200) DrawLine(i, -MAPH/2, i, MAPH/2, DARKGRAY);
                for(int i = -MAPH/2; i<=MAPH/2; i+=200) DrawLine(-MAPW/2, i, MAPW/2, i, DARKGRAY);
                    

                for (auto& food : foodList){
                    food.Update();
                    food.drawEntity(false);
                }
                for (auto& player : playerList){
                    player.draw();

                }
                

                

                

            EndMode2D();
            }else{
                ClearBackground(BG);
                
                if(IsKeyPressed(KEY_TAB)){
                    createFood(foodList);
                    createPlayers(playerList, foodList, "Player", WHITE);
                    running = true;
                    
                }
            }
       
            

            if(DebugMode){
                DrawFPS(10, 10);
            }
        

        EndDrawing();
    }

    CloseWindow();

    return 0;
}