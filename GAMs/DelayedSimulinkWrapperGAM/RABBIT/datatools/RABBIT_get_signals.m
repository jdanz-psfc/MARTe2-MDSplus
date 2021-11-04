function [sigs,sigs_params] = RABBIT_get_signals(machine,shot,t_win,varargin)
% [sigs,sigs_params] = RABBIT_get_signals(machine,'test',[],nsource)
% [sigs,sigs_params] = RABBIT_get_signals(machine,shot,t_window,nsource)

  if numel(varargin)>=1
     nsource = varargin{1};
  else
      nsource = 1; %default
  end

if ischar(shot)
  assert(strcmpi(shot,'test'),'underfined string input')

  sigs = RABBIT_in_test_signals(machine,nsource);
  sigs_params = [];
elseif isnumeric(shot)
  if strcmpi(machine,'tcv')
    [sigs,sigs_params] = RABBIT_in_signals_TCV(shot,t_win,nsource);
  elseif strcmpi(machine,'jet')
    [sigs,sigs_params] = RABBIT_in_signals_JET(shot,t_win);
  end
end

end

function sigs = RABBIT_in_test_signals(machine,nsource)

switch (machine)
  case 'TCV'
    sigs = RABBIT_in_test_signals_TCV(nsource); 
  case 'JET'
    sigs = RABBIT_in_test_signals_JET(nsource); 
  otherwise
    error('invalid machine definition')
end
    
end

function sigs = RABBIT_in_test_signals_TCV(nsource)

dt = 0.01;
nt = 2;
time = [0:dt:dt*(nt-1)];
nt = numel(time);
% test signals for a TCV-like machine
np=11;

sigs.profiles.rhotor_kinprof = timeseries(ones(nt,1)*(linspace(0,1,np)),time);
sigs.profiles.ne = timeseries(ones(nt,1)*(5e19*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Te = timeseries(ones(nt,1)*(3e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Ti = timeseries(ones(nt,1)*(3e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Zeff = timeseries(ones(nt,1)*(3*ones(1,np)),time);
sigs.profiles.omega_tor = timeseries(ones(nt,1)*(0*ones(1,np)),time);

%% Dummy equilibrium
nm = 28; nn = 65;
R=linspace(0.5,1.3,nm)';
Z=linspace(-1.1,1.1,nn)';
R0 = 0.88; Z0=0; a0 = 0.21; k0=1.5; B0=1.5;
[RR,ZZ] = meshgrid(R,Z);

Psi_sep = -0.03;

Psi = ( 1/a0^2*((RR-R0).^2 + 1/k0.^2*(ZZ-Z0).^2) ) * Psi_sep ;

nl = 17;
iota = 1./(linspace(0,1,nl).^5*5+1);
sigs.equil.Psi = timeseries(repmat(Psi',1,1,numel(time)),time);
sigs.equil.psi1d_n = timeseries(ones(nt,1)*linspace(0,1,nl),time);
sigs.equil.Volume = timeseries(ones(nt,1)*linspace(0,1,nl).^2*pi*a0.^2*k0*2*pi*R0,time);
sigs.equil.Area   = timeseries(ones(nt,1)*linspace(0,1,nl).^2*pi*a0.^2*k0,time);
sigs.equil.rhotor1d = timeseries(ones(nt,1)*linspace(0,1,nl),time);
sigs.equil.iota = timeseries(ones(nt,1)*iota,time); %- for COCOS=5
sigs.equil.F = timeseries(ones(nt,1)*(R0*B0*ones(1,nl)),time);
sigs.equil.Psi_sep = timeseries(ones(nt,1)*Psi_sep,time);
sigs.equil.Psi_axis = timeseries(ones(nt,1)*0,time);
sigs.equil.Rmag = timeseries(ones(nt,1)*R0,time);
sigs.equil.Zmag = timeseries(ones(nt,1)*Z0,time);

sigs.beam.Pinj = timeseries(ones(nt,1)*1e6*ones(1,nsource),time);
sigs.beam.Einj = timeseries(ones(nt,1)*25e3*ones(1,nsource),time);
sigs.beam.specmix = timeseries(repmat([0.49;0.36;0.15],1,nsource,numel(time)),time);

%%

end


function sigs = RABBIT_in_test_signals_JET(nsource)

dt = 0.1;
nt = 2;
time = [0:dt:dt*(nt-1)];
nt = numel(time);
% test signals for a TCV-like machine
np=11;

sigs.profiles.rhotor_kinprof = timeseries(ones(nt,1)*(linspace(0,1,np)),time);
sigs.profiles.ne = timeseries(ones(nt,1)*(1e19*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Te = timeseries(ones(nt,1)*(5e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Ti = timeseries(ones(nt,1)*(5e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Zeff = timeseries(ones(nt,1)*(3*ones(1,np)),time);
sigs.profiles.omega_tor = timeseries(ones(nt,1)*(0*ones(1,np)),time);

%% Dummy equilibrium
R = linspace(1.5,4,26)';
Z = linspace(-2.5,2.5,51)';

R0 = 2.8; Z0=0; a0 = 1.0; k0=1.5; B0=2.5;
[RR,ZZ] = meshgrid(R,Z);

Psi_sep = -0.03;

Psi = ( 1/a0^2*((RR-R0).^2 + 1/k0.^2*(ZZ-Z0).^2) ) * Psi_sep ;

Pinj = 1e6*ones(1,nsource);
Einj = 25e3*ones(1,nsource);

nl = 17;
iota = 1./(linspace(0,1,nl).^5*5+1);

[sigs.equil,sigs.beam] = buildsigs(nsource,time,Psi,Psi_sep,a0,k0,R0,Z0,B0,iota,Pinj,Einj);

end


function [sigs,sigs_params] = RABBIT_in_signals_TCV(shot,t_win,nsource)

assert(numel(t_win)==1,'only done for one time slice')
if numel(t_win)==1
    time = t_win+[0,0.1];
end
nt = numel(time);

assert(~~exist('gdat','file'),'can''t load TCV data since gdat.m not found')
%% Equilibrium
ge = gdat(shot,'eqdsk','time',t_win);
% convert to COCOS=5 (RABBIT), from COCOS=17 (LIUQE)
eqdsk = eqdsk_cocos_transform(ge.eqdsk,[17,5]);

Psi = eqdsk.psi';
psi1d_n = eqdsk.psimesh';
rhotor1d = eqdsk.rhotor'./eqdsk.rhotor(end);
iota = -1./eqdsk.q';
F = eqdsk.F';
Psi_sep = eqdsk.psiedge;
Psi_axis = eqdsk.psiaxis;
Rmag = eqdsk.raxis;
Zmag = eqdsk.zaxis;

%%
mdsopen(shot);
voltdi=tdi('tcv_eq($1,$2)','vol','liuqe.m');
areatdi=tdi('tcv_eq($1,$2)','area','liuqe.m');
psi1dtdi=tdi('tcv_eq($1,$2)','psi_values','liuqe.m');

it = iround(voltdi.dim{end},t_win);

psi1d = psi1dtdi.data(:,it); psiN = (psi1d-psi1d(1))/(psi1d(end)-psi1d(1));
Volume = interp1(psiN,voltdi.data(:,it),psi1d_n,'linear');
Area = Volume./Volume(end).*areatdi.data(it);

%% heating
% powers = gdat(shot,'powers'); % temp
Einj = repmat(25e3,1,nsource);
Pinj = repmat(1e6,1,nsource);
%%

sigs.equil.Psi = timeseries(repmat(Psi',1,1,numel(time)),time);
sigs.equil.psi1d_n = timeseries(ones(nt,1)*psi1d_n,time);
sigs.equil.Volume = timeseries(ones(nt,1)*Volume,time);
sigs.equil.Area   = timeseries(ones(nt,1)*Area,time);
sigs.equil.rhotor1d = timeseries(ones(nt,1)*rhotor1d,time);
sigs.equil.iota = timeseries(ones(nt,1)*iota,time); %- for COCOS=5
sigs.equil.F = timeseries(ones(nt,1)*F,time);
sigs.equil.Psi_sep = timeseries(ones(nt,1)*Psi_sep,time);
sigs.equil.Psi_axis = timeseries(ones(nt,1)*Psi_axis,time);
sigs.equil.Rmag = timeseries(ones(nt,1)*Rmag,time);
sigs.equil.Zmag = timeseries(ones(nt,1)*Zmag,time);

sigs.beam.Pinj = timeseries(ones(nt,1)*Pinj,time);
sigs.beam.Einj = timeseries(ones(nt,1)*Einj,time);
sigs.beam.specmix = timeseries(repmat([0.73;0.22;0.05],1,nsource,numel(time)),time);

np=21;
sigs.profiles.rhotor_kinprof = timeseries(ones(nt,1)*(linspace(0,1,np)),time);
sigs.profiles.ne = timeseries(ones(nt,1)*(5e19*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Te = timeseries(ones(nt,1)*(3e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Ti = timeseries(ones(nt,1)*(3e3*(1-linspace(0,sqrt(0.9),np).^2)),time);
sigs.profiles.Zeff = timeseries(ones(nt,1)*(3*ones(1,np)),time);
sigs.profiles.omega_tor = timeseries(ones(nt,1)*(0*ones(1,np)),time);


%% auxiliary parameters
sigs_params.Rgrid = eqdsk.rmesh;
sigs_params.Zgrid = eqdsk.zmesh;
sigs_params.l = numel(psi1d_n);
sigs_params.p = np;


end

function [equil,beam] = buildsigs(nsource,time,Psi,Psi_sep,a0,k0,R0,Z0,B0,iota,Pinj,Einj)
nl = numel(iota);
nt = numel(time);

equil.Psi = timeseries(repmat(Psi',1,1,numel(time)),time);
equil.psi1d_n = timeseries(ones(nt,1)*linspace(0,1,nl),time);
equil.Volume = timeseries(ones(nt,1)*linspace(0,1,nl).^2*pi*a0.^2*k0*2*pi*R0,time);
equil.Area   = timeseries(ones(nt,1)*linspace(0,1,nl).^2*pi*a0.^2*k0,time);
equil.rhotor1d = timeseries(ones(nt,1)*linspace(0,1,nl),time);
equil.iota = timeseries(ones(nt,1)*iota,time); %- for COCOS=5
equil.F = timeseries(ones(nt,1)*(R0*B0*ones(1,nl)),time);
equil.Psi_sep = timeseries(ones(nt,1)*Psi_sep,time);
equil.Psi_axis = timeseries(ones(nt,1)*0,time);
equil.Rmag = timeseries(ones(nt,1)*R0,time);
equil.Zmag = timeseries(ones(nt,1)*Z0,time);

beam.Pinj = timeseries(ones(nt,1)*Pinj,time);
beam.Einj = timeseries(ones(nt,1)*Einj,time);
beam.specmix = timeseries(repmat([0.49;0.36;0.15],1,nsource,numel(time)),time);
end

function sigs = RABBIT_in_signals_JET(shot)
error('not done yet')
end

