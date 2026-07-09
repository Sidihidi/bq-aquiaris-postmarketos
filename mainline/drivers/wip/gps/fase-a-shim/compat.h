/* shim bionic/legacy -> musl/moderno */
#ifndef _SHIM_COMPAT_H
#define _SHIM_COMPAT_H
#include <signal.h>
#include <string.h>
/* sys_siglist se retiro de glibc/musl modernos; solo se usa en logs */
static const char *const __shim_siglist[65] = { "sig" };
#define sys_siglist __shim_siglist
/* declaraciones que faltan (viven en libagent_6628.a / libsupl.a) */
extern int mtk_agps_agent_epo_file_update(void);
extern int mtk_agps_agent_epo_read_utc_time(void *start, void *expire);
extern int SUPL_encrypt(void *in, void *out, unsigned int len);
extern int SUPL_decrypt(void *in, void *out, unsigned int len);
#endif
/* mnl_common_6620.c usa property_get sin incluir el header */
#include <cutils/properties.h>
