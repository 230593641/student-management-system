/**
 * 学生成绩管理系统（单链表版本）
 *
 * 数据结构：带头结点的单链表。链表是数据结构课程的核心内容，
 *           本实现覆盖链表的创建、遍历、查找、插入、删除、排序等基本操作。
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

#define NAME_LEN 20        /* 姓名最大长度 */
#define FILENAME "students.txt"

/* 学生结点：数据域 + 指针域 */
typedef struct Student {
    char id[12];            /* 学号 */
    char name[NAME_LEN];    /* 姓名 */
    int c_score;            /* C语言成绩（0-100） */
    int ds_score;           /* 数据结构成绩（0-100） */
    float average;          /* 平均分，自动计算 */
    struct Student *next;   /* 指向下一个结点的指针 */
} Student;

/* 头结点：不存放学生数据，只作为链表入口，简化插入/删除的边界处理 */
Student *head = NULL;

/* 函数声明 */
void menu(void);
Student *create_list(void);
Student *create_node(void);
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
Student *find_node_by_id(const char *id);
void free_list(void);
int  is_all_digits(const char *s);

int main(void)
{
    head = create_list();          /* 创建带头结点的链表 */
    load_from_file();              /* 启动时加载历史数据 */

    int choice;
    do {
        menu();
        printf("请输入您的选择（0-9）：");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); /* 清空非法输入 */
            choice = -1;               /* 无效选择，走 default 分支 */
        } else {
            while (getchar() != '\n'); /* 清空输入缓冲 */
        }
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

    free_list();                   /* 退出前释放所有结点，避免内存泄漏 */
    return 0;
}

/* 打印主菜单 */
void menu(void)
{
    printf("=========== 学生成绩管理系统（单链表版）===========\n");
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
    printf("===============================================\n");
}

/* 创建带头结点的空链表 */
Student *create_list(void)
{
    Student *h = (Student *)malloc(sizeof(Student));
    if (h == NULL) {
        printf("内存分配失败，程序退出。\n");
        exit(1);
    }
    h->next = NULL;
    return h;
}

/* 创建单个学生结点（数据域由调用方填充） */
Student *create_node(void)
{
    Student *s = (Student *)malloc(sizeof(Student));
    if (s == NULL) {
        printf("内存分配失败。\n");
        exit(1);
    }
    s->next = NULL;
    return s;
}

/* 判断字符串是否全部为数字（学号合法性校验） */
int is_all_digits(const char *s)
{
    if (s[0] == '\0')
        return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9')
            return 0;
    }
    return 1;
}

/* 录入一个学生（尾插法：新结点追加到链表末尾） */
void add_student(void)
{
    Student *s = create_node();
    printf("请输入学号：");
    scanf("%11s", s->id);
    while (getchar() != '\n');

    /* 学号格式校验：必须为纯数字 */
    if (!is_all_digits(s->id)) {
        printf("学号只能由数字组成，录入失败。\n");
        free(s);
        return;
    }

    /* 学号查重，避免重复录入 */
    if (find_node_by_id(s->id) != NULL) {
        printf("该学号已存在，录入失败。\n");
        free(s);
        return;
    }

    printf("请输入姓名：");
    scanf("%19s", s->name);
    while (getchar() != '\n');

    s->c_score  = input_score("C语言成绩（0-100）");
    s->ds_score = input_score("数据结构成绩（0-100）");
    calc_average(s);

    /* 尾插：遍历到最后一个结点后挂接 */
    Student *p = head;
    while (p->next != NULL)
        p = p->next;
    p->next = s;

    printf("录入成功！\n");
}

/* 显示所有学生：遍历链表 */
void show_all(void)
{
    if (head->next == NULL) {
        printf("暂无学生数据。\n");
        return;
    }
    printf("%-12s %-10s %-12s %-14s %-8s\n", "学号", "姓名", "C语言", "数据结构", "平均分");
    printf("--------------------------------------------------------------\n");
    Student *p = head->next;
    while (p != NULL) {
        printf("%-12s %-10s %-12d %-14d %-8.2f\n",
               p->id, p->name, p->c_score, p->ds_score, p->average);
        p = p->next;
    }
}

/* 按学号精确查询 */
void search_by_id(void)
{
    char id[12];
    printf("请输入要查询的学号：");
    scanf("%11s", id);
    while (getchar() != '\n');

    Student *p = find_node_by_id(id);
    if (p == NULL) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }
    printf("%-12s %-10s %-12s %-14s %-8s\n", "学号", "姓名", "C语言", "数据结构", "平均分");
    printf("--------------------------------------------------------------\n");
    printf("%-12s %-10s %-12d %-14d %-8.2f\n",
           p->id, p->name, p->c_score, p->ds_score, p->average);
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
    Student *p = head->next;
    while (p != NULL) {
        if (strstr(p->name, name) != NULL) {
            printf("%-12s %-10s %-12d %-14d %-8.2f\n",
                   p->id, p->name, p->c_score, p->ds_score, p->average);
            found = 1;
        }
        p = p->next;
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

    Student *p = find_node_by_id(id);
    if (p == NULL) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }

    printf("当前信息：学号 %s，姓名 %s，C语言 %d，数据结构 %d\n",
           p->id, p->name, p->c_score, p->ds_score);

    printf("请输入新的姓名：");
    scanf("%19s", p->name);
    while (getchar() != '\n');
    p->c_score  = input_score("请输入新的C语言成绩（0-100）");
    p->ds_score = input_score("请输入新的数据结构成绩（0-100）");
    calc_average(p);
    printf("修改成功。\n");
}

/* 删除指定学号的学生：找到前驱结点后修改指针，跳过并释放目标结点 */
void delete_student(void)
{
    char id[12];
    printf("请输入要删除的学号：");
    scanf("%11s", id);
    while (getchar() != '\n');

    Student *prev = head;          /* 前驱指针，从头结点开始 */
    Student *cur  = head->next;    /* 当前指针 */
    while (cur != NULL && strcmp(cur->id, id) != 0) {
        prev = cur;
        cur  = cur->next;
    }
    if (cur == NULL) {
        printf("未找到学号为 %s 的学生。\n", id);
        return;
    }

    /* 删除前确认，防止误操作 */
    char confirm;
    printf("确认删除学号为 %s 的学生（%s）吗？(y/n)：", cur->id, cur->name);
    scanf("%c", &confirm);
    while (getchar() != '\n');
    if (confirm != 'y' && confirm != 'Y') {
        printf("已取消删除。\n");
        return;
    }

    prev->next = cur->next;        /* 跳过待删结点 */
    free(cur);                     /* 释放结点内存 */
    printf("已删除学号为 %s 的学生。\n", id);
}

/* 按平均分从高到低排序（链表冒泡排序，交换相邻结点的数据域） */
void sort_by_average(void)
{
    if (head->next == NULL || head->next->next == NULL) {
        printf("学生人数不足，无需排序。\n");
        return;
    }
    int swapped;
    do {
        swapped = 0;
        Student *p = head->next;   /* p 指向当前比较对的前一个 */
        while (p->next != NULL) {
            if (p->average < p->next->average) {
                /* 交换两个结点的数据域（学号、姓名、成绩、平均分） */
                char tmp_id[12], tmp_name[NAME_LEN];
                int tmp_c, tmp_ds;
                float tmp_avg;

                strcpy(tmp_id, p->id);
                strcpy(tmp_name, p->name);
                tmp_c  = p->c_score;  tmp_ds = p->ds_score;  tmp_avg = p->average;

                strcpy(p->id, p->next->id);    strcpy(p->name, p->next->name);
                p->c_score = p->next->c_score; p->ds_score = p->next->ds_score;
                p->average = p->next->average;

                strcpy(p->next->id, tmp_id);   strcpy(p->next->name, tmp_name);
                p->next->c_score = tmp_c;      p->next->ds_score = tmp_ds;
                p->next->average = tmp_avg;

                swapped = 1;
            }
            p = p->next;
        }
    } while (swapped);

    printf("排序完成，已按平均分从高到低排列，当前结果：\n");
    show_all();
}

/* 成绩统计：各科平均分、最高分、最低分、及格率、分数段分布 */
void statistics(void)
{
    if (head->next == NULL) {
        printf("暂无数据，无法统计。\n");
        return;
    }
    int count = 0;
    int sum_c = 0, sum_ds = 0, max_c = 0, max_ds = 0, min_c = 100, min_ds = 100;
    int pass_c = 0, pass_ds = 0;   /* 及格线 60 分 */
    /* 分数段：优>=90，良80-89，中70-79，及格60-69，不及格<60 */
    int seg_c[5] = {0}, seg_ds[5] = {0};

    Student *p = head->next;
    while (p != NULL) {
        count++;
        sum_c  += p->c_score;
        sum_ds += p->ds_score;
        if (p->c_score > max_c)  max_c  = p->c_score;
        if (p->ds_score > max_ds) max_ds = p->ds_score;
        if (p->c_score < min_c)  min_c  = p->c_score;
        if (p->ds_score < min_ds) min_ds = p->ds_score;
        if (p->c_score >= 60)  pass_c++;
        if (p->ds_score >= 60) pass_ds++;

        if (p->c_score >= 90)       seg_c[0]++;
        else if (p->c_score >= 80)  seg_c[1]++;
        else if (p->c_score >= 70)  seg_c[2]++;
        else if (p->c_score >= 60)  seg_c[3]++;
        else                        seg_c[4]++;

        if (p->ds_score >= 90)      seg_ds[0]++;
        else if (p->ds_score >= 80) seg_ds[1]++;
        else if (p->ds_score >= 70) seg_ds[2]++;
        else if (p->ds_score >= 60) seg_ds[3]++;
        else                        seg_ds[4]++;

        p = p->next;
    }

    printf("========== 成绩统计（共 %d 人）==========\n", count);
    printf("科目      平均分   最高分   最低分   及格率\n");
    printf("C语言     %6.2f   %6d   %6d   %6.1f%%\n",
           (float)sum_c / count, max_c, min_c, (float)pass_c / count * 100);
    printf("数据结构  %6.2f   %6d   %6d   %6.1f%%\n",
           (float)sum_ds / count, max_ds, min_ds, (float)pass_ds / count * 100);
    printf("---------------------------------------------\n");
    printf("分数段分布：\n");
    printf("科目      优(>=90)  良(80-89)  中(70-79)  及格(60-69)  不及格(<60)\n");
    printf("C语言     %5d     %5d     %5d     %5d      %5d\n",
           seg_c[0], seg_c[1], seg_c[2], seg_c[3], seg_c[4]);
    printf("数据结构  %5d     %5d     %5d     %5d      %5d\n",
           seg_ds[0], seg_ds[1], seg_ds[2], seg_ds[3], seg_ds[4]);
}

/* 保存学生数据到文本文件（遍历链表逐结点写入） */
void save_to_file(void)
{
    FILE *fp = fopen(FILENAME, "w");
    if (fp == NULL) {
        printf("文件打开失败，保存未成功。\n");
        return;
    }
    int count = 0;
    Student *p = head->next;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    fprintf(fp, "%d\n", count);

    p = head->next;
    while (p != NULL) {
        fprintf(fp, "%s %s %d %d %.2f\n",
                p->id, p->name, p->c_score, p->ds_score, p->average);
        p = p->next;
    }
    fclose(fp);
    printf("数据已保存到 %s。\n", FILENAME);
}

/* 程序启动时从文件加载数据（读取后用尾插法重建链表） */
void load_from_file(void)
{
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
        return;   /* 文件不存在视为首次运行 */

    int n = 0;
    if (fscanf(fp, "%d", &n) != 1) {
        fclose(fp);
        return;
    }
    for (int i = 0; i < n; i++) {
        Student *s = create_node();
        if (fscanf(fp, "%11s %19s %d %d %f",
                   s->id, s->name, &s->c_score, &s->ds_score, &s->average) != 5) {
            free(s);
            break;
        }
        /* 尾插 */
        Student *tail = head;
        while (tail->next != NULL)
            tail = tail->next;
        tail->next = s;
    }
    fclose(fp);
    printf("已从 %s 加载 %d 条学生数据。\n", FILENAME, n);
}

/* 带范围校验的成绩输入（含非法字符处理，防止死循环） */
int input_score(const char *prompt)
{
    int score;
    do {
        printf("请输入%s：", prompt);
        if (scanf("%d", &score) != 1) {
            while (getchar() != '\n'); /* 清掉非法输入 */
            printf("输入无效，成绩必须是 0-100 的整数，请重新输入。\n");
            continue;
        }
        while (getchar() != '\n');
        if (score < 0 || score > 100)
            printf("成绩必须在 0-100 之间，请重新输入。\n");
    } while (score < 0 || score > 100);
    return score;
}

/* 计算平均分并写入结点 */
void calc_average(Student *s)
{
    s->average = (s->c_score + s->ds_score) / 2.0f;
}

/* 按学号查找，返回结点指针；未找到返回 NULL */
Student *find_node_by_id(const char *id)
{
    Student *p = head->next;
    while (p != NULL) {
        if (strcmp(p->id, id) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

/* 释放整个链表的内存，防止内存泄漏 */
void free_list(void)
{
    Student *p = head;
    while (p != NULL) {
        Student *tmp = p;
        p = p->next;
        free(tmp);
    }
}
