function passed = RABBIT_test_run_simulink(compilationMode,machine,nsourcelist,compileonly)
if nargin<3
  nsourcelist = [1 2]; % number of sources to check for
end
if nargin<4
  compileonly=false;
end

fprintf('\n RABBIT Simulink run test: START \n');

mexrabbit(compilationMode);

for nsource = nsourcelist
  RABBIT_simulink_tests(nsource,machine,compileonly);
end

if compileonly
  compstr = '(Compilation only)';
else
  compstr = '';
end

fprintf('\n RABBIT %s MODE TEST %s for %s: DONE \n\n',upper(compilationMode),compstr,machine);

passed=true;
end
