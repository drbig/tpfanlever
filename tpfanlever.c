/** @file
 * small/tpfanlever/tpfanlever.c - Simple throttle for ThinkPad fan.
 *
 * @author Piotr S. Staszewski
 * @date 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Configurable defines

#define SENSOR    "/sys/class/thermal/thermal_zone0/temp"
#define FAN       "/proc/acpi/ibm/fan"
#define LEVEL_LOW "level 1"
#define LEVEL_HI  "level auto"
#define THRESHOLD 47000
#define GRACE     16
#define DELAY     2

// Internal defines

#ifdef DEBUG
  #define dbg(code) code;
#else
  #define dbg(code)
#endif

// Internal variables

static int fdSensor, fdFan;

// Internal routines

static void sig_handler(int signum) {
  close(fdFan);
  close(fdSensor);
  exit(0);
}

static void die(const char *msg) {
  if (errno) {
    perror(msg);
    exit(2);
  } else {
    fprintf(stderr, "ABORT: %s\n", msg);
    exit(1);
  }
}

// Main routine

int main(int argc, char *argv[]) {
  char buffer[16];
  int temp, mode, grace;

  mode = -1;
  grace = 0;

  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  if ((fdSensor = open(SENSOR, O_RDONLY)) < 0)
    die("Can't open sensor file for reading");

  if ((fdFan = open(FAN, O_WRONLY)) < 0)
    die("Can't open fan file for writing");

  while (1) {
    if (read(fdSensor, &buffer, sizeof(buffer)) > 0) {
      dbg(printf("%s", buffer));

      temp = atoi(buffer);
      dbg(printf("%d : %s\n", temp, temp > THRESHOLD ? LEVEL_HI : LEVEL_LOW));

      if (temp > THRESHOLD) {
        if (mode != 2) {
          dbg(printf("CHANGE TO %s\n", LEVEL_HI));
          write(fdFan, LEVEL_HI, sizeof(LEVEL_HI));
          mode = 2;
        }
        grace = 0;
      } else {
        if ((mode != 1) && (grace++ >= GRACE)) {
          dbg(printf("CHANGE TO %s\n", LEVEL_LOW));
          write(fdFan, LEVEL_LOW, sizeof(LEVEL_LOW));
          mode = 1;
          grace = 0;
        }
      }
    }

    sleep(DELAY);
    lseek(fdSensor, 0, SEEK_SET);
  }
}

// vim: ts=2 et sw=2 sts=2
