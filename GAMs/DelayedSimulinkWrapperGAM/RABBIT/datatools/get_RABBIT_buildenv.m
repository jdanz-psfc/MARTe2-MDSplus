function [includePath,sourcePath,sourceFiles] = get_RABBIT_buildenv

RABBITpath = getenv('RABBIT_PATH');
IMPIpath = getenv('IMPI_PATH');

assert(~isempty(RABBITpath),'$RABBIT_PATH environment variable must be defined to run compilation tests');
assert(~isempty(IMPIpath),'$RABBIT_PATH environment variable must be defined to run compilation tests');

includePath = {fullfile(RABBITpath,'src'),...
    fullfile(RABBITpath,'mextools'),...
    fullfile(RABBITpath,'lib')};
sourcePath = {fullfile(RABBITpath,'src')};
sourceFiles = {'RABBIT_sfun.c'};

end
