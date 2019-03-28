
#define MALLOC_LOGGING 1

#include DEVKITPPC_STDLIB_INCLUDE
#include <string.h>

#if MALLOC_LOGGING

#ifdef __cplusplus
extern "C" {
#endif

#undef free
#undef malloc
#undef calloc
#undef realloc
#undef strdup

#define __fbpadding(width) ((u16)(((u16)(width) + 15) & ~15))
#define _SYS_AllocateFramebuffer(rmode) \
	my_memalign(32,(__fbpadding((rmode)->fbWidth)*(rmode)->xfbHeight*2), __FILE__, __LINE__)

#define free(a) my_free(a, __FILE__, __LINE__)
#define malloc(a) my_malloc(a, __FILE__, __LINE__)
#define calloc(a, b) my_calloc(a, b, __FILE__, __LINE__)
#define realloc(a, b) my_realloc(a, b, __FILE__, __LINE__)
#define memalign(a, b) my_memalign(a, b, __FILE__, __LINE__)
#define strdup(a) my_strdup(a, __FILE__, __LINE__)

extern void my_free(void *buf, const char *file, int line);
extern void *my_malloc(int size, const char *file, int line);
extern void *my_calloc(int num, int size, const char *file, int line);
extern void *my_realloc(void *buf, int size, const char *file, int line);
extern void *my_memalign(int align, int size, const char *file, int line);
extern char *my_strdup(const char *str, const char *file, int line);

extern void allocLogStart(void);
extern void allocLogStop(void);
extern void allocLogSetMarker(void);
extern void allocLogPrint(void);

#ifdef __cplusplus
}
#endif
#else
#define _SYS_AllocateFramebuffer SYS_AllocateFramebuffer
#define allocLogStart()
#define allocLogStop()
#define allocLogSetMarker()
#define allocLogPrint()
#endif

