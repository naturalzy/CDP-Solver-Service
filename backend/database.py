# backend/database.py
import pymysql
import json

# ⚠️ 密码必须是 123456，跟刚才登录 MySQL 用的一样
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': '123456', 
    'database': 'cdp_db',
    'charset': 'utf8mb4',
    'cursorclass': pymysql.cursors.DictCursor
}

def save_task(task_id, status, result_data):
    conn = pymysql.connect(**DB_CONFIG)
    try:
        with conn.cursor() as cursor:
            objective = result_data.get('objective') if result_data else None
            elapsed_time = result_data.get('elapsed_time') if result_data else None
            solution_vector = json.dumps(result_data.get('solution_vector')) if result_data else None
            
            sql = """
                INSERT INTO tasks (task_id, status, objective, elapsed_time, solution_vector)
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(sql, (task_id, status, objective, elapsed_time, solution_vector))
        conn.commit()
    finally:
        conn.close()

def get_task(task_id):
    conn = pymysql.connect(**DB_CONFIG)
    try:
        with conn.cursor() as cursor:
            sql = "SELECT * FROM tasks WHERE task_id = %s"
            cursor.execute(sql, (task_id,))
            row = cursor.fetchone()
            if row:
                if row['solution_vector']:
                    row['solution_vector'] = json.loads(row['solution_vector'])
                return row
            return None
    finally:
        conn.close()