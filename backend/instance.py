# backend/instance.py
import os

class InstanceData:
    """对应 C++ 里的 struct instance_data"""
    def __init__(self):
        self.num_v = 0          # 节点数量
        self.capacity = 0.0     # 容量约束
        self.weight = []        # 每个节点的权重 (List[float])
        self.distance = []      # 距离矩阵 (List[List[float]])

def read_instance(instance_path: str) -> InstanceData:
    """读取 CDP 实例文件，返回 InstanceData 对象"""
    if not os.path.exists(instance_path):
        raise FileNotFoundError(f"实例文件不存在: {instance_path}")
    
    ins = InstanceData()
    
    with open(instance_path, 'r') as f:
        # 读取所有数字，自动处理空格和换行
        tokens = f.read().split()
    
    idx = 0
    ins.num_v = int(tokens[idx])
    idx += 1
    ins.capacity = float(tokens[idx])
    idx += 1
    
    # 读取权重
    ins.weight = [float(tokens[idx + i]) for i in range(ins.num_v)]
    idx += ins.num_v
    
    # 读取距离矩阵
    for i in range(ins.num_v):
        row = [float(tokens[idx + j]) for j in range(ins.num_v)]
        ins.distance.append(row)
        idx += ins.num_v
        
    return ins

def free_instance(ins: InstanceData):
    """Python 有自动垃圾回收，其实不需要手动释放，但为了和 C++ 逻辑对应，写个空函数占位"""
    pass