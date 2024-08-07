#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>  // Para inet_ntoa, etc.
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>Hello, World</html>\r\n";

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    
    // Crear un socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Socket created\n");

    // Configurar la estructura sockaddr_in
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Vincular el socket a la dirección
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Socket successfully bound to address\n");

    // Escuchar conexiones
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Listening for connections\n");

    // Aceptar conexiones y manejar solicitudes
    for (;;) {
        int clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;  // Continuar con la siguiente iteración del bucle
        }

        printf("Connection accepted\n");

        // Leer datos del cliente
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead == SOCKET_ERROR) {
            printf("Read failed: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            continue;
        }
        buffer[bytesRead] = '\0';  // Asegurarse de que buffer sea una cadena terminada en nulo

        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), method, version, uri);

        // Enviar respuesta al cliente
        int bytesSent = send(clientSocket, resp, (int)strlen(resp), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Write failed: %d\n", WSAGetLastError());
        }

        // Cerrar el socket del cliente
        closesocket(clientSocket);
    }

    // Limpiar y cerrar
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}