#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

using namespace std;

const int PORT = 8080;

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r)
{
    wasSigHup = 1;
}

int main()
{
    // Настройка обработчика сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHupHandler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // Блокировка сигнала SIGHUP
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Слушать на всех интерфейсах
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));

    listen(server_fd, 10);  // Очередь до 10 подключений

    cout << "Сервер запущен на порту " << PORT << endl;

    vector<int> clients;

    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int maxFd = server_fd;

        // Добавляем клиентские сокеты в набор
        for (int clientFd : clients) {
            FD_SET(clientFd, &readfds);
            if (clientFd > maxFd) {
                maxFd = clientFd;
            }
        }

        // Вызываем pselect один раз для всего набора
        int ready = pselect(maxFd + 1, &readfds, NULL, NULL, NULL, &origMask);

        if (ready == -1) {
            if (errno == EINTR) {
                if (wasSigHup) {
                    cout << "Получен сигнал SIGHUP" << endl;
                    wasSigHup = 0;
                }
                continue; // Перезапускаем select после сигнала
            }
            perror("pselect failed");
            break;
        }

        // Проверяем серверный сокет на новые подключения
        if (FD_ISSET(server_fd, &readfds)) {
            int clientFd = accept(server_fd, NULL, NULL);
            if (clientFd != -1) {
                clients.push_back(clientFd);
                cout << "Новый клиент подключен: " << clientFd << endl;
            }
            ready--;
        }

        // Проверяем клиентские сокеты на данные
        if (ready > 0) {
            auto it = clients.begin();
            while (it != clients.end()) {
                int clientFd = *it;

                if (FD_ISSET(clientFd, &readfds)) {
                    char buffer[1024];
                    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

                    if (bytesRead > 0) {
                        cout << "Получено " << bytesRead << " байт от клиента " << clientFd << endl;
                        send(clientFd, buffer, bytesRead, 0);
                    }
                    else if (bytesRead == 0) {
                        // Соединение закрыто клиентом
                        cout << "Клиент " << clientFd << " отключился" << endl;
                        close(clientFd);
                        it = clients.erase(it);
                        continue;
                    }
                    else {
                        // Ошибка при чтении
                        perror("recv failed");
                        close(clientFd);
                        it = clients.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }
    }
}