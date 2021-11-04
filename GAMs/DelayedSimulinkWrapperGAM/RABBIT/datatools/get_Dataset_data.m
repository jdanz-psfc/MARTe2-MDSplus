function data = get_Dataset_data(DataSet,signals,time,varargin)
% function data = get_DataSet_data(logsout,signals,time,{blockPathStr})
%
% get DataSet snapshot for specified signals at given time values.
%     logsout: logging object from Simulink simulation (Dataset format)
%     signals: list of signals to get (cell of strings)
%     time: time vector to get signal. Empty: all times
%     
%
% Optional BlockPathStr can be used to choose only signals where the blockpath
% contains BlockPathStr, to resolve conflicts in case multiple signals with
% the same name exist.

% input checks
if numel(varargin)>=1
  blockPathStr = varargin{1};
  assert(ischar(blockPathStr));
else
  blockPathStr = '';
end
assert(isa(DataSet,'Simulink.SimulationData.Dataset'),'Dataset is not a Simulink.SimulationData.Dataset')
assert(iscell(signals),'signals must be cell array of signal name strings')
assert(isnumeric(time) | isempty(time),'time must be a number, or empty');

% get signals
for isig=1:numel(signals);
  mysigname = signals{isig};
  
  myts = get_timeseries(DataSet,mysigname,blockPathStr);
  
  if isig==1
    timebasis = myts.Time; % store first time basis
    if isempty(time)
        % take all
        it = 1:numel(myts.Time);
    else
        % time specified
        it = find_time_indices(myts.Time,time);
    end
  else
    assert(same_timebasis(myts.Time,timebasis),...
        'not all signals are on the same time basis! Present signal: %s',mysigname)
  end
  if myts.isTimeFirst
  data.(mysigname) = myts.Data(it,:);
  else
      if ndims(myts.Data==3)
          data.(mysigname) = myts.Data(:,:,it);
      else
          data.(mysigname) = myts.Data(:,it);
      end
  end
end
data.time = timebasis(it);

end

function mySignal = findUniqueSignal(mySignal,blockPathStr)
% multiple signals with the same name were found.
% Use BlockPathString to filter
mysigname=mySignal.getElement(1).Name;

if isempty(blockPathStr)
  error('multiple instances of signal name ''%s'' found, must supply blockPathStr to disginguish them',...
    mysigname)
end

matchblock = false(mySignal.numElements,1); % init
for ii=1:mySignal.numElements
  blockPath = mySignal.getElement(ii).BlockPath.convertToCell;
  matchblock(ii) = any(~isempty(cell2mat(strfind(blockPath,blockPathStr))));
end
assert(sum(matchblock)==1,...
  'multiple signal names %s found with BlockPath matching %s, can not extract unique signal',mysigname,blockPathStr);
mySignal = mySignal.getElement(find(matchblock,1,'first'));
end

function it = find_time_indices(timeVector,reqTime)

% check time
assert(all(reqTime>=timeVector(1)) && all(reqTime<=timeVector(end)),...
  'Time out of range.\n  Requested times: \n%3.3f\n  Data time window:\n [%3.3f,%3.3f]',...
  reqTime,timeVector(1),timeVector(end));

if numel(reqTime)==1
  it = find(timeVector>=reqTime,1,'first');
elseif numel(reqTime)==2
  itstart = find(timeVector>=reqTime(1),1,'first');
  itend = find(timeVector<=reqTime(end),1,'last');
  it = itstart:itend;
else
  error('time must be a scalar (snapshot), or two values (start-end time)');
end

end

function myts = get_timeseries(DataSet,mysigname,blockPathStr)

try
  mySignal = DataSet.getElement(mysigname);
  if isa(mySignal,'Simulink.SimulationData.Dataset')
    mySignal = findUniqueSignal(mySignal,blockPathStr);
  end
  myts = mySignal.Values;
catch ME
  warning('Could not get %s',mysigname);
  rethrow(ME)
end
end

function arethesame=same_timebasis(time1,time2)
if numel(time1)~=numel(time2)
    arethesame = false;
elseif ~all(time1==time2)
    arethesame = false;
else
    arethesame=true;
end
end