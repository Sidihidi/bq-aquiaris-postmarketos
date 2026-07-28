# 1) IOPlatformExpert: dar una fecha razonable cuando no hay RTC
p = "/home/cpcd/darwin-krillin/xnu/iokit/Kernel/IOPlatformExpert.cpp"
s = open(p).read()
old = """long PEGetGMTTimeOfDay(void)
{
	long	result = 0;

	if( gIOPlatform)		result = gIOPlatform->getGMTTimeOfDay();

	return (result);
}"""
new = """long PEGetGMTTimeOfDay(void)
{
	long	result = 0;

	if( gIOPlatform)		result = gIOPlatform->getGMTTimeOfDay();

	/*
	 * M131 (krillin): el krillin no tiene RTC cableado en este port, asi que
	 * AppleARMPE se queda con IOPlatformExpert::getGMTTimeOfDay() = 0.  Con eso
	 * clock_initialize_calendar() hace epoch = 0 - uptime (TIME_SUB), o sea un
	 * time_t NEGATIVO, y las conversiones de fecha de libc se vuelven locas:
	 * M130 pillo a launchd girando en _tmcomp/_time2sub (el bucle de mktime).
	 * Damos una fecha fija sensata para que converjan.  Cuando se porte el RTC
	 * del MT6583 (mt6323 rtc), esto sobra.
	 */
	if (result == 0)
		result = 1785110400L;	/* 2026-07-27 00:00:00 UTC */

	return (result);
}"""
if old not in s:
    print("NO MATCH IOPlatformExpert"); raise SystemExit(1)
open(p, "w").write(s.replace(old, new, 1))
print("IOPlatformExpert: fecha por defecto")

# 2) model_dep.c: medir calendario y uptime en el TICK
p = "/home/cpcd/darwin-krillin/xnu/osfmk/arm/model_dep.c"
s = open(p).read()
old = """    {
        extern unsigned int gFaultCount, gFaultRepeat;   /* M126 */
        PE_early_puts(" NF=");
        PE_early_puthex(gFaultCount);
        PE_early_puts(" NFR=");
        PE_early_puthex(gFaultRepeat);
    }"""
new = """    {
        extern unsigned int gFaultCount, gFaultRepeat;   /* M126 */
        PE_early_puts(" NF=");
        PE_early_puthex(gFaultCount);
        PE_early_puts(" NFR=");
        PE_early_puthex(gFaultRepeat);
    }
    {
        /* M131: la hora que ve userland.  CAL debe avanzar ~1/s y valer
         * ~0x6A6... (2026).  Si esta congelada o es absurda, las conversiones
         * de fecha de libc no convergen (M130: launchd girando en _tmcomp). */
        clock_sec_t _cs = 0;
        clock_usec_t _cu = 0;
        clock_get_calendar_microtime(&_cs, &_cu);
        PE_early_puts(" CAL=");
        PE_early_puthex((unsigned int) _cs);
        _cs = 0; _cu = 0;
        clock_get_system_microtime(&_cs, &_cu);
        PE_early_puts(" UP=");
        PE_early_puthex((unsigned int) _cs);
    }"""
if old not in s:
    print("NO MATCH model_dep"); raise SystemExit(1)
open(p, "w").write(s.replace(old, new, 1))
print("model_dep.c: CAL/UP en el TICK")
