#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdint>

using namespace std ;

//Function to ensure all bytes are sent :
bool send_all(int socket_fd, const char* buffer, size_t length)
{
    size_t total = 0 ;

    while (total < length)
    {
        ssize_t bytes_sent = send(socket_fd, buffer + total,
                                  length - total, 0) ;

        if (bytes_sent <= 0)
        {
            perror("Send failed") ;
            return false ;
        }

        total += bytes_sent ;
    }

    return true ;
}

int main()
{
    //Getting filename from user :
    string filename ;

    cout << "Enter filename to transfer: " ;
    cin >> filename ;

    //Validating filename :
    if (filename.empty() || filename.length() > 255 ||
        filename.find('/') != string::npos ||
        filename.find('\\') != string::npos)
    {
        cout << "Invalid filename!" << endl ;
        return 1 ;
    }

    //Opening source file :
    ifstream input_file(filename, ios::binary) ;

    if (!input_file)
    {
        cout << "File opening failed!" << endl ;
        return 1 ;
    }

    //Getting file size :
    input_file.seekg(0, ios::end) ;

    streamsize file_size = input_file.tellg() ;

    if (file_size < 0)
    {
        cout << "Unable to determine file size!" << endl ;
        input_file.close() ;
        return 1 ;
    }

    input_file.seekg(0, ios::beg) ;

    if (!input_file)
    {
        cout << "File positioning failed!" << endl ;
        input_file.close() ;
        return 1 ;
    }

    cout << "File size: " << file_size << " bytes" << endl ;

    //Creating client socket :
    int client_fd = socket(AF_INET, SOCK_STREAM, 0) ;

    if (client_fd == -1)
    {
        perror("Socket creation failed") ;
        input_file.close() ;
        return 1 ;
    }

    //Defining server address :
    struct sockaddr_in server_address{} ;

    server_address.sin_family = AF_INET ;
    server_address.sin_port = htons(8080) ;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1") ;

    //Connecting to server :
    if (connect(client_fd, (struct sockaddr*)&server_address,
                sizeof(server_address)) == -1)
    {
        perror("Connection failed") ;
        input_file.close() ;
        close(client_fd) ;
        return 1 ;
    }

    cout << "Connected to server!" << endl ;

    //Sending filename to server :
    char filename_header[256] = {} ;

    strncpy(filename_header, filename.c_str(),
            sizeof(filename_header) - 1) ;

    if (!send_all(client_fd, filename_header,
                  sizeof(filename_header)))
    {
        input_file.close() ;
        close(client_fd) ;
        return 1 ;
    }

    cout << "Filename sent successfully!" << endl ;

    //Converting file size to 64-bit unsigned integer :
    uint64_t file_size_network = static_cast<uint64_t>(file_size) ;

    //Encoding file size in network byte order (big-endian) :
    char size_header[8] ;

    for (int i = 0 ; i < 8 ; i++)
    {
        size_header[7 - i] = (file_size_network >> (i * 8)) & 0xFF ;
    }

    //Sending file size to server :
    if (!send_all(client_fd, size_header, sizeof(size_header)))
    {
        input_file.close() ;
        close(client_fd) ;
        return 1 ;
    }

    cout << "File size sent successfully!" << endl ;

    //Creating buffer :
    char buffer[4096] ;

    //Reading and sending file data :
    uint64_t total_sent = 0 ;

    while (true)
    {
        input_file.read(buffer, sizeof(buffer)) ;

        streamsize bytes_read = input_file.gcount() ;

        if (bytes_read > 0)
        {
            //Sending the entire chunk using send_all() :
            if (!send_all(client_fd, buffer, bytes_read))
            {
                input_file.close() ;
                close(client_fd) ;
                return 1 ;
            }

            total_sent += bytes_read ;
        }

        if (input_file.eof())
            break ;

        if (!input_file)
        {
            cout << "File reading failed!" << endl ;
            input_file.close() ;
            close(client_fd) ;
            return 1 ;
        }
    }

    //Closing file :
    input_file.close() ;

    //Indicating completion of sending :
    shutdown(client_fd, SHUT_WR) ;

    cout << "Total bytes sent: " << total_sent << endl ;
    cout << "File sent successfully!" << endl ;

    //Closing client socket :
    close(client_fd) ;

    return 0 ;
}
