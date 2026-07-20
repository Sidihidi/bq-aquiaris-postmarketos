/* Alias para los thunks memcpy.N/memset.N que ld64-274.2 genera durante el link
 * (interworking ARM). Apuntan a las implementaciones reales en bcopy.s / bzero.s. */
.text
.globl _memcpy.2
.globl _memcpy.417
.globl _memset.188
.set _memcpy.2, _memcpy
.set _memcpy.417, _memcpy
.set _memset.188, _memset
