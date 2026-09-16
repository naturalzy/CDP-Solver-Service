# backend/run_test.py
from instance import read_instance

if __name__ == "__main__":
    # 注意：路径必须是你电脑上真实存在的路径
    path = "../CDP_Instances/CDP_Instances_b02/MDG-b_10_n500_b02_m50.txt"
    
    try:
        ins = read_instance(path)
        print("读取成功！")
        print(f"节点数: {ins.num_v}")
        print(f"容量: {ins.capacity}")
        print(f"权重前5个: {ins.weight[:5]}")
        print(f"距离矩阵第一行前5个: {ins.distance[0][:5]}")
    except Exception as e:
        print(f"报错了: {e}")