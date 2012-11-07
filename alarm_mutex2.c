/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */
#include <pthread.h>
#include <time.h>
#include "errors.h"

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
typedef struct alarm_tag {
    struct alarm_tag    *link;
    int                 seconds;
    time_t              time;   /* seconds from EPOCH */
    char                message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list = NULL;

void* alarm_thread(void *arg)
{
    alarm_t* alarm;
    int sleep_time;
    time_t now;
    int status;

    while (1)
    {
        status = pthread_mutex_lock(&alarm_mutex);
        if (status != 0)
            err_abort(status, "Lock mutex");
        alarm = alarm_list;

        /*
        * if the alarm list is empty, wait for one second. this allows the main thread to run, and read another command.
        * if the list is not empty, remove the first item. compute the number of seconds to wait, if the result is less than 0
        * (the time has passed), then set the sleep_time to 0.
        */
        if (alarm == NULL)
            sleet_time = 1;
        else
        {
            alarm_list = alarm->link;  /* remove the first item */
            now = time (NULL);
            if (alarm->time <= now)
                sleep_time = 0;
            else
                sleep_time = now - alarm->time;
        }

         /*
         * unlock the mutex before waiting, so that the main thread can lock it to insert a new alarm request. if the sleep_time
         * is 0, then call sched_yield, giving the main thread a chance to run if it has been readied by user input, without delaying
         * the message if there's no input.
         */
         status = pthread_mutex_unlock(&alarm_mutex);
         if (status != 0)
            err_abort(status, "unlock mutex");

         if (sleep_time > 0)
            sleep(sleep_time);
         else
            sched_yield();

        /* if a timer expired, print the message and free the structure. */
        if (alarm != NULL)
        {
            printf();
            free(alarm);
        }
        
    }
}


int main(int argc, char** argv)
{
    int status;
    char line[128];
    alarm_t* alarm, **last, *next;
    pthread_t thread;

    status = pthread_create(&thread, NULL, alarm_thread, NULL);
    if (status != 0)
        err_abort(status, "failed to create thread");

    while (1)
    {
        printf("alarm> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        if (strlen(line) <= 1)
            continue;

        alarm = (alarm_t*)malloc(sizeof(alarm_t));
        if (alarm == NULL)
            errno_abort("allocate alarm");

        /*
         * parse input line into seconds (%d) and a message (%64[^\n]), consisting of up to 64 characters separated from the
         * seconds by whitespace.
         */
         if (sscanf(line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2)
        {
            fprintf(stderr, "bad command\n");
            free(alarm);
        }
        else
        {
            status = pthread_mutex_lock(&alarm_mutex);
            if (status != 0)
                err_abort(status, "failed to lock mutex");

            alarm->time = time(NULL) + alarm->seconds;

            /* insert the new alram into the list of alarms, sorted by expiration tiem */
            last = & alarm_list;
            next = *last;
            while (next != NULL)
            {
                if (next->time >= alarm->time)
                {
                    alarm->link = next;
                    *last = alarm;
                    break;
                }
                last = &next->link;
                next = next->link;
            }

            /* if we reached the end of list, insert the new alarm there */
            if (next == NULL)
            {
                *last = alarm;
                alarm->link = NULL;
            }

            pthread_mutex_unlock(&alarm_mutex);
        }
        
    }
}

int main (int argc, char *argv[])
{
    while (1) {
        if (sscanf (line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message) < 2) {
        } else {
            status = pthread_mutex_lock (&alarm_mutex);

          /*
             * If we reached the end of the list, insert the new
             * alarm there. ("next" is NULL, and "last" points
             * to the link field of the last item, or to the
             * list header).
             */
            if (next == NULL) {
                *last = alarm;
                alarm->link = NULL;
            }
#ifdef DEBUG
            printf ("[list: ");
            for (next = alarm_list; next != NULL; next = next->link)
                printf ("%d(%d)[\"%s\"] ", next->time,
                    next->time - time (NULL), next->message);
            printf ("]\n");
#endif
            status = pthread_mutex_unlock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");
        }
    }
}
