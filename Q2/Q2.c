#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
int robo_prep[1000], robo_v[1000]; // *ves_cap;
pthread_mutex_t mutex[100];
pthread_mutex_t mutex_wait;
pthread_mutex_t mutex_serve[100];

int front = -1, rear = -1, size = 100;
int table_free[100], portion_alot[100], table_cap[100], stud_queue[100], stud_stat[100];
int bir_cooked[100], no_fed = 0;
int m, n, k;
struct cap
{
    int a[15];
};
struct cap *ves_cap[100];
struct info
{
    int chef;
    int v_no;
};
struct info *tab_info[100];

void enQueue(int value, int arr[])
{
    if ((front == 0 && rear == size - 1) ||
        (rear == (front - 1) % (size - 1)))
    {
        printf("\nQueue is Full");
        return;
    }

    else if (front == -1) /* Insert First Element */
    {
        front = rear = 0;
        arr[rear] = value;
    }

    else if (rear == size - 1 && front != 0)
    {
        rear = 0;
        arr[rear] = value;
    }

    else
    {
        rear++;
        arr[rear] = value;
    }
    //sleep(1);
}
int total_inq()
{
    int sz;
    if (front == -1 && rear == -1)
    {
        sz = 0;
    }
    else
    {
        sz = front > rear ? (size - front + rear + 1) : (rear - front + 1);
    }
    return sz;
}
// Function to delete element from Circular Queue
int deQueue(int arr[])
{
    if (front == -1)
    {
        printf("\nQueue is Empty");
        return -1;
    }

    int data = arr[front];
    arr[front] = -1;
    if (front == rear)
    {
        front = -1;
        rear = -1;
    }
    else if (front == size - 1)
        front = 0;
    else
        front++;

    return data;
}

int serving_status[100];
struct serve
{
    int num;
};
struct serve *table_total[100];
//struct serve *student[100];
void wait_for_slot(int i)
{
    while (1)
    {
        if (i == stud_queue[front])
        {
            break;
        }
    }
    while(i == stud_queue[front]){
        continue;
    }
    
    return;
}
void biryani_ready(int i)
{

    pthread_mutex_lock(&mutex_serve[i]);
    bir_cooked[i] = 1;
    int c = 0;
    while (c < robo_v[i])
    {
        for (int j = 1; j <= n && c < robo_v[i]; j++)
        {
            if (table_free[j] == 0)
            {
                portion_alot[j] = ves_cap[i]->a[c++];
                table_free[j] = 1;
                printf("Service table %d has been given %d portions from %d chef container %d\n", j, portion_alot[j], i, c);
                tab_info[j]->chef = i;
                tab_info[j]->v_no = c;
            }
        }
        if (c < robo_v[i])
        {
            int x = 1;
            while (1)
            {
                while (x <= n)
                {
                    if (table_free[x] == 0)
                    {
                        break;
                    }
                    x++;
                }
                if (table_free[x] == 0)
                {
                    break;
                }
            }

            //pthread_cond_wait(&cond[i], &mutex_serve[i]);
        }
    }

    int v_emp = 0;
    while (v_emp < robo_v[i])
    {
        v_emp = 0;
        for (int x = 0; x <= robo_v[i]; x++)
        {
            if (ves_cap[i]->a[x] == 0)
            {
                v_emp++;
            }
        }
    }
    printf("chef %d finished vessels are %d\n", i, v_emp);
    bir_cooked[i] = 0;
    pthread_mutex_unlock(&mutex_serve[i]);
}
void *robothread(void *i)
{
    pthread_mutex_lock(&mutex_wait);
    struct serve *ki = i;
    int num = ki->num;
    // pthread_mutex_lock(&mutex[n]);
    pthread_mutex_unlock(&mutex_wait);
    while (1)
    {
        if (no_fed == k)
        {
            break;
        }
        robo_prep[num] = rand() % 4 + 2;
        robo_v[num] = rand() % 10 + 1;
        int cap_count = 0;
        ves_cap[num] = malloc(sizeof(struct cap));

        while (cap_count < robo_v[num])
            ves_cap[num]->a[cap_count++] = rand() % 26 + 25;
        //printf("halo\n");
        printf("robot %d, vessels %d, prep time %d\n", ki->num, robo_v[num], robo_prep[num]);

        sleep(robo_prep[num]);
        printf("Robot %d finished cooking %d vessels in %d time.\n", num, robo_v[num], robo_prep[num]);
        biryani_ready(num);
    }
}
int ready_to_serve(int i, int no, int served)
{

    if (front == -1 || portion_alot[i] == 0)
    {
        return 0;
    }
    printf("Serving Table %d is ready to serve with %d slots\n", i, no);
    table_total[i]->num -= no;
    portion_alot[i] -= no;
    table_cap[i] = 0;
    no_fed += no;
    for (int j = 0; j < no; j++)
    {
        if (front == -1)
        {
            j--;
            continue;
        }
        int temp = deQueue(stud_queue);
        printf("Student %d ready to be assigned\n", temp);
        stud_stat[temp] = 1;
        printf("Student %d assigned a slot on the serving table %d and waiting to be served\n", temp, i);
        served++;
    }
    return served;
}
void *tablethread(void *i)
{
    pthread_mutex_lock(&mutex_wait);
    struct serve *ki = i;
    int num = ki->num;
    // pthread_mutex_lock(&mutex[n]);

    pthread_mutex_unlock(&mutex_wait);
    while (1)
    {
        //pthread_mutex_lock(&mutex[num]);
        if (table_free[num] == 1)
        {
            table_cap[num] = rand() % 10 + 1;
            if (table_cap[num] > portion_alot[num])
            {
                table_cap[num] = portion_alot[num];
            }
            if (table_cap[num] > total_inq())
            {
                table_cap[num] = total_inq();
            }

            int served = ready_to_serve(num, table_cap[num], served);

            //printf("For table %d portion left = %d, table_total = %d, no__fed = %d\n", num, portion_alot[num], table_total[num]->num, no_fed);
            if (portion_alot[num] == 0)
            {
                int temp = tab_info[num]->chef;
                int ves_num = tab_info[num]->v_no;
                ves_cap[temp]->a[ves_num] = 0;
                printf("Serving Container of Table %d is empty, waiting for refill\n", num);
            }
            if (portion_alot[num] == 0 && table_total[num]->num != 0 && no_fed != k)
            {
                table_free[num] = 0;
            }
        }
        if (table_free[num] == 0)
        {
            portion_alot[num] = 0;
            table_cap[num] = 0;
            //pthread_cond_signal(&cond[num]);
        }
        if (table_total[num]->num == 0 || no_fed == k)
        {
            break;
        }
        else if (front == -1)
        {
            while (front == -1)
            {
            }
           
        }
        //pthread_mutex_unlock(&mutex[num]);
    }
}
void *studentthread(void *i)
{
    pthread_mutex_lock(&mutex_wait);
    struct serve *ki = i;
    int num = ki->num;
    // pthread_mutex_lock(&mutex[n]);
    printf("Student %d arrived\n", num);
    enQueue(num, stud_queue);

    //sleep(1);
    pthread_mutex_unlock(&mutex_wait);
    printf("Student %d is waiting to be allocated a slot on the serving table\n", num);
    wait_for_slot(num);
}
int main()
{

    scanf("%d %d %d", &m, &n, &k);
    pthread_t robo[m + 1], table[n + 1], stud[k + 1];

    for (int i = 1; i <= m; i++)
    {
        bir_cooked[i] = 0;
        pthread_mutex_init(&mutex[i], NULL);
        pthread_mutex_init(&mutex_serve[i], NULL);
        //pthread_cond_init(&cond[i], NULL);
        //pthread_cond_init(&cond_p[i], NULL);
    }
    for (int i = 1; i <= n; i++)
    {
        table_total[i] = (struct serve *)malloc(sizeof(struct serve));
        table_total[i]->num = 100;
        table_free[i] = 0;
        tab_info[i] = (struct info *)malloc(sizeof(struct info));
    }
    // int robo_prep[m], robo_v[m], ves_cap[100];
    pthread_mutex_init(&mutex_wait, NULL);
    for (int i = 1; i <= m; i++)
    {
        //printf("halo\n");

        struct serve *ki;
        ki = (struct serve *)malloc(sizeof(struct serve));
        ki->num = i;

        pthread_create(&robo[i], NULL, robothread, ki);

        sleep(2); // CHANGE THIS
        //pthread_join(robo[i], NULL);
    }
    //pthread_mutex_destroy(&mutex_wait);
    // for(int i = 1; i<=m; i++){
    //     pthread_join(robo[i], NULL);
    // }
    //sleep(1);
    struct serve *se;
    se = (struct serve *)malloc(sizeof(struct serve));

    se->num = 0;

    for (int i = 1; i <= n; i++)
    {

        serving_status[i] = 1;
        se->num = i;
        pthread_create(&table[i], NULL, tablethread, se);
        sleep(2);
    }
    for (int i = 1; i <= k; i++)
    {

        stud_stat[i] = 1;
        se->num = i;
        pthread_create(&stud[i], NULL, studentthread, se);
        sleep(2);
    }
}
