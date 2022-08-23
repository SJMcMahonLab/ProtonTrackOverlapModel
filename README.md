# Proton Track Overlap Model
TOPAS-nBio 1.0 (TOPAS 3.6.1) parameter files and MATLAB R2020a analysis codes used to model the distribution and yield of chemical species after proton irradiation, to investigate the possibility of physico-chemical inter-track interactions within ultra-high dose-rate proton therapy.
Files are separated into two folders, the AnalyticTrackModel and the InstantaneousDeliveryModel, which are summarised below.
The AnalyticTrackModel simulates proton tracks through a shallow cylindrical water phantom, recording the radial position of physical and chemical interactions around the track to estimate track dimensions for a geometric track overlap model.
The InstantaneousDeliveryModel simulates independently and instantaneously delivered proton tracks to a small water phantom, recording the energy-averaged G-Values of chemical species to compare any changes in yields across the chemical stage.

# Analytic Track Model 
**Topas-nBio Parameter Files**

**ProtonTrackRadius_Physics.txt**
This parameter file simulates proton irradiation through the centre of a shallow cylindrical water phantom, outputting the positional and energy deposition information of physical interactions around the track.

**ProtonTrackRadius_Chemistry.txt**
This parameter files simulates proton irradiation through the centre of a shallow cylindrical water phantom, outputting the positional information of chemical species around the track at given timepoints.

**TopasChemistry.txt**
This file defines the standard rate constants for relevant chemical processes.

**Bragg_Peak_Profile.txt** 
This file simulates proton irradiation in a 10 cm water phantom, outputting the dose deposition profile data.

**MATLAB Analysis Codes**

**Physics_TrackRadius_LET.m**
This code analyses pTuple data from **ProtonTrackRadius_Physics.txt**, outputting an energy-weighted histogram of the physical interactions as a function of radial distance around the proton track. The LET is also calculated using the total energy deposited and cylinder depth.

**Chemistry_TrackRadius_SpeciesSpecific.m**
This code analyses pTuple data from **ProtonTrackRadius_Chemistry.txt**, outputting an histogram of the chemical species as a function of radial distance around the proton track. The code can be used for specific species or at the 1 ps timepoint to exclude any excited or ionised water molecules which are still present.

**Chemistry_TrackRadius.m**
This code analyses pTuple data from **ProtonTrackRadius_Chemistry.txt**, outputting an histogram of the chemical species as a function of radial distance around the proton track. The code was used for all species for timepoints after 1 ps. 

# Instantaneous Delivery Model
**TOPAS-nBio Setup**

**MultipleSources.m**
This tool generates multiple beam sources (to produce a single Geant4 event) to create parameter file **Source_Simultaneous.txt** for an instantaneous proton delivery.

**TOPAS-nBio Parameter Files**

**Independent_Delivery.txt**
File for water phantom proton irradiation for independently delivered proton tracks, reads in **Source_Independent.txt**.

**Simultaneous_Delivery.txt**
File for water phantom proton irradiation for simultaneously delivered proton tracks, reads in **Source_Simultaneous.txt**.

**Source_Independent.txt**
File of the beam delivery information, using a single source with multiple tracks for individually delivered protons.

**Source_Simultaneous.txt**
File of the beam delivery information, using multiple sources each with an individual proton for instantaneously delivered protons.

**TOPAS-nBio Extensions**

**ScoreGValueEnergy.hh**

**ScoreGValueEnergy.cc**
Scoring extension files which calculate the G-Value of each chemical species, weighting by the total energy deposited.

# Contact 
For any questions or comments please contact sthompson76 (at) qub.ac.uk.

