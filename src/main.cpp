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
const Color BG = {27, 27, 27, 255};

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
        posX += velX*GetFrameTime();
        posY += velY * GetFrameTime();

        velX *= 0.90f;
        velY *= 0.90f;
    }
};



class PlayerCell : public Entity{
        public: 
        float targetRadius;
        float velX = 0.0f;
        float velY = 0.0f;
        float mergeTimer = 0.0f;

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
    std::list<Player>* players;
    std::list<Food>* foods;
    Camera2D camera = {0};
    std::string name;
    Color color;
    bool isAI = false;
    Player(std::list<Player>& playerList, std::list<Food>& foodList, float x, float y, Color c, std::string n, bool ai){
        isAI = ai;
        color = c;
        name = n;
        players = &playerList;
        foods = &foodList;

        float startRadius = ai ? (float)GetRandomValue(10, 200) : 300.0f;
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
            Vector2 mousePos = GetMousePosition();
            Vector2 screenCenter = { (float)winW / 2.0f, (float)winH / 2.0f };
            
            float dx = mousePos.x - screenCenter.x;
            float dy = mousePos.y - screenCenter.y;
            float baseAngle = atan2f(dy, dx); 

            for (auto& cell : cells) {
                if (cell.targetRadius >= 20.0f) {
                    float ejectR = 5.0f; 
                    float currentArea = cell.targetRadius * cell.targetRadius;
                    float foodArea = ejectR * ejectR;
                    cell.targetRadius = sqrtf(currentArea - foodArea);

                   
                    float randomOffset = (GetRandomValue(-100, 100) / 1000.0f); 
                    float finalAngle = baseAngle + randomOffset;

                    float ndx = cosf(finalAngle);
                    float ndy = sinf(finalAngle);

                    Food f;
                    f.radius = ejectR;
                    f.posX = cell.posX + ndx * (cell.radius + 5.0f);
                    f.posY = cell.posY + ndy * (cell.radius + 5.0f);

                    f.velX = ndx * 600.0f; 
                    f.velY = ndy * 600.0f;
                    
                    f.color = color;
                    foods->push_back(f);
                }
            }
        }
    } else {
        
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
                if (sqrt(ddx*ddx+ddy*ddy) < cell.radius ){
                    cell.targetRadius = sqrt(cell.targetRadius*cell.targetRadius + food.radius*food.radius);
                    food.radius = 5.0f;
                    food.posX = GetRandomValue(-MAPW/2,MAPW/2);
                    food.posY = GetRandomValue(-MAPH/2, MAPH/2);
                }
            }
        }
        
    }


    

    

    void eatEnemy() {
    for (auto& myCell : cells) {
        for (auto& otherPlayer : *players) {
            if (&otherPlayer == this) continue;

            for (auto it = otherPlayer.cells.begin(); it != otherPlayer.cells.end(); ) {
                float ddx = myCell.posX - it->posX;
                float ddy = myCell.posY - it->posY;
                float distanceSq = ddx * ddx + ddy * ddy; 
                
                
                if (distanceSq < (myCell.radius * myCell.radius)) { 
                    if (myCell.targetRadius > it->radius * 1.15f) {
                        
                        myCell.targetRadius = sqrtf(myCell.targetRadius * myCell.targetRadius + it->radius * it->radius);
                        
                        if (otherPlayer.isAI) {
                            
                            it->posX = GetRandomValue(-MAPW/2, MAPW/2);
                            it->posY = GetRandomValue(-MAPH/2, MAPH/2);
                            it->targetRadius = GetRandomValue(20, 50);
                            it->radius = it->targetRadius;
                            ++it;
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






int main(){

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(winW, winH, "Agario");
    SetTargetFPS(targetFPS);
    Image icon = LoadImage("resources/Cell_in-game_2.png");
    SetWindowIcon(icon);

    
    
    
    std::list<Food> foodList;

    for (int i = 0; i <= foodAmount; i++){
        Food food;
        food.posX = GetRandomValue((-MAPW/2), (MAPW/2));
        food.posY = GetRandomValue((-MAPH/2), (MAPH/2));
        food.color = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        foodList.push_back(food);
    }


    std::list<Player> playerList;
    playerList.emplace_back(playerList, foodList, 0.0f, 0.0f, WHITE, "Player", false);

    for (int i = 0; i <= enemyAmount; i++){
        int randomIDX = GetRandomValue(0, names.size() -1);
        auto it = names.begin();
        std::advance(it, randomIDX);
        
        Color randomCol = {(unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255};
        
        playerList.emplace_back(playerList, foodList, GetRandomValue((-MAPW/2), (MAPW/2)), GetRandomValue((-MAPH/2), (MAPH/2)), randomCol, *it, true);
    }

    



    

    

    while (!WindowShouldClose()){
        
        Player* realPlayer = nullptr;

        

        for (auto& player : playerList){
            if(!player.isAI) realPlayer = &player;
            player.Movement();
            player.eatFood();
            player.eatEnemy();
            player.UpdateCamera();
        }

        BeginDrawing();
        ClearBackground(BG);
            if (running && realPlayer != nullptr){
                
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