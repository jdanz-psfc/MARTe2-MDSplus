classdef RABBIT_tests < matlab.unittest.TestCase
  % class for various tests for RABBIT-Simulink
  properties
    defaultMachine = 'TCV';
  end
  
  methods(TestClassSetup)
    function setupEnvironment(testCase)
      %comment
      oldPath = path;
      testCase.addTeardown(@path,oldPath);
      testCase.addTeardown(@cd,pwd);

      run('../set_RABBIT_env');
      cd(fileparts(mfilename('fullpath'))); % cd to this directory
    end
  end
  
  methods (Test)
    % full tests for errors
    
    function testMex(testCase)
      testCase.verifyWarningFree(@mexrabbit,'mock');
    end
    
    function testSimulinkMock(testCase)
      testCase.assertTrue(RABBIT_test_run_simulink('mock',testCase.defaultMachine));
    end
    
    function testSimulinkCompileOnly(testCase)
      compileonly = true;
      nsourcelist = 1;
      testCase.assertTrue(RABBIT_test_run_simulink('debug',testCase.defaultMachine,nsourcelist,compileonly));
    end
    
    function testSimulinkCompilationModeDebug(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_test_run_simulink('debug',testCase.defaultMachine));
    end
    
    function testSimulinkCompilationModeTiming(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_test_run_simulink('timing',testCase.defaultMachine));
    end
    
    function testSimulinkCompilationModeProduction(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_test_run_simulink('production',testCase.defaultMachine));
    end

    
    function testSimulinkMachines(testCase)
      assume_runLibTests(testCase);
      nsource = 1;
      testCase.verifyTrue(RABBIT_test_run_simulink('debug','JET',nsource));
    end
    
    function testCodeGen(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_codegen_tests);
    end
    
    function testDataRW(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_datarw_test);
    end
    
    function testVsFORTRAN(testCase)
      assume_runLibTests(testCase);
      testCase.verifyTrue(RABBIT_test_vs_FORTRAN);
    end
  end
end


function assume_runLibTests(testCase)
envVarDefined = ~isempty(getenv('RABBIT_PATH'));
testCase.assumeTrue(envVarDefined,'Environment variable $RABBIT_path not defined, skipping test')
end
