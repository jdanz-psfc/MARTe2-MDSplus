function datadir_path = write_RABBIT_input_data(logsout,RABBIT_params,time,dirname)
% write RABBIT input data from Simulink simulation log
assert(~isa(RABBIT_params,'Simulink.Params'),'Must pass parameter structure directly');

%% get data
signal_list = {'rhotor_kinprof','Te','Ti','ne','Zeff','omega_tor',...
    'Psi','psi1d_n','Volume','Area',...
    'rhotor1d','iota','F',...
    'Psi_sep','Psi_axis','Rmag','Zmag',...
    'Pinj','Einj','specmix'};

data = get_Dataset_data(logsout,signal_list,time,'RABBIT');

%% set up write dir
datadir_path = setup_dir(dirname);

%% write files
write_eq_data_file(data,RABBIT_params,datadir_path);

write_beam_data_file(RABBIT_params,datadir_path)

write_timevarying_data_file(data,RABBIT_params,datadir_path)

write_curent_namelist(datadir_path);

fprintf('RABBIT input files saved to %s\n',datadir_path);
end

function datadir_path = setup_dir(dirname)

if exist(dirname,'dir');
    datadir_path = dirname; % exists locally
else
    % define in default location w.r.t. present file
    datadir_path = fullfile(fileparts(mfilename('fullpath')),'..','data',dirname);
end

% create dir if it does not exist
if ~exist(datadir_path,'dir')
  % at least lower one should exist
  parentdir = fileparts(datadir_path);
  if strcmp(parentdir,datadir_path)
    parentdir = fileparts(datadir_path(1:end-1)); % remove trailing slash to get parent dir
  end
  if ~exist(parentdir,'dir')
    error('neither directory %s nor its parent directory do not exist',parentdir);
  else
    mkdir(datadir_path);
  end
end
end

function write_eq_data_file(data,params,path)

nt = numel(data.time);

mkdir(fullfile(path,'equ'));

for iit=1:nt
  fname = fullfile(path,'equ',sprintf('equ_%d.dat',iit));
  fprintf('   writing %s\n',fname)
  fid = fopen(fname,'w+'); assert(fid~=-1,'error opening file %s',fname)
  % some specific changes for ASCII file input definitions which are
  % different than RT interface (sorry)
  
  q = 1./data.iota(iit,:); q(isinf(q))=1e10*sign(q(isinf(q))); % another adhoc fix
  psi1d = data.psi1d_n(iit,:)*(data.Psi_sep(iit)-data.Psi_axis(iit))+data.Psi_axis(iit);
  
  nR = numel(params.R);
  nZ = numel(params.Z);
  PsiRZ = reshape(data.Psi(:,:,iit),[],1);
  
  fprintf(fid,'%d ',nR); fprintf(fid,'\n');
  fprintf(fid,'%d ',nZ); fprintf(fid,'\n');
  
  %2D
  fprintf(fid,'%15.10e ',params.R); fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',params.Z); fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',PsiRZ);  fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',-ones(numel(PsiRZ),1)); fprintf(fid,'\n'); % was rhotorN(2D) (obsolete)
  
  % profiles
  nr = size(data.psi1d_n,2);
  fprintf(fid,'%d ',nr); fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',psi1d);        fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Volume(iit,:));  fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Area(iit,:));    fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.rhotor1d(iit,:)); fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',q);            fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.F(iit,:));       fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Psi_sep(iit,:));    fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Psi_axis(iit,:));    fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',0);            fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Rmag(iit,:));     fprintf(fid,'\n');
  fprintf(fid,'%15.10e ',data.Zmag(iit,:));     fprintf(fid,'\n');
  
  fclose(fid);
end
end

function write_beam_data_file(params,path)

fname = fullfile(path,'beams.dat');
fprintf('   writing %s\n',fname)
fid = fopen(fname,'w+'); assert(fid~=-1,'error opening file %s',fname)

fprintf(fid, '# no. of sources:\n');
fprintf(fid, '%10d\n',params.nsource);
fprintf(fid, '# nv:\n');
fprintf(fid, '%10d\n',3);
fprintf(fid, '# start pos: [m]\n');
fprintf(fid, '%15.8f %15.8f %15.8f\n',params.xstart);
fprintf(fid, '# beam unit vector:\n');
fprintf(fid, '%15.8f %15.8f %15.8f\n',params.xvec);
fprintf(fid, '# beam-width-polynomial coefficients:\n');
fprintf(fid, '%15.8f %15.8f %15.8f\n',params.beamwidthpoly);
fprintf(fid, '# Injection energy [eV]:\n');
fprintf(fid, '%15.8f\n ',-1*ones(params.nsource,1));
fprintf(fid, '# Particle fraction of full/half/third energy:\n');
fprintf(fid, '%15.8f %15.8f %15.8f\n',-1*ones(params.nsource,3));
fprintf(fid, '# A beam [u]\n');
fprintf(fid, '%15.3f\n',params.abeam);

fclose(fid);
end


function write_timevarying_data_file(data,params,path)

%%
fname = fullfile(path,'timetraces.dat');
fprintf('   writing %s\n',fname)
fid = fopen(fname,'w+'); assert(fid~=-1,'error opening file %s',fname)

nt = numel(data.time);


%%
fprintf(fid,'%d\n',nt);
fprintf(fid,'%d\n',params.p);
fprintf(fid,'%10s\n','rho_tor');
fprintf(fid,'%15.8f\n',data.time);
fprintf(fid,'%15.8f ',data.rhotor_kinprof(1,:)); fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.Te*1e-3);fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.Ti*1e-3);fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.ne*1e-6);fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.omega_tor); fprintf(fid,'\n');% check omegator/v_tor?
fprintf(fid,'%15.8e ',data.Zeff);fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.Pinj);
fprintf(fid,'--- Pinj ends ---\n');
fprintf(fid,' #no. of sources:\n');
fprintf(fid,'%d\n',params.nsource);
fprintf(fid,'# nv:\n');
fprintf(fid,'%d\n',params.nv);
fprintf(fid,'# nv stored in file:\n');
fprintf(fid,'%d\n',2);
fprintf(fid,'Time-dependent Einj and specmix follow:\n');
fprintf(fid,'%15.8e ',data.Einj); fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.specmix(1,:)); fprintf(fid,'\n');
fprintf(fid,'%15.8e ',data.specmix(2,:)); fprintf(fid,'\n');
fclose(fid);

%
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

function write_curent_namelist(dir)

% current namelist namelist_usr points to some .nml
% Copy it to the data folder

%%
[s,w] = unix('readlink ${RABBIT_PATH}/inputs/namelist_used');
assert(~s,'error reading link: %s',w);
src = deblank(w);
dest = fullfile(dir,'namelist.nml');
cmd = sprintf('cp %s %s',src,dest);
disp(cmd);
s = unix(cmd);
assert(~s,'error copying');
end
