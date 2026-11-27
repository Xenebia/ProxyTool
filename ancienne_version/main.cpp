#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>

#include "config.h"
#include "proxy.h"

namespace fs = std::filesystem;

// DATE / HEURE

std::string getDateTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time =
        std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
    localtime_s(&localTime, &time);
    std::ostringstream stream;
    stream << std::put_time(
        &localTime,
        "%Y-%m-%d %H:%M:%S"
    );
    return stream.str();
}

// JOURNAL

void writeAudit(const std::string& action)
{
    try
    {
        fs::create_directories(AUDIT_DIRECTORY);
        std::ofstream file(
            AUDIT_FILE,
            std::ios::app
        );

        if (!file)
            return;

        file
            << "["
            << getDateTime()
            << "] "
            << action
            << "\n";
    }
    catch (...)
    {
        // Le programme continue même si le journal
        // ne peut pas être écrit.
    }
}

// MOT DE PASSE

bool checkPassword()
{
    std::string password;

    std::cout << "Mot de passe : ";
    std::cin >> password;

    return password == PROGRAM_PASSWORD;
}

// MENU

void showMenu()
{
	std::cout << "\n";
    std::cout << "ProxyTool C++\n";
    std::cout << "\n";
    std::cout << "1. Activer le proxy\n";
    std::cout << "2. Desactiver le proxy\n";
    std::cout << "3. Quitter\n";
    std::cout << "\n> ";
}

// MAIN

int main()
{
    std::cout
        << "ProxyTool C++\n"
        << "\n";

    std::cout
        << "Ce programme modifie le proxy Windows\n"
        << "pour le compte utilisateur actuel.\n\n";

    if (!checkPassword())
    {
        std::cout << "\nMot de passe incorrect.\n";

        writeAudit(
            "Tentative avec un mot de passe incorrect."
        );

        return 1;
    }

    writeAudit(
        "Authentification reussie."
    );

    while (true)
    {
        showMenu();
        int choice = 0;
        if (!(std::cin >> choice))
        {
            if (std::cin.eof())
                break;

            // Saisie non numerique : on vide le flux (sinon boucle infinie).
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
        }

        // ACTIVER

        if (choice == 1)
        {
            std::cout
                << "\nCeci va modifier le proxy systeme de Windows\n"
                << "pour le compte utilisateur actuel.\n"
                << "Si le proxy n'est pas joignable sur ce reseau,\n"
                << "l'acces a Internet sera coupe jusqu'a la desactivation.\n"
                << "Continuer ? (o/n) : ";

            std::string answer;
            std::cin >> answer;

            if (answer != "o" && answer != "O" &&
                answer != "oui" && answer != "OUI" && answer != "Oui")
            {
                std::cout << "\nActivation annulee.\n";
                writeAudit("Activation du proxy annulee par l'utilisateur.");
                continue;
            }

            std::cout << "\nConfiguration du proxy...\n";
            if (setProxy())
            {
                std::cout
                    << "Proxy active : "
                    << "***.***.***.***"
                    << ":"
                    << "****##"
                    << "\n";
				
                writeAudit(
                    "Proxy active : " +
                    std::string(PROXY_ADDRESS) +
                    ":" +
                    std::string(PROXY_PORT)
                );
            }
            else
            {
                std::cout
                    << "Erreur lors de la configuration.\n";

                writeAudit(
                    "ERREUR : impossible de configurer le proxy."
                );
            }
        }

        // DESACTIVER

        else if (choice == 2)
        {
            std::cout << "\nDesactivation du proxy...\n";
            if (disableProxy())
            {
                std::cout
                    << "Proxy desactive.\n";

                writeAudit(
                    "Proxy desactive."
                );
            }
            else
            {
                std::cout
                    << "Erreur lors de la desactivation.\n";

                writeAudit(
                    "ERREUR : impossible de desactiver le proxy."
                );
            }
        }

        // QUITTER

        else if (choice == 3)
        {
            writeAudit(
                "Programme ferme."
            );

            std::cout
                << "\nFermeture de ProxyTool.\n";

            break;
        }
        else
        {
            std::cout
                << "\nChoix invalide.\n";
        }
    }
    return 0;
}