#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"
#include <cstdio>
#include <functional>


void lcd_initialise(DFRobot_RGBLCD1602 &a) 
{
    a.init();
    a.display();
    a.clear();
    printf("LCD initialised\n");
}

void screenCheck(bool &screenChn, DFRobot_RGBLCD1602 &a, int &screenN)
{
    if(screenChn == true) {
        a.clear();
        printf("Screen number is %i\n",screenN);
        screenChn = false;
    }
}

/*
getInformation_https:
Function to get JSON data from a server
Arguments:
    - int for size of HTTPS request
    - const char* (string) for the host e.g. "api.ipgeolocation.io"
    - const char* for HTTPS certificate used in the TLSSocket
    - const char* for requested resource
    - Note: const char strings can be placed in certificates_hosts.h, and then include certificates_host in .cpp file

Returns: 
    - returns a pointer to a string of JSON data
*/ 

char* getInformation_https(int BUFFER_SIZE_REQUEST, const char* hostChoice, const char* certificate, const char* resourceWanted)
{
    static constexpr int BUFFER_SIZE_RESPONSE = 4000;
   
    NetworkInterface *network = nullptr;

    do {
        printf("\nStep 1: Getting pointer to default network interface...\n");
        network = NetworkInterface::get_default_instance();

        if (!network) {
            printf("Failed to complete step 1. Will try again.\n");
        }

        ThisThread::sleep_for(1000ms);
    } while (network == nullptr);

    // Variable for receiving the results of code performed
    nsapi_size_or_error_t result;

    do {
        printf("Step 2: Connecting to the network...\n");
        result = network->connect();

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to copmplete step 2. Error code: %d\n", result);
        }

    } while (result != NSAPI_ERROR_OK);

    printf("Connected to network\n");

    SocketAddress address;

    do {
        printf("Step 3: Get local IP address...\n");
        result = network->get_ip_address(&address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to copmplete step 3. Error code: %d\n", result);
        }
    } while (result != NSAPI_ERROR_OK);

    printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

    while (true)
    {
        // Socket used for HTTPS
        TLSSocket *socket = new TLSSocket;
        socket->set_timeout(1000);

        printf("Step 4: Open TLSSocket\n");
        result = socket->open(network);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 4. Error code: %d\n", result);
            delete socket;
            continue;
        }

        // Using hostChoice variable
        printf("Step 5: Get IP address of host (web server)\n");
        result = network->gethostbyname(hostChoice, &address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 5. Error code: %d\n", result);
            delete socket;
            continue;
        }

        printf("IP address of server %s is %s\n", hostChoice, address.get_ip_address());

        // Set server TCP port number
        address.set_port(443);

        printf("Step 6: Set certificate\n");
        result = socket->set_root_ca_cert(certificate);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 6. Error code: %d\n", result);
            delete socket;
            continue;
        }

        socket->set_hostname(hostChoice);

        printf("Step 7: Connect to server at given address \n");
        result = socket->connect(address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 7. Error code: %d\n", result);
            delete socket;
            continue;
        }

        printf("Step 8: Create and send HTTPS request\n");
        char https_request[BUFFER_SIZE_REQUEST];
        snprintf(https_request, BUFFER_SIZE_REQUEST,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n" 
        , resourceWanted, hostChoice);

        nsapi_size_t bytes_to_send = strlen(https_request);
        nsapi_size_or_error_t sent_bytes = 0;

        // Loop to send HTTPS request
        while (bytes_to_send) {
            sent_bytes = socket->send(https_request, bytes_to_send);

            // Error check
            if (sent_bytes < 0) {
                break;
            } else {
                printf("Send %d bytes \n", sent_bytes);
            }

            bytes_to_send -= sent_bytes;
        }

        if (sent_bytes < 0)
        {
            printf("Failed to send HTTPS request. Error code: %d\n", sent_bytes);
            delete socket;
            continue;
        }

        printf("Step 8 completed. \n");

        printf("Step 9: Receive repsonse from server\n");

        static char http_response[BUFFER_SIZE_RESPONSE];
        memset(http_response, 0, sizeof(http_response));

        nsapi_size_t remaining_bytes = BUFFER_SIZE_RESPONSE;
        nsapi_size_or_error_t received_bytes = 0;

        // Loop as long as there is data to receive
        while (remaining_bytes > 0)
        {
            nsapi_size_or_error_t result = socket->recv(http_response + received_bytes, remaining_bytes);

            // Error check
            if (result < 0)
            {
                received_bytes = result;
                break;
            } else {
                printf("Received %d bytes\n", result);
            }


            if (result == 0)
                break;

            received_bytes += result;
            remaining_bytes -= result;
        }

        if (received_bytes < 0) {
            printf("Failed to receive HTTPS response. Error code: %d\n", received_bytes);
            delete socket;
            continue;
        }

        printf("\nReceived %d bytes with HTTPS status code: %.*s\n", received_bytes,
        strstr(http_response, "\n") - http_response, http_response);

        //printf("\n\nWhole response:\n%s",http_response);

        printf("Step 10: Getting JSON information\n");

        char *json_begin = strchr(http_response, '{');
        char *json_end = strrchr(http_response, '}');

        // Check if we actually got JSON in the response
        if (json_begin == nullptr || json_end == nullptr) {
            printf("Failed to find JSON in response\n");
            delete socket;
            continue;
        }
        else {
            delete socket;
            nsapi_size_or_error_t result = -1;
            while (result != NSAPI_ERROR_OK) {
                result = network->disconnect();
                network = nullptr;
            }
            json_end[1] = 0;
            return json_begin;
        }        
    }  
}

/*
getInformation_http:
Function to get JSON data from a server
Arguments:
    - int for size of HTTPS request
    - const char* (string) for the host e.g. "api.ipgeolocation.io"
    - const char* for requested resource
    - Note: const char strings can be placed in certificates_hosts.h, and then include certificates_host in .cpp file

Returns: 
    - returns a pointer to a string of JSON data
*/ 

char* getInformation_http(int BUFFER_SIZE_REQUEST,  const char* hostChoice, char* resourceWanted)
{
    static constexpr int BUFFER_SIZE_RESPONSE = 4000;
   
    NetworkInterface *network = nullptr;

    do {
        printf("Step 1: Getting pointer to default network interface...\n");
        network = NetworkInterface::get_default_instance();

        if (!network) {
            printf("Failed to complete step 1. Will try again.\n");
        }

        ThisThread::sleep_for(1000ms);
    } while (network == nullptr);

    // Variable for receiving the results of code performed
    nsapi_size_or_error_t result;

    do {
        printf("Step 2: Connecting to the network...\n");
        result = network->connect();

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to copmplete step 2. Error code: %d\n", result);
        }

    } while (result != NSAPI_ERROR_OK);

    printf("Connected to network\n");

    SocketAddress address;

    do {
        printf("Step 3: Get local IP address...\n");
        result = network->get_ip_address(&address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to copmplete step 3. Error code: %d\n", result);
        }
    } while (result != NSAPI_ERROR_OK);

    printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());


    while (true)
    {
        // Socket used for HTTPS
        TCPSocket *socket = new TCPSocket;
        socket->set_timeout(1000);

        printf("Step 4: Open TCPSocket\n");
        result = socket->open(network);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 4. Error code: %d\n", result);
            delete socket;
            continue;
        }

        // Using hostChoice variable
        printf("Step 5: Get IP address of host (web server)\n");
        result = network->gethostbyname(hostChoice, &address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 5. Error code: %d\n", result);
            delete socket;
            continue;
        }

        printf("IP address of server %s is %s\n", hostChoice, address.get_ip_address());

        // Set server TCP port number
        address.set_port(80);

        printf("Step 6: Connect to server at given address \n");
        result = socket->connect(address);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to complete step 6. Error code: %d\n", result);
            delete socket;
            continue;
        }

        printf("Step 7: Create and send HTTPS request\n");
        char http_request[BUFFER_SIZE_REQUEST];
        snprintf(http_request, BUFFER_SIZE_REQUEST,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n" 
        , resourceWanted, hostChoice);

        nsapi_size_t bytes_to_send = strlen(http_request);
        nsapi_size_or_error_t sent_bytes = 0;

        // Loop to send HTTPS request
        while (bytes_to_send) {
            sent_bytes = socket->send(http_request, bytes_to_send);

            // Error check
            if (sent_bytes < 0) {
                break;
            } else {
                printf("Send %d bytes \n", sent_bytes);
            }

            bytes_to_send -= sent_bytes;
        }

        if (sent_bytes < 0)
        {
            printf("Failed to send HTTP request. Error code: %d\n", sent_bytes);
            delete socket;
            continue;
        }

        printf("Step 7 completed. \n");

        printf("Step 8: Receive repsonse from server\n");

        static char http_response[BUFFER_SIZE_RESPONSE];
        memset(http_response, 0, sizeof(http_response));

        nsapi_size_t remaining_bytes = BUFFER_SIZE_RESPONSE;
        nsapi_size_or_error_t received_bytes = 0;

        // Loop as long as there is data to receive
        while (remaining_bytes > 0)
        {
            nsapi_size_or_error_t result = socket->recv(http_response + received_bytes, remaining_bytes);

            // Error check
            if (result < 0)
            {
                received_bytes = result;
                break;
            } else {
                printf("Received %d bytes\n", result);
            }


            if (result == 0)
                break;

            received_bytes += result;
            remaining_bytes -= result;
        }

        if (received_bytes < 0) {
            printf("Failed to receive HTTP response. Error code: %d\n", received_bytes);
            delete socket;
            continue;
        }

        printf("\nReceived %d bytes with HTTP status code: %.*s\n", received_bytes,
        strstr(http_response, "\n") - http_response, http_response);

        //printf("\n\nWhole response:\n%s",http_response);

        printf("Step 10: Getting JSON information\n");

        char *json_begin = strchr(http_response, '{');
        char *json_end = strrchr(http_response, '}');

        // Check if we actually got JSON in the response
        if (json_begin == nullptr || json_end == nullptr) {
            printf("Failed to find JSON in response\n");
            delete socket;
            continue;
        }
        else {
            delete socket;
            nsapi_size_or_error_t result = -1;
            while (result != NSAPI_ERROR_OK) {
                result = network->disconnect();
                network = nullptr;
            }
            json_end[1] = 0;
            return json_begin;
        }        
    }
}