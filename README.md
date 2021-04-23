# Data-driven collective variables for enhanced sampling
#### Luigi Bonati, Valerio Rizzi, and Michele Parrinello, _J. Phys. Chem. Lett._ 11, 2998-3004 (2020).

[![DOI](http://img.shields.io/badge/DOI-10.1073%2Fpnas.1811056115-yellow)](https://doi.org/10.1073/pnas.1811056115)
[![arXiv](https://img.shields.io/badge/arXiv-2002.06562-critical)](https://arxiv.org/abs/1806.06006)
[![plumID:20.004](https://img.shields.io/badge/plumID-20.004-blue)](https://www.plumed-nest.org/eggs/20/004/)
[![MaterialsCloud](https://img.shields.io/badge/MaterialsCloud-2020.0035-lightgrey)](https://doi.org/10.24435/materialscloud:2020.0035/v1)

This repository contains:
1. the code necessary to train and use a neural-network collective variable optimized with Fisher's discriminant analysis 
2. input file to reproduce the simulations reported in the paper

#### Requirements
- Pytorch and LibTorch (v == 1.4)
- PLUMED2

#### Tutorial
Here you can find a [Google Colab notebook](https://colab.research.google.com/drive/1dG0ohT75R-UZAFMf_cbYPNQwBaOsVaAA) with the code and instructions for Deep-LDA CV training and export.

#### Code and results availability
The code and input files are available also on the [PLUMED-NEST](https://www.plumed-nest.org/eggs/20/004/) archive while the results of the simulations are available in the [Materials Cloud repository](https://archive.materialscloud.org/2020.0035/v1).

#### Errata
There is a typo in the definition of <img src="https://tex.s2cms.ru/svg/%5Cwidetilde%7B%5Cmathbf%7Bw%7D%7D_i" alt="\widetilde{\mathbf{w}}_i" /> below eq. 7. The correct formula is
<img src="https://tex.s2cms.ru/svg/%5Cwidetilde%7B%5Cmathbf%7Bw%7D%7D_i%3D%5Cmathbf%7BL%7D%5ET%20%5Cmathbf%7Bw%7D_i" alt="\widetilde{\mathbf{w}}_i=\mathbf{L}^T \mathbf{w}_i" /> .

#### Contact
If you have comments or questions please send an email to luigi bonati [at] phys chem ethz ch .
