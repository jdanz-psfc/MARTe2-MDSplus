function plot_RABBIT_output(logsout,time)
% plot_RABBIT_output(logsout,time)

%% time plots
signal_list = {'powe_total','powi_total'...
    'Pshine','Prot','Porbitloss','Pcxloss',...
    'Inb_total','exitflag'};

data = get_Dataset_data(logsout,signal_list,[],'RABBIT');

figure('name','RABBIT output time traces','position',[150 150 1000 500]);
plot_time(data,time);

%% profile plots
signal_list = {'powe','powi','press','bdep','bdens','jfi',...
    'jnbcd','torqe','torqi','torqimp',...
    'nrate','rhotor_out'};

% get signals only at specified times
data = get_Dataset_data(logsout,signal_list,time,'RABBIT');

figure('name','RABBIT output profiles','position',[50 50 1200 500]);
plot_profiles(data)
end
%%
function plot_time(data,time)
% later add marker for t=time;
%%
nbeams = size(data.powe_total,2);

for ib = 1:nbeams % for each beam
    %%
    if ismatrix(data.powe_total)
        dd=[data.powe_total,data.powi_total,...
        data.Prot,data.Pshine,data.Porbitloss,...
        data.Pcxloss]'/1e6;
    else
    dd = squeeze([data.powe_total(:,ib,:),data.powi_total(:,ib,:),...
        data.Prot(:,ib,:),data.Pshine(:,ib,:),data.Porbitloss(:,ib,:),...
        data.Pcxloss(:,ib,:)])/1e6;
    end
    
    cdd = cumsum(flipud(dd));
    
    ncurves = size(cdd,1);

    %% total power plot
    ax=subplot(nbeams,3,(ib-1)*3+1);
    colors = get_cols;
    for ii=1:ncurves
        C = colors(ii,:);
    plot(ax,data.time,dd(ii,:),'color',C);
    hold(ax,'on');
    end
    ylabel(ax,sprintf('Beam: %d, P [MW]',ib)); xlabel(ax,'time [s]');
    legend(ax,{'el','ion','rot','shine','orbit','cx'},'box','off')
    
    %% cumsum plot
    ax=subplot(nbeams,3,(ib-1)*3+2);
    zz = zeros(1,ncurves);
    tt = [data.time(1);data.time;data.time(end)];
    yy = [zz;cdd';zz];
    
    colors = flipud(get_cols());
    for ii=ncurves:-1:1
        C = colors(ii,:);
        hp(ii)=patch(tt,yy(:,ii),C,'parent',ax);
        hold(ax,'on')
        ylabel(ax,'Cumulative P [MW]'); xlabel(ax,'time [s]');
    end
    legend(fliplr(hp),{'el','ion','rot','pshine','orbit','cx'},'box','off')
    
    %% CD plot
    ax=subplot(nbeams,3,(ib-1)*3+3);
    plot(ax,data.time,squeeze(data.Inb_total)/1e3);
    ylabel(ax,'Icd [kA]'); xlabel(ax,'time[s]');
end
end

function cols = get_cols()

cols = [0.2 0.2 1
    0 0 0.4
    0.5 0.5 0
    0 0.5 0
    0 0 0
    1 0 0];

end

function plot_profiles(data)

fields = fieldnames(data);
nfields = numel(fields);
nplot = nfields - 1;
nrow = 2; ncol = ceil(nplot/nrow);
iplot = 1;
nbeams = size(data.powe,2);

for ii=1:nfields
    myfield = fields{ii};
    if strcmp(myfield,'rhotor_out') || strcmp(myfield,'time')
        continue
    end
    hax(ii)=subplot(nrow,ncol,iplot);
    ax = hax(ii);
    plot(ax,data.rhotor_out,data.(myfield))
    xlabel(ax,'\rho_{tor}');
    ylabel(ax,myfield);
    iplot = iplot+1;
end

for ib = 1:nbeams
    legstr{ib} = sprintf('beam %d\n',ib);
end

legend(hax(1),legstr,'box','off','location','best');
title(hax(1),sprintf('time = %2.2f',data.time));

end