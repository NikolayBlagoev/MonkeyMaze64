#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;

char out[1024] = {0};

void sendReceive(int sock){
    char buffer[1024] = { 0 };
    while (true) {
        int valread = recv(sock, buffer, 1024, MSG_DONTWAIT);
        if (strlen(buffer) != 0) { cout << buffer << endl; }
        if (strlen(out) > 0) {
            send(sock, out, strlen(out), 0);
            out[0] = 0;
        }
        buffer[0] = 0;   
    }
}

int main(int argc, char* argv[]) {
  int sock = 0;
  int valread;
  struct sockaddr_in serv_addr;
  
  char buffer[1024] = {0};
  int port = 31691;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cout << "Socket creation error" << endl;
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(31691);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    cout << "Invalid address/ Address not supported" << endl;
    return 1;
  }

  while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    cout << "Waiting for connection" << endl;
    sleep(5);
  }

  cout << "Established connection" << endl;

  sendReceive(sock);

  return 0;
}