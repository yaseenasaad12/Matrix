#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include<pthread.h>
#define MAX_ROWS 20
#define MAX_COLS 20

void read_matrix_from_file(const char *filename, int matrix[MAX_ROWS][MAX_COLS], int *rows, int *cols) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    fscanf(file, "row=%d col=%d", rows, cols);
    
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    
    fclose(file);
}
void write_matrix_to_file(const char *filename, int matrix[MAX_ROWS][MAX_COLS], int rows, int cols) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(file, "row=%d col=%d\n", rows, cols);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
}
struct thread_data{
    int row1;
    int col1;
    int row2;
    int col2;
    int matrix1[MAX_ROWS][MAX_COLS];
    int matrix2[MAX_ROWS][MAX_COLS];
    int result[MAX_ROWS][MAX_COLS];
    int row_count ;
    int col_count ;
    //int row[MAX_ROWS];
    //int col[MAX_COLS];
    //int result2[MAX_ROWS] ;
};

void* multiply1(void* ptr)
{
   struct thread_data *data = (struct thread_data*)ptr;
   for(int i=0 ;i<data->row1;i++)
   {
       for(int j=0;j<data->col2;j++)
       {
           data->result[i][j]=0;
           for(int k=0;k<data->col1;k++)
           {
               data->result[i][j] += data->matrix1[i][k] * data->matrix2[k][j];
           }
       }
   }
}
void* multiply2(void* ptr)
{
    struct thread_data *data = (struct thread_data*)ptr;
    int row_count = data->row_count;

    for (int i = 0; i < data->col2; i++) {
        data->result[row_count][i] = 0;
        for (int j = 0; j < data->col1; j++) {
            data->result[row_count][i] += data->matrix1[row_count][j] * data->matrix2[j][i];
        }
    }
    
}
void* multiply3(void* ptr)
{
    struct thread_data *data = (struct thread_data*)ptr;
    int row_count = data->row_count;
    int col_count = data->col_count;
    data->result[row_count][col_count] = 0;
    for (int i = 0; i < data->col1; i++) {
        data->result[row_count][col_count] += data->matrix1[row_count][i] * data->matrix2[i][col_count];
    }
    
}

int main() {
    int matrix1[MAX_ROWS][MAX_COLS], matrix2[MAX_ROWS][MAX_COLS], result[MAX_ROWS][MAX_COLS];
    int row1, col1, row2, col2;

    // Read matrices from files
    read_matrix_from_file("a.txt", matrix1, &row1, &col1);
    read_matrix_from_file("b.txt", matrix2, &row2, &col2);
    
    if(col1 != row2) {
        printf("Error : Matrix multiplication not possible\n");
        return 0;
    }
    else{

        struct thread_data *data=malloc(sizeof(struct thread_data));
        data->row1=row1;
        data->col1=col1;
        data->row2=row2;
        data->col2=col2;
        for(int i=0;i<row1;i++)
        {
            for(int j=0;j<col1;j++)
            {
                data->matrix1[i][j]=matrix1[i][j];
            }
        }
        for(int i=0;i<row2;i++)
        {
            for(int j=0;j<col2;j++)
            {
                data->matrix2[i][j]=matrix2[i][j];
            }
        }

        //First method (thread per matrix)  
        struct timeval stop, start;
        gettimeofday(&start, NULL);
        pthread_t thread1;
        pthread_create(&thread1, NULL, multiply1 , (void*)data);
        pthread_join(thread1, NULL);

        for(int i=0;i<row1;i++)
        {
            for(int j=0;j<col2;j++)
            {
              result[i][j] = data->result[i][j];
            }
            
        }
        
        write_matrix_to_file("c_per_matrix.txt",result, row1, col2);

        gettimeofday(&stop, NULL);
        printf("thread per matrix:\n");
        printf("Number of threads: 1\n");
        printf("Time taken:\n");
        printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
        printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
        printf("===========================================\n"); 

        //second method (thread per row)
    
        gettimeofday(&start, NULL);
        gettimeofday(&stop, NULL);

        pthread_t thread2[row1];
        for(int i=0 ;i<row1 ;i++)
        {
            data->row_count = i;
            pthread_create(&thread2[i], NULL, multiply2 , (void*)data);

        }

        for (int j = 0; j < row1; j++) {
            pthread_join(thread2[j], NULL);
        }

        for(int i=0;i<row1;i++)
        {
            for(int j=0;j<col2;j++)
            {
              result[i][j] = data->result[i][j];
            }
            
        }


        write_matrix_to_file("c_per_row.txt",result, row1, col2);


        gettimeofday(&stop, NULL);
        printf("thread per row:\n");
        printf("Number of threads: %d\n", row1);
        printf("Time taken:\n");
        printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
        printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
        printf("===========================================\n"); 

        //Third method (thread per element)
        gettimeofday(&start, NULL);
        gettimeofday(&stop, NULL);
        
        pthread_t thread3[row1][col2];
        for(int i=0 ;i<row1 ;i++)
         for(int j=0 ;j<col2 ;j++)
        {
            data->row_count = i;
            data->col_count = j;
            pthread_create(&thread3[i][j], NULL, multiply3 , (void*)data);

        }

        for (int i = 0; i < row1; i++) {
            for (int j = 0; j < col2; j++) {
                pthread_join(thread3[i][j], NULL);
            }
        }

        for(int i=0;i<row1;i++)
        {
            for(int j=0;j<col2;j++)
            {
              result[i][j] = data->result[i][j];
            }
            
        }




        write_matrix_to_file("c_per_element.txt",result, row1, col2);
        gettimeofday(&stop, NULL);
        printf("thread per element:\n");
        printf("Number of threads: %d\n", row1*col2);
        printf("Time taken:\n");
        printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
        printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
        printf("===========================================\n"); 

    }
}
