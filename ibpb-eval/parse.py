#!/usr/bin/env python3
import os
import re
import sys
import statistics


def read_logs(dirname):
    logs = os.listdir(dirname)
    todo = [x for x in logs if not x.endswith(".log") and not x.endswith(".html")]
    if len(todo) == 0:
        return None

    print(f"We have {len(todo)} files to check for {dirname}")
    results = {
        "single": {},
        "multi": {}
    }
    for fname in todo:
        with open(os.path.join(dirname, fname)) as f:
            cat = None
            lines = f.readlines()
            for i, ln in enumerate(lines):
                if "running 1 parallel copy of tests" in ln:
                    cat = "single"
                if "parallel copies of tests" in ln:
                    # multi
                    cat = "multi"
                if "RESULT" in ln:
                    for l in lines[i+1:i+1+12]:
                        #  print(l.strip())
                        split = re.split(r" {2,}", l)
                        case = split[0]
                        result = split[2]
                        #  print(f"{cat}:{dirname}/{fname}")
                        if not case in results[cat]:
                            results[cat][case] = []

                        results[cat][case] += [float(result)]


    retval = {}

    for cat in ["single", "multi"]:
        scores = []
        for case in results[cat]:
            med = statistics.median(results[cat][case])
            scores += [med]
        geomean = statistics.geometric_mean(scores)
        retval[cat] = geomean
    return retval

def calc_overhead(base, patched):
    return base/patched - 1

def calc_overhead_wrt_base(base, patched):
    return 1 - patched/base

def print_result(baseline, other):
    for cat in ["single", "multi"]:
        bl = baseline[cat]
        o = other[cat]
        print(f"{cat}: {round(bl, 2)} vs {round(o,2)}")
        print(f"   {round(100*calc_overhead(bl, o),2)}% overhead")

if len(sys.argv) < 2:
    print(f"usage: {sys.argv[0]} topdir");
    print(f"example: {sys.argv[0]} ./raw/cn102");
    exit(1)

topdir = sys.argv[1]

baseline = read_logs(os.path.join(topdir, "baseline"))
ibpb = read_logs(os.path.join(topdir, "ibpb"))
ibpbnosmt = None

if os.path.exists(os.path.join(topdir, "ibpb,nosmt")):
    ibpbnosmt = read_logs(os.path.join(topdir, "ibpb,nosmt"))

print("IBPB -------")
print_result(baseline, ibpb)
print("------------")
if ibpbnosmt:
    print("IBPB,NOSMT --")
    print_result(baseline, ibpbnosmt)
    print("-------------")
