
// Sokoban
// Written by YOUR-NAME (YOUR-ZID),
// on 03-07-2024
// Create Sokoban game
#include <stdio.h>
#include <string.h>
//Directive for utility functions
#include <stdlib.h>
// Definning rows and columns
#define ROWS 10
#define COLS 10
#define MAX_UNDO 1500
//Tile values
enum base { 
    NONE,
    WALL,
    STORAGE
};
// Single tile onthe board
struct tile {
    enum base base;
    int box;
};

struct game_state {
    struct tile board[ROWS][COLS];
    int player_row;
    int player_col;
};

// Function prototypes
void init_board(struct tile board[ROWS][COLS]);
void print_line(void);
void print_title(void);
void print_board(struct tile board[ROWS][COLS], int player_row, int player_col);
//Stage 2 moving the player
void move_player(char move, struct tile board[ROWS][COLS], int *player_row, int *player_col);
//Stage 3 checking for win condition
int check_win(struct tile board[ROWS][COLS]);
// Restetting and saving game state
void reset_board(struct tile board[ROWS][COLS], int *player_row, int *player_col);
void save_state(struct game_state *stack, int *top, struct tile board[ROWS][COLS], int player_row, int player_col);
//stage 4 undo command
void undo_move(struct game_state *stack, int *top, struct tile board[ROWS][COLS], int *player_row, int *player_col);
int load_level(const char *filename, struct tile board[ROWS][COLS], int *player_row, int *player_col);

int main(void) {
    struct tile board[ROWS][COLS];
    struct game_state undo_stack[MAX_UNDO];
    int undo_top = -1;
// Creation of levels
    char filename[100];
    int player_row, player_col;
    int level_number = 1;

    while (1) {
        sprintf(filename, "level%d.txt", level_number);

        //Loading level
        if (load_level(filename, board, &player_row, &player_col) != 0) {
            printf("Failed to load level %s\n", filename);
            break; 
        }
    }
        //Looping Current level
        char move;
        while (1) {
            print_board(board, player_row, player_col);
            if (check_win(board)) {
                printf("You win!\n");
                break;
            }
            printf("Enter move (a/b/c/d) or r to reset or u to undo: ");
            scanf(" %c", &move);
            if (move == 'r') {
                reset_board(board, &player_row, &player_col);
                undo_top = -1;
            } else if (move == 'u') {
                undo_move(undo_stack, &undo_top, board, &player_row, &player_col);
            } else {
                save_state(undo_stack, &undo_top, board, player_row, player_col);
                move_player(move, board, &player_row, &player_col);
            }

        // Level increment
        level_number++;

        // Quiting level
        printf("Press 'Enter' to load the next level or 'q' to quit: ");
        getchar(); 
        char choice = getchar();
        if (choice == 'q') {
            printf("Exiting...\n");
            break;
        }
    }

    return 0;
}

int load_level(const char *filename, struct tile board[ROWS][COLS], int *player_row, int *player_col) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: could not open file %s\n", filename);
        return -1;
    }

    init_board(board);
    char type;
    int row, col;
    while (fscanf(file, " %c %d %d", &type, &row, &col) == 3) {
        switch (type) {
            case 'P':
                *player_row = row;
                *player_col = col;
                break;
            case 'W':
                board[row][col].base = WALL;
                break;
            case 'S':
                board[row][col].base = STORAGE;
                break;
            case 'B':
                board[row][col].box = 1;
                break;
            default:
                printf("Error: unknown type %c in file %s\n", type, filename);
                fclose(file);
                return -1;
        }
    }
    fclose(file);
    return 0;
}

void move_player(char move, struct tile board[ROWS][COLS], int *player_row, int *player_col) {
    int new_row = *player_row;
    int new_col = *player_col;

    if (move == 'a') new_row--;
    else if (move == 'b') new_row++;
    else if (move == 'c') new_col--;
    else if (move == 'd') new_col++;
// Checking if player can move to new position
    if (new_row >= 0 && new_row < ROWS && new_col >= 0 && new_col < COLS) {
        if (board[new_row][new_col].base != WALL) {
            if (board[new_row][new_col].box) {
                int box_new_row = new_row + (new_row - *player_row);
                int box_new_col = new_col + (new_col - *player_col);
                if (box_new_row >= 0 && box_new_row < ROWS && box_new_col >= 0 && box_new_col < COLS
                    && board[box_new_row][box_new_col].base != WALL
                    && !board[box_new_row][box_new_col].box) {
                    board[new_row][new_col].box = 0;
                    board[box_new_row][box_new_col].box = 1;
                    *player_row = new_row;
                    *player_col = new_col;
                }
            } else {
                *player_row = new_row;
                *player_col = new_col;
            }
        }
    }
}
//Checking for win condition
int check_win(struct tile board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j].box && board[i][j].base != STORAGE) {
                return 0;
            }
        }
    }
    return 1;
}

void reset_board(struct tile board[ROWS][COLS], int *player_row, int *player_col) {
    init_board(board);
    *player_row = 1;
    *player_col = 1;
}

void save_state(struct game_state *stack, int *top, struct tile board[ROWS][COLS], int player_row, int player_col) {
    if (*top < MAX_UNDO - 1) {
        *top += 1;
        memcpy(stack[*top].board, board, sizeof(struct tile) * ROWS * COLS);
        stack[*top].player_row = player_row;
        stack[*top].player_col = player_col;
    }
}

void undo_move(struct game_state *stack, int *top, struct tile board[ROWS][COLS], int *player_row, int *player_col) {
    if (*top >= 0) {
        memcpy(board, stack[*top].board, sizeof(struct tile) * ROWS * COLS);
        *player_row = stack[*top].player_row;
        *player_col = stack[*top].player_col;
        *top -= 1;
    }
}
// Initialising board to initial values
void init_board(struct tile board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1) {
                board[i][j].base = WALL;
            } else {
                board[i][j].base = NONE;
            }
            board[i][j].box = 0;
        }
    }
}

void print_line(void) {
    for (int i = 0; i < COLS * 4 + 1; i++) {
        printf("-");
    }
    printf("\n");
}
// Printing game title
void print_title(void) {
    print_line();
    char *title = "S O K O B A N";
    int len = COLS * 4 + 1;
    int n_white = len - strlen(title) - 2;
    printf("|");
    for (int i = 0; i < n_white / 2; i++) {
        printf(" ");
    }
    printf("%s", title);
    for (int i = 0; i < (n_white + 1) / 2; i++) {
        printf(" ");
    }
    printf("|\n");
}
// Printing current state of game
void print_board(struct tile board[ROWS][COLS], int player_row, int player_col) {
    print_title();
    for (int i = 0; i < ROWS; i++) {
        print_line();
        for (int j = 0; j < COLS; j++) {
            printf("|");

            struct tile curr = board[i][j];
            if (i == player_row && j == player_col) {
                printf("^_^");
            } else if (curr.base == WALL) {
                printf("===");
            } else if (curr.base == STORAGE) {
                printf(" * ");
            } else if (curr.box) {
                printf("[ ]");
            } else {
                printf("   ");
            }
        }
        printf("|\n");
    }
    print_line();
}
