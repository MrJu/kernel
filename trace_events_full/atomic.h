#undef TRACE_SYSTEM
#define TRACE_SYSTEM atomic

#if !defined(_TRACE_EVENT_ATOMIC_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_EVENT_ATOMIC_H

#include <linux/tracepoint.h>

static int foo_bar_pre(void);
static void foo_bar_post(void);

#ifndef __TRACE_EVENT_ATOMIC_HELPER_FUNCTIONS
#define __TRACE_EVENT_ATOMIC_HELPER_FUNCTIONS
static inline int __length_of(const int *list)
{
	int i;

	if (!list)
		return 0;

	for (i = 0; list[i]; i++)
		;
	return i;
}
#endif

TRACE_EVENT(foo_bar,
	TP_PROTO(const char *foo, int bar, const int *lst,
			const struct cpumask *mask),

	TP_ARGS(foo, bar, lst, mask),

	TP_STRUCT__entry(
		__string( foo,  foo                  )
		__field(  int,  bar                  )
		__array(  int,  list,               4)
		__bitmask(cpus, num_possible_cpus()  )
	),

	TP_fast_assign(
		__assign_str(foo, foo);
		__entry->bar = bar;
		memcpy(__entry->list, lst, sizeof(int) * 4);
		__assign_bitmask(cpus, cpumask_bits(mask), num_possible_cpus());
	),

	TP_printk("foo: %s %d %s %s %s (%s)",
			__get_str(foo),
			__entry->bar,
			__print_symbolic(__entry->bar,
					{ 0, "zero"  },
					{ 1, "one"   },
					{ 2, "two"   },
					{ 3, "three" },
					{ 4, "four"  },
					{ 5, "five"  },
					{ 6, "six"   },
					{ 7, "seven" }
			),
			__print_flags(__entry->bar, "|",
					{ 1, "bit0" },
					{ 2, "bit1" },
					{ 4, "bit2" },
					{ 8, "bit3" }
			),
			__print_array(__entry->list, 4, sizeof(int)),
			__get_bitmask(cpus))
);

TRACE_EVENT_CONDITION(foo_bar_with_cond,
	TP_PROTO(const char *foo, int bar),

	TP_ARGS(foo, bar),

	TP_CONDITION(!(bar % 10)),

	TP_STRUCT__entry(
		__string(foo, foo)
		__field( int, bar)
	),

	TP_fast_assign(
		__assign_str(foo, foo);
		__entry->bar = bar;
	),

	TP_printk("foo: %s %d", __get_str(foo), __entry->bar)
);

TRACE_EVENT_FN(foo_bar_with_fn,
	TP_PROTO(const char *foo, int bar),

	TP_ARGS(foo, bar),

	TP_STRUCT__entry(
		__string(foo, foo)
		__field( int, bar)
	),

	TP_fast_assign(
		__assign_str(foo, foo);
		__entry->bar = bar;
	),

	TP_printk("foo: %s %d", __get_str(foo), __entry->bar),

	foo_bar_pre,
	foo_bar_post
);

DECLARE_EVENT_CLASS(foo_template,
	TP_PROTO(const char *foo, int bar),

	TP_ARGS(foo, bar),

	TP_STRUCT__entry(
		__string(foo, foo)
		__field( int, bar)
	),

	TP_fast_assign(
		__assign_str(foo, foo);
		__entry->bar = bar;
	),

	TP_printk("foo: %s %d", __get_str(foo), __entry->bar)
);

DEFINE_EVENT(foo_template, foo_with_template_simple,
	TP_PROTO(const char *foo, int bar),
	TP_ARGS(foo, bar));

DEFINE_EVENT_CONDITION(foo_template, foo_with_template_cond,
	TP_PROTO(const char *foo, int bar),
	TP_ARGS(foo, bar),
	TP_CONDITION(!(bar % 10)));

DEFINE_EVENT_FN(foo_template, foo_with_template_fn,
	TP_PROTO(const char *foo, int bar),
	TP_ARGS(foo, bar),
	foo_bar_pre,
	foo_bar_post);

DEFINE_EVENT_PRINT(foo_template, foo_with_template_print,
	TP_PROTO(const char *foo, int bar),
	TP_ARGS(foo, bar),
	TP_printk("bar: %s %d", __get_str(foo), __entry->bar));
#endif /* _TRACE_EVENT_ATOMIC_H */

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE atomic
#include <trace/define_trace.h>
