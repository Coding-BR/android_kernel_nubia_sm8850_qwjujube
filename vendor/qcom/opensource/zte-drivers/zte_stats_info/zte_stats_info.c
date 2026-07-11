// SPDX-License-Identifier: GPL-2.0-only
/*
 * zte_stats_info.c - ZTE Custom Taskstats Interface
 *
 * Implements a lightweight per-CPU task and group statistics tracker
 * exposed via Generic Netlink ("ZTE_STATS").
 *
 * Reconstructed from binary analysis of zte_stats_info.ko
 * extracted from NX809J (Red Magic 11 Pro) production firmware.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/sched/cputime.h>
#include <linux/rwsem.h>
#include <linux/percpu.h>
#include <linux/slab.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/bitmap.h>
#include <linux/pid_namespace.h>
#include <net/genetlink.h>

#define FAMILY_NAME "ZTE_STATS_CUSTOM"

/* Custom ZTE Taskstats Payload Struct (352 bytes) */
struct zte_taskstats {
	u16 version;            /* 0 */
	u32 ac_exitcode;        /* 4 */
	u8 ac_flag;             /* 8 */
	u8 ac_nice;             /* 9 */
	u8 pad[6];              /* 10 */
	u64 ac_utime;           /* 16 */
	u64 ac_stime;           /* 24 */
	u64 ac_nvcsw;           /* 32 */
	u64 ac_nivcsw;          /* 40 */
	u64 pad2[2];            /* 48 */
	u64 ac_etime;           /* 64 */
	u64 ac_btime;           /* 72 */
	char ac_comm[32];       /* 80 */
	u32 ac_sched;           /* 112 */
	u32 pad3[3];            /* 116 */
	u32 ac_pid;             /* 128 */
	u32 ac_ppid;            /* 132 */
	u32 ac_btime_sec;       /* 136 */
	u32 pad4;               /* 140 */
	u64 ac_active;          /* 144 */
	u64 ac_utimes;          /* 152 */
	u64 ac_stimes;          /* 160 */
	u64 ac_minflt;          /* 168 */
	u64 ac_majflt;          /* 176 */
	u64 ac_read_bytes;      /* 184 */
	u64 ac_write_bytes;     /* 192 */
	u64 cgroup_mem_rss;     /* 200 */
	u64 cgroup_mem_vsize;   /* 208 */
	u64 ac_swaps;           /* 216 */
	u64 ac_nvcsw2;          /* 224 */
	u64 ac_nivcsw2;         /* 232 */
	u32 ac_uid;             /* 240 */
	u32 ac_gid;             /* 244 */
	u64 pad5[3];            /* 248 */
	u64 ac_stime_coarse;    /* 272 */
	u64 ac_utime_coarse;    /* 280 */
	u64 ac_utimes_micro;    /* 288 */
	u64 ac_stimes_micro;    /* 296 */
	u64 ac_etime_micro;     /* 304 */
	u64 pad6[4];            /* 312 */
	u64 ac_btime_seconds;   /* 344 */
};

/* Netlink attributes */
enum {
	ZTE_TASKSTATS_ATTR_UNSPEC,
	ZTE_TASKSTATS_ATTR_PID,
	ZTE_TASKSTATS_ATTR_TGID,
	ZTE_TASKSTATS_ATTR_REGISTER_CPUMASK,
	ZTE_TASKSTATS_ATTR_DEREGISTER_CPUMASK,
	__ZTE_TASKSTATS_ATTR_MAX,
};
#define ZTE_TASKSTATS_ATTR_MAX (__ZTE_TASKSTATS_ATTR_MAX - 1)

enum {
	ZTE_TASKSTATS_TYPE_UNSPEC,
	ZTE_TASKSTATS_TYPE_PID,
	ZTE_TASKSTATS_TYPE_TGID,
	ZTE_TASKSTATS_TYPE_STATS,
};

enum {
	ZTE_TASKSTATS_CMD_UNSPEC,
	ZTE_TASKSTATS_CMD_GET,
	ZTE_TASKSTATS_CMD_NEW,
	__ZTE_TASKSTATS_CMD_MAX,
};

/* Listener structures */
struct zte_listener_cpu {
	struct rw_semaphore sem;
	struct list_head list;
};

struct zte_listener {
	struct list_head list;
	u32 pid;
	u8 valid;
};

static DEFINE_PER_CPU(struct zte_listener_cpu, zte_listener_array);
static DEFINE_PER_CPU(u32, zte_taskstats_seqnum);

/* NLA Policy */
static const struct nla_policy zte_taskstats_cmd_get_policy[ZTE_TASKSTATS_ATTR_MAX + 1] = {
	[ZTE_TASKSTATS_ATTR_PID] = { .type = NLA_U32 },
	[ZTE_TASKSTATS_ATTR_TGID] = { .type = NLA_U32 },
	[ZTE_TASKSTATS_ATTR_REGISTER_CPUMASK] = { .type = NLA_STRING },
	[ZTE_TASKSTATS_ATTR_DEREGISTER_CPUMASK] = { .type = NLA_STRING },
};

static const struct nla_policy zte_cgroupstats_cmd_get_policy[] __used = {
	[0] = { .type = NLA_U32 },
};

/* Forward Declarations of Netlink handlers */
static int zte_taskstats_user_cmd(struct sk_buff *skb, struct genl_info *info);
static int zte_cgroupstats_user_cmd(struct sk_buff *skb, struct genl_info *info);

static const struct genl_small_ops zte_taskstats_ops[] = {
	{
		.cmd = ZTE_TASKSTATS_CMD_GET,
		.doit = zte_taskstats_user_cmd,
	},
	{
		.cmd = ZTE_TASKSTATS_CMD_NEW,
		.doit = zte_cgroupstats_user_cmd,
	},
};

static struct genl_family zte_family = {
	.name = FAMILY_NAME,
	.version = 1,
	.maxattr = ZTE_TASKSTATS_ATTR_MAX,
	.policy = zte_taskstats_cmd_get_policy,
	.small_ops = zte_taskstats_ops,
	.n_small_ops = ARRAY_SIZE(zte_taskstats_ops),
	.module = THIS_MODULE,
};

/* ======================================================================
 * Local Task Sighand Lock implementation (avoid GKI non-exported symbols)
 * ====================================================================== */

struct sighand_struct *__zte_lock_task_sighand(struct task_struct *tsk, unsigned long *flags)
{
	struct sighand_struct *sighand;

	rcu_read_lock();
	for (;;) {
		sighand = rcu_dereference(tsk->sighand);
		if (!sighand)
			break;
		spin_lock_irqsave(&sighand->siglock, *flags);
		if (likely(sighand == rcu_access_pointer(tsk->sighand)))
			break;
		spin_unlock_irqrestore(&sighand->siglock, *flags);
	}
	rcu_read_unlock();

	return sighand;
}


/* ======================================================================
 * Listener Registry API
 * ====================================================================== */

static int zte_parse(struct nlattr *nla, unsigned long *mask)
{
	char *buf;
	size_t len;
	int ret;

	if (!nla)
		return 1;

	len = nla_len(nla);
	if (len > 0x124 || !len)
		return -EINVAL;

	buf = kmalloc(len + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	nla_strscpy(buf, nla, len + 1);

	ret = bitmap_parselist(buf, mask, nr_cpu_ids);
	kfree(buf);

	return ret;
}

static int zte_add_del_listener(u32 pid, const unsigned long *mask, int is_del)
{
	int cpu;
	struct zte_listener_cpu *cpu_sem;
	struct zte_listener *curr, *tmp;

	/* Stock driver only handles the first word of possible CPUs on this target. */
	if ((*mask & ~(*cpumask_bits(cpu_possible_mask))) != 0)
		return -EINVAL;

	if (task_active_pid_ns(current) != &init_pid_ns)
		return -EINVAL;

	if (is_del) {
		for_each_cpu(cpu, to_cpumask(mask)) {
			cpu_sem = per_cpu_ptr(&zte_listener_array, cpu);
			down_write(&cpu_sem->sem);
			list_for_each_entry_safe(curr, tmp, &cpu_sem->list, list) {
				if (curr->pid == pid) {
					list_del(&curr->list);
					kfree(curr);
					break;
				}
			}
			up_write(&cpu_sem->sem);
		}
	} else {
		for_each_cpu(cpu, to_cpumask(mask)) {
			struct zte_listener *new_listener;
			bool exists = false;

			new_listener = kmalloc_node(sizeof(*new_listener), GFP_KERNEL, cpu);
			if (!new_listener) {
				zte_add_del_listener(pid, mask, 1);
				return -ENOMEM;
			}
			new_listener->pid = pid;
			new_listener->valid = 1;

			cpu_sem = per_cpu_ptr(&zte_listener_array, cpu);
			down_write(&cpu_sem->sem);

			list_for_each_entry(curr, &cpu_sem->list, list) {
				if (curr->pid == pid && curr->valid) {
					exists = true;
					break;
				}
			}

			if (!exists)
				list_add(&new_listener->list, &cpu_sem->list);
			up_write(&cpu_sem->sem);
			if (exists)
				kfree(new_listener);
		}
	}

	return 0;
}

/* ======================================================================
 * Netlink reply generation
 * ====================================================================== */

static int zte_prepare_reply(struct genl_info *info, struct sk_buff **skbp, u16 payload_len)
{
	struct sk_buff *skb;
	void *reply;

	/* Allocate socket buffer aligned to generic netlink requirements */
	skb = genlmsg_new(payload_len, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	if (!info) {
		/* Atomic increment sequence number per-cpu */
		this_cpu_inc(zte_taskstats_seqnum);
	}

	reply = genlmsg_put(skb, info ? info->snd_portid : 0,
			info ? info->snd_seq : this_cpu_read(zte_taskstats_seqnum),
			&zte_family, 0, ZTE_TASKSTATS_CMD_GET);
	if (!reply) {
		nlmsg_free(skb);
		return -EINVAL;
	}

	*skbp = skb;
	return 0;
}

static struct zte_taskstats *zte_mk_reply(struct sk_buff *skb, u32 type, u32 pid)
{
	struct nlattr *na, *ret;

	na = nla_nest_start_noflag(skb, type);
	if (!na)
		return NULL;

	if (nla_put_u32(skb, ZTE_TASKSTATS_TYPE_PID, pid) < 0) {
		nla_nest_cancel(skb, na);
		goto err;
	}

	ret = nla_reserve_64bit(skb, ZTE_TASKSTATS_TYPE_STATS, sizeof(struct zte_taskstats), 0);
	if (!ret) {
		nla_nest_cancel(skb, na);
		goto err;
	}

	nla_nest_end(skb, na);
	return nla_data(ret);

err:
	return NULL;
}

/* ======================================================================
 * Task Stats Gathering Core
 * ====================================================================== */

static int zte_taskstats_user_cmd(struct sk_buff *skb, struct genl_info *info)
{
	int ret = 0;
	unsigned long cpu_mask;
	struct sk_buff *rep_skb = NULL;
	struct zte_taskstats *stats;

	if (info->attrs[ZTE_TASKSTATS_ATTR_REGISTER_CPUMASK]) {
		cpu_mask = 0;
		ret = zte_parse(info->attrs[ZTE_TASKSTATS_ATTR_REGISTER_CPUMASK], &cpu_mask);
		if (!ret)
			ret = zte_add_del_listener(info->snd_portid, &cpu_mask, 0);
		return ret;
	}

	if (info->attrs[ZTE_TASKSTATS_ATTR_DEREGISTER_CPUMASK]) {
		cpu_mask = 0;
		ret = zte_parse(info->attrs[ZTE_TASKSTATS_ATTR_DEREGISTER_CPUMASK], &cpu_mask);
		if (!ret)
			ret = zte_add_del_listener(info->snd_portid, &cpu_mask, 1);
		return ret;
	}

	if (info->attrs[ZTE_TASKSTATS_ATTR_PID]) {
		u32 pid = nla_get_u32(info->attrs[ZTE_TASKSTATS_ATTR_PID]);
		struct task_struct *task;
		struct pid_namespace *active_ns;

		ret = zte_prepare_reply(info, &rep_skb, sizeof(struct zte_taskstats) + 32);
		if (ret)
			return ret;

		stats = zte_mk_reply(rep_skb, ZTE_TASKSTATS_TYPE_PID, pid);
		if (!stats)
			return -ENOMEM;

		rcu_read_lock();
		task = find_task_by_vpid(pid);
		if (task) {
			refcount_inc(&task->usage);
			rcu_read_unlock();

			active_ns = task_active_pid_ns(current);
			memset(stats, 0, sizeof(*stats));

			stats->version = 10;
			stats->ac_exitcode = task->exit_code;
			stats->ac_flag = task->flags & 0xFF;
			stats->ac_nice = task_nice(task);

			u64 ttime = task->utime + task->stime;
			stats->ac_utime = task->utime;
			stats->ac_stime = task->stime;
			stats->ac_etime = ttime;
			stats->ac_btime = task->start_time;

			stats->ac_nvcsw = task->nvcsw;
			stats->ac_nivcsw = task->nivcsw;
			stats->ac_pid = task_pid_nr_ns(task, active_ns);
			stats->ac_ppid = task_tgid_nr_ns(task, active_ns);

			stats->ac_active = (ktime_get_ns() - task->start_time) / 1000;
			stats->ac_btime_seconds = ktime_get_real_seconds() - div_u64(stats->ac_active, 1000000);
			stats->ac_btime_sec = stats->ac_btime_seconds > U32_MAX ? U32_MAX : stats->ac_btime_seconds;
			stats->ac_utimes = task->utime / 1000;
			stats->ac_stimes = task->stime / 1000;

			strncpy(stats->ac_comm, task->comm, sizeof(stats->ac_comm));

			struct mm_struct *mm = get_task_mm(task);
			if (mm) {
				stats->cgroup_mem_rss = get_mm_rss(mm) * PAGE_SIZE;
				stats->cgroup_mem_vsize = mm->total_vm * PAGE_SIZE;
				mmput(mm);
			}

			if (refcount_dec_and_test(&task->usage))
				__put_task_struct(task);

			/* Send response back */
			genlmsg_end(rep_skb, stats);
			return genlmsg_unicast(genl_info_net(info), rep_skb, info->snd_portid);
		}
		rcu_read_unlock();
		nlmsg_free(rep_skb);
		return -ESRCH;
	}

	if (info->attrs[ZTE_TASKSTATS_ATTR_TGID]) {
		u32 tgid = nla_get_u32(info->attrs[ZTE_TASKSTATS_ATTR_TGID]);
		struct task_struct *task, *t;

		ret = zte_prepare_reply(info, &rep_skb, sizeof(struct zte_taskstats) + 32);
		if (ret)
			return ret;

		stats = zte_mk_reply(rep_skb, ZTE_TASKSTATS_TYPE_TGID, tgid);
		if (!stats)
			return -ENOMEM;

		rcu_read_lock();
		task = find_task_by_vpid(tgid);
		if (task) {
			unsigned long flags;

			if (__zte_lock_task_sighand(task, &flags)) {
				struct sighand_struct *sighand = rcu_dereference(task->sighand);
				if (task->signal->stats)
					memcpy(stats, task->signal->stats, sizeof(*stats));
				else
					memset(stats, 0, sizeof(*stats));
				stats->version = 10;

				/* Aggregate stats for all threads in group */
				for_each_thread(task, t) {
					if (t->exit_state)
						continue;
					stats->ac_utime += t->utime;
					stats->ac_stime += t->stime;
					stats->ac_nvcsw += t->nvcsw;
					stats->ac_nivcsw += t->nivcsw;
					stats->ac_minflt += t->min_flt;
					stats->ac_majflt += t->maj_flt;
				}
				if (sighand)
					spin_unlock_irqrestore(&sighand->siglock, flags);
				rcu_read_unlock();

				/* Send response back */
				genlmsg_end(rep_skb, stats);
				return genlmsg_unicast(genl_info_net(info), rep_skb, info->snd_portid);
			}
		}
		rcu_read_unlock();
		nlmsg_free(rep_skb);
		return -ESRCH;
	}

	return -EINVAL;
}

static int zte_cgroupstats_user_cmd(struct sk_buff *skb, struct genl_info *info)
{
	/* Unused but registered stub in original ZTE driver */
	return 0;
}

/* ======================================================================
 * Module Initialization and Exit
 * ====================================================================== */

static int __init zte_taskstats_init_early(void)
{
	int cpu;
	int ret;

	/* Initialize lists and semaphores for all possible CPUs */
	for_each_possible_cpu(cpu) {
		struct zte_listener_cpu *cpu_sem = per_cpu_ptr(&zte_listener_array, cpu);
		init_rwsem(&cpu_sem->sem);
		INIT_LIST_HEAD(&cpu_sem->list);
	}

	pr_err("ZTE_DBG: init family=[%s]\n", zte_family.name);
	ret = genl_register_family(&zte_family);
	pr_err("ZTE_DBG: genl_register_family ret=%d\n", ret);
	if (ret) {
		pr_err("%s: Failed to register ZTE stats family\n", __func__);
		return ret;
	}

	pr_err("ZTE_DBG: Family registered OK\n");
	pr_info("ZTE taskstats registered\n");
	return 0;
}

module_init(zte_taskstats_init_early);

MODULE_DESCRIPTION("ZTE Custom Taskstats Interface Driver");
MODULE_LICENSE("GPL");
