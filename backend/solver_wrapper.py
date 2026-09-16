import subprocess
import json
import os

def run_cdp_solver(instance_path: str, time_limit: float, seed: int):
    # 注意：exe 在上一级目录，所以路径用 ../
    exe_path = "../cdp_solver.exe"
    result_json_path = "result.json"  # 👈 统一用这个变量，全部指向 backend 文件夹

    # 核心改进1：前置清理旧文件（确保删的是 backend 目录下的文件）
    if os.path.exists(result_json_path):
        os.remove(result_json_path)

    # 构造命令行参数
    cmd = [exe_path, instance_path, str(time_limit), str(seed)]
    
    try:
        # 执行 C++ 程序，超时时间设为 time_limit + 60秒
        result = subprocess.run(
            cmd, 
            capture_output=True, 
            encoding='utf-8',
            errors='ignore', 
            timeout=time_limit + 60
        )
        
        if result.returncode != 0:
            return {"status": "error", "message": result.stderr}
        
        # 核心改进2：检查文件是否真的生成了
        if not os.path.exists(result_json_path):
            return {"status": "error", "message": "C++ 程序执行完毕，但未生成 result.json"}
        
        # 读取 C++ 生成的 result.json
        with open(result_json_path, "r") as f:
            data = json.load(f)
            
        return {"status": "success", "data": data}
        
    except subprocess.TimeoutExpired:
        return {"status": "error", "message": "算法执行超时"}
    except Exception as e:
        return {"status": "error", "message": str(e)}