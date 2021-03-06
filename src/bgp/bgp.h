/*  
    pmacct (Promiscuous mode IP Accounting package)
    pmacct is Copyright (C) 2003-2016 by Paolo Lucente
*/

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "bgp_hash.h"
#include "bgp_prefix.h"
#include "bgp_packet.h"
#include "bgp_table.h"
#include "bgp_logdump.h"

#ifndef _BGP_H_
#define _BGP_H_

/* defines */

/* BGP finite state machine status.  */
#define Idle                                     1
#define Connect                                  2
#define Active                                   3
#define OpenSent                                 4
#define OpenConfirm                              5
#define Established                              6
#define Clearing                                 7
#define Deleted                                  8

#define BGP_ATTR_MIN_LEN        3       /* Attribute flag, type length. */

/* BGP4 attribute type codes.  */
#define BGP_ATTR_ORIGIN                          1
#define BGP_ATTR_AS_PATH                         2
#define BGP_ATTR_NEXT_HOP                        3
#define BGP_ATTR_MULTI_EXIT_DISC                 4
#define BGP_ATTR_LOCAL_PREF                      5
#define BGP_ATTR_ATOMIC_AGGREGATE                6
#define BGP_ATTR_AGGREGATOR                      7
#define BGP_ATTR_COMMUNITIES                     8
#define BGP_ATTR_ORIGINATOR_ID                   9
#define BGP_ATTR_CLUSTER_LIST                   10
#define BGP_ATTR_DPA                            11
#define BGP_ATTR_ADVERTISER                     12
#define BGP_ATTR_RCID_PATH                      13
#define BGP_ATTR_MP_REACH_NLRI                  14
#define BGP_ATTR_MP_UNREACH_NLRI                15
#define BGP_ATTR_EXT_COMMUNITIES                16
#define BGP_ATTR_AS4_PATH                       17
#define BGP_ATTR_AS4_AGGREGATOR                 18
#define BGP_ATTR_AS_PATHLIMIT                   21

/* BGP Attribute flags. */
#define BGP_ATTR_FLAG_OPTIONAL  0x80    /* Attribute is optional. */
#define BGP_ATTR_FLAG_TRANS     0x40    /* Attribute is transitive. */
#define BGP_ATTR_FLAG_PARTIAL   0x20    /* Attribute is partial. */
#define BGP_ATTR_FLAG_EXTLEN    0x10    /* Extended length flag. */

/* BGP misc */
#define MAX_BGP_PEERS_DEFAULT 4
#define MAX_HOPS_FOLLOW_NH 20
#define MAX_NH_SELF_REFERENCES 1

/* Maximum BGP standard/extended community patterns supported:
   nfacctd_bgp_stdcomm_pattern, nfacctd_bgp_extcomm_pattern */
#define MAX_BGP_COMM_PATTERNS 16

/* structures */
struct bgp_rt_structs {
  struct hash *attrhash;
  struct hash *ashash;
  struct hash *comhash;
  struct hash *ecomhash;
  struct bgp_table *rib[AFI_MAX][SAFI_MAX];
};

struct bgp_misc_structs {
  struct bgp_peer_log *peers_log;
  u_int64_t log_seq;
  struct timeval log_tstamp;
  char log_tstamp_str[SRVBUFLEN];
  char peer_str[SRVBUFLEN]; /* bmp_router vs peer_src_ip */

#if defined WITH_RABBITMQ
  struct p_amqp_host *msglog_amqp_host;
#endif
#if defined WITH_KAFKA
  struct p_kafka_host *msglog_kafka_host;
#endif
  
  int max_peers;
  char *neighbors_file;
  char *dump_file;
  char *dump_amqp_routing_key;
  int dump_amqp_routing_key_rr;
  char *dump_kafka_topic;
  int dump_kafka_topic_rr;
  char *msglog_file;
  char *msglog_amqp_routing_key;
  int msglog_amqp_routing_key_rr;
  char *msglog_kafka_topic;
  int msglog_kafka_topic_rr;
};

/* all require definition of bgp_rt_structs */
#include "bgp_aspath.h"
#include "bgp_community.h"
#include "bgp_ecommunity.h"

struct bgp_peer_buf {
  char *base;
  u_int32_t len;
  u_int32_t truncated_len;
};

struct bgp_peer {
  int fd;
  int lock;
  int type; /* ie. BGP vs BMP */
  u_int8_t status;
  as_t myas;
  as_t as;
  u_int16_t ht;
  time_t last_keepalive;
  struct host_addr id;
  struct host_addr addr;
  char addr_str[INET6_ADDRSTRLEN];
  u_int16_t tcp_port;
  u_int8_t cap_mp;
  char *cap_4as;
  u_int8_t cap_add_paths;
  u_int32_t msglen;
  struct bgp_peer_buf buf;
  struct bgp_peer_log *log;
  void *bmp_se; /* struct bmp_dump_se_ll */
};

struct bgp_peer_batch {
  int num;
  int num_current;
  time_t base_stamp;
  int interval;
};

struct bgp_nlri {
  afi_t afi;
  safi_t safi;
  u_char *nlri;
  u_int16_t length;
};

struct bgp_attr {
  struct aspath *aspath;
  struct community *community;
  struct ecommunity *ecommunity;
  unsigned long refcnt;
  u_int32_t flag;
  struct in_addr nexthop;
  struct host_addr mp_nexthop;
  u_int32_t med;
  u_int32_t local_pref;
  struct {
    u_int32_t as;
    u_char ttl;
  } pathlimit;
  u_char origin;
};

struct bgp_comm_range {
  u_int32_t first;
  u_int32_t last;
};

#include "bgp_msg.h"
#include "bgp_lookup.h"
#include "bgp_util.h"

/* prototypes */
#if (!defined __BGP_C)
#define EXT extern
#else
#define EXT
#endif
EXT void nfacctd_bgp_wrapper();
EXT void skinny_bgp_daemon();
#undef EXT

/* global variables */
#if (!defined __BGP_C) && (!defined __BGP_UTIL_C) && (!defined __BGP_LOOKUP_C) && (!defined __BGP_MSG_C) && (!defined __BGP_LOGDUMP_C)
#define EXT extern
#else
#define EXT
#endif
EXT struct bgp_peer *peers;
EXT char *std_comm_patterns[MAX_BGP_COMM_PATTERNS];
EXT char *ext_comm_patterns[MAX_BGP_COMM_PATTERNS];
EXT char *std_comm_patterns_to_asn[MAX_BGP_COMM_PATTERNS];
EXT struct bgp_comm_range peer_src_as_ifrange; 
EXT struct bgp_comm_range peer_src_as_asrange; 
EXT u_int32_t (*bgp_route_info_modulo)(struct bgp_peer *, path_id_t *);
EXT int nfacctd_bgp_msglog_backend_methods;
EXT int bgp_table_dump_backend_methods;

EXT struct bgp_rt_structs inter_domain_routing_dbs[FUNC_TYPE_MAX], *bgp_routing_db;
EXT struct bgp_misc_structs inter_domain_misc_dbs[FUNC_TYPE_MAX], *bgp_misc_db;
#undef EXT
#endif 
