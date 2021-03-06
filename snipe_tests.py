#/bin/python
# -*- coding: utf-8 -*-
#
# TEST SUITE FOR SNIPE 
#   You will need python 2.7 and pyserial installed at a minimum.
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


DATA_STRING      =    "#####DATA#####"
VALUE_MISSING    =    "VALUE_MISSING"
VALUE_ERROR      =    "VALUE_ERROR"
Q_REQUIRED       =    "?_MISSING"
ERR_SID_TOO_LONG =    "SID_>_30"
OUT_OF_RANGE     =    "OUT_OF_RANGE"
BYTE_SETTING_ERR =    "BYTE_SETTING_ERR"
NONE_FOUND       =    "NONE_FOUND"
DATA_LENGTH_ERR  =    "DATA_LENGTH_ERR"

class SnipeTests(unittest.TestCase):

#    def setUp(self):
#        pass
#    def tearDown(self):
#        pass

    port = ""
    baudrate = 57600
    timeout = 1
    ser = serial.Serial()

    @classmethod
    def setUpClass(cls):
        """
        Init function for all the test cases in this class.  Called once.
        """
        in_header_mode = True
        try:
            SnipeTests.ser = serial.Serial(port =     SnipeTests.port,
                                baudrate = SnipeTests.baudrate,
                                timeout =  SnipeTests.timeout)
            if not SnipeTests.ser.isOpen():
                raise Exception("Serial port is not open: " + SnipeTests.port)


            while in_header_mode:
                resp = SnipeTests.ser.readline()
                print "resp:\t" + resp.strip()
                if DATA_STRING in resp:
                    in_header_mode = False

        except Exception, e:
            print "ERROR SETTING UP SERIAL PORT: "
            print e.args
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
        except Exception, e:
            print "ERROR CLOSING SERIAL PORT: "
            print e.args



    def _handle_cmd_exp(self, cmd, exp):
        """
        Assert that given a command (cmd) we get the expected (exp) response.
        :param cmd: command to send
        :param exp: response we expect, or expect to be part of the response.
        :return:
        """
        print "cmd:\t%s" % cmd
        SnipeTests.ser.write(cmd + "\r\n")
        time.sleep(0.05)
        resp = SnipeTests.ser.readline()
        while resp:
            if resp.startswith("#"):
                print "#:\t%s" % resp.strip()
                resp = SnipeTests.ser.readline()
            else:
                break
        print "exp:\t%s"  % exp
        print "resp:\t%s" % resp.strip()
        try:
            self.assertTrue(exp in resp)
            print "\t\t\t\t===PASS==="
        except AssertionError, a:
            print "\t\t\t\t!!!FAIL!!!"
            raise a

    def _test_value_missing_conditions(self, cmd):
        """
        For a given command, test improperly formatted
        or null value conditions.
        :param cmd:  e.g. "A0"   or "D2"
        """
        xcmd = ">" + cmd
        exp =  "!" + cmd + ":" + VALUE_MISSING
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
        self._handle_cmd_exp(cmd,exp)




    def test_A0_thru_A3(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        for i in xrange(0, 4):
            cmd = ">A%d:?" % i
            exp = "@A%d:" % i
            self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("A0")
        self._test_q_required("A0")


    def test_D2_thru_D6(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        # Set all to 0, then 1
        for v in xrange(0,2):
            for i in xrange(2, 7):
                cmd = ">D%d:%d" % (i, v)
                exp = "@D%d:%d:BIN" % (i, v)
                self._handle_cmd_exp(cmd, exp)

        # check the ? -- all should still be at 1
        for i in xrange(2, 7):
            cmd = ">D%d:?" % i
            exp = "@D%d:1:BIN" % i
            self._handle_cmd_exp(cmd, exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("D3")
        cmd = ">D4:3"
        exp = "!D4:" + VALUE_ERROR
        self._handle_cmd_exp(cmd, exp)



    def test_SID(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">SID:?"
        exp = "@SID:"      #won't test the full thing
        print "cmd:\t%s" % cmd
        SnipeTests.ser.write(cmd + '\r\n')
        time.sleep(0.05)
        resp = SnipeTests.ser.readline()
        while resp:
            if resp.startswith("#"):
                print "#:\t%s" % resp.strip()
                resp = SnipeTests.ser.readline()
            else:
                break
        print "resp:\t%s" % resp.strip()
        try:
            self.assertTrue(exp in resp)
            print "\t\t\t\t===PASS==="
        except AssertionError, a:
            print "\t\t\t\t!!!FAIL!!!"
            raise a
        oldsid = resp.split(":")[1]

        # set it to a test string
        cmd = ">SID:TEST"
        exp = "@SID:TEST"
        self._handle_cmd_exp(cmd,exp)
        # set it back
        cmd = ">SID:" + oldsid
        exp = "@SID:" + oldsid
        self._handle_cmd_exp(cmd, exp)


        # poorly formatted command testing
        self._test_value_missing_conditions("SID")
        cmd = ">SID:1234567890123456789012345678901234567890"
        exp = "!SID:" + ERR_SID_TOO_LONG
        self._handle_cmd_exp(cmd, exp)


    def test_TID(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">TID:1234"
        exp = "@TID:1234"
        self._handle_cmd_exp(cmd, exp)
        # verify it is echoed back in a multi-token command
        cmd = ">SID:? TID:1234"
        exp = "TID:1234"
        self._handle_cmd_exp(cmd,exp)

        # poorly formatted command testing
        self._test_value_missing_conditions("TID")


    def test_VER(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">VER:?"
        exp = "@VER:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("VER")
        self._test_q_required("VER")


    def test_DESC(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">DESC:?"
        exp = "@DESC:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("DESC")
        self._test_q_required("DESC")


    def test_I2A(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">I2A:123"
        exp = "@I2A:123"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2A:?"
        exp = "@I2A:123"
        self._handle_cmd_exp(cmd,exp)
        # test out of range
        cmd = ">I2A:270"
        exp = "!I2A:" + OUT_OF_RANGE
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2A")


    def test_I2B(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">I2B:3"
        exp = "@I2B:3"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2B:?"
        exp = "@I2B:3"
        self._handle_cmd_exp(cmd,exp)
        # test out of range
        cmd = ">I2B:20"
        exp = "!I2B:" + BYTE_SETTING_ERR
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2B")


    def test_I2S(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        cmd = ">I2S:5"
        exp = "@I2S:5"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2S:?"
        exp = "@I2S:5"
        self._handle_cmd_exp(cmd,exp)
        # test out of range
        cmd = ">I2S:270"
        exp = "!I2S:" + OUT_OF_RANGE
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2S")



    def test_I2W(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name


        cmd = ">I2B:1 I2W:0xFF"
        exp = "@I2B:1 I2W:0xFF"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2W:?"
        exp = "@I2W:0xFF"
        self._handle_cmd_exp(cmd,exp)

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
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name


        cmd = ">I2A:104 I2S:3 I2B:1 I2R:?"
        exp = "@I2A:104 I2S:3 I2B:1 I2R:"
        self._handle_cmd_exp(cmd, exp)
        cmd = ">I2R:?"
        exp = "@I2R:"
        self._handle_cmd_exp(cmd,exp)

        # poorly formatted
        self._test_value_missing_conditions("I2R")
        self._test_q_required("I2R")



    def test_I2F(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        # we don't know what's hooked up to your arduino, so just check for a response
        cmd = ">I2F:?"
        exp = "@I2F:"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("I2F")
        self._test_q_required("I2F")


    def test_BLINK(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

        # we don't know what's hooked up to your arduino, so just check for a response
        cmd = ">BLINK:5000"
        exp = "@BLINK:5000"
        self._handle_cmd_exp(cmd, exp)

        # poorly formatted
        self._test_value_missing_conditions("BLINK")


    def test_lowercase(self):
        print "\n--------------> ", sys._getframe().f_code.co_name, " <-------------- "   # cool trick prints current function name

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









if __name__ == '__main__':
    #parser = OptionParser()
    #parser.add_option("-p", "--port", dest="port",
    #              help="Set SNIPE Arduino port", metavar="PORT")
    #parser.add_option("-b", "--baud", dest="baud", default=57600,
    #              help="Baud rate for SNIPE Arduino, default 57600")
    #
    #(options, args) = parser.parse_args()
    #if options.port:
    #    SnipeTests.port = options.port
    #if options.baud:
    #    SnipeTests.baudrate = options.baud
    if len(sys.argv) > 1:
        SnipeTests.port = sys.argv.pop()


    print "----- RUNNING SNIPE TEST SUITE  v20160211 -------"
    print "port: " + SnipeTests.port + "\tbaudrate: " + str(SnipeTests.baudrate)
    print "-------------------------------------------------"
    try:
        unittest.main()
    except:
        pass

    