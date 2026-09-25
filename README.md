# 学生成绩管理系统

一个从 **C 语言控制台版** 升级为 **Web 版（Flask + MySQL）** 的学生成绩管理系统，覆盖数据录入、查询、修改、删除、排序、统计等功能，数据持久化存储于 MySQL 数据库，并提供图形化前端界面。

> 本仓库同时保留最初的 C 语言单链表控制台版（`main.c`），以及在此基础上扩展的 Web 版。

---

## 一、Web 版（Flask + MySQL，当前主版本）

### 技术栈

- 后端：Python 3 + Flask 3
- 数据库：MySQL 8.0
- 数据库驱动：PyMySQL（原生 SQL，便于课程演示）
- 前端：原生 HTML / CSS / JavaScript（无构建步骤）

### 功能

| 功能 | 说明 |
| --- | --- |
| 数据录入 | 学号（主键查重 + 纯数字校验）、姓名、C语言成绩、数据结构成绩（0-100 校验） |
| 列表展示 | 表格展示学号、姓名、两科成绩、平均分 |
| 按学号查询 | 精确查询 |
| 按姓名查询 | 关键字模糊匹配 |
| 修改信息 | 弹窗编辑，平均分自动重算 |
| 删除学生 | 二次确认后删除 |
| 排序 | 按平均分 / 单科 / 学号升降序 |
| 成绩统计 | 各科平均分、最高、最低、及格率；优/良/中/及格/不及格分数段分布 |

### 项目结构

```
student-management-system/
├── app.py              # Flask 后端（路由 + REST API）
├── db.py               # 数据库连接与建库建表
├── config.py           # 数据库连接配置
├── seed_from_txt.py    # 从旧 students.txt 迁移数据
├── requirements.txt    # Python 依赖
├── templates/
│   └── index.html      # 前端页面
├── static/
│   ├── css/style.css
│   └── js/app.js
├── main.c              # C 语言控制台版（单链表，保留）
└── 运行Web版.bat       # 一键启动脚本
```

### 运行步骤

1. 安装依赖：
   ```bash
   pip install -r requirements.txt
   ```
2. 确认 MySQL 服务已启动，并在 `config.py` 中填好账号密码。
3. 初始化数据库并迁移旧数据（首次运行）：
   ```bash
   python db.py        # 见 app.py 启动时会自动建库建表
   python seed_from_txt.py   # 把 students.txt 的旧数据导入
   ```
4. 启动：
   ```bash
   python app.py
   # 或双击 运行Web版.bat
   ```
5. 浏览器访问：http://127.0.0.1:5000

### 数据库表（students）

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| id | VARCHAR(12) 主键 | 学号 |
| name | VARCHAR(20) | 姓名 |
| c_score | INT | C语言成绩 0-100 |
| ds_score | INT | 数据结构成绩 0-100 |
| average | DECIMAL(5,2) | 平均分（自动计算） |
| created_at | TIMESTAMP | 录入时间 |

---

## 二、C 语言控制台版（原始版本）

用 C 语言实现的**带头结点单链表**学生成绩管理，适合作为数据结构课程设计。

```bash
gcc main.c -o student_management.exe
./student_management.exe
```

覆盖链表创建、遍历、查找、插入、删除、冒泡排序、文件读写（`students.txt`）。

---

## 设计要点

- **分层架构**：前端页面 → Flask 路由（API）→ PyMySQL → MySQL，职责清晰
- **数据校验**：后端统一校验学号格式、成绩范围、重号，防止脏数据
- **统计聚合**：直接用 SQL 聚合计算平均分 / 及格率 / 分数段，避免在应用层遍历
- **接口规范**：RESTful JSON 接口，错误返回统一 `{"error": "..."}`
- **数据迁移**：`seed_from_txt.py` 将旧版文本数据平滑导入数据库
