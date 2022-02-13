#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "simdxorshift128plus.h"
#include <smmintrin.h>
#include <immintrin.h>

// defaults
static int repetitions = 100;
static u_int64_t columns = 128;
static u_int64_t rows = 128;
static u_int8_t show_progress = 0;
static u_int8_t produce_output = 0;
static int rank, cluster;
static char output_fname[255] = "life_";
// getopt
static struct option long_options[] =
        {
                {"help",        optional_argument, NULL, 'h'},
                {"output",      optional_argument, NULL, 'o'},
                {"progress",    optional_argument, NULL, 'p'},
                {"repetitions", optional_argument, NULL, 'R'},
                {"size",        optional_argument, NULL, 's'},
                {NULL,          0,                 NULL, 0}};

void field_initializer(u_int8_t *state) {
    //fills fields with random numbers 0 = dead, 1 = alive
    srand(time(NULL) + rank);
    int num_one = rand();
    int num_two = rand() + rank;

    // create a new key
    avx_xorshift128plus_key_t mykey;
    avx_xorshift128plus_init(num_one, num_two, &mykey); // values 324, 4444 are arbitrary, must be non-zero


    for (int i = 0; i < columns * rows; i = i + 32) {
        // generate 32 random bytes, do this as many times as you want
        __m256i randomstuff = avx_xorshift128plus(&mykey);
        state[i] = _mm256_extract_epi8(randomstuff, 0) % 2;
        state[i + 1] = _mm256_extract_epi8(randomstuff, 1) % 2;
        state[i + 2] = _mm256_extract_epi8(randomstuff, 2) % 2;
        state[i + 3] = _mm256_extract_epi8(randomstuff, 3) % 2;
        state[i + 4] = _mm256_extract_epi8(randomstuff, 4) % 2;
        state[i + 5] = _mm256_extract_epi8(randomstuff, 5) % 2;
        state[i + 6] = _mm256_extract_epi8(randomstuff, 6) % 2;
        state[i + 7] = _mm256_extract_epi8(randomstuff, 7) % 2;
        state[i + 8] = _mm256_extract_epi8(randomstuff, 8) % 2;
        state[i + 9] = _mm256_extract_epi8(randomstuff, 9) % 2;
        state[i + 10] = _mm256_extract_epi8(randomstuff, 10) % 2;
        state[i + 11] = _mm256_extract_epi8(randomstuff, 11) % 2;
        state[i + 12] = _mm256_extract_epi8(randomstuff, 12) % 2;
        state[i + 13] = _mm256_extract_epi8(randomstuff, 13) % 2;
        state[i + 14] = _mm256_extract_epi8(randomstuff, 14) % 2;
        state[i + 15] = _mm256_extract_epi8(randomstuff, 15) % 2;
        state[i + 16] = _mm256_extract_epi8(randomstuff, 16) % 2;
        state[i + 17] = _mm256_extract_epi8(randomstuff, 17) % 2;
        state[i + 18] = _mm256_extract_epi8(randomstuff, 18) % 2;
        state[i + 19] = _mm256_extract_epi8(randomstuff, 19) % 2;
        state[i + 20] = _mm256_extract_epi8(randomstuff, 20) % 2;
        state[i + 21] = _mm256_extract_epi8(randomstuff, 21) % 2;
        state[i + 22] = _mm256_extract_epi8(randomstuff, 22) % 2;
        state[i + 23] = _mm256_extract_epi8(randomstuff, 23) % 2;
        state[i + 24] = _mm256_extract_epi8(randomstuff, 24) % 2;
        state[i + 25] = _mm256_extract_epi8(randomstuff, 25) % 2;
        state[i + 26] = _mm256_extract_epi8(randomstuff, 26) % 2;
        state[i + 27] = _mm256_extract_epi8(randomstuff, 27) % 2;
        state[i + 28] = _mm256_extract_epi8(randomstuff, 28) % 2;
        state[i + 29] = _mm256_extract_epi8(randomstuff, 29) % 2;
        state[i + 30] = _mm256_extract_epi8(randomstuff, 30) % 2;
        state[i + 31] = _mm256_extract_epi8(randomstuff, 31) % 2;
    }
}

void calculate_next_gen(u_int8_t *state, u_int8_t *state_old) {
    //i = row, j = column

    // corners
    //calculate_corners(state, state_old);
    // left and right edge
    //calculate_left_right(state, state_old);
    // top and bottom edge
    //calculate_top_bottom(state, state_old);

    // devide rows
    u_int64_t block_size = rows / cluster;
    u_int64_t first_row = block_size * rank;
    u_int64_t last_row = first_row + block_size;
    // creating buffer
    u_int8_t *tmp_buffer = (u_int8_t *) malloc(columns * block_size * sizeof(u_int8_t));
    u_int8_t *new_test_state = (u_int8_t *) malloc(columns * rows * sizeof(u_int8_t));



    // in case we process the first rows
    if (first_row == 0) {
        first_row = 1;
        for (int i = 1; i < columns - 1; i++) {
            u_int8_t sum_of_t_edge = state_old[i - 1] +
                                     state_old[i + 1] +
                                     state_old[columns + (i - 1)] +
                                     state_old[columns + i] +
                                     state_old[columns + (i + 1)] +
                                     state_old[(rows - 1) * columns + i] +
                                     state_old[(rows - 1) * columns + i + 1] +
                                     state_old[(rows - 1) * columns + i - 1];
            tmp_buffer[i] = (sum_of_t_edge == 3) | ((sum_of_t_edge == 2) & state_old[i]);
        }
        u_int8_t corner_sum;
        // top left
        corner_sum = state_old[1] +
                     state_old[columns] +
                     state_old[columns + 1] +
                     state_old[(rows - 1) * columns] +
                     state_old[(rows - 1) * columns + 1] +
                     state_old[columns - 1] +
                     state_old[2 * columns - 1] +
                     state_old[rows * columns - 1];
        tmp_buffer[0] = (corner_sum == 3) | ((corner_sum == 2) & state_old[0]);
        // top right
        corner_sum = state_old[columns - 2] +
                     state_old[2 * columns - 1] +
                     state_old[2 * columns - 2] +
                     state_old[rows * columns - 1] +
                     state_old[rows * columns - 2] +
                     state_old[0] +
                     state_old[columns] +
                     state_old[(rows - 1) * columns];
        tmp_buffer[columns - 1] = (corner_sum == 3) | ((corner_sum == 2) & state_old[columns - 1]);



        // in case we process the last rows
    } else if (last_row == rows) {
        last_row = rows - 1;
        for (int i = 1; i < columns - 1; i++) {
            u_int8_t sum_of_b_edge = state_old[(rows - 1) * columns + (i - 1)] +
                                     state_old[(rows - 1) * columns + (i + 1)] +
                                     state_old[(rows - 2) * columns + (i - 1)] +
                                     state_old[(rows - 2) * columns + i] +
                                     state_old[(rows - 2) * columns + (i + 1)] +
                                     state_old[i] +
                                     state_old[i - 1] +
                                     state_old[i + 1];
            tmp_buffer[(block_size - 1) * columns + i] =
                    (sum_of_b_edge == 3) | ((sum_of_b_edge == 2) & state_old[(rows - 1) * columns + i]);
        }
        u_int8_t corner_sum;
        // bottom left
        corner_sum = state_old[(rows - 2) * columns] +
                     state_old[(rows - 2) * columns + 1] +
                     state_old[(rows - 1) * columns + 1] +
                     state_old[0] +
                     state_old[1] +
                     state_old[columns - 1] +
                     state_old[(rows - 1) * columns - 1] +
                     state_old[(rows * columns - 1)];
        tmp_buffer[(block_size - 1) * columns] =
                (corner_sum == 3) | ((corner_sum == 2) & state_old[(rows - 1) * columns]);
        // bottom right
        corner_sum = state_old[0] +
                     state_old[columns - 1] +
                     state_old[columns - 2] +
                     state_old[(rows - 2) * columns] +
                     state_old[(rows - 1) * columns] +
                     state_old[(rows - 1) * columns - 1] +
                     state_old[(rows - 1) * columns - 2] +
                     state_old[(rows * columns - 2)];
        state[block_size * columns - 1] = (corner_sum == 3) | ((corner_sum == 2) & state_old[rows * columns - 1]);
    }


    // middle
    for (int i = first_row; i < last_row; i++) {
        for (int j = 1; j < columns - 1; j++) {
            //count up a number (8)
            u_int8_t sum_of_8 = state_old[(i - 1) * columns + (j - 1)] +
                                state_old[(i - 1) * columns + j] +
                                state_old[(i - 1) * columns + (j + 1)] +
                                state_old[i * columns + (j - 1)] +
                                state_old[i * columns + (j + 1)] +
                                state_old[(i + 1) * columns + (j - 1)] +
                                state_old[(i + 1) * columns + j] +
                                state_old[(i + 1) * columns + (j + 1)];
            tmp_buffer[(i - rank * block_size) * columns + j] =
                    (sum_of_8 == 3) | ((sum_of_8 == 2) & state_old[i * columns + j]);
        }
        // left and right
        u_int8_t sum_of_l_edge = state_old[i * columns + 1] +
                                 state_old[(i - 1) * columns] +
                                 state_old[(i - 1) * columns + 1] +
                                 state_old[(i + 1) * columns] +
                                 state_old[(i + 1) * columns + 1] +
                                 state_old[i * columns - 1] +
                                 state_old[(i + 1) * columns - 1] +
                                 state_old[(i + 2) * columns - 1];
        tmp_buffer[(i - rank * block_size) * columns] =
                (sum_of_l_edge == 3) | ((sum_of_l_edge == 2) & state_old[i * columns]);
        u_int8_t sum_of_r_edge = state_old[(i + 1) * columns - 2] +
                                 state_old[i * columns - 2] +
                                 state_old[i * columns - 1] +
                                 state_old[(i + 2) * columns - 2] +
                                 state_old[(i + 2) * columns - 1] +
                                 state_old[(i - 1) * columns] +
                                 state_old[i * columns] +
                                 state_old[(i + 1) * columns];
        tmp_buffer[((i - rank * block_size) + 1) * columns - 1] =
                (sum_of_r_edge == 3) | ((sum_of_r_edge == 2) & state_old[(i + 1) * columns - 1]);
    }
    MPI_Allgather(tmp_buffer, block_size * columns, MPI_UINT8_T, state, block_size * columns, MPI_UINT8_T,
                  MPI_COMM_WORLD);
    return;
}

void write_pbm_file(u_int8_t *state, int i) {
    FILE *fptr;
    char new_filename[65];
    sprintf(new_filename, "%s%06d.pbm", output_fname, i);
    fptr = fopen(new_filename, "w");
    fprintf(fptr, "P1\n");
    fprintf(fptr, "# This is the %06d result. Have fun :)\n", i);
    fprintf(fptr, "%lu %lu\n", columns, rows);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            fprintf(fptr, "%d ", state[i * columns + j]);
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
    return;
}


void argments(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt_long(argc, argv, "hpR:s:o:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'R':
                if (strlen(optarg) > 254) {
                    printf("Given repetitions too large.\n");
                    exit(1);
                }
                repetitions = atoi(optarg);
                break;
            case 'o':
                printf("%s", optarg);
                if (strlen(optarg) > 254) {
                    printf("Output filename too big.\n");
                    exit(1);
                }
                sprintf(output_fname, "%s", optarg);
                produce_output = 1;
                break;
            case 'p':
                show_progress = 1;
                break;
            case 's':
                if (strlen(optarg) > 254) {
                    printf("Given size too large.\n");
                    exit(1);
                }
                char size[255];
                sprintf(size, "%s", optarg);
                char *word = strtok(size, ",");
                columns = strtol(word, NULL, 10);
                word = strtok(NULL, ",");
                rows = strtol(word, NULL, 10);
                break;
            case 'h':
                printf("Welcome to the game of life!\nAvailable arguments:\n");
                printf("-h, --help                 prints this help page and exits\n");
                printf("-o, --output [filename]    default: life_xxxxxx.pbm, provide an output filename\n");
                printf("-p, --progress             default: false; prints progress on terminal\n");
                printf("-R, --repetitions [int]    default: 3 repetitions; specifies the number of images created\n");
                printf("-s, --size <columns,rows>  default: 128x128; specifies the number of columns and rows\n");
                exit(0);
        }
    }
    return;
}

int main(int argc, char *argv[]) {
    // MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // arguments
    argments(argc, argv);
    // welcome information
    if (rank == 0) {
        printf("Welcome to the MPI game of life!\n");
        printf("We are running with %d ranks\n", cluster);
        printf("Game size: Columns: %lu, Rows: %lu.\n", columns, rows);
        printf("Starting now...\n");
    }
    // initializing states and pointers
    u_int8_t *state_1 = (u_int8_t *) malloc(columns * rows * sizeof(u_int8_t));
    u_int8_t *state_2 = (u_int8_t *) malloc(columns * rows * sizeof(u_int8_t));
    u_int8_t *state_in = state_1;
    u_int8_t *state_out = state_2;
    u_int8_t *state_tmp = NULL;
    // starting clock
    clock_t t;
    double time_rand;
    double time_calc = 0;
    double time_out = 0;
    // filling with random numbers
    if (rank == 0) {
        t = clock();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    field_initializer(state_1);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        t = clock() - t;
        time_rand = ((double) t) / CLOCKS_PER_SEC; // in seconds
        // write random pattern as -1 file
        if (produce_output) {
            t = clock();
            write_pbm_file(state_in, -1);
            t = clock() - t;
            time_out += ((double) t) / CLOCKS_PER_SEC;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //calculation
    for (int i = 0; i < repetitions; i++) {
        if (rank == 0) {
            t = clock();
        }
        MPI_Barrier(MPI_COMM_WORLD);
        calculate_next_gen(state_out, state_in);
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            t = clock() - t;
            time_calc += ((double) t) / CLOCKS_PER_SEC;
        }
        state_tmp = state_in;
        state_in = state_out;
        state_out = state_tmp;
        if (rank == 0) {
            if (show_progress) {
                double percentage = 100.0 * (i + 1) / repetitions;
                printf("%.1lf%c\n", percentage, 37);
            }
            if (produce_output) {
                t = clock();
                write_pbm_file(state_in, i);
                t = clock() - t;
                time_out += ((double) t) / CLOCKS_PER_SEC;
            }
        }
    }
    if (rank == 0) {
        printf("Field initializer took %f seconds to execute.\n", time_rand);
        printf("Calculation took %f seconds to execute.\n", time_calc);
        printf("Writing pbm files took %f seconds to execute.\n", time_out);
        printf("Done :)\n");
    }
    // MPI
    MPI_Finalize();
    return 0;
}
