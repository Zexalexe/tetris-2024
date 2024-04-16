//Using SDL and standard IO
#define SDL_MAIN_HANDLED //avoid  "undefined reference to 'WinMain'"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include "include/easy_sdl.h"
#include "include/tetris.h"


using namespace std;


static int MaxFallSpeed = 1000;

TextStyle_t normal = {
        .solid = true,
        .size = 42,
        .italic = false,
        .underline = false,
        .bold = false,
        .strikethrough = false,
        .font = NULL,
        .foreground = {255, 255,255, 255}
};

TextStyle_t highlight = {
        .solid = true,
        .size = 42,
        .italic = false,
        .underline = false,
        .bold = false,
        .strikethrough = false,
        .font = NULL,
        .foreground = {255, 0,0, 255}
};

TextStyle_t info = {
    .solid = true,
    .size = 40,
    .italic = false,
    .underline = false,
    .bold = true,
    .strikethrough = false,
    .font = NULL,
    .foreground = {255, 255, 255, 255}
};


Player_Data_t playerOne;

World_Data_t currentGame;

static char world[WORLD_HEIGHT][WORLD_WIDTH];

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 938;

const int MENU_OPTIONS_COUNT = 3;
char* MENU_OPTIONS[MENU_OPTIONS_COUNT] = {"Start Game","Credits", "Quit"};


const int PAUSE_OPTIONS_COUNT = 3;
char* PAUSE_OPTIONS[PAUSE_OPTIONS_COUNT] = {"New Game", "Resume", "Menu"};

int selectedOption = 0;

void menu(SDL_Event*);
void game(SDL_Event*);
void pause(SDL_Event*);
void credits(SDL_Event*);

void (*gestore_eventi)(SDL_Event *) = &menu;


//TODO Compleate questo elenco con TUTTI gli assets che useremo
//IMPORTANTE: dare un ordine fisso agli assets
char* assetsOrigin[]  = {
    "assets/blocchi/arancione.png",
    "assets/blocchi/blu.png",
    "assets/blocchi/giallo.png",
    "assets/blocchi/rosa.png",
    "assets/blocchi/rosso.png",
    "assets/blocchi/verde.png",
    "assets/blocchi/viola.png",
    "assets/grafica/menu.png",
    "assets/grafica/game.png",
    "assets/grafica/pausa.png",
    "assets/fonts/ka1.ttf",
    "assets/fonts/Monaco.ttf",
};

Easy_Asset_t* assets[sizeof(assetsOrigin)/sizeof(char*)];



void handleInput() {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_QUIT){
            freeEasySDL();
            exit(0);
        }else if (e.key.keysym.sym == SDLK_ESCAPE){
            gestore_eventi = &pause;
        }
        else {
            gestore_eventi(&e);
        }
    }
}


//TODO implement function draw background
void drawBackground() {
    Easy_Asset_t *asset = loadAsset("assets/grafica/game.png");
    drawAsset(0,0, asset);
}

void drawInfo(Player_Data_t player) {
    setTextStyle(&info);

    const int N_INFO = 3;

    string topString = to_string(currentGame.topScore);
    string scoreString = to_string(playerOne.score);
    string levelString = to_string(currentGame.level);

    char* textInfo[N_INFO] = {&topString[0],&scoreString[0], &levelString[0]};


        int startSize = 1000;
        int startY = 200;
        int x = 235;

        int nextBlockY = 460;
        int nextBlockX = 1095;

        if(playerOne.score > 0){
            int textSize = 15 * playerOne.score / 10 ;

            TextStyle_t textStyle = info;
            textStyle.size = textSize;
            setTextStyle(&textStyle);

            drawText(
                (uint16_t ) (x-10) + currentGame.topScore / 10000,(uint16_t ) startY,
                (uint16_t ) 50 + (currentGame.topScore) / 800, (uint16_t ) 50,
                textInfo[0], TEXT_LEFT
                );
        drawText(
                (uint16_t ) (x-10) + (playerOne.score) / 10000 ,(uint16_t ) startY + 1 * 330,
                (uint16_t ) 50 + (playerOne.score) / 800, (uint16_t ) 50,
                textInfo[1], TEXT_LEFT
                );
        drawText(
                (uint16_t ) x,(uint16_t ) startY + 2 * 305,
                (uint16_t ) 35, (uint16_t ) 35,
                textInfo[2], TEXT_LEFT
                );
        }
        else if (playerOne.score == 0){
            drawText(
                (uint16_t ) x,(uint16_t ) startY,
                (uint16_t ) 50, (uint16_t ) 50,
                textInfo[0], TEXT_LEFT
                );
        drawText(
                (uint16_t ) x,(uint16_t ) startY + 1 * 330,
                (uint16_t ) 50, (uint16_t ) 50,
                textInfo[1], TEXT_LEFT
                );
        drawText(
                (uint16_t ) x,(uint16_t ) startY + 2 * 305,
                (uint16_t ) 35, (uint16_t ) 35,
                textInfo[2], TEXT_LEFT
                );
        }


        for(int i = 0; i < SHAPE_HEIGHT; i++){
            for(int j = 0; j < SHAPE_WIDTH; j++){
                char point = blocks[currentGame.nextBlock][player.rotation][i][j];
            if(point != ' '){
                Easy_Asset_t *block = loadAsset(assetsOrigin[point - '1']);
                drawAsset(
                          nextBlockX + j * 32,
                          nextBlockY + i * 32,
                          block
                          );
            }
        }
    }
}

//TODO fix segmentation fault
void drawWorld() {
    for(int i = 0; i < WORLD_HEIGHT; i++){
        for(int j = 0; j < WORLD_WIDTH; j++) {
            if(world[i][j]  >= '1' && world[i][j] <= '7'){
                Easy_Asset_t *block = loadAsset(assetsOrigin[world[i][j] - '1']);
                drawAsset(
                          540 + j * 32,
                          i * 32,
                          block
                          );
            }
        }
    }
}

//TODO implement function draw player block
void drawPlayerBlock(Player_Data_t player) {
    for(int i = 0; i < SHAPE_HEIGHT; i++){
        for(int j = 0; j < SHAPE_WIDTH; j++){
                char point = blocks[player.assetIdx][player.rotation][i][j];
            if(point != ' '){
                Easy_Asset_t *block = loadAsset(assetsOrigin[point - '1']);
                drawAsset(
                          540 + (j +player.x) * 32,
                          (i + player.y) * 32,
                          block
                          );
            }
        }
    }
}



void drawGame(){
         // Clear the screen
            SDL_Renderer *renderer = getSDLRender();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

        //TODO drawbackground
            drawBackground();

        //TODO drawpoints and all the text info
            drawInfo(playerOne);

        //TODO drawworld
            drawWorld();

        //TODO draw the playerblock
            drawPlayerBlock(playerOne);

            int currentTime = SDL_GetTicks();
            int s = currentTime - currentGame.lastTime;

            if (s >= currentGame.fallSpeed) {
                    playerOne.y += 1;
                    currentGame.lastTime = currentTime;
            }


        //TODO check that after the call to placeIt we have to generate a new block





            SDL_RenderPresent(renderer);
}



//TODO implement the game mode
void game(SDL_Event* e){

        if(e->type == SDL_QUIT){
            freeEasySDL();
            exit(0);
        }
        else if(e->type == SDL_KEYDOWN){
            switch(e->key.keysym.sym){
                        case SDLK_s:
                            playerOne.y += 1;
                            break;
                        case SDLK_a:
                            if (!isColliding(playerOne.y, playerOne.x - 1, blocks, playerOne.assetIdx, world)) {
                            playerOne.x -= 1;
                            }
                            break;
                        case SDLK_d:
                            //TODO fix isColliding issues
                            if (!isColliding(playerOne.y, playerOne.x + 1, blocks, playerOne.assetIdx, world)) {
                            playerOne.x += 1;
                            }
                            break;
                        case SDLK_q:
                            playerOne.rotation = (playerOne.rotation - 1 + ROTATION) % ROTATION;
                            break;
                        case SDLK_e:
                            playerOne.rotation = (playerOne.rotation + 1) % ROTATION;
                            break;
                    }
        }
}


void menu(SDL_Event* e){
    if (e->type == SDL_QUIT) {
        freeEasySDL();
        exit(0);
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_w:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_s:
                 selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_RETURN:
                switch (selectedOption) {
                    case 0:
                        cout << "Starting the game..." << endl;
                        //TODO inizializzare la matrice world[][] e tutte le variabili relative (punteggio, level, etc.)
                        setupNewGames(playerOne, currentGame, world);
                        gestore_eventi = &game;
                        break;
                    case 1:
                        cout << "Credits..." << endl;
                        gestore_eventi = &credits;
                        break;
                    case 2:
                        exit(0);
                        break;
                    default:

                        break;
                }
                break;
        }
    }
}

void drawMenu() {
    // Clear the screen
    SDL_Renderer *renderer = getSDLRender();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the background image
    Easy_Asset_t *asset = loadAsset("assets/grafica/menu.png");
    drawAsset(0,0, asset);

    normal.size = 42;
    highlight.size = 42;

    // Draw menu options
    int startY = (SCREEN_HEIGHT - MENU_OPTIONS_COUNT ) / 2;
    for (int i = 0; i < MENU_OPTIONS_COUNT; ++i) {
        int x = SCREEN_WIDTH / 2;
        int y = startY + i * 100;
        bool isSelected = (i == selectedOption);
        if(isSelected){
            setTextStyle(&highlight);
        }else{
            setTextStyle(&normal);
        }
        drawText(
                (uint16_t ) x,(uint16_t ) y,
                (uint16_t ) SCREEN_WIDTH, (uint16_t ) 50,
                MENU_OPTIONS[i], TEXT_CENTERED
        );
    }

    // Update the screen
    SDL_RenderPresent(renderer);
}


void pause(SDL_Event* e){
    if (e->type == SDL_QUIT) {
        freeEasySDL();
        exit(0);
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_w:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_s:
                 selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_RETURN:
                switch (selectedOption) {
                    case 0:
                        cout << "Starting new game..." << endl;
                        //TODO inizializzare la matrice world[][] e tutte le variabili relative (punteggio, level, etc.)
                        setupNewGames(playerOne, currentGame, world);
                        gestore_eventi = &game;
                        break;
                    case 1:
                        cout << "Resume game..." << endl;
                        gestore_eventi = &game;
                        break;
                    case 2:
                        cout << "Menu..." << endl;
                        gestore_eventi = &menu;
                        break;
                    default:

                        break;
                }
                break;
        }
    }
}

void drawPause() {
    // Clear the screen
    SDL_Renderer *renderer = getSDLRender();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the background image
    Easy_Asset_t *asset = loadAsset("assets/grafica/pausa.png");
    drawAsset(0,0, asset);

    normal.size = 58;
    highlight.size = 58;

    // Draw menu options
    int startY = (SCREEN_HEIGHT - PAUSE_OPTIONS_COUNT ) / 3;
    for (int i = 0; i < PAUSE_OPTIONS_COUNT; ++i) {
        int x = SCREEN_WIDTH / 2;
        int y = startY + i * 150;
        bool isSelected = (i == selectedOption);
        if(isSelected){
            setTextStyle(&highlight);
        }else{
            setTextStyle(&normal);
        }
        drawText(
                (uint16_t ) x,(uint16_t ) y,
                (uint16_t ) SCREEN_WIDTH, (uint16_t ) 60,
                PAUSE_OPTIONS[i], TEXT_CENTERED
        );
    }

    // Update the screen
    SDL_RenderPresent(renderer);
}

void credits(SDL_Event* e){
    if (e->type == SDL_QUIT) {
        freeEasySDL();
        exit(0);
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_w:
                selectedOption = (selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_s:
                 selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % MENU_OPTIONS_COUNT;
                break;
            case SDLK_RETURN:
                switch (selectedOption) {
                    case 0:
                        cout << "Starting the game..." << endl;
                        //TODO inizializzare la matrice world[][] e tutte le variabili relative (punteggio, level, etc.
                        setupNewGames(playerOne, currentGame, world);
                        gestore_eventi = &game;
                        break;
                    case 1:
                        cout << "Credits..." << endl;
                        gestore_eventi = &credits;
                        break;
                    case 2:
                        exit(0);
                        break;
                    default:

                        break;
                }
                break;
        }
    }
}

void drawCredits(){
    // Clear the screen
    SDL_Renderer *renderer = getSDLRender();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the background image
    Easy_Asset_t *asset = loadAsset("assets/grafica/menu.png");
    drawAsset(0,0, asset);

    normal.size = 42;
    highlight.size = 42;

    // Draw menu options
    int startY = (SCREEN_HEIGHT - MENU_OPTIONS_COUNT ) / 2;
    for (int i = 0; i < MENU_OPTIONS_COUNT; ++i) {
        int x = SCREEN_WIDTH / 2;
        int y = startY + i * 100;
        bool isSelected = (i == selectedOption);
        if(isSelected){
            setTextStyle(&highlight);
        }else{
            setTextStyle(&normal);
        }
        drawText(
                (uint16_t ) x,(uint16_t ) y,
                (uint16_t ) SCREEN_WIDTH, (uint16_t ) 50,
                MENU_OPTIONS[i], TEXT_CENTERED
        );
    }

    // Update the screen
    SDL_RenderPresent(renderer);
}


int main(int argc, char** args) {
    bool quit = false;

    initEasySDL("Tetris 2024", SCREEN_WIDTH , SCREEN_HEIGHT,
                (uint32_t) EASY_SDL_DEFAULT_WINDOW_OPTIONS);
    normal.font = loadAsset("assets/fonts/ka1.ttf");
    highlight.font = normal.font;
    info.font = loadAsset("assets/fonts/Monaco.ttf");
    gestore_eventi =  &menu;


    while (!quit) {
        handleInput();
        if(gestore_eventi == &game){
            drawGame();
        }
        if(gestore_eventi == &menu){
            drawMenu();
        }
        if(gestore_eventi == &pause) {
            drawPause();
        }
        if(gestore_eventi == &credits){
            drawCredits();
        }
        SDL_Delay(10);
    }

}
