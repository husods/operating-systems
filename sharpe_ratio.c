#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd_parent_to_child[2], pipefd_child_to_parent[2];
    pid_t pid;

    // pipe olusturma
    if (pipe(pipefd_parent_to_child) == -1 || pipe(pipefd_child_to_parent) == -1) {
        perror("Pipe oluşturma başarısız!");
        return 1;
    }

    // fork olsturma
    pid = fork();
    if (pid == -1) {
        perror("Fork oluşturma başarısız!");
        return 1;
    }

    if (pid == 0) {
        close(pipefd_parent_to_child[1]); // Parent yazma ucu kapatma
        close(pipefd_child_to_parent[0]); // Child okuma ucu kapatma

        double expected, risk_free_rate, risk, sharpe_ratio;

        // parent'dan verileri al ve shape ratio dondur
        while (read(pipefd_parent_to_child[0], &expected, sizeof(double)) > 0) {
            read(pipefd_parent_to_child[0], &risk, sizeof(double));
            read(pipefd_parent_to_child[0], &risk_free_rate, sizeof(double));

            sharpe_ratio = (expected - risk_free_rate) / risk;
            
            write(pipefd_child_to_parent[1], &sharpe_ratio, sizeof(double));
        }

        close(pipefd_parent_to_child[0]);
        close(pipefd_child_to_parent[1]);
        exit(0);
    } 
    
    else {
        close(pipefd_parent_to_child[0]); // Parent okuma ucu kapatma
        close(pipefd_child_to_parent[1]); // Child yazma ucu kapatma

        double expected_p, risk_free_rate_p, risk_p, sharpe_ratio_p, best_sharpe = -1.0;
        int selected_investment = 0, investment_no = 1;

        char line[100];
        while (fgets(line, sizeof(line), stdin) != NULL) {
            if (strcmp(line, "finish") == 0) break;

            sscanf(line, "%lf %lf %lf", &expected_p, &risk_p, &risk_free_rate_p);

            // verileri child'a gonder
            write(pipefd_parent_to_child[1], &expected_p, sizeof(double));
            write(pipefd_parent_to_child[1], &risk_p, sizeof(double));
            write(pipefd_parent_to_child[1], &risk_free_rate_p, sizeof(double));

            // shape ratios'u yazdir
            read(pipefd_child_to_parent[0], &sharpe_ratio_p, sizeof(double));
            printf("%.2f\n" ,sharpe_ratio_p);

            // en iyi yatirim
            if (sharpe_ratio_p > best_sharpe) {
                best_sharpe = sharpe_ratio_p;
                selected_investment = investment_no;
            }
            investment_no++;
        }

        printf("Selected Investment: %d\n", selected_investment);
        
        close(pipefd_parent_to_child[1]);
        close(pipefd_child_to_parent[0]);
        wait(NULL);
    }

    return 0;
}
