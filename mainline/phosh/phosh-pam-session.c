/*
 * phosh-pam-session — abre una sesión elogind ACTIVA para sxmo y lanza Phosh (krillin / MT6582).
 *
 * El problema: `su`=busybox no tiene PAM, así que el `su - sxmo` que lanzaba Phosh nunca creaba
 * una sesión de logind → gsd-power/UPower/suspend no tenían sesión. Pero `pam_elogind.so` sí existe.
 *
 * Este helper hace pam_open_session (con pam_elogind) y luego exec del lanzador de Phosh.
 *   - ★ XDG_VTNR=1 es IMPRESCINDIBLE: sin él pam_open_session da rc=14; con él rc=0.
 *   - pam_getenvlist()->putenv() propaga XDG_SESSION_ID a phoc (lo necesita libseat-logind).
 *   - launch_phosh.sh debe exportar LIBSEAT_BACKEND=logind (phoc usa la sesión elogind de asiento).
 *
 * Build (en el móvil): apk add linux-pam-dev; gcc -O2 -o /usr/local/bin/phosh-pam-session phosh-pam-session.c -lpam
 * Requiere /etc/pam.d/phosh-session:
 *     auth     required pam_permit.so
 *     account  required pam_permit.so
 *     session  required pam_elogind.so
 *
 * NOTA (2026-06-24): la sesión sale Active=yes, pero el SLIDER de brillo sigue sin ir — gsd-power
 * en este build delega el brillo de pantalla a org.gnome.Shell.Brightness (ausente en Phosh) y no
 * expone .Screen. Es problema del source de Phosh, no de la sesión. La sesión activa SÍ es la base
 * para batería/suspend/power UI. Brillo usable hoy: comando `bl`.
 */
#include <security/pam_appl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

static int cv(int n, const struct pam_message **m, struct pam_response **r, void *d)
{
    *r = calloc(n, sizeof(struct pam_response));
    return PAM_SUCCESS;
}

int main(int argc, char **argv)
{
    pam_handle_t *ph;
    struct pam_conv pc = { cv, NULL };
    if (pam_start("phosh-session", "sxmo", &pc, &ph) != PAM_SUCCESS) {
        fprintf(stderr, "pam_start fail\n");
        return 2;
    }
    pam_set_item(ph, PAM_TTY, "seat0");
    pam_putenv(ph, "XDG_SEAT=seat0");
    pam_putenv(ph, "XDG_VTNR=1");          /* <-- el que faltaba */
    pam_putenv(ph, "XDG_SESSION_TYPE=wayland");
    pam_putenv(ph, "XDG_SESSION_CLASS=user");

    int rc = pam_open_session(ph, 0);
    fprintf(stderr, "open_session rc=%d (%s)\n", rc, pam_strerror(ph, rc));
    if (rc != PAM_SUCCESS)
        return 3;

    char **pe = pam_getenvlist(ph);        /* propaga XDG_SESSION_ID, etc. a phoc */
    for (int i = 0; pe && pe[i]; i++)
        putenv(pe[i]);

    struct passwd *pw = getpwnam("sxmo");
    initgroups("sxmo", pw->pw_gid);
    setgid(pw->pw_gid);
    setuid(pw->pw_uid);
    setenv("HOME", pw->pw_dir, 1);
    setenv("USER", "sxmo", 1);
    setenv("LOGNAME", "sxmo", 1);
    chdir(pw->pw_dir);

    const char *cmd = argc > 1 ? argv[1] : "sh /usr/local/bin/launch_phosh.sh";
    execl("/bin/sh", "sh", "-c", cmd, (char *)0);
    return 1;
}
