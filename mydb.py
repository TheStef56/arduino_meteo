import struct, time, traceback

DATA_SIZE = 33 + 4 # 4 bytes of epoch
DB_LOCK = False

def check_for_lock():
    global DB_LOCK
    while DB_LOCK:
        time.sleep(0.01)

def unpack_data(data):
    wind_speed_open  = struct.unpack('f', data[:4])[0]
    wind_speed_close = struct.unpack('f', data[4:8])[0]
    wind_speed_high  = struct.unpack('f', data[8:12])[0]
    wind_speed_low   = struct.unpack('f', data[12:16])[0]
    temperature      = struct.unpack('f', data[16:20])[0]
    humidity         = struct.unpack('f', data[20:24])[0]
    bmp              = struct.unpack('f', data[24:28])[0]
    battery          = struct.unpack('f', data[28:32])[0]
    wind_dir         = data[32]
    return (wind_speed_open,
            wind_speed_close,
            wind_speed_high,
            wind_speed_low,
            temperature,
            humidity,
            bmp,
            battery,
            wind_dir)

                
def write_to_db(wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low,
                temperature,
                humidity,
                bmp,
                battery,
                wind_dir
                ):
    global DATA_SIZE, DB_LOCK
    check_for_lock()
    DB_LOCK = True
    try:
        db = open("database.bin", "ab+")
        old_data = db.read()
        if len(old_data) % DATA_SIZE != 0:
            print(f"ERROR: Invalid Database Size: {len(old_data)}")
            db.close()
            db = open("database.bin", "wb")
        data = bytearray()
        data += int(time.time()).to_bytes(4, byteorder='big')
        data += struct.pack('f', wind_speed_open)
        data += struct.pack('f', wind_speed_close)
        data += struct.pack('f', wind_speed_high)
        data += struct.pack('f', wind_speed_low)
        data += struct.pack('f', temperature)
        data += struct.pack('f', humidity)
        data += struct.pack('f', bmp)
        data += struct.pack('f', battery)
        data.append(wind_dir)
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
                epoch = int.from_bytes(window[:4], byteorder='big')
                window = window[4:]
                (wind_speed_open,
                wind_speed_close,
                wind_speed_high,
                wind_speed_low,
                temperature,
                humidity,
                bmp,
                battery,
                wind_dir) = unpack_data(window)
                result.append([
                    epoch,
                    wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low,
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