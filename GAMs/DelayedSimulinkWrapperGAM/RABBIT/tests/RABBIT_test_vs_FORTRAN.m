function passed = RABBIT_test_vs_FORTRAN(varargin)
% passed = RABBIT_test_vs_FORTRAN
% passed = RABBIT_test_vs_FORTRAN(true); % optional plots

if nargin>0
  doplot = varargin{1};
else
  doplot = false;
end

passed = test_vs_FORTRAN(doplot,'production');

end

function passed = test_vs_FORTRAN(doplot,mexmode)

%%
fprintf('\n\n==== Testing vs FORTRAN basic case ====\n')
mexrabbit(mexmode);
datdir = fullfile(fileparts(mfilename('fullpath')),'..','data','testcase');
[params,signals,namelist] = read_RABBIT_input_data(datdir);
set_RABBIT_namelist(namelist);
time = signals.profiles.ne.Time; dt=median(diff(time));
assignin('base','RABBITparams',params);
assignin('base','RABBITsignals',signals);
assignin('base','t_start',time(1));
assignin('base','t_end',time(end));
assignin('base','dt',dt);

%%
fprintf('Running Simulink RABBIT...')
sim('RABBIT_sfunction_test');

%% Compare to saved data
fname = fullfile(datdir,'rtfi_result_oav.sav');
idlsav = restore_idl(fname,'lowercase');

signallist = {'powe_total','powi_total','Pshine','Prot','Porbitloss','Pcxloss'};
data = get_Dataset_data(logsout,signallist,[]);
errornorm = compareSimulinkIDL(data,idlsav,doplot);

fprintf('Error norm: %3.3e',errornorm);
fprintf('\n\n==== Done testing vs FORTRAN basic case ====\n')

passed = (errornorm < 1e-2);
if ~passed
  fprintf('ERRORNORM w.r.t. FORTRAN RESULTS EXCEEDS TOLERANCE\n')
end
end
