import json, socket, threading
from datetime import datetime
from Crypto.Cipher import AES
from flask import Flask, render_template
from mydb import *
from env import AES_KEY, HOST, SOCKET_PORT, WEB_PORT
from proto import WindData

app = Flask(__name__)
DATA_SIZE = WindData.size 

DATACHANGED = False

themes = [
    "Light",
    "Dark"
]

arrangements = [
    "Landscape",
    "Grid"
]

time_frames = [
    "Unfiltered",
    "5m",
    "30m",
    "1h",
    "3h",
    "6h",
    "12h",
    "1d",
    "7d",
]

time_zones = [
    "UTC-12 (B|IT)",
    "UTC-11 (SST)",
    "UTC-10 (HST)",
    "UTC-9 (AKST)",
    "UTC-8 (PST)",
    "UTC-7 (MST)",
    "UTC-6 (CST)",
    "UTC-5 (EST)",
    "UTC-4 (AST)",
    "UTC-3 (ART)",
    "UTC-2 (GST)",
    "UTC-1 (AZOT)",
    "UTC-0 (GMT)",
    "UTC+1 (CET)",
    "UTC+2 (EET)",
    "UTC+3 (MSK)",
    "UTC+4 (GST)",
    "UTC+5 (PKT)",
    "UTC+6 (BST)",
    "UTC+7 (ICT)",
    "UTC+8 (CST)",
    "UTC+9 (JST)",
    "UTC+10 (AEST)",
    "UTC+11 (SBT)",
    "UTC+12 (NZST)",
    "UTC+13 (TOT)",
    "UTC+14 (LINT)"
]

def socket_listener():
    global DATACHANGED
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind((HOST, SOCKET_PORT))
            s.listen(5)
            s.settimeout(60)
            print(f"Socket server listening on port {SOCKET_PORT}...")
            while True:
                try:
                    conn, _ = s.accept()
                except TimeoutError:
                    continue
                except Exception as e:
                    traceback.print_exception(e)
                    continue
                data = conn.recv(1024)
                crypted = data[:DATA_SIZE]
                nonce = data[DATA_SIZE:DATA_SIZE+12]
                auth_tag = data[DATA_SIZE+12:]
                aesgcm = AES.new(AES_KEY, AES.MODE_GCM, nonce=nonce)
                decrypted = aesgcm.decrypt_and_verify(crypted, auth_tag)
                w = WindData()
                w.from_binary(decrypted)

                print(f"""
        timestamp:      : {datetime.now().strftime("%H:%M:%S")}
        wind_speed_open : {w.windSpeedOpen}km/h
        wind_speed_close: {w.windSpeedClose}km/h
        wind_speed_high : {w.windSpeedHigh}km/h
        wind_speed_low  : {w.windSpeedLow}km/h
        wind_mean       : {w.windMean}km/h
        temperature     : {w.temperature}°C
        humidity        : {w.humidity}%
        bmp             : {w.bmp}hPa
        battery         : {w.batteryVolts}V
        wind_dir        : {w.windDirection}
                """)
                conn.close()

                write_to_db(*w.get_values())
                DATACHANGED = True
        except Exception as e:
            traceback.print_exception(e)
        finally:
            try:
                conn.close()
            except:
                pass

@app.route('/')
def home(): 
    return render_template("index.html", themes       = themes,
                                         arrangements = arrangements,
                                         time_frames  = time_frames,
                                         time_zones   = time_zones)

@app.route('/data')
def data():
    return json.dumps(read_from_db())

@app.route('/data-changed')
def data_changed():
    global DATACHANGED
    if DATACHANGED:
        DATACHANGED = False
        return "yes"
    else:
        return "no"

if __name__ == '__main__':
    threading.Thread(target=socket_listener, daemon=True).start()
    app.run(host=HOST, port=WEB_PORT)
