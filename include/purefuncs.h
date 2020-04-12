#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#include <hollywood/types.h>

#ifdef HW_AMIGA

void *pure_malloc(size_t size);
void *pure_realloc(void *memblock, size_t size);
void *pure_calloc(size_t num, size_t size);
void pure_free(void *ptr);
ldiv_t pure_ldiv (long int numer, long int denom);
size_t pure_strlen(const char *src);
char *pure_strcat(char *dst, const char *src);
int pure_strcmp(const char *s1, const char *s2);
char *pure_strcpy(char *dst, const char *src);
char *pure_strdup(const char *str);
FILE *pure_fopen(char *filename, char *modes);
int pure_fclose(FILE *stream);
int pure_fseek(FILE *fp, long offset, int how);
long pure_ftell(FILE *fp);
int pure_ferror(FILE *fp);
int pure_feof(FILE *fp);
size_t pure_fread(void *buf, size_t objsize, size_t nobjs, FILE *fp);
size_t pure_fwrite(const void *buf, size_t objsize, size_t nobjs, FILE *fp);
int pure_snprintf(char *buffer, size_t count, const char *format, ...);
int pure_vsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
int pure_printf(const char *format, ...);
void pure_qsort(void *array, size_t num, size_t size, int (*cmpfunc)(const void *arg1, const void *arg2));
int pure_sscanf(const char *str, const char *ctl, ...);
double pure_strtod(const char *s, char **tp);
long pure_strtol(const char *str, char **tail, int base);
int pure_gettimeofday(void *tv, void *restrict);
int pure_toupper(int c);

#else

#if defined(_MSC_VER)
#define pure_snprintf _snprintf
#define pure_vsnprintf _vsnprintf
#else
#define pure_snprintf snprintf
#define pure_vsnprintf vsnprintf
#endif

#define pure_malloc malloc
#define pure_free free
#define pure_ldiv ldiv
#define pure_realloc realloc
#define pure_calloc calloc
#define pure_fopen fopen
#define pure_fclose fclose
#define pure_fread fread
#define pure_feof feof
#define pure_ferror ferror
#define pure_fwrite fwrite
#define pure_strlen strlen
#define pure_strcat strcat
#define pure_strcmp strcmp
#define pure_strcpy strcpy
#define pure_strtod strtod
#define pure_strtol strtol
#define pure_sscanf sscanf
#define pure_toupper toupper

#endif

#define pure_printf printf

#ifdef __VBCC__
void pure_qsort(void *array, size_t num, size_t size, int (*cmpfunc)(const void *arg1, const void *arg2));
char *mystrdup(const char *s1);
#define pure_strdup(str) mystrdup(str)
#else
#define pure_qsort qsort
#define pure_strdup(str) strdup(str)
#endif

int pure_gettimeofday(void *tv, void *restrict);

long int pure_lrint(double x);
long pure_lrintf(float x);
double pure_rint(double x);

int validate(const char *s);
int getnextchar(const char *s, int *idx);
void composechar(char *s, int ch);
