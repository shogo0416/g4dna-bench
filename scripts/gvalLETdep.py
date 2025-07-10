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
import numpy as np
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from enum import IntEnum
import copy
import sys
import glob
import re
import pandas as pd

PDF_FILE   = 'plots.pdf'
GVAL_FILE1 = 'gval.csv'
GVAL_FILE2 = 'trkG.csv'
LET_FILE   = 'LET.csv'
CSV_OUT    = 'output_gval.csv'

GVAL_LABEL = 'MPEXS-DNA Sim.'
LET_LABEL  = 'MPEXS-DNA Sim.'

FIG_SIZE_X = 5
FIG_SIZE_Y = 4
DPI = 60
FONT_SIZE = 8
LEGEND_FONT_SIZE = 6
LINESTYLE = ["dashed", "dashdot", "dotted", "solid"]
LINEWIDTH = 2.0
MARKER_SIZE1 = 5
MARKER_SIZE2 = 30

# supported particle
PROTON = 'proton'
ALPHA  = 'alpha'
CARBON = 'carbon'

#ELIM_LOW1 = 0.5 # in MeV/u
ELIM_LOW1 = 0.7 # in MeV/u
ELIM_LOW2 = 2.0 # in MeV/u

#-------------------------------------------------------------------------------
class ParticleConfigs():

    #---------------------------------------------------------------------------
    def __init__(self, particle_kind):
        if particle_kind == PROTON:
            # for LET plot
            self.LET_xaxis_lim = [0.1, 100.0]  # MeV
            self.LET_yaxis_lim = [0.1, 100.0]  # keV/um
            # for G-value plot
            self.gval_xaxis_lim = [0.1, 100.0] # keV/um
            # mass number
            self.mass_number = 1.0

        elif particle_kind == ALPHA:
            # for LET plot
            self.LET_xaxis_lim = [0.1, 1000.0]  # MeV
            self.LET_yaxis_lim = [1.0, 10000.0] # keV/um
            # for G-value plot
            self.gval_xaxis_lim = [1.0, 1000.0] # keV/um
            # mass number
            self.mass_number = 4.0

        elif particle_kind == CARBON:
            # for LET plot
            self.LET_xaxis_lim = [1.0, 10000.0] # MeV
            self.LET_yaxis_lim = [1.0, 10000.0] # keV/um
            # for G-value plot
            self.gval_xaxis_lim = [1.0, 1000.0] # keV/um
            # mass number
            self.mass_number = 12.0

        else:
            print(f"[ERROR] {particle_kind} is NOT support particle.")
            sys.exit(1)

        self.particle_kind = particle_kind
        self.energy_range = sorted(self._find_directory(glob.glob("*")))

    #---------------------------------------------------------------------------
    def _check_particle_type(self, dirname):
        pattern = r'\d+\.?\d*MeV'
        ptype = re.sub(pattern, '', dirname)
        if self.particle_kind == PROTON:
            plabel = 'p'
        elif self.particle_kind == ALPHA:
            plabel = 'Alpha'
        elif self.particle_kind == CARBON:
            plabel = 'C'
        if ptype in plabel:
            return True
        return False

    #---------------------------------------------------------------------------
    def _find_directory(self, files_and_dirs):
        dirlist = []
        pattern = r'\d+\.?\d*'
        for x in files_and_dirs:
            if os.path.isdir(x) and self._check_particle_type(x):
                key = re.findall(pattern, x)[0]
                if self.particle_kind == PROTON:
                    A = 1
                elif self.particle_kind == ALPHA:
                    A = 4
                elif self.particle_kind == CARBON:
                    A = 12
                E = float(key)
                if E / A >= ELIM_LOW1:
                    dirlist.append(E)
        return dirlist

    #---------------------------------------------------------------------------
    def get_particle_kind(self):
        return self.particle_kind

    #---------------------------------------------------------------------------
    def get_energy_range(self):
        return self.energy_range

    #---------------------------------------------------------------------------
    def get_LET_axis_lim(self):
        return self.LET_xaxis_lim, self.LET_yaxis_lim

    #---------------------------------------------------------------------------
    def get_gval_axis_lim(self):
        return self.gval_xaxis_lim

    #---------------------------------------------------------------------------
    def get_mass_number(self):
        return self.mass_number

#-------------------------------------------------------------------------------
class Molecule(IntEnum):
    eaq  = 0
    OH   = 1
    H    = 2
    H2   = 3
    H2O2 = 4
    HO2  = 5
    O2   = 6
    O2m  = 7
    HO2_plus_O2  = 8
    HO2_plus_O2m = 9

#-------------------------------------------------------------------------------
class LETData(IntEnum):
    Mean  = 0
    Sigma = 1

#-------------------------------------------------------------------------------
# experimental data
def get_expdata(part_type, mole_type, plt):

    #---------------------------------------------------------------------------
    # for Carbons
    if part_type == CARBON and mole_type == 'H2O2':
        # Pastina, B. (1999)
        LET1  = [756.463, 657.933, 613.590]
        GVal1 = [0.85, 0.933, 0.967]
        GVal1_err = [0.042, 0.0455, 0.049]
        plt.errorbar(LET1, GVal1, yerr= GVal1_err, fmt='o', ms=MARKER_SIZE1,
                     label="Exp. Pastina, B. (1999)", color='orange')
        # Wasseline-Trupin. V. (2002)
        LET2  = [32.5]
        GVal2 = [0.9262]
        plt.scatter(LET2, GVal2, label="Exp. Wasseline-Trupin. V. (2002)",
                    color='red', s=MARKER_SIZE2)
        # S.Yamashita (2008)
        LET3 = [11.05, 13.50, 14.53, 21.54, 106.90]
        LET3_err = [0.0, 0.0, 0.0, 0.0, 35.27]
        GVal3 = [0.71, 0.759, 0.894, 0.820, 0.979]
        GVal3_err = [0.0734, 0.0796, 0.0918, 0.0857, 0.0980]
        plt.errorbar(LET3, GVal3, xerr=LET3_err, yerr=GVal3_err, fmt='o',
                     ms=MARKER_SIZE1, label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')
        # T.Kusumoto (2023)
        #LET4 = [357.1, 417.4, 525.6]
        #GVal4 = [1.34, 1.26, 1.1]
        #plt.scatter(LET4, GVal4, label="Exp. T.Kusumoto (2023)",
        #            color='darkviolet', s=MARKER_SIZE2)
        # Pastina and LaVerne (2001)
        LET5  = [0.23, 13.8, 34.8, 156.0]
        GVal5 = [0.70, 0.74, 0.76, 1.00]
        plt.scatter(LET5, GVal5, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == CARBON and mole_type == 'HO2 + O2':
        # H.C Laverne
        LET  = [40, 60, 80, 100, 200, 400, 600, 800]
        GVal = [0.027963026, 0.034953783, 0.038449161, 0.043692228,
                0.066412187, 0.117095172, 0.171273535, 0.230694965]
        plt.scatter(LET, GVal, label="Exp. H. C Laverne", color='red',
                    s=MARKER_SIZE2)

    elif part_type == CARBON and mole_type == 'HO2 + O2-':
        # MC Sim. Gervais, B. (2006)
        LET1 = [36.13738961, 43.69900141, 47.19221198,
                56.29776568, 70.26812249, 78.68179575,
                97.76351712, 120.924621,  134.1841981,
                168.2415273, 207.1602771, 230.9179856,
                286.9196645, 356.5027369, 397.3874481,
                502.7768186, 616.2882533, 683.8652412]
        GVal1 = [0.029054005, 0.030437529, 0.030898704,
                 0.032282228, 0.034357514, 0.035279863,
                 0.037585737, 0.040122197, 0.041505721,
                 0.044733944, 0.048192754, 0.049806866,
                 0.053918613, 0.057877423, 0.059952709,
                 0.064333868, 0.068253853, 0.070329139]
        plt.plot(LET1, GVal1, label="MC Sim. Gervais, B. (2006)",
                 color='orange', linewidth=LINEWIDTH)
        # MC Sim. Meesungnoen, J. (2005)
        LET2 = [98.21754825, 132.6456319, 171.0787445, 209.3884957,
                249.6473016, 291.870982,  341.2111267, 396.3029019,
                457.29533,   520.8466606, 552.2612866]
        GVal2 = [0.023698473, 0.025470664, 0.028674404, 0.032531512,
                 0.037193736, 0.042305634, 0.048743809, 0.055873204,
                 0.063880621, 0.072287034, 0.076304532]
        plt.plot(LET2, GVal2, label="MC Sim. Meesungnoen, J. (2005)",
                 color='red', linewidth=LINEWIDTH)
        # Baldacchino, G. (1998)
        LET3 =  [249.3550334]
        GVal3 = [0.058414621]
        plt.plot(LET3, GVal3, label="Exp. Baldacchino, G. (1998) for S ions",
                 color='m', marker='x', markersize=MARKER_SIZE1, linewidth=0.0)
        # Baldacchino, G. (1998) for Ar ions
        LET4  = [291.719245]
        GVal4 = [0.050642234]
        plt.plot(LET4, GVal4, label="Exp. Baldacchino, G. (1998) for Ar ions",
                 color='y', marker='+', markersize=MARKER_SIZE1, linewidth=0.0)

    elif part_type == CARBON and mole_type == 'OH':

        # Wasseline-Trupin. V. (2002)
        LET1  = [32.5]
        GVal1 = [1.32]
        plt.scatter(LET1, GVal1, label="Exp. Wasseline-Trupin. V. (2002)",
                    color='red', s=MARKER_SIZE2)

        # S.Yamashita (2008)
        LET2 = [11.05, 13.50, 14.53, 21.54, 106.90]
        LET2_err = [0.0, 0.0, 0.0, 0.0, 35.27]
        GVal2 = [1.98, 1.92, 1.8, 1.5, 0.9]
        GVal2_err = [0.195, 0.195, 0.18, 0.15, 0.09]
        plt.errorbar(LET2, GVal2, xerr=LET2_err, yerr=GVal2_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')

        # T.Maeyama (2011)
        LET3 = [11.022, 15.244, 21.084, 24.004, 27.329, 43.033]
        GVal3 = [1.8913, 1.8587, 1.6630, 1.4674, 1.3370, 1.2391]
        GVal3_err = [0.1957, 0.1957, 0.1957, 0.1630, 0.1630, 0.1304]
        plt.errorbar(LET3, GVal3, yerr=GVal3_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. T.Maeyama (2011) @100ns",
                     color='crimson')

        # Anderson and Hart (1961)
        LET4  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal4 = [1.89, 1.90, 1.88, 1.79, 1.73, 1.67, 1.5, 1.45, 1.31,
                 1.28, 1.11, 1.1, 1.13, 0.9, 0.74, 0.66, 0.67, 0.57, 0.51, 0.49]
        plt.scatter(LET4, GVal4, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')

        # Pastina and LaVerne (2001)
        LET5  = [0.23, 13.8, 34.8, 156.0]
        GVal5 = [2.70, 1.18, 0.63, 0.35]
        plt.scatter(LET5, GVal5, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')


    elif part_type == CARBON and mole_type == 'eaq':
        # Appleby and Schwarz (1969)
        LET1  = [0.30, 5.04, 22.34, 48.86]
        GVal1 = [2.7, 1.48, 0.72, 0.42]
        plt.scatter(LET1, GVal1, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')

        # Pastina and LaVerne (2001)
        LET2  = [0.23, 13.8, 34.8, 156.0]
        GVal2 = [2.60, 0.90, 0.30, 0.15]
        plt.scatter(LET2, GVal2, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

        # Wasseline-Trupin. V. (2002)
        LET3  = [32.5]
        GVal3 = [1.95]
        plt.scatter(LET3, GVal3, label="Exp. Wasseline-Trupin. V. (2002)",
                    color='red', s=MARKER_SIZE2)

        # S.Yamashita (2008)
        LET4 = [11.05, 13.50, 14.53, 21.54, 106.90]
        LET4_err = [0.0, 0.0, 0.0, 0.0, 35.27]
        GVal4 = [1.98, 1.95, 1.89, 1.62, 0.87]
        GVal4_err = [0.204, 0.195, 0.195, 0.165, 0.06]
        plt.errorbar(LET4, GVal4, xerr=LET4_err, yerr=GVal4_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')

    elif part_type == CARBON and mole_type == 'H':
        # Appleby and Schwarz (1969)
        LET2  = [0.30, 5.04, 22.34, 48.86]
        GVal2 = [0.61, 0.64, 0.42, 0.27]
        plt.scatter(LET2, GVal2, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET3  = [0.23, 13.8, 34.8, 156.0]
        GVal3 = [0.66, 0.57, 0.20, 0.10]
        plt.scatter(LET3, GVal3, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == CARBON and mole_type == 'H2':
        # Anderson and Hart (1961)
        LET1  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal1 = [0.6, 0.61, 0.6, 0.65, 0.62, 0.68, 0.63, 0.64, 0.68, 0.71, 0.77,
                 0.79, 0.8, 0.87, 0.95, 1.01, 0.94, 1.09, 1.01, 1.07]
        plt.scatter(LET1, GVal1, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Appleby and Schwarz (1969)
        LET2  = [0.30, 5.04, 22.34, 48.86]
        GVal2 = [0.43, 0.68, 0.95, 1.1]
        plt.scatter(LET2, GVal2, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET3  = [0.23, 13.8, 34.8, 156.0]
        GVal3 = [0.45, 0.64, 0.90, 1.20]
        plt.scatter(LET3, GVal3, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    #---------------------------------------------------------------------------
    # for Alpha Particles
    elif part_type == ALPHA and mole_type == 'H2O2':
        # Pastina, B. (1999)
        LET1  = [78.2668728, 92.67710884, 157.821126]
        GVal1 = [0.955485487, 0.970918997, 0.989585747]
        GVal1_err = [0.047774274, 0.04854595, 0.049479287]
        plt.errorbar(LET1, GVal1, yerr= GVal1_err, ms=MARKER_SIZE1, fmt='o',
                     label="Exp. Pastina, B. (1999)", color='orange')
        # LaVerne, J.A. (2000)
        LET2  = [147.1385765]
        GVal2 = [1.041036415]
        plt.scatter(LET2, GVal2, label="Exp. LaVerne, J.A. (2000)",
                    color='red', s=MARKER_SIZE2)
        # Anderson, A.R. (1961)
        LET3  = [48.11365995, 67.15058821, 81.63165779, 96.95226034,
                 92.84999265, 106.8074671, 109.7843131, 117.7212172,
                 166.3262766]
        GVal3 = [0.82, 0.95, 0.9, 0.91, 0.97, 0.98, 0.94, 1.09, 1.24]
        plt.scatter(LET3, GVal3, label="Exp. Anderson, A.R. (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Appleby, A. (1969)
        LET4  = [55.67086649, 102.407995]
        GVal4 = [1.001820728, 1.080252101]
        plt.scatter(LET4, GVal4, label="Exp. Appleby, A. (1969)",
                    color='m', s=MARKER_SIZE2, marker='h')
        # Burns, W.G. (1981)
        LET5  = [55, 80, 140]
        GVal5 = [0.820624235, 0.830096716, 0.944599858]
        plt.scatter(LET5, GVal5, label="Exp. Burns, W.G. (1981)",
                    color='y', s=MARKER_SIZE2, marker='*')
        # S.Yamashita (2008)
        LET6 = [2.23, 26.3]
        LET6_err = [0.0, 12.8]
        GVal6 = [0.686, 0.833]
        GVal6_err = [0.0735, 0.0857]
        plt.errorbar(LET6, GVal6, xerr=LET6_err, yerr=GVal6_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')

    elif part_type == ALPHA and mole_type == 'HO2 + O2-':
        # Meesungnoen, J. (2005)
        LET1 = [26.58187561, 34.29281383, 43.37512062, 53.44358296,
               66.27262102, 80.58421878, 96.71423267, 116.8278187,
               140.2083723, 167.1733608, 198.0304623, 214.1358811]
        GVal1 = [0.018746951, 0.020148864, 0.022163461, 0.024568595,
                0.027873413, 0.031622777, 0.035784197, 0.040912576,
                0.046535591, 0.052795278, 0.059589232, 0.062901024]
        plt.plot(LET1, GVal1, label="MC Sim. Meesungnoen, J. (2005)",
                 color='red', linewidth=LINEWIDTH)
        # Baldacchino, G. (1998) for S ions
        LET2 =  [249.3550334]
        GVal2 = [0.058414621]
        plt.plot(LET2, GVal2, label="Exp. Baldacchino, G. (1998) for S ions",
                 color='m', marker='x', markersize=MARKER_SIZE1, linewidth=0.0)
        # Baldacchino, G. (1998) for Ar ions
        LET3  = [291.719245]
        GVal3 = [0.050642234]
        plt.plot(LET3, GVal3, label="Exp. Baldacchino, G. (1998) for Ar ions",
                 color='y', marker='+', markersize=MARKER_SIZE1, linewidth=0.0)

    elif part_type == ALPHA and mole_type == 'OH':
        # S.Yamashita (2008)
        LET1 = [2.23, 9.05]
        LET1_err = [0.0, 3.22]
        GVal1 = [2.58, 1.89]
        GVal1_err = [0.255, 0.195]
        plt.errorbar(LET1, GVal1, xerr=LET1_err, yerr=GVal1_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')

        # T.Maeyama (2011)
        LET2 = [2.206, 3.429, 4.445, 6.560, 6.732]
        GVal2 = [2.739, 2.413, 2.283, 2.087, 2.120]
        GVal2_err = [0.261, 0.245, 0.228, 0.196, 0.196]
        plt.errorbar(LET2, GVal2, yerr=GVal2_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. T.Maeyama (2011) @100ns",
                     color='crimson')

        # Burns and Sims (1981)
        LET3  = [0.3, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal3 = [2.67, 2.55, 2.43, 2.33, 2.14, 2.07]
        plt.scatter(LET3, GVal3, label="Exp. Burns and Sims (1981)",
                    color='blue', s=MARKER_SIZE2, marker='<')
        # McCracken et al. (1998)
        LET4  = [0.4, 1.0, 3.0, 5.0, 6.0]
        GVal4 = [2.74, 2.57, 2.24, 2.05, 2.00]
        plt.scatter(LET4, GVal4, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Anderson and Hart (1961)
        LET5  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal5 = [1.89, 1.90, 1.88, 1.79, 1.73, 1.67, 1.5, 1.45, 1.31,
                 1.28, 1.11, 1.1, 1.13, 0.9, 0.74, 0.66, 0.67, 0.57, 0.51, 0.49]
        plt.scatter(LET5, GVal5, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Pastina and LaVerne (2001)
        LET6  = [0.23, 13.8, 34.8]
        GVal6 = [2.70, 1.18, 0.63]
        plt.scatter(LET6, GVal6, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')


    elif part_type == ALPHA and mole_type == 'eaq':
        # S.Yamashita (2008)
        LET1 = [2.23, 26.3]
        LET1_err = [0.0, 12.8]
        GVal1 = [2.43, 1.2]
        GVal1_err = [0.24, 0.12]
        plt.errorbar(LET1, GVal1, xerr=LET1_err, yerr=GVal1_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. S.Yamashita (2008) @100ns",
                     color='teal')

        # McCracken et al. (1998)
        LET2  = [0.4, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal2 = [2.67, 2.50, 2.29, 2.17, 1.95, 1.88]
        plt.scatter(LET2, GVal2, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Appleby and Schwarz (1969)
        LET3  = [0.30, 5.04, 22.34, 48.86]
        GVal3 = [2.7, 1.48, 0.72, 0.42]
        plt.scatter(LET3, GVal3, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET4  = [0.23, 13.8, 34.8]
        GVal4 = [2.60, 0.90, 0.30]
        plt.scatter(LET4, GVal4, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == ALPHA and mole_type == 'H':
        # Appleby and Schwarz (1969)
        LET1  = [0.30, 5.04, 22.34, 48.86]
        GVal1 = [0.61, 0.64, 0.42, 0.27]
        plt.scatter(LET1, GVal1, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET2  = [0.23, 13.8, 34.8]
        GVal2 = [0.66, 0.57, 0.20]
        plt.scatter(LET2, GVal2, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == ALPHA and mole_type == 'H2':
        # Burns and Sims (1981)
        LET1  = [0.3, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal1 = [0.42, 0.47, 0.50, 0.53, 0.55, 0.57]
        plt.scatter(LET1, GVal1, label="Exp. Burns and Sims (1981)",
                    color='blue', s=MARKER_SIZE2, marker='<')
        # McCracken et al. (1998)
        LET2  = [0.4, 1.0, 3.0, 5.0, 6.0]
        GVal2 = [0.41, 0.44, 0.50, 0.55, 0.56]
        plt.scatter(LET2, GVal2, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Anderson and Hart (1961)
        LET3  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal3 = [0.6, 0.61, 0.6, 0.65, 0.62, 0.68, 0.63, 0.64, 0.68, 0.71, 0.77,
                 0.79, 0.8, 0.87, 0.95, 1.01, 0.94, 1.09, 1.01, 1.07]
        plt.scatter(LET3, GVal3, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Appleby and Schwarz (1969)
        LET4  = [0.30, 5.04, 22.34, 48.86]
        GVal4 = [0.43, 0.68, 0.95, 1.1]
        plt.scatter(LET4, GVal4, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET5  = [0.23, 13.8, 34.8]
        GVal5 = [0.45, 0.64, 0.90]
        plt.scatter(LET5, GVal5, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    #---------------------------------------------------------------------------
    # for Protons
    if part_type == PROTON and mole_type == 'H2O2':
        # Pastina and LaVerne (1999)
        LET1  = [10.30016156, 13.84472723, 20.85768263, 34.92362096,
                 78.2668728, 92.67710884]
        GVal1 = [0.6781332, 0.75812819, 0.791074929, 0.769802424,
                 0.955485487, 0.970918997]
        GVal1_err = [0.03390666, 0.03790641, 0.039553746, 0.038490121,
                     0.047774274, 0.04854595]
        plt.errorbar(LET1, GVal1, yerr= GVal1_err, ms=MARKER_SIZE1, fmt='o',
                     label="Exp. Pastina and LaVerne (1999)", color='orange')
        # Wasseline-Trupin. V. (2002)
        LET2  = [2.488092114]
        GVal2 = [0.646778711]
        plt.scatter(LET2, GVal2, label="Exp. Wasseline-Trupin. V. (2002)",
                    color='red', s=MARKER_SIZE2)
        # Anderson, A.R. (1961)
        LET3  = [14.83773685, 16.38703888, 17.48046071, 19.02064819]
        GVal3 = [0.73, 0.68, 0.73, 0.77]
        plt.scatter(LET3, GVal3, label="Exp. Anderson, A.R. (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Burns, W.G. (1981)
        LET4  = [13.5]
        GVal4 = [0.716895648]
        plt.scatter(LET4, GVal4, label="Exp. Burns, W.G. (1981)",
                    color='y', s=MARKER_SIZE2, marker='*')
        # Burns and Sims (1981)
        LET5  = [0.3, 1.0, 2.0, 3.0, 4.0, 5.0]
        GVal5 = [0.60, 0.62, 0.64, 0.65, 0.67, 0.69]
        plt.scatter(LET5, GVal5, label="Exp. Burns and Sims (1981)",
                    color='blue', s=MARKER_SIZE2, marker='<')
        # McCracken et al. (1998)
        LET6  = [0.4, 1.0, 3.0, 5.0, 6.0]
        GVal6 = [0.61, 0.65, 0.71, 0.74, 0.75]
        plt.scatter(LET6, GVal6, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Appleby and Schwarz (1969)
        LET7  = [0.30, 5.04, 22.34, 48.86]
        GVal7 = [0.61, 0.91, 1.00, 1.08]
        plt.scatter(LET7, GVal7, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET8  = [0.23, 13.8, 34.8]
        GVal8 = [0.70, 0.74, 0.76]
        plt.scatter(LET8, GVal8, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == PROTON and mole_type == 'HO2 + O2':
        LET  = [8, 10, 20, 40, 60]
        GVal = [0.024467648, 0.026215337, 0.038449161, 0.05942143, 0.075150633]
        plt.scatter(LET, GVal, label="Exp. H. C Laverne", color='red',
                    s=MARKER_SIZE2)

    elif part_type == PROTON and mole_type == 'HO2 + O2-':
        # MC Sim. Gervais, B. (2006)
        LET1 = [3.286241007, 3.646582564, 4.00999119,  4.449693107,
                5.166103049, 5.970785015, 6.900805764, 8.420615,
                10.18259677, 12.53810282, 15.16165332, 18.16904178,
                22.37202544, 26.80962666, 32.41943942, 40.28172451,
                47.83703389, 58.37249431, 63.61160387]
        GVal1 = [0.023058734, 0.023058734, 0.023750496, 0.023981084,
                 0.024672846, 0.02513402,  0.025825782, 0.026748132,
                 0.028131656, 0.029976354, 0.03205164,  0.034588101,
                 0.037585737, 0.040583372, 0.044503357, 0.049345691,
                 0.054188025, 0.060875058, 0.063411519]
        plt.plot(LET1, GVal1, label="MC Sim. Gervais, B. (2006)",
                 color='orange', linewidth=LINEWIDTH)
        # MC Sim. Meesungnoen, J. (2005)
        LET2 = [9.039554779, 11.28783987, 13.55141658, 16.16212928,
                19.65551105, 24.21840661, 29.45398363, 36.52970597,
                45.60456185, 58.06300136, 70.16706534]
        GVal2 = [0.016524214, 0.017397713, 0.01865063, 0.020148864,
                 0.022220619, 0.024568595, 0.026955586, 0.029803889,
                 0.032699521, 0.035784197, 0.038065971]
        plt.plot(LET2, GVal2, label="MC Sim. Meesungnoen, J. (2005)",
                 color='red', linewidth=LINEWIDTH)

    elif part_type == PROTON and mole_type == 'eaq':
        # McCracken et al. (1998)
        LET1  = [0.4, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal1 = [2.67, 2.50, 2.29, 2.17, 1.95, 1.88]
        plt.scatter(LET1, GVal1, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Appleby and Schwarz (1969)
        LET2  = [0.30, 5.04, 22.34, 48.86]
        GVal2 = [2.7, 1.48, 0.72, 0.42]
        plt.scatter(LET2, GVal2, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Elliot et al. (1993)
        LET3  = [0.3]
        GVal3 = [2.64]
        plt.scatter(LET3, GVal3, label="Exp. Elliot et al. (1993)",
                    color='darkcyan', s=MARKER_SIZE2, marker='s')
        # Pastina and LaVerne (2001)
        LET4  = [0.23, 13.8, 34.8]
        GVal4 = [2.60, 0.90, 0.30]
        plt.scatter(LET4, GVal4, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == PROTON and mole_type == 'OH':
        # T.Maeyama (2011)
        LET1 = [0.523, 0.595, 0.771]
        GVal1 = [3.065, 2.804, 2.739]
        GVal1_err = [0.293, 0.261, 0.261]
        plt.errorbar(LET1, GVal1, yerr=GVal1_err, ms=MARKER_SIZE1,
                     fmt='o', label="Exp. T.Maeyama (2011) @100ns",
                     color='crimson')
        # Burns and Sims (1981)
        LET2  = [0.3, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal2 = [2.67, 2.55, 2.43, 2.33, 2.14, 2.07]
        plt.scatter(LET2, GVal2, label="Exp. Burns and Sims (1981)",
                    color='blue', s=MARKER_SIZE2, marker='<')
        # McCracken et al. (1998)
        LET3  = [0.4, 1.0, 3.0, 5.0, 6.0]
        GVal3 = [2.74, 2.57, 2.24, 2.05, 2.00]
        plt.scatter(LET3, GVal3, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Elliot et al. (1993)
        LET4  = [0.3]
        GVal4 = [2.86]
        plt.scatter(LET4, GVal4, label="Exp. Elliot et al. (1993)",
                    color='darkcyan', s=MARKER_SIZE2, marker='s')
        # Anderson and Hart (1961)
        LET5  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal5 = [1.89, 1.90, 1.88, 1.79, 1.73, 1.67, 1.5, 1.45, 1.31,
                 1.28, 1.11, 1.1, 1.13, 0.9, 0.74, 0.66, 0.67, 0.57, 0.51, 0.49]
        plt.scatter(LET5, GVal5, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Pastina and LaVerne (2001)
        LET6  = [0.23, 13.8, 34.8]
        GVal6 = [2.70, 1.18, 0.63]
        plt.scatter(LET6, GVal6, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == PROTON and mole_type == 'H':
        # Elliot et al. (1993)
        LET1  = [0.3]
        GVal1 = [0.56]
        plt.scatter(LET1, GVal1, label="Exp. Elliot et al. (1993)",
                    color='darkcyan', s=MARKER_SIZE2, marker='s')
        # Appleby and Schwarz (1969)
        ET2  = [0.30, 5.04, 22.34, 48.86]
        GVal2 = [0.61, 0.64, 0.42, 0.27]
        plt.scatter(LET2, GVal2, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET3  = [0.23, 13.8, 34.8]
        GVal3 = [0.66, 0.57, 0.20]
        plt.scatter(LET3, GVal3, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    elif part_type == PROTON and mole_type == 'H2':
        # Elliot et al. (1993)
        LET1  = [0.3]
        GVal1 = [0.45]
        plt.scatter(LET1, GVal1, label="Exp. Elliot et al. (1993)",
                    color='darkcyan', s=MARKER_SIZE2, marker='s')
        # Burns and Sims (1981)
        LET2  = [0.3, 1.0, 2.0, 3.0, 5.0, 6.0]
        GVal2 = [0.42, 0.47, 0.50, 0.53, 0.55, 0.57]
        plt.scatter(LET2, GVal2, label="Exp. Burns and Sims (1981)",
                    color='blue', s=MARKER_SIZE2, marker='<')
        # McCracken et al. (1998)
        LET3  = [0.4, 1.0, 3.0, 5.0, 6.0]
        GVal3 = [0.41, 0.44, 0.50, 0.55, 0.56]
        plt.scatter(LET3, GVal3, label="Exp. McCracken et al. (1998)",
                    color='c', s=MARKER_SIZE2, marker='>')
        # Anderson and Hart (1961)
        LET4  = [4.475, 4.542, 5.513, 6.101, 6.242, 6.803, 7.376, 7.652, 8.741,
                 9.75,  10.8,  12.03, 18.08, 28.39, 37.47, 44.56, 48.26, 54.77,
                 57.28, 63.56]
        GVal4 = [0.6, 0.61, 0.6, 0.65, 0.62, 0.68, 0.63, 0.64, 0.68, 0.71, 0.77,
                 0.79, 0.8, 0.87, 0.95, 1.01, 0.94, 1.09, 1.01, 1.07]
        plt.scatter(LET4, GVal4, label="Exp. Anderson and Hart (1961)",
                    color='green', s=MARKER_SIZE2, marker='^')
        # Appleby and Schwarz (1969)
        LET5  = [0.30, 5.04, 22.34, 48.86]
        GVal5 = [0.43, 0.68, 0.95, 1.1]
        plt.scatter(LET5, GVal5, label="Exp. Appleby and Schwarz (1969)",
                    color='m', s=MARKER_SIZE2, marker='d')
        # Pastina and LaVerne (2001)
        LET6  = [0.23, 13.8, 34.8]
        GVal6 = [0.45, 0.64, 0.90]
        plt.scatter(LET6, GVal6, label="Exp. Pastina and LaVerne (2001)",
                    color='orangered', s=MARKER_SIZE2, marker='+')

    plt.legend(fontsize=LEGEND_FONT_SIZE)

#-------------------------------------------------------------------------------
# function to get LET computed by SRIM
def get_SRIM_LET(part_type, plt):
    if part_type == CARBON:
        # SRIM code
        energy1 = [6.5, 10.0, 12.0, 24.0, 48.0, 96.0, 120.0, 240.0, 480.0,
                   960.0, 1920.0, 3840.0]
        LET1 = [914.6, 802.5, 746.6, 525.0, 341.0, 208.3, 174.5, 85.56, 49.1,
                30.79, 18.49, 12.0]
        plt.plot(energy1, LET1, label="Carbon (SRIM code)", color='orange',
                marker='o', linewidth=0, markersize=MARKER_SIZE1)
        # Ref.) Pastina, B. (1999)
        energy2 = [10, 20, 30]
        LET2 = [787, 703, 629]
        plt.plot(energy2, LET2, label="Ref.) Pastina, B. (1999)", color='red',
                 marker='^', linewidth=0, markersize=MARKER_SIZE1)

    elif part_type == ALPHA:
        # SRIM code
        energy1 = [1.0, 3.0, 6.0, 10.0, 20.0, 48.0, 100.0, 200.0, 399.9]
        LET1 = [2.227E+02, 1.287E+02, 8.023E+01, 5.641E+01, 3.257E+01,
                1.609E+01, 8.809E+00, 5.027E+00, 2.938E+00]
        plt.plot(energy1, LET1, label="Alpha (SRIM code)", color='orange',
                marker='o', linewidth=0, markersize=MARKER_SIZE1)
        # Ref.) Pastina, B. (1999)
        energy2 = [5, 15, 20]
        LET2 = [156, 92.1, 78]
        plt.plot(energy2, LET2, label="Ref.) Pastina, B. (1999)", color='red',
                 marker='^', linewidth=0, markersize=MARKER_SIZE1)

    elif part_type == PROTON:
        # SRIM code
        energy1 = [0.2, 0.5, 1.0, 2.0, 4.0, 10.0, 25.0, 50.0, 99.9]
        LET1 = [6.437E+01, 3.691E+01, 2.402E+01, 1.642E+01, 9.673E+00,
                4.657E+00, 2.207E+00, 1.261E+00, 7.371E-01]
        plt.plot(energy1, LET1, label="Proton (SRIM code)", color='orange',
                 marker='o', linewidth=0, markersize=MARKER_SIZE1)
        # Ref.) Pastina, B. (1999)
        energy2 = [2, 5, 10, 15]
        LET2 = [34.8, 20.7, 13.8, 10.3]
        plt.plot(energy2, LET2, label="Ref.) Pastina, B. (1999)", color='red',
                 marker='^', linewidth=0, markersize=MARKER_SIZE1)

    plt.legend(fontsize=LEGEND_FONT_SIZE)

#-------------------------------------------------------------------------------
def get_plot_title(mole_type):
    plot_title = {
        'OH'        : '$^{\cdot}\mathrm{OH}$',
        'eaq'       : '$\mathrm{e}^{-}_{\mathrm{aq}}$',
        'H'         : '$\mathrm{H}^{\cdot}$',
        'H2'        : '$\mathrm{H}_2$',
        'H2O2'      : '$\mathrm{H}_2\mathrm{O}_2$',
        'HO2'       : '$\mathrm{HO}_2^{\cdot}$',
        'O2'        : '$\mathrm{O}_2$',
        'O2m'       : '$\mathrm{O}_2^{\cdot-}$',
        'HO2 + O2'  : '$\mathrm{HO}_2^{\cdot} + \mathrm{O}_2$',
        'HO2 + O2-' : '$\mathrm{HO}_2^{\cdot} + \mathrm{O}_2^{-}$',
    }
    if mole_type in plot_title:
        return plot_title[mole_type]
    else:
        return mole_type

#-------------------------------------------------------------------------------
# function to make G-value plot
def make_gval_plot(pc, x1, y1, mole_type, pdf, df_in, x2=None, y2=None):

    # simulation result
    fig = plt.figure(figsize = (FIG_SIZE_X, FIG_SIZE_Y), dpi = DPI)
    plt.plot(x1, y1, linewidth = LINEWIDTH, marker='o',
             label=f"{GVAL_LABEL} w/ multiple-ionization",
             markersize=0)
    plt.title(get_plot_title(mole_type), fontsize = FONT_SIZE)
    plt.xlabel('LET (keV/um)', fontsize = FONT_SIZE)
    plt.ylabel('G-Value (#mole./100eV)', fontsize = FONT_SIZE)
    plt.tick_params(labelsize = FONT_SIZE)
    plt.minorticks_on()
    plt.xscale("log")
    plt.grid(linestyle="dotted")
    plt.xlim(pc.get_gval_axis_lim())

    # without multiple-ionization
    if df_in is not None and mole_type == 'H2O2':
        x3 = df_in['LET'].tolist()
        y3 = df_in[mole_type].tolist()
        plt.plot(x3, y3, linewidth=LINEWIDTH, linestyle='dashed', marker='o',
                 label=f"{GVAL_LABEL} w/o multiple-ionization",
                 markersize=0)

    if mole_type == 'HO2 + O2' or mole_type == 'HO2 + O2-' or \
            mole_type == 'HO2' or mole_type == 'O2' or mole_type == 'O2m':
        plt.ylim(bottom=0.0, top=0.1)
    elif mole_type == 'H2O2':
        plt.ylim(bottom=0.0, top=1.4)
    elif mole_type == 'H':
        plt.ylim(bottom=0.0, top=1.0)
    elif mole_type == 'H2':
        plt.ylim(bottom=0.0, top=1.6)
    else:
        plt.ylim(bottom=0.0, top=4.0)

    if x2 is not None and y2 is not None:
        label = f"{GVAL_LABEL} w/ multiple-ionization (Track-averaged G)"
        plt.plot(x2, y2, linewidth = LINEWIDTH, marker='o', label=label,
                 markersize=0, color="magenta")

    # other simulation data (ex.Geant4-DNA)
    #if COMP_WITH_OTHER_GVAL:
    #    counter = 0
    #    for dataset, label2 in zip (OTHER_GVAL_DATADIR, OTHER_GVAL_LABEL):
    #        get_other_simdata(counter, plt, mole_type, x, os.getcwd(),
    #                          dataset, label2)
    #        counter += 1

    # experiment data
    get_expdata(pc.get_particle_kind(), mole_type, plt)

    # save plot
    pdf.savefig(fig)
    plt.clf()

#-------------------------------------------------------------------------------
# function to make LET plot
def make_LET_plot(pc, LET, pdf, df_in=None):
    fig = plt.figure(figsize = (FIG_SIZE_X, FIG_SIZE_Y), dpi = DPI)
    label=f"{LET_LABEL} w/ multiple-ionization"
    plt.plot(pc.get_energy_range(), LET, linewidth = LINEWIDTH, marker='o',
             label=label, markersize=0)
    plt.tick_params(labelsize = FONT_SIZE)
    plt.xlabel('Incident Energy (MeV)', fontsize = FONT_SIZE)
    plt.ylabel('LET (keV/um)', fontsize = FONT_SIZE)
    plt.minorticks_on()
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(linestyle="dotted")
    xlim, ylim = pc.get_LET_axis_lim()
    plt.xlim(xlim)
    plt.ylim(ylim)

    # without multiple-ionization
    if df_in is not None:
        label=f"{LET_LABEL} w/o multiple-ionization"
        x = df_in['Ekin'].tolist()
        y = df_in['LET'].tolist()
        plt.plot(x, y, linewidth=LINEWIDTH, linestyle='dashed', marker='o',
                 label=label, markersize=0)

    # other simulation data (ex.Geant4-DNA)
    #if COMP_WITH_OTHER_LET:
    #    counter = 0
    #    for dataset, label2 in zip (OTHER_SIMDATA_DIR, OTHER_GVAL_LABEL):
    #        get_other_simdata_LET(counter, plt, os.getcwd(), dataset, label2)
    #        counter += 1

    # get SRIM code data
    get_SRIM_LET(pc.get_particle_kind(), plt)

    # save plot
    pdf.savefig(fig)

#-------------------------------------------------------------------------------
def get_data(particle_kind, filename, ekin):
    def get_dir_prefix(particle_kind):
        if particle_kind == 'proton':
            return 'p'
        elif particle_kind == 'alpha':
            return 'Alpha'
        elif particle_kind == 'carbon':
            return 'C'
    dirname = f'{get_dir_prefix(particle_kind)}{ekin}MeV'
    if not os.path.exists(dirname):
        return None
    os.chdir(dirname)
    array = np.loadtxt(filename, skiprows=1, delimiter=',')
    os.chdir('../')
    return array

#-------------------------------------------------------------------------------
def getDirectory(particle_kind, ekin):
    def get_dir_prefix(particle_kind):
        if particle_kind == 'proton':
            return 'p'
        elif particle_kind == 'alpha':
            return 'Alpha'
        elif particle_kind == 'carbon':
            return 'C'
    dirname = f'{get_dir_prefix(particle_kind)}{ekin}MeV'
    if not os.path.exists(dirname):
        print(f"[ERROR] {dirname} is not found...")
        sys.exit()
    return dirname

#-------------------------------------------------------------------------------
def getLET(particle_kind, ekin):
    dirname = getDirectory(particle_kind, ekin)
    os.chdir(dirname)
    array = np.loadtxt(LET_FILE, skiprows=1, delimiter=',')
    os.chdir('../')
    return array

#-------------------------------------------------------------------------------
def getGval(filename, particle_kind, ekin):
    dirname = getDirectory(particle_kind, ekin)
    os.chdir(dirname)
    array = np.loadtxt(filename, skiprows=1, delimiter=',')
    os.chdir('../')
    return array

#-------------------------------------------------------------------------------
# main function
def main(particle_kind, trkG, csv_out, file_in):

    df_in = None
    if file_in is not None:
        print(f"[MESSAGE] Input File: {file_in}")
        df_in = pd.read_csv(file_in)

    LET1 = []
    H2O2 = []
    HO2_plus_O2  = []
    HO2_plus_O2m = []
    HO2  = []
    O2   = []
    O2m  = []
    eaq  = []
    OH   = []
    H    = []
    H2   = []

    # LET vs track-averaged G
    LET2    = [] if trkG else None
    trkH2O2 = [] if trkG else None

    pc = ParticleConfigs(particle_kind)

    print(f"[MESSAGE] Particle Kind: {pc.get_particle_kind()}")

    for ekin in pc.get_energy_range():
        LET1.append(getLET(particle_kind, ekin)[LETData.Mean])
        gval = getGval(GVAL_FILE1, particle_kind, ekin)
        if gval.ndim == 1:
            H2O2.append(gval[Molecule.H2O2])
            HO2_plus_O2.append(gval[Molecule.HO2_plus_O2])
            HO2_plus_O2m.append(gval[Molecule.HO2_plus_O2m])
            HO2.append(gval[Molecule.HO2])
            O2.append(gval[Molecule.O2])
            O2m.append(gval[Molecule.O2m])
            eaq.append(gval[Molecule.eaq])
            OH.append(gval[Molecule.OH])
            H.append(gval[Molecule.H])
            H2.append(gval[Molecule.H2])
        elif gval.ndim == 2:
            H2O2.append(gval[0][Molecule.H2O2])
            HO2_plus_O2.append(gval[0][Molecule.HO2_plus_O2])
            HO2_plus_O2m.append(gval[0][Molecule.HO2_plus_O2m])
            HO2.append(gval[0][Molecule.HO2])
            O2.append(gval[0][Molecule.O2])
            O2m.append(gval[0][Molecule.O2m])
            eaq.append(gval[0][Molecule.eaq])
            OH.append(gval[0][Molecule.OH])
            H.append(gval[0][Molecule.H])
            H2.append(gval[0][Molecule.H2])

        # track-averaged G-value
        if trkG and ekin / pc.get_mass_number() <= ELIM_LOW2:
            LET2.append(getLET(particle_kind, ekin)[LETData.Mean])
            trkGval = getGval(GVAL_FILE2, particle_kind, ekin)
            trkH2O2.append(trkGval[Molecule.H2O2])

    with PdfPages(PDF_FILE) as pdf:
        make_LET_plot(pc, LET1, pdf, df_in)
        make_gval_plot(pc, LET1, H2O2,        'H2O2',      pdf, df_in, LET2, trkH2O2)
        make_gval_plot(pc, LET1, HO2_plus_O2, 'HO2 + O2',  pdf, df_in)
        make_gval_plot(pc, LET1, HO2_plus_O2m,'HO2 + O2-', pdf, df_in)
        make_gval_plot(pc, LET1, HO2,         'HO2',       pdf, df_in)
        make_gval_plot(pc, LET1, O2,          'O2',        pdf, df_in)
        make_gval_plot(pc, LET1, O2m,         'O2m',       pdf, df_in)
        make_gval_plot(pc, LET1, eaq,         'eaq',       pdf, df_in)
        make_gval_plot(pc, LET1, OH,          'OH',        pdf, df_in)
        make_gval_plot(pc, LET1, H,           'H',         pdf, df_in)
        make_gval_plot(pc, LET1, H2,          'H2',        pdf, df_in)
        for _LET, _H2O2, _eaq, _OH in zip(LET1, H2O2, eaq, OH):
            print("* LET: %f (keV/um), G(H2O2): %f, G(eaq): %f, G(OH): %f" %
                  (_LET, _H2O2, _eaq, _OH))

    if csv_out:
        df_out = pd.DataFrame({'Ekin': pc.get_energy_range(),
                               'LET' : LET1, 'eaq': eaq, 'OH': OH,
                               'H2O2': H2O2, 'H'  : H,   'H2': H2})
        df_out.to_csv(CSV_OUT, header=True, index=False)

#===============================================================================
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Make Plots")
    parser.add_argument('-p', '--particle_type', default=CARBON)
    parser.add_argument('-t', '--trkG', action='store_true')
    parser.add_argument('-c', '--csv_out', action='store_true')
    parser.add_argument('-f', '--file_in', default=None)
    args = parser.parse_args()
    main(args.particle_type, args.trkG, args.csv_out, args.file_in)
