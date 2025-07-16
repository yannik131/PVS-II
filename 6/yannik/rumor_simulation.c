#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    bool received_rumor;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int x;
    int y;
} member;

typedef struct {
    member *left;
    member *top;
    member *right;
    member *bottom;
} neighbors;

int correct_index(int i, int size) {
    if (i == -1)
        return size - 1;
    if (i == size)
        return 0;

    return i;
}

static int active_count = 0;
static bool simulation_over = false;
member world[10][10];
pthread_mutex_t global_mutex;

neighbors get_neighbors(int x, int y, int width, int height) {
    neighbors n;

    n.left = &world[y][correct_index(x - 1, width)];
    n.right = &world[y][correct_index(x + 1, width)];
    n.top = &world[correct_index(y - 1, height)][x];
    n.bottom = &world[correct_index(y + 1, height)][x];

    return n;
}

void wake_up(member *m) {
    pthread_mutex_lock(&m->mutex);
    m->received_rumor = true;
    pthread_cond_signal(&m->cond);
    pthread_mutex_unlock(&m->mutex);
}

void wake_up_neighbors(neighbors n) {
    wake_up(n.left);
    wake_up(n.right);
    wake_up(n.top);
    wake_up(n.bottom);
}

void *member_thread(void *data) {
    member *m = (member *)data;

    while (true) {
        pthread_mutex_lock(&global_mutex);
        while (!simulation_over && !m->received_rumor)
            pthread_cond_wait(&m->cond, &global_mutex);

        ++active_count;
        printf("world[%i][%i] got woken up, active_count = %i\n", m->x, m->y, active_count);
        pthread_mutex_unlock(&global_mutex);

        neighbors n = get_neighbors(m->x, m->y, 10, 10);
        wake_up_neighbors(n);

        pthread_mutex_lock(&m->mutex);
        m->received_rumor = false;
        pthread_mutex_unlock(&m->mutex);

        pthread_mutex_lock(&global_mutex);
        --active_count;
        printf("world[%i][%i] will now sleep, active_count = %i\n", m->x, m->y, active_count);
        if (active_count == 0) {
            simulation_over = true;
            for (int i = 0; i < 10; ++i) {
                for (int j = 0; j < 10; ++j)
                    pthread_cond_signal(&world[i][j].cond);
            }
            return NULL;
        }
        pthread_mutex_unlock(&global_mutex);

        sleep(1);
    }

    return NULL;
}

int main() {
    pthread_t threads[10][10];
    pthread_mutex_init(&global_mutex, NULL);

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            world[i][j].x = j;
            world[i][j].y = i;
            world[i][j].received_rumor = false;
            pthread_cond_init(&world[i][j].cond, NULL);
            pthread_mutex_init(&world[i][j].mutex, NULL);

            pthread_create(&threads[i][j], NULL, member_thread, &world[i][j]);
        }
    }

    pthread_mutex_lock(&global_mutex);
    world[3][4].received_rumor = true;
    pthread_cond_signal(&world[3][4].cond);
    pthread_mutex_unlock(&global_mutex);

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j)
            pthread_join(threads[i][j], NULL);
    }
}