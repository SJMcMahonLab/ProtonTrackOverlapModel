%Outputs multiple beam sources for the instantaneous delivery method 
%Read into topas file to generate multiple histories in a single event

Number_of_sources = 45;

for source = 1:Number_of_sources
    fprintf('%s%d%s\n', 's:So/MySource', source, '/Type = "Beam"');
    fprintf('%s%d%s\n', 's:So/MySource', source, '/BeamParticle = "proton"');
    fprintf('%s%d%s\n', 's:So/MySource', source, '/Component = "BeamPosition"');
    fprintf('%s%d%s\n', 'u:So/MySource', source, '/BeamEnergySpread = 0.0');
    fprintf('%s%d%s\n', 's:So/MySource', source, '/BeamPositionCutoffShape = "Rectangle"');
    fprintf('%s%d%s\n', 's:So/MySource', source, '/BeamPositionDistribution = "Flat"');
    fprintf('%s%d%s\n', 's:So/MySource', source, '/BeamAngularDistribution = "None"');
    fprintf('%s%d%s\n', 'd:So/MySource', source, '/BeamPositionCutoffX = 1.0 um');
    fprintf('%s%d%s\n', 'd:So/MySource', source, '/BeamPositionCutoffY = 1.0 um');
    fprintf('%s%d%s\n', 'd:So/MySource', source, '/BeamEnergy = 100.0 MeV');
    fprintf('%s%d%s\n\n', 'i:So/MySource', source, '/NumberOfHistoriesInRun = 1');
end
