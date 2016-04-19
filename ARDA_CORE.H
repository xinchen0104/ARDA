#ifndef GAME_CORE
#define GAME_CORE

#define DEFAULT_SCREEN_WIDTH    1920
#define DEFAULT_SCREEN_HEIGHT   1080

#define DEFAULT_SCREEN_BPP      32

#define SCREEN_FULLSCREEN       0
#define SCREEN_WINDOWED         1

#define _RGB32(a,r,g,b)         (uint32_t)((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))

#define ERROR_LOG(msg)          fprintf(stderr,"\n msg:  %s\n",SDL_GetError())

#define ERROR_CUSTOM            1001

#define SWAP(a,b,t)             {t=a; a=b; b=t;}

typedef int32_t ARDA_RESULT;

ARDA_RESULT ARDA_Init(void);
ARDA_RESULT ARDA_Shutdown(void);
#else

#endif
