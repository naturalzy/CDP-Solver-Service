# backend/test_db.py
from database import save_task, get_task

print("开始手动测试插入...")
# 伪造一条数据
fake_data = {"objective": 999.9, "elapsed_time": 1.23, "solution_vector": [1, 0, 1]}
save_task("test_manual_001", "success", fake_data)

print("插入完成，准备查询...")
result = get_task("test_manual_001")
print("查询结果:", result)