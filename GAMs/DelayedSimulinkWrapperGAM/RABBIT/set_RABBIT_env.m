function set_RABBIT_env
thispath=fileparts(mfilename('fullpath'));

addpath(fullfile(thispath));
addpath(fullfile(thispath,'datatools'));
addpath(fullfile(thispath,'tests'));

%% set paths for Simulink cache
myFolder = fullfile(thispath,'codegen','CacheFolder');
if ~exist(myFolder,'dir')
  mkdir(myFolder);
end
set_param(0,'CacheFolder',myFolder)
fprintf('Set Simulink CacheFolder to %s\n',myFolder);

%% set paths for code generation
myFolder = fullfile(thispath,'codegen','CodeGenFolder');
if ~exist(myFolder,'dir')
  mkdir(myFolder);
end
set_param(0,'CodeGenFolder',myFolder)
fprintf('Set Simulink CodeGenFolder to %s\n',myFolder);

end