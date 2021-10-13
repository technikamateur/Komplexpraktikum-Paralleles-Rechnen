#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

/*
  typedef enum omp_sched_t {
    omp_sched_static = 1,
    omp_sched_dynamic = 2,
    omp_sched_guided = 3,
    omp_sched_auto = 4
  } omp_sched_t;
*/

// defaults
static float repetitions = 100;
static u_int64_t columns = 128;
static u_int64_t rows = 128;
static u_int8_t show_progress = 0;
// getopt
static struct option long_options[] =
    {
        {"help", optional_argument, NULL, 'h'},
        {"progress", optional_argument, NULL, 'p'},
        {"repetitions", optional_argument, NULL, 'R'},
        {"size", optional_argument, NULL, 's'},
        {"threads", optional_argument, NULL, 't'},
        {NULL, 0, NULL, 0}};

void field_initializer(u_int8_t *state)
{
    //fills fields with random numbers 0 = dead, 1 = alive
    srand(time(0));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            state[(i * columns) + j] = rand() % 2;
        }
    }
    return;
}

void calculate_corners(u_int8_t *state, u_int8_t *state_old)
{
    u_int8_t corner_sum = 0;
    // top left
    corner_sum = state_old[1] +
                 state_old[columns] +
                 state_old[columns + 1] +
                 state_old[(rows - 1) * columns] +
                 state_old[(rows - 1) * columns + 1] +
                 state_old[columns - 1] +
                 state_old[2 * columns - 1] +
                 state_old[rows * columns - 1];
    state[0] = (corner_sum == 3) | ((corner_sum == 2) & state_old[0]);
    // top right
    corner_sum = state_old[columns - 2] +
                 state_old[2 * columns - 1] +
                 state_old[2 * columns - 2] +
                 state_old[rows * columns - 1] +
                 state_old[rows * columns - 2] +
                 state_old[0] +
                 state_old[columns] +
                 state_old[(rows - 1) * columns];
    state[columns - 1] = (corner_sum == 3) | ((corner_sum == 2) & state_old[columns - 1]);
    // bottom left
    corner_sum = state_old[(rows - 2) * columns] +
                 state_old[(rows - 2) * columns + 1] +
                 state_old[(rows - 1) * columns + 1] +
                 state_old[0] +
                 state_old[1] +
                 state_old[columns - 1] +
                 state_old[(rows - 1) * columns - 1] +
                 state_old[(rows * columns - 1)];
    state[(rows - 1) * columns] = (corner_sum == 3) | ((corner_sum == 2) & state_old[(rows - 1) * columns]);
    // bottom right
    corner_sum = state_old[0] +
                 state_old[columns - 1] +
                 state_old[columns - 2] +
                 state_old[(rows - 2) * columns] +
                 state_old[(rows - 1) * columns] +
                 state_old[(rows - 1) * columns - 1] +
                 state_old[(rows - 1) * columns - 2] +
                 state_old[(rows * columns - 2)];
    state[rows * columns - 1] = (corner_sum == 3) | ((corner_sum == 2) & state_old[rows * columns - 1]);
}

void calculate_left_right(u_int8_t *state, u_int8_t *state_old)
{
#pragma omp parallel for
    for (int i = 1; i < rows - 1; i++)
    {
        u_int8_t sum_of_l_edge = state_old[i * columns + 1] +
                                 state_old[(i - 1) * columns] +
                                 state_old[(i - 1) * columns + 1] +
                                 state_old[(i + 1) * columns] +
                                 state_old[(i + 1) * columns + 1] +
                                 state_old[i * columns - 1] +
                                 state_old[(i + 1) * columns - 1] +
                                 state_old[(i + 2) * columns - 1];
        state[i * columns] = (sum_of_l_edge == 3) | ((sum_of_l_edge == 2) & state_old[i * columns]);
        u_int8_t sum_of_r_edge = state_old[(i + 1) * columns - 2] +
                                 state_old[i * columns - 2] +
                                 state_old[i * columns - 1] +
                                 state_old[(i + 2) * columns - 2] +
                                 state_old[(i + 2) * columns - 1] +
                                 state_old[(i - 1) * columns] +
                                 state_old[i * columns] +
                                 state_old[(i + 1) * columns];
        state[(i + 1) * columns - 1] = (sum_of_r_edge == 3) | ((sum_of_r_edge == 2) & state_old[(i + 1) * columns - 1]);
    }
}

void calculate_top_bottom(u_int8_t *state, u_int8_t *state_old)
{
#pragma omp parallel for
    for (int i = 1; i < columns - 1; i++)
    {
        u_int8_t sum_of_t_edge = state_old[i - 1] +
                                 state_old[i + 1] +
                                 state_old[2 * columns + (i - 1)] +
                                 state_old[2 * columns + i] +
                                 state_old[2 * columns + (i + 1)] +
                                 state_old[(rows - 1) * columns + i] +
                                 state_old[(rows - 1) * columns + i + 1] +
                                 state_old[(rows - 1) * columns + i - 1];
        state[i] = (sum_of_t_edge == 3) | ((sum_of_t_edge == 2) & state_old[i]);
        u_int8_t sum_of_b_edge = state_old[(rows - 1) * columns + (i - 1)] +
                                 state_old[(rows - 1) * columns + (i + 1)] +
                                 state_old[(rows - 2) * columns + (i - 1)] +
                                 state_old[(rows - 2) * columns + i] +
                                 state_old[(rows - 2) * columns + (i + 1)] +
                                 state_old[i] +
                                 state_old[i - 1] +
                                 state_old[i + 1];
        state[(rows - 1) * columns + i] = (sum_of_b_edge == 3) | ((sum_of_b_edge == 2) & state_old[(rows - 1) * columns + i]);
    }
}

void calculate_next_gen(u_int8_t *state, u_int8_t *state_old)
{
    //i = row, j = column

    // corners
    calculate_corners(state, state_old);
    // left and right edge
    calculate_left_right(state, state_old);
    // top and bottom edge
    calculate_top_bottom(state, state_old);
    // middle
#pragma omp parallel for
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < columns - 1; j++)
        {
            //count up a number (8)
            u_int8_t sum_of_8 = state_old[(i - 1) * columns + (j - 1)] +
                                state_old[(i - 1) * columns + j] +
                                state_old[(i - 1) * columns + (j + 1)] +
                                state_old[i * columns + (j - 1)] +
                                state_old[i * columns + (j + 1)] +
                                state_old[(i + 1) * columns + (j - 1)] +
                                state_old[(i + 1) * columns + j] +
                                state_old[(i + 1) * columns + (j + 1)];
            state[i * columns + j] = (sum_of_8 == 3) | ((sum_of_8 == 2) & state_old[i * columns + j]);
        }
    }
    return;
}

void argments(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt_long(argc, argv, "hpR:s:t:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'R':
            if (strlen(optarg) > 4)
            {
                printf("Given repetitions too large. Allowed max.: 9999\n");
                exit(1);
            }
            repetitions = atoi(optarg);
            break;
        case 'p':
            show_progress = 1;
            break;
        case 's':
            if (strlen(optarg) > 11)
            {
                printf("Given size too large. Allowed max.: 99999x99999\n");
                exit(1);
            }
            char size[11];
            sprintf(size, "%s", optarg);
            char *word = strtok(size, ",");
            columns = strtol(word, NULL, 10);
            word = strtok(NULL, ",");
            rows = strtol(word, NULL, 10);
            break;
        case 't':
            omp_set_num_threads(atoi(optarg));
            break;
        case 'h':
            printf("Welcome to the game of life!\nAvailable arguments:\n");
            printf("-h, --help                 prints this help page and exits\n");
            printf("-p, --progress             default: false; prints progress on terminal\n");
            printf("-R, --repetitions [int]    default: 3 repetitions; specifies the number of images created\n");
            printf("-s, --size <columns,rows>  default: 128x128; specifies the number of columns and rows\n");
            printf("-t, --threads [int]        default: auto detection; specifies the number of threads which will be spawned\n\n");
            printf("You can set scheduling via environmental variables e.g: export OMP_SCHEDULE=\"dynamic\". Your scheduling is printed on startup\n");
            printf("omp_sched_static = 1,\nomp_sched_dynamic = 2,\nomp_sched_guided = 3,\nomp_sched_auto = 4");
            exit(0);
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    // arguments
    argments(argc, argv);
    omp_sched_t kind;
    int chunk;
    omp_get_schedule(&kind, &chunk);
    // welcome information
    printf("Welcome to the game of life!\n");
    printf("We are doing %.0lf repetitions with %d thread(s)!\n", repetitions, omp_get_max_threads());
    printf("Scheduling: %d %d\n", kind, chunk);
    printf("Game size: Columns: %lu, Rows: %lu.\n", columns, rows);
    printf("Starting now...\n");
    // initalizing states and pointers
    u_int8_t *state_1 = (u_int8_t *)malloc(columns * rows * sizeof(u_int8_t));
    u_int8_t *state_2 = (u_int8_t *)malloc(columns * rows * sizeof(u_int8_t));
    u_int8_t *state_in = state_1;
    u_int8_t *state_out = state_2;
    u_int8_t *state_tmp = NULL;
    field_initializer(state_1);
    //calculation
    for (float i = 0; i < repetitions; i++)
    {
        calculate_next_gen(state_out, state_in);
        state_tmp = state_in;
        state_in = state_out;
        state_out = state_tmp;
        if (show_progress)
        {
            printf("%.1lf%c\n", ((i + 1) / repetitions) * 100, 37);
        }
    }
    if (!show_progress)
    {
        printf("Done :)\n");
    }

    exit(0);
}