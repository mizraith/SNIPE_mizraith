# The following demo runs through various modes.  For full documentation refer to the StackLight readme.
#
# IF YOU DON'T SEE THE DEVICE YOU NEED A DRIVER
# Easiest way to get driver is to install Arduino software

import serial
import time

#sl = serial.Serial("COM7", 57600)         # 115200 / 57600 / 38400 / 19200  /  9600
#  PASSES TESTING AT 115200   If you change baud rate, re-test to make sure wait timing is not impacted.
sl = serial.Serial("/dev/cu.usbserial-A92517JR", 115200, timeout=0.25)
#sl = serial.Serial("/dev/cu.usbserial-A702Y4T1", 115200, timeout=0.25)
#sl = serial.Serial("/dev/tty.usbserial-A9H2AMEW", 115200, timeout=0.25)


# DELAY BETWEEN TX AND RX --- impacts failure rate and retries if you send messages too quick.
# Tests on an 8MHz nano over USB serial.   Tuned the nano delay loops to lower this number.
# When the nano heartbeat rate was set higher (25-50ms) the failure rate went up as well.
#     TURNED DOWN the heartbeat rate to 1ms and things stopped failing!!!
# results at  .05  -- no retries    51-55ms turnarounds.
# results at  .03  -- no retries, no garbled messages.  delta_ms around 33-35ms
# results at  .02  -- 0% retry rate. garbled input as we clobber prior message. delta_ms around 23-25ms
# results at  .01  -- 0% retry rate.  19-21ms turnarounds.  (nano heartbeat set to 5ms. When higher, failure rate went up too)
# results at  .005 -- 0% retry rate   10-26ms turnarounds
# results at  0    -- <0.25% retry rate    4-20ms turnarounds   <<<<<<<<<<<<<<<<<<<<<<<<  WINNER WINNER CHICKEN DINNER
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

# set mode to ON
cmd1 = lead + setmode + lightnum + delim + mode_on
send_command(cmd1)
cmd2 = lead + setmode + lightnum2 + delim + mode_on
send_command(cmd2)

# set the color using words   don't have to be all caps
colors = ["RED", "yellow", "ORANGE", "GREEN", "aqua", "VIOLET", "BLUE", "INDIGO"]
modes  = ["OFF", "STEADY", "FLASH", "PULSE", "RAINBOW"]   # again, don't have to be all caps

print(f"{'Setting Colors':-^80}")
for color in colors:
    cmd1 = lead + setcolor + lightnum + delim + color
    cmd2 = lead + setcolor + lightnum2 + delim + color
    send_command(cmd1)
    time.sleep(DELAY_BETWEEN)
    send_command(cmd2)
    time.sleep(.5)

cmd = lead + setcolor + lightnum + delim + "GREEN"
send_command(cmd)
cmd = lead + setcolor + lightnum2 + delim + "BLACK"
send_command(cmd)
time.sleep(0.5)



# set flash mode at 500ms
print(f"{'Setting Flash Mode':-^80}")            # yes, you can set two commands at a time if separated by a space
#   >SLM1:2 SLT1:1000
cmd = lead + setmode + lightnum + delim + mode_flash + " " + setcycletime + lightnum + delim + "1000"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(2)
cmd = lead + setmode + lightnum + delim + mode_flash + " " + setcycletime + lightnum + delim + "500"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(2)
cmd = lead + setmode + lightnum + delim + mode_flash + " " + setcycletime + lightnum + delim + "250"    # 500 is ms in flash cycle
send_command(cmd)
time.sleep(2)
# change color on the fly
print(f"{'Setting Back to Red':-^80}")
cmd = lead + setcolor + lightnum + delim + "RED"
send_command(cmd)
time.sleep(1)

# set pulse mode at 2000 ms
print(f"{'Setting Pulse Mode':-^80}")
cmd = lead + setmode + lightnum + delim + mode_pulse + " " + setcycletime + lightnum + delim + "2000"
send_command(cmd)
time.sleep(3)

print(f"{'Setting to 50%':-^80}")
# show percentage....not really useful for a stacklight inside a dome
cmd = lead + setpercentage + lightnum + delim + "50"
send_command(cmd)
time.sleep(1)

print(f"{'Stop pulsing':-^80}")
# whoops...forgot to take off of pulse...
cmd = lead + setmode + lightnum + delim + mode_on
send_command(cmd)
time.sleep(1)
print(f"{'Setting to 25%':-^80}")
# set to 25 %
cmd = lead + setpercentage + lightnum + delim + "25"
send_command(cmd)
time.sleep(1)
print(f"{'Setting to 100%':-^80}")
# set back to 100%
cmd = lead + setpercentage + lightnum + delim + "100"
send_command(cmd)
time.sleep(1)

# why not rainbow with a long interval
print(f"{'Setting Rainbow Mode':-^80}")
cmd = lead + setmode + lightnum + delim + mode_rainbow + " " + setcycletime + lightnum + delim + "5000"    # 5 second cycle
send_command(cmd)
time.sleep(2)

# turn off
print(f"{'Setting OFF':-^80}")
cmd = lead + setmode + lightnum + delim + mode_off
send_command(cmd)
time.sleep(1)

# back to steady green...we can set color while off
print(f"{'Leaving at green solid':-^80}")
cmd = lead + setcolor + lightnum + delim + "GREEN"
send_command(cmd)
time.sleep(DELAY_BETWEEN)
cmd = lead + setmode + lightnum + delim + mode_on
send_command(cmd)
time.sleep(1)

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

print(f"{'RAPID FIRE PERCENTAGE SETTING':-^80}")
ci = 0
for x in range(0, 100):
    print(f"\t\tPercentage: {x}")
    cmd = lead + setpercentage + lightnum2 + delim + str(x)
    send_command(cmd)
    # time.sleep(DELAY_BETWEEN)  # <<<< comment out for stress test rapid fire...faster than the full stacklight can show() update
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
cmd1 = lead + setmode + lightnum + delim + mode_rainbow + " " + setcycletime + lightnum + delim + "5000"
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
print(f"NUM_CMDS: {NUM_CMDS}\t\tTOTAL RETRIES => {TOTAL_RETRIES}")


sl.close()






