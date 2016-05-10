#!/usr/bin/env python
# -*- coding: utf-8 -*-

import datetime
import glob
import os
import os.path
import time
import getopt
import sys
from xml.dom.minidom import Document

class TestResult:

    def __init__(self, passed, time_taken, message, out, err):
        self.passed = passed
        self.time_taken = time_taken
        self.message = message
        self.out = out
        self.err = err

    def get_passed(self):
        return self.passed

    def get_time_taken(self):
        return self.time_taken

    def get_message(self):
        return self.message

    def get_err(self):
        return self.err

    def get_out(self):
        return self.out

def read_file(file_name):

    if not os.path.exists(file_name):
        return ''
#
# 2.5 style
#   with open(file_name, 'r') as f:
#       read_data = f.read()
#
    f = open(file_name, 'r')
    try:
        read_data = f.read()
    finally:
        f.close()

    return read_data

def run_test(test):
    ' returns True/False, time, out, err'
    runner = test + '.sh'

    base = os.environ['S2_LOGS_DIR'] + '/'
    message = base + test + '.out'
    err = base + test + '.log'
    out = base + test + '.e1'

    #command = './%s > /dev/null 2>&1' % (runner)
    command = './%s' % (runner)
    start_time = time.time()
    rc = os.system(command)
    stop_time = time.time()

    time_taken = stop_time - start_time
    passed = (rc == 0)

    if passed:
        passed_str = 'PASS'
    else:
        passed_str = 'FAILED'

    msg = read_file(message)

    if os.path.exists(test+'.expect_failure'):
        passed = not(passed)
        if passed:
            passed_str = 'PASS (EXPECTED FAILURE)'
        else:
            passed_str = 'FAILED (UNEXPECTED PASS)'
            msg = "== TEST EXPECTED TO FAIL BUT PASSED ==\n\n" + msg

    print "Test: %s, Pass: %s, Took: %.3f s" % (test, passed_str, time_taken)
    return TestResult(passed, time_taken, msg, read_file(err), read_file(out))


def print_test_result(results):
    print 'Total %s tests' % (len(results))
    for test in results.keys():
        result = results[test]

        if result.get_passed():
            passed = 'PASS'
        else:
            passed = 'FAILED'
        #passed = 'PASS' if result.get_passed() else 'FAILED'
        print "Test: %s, Pass: %s, Took: %.3f s" % (test, passed, result.get_time_taken())

def write_file(file_name, data):
#
# 2.5 style
#   with open(file_name, 'w') as f:
#       read_data = w.write(data)
#
    f = open(file_name, 'w')
    try:
        f.write(data)
    finally:
        f.close()

def print_test_result_xml(suite, results, report_file, extra):

    id = 0
    doc = Document()
    testsuites = doc.createElement("testsuites")
    doc.appendChild(testsuites)

    for test in results.keys():
        result = results[test]

        testsuite = doc.createElement("testsuite")
        testsuite.setAttribute("tests", str(len(results)))
        testsuite.setAttribute("errors", "0")
        testsuite.setAttribute("timestamp", str(datetime.datetime.now()))
        testsuite.setAttribute("name", suite)
        testsuite.setAttribute("package", extra+suite)
        testsuite.setAttribute("id", str(id))
        id += 1
        testsuites.appendChild(testsuite)
        total_time = 0

        failures = 0
        testcase = doc.createElement("testcase")
        testsuite.appendChild(testcase)
        testcase.setAttribute("name", extra + test)
        testcase.setAttribute("classname", suite)
        testcase.setAttribute("time", str(result.get_time_taken()))

        total_time += result.get_time_taken()
        if not result.get_passed():
            failures += 1
            failure = doc.createElement("failure")
            failure.setAttribute("message", result.get_message())
            #err = doc.createCDATASection(result.get_err())
            #failure.appendChild(err)
            testcase.appendChild(failure)

        testsuite.setAttribute("failures", str(failures))
        testsuite.setAttribute("time", str(total_time))
        stdout = doc.createElement("system-out")
        out = doc.createCDATASection(result.get_out())
        stdout.appendChild(out)
        stderr = doc.createElement("system-err")
        err = doc.createCDATASection(result.get_err())
        stderr.appendChild(err)
        testsuite.appendChild(stdout)
        testsuite.appendChild(stderr)

    xml = doc.toprettyxml(indent="  ")

    write_file(report_file, xml)

def main():

    opts, args = getopt.getopt(sys.argv[1:], "o:")
    outdir = os.getcwd()

    for o, a in opts:
        if o in ("-o", "--output"):
            outdir = a
        else:
            assert False, "unhandled option"

    cwd = os.getcwd()
    suite = os.path.basename(cwd)
    extra = os.environ['TEST_PREFIX']
    report_file = '%s/TEST-%s%s.xml' % (outdir, extra, suite)

    test_results = {}
    for test in glob.glob('%s/*.s2' % (cwd)):
        test = os.path.basename(test)
        (base, ext) = os.path.splitext(test)
        test_results[base] = run_test(base)

    print_test_result_xml(suite, test_results, report_file, extra)

if __name__ == "__main__":
    main()
