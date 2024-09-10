# Copyright Dragan Dragos Ovidiu 323CAb 2023-2024
# (am folosit 2 sleepday-uri)

Pentru implementarea temei m-am folosit de laboratorul 9.

Pentru inceput am creat 3 functii de care ma voi folosi de ele pe parcursul temei:
1. is_number - care verifica daca un string contine doar cifre
2. display_error - care afiseaza eraorea din cadrul mesajului primit de la server
3. verify_code - care verifica daca mesajul primit de la server este de tip eroare (cod_eroare >= 300) sau este un cod care semnifica ca este reusit. In cazul in care pachetul contine eroare atunci o afisez.

Functia register -> realizează înregistrarea unui utilizator prin trimiterea unei cereri POST către un server specific. Aceasta se face in felul urmator:
1. se citesc username-ul si parola
2. creeaza un obiect de tip json cu informatiile citite
3. Creează o cerere POST pentru a trimite datele de înregistrare la server.
4.  Deschide o conexiune către server, trimite cererea și primește răspunsul de la server.
5. Afișează un mesaj de succes în cazul în care înregistrarea este reușită, si de eroare in caz contrar.

Functia login_command -> realizeaza inregistrarea unui utilizator prin trimiterea unei cereri POST catre un server specific. Aceasta functie se face smilar cu cea de register. In plus, in cazul in care s-a putut face logarea, extrag cookie-ul din mesaj si il retin.

Functia enter_library_command -> se ocupa cu solicitarea accesului la o biblioteca pentru un utilizator deja autentificat. Aceasta trimite o cerere HTTP GET catre server folosind un cookie pentru a demonstra autentificarea, primeste un raspuns care contine un token de acces si verifica validitatea raspunsului. Daca raspunsul este valid, functia extrage si returneaza token-ul.

Functia get_book_command -> permite utilizatorului sa obtina informatii despre o carte specifica din biblioteca, utilizand un token de acces. Aceasta verifica daca utilizatorul este autentificat si daca are acces la biblioteca, validează ID-ul cartii, construieste o cerere HTTP GET si trimite cererea catre server. Raspunsul serverului, care contine detalii despre carte, este afisat la STDOUT.

Functia get_books_command ->  permite utilizatorului sa obtina o listă de carti din biblioteca. Aceasta functie este similara cu functia get_book.

Functia add_book_command -> permite utilizatorului sa adauge o noua carte in biblioteca digitala, utilizand un token de acces. Aceasta verifica daca utilizatorul este autentificat si daca are acces la biblioteca, citeste detaliile cartii de la utilizator, construieste un obiect JSON cu aceste detalii, si trimite o cerere HTTP POST catre server pentru a adauga cartea. Daca raspunsul de la server indica succes, afiseaza un mesaj de confirmare.

Functia delete_book_command -> permite utilizatorului sa stearga o carte din biblioteca, utilizand un token de acces. Utilizatorul este solicitat sa introduca ID-ul cartii pe care doreste sa o stearga, iar functia verifica daca utilizatorul este autentificat si are acces la biblioteca inainte de a trimite o cerere de stergere catre server. Creeaza cererea HTTP DELETE pentru a trimite cererea de stergere catre server.

Functia logout -> permite utlizatorului sa se delogheze de la serviciul de autentificare al aplicatiei. Aceasta trimite o cerere HTTP GET catre server pentru a efectua operatiunea de delogare utilizand cookie-ul de autentificare furnizat.

Functia compute_get_request -> construieste si returneaza un mesaj HTTP de tip GET care poate fi trimis catre un server pentru a cere o resursa.

Functia compute_post_request -> construiește și returnează un mesaj HTTP de tip POST care poate fi trimis către un server pentru a trimite date.

Am ales sa utilizez biblioteca "nlohmann/json" pentru a putea crea cu usurinta obiecte JSON si pentru a le trimite ca parametrii in cadrul functiilor compute_get_request si compute_post_request.