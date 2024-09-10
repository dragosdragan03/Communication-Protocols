# Copyright Dragan Dragos Ovidiu 323CAb 2023-2024
# (am folosit 2 sleepday-uri)

Pentru implementarea temei m-am folosit de laboratorul 7.

In cadrul acestei teme am creat 3 structuri de care ma voi folosi de ele pe parcurs:
- chat_packet care se ocupa cu trimiterea/receptionarea unui pachet;
- Message pentru receptionarea unui pachet primit de la un client UDP;
- tcp_clients cu detalii despre fiecare client 
(la ce socket este conectat, daca este activ sau inactiv, topicurile la care este abonat si numele acestuia);

Pentru inceput in server.cpp in main() am creat un socket TCP si
unul UDP pe care le voi folosi in functia run_chat_multi_server
care se ocupa cu gestionarea receptionarii si trimiterii pachetelor de pe aceste socketuri.
Tot in functia main apelez bind() pentru a asocia adresa serverului cu socketul tcp si udp create.

In functia run_chat_multi_server() creez un vector de polluri (pentru a nu limita numarul de clienti)
unde retin toate conexiunile facute si un vector de clienti TCP.
La inceput primele 3 sloturi sunt ocupate de listenfd (socketul pe care se asculta conexiuni),
udp_fd (file descriptorul clientului UDP),
STDIN_FILENO (filedescriptorul care se ocupa cu citirea de la tastara din server).

Pentru cazul in care pe socketul listenfd s-a receptionat un semnal,
verific daca subscriberul a mai fost adaugat pana acum si afisez un mesaj in acest caz
si inchid socketul corespunzator, iar in caz contrar adaug socketul nou in vectorull de polluri 
si creez o instanta noua a clientului pe care o adaug in vectorul de clienti TCP. 

Pentru cazul in care pe socketul udp_fd s-a receptionat un semnal,
receptionez pachetul trimis de catre clientul UDP si-l trimit la toti clientii
care sunt abonati la topicul respectiv + implementarea de wildcard.
In acest caz am implementat si trimiterea unui pachet care contine portul 
si ip-ul clientului UDP pe care il trimit la subscriber.

Pentru cazul in care pe socketul STDIN_FILENO s-a receptionat un semnal,
verific daca mesajul citit este "exit" si inchid toti subscriberii inclusiv
serverul si se iese din functie.

In final, verific daca serverul primeste de la un subscriber mesajul "exit", 
unde inchid conexiunea(socketul) cu subscriberul respectiv si setez campul clientului "is_active"
cu false (este inactiv (pentru a nu pierde topicurile la care a fost abonat inainte 
si pentru a le putea relua atunci cand se reconecteaza)).
Daca serverul primeste mesajul "subscribe ____" atunci adaug in vectorul de topicuri ale clientului, topicul. 
Daca serverul primeste "unsubscribe ___", verific daca clientul a fost abonat pana acum
la acest topic si il sterg din vector.

In subscriber.cpp in main() obtin un socket TCP pentru conectarea la server. 
Apoi, trimit un pachet care contine  <ID_CLIENT> <IP_SERVER> <PORT_SERVER> catre server.

In functia run_client() creez un vector pollfd care contine un socket pentru citirea de la tastatura 
si altul pentru citirea de la server.
In cazul in care pe socketul STDIN_FILENO s-a receptionat un semnal, 
receptionez continutul si verific daca contine "subscribe/unsubscribe" si afisez un mesaj corespunzator, 
iar apoi trimit un pachet catre server pentru a putea gestiona topicurile clientului TCP.
In cazul in care pe socketul sockfd s-a receptionat un semnal 
(adica se primeste un pachet de la server), cu ajutorul functiei recv_all receptionez pachetul 
si-l parsez in functie de ce tip este campul "tip_date" din cadrul structurii Message.
In cadrul acestui caz am implementat si receptionarea pachetului care contine portul si ip_portul clientului UDP.

In common.cpp am implementat functiile recv_all si send_all care primesc/trimit un pachet.