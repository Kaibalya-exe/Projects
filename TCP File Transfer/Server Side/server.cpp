#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstdint>

using namespace std;

//Function to ensure all requested bytes are received : 
//Using a seperate function rather than using a loop for chunks :
bool recv_all(int socket_fd, char* buffer, size_t length)
{
    size_t total = 0 ;

    while (total < length)
    {
        ssize_t bytes_received = recv(socket_fd, buffer + total, length - total, 0) ;

        if (bytes_received <= 0)
        {
            perror("Receive failed") ;
            return false ;
        }

        total += bytes_received ;
    }

    return true ;
}

int main()
{
    //Creating server socket :
    int server_fd = socket(AF_INET, SOCK_STREAM, 0) ;

    if (server_fd == -1)
    {
        perror("Socket creation failed") ;
        return 1 ;
    }

    //Allow reuse of local address :
    int opt = 1 ;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ;

    cout << "TCP Socket created successfully!" << endl ;

    //Creating server address structure :
    struct sockaddr_in address{} ;

    address.sin_family = AF_INET ;
    address.sin_port = htons(8080) ;
    address.sin_addr.s_addr = inet_addr("127.0.0.1") ;

    //Binding socket :
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("Bind failed") ;
        close(server_fd) ;
        return 1 ;
    }

    cout << "Socket successfully bound to port 8080!" << endl ;

    //Listening for client :
    if (listen(server_fd, 5) == -1)
    {
        perror("Listen failed") ;
        close(server_fd) ;
        return 1 ;
    }

    cout << "Server is now listening for connections!" << endl ;

    //Accepting client connection :
    int client_fd = accept(server_fd, NULL, NULL) ;

    if (client_fd == -1)
    {
        perror("Accept failed") ;
        close(server_fd) ;
        return 1 ;
    }

    cout << "Client connected successfully!" << endl ;

    //Receiving filename :
    char filename_header[256] = {} ;

    if (!recv_all(client_fd, filename_header, sizeof(filename_header)))
    {
        close(client_fd) ;
        close(server_fd) ;
        return 1 ;
    }

    //Ensure filename is null-terminated :
    filename_header[255] = '\0' ;

    cout << "Filename received: " << filename_header << endl ;

    //Receiving file size :
    char size_header[8] ;

    if (!recv_all(client_fd, size_header, sizeof(size_header)))
    {
        close(client_fd) ;
        close(server_fd) ;
        return 1 ;
    }

    //Decoding file size from network byte order :
    uint64_t file_size = 0 ;

    for (int i = 0 ; i < 8 ; i++)
    {
        file_size = (file_size << 8) |
                    static_cast<unsigned char>(size_header[i]) ;
    }

    cout << "File size received: " << file_size << " bytes" << endl ;

    //Save with a prefix to avoid overwriting original file :
    string output_filename = "received_" + string(filename_header) ;

    ofstream output_file(output_filename, ios::binary) ;

    if (!output_file)
    {
        cout << "File creation failed!" << endl ;
        close(client_fd) ;
        close(server_fd) ;
        return 1 ;
    }

    //Receiving file data :
    char buffer[4096] ;
    uint64_t total_received = 0 ;
    bool transfer_successful = true ;

    while (total_received < file_size)
    {
        //Calculate remaining bytes :
        uint64_t remaining = file_size - total_received ;

        //Determine how many bytes to receive :
        size_t bytes_to_receive =
            (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining ;

        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            bytes_to_receive,
            0
        ) ;

        if (bytes_received > 0)
        {
            output_file.write(buffer, bytes_received) ;

            if (!output_file)
            {
                cout << "File writing failed!" << endl ;
                transfer_successful = false ;
                break ;
            }

            total_received += bytes_received ;
        }
        else if (bytes_received == 0)
        {
            cout << "Connection closed before file was completely received!"
                 << endl ;
            transfer_successful = false ;
            break ;
        }
        else
        {
            perror("Receive failed") ;
            transfer_successful = false ;
            break ;
        }
    }

    //Closing output file :
    output_file.close() ;

    if (!output_file)
    {
        cout << "File closing/writing failed!" << endl ;
        transfer_successful = false ;
    }

    cout << "Total bytes received: " << total_received << endl ;
    cout << "File saved as " << output_filename << endl ;

    if (transfer_successful && total_received == file_size)
        cout << "File transfer completed successfully!" << endl ;
    else
        cout << "File transfer incomplete!" << endl ;

    //Closing sockets :
    close(client_fd) ;
    close(server_fd) ;

    return 0 ;
}
