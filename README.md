# Data-Driven Collective Variables for Enhanced Sampling
<p align="center">
  <img height="350" src="https://raw.githubusercontent.com/luigibonati/luigibonati.github.io/master/images/jpcl2020.png">
</p>

Code and inputs file related to the paper [**Data-driven collective variables for enhanced sampling**](https://pubs.acs.org/doi/10.1021/acs.jpclett.0c00535) by _Bonati, Rizzi and Parrinello_, J. Phys. Chem. Lett. 11, 2998-3004 (2020).

#### Requirements
- Pytorch and LibTorch (v >=1.2)
- PLUMED2

#### Tutorial
We have released a [Google Colab notebook](https://colab.research.google.com/drive/1dG0ohT75R-UZAFMf_cbYPNQwBaOsVaAA) with the code and instructions for Deep-LDA CV training and export.

#### Code and results availability
The code and input files are available also on PLUMED-NEST as [plumId:20.004](https://www.plumed-nest.org/eggs/20/004/) while the results of the simulations are available in the Materials Cloud repository as [materialscloud:2020.0035](https://archive.materialscloud.org/2020.0035/v1).

#### Errata
There is a typo in the definition of <img src="https://tex.s2cms.ru/svg/%5Cwidetilde%7B%5Cmathbf%7Bw%7D%7D_i" alt="\widetilde{\mathbf{w}}_i" /> below eq. 7. The correct formula is
<img src="https://tex.s2cms.ru/svg/%5Cwidetilde%7B%5Cmathbf%7Bw%7D%7D_i%3D%5Cmathbf%7BL%7D%5ET%20%5Cmathbf%7Bw%7D_i" alt="\widetilde{\mathbf{w}}_i=\mathbf{L}^T \mathbf{w}_i" /> .

#### Contact
If you have comments or questions please send an email to luigi bonati [at] phys chem ethz ch .
