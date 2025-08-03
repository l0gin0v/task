#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER 4096
#define LENLINE 1024
#define CONFIG_DIR "/.config/task/"
#define DATA_FILE "data.txt"

void help(char *argv[]) {
	printf("This program allows the user to structure their task list and mark completed ones.\n");
	printf("%s -r (--read)\n", argv[0]);
	printf("%s -a (--add) <task>\n", argv[0]);
	printf("%s -d (--delete) <number_task>\n", argv[0]);
}

char *get_data_path() {
	static char path[BUFFER];
	snprintf(path, sizeof(path), "%s%s%s", getenv("HOME"), CONFIG_DIR, DATA_FILE);
	return path;
}

int ensure_dir() {
	char path[BUFFER];
	snprintf(path, sizeof(path), "%s%s", getenv("HOME"), CONFIG_DIR);
	if (mkdir(path, 0700) == -1 && errno != EEXIST) {
		return 1;
	}
	return 0;
}

int show_tasks(char *filename) {
	char buffer[LENLINE];
	FILE *file = fopen(filename, "r");

	if (file) {
		int i = 1;
		while ((fgets(buffer, LENLINE, file)) != NULL) {
			printf("%d) %s", i, buffer);
			++i;
		}
		fclose(file);
	}
	return 0;
}

int add_task(char *filename, char *task) {
	FILE *file = fopen(filename, "a");

	if (!file) {
		perror("File error: cannot open file");
		return 1;
	}

	fputs(task, file);
	fputs("\n", file); // need fix
	fclose(file);
	printf("Task Up!");

	return 0;
}

int delete_task(char *filename, int line_to_del) {
	if (line_to_del == 0) {
		return 1;
	}

	FILE *original_file, *temp_file;
	char buffer[LENLINE];
	int current_line = 1;

	original_file = fopen(filename, "r");
	if (!original_file) {
		perror("File error: cannot open file");
		return 1;
	}

	char temp_path[BUFFER];
	snprintf(temp_path, sizeof(temp_path), "%s%s%s", getenv("HOME"), CONFIG_DIR, "temp.txt");
	temp_file = fopen(temp_path, "w");
	if (!temp_file) {
		perror("File error: cannot create tempfile");
	}

	while (fgets(buffer, sizeof(buffer), original_file)) {
		if (current_line != line_to_del) {
			fputs(buffer, temp_file);
		}
		++current_line;
	}
	if (current_line < line_to_del) {
		perror("Argument error: number of line to remove greater than factical number of lines.");
	}

	fclose(original_file);
	fclose(temp_file);

	remove(filename);
	if (rename(temp_path, filename) == -1) {
		perror("Rename error: cannot rename tempfile");
		remove(temp_path);
		return 1;
	}

	printf("Task Down!");
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		help(argv);
		return 1;
	}

	if (getenv("HOME") == NULL) {
		fprintf(stderr, "Environment error: HOME variable not set\n");
		return 1;
	}

	if (ensure_dir()) {
		perror("Direction error: cannot creare direction");
	}

	char flag = argv[1][1];
	if (flag == '-')
		flag = argv[1][2];	

	char *data_file = get_data_path();
	switch (flag) {
		case 'h':
			help(argv);
			break;
		case 'r':
			if (show_tasks(data_file)) {
				perror("Access error: file access denied");
				return 1;
			}
			break;
		case 'a':
			if (argc < 3) {
				help(argv);
				return 1;
			}
			if (add_task(data_file, argv[2])) {
				perror("Access error: file access denied");
				return 1;
			}
			break;
		case 'd':
			if (argc < 3) {
				help(argv);
				return 1;
			}
			if (delete_task(data_file, atoi(argv[2]))) {
				perror("Access error: file access denied");
				return 1;
			}
			break;
		default:
			printf("Invalid argument. Use -h | --help to show commands.");
			break;
	}

	return 0;
}
