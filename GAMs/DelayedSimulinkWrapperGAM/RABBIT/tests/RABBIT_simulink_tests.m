function RABBIT_simulink_tests(nsource,machine,compileonly)
if nargin<2
  machine = 'TCV';
end
if nargin<3
  compileonly = false;
end

fprintf('\n\n==== Testing wrapper in Simulink with %d source for %s ====\n',nsource,machine)

RABBITparams = RABBIT_loadparams(machine,'test',true,'nsource',nsource);
sigs = RABBIT_get_signals(machine,'test',[],nsource);
time = sigs.profiles.ne.Time;

assignin('base','RABBITparams',RABBITparams);
assignin('base','RABBITsignals',sigs);
assignin('base','t_start',time(1));
assignin('base','t_end',time(end));
assignin('base','dt',median(diff(time)));
%%
if compileonly
  fprintf('Only compiling Simulink model\n')
  RABBIT_sfunction_test([],[],[],'compile');
  RABBIT_sfunction_test([],[],[],'term');
else
  fprintf('Compiling and running Simulink model\n');
  sim('RABBIT_sfunction_test');
end

fprintf('\n=== Done testing wrapper in Simulink with %d source(s) ===\n\n',nsource);
end


