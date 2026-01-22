import time, random, struct
from proto import WindData

DATA_SIZE = WindData.size

def write_to_db(now, windData: WindData):
    global DATA_SIZE
    try:
        db = open("database.bin", "ab+")
        old_data = db.read()
        if len(old_data) % DATA_SIZE != 0:
            print(f"ERROR: Invalid Database Size: {len(old_data)}")
            db.close()
            db = open("database.bin", "wb")
        data = bytearray()
        data += int(now).to_bytes(4, byteorder='little')
        data += windData.to_binary()
        db.write(data)
        db.close()
    except Exception as e:
        print(e)

now    = time.time()
now   -= 3600*24*30
times  = 12*24*3
for x in range(times): 
    now += 5*60+ (random.random()*120-60)
    write_to_db(
        now,
        WindData(
            windSpeedOpen  = random.random()*60,
            windSpeedClose = random.random()*60,
            windSpeedHigh  = random.random()*60,
            windSpeedLow   = random.random()*60,
            windMean       = random.random()*60,
            temperature    = random.random()*40 - 5,
            humidity       = random.random()*100,
            bmp            = random.random()*12 + 1012,
            batteryVolts   = random.random()*3 + 11,
            windDirection  = random.random()*360
        )
    )
