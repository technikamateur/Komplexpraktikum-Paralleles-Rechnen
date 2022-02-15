#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include <math.h>

// defaults
static int repetitions = 100;
static u_int32_t columns = 128;
static u_int32_t rows = 128;
static u_int8_t show_progress = 0;
static u_int8_t produce_output = 0;
static char output_fname[255] = "life_";

// MPI
// works only for square
static int rank, cluster;
static u_int32_t block_rc;
static u_int32_t blocks_per_side;
static u_int32_t rank_index;

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
    unsigned seed = time(0) + rank;
    for (int i = 1; i < block_rc - 1; i++) {
        for (int j = 1; j < block_rc - 1; j++) {
            state[i * block_rc + j] = rand_r(&seed) % 2;
        }
    }
}

void init_neighbour(u_int32_t *neighbour_matrix) {
    for (int i = 1; i < blocks_per_side + 1; i++) {
        for (int j = 1; j < blocks_per_side + 1; j++) {
            neighbour_matrix[i * (blocks_per_side + 2) + j] = (i - 1) * blocks_per_side + (j - 1);
        }
    }
    for (int i = 1; i < blocks_per_side + 1; i++) {
        //left
        neighbour_matrix[i * (blocks_per_side + 2)] = neighbour_matrix[(i + 1) * (blocks_per_side + 2) - 2];
        //right
        neighbour_matrix[(i + 1) * (blocks_per_side + 2) - 1] = neighbour_matrix[i * (blocks_per_side + 2) + 1];
        //top
        neighbour_matrix[i] = neighbour_matrix[blocks_per_side * (blocks_per_side + 2) + i];
        //bottom
        neighbour_matrix[(blocks_per_side + 1) * (blocks_per_side + 2) + i] = neighbour_matrix[(blocks_per_side + 2) +
                                                                                               i];
    }
    //top left corner
    neighbour_matrix[0] = neighbour_matrix[blocks_per_side * (blocks_per_side + 2)];
    //top right corner
    neighbour_matrix[blocks_per_side + 1] = neighbour_matrix[(blocks_per_side + 1) * (blocks_per_side + 2) - 1];
    // bottom left
    neighbour_matrix[(blocks_per_side + 1) * (blocks_per_side + 2)] = neighbour_matrix[blocks_per_side + 2];
    //bottom right
    neighbour_matrix[(blocks_per_side + 2) * (blocks_per_side + 2) - 1] = neighbour_matrix[(blocks_per_side + 2) + 1];
    return;
}

void edge_maker(u_int8_t *state, u_int8_t *state_old) {
    // top and bottom + corners
    for (int i = 1; i < block_rc - 1; i++) {
        u_int8_t sum_of_t_edge = state_old[i - 1] +
                                 state_old[i] +
                                 state_old[i + 1] +
                                 state_old[block_rc + i - 1] +
                                 state_old[block_rc + i + 1] +
                                 state_old[2 * block_rc + i - 1] +
                                 state_old[2 * block_rc + i] +
                                 state_old[2 * block_rc + i + 1];
        state[block_rc + i] = (sum_of_t_edge == 3) | ((sum_of_t_edge == 2) & state_old[block_rc + i]);

        u_int8_t sum_of_b_edge = state_old[(block_rc - 3) * block_rc + i - 1] +
                                 state_old[(block_rc - 3) * block_rc + i] +
                                 state_old[(block_rc - 3) * block_rc + i + 1] +
                                 state_old[(block_rc - 2) * block_rc + i - 1] +
                                 state_old[(block_rc - 2) * block_rc + i + 1] +
                                 state_old[(block_rc - 1) * block_rc + i - 1] +
                                 state_old[(block_rc - 1) * block_rc + i] +
                                 state_old[(block_rc - 1) * block_rc + i + 1];
        state[(block_rc - 2) * block_rc + i] =
                (sum_of_b_edge == 3) | ((sum_of_b_edge == 2) & state_old[(block_rc - 2) * block_rc + i]);
    }

    for (int i = 2; i < block_rc - 2; i++) {
        u_int8_t sum_of_l_edge = state_old[(i - 1) * block_rc] +
                                 state_old[(i - 1) * block_rc + 1] +
                                 state_old[(i - 1) * block_rc + 2] +
                                 state_old[i * block_rc] +
                                 state_old[i * block_rc + 2] +
                                 state_old[(i + 1) * block_rc] +
                                 state_old[(i + 1) * block_rc + 1] +
                                 state_old[(i + 1) * block_rc + 2];
        state[i * block_rc + 1] = (sum_of_l_edge == 3) | ((sum_of_l_edge == 2) & state_old[i * block_rc + 1]);

        u_int8_t sum_of_r_edge = state_old[i * block_rc - 3] +
                                 state_old[i * block_rc - 2] +
                                 state_old[i * block_rc - 1] +
                                 state_old[(i + 1) * block_rc - 3] +
                                 state_old[(i + 1) * block_rc - 1] +
                                 state_old[(i + 2) * block_rc - 3] +
                                 state_old[(i + 2) * block_rc - 2] +
                                 state_old[(i + 2) * block_rc - 1];

        state[(i + 1) * block_rc - 2] =
                (sum_of_r_edge == 3) | ((sum_of_r_edge == 2) & state_old[(i + 1) * block_rc - 2]);
    }
    return;
}

void calculate_next_gen(u_int8_t *state, u_int8_t *state_old) {
    //i = row, j = column

    // edges and corners first
    edge_maker(state, state_old);
    // send everything
    // middle; two because: -1 overlap, -1 edge. Could be calculated here, but for performance done before.
    for (int i = 2; i < block_rc - 2; i++) {
        for (int j = 2; j < block_rc - 2; j++) {
            //count up a number (8)
            u_int8_t sum_of_8 = state_old[(i - 1) * block_rc + (j - 1)] +
                                state_old[(i - 1) * block_rc + j] +
                                state_old[(i - 1) * block_rc + (j + 1)] +
                                state_old[i * block_rc + (j - 1)] +
                                state_old[i * block_rc + (j + 1)] +
                                state_old[(i + 1) * block_rc + (j - 1)] +
                                state_old[(i + 1) * block_rc + j] +
                                state_old[(i + 1) * block_rc + (j + 1)];
            state[i * block_rc + j] = (sum_of_8 == 3) | ((sum_of_8 == 2) & state_old[i * block_rc + j]);
        }
    }
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
        if (columns != rows) {
            printf("Warning: Your field is not square.\n");
            exit(2);
        }
        printf("Starting now...\n");
    }
    // calc number of blocks per side
    blocks_per_side = (u_int32_t) sqrt((double) cluster);
    block_rc = rows / blocks_per_side + 2;
    // position in neighbour_matrix
    rank_index = (rank / blocks_per_side + 1) * (blocks_per_side + 2) + (rank % blocks_per_side + 1);
    u_int32_t *neighbour_matrix = (u_int32_t *) malloc(
            (blocks_per_side + 2) * (blocks_per_side + 2) * sizeof(u_int32_t));
    // initializing states and pointers
    u_int8_t *state_1 = (u_int8_t *) malloc(block_rc * block_rc * sizeof(u_int8_t));
    u_int8_t *state_2 = (u_int8_t *) malloc(block_rc * block_rc * sizeof(u_int8_t));
    u_int8_t *state_in = state_1;
    u_int8_t *state_out = state_2;
    u_int8_t *state_tmp = NULL;
    // starting clock
    clock_t t;
    double time_rand;
    double time_calc = 0;
    double time_out = 0;
    // filling with random numbers
    t = clock();
    field_initializer(state_1);
    t = clock() - t;
    time_rand = ((double) t) / CLOCKS_PER_SEC; // in seconds
    // write random pattern as -1 file
    if (produce_output) {
        t = clock();
        write_pbm_file(state_in, -1);
        t = clock() - t;
        time_out += ((double) t) / CLOCKS_PER_SEC;
    }
    //calculation
    for (int i = 0; i < repetitions; i++) {
        t = clock();
        calculate_next_gen(state_out, state_in);
        t = clock() - t;
        time_calc += ((double) t) / CLOCKS_PER_SEC;
        state_tmp = state_in;
        state_in = state_out;
        state_out = state_tmp;
        if (show_progress && rank == 0) {
            double percentage = 100.0 * (i + 1) / repetitions;
            printf("%.1lf%c\n", percentage, 37);
        }
        if (produce_output) {
            // wait for all processes to finish
            MPI_Barrier(MPI_COMM_WORLD);
            if (rank == 0) {
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