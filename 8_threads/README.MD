# Filtrowanie obrazów

Jedną z najprostszych operacji jaką można wykonać na obrazie jest operacja filtrowania (splotu). Operacja ta przyjmuje na wejściu dwie macierze:

Macierz IN×M reprezentującą obraz. Dla uproszczenia rozważamy jedynie obrazy w 256 odcieniach szarości. Każdy element macierzy I jest więc liczbą całkowitą z zakresu 0 do 255.
Macierz Kc×c reprezentującą filtr. Elementami tej macierzy są liczby zmiennoprzecinkowe. Dla uproszczenia zakładamy, że elementy macierzy K sumują się do jedności: ∑ci=1∑cj=1K[i,j]=1.
Operacja filtrowania tworzy nowy obraz J, którego piksele mają wartość:

J[x,y]=round(sx,y),

sx,y=∑ci=1∑cj=1I[max{1,x−ceil(c/2)+i},max{1,y−ceil(c/2)+j}]∗k[i,j].

Operacja  round()  oznacza zaokrąglenie do najbliższej liczby całkowitej a ceil() zaokrąglenie w górę do najbliższej liczby całkowitej.

Zwróć uwagę, że w powyższym opisie przyjęto matematyczną konwencję indeksowania elementów macierzy - od indeksu 1.

Zadanie 1
Napisz program, który wykonuje wielowątkową operację filtrowania obrazu. Program przyjmuje w argumentach wywołania:

liczbę wątków,
sposób podziału obrazu pomiędzy wątki, t.j. jedną z dwóch opcji: block / interleaved,
nazwę pliku z wejściowym obrazem,
nazwę pliku z definicją filtru,
nazwę pliku wynikowego.
Po wczytaniu danych (wejściowy obraz i definicja filtru) wątek główny tworzy tyle nowych wątków, ile zażądano w argumencie wywołania. Utworzone wątki równolegle tworzą wyjściowy (filtrowany) obraz. Każdy stworzony wątek odpowiada za wygenerowanie części wyjściowego obrazu:

Gdy program został uruchomiony z opcją block, k-ty wątek wylicza wartości pikseli w pionowym pasku o współrzędnych x-owych w przedziale od (k−1)∗ceil(N/m) do k∗ceil(N/m)−1, gdzie N to szerokość wyjściowego obrazu a m to liczba stworzonych wątków.
Gdy program został uruchomiony z opcją interleaved, k-ty wątek wylicza wartości pikseli, których współrzędne x-owe to: k−1, k−1+m, k−1+2∗m, k−1+3∗m, itd. (ponownie, m to liczba stworzonych wątków).
Po wykonaniu obliczeń wątek kończy pracę i zwraca jako wynik (patrz pthread_exit) czas rzeczywisty spędzony na tworzeniu przydzielonej mu części wyjściowego obrazu. Czas ten należy zmierzyć z dokładnością do mikrosekund. Wątek główny czeka na zakończenie pracy przez wątki wykonujące operację filtrowania. Po zakończeniu każdego wątku, wątek główny odczytuje wynik jego działania i wypisuje na ekranie informację o czasie, jaki zakończony wątek poświęcił na filtrowanie obrazu (wraz z identyfikatorem zakończonego wątku). Dodatkowo, po zakończeniu pracy przez wszystkie stworzone wątki, wątek główny zapisuje powstały obraz do pliku wynikowego i wypisuje na ekranie czas rzeczywisty spędzony w całej operacji filtrowania (z dokładnością do mikrosekund). W czasie całkowitym operacji filtrowania należy uwzględnić narzut związany z utworzeniem i zakończeniem wątków (ale bez czasu operacji wejścia/wyjścia).

Wykonaj pomiary czasu operacji filtrowania dla obrazu o rozmiarze kilkaset na kilkaset pikseli i dla kilku filtrów (można wykorzystać losowe macierze filtrów). Testy przeprowadź dla 1, 2, 4, i 8 wątków. Rozmiar filtrów dobierz w zakresie 3≤c≤65, tak aby uwidocznić wpływ liczby wątków na czas operacji filtrowania. Eksperymenty wykonaj dla obu wariantów podziału obrazu pomiędzy wątki (block  i interleaved). Wyniki zamieść w pliku Times.txt i dołącz do archiwum z rozwiązaniem zadania.
Format wejścia-wyjścia
Program powinien odczytywać i zapisywać obrazy w formacie ASCII PGM (Portable Gray Map). Pliki w tym formacie mają nagłówek postaci:

P2
W H
M
...

gdzie: W to szerokość obrazu w pikselach, H to wysokość obrazu w pikselach a M to maksymalna wartość piksela. Zakładamy, że obsługujemy jedynie obrazy w 256 odcieniach szarości: od 0 do 255 (a więc M=255). Po nagłówku, w pliku powinno być zapisanych W*H liczb całkowitych reprezentujących wartości kolejnych pikseli. Liczby rozdzielone są białymi znakami (np. spacją). Piksele odczytywane są wierszami, w kolejności od lewego górnego do prawego dolnego rogu obrazu.

Przykładowe obrazy w formacie ASCII PGM (jak również opis formatu) można znaleźć pod adresem: http://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html

W pierwszej linii pliku z definicją filtru powinna znajdować się liczba całkowita c określająca rozmiar filtru. Dalej, plik powinien zawierać c2 liczb zmiennoprzecinkowych określających wartości elementów filtru (w kolejności wierszy, od elementu K[1,1] do elementu K[c,c]).
