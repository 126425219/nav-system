#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- КОНСТАНТИ --- */
/* Запазваме оригиналните ограничения по задание */
#define MAX_BUILDINGS 10
#define MAX_ROOMS 10
#define MAX_LOGS 100
#define MAX_TEXT 200

/* Имена на файловете за база данни */
#define FILE_MAP "map_data.txt"
#define FILE_LOGS "history_log.txt"

/* --- СТРУКТУРИ ОТ ДАННИ --- */

/* Структура за отделна зала/кабинет */
typedef struct {
    char name[MAX_TEXT];
} RoomNode;

/* Структура за сграда, съдържаща списък от зали */
typedef struct {
    char name[MAX_TEXT];
    RoomNode rooms[MAX_ROOMS];
    int count_rooms;
} BuildingStruct;

/* Структура за записване на потребителска навигация */
typedef struct {
    char b_target[MAX_TEXT];  // Целева сграда
    char u_name[MAX_TEXT];    // Име на потребителя
    int scores[MAX_ROOMS];    // Оценки за разстояние/трудност
} LogEntry;

/* --- ГЛОБАЛНИ ПРОМЕНЛИВИ --- */
BuildingStruct uni_map[MAX_BUILDINGS]; // Масив с всички сгради
LogEntry history[MAX_LOGS];            // История на навигацията

int total_buildings = 0;
int total_logs = 0;

/* --- ДЕКЛАРАЦИИ НА ФУНКЦИИ --- */
void main_loop();
void menu_admin();
void menu_student();

/* Логически функции */
void add_building_logic();
void list_all_buildings();
void execute_navigation();
void show_stats();

/* Работа с файлове и памет */
void save_db();
void load_db();
void save_log_entry(LogEntry *entry, int r_count);
void load_log_db();
void clean_stdin();

/* --- MAIN --- */
int main(void) {
    // Зареждане на данните от файловете при стартиране
    load_db();
    load_log_db();
    
    // Стартиране на главното меню
    main_loop();
    
    return 0;
}

/* Помощна функция за изчистване на буфера след scanf */
void clean_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* --- МЕНЮТА --- */

void main_loop() {
    int opt = -1;
    while (opt != 0) {
        printf("\n=== Навигация в кампуса===\n");
        printf("1. Админ панел (Добавяне)\n");
        printf("2. Студентски панел (Навигация)\n");
        printf("0. Изход\n");
        printf("Вашият избор: ");

        if (scanf("%d", &opt) != 1) {
            clean_stdin();
            continue;
        }
        clean_stdin();

        // Използваме switch за по-чиста структура
        switch (opt) {
            case 1: menu_admin(); break;
            case 2: menu_student(); break;
            case 0: printf("Приятен ден!\n"); break;
            default: printf("Грешна опция!\n");
        }
    }
}

void menu_admin() {
    int opt;
    while (1) {
        printf("\n--- АДМИНИСТРАТОР ---\n");
        printf("1. Нова сграда\n");
        printf("2. Списък сгради\n");
        printf("3. Статистика\n");
        printf("0. Назад\n");
        printf(">> ");

        if (scanf("%d", &opt) != 1) { clean_stdin(); continue; }
        clean_stdin();

        switch (opt) {
            case 1: add_building_logic(); break;
            case 2: list_all_buildings(); break;
            case 3: show_stats(); break;
            case 0: return;
        }
    }
}

void menu_student() {
    int opt;
    while (1) {
        printf("\n--- СТУДЕНТ ---\n");
        printf("1. Виж сгради\n");
        printf("2. Навигирай\n");
        printf("0. Назад\n");
        printf(">> ");

        if (scanf("%d", &opt) != 1) { clean_stdin(); continue; }
        clean_stdin();

        switch (opt) {
            case 1: list_all_buildings(); break;
            case 2: execute_navigation(); break;
            case 0: return;
        }
    }
}

/* --- ОСНОВНА ЛОГИКА --- */

/* Добавяне на нова сграда от администратор */
void add_building_logic() {
    // Проверка дали е достигнат лимита на масива
    if (total_buildings >= MAX_BUILDINGS) {
        printf("Достигнат е лимитът на сградите!\n");
        return;
    }

    BuildingStruct *b = &uni_map[total_buildings];
    
    printf("Въведете име на сграда: ");
    fgets(b->name, MAX_TEXT, stdin);
    b->name[strcspn(b->name, "\n")] = 0; // Премахване на нов ред

    printf("Брой зали в сградата: ");
    scanf("%d", &b->count_rooms);
    clean_stdin();

    if (b->count_rooms > MAX_ROOMS) b->count_rooms = MAX_ROOMS;

    // Въвеждане на имената на залите
    for (int i = 0; i < b->count_rooms; i++) {
        printf(" - Име на зала %d: ", i + 1);
        fgets(b->rooms[i].name, MAX_TEXT, stdin);
        b->rooms[i].name[strcspn(b->rooms[i].name, "\n")] = 0;
    }

    total_buildings++;
    save_db(); // Автоматичен запис във файла
    printf("Сградата е записана успешно.\n");
}

/* Извеждане на списък с всички налични сгради */
void list_all_buildings() {
    if (total_buildings == 0) {
        printf("Няма въведени сгради в картата.\n");
        return;
    }
    printf("\n--- НАЛИЧНИ СГРАДИ ---\n");
    for (int i = 0; i < total_buildings; i++) {
        printf("%d. %s\n", i + 1, uni_map[i].name);
    }
}

/* Процес на навигация и оценка от страна на студента */
void execute_navigation() {
    if (total_buildings == 0) return;

    list_all_buildings();
    
    int selection;
    printf("Изберете номер на сграда: ");
    if (scanf("%d", &selection) != 1) { clean_stdin(); return; }
    clean_stdin();

    // Валидация на избора
    if (selection < 1 || selection > total_buildings) {
        printf("Невалиден номер.\n");
        return;
    }

    // Взимаме pointer към избраната сграда за по-лесен достъп
    BuildingStruct *target = &uni_map[selection - 1];
    
    // Подготвяме нов запис в историята
    LogEntry *log = &history[total_logs];
    strcpy(log->b_target, target->name);

    printf("Вашето име: ");
    fgets(log->u_name, MAX_TEXT, stdin);
    log->u_name[strcspn(log->u_name, "\n")] = 0;

    printf("Оценете разстоянието (1-Близо, 5-Далеч):\n");
    for (int i = 0; i < target->count_rooms; i++) {
        int val;
        printf(" -> До %s: ", target->rooms[i].name);
        if (scanf("%d", &val) != 1) { clean_stdin(); val = 3; }
        clean_stdin();
        log->scores[i] = val;
    }

    // Записваме резултата и увеличаваме брояча
    save_log_entry(log, target->count_rooms);
    total_logs++;
    printf("Данните са записани!\n");
}

/* Изчисляване и показване на средни оценки за разстоянията */
void show_stats() {
    if (total_buildings == 0) return;

    for (int i = 0; i < total_buildings; i++) {
        BuildingStruct *b = &uni_map[i];
        float sum[MAX_ROOMS] = {0};
        int voters = 0;

        // Обхождаме историята, за да намерим записи за конкретната сграда
        for (int k = 0; k < total_logs; k++) {
            if (strcmp(history[k].b_target, b->name) == 0) {
                for (int r = 0; r < b->count_rooms; r++) {
                    sum[r] += history[k].scores[r];
                }
                voters++;
            }
        }

        // Ако има гласували, показваме осреднените резултати
        if (voters > 0) {
            printf("\nСтатистика за: %s (Гласове: %d)\n", b->name, voters);
            for (int r = 0; r < b->count_rooms; r++) {
                printf(" * %s -> Ср. оценка: %.2f\n", b->rooms[r].name, sum[r] / voters);
            }
        }
    }
    printf("\nНатиснете Enter за продължаване...");
    getchar();
}


/* Запис на структурата на сградите във файл */
void save_db() {
    FILE *fp = fopen(FILE_MAP, "w");
    if (!fp) return;

    fprintf(fp, "%d\n", total_buildings);
    for (int i = 0; i < total_buildings; i++) {
        fprintf(fp, "%s\n%d\n", uni_map[i].name, uni_map[i].count_rooms);
        for (int j = 0; j < uni_map[i].count_rooms; j++) {
            fprintf(fp, "%s\n", uni_map[i].rooms[j].name);
        }
    }
    fclose(fp);
}

/* Четене на сградите от файл при старт */
void load_db() {
    FILE *fp = fopen(FILE_MAP, "r");
    if (!fp) return;

    if (fscanf(fp, "%d\n", &total_buildings) != 1) { fclose(fp); return; }

    for (int i = 0; i < total_buildings; i++) {
        fgets(uni_map[i].name, MAX_TEXT, fp);
        uni_map[i].name[strcspn(uni_map[i].name, "\n")] = 0;

        fscanf(fp, "%d\n", &uni_map[i].count_rooms);
        
        for (int j = 0; j < uni_map[i].count_rooms; j++) {
            fgets(uni_map[i].rooms[j].name, MAX_TEXT, fp);
            uni_map[i].rooms[j].name[strcspn(uni_map[i].rooms[j].name, "\n")] = 0;
        }
    }
    fclose(fp);
}

/* Добавяне (append) на един запис в лога */
void save_log_entry(LogEntry *entry, int r_count) {
    FILE *fp = fopen(FILE_LOGS, "a");
    if (!fp) return;

    fprintf(fp, "%s\n%s\n", entry->b_target, entry->u_name);
    for (int i = 0; i < r_count; i++) {
        fprintf(fp, "%d ", entry->scores[i]);
    }
    fprintf(fp, "\n");
    fclose(fp);
}

/* Зареждане на цялата история в паметта */
void load_log_db() {
    FILE *fp = fopen(FILE_LOGS, "r");
    if (!fp) return;

    while (total_logs < MAX_LOGS && fgets(history[total_logs].b_target, MAX_TEXT, fp)) {
        history[total_logs].b_target[strcspn(history[total_logs].b_target, "\n")] = 0;

        fgets(history[total_logs].u_name, MAX_TEXT, fp);
        history[total_logs].u_name[strcspn(history[total_logs].u_name, "\n")] = 0;

        for (int i = 0; i < MAX_ROOMS; i++) {
            fscanf(fp, "%d", &history[total_logs].scores[i]);
        }
        fgetc(fp); // Чистим символа за нов ред след числата
        total_logs++;
    }
    fclose(fp);
}