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
static u_int32_t rank_index, block_row, block_col, blocks_per_row, blocks_per_col, neighbour_row, neighbour_col;

// getopt
static struct option long_options[] =
        {
                {"help",        optional_argument, NULL, 'h'},
                {"output",      optional_argument, NULL, 'o'},
                {"progress",    optional_argument, NULL, 'p'},
                {"repetitions", optional_argument, NULL, 'R'},
                {"size",        optional_argument, NULL, 's'},
                {NULL,          0,                 NULL, 0}};

void field_initializer(u_int8_t *state, u_int32_t *neighbour_matrix) {
    //fills fields with random numbers 0 = dead, 1 = alive
    unsigned seed = time(0) + rank;
    for (int i = 1; i < block_row - 1; i++) {
        for (int j = 1; j < block_col - 1; j++) {
            state[i * block_col + j] = rand_r(&seed) % 2;
        }
    }
    // send everything
    MPI_Status status[16];
    MPI_Request request[16];
    // left and right row
    MPI_Datatype mpi_col;
    MPI_Type_vector(block_row - 2, 1, block_col, MPI_UINT8_T, &mpi_col);
    MPI_Type_commit(&mpi_col);
    //edges
    //bottom
    MPI_Isend(&state[(block_row - 2) * block_col + 1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index + neighbour_row], 0,
              MPI_COMM_WORLD, &request[0]);
    MPI_Irecv(&state[1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index - neighbour_row], 0, MPI_COMM_WORLD, &request[1]);
    //top
    MPI_Isend(&state[block_col + 1], block_col - 2, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row], 1,
              MPI_COMM_WORLD, &request[2]);
    MPI_Irecv(&state[(block_row - 1) * block_col + 1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index + neighbour_row], 1, MPI_COMM_WORLD, &request[3]);
    //left
    MPI_Isend(&state[block_col + 1], 1, mpi_col, neighbour_matrix[rank_index - 1], 6, MPI_COMM_WORLD, &request[12]);
    MPI_Irecv(&state[2 * block_col - 1], 1, mpi_col, neighbour_matrix[rank_index + 1], 6, MPI_COMM_WORLD, &request[13]);
    //right
    MPI_Isend(&state[2 * block_col - 2], 1, mpi_col, neighbour_matrix[rank_index + 1], 7, MPI_COMM_WORLD, &request[14]);
    MPI_Irecv(&state[block_col], 1, mpi_col, neighbour_matrix[rank_index - 1], 7, MPI_COMM_WORLD, &request[15]);
    //corners
    //top left
    MPI_Isend(&state[block_col + 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row - 1], 2,
              MPI_COMM_WORLD, &request[4]);
    MPI_Irecv(&state[block_row * block_col - 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row + 1], 2,
              MPI_COMM_WORLD, &request[5]);
    //top right
    MPI_Isend(&state[2 * block_col - 2], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row + 1], 3,
              MPI_COMM_WORLD, &request[6]);
    MPI_Irecv(&state[(block_row - 1) * block_col], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row - 1], 3,
              MPI_COMM_WORLD, &request[7]);
    //bottom left
    MPI_Isend(&state[(block_row - 2) * block_col + 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row - 1],
              4, MPI_COMM_WORLD, &request[8]);
    MPI_Irecv(&state[block_col - 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row + 1], 4,
              MPI_COMM_WORLD, &request[9]);
    //bottom right
    MPI_Isend(&state[(block_row - 1) * block_col - 2], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row + 1],
              5, MPI_COMM_WORLD, &request[10]);
    MPI_Irecv(&state[0], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row - 1], 5, MPI_COMM_WORLD,
              &request[11]);
    MPI_Waitall(16, request, status);
}

void init_chessboard() {
    u_int32_t exponent = (u_int32_t) log2((double) cluster);
    if (exponent & 1) {
        // ungerade
        blocks_per_col = pow(2, (exponent / 2));
        blocks_per_row = blocks_per_row * 2;
    } else {
        // gerade
        blocks_per_row = blocks_per_col = pow(2, (exponent / 2));
    }
    block_row = rows / blocks_per_row + 2;
    block_col = columns / blocks_per_col + 2;
    neighbour_row = blocks_per_row + 2;
    neighbour_col = blocks_per_col + 2;
    return;
}

void init_neighbour(u_int32_t *neighbour_matrix) {
    for (int i = 1; i < neighbour_row - 1; i++) {
        for (int j = 1; j < neighbour_col - 1; j++) {
            neighbour_matrix[i * neighbour_row + j] = (i - 1) * blocks_per_row + (j - 1);
        }
    }
    for (int i = 1; i < neighbour_row - 1; i++) {
        //left
        neighbour_matrix[i * neighbour_col] = neighbour_matrix[(i + 1) * neighbour_col - 2];
        //right
        neighbour_matrix[(i + 1) * neighbour_col - 1] = neighbour_matrix[i * neighbour_col + 1];
    }
    for (int i = 1; i < neighbour_col - 1; i++) {
        //top
        neighbour_matrix[i] = neighbour_matrix[(neighbour_row - 2) * neighbour_col + i];
        //bottom
        neighbour_matrix[(neighbour_row - 1) * neighbour_col + i] = neighbour_matrix[neighbour_col + i];
    }
    //top left corner
    neighbour_matrix[0] = neighbour_matrix[(neighbour_row - 1) * neighbour_col - 2];
    //top right corner
    neighbour_matrix[neighbour_col - 1] = neighbour_matrix[(neighbour_row - 2) * neighbour_col + 1];
    // bottom left
    neighbour_matrix[(neighbour_row - 1) * neighbour_col] = neighbour_matrix[neighbour_col];
    //bottom right
    neighbour_matrix[neighbour_row * neighbour_col - 1] = neighbour_matrix[neighbour_col + 1];
    return;
}

void edge_maker(u_int8_t *state, u_int8_t *state_old) {
    // top and bottom
    for (int i = 2; i < block_col - 2; i++) {
        u_int8_t sum_of_t_edge = state_old[i - 1] +
                                 state_old[i] +
                                 state_old[i + 1] +
                                 state_old[block_col + i - 1] +
                                 state_old[block_col + i + 1] +
                                 state_old[2 * block_col + i - 1] +
                                 state_old[2 * block_col + i] +
                                 state_old[2 * block_col + i + 1];
        state[block_col + i] = (sum_of_t_edge == 3) | ((sum_of_t_edge == 2) & state_old[block_col + i]);

        u_int8_t sum_of_b_edge = state_old[(block_col - 3) * block_col + i - 1] +
                                 state_old[(block_col - 3) * block_col + i] +
                                 state_old[(block_col - 3) * block_col + i + 1] +
                                 state_old[(block_col - 2) * block_col + i - 1] +
                                 state_old[(block_col - 2) * block_col + i + 1] +
                                 state_old[(block_col - 1) * block_col + i - 1] +
                                 state_old[(block_col - 1) * block_col + i] +
                                 state_old[(block_col - 1) * block_col + i + 1];
        state[(block_col - 2) * block_col + i] =
                (sum_of_b_edge == 3) | ((sum_of_b_edge == 2) & state_old[(block_col - 2) * block_col + i]);
    }
    //+ corners
    for (int i = 1; i < block_row - 1; i++) {
        u_int8_t sum_of_l_edge = state_old[(i - 1) * block_col] +
                                 state_old[(i - 1) * block_col + 1] +
                                 state_old[(i - 1) * block_col + 2] +
                                 state_old[i * block_col] +
                                 state_old[i * block_col + 2] +
                                 state_old[(i + 1) * block_col] +
                                 state_old[(i + 1) * block_col + 1] +
                                 state_old[(i + 1) * block_col + 2];
        state[i * block_col + 1] = (sum_of_l_edge == 3) | ((sum_of_l_edge == 2) & state_old[i * block_col + 1]);

        u_int8_t sum_of_r_edge = state_old[i * block_col - 3] +
                                 state_old[i * block_col - 2] +
                                 state_old[i * block_col - 1] +
                                 state_old[(i + 1) * block_col - 3] +
                                 state_old[(i + 1) * block_col - 1] +
                                 state_old[(i + 2) * block_col - 3] +
                                 state_old[(i + 2) * block_col - 2] +
                                 state_old[(i + 2) * block_col - 1];

        state[(i + 1) * block_col - 2] =
                (sum_of_r_edge == 3) | ((sum_of_r_edge == 2) & state_old[(i + 1) * block_col - 2]);
    }
    return;
}

void calculate_next_gen(u_int8_t *state, u_int8_t *state_old, u_int32_t *neighbour_matrix) {
    //i = row, j = column
    // edges and corners first
    edge_maker(state, state_old);

    // send everything
    MPI_Status status[16];
    MPI_Request request[16];
    // left and right row
    MPI_Datatype mpi_col;
    MPI_Type_vector(block_row - 2, 1, block_col, MPI_UINT8_T, &mpi_col);
    MPI_Type_commit(&mpi_col);
    //edges
    //bottom
    MPI_Isend(&state[(block_row - 2) * block_col + 1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index + neighbour_row], 0,
              MPI_COMM_WORLD, &request[0]);
    MPI_Irecv(&state[1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index - neighbour_row], 0, MPI_COMM_WORLD, &request[1]);
    //top
    MPI_Isend(&state[block_col + 1], block_col - 2, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row], 1,
              MPI_COMM_WORLD, &request[2]);
    MPI_Irecv(&state[(block_row - 1) * block_col + 1], block_col - 2, MPI_UINT8_T,
              neighbour_matrix[rank_index + neighbour_row], 1, MPI_COMM_WORLD, &request[3]);
    //left
    MPI_Isend(&state[block_col + 1], 1, mpi_col, neighbour_matrix[rank_index - 1], 6, MPI_COMM_WORLD, &request[12]);
    MPI_Irecv(&state[2 * block_col - 1], 1, mpi_col, neighbour_matrix[rank_index + 1], 6, MPI_COMM_WORLD, &request[13]);
    //right
    MPI_Isend(&state[2 * block_col - 2], 1, mpi_col, neighbour_matrix[rank_index + 1], 7, MPI_COMM_WORLD, &request[14]);
    MPI_Irecv(&state[block_col], 1, mpi_col, neighbour_matrix[rank_index - 1], 7, MPI_COMM_WORLD, &request[15]);
    //corners
    //top left
    MPI_Isend(&state[block_col + 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row - 1], 2,
              MPI_COMM_WORLD, &request[4]);
    MPI_Irecv(&state[block_row * block_col - 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row + 1], 2,
              MPI_COMM_WORLD, &request[5]);
    //top right
    MPI_Isend(&state[2 * block_col - 2], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row + 1], 3,
              MPI_COMM_WORLD, &request[6]);
    MPI_Irecv(&state[(block_row - 1) * block_col], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row - 1], 3,
              MPI_COMM_WORLD, &request[7]);
    //bottom left
    MPI_Isend(&state[(block_row - 2) * block_col + 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row - 1],
              4, MPI_COMM_WORLD, &request[8]);
    MPI_Irecv(&state[block_col - 1], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row + 1], 4,
              MPI_COMM_WORLD, &request[9]);
    //bottom right
    MPI_Isend(&state[(block_row - 1) * block_col - 2], 1, MPI_UINT8_T, neighbour_matrix[rank_index + neighbour_row + 1],
              5, MPI_COMM_WORLD, &request[10]);
    MPI_Irecv(&state[0], 1, MPI_UINT8_T, neighbour_matrix[rank_index - neighbour_row - 1], 5, MPI_COMM_WORLD,
              &request[11]);

    // middle; two because: -1 overlap, -1 edge. Could be calculated here, but for performance done before.
    for (int i = 2; i < block_row - 2; i++) {
        for (int j = 2; j < block_col - 2; j++) {
            //count up a number (8)
            u_int8_t sum_of_8 = state_old[(i - 1) * block_col + (j - 1)] +
                                state_old[(i - 1) * block_col + j] +
                                state_old[(i - 1) * block_col + (j + 1)] +
                                state_old[i * block_col + (j - 1)] +
                                state_old[i * block_col + (j + 1)] +
                                state_old[(i + 1) * block_col + (j - 1)] +
                                state_old[(i + 1) * block_col + j] +
                                state_old[(i + 1) * block_col + (j + 1)];
            state[i * block_col + j] = (sum_of_8 == 3) | ((sum_of_8 == 2) & state_old[i * block_col + j]);
        }
    }
    MPI_Waitall(16, request, status);
    return;
}

void write_pbm_file(u_int8_t *state, int i) {
    FILE *fptr;
    char new_filename[65];
    sprintf(new_filename, "%s%06d_%d.pbm", output_fname, i, rank);
    fptr = fopen(new_filename, "w");
    fprintf(fptr, "P1\n");
    fprintf(fptr, "# This is the %06d result. Have fun :)\n", i);
    fprintf(fptr, "%lu %lu\n", block_col, block_row);
    for (int i = 0; i < block_row; i++) {
        for (int j = 0; j < block_col; j++) {
            fprintf(fptr, "%d ", state[i * block_col + j]);
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
    // init blocks
    init_chessboard();
    // welcome information
    if (rank == 0) {
        printf("Welcome to the MPI game of life!\n");
        printf("We are running with %d ranks\n", cluster);
        printf("Game size: Columns: %lu, Rows: %lu.\n", columns, rows);
        printf("Blocks: Columns: %d, Rows: %d.\n", blocks_per_col, blocks_per_row);
        printf("Block size: Columns: %d, Rows: %d.\n", block_col - 2, block_row - 2);
        if (columns != rows) {
            printf("Warning: Your field is not square.\n");
            exit(2);
        }
        printf("Starting now...\n");
    }
    // position in neighbour_matrix
    rank_index = (rank / blocks_per_row + 1) * (blocks_per_row + 2) + (rank % blocks_per_col + 1);
    u_int32_t *neighbour_matrix = (u_int32_t *) malloc((blocks_per_row + 2) * (blocks_per_col + 2) * sizeof(u_int32_t));
    init_neighbour(neighbour_matrix);
    // initializing states and pointers
    u_int8_t *state_1 = (u_int8_t *) malloc(block_row * block_col * sizeof(u_int8_t));
    u_int8_t *state_2 = (u_int8_t *) malloc(block_row * block_col * sizeof(u_int8_t));
    u_int8_t *state_in = state_1;
    u_int8_t *state_out = state_2;
    u_int8_t *state_tmp = NULL;
    // init clock
    struct timespec rand_s, rand_e, calc_s, calc_e, out_s, out_e;
    clockid_t clk_id = CLOCK_MONOTONIC;
    double time_rand, time_calc = 0, time_out = 0;
    // filling with random numbers
    clock_gettime(clk_id, &rand_s);
    field_initializer(state_1, neighbour_matrix);
    clock_gettime(clk_id, &rand_e);
    time_rand += (double) (rand_e.tv_nsec-rand_s.tv_nsec) / 1000000000 + (double) (rand_e.tv_sec-rand_s.tv_sec);
    // write random pattern as -1 file
    if (produce_output) {
        clock_gettime(clk_id, &out_s);
        write_pbm_file(state_in, -1);
        clock_gettime(clk_id, &out_e);
        time_out += (double) (out_e.tv_nsec-out_s.tv_nsec) / 1000000000 + (double) (out_e.tv_sec-out_s.tv_sec);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //calculation
    for (int i = 0; i < repetitions; i++) {
        clock_gettime(clk_id, &calc_s);
        calculate_next_gen(state_out, state_in, neighbour_matrix);
        clock_gettime(clk_id, &calc_e);
        time_calc += (double) (calc_e.tv_nsec-calc_s.tv_nsec) / 1000000000 + (double) (calc_e.tv_sec-calc_s.tv_sec);
        state_tmp = state_in;
        state_in = state_out;
        state_out = state_tmp;
        if (show_progress && rank == 0) {
            double percentage = 100.0 * (i + 1) / repetitions;
            printf("%.1lf%c\n", percentage, 37);
        }
        if (produce_output) {
            clock_gettime(clk_id, &out_s);
            write_pbm_file(state_in, i);
            clock_gettime(clk_id, &out_e);
            time_out += (double) (out_e.tv_nsec-out_s.tv_nsec) / 1000000000 + (double) (out_e.tv_sec-out_s.tv_sec);
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