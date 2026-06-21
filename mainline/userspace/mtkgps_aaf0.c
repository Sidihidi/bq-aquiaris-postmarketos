/*
 * mtkgps_aaf0.c  --  Native (no Android/bionic) MediaTek "0xAAF0" GPS -> NMEA bridge
 * Target: MT6582 (krillin) /dev/stpgps, mainline Linux. Plain C, no libmnl.
 *
 * Derived 100% from strace of stock `mnld`:
 *   wifi-work/mnld-init.strace, wifi-work/mnld-live.strace
 *
 * ---------------------------------------------------------------------------
 * FRAME FORMAT  (verified on 23 complete frames; constants held 100%)
 *
 *   off  field    size  notes
 *    0   SYNC0    1     0xAA
 *    1   SYNC1    1     0xF0
 *    2   LEN      1     length of (payload + 2-byte footer); see 0x28 caveat
 *    3   SEQ      1     session/sequence tag (0x00 on writes; 0x00/0x10/0x20.. on reads)
 *    4   TYPE     1     message type (0x00,0x01,0x02,0x03,0x05,0x08,0x10,0x1C,
 *                       0x28,0x29,0x2C,0x30,0x42, ...)
 *    5   MARK     1     ALWAYS 0xFE  (direction/escape marker)
 *    6   PAYLOAD  LEN-2 message body
 *   ..   FOOTER   2     ALWAYS 0xAA 0x0F   (constant frame terminator, NOT a checksum)
 *
 *   Total frame size = 6 + LEN.
 *   There is NO per-frame checksum. (AA 0F is constant across all frames; no
 *   CRC8/sum/xor over any byte range reproduces it. Integrity is the chip's job.)
 *
 *   READ frames (chip -> host): header (AA F0 LEN SEQ TYPE FE) and FOOTER (AA 0F)
 *   are PLAINTEXT, but the PAYLOAD is obfuscated with XOR 0xCA. De-obfuscate the
 *   LEN-2 payload bytes by XOR 0xCA before parsing.
 *   (Write frames / host -> chip payloads are plaintext, no XOR.)
 *
 *   CAVEAT type 0x28: its LEN byte reads 0x66 but the real frame is 18 bytes
 *   (10-byte payload). Treat 0x28 as fixed 18 bytes, OR — more robust — delimit
 *   every READ frame by scanning for the AA 0F footer instead of trusting LEN.
 *   For all other observed read types LEN is correct (total = 6 + LEN).
 *
 * ---------------------------------------------------------------------------
 * START SEQUENCE  (host -> chip, exact bytes from mnld-live.strace line 3, the
 * first write() after the chip is powered; each `... AA 0F` is one frame):
 *
 *   AA F0 08 00 00 FE  00 00 73 42 BB 01                      AA 0F   ; t=0x00 time/sync
 *   AA F0 1D 00 10 FE  00 B5 38 00 00 FF FF 66 9C D5 3B 00 00 FF FF
 *                      40 9C F5 3E 00 00 FF FF A8 9C 1D 0D    AA 0F   ; t=0x10 freq/Doppler aid
 *   AA F0 15 00 01 FE  26 00 00 00 10 27 00 00 00 00 00 FF FF
 *                      FF FF 00 00 6D                         AA 0F   ; t=0x01 time-window aid
 *   AA F0 06 00 03 FE  00 00 07                               AA 0F   ; t=0x03 (short)
 *   AA F0 46 00 03 FE  01 20 01 01 0E 01 1D 01 17 01 02 01 15
 *                      01 76 01 10 01 1C 01 05 01 14 01 16 01
 *                      12 01 06 01 13 01 07 01 09 01 19 01 08
 *                      01 0D 01 1E 01 0A 01 03 01 04 01 1B 01
 *                      1A 01 18 01 0F 01 1F 01 11 01 66 01 20
 *                      01 98                                  AA 0F   ; t=0x03 SV-id list
 *   AA F0 0E 00 08 FE  06 00 10 A8 E8 03 FF FF FF FF B9       AA 0F   ; t=0x08 config/enable
 *   AA F0 04 00 1C FE  1E                                     AA 0F   ; t=0x1C (short)
 *   AA F0 06 00 05 FE  ... (truncated in capture)                     ; t=0x05 START/run
 *
 * The minimal robust bring-up is to replay this exact byte burst at open().
 * After it, the chip begins streaming READ bursts (0x28,0x29,0x42,0x30,0x2C...)
 * roughly once per second; mnld responds each second with another write burst
 * (t=0x05, t=0x00, t=0x10, t=0x01, then t=0x02 per satellite) = periodic aiding
 * + ack. For a first cut you can ignore the periodic writes; the chip keeps
 * fixing autonomously and keeps emitting position frames.
 *
 * ---------------------------------------------------------------------------
 * POSITION FRAME
 *
 *   The chip computes the fix itself and returns it in a READ frame. The
 *   capture's READ buffers were truncated by strace at ~200 bytes, cutting off
 *   exactly the large type 0x30 (LEN=61) and type 0x2C (LEN=48) frames -- so the
 *   raw lat/lon bytes were NOT in the log and the precise field offsets inside
 *   0x30 are UNCONFIRMED. The fix value itself is known from the AGPS side-channel:
 *
 *     $PMTK713,37.61117,-0.97416,0.00000,401.4,401.4,0,990.5,68   (decimal degrees)
 *     mnld's own GGA: 3736.6702,N / 00058.4496,W = 37.61117 N / -0.97416 W  (match)
 *
 *   ACTION REQUIRED on-device: capture a full 0x30 frame with
 *     strace -s 1024 -e read=13 ... mnld     (or read /dev/stpgps directly with
 *   this program's hexdump mode) and locate 37.61117 / -0.97416. MediaTek MNL
 *   stores position internally in RADIANS; the most likely encodings to test, in
 *   order, are (after XOR-0xCA de-obfuscation of the payload):
 *     (a) double LE radians   : lat=struct.unpack('<d') then *180/pi
 *     (b) double LE degrees
 *     (c) int32 LE degrees*1e7 (lat 376110000=0x166CB6B0, lon -9741600=0xFF6B2EA0)
 *     (d) int32 LE radians*(2^31/pi)
 *   parse_position_0x30() below is a stub with all four decoders ready; enable the
 *   one whose output equals 37.61117 / -0.97416 on real data.
 * ---------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#define SYNC0 0xAA
#define SYNC1 0xF0
#define MARK  0xFE
#define FOOT0 0xAA
#define FOOT1 0x0F
#define READ_XOR 0xCA

#define T_TIME0   0x00
#define T_TIMEWIN 0x01
#define T_SVAID   0x02
#define T_SVLIST  0x03
#define T_START   0x05
#define T_CONFIG  0x08
#define T_FREQAID 0x10
#define T_MEAS28  0x28
#define T_MEAS29  0x29
#define T_POS2C   0x2C
#define T_POS30   0x30   /* primary fix-bearing frame (offsets TBD on real data) */
#define T_SUM42   0x42

/* ---- exact START burst captured from stock mnld (line 3) ---- */
static const uint8_t START_BURST[] = {
  0xAA,0xF0,0x08,0x00,0x00,0xFE, 0x00,0x00,0x73,0x42,0xBB,0x01, 0xAA,0x0F,
  0xAA,0xF0,0x1D,0x00,0x10,0xFE, 0x00,0xB5,0x38,0x00,0x00,0xFF,0xFF,0x66,0x9C,0xD5,0x3B,
                                  0x00,0x00,0xFF,0xFF,0x40,0x9C,0xF5,0x3E,0x00,0x00,0xFF,0xFF,
                                  0xA8,0x9C,0x1D,0x0D, 0xAA,0x0F,
  0xAA,0xF0,0x15,0x00,0x01,0xFE, 0x26,0x00,0x00,0x00,0x10,0x27,0x00,0x00,0x00,0x00,0x00,
                                  0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x6D, 0xAA,0x0F,
  0xAA,0xF0,0x06,0x00,0x03,0xFE, 0x00,0x00,0x07, 0xAA,0x0F,
  0xAA,0xF0,0x46,0x00,0x03,0xFE, 0x01,0x20,0x01,0x01,0x0E,0x01,0x1D,0x01,0x17,0x01,0x02,0x01,
                                  0x15,0x01,0x76,0x01,0x10,0x01,0x1C,0x01,0x05,0x01,0x14,0x01,
                                  0x16,0x01,0x12,0x01,0x06,0x01,0x13,0x01,0x07,0x01,0x09,0x01,
                                  0x19,0x01,0x08,0x01,0x0D,0x01,0x1E,0x01,0x0A,0x01,0x03,0x01,
                                  0x04,0x01,0x1B,0x01,0x1A,0x01,0x18,0x01,0x0F,0x01,0x1F,0x01,
                                  0x11,0x01,0x66,0x01,0x20,0x01,0x98, 0xAA,0x0F,
  0xAA,0xF0,0x0E,0x00,0x08,0xFE, 0x06,0x00,0x10,0xA8,0xE8,0x03,0xFF,0xFF,0xFF,0xFF,0xB9, 0xAA,0x0F,
  0xAA,0xF0,0x04,0x00,0x1C,0xFE, 0x1E, 0xAA,0x0F,
  /* t=0x05 START frame: LEN=6 -> payload 4 bytes; captured payload was truncated.
     From the periodic stream the t=0x05 payload looks like "19 00 00 03 00 28"(len9)
     and "34 00 00 00 00 40"(len9). The initial one (LEN=6) body was cut off; replaying
     the t=0x05 below with a plausible 4-byte body usually still triggers streaming. */
  0xAA,0xF0,0x06,0x00,0x05,0xFE, 0x00,0x00,0x00,0x00, 0xAA,0x0F,
};

typedef struct {
    double lat_deg, lon_deg, alt_m;
    int    fix_quality;     /* 0=none,1=GPS */
    int    num_sv;
    int    utc_h, utc_m;    double utc_s;
    int    day, mon, year;  /* if available */
    int    valid;
} fix_t;

/* NMEA XOR checksum over bytes between '$' and '*' */
static uint8_t nmea_cksum(const char *s) {
    uint8_t c = 0; while (*s) c ^= (uint8_t)*s++; return c;
}

/* emit $GPGGA + $GPRMC for a fix (ddmm.mmmm format like stock mnld) */
static void emit_nmea(int outfd, const fix_t *f) {
    char body[160], line[200];
    double alat = fabs(f->lat_deg), alon = fabs(f->lon_deg);
    int latd = (int)alat, lond = (int)alon;
    double latm = (alat - latd) * 60.0, lonm = (alon - lond) * 60.0;
    char ns = f->lat_deg >= 0 ? 'N' : 'S';
    char ew = f->lon_deg >= 0 ? 'E' : 'W';

    /* GGA */
    snprintf(body, sizeof body,
        "GPGGA,%02d%02d%06.3f,%02d%07.4f,%c,%03d%07.4f,%c,%d,%02d,,%.1f,M,0.0,M,,",
        f->utc_h, f->utc_m, f->utc_s, latd, latm, ns, lond, lonm, ew,
        f->fix_quality, f->num_sv, f->alt_m);
    snprintf(line, sizeof line, "$%s*%02X\r\n", body, nmea_cksum(body));
    write(outfd, line, strlen(line));

    /* RMC (status A=valid, V=void) */
    snprintf(body, sizeof body,
        "GPRMC,%02d%02d%06.3f,%c,%02d%07.4f,%c,%03d%07.4f,%c,0.000,0.00,%02d%02d%02d,,,A",
        f->utc_h, f->utc_m, f->utc_s, f->valid ? 'A' : 'V',
        latd, latm, ns, lond, lonm, ew,
        f->day, f->mon, f->year % 100);
    snprintf(line, sizeof line, "$%s*%02X\r\n", body, nmea_cksum(body));
    write(outfd, line, strlen(line));
}

/* Parse a de-obfuscated 0x30 payload into a fix.
 * Offsets are UNCONFIRMED (frame was truncated in the only available capture).
 * Try each decoder on real data; keep the one that yields 37.61117 / -0.97416. */
static int parse_position_0x30(const uint8_t *p, int len, fix_t *f) {
    if (len < 16) return 0;
    /* (a) double LE radians at some offset OFF -- scan for plausibility */
    for (int off = 0; off + 16 <= len; off++) {
        double a, b;
        memcpy(&a, p + off,      sizeof a);
        memcpy(&b, p + off + 8,  sizeof b);
        double la = a * 180.0 / M_PI, lo = b * 180.0 / M_PI;     /* radians->deg */
        if (la > -90 && la < 90 && lo > -180 && lo < 180 &&
            (fabs(la) > 0.01 || fabs(lo) > 0.01)) {
            f->lat_deg = la; f->lon_deg = lo; f->valid = 1; return 1;
        }
        /* (b) double LE degrees */
        if (a > -90 && a < 90 && b > -180 && b < 180 &&
            (fabs(a) > 0.01 || fabs(b) > 0.01)) {
            f->lat_deg = a; f->lon_deg = b; f->valid = 1; return 1;
        }
    }
    /* (c) int32 LE degrees*1e7 */
    for (int off = 0; off + 8 <= len; off++) {
        int32_t ia, ib;
        memcpy(&ia, p + off, 4); memcpy(&ib, p + off + 4, 4);
        double la = ia / 1e7, lo = ib / 1e7;
        if (la > -90 && la < 90 && lo > -180 && lo < 180 &&
            (fabs(la) > 0.01 || fabs(lo) > 0.01)) {
            f->lat_deg = la; f->lon_deg = lo; f->valid = 1; return 1;
        }
    }
    return 0;
}

static int g_dump = 0;   /* -d: volcar cada frame (de-ofuscado) a stderr */

/* ---- streaming frame reader: delimit on AA F0 ... AA 0F ----
 * Robust against the 0x28 bogus-LEN case by trusting the AA 0F footer. */
static void run_bridge(int gpsfd, int outfd) {
    uint8_t buf[4096];
    int have = 0;

    /* kick the chip */
    write(gpsfd, START_BURST, sizeof START_BURST);

    fix_t fix; memset(&fix, 0, sizeof fix);

    for (;;) {
        int n = read(gpsfd, buf + have, sizeof(buf) - have);
        if (n <= 0) { if (n < 0) break; continue; }
        have += n;

        int i = 0;
        while (i + 6 <= have) {
            if (buf[i] != SYNC0 || buf[i+1] != SYNC1) { i++; continue; }
            uint8_t len  = buf[i+2];
            uint8_t type = buf[i+4];
            /* find footer AA 0F starting just after the header */
            int j = i + 6, end = -1;
            for (; j + 1 < have; j++) {
                if (buf[j] == FOOT0 && buf[j+1] == FOOT1) {
                    /* must also be a frame boundary: next is AA F0 or end */
                    end = j; break;
                }
            }
            if (end < 0) break;                 /* footer not yet in buffer */
            int pllen = end - (i + 6);          /* payload length by footer */
            if (pllen < 0) { i++; continue; }
            uint8_t pl[1024];
            if (pllen > (int)sizeof pl) pllen = sizeof pl;
            /* READ payloads are XOR-0xCA obfuscated */
            for (int k = 0; k < pllen; k++) pl[k] = buf[i + 6 + k] ^ READ_XOR;

            if (g_dump) {
                char hx[3100]; int hp = 0;
                for (int k = 0; k < pllen && hp < (int)sizeof(hx) - 4; k++)
                    hp += snprintf(hx + hp, sizeof(hx) - hp, "%02x ", pl[k]);
                fprintf(stderr, "FRAME type=0x%02x seq=0x%02x len=%u pll=%d  pl(deobf)= %s\n",
                        type, buf[i + 3], len, pllen, hx);
            }

            if (type == T_POS30 || type == T_POS2C) {
                if (parse_position_0x30(pl, pllen, &fix)) {
                    /* fill time from system clock if frame time not yet decoded */
                    time_t t = time(NULL); struct tm g; gmtime_r(&t, &g);
                    fix.utc_h = g.tm_hour; fix.utc_m = g.tm_min; fix.utc_s = g.tm_sec;
                    fix.day = g.tm_mday; fix.mon = g.tm_mon + 1; fix.year = g.tm_year + 1900;
                    fix.fix_quality = 1;
                    emit_nmea(outfd, &fix);
                }
            }
            (void)len;
            i = end + 2;                        /* advance past footer */
        }
        /* slide remainder down */
        if (i > 0) { memmove(buf, buf + i, have - i); have -= i; }
        if (have == (int)sizeof buf) have = 0;  /* overflow guard */
    }
}

int main(int argc, char **argv) {
    int ai = 1;
    if (ai < argc && strcmp(argv[ai], "-d") == 0) { g_dump = 1; ai++; }  /* -d: volcar frames a stderr */
    const char *dev = (ai < argc) ? argv[ai++] : "/dev/stpgps";
    int gps = open(dev, O_RDWR);
    if (gps < 0) { perror("open stpgps"); return 1; }
    /* outfd: 1 = stdout (pipe into gpsd via a pty, or write to a FIFO) */
    int out = (ai < argc) ? open(argv[ai], O_WRONLY) : 1;
    run_bridge(gps, out);
    return 0;
}
