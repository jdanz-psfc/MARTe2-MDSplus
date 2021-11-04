clear; 

nsource = 2;

RABBITparams = RABBIT_loadparams('TCV','nsource',nsource);
plot_beam_geom(RABBITparams);

shot = 59270;
twin = [1];
[RABBITsignals,sigparams] = RABBIT_get_signals('TCV',shot,twin,nsource);
RABBITparams.Value.R = sigparams.Rgrid;
RABBITparams.Value.Z = sigparams.Zgrid;
RABBITparams.Value.p = sigparams.p;
RABBITparams.Value.l = sigparams.l;

%% set kinetic profiles manually
rho = RABBITsignals.profiles.rhotor_kinprof.Data(1,:);
teprof = 2e3*(1-rho.^2);
tiprof = teprof;
neprof = 5e19*(1-rho.^3);
zeprof = 3*ones(size(rho));
RABBITsignals.profiles.Te.Data = repmat(teprof,2,1);
RABBITsignals.profiles.Ti.Data = repmat(tiprof,2,1);
RABBITsignals.profiles.ne.Data = repmat(neprof,2,1);
RABBITsignals.profiles.Zeff.Data = repmat(zeprof,2,1);

%%
time = RABBITsignals.profiles.ne.Time;

t_start = time(1);
t_end = time(1)+0.1;
dt = 0.001;
%%
plot_timeseries_struct(figure(1),RABBITsignals.beam);
%plot_timeseries_struct(figure(2),RABBITsignals.equil);
plot_timeseries_struct(figure(3),RABBITsignals.profiles);

%%
plottime = 1;
plot_RABBIT_inputs(RABBITparams,RABBITsignals,plottime);

%%
mexrabbit('production');
sim('RABBIT_sfunction_test');

%%
plot_RABBIT_output(logsout);
%%
dirname = 'TCV_test_case';
write_RABBIT_input_data(logsout,RABBITparams.Value,time,dirname)