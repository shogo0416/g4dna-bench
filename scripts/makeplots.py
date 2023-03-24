#!/usr/bin/env python3
"""
================================================================================
  BSD 2-Clause License

  Copyright (c) 2021-2023 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduction the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
================================================================================
"""
import argparse
import bz2
import csv
import glob
import json
import statistics
import math

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ptick
import numpy as np

from lmfit.models import GaussianModel, BreitWignerModel
from matplotlib.backends.backend_pdf import PdfPages

#-------------------------------------------------------------------------------
# units for length
mm = 1.0
m  = 1.0E+03 * mm
dm = 1.0E-01 * m
um = 1.0E-03 * mm

# units for volume
dm3 = dm * dm * dm
um3 = um * um * um

# units for time
s  = 1.0
ps = 1.0E-12 * s

mole  = 1.0
umole = 1.0E-06 * mole
uM    = umole / dm3
NA    = 6.02214179e+23 / mole # Avogadro number
PI    = 3.14159265358979323846

LABEL = {'H^0'   : 'H',   'H_2^0'  : 'H2',   'H2O2^0' : 'H2O2', 'H3O^1': 'H3Op',
         'HO_2^0': 'HO2', 'HO_2^-1': 'HO2m', 'O^0'    : 'O',    'O_2^0': 'O2',
         'O_2^-1': 'O2m', 'O_3^0'  : 'O3',   'O_3^-1' : 'O3m',   'OH^0': 'OH',
         'OH^-1' : 'OHm', 'O^-1'   : 'Om',   'e_aq^-1': 'e_aq'}

TITLE = {"e_aq": "$\mathrm{e}_{\mathrm{aq}}^{-}$",
         "H3Op": "$\mathrm{H}_3\mathrm{O}^{+}$",
         "OH"  : "$^{\cdot}\mathrm{OH}$",
         "OHm" : "$\mathrm{OH}^{-}$",
         "H2"  : "$\mathrm{H}_2$",
         "H2O2": "$\mathrm{H}_2\mathrm{O}_2$",
         "H"   : "$\mathrm{H}^{\cdot}$",
         "O"   : "$\mathrm{O(3P)}$",
         "Om"  : "$\mathrm{O}^{\cdot-}$",
         "O2"  : "$\mathrm{O}_2$",
         "O2m" : "$\mathrm{O}_2^{-}$",
         "O3"  : "$\mathrm{O}_3$",
         "O3m" : "$\mathrm{O}_3^{-}$",
         "HO2" : "$\mathrm{HO}_2^{\cdot}$",
         "HO2m": "$\mathrm{HO}_2^{\cdot-}$"}

FIG_SIZE_X = 8
FIG_SIZE_Y = 5
DPI = 60
FONT_SIZE = 14

XLABEL = "Time (ps)"
YLABEL = "G-value"

#===============================================================================
class MakeDataFrameFromCSV():

    input_files = []

    #---------------------------------------------------------------------------
    def __init__(self):
        None

    #---------------------------------------------------------------------------
    def input_files(self, files):
        self.input_files = files

    #---------------------------------------------------------------------------
    def _merge(self, file, counter):

        # make DataFrame
        if counter == 0:
            self.df = pd.read_csv(file)
        else:
            self.df += pd.read_csv(file)

        counter += 1

        return counter

    #---------------------------------------------------------------------------
    def _check_columns(self, val):
        rejects = ['time', 'StrandBreak', 'QStrandBreak', 'BaseDamage',
                   'SugarRadical', 'BaseRadical']
        for x in rejects:
            if val == x:
                return True
        return False

    #---------------------------------------------------------------------------
    def make_data_frame(self):

        counter = 0
        for file in self.input_files:
            counter = self._merge(file, counter)

        self.df /= counter
        self.df = self.df.rename(columns={'Time_ps' : 'time'})
        self.df = self.df.rename(columns=LABEL)

    #---------------------------------------------------------------------------
    def make_output_data_frame(self):
        output = {"time": self.df['time'].values}
        for x in self.df.columns:
            if self._check_columns(x):
                continue
            output[x] = self.df[x].values
        return pd.DataFrame.from_dict(output)

    #---------------------------------------------------------------------------
    def save_as_pickle(self):
        self.make_output_data_frame().to_pickle('gval.pkl')

    #---------------------------------------------------------------------------
    def get_data_frame(self):
        return self.df

#-------------------------------------------------------------------------------
def get_gvalue(df, score_time=-1.0):

    num_col = len(df.columns)
    if score_time <= 0.0:
        num_row = len(df)
        sr_gvals = df.iloc[num_row - 1, 1 : num_col]
    else:
        index = df.index[
            df['time'].apply(np.ceil) == score_time].to_list()[0]
        sr_gvals = df.iloc[index, 1 : num_col]

    def get(key):
        val = 0.0
        if key in sr_gvals.keys():
            val = sr_gvals[key]
        return val

    eaq  = get(LABEL['e_aq^-1'])
    OH   = get(LABEL['OH^0'])
    H    = get(LABEL['H^0'])
    H2   = get(LABEL['H_2^0'])
    H2O2 = get(LABEL['H2O2^0'])
    HO2  = get(LABEL['HO_2^0'])
    O2   = get(LABEL['O_2^0'])
    O2m  = get(LABEL['O_2^-1'])

    sum1 = HO2 + O2
    sum2 = HO2 + O2m

    print(f"[MESSAGE] G(H2O2): {H2O2} G(HO2+O2): {sum1}, G(HO2+O2-): {sum2}")

    gvals = [eaq, OH, H, H2, H2O2, HO2, O2, O2m, sum1, sum2]
    header = "eaq, OH, H, H2, H2O2, HO2, O2, O2m, HO2+O2, HO2+O2m"

    l = np.array([gvals])
    np.savetxt('gval.csv', l, delimiter=',', header=header, fmt='%.5f')

#-------------------------------------------------------------------------------
def make_gvalue_plot(df, pdfname='gval.pdf', negative=False):

    num_row = len(df)
    start_time = df['time'][0]
    end_time = df['time'][num_row - 1]
    xticks = [
       pow(10.0, x) for x in range(
            int(math.log10(start_time)), int(math.log10(end_time)) + 1)
    ]

    def make_plot(x, y, title, pdf):
        fig, ax = plt.subplots(figsize=(FIG_SIZE_X, FIG_SIZE_Y), dpi=DPI)
        plt.plot(x, y, label="simulation", linewidth = 3.0)
        plt.xlabel(XLABEL, fontsize=FONT_SIZE)
        if title == 'edep':
            plt.title("Energy Deposit", fontsize=FONT_SIZE)
            plt.ylabel("Energy Deposit (eV)", fontsize=FONT_SIZE)
        elif title == 'OCR1':
            plt.title("OCR", fontsize=FONT_SIZE)
            plt.ylabel(YLABEL, fontsize=FONT_SIZE)
        elif title == 'OCR2':
            plt.title("OCR", fontsize=FONT_SIZE)
            plt.ylabel("Yields (uM)", fontsize=FONT_SIZE)
        else:
            plt.title(title, fontsize=FONT_SIZE)
            plt.ylabel(YLABEL, fontsize=FONT_SIZE)
        plt.legend(fontsize=FONT_SIZE)
        plt.tick_params(labelsize=FONT_SIZE)
        plt.xscale("log")
        plt.xticks(xticks)
        plt.minorticks_on()
        if not negative:
            plt.ylim(bottom=0.0)
        plt.grid(linestyle='dotted')
        ax.yaxis.set_major_formatter(ptick.ScalarFormatter(useMathText=True))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
        pdf.savefig(fig)
        plt.clf() # clear this plot completely

    def get_plot_title(key):
        if key in TITLE:
            return TITLE[key]
        return key

    with PdfPages(pdfname) as pdf:
        for col in df.columns:
            if col == 'time':
                continue
            make_plot(df['time'], df[col], get_plot_title(col), pdf)

#===============================================================================
# main function
#===============================================================================
def main(file_type, score_time):

    m = MakeDataFrameFromCSV()
    m.input_files(glob.glob("./result*.csv"))
    m.make_data_frame()

    # save G-value time profile for each molecular species as a PDF file
    make_gvalue_plot(m.get_data_frame())

    # G-value at the end of simulation time
    get_gvalue(m.get_data_frame(), score_time)

    df = m.make_output_data_frame()

    # save as pickle file
    df.to_pickle('gval.pkl')

#-------------------------------------------------------------------------------
if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="Make G-value plots")
    parser.add_argument('-f', '--file_type',  default='csv')
    parser.add_argument('-s', '--score_time', default=-1.0, type=float)
    args = parser.parse_args()

    main(args.file_type, args.score_time)
