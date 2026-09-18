import redis

try:
    r = redis.Redis(host='127.0.0.1', port=6379, db=0, protocol=2)
    if r.ping():
        print("Redis 连接成功！")
    else:
        print("Redis 连接失败，请检查 Redis 服务是否开启。")
except Exception as e:
    print(f"Redis 连接报错: {e}")