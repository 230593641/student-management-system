# 数据库连接与初始化（PyMySQL，原生命令清晰，适合课程演示）
import pymysql
from config import (DB_HOST, DB_PORT, DB_USER, DB_PASSWORD,
                    DB_NAME, DB_CHARSET)

# students 表结构：学号为主键，平均分由 (C语言+数据结构)/2 计算
SCHEMA = """
CREATE TABLE IF NOT EXISTS students (
    id         VARCHAR(12)   PRIMARY KEY COMMENT '学号（主键）',
    name       VARCHAR(20)   NOT NULL     COMMENT '姓名',
    c_score    INT           NOT NULL     COMMENT 'C语言成绩 0-100',
    ds_score   INT           NOT NULL     COMMENT '数据结构成绩 0-100',
    average    DECIMAL(5,2)  NOT NULL     COMMENT '平均分（自动计算）',
    created_at TIMESTAMP     DEFAULT CURRENT_TIMESTAMP COMMENT '录入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='学生成绩表';
"""


def _raw_conn():
    """连接到 MySQL 服务器（未指定数据库，用于创建库）。"""
    return pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER,
                           password=DB_PASSWORD, charset=DB_CHARSET)


def get_conn():
    """返回连接到 student_management 库的连接对象。"""
    return pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER,
                           password=DB_PASSWORD, database=DB_NAME,
                           charset=DB_CHARSET, autocommit=True,
                           cursorclass=pymysql.cursors.DictCursor)


def init_db():
    """建库 + 建表（幂等：已存在则跳过）。"""
    conn = _raw_conn()
    try:
        with conn.cursor() as cur:
            cur.execute(
                f"CREATE DATABASE IF NOT EXISTS `{DB_NAME}` "
                "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci"
            )
        conn.commit()
    finally:
        conn.close()

    conn = get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute(SCHEMA)
    finally:
        conn.close()
