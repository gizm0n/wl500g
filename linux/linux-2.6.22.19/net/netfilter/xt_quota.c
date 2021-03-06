/*
 * netfilter module to enforce network quotas
 *
 * Sam Johnston <samj@samj.net>
 */
#include <linux/skbuff.h>
#include <linux/spinlock.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_quota.h>

struct xt_quota_priv {
	uint64_t quota;
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sam Johnston <samj@samj.net>");
MODULE_DESCRIPTION("Xtables: countdown quota match");
MODULE_ALIAS("ipt_quota");
MODULE_ALIAS("ip6t_quota");

static DEFINE_SPINLOCK(quota_lock);

static bool
match(const struct sk_buff *skb, struct xt_action_param *par)
{
	struct xt_quota_info *q = (void *)par->matchinfo;
	struct xt_quota_priv *priv = q->master;
	bool ret = q->flags & XT_QUOTA_INVERT;

	spin_lock_bh(&quota_lock);
	if (priv->quota >= skb->len) {
		priv->quota -= skb->len;
		ret = !ret;
	} else {
		/* we do not allow even small packets from now on */
		priv->quota = 0;
	}
	/* Copy quota back to matchinfo so that iptables can display it */
	q->quota = priv->quota;
	spin_unlock_bh(&quota_lock);

	return ret;
}

static bool checkentry(const struct xt_mtchk_param *par)
{
	struct xt_quota_info *q = par->matchinfo;

	if (q->flags & ~XT_QUOTA_MASK)
		return 0;

	q->master = kmalloc(sizeof(*q->master), GFP_KERNEL);
	if (q->master == NULL)
		return 0;

	q->master->quota = q->quota;
	return 1;
}

static void quota_mt_destroy(const struct xt_mtdtor_param *par)
{
	const struct xt_quota_info *q = par->matchinfo;

	kfree(q->master);
}

static struct xt_match xt_quota_match __read_mostly = {
	.name		= "quota",
	.revision   = 0,
	.family		= NFPROTO_UNSPEC,
	.match		= match,
	.checkentry	= checkentry,
	.destroy	= quota_mt_destroy,
	.matchsize	= sizeof(struct xt_quota_info),
	.me		= THIS_MODULE
};

static int __init xt_quota_init(void)
{
	return xt_register_match(&xt_quota_match);
}

static void __exit xt_quota_fini(void)
{
	xt_unregister_match(&xt_quota_match);
}

module_init(xt_quota_init);
module_exit(xt_quota_fini);
