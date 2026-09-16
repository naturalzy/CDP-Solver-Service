import requests
import json

url = "http://127.0.0.1:8000/solve"
payload = {
    "instance_path": "../CDP_Instances/CDP_Instances_b02/MDG-b_10_n500_b02_m50.txt",
    "time_limit": 10,
    "seed": 1
}
headers = {
    "Content-Type": "application/json"
}

print("正在请求 FastAPI，请耐心等待约 50 秒...")
response = requests.post(url, data=json.dumps(payload), headers=headers)
print("请求完成！")
print("状态码:", response.status_code)
print("返回结果:", response.json())