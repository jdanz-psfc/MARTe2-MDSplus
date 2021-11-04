function passed = RABBIT_codegen_tests

%%
setup_test_workspace;

%% test in mock mode
fprintf('\n RABBIT CODEGEN TEST: START \n\n');

%%
modelname = 'RABBIT_sfunction_test';
rtwbuild(modelname,'ForceTopModelBuild',true);
fprintf('\n RABBIT CODEGEN: DONE \n\n');

passed=true;
end


function setup_test_workspace
machine = 'TCV';
nsource = 1;
params = RABBIT_loadparams(machine,'test',true,'nsource',nsource);
sigs = RABBIT_get_signals(machine,'test',[],nsource);
time = sigs.profiles.ne.Time;
mexrabbit('production'); % make mex to pass model compilation
set_RABBIT_namelist('namelist_tcv.nml');
assignin('base','RABBITparams',params);
assignin('base','RABBITsignals',sigs);
assignin('base','t_start',time(1));
assignin('base','t_end',time(end));
assignin('base','dt',median(diff(time)));
end
