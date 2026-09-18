from fastapi import FastAPI
from pydantic import BaseModel
# 导入我们写好的数据库函数
from database import save_task, get_task as db_get_task, update_task_result 
from celery_worker import run_cdp_task
import uuid
import datetime

app = FastAPI(title="CDP Solver API")

class SolveRequest(BaseModel):
    instance_path: str
    time_limit: float = 60.0
    seed: int = 1

@app.post("/solve")
def solve(req: SolveRequest):
    # 1. 生成任务ID
    task_id = str(uuid.uuid4())
    
    # 2. 立刻向 MySQL 插入一条 PENDING 记录（占坑）
    save_task(task_id, "PENDING", None)
    
    # 3. 把任务丢进 Celery 异步队列（毫秒级返回，不阻塞）
    run_cdp_task.delay(req.instance_path, req.time_limit, req.seed, task_id)
    
    # 4. 秒返回 task_id，用户拿着这个号去查结果
    return {"task_id": task_id, "status": "PENDING"}

@app.get("/tasks/{task_id}")
def get_task_api(task_id: str):
    # 3. 从 MySQL 查询（用改过名的 db_get_task，避免冲突）
    task = db_get_task(task_id)
    if task:
        return task
    return {"status": "not_found"}