/* Kernel module to match one of a list of TCP/UDP(-Lite)/SCTP/DCCP ports:
   ports are in the same place so we can treat them as equal. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <linux/in.h>

#include <linux/netfilter/xt_multiport.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("x_tables multiple port match module");
MODULE_ALIAS("ipt_multiport");
MODULE_ALIAS("ip6t_multiport");

#if 0
#define duprintf(format, args...) printk(format , ## args)
#else
#define duprintf(format, args...)
#endif

static inline int
ports_match_v1(const struct xt_multiport_v1 *minfo,
	       u_int16_t src, u_int16_t dst)
{
	unsigned int i;
	u_int16_t s, e;

	for (i = 0; i < minfo->count; i++) {
		s = minfo->ports[i];

		if (minfo->pflags[i]) {
			/* range port matching */
			e = minfo->ports[++i];
			duprintf("src or dst matches with %d-%d?\n", s, e);

			if (minfo->flags == XT_MULTIPORT_SOURCE
			    && src >= s && src <= e)
				return 1 ^ minfo->invert;
			if (minfo->flags == XT_MULTIPORT_DESTINATION
			    && dst >= s && dst <= e)
				return 1 ^ minfo->invert;
			if (minfo->flags == XT_MULTIPORT_EITHER
			    && ((dst >= s && dst <= e)
				|| (src >= s && src <= e)))
				return 1 ^ minfo->invert;
		} else {
			/* exact port matching */
			duprintf("src or dst matches with %d?\n", s);

			if (minfo->flags == XT_MULTIPORT_SOURCE
			    && src == s)
				return 1 ^ minfo->invert;
			if (minfo->flags == XT_MULTIPORT_DESTINATION
			    && dst == s)
				return 1 ^ minfo->invert;
			if (minfo->flags == XT_MULTIPORT_EITHER
			    && (src == s || dst == s))
				return 1 ^ minfo->invert;
		}
	}

	return minfo->invert;
}

static bool
match_v1(const struct sk_buff *skb, struct xt_action_param *par)
{
	__be16 _ports[2], *pptr;
	const struct xt_multiport_v1 *multiinfo = par->matchinfo;

	if (par->fragoff != 0)
		return 0;

	pptr = skb_header_pointer(skb, par->thoff, sizeof(_ports), _ports);
	if (pptr == NULL) {
		/* We've been asked to examine this packet, and we
		 * can't.  Hence, no choice but to drop.
		 */
		duprintf("xt_multiport: Dropping evil offset=0 tinygram.\n");
		par->hotdrop = true;
		return 0;
	}

	return ports_match_v1(multiinfo, ntohs(pptr[0]), ntohs(pptr[1]));
}

static inline bool
check(u_int16_t proto,
      u_int8_t ip_invflags,
      u_int8_t match_flags,
      u_int8_t count)
{
	/* Must specify supported protocol, no unknown flags or bad count */
	return (proto == IPPROTO_TCP || proto == IPPROTO_UDP
		|| proto == IPPROTO_UDPLITE
		|| proto == IPPROTO_SCTP || proto == IPPROTO_DCCP)
		&& !(ip_invflags & XT_INV_PROTO)
		&& (match_flags == XT_MULTIPORT_SOURCE
		    || match_flags == XT_MULTIPORT_DESTINATION
		    || match_flags == XT_MULTIPORT_EITHER)
		&& count <= XT_MULTI_PORTS;
}

/* Called when user tries to insert an entry of this type. */
static bool checkentry_v1(const struct xt_mtchk_param *par)
{
	const struct ipt_ip *ip = par->entryinfo;
	const struct xt_multiport_v1 *multiinfo = par->matchinfo;

	return check(ip->proto, ip->invflags, multiinfo->flags,
		     multiinfo->count);
}

static bool checkentry6_v1(const struct xt_mtchk_param *par)
{
	const struct ip6t_ip6 *ip = par->entryinfo;
	const struct xt_multiport_v1 *multiinfo = par->matchinfo;

	return check(ip->proto, ip->invflags, multiinfo->flags,
		     multiinfo->count);
}

static struct xt_match xt_multiport_match[] __read_mostly = {
	{
		.name		= "multiport",
		.family		= AF_INET,
		.revision	= 1,
		.checkentry	= checkentry_v1,
		.match		= match_v1,
		.matchsize	= sizeof(struct xt_multiport_v1),
		.me		= THIS_MODULE,
	},
	{
		.name		= "multiport",
		.family		= AF_INET6,
		.revision	= 1,
		.checkentry	= checkentry6_v1,
		.match		= match_v1,
		.matchsize	= sizeof(struct xt_multiport_v1),
		.me		= THIS_MODULE,
	},
};

static int __init xt_multiport_init(void)
{
	return xt_register_matches(xt_multiport_match,
				   ARRAY_SIZE(xt_multiport_match));
}

static void __exit xt_multiport_fini(void)
{
	xt_unregister_matches(xt_multiport_match,
			      ARRAY_SIZE(xt_multiport_match));
}

module_init(xt_multiport_init);
module_exit(xt_multiport_fini);
