#!/usr/bin/env python
"""
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
"""
import glob
import json
import numpy as np
import matplotlib.pyplot as plt

DATASET_PREFIX = 'IRT-10.7.4-'

DATASET = [
    'gcc-native', 'fcc-clang', 'fcc-clang-fast', 'fcc-trad', 'fcc-trad-fast'
]

PLOT_TITLE = 'IRT'

#-------------------------------------------------------------------------------
def get_eps(dataset, olympic_style):

    file_list = glob.glob(f"{DATASET_PREFIX}{dataset}/benchmark*.json")
    num_file  = len(file_list)

    eps = []
    for x in file_list:
        with open(x) as f:
            js = json.load(f)
            eps.append(js['summary']['throughput'])

    eps.sort()
    if olympic_style:
        del eps[num_file - 1]
        del eps[0]

    eps = np.array(eps)

    print(f"DataSet: {dataset}, Mean: {eps.mean()}, STD: {eps.std()}")

    return eps.mean()

#-------------------------------------------------------------------------------
# main function
def main(olympic_style=False):

    x = []
    eps = []

    counter = 1
    for dataset in DATASET:
        x.append(counter)
        eps.append(get_eps(dataset, olympic_style))
        counter += 1

    fig, ax = plt.subplots(dpi=200)
    fig.subplots_adjust(left=0.2)
    p = ax.barh(x, eps, tick_label=DATASET)
    ax.set_title(PLOT_TITLE)
    ax.bar_label(p, label_type='center',fmt='%.2f')
    ax.set_xlabel("Throughput (#History/min)")

    plt.show()
    plt.savefig("throughput.pdf")

#===============================================================================
if __name__ == '__main__':
    main(olympic_style=False)
