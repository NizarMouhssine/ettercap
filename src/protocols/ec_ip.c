/*
    ettercap -- IP decoder module

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

    $Header: /home/drizzt/dev/sources/ettercap.cvs/ettercap_ng/src/protocols/ec_ip.c,v 1.1 2003/03/08 13:53:38 alor Exp $
*/

#include <ec.h>
#include <ec_decode.h>

/* globals */

struct ip_header {
#ifndef WORDS_BIGENDIAN
   u_int8   ihl:4;
   u_int8   version:4;
#else 
   u_int8   version:4;
   u_int8   ihl:4;
#endif
   u_int8   tos;
   u_int16  tot_len;
   u_int16  id;
   u_int16  frag_off;
#define IP_DF 0x4000
   u_int8   ttl;
   u_int8   protocol;
   u_int16  check;
   u_int32  saddr;
   u_int32  daddr;
/*The options start here. */
};


/* protos */

FUNC_DECODER(decode_ip);
void ip_init(void);

/*******************************************/

/*
 * this function is the initializer.
 * it adds the entry in the table of registered decoder
 */

void __init ip_init(void)
{
   add_decoder(NET_LAYER, LL_TYPE_IP, decode_ip);
}


FUNC_DECODER(decode_ip)
{
   FUNC_DECODER_PTR(next_decoder);
   struct ip_header *ip;

   ip = (struct ip_header *)DECODE_DATA;
  
   DECODED_LEN = ip->ihl * 4;

   /* IP addresses */
   ip_addr_init(&PACKET->L3.src, AF_INET, (char *)&ip->saddr);
   ip_addr_init(&PACKET->L3.dst, AF_INET, (char *)&ip->daddr);

   /*
    * if the L3 is parsed for the first time,
    * set the fwd_packet pointer
    * this is needed because incapsulated packet may 
    * overwrite the L3.header pointer used by send_to_L3
    */
   if (PACKET->L3.header == NULL) {
      PACKET->fwd_packet = (u_char *)DECODE_DATA;
      PACKET->fwd_len = PACKET->len - PACKET->L2.len;
   }
   
   /* other relevant infos */
   PACKET->L3.header = (u_char *)DECODE_DATA;
   PACKET->L3.len = DECODED_LEN;
   
   if (ip->ihl * 4 != sizeof(struct ip_header)) {
      PACKET->L3.options = (u_char *)(DECODE_DATA) + sizeof(struct ip_header);
      PACKET->L3.optlen = (ip->ihl * 4) - sizeof(struct ip_header);
   } else {
      PACKET->L3.options = NULL;
      PACKET->L3.optlen = 0;
   }
   
   PACKET->L3.proto = htons(LL_TYPE_IP);
   PACKET->L3.ttl = ip->ttl;
   
   /* XXX - implemet checksum check */

#if 0
   /* XXX - implement fingerprint */
   
   /* if there is a TCP packet, try to passive fingerprint it */
   if (ip->protocol == LN_TYPE_TCP) {
      /* initialize passive fingerprint */
      BUCKET->L4->fingerprint = fingerprint_alloc();
  
      /* collect ifos for passive fingerprint */
      fingerprint_push(BUCKET->L4->fingerprint, FINGER_TTL, ip->ttl);
      fingerprint_push(BUCKET->L4->fingerprint, FINGER_DF, ntohs(ip->frag_off) & IP_DF);
      fingerprint_push(BUCKET->L4->fingerprint, FINGER_LT, ip->ihl * 4);
   }
#endif
   
   next_decoder = get_decoder(PROTO_LAYER, ip->protocol);

   EXECUTE_DECODER(next_decoder);
   
   /* XXX - implement modification checks */
#if 0
   if (po->flags & PO_MOD_LEN)
      
   if (po->flags & PO_MOD_CHECK)
#endif   
      
   return NULL;
}

/* EOF */

// vim:ts=3:expandtab

