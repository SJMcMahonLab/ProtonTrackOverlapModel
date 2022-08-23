clc;
close all;
%Reads in pTuple data files from topas simulation
%Outputs histogram of specified chemical species with increasing
%radial distance around proton track 

fname = {'pTuple_Filename_1.phsp','pTuple_Filename_1.phsp','pTuple_Filename_1.phsp'};
totalfiles = max(size(fname));
%create empty arrays
recordspecies_per_micron = zeros(40000,totalfiles);
recordcumulative_species = zeros(40000,totalfiles);

%read in position and species information from each simulation file
for filenumber = 1:totalfiles 
    pTuple = importdata(fname{filenumber});    
    numberofspecies = size(pTuple(:,1),1);
    molecule = pTuple(:,1);
    radialposition = zeros(numberofspecies,1);
    xdata = pTuple(:,2);
    ydata = pTuple(:,3);
    histories = max(pTuple(:,5))+1;
    clearvars pTuple

    %calculate radial position for specific chemical species
    for i = 1:numberofspecies
        x = xdata(i, :);
        y = ydata(i, :);
        
        %select molecule ID/IDs, calculate radial position
        if  molecule(i,1) < 9
            radialposition(i,:) = sqrt(x.^2 + y.^2 );
        end      
    end

    %histogram of the radial position of species
    radialdata = nonzeros(radialposition);
    radial_hist = transpose(histcounts(radialdata, 'Normalization','pdf','BinEdges',(0:0.001:40)));
    
    %radial and cumulative distributions of chemical species
    species_per_micron = radial_hist/(histories*0.001);
    running_sum = cumsum(species_per_micron);
    totalspecies = sum(species_per_micron);
    cumulative_species = (running_sum/totalspecies)*100;
    
    %record for each data file
    recordspecies_per_micron(:,filenumber) = species_per_micron;
    recordcumulative_species(:,filenumber) = cumulative_species;
    
    clearvars -except fname filenumber totalfiles recordspecies_per_micron recordcumulative_species
end

%exporting radial and cumulative distributions of species to text file
outputspecies_per_micron = table(recordspecies_per_micron);
filename = sprintf('%s','Chemistry_Radial_Distribution');
writetable(outputspecies_per_micron, filename, 'WriteVariableNames',0,'Delimiter',' ');

outputcumulative_species = table(recordcumulative_species);
filename = sprintf('%s','Chemistry_Cumulative_Distribution');
writetable(outputcumulative_species, filename,'WriteVariableNames',0,'Delimiter',' ');
    
%exit;
