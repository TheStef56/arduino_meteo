#!/usr/bin/python3 -u
import json, socket, threading, sys, asyncio
from datetime import datetime
from Crypto.Cipher import AES
from flask import Flask, render_template, request
from mydb import *
from env import *
from proto import WindData
from datetime import datetime
from werkzeug.middleware.proxy_fix import ProxyFix
from pyrogram import Client

# env.py example:

# KEYFILE="/etc/letsencrypt/live/mydomain/privkey.pem"
# CERTFILE="/etc/letsencrypt/live/mydomain/fullchain.pem"
# PATH = "~/arduino_meteo"
# HOST = '0.0.0.0'
# SOCKET_PORT = 9000
# WEB_PORT = 5000
# AES_KEY = bytes([1,1,1,1,1,1,1,1
#                 1,1,1,1,1,1,1,1,1,
#                  1,1,1,1,1,1,1,1
#                 1,1,1,1,1,1,1,1,1,
#                  ])

app = Flask(__name__)
app.wsgi_app = ProxyFix(
    app.wsgi_app,
    x_for=1,
    x_proto=1,
    x_host=1
)

DATA_SIZE = WindData.size 

SEND_EMERGENCY_MESSAGE = False
LAST_RECEIVED = datetime.now().timestamp()

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
    "UTC+2 (CEST/EET)",
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

# SOCKET -----------------------------------------------------------------------------------------

def socket_listener():
    global LAST_RECEIVED, SEND_EMERGENCY_MESSAGE
    socket_timeout = 130 # seconds
    connection_timeout = 30 # seconds
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, SOCKET_PORT))
            s.listen()
            s.settimeout(socket_timeout)
            print(f"Socket server listening on port {SOCKET_PORT}...")
            while True:
                try:
                    conn, addr = s.accept()
                    conn.settimeout(connection_timeout)
                except TimeoutError:
                    if datetime.now().timestamp() - LAST_RECEIVED >= 60*8: #8 min
                        SEND_EMERGENCY_MESSAGE = True
                    break
                except Exception as e:
                    traceback.print_exception(e)
                    time.sleep(5)
                    continue

                try:
                    data = conn.recv(1024)
                except TimeoutError:
                    print(f"connection was open for more than {connection_timeout} sec. closing it. r_addr: {addr}")
                    conn.close()
                    continue
                except Exception as e:
                    conn.close()
                    traceback.print_exception(e)
                    continue

                try:
                    crypted = data[:DATA_SIZE]
                    nonce = data[DATA_SIZE:DATA_SIZE+12]
                    auth_tag = data[DATA_SIZE+12:]
                    aesgcm = AES.new(AES_KEY, AES.MODE_GCM, nonce=nonce)
                    decrypted = aesgcm.decrypt_and_verify(crypted, auth_tag)
                except ValueError:
                    conn.close()
                    print(f"ValueError: probably garbage was sent. Ignoring. r_addr: {addr}")
                    continue
                except Exception as e:
                    conn.close()
                    traceback.print_exception(e)
                    continue
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
                write_to_db(w)
                LAST_RECEIVED = datetime.now().timestamp()
        except Exception as e:
            traceback.print_exception(e)
        finally:
            try:
                conn.close()
            except:
                pass
            s.close()
            time.sleep(1)

# ROUTES ------------------------------------------------------------------------------------

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
    client_size = request.args.get("size")
    db_size = str(len(read_from_db()))
    if client_size != db_size:
        return "yes"
    else:
        return "no"

# Proxy logger -----------------------------------------------------------------------------------

from gevent.pywsgi import WSGIServer, WSGIHandler
from gevent import ssl

class ProxyFixHandler(WSGIHandler):
    def format_request(self):
        forwarded_for = self.headers.get("X-Forwarded-For")

        if forwarded_for:
            client_ip = forwarded_for.split(",")[0].strip()
        else:
            client_ip = self.client_address[0]

        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        return '%s - - [%s] "%s" %s %s' % (
            client_ip,
            now,
            self.requestline,
            self.status,
            self.response_length or '-'
        )
    
class WSGIServerWithLog(WSGIServer):
    def handle_error(self, r, client_address):
        exc = r
        if isinstance(exc, ssl.SSLError):
            msg = str(exc)
            if "HTTP_REQUEST" in msg:
                print("TEST")
                return

        # fallback to default logging
        super().handle_error(r, client_address)
    
# TELEGRAM EMERGENCY MESSAGE -------------------------------------------------------------------

TELEGRAM_APP = Client("my_account", api_id=API_ID, api_hash=API_HASH, bot_token=TOKEN)

async def telegram_worker():
    global SEND_EMERGENCY_MESSAGE, TELEGRAM_APP
    while True:
        if SEND_EMERGENCY_MESSAGE is True:
            SEND_EMERGENCY_MESSAGE = False
            await TELEGRAM_APP.send_message(CHAT, "We have not received updates in a while!")
            print("EMERGENCY TELEGRAM MESSAGE SENT!")
        await asyncio.sleep(1)

# ---------------------------------------------------------------------------------------------

async def main():
    from env import KEYFILE, CERTFILE
    def run_server():
        http_server = WSGIServerWithLog(
            (HOST, WEB_PORT),
            app,
            handler_class=ProxyFixHandler,
            keyfile=KEYFILE,
            certfile=CERTFILE,
            log=sys.stderr
        )
        http_server.serve_forever()

    threading.Thread(target=socket_listener, daemon=True).start() 
    threading.Thread(target=run_server, daemon=True).start() 
    
    await TELEGRAM_APP.start()
    await asyncio.create_task(telegram_worker())

if __name__ == '__main__':
    asyncio.run(main())