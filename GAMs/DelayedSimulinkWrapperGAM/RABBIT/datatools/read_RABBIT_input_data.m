function [RABBITparams,RABBITsignals,namelist] = read_RABBIT_input_data(datdir)
% [RABBITparams,RABBITsignals,namelist] = read_RABBIT_input_data(datdir)
% read RABBIT input data from .dat files in datdir

assert(~~exist(datdir),'%s does not exist',datdir);

[eqdata,beams,tvdata] = read_data_files(datdir);

[nmlparams,namelist] = read_namelistinfo(datdir);

[RABBITparams,RABBITsignals] = sort_data(eqdata,beams,tvdata,nmlparams,datdir);


%plot_equilibrium(eqparams,eqdata,1);

end

function [eqdata,beams,tvdata,nmlparams] = read_data_files(datdir)
eqdata = read_eq_data_file(datdir);
beams = read_beam_data_file(datdir);
tvdata = read_timevarying_data_file(datdir);
end

function [nmlparams,namelistfile] = read_namelistinfo(datadir)
dd = dir(fullfile(datadir,'*.nml'));
assert(numel(dd)==1,'multiple .nml files found in dir %s',datadir);
namelistfile = fullfile(datadir,dd.name);
nmlparams = read_namelists(namelistfile);

end

function nmlparams = read_namelists(namelistfile)
    
species = read_namelist(namelistfile,'species');
output_settings = read_namelist(namelistfile,'output_settings');

nmlparams = join_struct(species,output_settings);

end

function jstruct = join_struct(varargin)
   jstruct = varargin{1};
   for ii=2:nargin
       mystruct = varargin{ii};
       myfieldnames = fieldnames(mystruct);
       for jj=1:numel(myfieldnames)
           jstruct.(myfieldnames{jj}) = mystruct.(myfieldnames{jj});
       end
    end
end

function [eqdata] = read_eq_data_file(datdir)

% search for files
eqdir = fullfile(datdir,'equ');
assert(~~exist(eqdir,'dir'),'%s folder not found',eqdir)

%% build list of eq file names
dircontents = dir(eqdir);
eqfnames = {}; eqfmax = 0;
for ii=1:numel(dircontents);
    fname = dircontents(ii).name;
    if strncmp(fname,'equ',3)
        eqfnames = [eqfnames,{fname}];
        eqnr = sscanf(fname,'equ_%d');
        assert(~isempty(eqnr),'invalid format %s',fname)
        eqfmax = max(eqfmax,eqnr);
    end
end
assert(numel(eqfnames)==eqfmax,'found %d equ_ files but maximum index is %d',numel(eqfnames),eqfmax)

nt = eqfmax; % number of times for eq files
%%

for iit=1:nt
    fname = fullfile(eqdir,sprintf('equ_%d.dat',iit));
    fprintf('   reading %s\n',fname)
    %%
    fid = fopen(fname,'r'); assert(fid~=-1,'error opening file %s',fname)
    
    % 2D
    nR = fscanf(fid,'%d',1);  eqdata.nR = nR;
    nZ = fscanf(fid,'%d',1);  eqdata.nZ = nZ;
    
    eqdata.Rgrid = fscanf(fid,'%f ',nR);
    eqdata.Zgrid = fscanf(fid,'%f ',nZ);
    PsiRZ = fscanf(fid,'%f ',nR*nZ);
    rhotorN2D = fscanf(fid,'%f ',nR*nZ);
    
    % profiles
    nr = fscanf(fid,'%d',1); eqdata.nr = nr;
    psi1d = fscanf(fid,'%f ',nr);
    eqdata.Volume(iit,:) = fscanf(fid,'%f ',nr);
    eqdata.Area(iit,:) = fscanf(fid,'%f ',nr);
    eqdata.rhotorN(iit,:) = fscanf(fid,'%f ',nr);
    q = fscanf(fid,'%f ',nr);
    eqdata.F(iit,:) = fscanf(fid,'%f',nr);
    
    % scalars
    eqdata.psib(iit,:) = fscanf(fid,'%f ',1);
    eqdata.psia(iit,:) = fscanf(fid,'%f ',1);
    a = fscanf(fid,'%f',1);
    eqdata.Rax(iit,:) = fscanf(fid,'%f ',1);
    eqdata.Zax(iit,:) = fscanf(fid,'%f ',1);
    
    fclose(fid);
    
    
    eqdata.PsiRZ(:,:,iit) = reshape(PsiRZ,nR,nZ);
    eqdata.rhotorN2(:,:,iit) = reshape(rhotorN2D,nR,nZ);
    % some specific changes for ASCII file input definitions which are
    % different than RT interface (sorry)
    eqdata.iota(iit,:) = 1./q;
    eqdata.psiN1d(iit,:) = (psi1d-eqdata.psia(iit))./(eqdata.psib(iit)-eqdata.psia(iit));
    
end

end

function plot_equilibrium(eqdata,iit)
[RR,ZZ] = meshgrid(eqdata.Rgrid,eqdata.Zgrid);
contourf(RR,ZZ,reshape(eqdata.PsiRZ(iit,:,:),eqdata.nR,eqdata.nZ)'); axis equal; colorbar
hold on; plot(eqdata.Rax(iit,:),eqdata.Zax(iit,:),'+w');
drawnow
end

function beams = read_beam_data_file(datdir)

fname = fullfile(datdir,'beams.dat');
fprintf('   reading %s\n',fname)
fid = fopen(fname,'r'); assert(fid~=-1,'error opening file %s',fname)

fgetl(fid);
nsource = fscanf(fid, '%10d\n',1);
fgetl(fid);
nv = fscanf(fid, '%10d\n',1);
fgetl(fid);
xstart = fscanf(fid,'%f\n',3*nsource);
fgetl(fid);
xvec = fscanf(fid,'%f\n',3*nsource);
fgetl(fid);
beamwidthpoly = fscanf(fid,'%f\n',3*nsource);
fgetl(fid);
Einj = fscanf(fid,'%f\n',nsource);
fgetl(fid);
specmix = fscanf(fid,'%f\n',3*nsource);
fgetl(fid);
abeam = fscanf(fid,'%f\n',nsource);

fclose(fid);

%% order data
beams.nsource = nsource;
beams.nv = nv;
beams.xstart = reshape(xstart,3,nsource);
beams.xvec = reshape(xvec,3,nsource);
beams.beamwidthpoly = reshape(beamwidthpoly,3,nsource);
beams.specmix = reshape(specmix,nv,nsource);
beams.Einj = Einj;
beams.abeam = abeam;
end

function tvdata = read_timevarying_data_file(datdir)

%%
fname = fullfile(datdir,'timetraces.dat');

fprintf('   reading %s\n',fname)
fid = fopen(fname,'r'); assert(fid~=-1,'error opening file %s',fname)

nt = fscanf(fid,'%d\n',1);
np = fscanf(fid,'%d\n',1);

fscanf(fid,'%s\n',1);
time = fscanf(fid,'%f\n',nt);
rhotor_kinprof = fscanf(fid,'%f ',np);
Te = fscanf(fid,'%f\n',np*nt);
Ti = fscanf(fid,'%f\n',np*nt);
ne = fscanf(fid,'%f\n',np*nt);
omega_tor = fscanf(fid,'%f\n',np*nt);
Zeff = fscanf(fid,'%f ',np*nt);

% two versions of this file exist: with or without time-varyin Einj etc
% data. Distinguish between them depending on string #no. of sources.

loc1 = ftell(fid);
while ~feof(fid)
    str = fgetl(fid);
    if ~isempty(strfind(str,'#no. of sources:'))
        extendedfile = true; 
        break
    else
        extendedfile = false;
    end
end

if ~extendedfile
    fseek(fid,loc1,'bof'); % back to where we were
    Pinj = fscanf(fid,'%f '); % until end of file
    nsource = floor(numel(Pinj)/nt);
    assert(nsource*nt == numel(Pinj),'invalid number of Pinj points')
else
    nsource = fscanf(fid,'%d',1);
    loc2 = ftell(fid);
    fseek(fid,loc1,'bof');
    Pinj = fscanf(fid,'%f\n',nsource*nt);
    fseek(fid,loc2,'bof');
    fgetl(fid); fgetl(fid);
    nv = fscanf(fid,'%d\n',1);
    fgetl(fid);
    nvstored = fscanf(fid,'%d\n',1);
    fgetl(fid);
    Einj = fscanf(fid,'%f\n',nsource*nt);
    for ispec = 1:nvstored
    specmix(ispec,:) = fscanf(fid,'%f\n',nt);
    end
    specmix(ispec+1,:) = 1-sum(specmix(1:nvstored,:)); % last one
end
fclose(fid);

%%

tvdata.nt = nt;
tvdata.np = np;
tvdata.nsource = nsource;

tvdata.rhotor_kinprof = rhotor_kinprof;
tvdata.time = time;

tvdata.Te = 1e3*reshape(Te,nt,np);
tvdata.Ti = 1e3*reshape(Ti,nt,np);
tvdata.ne = 1e6*reshape(ne,nt,np);
tvdata.omega_tor = reshape(omega_tor,nt,np);
tvdata.Zeff = reshape(Zeff,nt,np);

tvdata.Pinj = reshape(Pinj,nt,nsource);

if extendedfile
    tvdata.Einj = reshape(Einj,nt,nsource);
    tvdata.specmix = reshape(specmix,nv,nsource,nt);
end

% openw, 44, dir+'/timetraces.dat'
% printf, 44, n_elements(time)
% printf, 44, n_elements(rho)
% printf, 44, 'rho_tor'
% printf, 44, time  ;[s]
% printf, 44, rho   ;rho_tor
% printf, 44, transpose(profarr.te) * 1d-3  ;[keV], shape: [time, rho]
% printf, 44, transpose(profarr.ti) * 1d-3  ;[keV]
% printf, 44, transpose(profarr.dene) * 1d-6  ;[1/cm^3]
% printf, 44, transpose(profarr.vtor)*signIp  ;[rad/s, sign wrt. tor. angle phi]
% printf, 44, transpose(profarr.zeff)
% if keyword_set(addpnbi) then printf, 44, pnbi ;[W], shape: [time, beam]
% if keyword_set(Einj_t) then printf, 44, Einj_t ;[W], shape: [time, beam]
% if keyword_set(specmix1_t) then printf, 44, specmix1_t  ;specMix of full energy component ;[-], shape [time, beam]
% if keyword_set(specmix2_t) then printf, 44, specmix2_t  ;specMix of half energy component ;[-], shape [time, beam]
% ; leave out specMix of 1/third energy component (not necessary since they add up to one)
% close, 44

end

function [params,sigs] = sort_data(eqdata,beams,tvdata,nmlparams,datpath)

%%
pp.aplasma = nmlparams.aplasma;
pp.zplasma = nmlparams.zplasma;
pp.aimp = nmlparams.aimp;
pp.zimp = nmlparams.zimp;
pp.abeam = beams.abeam;
pp.zbeam = ones(beams.nsource,1);
pp.xstart = beams.xstart;
pp.xvec = beams.xvec;
pp.beamwidthpoly = beams.beamwidthpoly;
pp.nv = beams.nv;
pp.nsource = beams.nsource;
pp.nrhotor_out = nmlparams.nrhoout;
pp.R = eqdata.Rgrid;
pp.Z = eqdata.Zgrid;
pp.l = eqdata.nr;
pp.p = tvdata.np;

params = Simulink.Parameter;
params.Value = pp;

%%
time = tvdata.time;
dt = diff(time); assert(all(abs(diff(dt))<1e-6),'non-uniform time not supported')

sigs.profiles.rhotor_kinprof = timeseries([1;1]*tvdata.rhotor_kinprof',[time(1),time(end)]);
sigs.profiles.ne = timeseries(tvdata.ne,time);
sigs.profiles.Te = timeseries(tvdata.Te,time);
sigs.profiles.Ti = timeseries(tvdata.Ti,time);
sigs.profiles.Zeff = timeseries(tvdata.Zeff,time);
sigs.profiles.omega_tor = timeseries(tvdata.omega_tor,time);

sigs.equil.Psi = timeseries(eqdata.PsiRZ,time);
sigs.equil.psi1d_n = timeseries(eqdata.psiN1d,time);
sigs.equil.Volume = timeseries(eqdata.Volume,time);
sigs.equil.Area   = timeseries(eqdata.Area,time);
sigs.equil.rhotor1d = timeseries(eqdata.rhotorN,time);
sigs.equil.iota = timeseries(eqdata.iota,time);
sigs.equil.F = timeseries(eqdata.F,time);
sigs.equil.Psi_sep = timeseries(eqdata.psib,time);
sigs.equil.Psi_axis = timeseries(eqdata.psia,time);
sigs.equil.Rmag = timeseries(eqdata.Rax,time);
sigs.equil.Zmag = timeseries(eqdata.Zax,time);

if isfield(tvdata,'Einj');
    Einj = tvdata.Einj;
else
    Einj = repmat(beams.Einj',numel(time),1);
end

if isfield(tvdata,'specmix');
    specmix = tvdata.specmix;
else
    specmix = repmat(beams.specmix,1,1,numel(time));
end

sigs.beam.Pinj = timeseries(tvdata.Pinj,time);
sigs.beam.Einj = timeseries(Einj,time);
sigs.beam.specmix = timeseries(specmix,time);

%%

end
