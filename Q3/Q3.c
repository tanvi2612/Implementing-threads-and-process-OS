#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
sem_t server;
pthread_mutex_t mutex_cab[100];
pthread_mutex_t mutex_wait;
int n, m, k;
struct ride_info
{
    int cabtype, wait_time, ride_time, cab;
};
struct ride_info *riders[100];
struct point
{
    int num;
};
int min(int x, int y)
{
    return (x < y) ? x : y;
}
int cab_stat[100] = {0}, min_one = 101, cab_free[100] = {0}, pool_one[100] = {0}, pool_two[100] = {0};
void accept_state(int i, int cab, int mode_i);
void accept_payment(int i)
{
    sem_wait(&server);
    printf("Payment of rider %d is being processed \n", i);
    sleep(2);
    printf("Payment of rider %d is completed \n", i);
    sem_post(&server);
}
void onride(int r, int c)
{
    printf("Ride started for rider %d in cab %d with mode %d\n", r, c, cab_stat[c]);
    pthread_mutex_lock(&mutex_cab[c]);

    int rtime = riders[r]->ride_time;
    int stat = cab_stat[c];
    if (stat == 2)
    {
        rtime = min(riders[pool_two[c]]->ride_time, riders[pool_one[c]]->ride_time);
        sleep(rtime);
        cab_stat[c] = 3;
        pool_two[c] = 0;
        printf("Ride ended for rider %d in cab %d mode of cab is %d\n", r, c, cab_stat[c]);
        pthread_mutex_unlock(&mutex_cab[c]);

        return;
    }
    if (stat == 1)
    {

        sleep(rtime);
        cab_stat[c] = 0;
        printf("Ride ended for rider %d in cab %d mode of cab is %d\n", r, c, cab_stat[c]);
        pthread_mutex_unlock(&mutex_cab[c]);
        return;
    }
    while (rtime > 0)
    {
        stat = cab_stat[c];
        if (stat == 3)
        {
            sleep(1);
            rtime--;
            if (cab_stat[c] == 3)
            {
                continue;
            }
            else if (cab_stat[c] == 2)
            {
                if (rtime < riders[pool_two[c]]->ride_time)
                {
                    sleep(rtime);

                    riders[pool_two[c]]->ride_time -= rtime;
                    rtime = 0;
                    pool_one[c] = pool_two[c];
                    pool_two[c] = 0;
                    cab_stat[c] = 3;
                    //cab_stat[c] = 0;
                    printf("Ride ended for rider %d in cab %d mode of cab is %d\n", r, c, cab_stat[c]);
                    pthread_mutex_unlock(&mutex_cab[c]);
                    //onride(pool_one[c], c);
                    return;
                }
                else if (rtime == riders[pool_two[c]]->ride_time)
                {
                    sleep(rtime);

                    riders[pool_two[c]]->ride_time -= rtime;
                    rtime = 0;
                    
                    cab_stat[c] = 0;
                    printf("Ride ended for rider %d in cab %d mode of cab is %d\n", r, c, cab_stat[c]);
                    //printf("Ride ended for rider %d in cab %d mode of cab is %d\n", pool_two[c], c, cab_stat[c]);
                    pool_one[c] = 0;
                    pool_two[c] = 0;
                    
                    pthread_mutex_unlock(&mutex_cab[c]);
                    //onride(pool_one[c], c);
                    return;
                }
                else
                {
                    rtime -= riders[pool_two[c]]->ride_time;
                    pthread_mutex_unlock(&mutex_cab[c]);

                    sleep(1);
                    rtime--;
                    pthread_mutex_lock(&mutex_cab[c]);
                }
            }
        }
    }
    cab_stat[c] = 0;
    printf("Ride ended for rider %d in cab %d mode of cab is %d\n", r, c, cab_stat[c]);
    pthread_mutex_unlock(&mutex_cab[c]);
}
void book_cab(int i)
{
    int cab = 0;
    int mode_i = riders[i]->cabtype;
    int tp = riders[i]->wait_time;
    int temp = 0, flag = 0;
    while (temp++ <= tp)
    {
        for (int x = 1; x <= n; x++)
        {
            if (cab_stat[x] == 0)
            {
                cab = x;
                break;
            }
        }
        if (mode_i == 1 && cab != 0)
        {
            flag = 1;
            accept_state(i, cab, mode_i);
        }
        else if (mode_i == 2)
        {
            for (int x = 1; x <= n; x++)
            {

                if (cab_stat[x] == 3)
                {
                    cab = x;
                    break;
                }
            }
            if (cab != 0)
            {
                flag = 1;
                accept_state(i, cab, mode_i);
            }
        }
        if (flag == 1)
        {
            break;
        }
        sleep(1);
    }
    if (flag == 0)
    {
        printf("TimeOut\n");
    }
}
void accept_state(int i, int cab, int mode_i)
{
    printf("Rider %d booked cab %d for %d time as %d mode\n", i, cab, riders[i]->ride_time, mode_i);
    // int tp = riders[i]->wait_time;
    // int mode_i = riders[i]->cabtype;
    int flag = 0;
    riders[i]->cab = cab;
    int state_c = cab_stat[cab];
    if (state_c == 0)
    {
        if (mode_i == 1)
        {
            cab_stat[cab] = 1;
        }
        else if (mode_i == 2)
        {
            cab_stat[cab] = 3;
            pool_one[cab] = i;
        }
    }
    else if (state_c == 3)
    {

        cab_stat[cab] = 2;

        if (pool_two[cab] == 0)
        {
            pool_two[cab] = i;
        }
        else if (pool_one[cab] == 0)
        {
            pool_one[cab] = pool_two[cab];
            pool_two[cab] = i;
        }
    }
    //printf("halo\n");
    onride(i, cab);
    accept_payment(i);
}
void *riderthread(void *i)
{
    pthread_mutex_lock(&mutex_wait);
    struct point *ki = i;
    int num = ki->num;
    int tp = riders[num]->wait_time;
    pthread_mutex_unlock(&mutex_wait);
    book_cab(num);
    //sleep(tp);
}
void *driverthread(void *i)
{
    pthread_mutex_lock(&mutex_wait);
    struct point *ki = i;
    int num = ki->num;
    cab_stat[num] = 0;
    pthread_mutex_unlock(&mutex_wait);
}

pthread_mutex_t mutex_wait;
int main()
{

    for (int i = 1; i <= n; i++)
    {
        pthread_mutex_init(&mutex_cab[i], NULL);
    }

    printf("Enter the number of cabs, riders and Payment servers: ");
    scanf("%d %d %d", &n, &m, &k);
    sem_init(&server, 0, k);
    printf("\nFollowing are the various cab types available for a rider:\n Enter 1 for Premium\n Enter 2 for Pool\nRider details:\nCabtype\tMax_wait_time\tRide_time\n");
    pthread_t riders_th[m + 1], cabs_th[n + 1], payment_th[k + 1];
    for (int i = 1; i <= m; i++)
    {
        riders[i] = (struct ride_info *)malloc(sizeof(struct ride_info));
        riders[i]->cabtype = 1 + rand()%2;
        riders[i]->wait_time = 1 + rand()%6;
        riders[i]->ride_time = 3+ rand()%8;
        printf("%d\t%d\t%d\n", riders[i]->cabtype, riders[i]->wait_time,riders[i]->ride_time);
    }
    for (int i = 0; i <= n; i++)
    {
        cab_stat[i] = -1;
    }
    struct point *ki;
    pthread_mutex_init(&mutex_wait, NULL);
    for (int i = 1; i <= n; i++)
    {
        ki = (struct point *)malloc(sizeof(struct point));
        ki->num = i;
        pthread_create(&cabs_th[i], NULL, driverthread, ki);
        sleep(1);
    }
    for (int i = 1; i <= m; i++)
    {
        printf("Enter Rider %d\n", i);
        ki = (struct point *)malloc(sizeof(struct point));
        ki->num = i;
        pthread_create(&riders_th[i], NULL, riderthread, ki);
        sleep(1);
    }
    for (int i = 1; i <= m; i++)
    {
        pthread_join(riders_th[i], NULL);
    }

    pthread_mutex_destroy(&mutex_wait);
    sem_destroy(&server);
    exit(0);
}