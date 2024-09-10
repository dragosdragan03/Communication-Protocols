#include "include/queue.h"
#include "include/lib.h"
#include "include/protocols.h"
#include <arpa/inet.h> /* ntoh, hton and inet_ functions */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Routing table
IP urile hostilor*/
struct route_table_entry *rtable;
int rtable_len;

/* ARP table*/
struct arp_table_entry *arp_table;
int arp_table_len;

struct queue *packets_queue;
struct queue *packets_queue_len;
struct queue *packets_interface;

int compare_table(const void *first_value, const void *second_value) {
	struct route_table_entry *v1 = (struct route_table_entry *)first_value;
	struct route_table_entry *v2 = (struct route_table_entry *)second_value;

	if (ntohl(v1->prefix) == ntohl(v2->prefix)) {
		if (ntohl(v1->mask) > ntohl(v2->mask))
			return 1;
		else if (ntohl(v1->mask) < ntohl(v2->mask))
			return -1;
		else
			return 0;
	} else
		if (ntohl(v1->prefix) > ntohl(v2->prefix))
			return 1;
		else
			return -1;
}

/*
functie care face o cautare binara pentru a returna cea mai buna ruta (&rtable[i]), sau NULL daca nu a fost gasita
*/
struct route_table_entry *get_best_route(uint32_t ip_dest) {
	int left = 0, right = rtable_len - 1, index = -1;
	while (left <= right) {
		int mid = left + (right - left) / 2;

		if (rtable[mid].prefix == (ip_dest & rtable[mid].mask)) {
			if ((ntohl(rtable[mid].mask) > ntohl(rtable[index].mask)) || index == -1) {
				index = mid;
			}
		}

		if (ntohl(rtable[mid].prefix) > ntohl(ip_dest))
			right = mid - 1;
		else
			left = mid + 1;
	}

	if (index == -1)
		return NULL;

	return &rtable[index];
}

/*  parcurg lista arp_table si returnez (&arp_table[i])
 care se potriveste cu adresa ip (given_ip) data ca parametru
 */
struct arp_table_entry *get_arp_entry(uint32_t given_ip) {

	for (int i = 0; i < arp_table_len; i++) { // pentru a afla destinatia routerului
		if (arp_table[i].ip == given_ip)
			return &arp_table[i];
	}

	return NULL;
}

/*functie care primeste un arp_request si trimite un arp_reply*/
void receive_arp_request(char *buf, int interface, size_t packet_len)
{
	struct ether_header *eth_hdr = (struct ether_header *)buf;
	struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));

	//Antetul Ethernetului
	uint8_t mac[6];
	get_interface_mac(interface, mac);
	//interschimb destinatarul cu sursa
	for (int i = 0; i < 6; i++) {
		eth_hdr->ether_dhost[i] = eth_hdr->ether_shost[i];
		eth_hdr->ether_shost[i] = mac[i];

		arp_hdr->tha[i] = arp_hdr->sha[i];
		arp_hdr->sha[i] = mac[i];
	}

	//Antetul ARP ului
	arp_hdr->tpa = arp_hdr->spa;
	arp_hdr->spa = inet_addr(get_interface_ip(interface));

	arp_hdr->op = htons(2); // transform pachetul intr un pachet de tip arp reply

	send_to_link(interface, buf, packet_len);
}

/* este primit un arp_reply si trimite pachetul care asteapta MAC ul*/
void receive_arp_reply(char *buf)
{
	struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));

	struct arp_table_entry *new_mac;
	new_mac = get_arp_entry(arp_hdr->spa); // verific sa vad daca exista deja adresa ip returnata

	if (new_mac) // inseamna ca exista deja adresa deci nu fac nimic
		return;

	// inseamna ca nu exista adresa si trebuie adaugata in arp_table + verific in queue daca se poate trimite un packet
	arp_table[arp_table_len].ip = arp_hdr->spa;
	for (int i = 0; i < 6; i++)
		arp_table[arp_table_len].mac[i] = arp_hdr->sha[i];

	arp_table_len++;

	if (queue_empty(packets_queue))
		return;

	struct cell *head = packets_queue->head; // retin primul nod in queue care de fapt este un packet
	struct cell *head_for_len = packets_queue_len->head;
	struct cell *head_for_interface = packets_interface->head;

	char *packet = (char *)head->element; // continutul pachetului
	size_t packet_len = *(size_t *)head_for_len->element;
	int interface = *(int *)head_for_interface->element;

	struct ether_header *eth_hdr = (struct ether_header *)packet;
	struct iphdr *ip_hdr = (struct iphdr *)(packet + sizeof(struct ether_header)); // headerul IP din packetul bagat in coada
	struct route_table_entry *rtable_best_route;
	rtable_best_route = get_best_route(ip_hdr->daddr);

	struct arp_table_entry *mac_to_send;
	mac_to_send = get_arp_entry(rtable_best_route->next_hop);
	uint8_t mac[6];
	get_interface_mac(interface, mac);
	for (int i = 0; i < 6; i++) {
		eth_hdr->ether_dhost[i] = mac_to_send->mac[i]; // macul urmatoarei destinatii
		eth_hdr->ether_shost[i] = mac[i];
	}

	send_to_link(rtable_best_route->interface, packet, packet_len);
	queue_deq(packets_queue);
	queue_deq(packets_queue_len);
	queue_deq(packets_interface);

}

void send_arp_request(char *buf, int interface)
{
	char *packet_to_send = malloc(sizeof(struct ether_header) + sizeof(struct arp_header)); // aloc dinamic un pachet de tip ARP req

	struct ether_header *new_eth_hdr = (struct ether_header *)packet_to_send; // headerul ethernetului
	struct arp_header *new_arp_header = (struct arp_header *)(packet_to_send + sizeof(struct ether_header)); // headerul IP

	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header)); // headerul IP din pachetul primit

	//Antetul headerului
	new_eth_hdr->ether_type = htons(ETHERTYPE_ARP);

	struct route_table_entry *rtable_best_route = get_best_route(ip_hdr->daddr);

	uint8_t mac[6];
	get_interface_mac(rtable_best_route->interface, mac);
	for (int i = 0; i < 6; i++) {
		new_eth_hdr->ether_dhost[i] = 0xFF;
		new_eth_hdr->ether_shost[i] = mac[i];

		new_arp_header->tha[i] = 0;
		new_arp_header->sha[i] = mac[i];
	}

	memcpy(packet_to_send, new_eth_hdr, sizeof(struct ether_header));

	// Antetul ARP ului
	new_arp_header->htype = htons(1);
	new_arp_header->ptype = htons(ETHERTYPE_IP);
	new_arp_header->hlen = 6;
	new_arp_header->plen = 4;
	new_arp_header->op = htons(1);
	new_arp_header->tpa = rtable_best_route->next_hop;
	new_arp_header->spa = inet_addr(get_interface_ip(rtable_best_route->interface));

	memcpy(packet_to_send + sizeof(struct ether_header), new_arp_header, sizeof(struct arp_header));

	send_to_link(rtable_best_route->interface, packet_to_send, sizeof(struct ether_header) + sizeof(struct arp_header));
}

/*formez un pachet de tip icmp si l trimit*/
void icmp_message(char *buf, int interface, int type)
{
	unsigned long total_size_struct = sizeof(struct ether_header) + 2 * sizeof(struct iphdr) + sizeof(struct icmphdr); // spatiul structurilor din pachet
	char *payload = (char *)(buf + total_size_struct - sizeof(struct iphdr) - sizeof(struct icmphdr)); // payload ul
	char *new_buf = malloc(total_size_struct + 8); // pointer catre inceputul pachetului pe care vreau sa l trimit de tip icmp

	struct ether_header *eth_hdr = (struct ether_header *)buf; // headerul ethernetului
	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header)); // headerul IP

	//Populez noul eth_hdr
	uint8_t mac[6];
	get_interface_mac(interface, mac);
	struct ether_header *new_eth_hdr = malloc(sizeof(struct ether_header));
	for (int i = 0; i < 6; i++) {
		new_eth_hdr->ether_dhost[i] = eth_hdr->ether_shost[i];
		new_eth_hdr->ether_shost[i] = mac[i];
	}

	new_eth_hdr->ether_type = htons(ETHERTYPE_IP);
	memcpy(new_buf, new_eth_hdr, sizeof(struct ether_header));
	int current_size = sizeof(struct ether_header);

	// Populez noul ip_hdr
	struct iphdr *new_ip_hdr = malloc(sizeof(struct iphdr));
	new_ip_hdr->tos = 0;
	new_ip_hdr->frag_off = 0;
	new_ip_hdr->version = 4;
	new_ip_hdr->ihl = 5;
	new_ip_hdr->id = 1;
	new_ip_hdr->daddr = ip_hdr->saddr;
	new_ip_hdr->protocol = 1;
	new_ip_hdr->tot_len = htons(2 * sizeof(struct iphdr) + sizeof(struct icmphdr) + 8);
	new_ip_hdr->saddr = inet_addr(get_interface_ip(interface));
	new_ip_hdr->ttl = 50;
	new_ip_hdr->check = htons(checksum((uint16_t *)new_ip_hdr, sizeof(struct iphdr)));

	//adaug in new_buf ip_headerul format 
	memcpy(new_buf + current_size, new_ip_hdr, sizeof(struct  iphdr));
	current_size += sizeof(struct iphdr);

	//populez new_icmphd
	struct icmphdr *new_icmphdr = malloc(sizeof(struct  icmphdr *));
	new_icmphdr->type = type;
	new_icmphdr->code = 0;
	new_icmphdr->checksum = htons(checksum((uint16_t *)new_icmphdr, sizeof(struct icmphdr)) + sizeof(struct iphdr) + 8);

	// adaug icmp_header pe care l am creat si alocat dinamic in new_buf
	memcpy(new_buf + current_size, new_icmphdr, sizeof(struct  icmphdr));
	current_size += sizeof(struct  icmphdr);

	// adaug in new_buf, headerul IPv4 pe care l am primit (originalul)
	memcpy(new_buf + current_size, ip_hdr, sizeof(struct  iphdr));
	current_size += sizeof(struct  iphdr);

	// adaug primii 8 biti din payload, in pachet
	memcpy(new_buf + current_size, payload, 8);

	send_to_link(interface, new_buf, total_size_struct + 8); // trimit pachetul icmp format
}

/*functie care trimite un pachet icmp de tip Echo reply
in loc sa aloc dinamic un nou pachet si sa l formez de la 0, aleg sa modific header-ele pe care le am primit in pachet*/
void icmp_echo_request(char *buf, int interface, size_t packet_len)
{
	struct ether_header *eth_hdr = (struct ether_header *)buf; // headerul ethernetului
	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));

	uint8_t mac[6];
	get_interface_mac(interface, mac);
	for (int i = 0; i < 6; i++) {
		eth_hdr->ether_dhost[i] = eth_hdr->ether_shost[i];
		eth_hdr->ether_shost[i] = mac[i];
	}

	ip_hdr->daddr = ip_hdr->saddr;
	ip_hdr->saddr = inet_addr(get_interface_ip(interface));
	ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));

	icmp_hdr->type = 0; // schimb type ul pachetului
	send_to_link(interface, buf, packet_len); // trimit pachetul ICMP modificat
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];
	int interface;
	size_t packet_len;

	// Do not modify this line
	init(argc - 2, argv + 2);

	rtable = malloc(sizeof(struct route_table_entry) * 80000); // aloc dinamic tabela de rutare

	arp_table = malloc(sizeof(struct arp_table_entry) * 100);

	rtable_len = read_rtable(argv[1], rtable);

	qsort(rtable, rtable_len, sizeof(struct route_table_entry), compare_table);

	packets_queue = queue_create(); // creez lista de pachete
	packets_queue_len = queue_create();
	packets_interface = queue_create();

	while (1) {
		interface = recv_from_any_link(buf, &packet_len); // inseamna ca primeste pachetul

		struct ether_header *eth_hdr = (struct ether_header *)buf; // headerul ethernetului

		if (eth_hdr->ether_type == ntohs(ETHERTYPE_ARP)) { // inseamna ca pachetul este de tip ARP
			struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header)); // retin structura arp ului
			if (arp_hdr->op == htons(1)) { // inseamna ca este un pachet ARP de tip request
				receive_arp_request(buf, interface, packet_len);
			} else { // inseamna ca este un pachet ARP de tip reply
				receive_arp_reply(buf);
			}
		} else if (eth_hdr->ether_type == ntohs(ETHERTYPE_IP)) {
			struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header)); // headerul IP

			if (ip_hdr->protocol == 1) {
				struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));
				if (icmp_hdr->type == 8 && ip_hdr->daddr == inet_addr(get_interface_ip(interface))) {
					icmp_echo_request(buf, interface, packet_len);
					continue;
				}
			}

			int sum1 = ntohs(ip_hdr->check);
			ip_hdr->check = 0;
			int sum2 = checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)); // verific daca a fost primit bine pachdetul (nu a fost corupt pachetul)

			if (sum1 == sum2) { // daca sunt egale in seamna ca pachetul nu a fost corupt
				struct route_table_entry *rtable_best_route;
				rtable_best_route = get_best_route(ip_hdr->daddr);
				if (rtable_best_route == NULL) { // trimit un pachet ICMP, deoarece nu s a putut gasi best_route
					icmp_message(buf, interface, 3);
					continue;
				}

				if (ip_hdr->ttl > 1) {
					ip_hdr->ttl--;
					ip_hdr->check = 0;
					ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));
				} else { // inseamna ca trebuie sa trimit un pachet ICMP, deoarece a fost depasit TTL 
					icmp_message(buf, interface, 11);
					continue;
				}

				struct arp_table_entry *new_mac;
				new_mac = get_arp_entry(rtable_best_route->next_hop);
				if (new_mac == NULL) {
					// daca nu a gasit adresa mac atunci pachetul este bagat in coada si trimis un arp_request
					send_arp_request(buf, interface);

					char *packet_to_enq = malloc(packet_len);
					memcpy(packet_to_enq, buf, packet_len);
					queue_enq(packets_queue, packet_to_enq);

					size_t *packet_len_to_enq = malloc(sizeof(size_t));
					memcpy(packet_len_to_enq, &packet_len, sizeof(size_t));
					queue_enq(packets_queue_len, packet_len_to_enq);

					int *interface_to_enq = malloc(sizeof(int));
					memcpy(interface_to_enq, &interface, sizeof(int));

					queue_enq(packets_interface, interface_to_enq);

					continue;
				}

				uint8_t mac[6];
				get_interface_mac(interface, mac);
				for (int i = 0; i < 6; i++) {
					eth_hdr->ether_dhost[i] = new_mac->mac[i]; // macul urmatoarei destinatii
					eth_hdr->ether_shost[i] = mac[i];
				}
				send_to_link(rtable_best_route->interface, buf, packet_len);
			}
		}
	}
}
