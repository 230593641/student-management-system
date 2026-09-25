"""
学生成绩管理系统 · Web 版（Flask + MySQL）
=========================================
功能：学生信息录入、列表、查询、修改、删除、排序、统计，数据存储于 MySQL。
技术栈：Flask 3.x + PyMySQL 1.2 + MySQL 8.0，前端为原生 HTML/CSS/JS。

运行方式（项目根目录）：
    python app.py
默认访问：http://127.0.0.1:5000
"""
import pymysql
from flask import Flask, jsonify, render_template, request

import db
from config import DB_NAME

app = Flask(__name__)


# ---------- 前端页面 ----------
@app.route('/')
def index():
    return render_template('index.html')


# ---------- 工具函数 ----------
def validate(payload):
    """校验录入/修改的数据，返回 (是否合法, 错误信息, 平均分)。"""
    sid = str(payload.get('id', '')).strip()
    name = str(payload.get('name', '')).strip()
    try:
        c = int(payload.get('c_score'))
        d = int(payload.get('ds_score'))
    except (TypeError, ValueError):
        return False, '成绩必须是整数。', None
    if not sid:
        return False, '学号不能为空。', None
    if not sid.isdigit():
        return False, '学号只能由数字组成。', None
    if len(sid) > 11:
        return False, '学号长度不能超过 11 位。', None
    if not name:
        return False, '姓名不能为空。', None
    if len(name) > 19:
        return False, '姓名长度不能超过 19 个字符。', None
    if not (0 <= c <= 100):
        return False, 'C语言成绩必须在 0-100 之间。', None
    if not (0 <= d <= 100):
        return False, '数据结构成绩必须在 0-100 之间。', None
    return True, '', round((c + d) / 2.0, 2)


# ---------- 学生 CRUD ----------
@app.route('/api/students', methods=['GET'])
def list_students():
    """返回全部学生（按学号升序）。"""
    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT id, name, c_score, ds_score, average "
                        "FROM students ORDER BY id")
            rows = cur.fetchall()
    finally:
        conn.close()
    return jsonify({'data': rows, 'total': len(rows)})


@app.route('/api/students', methods=['POST'])
def add_student():
    """新增学生。"""
    payload = request.get_json(silent=True) or {}
    ok, msg, avg = validate(payload)
    if not ok:
        return jsonify({'error': msg}), 400

    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT COUNT(*) AS n FROM students WHERE id=%s",
                        (payload['id'],))
            if cur.fetchone()['n'] > 0:
                return jsonify({'error': '该学号已存在。'}), 409
            cur.execute(
                "INSERT INTO students (id, name, c_score, ds_score, average) "
                "VALUES (%s, %s, %s, %s, %s)",
                (payload['id'], payload['name'],
                 int(payload['c_score']), int(payload['ds_score']), avg))
        conn.commit()
    except pymysql.MySQLError as e:
        return jsonify({'error': f'数据库错误：{e}'}), 500
    finally:
        conn.close()
    return jsonify({'message': '录入成功', 'average': avg}), 201


@app.route('/api/students/<sid>', methods=['GET'])
def get_student(sid):
    """按学号查询单个学生。"""
    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT id, name, c_score, ds_score, average "
                        "FROM students WHERE id=%s", (sid,))
            row = cur.fetchone()
    finally:
        conn.close()
    if row is None:
        return jsonify({'error': '未找到该学号。'}), 404
    return jsonify({'data': row})


@app.route('/api/students/<sid>', methods=['PUT'])
def update_student(sid):
    """修改学生信息（姓名、两科成绩）。"""
    payload = request.get_json(silent=True) or {}
    ok, msg, avg = validate({**payload, 'id': sid})
    if not ok:
        return jsonify({'error': msg}), 400

    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT COUNT(*) AS n FROM students WHERE id=%s", (sid,))
            if cur.fetchone()['n'] == 0:
                return jsonify({'error': '未找到该学号。'}), 404
            cur.execute(
                "UPDATE students SET name=%s, c_score=%s, ds_score=%s, "
                "average=%s WHERE id=%s",
                (payload['name'], int(payload['c_score']),
                 int(payload['ds_score']), avg, sid))
        conn.commit()
    except pymysql.MySQLError as e:
        return jsonify({'error': f'数据库错误：{e}'}), 500
    finally:
        conn.close()
    return jsonify({'message': '修改成功', 'average': avg})


@app.route('/api/students/<sid>', methods=['DELETE'])
def delete_student(sid):
    """删除学生。"""
    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("DELETE FROM students WHERE id=%s", (sid,))
            if cur.rowcount == 0:
                return jsonify({'error': '未找到该学号。'}), 404
        conn.commit()
    except pymysql.MySQLError as e:
        return jsonify({'error': f'数据库错误：{e}'}), 500
    finally:
        conn.close()
    return jsonify({'message': '删除成功'})


# ---------- 搜索（按学号精确 / 按姓名模糊） ----------
@app.route('/api/students/search', methods=['GET'])
def search_students():
    """type=id 精确学号；type=name 姓名关键字模糊。"""
    q = request.args.get('q', '').strip()
    stype = request.args.get('type', 'id')
    if not q:
        return jsonify({'error': '搜索关键字不能为空。'}), 400

    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            if stype == 'id':
                cur.execute("SELECT id, name, c_score, ds_score, average "
                            "FROM students WHERE id=%s ORDER BY id", (q,))
            else:  # name 模糊
                like = f'%{q}%'
                cur.execute("SELECT id, name, c_score, ds_score, average "
                            "FROM students WHERE name LIKE %s ORDER BY id",
                            (like,))
            rows = cur.fetchall()
    finally:
        conn.close()
    return jsonify({'data': rows, 'total': len(rows)})


# ---------- 排序（按平均分降序，可指定字段） ----------
@app.route('/api/students/sorted', methods=['GET'])
def sorted_students():
    field = request.args.get('field', 'average')
    order = request.args.get('order', 'desc')
    allow = {'id', 'name', 'c_score', 'ds_score', 'average'}
    if field not in allow:
        field = 'average'
    if order not in ('asc', 'desc'):
        order = 'desc'
    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute(f"SELECT id, name, c_score, ds_score, average "
                        f"FROM students ORDER BY `{field}` {order.upper()}, id")
            rows = cur.fetchall()
    finally:
        conn.close()
    return jsonify({'data': rows, 'total': len(rows)})


# ---------- 统计 ----------
@app.route('/api/stats', methods=['GET'])
def statistics():
    """各科平均/最高/最低/及格率 + 分数段分布（优/良/中/及格/不及格）。"""
    conn = db.get_conn()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT COUNT(*) AS total FROM students")
            total = cur.fetchone()['total']
            if total == 0:
                return jsonify({'data': None})
            cur.execute("""
                SELECT
                  ROUND(AVG(c_score),2)  AS c_avg,
                  MAX(c_score)  AS c_max,  MIN(c_score)  AS c_min,
                  ROUND(SUM(c_score>=60)/COUNT(*)*100,1) AS c_pass,
                  ROUND(AVG(ds_score),2) AS ds_avg,
                  MAX(ds_score) AS ds_max, MIN(ds_score) AS ds_min,
                  ROUND(SUM(ds_score>=60)/COUNT(*)*100,1) AS ds_pass
                FROM students""")
            r = cur.fetchone()
            # 分数段：>=90 优，80-89 良，70-79 中，60-69 及格，<60 不及格
            cur.execute("""
                SELECT
                  SUM(c_score>=90)   AS c_ex, SUM(c_score BETWEEN 80 AND 89) AS c_go,
                  SUM(c_score BETWEEN 70 AND 79) AS c_mid,
                  SUM(c_score BETWEEN 60 AND 69) AS c_passg,
                  SUM(c_score<60)   AS c_fail,
                  SUM(ds_score>=90) AS ds_ex, SUM(ds_score BETWEEN 80 AND 89) AS ds_go,
                  SUM(ds_score BETWEEN 70 AND 79) AS ds_mid,
                  SUM(ds_score BETWEEN 60 AND 69) AS ds_passg,
                  SUM(ds_score<60) AS ds_fail
                FROM students""")
            seg = cur.fetchone()
    finally:
        conn.close()

    data = {
        'total': total,
        'subjects': {
            'C语言':   {'avg': r['c_avg'],  'max': r['c_max'],
                        'min': r['c_min'],  'pass': r['c_pass']},
            '数据结构': {'avg': r['ds_avg'], 'max': r['ds_max'],
                        'min': r['ds_min'], 'pass': r['ds_pass']},
        },
        'segments': {
            'C语言':   [seg['c_ex'], seg['c_go'], seg['c_mid'],
                        seg['c_passg'], seg['c_fail']],
            '数据结构': [seg['ds_ex'], seg['ds_go'], seg['ds_mid'],
                        seg['ds_passg'], seg['ds_fail']],
        },
    }
    return jsonify({'data': data})


# ---------- 启动 ----------
def main():
    db.init_db()
    print(f"数据库 {DB_NAME} 就绪。")
    print("学生成绩管理系统已启动：http://127.0.0.1:5000")
    app.run(host='127.0.0.1', port=5000, debug=True)


if __name__ == '__main__':
    main()
