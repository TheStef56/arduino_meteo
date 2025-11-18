import json, socket, threading
from Crypto.Cipher import AES
from flask import Flask, render_template
from mydb import *
from env import AES_KEY

app = Flask(__name__)
HOST = '0.0.0.0'
SOCKET_PORT = 9000
WEB_PORT = 5000

DATACHANGED = False

def socket_listener():
    global DATACHANGED
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind((HOST, SOCKET_PORT))
            s.listen(5)
            print(f"Socket server listening on port {SOCKET_PORT}...")
            while True:
                conn, _ = s.accept()
                data = conn.recv(1024)
                crypted = data[:36]
                nonce = data[36:48]
                auth_tag = data[48:]
                aesgcm = AES.new(AES_KEY, AES.MODE_GCM, nonce=nonce)
                decrypted = aesgcm.decrypt_and_verify(crypted, auth_tag)
                (wind_speed_open,
                wind_speed_close,
                wind_speed_high,
                wind_speed_low,
                temperature,
                humidity,
                bmp,
                battery,
                wind_dir) = unpack_data(decrypted)

                print(f"""
        wind_speed_open : {wind_speed_open}km/h
        wind_speed_close: {wind_speed_close}km/h
        wind_speed_high : {wind_speed_high}km/h
        wind_speed_low  : {wind_speed_low}km/h
        temperature     : {temperature}°C
        humidity        : {humidity}%
        bmp             : {bmp}hPa
        battery         : {battery}V
        wind_dir        : {wind_dir}
                """)
                conn.close()

                write_to_db(wind_speed_open, wind_speed_close, wind_speed_high, wind_speed_low,
                            temperature,
                            humidity,
                            bmp,
                            battery,
                            wind_dir
                            )
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
    return render_template("index.html")

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
