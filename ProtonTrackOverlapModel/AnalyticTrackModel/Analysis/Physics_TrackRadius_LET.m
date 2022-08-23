clc;
close all;
%Reads in pTuple data files from topas simulation
%Calculates LET and energy-weighted histogram of the physical interactions
%with increasing radial distance around proton track 

fname = {'pTuple_Filename_1.phsp','pTuple_Filename_2.phsp','pTuple_Filename_3.phsp'} ;
totalfiles = max(size(fname));
%histogram variables
maxradius = 40;
binsize = 0.001;
bins = maxradius/binsize;
%create empty arrays 
recordLET = zeros(1,totalfiles);
recordenergy_dep_rate = zeros(bins,totalfiles);
recordcumulative_energy = zeros(bins,totalfiles);

%read in position and interaction information from each simulation file
for filenumber = 1:totalfiles
    pTuple = importdata(fname{filenumber});
    totalinteractions = size(pTuple(:,1),1);
    molecule = pTuple(:,1);
    radialposition = zeros(totalinteractions,1);
    xdata = pTuple(:,2);
    ydata = pTuple(:,3);
    energydata = pTuple(:,6);
    histories = max(pTuple(:,5))+1;
    clearvars pTuple

    %empty histogram array
    radial_hist = zeros(bins,1);
    %calculate radial position of interactions
    radialdata = sqrt(xdata.^2 + ydata.^2 );
    
    %energy weighted histogram of the radial position of interactions  
    for i = 1:totalinteractions
        energydeposited = energydata(i,:); 
        radialposition = radialdata(i,:);
            
        if radialposition < maxradius
            radial_hist(floor(radialposition/binsize)+1) = radial_hist(floor(radialposition/binsize)+1) + energydeposited; 
        end
    end
    
    %radial and cumulative distributions of energy deposition
    energy_dep_rate = radial_hist/(histories*binsize);
    running_sum = cumsum(energy_dep_rate);
    totalenergydeposited = sum(energy_dep_rate);
    cumulative_energy = (running_sum/totalenergydeposited)*100;
    
    %record for each data file    
    recordenergy_dep_rate(:,filenumber) = energy_dep_rate;
    recordcumulative_energy(:,filenumber) = cumulative_energy;
   
    %LET calculation
    cylinderwidth = 4;
    LET = sum(energydata)/(cylinderwidth*histories);
    recordLET(1,filenumber) = LET;
    clearvars -except fname filenumber totalfiles recordLET bins binsize maxradius recordenergy_dep_rate recordcumulative_energy
end

%exporting radial energy deposition data and LET to text file
outputenergy_dep_rate = table(recordenergy_dep_rate);
filename = sprintf('%s','Physics_Rate_of_Energy_Deposition');
writetable(outputenergy_dep_rate, filename, 'WriteVariableNames',0,'Delimiter',' ');

outputcumulative_energy = table(recordcumulative_energy);
filename = sprintf('%s','Physics_Cumulative_Energy_Distribution');
writetable(outputcumulative_energy, filename,'WriteVariableNames',0,'Delimiter',' ');

outputLET = table(recordLET);
filename = sprintf('%s','Physics_LET');
writetable(outputLET, filename,'WriteVariableNames',0,'Delimiter',' ');

%exit