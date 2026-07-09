/* shim: cutils/properties.h -> getenv */
#ifndef _SHIM_CUTILS_PROPERTIES_H
#define _SHIM_CUTILS_PROPERTIES_H
#include <stdlib.h>
#include <string.h>
#define PROPERTY_KEY_MAX 32
#define PROPERTY_VALUE_MAX 92
static inline int property_get(const char *key, char *value, const char *default_value)
{
    const char *v = getenv(key);
    if (!v || !*v) v = default_value ? default_value : "";
    strncpy(value, v, PROPERTY_VALUE_MAX - 1);
    value[PROPERTY_VALUE_MAX - 1] = 0;
    return (int)strlen(value);
}
static inline int property_set(const char *key, const char *value)
{
    return setenv(key, value, 1);
}
#endif
