
# ppg_plot.py
# real-time serial plotter for ppg samples
#
# expects stm32 to print one line per sample over uart in the following format:
# "adc_value/r/n"
#
# uses pyserial and matplotlib

# This file was originally used to plot accelerometer data which is why everything is generalized to number of channels


import serial
from collections import deque
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


PORT = "/dev/ttyACM0"
BAUDRATE = 115200
WINDOW_WIDTH = 500 # number of samples on screen at a time
NUM_CHANNELS = 1 # number of things we are plotting


ser = serial.Serial(PORT, BAUDRATE, timeout=1)
ser.reset_input_buffer()

channels = []
values = []
for i in range (NUM_CHANNELS):
    channels.append(deque([0] * WINDOW_WIDTH, maxlen=WINDOW_WIDTH))

def update(frame):
    while ser.in_waiting:
        raw_data = ser.readline()
        try:
            decoded_data = raw_data.decode("ascii").strip().split(",")
            if len(decoded_data) != NUM_CHANNELS:
                continue
            sample = [int(p) for p in decoded_data]
        except (UnicodeDecodeError, ValueError):
            continue
        for i in range(NUM_CHANNELS):
            channels[i].append(sample[i])
    return []

fig, ax = plt.subplots(1, 1, sharex=True)

labels = ["max30102 ppg adc value"]
lines = []
for i in range(NUM_CHANNELS):
    lines.append(ax.plot(range(WINDOW_WIDTH), channels[i], label=labels[i])[0])

ax.set_title("MAX30102 ADC Output")
ax.legend(loc="upper right")

def update_plot(frame):
    update(frame)
    for i in range(NUM_CHANNELS):
        lines[i].set_ydata(channels[i])
        # data = np.array(channels[i])
        # ax.set_ylim(data.min() - 50, data.max() + 200)
        ax.relim();
        ax.autoscale_view(scalex=False)
    return lines


try:
    anim = FuncAnimation(fig, update_plot, interval=50, cache_frame_data=False)
    plt.show()

except KeyboardInterrupt:
    print("keyboard interrupt detected")

finally:
    if ser.is_open:
        ser.close()
        print("serial port closed")