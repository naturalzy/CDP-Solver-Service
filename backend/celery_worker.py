# backend/celery_worker.py
from celery import Celery
from solver_wrapper import run_cdp_solver
from database import update_task_result

celery_app = Celery(
    "cdp_tasks",
    broker="redis://127.0.0.1:6379/0",
    backend="redis://127.0.0.1:6379/1"
)

# 👇 重点在这里！参数列表里必须加上 task_id: str
@celery_app.task(name="run_cdp_task")
def run_cdp_task(instance_path: str, time_limit: float, seed: int, task_id: str):
    """Celery 异步任务：调用 C++ 求解器并更新数据库"""
    result = run_cdp_solver(instance_path, time_limit, seed)
    
    if result["status"] == "success":
        update_task_result(task_id, "SUCCESS", result.get("data"))
    else:
        update_task_result(task_id, "ERROR", None)
    
    return result