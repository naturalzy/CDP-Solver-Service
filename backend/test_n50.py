import requests
import json

url = "http://127.0.0.1:8000/solve"
# 这次路径明确指向 n50 小实例
payload = {
    "instance_path": "../CDP_Instances/CDP_Instances_b02/GKD-b_11_n50_b02_m5.txt",
    "time_limit": 10,
    "seed": 1
}
headers = {"Content-Type": "application/json"}

print("正在请求 n50 实例，请等待...")
response = requests.post(url, data=json.dumps(payload), headers=headers)
print("请求完成！")
print("状态码:", response.status_code)
print("返回结果:", response.json())