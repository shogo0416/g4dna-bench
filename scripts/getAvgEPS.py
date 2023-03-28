#!/usr/bin/env python
"""
================================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2023 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
================================================================================
"""
import argparse
import glob
import json
import numpy as np

#===============================================================================
# Main Function
#===============================================================================
def main(chem):

    files = glob.glob('benchmark_*.json')
    if len(files) == 0:
        print("[ERROR] No benchmark files found. Stop the process.")
        return

    eps = []
    for x in files:
        with open(x) as f:
            js = json.load(f)
            if chem:
                eps.append(js['thread0']['ChemistryStage']['EPSScore'])
            else:
                eps.append(js['summary']['throughput'])

    num = len(eps)
    if num == 0:
        print("[ERROR] Could not get EPS scores. Stop the process.")
        return
    else:
        print(f"[MESSAGE] {num} files are loaded.")

    eps = np.array(eps)
    if num > 4:
        eps = np.sort(eps)
        mean = np.mean(eps[1:num-1])
        stddev = np.std(eps[1:num-1])
    else:
        mean = np.mean(eps)
        stddev = np.std(eps)

    print(f"[MESSAGE] Average EPS Score: {mean} +/- {stddev} Events/min")

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Get EPS Score")
    parser.add_argument('-c', '--chem', action='store_true')
    args = parser.parse_args()
    main(args.chem)
