/* This is a module which is used for setting up fake conntracks
 * on packets so that they are not seen by the conntrack/NAT code.
 */
#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_conntrack.h>

MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_NOTRACK");

static unsigned int
target(struct sk_buff *skb, const struct xt_action_param *par)
{
	/* Previously seen (loopback)? Ignore. */
	if (skb->nfct != NULL)
		return XT_CONTINUE;

	/* Attach fake conntrack entry.
	   If there is a real ct entry correspondig to this packet,
	   it'll hang aroun till timing out. We don't deal with it
	   for performance reasons. JK */
	skb->nfct = &nf_ct_untracked_get()->ct_general;
	skb->nfctinfo = IP_CT_NEW;
	nf_conntrack_get(skb->nfct);

	return XT_CONTINUE;
}

static struct xt_target xt_notrack_target __read_mostly = {
	.name		= "NOTRACK",
	.family		= NFPROTO_UNSPEC,
	.target		= target,
	.table		= "raw",
	.me		= THIS_MODULE,
};

static int __init xt_notrack_init(void)
{
	return xt_register_target(&xt_notrack_target);
}

static void __exit xt_notrack_fini(void)
{
	xt_unregister_target(&xt_notrack_target);
}

module_init(xt_notrack_init);
module_exit(xt_notrack_fini);
