#include <iostream>;
#include <fstream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
using namespace std;


class UserAccount {       
public:             
    string userName;       
    string password;
    string realName;
    string role;


    UserAccount(string userName, string password) {
        this->userName = userName;
        this->password = password;
    }
    static boolean registerNewAccount(UserAccount newAccount) {
        ifstream file;
        ofstream newuser;
        string username = newAccount.userName;
        string password = newAccount.password;
        string user, line;
        string pass;
        file.open("users.txt", ios::app);
        std::ifstream file1("users.txt");

        newuser.open("users.txt", ios::app);
        bool uservalid = true;

        std::string str;
        while (std::getline(file1, str)) {
            istringstream line(str);
            string splittedString;
            std::getline(line, splittedString, ' ');
            string lineUserName = splittedString;
            if (lineUserName._Equal(username)) {
                std::cout << "Deny Register Account: " << username << '\n';

                file.close();
                newuser.close();
                return false;

            }
        }

        newuser << username << " " << password << endl;;
        std::cout << "Accept Register Account: " << username << '\n';

        file.close();
        newuser.close();
        return true;

    }

    static boolean accountLoggin(UserAccount newAccount) {
        ifstream file;
        ofstream newuser;
        string username = newAccount.userName;
        string password = newAccount.password;
        string user, line;
        string pass;
        file.open("users.txt", ios::app);
        std::ifstream file1("users.txt");

        bool uservalid = true;

        std::string str;
        while (std::getline(file1, str)) {
            istringstream line(str);
            string splittedString;
            std::getline(line, splittedString, ' ');
            string lineUserName = splittedString;
            std::getline(line, splittedString, ' ');
            string linePassword = splittedString;
            if ((lineUserName._Equal(username) && linePassword._Equal(password))) {
                std::cout << "Accept Loggin for " << username << '\n';
                file.close();
                return true;
            }
        }
        std::cout << "Deny Loggin for: " << username << '\n';

        file.close();
        return false;

    }


};
