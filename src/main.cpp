#ifdef BUILD_OSX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include "res_path.h"

#ifdef BUILD_WIN32
#include <windows.h>
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TOP_SCREEN_MARGIN 40
#define LEFT_SCREEN_MARGIN 100
#define TILE_PIXEL_SIZE 120

enum TileValue {
  EMPTY_TILE = 0, COMPUTER_TILE, PLAYER_TILE
};

enum  GameEndStatus {  
  NO_END = 0, DRAW_END, COMPUTER_WINS_END, PLAYER_WINS_END
};

struct PlayerInput {
  bool keyPressed[3][3];
};
  
struct GameState {
  int freeTilesCount;
  TileValue board[3][3];
  GameEndStatus endStatus;
  bool running;
};

struct SpriteSheet {
  SDL_Texture * texture;
  SDL_Rect clips[7];
};



SDL_Texture *
sdlLoadTexture(char* resourcePath, char* filename, SDL_Renderer *ren) {

  char* imagePath  = (char*) malloc(strlen(resourcePath) + strlen(filename) + 1);
  if (imagePath) {
    strcpy(imagePath, resourcePath);
    strcat(imagePath, filename);
  } else {
    fprintf(stderr,"malloc failed!\n");
    return 0;
  }

  SDL_Surface *bmp = SDL_LoadBMP(imagePath);
  free(imagePath);
  if (bmp == 0) {
    fprintf(stderr, "SDL_LoadBMP Error: %s\n", SDL_GetError());
    return 0;
  }

  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
  SDL_FreeSurface(bmp);

  return tex;
}

static void
sdlRenderGame(GameState* gameState, SDL_Renderer *ren, SpriteSheet* spriteSheet) {

  SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(ren);

  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      int screenX = LEFT_SCREEN_MARGIN + column * TILE_PIXEL_SIZE;
      int screenY = TOP_SCREEN_MARGIN + row * TILE_PIXEL_SIZE;
      SDL_Rect dstrect = {screenX, screenY, TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};
      SDL_Rect* srcrect;
      switch (gameState->board[row][column]) {
        case COMPUTER_TILE: {
          srcrect = &spriteSheet->clips[5];

        } break;

        case PLAYER_TILE: {
          srcrect = &spriteSheet->clips[2];

        } break;

        default: {
          srcrect = &spriteSheet->clips[6];
        }
      }
      SDL_RenderCopy(ren, spriteSheet->texture, srcrect, &dstrect);
    }
  }
  SDL_RenderPresent(ren);
}

static void
gameUpdateStatus(GameState* gameState) {

  for (int row = 0; row < 3; ++row) {
    if (gameState->board[row][0] != EMPTY_TILE &&
        gameState->board[row][0] == gameState->board[row][1] &&
        gameState->board[row][0] == gameState->board[row][2]) {
      gameState->endStatus = (gameState->board[row][0] == COMPUTER_TILE)
                           ? COMPUTER_WINS_END : PLAYER_WINS_END;
      return;
    } 
  }
  for (int column = 0; column < 3; ++column) {
    if (gameState->board[0][column] != EMPTY_TILE &&
        gameState->board[0][column] == gameState->board[1][column] &&
        gameState->board[0][column] == gameState->board[2][column]) {
      gameState->endStatus = (gameState->board[0][column] == COMPUTER_TILE)
                           ? COMPUTER_WINS_END : PLAYER_WINS_END;
      return;
    } 
  }
  if (gameState->board[0][0] != EMPTY_TILE &&
      gameState->board[0][0] == gameState->board[1][1] &&
      gameState->board[0][0] == gameState->board[2][2]) {
    gameState->endStatus = (gameState->board[0][0] == COMPUTER_TILE)
                         ? COMPUTER_WINS_END : PLAYER_WINS_END;
    return;
  }
  if (gameState->board[0][2] != EMPTY_TILE &&
      gameState->board[0][2] == gameState->board[1][1] &&
      gameState->board[0][2] == gameState->board[2][0]) {
    gameState->endStatus = (gameState->board[0][2] == COMPUTER_TILE)
                         ? COMPUTER_WINS_END : PLAYER_WINS_END;
    return;
  }
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      if (gameState->board[row][column] == EMPTY_TILE) {
        gameState->endStatus = NO_END;
        return;
      }
    }
  }
  gameState->endStatus = DRAW_END;
}

static bool
gameUpdateLineMoveRow(GameState* gameState, TileValue tileValue, 
                                int row, int columnA, int columnB) {
  int columnC = 3 - columnA - columnB;
  if (gameState->board[row][columnA] == tileValue &&
      gameState->board[row][columnA] == gameState->board[row][columnB] &&
      gameState->board[row][columnC] == EMPTY_TILE) {
    gameState->board[row][columnC] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  return false;
}

static bool
gameUpdateLineMoveColumn(GameState* gameState, TileValue tileValue, 
                                int column, int rowA, int rowB) {
  int rowC = 3 - rowA - rowB;
  if (gameState->board[rowA][column] == tileValue &&
      gameState->board[rowA][column] == gameState->board[rowB][column] &&
      gameState->board[rowC][column] == EMPTY_TILE) {
    gameState->board[rowC][column] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  return false;
}

static bool
gameUpdateLineMoveDiagonal(GameState* gameState, TileValue tileValue, 
                                int cellA, int cellB) {
  int cellC = 3 - cellA - cellB;
  if (gameState->board[cellA][cellA] == tileValue &&
      gameState->board[cellA][cellA] == gameState->board[cellB][cellB] &&
      gameState->board[cellC][cellC] == EMPTY_TILE) {
    gameState->board[cellC][cellC] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  return false;
}

static bool
gameUpdateLineMoveInvDiagonal(GameState* gameState, TileValue tileValue, 
                                     int columnA, int columnB) {
  int columnC = 3 - columnA - columnB;
  int rowA = 2 - columnA;
  int rowB = 2 - columnB;
  int rowC = 2 - columnC;
  if (gameState->board[rowA][columnA] == tileValue &&
      gameState->board[rowA][columnA] == gameState->board[rowB][columnB] &&
      gameState->board[rowC][columnC] == EMPTY_TILE) {
    gameState->board[rowC][columnC] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  return false;
}

static bool
gameUpdateComputerCornerMove(GameState* gameState) {
  if (gameState->freeTilesCount < 8) {
    return false;
  }
  if (gameState->board[0][0] == EMPTY_TILE) {
    gameState->board[0][0] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[0][2] == EMPTY_TILE) {
    gameState->board[0][2] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[2][2] == EMPTY_TILE) {
    gameState->board[2][2] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[2][0] == EMPTY_TILE) {
    gameState->board[2][0] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  return false;
}

static bool
gameUpdatePlayerCornerMove(GameState* gameState) {
  if (gameState->freeTilesCount < 8) {
    return false;
  }
  if (gameState->board[0][0] == PLAYER_TILE) {
    gameState->board[1][1] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[0][2] == PLAYER_TILE) {
    gameState->board[1][1] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[2][2] == PLAYER_TILE) {
    gameState->board[1][1] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  if (gameState->board[2][0] == PLAYER_TILE) {
    gameState->board[1][1] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  }
  return false;
}

static bool
gameUpdateLineMove(GameState* gameState, TileValue tileValue) {
  for (int row = 0; row < 3; ++row) {
    if (gameUpdateLineMoveRow(gameState, tileValue, row, 0, 1) ||
        gameUpdateLineMoveRow(gameState, tileValue, row, 1, 2) ||
        gameUpdateLineMoveRow(gameState, tileValue, row, 0, 2)) {
      return true;
    }
  }
  for (int column = 0; column < 3; ++column) {
    if (gameUpdateLineMoveColumn(gameState, tileValue, column, 0, 1) ||
        gameUpdateLineMoveColumn(gameState, tileValue, column, 1, 2) ||
        gameUpdateLineMoveColumn(gameState, tileValue, column, 0, 2)) {
      return true;
    }
  }
  if (gameUpdateLineMoveDiagonal(gameState, tileValue, 0, 1) ||
      gameUpdateLineMoveDiagonal(gameState, tileValue, 1, 2) ||
      gameUpdateLineMoveDiagonal(gameState, tileValue, 0, 2)) {
    return true;
  }
  if (gameUpdateLineMoveInvDiagonal(gameState, tileValue, 0, 1) ||
      gameUpdateLineMoveInvDiagonal(gameState, tileValue, 1, 2) ||
      gameUpdateLineMoveInvDiagonal(gameState, tileValue, 0, 2)) {
    return true;
  }
  return false;
}

static bool
gameUpdateTrapMoveNoCenter(GameState* gameState, TileValue tileValue, 
                                  int rowA, int columnA) {

  if (gameState->board[rowA][columnA] != tileValue) {
    return false;
  }
  int rowB = 1;
  int rowC = 2;
  switch (rowA) {
    case 1: {
      rowB = 0;
      rowC = 2;
    } break;
    case 2: {
      rowB = 0;
      rowC = 1;
    } break;
  }
  int columnB = 1;
  int columnC = 2;
  switch (columnA) {
    case 1: {
      columnB = 0;
      columnC = 2;
    } break;
    case 2: {
      columnB = 0;
      columnC = 1;
    } break;
  }

  if (((gameState->board[rowA][columnB] == tileValue && gameState->board[rowA][columnC] == EMPTY_TILE) || 
       (gameState->board[rowA][columnC] == tileValue && gameState->board[rowA][columnB] == EMPTY_TILE)) &&
      gameState->board[rowB][columnA] == EMPTY_TILE &&
      gameState->board[rowC][columnA] == EMPTY_TILE) {
    gameState->board[rowB][columnA] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  if (((gameState->board[rowB][columnA] == tileValue && gameState->board[rowC][columnA] == EMPTY_TILE) || 
       (gameState->board[rowC][columnA] == tileValue && gameState->board[rowB][columnA] == EMPTY_TILE)) &&
      gameState->board[rowA][columnB] == EMPTY_TILE &&
      gameState->board[rowA][columnC] == EMPTY_TILE) {
    gameState->board[rowA][columnB] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  if (((gameState->board[rowA][columnB] == tileValue && gameState->board[rowA][columnC] == EMPTY_TILE) || 
       (gameState->board[rowA][columnC] == tileValue && gameState->board[rowA][columnB] == EMPTY_TILE)) &&
      gameState->board[rowB][columnB] == EMPTY_TILE &&
      gameState->board[rowC][columnC] == EMPTY_TILE) {
    gameState->board[rowB][columnB] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  if (((gameState->board[rowB][columnB] == tileValue && gameState->board[2][columnC] == EMPTY_TILE) || 
       (gameState->board[rowC][columnC] == tileValue && gameState->board[rowB][columnB] == EMPTY_TILE)) &&
      gameState->board[rowA][columnB] == EMPTY_TILE &&
      gameState->board[rowA][columnC] == EMPTY_TILE) {
    gameState->board[rowA][columnB] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  if (((gameState->board[rowB][columnA] == tileValue && gameState->board[rowC][columnA] == EMPTY_TILE) || 
       (gameState->board[rowC][columnA] == tileValue && gameState->board[rowB][columnA] == EMPTY_TILE)) &&
      gameState->board[rowB][columnB] == EMPTY_TILE &&
      gameState->board[rowC][columnC] == EMPTY_TILE) {
    gameState->board[rowB][columnB] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 
  if (((gameState->board[rowB][columnB] == tileValue && gameState->board[rowC][columnC] == EMPTY_TILE) || 
       (gameState->board[rowC][columnC] == tileValue && gameState->board[rowB][columnB] == EMPTY_TILE)) &&
      gameState->board[rowB][columnA] == EMPTY_TILE &&
      gameState->board[rowC][columnA] == EMPTY_TILE) {
    gameState->board[rowB][columnA] = COMPUTER_TILE;  
    --gameState->freeTilesCount;  
    return true;
  } 

  return false;
}

static bool
gameUpdateTrapMoveCenter(GameState* gameState, TileValue tileValue) {
  return false;
}

static bool
gameUpdateTrapMove(GameState* gameState, TileValue tileValue) {
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      if ((row != 2) && (column != 2)) {
        if (gameUpdateTrapMoveNoCenter(gameState, COMPUTER_TILE, row, column)) {
          return true;
        }
      } else {
         if (gameUpdateTrapMoveCenter(gameState, COMPUTER_TILE)) {
          return true;
        }
      }
    }
  }
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      if ((row != 2) && (column != 2)) {
        if (gameUpdateTrapMoveNoCenter(gameState, PLAYER_TILE, row, column)) {
          return true;
        }
      } else {
         if (gameUpdateTrapMoveCenter(gameState, PLAYER_TILE)) {
          return true;
        }
      }
    }
  }
  return false;
}

static void
gameUpdateRandomMove(GameState* gameState) {
#ifdef BUILD_WIN32
  int randomTileIndex = rand() % gameState->freeTilesCount;
#else  
  int randomTileIndex = random() % gameState->freeTilesCount;
#endif  
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      if (gameState->board[row][column] != EMPTY_TILE) {
        continue;
      }
      if (randomTileIndex == 0) {
        gameState->board[row][column] = COMPUTER_TILE;  
        --gameState->freeTilesCount;  
        return;
      }
      --randomTileIndex;
    }
  }
}

static void
gameUpdateComputer(GameState* gameState) {
  if (gameState->freeTilesCount == 0) {
    return;
  }
  if (!gameUpdateLineMove(gameState, COMPUTER_TILE) &&
      !gameUpdateLineMove(gameState, PLAYER_TILE) &&
      !gameUpdateTrapMove(gameState, COMPUTER_TILE) &&
      !gameUpdateTrapMove(gameState, PLAYER_TILE) &&
      !gameUpdatePlayerCornerMove(gameState) &&
      !gameUpdateComputerCornerMove(gameState)) {
    gameUpdateRandomMove(gameState);
  }
  gameUpdateStatus(gameState);
}

static bool
gameUpdatePlayer(GameState* gameState, PlayerInput *input) {
  int playerMoveRow = -1;
  int playerMoveColumn = -1;
  for (int row = 0; row < 3 && playerMoveRow == -1; ++row) {
    for (int column = 0; column < 3; ++column) {
      if (input->keyPressed[row][column]) {
        playerMoveRow = row;
        playerMoveColumn = column;
        input->keyPressed[row][column] = false;
        break;
      }
    }
  }
  if (playerMoveRow < 0 || playerMoveColumn < 0) {
    return false;
  }
  assert(playerMoveRow < 3 && playerMoveColumn < 3);
  if (gameState->board[playerMoveRow][playerMoveColumn] != EMPTY_TILE) {
    return false;
  }
  gameState->board[playerMoveRow][playerMoveColumn] = PLAYER_TILE;
  --gameState->freeTilesCount;  

  gameUpdateStatus(gameState);

  return true;
}

static void
sdlHandleEvent(GameState* gameState, SDL_Event *event, PlayerInput *input) {

  printf("event->type: %x\n", event->type);

  switch (event->type) {

    case SDL_QUIT: {
      gameState->running = false;
    } break;

    case SDL_KEYDOWN: {
      SDL_Keycode keyCode = event->key.keysym.sym;

      if (event->key.repeat == 0) {
        switch (keyCode) {
          case SDLK_q: {
            input->keyPressed[0][0] = true;
          } break;
          case SDLK_w: {
            input->keyPressed[0][1] = true;
          } break;
          case SDLK_e: {
            input->keyPressed[0][2] = true;
          } break;
          case SDLK_a: {
            input->keyPressed[1][0] = true;
          } break;
          case SDLK_s: {
            input->keyPressed[1][1] = true;
          } break;
          case SDLK_d: {
            input->keyPressed[1][2] = true;
          } break;
          case SDLK_z: {
            input->keyPressed[2][0] = true;
          } break;
          case SDLK_x: {
            input->keyPressed[2][1] = true;
          } break;
          case SDLK_c: {
            input->keyPressed[2][2] = true;
          } break;
          case SDLK_ESCAPE: {
            gameState->running = false;
          } break;
        }
      }        
    } break;
  }
}

static int
sdlGameEnd(GameState* gameState, SDL_Window *win) {
  char message[1000];
  switch (gameState->endStatus) {
    case DRAW_END: {
      sprintf(message, 
              "The game ended in DRAW\nDo you want to play again?");
    } break;
    case PLAYER_WINS_END: {
      sprintf(message, 
              "The game ended with PLAYER winning\nDo you want to play again?");
    } break;
    case COMPUTER_WINS_END: {
      sprintf(message, 
              "The game ended with COMPUTER winning\nDo you want to play again?");
    } break;

    default:
      assert(false);
  }

  SDL_MessageBoxButtonData buttons[] = {
    { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "no" },
    { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes" }
  };
  SDL_MessageBoxData messageboxdata = {
    SDL_MESSAGEBOX_INFORMATION,
    win,
    "GAME OVER",
    message,
    SDL_arraysize(buttons), 
    buttons 
  };
  int buttonid = -1;
  if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
    fprintf(stderr, "SDL_ShowMessageBox Error: %s\n", SDL_GetError());
    return 1;
  }
  printf("buttonid = %d\n", buttonid);
  if (buttonid == 1) {
      *gameState = {};
      gameState->freeTilesCount = 9;  
      gameState->running = true;
      gameState->endStatus = NO_END;
  } else {
      gameState->running = false;
  }
  return 0;
}

int 
main(int, char**) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

  atexit(SDL_Quit);

  char* resPath = sdlGetResourcePath();
  if (resPath == 0) {
    fprintf(stderr, "sdlGetResourcePath Error\n");
    return 1;
  }

  SDL_Window *win = SDL_CreateWindow("Tic Tac Toe", 100, 100, 
                                     SCREEN_WIDTH, SCREEN_HEIGHT, 
                                     SDL_WINDOW_SHOWN);
  if (win == 0) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | 
                                         SDL_RENDERER_PRESENTVSYNC);
  if (ren == 0) {
    SDL_DestroyWindow(win);
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Texture* tiles = sdlLoadTexture(resPath, "tiles.bmp", ren);

  if (!tiles) {
    fprintf(stderr, "sdlLoadTexture Error: %s\n", SDL_GetError());
    return 1;
  }

  SpriteSheet spriteSheet;
  spriteSheet.texture = tiles;
  spriteSheet.clips[0].x =   0;
  spriteSheet.clips[0].y =   0;
  spriteSheet.clips[0].h =   42;
  spriteSheet.clips[0].w =   42;
  spriteSheet.clips[1].x =   39;
  spriteSheet.clips[1].y =   0;
  spriteSheet.clips[1].h =   42;
  spriteSheet.clips[1].w =   42;
  spriteSheet.clips[2].x =   78;
  spriteSheet.clips[2].y =   0;
  spriteSheet.clips[2].h =   42;
  spriteSheet.clips[2].w =   42;
  spriteSheet.clips[3].x =   0;
  spriteSheet.clips[3].y =   39;
  spriteSheet.clips[3].h =   42;
  spriteSheet.clips[3].w =   42;
  spriteSheet.clips[4].x =   39;
  spriteSheet.clips[4].y =   39;
  spriteSheet.clips[4].h =   42;
  spriteSheet.clips[4].w =   42;
  spriteSheet.clips[5].x =   78;
  spriteSheet.clips[5].y =   39;
  spriteSheet.clips[5].h =   42;
  spriteSheet.clips[5].w =   42;
  spriteSheet.clips[6].x =   0;
  spriteSheet.clips[6].y =   78;
  spriteSheet.clips[6].h =   42;
  spriteSheet.clips[6].w =   42;

#ifdef BUILD_WIN32
  srand((unsigned) time(0));
#else
  srandom(time(0));
#endif
  GameState gameState = {};
  gameState.freeTilesCount = 9;  
  gameState.running = true;
  gameState.endStatus = NO_END;
    
  PlayerInput input = {};

  while (gameState.running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      sdlHandleEvent(&gameState, &event, &input);
    }
    if (gameState.running) {
      if (gameUpdatePlayer(&gameState, &input) && 
          gameState.endStatus == NO_END) {
        gameUpdateComputer(&gameState);
      }
    
      sdlRenderGame(&gameState, ren, &spriteSheet);
      
      if (gameState.endStatus != NO_END) {
        if (sdlGameEnd(&gameState, win)) {
          return 1;
        }
      }
    }
  }
  return 0;
}