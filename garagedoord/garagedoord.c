//
// Purpose: Daemon for initializing/montoring GPIO for Garagedoor
// File:    garagedoord.c
// Author:  smanders
// Date:    Feb 6, 2013
// License: GPLv2
//

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <bcm2835.h>

//============ User changable Parameters ===================
#define PIN_RELAY              23
#define PIN_STATUS             9
#define DOOR_OPEN_NOTIF_TIME   (10 * 60)  // seconds

#define DEBUG_ENABLED

//============ Internal Parameters ===================
#define LOOP_SLEEP_TIME    5 // seconds
#define TRUE  1
#define FALSE 0
#define PIDFILE  "/var/run/garagedoord.pid"

// Notifications
#define NOTIF_ENABLE_EMAIL  FALSE
#define NOTIF_ENABLE_PROWL  FALSE

#define PROWL_SERVER        "api.prowlapp.com"
#define PROWL_URL           "publicapi/add"
#define PROWL_API_KEY       "YourProwlApiKeyHere"

typedef unsigned char bool;

// Daemon Parameters
#define DAEMON_NAME "garagedoord"

void daemonShutdown();
void signal_handler(int sig);
void daemonize(char *rundir, char *pidfile);

int pidFilehandle;

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            syslog(LOG_WARNING, "Received SIGHUP signal.");
            break;
        case SIGINT:
        case SIGTERM:
            syslog(LOG_INFO, "Daemon exiting");
            daemonShutdown();
            exit(EXIT_SUCCESS);
            break;
        default:
            syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sig));
            break;
    }
}

void daemonShutdown()
{
    close(pidFilehandle);
}

void daemonize(char *rundir, char *pidfile)
{
    int pid, sid, i;
    char str[10];
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Check if parent process id is set */
    if (getppid() == 1) {
        /* PPID exists, therefore we are already a daemon */
        return;
    }

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */

    /* Fork*/
    pid = fork();

    if (pid < 0) {
        /* Could not fork */
        syslog(LOG_INFO, "Could not fork() successfully");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        /* Child created ok, so exit parent process */
        syslog(LOG_DEBUG,"Child process created: %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    /* Child continues */

    umask(027); /* Set file permissions 750 */

    /* Get a new process group */
    sid = setsid();

    if (sid < 0) {
        syslog(LOG_INFO, "Could not setsid() successfully");
        exit(EXIT_FAILURE);
    }

    /* close all descriptors */
    for (i = getdtablesize(); i >= 0; --i) {
        close(i);
    }

    /* Route I/O connections */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    chdir(rundir); /* change running directory */

    /* Ensure only one copy */
    pidFilehandle = open(pidfile, O_RDWR|O_CREAT, 0600);

    if (pidFilehandle == -1 ) {
        /* Couldn't open lock file */
        syslog(LOG_INFO, "Could not open PID lock file %s, exiting", pidfile);
        exit(EXIT_FAILURE);
    }

    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
        /* Couldn't get lock on lock file */
        syslog(LOG_INFO, "Could not lock PID lock file %s, exiting", pidfile);
        exit(EXIT_FAILURE);
    }

    /* Get and format PID */
    sprintf(str,"%d\n",getpid());

    /* write pid to lockfile */
    write(pidFilehandle, str, strlen(str));
}

void logging_init()
{
    /* Debug logging */
#ifdef DEBUG_ENABLED
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(DAEMON_NAME, LOG_CONS, LOG_USER);
#else /* DEBUG_ENABLED */
    /* Logging */
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
#endif /* DEBUG_ENABLED */

}

int gpio_pin_setup(char pin_relay, char pin_status)
{
    char  cmd[512];
    char  buf[32];
    char  filename[80];
    FILE *fp_direction;

    // Make system calls to expose gpio fs to webserver
    sprintf(filename, "/sys/class/gpio/gpio%d/value",pin_status);
    if (fopen(filename, "r") == NULL) {
        syslog(LOG_DEBUG,"Exporting GPIO%d for webserver access\n", pin_status);
        sprintf(cmd,"/bin/echo %d > /sys/class/gpio/export",pin_status);
        system(cmd);
    } else {
        syslog(LOG_DEBUG,"GPIO%d already exported for webserver access\n", pin_status);
    }

    // Make system calls to expose gpio fs to webserver
    sprintf(filename, "/sys/class/gpio/gpio%d/value",pin_relay);
    if (fopen(filename, "r") == NULL) {
        syslog(LOG_DEBUG,"Exporting GPIO%d for webserver access\n", pin_relay);
        sprintf(cmd,"/bin/echo %d > /sys/class/gpio/export", pin_relay);
        system(cmd);
    } else {
        syslog(LOG_DEBUG,"GPIO%d already exported for webserver access\n", pin_relay);
    }

    sprintf(filename, "/sys/class/gpio/gpio%d/direction",pin_relay);
    fp_direction = fopen(filename,"r");
    if (!fp_direction) {
        syslog(LOG_DEBUG,"Unable to open \"%s\"\n", filename);
        return -1;
    }

    fscanf(fp_direction,"%s",buf);
    fclose(fp_direction);

    if (strcmp(buf,"out")) {
        syslog(LOG_DEBUG,"Setting Direction to OUT on GPIO%d\n", pin_relay);
        //bcm2835_gpio_fsel(pin_relay, BCM2835_GPIO_FSEL_OUTP);
        //bcm2835_gpio_write(pin_relay, HIGH);

        sprintf(cmd,"/bin/echo out > /sys/class/gpio/gpio%d/direction",pin_relay);
        system(cmd);
    } else {
        syslog(LOG_DEBUG,"Direction already set to OUT on GPIO%d\n", pin_relay);
    }

    sprintf(cmd,"/bin/echo 1 > /sys/class/gpio/gpio%d/value",pin_relay);
    system(cmd);

    // Set permissions so webserver can update GPIO out
    sprintf(cmd,"/bin/chown www-data:www-data /sys/class/gpio/gpio%d/value",pin_relay);
    system(cmd);

}

int send_prowl_notif(char *msg)
{
    char  cmd[512];

    sprintf(cmd,"/usr/bin/curl https://%s/%s -F apikey=%s -F application=\"garagedoord\" "
                " -F event=\"%s\"",
                PROWL_SERVER, PROWL_URL, PROWL_API_KEY, msg);
    system(cmd);
}



int main(int argc, char **argv)
{
    bool   open = FALSE;
    bool   prev_open = FALSE;
    bool   timer_running = FALSE;
    struct timespec exp_time;
    struct timespec cur_time;
    char   msg[128];

    logging_init();

    syslog(LOG_INFO, "Process starting");

    /* Deamonize */
    daemonize("/tmp/", PIDFILE);

    syslog(LOG_INFO, "Daemon running");

    // If you call this, it will not actually access the GPIO
    // Use for testing
    //bcm2835_set_debug(1);

    if (!bcm2835_init()) {
        syslog(LOG_INFO, "Could not call bcm2835_init() successfully");
	exit(EXIT_FAILURE);
    }

    if (!gpio_pin_setup(PIN_RELAY, PIN_STATUS)) {
        syslog(LOG_INFO, "Could not call gpio_pin_setup() successfully");
	exit(EXIT_FAILURE);
    }

    while (1)
    {
        open = (bcm2835_gpio_lev(PIN_STATUS) == LOW ? TRUE : FALSE);

        //syslog(LOG_DEBUG,"Current State: open: %d, prev_open: %d, timer_running: %d\n",open,prev_open,timer_running);

        if  ((timer_running == FALSE) && 
             (open          == TRUE) && 
             (prev_open     == FALSE)) {

            syslog(LOG_INFO,"Garagedoor opened! Starting Timer\n");

            // Create timestamp
            clock_gettime(CLOCK_MONOTONIC, &exp_time);

            exp_time.tv_sec += DOOR_OPEN_NOTIF_TIME;

            // Then start timer
            timer_running = TRUE;

        } else if (timer_running == TRUE) {

            clock_gettime(CLOCK_MONOTONIC, &cur_time);

            if (open == FALSE) {
                syslog(LOG_INFO,"Garagedoor has been closed. Stopping timer. Open for %d minutes\n",
                      (exp_time.tv_sec - cur_time.tv_sec) / 60);
                // Cancel timer
                timer_running = FALSE;
            
            } else if (cur_time.tv_sec > exp_time.tv_sec) {
                snprintf(msg,sizeof(msg),"Garagedoor open for longer than %d minutes\n", DOOR_OPEN_NOTIF_TIME / 60);
                syslog(LOG_INFO,msg);
                // Send Notification
                if (NOTIF_ENABLE_PROWL) {
                    send_prowl_notif(msg);
                }

                // Cancel Timer
                timer_running = FALSE;
            }
        }  else if  ((open      == FALSE) && 
                     (prev_open == TRUE)) {

                clock_gettime(CLOCK_MONOTONIC, &cur_time);

                snprintf(msg,sizeof(msg),"Garagedoor closed after timer expired. Open for %d minutes\n", 
                (cur_time.tv_sec - exp_time.tv_sec + DOOR_OPEN_NOTIF_TIME) / 60);
                syslog(LOG_INFO,msg);
                // Send Notification
                if (NOTIF_ENABLE_PROWL) {
                    send_prowl_notif(msg);
                }
        }

        // Store Previous State
        prev_open = open;

        // Sleep for 10 seconds
	bcm2835_delay(LOOP_SLEEP_TIME*1000);
    }

    return 0;
}
