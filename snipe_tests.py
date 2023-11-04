# /bin/python
# -*- coding: utf-8 -*-
#
# TEST SUITE FOR SNIPE 
#   UPDATED to use Python3 syntax adn Python3 pyserial.
#
# To use the test suite, first load up your Arduino with SNIPE and
# make a note of the comm port (e.g. /dev/tty.usbmodem1411).
#
# From your command line, launch the python script as main with
# the port as the first argument, e.g.:
#
#            python SNIPE_tests.py /dev/tty.usbmodem1411
#
# Sit back and watch python hammer your little Arduino and verify
# that the comm functions and grammar are all correct.  The script
# provides a full printout of the comm transactions to aid in
# debugging.
#

import os
import sys
import time
import unittest
from datetime import datetime, timedelta
import serial
from optparse import OptionParser

DATA_STRING = "#####DATA#####"
VALUE_MISSING = "VALUE_MISSING"
VALUE_ERROR = "VALUE_ERROR"
Q_REQUIRED = "?_MISSING"
ERR_SID_TOO_LONG = "SID_>_30"
OUT_OF_RANGE = "OUT_OF_RANGE"
BYTE_SETTING_ERR = "BYTE_SETTING_ERR"
NONE_FOUND = "NONE_FOUND"
DATA_LENGTH_ERR = "DATA_LENGTH_ERR"

colors = {"RED": "0xFF0000",
          "ORANGE": "0xFF5500",
          "YELLOW": "0xFFFF00",
          "GREEN": "0x00FF00",
          "AQUA": "0x00FFFF",
          "BLUE": "0x0000FF",
          "INDIGO": "0x3300FF",
          "VIOLET": "0xFF00FF",
          "WHITE": "0xFFFFFF",
          "BLACK": "0x000000"}


class SnipeTests(unittest.TestCase):
    #    def setUp(self):
    #        pass
    #    def tearDown(self):
    #        pass

    port = ""
    baudrate = 115200
    timeout = 1
    ser = serial.Serial()

    @classmethod
    def setUpClass(cls):
        """
        Init function for all the test cases in this class.  Called once.
        """
        in_header_mode = True
        try:
            SnipeTests.ser = serial.Serial(port=SnipeTests.port,
                                           baudrate=SnipeTests.baudrate,
                                           timeout=SnipeTests.timeout)
            if not SnipeTests.ser.isOpen():
                raise Exception("Serial port is not open: " + SnipeTests.port)

            while in_header_mode:
                resp = SnipeTests.ser.readline().decode()
                print("resp:\t" + resp.strip())
                if DATA_STRING in resp:
                    in_header_mode = False

        except Exception as e:
            print("ERROR SETTING UP SERIAL PORT: ")
            print(e.args)
        finally:
            pass

    @classmethod
    def tearDownClass(cls):
        """
        Common tear down function for all the test cases in this class.  Called once.
        :return:
        """
        try:
            SnipeTests.ser.close()
        except Exception as e:
            print("ERROR CLOSING SERIAL PORT: ")
            print(e.args)

    def _handle_cmd_exp(self, cmd, exp):
        """
        Assert that given a command (cmd) we get the expected (exp) response.
        :param cmd: command to send
        :param exp: response we expect, or expect to be part of the response.
        :return:
        """
        print("cmd:\t%s" % cmd)
        tx = cmd + "\r\n"
        start_ns = time.time_ns()
        SnipeTests.ser.write(tx.encode())
        time.sleep(0.03)
        resp = SnipeTests.ser.readline().decode()
        end_ns = time.time_ns()
        delta_ms = round((end_ns - start_ns) / 10e5, 0)
        while resp:
            if resp.startswith("#"):
                print("#:\t%s" % resp.strip())
                resp = SnipeTests.ser.readline().decode()
            else:
                break
        print(f"exp:\t{exp}")
        print(f"resp:\t{resp.strip()}")
        print(f"(ms):\t{delta_ms}")
        try:
            self.assertTrue(exp in resp, f"\nexpected: {exp}   but got: {resp}\n\n\t\t\t\t!!!FAIL!!!")
            print("\t\t\t\t===PASS===")
        except AssertionError as a:
            print("\t\t\t\t!!!FAIL!!!")
            raise a

    def _test_value_missing_conditions(self, cmd):
        """
        For a given command, test improperly formatted
        or null value conditions.
        :param cmd:  e.g. "A0"   or "D2"
        """
        xcmd = ">" + cmd
        exp = "!" + cmd + ":" + VALUE_MISSING
        self._handle_cmd_exp(xcmd, exp)
        xcmd = ">" + cmd + ":"
        exp = "!" + cmd + ":" + VALUE_MISSING
        self._handle_cmd_exp(xcmd, exp)

    def _test_q_required(self, cmd):
        """
        For a given query-only command, check what
        happens when we provide something other than a ?
        :param cmd: e.g. "A0"  or "VER"
        :return:
        """
        xcmd = ">" + cmd + ":1"
        exp = "!" + cmd + ":" + Q_REQUIRED
        self._handle_cmd_exp(xcmd, exp)

    def test_improper_command(self):
        # test poor formatted command
        cmd = "?"
        exp = "!Invalid_input:"
        self._handle_cmd_exp(cmd, exp)

    def test_A0_thru_A3(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        for i in range(0, 4):
            cmd = ">A%d:?" % i
            exp = "@A%d:" % i
            self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("A0")
        self._test_q_required("A0")

    def test_D2_thru_D6(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # Set all to 0, then 1
        for v in range(0, 2):
            for i in range(2, 6):   # in v4 port D6 is attached to beep
                cmd = ">D%d:%d" % (i, v)
                exp = "@D%d:%d:BIN" % (i, v)
                self._handle_cmd_exp(cmd, exp)

        # check the ? -- all should still be at 1
        for i in range(2, 6):   # in v4 port D6 is attached to beep
            cmd = ">D%d:?" % i
            exp = "@D%d:1:BIN" % i
            self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("D3")
        cmd = ">D4:3"
        exp = "!D4:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

    # ECHO has been removed as redundant.
    # def test_ECHO(self):
    #     print("\n--------------> ", sys._getframe().f_code.co_name, " <-------------- ")   # cool trick prints current function name
    #
    #     cmd = ">ECHO:1"
    #     exp = "@ECHO:1:BIN"
    #     self._handle_cmd_exp(cmd, exp)
    #
    #     cmd = ">ECHO:0"
    #     exp = "@ECHO:0:BIN"    # We should have gotten a verbose response but not going ot worry about the verbose specifics at this point
    #     self._handle_cmd_exp(cmd, exp)
    #
    #     # check the ? -- all should still be at 0
    #     cmd = ">ECHO:?"
    #     exp = "@ECHO:0:BIN"
    #     self._handle_cmd_exp(cmd, exp)
    #
    #     # poorly formatted command testing
    #     self._test_value_missing_conditions("ECHO")
    #     cmd = ">ECHO:3"
    #     exp = "!ECHO:" + VALUE_ERROR
    #     self._handle_cmd_exp(cmd, exp)

    def test_SID(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">SID:?"
        exp = "@SID:"  # won't test the full thing
        print("cmd:\t%s" % cmd)
        tx = cmd + '\r\n'
        SnipeTests.ser.write(tx.encode())
        time.sleep(0.05)
        resp = SnipeTests.ser.readline().decode()
        while resp:
            if resp.startswith("#"):
                print("#:\t%s" % resp.strip())
                resp = SnipeTests.ser.readline().decode()
            else:
                break
        print("resp:\t%s" % resp.strip())
        try:
            self.assertTrue(exp in resp, f"\nexpected: {exp}   but got: {resp}\n\n\t\t\t\t!!!FAIL!!!")
            print("\t\t\t\t===PASS===")
        except AssertionError as a:
            raise a
        oldsid = resp.split(":")[1]

        # set it to a test string
        cmd = ">SID:TEST"
        exp = "@SID:TEST"
        self._handle_cmd_exp(cmd, exp)
        # set it back
        cmd = ">SID:" + oldsid
        exp = "@SID:" + oldsid
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("SID")
        cmd = ">SID:1234567890123456789012345678901234567890"
        exp = "!SID:" + ERR_SID_TOO_LONG
        self._handle_cmd_exp(cmd, exp)

    def test_VER(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">VER:?"
        exp = "@VER:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("VER")
        self._test_q_required("VER")

    def test_DESC(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">DESC:?"
        exp = "@DESC:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("DESC")
        self._test_q_required("DESC")

    def test_I2A(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">I2A:123"
        exp = "@I2A:123"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2A:?"
        exp = "@I2A:123"
        self._handle_cmd_exp(cmd, exp)
        # test out of range
        cmd = ">I2A:270"
        exp = "!I2A:" + OUT_OF_RANGE
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2A")

    def test_I2B(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">I2B:3"
        exp = "@I2B:3"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2B:?"
        exp = "@I2B:3"
        self._handle_cmd_exp(cmd, exp)
        # test out of range
        cmd = ">I2B:20"
        exp = "!I2B:" + BYTE_SETTING_ERR
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2B")

    def test_I2S(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">I2S:5"
        exp = "@I2S:5"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2S:?"
        exp = "@I2S:5"
        self._handle_cmd_exp(cmd, exp)
        # test out of range
        cmd = ">I2S:270"
        exp = "!I2S:" + OUT_OF_RANGE
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2S")

    def test_I2W(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">I2B:1 I2W:0xFF"
        exp = "@I2B:1 I2W:0xFF"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2W:?"
        exp = "@I2W:0xFF"
        self._handle_cmd_exp(cmd, exp)

        # test byte error
        cmd = ">I2B:2 I2W:0xFF"
        exp = "!I2B:2 I2W:" + DATA_LENGTH_ERR
        self._handle_cmd_exp(cmd, exp)

        # test poorly formatted value
        cmd = ">I2W:FF"
        exp = "!I2W:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2W")

    def test_I2R(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        cmd = ">I2A:104 I2S:3 I2B:1 I2R:?"
        exp = "@I2A:104 I2S:3 I2B:1 I2R:"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2R:?"
        exp = "@I2R:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2R")
        self._test_q_required("I2R")

    def test_I2F(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # we don't know what's hooked up to your arduino, so just check for a response
        cmd = ">I2F:?"
        exp = "@I2F:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2F")
        self._test_q_required("I2F")

    def test_BLINK(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # we don't know what's hooked up to your arduino, so just check for a response
        cmd = ">BLINK:10000"
        exp = "@BLINK:10000"
        self._handle_cmd_exp(cmd, exp)

        cmd = ">BLINK:?"
        exp = "@BLINK:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("BLINK")

    def test_lowercase(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # test a few lowercase commands to make sure it doesn't matter
        cmd = ">sid:?"
        exp = "@SID:"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">Blink:5000"
        exp = "@BLINK:"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">vEr:?"
        exp = "@VER:"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">deSc:?"
        exp = "@DESC:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("BLINK")

    # NEW 4.0 Stack Light Commands

    def test_SLC1_thru_SLC3_basic(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        clr = 0x555555
        clrtxt = "0x555555"
        # Basic set colors
        for i in range(1, 4):
            cmd = ">SLC%d:%d" % (i, clr)
            exp = "@SLC%d:%s" % (i, clrtxt)
            self._handle_cmd_exp(cmd, exp)

        # check the ? -- all should still be at same color
        for i in range(1, 4):
            cmd = ">SLC%d:?" % i
            exp = "@SLC%d:%s" % (i, clrtxt)
            self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("SLC1")
        cmd = ">SLC1:0x1234567"
        exp = "!SLC1:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

    def test_SLC1_colornames(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # SET by uppercase name
        for key, value in colors.items():
            cmd = f">SLC1:{key}"
            exp = f"@SLC1:{value}:{key}"
            self._handle_cmd_exp(cmd, exp)

        # SET with lowercase name
        for key, value in colors.items():
            cmd = f">SLC1:{key.lower()}"
            exp = f"@SLC1:{value}:{key}"
            self._handle_cmd_exp(cmd, exp)

        # Fail intentionally
        cmd = f">SLC1:rojo"
        exp = f"!SLC1:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

    def test_SLC1_colorvalues(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        # SET known values first
        for key, value in colors.items():
            cmd = f">SLC1:{value}"
            exp = f"@SLC1:{value}:{key}"
            self._handle_cmd_exp(cmd, exp)

        # Set a color value, then go off by digit and validate it's "cleared"
        cmd = f">SLC1:0xFF0000"
        exp = f"@SLC1:0xFF0000:RED"
        self._handle_cmd_exp(cmd, exp)
        # off by one....should clear the 'red' that is appended since we don't have a color match
        cmd = f">SLC1:0xFF0001"
        exp = f"@SLC1:0xFF0001"
        self._handle_cmd_exp(cmd, exp)
        # set to another color to make sure
        cmd = f">SLC1:0x00FF00"
        exp = f"@SLC1:0x00FF00:GREEN"
        self._handle_cmd_exp(cmd, exp)

    def test_SLP_query(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        for i in range(1, 4):
            cmd = f">SLP{i}:99"
            exp = f"@SLP{i}:99"
            self._handle_cmd_exp(cmd, exp)

        for i in range(1, 4):
            cmd = f">SLP{i}:?"
            exp = f"@SLP{i}:99"
            self._handle_cmd_exp(cmd, exp)

    def test_SLM_query(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        for i in range(1, 4):
            cmd = f">SLM{i}:0"
            exp = f"@SLM{i}:0"
            self._handle_cmd_exp(cmd, exp)

        for i in range(1, 4):
            cmd = f">SLM{i}:?"
            exp = f"@SLM{i}:0"
            self._handle_cmd_exp(cmd, exp)


    # def test_SLA_query(self):
    #     print("\n--------------> ", sys._getframe().f_code.co_name,
    #           " <-------------- ")  # cool trick prints current function name
    #
    #     cmd = f">SLA:0"    # we won't turn it on....too loud
    #     exp = f"@SLA:0"
    #     self._handle_cmd_exp(cmd, exp)
    #
    #     cmd = f">SLA:?"
    #     exp = f"@SLA:0"
    #     self._handle_cmd_exp(cmd, exp)


    def test_SLP_missing_value(self):
        # poorly formatted command testing
        self._test_value_missing_conditions("SLP1")

    def test_SLM_missing_value(self):
        # poorly formatted command testing
        self._test_value_missing_conditions("SLM1")

    def test_SLP_values(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name

        values = ['0', '20', '40', '60', '80', '100']
        hexvals = ['0x0', '0x14', '0x28', '0x3C', '0x50', '0x64']
        # test good values
        for value in values:
            cmd = f">SLP1:{value}"
            exp = f"@SLP1:{value}"
            self._handle_cmd_exp(cmd, exp)
            time.sleep(.125)

        # test hex values
        for i in range(len(values)):
            cmd = f">SLP1:{hexvals[i]}"
            exp = f"@SLP1:{values[i]}"  # it converts to decimal for us
            self._handle_cmd_exp(cmd, exp)
            time.sleep(.125)

        # test out of range
        cmd = f">SLP1:101"
        exp = f"!SLP1:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

        # test misformatted as float
        cmd = f">SLP1:50.3"
        exp = f"!SLP1:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

    def test_SLM_values(self):
        print("\n--------------> ", sys._getframe().f_code.co_name,
              " <-------------- ")  # cool trick prints current function name
        default = "500"
        minval = 100
        maxval = 10000
        # test with no value given...default is 500ms on startup
        for i in range(1, 4):
            for mode in range(0, 5):
                cmd = f">SLM{i}:{mode}"
                exp = f"@SLM{i}:{mode}"
                self._handle_cmd_exp(cmd, exp)
        # test in range cycle values, regardless of mode
        for mode in range(0, 5):
            for cycle in range(minval, maxval, 2000):
                cmd = f">SLM1:{mode}:{cycle}"
                exp = f"@SLM1:{mode}:{cycle}"
                self._handle_cmd_exp(cmd, exp)

        # test below range
        cmd = f">SLM1:1:{minval - 5}"
        exp = f"@SLM1:1:{minval}"
        self._handle_cmd_exp(cmd, exp)

        # test above range
        cmd = f">SLM1:1:{maxval + 5}"
        exp = f"@SLM1:1:{maxval}"
        self._handle_cmd_exp(cmd, exp)

        # test messed up value
        cmd = f">SLM1:1:abcd"
        exp = f"@SLM1:1:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)

        # test float value -- it works but ignores the decimal
        cmd = f">SLM1:1:1000.8"
        exp = f"@SLM1:1:1000"
        self._handle_cmd_exp(cmd, exp)


# SLC1, SLC2, SLC3
# SLM1, SLM2, SLM3
# SLA


if __name__ == '__main__':
    # parser = OptionParser()
    # parser.add_option("-p", "--port", dest="port",
    #              help="Set SNIPE Arduino port", metavar="PORT")
    # parser.add_option("-b", "--baud", dest="baud", default=57600,
    #              help="Baud rate for SNIPE Arduino, default 57600")
    #
    # (options, args) = parser.parse_args()
    # if options.port:
    #    SnipeTests.port = options.port
    # if options.baud:
    #    SnipeTests.baudrate = options.baud
    if len(sys.argv) > 1:
        SnipeTests.port = sys.argv.pop()
    else:
        SnipeTests.port = "/dev/tty.usbserial-A92517JR"

    print("----- RUNNING SNIPE TEST SUITE  v20160211 -------")
    print("port: " + SnipeTests.port + "\tbaudrate: " + str(SnipeTests.baudrate))
    print("-------------------------------------------------")
    try:
        unittest.main()
    except:
        pass
