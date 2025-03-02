#include <arpa/inet.h>
#include <string.h>
#include "queue.h"
#include "lib.h"
#include "protocols.h"
#define ETHER_IP 0x0800
#define ETHER_ARP 0x0806
#define MAX_SIZE 100000
#define ARP_REQUEST 1
#define ARP_REPLY 2

struct route_table_entry *rtable;
int rtable_len;

struct arp_table_entry *mac_entry;
int mac_entry_len;

queue que;

struct packet {
	char* buf;
	size_t len;
	int interface;
};

// finds the best next hop, with the help of LPM algorythm
struct route_table_entry *get_best_route(uint32_t ip_dest)
{
	int i, idx = -1;

	for(i = 0; i < rtable_len; i++){
		if((ip_dest & rtable[i].mask) == rtable[i].prefix && rtable[i].mask > rtable[idx].mask){
			idx = i;
		}
	}

	if(idx == -1){
		return NULL;
	}

	else return (&rtable[idx]);
}

// returns the mac address of next hop from arp table
struct arp_table_entry *get_mac_entry(uint32_t given_ip)
{
	for (int i = 0; i < mac_entry_len; i++) {
		if(mac_entry[i].ip == given_ip) {
			return &mac_entry[i];
		}
	}

	return NULL;
}

// creates and send icmp packet with specific type
void icmp_custom_reply(char* buf, size_t len, int interface, int type)
{
	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
	struct icmphdr *icmp_hdr = (struct icmphdr *)
	(buf + sizeof(struct ether_header) + sizeof(struct iphdr));

	// saves the old ip header and last 64 bytes
	memcpy((char*)icmp_hdr + sizeof(icmp_hdr), (char*)ip_hdr,
	 sizeof(struct iphdr) + sizeof(struct icmphdr));
	
	ip_hdr->protocol = 0x01;
	ip_hdr->ttl = 255;
	ip_hdr->tot_len = htons(2 * sizeof(struct iphdr) + 2 * sizeof(struct icmphdr));
	ip_hdr->frag_off = 0;

	icmp_hdr->type = type;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	memset(&icmp_hdr->un, 0, 4);


	ip_hdr->daddr = ip_hdr->saddr;

	ip_hdr->saddr = inet_addr(get_interface_ip(interface));
	
	icmp_hdr->checksum = 0;
	icmp_hdr->checksum =  htons(checksum((uint16_t *)icmp_hdr,
				ntohs(ip_hdr->tot_len) - sizeof(struct iphdr)));

	struct ether_header *eth_hdr = (struct ether_header *) buf;

	memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);
	get_interface_mac(interface, eth_hdr->ether_shost);
	eth_hdr->ether_type = htons(ETHER_IP);

	len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr)
		+ sizeof(struct iphdr) + sizeof(struct icmphdr);
	send_to_link(interface, buf, len);
}

// resumes the sending of the packet after updating the arp table
void continue_after_arp(char* buf, size_t len, int interface)
{
	if (queue_empty(que)) {
		return;
	}
	struct packet *pkt = (struct packet*)queue_deq(que);
	struct arp_header *arp_hdr = (struct arp_header *)( buf + sizeof(struct ether_header));
	struct ether_header *eth_hdr = (struct ether_header *) pkt->buf;

	uint8_t smac[6];
	get_interface_mac(pkt->interface, smac);
	mac_entry[mac_entry_len].ip = arp_hdr->spa;
	for (int i = 0; i < 6; i++) {
		mac_entry[mac_entry_len].mac[i] = arp_hdr->sha[i];
		eth_hdr->ether_dhost[i] = arp_hdr->sha[i];
		eth_hdr->ether_shost[i] = smac[i];
	}
	mac_entry_len++;

	send_to_link(pkt->interface, pkt->buf, pkt->len);
	free(pkt->buf);
	free(pkt);
}

// creates a arp request and sends it to everybody
void create_arp_request(struct route_table_entry *best_route)
{
	char* arp_buf = malloc(sizeof(struct ether_header) + sizeof(struct arp_header));
	size_t arp_len = sizeof(struct ether_header) + sizeof(struct arp_header);

	struct ether_header *eth_hdr = (struct ether_header *) arp_buf;
	hwaddr_aton("FF:FF:FF:FF:FF:FF", eth_hdr->ether_dhost);
	get_interface_mac(best_route->interface, eth_hdr->ether_shost);
	eth_hdr->ether_type = htons(ETHER_ARP);

	struct arp_header *arp_hdr = (struct arp_header *)( arp_buf + sizeof(struct ether_header));
	arp_hdr->htype = htons(1);
	arp_hdr->ptype = htons(ETHER_IP);
	arp_hdr->hlen = 6;
	arp_hdr->plen = 4;
	arp_hdr->op = htons(ARP_REQUEST);

	arp_hdr->spa = inet_addr(get_interface_ip(best_route->interface));
	arp_hdr->tpa = best_route->next_hop;
	get_interface_mac(best_route->interface, arp_hdr->sha);
	hwaddr_aton("FF:FF:FF:FF:FF:FF", arp_hdr->tha);

	send_to_link(best_route->interface, arp_buf, arp_len);
}

// send a arp reply with it's mac address, to the source of the arp request
void arp_reply(char* buf, size_t len, int interface)
{
	struct arp_header *arp_hdr = (struct arp_header *)( buf + sizeof(struct ether_header));

	// resume the queued packet
	if (arp_hdr->op == htons(ARP_REPLY)) {
		continue_after_arp(buf, len, interface);
		return;
	}

	struct ether_header *eth_hdr = (struct ether_header *) buf;
	uint8_t smac[6];
	get_interface_mac(interface, smac);
	arp_hdr->op = htons(ARP_REPLY);

	
	for(int i = 0; i < 6; i++){

		mac_entry[mac_entry_len].mac[i] = arp_hdr->tha[i];
		eth_hdr->ether_dhost[i] = eth_hdr->ether_shost[i];
		eth_hdr->ether_shost[i] = smac[i];

		uint8_t aux = arp_hdr->sha[i];
		arp_hdr->sha[i] = arp_hdr->tha[i];
		arp_hdr->tha[i] = aux;
	}
	struct arp_table_entry *dest_mac_entry = get_mac_entry(arp_hdr->tpa);
	if (dest_mac_entry == NULL) {
		for(int i = 0; i < 6; i++){
			mac_entry[mac_entry_len].mac[i] = arp_hdr->tha[i];
		}
		mac_entry[mac_entry_len].ip = arp_hdr->tpa;
		mac_entry_len++;
	}
	

	uint16_t aux = arp_hdr->spa;
	arp_hdr->spa = arp_hdr->tpa;
	arp_hdr->tpa = aux;

	send_to_link(interface, buf, len);
}

// responds to ip message, and sends it to next hop
void ip_request(char* buf, size_t len, int interface)
{
	struct ether_header *eth_hdr = (struct ether_header *) buf;
	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));

	uint16_t packet_checksum = ntohs(ip_hdr->check);
	ip_hdr->check = 0;
	if(packet_checksum != checksum((uint16_t *)ip_hdr, sizeof(struct iphdr))) {
		return;
	}
	ip_hdr->check = htons(packet_checksum);

	// if the router is the destination of ping, send icmp reply
	if(ip_hdr->daddr == inet_addr(get_interface_ip(interface))) {
		icmp_custom_reply(buf, len, interface, 0);
		return;
	}

	struct route_table_entry *best_route = get_best_route(ip_hdr->daddr);
	// if there is no next hop, send icmp host unreachable
	if(best_route == NULL) {
		icmp_custom_reply(buf, len, interface, 3);
		return;
	}

	// if ttl expired, send icmp timeout
	if(ip_hdr->ttl <= 1) {
		icmp_custom_reply(buf, len, interface, 11);
		return;
	}
	ip_hdr->ttl--;

	ip_hdr->check = ~(~ip_hdr->check +  ~((uint16_t)ip_hdr->ttl + 1) + (uint16_t)ip_hdr->ttl) - 1;
	uint8_t smac[6];
	struct arp_table_entry *dest_mac_entry = get_mac_entry(best_route->next_hop);

	// if we don't know the mac address of next hop, the router needs to update the arp table
	// by sending a arp request to next hop
	if (dest_mac_entry == NULL) {
		struct packet* pkt = malloc(sizeof(struct packet));
		pkt->buf = malloc(MAX_PACKET_LEN);
		memcpy(pkt->buf, buf, len);
		pkt->len = len;
		pkt->interface = best_route->interface;
		queue_enq(que, (void*)pkt);

		create_arp_request(best_route);
		return;
	}

	get_interface_mac(best_route->interface, smac);
	for(int i = 0; i < 6; i++){
		eth_hdr->ether_dhost[i] = dest_mac_entry->mac[i];
		eth_hdr->ether_shost[i] = smac[i];
	}

	send_to_link(best_route->interface, buf, len);
}

int main(int argc, char *argv[])
{
	char* buf = malloc(MAX_PACKET_LEN);

	// Do not modify this line
	init(argc - 2, argv + 2);

	rtable = malloc(sizeof(struct route_table_entry) * MAX_SIZE);

	rtable_len = read_rtable(argv[1], rtable);

	mac_entry = malloc(sizeof(struct arp_table_entry) * MAX_SIZE);

	mac_entry_len = 0;

	que = queue_create();

	// STARTING ROUTER
	while (1) {

		int interface;
		size_t len;

		// Waiting data..
		interface = recv_from_any_link(buf, &len);
		printf("%d", interface);
		DIE(interface < 0, "recv_from_any_links\n");

		struct ether_header *eth_hdr = (struct ether_header *) buf;

		uint8_t *imac = malloc(sizeof(uint8_t) * 6);
		get_interface_mac(interface, imac);

		int found_mac = memcmp(imac, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));

		hwaddr_aton("FF:FF:FF:FF:FF:FF", imac);
		int found_broadcast = memcmp(imac, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));

		// if it's not for you, drop the packet
		if (!found_mac && !found_broadcast) {
			continue;
		}

		// check protocol type
		switch (ntohs(eth_hdr->ether_type)) {
			case ETHER_IP:
				ip_request(buf, len, interface);
				break;
			case ETHER_ARP:
				arp_reply(buf, len, interface);
				break;
			default:
				continue;
				break;
		}
		free(imac);
	}
	free(rtable);
	free(mac_entry);
	free(buf);
}
