from fastapi import FastAPI
from pydantic import BaseModel
from solver_wrapper import run_cdp_solver
# 导入我们写好的数据库函数
from database import save_task, get_task as db_get_task  
import uuid
import datetime

app = FastAPI(title="CDP Solver API")

class SolveRequest(BaseModel):
    instance_path: str
    time_limit: float = 60.0
    seed: int = 1

@app.post("/solve")
def solve(req: SolveRequest):
    task_id = str(uuid.uuid4())
    
    # 1. 调用底层 C++ 求解器
    res = run_cdp_solver(req.instance_path, req.time_limit, req.seed)
    
    # 2. 存入 MySQL 数据库（直接调库，不再用内存字典）
    save_task(task_id, res["status"], res.get("data"))
    
    # 3. 如果失败，把具体原因也返回给网页！👇 这里是我们新增的改动
    if res["status"] == "error":
        return {
            "task_id": task_id, 
            "status": "error", 
            "message": res.get("message", "未知错误") 
        }
        
    return {"task_id": task_id, "status": "success"}

@app.get("/tasks/{task_id}")
def get_task_api(task_id: str):
    # 3. 从 MySQL 查询（用改过名的 db_get_task，避免冲突）
    task = db_get_task(task_id)
    if task:
        return task
    return {"status": "not_found"}