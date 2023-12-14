# The following demo runs through various modes.  For full documentation refer to the StackLight readme.
#
# IF YOU DON'T SEE THE DEVICE YOU NEED A DRIVER
# Easiest way to get driver is to install Arduino software

import serial
import time

#sl = serial.Serial("COM7", 57600)         # 115200 / 57600 / 38400 / 19200  /  9600
#  PASSES TESTING AT 115200   If you change baud rate, re-test to make sure wait timing is not impacted.
sl = serial.Serial("/dev/cu.usbserial-A9H2AMEW", 115200, timeout=0.25)
DELAY_BETWEEN = 0.001

READY_STRING = "#####READY#####"    # NEW IN V4

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

serialnumber = "SN"
uniqueID = "UID"

setcolor = "SLC"
setmode = "SLM"
setcycletime = "SLT"
setpercentage = "SLP"

NUM_CMDS = 0
TOTAL_RETRIES = 0

def send_command(cmd):
    global NUM_CMDS
    global TOTAL_RETRIES

    tx = cmd + crlf
    start_ns = time.time_ns()
    retries = 0
    NUM_CMDS += 1
    not_complete = True
    sl.write(tx.encode())

    print(f"\n___SEND___:\tTX: {cmd:<20}")

    # --- Receive Open Loop
    while(not_complete):
        time.sleep(DELAY_BETWEEN)
        rx = sl.readline().decode().rstrip()
        if rx == "":
            not_complete = False
        elif rx.startswith("#"):
            # it's a debug response or comment
            print(f"\t\tdebug resp:\t{rx}")
            continue
        elif  rx.startswith("@"):
            not_complete = False
        print(f"\t\tretry#:{retries}\t\t  response:\t{rx:<20}")
        retries += 1

    retries = retries - 1   # correct for do-while above

    end_ns = time.time_ns()
    delta_ns = end_ns - start_ns
    delta_ms = round((end_ns - start_ns) / 10e5, 0)
    print(f"___RESULT___:\tTX: {cmd:<20}\tRX:\t{rx:<20}\t\tdelta_ms: {delta_ms}\t\tRETRIES: {retries}")   #   s:{start_ns}  e:{end_ns}")
    TOTAL_RETRIES += retries

# When a stack light comes up - it starts in MODE 0 (off) and color BLACK (0x000000)
# We need to parse through its' preamble
while True:
    rx_line = sl.readline().decode().rstrip()
    print(rx_line)
    if READY_STRING in rx_line:
        break
#  Done with our header

# Let's get our UID and Serial Number for example
print(f"{'Getting UniqueID and Serial Number':-^80}")
cmd = lead + uniqueID
send_command(cmd)
cmd = lead + serialnumber
send_command(cmd)

time.sleep(0.05)
cmd = lead + "SLINFO"
send_command(cmd)
print(f"{'TOTALS':-^80}")
print(f"NUM_CMDS: {NUM_CMDS}\t\tTOTAL RETRIES => {TOTAL_RETRIES}")


sl.close()






