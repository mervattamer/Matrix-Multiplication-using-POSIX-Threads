#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

FILE *fp;

struct arg_struct
{
    int row_1;
    int col_1;
    int row_2;
    int col_2;
    int row;
    int col;
    int **m1;
    int **m2;
    int **res;
};

bool read_input_matrices(char *fname, struct arg_struct *args)
{
//printf(fname);
    fp = fopen(fname, "r");
    if (fp == NULL)
    {
        printf("file does not exist");
        return false;
    }

    int r,c, r2,c2;
    int** mat1;
    int** mat2;
    fscanf(fp, "%d %d", &r, &c);
    args->row_1 = r;
    args-> col_1 = c;
    mat1 = (int**) malloc(r * sizeof(int*));
    for(int i=0; i<r; i++)
    {
        mat1[i] = (int*) malloc(c* sizeof(int));
        for(int j = 0; j<c; j++)
            fscanf(fp, "%d", &mat1[i][j]);
    }
    args -> m1 = mat1;



    // matrix 2
    fscanf(fp, "%d %d", &r2, &c2);
    args->row_2 = r2;
    args-> col_2 = c2;
    mat2 = (int**) malloc(r2 * sizeof(int*));
    for(int i=0; i<r2; i++)
    {
        mat2[i] = (int*) malloc(c2* sizeof(int));
        for(int j = 0; j<c2; j++)
            fscanf(fp, "%d", &mat2[i][j]);
    }
    args -> m2 = mat2;


    fclose(fp);
    return true;
}

void* matrix_row(void *args)
{
  struct arg_struct *args_elem = (struct arg_struct *) args;
  int cols2 = args_elem->col_2;
  int cols1 = args_elem -> col_1;
  int rows = args_elem -> row;
  
  for(int i= 0; i<cols2; i++ )
  {
    args_elem->res[rows][i] = 0;
    for(int j = 0; j<cols1; j++)
    {
      args_elem->res[rows][i] += args_elem->m1[rows][j] * args_elem->m2[j][i];
    }
  }
  
  free(args_elem);
  pthread_exit(NULL);
}

void* matrix_element(void* args)
{
    struct arg_struct *args_elem = (struct arg_struct *) args;
    int rows =args_elem->row_2;
    int r = args_elem->row;
    int c = args_elem-> col;


    args_elem->res[r][c] = 0;
    for (int i = 0; i < rows; i++)
    {
        //printf("Computing element: m1[%d][%d] * m2[%d][%d]\n", r, i, i, c);

        args_elem->res[r][c] += args_elem->m1[r][i] * args_elem->m2[i][c];
    }
    
    //printf("Completed for row=%d, col=%d\n", r, c);
    free(args_elem);
    pthread_exit(NULL);
    
}


int main(int argc, char* argv[])
{

    if(argc !=2)
    {
        printf("insert input file name");
        return 0;
    }
    char *fname = argv[1];
    struct arg_struct *args = malloc(sizeof(struct arg_struct));
    if(read_input_matrices(fname, args) ==true)
    {


//printf("heree");
        int rows = args->row_1;
        int cols = args -> col_2;
        args->row = rows;
        args-> col = cols;
        
        
      int **res = (int **) calloc(rows, sizeof(int *));
        for (int i = 0; i < rows; i++)
        {
            res[i] = (int *) calloc(cols, sizeof(int));
        }
        args -> res = res;
        
        clock_t start, end;
        
        start = clock();
        pthread_t thread_elem[rows][cols];


        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                struct arg_struct *args_elem = malloc(sizeof(struct arg_struct));
                args_elem->row_1 = args->row_1;
                args_elem->row_2 = args->row_2;
                args_elem->col_1 = args-> col_1;
                args_elem ->col_2 = args->col_2;
                args_elem->m1 = args->m1;
                args_elem->m2 = args->m2;
                args_elem-> res = args->res;
                args_elem->row = i;
                args_elem -> col = j;
                if (pthread_create(&thread_elem[i][j], NULL, matrix_element, (void*) args_elem) != 0)
                {
                    perror("pthread_create");
                    exit(1);
                }
            }
            //free(args_elem);
        }

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                pthread_join(thread_elem[i][j], NULL);
            }
        }

        //printf("%d %d\n", rows, cols);
        for (int i = 0; i <rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                printf("%d ", args->res[i][j]);
            }
            printf("\n");
        }

        end = clock();
        printf("END1 %.6f\n",(double)(end-start)/CLOCKS_PER_SEC);
        
        
        args -> res = res;
        
        start = clock();
        pthread_t thread_row[rows];
        
        for(int i =0; i<rows; i++)
        {
                struct arg_struct *args_elem = malloc(sizeof(struct arg_struct));
                args_elem->row_1 = args->row_1;
                args_elem->row_2 = args->row_2;
                args_elem->col_1 = args-> col_1;
                args_elem ->col_2 = args->col_2;
                args_elem->m1 = args->m1;
                args_elem->m2 = args->m2;
                args_elem-> res = args->res;
                args_elem->row = i;
                args_elem -> col = 0;
                
                if (pthread_create(&thread_row[i], NULL, matrix_row, (void*) args_elem) != 0)
                {
                    perror("pthread_create");
                    exit(1);
                }
        }
        
        for (int i = 0; i < rows; i++)
        {
            pthread_join(thread_row[i], NULL);
            
        }
        
        for (int i = 0; i <rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                printf("%d ", args->res[i][j]);
            }
            printf("\n");
        }
        
        end = clock();
        printf("END2 %.6f\n",(double)(end-start)/CLOCKS_PER_SEC);



        for (int i = 0; i < rows; i++)
            free(res[i]);
        free(res);
        free(args);
    }
    return 0;
}
