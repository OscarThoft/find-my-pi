#include <poll.h> // poll() defined here
#include <linux/gpio.h> // GPIO character device API
#include <sys/ioctl.h> // ioctl() defined here
#include <string.h> // memset() and strlen() defined here
#include <fcntl.h> // open() and O_RDONLY defined here
#include <unistd.h> // close() defined here
#include <iostream> 

// Define GPIO lines
#define BTN_A 27
#define BTN_B 22
#define BTN_UP 12
#define BTN_DOWN 16
#define BTN_LEFT 5
#define BTN_RIGHT 6

#define POLL_TIMEOUT_MS 0

int main(int argc, char* argv[]) {
  // Open gpiochip0 as read-only device
  int fd = open("/dev/gpiochip0", O_RDONLY);
  if (fd < 0) { perror("open()"); return EXIT_FAILURE; }

  // Array of structs holding file descriptors to poll
  struct pollfd fds[6];
  // Array of structs to request event-based GPIO lines
  struct gpioevent_request req[6];
  // Array of structs to store GPIO event data
  struct gpioevent_data data[6];

  // Populate gpioevent_request struct with 0s to be safe
  for (int i = 0; i < 6; ++i) {
    memset(&req[i], 0, sizeof(req[i]));
  }

  // Specify GPIO lines we want to access
  req[0].lineoffset = BTN_A;

  
// til 1 kanp A
    // Set GPIO lines to report both rising and falling edges
    req[0].eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    // Set GPIO lines as inputs
    req[0].handleflags = GPIOHANDLE_REQUEST_INPUT;
    // Set a name to be shown when gpioinfo is called
    strcpy(req[0].consumer_label, "Userspace GPIO event monitor");

    int ret_val = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req[0]);
    if (ret_val < 0) {
      perror("ioctl()");
      close(fd);
      return EXIT_FAILURE;
    }
    

    fds[0].fd = req[0].fd; // Copy file descriptor to fd in struct pollfd
    fds[0].events = POLLIN; // Event to monitor: data available to read
    fds[0].revents = 0; // Reset revents before monitoring GPIO lines

    std::cout << "Monitoring button A for Rising edge...\n";

// bruges til at sætte found 
    bool found = false;

  // Read events on all GPIO lines infinitely
  while (true) {
      // Use poll() to monitor all file descriptors
      // int ret_val = poll(fds, 6, POLL_TIMEOUT_MS); // til 6 knapper
      int ret_val = poll(fds, 1, -1);
      // std::cout << "Waiting...\n";
      if (ret_val < 0) { // Handle error
        perror("poll()");
        { close(req[0].fd); }
        close(fd);
        return EXIT_FAILURE;
      }

  
      if (fds[0].revents & POLLIN) 
      { // Check if data is available
        ssize_t bytes = read(fds[0].fd, &data[0], sizeof(data[0]));
        if (bytes != sizeof(data[0])) { perror("read"); } // Handle error
        else if (data[0].id == GPIOEVENT_EVENT_FALLING_EDGE) 
            { // Rising edge
              found = true;
                std::cout << "Fundet butten pressed\n";
                break;
               // sætter et SW flag istedet 
               // almindelig string buffer, der skriver hvad der er sket. 
            } 
        // ikke nødvendigt fordi vi bruger break
        // usleep(50000); // 50 ms sleep to handle debounce (accidental press)
        // kig begge dele hvis man vil være sikker
        // kun kigge efter level 
       }// End of if (fds[i].revents & POLLIN)

  } // End of while loop


  close(req[0].fd);
  
  
  close(fd);
  return EXIT_SUCCESS;
}
