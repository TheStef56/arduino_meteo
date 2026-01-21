import time, traceback
from proto import WindData

DATA_SIZE = WindData().size + 4
DB_LOCK = False

TIME_FRAME = 5
MAX_TIME_SPAN = 60*60*24*30
MAX_SIZE = int(MAX_TIME_SPAN*DATA_SIZE/TIME_FRAME)


def check_for_lock():
    global DB_LOCK
    while DB_LOCK:
        time.sleep(0.01)

def truncate_db_size():
    global DB_LOCK, MAX_SIZE, DATA_SIZE
    check_for_lock()
    DB_LOCK = True
    data = []
    try:
        with open("database.bin", "rb") as db:
            data = db.read()
            old_size = len(data)
            if  old_size > MAX_SIZE:
                new_size = int(((MAX_SIZE * 0.1)//DATA_SIZE)*DATA_SIZE)
                data = data[old_size - new_size:]
        with open("database.bin", "wb") as db:
            if len(data) > 0:
                db.write(data)
    except Exception as e:
        traceback.print_exception(e)
    DB_LOCK = False

                
def write_to_db(wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low, wind_mean,
                temperature,
                humidity,
                bmp,
                battery,
                wind_dir
                ):
    global DATA_SIZE, DB_LOCK
    check_for_lock()
    truncate_db_size()
    DB_LOCK = True
    try:
        db = open("database.bin", "ab+")
        old_data = db.read()
        if len(old_data) % DATA_SIZE != 0:
            print(f"ERROR: Invalid Database Size: {len(old_data)}")
            db.close()
            db = open("database.bin", "wb")
        data = bytearray()
        data += int(time.time()).to_bytes(4, byteorder='little')
        w = WindData(wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low, wind_mean, temperature, humidity, bmp, battery, wind_dir)
        data += w.get_binary()
        db.write(data)
        db.close()
    except Exception as e:
        traceback.print_exception(e)
    DB_LOCK = False

def read_from_db():
    global DATA_SIZE, DB_LOCK
    check_for_lock()
    DB_LOCK = True
    result = []
    try:
        with open("database.bin", "rb") as db:
            dbbytes = db.read()
            if len(dbbytes) % DATA_SIZE != 0:
                print(f"ERROR: Invalid Database Size: {len(dbbytes)}")
            start = 0
            end = DATA_SIZE
            while end <= len(dbbytes):
                window = dbbytes[start:end]
                start += DATA_SIZE
                end += DATA_SIZE
                epoch = int.from_bytes(window[:4], byteorder='little')
                window = window[4:]
                w = WindData()
                w.from_binary(window)
                (wind_speed_open,
                wind_speed_close,
                wind_speed_high,
                wind_speed_low,
                wind_mean,
                temperature,
                humidity,
                bmp,
                battery,
                wind_dir) = w.get_values()
                result.append([
                    epoch,
                    wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low, wind_mean,
                    temperature,
                    humidity,
                    bmp,
                    battery,
                    wind_dir,
                ])
    except Exception as e:
        traceback.print_exception(e)
    DB_LOCK = False
    return result