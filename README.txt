    -parcurg vectorul de segmente
    -daca adresa la care am primit page fault nu se gaseste in vectorul de segmente rulez handler ul default
    -in caz afirmativ:
            -calculez numarul de pagini din segment si aloc vectorul "data",
                     unde voi retine daca paginile sunt mapate sau nu
            -daca pagina a fost mapata,rulez handler-ul default
            -calculez distanta de la adresa unde s a produs page fault ul si inceputul paginii
            -vad care este ultima pagina de unde trebuiesc citite datele din fisier
            -daca page fault ul s a produs la ultima pagina:
                    -aloc pagina,citesc din fisier de la inceputul paginii unde s a produs page fault ul
                         pana la file-size
                    -zerorizez restul paginii
            -daca page fault ul nu s a produs la ultima pagina:
                    -aloc pagina si citesc din fisier dimensiunea unei pagini,
                        incepand cu pagina unde s a produs page fault ul
            -daca page fault ul este intre mem-size si file-size,aloc memorie zerorizata
            -notez pagina ca a fost mapata si pun permisiunile necesare
				