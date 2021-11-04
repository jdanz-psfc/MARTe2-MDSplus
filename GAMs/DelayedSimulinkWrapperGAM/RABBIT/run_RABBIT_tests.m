function passed = run_RABBIT_tests(mytest)
if nargin==0
  mytest = 'all';
end

import matlab.unittest.TestSuite;

basepath = fileparts(mfilename('fullpath'));
testfilename = fullfile(basepath,'tests','RABBIT_tests.m');

switch mytest
  case 'all';
    % all
    suite = TestSuite.fromFile(testfilename);
  otherwise
    import matlab.unittest.selectors.HasName;
    import matlab.unittest.constraints.ContainsSubstring;

    selector = HasName(ContainsSubstring(mytest));
    suite = TestSuite.fromFile(testfilename,selector);
    if isempty(suite)
      fullsuite = TestSuite.fromFile(testfilename);
      fprintf('No valid test selected.\n Valid tests in file %s: \n\n',testfilename);
      fprintf('  %s\n',fullsuite.Name)
      passed = false;
      return
    end
end
results = run(suite);
%%
disp(table(results));
tres = table(results);
fprintf('\nTotal test duration: %5.2fs\n',sum(tres.Duration))

if all([results.Passed])
  fprintf('\nPassed all tests\n')
  passed = true;
else
  fprintf('\nSome tests Failed or Incomplete\n')
  if any([results.Incomplete])
    fprintf('\nIncomplete:\n')
    disp(table(results([results.Incomplete])))
  end
  if any([results.Failed])
    fprintf('\nFailed:\n')
    disp(table(results([results.Failed])));
  end
  passed = false;
end

end
