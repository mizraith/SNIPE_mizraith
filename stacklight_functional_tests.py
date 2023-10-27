# The following demo runs through various modes.  For full documentation refer to the StackLight readme.
#
# IF YOU DON'T SEE THE DEVICE YOU NEED A DRIVER
# Easiest way to get driver is to install Arduino software

import serial
import time

#sl = serial.Serial("COM7", 57600)         # 115200 / 57600 / 38400 / 19200  /  9600
sl = serial.Serial("/dev/cu.usbserial-A92517JR", 57600, timeout=0.25)      # AT 9600 we don't see the serial interrupt breaking the input string


lead = ">"
crlf = '\r\n'
query = '?'
delim = ":"

lightnum = "1"
lightnum2 = "2"

mode_off = "0"
mode_on = "1"
mode_flash = "2"
mode_pulse = "3"
mode_rainbow = "4"


setcolor = "SLC"
setmode = "SLM"
setpercentage = "SLP"

NUM_CMDS = 0
TOTAL_BAD = 0

def send_command(cmd):
    global NUM_CMDS
    global TOTAL_BAD

    tx = cmd + crlf
    start_ns = time.time_ns()
    retries = 0
    not_complete = True
    sl.write(tx.encode())
    time.sleep(0.03)
    print(f"TX: {cmd:<20}")

    # --- Receive Open Loop
    while(not_complete):
        rx = sl.readline().decode().rstrip()
        if rx == "":
            not_complete = False
        elif  rx.startswith("@"):
            not_complete = False
        print(f"{retries}\t\tRX: {rx:<20}")
        time.sleep(0.03)
        retries += 1

    # --- Receive Counting "Bads"
    # while(not_complete):
    #     sl.write(tx.encode())
    #     NUM_CMDS += 1
    #     #time.sleep(0.025)
    #     # @ 50ms TOTAL_BAD = 12
    #     # @ 25ms TOTAL_BAD = 64, 59
    #     # @ 10ms TOTAL_BAD = 20, 16
    #     # @ 0ms TOTAL_BAD = 15, 17, 18, 14, 22
    #
    #     rx = sl.readline().decode().rstrip()
    #     if rx[0] == "@" or retries > 5:
    #         not_complete = False
    #     else:
    #         retries += 1
    #         print(f"{retries}\ttx: {cmd}\t\ttx: {rx}")
    end_ns = time.time_ns()
    delta_ns = end_ns - start_ns
    delta_ms = round((end_ns - start_ns) / 10e5, 0)
    print(f"\tTX: {cmd:<20} \t RX:{rx:<20}\t\tdelta_ms: {delta_ms}\t\tRETRIES: {retries}")   #   s:{start_ns}  e:{end_ns}")
    TOTAL_BAD += retries

# When a stack light comes up - it starts in MODE 0 (off) and color BLACK (0x000000)
# We need to parse through its' preamble
while True:
    rx_line = sl.readline().decode().rstrip()
    print(rx_line)
    if "###DATA###" in rx_line:
        break
#  Done with our header

# set mode to ON
cmd1 = lead + setmode + lightnum + delim + mode_on
send_command(cmd1)
cmd2 = lead + setmode + lightnum2 + delim + mode_on
send_command(cmd2)

# set the color using words   don't have to be all caps
colors = ["RED", "yellow", "ORANGE", "GREEN", "aqua", "VIOLET", "BLUE", "INDIGO"]

print(f"{'Setting Colors':-^80}")
for color in colors:
    cmd1 = lead + setcolor + lightnum + delim + color
    cmd2 = lead + setcolor + lightnum2 + delim + color
    send_command(cmd1)
    time.sleep(.05)
    send_command(cmd2)
    time.sleep(1)

cmd = lead + setcolor + lightnum + delim + "GREEN"
send_command(cmd)
cmd = lead + setcolor + lightnum2 + delim + "BLACK"
send_command(cmd)
time.sleep(0.5)


# set flash mode at 500ms
print(f"{'Setting Flash Mode':-^80}")
cmd = lead + setmode + lightnum + delim + mode_flash + delim + "1000"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(3)
cmd = lead + setmode + lightnum + delim + mode_flash + delim + "500"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(3)
cmd = lead + setmode + lightnum + delim + mode_flash + delim + "250"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(3)
# change color on the fly
print(f"{'Setting Back to Red':-^80}")
cmd = lead + setcolor + lightnum + delim + "RED"
send_command(cmd)
time.sleep(2)



# set pulse mode at 2000 ms
print(f"{'Setting Pulse Mode':-^80}")
cmd = lead + setmode + lightnum + delim + mode_pulse + delim + "2000"
send_command(cmd)
time.sleep(5)

print(f"{'Setting to 50%':-^80}")
# show percentage....not really useful for a stacklight inside a dome
cmd = lead + setpercentage + lightnum + delim + "50"
send_command(cmd)
time.sleep(2)

print(f"{'Stop pulsing':-^80}")
# whoops...forgot to take off of pulse...
cmd = lead + setmode + lightnum + delim + mode_on
send_command(cmd)
time.sleep(2)

print(f"{'Setting to 25%':-^80}")
# set to 25 %
cmd = lead + setpercentage + lightnum + delim + "25"
send_command(cmd)
time.sleep(2)
print(f"{'Setting to 100%':-^80}")
# set back to 100%
cmd = lead + setpercentage + lightnum + delim + "100"
send_command(cmd)
time.sleep(2)

# why not rainbow with a long interval
print(f"{'Setting Rainbow Mode':-^80}")
cmd = lead + setmode + lightnum + delim + mode_rainbow + delim + "5000"    # 5 second cycle
send_command(cmd)
time.sleep(5)

# turn off
print(f"{'Setting OFF':-^80}")
cmd = lead + setmode + lightnum + delim + mode_off
send_command(cmd)
time.sleep(0.25)

# back to steady green...we can set color while off
print(f"{'Leaving at green solid':-^80}")
cmd = lead + setcolor + lightnum + delim + "GREEN"
send_command(cmd)
time.sleep(0.1)
cmd = lead + setmode + lightnum + delim + mode_on
send_command(cmd)

# Work with StackLight #2
print(f"{'Setting up, working with StackLight #2':-^80}")
# color
cmd1 = lead + setcolor + lightnum + delim + colors[0]
cmd2 = lead + setcolor + lightnum2 + delim + colors[0]
send_command(cmd1)
send_command(cmd2)
# percentage
cmd = lead + setpercentage + lightnum2 + delim + "0"
send_command(cmd)
# mode
cmd = lead + setmode + lightnum2 + delim + mode_on
send_command(cmd)

ci = 0
for x in range(0, 100):
    print(f"\t\tPercentage: {x}")
    cmd = lead + setpercentage + lightnum2 + delim + str(x)
    send_command(cmd)
    time.sleep(0.1)
    if x % len(colors) == 0:
        ci += 1
        if ci >= len(colors):
            ci = 0
        cmd1 = lead + setcolor + lightnum + delim + colors[ci-1]
        cmd2 = lead + setcolor + lightnum2 + delim + colors[ci-1]
        send_command(cmd1)
        send_command(cmd2)

time.sleep(1)
print(f"{'Percentage Complete -- setting dark':-^80}")
cmd2 = lead + setmode + lightnum2 + delim + mode_off
cmd1 = lead + setmode + lightnum + delim + mode_rainbow + delim + "5000"
send_command(cmd2)
send_command(cmd1)

print(f"{'Getting Info':-^80}")
cmd = ">SLINFO" + crlf
sl.write(cmd.encode())
while True:
    try:
        rx = sl.readline().decode().rstrip()
        print(rx)
        if rx == "@SLINFO":
            break
        if rx[0] == "@":
            break
    except Exception as e:
        print(e)
        break

print(f"{'TOTALS':-^80}")
print(f"NUM_SENDS: {NUM_CMDS}\t\tTOTAL BAD => {TOTAL_BAD}")


sl.close()






