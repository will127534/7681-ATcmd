import os, sys, time
from optparse import OptionParser
import logging

import serial
import xmodem

class pgfile(object):
    def __init__(self, filepath, callback):
        self.btotal = os.stat(filepath).st_size
        self.bread = 0
        self.f = file(filepath, "rb")
        self.cb = callback
        self.t_start = time.time()

    def read(self, size=1):
        r = self.f.read(size)
        self.bread += len(r)
        t_now = time.time()
        if t_now > self.t_start + 1 or self.bread == self.btotal:
            self.cb(self.bread, self.btotal)
            self.t_start = t_now
        return r

    def close(self):
        return self.f.close()

    def write(self, data):
        return 0

class COMport(object):
    def __init__(self, port, baudrate):
        self.s = serial.Serial(opt.com_port, 115200)
        self.buffer = ""

    def send(self, text):
        self.s.write(text + chr(13))

    def waitfor(self, text, timeout = None):
        t_start = time.time()
        while True:
            if timeout and (time.time() > t_start + timeout):
                return False
            l = self.s.inWaiting()
            if not l:
                time.sleep(0.1)
                continue
            self.buffer += self.s.read(l)

            #print >> sys.stderr, "[log] buffer =", self.buffer

            while len(self.buffer) >= len(text):
                if self.buffer.startswith(text):
                    self.buffer = self.buffer[len(text):]
                    return True

                self.buffer = self.buffer[1:]

    def sendXmodem(self, filepath):

        def getc(size, timeout=1):
            return self.s.read(size)

        def putc(data, timeout=1):  
            return self.s.write(data)

        def pgupdate(read, total):
            print "\r%d/%d bytes (%2.f%%) ..." % (read, total, read*100/total),

        m = xmodem.XMODEM(getc, putc)
        f = pgfile(filepath, pgupdate)
        return m.send(f)

def main(port, baud_rate, bin_path, recovery_mode=True):
    try:
        s = COMport(port, baud_rate)
    except serial.serialutil.SerialException as e:
        print >> sys.stderr, "Error opening", opt.com_port, "!!!! :", e
        sys.exit(-1)

    if recovery_mode:
        s.send("AT#Reboot")
        if not s.waitfor("Recovery Mode", 2):
            print "Failed to enter 'Recovery Mode' automatically, please press reset key manually"
            s.waitfor("Recovery Mode")

        print "Recovery Mode entered, Starting to upload..."

    s.send("AT#UpdateFW")
    s.waitfor("C")
    if s.sendXmodem(opt.bin_path):
        print "Update done! bye"
        return True
    else:
        print "Error: Fail to update!!!!"
        return False

def do_default(port, baud_rate, product_id, product_key):
    try:
        s = COMport(port, baud_rate)
    except serial.serialutil.SerialException as e:
        print >> sys.stderr, "Error opening", opt.com_port, "!!!! :", e
        sys.exit(-1)

    s.send("AT#Reboot")
    if not s.waitfor("Recovery Mode", 2):
        print "Failed to reboot, please press reset key manually"
        s.waitfor("Recovery Mode")

    print "Reseting to default...",
    s.waitfor("<== RecoveryMode")
    time.sleep(0.5)
    s.send("AT#Default")
    if not s.waitfor("SM=1, Sub=0", 5):
        print "Failed!!"
        sys.exit(-1)
    print "done"

    if product_id:
        print "Setting MCS product id to", product_id, "...", 
        s.send("AT#FLASH -s0x18133 -c" + product_id)
        if not s.waitfor("[0x18133]="+product_id, 2):
            print "Failed!!"
            sys.exit(-1)
        print "done"

    if product_key:
        print "Setting MCS product id to", product_key, "...", 
        s.send("AT#FLASH -s0x18143 -c" + product_key)
        if not s.waitfor("[0x18143]="+product_key, 2):
            print "Failed!!"
            sys.exit(-1)
        print "done"

    print "Reboot again to take effeft"
    s.send("AT#Reboot")

if __name__ == "__main__":

    logging.basicConfig()

    parser = OptionParser(usage="python %prog [options]")
    parser.add_option("-f", dest="bin_path", help="path of bin to be upload")
    parser.add_option("-c", dest="com_port", help="COM port, can be COM1, COM2, ..., COMx")
    parser.add_option("-b", dest="baud_rate", help="Baud rate, can be 9600, 115200, etc...", type="int", default=115200)
    parser.add_option("--default", action="store_true", dest="f_def", default=False, help="Reset 7681 to initial state",)
    parser.add_option("-p", dest="product_id", help="Config MCS product id, only work with --default")
    parser.add_option("-k", dest="product_key", help="Config MCS product key, only work with --default")
    (opt, args) = parser.parse_args()

    if opt.f_def:
        if not opt.com_port:
            print >> sys.stderr, "\nError: Invalid parameter!! Please specify COM port\n"
            parser.print_help()
            sys.exit(-1)

        do_default(opt.com_port, opt.baud_rate, opt.product_id, opt.product_key)
        sys.exit(0)

    if not opt.bin_path or not opt.com_port:
        print >> sys.stderr, "\nError: Invalid parameter!! Please specify COM port and bin.\n"
        parser.print_help()
        sys.exit(-1)

    if not os.path.exists(opt.bin_path):
        print >> sys.stderr, "\nError: File [ %s ] not found !!!\n" % (opt.bin_path)
        parser.print_help()
        sys.exit(-1)

    if opt.product_id or opt.product_key:
        print >> sys.stderr, "\nError: MCS product configuration is only supported with --default\n"
        parser.print_help()
        sys.exit(-1)

    main(opt.com_port, opt.baud_rate, opt.bin_path)

    # test
    if 0:
        import time
        count = 0
        fail_count = 0
        while True:
            time.sleep(1)
            count +=1
            if not main(opt.com_port, opt.baud_rate, opt.bin_path, False):
                fail_count +=1
            print "Fail/Total = %d/%d" % (fail_count, count)
