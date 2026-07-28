p = "/home/cpcd/darwin-krillin/xnu/osfmk/kern/sched_prim.c"
s = open(p).read()
old = """#if CSW_CHECK_ALWAYS_PREEMPT
	return AST_PREEMPT | AST_URGENT;
#endif

	ast_t			result = AST_NONE;
	thread_t		thread = processor->active_thread;

	if (first_timeslice(processor)) {
		if (rt_runq.count > 0)
			return (AST_PREEMPT | AST_URGENT);
	}
	else {
		if (rt_runq.count > 0 && BASEPRI_RTQUEUES >= processor->current_pri)
			return (AST_PREEMPT | AST_URGENT);
	}

	result = SCHED(processor_csw_check)(processor);
	if (result != AST_NONE)
		return (result);

	if (SCHED(should_current_thread_rechoose_processor)(processor))
		return (AST_PREEMPT);
	
	if (machine_processor_is_inactive(processor))
		return (AST_PREEMPT);

	if (thread->state & TH_SUSP)
		return (AST_PREEMPT);

	return (AST_NONE);
}"""
new = """#if CSW_CHECK_ALWAYS_PREEMPT
	return AST_PREEMPT | AST_URGENT;
#endif

	ast_t			result = AST_NONE;
	thread_t		thread = processor->active_thread;

	if (first_timeslice(processor)) {
		if (rt_runq.count > 0)
			{ csw_dbg(1); return (AST_PREEMPT | AST_URGENT); }
	}
	else {
		if (rt_runq.count > 0 && BASEPRI_RTQUEUES >= processor->current_pri)
			{ csw_dbg(2); return (AST_PREEMPT | AST_URGENT); }
	}

	result = SCHED(processor_csw_check)(processor);
	if (result != AST_NONE)
		{ csw_dbg(3); return (result); }

	if (SCHED(should_current_thread_rechoose_processor)(processor))
		{ csw_dbg(4); return (AST_PREEMPT); }
	
	if (machine_processor_is_inactive(processor))
		{ csw_dbg(5); return (AST_PREEMPT); }

	if (thread->state & TH_SUSP)
		{ csw_dbg(6); return (AST_PREEMPT); }

	csw_dbg(0);
	return (AST_NONE);
}"""
if old not in s:
    print("NO MATCH csw_check body"); raise SystemExit(1)
s = s.replace(old, new, 1)

# helper justo antes del #define
anchor = "#define CSW_CHECK_ALWAYS_PREEMPT 0"
helper = """/*
 * M123: que rama de csw_check dispara la expulsion.  El astloop sigue girando
 * (launchd con TH_RUN y 0 muestras en usuario), asi que alguien re-arma el AST
 * en cada vuelta; el unico que le da valor es csw_check via ast_check().
 *   1 = rt_runq (primer timeslice)     4 = should_current_thread_rechoose
 *   2 = rt_runq (timeslice agotado)    5 = machine_processor_is_inactive
 *   3 = SCHED(processor_csw_check)     6 = TH_SUSP        0 = AST_NONE (sano)
 */
static void csw_dbg(int code)
{
	extern int gMachTrace;
	extern void PE_early_puts(char *s);
	extern void PE_early_puthex(unsigned int v);
	static int n = 0;

	if (!gMachTrace || n >= 40)
		return;
	n++;
	PE_early_puts("CSW=");
	PE_early_puthex((unsigned int) code);
	PE_early_puts("\\n");
}

#define CSW_CHECK_ALWAYS_PREEMPT 0"""
s = s.replace(anchor, helper, 1)
open(p, "w").write(s)
print("csw_check instrumentado (M123)")
