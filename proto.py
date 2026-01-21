import struct

class WindData:
    def __init__(self, windSpeedOpen=None, windSpeedClose=None, windSpeedHigh=None, windSpeedLow=None, windMean=None, temperature=None, humidity=None, bmp=None, batteryVolts=None, windDirection=None, ):
        self.windSpeedOpen = windSpeedOpen
        self.windSpeedClose = windSpeedClose
        self.windSpeedHigh = windSpeedHigh
        self.windSpeedLow = windSpeedLow
        self.windMean = windMean
        self.temperature = temperature
        self.humidity = humidity
        self.bmp = bmp
        self.batteryVolts = batteryVolts
        self.windDirection = windDirection
        self.size = 40

    def from_binary(self, data):
        self.windSpeedOpen = struct.unpack('f', data[0:4])[0]
        self.windSpeedClose = struct.unpack('f', data[4:8])[0]
        self.windSpeedHigh = struct.unpack('f', data[8:12])[0]
        self.windSpeedLow = struct.unpack('f', data[12:16])[0]
        self.windMean = struct.unpack('f', data[16:20])[0]
        self.temperature = struct.unpack('f', data[20:24])[0]
        self.humidity = struct.unpack('f', data[24:28])[0]
        self.bmp = struct.unpack('f', data[28:32])[0]
        self.batteryVolts = struct.unpack('f', data[32:36])[0]
        self.windDirection = struct.unpack('f', data[36:40])[0]

    def get_values(self):
        return (
            self.windSpeedOpen,
            self.windSpeedClose,
            self.windSpeedHigh,
            self.windSpeedLow,
            self.windMean,
            self.temperature,
            self.humidity,
            self.bmp,
            self.batteryVolts,
            self.windDirection,
        )

    def get_binary(self):
        data = bytearray()
        data += struct.pack('f', self.windSpeedOpen)
        data += struct.pack('f', self.windSpeedClose)
        data += struct.pack('f', self.windSpeedHigh)
        data += struct.pack('f', self.windSpeedLow)
        data += struct.pack('f', self.windMean)
        data += struct.pack('f', self.temperature)
        data += struct.pack('f', self.humidity)
        data += struct.pack('f', self.bmp)
        data += struct.pack('f', self.batteryVolts)
        data += struct.pack('f', self.windDirection)
        return data

