#ifndef GAME_CORE
#define GAME_CORE

#define DEFAULT_SCREEN_WIDTH    1920
#define DEFAULT_SCREEN_HEIGHT   1080

#define DEFAULT_SCREEN_BPP      32

#define SCREEN_FULLSCREEN       0
#define SCREEN_WINDOWED         1

#define _ARGB32(a,r,g,b)         (uint32_t)((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))
#define _RGBA32(r,g,b,a)         (uint32_t)((a) + ((b) << 8) + ((g) << 16) + ((r) << 24))

#define SET_BIT(word,bit_flag)   ((word)=((word) | (bit_flag)))
#define RESET_BIT(word,bit_flag) ((word)=((word) & (~bit_flag)))

#define ERROR_CUSTOM            1001

#define SWAP(a,b,t)             {t=a; a=b; b=t;}

#define SIGN(x) ((x) > 0 ? (1) : (-1))

typedef int32_t ARDA_RESULT;
//typedef unsigned char UCHAR;

ARDA_RESULT ARDA_Init(void);
ARDA_RESULT ARDA_Shutdown(void);
inline void MemSetDWord(void* dest, uint32_t data, uint32_t count);

uint32_t Get_Clock(void);
uint32_t Start_Clock(void);
uint32_t Wait_Clock(uint32_t count);

inline void MemSetDWord(void* dest, uint32_t data, uint32_t count){
        asm(
                "movq %0, %%rdi\n\t"
                "movl %2, %%ecx\n\t"
                "movl %1, %%eax\n\t"
                "rep\n\t"
                "stosl"
                ::"m"(dest),"ir"(data),"ir"(count):"rdi","ecx","eax"
        );
}

#else

#endif
