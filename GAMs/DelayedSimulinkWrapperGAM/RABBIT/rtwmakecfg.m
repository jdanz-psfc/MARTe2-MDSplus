function makeInfo = rtwmakecfg

[includePath,sourcePath,sourceFiles] = get_RABBIT_buildenv;


disp(['Running rtwmakecfg from folder: ',pwd]);
    makeInfo.includePath = includePath;
    makeInfo.sourcePath = sourcePath;
    makeInfo.sources  = sourceFiles;
    makeInfo.linkLibsObjs = {}; % libraryFiles;
    
    makeInfo.precompile = true;
    
    %makeInfo.library(1).Name     = 'myprecompiledlib';
    %makeInfo.library(1).Location = fullfile(pwd,'somdir2','lib');
    %makeInfo.library(1).Modules  = {'srcfile1' 'srcfile2' 'srcfile3' };
