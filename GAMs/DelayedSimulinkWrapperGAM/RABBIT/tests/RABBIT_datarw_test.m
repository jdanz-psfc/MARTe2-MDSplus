function passed = RABBIT_datarw_test
% test data write and data read functions

%% setup
fprintf('\n\n==== RABBIT READWRITE FUNCTION TESTS ====\n')
machine = 'TCV';
nsource = 1;
sigs = RABBIT_get_signals(machine,'test',[],nsource);
time = sigs.profiles.ne.Time;
params = RABBIT_loadparams('TCV','test',true,'nsource',nsource);
assignin('base','RABBITparams',params);
assignin('base','RABBITsignals',sigs);
assignin('base','t_start',time(1));
assignin('base','t_end',time(end));
assignin('base','dt',median(diff(time)));

%% run
mexrabbit('mock'); % only mock needed here
sim('RABBIT_sfunction_test');

%% write 
datdir = 'test_tmp';
dirpath = write_RABBIT_input_data(logsout,params.Value,time,datdir);

%% read
[paramsread,sigsread,namelist] = read_RABBIT_input_data(dirpath);

%% compare
params_match = struct_compare(paramsread.Value,params.Value);
sigs_match = struct_compare(sigsread,sigs);

passed = params_match & sigs_match;

%% cheer/boo
if passed
    str = 'PASSED';
else
    str = 'FAILED';
end
fprintf('\n\n==== RABBIT READWRITE FUNCTION TESTS %s ====\n',str)

    
end

function passed = struct_compare(S1,S2)
fieldn = fieldnames(S1);
nfields = numel(fieldn);

passed = true; % init
for ifield = 1:nfields
    myfield = fieldn{ifield};
    assert(isfield(S2,myfield),'%s is not a field of Structure 2')
    
    F1 = S1.(myfield);
    F2 = S2.(myfield);
    if isstruct(F1)
        normok = struct_compare(F1,F2); % recursive call
    else
        if isnumeric(F1)
            err = norm(F1 - F2)/(norm(F1)+eps);
        elseif isa(F1,'timeseries')
            if ismatrix(F1.Data)
            err = norm(F1.Data-F2.Data)./(norm(F1.data)+eps);
            elseif ndims(F1.Data)==3 % 3D special case
                err = norm(sum(F1.Data-F2.Data,3))./(norm(sum(F1.data,3))+eps);
            else
                error('can''t handle more than 3D')
            end
        else
            error('don''t know how to compare this type')
        end
        normok = (err < 1e-2);
        if ~normok, 
            warning('field %s value does not match within tolerance',myfield); 
        end
    end
    passed = passed & normok;
end
end
