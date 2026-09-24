/**
 * 学生成绩管理系统
 * 
 * 功能：学生信息的录入、显示、查询、修改、删除、排序、统计，
 *       并支持数据保存到文件与从文件加载。
 * 
 * 编译运行（需要 GCC）：
 *   gcc main.c -o student_management.exe
 *   ./student_management.exe
 * 
 * 数据文件：默认保存在程序同目录下的 students.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100   /* 最大学生数量 */
#define NAME_LEN 20        /* 姓名最大长度 */
#define FILENAME "students.txt"

/* 学生结构体：一条学生记录包含学号、姓名、两科成绩与平均分 */
typedef struct {
    char id[12];       /* 学号 */
    char name[NAME_LEN]; /* 姓名 */
    int c_score;       /* C语言成绩（0-100） */
    int ds_score;      /* 数据结构成绩（0-100） */
    float average;     /* 平均分，自动计算 */
} Student;

/* 全局学生数组与当前人数 */
Student students[MAX_STUDENTS];
int count = 0;

/* 函数声明 */
void menu(void);
void add_student(void);
void show_all(void);
void search_by_id(void);
void search_by_name(void);
void modify_student(void);
void delete_student(void);
void sort_by_average(void);
void statistics(void);
void save_to_file(void);
void load_from_file(void);
int  input_score(const char *prompt);
void calc_average(Student *s);
int  find_index_by_id(const char *id);

int main(void)
{
    load_from_file();
    int choice;
    do {
        menu();
        printf("请输入您的选择（0-9）：");
        scanf("%d", &choice);
        while (getchar() != '\n');   /* 清空输入缓冲，防止残留换行影响后续输入 */
        switch (choice) {
            case 1: add_student();      break;
            case 2: show_all();         break;
            case 3: search_by_id();     break;
            case 4: search_by_name();   break;
            case 5: modify_student();   break;
            case 6: delete_student();   break;
            case 7: sort_by_average();  break;
            case 8: statistics();       break;
            case 9: save_to_file();     break;
            case 0: save_to_file(); printf("感谢使用，再见！\n"); break;
            default: printf("无效选项，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);
    return 0;
}

/* 打印主菜单 */
void menu(void)
{
    printf("=========== 学生成绩管理系统 ===========\n");
    printf("  1. 录入学生信息\n");
    printf("  2. 显示所有学生\n");
    printf("  3. 按学号查询\n");
    printf("  4. 按姓名查询\n");
    printf("  5. 修改学生信息\n");
    printf("  6. 删除学生\n");
    printf("  7. 按平均分排序\n");
    printf("  8. 成绩统计\n");
    printf("  9. 保存数据到文件\n");
    printf("  0. 退出系统（自动保存）\n");
    printf("=========================================\n");
}

/* 录入一个学生 */
void add_student(void)
{
    if (count >= MAX_STUDENTS) {
        printf("存储空间已满，无法继续录入。\n");
        return;
    }
    Student s;
    printf("请输入学号：");
    scanf("%11s", s.id);
    while (getchar() != '\n');

    /* 学号查重，避免重复录入 */
    if (find_index_by_id(s.id) >= 0) {
        printf("该学号已存在，录入失败。\n");
        return;
    }

    printf("请输入姓名：");
    scanf("%19s", s.name);
    while (getchar() != '\n');

    s.c_score  = input_score("C语言成绩（0-100）");
    s.ds_score = input_score("数据结构成绩（0-100）");
    calc_average(&s);

    students[count++] = s;
    printf("录入成功！当前共 %d 名学生。\n", count);
}

/* 显示所有学生 */
void show_all(void)
{
    if (count == 0) {
        printf("暂无学生数据。\n");
        return;
    }
    printf("%-12s %-10s %-12s %-14s %-8s\n", "学号", "姓名", "C语言", "数据结构", "平均分");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-12s %-10s %-12d %-14d %-8.2f\n",
               students[i].id, students[i].name,
               students[i].c_score, students[i].ds_score,
               students[i].average);
    }
}

/* 按学号精确查询 */
void search_by_id(void)
{
    char id[12];
    printf("请输入要查询的学号：");
    scanf("%11s", id);
    while (getchar() != '\n');

    int idx = find_index_by_id(id);
    if (idx < 0) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }
    printf("%-12s %-10s %-12s %-14s %-8s\n", "学号", "姓名", "C语言", "数据结构", "平均分");
    printf("--------------------------------------------------------------\n");
    printf("%-12s %-10s %-12d %-14d %-8.2f\n",
           students[idx].id, students[idx].name,
           students[idx].c_score, students[idx].ds_score,
           students[idx].average);
}

/* 按姓名模糊查询（支持关键字） */
void search_by_name(void)
{
    char name[NAME_LEN];
    printf("请输入要查询的姓名（支持关键字）：");
    scanf("%19s", name);
    while (getchar() != '\n');

    int found = 0;
    printf("%-12s %-10s %-12s %-14s %-8s\n", "学号", "姓名", "C语言", "数据结构", "平均分");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        if (strstr(students[i].name, name) != NULL) {
            printf("%-12s %-10s %-12d %-14d %-8.2f\n",
                   students[i].id, students[i].name,
                   students[i].c_score, students[i].ds_score,
                   students[i].average);
            found = 1;
        }
    }
    if (!found)
        printf("未找到姓名包含 %s 的学生。\n", name);
}

/* 修改指定学号的学生信息 */
void modify_student(void)
{
    char id[12];
    printf("请输入要修改的学号：");
    scanf("%11s", id);
    while (getchar() != '\n');

    int idx = find_index_by_id(id);
    if (idx < 0) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }

    printf("当前信息：学号 %s，姓名 %s，C语言 %d，数据结构 %d\n",
           students[idx].id, students[idx].name,
           students[idx].c_score, students[idx].ds_score);

    printf("请输入新的姓名：");
    scanf("%19s", students[idx].name);
    while (getchar() != '\n');
    students[idx].c_score  = input_score("请输入新的C语言成绩（0-100）");
    students[idx].ds_score = input_score("请输入新的数据结构成绩（0-100）");
    calc_average(&students[idx]);
    printf("修改成功。\n");
}

/* 删除指定学号的学生 */
void delete_student(void)
{
    char id[12];
    printf("请输入要删除的学号：");
    scanf("%11s", id);
    while (getchar() != '\n');

    int idx = find_index_by_id(id);
    if (idx < 0) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }

    /* 后面的元素整体前移覆盖，实现删除 */
    for (int i = idx; i < count - 1; i++)
        students[i] = students[i + 1];
    count--;
    printf("已删除学号为 %s 的学生，当前共 %d 名学生。\n", id, count);
}

/* 按平均分从高到低排序（冒泡排序，考研数据结构经典算法） */
void sort_by_average(void)
{
    if (count < 2) {
        printf("学生人数不足，无需排序。\n");
        return;
    }
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (students[j].average < students[j + 1].average) {
                Student tmp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = tmp;
            }
        }
    }
    printf("排序完成，已按平均分从高到低排列，当前结果：\n");
    show_all();
}

/* 成绩统计：各科平均分、最高分、最低分、及格率 */
void statistics(void)
{
    if (count == 0) {
        printf("暂无数据，无法统计。\n");
        return;
    }
    int sum_c = 0, sum_ds = 0, max_c = 0, max_ds = 0, min_c = 100, min_ds = 100;
    int pass_c = 0, pass_ds = 0;   /* 及格线 60 分 */

    for (int i = 0; i < count; i++) {
        sum_c  += students[i].c_score;
        sum_ds += students[i].ds_score;
        if (students[i].c_score > max_c)  max_c  = students[i].c_score;
        if (students[i].ds_score > max_ds) max_ds = students[i].ds_score;
        if (students[i].c_score < min_c)  min_c  = students[i].c_score;
        if (students[i].ds_score < min_ds) min_ds = students[i].ds_score;
        if (students[i].c_score >= 60)  pass_c++;
        if (students[i].ds_score >= 60) pass_ds++;
    }

    printf("========== 成绩统计（共 %d 人）==========\n", count);
    printf("科目      平均分   最高分   最低分   及格率\n");
    printf("C语言     %6.2f   %6d   %6d   %6.1f%%\n",
           (float)sum_c / count, max_c, min_c, (float)pass_c / count * 100);
    printf("数据结构  %6.2f   %6d   %6d   %6.1f%%\n",
           (float)sum_ds / count, max_ds, min_ds, (float)pass_ds / count * 100);
}

/* 保存学生数据到文本文件（UTF-8 编码，便于阅读） */
void save_to_file(void)
{
    FILE *fp = fopen(FILENAME, "w");
    if (fp == NULL) {
        printf("文件打开失败，保存未成功。\n");
        return;
    }
    fprintf(fp, "%d\n", count);
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %s %d %d %.2f\n",
                students[i].id, students[i].name,
                students[i].c_score, students[i].ds_score,
                students[i].average);
    }
    fclose(fp);
    printf("数据已保存到 %s。\n", FILENAME);
}

/* 程序启动时从文件加载数据 */
void load_from_file(void)
{
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
        return;   /* 文件不存在视为首次运行 */
    if (fscanf(fp, "%d", &count) != 1) {
        count = 0;
        fclose(fp);
        return;
    }
    for (int i = 0; i < count; i++) {
        if (fscanf(fp, "%11s %19s %d %d %f",
                   students[i].id, students[i].name,
                   &students[i].c_score, &students[i].ds_score,
                   &students[i].average) != 5) {
            count = i;   /* 文件异常时只保留完整读取的部分 */
            break;
        }
    }
    fclose(fp);
    printf("已从 %s 加载 %d 条学生数据。\n", FILENAME, count);
}

/* 带范围校验的成绩输入 */
int input_score(const char *prompt)
{
    int score;
    do {
        printf("请输入%s：", prompt);
        scanf("%d", &score);
        while (getchar() != '\n');
        if (score < 0 || score > 100)
            printf("成绩必须在 0-100 之间，请重新输入。\n");
    } while (score < 0 || score > 100);
    return score;
}

/* 计算平均分并写入结构体 */
void calc_average(Student *s)
{
    s->average = (s->c_score + s->ds_score) / 2.0f;
}

/* 按学号查找，返回数组下标；未找到返回 -1 */
int find_index_by_id(const char *id)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(students[i].id, id) == 0)
            return i;
    }
    return -1;
}
