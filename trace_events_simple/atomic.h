#undef TRACE_SYSTEM
#define TRACE_SYSTEM atomic

#if !defined(_TRACE_EVENT_ATOMIC_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_EVENT_ATOMIC_H

#include <linux/tracepoint.h>

TRACE_EVENT(atomic_step,
	TP_PROTO(const char *step),

	TP_ARGS(step),

	TP_STRUCT__entry(
		__string(step, step)
	),

	TP_fast_assign(
		__assign_str(step, step);
	),

	TP_printk("%s", __get_str(step))
);

TRACE_EVENT(atomic_task,
	TP_PROTO(const char *name, int id, pid_t pid,
			int prio, struct task_struct *task),

	TP_ARGS(name, id, pid, prio, task),

	TP_STRUCT__entry(
		__string(               comm,	name)
		__field(                 int,	id  )
		__field(               pid_t,	pid )
		__field(                 int,	prio)
		__field(struct task_struct *,	task)
	),

	TP_fast_assign(
		__assign_str(comm, name);
		__entry->id = id;
		__entry->pid = pid;
		__entry->prio = prio;
		__entry->task = task;
	),

	TP_printk("name:%s id:%d pid:%d prio:%d task_struct:0x%px",
			__get_str(comm), __entry->id, __entry->pid,
			__entry->prio, __entry->task)
);
#endif /* _TRACE_EVENT_ATOMIC_H */

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE atomic
#include <trace/define_trace.h>
