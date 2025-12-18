import time, random, struct
with open("data.size", "r") as f:
    DATA_SIZE = int(f.read()) 

def write_to_db(now,
                temperature,
                humidity,
                wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low, wind_speed_mean,
                wind_dir,
                bmp,
                battery
                ):
    global DATA_SIZE
    try:
        db = open("database.bin", "ab+")
        old_data = db.read()
        if len(old_data) % DATA_SIZE != 0:
            print(f"ERROR: Invalid Database Size: {len(old_data)}")
            db.close()
            db = open("database.bin", "wb")
        data = bytearray()
        data += int(now).to_bytes(4, byteorder='big')
        data += struct.pack('f', wind_speed_open)
        data += struct.pack('f', wind_speed_close)
        data += struct.pack('f', wind_speed_high)
        data += struct.pack('f', wind_speed_low)
        data += struct.pack('f', wind_speed_mean)
        data += struct.pack('f', temperature)
        data += struct.pack('f', humidity)
        data += struct.pack('f', bmp)
        data += struct.pack('f', battery)
        data += struct.pack('f', wind_dir)
        db.write(data)
        db.close()
    except Exception as e:
        print(e)

now = time.time()
now -= 3600*24*30
times = 12*24*30
for x in range(times):
    now += 5*60+ (random.random()*120-60)
    write_to_db(
        now,
        wind_speed_open  = random.random()*60,
        wind_speed_close = random.random()*60,
        wind_speed_high  = random.random()*60,
        wind_speed_low   = random.random()*60,
        wind_speed_mean  = random.random()*60,
        temperature      = random.random()*40 - 5,
        humidity         = random.random()*100,
        bmp              = random.random()*12 + 1012,
        battery          = random.random()*3 + 11, 
        wind_dir         = random.random()*360
    )
