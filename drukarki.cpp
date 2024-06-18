#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
// do budowania stringow, aby wyswietlanie nie bylo przerywane przez inne watki
#include <sstream>

const int LICZBA_DRUKAREK = 4;
const int LICZBA_WATKOW_DRUKUJACYCH = 7;

// Kolory ANSI
const std::string RESET = "\033[0m";
const std::string BLUE = "\033[1;34m";
const std::string GREEN = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string RED = "\033[1;31m";
const std::string CYAN = "\033[1;36m";

class Monitor_PK {
private:
    // zalozylem ze dane sa typu int
    std::queue<int> queue; // kolejka do przekazywania/pobierania danych
    std::mutex mtx; // zamek
    std::condition_variable cv; // zmienna warunkowa

public:
    Monitor_PK() {} 

    // wstawianie do monitora
    void wstaw(int n) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(n);
        cv.notify_one();
    }

    // pobieranie z monitora
    int pobierz() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        int value = queue.front();
        queue.pop();

        return value;
    }
};

Monitor_PK klucze;
std::vector<Monitor_PK> bufor_drukarki(LICZBA_DRUKAREK);

void drukuj(int nr_drukarki, int dane) {
    std::ostringstream oss;
    oss << BLUE << "Drukarka " << nr_drukarki << " drukuje dane " << dane << RESET << std::endl;
    std::cout << oss.str();
}

void watek_drukarki(int n) {
    klucze.wstaw(n);

    while(true) {
        int dane = bufor_drukarki[n].pobierz();
        drukuj(n, dane);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// seed przekazywany do kazdego watka z osobna poniewaz inaczej nie dziala losowosc
void watek_drukujacy(unsigned int seed) {
    srand(seed);
    while(true) {
        // pobieranie klucza drukarki
        int klucz_drukarki = klucze.pobierz();
        {
            std::ostringstream oss;
            oss << GREEN << "Pobrano klucz nr " << klucz_drukarki << RESET << std::endl;
            std::cout << oss.str();
        }
        // male opoznienie
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // przesylanie danych
        int liczba_danych = 5; // przykladowa liczba danych
        for(int i = 0; i < liczba_danych; i++) {
            int dane = rand() % 1000;
            bufor_drukarki[klucz_drukarki].wstaw(dane);
            {
                std::ostringstream oss;
                oss << YELLOW << "Wstawiono dane " << dane << " do drukarki " << klucz_drukarki << RESET << std::endl;
                std::cout << oss.str();
            }
        }

        // zwrocenie klucza drukarki
        klucze.wstaw(klucz_drukarki);
        {
            std::ostringstream oss;
            oss << RED << "Oddano klucz nr " << klucz_drukarki << RESET << std::endl;
            std::cout << oss.str();
        }
        // czekanie pare sekund
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

int main(int argc, char** argv) {
    {
        std::ostringstream oss;
        oss << CYAN << "Exit with Ctrl+C" << RESET << std::endl;
        std::cout << oss.str();
    }

    // potrzebne do zainicjowania generatora losowego
    auto aktualny_czas = time(NULL);

    std::vector<std::thread> watki_drukarek;
    for(int i = 0; i < LICZBA_DRUKAREK; i++)
        watki_drukarek.push_back(std::thread(watek_drukarki, i));

    std::vector<std::thread> watki_drukujace;
    for(int i = 0; i < LICZBA_WATKOW_DRUKUJACYCH; i++)
        watki_drukujace.push_back(std::thread(watek_drukujacy, aktualny_czas + i));

    for(auto& t : watki_drukarek)
        t.join();

    for(auto& t: watki_drukujace)
        t.join();

    return EXIT_SUCCESS;
}

