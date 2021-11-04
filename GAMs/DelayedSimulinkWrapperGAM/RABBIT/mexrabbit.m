function mexrabbit(varargin)
% mexrabbit(mode)
% compiles MEX for RABBIT S-function.
%  options for mode: 'mock','debug','timing','production'

% go to current file's directory
mfiledir = fileparts(mfilename('fullpath'));
olddir = pwd;

% suppress warnings about gcc versions
set_warnings('off')

try
    % go to correct dir
    cd(mfiledir);
    
    % get desired mex mode
    mexmode = handle_inputs(varargin{:});
    
    % checks
    check_environment(mexmode)
    
    % cleanup
    delete_old_mex()
    
    % get mex command for this mode
    mexcmd = define_mexcmd(mexmode);
    
    % execute mex command
    disp(mexcmd);
    eval(mexcmd);
    cd(olddir)
    set_warnings('on')

catch ME
    cd(olddir)
    rethrow(ME)
    set_warnings('on')

end

end

function set_warnings(onoff)
warnID1 = 'MATLAB:mex:GccVersion';
warnID2 = 'MATLAB:mex:GccVersion_link';
warning(onoff,warnID1);
warning(onoff,warnID2);
end


function mexmode = handle_inputs(varargin)
if nargin > 0
  if strcmp(deblank(lower(varargin{1})),'mock')
    mexmode = 'mock';
  elseif strcmp(deblank(lower(varargin{1})),'debug')
    mexmode = 'debug';
  elseif strcmp(deblank(lower(varargin{1})),'timing')
    mexmode = 'timing';
  elseif strcmp(deblank(lower(varargin{1})),'production')
    mexmode = 'production';
  else
    error('invalid mex mode %s for RABBIT wrapper',varargin{1});
  end
else
  mexmode = 'mock'; % default
end
end

function check_environment(mexmode)

RABBITpath =  getenv('RABBIT_PATH');
LD_LIBRARY_path =  getenv('LD_LIBRARY_PATH');
IMPIpath =  getenv('IMPI_PATH');

fprintf('Environment variables:\n');
fprintf('   RABBIT_PATH=%s\n',RABBITpath);
fprintf('   IMPI_PATH=%s\n',IMPIpath);
fprintf('   LD_LIBRARY_PATH=%s\n',LD_LIBRARY_path);

switch mexmode
    case 'mock'
        % linking stuff not necessary
        return
    otherwise
        assert(~isempty(RABBITpath),'RABBIT_PATH environment variable must be defined');
        assert(~isempty(strfind(LD_LIBRARY_path,RABBITpath)),'LD_LIBRARY_PATH must contain $RABBIT_PATH/ in link');
        assert(~isempty(strfind(LD_LIBRARY_path,IMPIpath)),'LD_LIBRARY_PATH must contain $IMPI_PATH/ in link');
end

end

function delete_old_mex

if exist('RABBIT_sfun.mexa64','file')
  !rm RABBIT_sfun.mexa64
end
end


function mexcmd = define_mexcmd(mexmode)

mextoolspath = fullfile(fileparts(mfilename('fullpath')),'mextools'); % path to TCV_RT lib

mexbasestr = sprintf('mex src/RABBIT_sfun.c -I./src -I%s',mextoolspath); % basic path for stuff for the wrapper.
mexrabbitlibstr = get_mexrabbitstring(mexmode);

fprintf('\n\n');
switch mexmode
  case 'mock'
    disp('mexing RABBIT wrapper in mock mode, no link to libRabbit');
    mexcmd = sprintf('%s -v -g -DMOCK -DVERBOSE -DTIMER',mexbasestr);
  case 'debug'
    disp('mexing RABBIT wrapper in debug mode');
    mexcmd = sprintf('%s %s -v -g -DVERBOSE -DTIMER',mexbasestr,mexrabbitlibstr);
  case 'timing'
    disp('mexing RABBIT wrapper in execution timing mode');
    mexcmd = sprintf('%s %s -DTIMER',mexbasestr,mexrabbitlibstr);
  case 'production'
    disp('mexing RABBIT wrapper in production mode');
    mexcmd = sprintf('%s %s',mexbasestr,mexrabbitlibstr);
  otherwise
    error('invalid mode');
end
end


%%
function str = get_mexrabbitstring(mexmode)

switch mexmode
    case {'debug'}
        libRabbit = 'libRabbitMore_debug.so';
    otherwise
        libRabbit = 'libRabbitMore.so';
end
%libRabbit = 'libRabbitMore.so'; % optional override (uncomment)
%libRabbit = 'libRabbitMore_debug.so';

sstr = ['${RABBIT_PATH}/lib/%s '...
  'CXXFLAGS="$CXXFLAGS -fopenmp -std=c++11" ' ...
  'LDFLAGS="$LDFLAGS -fopenmp"'];

str = sprintf(sstr,libRabbit);
%%

end
