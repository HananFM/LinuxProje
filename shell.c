/*
* Linux Shell Uygulaması
* 
* Grup Üyeleri:

*[Hanan Haidar-B211210581]
*[Reem Almadhagi-Y245012049]
*[Dania Almassri-B221210569]
*[tasnim abdulhay-B211210585]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// Global değişkenler
pid_t background_pids[100];  // Arkaplan proseslerinin PID'lerini tutmak için
int bg_count = 0;            // Aktif arkaplan prosesi sayısı

// Fonksiyon prototipleri
char **tokenize(char *line);
void execute_command(char **tokens, int background);
void handle_pipe(char **tokens);
void handle_io_redirection(char **tokens);
void cleanup(char **tokens);
void sigchld_handler(int signum);
void execute_background(char **tokens);
void process_commands(char *input);

// Komut çalıştırma hatası için yardımcı fonksiyon
void command_error(const char *cmd, const char *msg) {
    fprintf(stderr, "Hata: '%s' komutu %s\n", cmd, msg);
}

// Dosya işleme hatası için yardımcı fonksiyon
void file_error(const char *filename, const char *operation) {
    fprintf(stderr, "Hata: '%s' dosyası %s işlemi başarısız\n", filename, operation);
}

int main() {
    char input[MAX_INPUT_SIZE];
    char **tokens;
    
    // SIGCHLD sinyali için handler ayarla
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        // Prompt göster
        printf("> ");
        fflush(stdout);

        // Kullanıcı girdisini oku
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Satır sonu karakterini kaldır
        input[strcspn(input, "\n")] = 0;

        // Boş girdiyi kontrol et
        if (strlen(input) == 0) {
            continue;
        }

        // quit komutu kontrolü
        if (strcmp(input, "quit") == 0) {
            // Arkaplan proseslerinin bitmesini bekle
            for (int i = 0; i < bg_count; i++) {
                int status;
                waitpid(background_pids[i], &status, 0);
                printf("[%d] retval: %d\n", background_pids[i], WEXITSTATUS(status));
            }
            break;
        }

        // Komutları işle
        process_commands(input);
    }

    return 0;
}

// SIGCHLD sinyal handler'ı
void sigchld_handler(int signum) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_count; i++) {
            if (background_pids[i] == pid) {
                printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
                // PID'i listeden kaldır
                for (int j = i; j < bg_count - 1; j++) {
                    background_pids[j] = background_pids[j + 1];
                }
                bg_count--;
                break;
            }
        }
    }
}

// Girdiyi token'lara ayırma
char **tokenize(char *line) {
    char **tokens = malloc(MAX_NUM_TOKENS * sizeof(char*));
    char *token = strtok(line, " \t\r\n");
    int i = 0;

    while (token != NULL && i < MAX_NUM_TOKENS - 1) {
        tokens[i] = strdup(token);
        token = strtok(NULL, " \t\r\n");
        i++;
    }
    tokens[i] = NULL;
    return tokens;
}

// Pipe işlemi
void handle_pipe(char **tokens) {
    int i;
    for (i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;
            break;
        }
    }

    int pipefd[2];
    pipe(pipefd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // İlk çocuk proses
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        char path[1024];
        if (strchr(tokens[0], '/') == NULL) {
            // Eğer komutta '/' yoksa, mevcut dizine de bak
            sprintf(path, "./%s", tokens[0]);
            if (access(path, X_OK) == 0) {
                execv(path, tokens);
            }
        }
        execvp(tokens[0], tokens);
        command_error(tokens[0], "çalıştırılamadı");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // İkinci çocuk proses
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(tokens[i + 1], &tokens[i + 1]);
        command_error(tokens[i + 1], "çalıştırılamadı");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// I/O yönlendirme işlemi
void handle_io_redirection(char **tokens) {
    int i, in_redirect = 0, out_redirect = 0;
    char *input_file = NULL, *output_file = NULL;

    // Yönlendirme sembollerini ara
    for (i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            in_redirect = 1;
            tokens[i] = NULL;
            if (tokens[i + 1]) {
                input_file = tokens[i + 1];
            }
        } else if (strcmp(tokens[i], ">") == 0) {
            out_redirect = 1;
            tokens[i] = NULL;
            if (tokens[i + 1]) {
                output_file = tokens[i + 1];
            }
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Çocuk proses
        if (in_redirect) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                file_error(input_file, "açma");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (out_redirect) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                file_error(output_file, "oluşturma");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        char path[1024];
        if (strchr(tokens[0], '/') == NULL) {
            // Eğer komutta '/' yoksa, mevcut dizine de bak
            sprintf(path, "./%s", tokens[0]);
            if (access(path, X_OK) == 0) {
                execv(path, tokens);
            }
        }
        execvp(tokens[0], tokens);
        command_error(tokens[0], "çalıştırılamadı");
        exit(1);
    } else {
        // Ebeveyn proses
        if (!in_redirect && !out_redirect) {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

// Arkaplan işleme fonksiyonu
void execute_background(char **tokens) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork hatası");
        return;
    }
    
    if (pid == 0) {
        // Çocuk proses
        char path[1024];
        if (strchr(tokens[0], '/') == NULL) {
            // Eğer komutta '/' yoksa, mevcut dizine de bak
            sprintf(path, "./%s", tokens[0]);
            if (access(path, X_OK) == 0) {
                execv(path, tokens);
            }
        }
        execvp(tokens[0], tokens);
        command_error(tokens[0], "çalıştırılamadı");
        exit(1);
    } else {
        // Ebeveyn proses
        // Arkaplan prosesini listeye ekle
        background_pids[bg_count++] = pid;
        printf("[%d] Arkaplan prosesi başlatıldı\n", pid);
    }
}

// Noktalı virgül ile ayrılmış komutları işleme
void process_commands(char *input) {
    char *command;
    char *saveptr;
    
    // Noktalı virgül ile komutları ayır
    command = strtok_r(input, ";", &saveptr);
    
    while (command != NULL) {
        // Baştaki ve sondaki boşlukları temizle
        while (*command == ' ') command++;
        
        // Komutu tokenize et ve çalıştır
        char **tokens = tokenize(command);
        if (tokens != NULL) {
            // Arkaplan çalışma kontrolü
            int background = 0;
            int i;
            for (i = 0; tokens[i] != NULL; i++) {
                if (strcmp(tokens[i], "&") == 0) {
                    background = 1;
                    tokens[i] = NULL;
                    break;
                }
            }

            // Pipe kontrolü
            int has_pipe = 0;
            for (i = 0; tokens[i] != NULL; i++) {
                if (strcmp(tokens[i], "|") == 0) {
                    has_pipe = 1;
                    break;
                }
            }

            if (background) {
                execute_background(tokens);
            } else if (has_pipe) {
                handle_pipe(tokens);
            } else {
                handle_io_redirection(tokens);
            }

            cleanup(tokens);
        }
        
        command = strtok_r(NULL, ";", &saveptr);
    }
}

// Bellek temizleme
void cleanup(char **tokens) {
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
