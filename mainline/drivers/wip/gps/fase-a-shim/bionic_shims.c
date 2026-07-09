/* shims bionic->glibc/musl para linkar libmnl_6628.a (Fase A GPS) */
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

/* ---- bionic libc ---- */
size_t __strlen_chk(const char *s, size_t buf_size) { (void)buf_size; return strlen(s); }
int *__errno(void) { return &errno; }

/* ---- KAL: mutex del porting layer (mtk_gps.h: INT32 fn(MTK_GPS_MUTEX_ENUM)) ---- */
#define MTK_GPS_MUTEX_MAX 32
static pthread_mutex_t mnl_mutexes[MTK_GPS_MUTEX_MAX];
int mtk_gps_sys_create_mutex(int mutex_idx)
{
    if (mutex_idx < 0 || mutex_idx >= MTK_GPS_MUTEX_MAX) return -1; /* MTK_GPS_ERROR */
    pthread_mutex_init(&mnl_mutexes[mutex_idx], NULL);
    return 0; /* MTK_GPS_SUCCESS */
}
int mtk_gps_sys_destroy_mutex(int mutex_idx)
{
    if (mutex_idx < 0 || mutex_idx >= MTK_GPS_MUTEX_MAX) return -1;
    pthread_mutex_destroy(&mnl_mutexes[mutex_idx]);
    return 0;
}
/* por si el blob tambien llama take/give sin implementarlos el glue */
int mtk_gps_sys_take_mutex(int mutex_idx)
{
    if (mutex_idx < 0 || mutex_idx >= MTK_GPS_MUTEX_MAX) return -1;
    pthread_mutex_lock(&mnl_mutexes[mutex_idx]);
    return 0;
}
int mtk_gps_sys_give_mutex(int mutex_idx)
{
    if (mutex_idx < 0 || mutex_idx >= MTK_GPS_MUTEX_MAX) return -1;
    pthread_mutex_unlock(&mnl_mutexes[mutex_idx]);
    return 0;
}
