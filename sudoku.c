/**
 * @file SudokuSolver.c
 * @author [Fang Wu]
 * @brief Sudoku Puzzle Solver with Threading
 *
 * This file contains the implementation of a Sudoku puzzle solver. The solver is designed to work with Sudoku puzzles of any size, 
 * specifically optimized for standard sizes like 9x9. It utilizes multi-threading to concurrently validate rows, columns, 
 * and subgrids within the puzzle, ensuring that each number appears exactly once in each row, column, and subgrid as per Sudoku rules.
 * 
 * Key features include:
 * - Validation of Sudoku puzzles for completeness and validity.
 * - Filling in  missing numbers in rows, columns, and subgrids.
 * - Multi-threaded approach to concurrently check rows, columns, and subgrids.
 * 
 * Note: This solver is capable of solving simple cases where only one number is missing in a row, column, or subgrid.
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity


typedef struct {
    int row;
    int column;
    int psize;
    int **grid;
    bool *complete;
    bool *invalid;
} parameters;

/**
 * @brief Fills in cells with a missing number in a Sudoku puzzle.
 *
 * @purpose This function scans a Sudoku puzzle grid for empty cells (denoted by 0) and 
 *          attempts to fill them if only one valid number can be placed according to 
 *          Sudoku rules. 
 * 
 * @pre-condition The Sudoku grid is initialized and provided. The grid size (psize) 
 *                is defined, and the grid starts with 1-based indexing where grid[1][1] 
 *                is the first cell. 
 * @post-condition For cells where only one number is possible, that number is placed
 *                 in the cell. The function directly modifies the grid to fill in these numbers.
 *
 * @param grid A pointer to a 2D array representing the Sudoku puzzle grid.
 * @param psize The size of the puzzle, indicating a psize x psize grid.
 */
void solveMissingNumber(int **grid, int psize) {
    int row, col, num, missingNum;
    // Array to track possible numbers 
    bool possible[psize + 1]; 
    
    for (row = 1; row <= psize; ++row) 
    {
        for (col = 1; col <= psize; ++col) 
        {
            if (grid[row][col] == 0) { // If the cell is empty
                memset(possible, true, sizeof(possible)); // Initialize all numbers as possible
                
                // Check the row and column for the number
                for (num = 1; num <= psize; ++num) 
                {
                    if (grid[row][num] > 0) 
                    {
                        possible[grid[row][num]] = false; // Mark number as not possible
                    }
                    if (grid[num][col] > 0) 
                    {
                        possible[grid[num][col]] = false; // Mark number as not possible
                    }
                }

                // Count possible numbers 
                int countPossible = 0;
                for (num = 1; num <= psize; ++num) {
                    if (possible[num]) 
                    {
                        ++countPossible;
                        missingNum = num; 
                    }
                }

                // If only one number is possible, fill it in
                if (countPossible == 1) 
                {
                    grid[row][col] = missingNum;
                }
            }
        }
    }
}

/**
 * @brief Checks a  column in a Sudoku puzzle for validity.
 *
 * @purpose This function is tasked with validating a  column within a Sudoku puzzle.
 *          It verifies that each number in the column is unique (no duplicates) and marks
 *          the puzzle as invalid if any duplicates are found. It also checks if the column
 *          is incomplete by finding 0. 
 * 
 * @pre-condition The `parameters` struct is properly initialized
 *                with the column to check, the puzzle size, and pointers to flags indicating the
 *                puzzle's validity and completeness.
 * 
 * @post-condition The column is checked for duplicates and completeness. If a duplicate is found,
 *                 the puzzle's invalid flag is set to true. If an unfilled cell (zero) is found,
 *                 the puzzle's complete flag is set to false. 
 *
 * @param params A void pointer to a `parameters` struct containing the column to be checked, 
 *               the size of the Sudoku puzzle, and pointers to flags for marking the puzzle as 
 *               invalid or incomplete.
 */
void *checkCol(void *params) 
{
    parameters *data = (parameters *)params; // Corrected to avoid unnecessary malloc
    int size = data->psize;
    // Array to track numbers seen in the column
    bool seen[size + 1]; 
    bool rowComplete = true; 

    // Initialize all elements in seen to 0 = false
    memset(seen, 0, sizeof(seen));
    for (int row = 1; row <= size; row++) 
    {
        int val = data->grid[row][data->column];

        if (val <= 0) {
            rowComplete = false; 
            continue; 
        }
        if (seen[val]) 
        {
            *(data->invalid) = true;
            break; 
        }
        seen[val] = true; 
    }

    if (!rowComplete) 
    {
        *(data->complete) = false;
    }
    pthread_exit(NULL); 
}

/**
 * @brief Checks a  column in a Sudoku puzzle for validity.
 *
 * @purpose This function is tasked with validating a  column within a Sudoku puzzle.
 *          It verifies that each number in the row is unique (no duplicates) and marks
 *          the puzzle as invalid if any duplicates are found. It also checks if the column
 *          is incomplete by finding 0. 
 * 
 * @pre-condition The `parameters` struct is properly initialized
 *                with the row to check, the puzzle size, and pointers to flags indicating the
 *                puzzle's validity and completeness.
 * 
 * @post-condition The row is checked for duplicates and completeness. If a duplicate is found,
 *                 the puzzle's invalid flag is set to true. If an unfilled cell (zero) is found,
 *                 the puzzle's complete flag is set to false. 
 *
 * @param params A void pointer to a `parameters` struct containing the row to be checked, 
 *               the size of the Sudoku puzzle, and pointers to flags for marking the puzzle as 
 *               invalid or incomplete.
 */
void *checkRow(void *params) {
    parameters *data = (parameters *)params; 
    int size = data->psize;
    // Array to track numbers seen in the row
    bool seen[size + 1]; 
    bool rowComplete = true;

    // Initialize all elements in seen to 0 = false
    memset(seen, 0, sizeof(seen));

    for (int col = 1; col <= size; col++) 
    {
        int val = data->grid[data->row][col]; 

        if (val <= 0) 
        {
            rowComplete = false; 
            continue; 
        }
        if (seen[val]) 
        {
            *(data->invalid) = true;
            break; 
        }
        seen[val] = true;
    }

    if (!rowComplete) 
    {
        *(data->complete) = false;
    }
    pthread_exit(NULL);
}

/**
 * @brief Validates a specific subgrid within a Sudoku puzzle.
 *
 * @purpose To ensure each number within a specified subgrid of the Sudoku puzzle appears only once,
 *          contributing to the overall puzzle's validity. 
 * 
 * @pre-condition The `parameters` struct is properly initialized
 *                with the row to check, the puzzle size, and pointers to flags indicating the
 *                puzzle's validity and completeness.
 * 
 * @post-condition Analyzes the specified subgrid for number uniqueness. If a duplicate number is found within
 *                 the subgrid, the puzzle's invalid flag is set to true.
 *
 *  @param params A void pointer to a `parameters` struct containing the row to be checked, 
 *               the size of the Sudoku puzzle, and pointers to flags for marking the puzzle as 
 *               invalid or incomplete.
 */
void *checkSubgrid(void *params) {
    parameters *data = (parameters *)params; // Use passed parameters directly
    int subGridSize = sqrt(data->psize);
    bool seen[data->psize + 1];
    memset(seen, 0, sizeof(seen));
    int incrementC = 0;
    int incrementR = 0;
    

    for (int i = 1; i < subGridSize + 1; i++) 
    {
        incrementC = 0;
        for (int j = 1; j < subGridSize + 1; j++) 
        {
            int val = data->grid[data->row + incrementR][data->column + incrementC];
            if (seen[val]) 
            {
                *(data->invalid) = true;
                break;
            }
            seen[val] = true;
            incrementC++;
        }
        incrementR++;
    }
    
    pthread_exit(NULL);
}

/**
 * @brief Validates the entire Sudoku puzzle, checking rows, columns, and subgrids for completeness and validity.
 *
 * @purpose This function orchestrates a comprehensive validation of the Sudoku puzzle by spawning separate threads
 *          to independently validate all rows, columns, and subgrids (for 9x9 puzzles).
 *
 * @pre-condition The Sudoku puzzle is initialized and loaded into a 2D grid with indices starting from 1 to accommodate
 *                the puzzle format where grid[1][1] is the first cell. The puzzle size (`psize`) is known, and `complete`
 *                and `valid` flags are initialized but not set.
 *
 * @post-condition After executing, the puzzle's `complete` flag is set to false if any cell is unfilled (contains 0),
 *                 and the `valid` flag is set to false if any rule violations are found 
 *
 * @param psize The size of the puzzle, determining how many rows and columns (and possibly subgrids) need to be checked.
 * @param grid A pointer to the 2D array representing the Sudoku puzzle.
 * @param complete A pointer to a boolean flag indicating whether the puzzle is completely filled.
 * @param valid A pointer to a boolean flag indicating whether the puzzle adheres to all Sudoku rules.
 */
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
    int sqrtPsize = (int)sqrt(psize);
    bool flag;
    
    if ((sqrtPsize * sqrtPsize == psize) && (sqrtPsize > 1)) {flag = true;} 
    else {flag = false;}

    int subGridSize = sqrtPsize;
    int totalThreads = psize * 2 + (flag ? psize : 0);
    pthread_t threads[totalThreads];
    parameters *params[totalThreads];
    int threadIndex = 0;
    bool listInvalid[totalThreads];
    bool listComplete[totalThreads];

    for (int i = 0; i < totalThreads; i++) {
        listInvalid[i] = false; 
        listComplete[i] = true;
    }

    // Allocate and initialize parameters for each thread
    // Grid starts from row-1 col-1: grid[1][1]
    for (int i = 1; i <= psize; i++) {
        // Check Rows 
        params[threadIndex] = (parameters *)malloc(sizeof(parameters));
        *params[threadIndex] = (parameters){
            .row = i, 
            .column = 1, 
            .psize = psize,
            .grid = grid,
            .complete = &listComplete[threadIndex],
            .invalid = &listInvalid[threadIndex]
        };
        pthread_create(&threads[threadIndex], NULL, checkRow, params[threadIndex]);
        threadIndex++;

        // Check Columns 
        params[threadIndex] = (parameters *)malloc(sizeof(parameters));
        *params[threadIndex] = (parameters){
            .row = 1, 
            .column = i, 
            .psize = psize,
            .grid = grid,
            .complete = &listComplete[threadIndex],
            .invalid = &listInvalid[threadIndex]
        };
        pthread_create(&threads[threadIndex], NULL, checkCol, params[threadIndex]);
        threadIndex++;
    }

    if (flag)
    {
        for (int row = 1; row <= psize; row += subGridSize) {
            for (int col = 1; col <= psize; col += subGridSize) {
                params[threadIndex] = (parameters *)malloc(sizeof(parameters));
                *params[threadIndex] = (parameters){
                    .row = row, 
                    .column = col, 
                    .psize = psize,
                    .grid = grid,
                    .complete = &listComplete[threadIndex],
                    .invalid = &listInvalid[threadIndex]
                };
                pthread_create(&threads[threadIndex], NULL, checkSubgrid, params[threadIndex]);
                threadIndex++;
            }
        }
    }
    
    *complete = true;
    *valid = true;
    for (int i = 0; i < totalThreads; i++) {
        pthread_join(threads[i], NULL);
        if (*params[i]->invalid == true) {
            *valid = false;
        }
        if (*params[i]->complete == false) {
            *complete = false;
        }
       
    }
}
// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

/**
 * @brief Main entry point for the Sudoku puzzle solver.
 *
 * @purpose This function serves as the starting point for the application. It reads a Sudoku puzzle from a file,
 *          verifies its completeness and validity, attempts to solve it if incomplete, and finally prints the
 *          outcome. 
 * 
 * @pre-condition Expects a command line argument specifying the path to a text file containing the Sudoku puzzle.
 *                The puzzle format is specified as the first line indicating the size (N x N) followed by N lines
 *                of N numbers each, representing the Sudoku grid.
 * 
 * @post-condition The program reads the puzzle, checks for its completeness and validity using `checkPuzzle`,
 *                 attempts to fill in simple missing cases with `solveMissingNumber` if the puzzle is incomplete,
 *                 and prints the puzzle's status (complete and valid or not) along with the final grid state.
 *                 If the puzzle is solved (either was already complete or made complete), it outputs that the puzzle is
 *                 complete and whether it is valid. For incomplete puzzles, it attempts to solve and shows the solve attempt results.
 *
 * @param argc The number of command line arguments.
 * @param argv The command line arguments, where argv[1] is expected to be the path to the puzzle file.
 * @return Returns `EXIT_SUCCESS` if the program runs to completion without errors, or `EXIT_FAILURE` if there are issues
 *         
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ./sudoku puzzle.txt\n");
        return EXIT_FAILURE;
    }
    int **grid = NULL;
    int sudokuSize = readSudokuPuzzle(argv[1], &grid);
    bool valid = false;
    bool complete = false;

    checkPuzzle(sudokuSize, grid, &complete, &valid);
    // Output results
    if (complete) {
        printf("Complete puzzle? true\n");
        if (valid) {
            printf("Valid puzzle? true\n");
        } else {
            printf("Valid puzzle? false\n");
        }
    } else {
        printf("Complete puzzle? false.\n");
        printSudokuPuzzle(sudokuSize, grid);
        printf("Solve result: \n");
        //Normally iterate 2-3 times is enough... but just to be sure I set to 5 times... xd
        for (int i = 0; i < 5; i++)
        {
            solveMissingNumber(grid, sudokuSize);
        }
    }

    printSudokuPuzzle(sudokuSize, grid);
    deleteSudokuPuzzle(sudokuSize, grid);
    return EXIT_SUCCESS;
}
