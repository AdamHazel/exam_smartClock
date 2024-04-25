#include "DFRobot_RGBLCD1602.h"
#include "classes.h"
#include "mbed.h"
#include <cstdio>
#include <string>

#define JSON_NOEXCEPTION
#include "ipgeolocation.h"
#include "json.hpp"


#define WAIT_TIME_MS (10000ms)
#define HTTP_RESPONSE_BUFFER_SIZE (4000)

using json = nlohmann::json;

// Declare local function(s)
static const char *get_nsapi_error_string(nsapi_error_t err);

void startUp(DFRobot_RGBLCD1602 &lcd) {

  time_t time;  
  std::string longit;
  std::string latit;
  std::string city;

  NetworkInterface *network = nullptr;

  do {
    printf("Get pointer to default network interface ...\n");
    network = NetworkInterface::get_default_instance();

    if (!network) {
      printf("Failed to get default network interface\n");
    }

    ThisThread::sleep_for(1000ms);
  } while (network == nullptr);

  nsapi_size_or_error_t result;

  do {
    printf("Connecting to the network...\n");
    result = network->connect();

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to network: %d %s \n", result,
             get_nsapi_error_string(result));
    }
  } while (result != NSAPI_ERROR_OK);

  SocketAddress address;

  do {
    printf("Get local IP address...\n");
    result = network->get_ip_address(&address);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to get local IP address: %d %s\n", result,
             get_nsapi_error_string(result));
    }
  } while (result != NSAPI_ERROR_OK);

  printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

  while (true) {

    printf("Remaining stack space = %u bytes\n",
           osThreadGetStackSpace(ThisThread::get_id()));
    // ThisThread::sleep_for(WAIT_TIME_MS);

    // TLSSocket is used for HTTPS (HTTP secured with TLS/SSL).
    // This TLS socket is allocated on stack and takes approx 1500 bytes of
    // stack memory. So make sure you have enough stack size
    TLSSocket socket;
    // Alternatively you might allocate from heap:
    // TLSSocket *socket = new TLSSocket;
    // but then you MUST remember to free up memory when then local variable
    // holding the pointer to the allocated socket object goes out of scope:
    // delete socket;
    // Otherwise you have created a memory leak

    // Configure timeout on socket receive
    // (returns NSAPI_ERROR_WOULD_BLOCK on timeout)
    socket.set_timeout(1000);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %d %s\n", result,
             get_nsapi_error_string(result));
      continue;
    }

    const char host[] = "api.ipgeolocation.io";

    // Get IP address of host (web server) by name
    result = network->gethostbyname(host, &address);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %d %s\n", host, result,
             get_nsapi_error_string(result));
      continue;
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    // Set server TCP port number, 443 for HTTPS
    address.set_port(443);

    // Set the root certificate of the web site.
    // See include/ipify_org_ca_root_certificate.h for how to
    // download the certificate and add it to source code
    result = socket.set_root_ca_cert(ipgeolocationcert);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate of the web site: %d %s\n", result,
             get_nsapi_error_string(result));
      continue;
    }

    // ### IMPORTANT ###
    // Often a server has several virtual hosts to serve, and
    // it is therefore necessary to tell which host to connect to,
    // in order to let the TLS handshake succeed
    socket.set_hostname(host);

    // Connect to server at the given address
    result = socket.connect(address);

    // Check result
    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to server at %s: %d %s\n", host, result,
             get_nsapi_error_string(result));
      continue;
    }

    printf("Successfully connected to server %s\n", host);

    // Create HTTP request
    const char http_request[] =
        "GET /timezone?REDACTED HTTP/1.1\r\n"
        "Host: api.ipgeolocation.io\r\n"
        "Connection: close\r\n"
        "\r\n";

    // The request might not be fully sent in one go,
    // so keep track of how much we have sent
    nsapi_size_t bytes_to_send = strlen(http_request);
    nsapi_size_or_error_t sent_bytes = 0;

    printf("\nSending message: \n%s\n", http_request);

    // Loop as long as there are more data to send
    while (bytes_to_send) {
      // Try to send the remaining data.
      // send() returns how many bytes were actually sent
      sent_bytes = socket.send(http_request + sent_bytes, bytes_to_send);

      if (sent_bytes < 0) {
        // Negative return values from send() are errors
        break;
      } else {
        printf("Sent %d bytes\n", sent_bytes);
      }

      bytes_to_send -= sent_bytes;
    }

    if (sent_bytes < 0) {
      printf("Failed to send HTTP request: %d %s\n", sent_bytes,
             get_nsapi_error_string(sent_bytes));
      continue;
    }

    printf("Complete message sent\n");

    // The respone needs to be stored in memory. The memory object is called a
    // buffer. If you make this buffer static it will be placed in bss (memory
    // for global and static variables) and won't use the main thread stack
    // memory
    static char http_response[HTTP_RESPONSE_BUFFER_SIZE];

    // Nullify response buffer
    memset(http_response, 0, sizeof(http_response));

    nsapi_size_t remaining_bytes = HTTP_RESPONSE_BUFFER_SIZE;
    nsapi_size_or_error_t received_bytes = 0;

    // Loop as long as there are more data to read,
    // we might not read all in one call to recv()
    while (remaining_bytes > 0) {
      nsapi_size_or_error_t result =
          socket.recv(http_response + received_bytes, remaining_bytes);

      // Negative return values from recv() are errors
      if (result < 0) {
        received_bytes = result;
        break;
      } else {
        printf("Received %d bytes\n", result);
      }

      // If the result is 0 there are no more bytes to read
      if (result == 0) {
        break;
      }

      received_bytes += result;
      remaining_bytes -= result;
    }

    if (received_bytes < 0) {
      printf("Failed to receive HTTP respons: %d %s\n", received_bytes,
             get_nsapi_error_string(received_bytes));
      continue;
    }

    printf("\nReceived %d bytes with HTTP status code: %.*s\n", received_bytes,
           strstr(http_response, "\n") - http_response, http_response);


    // Find the start and end of the JSON data.
    // If the JSON response is an array you need to replace this with [ and ]
    char *json_begin =
        strchr(http_response, '{');               // Search first occurence of {
    char *json_end = strrchr(http_response, '}'); // Search last occurence of }

    // Check if we actually got JSON in the response
    if (json_begin == nullptr || json_end == nullptr) {
      printf("Failed to find JSON in response\n");
      continue;
    }

    // End the string after the end of the JSON data in case the response
    // contains trailing data
    json_end[1] = 0;

    // Parse response as JSON, starting from the first {
    json document = json::parse(json_begin);

    if (document.is_discarded()) {
      printf("The input is invalid JSON\n");
      continue;
    }

    // Get info from API
    //printf("If you came this far, then yay!\n");
    time = document["date_time_unix"].get<float>() + (document["timezone_offset_with_dst"].get<float>() * 3600);
    set_time(time);
    latit = document["geo"]["latitude"];
    longit = document["geo"]["longitude"];
    city = document["geo"]["city"];
    break;
  }

/*
  printf("Time as a basic string %s\n", ctime(&time));
  printf("Longitude is  %s\n", longit.c_str());
  printf("Latitude is %s\n", latit.c_str());
  printf("City is %s\n", city.c_str());
  */


  printf("Start up screens should be showing...\n");
  printf("Start screen 1\n");

  
  // Start up screen one
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Unix epoch time:");
  lcd.setCursor(0,1);
  lcd.printf("%.0f", (float) time);
  ThisThread::sleep_for(2s);

  printf("Start screen 2\n");
  // Start up screen two
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Lat: %s", latit.c_str());
  lcd.setCursor(0, 1);
  lcd.printf("Lon: %s", longit.c_str());
  ThisThread::sleep_for(2s);

  printf("Start screen 3\n");
  // Start up screen three
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("City:");
  lcd.setCursor(0, 1);
  lcd.printf("%s", city.c_str());
  ThisThread::sleep_for(2s);

  lcd.clear();
};

static const char *get_nsapi_error_string(nsapi_error_t err) {
  switch (err) {
  case NSAPI_ERROR_OK:
    return "NSAPI_ERROR_OK";
  case NSAPI_ERROR_WOULD_BLOCK:
    return "NSAPI_ERROR_WOULD_BLOCK";
  case NSAPI_ERROR_UNSUPPORTED:
    return "NSAPI_ERROR_UNSUPPORTED";
  case NSAPI_ERROR_PARAMETER:
    return "NSAPI_ERROR_PARAMETER";
  case NSAPI_ERROR_NO_CONNECTION:
    return "NSAPI_ERROR_NO_CONNECTION";
  case NSAPI_ERROR_NO_SOCKET:
    return "NSAPI_ERROR_NO_SOCKET";
  case NSAPI_ERROR_NO_ADDRESS:
    return "NSAPI_ERROR_NO_ADDRESS";
  case NSAPI_ERROR_NO_MEMORY:
    return "NSAPI_ERROR_NO_MEMORY";
  case NSAPI_ERROR_NO_SSID:
    return "NSAPI_ERROR_NO_SSID";
  case NSAPI_ERROR_DNS_FAILURE:
    return "NSAPI_ERROR_DNS_FAILURE";
  case NSAPI_ERROR_DHCP_FAILURE:
    return "NSAPI_ERROR_DHCP_FAILURE";
  case NSAPI_ERROR_AUTH_FAILURE:
    return "NSAPI_ERROR_AUTH_FAILURE";
  case NSAPI_ERROR_DEVICE_ERROR:
    return "NSAPI_ERROR_DEVICE_ERROR";
  case NSAPI_ERROR_IN_PROGRESS:
    return "NSAPI_ERROR_IN_PROGRESS";
  case NSAPI_ERROR_ALREADY:
    return "NSAPI_ERROR_ALREADY";
  case NSAPI_ERROR_IS_CONNECTED:
    return "NSAPI_ERROR_IS_CONNECTED";
  case NSAPI_ERROR_CONNECTION_LOST:
    return "NSAPI_ERROR_CONNECTION_LOST";
  case NSAPI_ERROR_CONNECTION_TIMEOUT:
    return "NSAPI_ERROR_CONNECTION_TIMEOUT";
  case NSAPI_ERROR_ADDRESS_IN_USE:
    return "NSAPI_ERROR_ADDRESS_IN_USE";
  case NSAPI_ERROR_TIMEOUT:
    return "NSAPI_ERROR_TIMEOUT";
  case NSAPI_ERROR_BUSY:
    return "NSAPI_ERROR_BUSY";
  default:
    return "NSAPI_ERROR_UNKNOWN";
  }
}