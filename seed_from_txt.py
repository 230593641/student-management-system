"""
迁移脚本：把旧的 students.txt（C 语言版文本数据）导入 MySQL。
用法：python seed_from_txt.py
格式：首行人数，之后每行：学号 姓名 C语言 数据结构 平均分
"""
import db


def load_txt(path='students.txt'):
    rows = []
    with open(path, 'r', encoding='utf-8') as f:
        lines = [ln for ln in f.read().splitlines() if ln.strip()]
    for ln in lines[1:]:
        parts = ln.split()
        if len(parts) < 4:
            continue
        sid, name = parts[0], parts[1]
        try:
            c = int(parts[2]); d = int(parts[3])
        except ValueError:
            continue
        avg = round((c + d) / 2.0, 2)
        rows.append((sid, name, c, d, avg))
    return rows


def main():
    rows = load_txt()
    if not rows:
        print('students.txt 中没有可导入的数据。')
        return
    conn = db.get_conn()
    added = 0
    try:
        with conn.cursor() as cur:
            for sid, name, c, d, avg in rows:
                cur.execute(
                    "SELECT COUNT(*) AS n FROM students WHERE id=%s", (sid,))
                if cur.fetchone()['n'] > 0:
                    print(f'跳过（已存在）：{sid}')
                    continue
                cur.execute(
                    "INSERT INTO students (id, name, c_score, ds_score, average) "
                    "VALUES (%s, %s, %s, %s, %s)", (sid, name, c, d, avg))
                added += 1
        conn.commit()
    finally:
        conn.close()
    print(f'导入完成：新增 {added} 条，跳过重复若干。')


if __name__ == '__main__':
    main()
