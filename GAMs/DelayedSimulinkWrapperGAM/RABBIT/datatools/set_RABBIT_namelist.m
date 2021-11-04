function set_RABBIT_namelist(namelist)
% set namelist to symbolic link to given namelist path

RABBITpath = getenv('RABBIT_PATH');
if isempty(RABBITpath)
  RABBITpath = fullfile(fileparts(mfilename('fullpath')),'..');
end

if ~exist(namelist,'file');
    % is only filename is given, check if it is in /inputs
    namelistpath = fullfile(RABBITpath,'inputs',namelist);
    assert(~~exist(namelistpath,'file'),'Neither %s nor %s exist!',namelist, namelistpath);
else
    namelistpath = namelist;
end

namelistlink = fullfile(RABBITpath,'inputs','namelist_used');

cmd = sprintf('ln -sf %s %s',namelistpath,namelistlink); % ---symbolic, --force

fprintf('\nSetting RABBIT namelist link: \n   %s\n\n',cmd)
[s,w] = unix(cmd);
assert(s==0,'error: %s',w);